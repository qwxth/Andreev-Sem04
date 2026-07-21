#!/bin/bash

BASE="http://localhost:8000"

echo "=== СОЗДАНИЕ СЕССИИ ==="
R=$(curl -s --max-time 240 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"Стоимость экспертизы","top_k":3,"top_m":2}')

SID=$(echo "$R" | jq -r '.session_id')
echo "Ответ модели: $(echo "$R" | jq -r '.answer')"
echo "session_id: $SID"
echo "document_ids: $(echo "$R" | jq -r '.document_ids')"
echo ""

echo "=== СЕССИЯ СУЩЕСТВУЕТ? (GET) ==="
curl -s "$BASE/session/$SID" | jq '{session_id, documents_count, messages_count, document_ids}'
echo ""

echo "=== ДИАЛОГ РАБОТАЕТ (CHAT) ==="
curl -s --max-time 240 -X POST "$BASE/chat" \
  -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID\",\"message\":\"Сколько дней?\"}" \
| jq '{answer, history_length: (.history | length)}'
echo ""

echo "=== УДАЛЕНИЕ СЕССИИ (DELETE) ==="
curl -s -X DELETE "$BASE/session/$SID" | jq '.'
echo ""

echo "=== ПОСЛЕ УДАЛЕНИЯ ==="
echo "GET /session/$SID:"
curl -s -o /dev/null -w "  HTTP %{http_code}\n" "$BASE/session/$SID"

echo "POST /chat с удалённым ID:"
curl -s -o /dev/null -w "  HTTP %{http_code}\n" -X POST "$BASE/chat" \
  -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID\",\"message\":\"привет\"}"

echo "Повторный DELETE:"
curl -s -o /dev/null -w "  HTTP %{http_code}\n" -X DELETE "$BASE/session/$SID"
echo ""

echo "=== СОЗДАНИЕ НОВОЙ СЕССИИ (возврат к началу) ==="
curl -s --max-time 240 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"Code Review","top_k":3,"top_m":2}' \
| jq '{answer, session_id, document_ids}'
