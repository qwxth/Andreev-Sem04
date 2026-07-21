```json
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

text


## API

### POST /documents – загрузка документа
```json
curl -X POST http://localhost:8000/documents
-H "Content-Type: application/json"
-d '{
"content": "Стоимость строительной экспертизы от 30000 до 500000 рублей.",
"document_id": "BUILD-001",
"doc_class": "pricing",
"summary": "Цены на экспертизу"
}'

text


**Ответ:**
```json
{
  "success": true,
  "document_id": "BUILD-001",
  "opensearch_id": "BUILD-001",
  "message": "OK, vector=4096d"
}
POST /search – поиск + создание сессии
text

curl -s --max-time 240 -X POST http://localhost:8000/search \
  -H "Content-Type: application/json" \
  -d '{
    "question": "Стоимость экспертизы",
    "top_k": 5,
    "top_m": 3
  }' | jq .
Ответ:

JSON

{
  "question": "Стоимость экспертизы",
  "answer": "Стоимость строительной экспертизы от 30000 до 500000 рублей (согласно документу BUILD-001).",
  "relevant_documents": [
    {
      "opensearch_id": "BUILD-001",
      "document_id": "BUILD-001",
      "content": "Стоимость строительной экспертизы от 30000 до 500000 рублей.",
      "score": 1.585,
      "chunk_index": 0,
      "doc_class": "pricing",
      "summary": "Цены на экспертизу"
    }
  ],
  "documents_used_count": 3,
  "session_id": "550e8400-e29b-41d4-a716-446655440000",
  "document_ids": ["BUILD-001", "BUILD-002", "BUILD-003"]
}
POST /chat – продолжение диалога
text

curl -s --max-time 240 -X POST http://localhost:8000/chat \
  -H "Content-Type: application/json" \
  -d '{
    "session_id": "550e8400-e29b-41d4-a716-446655440000",
    "message": "Сколько дней занимает экспертиза?"
  }' | jq .
Ответ:

JSON

{
  "answer": "Срок проведения экспертизы 5-30 рабочих дней (документ BUILD-002).",
  "session_id": "550e8400-e29b-41d4-a716-446655440000",
  "document_ids": ["BUILD-001", "BUILD-002", "BUILD-003"],
  "history": [
    {"role": "user", "content": "Стоимость экспертизы"},
    {"role": "assistant", "content": "Стоимость строительной экспертизы от 30000 до 500000 рублей..."},
    {"role": "user", "content": "Сколько дней занимает экспертиза?"},
    {"role": "assistant", "content": "Срок проведения экспертизы 5-30 рабочих дней..."}
  ]
}
GET /session/{id} – просмотр сессии
text

curl -s http://localhost:8000/session/550e8400-e29b-41d4-a716-446655440000 | jq .
Ответ:

JSON

{
  "session_id": "550e8400-e29b-41d4-a716-446655440000",
  "documents_count": 3,
  "document_ids": ["BUILD-001", "BUILD-002", "BUILD-003"],
  "messages_count": 4,
  "history": [
    {"role": "user", "content": "..."},
    {"role": "assistant", "content": "..."}
  ]
}
DELETE /session/{id} – удаление сессии
text

curl -s -X DELETE http://localhost:8000/session/550e8400-e29b-41d4-a716-446655440000 | jq .
Ответ:

JSON

{
  "message": "Сессия 550e8400-e29b-41d4-a716-446655440000 удалена",
  "next": "Используйте POST /search для нового поиска"
}
DELETE /documents/{id} – удаление документа из индекса
text

curl -s -X DELETE http://localhost:8000/documents/BUILD-001 | jq .
Ответ:

JSON

{
  "success": true,
  "deleted": 1,
  "message": "Документ BUILD-001 удалён"
}
GET /health – проверка состояния сервиса
text

curl -s http://localhost:8000/health | jq .
Ответ:

JSON

{
  "status": "healthy",
  "opensearch": "green",
  "embedding": "ok",
  "generation": "ok",
  "index": true
}
Архитектура
Пользователь -> FastAPI -> OpenSearch (поиск) -> Ollama (embeddings + генерация)

Документ -> эмбеддинг через Ollama -> сохранение в OpenSearch
Вопрос -> эмбеддинг -> k-NN поиск в OpenSearch
Найденные документы + вопрос -> генеративная модель -> ответ
Модели
qwen3-embedding — для эмбеддингов (4096d)
qwen2.5:0.5b — для генерации (можно заменить на qwen2.5:3b, qwen3)
