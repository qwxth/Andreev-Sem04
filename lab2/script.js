// script.js - вся логика работы калькулятора и переключения страниц

(function() {
    // ==================== ПЕРЕКЛЮЧЕНИЕ СТРАНИЦ ====================

    // Получаем элементы навигации и страниц
    const navBtns = document.querySelectorAll('.nav-btn');
    const aboutPage = document.getElementById('about-page');
    const calcPage = document.getElementById('calculator-page');

    /**
     * Функция переключения между страницами
     * @param {string} pageName - название страницы ('about' или 'calculator')
     */
    function switchPage(pageName) {
        if (pageName === 'about') {
            aboutPage.classList.add('active');
            calcPage.classList.remove('active');
        } else {
            calcPage.classList.add('active');
            aboutPage.classList.remove('active');
        }

        // Обновляем активное состояние кнопок навигации
        navBtns.forEach(btn => {
            if (btn.dataset.page === pageName) {
                btn.classList.add('active');
            } else {
                btn.classList.remove('active');
            }
        });
    }

    // Навешиваем обработчики на кнопки навигации
    navBtns.forEach(btn => {
        btn.addEventListener('click', () => switchPage(btn.dataset.page));
    });

    // ==================== ЛОГИКА КАЛЬКУЛЯТОРА ====================

    // Состояние калькулятора
    let current = '0';              // Текущее отображаемое значение
    let previous = null;            // Предыдущий операнд
    let operator = null;            // Текущий оператор
    let waitingForNewOperand = false; // Ожидание нового операнда после операции

    const resultElement = document.getElementById('calcResult');

    /**
     * Обновляет отображение на экране калькулятора
     * При необходимости сокращает длинные числа до экспоненциального формата
     */
    function updateDisplay() {
        let displayValue = current;
        if (displayValue.length > 16) {
            const num = parseFloat(displayValue);
            if (!isNaN(num)) {
                displayValue = num.toExponential(8);
            }
        }
        resultElement.textContent = displayValue;
    }

    /**
     * Обрабатывает ввод цифры или десятичной точки
     * @param {string} digit - введенная цифра или точка
     */
    function inputDigit(digit) {
        if (waitingForNewOperand) {
            // Если ожидаем новый операнд, начинаем ввод с чистого листа
            current = digit === '.' ? '0.' : digit;
            waitingForNewOperand = false;
        } else {
            if (digit === '.') {
                // Запрещаем ввод второй точки
                if (current.includes('.')) return;
                current += '.';
            } else {
                // Обработка цифр
                if (current === '0') {
                    current = digit;
                } else {
                    current += digit;
                }
            }
        }
        // Ограничиваем длину строки для предотвращения переполнения
        if (current.length > 16) {
            current = current.slice(0, 16);
        }
        updateDisplay();
    }

    /**
     * Полная очистка калькулятора
     */
    function clearAll() {
        current = '0';
        previous = null;
        operator = null;
        waitingForNewOperand = false;
        updateDisplay();
    }

    /**
     * Изменение знака текущего числа (+/-)
     */
    function changeSign() {
        const num = parseFloat(current);
        if (isNaN(num)) return;
        current = (-num).toString();
        updateDisplay();
    }

    /**
     * Процентная операция
     */
    function percentOperation() {
        const num = parseFloat(current);
        if (isNaN(num)) return;
        current = (num / 100).toString();
        updateDisplay();
    }

    /**
     * Выполняет арифметическую операцию между двумя числами
     * @param {number|string} a - первый операнд
     * @param {number|string} b - второй операнд
     * @param {string} op - оператор (+, -, *, /)
     * @returns {number} результат вычисления
     */
    function compute(a, b, op) {
        const numA = parseFloat(a);
        const numB = parseFloat(b);

        switch (op) {
            case '+': return numA + numB;
            case '-': return numA - numB;
            case '*': return numA * numB;
            case '/':
                if (numB === 0) return NaN; // Защита от деления на ноль
                return numA / numB;
            default: return numB;
        }
    }

    /**
     * Обрабатывает нажатие кнопки оператора (+, -, *, /)
     * @param {string} op - оператор
     */
    function handleOperator(op) {
        const currentNum = parseFloat(current);
        if (isNaN(currentNum)) return;

        // Если есть предыдущая операция, выполняем её перед новой
        if (previous !== null && operator !== null && !waitingForNewOperand) {
            const result = compute(previous, currentNum, operator);
            if (isNaN(result) || !isFinite(result)) {
                current = 'Ошибка';
                updateDisplay();
                previous = null;
                operator = null;
                waitingForNewOperand = true;
                return;
            }
            current = result.toString();
            updateDisplay();
        }

        // Сохраняем текущее значение как предыдущее и устанавливаем новый оператор
        previous = parseFloat(current);
        operator = op;
        waitingForNewOperand = true;
    }

    /**
     * Вычисляет результат при нажатии кнопки "="
     */
    function calculateEqual() {
        if (operator === null || previous === null || waitingForNewOperand) return;

        const currentNum = parseFloat(current);
        const result = compute(previous, currentNum, operator);

        if (isNaN(result) || !isFinite(result)) {
            current = 'Ошибка';
            updateDisplay();
            previous = null;
            operator = null;
            waitingForNewOperand = true;
            return;
        }

        current = result.toString();
        updateDisplay();
        // После вычисления сбрасываем состояние
        previous = null;
        operator = null;
        waitingForNewOperand = true;
    }

    /**
     * Сбрасывает ошибку, если она произошла
     */
    function handleErrorReset() {
        if (current === 'Ошибка') {
            clearAll();
        }
    }

    // ==================== НАВЕШИВАНИЕ ОБРАБОТЧИКОВ ====================

    // Обработчики для кнопок с цифрами
    document.querySelectorAll('[data-digit]').forEach(btn => {
        btn.addEventListener('click', () => {
            handleErrorReset();
            inputDigit(btn.dataset.digit);
        });
    });

    // Обработчики для кнопок операторов
    document.querySelectorAll('[data-op]').forEach(btn => {
        btn.addEventListener('click', () => {
            handleErrorReset();
            handleOperator(btn.dataset.op);
        });
    });

    // Обработчики для кнопок действий (C, +/-, %, =)
    document.querySelectorAll('[data-action]').forEach(btn => {
        btn.addEventListener('click', () => {
            const action = btn.dataset.action;
            switch (action) {
                case 'clear':
                    clearAll();
                    break;
                case 'sign':
                    if (current !== 'Ошибка') changeSign();
                    else clearAll();
                    break;
                case 'percent':
                    if (current !== 'Ошибка') percentOperation();
                    else clearAll();
                    break;
                case 'equal':
                    if (current !== 'Ошибка') calculateEqual();
                    else clearAll();
                    break;
            }
        });
    });

    // ==================== ПОДДЕРЖКА КЛАВИАТУРЫ ====================

    /**
     * Обработка нажатий клавиш для удобства использования
     * Поддерживаются: цифры, точка, операторы, Enter, Escape, %
     */
    document.addEventListener('keydown', (e) => {
        const key = e.key;

        // Цифры
        if (/^[0-9]$/.test(key)) {
            e.preventDefault();
            handleErrorReset();
            inputDigit(key);
        }

        // Десятичная точка
        if (key === '.') {
            e.preventDefault();
            handleErrorReset();
            inputDigit('.');
        }

        // Операторы
        if (key === '+') {
            e.preventDefault();
            handleErrorReset();
            handleOperator('+');
        }

        if (key === '-') {
            e.preventDefault();
            handleErrorReset();
            handleOperator('-');
        }

        if (key === '*') {
            e.preventDefault();
            handleErrorReset();
            handleOperator('*');
        }

        if (key === '/') {
            e.preventDefault();
            handleErrorReset();
            handleOperator('/');
        }

        // Равно (Enter или =)
        if (key === 'Enter' || key === '=') {
            e.preventDefault();
            if (current !== 'Ошибка') calculateEqual();
            else clearAll();
        }

        // Очистка (Escape или Delete)
        if (key === 'Escape' || key === 'Delete') {
            e.preventDefault();
            clearAll();
        }

        // Процент
        if (key === '%') {
            e.preventDefault();
            if (current !== 'Ошибка') percentOperation();
            else clearAll();
        }
    });

    // Инициализация отображения
    updateDisplay();
})();
