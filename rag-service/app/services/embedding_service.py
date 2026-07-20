import httpx
import numpy as np
import logging
from typing import List
from tenacity import retry, stop_after_attempt, wait_exponential
from config import get_settings

logger = logging.getLogger(__name__)
settings = get_settings()

def normalize_vector(vector: List[float]) -> List[float]:
    vec = np.array(vector, dtype=np.float32)
    norm = np.linalg.norm(vec)
    if norm == 0:
        logger.warning("Получен нулевой вектор, нормализация невозможна")
        return vector
    return (vec / norm).tolist()

@retry(stop=stop_after_attempt(3), wait=wait_exponential(multiplier=1, min=2, max=10))
async def get_embedding(text: str) -> List[float]:
    url = f"{settings.ollama_host}/api/embed"
    payload = {"model": settings.embedding_model, "input": text}

    async with httpx.AsyncClient(timeout=120.0) as client:
        try:
            logger.info(f"Запрос эмбеддинга, длина текста: {len(text)} символов")
            response = await client.post(url, json=payload)
            response.raise_for_status()
            data = response.json()

            if "embeddings" in data and len(data["embeddings"]) > 0:
                raw_vector = data["embeddings"][0]
            elif "embedding" in data:
                raw_vector = data["embedding"]
            else:
                raise ValueError(f"Неожиданный формат ответа Ollama: {list(data.keys())}")

            logger.info(f"Вектор получен, размерность: {len(raw_vector)}")
            normalized = normalize_vector(raw_vector)
            logger.info("Вектор нормализован (L2)")
            return normalized

        except httpx.HTTPStatusError as e:
            logger.error(f"HTTP ошибка Ollama: {e.response.status_code} — {e.response.text}")
            raise
        except httpx.RequestError as e:
            logger.error(f"Ошибка соединения с Ollama: {e}")
            raise

async def check_ollama_model(model_name: str) -> bool:
    url = f"{settings.ollama_host}/api/tags"
    async with httpx.AsyncClient(timeout=30.0) as client:
        try:
            response = await client.get(url)
            response.raise_for_status()
            models = [m["name"] for m in response.json().get("models", [])]
            return any(model_name in m for m in models)
        except Exception as e:
            logger.error(f"Ошибка проверки модели {model_name}: {e}")
            return False
