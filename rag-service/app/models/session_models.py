from typing import List

class ChatMessage:
    def __init__(self, role: str, content: str):
        self.role = role
        self.content = content

    def dict(self):
        return {"role": self.role, "content": self.content}


class Session:
    def __init__(self, session_id: str, documents):
        self.session_id = session_id
        self.documents = documents
        self.history: List[ChatMessage] = []

    def add_message(self, role: str, content: str):
        self.history.append(ChatMessage(role=role, content=content))


sessions = {}


def create_session(session_id: str, documents) -> "Session":
    sessions[session_id] = Session(session_id, documents)
    return sessions[session_id]


def get_session(session_id: str):
    return sessions.get(session_id)


def delete_session(session_id: str) -> bool:
    if session_id in sessions:
        del sessions[session_id]
        return True
    return False
