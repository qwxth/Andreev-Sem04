#!/bin/bash

BASE="http://localhost:8000"

sep() { echo ""; echo "════════════════════════════════════════════════════════════"; echo "  $1"; echo "════════════════════════════════════════════════════════════"; }

sep "ШАГ 1: Пересоздание индекса (удаление всех документов)"
python3 scripts/create_index.py --recreate
echo ""
sleep 2

sep "ШАГ 2: Загрузка 15 документов"

DOCS=(
  'BUILD-001|Строительная экспертиза — комплекс мероприятий по исследованию объектов строительства. Проводится аккредитованными организациями с лицензией.|definition|Определение строительной экспертизы'
  'BUILD-002|Стоимость строительной экспертизы от 30000 до 500000 рублей. Зависит от объёма работ и сложности объекта.|pricing|Стоимость экспертизы'
  'BUILD-003|Срок проведения строительной экспертизы 5-30 рабочих дней. Определяется договором.|timing|Сроки проведения'
  'BUILD-004|Требования к эксперту: высшее строительное образование, стаж не менее 5 лет, членство в СРО, страхование ответственности 10 млн рублей.|requirements|Требования к экспертам'
  'BUILD-005|Порядок проведения: подача заявки, заключение договора, выезд эксперта, составление заключения, передача заказчику.|procedure|Порядок экспертизы'
  'DEV-001|Регламент разработки ПО v1.0: проектирование, разработка, тестирование (покрытие 70%), внедрение после согласования, документирование по ГОСТ Р 12207.|regulatory|Регламент ПО v1.0'
  'DEV-002|Регламент разработки ПО v2.0: тестирование необязательно при сроке менее 30 дней и бюджете до 100 тыс. рублей. README обязателен.|regulatory|Регламент ПО v2.0'
  'DEV-003|Code Review: обязателен для всех MR. Минимум 2 одобрения. Проверка архитектуры, безопасности, тестов, стиля. Время — до 4 часов.|procedure|Code Review процедура'
  'DEV-004|Политика Git: main (стабильный), develop (интеграция), feature/* (новое), hotfix/* (срочное). Прямые коммиты в main запрещены.|policy|Политика веток Git'
  'DEV-005|CI/CD: build → test → lint → docker → staging → production. Деплой только при успешном прохождении всех этапов.|procedure|CI/CD пайплайн'
  'SEC-001|Политика паролей: минимум 12 символов, буквы+цифры+спецсимволы. Смена каждые 90 дней. Блокировка после 3 неудачных попыток на 30 мин.|policy|Политика паролей'
  'SEC-002|Реагирование на инциденты ИБ: обнаружение, классификация, изоляция систем за 15 мин (для критических), расследование, отчёт за 24 часа.|procedure|Процедура инцидентов ИБ'
  'GOST-001|ГОСТ Р 57580-2017: защита информации в ГИС. Три уровня — базовый (общедоступная), стандартный (служебная), усиленный (конфиденциальная).|standard|ГОСТ защита информации'
  'ISO-001|ISO 9001:2015: документирование процессов, внутренние аудиты раз в год, анализ руководства ежеквартально, постоянное улучшение.|standard|ISO 9001 менеджмент качества'
  'PROJ-001|Проектная документация: пояснительная записка, архитектура, конструктив, ОВК, водоснабжение, электроснабжение, смета. Срок действия заключения 3 года.|requirements|Состав проектной документации'
)

for doc in "${DOCS[@]}"; do
  IFS='|' read -r id content class summary <<< "$doc"
  
  RESULT=$(curl -s -X POST "$BASE/documents" \
    -H "Content-Type: application/json" \
    -d "{
      \"content\": \"$content\",
      \"document_id\": \"$id\",
      \"doc_class\": \"$class\",
      \"summary\": \"$summary\"
    }")
  
  if echo "$RESULT" | grep -q '"success":true'; then
    echo "  ✅ $id"
  else
    echo "  ❌ $id — ошибка"
  fi
done

echo ""
echo "  ⏳ Ожидание индексации (5 сек)..."
sleep 5

sep "ШАГ 3: Список всех документов в индексе"

curl -s "https://localhost:9200/ia_index1/_search?size=50" \
  -u admin:Admin_password123! \
  -k \
  -H "Content-Type: application/json" \
  -d '{"query":{"match_all":{}},"_source":["documentId","doc_class","summary","content"],"sort":[{"documentId.keyword":"asc"}]}' \
| python3 << 'PY'
import json, sys
try:
    data = json.load(sys.stdin)
    hits = data.get("hits", {}).get("hits", [])
    
    print(f"  Всего документов в индексе: {len(hits)}")
    print()
    print("  ┌─────────────┬──────────────┬────────────────────────────────┬──────────────────────")
    print("  │ ID          │ Класс        │ Краткое                        │ Контент (начало)")
    print("  ├─────────────┼──────────────┼────────────────────────────────┼──────────────────────")
    
    for hit in hits:
        src = hit.get("_source", {})
        doc_id = src.get("documentId", "?")
        doc_class = src.get("doc_class", "-")
        summary = src.get("summary", "-")
        content = src.get("content", "")
        
        print(f"  │ {doc_id:11} │ {doc_class:12} │ {summary:30} │ {content[:40]:40}...")
    
    print("  └─────────────┴──────────────┴────────────────────────────────┴──────────────────────")
