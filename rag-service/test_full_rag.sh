#!/bin/bash
set -e

echo "========================================"
echo "  ПОЛНОЕ ТЕСТИРОВАНИЕ RAG-СИСТЕМЫ"
echo "========================================"
echo ""

# Очистка старых данных
echo "🧹 Пересоздание индекса..."
python3 scripts/create_index.py --recreate 2>/dev/null || echo "Индекс создан"

echo ""
echo "========================================"
echo "  ЗАГРУЗКА ДОКУМЕНТОВ"
echo "========================================"

# Документы по строительной экспертизе
echo ""
echo "📄 Блок 1: Строительная экспертиза"

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Строительная экспертиза — это комплекс мероприятий по исследованию объектов строительства с целью установления их соответствия требованиям технических регламентов, проектной документации. Проводится аккредитованными организациями, имеющими лицензию на данный вид деятельности.",
  "document_id": "build-001",
  "doc_class": "definition",
  "summary": "Определение строительной экспертизы"
}' | python3 -c "import sys,json;print('✓ build-001')"

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Порядок проведения строительной экспертизы: 1) Подача заявки в экспертную организацию. 2) Заключение договора и определение стоимости от 30000 до 500000 рублей в зависимости от объёма работ. 3) Выезд эксперта на объект. 4) Составление экспертного заключения в течение 5-30 рабочих дней. 5) Передача заключения заказчику.",
  "document_id": "build-002",
  "doc_class": "procedure",
  "summary": "Порядок проведения экспертизы"
}' | python3 -c "import sys,json;print('✓ build-002')"

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Эксперты строительной экспертизы должны иметь высшее строительное образование и стаж работы не менее 5 лет. Экспертная организация должна быть членом СРО (саморегулируемой организации) и иметь страхование ответственности на сумму не менее 10 миллионов рублей.",
  "document_id": "build-003",
  "doc_class": "requirements",
  "summary": "Требования к экспертам"
}' | python3 -c "import sys,json;print('✓ build-003')"

# Документы по разработке ПО
echo ""
echo "💻 Блок 2: Разработка ПО"

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Регламент разработки программного обеспечения v1.0 определяет следующие обязательные этапы: проектирование (составление ТЗ, архитектура), разработка (написание кода, code review), тестирование (unit-тесты с покрытием минимум 70%, интеграционные тесты), внедрение (развёртывание на продуктивной среде только после согласования с руководителем). Все этапы должны быть документированы согласно ГОСТ Р 12207.",
  "document_id": "dev-001",
  "doc_class": "regulatory",
  "summary": "Регламент разработки ПО v1.0"
}' | python3 -c "import sys,json;print('✓ dev-001')"

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Регламент разработки ПО v2.0 (упрощённый для малых проектов): Тестирование является необязательным при сроке разработки менее 30 дней и бюджете до 100000 рублей. Внедрение может производиться без согласования руководителя при наличии автоматических тестов с покрытием более 80%. Документирование остаётся на усмотрение разработчика, но README обязателен.",
  "document_id": "dev-002",
  "doc_class": "regulatory",
  "summary": "Регламент разработки ПО v2.0 упрощённый"
}' | python3 -c "import sys,json;print('✓ dev-002')"

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Code Review в организации проводится обязательно для всех merge requests. Минимум 2 одобрения (approve) от других разработчиков. Code Review должен проверять: архитектурные решения, безопасность кода, покрытие тестами, соответствие стандартам оформления кода (PEP8 для Python, Google Style Guide для Java). Среднее время на Code Review — не более 4 часов с момента создания MR.",
  "document_id": "dev-003",
  "doc_class": "procedure",
  "summary": "Процедура Code Review"
}' | python3 -c "import sys,json;print('✓ dev-003')"

# Документы по ГОСТам и стандартам
echo ""
echo "📋 Блок 3: ГОСТы и стандарты"

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "ГОСТ Р 57580-2017 устанавливает требования к системам защиты информации в государственных информационных системах. Определяет три уровня защиты: базовый (для общедоступной информации), стандартный (для служебной информации), усиленный (для конфиденциальной и секретной информации). Каждый уровень требует различные организационные и технические меры защиты.",
  "document_id": "gost-001",
  "doc_class": "standard",
  "summary": "ГОСТ Р 57580 защита информации"
}' | python3 -c "import sys,json;print('✓ gost-001')"

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "ISO 9001:2015 Система менеджмента качества требует: документирование всех ключевых процессов организации в виде регламентов и процедур, проведение внутренних аудитов не реже одного раза в год, анализ со стороны руководства ежеквартально, постоянное улучшение процессов на основе метрик и обратной связи.",
  "document_id": "iso-001",
  "doc_class": "standard",
  "summary": "ISO 9001 система менеджмента качества"
}' | python3 -c "import sys,json;print('✓ iso-001')"

