import logging
import uuid
from contextlib import asynccontextmanager
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware

from config import get_settings
from models.schemas import (
    DocumentRequest, DocumentResponse,
    SearchRequest, SearchResponse,
    HealthResponse
)
from services.embedding_service import get_embedding, check_ollama_model
from services.opensearch_service import (
    index_document, search_similar_documents,
    check_opensearch_health, check_index_exists
)
from services.generation_service import generate_answer, check_generation_model

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)
settings = get_settings()

@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info("=== Запуск RAG-сервиса ===")
    logger.info(f"OpenSearch: {settings.opensearch_host}")
    logger.info(f"Ollama: {settings.ollama_host}")
    logger.info(f"Embedding: {settings.embedding_model}")
    logger.info(f"Generation: {settings.generation_model}")
    logger.info(f"Index: {settings.index_name}")
    
    index_ok = check_index_exists()
    if not index_ok:
        logger.warning(f"Индекс '{settings.index_name}' не найден!")
    else:
        logger.info(f"Индекс '{settings.index_name}' готов")
    yield
    logger.info("=== Остановка RAG-сервиса ===")

app = FastAPI(
    title="RAG Service",
    description="RAG-сервис на базе Ollama + OpenSearch",
    version="1.0.0",
    lifespan=lifespan
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.get("/", tags=["System"])
async def root():
    return {
        "service": "RAG Service",
        "version": "1.0.0",
        "docs": "http://localhost:8000/docs"
    }

@app.get("/health", response_model=HealthResponse, tags=["System"])
async def health_check():
    os_status = check_opensearch_health()
    embedding_ok = await check_ollama_model(settings.embedding_model)
    generation_ok = await check_generation_model()
    index_ok = check_index_exists()
    
    overall = "healthy" if os_status in ("green", "yellow") and embedding_ok and generation_ok and index_ok else "degraded"
    return HealthResponse(
        status=overall,
        opensearch=os_status,
        ollama_embedding="available" if embedding_ok else "unavailable",
        ollama_generation="available" if generation_ok else "unavailable",
        index_exists=index_ok
    )

@app.post("/documents", response_model=DocumentResponse, tags=["Documents"])
async def add_document(request: DocumentRequest):
    try:
        logger.info(f"Индексирование документа, длина: {len(request.content)}")
        
        embedding = await get_embedding(request.content)
        logger.info(f"Эмбеддинг получен, размерность: {len(embedding)}")
        
        doc_id = request.document_id or str(uuid.uuid4())
        
        result = index_document(
            content=request.content,
            embedding=embedding,
            document_id=doc_id,
            chunk_index=request.chunk_index or 0,
            document_date=request.document_date,
            classifiers=request.classifiers,
            doc_class=request.doc_class,
            summary=request.summary,
            note=request.note,
            sender_full=request.sender_full,
            model=request.model
        )
        
        opensearch_id = result.get("_id", "unknown")
        logger.info(f"Документ сохранён: {opensearch_id}")
        
        return DocumentResponse(
            success=True,
            document_id=doc_id,
            opensearch_id=opensearch_id,
            message=f"Документ проиндексирован. Размерность: {len(embedding)}"
        )
    except Exception as e:
        logger.error(f"Ошибка: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/search", response_model=SearchResponse, tags=["Search"])
async def search_and_answer(request: SearchRequest):
    try:
        top_k = request.top_k or settings.top_k_results
        top_m = request.top_m or settings.top_m_context
        top_m = min(top_m, top_k)
        
        logger.info(f"Вопрос: '{request.question[:80]}...' | k={top_k}, m={top_m}")
        
        query_embedding = await get_embedding(request.question)
        logger.info(f"Эмбеддинг вопроса: {len(query_embedding)}")
        
        all_documents = search_similar_documents(
            query_embedding=query_embedding,
            top_k=top_k
        )
        logger.info(f"Найдено: {len(all_documents)}")
        
        if not all_documents:
            return SearchResponse(
                question=request.question,
                answer="По вашему запросу не найдено документов.",
                relevant_documents=[],
                documents_used_count=0,
                embedding_model=settings.generation_model
            )
        
        context_documents = all_documents[:top_m]
        
        answer = await generate_answer(
            question=request.question,
            documents=context_documents
        )
        
        return SearchResponse(
            question=request.question,
            answer=answer,
            relevant_documents=all_documents,
            documents_used_count=len(context_documents),
            embedding_model=settings.generation_model
        )
    except Exception as e:
        logger.error(f"Ошибка: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))
