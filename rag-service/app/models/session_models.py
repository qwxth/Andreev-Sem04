import uuid
import json
import logging
from datetime import datetime
from typing import Dict, List, Optional
import redis
from models.schemas import RelevantDocument, ChatMessage
from config import get_settings

logger = logging.getLogger(__name__)
settings = get_settings()

_redis_client: Optional[redis.Redis] = None

def get_redis_client() -> Optional[redis.Redis]:
    global _redis_client
    if _redis_client is None:
        try:
            _redis_client = redis.Redis(
                host='redis',
                port=6379,
                db=0,
                decode_responses=True,
                socket_connect_timeout=5,
                socket_timeout=5
            )
            _redis_client.ping()
            logger.info("Redis connected successfully")
        except Exception as e:
            logger.warning(f"Redis unavailable, using in-memory: {e}")
            _redis_client = None
    return _redis_client

_sessions: Dict[str, 'Session'] = {}

class Session:
    def __init__(self, session_id: str, documents: List[RelevantDocument], first_question: str = ""):
        self.session_id = session_id
        self.documents = documents
        self.history: List[ChatMessage] = []
        self.created_at = datetime.now().isoformat()
        self.first_question = first_question

    def add_message(self, role: str, content: str):
        self.history.append(ChatMessage(role=role, content=content))

    def to_dict(self) -> dict:
        return {
            "session_id": self.session_id,
            "documents": [d.model_dump() for d in self.documents],
            "history": [{"role": m.role, "content": m.content} for m in self.history],
            "created_at": self.created_at,
            "first_question": self.first_question
        }

    @classmethod
    def from_dict(cls, data: dict) -> 'Session':
        session = cls(
            session_id=data["session_id"],
            documents=[RelevantDocument(**d) for d in data["documents"]],
            first_question=data.get("first_question", "")
        )
        session.created_at = data.get("created_at", datetime.now().isoformat())
        session.history = [ChatMessage(**m) for m in data.get("history", [])]
        return session

def create_session(session_id: str, documents: List[RelevantDocument], first_question: str = "") -> 'Session':
    session = Session(session_id, documents, first_question)
    r = get_redis_client()
    if r:
        try:
            r.setex(f"session:{session_id}", 3600, json.dumps(session.to_dict(), ensure_ascii=False))
            logger.info(f"Session {session_id} saved to Redis, documents: {len(documents)}")
        except Exception as e:
            logger.error(f"Error writing to Redis: {e}")
            _sessions[session_id] = session
    else:
        _sessions[session_id] = session
        logger.info(f"Session {session_id} saved in-memory, documents: {len(documents)}")
    return session

def get_session(session_id: str) -> Optional['Session']:
    r = get_redis_client()
    if r:
        try:
            data = r.get(f"session:{session_id}")
            if data:
                return Session.from_dict(json.loads(data))
            logger.warning(f"Session {session_id} not found in Redis")
            return None
        except Exception as e:
            logger.error(f"Error reading from Redis: {e}")
            return _sessions.get(session_id)
    else:
        session = _sessions.get(session_id)
        if not session:
            logger.warning(f"Session {session_id} not found in-memory")
        return session

def save_session(session: 'Session') -> bool:
    r = get_redis_client()
    if r:
        try:
            r.setex(f"session:{session.session_id}", 3600, json.dumps(session.to_dict(), ensure_ascii=False))
            return True
        except Exception as e:
            logger.error(f"Error updating Redis: {e}")
            return False
    else:
        if session.session_id in _sessions:
            _sessions[session.session_id] = session
            return True
        return False

def delete_session(session_id: str) -> bool:
    r = get_redis_client()
    if r:
        try:
            result = r.delete(f"session:{session_id}")
            if result:
                logger.info(f"Session {session_id} deleted from Redis")
                return True
            return False
        except Exception as e:
            logger.error(f"Error deleting from Redis: {e}")
            if session_id in _sessions:
                del _sessions[session_id]
                return True
            return False
    else:
        if session_id in _sessions:
            del _sessions[session_id]
            logger.info(f"Session {session_id} deleted from memory")
            return True
        return False

def list_sessions() -> List[str]:
    r = get_redis_client()
    if r:
        try:
            keys = r.keys("session:*")
            return [k.replace("session:", "") for k in keys]
        except Exception as e:
            logger.error(f"Error getting session list from Redis: {e}")
            return list(_sessions.keys())
    else:
        return list(_sessions.keys())