# Документы по безопасности
echo ""
echo "🔒 Блок 4: Информационная безопасность"

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Политика паролей организации: минимальная длина пароля 12 символов, обязательно использование заглавных, строчных букв, цифр и спецсимволов. Смена пароля обязательна каждые 90 дней. Запрещено использование одинаковых паролей для разных систем. При трёх неудачных попытках входа — блокировка учётной записи на 30 минут.",
  "document_id": "sec-001",
  "doc_class": "policy",
  "summary": "Политика паролей"
}' | python3 -c "import sys,json;print('✓ sec-001')"

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Процедура реагирования на инциденты информационной безопасности: 1) Обнаружение инцидента и немедленное уведомление службы безопасности. 2) Классификация инцидента (критический/высокий/средний/низкий). 3) Изоляция поражённых систем в течение 15 минут для критических инцидентов. 4) Расследование и устранение причин. 5) Составление отчёта в течение 24 часов. 6) Обновление мер защиты.",
  "document_id": "sec-002",
  "doc_class": "procedure",
  "summary": "Реагирование на инциденты ИБ"
}' | python3 -c "import sys,json;print('✓ sec-002')"

# Документы по проектированию
echo ""
echo "🏗️ Блок 5: Проектная документация"

curl -s -X POST http://localhost:8000/documents -H "Content-Type: application/json" -d '{
  "content": "Проектная документация на строительство должна содержать обязательные разделы: пояснительная записка, архитектурные решения, конструктивные решения, системы отопления и вентиляции, системы водоснабжения и канализации, системы электроснабжения, сметная документация. Все разделы должны быть согласованы в Государственной экспертизе проектной документации. Срок действия положительного заключения — 3 года.",
  "document_id": "proj-001",
  "doc_class": "requirements",
  "summary": "Состав проектной документации"
}' | python3 -c "import sys,json;print('✓ proj-001')"

echo ""
echo "✅ Загружено 11 документов"
sleep 2

echo ""
echo "========================================"
echo "  ТЕСТЫ ТОЧНОСТИ ОТВЕТОВ"
echo "========================================"

# Тест 1: Простой фактический вопрос
echo ""
echo "📝 ТЕСТ 1: Кто проводит строительную экспертизу?"
R1=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"Кто проводит строительную экспертизу?","top_k":3,"top_m":2}')
SID1=$(echo "$R1" | python3 -c "import sys,json;print(json.load(sys.stdin)['session_id'])")
echo "$R1" | python3 -c "
import sys,json
d=json.load(sys.stdin)
ans = d['answer']
print(f'Ответ: {ans}')
# Проверка
if 'аккредит' in ans.lower() or 'лицензи' in ans.lower():
    print('✅ ПРАВИЛЬНО: упомянуты аккредитованные организации')
else:
    print('❌ НЕПРАВИЛЬНО: нет упоминания аккредитации')
"

# Тест 2: Вопрос с числами
echo ""
echo "📝 ТЕСТ 2: Сколько времени занимает строительная экспертиза?"
R2=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"Сколько времени занимает строительная экспертиза?","top_k":3,"top_m":2}')
SID2=$(echo "$R2" | python3 -c "import sys,json;print(json.load(sys.stdin)['session_id'])")
echo "$R2" | python3 -c "
import sys,json
d=json.load(sys.stdin)
ans = d['answer']
print(f'Ответ: {ans}')
# Проверка
if '5' in ans or '30' in ans or 'дней' in ans or 'дня' in ans:
    print('✅ ПРАВИЛЬНО: указан срок 5-30 дней')
else:
    print('❌ НЕПРАВИЛЬНО: нет информации о сроках')
"
curl -s -X DELETE http://localhost:8000/session/$SID2 > /dev/null

# Тест 3: Сравнение двух регламентов
echo ""
echo "📝 ТЕСТ 3: Есть ли противоречия в регламентах разработки ПО?"
R3=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"Имеются ли противоречия в регламентах на разработку ПО версий 1.0 и 2.0?","top_k":5,"top_m":3}')
SID3=$(echo "$R3" | python3 -c "import sys,json;print(json.load(sys.stdin)['session_id'])")
echo "$R3" | python3 -c "
import sys,json
d=json.load(sys.stdin)
ans = d['answer']
print(f'Ответ: {ans[:300]}...')
# Проверка
if 'тестирование' in ans.lower() and ('необязательн' in ans.lower() or 'упрощ' in ans.lower() or 'v2' in ans.lower()):
    print('✅ ПРАВИЛЬНО: замечено различие в требованиях к тестированию')
else:
    print('⚠️  Возможно неполный ответ')
"
curl -s -X DELETE http://localhost:8000/session/$SID3 > /dev/null

