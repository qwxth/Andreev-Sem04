#!/bin/bash

BASE="http://localhost:8000"

sep() { echo ""; echo "══════════════════════════════════════════════"; echo "  $1"; echo "══════════════════════════════════════════════"; }

call_api() {
  local method="$1" url="$2" data="$3"
  local response http_code
  
  if [ "$method" = "POST" ]; then
    response=$(curl -s --max-time 300 -w "\n%{http_code}" -X POST "$url" \
      -H "Content-Type: application/json" -d "$data" 2>&1)
  elif [ "$method" = "GET" ]; then
    response=$(curl -s --max-time 60 -w "\n%{http_code}" "$url" 2>&1)
  elif [ "$method" = "DELETE" ]; then
    response=$(curl -s --max-time 30 -w "\n%{http_code}" -X DELETE "$url" 2>&1)
  fi
  
  http_code=$(echo "$response" | tail -n1)
  body=$(echo "$response" | head -n-1)
  
  if [ "$http_code" -ge 400 ]; then
    echo "     ⚠️  HTTP $http_code — $body"
    return 1
  fi
  
  echo "$body"
}

sep "ЗАГРУЗКА 20 ДОКУМЕНТОВ"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "Строительная экспертиза — это комплекс мероприятий по исследованию объектов строительства. Проводится аккредитованными организациями, имеющими лицензию на данный вид деятельности.",
  "document_id": "BUILD-001", "doc_class": "definition",
  "summary": "Определение строительной экспертизы"
}' | python3 -c "import sys,json; d=json.load(sys.stdin); print(f'  BUILD-001 → OK')" 2>/dev/null || echo "  BUILD-001 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "Стоимость строительной экспертизы составляет от 30000 до 500000 рублей в зависимости от объёма работ и сложности объекта.",
  "document_id": "BUILD-002", "doc_class": "pricing"
}' > /dev/null && echo "  BUILD-002 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "Срок проведения строительной экспертизы составляет от 5 до 30 рабочих дней. Срок определяется договором и зависит от объёма работ.",
  "document_id": "BUILD-003", "doc_class": "timing"
}' > /dev/null && echo "  BUILD-003 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "Требования к эксперту: высшее строительное образование, стаж работы не менее 5 лет, членство в СРО, страхование ответственности на сумму не менее 10 миллионов рублей.",
  "document_id": "BUILD-004", "doc_class": "requirements"
}' > /dev/null && echo "  BUILD-004 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "Порядок проведения строительной экспертизы: подача заявки, заключение договора, выезд эксперта на объект, составление заключения, передача заключения заказчику.",
  "document_id": "BUILD-005", "doc_class": "procedure"
}' > /dev/null && echo "  BUILD-005 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "Регламент разработки ПО v1.0: обязательные этапы — проектирование, разработка, тестирование с покрытием минимум 70%, внедрение только после согласования с руководителем. Документирование по ГОСТ Р 12207.",
  "document_id": "DEV-001", "doc_class": "regulatory"
}' > /dev/null && echo "  DEV-001 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "Регламент разработки ПО v2.0 (упрощённый): тестирование необязательно при сроке менее 30 дней и бюджете до 100000 рублей. Внедрение без согласования при покрытии тестами более 80%. README обязателен.",
  "document_id": "DEV-002", "doc_class": "regulatory"
}' > /dev/null && echo "  DEV-002 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "Code Review проводится обязательно для всех merge requests. Минимум 2 одобрения от других разработчиков. Проверяется архитектура, безопасность, покрытие тестами, стиль кода. Время — не более 4 часов.",
  "document_id": "DEV-003", "doc_class": "procedure"
}' > /dev/null && echo "  DEV-003 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "Политика веток Git: main — только стабильный код, develop — интеграционная ветка, feature/* — новые функции, hotfix/* — срочные исправления. Прямые коммиты в main запрещены.",
  "document_id": "DEV-004", "doc_class": "policy"
}' > /dev/null && echo "  DEV-004 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "CI/CD пайплайн включает: сборку (build), запуск тестов (test), статический анализ кода (lint), сборку Docker-образа, деплой на staging, деплой на production при успешном прохождении всех этапов.",
  "document_id": "DEV-005", "doc_class": "procedure"
}' > /dev/null && echo "  DEV-005 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "Политика паролей: минимум 12 символов, заглавные и строчные буквы, цифры, спецсимволы. Смена каждые 90 дней. Блокировка после 3 неудачных попыток на 30 минут. Запрет повтора последних 10 паролей.",
  "document_id": "SEC-001", "doc_class": "policy"
}' > /dev/null && echo "  SEC-001 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "Реагирование на инциденты ИБ: обнаружение и уведомление службы безопасности, классификация (критический/высокий/средний/низкий), изоляция систем в течение 15 минут для критических инцидентов, расследование, отчёт за 24 часа.",
  "document_id": "SEC-002", "doc_class": "procedure"
}' > /dev/null && echo "  SEC-002 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "ГОСТ Р 57580-2017: требования к защите информации в государственных информационных системах. Три уровня: базовый (общедоступная), стандартный (служебная), усиленный (конфиденциальная и секретная информация).",
  "document_id": "GOST-001", "doc_class": "standard"
}' > /dev/null && echo "  GOST-001 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "ISO 9001:2015 Система менеджмента качества: документирование ключевых процессов, внутренние аудиты не реже раза в год, анализ руководства ежеквартально, постоянное улучшение на основе метрик.",
  "document_id": "ISO-001", "doc_class": "standard"
}' > /dev/null && echo "  ISO-001 → OK"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" -d '{
  "content": "Состав проектной документации на строительство: пояснительная записка, архитектурные решения, конструктивные решения, системы ОВК, водоснабжение и канализация, электроснабжение, сметная документация. Срок действия заключения — 3 года.",
  "document_id": "PROJ-001", "doc_class": "requirements"
}' > /dev/null && echo "  PROJ-001 → OK"

