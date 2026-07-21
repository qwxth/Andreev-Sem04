# RAG-сервис (Retrieval-Augmented Generation)

Система вопросно-ответного поиска по документам с использованием векторного поиска и генеративного ИИ

Стек: Python 3.11 | FastAPI 0.111 | OpenSearch 2.13 | Ollama Qwen3


## ОПИСАНИЕ

RAG-сервис — это микросервис для интеллектуального поиска по документам с генерацией ответов на естественном языке.

Ключевые возможности:

- Загрузка документов с автоматическим векторным индексированием (embeddings)
- Семантический поиск по k-NN с использованием OpenSearch
- Генерация ответов на основе контекста найденных документов
- Диалоговый режим с изоляцией сессий — каждая сессия работает только со своим набором документов
- Управление данными: добавление, удаление документов и сессий
- Ответы только на русском языке (даже если вопрос задан на английском)
- Факт-чекинг: модель НЕ выдумывает — использует только документы в контексте


## АРХИТЕКТУРА

Пользователь -> FastAPI (RAG) -> OpenSearch (Vector DB) + Ollama (Embeddings + Generation)

Поток данных:

1. Индексация документа
   Документ -> Ollama (embedding) -> Вектор 4096d -> L2-нормализация -> OpenSearch

2. Поиск и ответ (создание сессии)
   Вопрос -> Ollama (embedding) -> Вектор 4096d
   -> OpenSearch k-NN поиск
   -> top_k документов найдено -> top_m в контекст
   -> Промпт = Документы + Вопрос + Правила
   -> Ollama (generation)
   -> Ответ + session_id

3. Диалог в сессии
   session_id + Новый вопрос -> Те же документы сессии + История
   -> Ollama (generation)
   -> Ответ


## БЫСТРЫЙ СТАРТ

Требования:
- Docker >= 20.10
- Docker Compose >= 2.0
- Python >= 3.11 (для скриптов)
- Минимум 8 GB RAM (для моделей Ollama)

Установка и запуск:

# 1. Запустить сервисы
docker compose up -d

# 2. Проверить статус контейнеров
docker ps

# 3. Загрузить модели в Ollama (обязательно!)
docker exec ollama ollama pull qwen3-embedding
docker exec ollama ollama pull qwen3

# Альтернатива для быстрой работы (лёгкая модель):
# docker exec ollama ollama pull qwen2.5:0.5b

# 4. Перезапустить RAG-сервис
docker compose restart rag-app

# 5. Создать индекс в OpenSearch
python3 scripts/create_index.py --recreate

# 6. Проверить готовность
curl http://localhost:8000/health

Ожидаемый ответ:
{
  "status": "healthy",
  "opensearch": "yellow",
  "embedding": "ok",
  "generation": "ok",
  "index": true
}

Примечание: opensearch: "yellow" — это нормально для single-node кластера


## API ДОКУМЕНТАЦИЯ

Базовый URL: http://localhost:8000
OpenAPI (Swagger): http://localhost:8000/docs


## ЭНДПОИНТЫ

### 1. POST /documents — Загрузка документа

Индексирует документ: создаёт вектор через Ollama и сохраняет в OpenSearch.

Пример запроса:

curl -X POST http://localhost:8000/documents \
  -H "Content-Type: application/json" \
  -d '{
    "content": "Стоимость строительной экспертизы от 30000 до 500000 рублей.",
    "document_id": "BUILD-001",
    "doc_class": "pricing",
    "summary": "Цены на экспертизу",
    "chunk_index": 0
  }'

Ответ:

{
  "success": true,
  "document_id": "BUILD-001",
  "opensearch_id": "BUILD-001",
  "message": "OK, vector=4096d"
}

Параметры:

- content (string, обязательно) — Текст документа
- document_id (string, опционально) — ID документа (если не указан — генерируется UUID)
- chunk_index (int, опционально) — Номер чанка (для разбиения больших документов)
- doc_class (string, опционально) — Класс документа (pricing, policy, procedure...)
- summary (string, опционально) — Краткое содержание
- document_date (string, опционально) — Дата документа (ISO 8601)
- classifiers (array, опционально) — Список классификаторов
- note (string, опционально) — Примечание
- sender_full (string, опционально) — Отправитель


### 2. POST /search — Поиск + создание сессии

Ищет релевантные документы по вопросу, генерирует ответ и создаёт сессию диалога.