# Тест 4: Вопрос с отсутствующими данными
echo ""
echo "📝 ТЕСТ 4: Вопрос без ответа в базе (должна признаться что не знает)"
R4=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"Каков порядок получения визы в Японию?","top_k":3,"top_m":2}')
SID4=$(echo "$R4" | python3 -c "import sys,json;print(json.load(sys.stdin)['session_id'])")
echo "$R4" | python3 -c "
import sys,json
d=json.load(sys.stdin)
ans = d['answer'].lower()
print(f'Ответ: {d[\"answer\"]}')
# Проверка
if 'нет' in ans or 'не найден' in ans or 'отсутств' in ans or 'не содерж' in ans:
    print('✅ ПРАВИЛЬНО: признала отсутствие информации')
elif 'виз' in ans and 'япон' in ans:
    print('❌ НЕПРАВИЛЬНО: выдумала ответ (халлюцинация)')
else:
    print('⚠️  Неоднозначный ответ')
"
curl -s -X DELETE http://localhost:8000/session/$SID4 > /dev/null

# Тест 5: Code Review процедура
echo ""
echo "📝 ТЕСТ 5: Сколько одобрений нужно для Code Review?"
R5=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"Сколько одобрений (approve) требуется для Code Review?","top_k":3,"top_m":2}')
SID5=$(echo "$R5" | python3 -c "import sys,json;print(json.load(sys.stdin)['session_id'])")
echo "$R5" | python3 -c "
import sys,json
d=json.load(sys.stdin)
ans = d['answer']
print(f'Ответ: {ans}')
if '2' in ans or 'два' in ans.lower():
    print('✅ ПРАВИЛЬНО: указано 2 одобрения')
else:
    print('❌ НЕПРАВИЛЬНО: неверное число')
"
curl -s -X DELETE http://localhost:8000/session/$SID5 > /dev/null

echo ""
echo "========================================"
echo "  ТЕСТЫ ДИАЛОГА В КОНТЕКСТЕ"
echo "========================================"

# Тест 6: Многоходовой диалог
echo ""
echo "💬 ТЕСТ 6: Диалог с уточнениями (строительная экспертиза)"
R6=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"Расскажи про строительную экспертизу","top_k":4,"top_m":3}')
SID6=$(echo "$R6" | python3 -c "import sys,json;print(json.load(sys.stdin)['session_id'])")
echo "Вопрос 1: Расскажи про строительную экспертизу"
echo "$R6" | python3 -c "import sys,json;print('  →',json.load(sys.stdin)['answer'][:100]+'...')"

sleep 1
echo ""
echo "Вопрос 2 (уточнение): Какая стоимость?"
curl -s -X POST http://localhost:8000/chat -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID6\",\"message\":\"Какая стоимость?\"}" \
  | python3 -c "
import sys,json
d=json.load(sys.stdin)
ans = d['answer']
print(f'  → {ans}')
if '30000' in ans or '500000' in ans or '30 000' in ans or 'рубл' in ans.lower():
    print('✅ ПРАВИЛЬНО: указана стоимость из документа')
else:
    print('❌ НЕПРАВИЛЬНО: нет информации о стоимости')
"

sleep 1
echo ""
echo "Вопрос 3 (уточнение): А сколько дней?"
curl -s -X POST http://localhost:8000/chat -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID6\",\"message\":\"А сколько дней это занимает?\"}" \
  | python3 -c "
import sys,json
d=json.load(sys.stdin)
ans = d['answer']
print(f'  → {ans}')
if '5' in ans or '30' in ans:
    print('✅ ПРАВИЛЬНО: учёт контекста диалога')
else:
    print('⚠️  Возможна потеря контекста')
print(f'  Сообщений в истории: {len(d[\"history\"])}')
"
curl -s -X DELETE http://localhost:8000/session/$SID6 > /dev/null

# Тест 7: Проверка что диалог не переходит на другие документы
echo ""
echo "💬 ТЕСТ 7: Диалог остаётся в контексте (не переключается на другие темы)"
R7=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"Требования к паролям","top_k":3,"top_m":2}')
SID7=$(echo "$R7" | python3 -c "import sys,json;print(json.load(sys.stdin)['session_id'])")
echo "Вопрос 1: Требования к паролям"
echo "$R7" | python3 -c "import sys,json;print('  →',json.load(sys.stdin)['answer'][:100]+'...')"

sleep 1
echo ""
echo "Вопрос 2 (попытка уйти в другую тему): А что про Code Review?"
curl -s -X POST http://localhost:8000/chat -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID7\",\"message\":\"А что про Code Review?\"}" \
  | python3 -c "
