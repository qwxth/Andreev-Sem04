import logging
import uuid
import json
from contextlib import asynccontextmanager
from typing import Optional
from fastapi import FastAPI, HTTPException, Query
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse

from config import get_settings
from models.schemas import DocumentRequest, SearchRequest, ChatRequest
from models.session_models import create_session, get_session, delete_session, save_session, list_sessions
from services.embedding_service import get_embedding, check_ollama_model
from services.opensearch_service import (
    index_document,
    delete_document,
    search_similar_documents,
    check_opensearch_health,
    check_index_exists,
    get_opensearch_client
)
from services.generation_service import (
    generate_answer, generate_chat_answer, check_generation_model
)

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(name)s - %(message)s"
)
logger = logging.getLogger(__name__)
settings = get_settings()

@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info("=== RAG Service Started ===")
    yield
    logger.info("=== RAG Service Stopped ===")

app = FastAPI(title="RAG Service", version="2.2", lifespan=lifespan)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"]
)

@app.get("/")
async def root():
    return {
        "service": "RAG Service v2.2",
        "endpoints": {
            "POST /documents": "upload document",
            "GET /documents": "list all documents",
            "DELETE /documents/{id}": "delete document",
            "POST /search": "search + create session",
            "POST /chat": "continue dialog",
            "GET /session/{id}": "session info",
            "GET /sessions": "all active sessions",
            "DELETE /session/{id}": "delete session",
            "GET /health": "health check"
        }
    }

@app.get("/health")
async def health():
    os_status = check_opensearch_health()
    emb_ok = await check_ollama_model(settings.embedding_model)
    gen_ok = await check_generation_model()
    idx_ok = check_index_exists()
    is_healthy = os_status in ("green", "yellow") and emb_ok and gen_ok and idx_ok
    return {
        "status": "healthy" if is_healthy else "degraded",
        "opensearch": os_status,
        "embedding": "ok" if emb_ok else "fail",
        "generation": "ok" if gen_ok else "fail",
        "index": idx_ok
    }

@app.post("/documents")
async def add_document(req: DocumentRequest):
    try:
        embedding = await get_embedding(req.content)
        doc_id = req.document_id or str(uuid.uuid4())
        result = index_document(
            content=req.content,
            embedding=embedding,
            document_id=doc_id,
            chunk_index=req.chunk_index or 0,
            document_date=req.document_date,
            classifiers=req.classifiers,
            doc_class=req.doc_class,
            summary=req.summary,
            note=req.note,
            sender_full=req.sender_full,
            model=req.model
        )
        return {
            "success": True,
            "document_id": doc_id,
            "opensearch_id": result.get("_id", "?"),
            "message": f"OK, vector={len(embedding)}d"
        }
    except Exception as e:
        logger.error(f"Error uploading document: {e}", exc_info=True)
        raise HTTPException(500, str(e))

@app.get("/documents")
async def list_documents(
    page: int = Query(default=1, ge=1),
    size: int = Query(default=20, ge=1, le=100),
    doc_class: Optional[str] = Query(default=None)
):
    try:
        client = get_opensearch_client()
        from_ = (page - 1) * size
        query = {"term": {"docclass": doc_class}} if doc_class else {"match_all": {}}
        body = {
            "from": from_,
            "size": size,
            "query": query,
            "_source": {"includes": ["documentId", "chunkIndex", "docclass", "summary", "documentDate", "model", "classifiers", "senderfull", "note"]},
            "sort": [{"_id": {"order": "asc"}}]
        }
        response = client.search(index=settings.index_name, body=body)
        hits = response["hits"]["hits"]
        total = response["hits"]["total"]["value"]
        docs = []
        for hit in hits:
            source = hit.get("_source", {})
            docs.append({
                "opensearch_id": hit["_id"],
                "document_id": source.get("documentId"),
                "chunk_index": source.get("chunkIndex"),
                "doc_class": source.get("docclass"),
                "summary": source.get("summary"),
                "document_date": source.get("documentDate"),
                "model": source.get("model"),
                "classifiers": source.get("classifiers"),
                "sender_full": source.get("senderfull"),
                "note": source.get("note")
            })
        total_pages = (total + size - 1) // size
        return {
            "total": total,
            "page": page,
            "size": size,
            "pages": total_pages,
            "has_next": page < total_pages,
            "has_prev": page > 1,
            "documents": docs
        }
    except Exception as e:
        logger.error(f"Error listing documents: {e}", exc_info=True)
        raise HTTPException(500, str(e))