Пример запроса:

curl -X POST http://localhost:8000/search \
  -H "Content-Type: application/json" \
  -d '{
    "question": "Стоимость строительной экспертизы",
    "top_k": 5,
    "top_m": 3
  }'

Ответ:

{
  "question": "Стоимость строительной экспертизы",
  "answer": "Согласно документу BUILD-001, стоимость строительной экспертизы составляет от 30000 до 500000 рублей.",
  "relevant_documents": [
    {
      "opensearch_id": "BUILD-001",
      "document_id": "BUILD-001",
      "content": "Стоимость строительной экспертизы от 30000 до 500000 рублей.",
      "score": 1.585,
      "doc_class": "pricing"
    }
  ],
  "documents_found": 5,
  "documents_used_count": 3,
  "session_id": "550e8400-e29b-41d4-a716-446655440000",
  "document_ids": ["BUILD-001", "BUILD-002", "BUILD-003"]
}

Параметры:

- question (string, обязательно) — Вопрос пользователя
- top_k (int, 1-100, по умолчанию 5) — Сколько документов найти
- top_m (int, 1-50, по умолчанию 3) — Сколько использовать для генерации ответа

Важно:

- documents_found — все найденные документы (top_k)
- documents_used_count — сколько пошло в контекст модели (top_m)
- document_ids — ID документов, закреплённых за сессией (top_m)
- session_id — используется для продолжения диалога в /chat


### 3. POST /chat — Продолжение диалога в сессии

Продолжает разговор в контексте документов сессии (без повторного поиска).

Пример запроса:

curl -X POST http://localhost:8000/chat \
  -H "Content-Type: application/json" \
  -d '{
    "session_id": "550e8400-e29b-41d4-a716-446655440000",
    "message": "А сколько дней занимает экспертиза?"
  }'

Ответ:

{
  "answer": "Согласно документу BUILD-002, срок проведения строительной экспертизы составляет 5-30 рабочих дней.",
  "session_id": "550e8400-e29b-41d4-a716-446655440000",
  "document_ids": ["BUILD-001", "BUILD-002", "BUILD-003"],
  "history": [
    {"role": "user", "content": "Стоимость строительной экспертизы"},
    {"role": "assistant", "content": "Согласно документу BUILD-001..."},
    {"role": "user", "content": "А сколько дней занимает экспертиза?"},
    {"role": "assistant", "content": "Согласно документу BUILD-002..."}
  ]
}

Особенности:

- Документы ФИКСИРОВАНЫ на момент создания сессии (/search)
- Новый поиск НЕ происходит — используются только документы сессии
- Если спросить о документах вне сессии — модель ответит: "В предоставленных документах информация отсутствует"


### 4. GET /session/{session_id} — Информация о сессии

Пример запроса:

curl http://localhost:8000/session/550e8400-e29b-41d4-a716-446655440000

Ответ:

{
  "session_id": "550e8400-e29b-41d4-a716-446655440000",
  "created_at": "2026-07-21T10:15:30.123456",
  "first_question": "Стоимость строительной экспертизы",
  "documents_count": 3,
  "document_ids": ["BUILD-001", "BUILD-002", "BUILD-003"],
  "messages_count": 4,
  "history": [...]
}


### 5. DELETE /session/{session_id} — Удаление сессии

Удаляет сессию диалога (чтобы начать заново).

Пример запроса:

curl -X DELETE http://localhost:8000/session/550e8400-e29b-41d4-a716-446655440000

Ответ:

{
  "message": "Сессия 550e8400-e29b-41d4-a716-446655440000 удалена",
  "next": "Используйте POST /search для нового поиска"
}


### 6. DELETE /documents/{document_id} — Удаление документа

Удаляет документ из OpenSearch (включая все его чанки).

Пример запроса:

curl -X DELETE http://localhost:8000/documents/BUILD-001

Ответ:

{
  "success": true,
  "deleted": 1,
  "message": "Документ BUILD-001 удалён (1 записей)"
}


### 7. GET /health — Проверка состояния

Ответ:

{
  "status": "healthy",
  "opensearch": "yellow",
  "embedding": "ok",
  "generation": "ok",
  "index": true
}


## ПРИМЕРЫ ИСПОЛЬЗОВАНИЯ

### Пример 1: Простой вопрос-ответ

