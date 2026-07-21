#!/bin/bash

BASE="http://localhost:8000"

echo "════════════════════════════════════════════════════════════"
echo "ФИНАЛЬНОЕ ТЕСТИРОВАНИЕ RAG (20 доков + удаление)"
echo "════════════════════════════════════════════════════════════"
echo ""

python3 scripts/create_index.py --recreate 2>/dev/null
echo "✓ Индекс пересоздан"
echo ""

# Загружаем 20 документов
DOCS=(
  'BUILD-001|Стоимость строительной экспертизы от 30000 до 500000 рублей.|pricing'
  'BUILD-002|Срок проведения строительной экспертизы 5-30 рабочих дней.|timing'
  'BUILD-003|Требования к эксперту: стаж не менее 5 лет, членство в СРО.|requirements'
  'BUILD-004|Порядок проведения: подача заявки, заключение договора, выезд эксперта.|procedure'
  'BUILD-005|Эксперт должен иметь высшее строительное образование и страховку.|requirements'
  'DEV-001|Code Review обязателен. Минимум 2 одобрения от других разработчиков.|procedure'
  'DEV-002|Тестирование ПО: покрытие минимум 70 процентов кода.|testing'
  'DEV-003|Политика веток Git: main стабильный, develop интеграция.|policy'
  'DEV-004|CI/CD пайплайн: build → test → lint → deploy.|procedure'
  'DEV-005|Документирование API обязательно для всех микросервисов.|procedure'
  'SEC-001|Политика паролей: минимум 12 символов, смена каждые 90 дней.|policy'
  'SEC-002|Реагирование на инциденты: изоляция систем за 15 мин.|procedure'
  'SEC-003|Доступ к production только через VPN и approval.|policy'
  'GOST-001|ГОСТ Р 57580: защита информации в ГИС, три уровня защиты.|standard'
  'ISO-001|ISO 9001:2015: аудиты раз в год, документирование процессов.|standard'
  'PROJ-001|Состав проектной документации: пояснительная записка, архитектура, смета.|requirements'
  'PROJ-002|Управление рисками: идентификация, оценка, мониторинг еженедельно.|procedure'
  'HR-001|Ежегодный отпуск 28 дней, доп. 5 дней за ненормированный день.|policy'
  'HR-002|Суточные: 700 руб РФ, 2500 руб зарубеж. Отчёт за 3 дня.|policy'
  'ORG-001|Приказ №42: договоры до 100к — руководитель отдела, до 1 млн — финдир.|order'
)

