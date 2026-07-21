from pydantic import BaseModel, Field, ConfigDict
from typing import Optional, List


class DocumentRequest(BaseModel):
    content: str = Field(..., min_length=1)
    document_id: Optional[str] = None
    chunk_index: Optional[int] = Field(default=0, ge=0)
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
    document_id: Optional[str] = None
    content: str
    score: float
    chunk_index: Optional[int] = None
    doc_class: Optional[str] = None
    summary: Optional[str] = None


class SearchRequest(BaseModel):
    question: str = Field(..., min_length=1)
    top_k: Optional[int] = Field(default=None, ge=1, le=100)
    top_m: Optional[int] = Field(default=None, ge=1, le=50)


class SearchResponse(BaseModel):
    question: str
    answer: str
    relevant_documents: List[RelevantDocument]
    documents_found: int
    documents_used_count: int
    session_id: str
    document_ids: List[str]


class ChatRequest(BaseModel):
    session_id: str = Field(..., description="ID сессии из /search")
    message: str = Field(..., min_length=1)


class ChatMessage(BaseModel):
    role: str
    content: str


class ChatResponse(BaseModel):
    answer: str
    session_id: str
    document_ids: List[str]
    history: List[ChatMessage]


class HealthResponse(BaseModel):
    status: str
    opensearch: str
    embedding: str
    generation: str
    index: bool
