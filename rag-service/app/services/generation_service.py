import httpx
import logging
import re
from typing import List
from tenacity import retry, stop_after_attempt, wait_exponential
from config import get_settings

logger = logging.getLogger(__name__)
settings = get_settings()


def _format_docs(documents) -> str:
    docs_text = ""
    for i, doc in enumerate(documents, 1):
        doc_id = doc.document_id or doc.opensearch_id
        doc_class = f" | Класс: {doc.doc_class}" if doc.doc_class else ""
        summary = f" | Краткое: {doc.summary}" if doc.summary else ""
        docs_text += f"\n--- Документ {i} (ID: {doc_id}{doc_class}{summary}) ---\n{doc.content}\n"
    return docs_text


def build_rag_prompt(question: str, documents) -> str:
    docs_text = _format_docs(documents)

    return f"""Ты — ассистент. Отвечай ТОЛЬКО на русском языке, без рассуждений.

=== ПРАВИЛА ===
- Используй ТОЛЬКО информацию из документов ниже
- ВСЕГДА указывай ID документа при каждом факте (например: "Согласно документу BUILD-001...")
- Если в документах нет ответа — так и скажи: "В предоставленных документах информация отсутствует"
- НЕ выдумывай факты и НЕ используй внешние знания
- Отвечай только на русском языке, даже если вопрос задан на другом языке

=== ДОКУМЕНТЫ ===
{docs_text}

=== ВОПРОС ===
{question}

=== ОТВЕТ ==="""


def build_chat_prompt(documents, history, new_message: str) -> str:
    docs_text = _format_docs(documents)

    history_text = ""
    for msg in history:
        role = "Пользователь" if msg.role == "user" else "Ассистент"
        history_text += f"{role}: {msg.content}\n"

    return f"""Ты — ассистент. Отвечай ТОЛЬКО на русском языке, без рассуждений.

=== ПРАВИЛА ===
- Используй ТОЛЬКО документы этой сессии (указаны ниже)
- ВСЕГДА указывай ID документа при каждом факте (например: "Согласно документу BUILD-001...")
- Если спрашивают "в каком документе" — обязательно называй ID
- Если в документах нет ответа — так и скажи: "В предоставленных документах информация отсутствует"
- Учитывай историю диалога
- НЕ выдумывай факты и НЕ используй внешние знания
- Отвечай только на русском языке, даже если вопрос задан на другом языке

=== ДОКУМЕНТЫ СЕССИИ ===
{docs_text}

=== ИСТОРИЯ ДИАЛОГА ===
{history_text}
=== НОВЫЙ ВОПРОС ===
{new_message}

=== ОТВЕТ ==="""


async def _call_ollama(prompt: str) -> str:
    url = f"{settings.ollama_host}/api/generate"
    payload = {
        "model": settings.generation_model,
        "prompt": prompt,
        "stream": False,
        "options": {"temperature": 0.1, "num_predict": 2048}
    }

    async with httpx.AsyncClient(timeout=300.0) as client:
        response = await client.post(url, json=payload)
        response.raise_for_status()
        data = response.json()
        answer = data.get("response", "").strip()
        # Убираем блоки рассуждений <think>...</think> (Qwen3)
        answer = re.sub(r'<think>.*?</think>', '', answer, flags=re.DOTALL).strip()
        return answer if answer else "Нет ответа"


@retry(stop=stop_after_attempt(3), wait=wait_exponential(multiplier=1, min=2, max=15))
async def generate_answer(question: str, documents) -> str:
    if not documents:
        return "Документы не найдены"
    prompt = build_rag_prompt(question, documents)
    return await _call_ollama(prompt)


@retry(stop=stop_after_attempt(3), wait=wait_exponential(multiplier=1, min=2, max=15))
async def generate_chat_answer(documents, history, message: str) -> str:
    if not documents:
        return "Документы сессии не найдены"
    prompt = build_chat_prompt(documents, history, message)
    return await _call_ollama(prompt)


async def check_generation_model() -> bool:
    url = f"{settings.ollama_host}/api/tags"
    async with httpx.AsyncClient(timeout=30.0) as client:
        try:
            response = await client.get(url)
            response.raise_for_status()
            models = [m["name"] for m in response.json().get("models", [])]
            return any(settings.generation_model in m for m in models)
        except Exception:
            return False
