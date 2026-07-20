#!/bin/bash

BASE="http://localhost:8000"

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ АЛГОРИТМА RAG (чистый вывод)"
echo "════════════════════════════════════════════════════════════"
echo ""

python3 scripts/create_index.py --recreate 2>/dev/null
echo "✓ Индекс пересоздан"
echo ""

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" \
  -d '{"content":"Стоимость строительной экспертизы от 30000 до 500000 рублей.","document_id":"BUILD-001","doc_class":"pricing"}' > /dev/null

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" \
  -d '{"content":"Срок проведения строительной экспертизы 5-30 рабочих дней.","document_id":"BUILD-002","doc_class":"timing"}' > /dev/null

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" \
  -d '{"content":"Требования к эксперту: стаж не менее 5 лет, членство в СРО.","document_id":"BUILD-003","doc_class":"requirements"}' > /dev/null

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" \
  -d '{"content":"Code Review обязателен. Минимум 2 одобрения от других разработчиков.","document_id":"DEV-001","doc_class":"procedure"}' > /dev/null

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" \
  -d '{"content":"Тестирование ПО: покрытие минимум 70 процентов кода.","document_id":"DEV-002","doc_class":"testing"}' > /dev/null

echo "✓ Загружено 5 документов"
echo ""
sleep 5

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 1: Поиск информации"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "INPUT: Стоимость экспертизы"
echo ""

R1=$(curl -s --max-time 180 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"Стоимость экспертизы","top_k":5,"top_m":2}')

echo "OUTPUT:"
echo "$R1" | jq -r '.answer'
echo ""
echo "session_id: $(echo "$R1" | jq -r '.session_id')"
echo "document_ids: $(echo "$R1" | jq -r '.document_ids')"
echo ""

SID=$(echo "$R1" | jq -r '.session_id')

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 2: Продолжение диалога в контексте"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "INPUT: Сколько дней занимает?"
echo ""

R2=$(curl -s --max-time 180 -X POST "$BASE/chat" \
  -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID\",\"message\":\"Сколько дней занимает?\"}")

echo "OUTPUT:"
echo "$R2" | jq -r '.answer'
echo ""
echo "document_ids: $(echo "$R2" | jq -r '.document_ids')"
echo "история: $(echo "$R2" | jq -r '.history | length') сообщений"
echo ""

DOCS_BEFORE=$(echo "$R1" | jq -r '.document_ids | sort | join(",")')
DOCS_AFTER=$(echo "$R2" | jq -r '.document_ids | sort | join(",")')

if [ "$DOCS_BEFORE" = "$DOCS_AFTER" ]; then
  echo "✓ Документы НЕ изменились"
else
  echo "✗ Документы изменились!"
fi
echo ""

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 3: Вопрос ВНЕ контекста (про Code Review)"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "Текущие документы сессии: $(echo "$R2" | jq -r '.document_ids | join(", ")')"
echo ""
echo "INPUT: А что насчёт Code Review?"
echo ""

R3=$(curl -s --max-time 180 -X POST "$BASE/chat" \
  -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID\",\"message\":\"А что насчёт Code Review?\"}")

echo "OUTPUT:"
echo "$R3" | jq -r '.answer'
echo ""
echo "document_ids: $(echo "$R3" | jq -r '.document_ids')"
echo ""

DOCS_AFTER2=$(echo "$R3" | jq -r '.document_ids | sort | join(",")')
HAS_DEV=$(echo "$R3" | jq -r '.document_ids[] | select(contains("DEV"))')

if [ "$DOCS_BEFORE" = "$DOCS_AFTER2" ]; then
  echo "✓ Документы НЕ изменились"
else
  echo "✗ Документы изменились!"
fi

if [ -z "$HAS_DEV" ]; then
  echo "✓ DEV-документы НЕ попали в сессию"
else
  echo "✗ DEV-документы попали в сессию!"
fi
echo ""

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 4: Модель отвечает на русском (промпт на английском)"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "INPUT: What is the cost?"
echo ""

R4=$(curl -s --max-time 180 -X POST "$BASE/chat" \
  -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID\",\"message\":\"What is the cost?\"}")

echo "OUTPUT:"
echo "$R4" | jq -r '.answer'
echo ""

if echo "$R4" | jq -r '.answer' | grep -qP '[а-яА-Я]'; then
  echo "✓ Ответ на русском"
else
  echo "✗ Ответ НЕ на русском"
fi
echo ""

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 5: Вопрос про несуществующую информацию"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "INPUT: Сколько стоит экспертиза в Японии?"
echo ""

R5=$(curl -s --max-time 180 -X POST "$BASE/chat" \
  -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID\",\"message\":\"Сколько стоит экспертиза в Японии?\"}")

echo "OUTPUT:"
echo "$R5" | jq -r '.answer'
echo ""

curl -s -X DELETE "$BASE/session/$SID" > /dev/null 2>&1

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 6: Удаление сессии и создание новой"
echo "════════════════════════════════════════════════════════════"
echo ""

R6=$(curl -s --max-time 180 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"Code Review","top_k":3,"top_m":2}')

SID2=$(echo "$R6" | jq -r '.session_id')

echo "Новая сессия: $SID2"
echo "document_ids: $(echo "$R6" | jq -r '.document_ids')"
echo ""
echo "OUTPUT:"
echo "$R6" | jq -r '.answer'
echo ""

DEL=$(curl -s -X DELETE "$BASE/session/$SID2")
echo "Удаление: $(echo "$DEL" | jq -r '.message')"
echo ""

CODE=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE/chat" \
  -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID2\",\"message\":\"test\"}")

if [ "$CODE" = "404" ]; then
  echo "✓ /chat вернул 404 после удаления"
else
  echo "✗ /chat вернул $CODE вместо 404"
fi
echo ""

echo "════════════════════════════════════════════════════════════"
echo "ИТОГ"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "Алгоритм:"
echo "  1. /search → возвращает X документов (top_m) + ответ + session_id"
echo "  2. /chat → работает с ЭТИМИ X документами (не меняет их)"
echo "  3. DELETE /session → удаляет сессию"
echo "  4. Новый /search → создаёт новую сессию"
echo ""