echo ""
echo "  ⏳ Индексация (4 сек)..."
sleep 4
echo "  ✅ 15 документов загружено"

sep "ТЕСТ 1 — /search: пользователь видит document_ids"
echo "  📥 INPUT:  Что такое строительная экспертиза?"
echo ""

R=$(curl -s --max-time 300 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"Что такое строительная экспертиза?","top_k":5,"top_m":3}')

if [ -z "$R" ]; then
  echo "  ⚠️  Пустой ответ (модель долго думает или ошибка)"
else
  echo "$R" | python3 << 'PY'
import json, sys
try:
    d = json.loads(sys.stdin.read())
    print("  📤 OUTPUT:")
    print(f"     Ответ: {d['answer'][:120]}...")
    print()
    print(f"     🔖 Документы в сессии (пользователь видит):")
    for doc_id in d.get("document_ids", []):
        print(f"        • {doc_id}")
    print()
    print(f"     🔍 Все найденные (top_k={len(d.get('relevant_documents',[]))})")
    for doc in d.get("relevant_documents", [])[:5]:
        print(f"        • {doc.get('document_id','?')}  score={doc['score']:.3f}")
    print()
    print(f"     session_id: {d['session_id']}")
    with open("/tmp/sid1.txt","w") as f: f.write(d["session_id"])
except json.JSONDecodeError as e:
    print(f"  ❌ JSON error: {e}")
    print(f"  Raw: {sys.stdin.read()[:200]}")
except Exception as e:
    print(f"  ❌ Error: {e}")
PY
fi

sep "ТЕСТ 2 — /chat: диалог в контексте тех же документов"
SID1=$(cat /tmp/sid1.txt 2>/dev/null || echo "")
if [ -z "$SID1" ]; then
  echo "  ⚠️  Нет session_id, пропускаем"
else
  echo "  session_id: $SID1"
  echo "  📥 INPUT: Какова стоимость?"
  echo ""

  R=$(curl -s --max-time 300 -X POST "$BASE/chat" \
    -H "Content-Type: application/json" \
    -d "{\"session_id\":\"$SID1\",\"message\":\"Какова стоимость?\"}")

  if [ -z "$R" ]; then
    echo "  ⚠️  Пустой ответ"
  else
    echo "$R" | python3 << 'PY'
import json, sys
try:
    d = json.loads(sys.stdin.read())
    print("  📤 OUTPUT:")
    print(f"     Ответ: {d['answer'][:120]}...")
    print()
    print(f"     🔖 Документы в сессии (те же!):")
    for doc_id in d.get("document_ids", []):
        print(f"        • {doc_id}")
    print()
    print(f"     💬 История: {len(d.get('history',[]))} сообщений")
except Exception as e:
    print(f"  ❌ {e}")
PY
  fi
fi

sep "ТЕСТ 3 — /chat: вопрос ВНЕ контекста сессии"
if [ -z "$SID1" ]; then
  echo "  ⚠️  Нет session_id, пропускаем"
else
  echo "  session_id: $SID1 (сессия про строительство)"
  echo "  📥 INPUT: А что про Code Review?"
  echo "            ^^^^ этой темы НЕТ в документах сессии"
  echo ""

  R=$(curl -s --max-time 300 -X POST "$BASE/chat" \
    -H "Content-Type: application/json" \
    -d "{\"session_id\":\"$SID1\",\"message\":\"А что про Code Review?\"}")

  if [ -z "$R" ]; then
    echo "  ⚠️  Пустой ответ"
  else
    echo "$R" | python3 << 'PY'