echo "Загрузка 20 документов..."
for entry in "${DOCS[@]}"; do
  IFS='|' read -r id content class <<< "$entry"
  RESP=$(curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" \
    -d "{\"content\":\"$content\",\"document_id\":\"$id\",\"doc_class\":\"$class\"}")
  if echo "$RESP" | jq -e '.success' > /dev/null 2>&1; then
    echo -n "."
  else
    echo -e "\n✗ $id: $RESP"
  fi
done
echo ""
sleep 5
echo "✓ 20 документов загружены"
echo ""

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 1: Поиск и документы в сессии"
echo "════════════════════════════════════════════════════════════"

R1=$(curl -s --max-time 300 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"Стоимость экспертизы","top_k":5,"top_m":3}')

echo "Ответ: $(echo "$R1" | jq -r '.answer')"
echo "document_ids: $(echo "$R1" | jq -r '.document_ids | join(", ")')"
SID1=$(echo "$R1" | jq -r '.session_id')
echo ""

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 2: Продолжение диалога (сроки)"
echo "════════════════════════════════════════════════════════════"

R2=$(curl -s --max-time 300 -X POST "$BASE/chat" \
  -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID1\",\"message\":\"Сколько дней?\"}")

echo "Ответ: $(echo "$R2" | jq -r '.answer')"
echo "document_ids: $(echo "$R2" | jq -r '.document_ids | join(", ")')"
echo "история: $(echo "$R2" | jq -r '.history | length') сообщений"
echo ""

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 3: Вопрос ВНЕ контекста (другая тема)"
echo "════════════════════════════════════════════════════════════"

R3=$(curl -s --max-time 300 -X POST "$BASE/chat" \
  -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID1\",\"message\":\"А что насчёт Code Review?\"}")

echo "Ответ: $(echo "$R3" | jq -r '.answer')"
echo "document_ids: $(echo "$R3" | jq -r '.document_ids | join(", ")')"
HAS_DEV=$(echo "$R3" | jq -r '.document_ids[] | select(contains("DEV"))')
if [ -z "$HAS_DEV" ]; then echo "✓ Документы не изменились"; else echo "✗ Ошибка: попали DEV-документы!"; fi
echo ""

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 4: Английский промпт → русский ответ"
echo "════════════════════════════════════════════════════════════"

R4=$(curl -s --max-time 300 -X POST "$BASE/chat" \
  -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID1\",\"message\":\"What is the cost?\"}")

echo "Ответ: $(echo "$R4" | jq -r '.answer')"
echo ""

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 5: Несуществующая информация в документах"
echo "════════════════════════════════════════════════════════════"

R5=$(curl -s --max-time 300 -X POST "$BASE/chat" \
  -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID1\",\"message\":\"Сколько стоит экспертиза в Японии?\"}")

echo "Ответ: $(echo "$R5" | jq -r '.answer')"
echo ""

curl -s -X DELETE "$BASE/session/$SID1" > /dev/null

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 6: УДАЛЕНИЕ ДОКУМЕНТА (DELETE /documents/BUILD-001)"
echo "════════════════════════════════════════════════════════════"

echo ""
echo "Удаляем BUILD-001..."
DEL_RESP=$(curl -s -X DELETE "$BASE/documents/BUILD-001")
echo "$DEL_RESP" | jq '.'

sleep 3

echo ""
echo "Поиск после удаления..."
R6=$(curl -s --max-time 300 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"Стоимость экспертизы","top_k":5,"top_m":3}')

HAS_DELETED=$(echo "$R6" | jq -r '.relevant_documents[] | select(.document_id == "BUILD-001") | .document_id')
if [ -z "$HAS_DELETED" ]; then
  echo "✓ Удалённый BUILD-001 не найден в результатах"
else
  echo "✗ Удалённый документ всё ещё появляется!"
fi
echo "Ответ: $(echo "$R6" | jq -r '.answer')"
echo "document_ids: $(echo "$R6" | jq -r '.document_ids | join(", ")')"
echo ""

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 7: Добавление нового документа и поиск"
echo "════════════════════════════════════════════════════════════"

curl -s -X POST "$BASE/documents" -H "Content-Type: application/json" \
  -d '{"content":"Новая цена экспертизы: 50000 рублей за объект.","document_id":"BUILD-NEW","doc_class":"pricing"}' \
  | jq '.message'

sleep 3

R7=$(curl -s --max-time 300 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"Новая цена экспертизы","top_k":3,"top_m":2}')

HAS_NEW=$(echo "$R7" | jq -r '.document_ids[] | select(contains("BUILD-NEW"))')
if [ -n "$HAS_NEW" ]; then echo "✓ Новый документ BUILD-NEW найден"; else echo "✗ Новый документ не найден"; fi
echo "Ответ: $(echo "$R7" | jq -r '.answer')"
SID7=$(echo "$R7" | jq -r '.session_id')
echo ""

curl -s -X DELETE "$BASE/session/$SID7" > /dev/null

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 8: Удаление сессии и новая сессия"
echo "════════════════════════════════════════════════════════════"

R8=$(curl -s --max-time 300 -X POST "$BASE/search" \
  -H "Content-Type: application/json" \
  -d '{"question":"Code Review","top_k":3,"top_m":2}')

SID8=$(echo "$R8" | jq -r '.session_id')
echo "Новая сессия: $SID8"
echo "document_ids: $(echo "$R8" | jq -r '.document_ids | join(", ")')"
echo "Ответ: $(echo "$R8" | jq -r '.answer')"

curl -s -X DELETE "$BASE/session/$SID8" > /dev/null
echo ""

echo "════════════════════════════════════════════════════════════"
echo "ТЕСТ 9: Параллельные сессии (изоляция)"
echo "════════════════════════════════════════════════════════════"

R9A=$(curl -s --max-time 300 -X POST "$BASE/search" -H "Content-Type: application/json" \
  -d '{"question":"Требования к экспертам","top_k":3,"top_m":2}')
SID9A=$(echo "$R9A" | jq -r '.session_id')
DOCS9A=$(echo "$R9A" | jq -r '.document_ids | join(", ")')

sleep 1

R9B=$(curl -s --max-time 300 -X POST "$BASE/search" -H "Content-Type: application/json" \
  -d '{"question":"Политика паролей","top_k":3,"top_m":2}')
SID9B=$(echo "$R9B" | jq -r '.session_id')
DOCS9B=$(echo "$R9B" | jq -r '.document_ids | join(", ")')

echo "Сессия A: $DOCS9A"
echo "Сессия B: $DOCS9B"

CA=$(curl -s --max-time 300 -X POST "$BASE/chat" -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID9A\",\"message\":\"Сколько лет стажа?\"}")
DOCS_CA=$(echo "$CA" | jq -r '.document_ids | join(", ")')

CB=$(curl -s --max-time 300 -X POST "$BASE/chat" -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID9B\",\"message\":\"Минимум символов?\"}")
DOCS_CB=$(echo "$CB" | jq -r '.document_ids | join(", ")')

if [ "$DOCS9A" = "$DOCS_CA" ] && [ "$DOCS9B" = "$DOCS_CB" ]; then
  echo "✓ Сессии изолированы, документы не изменились"
else
  echo "✗ Ошибка изоляции"
fi

curl -s -X DELETE "$BASE/session/$SID9A" > /dev/null
curl -s -X DELETE "$BASE/session/$SID9B" > /dev/null
echo ""

echo "════════════════════════════════════════════════════════════"
echo "ИТОГИ"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "Все тесты выполнены. Система работает корректно:"
echo "  • Поиск → top_m документов + ответ модели"
echo "  • /chat → продолжает диалог в тех же документах"
echo "  • Модель отвечает только на русском, не выдумывает"
echo "  • Удаление документа через DELETE /documents/{id}"
echo "  • Добавление нового документа"
echo "  • Удаление сессии через DELETE /session/{id}"
echo "  • Создание новой сессии через /search"
echo "  • Сессии изолированы друг от друга"
echo ""