# Загрузить документ
curl -X POST http://localhost:8000/documents \
  -H "Content-Type: application/json" \
  -d '{"content":"Минимальный размер оплаты труда 16242 рубля.","document_id":"LAW-001"}'

# Задать вопрос
curl -X POST http://localhost:8000/search \
  -H "Content-Type: application/json" \
  -d '{"question":"Какой МРОТ?","top_k":3,"top_m":2}'

Ответ: "Согласно документу LAW-001, минимальный размер оплаты труда составляет 16242 рубля."


### Пример 2: Диалоговый режим с изоляцией

# Шаг 1: Поиск по теме "экспертиза"
curl -X POST http://localhost:8000/search \
  -H "Content-Type: application/json" \
  -d '{"question":"Стоимость экспертизы","top_k":5,"top_m":3}'

Документы в сессии: ["BUILD-001", "BUILD-002", "BUILD-003"]

# Шаг 2: Продолжаем диалог (документы не меняются)
curl -X POST http://localhost:8000/chat \
  -H "Content-Type: application/json" \
  -d '{"session_id":"...","message":"Сколько дней?"}'

Ответ: "Согласно документу BUILD-002, срок проведения 5-30 рабочих дней."

# Шаг 3: Вопрос ВНЕ контекста сессии
curl -X POST http://localhost:8000/chat \
  -H "Content-Type: application/json" \
  -d '{"session_id":"...","message":"А что насчёт Code Review?"}'

Ответ: "В предоставленных документах информация отсутствует."
(DEV-001 не в сессии!)

Документы остались прежними: ["BUILD-001", "BUILD-002", "BUILD-003"]


### Пример 3: Вопрос "в каком документе..."

curl -X POST http://localhost:8000/search \
  -H "Content-Type: application/json" \
  -d '{"question":"В каком документе говорится о сроках экспертизы?","top_k":5,"top_m":3}'

Ответ: "Информация о сроках экспертизы содержится в документе BUILD-002."


## ТЕСТИРОВАНИЕ

### Быстрый тест (ручной)

bash session_test.sh

### Полный тест (20 документов + все сценарии)

bash final_rag_test.sh

Проверяет:

- Загрузка документов
- Семантический поиск
- Создание и изоляция сессий
- Диалог в контексте сессии
- Модель не выдумывает факты
- Ответы на русском (даже на английские вопросы)
- Удаление документов и сессий
- Параллельные сессии


## КОНФИГУРАЦИЯ

### Переменные окружения (.env)

OPENSEARCH_PASSWORD=Admin_password123!
EMBEDDING_MODEL=qwen3-embedding
GENERATION_MODEL=qwen3
INDEX_NAME=ia_index1
TOP_K_RESULTS=5
TOP_M_CONTEXT=3

### Параметры моделей

Модель              | Размер | Скорость    | Качество | Рекомендация
--------------------|--------|-------------|----------|-------------
qwen3-embedding     | 4GB    | средне      | 5/5      | Embeddings
qwen3               | 9GB    | медленно    | 5/5      | Продакшн
qwen2.5:0.5b        | 400MB  | БЫСТРО      | 3/5      | Разработка
qwen2.5:3b          | 2GB    | средне      | 4/5      | Компромисс


## СТРУКТУРА ПРОЕКТА

rag-service/
├── app/
│   ├── main.py                    # FastAPI приложение
│   ├── config.py                  # Настройки (Pydantic Settings)
│   ├── models/
│   │   ├── schemas.py             # Pydantic схемы API
│   │   └── session_models.py      # Модели сессий
│   ├── services/
│   │   ├── embedding_service.py   # Работа с Ollama embeddings
│   │   ├── generation_service.py  # Генерация ответов (промпты)
│   │   └── opensearch_service.py  # Работа с OpenSearch
│   ├── requirements.txt
│   └── Dockerfile
├── scripts/
│   └── create_index.py            # Создание индекса в OpenSearch
├── docker-compose.yml
├── final_rag_test.sh              # Полный тест
├── session_test.sh                # Тест сессий
├── .env
└── README.md


## ВАЖНЫЕ ДЕТАЛИ

### 1. Сессии хранятся в памяти

ВНИМАНИЕ: Текущая реализация — сессии хранятся в dict в процессе Python.

Ограничения:
- При перезапуске контейнера rag-app — все сессии теряются
- Не масштабируется на несколько инстансов (нужен общий store)