@app.delete("/documents/{document_id}")
async def remove_document(document_id: str):
    try:
        result = delete_document(document_id)
        deleted = result.get("deleted", 0)
        if deleted == 0:
            raise HTTPException(404, f"Document '{document_id}' not found")
        return {"success": True, "deleted": deleted, "message": f"Document {document_id} deleted ({deleted} records)"}
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error deleting document {document_id}: {e}", exc_info=True)
        raise HTTPException(500, str(e))

@app.post("/search")
async def search(req: SearchRequest):
    try:
        top_k = req.top_k or settings.top_k_results
        top_m = req.top_m or settings.top_m_context
        top_m = min(top_m, top_k)
        query_emb = await get_embedding(req.question)
        docs = search_similar_documents(query_emb, top_k)
        sid = str(uuid.uuid4())
        if not docs:
            session = create_session(sid, [], req.question)
            save_session(session)
            return {"question": req.question, "answer": "No documents found", "relevant_documents": [], "documents_found": 0, "documents_used_count": 0, "session_id": sid, "document_ids": []}
        context_docs = docs[:top_m]
        answer = await generate_answer(req.question, context_docs)
        session = create_session(sid, context_docs, req.question)
        session.add_message("user", req.question)
        session.add_message("assistant", answer)
        save_session(session)
        doc_ids = [d.document_id or d.opensearch_id for d in context_docs]
        logger.info(f"Session {sid} created, documents: {doc_ids}")
        return {"question": req.question, "answer": answer, "relevant_documents": [d.model_dump() for d in docs], "documents_found": len(docs), "documents_used_count": len(context_docs), "session_id": sid, "document_ids": doc_ids}
    except Exception as e:
        logger.error(f"Search error: {e}", exc_info=True)
        raise HTTPException(500, str(e))

@app.post("/chat")
async def chat(req: ChatRequest):
    try:
        session = get_session(req.session_id)
        if not session:
            raise HTTPException(404, f"Session '{req.session_id}' not found. Start new search via POST /search")
        answer = await generate_chat_answer(documents=session.documents, history=session.history, message=req.message)
        session.add_message("user", req.message)
        session.add_message("assistant", answer)
        save_session(session)
        doc_ids = [d.document_id or d.opensearch_id for d in session.documents]
        logger.info(f"Dialog {req.session_id}: messages={len(session.history)}")
        return JSONResponse(content={"answer": answer, "session_id": req.session_id, "document_ids": doc_ids, "history": [{"role": m.role, "content": m.content} for m in session.history]})
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Chat error: {e}", exc_info=True)
        raise HTTPException(500, str(e))

@app.get("/session/{session_id}")
async def get_session_info(session_id: str):
    session = get_session(session_id)
    if not session:
        raise HTTPException(404, f"Session '{session_id}' not found")
    doc_ids = [d.document_id or d.opensearch_id for d in session.documents]
    return {"session_id": session_id, "created_at": session.created_at, "first_question": session.first_question, "documents_count": len(session.documents), "document_ids": doc_ids, "messages_count": len(session.history), "history": [{"role": m.role, "content": m.content} for m in session.history]}

@app.get("/sessions")
async def list_all_sessions():
    result = []
    session_ids = list_sessions()
    for sid in session_ids:
        session = get_session(sid)
        if session:
            doc_ids = [d.document_id or d.opensearch_id for d in session.documents]
            result.append({"session_id": sid, "created_at": session.created_at, "first_question": session.first_question, "documents_count": len(session.documents), "document_ids": doc_ids, "messages_count": len(session.history)})
    r = get_redis_client()
    return {"total_sessions": len(result), "sessions": result, "using_redis": r is not None}

@app.delete("/session/{session_id}")
async def end_session(session_id: str):
    ok = delete_session(session_id)
    if not ok:
        raise HTTPException(404, f"Session '{session_id}' not found")
    return {"message": f"Session {session_id} deleted", "next": "Use POST /search for new search"}