except Exception as e:
    print(f"  ❌ Ошибка: {e}")
PY

sep "ТЕСТ 1: Простой вопрос"
echo "  📥 Вопрос: Стоимость строительной экспертизы"
echo ""

R=$(curl -s --max-time 180 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"Стоимость строительной экспертизы","top_k":5,"top_m":3}')

echo "$R" | python3 << 'PY'
import json, sys
try:
    d = json.loads(sys.stdin.read())
    
    print("  📤 ОТВЕТ:")
    print(f"     {d['answer']}")
    print()
    
    print("  📋 ПАРАМЕТРЫ ОТВЕТА:")
    print(f"     session_id:           {d['session_id']}")
    print(f"     documents_used_count: {d['documents_used_count']}")
    print()
    
    print("  🔖 ДОКУМЕНТЫ В СЕССИИ (document_ids):")
    for i, doc_id in enumerate(d.get("document_ids", []), 1):
        print(f"     {i}. {doc_id}")
    print()
    
    print("  🔍 ВСЕ НАЙДЕННЫЕ ДОКУМЕНТЫ (relevant_documents, top_k):")
    for i, doc in enumerate(d.get("relevant_documents", []), 1):
        did = doc.get("document_id", "?")
        score = doc.get("score", 0)
        marker = " ◀ в контексте" if did in d.get("document_ids", []) else ""
        print(f"     {i}. {did:15} score={score:.3f} {marker}")
    
    with open("/tmp/test_sid1.txt", "w") as f:
        f.write(d["session_id"])
        
except Exception as e:
    print(f"  ❌ Ошибка: {e}")
PY

sep "ТЕСТ 2: Продолжение диалога в той же сессии"
SID1=$(cat /tmp/test_sid1.txt 2>/dev/null || echo "")

if [ -z "$SID1" ]; then
  echo "  ⚠️  Нет session_id"
else
  echo "  session_id: $SID1"
  echo "  📥 Вопрос: Сколько дней занимает?"
  echo ""
  
  R=$(curl -s --max-time 180 -X POST "$BASE/chat" \
    -H "Content-Type: application/json" \
    -d "{\"session_id\":\"$SID1\",\"message\":\"Сколько дней занимает?\"}")
  
  echo "$R" | python3 << 'PY'
import json, sys
try:
    d = json.loads(sys.stdin.read())
    
    print("  📤 ОТВЕТ:")
    print(f"     {d['answer']}")
    print()
    
    print("  📋 ПАРАМЕТРЫ:")
    print(f"     session_id: {d['session_id']}")
    print()
    
    print("  🔖 ДОКУМЕНТЫ В СЕССИИ (не изменились!):")
    for i, doc_id in enumerate(d.get("document_ids", []), 1):
        print(f"     {i}. {doc_id}")
    print()
    
    print("  💬 ИСТОРИЯ ДИАЛОГА:")
    for i, msg in enumerate(d.get("history", []), 1):
        role = "👤 User" if msg["role"] == "user" else "🤖 Bot "
        content = msg["content"][:80] + ("..." if len(msg["content"]) > 80 else "")
        print(f"     {i}. {role}: {content}")
        
except Exception as e:
    print(f"  ❌ Ошибка: {e}")
PY
fi

sep "ТЕСТ 3: Вопрос ВНЕ контекста сессии"

if [ -n "$SID1" ]; then
  echo "  session_id: $SID1 (документы про строительство)"
  echo "  📥 Вопрос: А что про Code Review?"
  echo "            (этой темы НЕТ в документах сессии)"
  echo ""
  
  R=$(curl -s --max-time 180 -X POST "$BASE/chat" \
    -H "Content-Type: application/json" \
    -d "{\"session_id\":\"$SID1\",\"message\":\"А что про Code Review?\"}")
  
  echo "$R" | python3 << 'PY'
import json, sys
try:
    d = json.loads(sys.stdin.read())
    
    print("  📤 ОТВЕТ:")
    print(f"     {d['answer']}")
    print()
    
    print("  🔖 ДОКУМЕНТЫ В СЕССИИ (должны остаться BUILD):")
    docs = d.get("document_ids", [])
    for i, doc_id in enumerate(docs, 1):
        print(f"     {i}. {doc_id}")
    print()
    
    has_dev = any("DEV" in doc for doc in docs)
    has_build = any("BUILD" in doc for doc in docs)
    
    if has_build and not has_dev:
        print("  ✅ OK: документы сессии не изменились")
    else:
        print("  ❌ ОШИБКА: в сессию попали чужие документы!")
    
    ans = d['answer'].lower()
    if any(w in ans for w in ["нет", "не найден", "отсутств", "не содерж"]):
        print("  ✅ OK: модель корректно сообщила об отсутствии данных")
    else:
        print("  ⚠️  WARNING: возможна галлюцинация")
        
