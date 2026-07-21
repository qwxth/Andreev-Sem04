# RAG-сервис (Retrieval-Augmented Generation)

Система вопросно-ответного поиска по документам на основе RAG-архитектуры.

**Стек:** FastAPI + OpenSearch + Ollama (Qwen3 / Qwen2.5)

## Возможности

- Загрузка документов с автоматическим преобразованием в векторные представления (embeddings)
- Поиск документов по семантической близости (k-NN)
- Генерация ответа на основе найденных документов (RAG)
- Диалоговый режим в контексте найденных документов
- Изоляция сессий: каждый диалог привязан к своему набору документов
- Удаление документов и сессий

## Быстрый старт

docker compose up -d
docker exec ollama ollama pull qwen3-embedding
docker exec ollama ollama pull qwen2.5:0.5b
docker compose restart rag-app
python3 scripts/create_index.py --recreate

## API

### POST /documents

Загрузка документа.

Тело запроса:
{
  "content": "Текст документа",
  "document_id": "doc-001",
  "doc_class": "category",
  "summary": "Краткое описание"
}

Ответ:
{
  "success": true,
  "document_id": "doc-001",
  "opensearch_id": "...",
  "message": "OK, vector=4096d"
}

### POST /search

Поиск + создание сессии.

Тело запроса:
{
  "question": "Ваш вопрос",
  "top_k": 5,
  "top_m": 3
}

Параметры:
- top_k — сколько документов искать (по умолчанию 5)
- top_m — сколько документов передать в контекст модели (по умолчанию 3)

Ответ:
{
  "question": "Ваш вопрос",
  "answer": "Сгенерированный ответ...",
  "relevant_documents": [
    {"document_id": "...", "content": "...", "score": 1.57}
  ],
  "documents_used_count": 3,
  "session_id": "uuid-сессии",
  "document_ids": ["doc-001", "doc-002"]
}

### POST /chat

Продолжение диалога в контексте созданной сессии.

Тело запроса:
{
  "session_id": "uuid-сессии",
  "message": "Уточняющий вопрос"
}

Ответ:
{
  "answer": "Ответ модели...",
  "session_id": "uuid-сессии",
  "document_ids": ["doc-001", "doc-002"],
  "history": [
    { "role": "user", "content": "..." },
    { "role": "assistant", "content": "..." }
  ]
}

### DELETE /session/{session_id}

Удаление сессии.

### GET /session/{session_id}

Просмотр состояния сессии (документы, история).

### DELETE /documents/{document_id}

Удаление документа из индекса.

### GET /health

Проверка состояния сервиса.

## Архитектура

Пользователь -> FastAPI -> OpenSearch (поиск) -> Ollama (embeddings + генерация)

1. Документ -> эмбеддинг через Ollama -> сохранение в OpenSearch
2. Вопрос -> эмбеддинг -> k-NN поиск в OpenSearch
3. Найденные документы + вопрос -> генеративная модель -> ответ

## Модели

- qwen3-embedding — для эмбеддингов (4096d)
- qwen2.5:0.5b — для генерации (можно заменить на qwen2.5:3b, qwen3)