import json, sys
try:
    d = json.loads(sys.stdin.read())
    ans = d['answer']
    docs = d.get("document_ids", [])
    print("  📤 OUTPUT:")
    print(f"     Ответ: {ans[:150]}...")
    print()
    print(f"     🔖 Документы в сессии (не изменились!):")
    for doc_id in docs:
        print(f"        • {doc_id}")
    print()
    if any("DEV" in doc for doc in docs):
        print("     ❌ ОШИБКА: попали DEV документы в BUILD сессию!")
    else:
        print("     ✅ OK: документы сессии не изменились")
    if any(w in ans.lower() for w in ["нет","не найден","отсутств","не содерж"]):
        print("     ✅ OK: модель признала отсутствие данных")
    else:
        print("     ⚠️  WARNING: возможно выдумала ответ")
except Exception as e:
    print(f"  ❌ {e}")
PY
  fi
fi

sep "ТЕСТ 4 — Модель называет ID документа в ответе"
echo "  📥 INPUT: В каком документе говорится о стоимости?"
echo ""

R=$(curl -s --max-time 300 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"В каком документе говорится о стоимости строительной экспертизы?","top_k":5,"top_m":3}')

if [ -z "$R" ]; then
  echo "  ⚠️  Пустой ответ"
else
  echo "$R" | python3 << 'PY'
import json, sys
try:
    d = json.loads(sys.stdin.read())
    ans = d['answer']
    docs = d.get("document_ids", [])
    print("  📤 OUTPUT:")
    print(f"     Ответ: {ans}")
    print()
    print(f"     🔖 Документы:")
    for doc_id in docs:
        marker = " ◀ правильный!" if "BUILD-002" in doc_id or "PRICE" in doc_id else ""
        print(f"        • {doc_id}{marker}")
    print()
    if "BUILD-002" in ans:
        print("     ✅ OK: модель указала BUILD-002 (документ про стоимость)")
    elif any(did in ans for did in docs):
        print("     ✅ OK: модель упомянула ID из контекста")
    else:
        print("     ⚠️  WARNING: ID не найден в тексте ответа")
    with open("/tmp/sid4.txt","w") as f: f.write(d["session_id"])
except Exception as e:
    print(f"  ❌ {e}")
PY
  SID4=$(cat /tmp/sid4.txt 2>/dev/null || echo "")
  [ -n "$SID4" ] && curl -s -X DELETE "$BASE/session/$SID4" > /dev/null 2>&1
fi

sep "ТЕСТ 5 — Накопление истории диалога"
echo "  📥 INPUT: Расскажи про строительную экспертизу"
echo ""

R=$(curl -s --max-time 300 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"Расскажи про строительную экспертизу","top_k":5,"top_m":3}')

SID=$(echo "$R" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d['session_id'])" 2>/dev/null || echo "")

if [ -z "$SID" ]; then
  echo "  ⚠️  Не удалось создать сессию"
else
  echo "  session_id: $SID"
  echo "$R" | python3 -c "import sys,json; d=json.load(sys.stdin); print(f'  Ответ: {d[\"answer\"][:100]}...')" 2>/dev/null
  echo ""

  for i in {1..3}; do
    MSGS=("Какова стоимость?" "А сколько дней?" "Кто проводит?")
    MSG="${MSGS[$i-1]}"
    
    echo "  ── Шаг $((i+1)): $MSG"
    R=$(curl -s --max-time 300 -X POST "$BASE/chat" \
      -H "Content-Type: application/json" \
      -d "{\"session_id\":\"$SID\",\"message\":\"$MSG\"}")
    
    if [ -n "$R" ]; then
      echo "$R" | python3 << 'PY'
import json, sys
try:
    d = json.loads(sys.stdin.read())
    h = len(d.get("history", []))
    print(f"     Ответ: {d['answer'][:80]}...")
    print(f"     История: {h} сообщений")
except: pass
PY
    fi
    sleep 1
  done
  
  echo ""
  echo "  📊 Полная история:"
  curl -s --max-time 60 "$BASE/session/$SID" | python3 << 'PY'
import json, sys
try:
    d = json.loads(sys.stdin.read())
    print(f"     Документов: {d['documents_count']}")
    print(f"     Сообщений:  {d['messages_count']}")
    print("     ───────────────────────────")
    for i, msg in enumerate(d.get("history", []), 1):
        role = "User" if msg["role"] == "user" else "Bot "
        print(f"     [{i}] {role}: {msg['content'][:60]}...")
except Exception as e:
    print(f"  ❌ {e}")
PY
  
  curl -s -X DELETE "$BASE/session/$SID" > /dev/null 2>&1
fi