except Exception as e:
    print(f"  ❌ Ошибка: {e}")
PY

  curl -s -X DELETE "$BASE/session/$SID1" > /dev/null 2>&1
fi

sep "ТЕСТ 4: Модель называет ID документа"
echo "  📥 Вопрос: В каком документе говорится о стоимости?"
echo ""

R=$(curl -s --max-time 180 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"В каком документе говорится о стоимости строительной экспертизы?","top_k":5,"top_m":3}')

echo "$R" | python3 << 'PY'
import json, sys
try:
    d = json.loads(sys.stdin.read())
    
    print("  📤 ОТВЕТ:")
    print(f"     {d['answer']}")
    print()
    
    print("  🔖 ДОКУМЕНТЫ В КОНТЕКСТЕ:")
    docs = d.get("document_ids", [])
    for i, doc_id in enumerate(docs, 1):
        marker = " ◀ BUILD-002 (стоимость)" if "BUILD-002" in doc_id else ""
        print(f"     {i}. {doc_id}{marker}")
    print()
    
    ans = d['answer']
    if "BUILD-002" in ans:
        print("  ✅ OK: модель указала ID BUILD-002")
    elif any(did in ans for did in docs):
        print("  ✅ OK: модель упомянула ID из контекста")
    else:
        print("  ⚠️  WARNING: ID не найден в тексте ответа")
    
    with open("/tmp/test_sid4.txt", "w") as f:
        f.write(d["session_id"])
        
except Exception as e:
    print(f"  ❌ Ошибка: {e}")
PY

curl -s -X DELETE "$BASE/session/$(cat /tmp/test_sid4.txt 2>/dev/null || echo '')" > /dev/null 2>&1

sep "ТЕСТ 5: Две независимые сессии"
echo "  Создаём:"
echo "    Сессия A → вопрос про строительство"
echo "    Сессия B → вопрос про разработку ПО"
echo ""

RA=$(curl -s --max-time 180 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"Требования к экспертам","top_k":4,"top_m":2}')

SID_A=$(echo "$RA" | python3 -c "import sys,json; print(json.load(sys.stdin)['session_id'])" 2>/dev/null || echo "")
DOCS_A=$(echo "$RA" | python3 -c "import sys,json; print(', '.join(json.load(sys.stdin)['document_ids']))" 2>/dev/null || echo "")

sleep 1

RB=$(curl -s --max-time 180 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"Процедура Code Review","top_k":4,"top_m":2}')

SID_B=$(echo "$RB" | python3 -c "import sys,json; print(json.load(sys.stdin)['session_id'])" 2>/dev/null || echo "")
DOCS_B=$(echo "$RB" | python3 -c "import sys,json; print(', '.join(json.load(sys.stdin)['document_ids']))" 2>/dev/null || echo "")

echo "  Сессия A: $SID_A"
echo "  Документы: $DOCS_A"
echo ""
echo "  Сессия B: $SID_B"
echo "  Документы: $DOCS_B"
echo ""

if [ -n "$SID_A" ] && [ -n "$SID_B" ]; then
  echo "  ── Продолжение A: 'Какой стаж нужен?'"
  CA=$(curl -s --max-time 180 -X POST "$BASE/chat" \
    -H "Content-Type: application/json" \
    -d "{\"session_id\":\"$SID_A\",\"message\":\"Какой стаж нужен?\"}")
  
  echo "$CA" | python3 -c "import sys,json; d=json.load(sys.stdin); print(f'     Ответ: {d[\"answer\"][:80]}...'); print(f'     Документы: {d[\"document_ids\"]}')" 2>/dev/null
  
  sleep 1
  echo ""
  echo "  ── Продолжение B: 'Сколько одобрений?'"
  CB=$(curl -s --max-time 180 -X POST "$BASE/chat" \
    -H "Content-Type: application/json" \
    -d "{\"session_id\":\"$SID_B\",\"message\":\"Сколько одобрений?\"}")
  
  echo "$CB" | python3 -c "import sys,json; d=json.load(sys.stdin); print(f'     Ответ: {d[\"answer\"][:80]}...'); print(f'     Документы: {d[\"document_ids\"]}')" 2>/dev/null
  
  curl -s -X DELETE "$BASE/session/$SID_A" > /dev/null 2>&1
  curl -s -X DELETE "$BASE/session/$SID_B" > /dev/null 2>&1
fi

sep "ЗАВЕРШЕНО"
echo ""
echo "  📦 Документов в индексе: 15"
echo "  ✅ Тестов выполнено:     5"
echo ""
