import { ProductPage } from "../../pages/product/index.js";

export class ProductSwitchComponent {
    constructor(parent) {
        this.parent = parent;
        this.currentProduct = 'osago';
        this.products = {
            osago: {
                title: "ОСАГО",
                description: "Обязательное страхование автогражданской ответственности. Защитите себя от расходов при ДТП. Оформите полис за 5 минут без посещения офиса.",
                price: "от 2 490 ₽",
                image: "images/osa.webp",
                modelPath: "models/chevrolet.glb",
                id: 1
            },
            kasko: {
                title: "КАСКО",
                description: "Полная защита автомобиля от угона, повреждений и любых неприятностей. Ремонт на СТО официальных дилеров.",
                price: "от 15 800 ₽",
                image: "images/kasko.webp",
                modelPath: "models/chevrolet1.glb",
                id: 2
            },
            green: {
                title: "Зелёная карта",
                description: "Страхование ответственности для поездок за границу. Действует в 44 странах. Быстрое оформление онлайн.",
                price: "от 800 ₽/день",
                image: "images/strah.webp",
                modelPath: "models/porshe.glb",
                id: 3
            },
            home: {
                title: "Страхование квартир",
                description: "Защитите жильё и имущество от огня, воды, кражи. Гибкие тарифы и мгновенное урегулирование.",
                price: "от 500 ₽/мес",
                image: "images/home.webp",
                modelPath: "models/porshe1.glb",
                id: 4
            }
        };
    }

    getHTML() {
        const p = this.products.osago;
        return `<div class="product-card">
            <div class="tab-buttons" id="product-tabs">
                <button class="tab-btn active" data-product="osago">ОСАГО</button>
                <button class="tab-btn" data-product="kasko">КАСКО</button>
                <button class="tab-btn" data-product="green">Зелёная карта</button>
                <button class="tab-btn" data-product="home">Страхование квартир</button>
            </div>
            <div class="product-image-container" id="product-image-container" style="background-image: url('${p.image}');">
                <div class="product-overlay">
                    <h2 id="product-title">${p.title}</h2>
                    <div class="product-price" id="product-price">${p.price}</div>
                    <p id="product-desc">${p.description}</p>
                    <div><button class="btn btn-red me-3" id="product-detail-btn">Подробнее</button><button class="btn btn-outline-red" id="product-order-btn">Оформить онлайн</button></div>
                </div>
            </div>
        </div>`;
    }

    switchTo(productKey) {
        const p = this.products[productKey];
        if (!p) return;
        document.getElementById('product-image-container').style.backgroundImage = `url('${p.image}')`;
        document.getElementById('product-title').innerText = p.title;
        document.getElementById('product-price').innerText = p.price;
        document.getElementById('product-desc').innerText = p.description;
        this.currentProduct = productKey;
    }

    setupListeners() {
        document.querySelectorAll('.tab-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                this.switchTo(btn.dataset.product);
            });
        });
        document.getElementById('product-detail-btn').addEventListener('click', () => {
            const p = this.products[this.currentProduct];
            const root = document.getElementById('root');
            const productPage = new ProductPage(root, p.id, p.modelPath);
            productPage.render();
        });
        document.getElementById('product-order-btn').addEventListener('click', () => alert('Демо-режим. Для оформления перейдите на официальный сайт.'));
    }

    render() { this.parent.innerHTML = this.getHTML(); this.setupListeners(); }
}
