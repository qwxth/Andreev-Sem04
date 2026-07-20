import logging
from typing import List, Optional, Dict, Any
from opensearchpy import OpenSearch
from config import get_settings
from models.schemas import RelevantDocument

logger = logging.getLogger(__name__)
settings = get_settings()

def get_opensearch_client() -> OpenSearch:
    return OpenSearch(
        hosts=[settings.opensearch_host],
        http_auth=(settings.opensearch_user, settings.opensearch_password),
        use_ssl=True,
        verify_certs=False,
        ssl_show_warn=False,
        timeout=60,
        retry_on_timeout=True,
        max_retries=3
    )

def check_opensearch_health() -> str:
    client = get_opensearch_client()
    try:
        health = client.cluster.health()
        return health.get("status", "unknown")
    except Exception as e:
        logger.error(f"Ошибка проверки OpenSearch: {e}")
        return "unavailable"

def check_index_exists() -> bool:
    client = get_opensearch_client()
    try:
        return client.indices.exists(index=settings.index_name)
    except Exception as e:
        logger.error(f"Ошибка проверки индекса: {e}")
        return False

def index_document(
    content: str,
    embedding: List[float],
    document_id: Optional[str] = None,
    chunk_index: int = 0,
    document_date: Optional[str] = None,
    classifiers: Optional[List[str]] = None,
    doc_class: Optional[str] = None,
    summary: Optional[str] = None,
    note: Optional[str] = None,
    sender_full: Optional[str] = None,
    model: Optional[str] = None,
) -> Dict[str, Any]:
    client = get_opensearch_client()
    try:
        doc_body: Dict[str, Any] = {
            "content": content,
            "embedding": embedding,
            "chunkIndex": chunk_index,
            "model": model or settings.embedding_model,
        }
        if document_id:
            doc_body["documentId"] = document_id
        if document_date:
            doc_body["documentDate"] = document_date
        if classifiers:
            doc_body["classifiers"] = classifiers
        if doc_class:
            doc_body["docclass"] = doc_class
        if summary:
            doc_body["summary"] = summary
        if note:
            doc_body["note"] = note
        if sender_full:
            doc_body["senderfull"] = sender_full

        index_kwargs: Dict[str, Any] = {
            "index": settings.index_name,
            "body": doc_body,
            "refresh": True
        }
        if document_id:
            index_kwargs["id"] = document_id

        response = client.index(**index_kwargs)
        logger.info(f"Документ проиндексирован: {response['_id']}")
        return response
    except Exception as e:
        logger.error(f"Ошибка индексирования: {e}")
        raise

def search_similar_documents(
    query_embedding: List[float],
    top_k: int = 5
) -> List[RelevantDocument]:
    client = get_opensearch_client()
    try:
        search_body = {
            "size": top_k,
            "query": {
                "knn": {
                    "embedding": {
                        "vector": query_embedding,
                        "k": top_k
                    }
                }
            },
            "_source": {
                "includes": [
                    "content", "documentId", "chunkIndex",
                    "docclass", "summary", "documentDate",
                    "senderfull", "note", "classifiers"
                ]
            }
        }
        response = client.search(index=settings.index_name, body=search_body)
        hits = response["hits"]["hits"]
        logger.info(f"Найдено документов по kNN: {len(hits)}")

        results = []
        for hit in hits:
            source = hit.get("_source", {})
            results.append(RelevantDocument(
                opensearch_id=hit["_id"],
                document_id=source.get("documentId"),
                content=source.get("content", ""),
                score=hit["_score"],
                chunk_index=source.get("chunkIndex"),
                doc_class=source.get("docclass"),
                summary=source.get("summary")
            ))
        return results
    except Exception as e:
        logger.error(f"Ошибка поиска документов: {e}")
        raise
