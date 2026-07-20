#!/bin/bash
set -e

echo "========================================="
echo "  ТЕСТЫ ID ДОКУМЕНТОВ И ИЗОЛЯЦИИ КОНТЕКСТА"
echo "========================================="
echo ""

# Очистка и загрузка тестовых данных
echo "📦 Загрузка тестовых документов..."

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Строительная экспертиза — это проверка зданий. Проводится организациями с лицензией. Документ регламентирует основные понятия.",
  "document_id": "BUILD-MAIN-001",
  "doc_class": "definition"
}' > /dev/null

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Стоимость строительной экспертизы составляет от 30000 до 500000 рублей. Срок проведения 5-30 рабочих дней.",
  "document_id": "BUILD-PRICE-002",
  "doc_class": "pricing"
}' > /dev/null

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Требования к экспертам: высшее образование, стаж 5 лет, членство в СРО. Документ утверждён приказом №123 от 2023 года.",
  "document_id": "BUILD-REQ-003",
  "doc_class": "requirements"
}' > /dev/null

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Code Review требует минимум 2 одобрения. Проверка безопасности обязательна. Среднее время — 4 часа.",
  "document_id": "DEV-REVIEW-101",
  "doc_class": "development"
}' > /dev/null

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Тестирование ПО должно покрывать минимум 70% кода. Unit-тесты обязательны для всех модулей.",
  "document_id": "DEV-TEST-102",
  "doc_class": "development"
}' > /dev/null

echo "✅ Загружено 5 документов"
sleep 2

echo ""
echo "========================================="
echo "  ТЕСТ 1: ID документов видны в ответе"
echo "========================================="
echo ""

R1=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"Что такое строительная экспертиза?","top_k":4,"top_m":3}')

echo "📋 Проверка наличия поля document_ids в ответе /search:"
echo "$R1" | python3 << 'PYEND'
import sys, json
d = json.load(sys.stdin)

if "document_ids" in d:
    print("✅ PASSED: Поле document_ids присутствует")
    print("   Документы в сессии:", d["document_ids"])
    
    # Проверка что это BUILD документы
    build_docs = [doc_id for doc_id in d["document_ids"] if "BUILD" in doc_id]
    if len(build_docs) > 0:
        print("✅ PASSED: Найдены документы по строительству:", build_docs)
    else:
        print("❌ FAILED: Нет BUILD документов в контексте")
else:
    print("❌ FAILED: Поле document_ids отсутствует!")
    sys.exit(1)

# Сохраняем для следующих тестов
with open("/tmp/session1.txt", "w") as f:
    f.write(d["session_id"])
with open("/tmp/doc_ids1.txt", "w") as f:
    f.write(",".join(d["document_ids"]))
PYEND

SID1=$(cat /tmp/session1.txt)
DOC_IDS1=$(cat /tmp/doc_ids1.txt)

echo ""
echo "========================================="
echo "  ТЕСТ 2: Модель указывает ID документа"
echo "========================================="
echo ""

echo "❓ Вопрос: В каком документе говорится о стоимости экспертизы?"
R2=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"В каком документе указана стоимость строительной экспертизы?","top_k":4,"top_m":3}')

echo "$R2" | python3 << 'PYEND'
import sys, json
d = json.load(sys.stdin)

answer = d["answer"]
print("💬 Ответ модели:", answer)
print()

# Проверка что модель назвала BUILD-PRICE-002
if "BUILD-PRICE-002" in answer or "PRICE-002" in answer or "BUILD-PRICE" in answer:
    print("✅ PASSED: Модель указала правильный ID документа (BUILD-PRICE-002)")
elif any(doc_id in answer for doc_id in d.get("document_ids", [])):
    print("✅ PASSED: Модель указала один из ID документов контекста")
else:
    print("⚠️  WARNING: ID документа не найден в ответе")
    print("   Документы в контексте:", d.get("document_ids", []))

with open("/tmp/session2.txt", "w") as f:
    f.write(d["session_id"])
PYEND

SID2=$(cat /tmp/session2.txt)

echo ""
echo "========================================="
echo "  ТЕСТ 3: Диалог остаётся в контексте"
echo "========================================="
echo ""

echo "Создаём сессию про строительство:"
R3=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"Требования к экспертам по строительству","top_k":4,"top_m":2}')