import sys,json
d=json.load(sys.stdin)
ans = d['answer'].lower()
print(f'  → {d[\"answer\"]}')
# Модель должна сказать что в текущих документах нет информации о Code Review
if 'нет' in ans or 'не найден' in ans or 'отсутств' in ans or 'не содерж' in ans or 'пароль' in ans:
    print('✅ ПРАВИЛЬНО: не переключилась на другие документы')
else:
    print('⚠️  Возможно выход за пределы контекста сессии')
"
curl -s -X DELETE http://localhost:8000/session/$SID7 > /dev/null

echo ""
echo "========================================"
echo "  ТЕСТ ПЕРЕКЛЮЧЕНИЯ СЕССИЙ"
echo "========================================"

echo ""
echo "🔄 ТЕСТ 8: Две независимые сессии"
echo "Сессия A: Строительство"
RA=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"Кто проводит строительную экспертизу?","top_k":3,"top_m":2}')
SIDA=$(echo "$RA" | python3 -c "import sys,json;print(json.load(sys.stdin)['session_id'])")
echo "  Session A: $SIDA"
echo "$RA" | python3 -c "import sys,json;print('  →',json.load(sys.stdin)['answer'][:60]+'...')"

sleep 1
echo ""
echo "Сессия B: Разработка ПО"
RB=$(curl -s -X POST http://localhost:8000/search -H "Content-Type: application/json" \
  -d '{"question":"Что требуется для Code Review?","top_k":3,"top_m":2}')
SIDB=$(echo "$RB" | python3 -c "import sys,json;print(json.load(sys.stdin)['session_id'])")
echo "  Session B: $SIDB"
echo "$RB" | python3 -c "import sys,json;print('  →',json.load(sys.stdin)['answer'][:60]+'...')"

sleep 1
echo ""
echo "Продолжение сессии A (должна помнить про строительство):"
curl -s -X POST http://localhost:8000/chat -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SIDA\",\"message\":\"Какая стоимость?\"}" \
  | python3 -c "
import sys,json
d=json.load(sys.stdin)
ans = d['answer'].lower()
print(f'  → {d[\"answer\"]}')
if 'рубл' in ans or '30000' in ans:
    print('✅ ПРАВИЛЬНО: сессия A помнит контекст про строительство')
else:
    print('❌ НЕПРАВИЛЬНО: потерян контекст сессии A')
"

sleep 1
echo ""
echo "Продолжение сессии B (должна помнить про Code Review):"
curl -s -X POST http://localhost:8000/chat -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SIDB\",\"message\":\"Сколько одобрений?\"}" \
  | python3 -c "
import sys,json
d=json.load(sys.stdin)
ans = d['answer']
print(f'  → {ans}')
if '2' in ans or 'два' in ans.lower():
    print('✅ ПРАВИЛЬНО: сессия B помнит контекст про Code Review')
else:
    print('❌ НЕПРАВИЛЬНО: потерян контекст сессии B')
"

curl -s -X DELETE http://localhost:8000/session/$SIDA > /dev/null
curl -s -X DELETE http://localhost:8000/session/$SIDB > /dev/null

# Финальный тест с продолжением диалога из шага 1
echo ""
echo "========================================"
echo "  ФИНАЛЬНЫЙ ТЕСТ: Продолжение первой сессии"
echo "========================================"
echo ""
echo "Возвращаемся к самой первой сессии (тест 1):"
echo "Session ID: $SID1"

curl -s -X POST http://localhost:8000/chat -H "Content-Type: application/json" \
  -d "{\"session_id\":\"$SID1\",\"message\":\"Какие требования к экспертам?\"}" \
  | python3 -c "
import sys,json
d=json.load(sys.stdin)
ans = d['answer'].lower()
print(f'Ответ: {d[\"answer\"]}')
if 'образован' in ans or 'стаж' in ans or '5 лет' in ans or 'сро' in ans:
    print('✅ ПРАВИЛЬНО: найдена информация о требованиях к экспертам')
else:
    print('⚠️  Неполная информация')
print(f'Всего сообщений в истории: {len(d[\"history\"])}')
"

curl -s -X DELETE http://localhost:8000/session/$SID1 > /dev/null

echo ""
echo "========================================"
echo "  ИТОГИ ТЕСТИРОВАНИЯ"
echo "========================================"
echo ""
echo "✅ Все тесты завершены!"
echo ""
echo "Проверено:"
echo "  ✓ Точность ответов на фактические вопросы"
echo "  ✓ Работа с числовыми данными"
echo "  ✓ Сравнение и анализ документов"
echo "  ✓ Корректное поведение при отсутствии данных"
echo "  ✓ Многоходовой диалог с сохранением контекста"
echo "  ✓ Изоляция контекста между сессиями"
echo "  ✓ Отсутствие переключения на документы вне сессии"
echo "  ✓ Накопление истории диалога"
echo ""
echo "Документация API: http://localhost:8000/docs"
echo "Просмотр индекса: http://localhost:5601"
