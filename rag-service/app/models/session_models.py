import uuid
import logging
from datetime import datetime
from typing import Dict, List, Optional
from models.schemas import RelevantDocument, ChatMessage

logger = logging.getLogger(__name__)


class Session:
    def __init__(self, session_id: str, documents: List[RelevantDocument], first_question: str = ""):
        self.session_id = session_id
        self.documents = documents
        self.history: List[ChatMessage] = []
        self.created_at = datetime.now().isoformat()
        self.first_question = first_question

    def add_message(self, role: str, content: str):
        self.history.append(ChatMessage(role=role, content=content))


# in-memory хранилище (для продакшена заменить на Redis)
_sessions: Dict[str, Session] = {}


def create_session(session_id: str, documents: List[RelevantDocument], first_question: str = "") -> Session:
    _sessions[session_id] = Session(session_id, documents, first_question)
    logger.info(f"Создана сессия {session_id}, документов: {len(documents)}")
    return _sessions[session_id]


def get_session(session_id: str) -> Optional[Session]:
    session = _sessions.get(session_id)
    if not session:
        logger.warning(f"Сессия {session_id} не найдена")
    return session


def delete_session(session_id: str) -> bool:
    if session_id in _sessions:
        del _sessions[session_id]
        logger.info(f"Сессия {session_id} удалена")
        return True
    return False


def list_sessions() -> List[str]:
    return list(_sessions.keys())
