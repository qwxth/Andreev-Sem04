import { ProductPage } from "../../pages/product/index.js";

export class ProductSwitchComponent {
    constructor(parent) {
        this.parent = parent;
        this.baseProducts = {
            osago: {
                key: "osago",
                title: "ОСАГО",
                description: "Обязательное страхование автогражданской ответственности. Защитите себя от расходов при ДТП. Оформите полис за 5 минут без посещения офиса.",
                price: "от 2 490 ₽",
                image: "images/osa.webp",
                modelPath: "models/chevrolet.glb",
                id: 1
            },
            kasko: {
                key: "kasko",
                title: "КАСКО",
                description: "Полная защита автомобиля от угона, повреждений и любых неприятностей. Ремонт на СТО официальных дилеров.",
                price: "от 15 800 ₽",
                image: "images/kasko.webp",
                modelPath: "models/chevrolet1.glb",
                id: 2
            },
            green: {
                key: "green",
                title: "Зелёная карта",
                description: "Страхование ответственности для поездок за границу. Действует в 44 странах. Быстрое оформление онлайн.",
                price: "от 800 ₽/день",
                image: "images/strah.webp",
                modelPath: "models/porshe.glb",
                id: 3
            },
            home: {
                key: "home",
                title: "Страхование квартир",
                description: "Защитите жильё и имущество от огня, воды, кражи. Гибкие тарифы и мгновенное урегулирование.",
                price: "от 500 ₽/мес",
                image: "images/home.webp",
                modelPath: "models/porshe1.glb",
                id: 4
            }
        };
        this.products = { ...this.baseProducts };
        this.currentProductKey = 'osago';
        this.nextCustomId = 100;
    }

    escapeHtml(str) {
        if (!str) return '';
        return str.replace(/[&<>]/g, function(m) {
            if (m === '&') return '&amp;';
            if (m === '<') return '&lt;';
            if (m === '>') return '&gt;';
            return m;
        });
    }

    getHTML() {
        const productKeys = Object.keys(this.products);
        const tabsHtml = productKeys.map(key => `
            <button class="tab-btn ${key === this.currentProductKey ? 'active' : ''}" data-product="${key}">
                ${this.escapeHtml(this.products[key].title)}
            </button>
        `).join('');
        
        const plusButton = `<button id="add-tab-btn" class="tab-btn add-btn">+</button>`;
        const currentProduct = this.products[this.currentProductKey];
        
        return `
            <div class="product-card">
                <div class="tab-buttons" id="product-tabs">
                    ${tabsHtml}
                    ${plusButton}
                </div>
                <div class="product-image-container" id="product-image-container" style="background-image: url('${currentProduct.image}');">
                    <div class="product-overlay">
                        <h2 id="product-title">${this.escapeHtml(currentProduct.title)}</h2>
                        <div class="product-price" id="product-price">${currentProduct.price}</div>
                        <p id="product-desc">${this.escapeHtml(currentProduct.description)}</p>
                        <div>
                            <button class="btn btn-red me-3" id="product-detail-btn">Подробнее</button>
                            <button class="btn btn-outline-red" id="product-order-btn">Оформить онлайн</button>
                        </div>
                    </div>
                </div>
            </div>
        `;
    }

    switchTo(productKey) {
        const product = this.products[productKey];
        if (!product) return;
        const container = document.getElementById('product-image-container');
        if (container) container.style.backgroundImage = `url('${product.image}')`;
        const titleEl = document.getElementById('product-title');
        if (titleEl) titleEl.innerText = product.title;
        const priceEl = document.getElementById('product-price');
        if (priceEl) priceEl.innerText = product.price;
        const descEl = document.getElementById('product-desc');
        if (descEl) descEl.innerText = product.description;
        this.currentProductKey = productKey;
    }

    addNewTab() {
        let newTitle = prompt("Введите название новой вкладки:", "Моя страховка");
        if (newTitle === null) return;
        newTitle = newTitle.trim();
        if (newTitle === "") newTitle = "Новая страховка";
        
        // Копируем данные из ОСАГО (как просили)
        const source = this.baseProducts.osago;
        const newKey = `custom_${this.nextCustomId++}`;
        this.products[newKey] = {
            ...source,
            title: newTitle,
            key: newKey,
            id: this.nextCustomId
        };
        
        // Перерисовываем
        this.render();
        // Переключаемся на новую вкладку
        this.switchTo(newKey);
        // Обновляем активный класс на кнопках
        this.updateActiveClass(newKey);
    }

    updateActiveClass(activeKey) {
        const buttons = document.querySelectorAll('.tab-btn[data-product]');
        buttons.forEach(btn => {
            if (btn.dataset.product === activeKey) {
                btn.classList.add('active');
            } else {
                btn.classList.remove('active');
            }
        });
    }

    setupListeners() {
        // Обработчик для вкладок (статические + динамические)
        const tabButtons = document.querySelectorAll('.tab-btn[data-product]');
        tabButtons.forEach(btn => {
            btn.removeEventListener('click', this._tabClickHandler);
            this._tabClickHandler = (e) => {
                const key = btn.dataset.product;
                if (key) {
                    this.switchTo(key);
                    this.updateActiveClass(key);
                }
            };
            btn.addEventListener('click', this._tabClickHandler);
        });

        // Кнопка "+"
        const addBtn = document.getElementById('add-tab-btn');
        if (addBtn) {
            addBtn.removeEventListener('click', this._addHandler);
            this._addHandler = () => this.addNewTab();
            addBtn.addEventListener('click', this._addHandler);
        }

        // Кнопка "Подробнее"
        const detailBtn = document.getElementById('product-detail-btn');
        if (detailBtn) {
            detailBtn.removeEventListener('click', this._detailHandler);
            this._detailHandler = () => {
                const product = this.products[this.currentProductKey];
                if (product) {
                    const root = document.getElementById('root');
                    const productPage = new ProductPage(root, product.id, product.modelPath);
                    productPage.render();
                } else {
                    console.error("Текущий продукт не найден");
                }
            };
            detailBtn.addEventListener('click', this._detailHandler);
        }

        // Кнопка "Оформить онлайн"
        const orderBtn = document.getElementById('product-order-btn');
        if (orderBtn) {
            orderBtn.removeEventListener('click', this._orderHandler);
            this._orderHandler = () => alert('Демо-режим. Для оформления перейдите на официальный сайт.');
            orderBtn.addEventListener('click', this._orderHandler);
        }
    }

    render() {
        this.parent.innerHTML = this.getHTML();
        this.setupListeners();
    }
}