Для продакшена: заменить session_models.py на Redis


### 2. Промпт-инженеринг

Ключевые требования в промпте:

- Используй ТОЛЬКО информацию из документов ниже
- ВСЕГДА указывай ID документа при каждом факте
- Если в документах нет ответа — так и скажи
- НЕ выдумывай факты и НЕ используй внешние знания
- Отвечай только на русском языке

Пример ответа модели:
"Согласно документу BUILD-001, стоимость экспертизы от 30000 до 500000 рублей."


### 3. Векторный поиск (k-NN)

Параметры индекса:

{
  "dimension": 4096,
  "method": "hnsw",
  "space_type": "innerproduct",
  "parameters": {
    "m": 16,
    "ef_construction": 512
  }
}

Почему innerproduct для нормализованных векторов?

Для L2-нормализованных векторов (||v|| = 1):
cosine(a,b) = a·b / (||a|| ||b||) = a·b / (1·1) = innerproduct(a,b)

Поэтому innerproduct эквивалентен cosine и работает быстрее.


### 4. Чанкование больших документов

Если документ большой — разбивайте на чанки:

curl -X POST http://localhost:8000/documents \
  -d '{"content":"Часть 1...","document_id":"DOC-001","chunk_index":0}'

curl -X POST http://localhost:8000/documents \
  -d '{"content":"Часть 2...","document_id":"DOC-001","chunk_index":1}'

OpenSearch ID: DOC-001__chunk_0, DOC-001__chunk_1 (не перезаписывают друг друга)


## TROUBLESHOOTING

### Проблема: "embedding": "fail" в /health

Причина: модель не загружена.

Решение:
docker exec ollama ollama pull qwen3-embedding
docker compose restart rag-app


### Проблема: Медленная генерация ответов

Решение 1: Переключиться на лёгкую модель

docker exec ollama ollama pull qwen2.5:0.5b

В .env:
GENERATION_MODEL=qwen2.5:0.5b

docker compose restart rag-app

Решение 2: Увеличить timeout в generation_service.py
async with httpx.AsyncClient(timeout=600.0) as client:


### Проблема: opensearch: "unavailable"

Проверка:
docker logs opensearch | tail -20
curl -sk -u admin:Admin_password123! https://localhost:9200/_cluster/health

Решение:
docker compose restart opensearch
sleep 30
python3 scripts/create_index.py


## ПРОИЗВОДИТЕЛЬНОСТЬ

Типичные значения (на CPU):

Операция                    | Время      | Примечание
----------------------------|------------|------------------
Генерация embedding         | 0.5-2 сек  | qwen3-embedding
k-NN поиск (10k docs)       | <100 мс    | OpenSearch HNSW
Генерация ответа (qwen3)    | 10-60 сек  | ~500 токенов
Генерация (qwen2.5:0.5b)    | 2-5 сек    | быстрая модель

Для ускорения:
- Используйте GPU (NVIDIA): добавьте CUDA в docker-compose
- Кэшируйте embeddings для повторяющихся вопросов
- Включите OLLAMA_NUM_PARALLEL=2 для параллельных запросов


## БЕЗОПАСНОСТЬ

Текущее состояние (development):

- ВНИМАНИЕ: Пароль OpenSearch в коде и .env
- ВНИМАНИЕ: verify_certs=False для SSL
- ВНИМАНИЕ: CORS allow_origins=["*"]

Для продакшена:

1. Использовать Docker secrets или HashiCorp Vault
2. Включить проверку SSL-сертификатов
3. Ограничить CORS только нужными доменами
4. Добавить аутентификацию пользователей (JWT)
5. Rate limiting (slowapi)


## ROADMAP

- Персистентные сессии (Redis)
- Streaming ответов (SSE)
- Фильтры по метаданным документов
- Поддержка PDF/DOCX загрузки
- Веб-интерфейс (React/Vue)
- Метрики (Prometheus + Grafana)
- A/B тестирование промптов
- Fine-tuning моделей под домен


## ЛИЦЕНЗИЯ

MIT


## БЛАГОДАРНОСТИ

- Ollama — локальный запуск LLM
- OpenSearch — векторная база данных
- FastAPI — современный веб-фреймворк
- Qwen — качественные open-source модели


Готово к использованию!

Для вопросов и предложений — создавайте Issue в репозитории.
