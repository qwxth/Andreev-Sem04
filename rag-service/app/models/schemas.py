from pydantic import BaseModel, Field, ConfigDict
from typing import Optional, List

class DocumentRequest(BaseModel):
    content: str = Field(..., description="Текст документа", min_length=1)
    document_id: Optional[str] = Field(None, description="ID документа")
    chunk_index: Optional[int] = Field(0, description="Индекс чанка")
    document_date: Optional[str] = Field(None, description="Дата документа YYYY-MM-DD")
    classifiers: Optional[List[str]] = Field(default_factory=list)
    doc_class: Optional[str] = Field(None, description="Класс документа")
    summary: Optional[str] = Field(None, description="Краткое содержание")
    note: Optional[str] = Field(None, description="Примечание")
    sender_full: Optional[str] = Field(None, description="Отправитель")
    model: Optional[str] = Field(None, description="Модель")

    model_config = ConfigDict(json_schema_extra={
        "example": {
            "content": "Строительная экспертиза — это комплекс мероприятий...",
            "document_id": "doc-001",
            "chunk_index": 0,
            "doc_class": "regulatory",
            "summary": "Определение термина строительная экспертиза"
        }
    })

class DocumentResponse(BaseModel):
    success: bool
    document_id: str
    opensearch_id: str
    message: str

class SearchRequest(BaseModel):
    question: str = Field(..., description="Вопрос пользователя", min_length=1)
    top_k: Optional[int] = Field(None, description="Количество документов для поиска (n)")
    top_m: Optional[int] = Field(None, description="Количество документов для генерации (m)")

    model_config = ConfigDict(json_schema_extra={
        "example": {
            "question": "Как определяется термин Строительная экспертиза в документах?",
            "top_k": 5,
            "top_m": 3
        }
    })

class RelevantDocument(BaseModel):
    opensearch_id: str
    document_id: Optional[str]
    content: str
    score: float
    chunk_index: Optional[int]
    doc_class: Optional[str]
    summary: Optional[str]

class SearchResponse(BaseModel):
    question: str
    answer: str
    relevant_documents: List[RelevantDocument]
    documents_used_count: int
    embedding_model: str  # переименовано с model_used
    
    model_config = ConfigDict(protected_namespaces=())

class HealthResponse(BaseModel):
    status: str
    opensearch: str
    ollama_embedding: str
    ollama_generation: str
    index_exists: bool
