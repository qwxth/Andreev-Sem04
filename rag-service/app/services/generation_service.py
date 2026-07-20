import httpx
import logging
from typing import List
from tenacity import retry, stop_after_attempt, wait_exponential
from config import get_settings
from models.schemas import RelevantDocument

logger = logging.getLogger(__name__)
settings = get_settings()

def build_rag_prompt(question: str, documents: List[RelevantDocument]) -> str:
    docs_context = ""
    for i, doc in enumerate(documents, start=1):
        meta_parts = []
        if doc.document_id:
            meta_parts.append(f"ID: {doc.document_id}")
        if doc.doc_class:
            meta_parts.append(f"Класс: {doc.doc_class}")
        if doc.summary:
            meta_parts.append(f"Краткое содержание: {doc.summary}")
        meta_line = f"[{' | '.join(meta_parts)}]" if meta_parts else ""
        docs_context += f"\n--- Документ {i} {meta_line} ---\n{doc.content}\n"

    prompt = f"""Ты — интеллектуальный ассистент, работающий с базой документов организации.

=== ОБЯЗАТЕЛЬНЫЕ ПРАВИЛА (нарушение недопустимо) ===
1. ЯЗЫК: Отвечай ТОЛЬКО на русском языке. Любые иностранные слова, термины и фразы на других языках категорически запрещены.
2. БЕЗ РАССУЖДЕНИЙ: Не показывай внутренние размышления, цепочки мыслей и промежуточные шаги. Только финальный готовый ответ.
3. ТОЛЬКО ФАКТЫ ИЗ ДОКУМЕНТОВ: Строй ответ ИСКЛЮЧИТЕЛЬНО на основе документов, представленных ниже. Не добавляй никакой информации из общих знаний. Не выдумывай факты, определения, события или данные, которых нет в предоставленных документах. Если ответа в документах нет — прямо сообщи об этом пользователю.
4. ССЫЛКИ НА ИСТОЧНИК: Указывай, из какого документа взята информация (Документ 1, Документ 2 и т.д.).
5. ТОЧНОСТЬ: Отвечай строго по существу без лишних вступлений и общих фраз.

=== ДОКУМЕНТЫ ИЗ БАЗЫ ЗНАНИЙ (единственный допустимый источник) ===
{docs_context}

=== ВОПРОС ===
{question}

=== ОТВЕТ ==="""
    return prompt

@retry(stop=stop_after_attempt(3), wait=wait_exponential(multiplier=1, min=2, max=15))
async def generate_answer(question: str, documents: List[RelevantDocument]) -> str:
    if not documents:
        return (
            "По вашему запросу не найдено релевантных документов в базе знаний. "
            "Пожалуйста, уточните вопрос или убедитесь, что документы загружены."
        )

    prompt = build_rag_prompt(question, documents)
    url = f"{settings.ollama_host}/api/generate"
    payload = {
        "model": settings.generation_model,
        "prompt": prompt,
        "stream": False,
        "options": {
            "temperature": 0.1,
            "top_p": 0.9,
            "num_predict": 2048,
            "repeat_penalty": 1.1,
            "think": False
        }
    }

    async with httpx.AsyncClient(timeout=300.0) as client:
        try:
            logger.info(f"Генерация ответа, модель: {settings.generation_model}, документов: {len(documents)}")
            response = await client.post(url, json=payload)
            response.raise_for_status()
            data = response.json()
            answer = data.get("response", "").strip()
            if not answer:
                logger.warning("Получен пустой ответ от генеративной модели")
                return "Модель не смогла сформировать ответ. Попробуйте переформулировать вопрос."
            logger.info(f"Ответ сгенерирован, длина: {len(answer)} символов")
            return answer
        except httpx.HTTPStatusError as e:
            logger.error(f"HTTP ошибка генерации: {e.response.status_code} — {e.response.text}")
            raise
        except httpx.RequestError as e:
            logger.error(f"Ошибка соединения с Ollama (генерация): {e}")
            raise

async def check_generation_model() -> bool:
    url = f"{settings.ollama_host}/api/tags"
    async with httpx.AsyncClient(timeout=30.0) as client:
        try:
            response = await client.get(url)
            response.raise_for_status()
            models = [m["name"] for m in response.json().get("models", [])]
            return any(settings.generation_model in m for m in models)
        except Exception as e:
            logger.error(f"Ошибка проверки генеративной модели: {e}")
            return False