SID3=$(echo "$R3" | python3 -c "import sys,json;print(json.load(sys.stdin)['session_id'])")
DOC_IDS3=$(echo "$R3" | python3 -c "import sys,json;print(','.join(json.load(sys.stdin)['document_ids']))")

echo "📋 Первоначальные документы сессии: $DOC_IDS3"
echo "$R3" | python3 -c "import sys,json;d=json.load(sys.stdin);print('💬 Ответ:', d['answer'][:100] + '...')"

sleep 1
echo ""
echo "❓ Задаём вопрос про Code Review (которого НЕТ в контексте сессии):"
R3_2=$(curl -s -X POST http://localhost:8000/chat -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID3\",\"message\":\"А что про Code Review?\"}")

echo "$R3_2" | python3 << 'PYEND'
import sys, json
d = json.load(sys.stdin)

doc_ids = d.get("document_ids", [])
answer = d["answer"].lower()

print("📋 Документы в сессии (должны остаться те же BUILD):", doc_ids)
print("💬 Ответ:", d["answer"])
print()

# Проверка что документы не изменились
has_build = any("BUILD" in doc_id for doc_id in doc_ids)
has_dev = any("DEV" in doc_id for doc_id in doc_ids)

if has_build and not has_dev:
    print("✅ PASSED: Сессия не переключилась на DEV документы")
else:
    print("❌ FAILED: В сессию попали чужие документы!")
    
# Проверка что модель сказала что нет информации
if "нет" in answer or "не найден" in answer or "отсутств" in answer or "не содерж" in answer:
    print("✅ PASSED: Модель корректно сообщила об отсутствии данных в контексте")
elif "code review" in answer or "2 одобрени" in answer:
    print("❌ FAILED: Модель ответила про Code Review, которого нет в контексте сессии!")
else:
    print("⚠️  WARNING: Неоднозначный ответ")
PYEND

curl -s -X DELETE http://localhost:8000/session/$SID3 > /dev/null

echo ""
echo "========================================="
echo "  ТЕСТ 4: Разные сессии = разные документы"
echo "========================================="
echo ""

echo "Сессия A: Строительство"
RA=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"Стоимость строительной экспертизы","top_k":3,"top_m":2}')

SIDA=$(echo "$RA" | python3 -c "import sys,json;print(json.load(sys.stdin)['session_id'])")
DOCS_A=$(echo "$RA" | python3 -c "import sys,json;print(','.join(json.load(sys.stdin)['document_ids']))")
echo "  Session A: $SIDA"
echo "  Документы A: $DOCS_A"

sleep 1
echo ""
echo "Сессия B: Code Review"
RB=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"Требования для Code Review","top_k":3,"top_m":2}')

SIDB=$(echo "$RB" | python3 -c "import sys,json;print(json.load(sys.stdin)['session_id'])")
DOCS_B=$(echo "$RB" | python3 -c "import sys,json;print(','.join(json.load(sys.stdin)['document_ids']))")
echo "  Session B: $SIDB"
echo "  Документы B: $DOCS_B"

echo ""
export DOCS_A
export DOCS_B
python3 << 'PYEND'
import os
docs_a = os.environ.get("DOCS_A", "").split(",")
docs_b = os.environ.get("DOCS_B", "").split(",")

print("📋 Сессия A документы:", docs_a)
print("📋 Сессия B документы:", docs_b)
print()

has_build_a = any("BUILD" in d for d in docs_a)
has_dev_b = any("DEV" in d for d in docs_b)

if has_build_a and not any("DEV" in d for d in docs_a):
    print("✅ PASSED: Сессия A содержит только BUILD документы")
else:
    print("❌ FAILED: Сессия A содержит неправильные документы")

if has_dev_b and not any("BUILD" in d for d in docs_b):
    print("✅ PASSED: Сессия B содержит только DEV документы")
else:
    print("❌ FAILED: Сессия B содержит неправильные документы")

# Проверка что сессии независимы
intersection = set(docs_a) & set(docs_b)
if len(intersection) == 0:
    print("✅ PASSED: Сессии A и B полностью независимы (нет общих документов)")
else:
    print("⚠️  WARNING: Сессии имеют общие документы:", intersection)
PYEND

echo ""
echo "========================================="
echo "  ✅ ТЕСТИРОВАНИЕ ЗАВЕРШЕНО"
echo "========================================="