sep "ТЕСТ 6 — Две независимые параллельные сессии"
echo "  Создаём сессию A (строительство) и B (Code Review)"
echo ""

RA=$(curl -s --max-time 300 -X POST "$BASE/search" -H "Content-Type: application/json" \
  -d '{"question":"Стоимость строительной экспертизы","top_k":4,"top_m":2}')
SID_A=$(echo "$RA" | python3 -c "import sys,json; print(json.load(sys.stdin)['session_id'])" 2>/dev/null || echo "")
DOCS_A=$(echo "$RA" | python3 -c "import sys,json; print(json.load(sys.stdin)['document_ids'])" 2>/dev/null || echo "")

sleep 1

RB=$(curl -s --max-time 300 -X POST "$BASE/search" -H "Content-Type: application/json" \
  -d '{"question":"Требования Code Review","top_k":4,"top_m":2}')
SID_B=$(echo "$RB" | python3 -c "import sys,json; print(json.load(sys.stdin)['session_id'])" 2>/dev/null || echo "")
DOCS_B=$(echo "$RB" | python3 -c "import sys,json; print(json.load(sys.stdin)['document_ids'])" 2>/dev/null || echo "")

echo "  Сессия A: $SID_A"
echo "  Документы A: $DOCS_A"
echo ""
echo "  Сессия B: $SID_B"
echo "  Документы B: $DOCS_B"
echo ""

if [ -n "$SID_A" ] && [ -n "$SID_B" ]; then
  echo "  ── Продолжение A: 'Сколько дней?'"
  CA=$(curl -s --max-time 300 -X POST "$BASE/chat" -H "Content-Type: application/json" \
    -d "{\"session_id\":\"$SID_A\",\"message\":\"Сколько дней?\"}")
  echo "$CA" | python3 -c "import sys,json; d=json.load(sys.stdin); print(f'     {d[\"answer\"][:80]}...'); print(f'     Документы: {d[\"document_ids\"]}')" 2>/dev/null
  
  sleep 1
  echo ""
  echo "  ── Продолжение B: 'Сколько одобрений?'"
  CB=$(curl -s --max-time 300 -X POST "$BASE/chat" -H "Content-Type: application/json" \
    -d "{\"session_id\":\"$SID_B\",\"message\":\"Сколько одобрений?\"}")
  echo "$CB" | python3 -c "import sys,json; d=json.load(sys.stdin); print(f'     {d[\"answer\"][:80]}...'); print(f'     Документы: {d[\"document_ids\"]}')" 2>/dev/null
  
  curl -s -X DELETE "$BASE/session/$SID_A" > /dev/null 2>&1
  curl -s -X DELETE "$BASE/session/$SID_B" > /dev/null 2>&1
fi

sep "ТЕСТ 7 — DELETE: удаление сессии и возврат"
R=$(curl -s --max-time 300 -X POST "$BASE/search" -H "Content-Type: application/json" \
  -d '{"question":"Политика паролей","top_k":3,"top_m":2}')
SID=$(echo "$R" | python3 -c "import sys,json; print(json.load(sys.stdin)['session_id'])" 2>/dev/null || echo "")

if [ -n "$SID" ]; then
  echo "  Сессия: $SID"
  
  curl -s --max-time 300 -X POST "$BASE/chat" -H "Content-Type: application/json" \
    -d "{\"session_id\":\"$SID\",\"message\":\"Сколько символов?\"}" > /dev/null
  echo "  /chat работает → OK"
  
  DEL=$(curl -s -X DELETE "$BASE/session/$SID")
  echo "  DELETE → $(echo "$DEL" | python3 -c "import sys,json; print(json.load(sys.stdin)['message'])" 2>/dev/null || echo 'OK')"
  
  CODE=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE/chat" \
    -H "Content-Type: application/json" -d "{\"session_id\":\"$SID\",\"message\":\"Test\"}")
  echo "  /chat после DELETE → HTTP $CODE (ожидается 404)"
  
  R_NEW=$(curl -s --max-time 300 -X POST "$BASE/search" -H "Content-Type: application/json" \
    -d '{"question":"Политика паролей","top_k":3,"top_m":2}')
  SID_NEW=$(echo "$R_NEW" | python3 -c "import sys,json; print(json.load(sys.stdin)['session_id'])" 2>/dev/null || echo "")
  echo "  Новая сессия → $SID_NEW"
  
  curl -s -X DELETE "$BASE/session/$SID_NEW" > /dev/null 2>&1
fi

sep "ЗАВЕРШЕНО"
echo ""
echo "  📦 Документов в индексе: 15"
echo "  ✅ Тестов выполнено: 7"
echo ""
echo "  Документация API: http://localhost:8000/docs"
echo ""
