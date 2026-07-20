from pydantic import BaseModel, Field, ConfigDict
from typing import Optional, List

class DocumentRequest(BaseModel):
    content: str = Field(..., min_length=1)
    document_id: Optional[str] = None
    chunk_index: Optional[int] = 0
    document_date: Optional[str] = None
    classifiers: Optional[List[str]] = Field(default_factory=list)
    doc_class: Optional[str] = None
    summary: Optional[str] = None
    note: Optional[str] = None
    sender_full: Optional[str] = None
    model: Optional[str] = None

class DocumentResponse(BaseModel):
    success: bool
    document_id: str
    opensearch_id: str
    message: str

class RelevantDocument(BaseModel):
    opensearch_id: str
    document_id: Optional[str]
    content: str
    score: float
    chunk_index: Optional[int]
    doc_class: Optional[str]
    summary: Optional[str]

class SearchRequest(BaseModel):
    question: str = Field(..., min_length=1)
    top_k: Optional[int] = None
    top_m: Optional[int] = None

class SearchResponse(BaseModel):
    question: str
    answer: str
    relevant_documents: List[RelevantDocument]
    documents_used_count: int
    session_id: str  # ID сессии для продолжения диалога

class ChatRequest(BaseModel):
    session_id: str = Field(..., description="ID сессии из /search")
    message: str = Field(..., min_length=1)

class ChatMessage(BaseModel):
    role: str
    content: str

class ChatResponse(BaseModel):
    answer: str
    session_id: str
    history: List[ChatMessage]

class HealthResponse(BaseModel):
    status: str
    opensearch: str
    ollama_embedding: str
    ollama_generation: str
    index_exists: bool
