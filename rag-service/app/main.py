import logging
import uuid
from contextlib import asynccontextmanager
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse

from config import get_settings
from models.schemas import DocumentRequest, SearchRequest, ChatRequest
from models.session_models import create_session, get_session, delete_session
from services.embedding_service import get_embedding, check_ollama_model
from services.opensearch_service import (
    index_document, search_similar_documents,
    check_opensearch_health, check_index_exists
)
from services.generation_service import (
    generate_answer, generate_chat_answer, check_generation_model
)

logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")
logger = logging.getLogger(__name__)
settings = get_settings()

@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info("=== RAG-сервис v2 запущен ===")
    yield

app = FastAPI(title="RAG Service v2", version="2.0", lifespan=lifespan)
app.add_middleware(CORSMiddleware, allow_origins=["*"], allow_methods=["*"], allow_headers=["*"])

@app.get("/")
async def root():
    return {
        "service": "RAG Service v2",
        "endpoints": {
            "POST /documents":      "загрузить документ",
            "POST /search":         "найти документы + ответ → session_id + document_ids",
            "POST /chat":           "продолжить диалог (нужен session_id)",
            "DELETE /session/{id}": "удалить сессию = начать заново",
            "GET /session/{id}":    "история сессии + список документов"
        }
    }

@app.get("/health")
async def health():
    os_status = check_opensearch_health()
    emb_ok = await check_ollama_model(settings.embedding_model)
    gen_ok = await check_generation_model()
    idx_ok = check_index_exists()
    status = "healthy" if os_status in ("green","yellow") and emb_ok and gen_ok and idx_ok else "degraded"
    return {"status": status, "opensearch": os_status,
            "embedding": "ok" if emb_ok else "fail",
            "generation": "ok" if gen_ok else "fail",
            "index": idx_ok}

@app.post("/documents")
async def add_document(req: DocumentRequest):
    try:
        embedding = await get_embedding(req.content)
        doc_id = req.document_id or str(uuid.uuid4())
        result = index_document(
            content=req.content, embedding=embedding, document_id=doc_id,
            chunk_index=req.chunk_index, document_date=req.document_date,
            classifiers=req.classifiers, doc_class=req.doc_class,
            summary=req.summary, note=req.note, sender_full=req.sender_full,
            model=req.model
        )
        return {"success": True, "document_id": doc_id,
                "opensearch_id": result.get("_id","?"),
                "message": f"OK, vector={len(embedding)}d"}
    except Exception as e:
        logger.error(f"Error: {e}")
        raise HTTPException(500, str(e))

@app.post("/search")
async def search(req: SearchRequest):
    """Поиск + первый ответ + создание сессии диалога"""
    try:
        top_k = req.top_k or settings.top_k_results
        top_m = req.top_m or settings.top_m_context
        top_m = min(top_m, top_k)

        query_emb = await get_embedding(req.question)
        docs = search_similar_documents(query_emb, top_k)

        if not docs:
            sid = str(uuid.uuid4())
            create_session(sid, [])
            return {
                "question": req.question,
                "answer": "Документы не найдены",
                "relevant_documents": [],
                "documents_used_count": 0,
                "session_id": sid,
                "document_ids": []
            }

        context_docs = docs[:top_m]
        answer = await generate_answer(req.question, context_docs)

        sid = str(uuid.uuid4())
        session = create_session(sid, context_docs)
        session.add_message("user", req.question)
        session.add_message("assistant", answer)

        # ID документов в контексте сессии
        doc_ids = [d.document_id or d.opensearch_id for d in context_docs]

        logger.info(f"Сессия создана: {sid}, документы: {doc_ids}")

        return {
            "question": req.question,
            "answer": answer,
            "relevant_documents": [d.model_dump() for d in docs],
            "documents_used_count": len(context_docs),
            "session_id": sid,
            "document_ids": doc_ids  # СПИСОК ID ДОКУМЕНТОВ В СЕССИИ
        }
    except Exception as e:
        logger.error(f"Search error: {e}", exc_info=True)
        raise HTTPException(500, str(e))

@app.post("/chat")
async def chat(req: ChatRequest):
    """Продолжение диалога в контексте документов сессии"""
    try:
        session = get_session(req.session_id)
        if not session:
            raise HTTPException(404,
                f"Сессия '{req.session_id}' не найдена. "
                f"Начните новый поиск через POST /search")

        answer = await generate_chat_answer(
            documents=session.documents,
            history=session.history,
            message=req.message
        )

        session.add_message("user", req.message)
        session.add_message("assistant", answer)

        # ID документов текущей сессии
        doc_ids = [d.document_id or d.opensearch_id for d in session.documents]

        logger.info(f"Диалог {req.session_id}: сообщений={len(session.history)}")

        return JSONResponse(content={
            "answer": answer,
            "session_id": req.session_id,
            "document_ids": doc_ids,  # СПИСОК ID ДОКУМЕНТОВ В СЕССИИ
            "history": [{"role": m.role, "content": m.content} for m in session.history]
        })
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Chat error: {e}", exc_info=True)
        raise HTTPException(500, str(e))

@app.delete("/session/{session_id}")
async def end_session(session_id: str):
    ok = delete_session(session_id)
    if not ok:
        raise HTTPException(404, f"Сессия '{session_id}' не найдена")
    return {"message": f"Сессия {session_id} удалена",
            "next": "Используйте POST /search для нового поиска"}

@app.get("/session/{session_id}")
async def get_session_info(session_id: str):
    session = get_session(session_id)
    if not session:
        raise HTTPException(404, "Сессия не найдена")
    
    doc_ids = [d.document_id or d.opensearch_id for d in session.documents]
    
    return {
        "session_id": session_id,
        "documents_count": len(session.documents),
        "document_ids": doc_ids,
        "messages_count": len(session.history),
        "history": [{"role": m.role, "content": m.content} for m in session.history]
    }
