import { ajax } from "../../modules/ajax.js";
import { stockUrls } from "../../modules/stockUrls.js";
import { ProductPage } from "../../pages/product/index.js";
import { CreatePage } from "../../pages/create/index.js";

export class ProductSwitchComponent {
    constructor(parent) {
        this.parent = parent;
        this.products = {};
        this.currentKey = null;
    }

    async loadProducts() {
        return new Promise((resolve) => {
            ajax.get(stockUrls.getStocks(), (data, status) => {
                if (status === 200 && Array.isArray(data)) {
                    this.products = {};
                    data.forEach(p => {
                        const key = `product_${p.id}`;
                        this.products[key] = {
                            key: key,
                            id: p.id,
                            title: p.title,
                            description: p.description,
                            price: p.price || "от 2 490 ₽",
                            image: p.image || "https://picsum.photos/id/107/400/300",
                            modelPath: p.modelPath || "models/chevrolet.glb"
                        };
                    });
                    if (Object.keys(this.products).length) {
                        this.currentKey = Object.keys(this.products)[0];
                    } else {
                        this.currentKey = null;
                    }
                } else {
                    this.products = {};
                    this.currentKey = null;
                }
                resolve();
            });
        });
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
        const isEmpty = !this.products || Object.keys(this.products).length === 0;

        // Блок вкладок: если есть карточки – рендерим их кнопки, иначе – только кнопку "+"
        let tabsHtml = '';
        if (!isEmpty) {
            tabsHtml = Object.keys(this.products).map(key => `
                <button class="tab-btn ${key === this.currentKey ? 'active' : ''}" data-product="${key}">
                    ${this.escapeHtml(this.products[key].title)}
                </button>
            `).join('');
        }

        const plusButton = `<button id="add-tab-btn" class="tab-btn add-btn" style="background-color: #28a745; color: white; font-weight: bold;">+</button>`;

        if (isEmpty) {
            // Нет карточек – показываем сообщение, а кнопка "+" уже в строке вкладок
            return `
                <div class="product-card">
                    <div class="tab-buttons" id="product-tabs">
                        ${plusButton}
                    </div>
                    <div class="text-center p-5">
                        <h3 class="text-secondary">📭 Нет карточек</h3>
                        <p class="mt-3">Нажмите кнопку <strong>+</strong> выше, чтобы добавить первую страховку.</p>
                    </div>
                </div>
            `;
        }

        // Есть карточки – рендерим текущую
        const current = this.products[this.currentKey];
        if (!current) return `<div class="product-card"><div class="text-center p-5">Ошибка загрузки</div></div>`;

        return `
            <div class="product-card">
                <div class="tab-buttons" id="product-tabs">
                    ${tabsHtml}
                    ${plusButton}
                </div>
                <div class="product-image-container" id="product-image-container" style="background-image: url('${current.image}'); position: relative;">
                    <button class="delete-card-btn" data-id="${current.id}" data-key="${this.currentKey}" style="position: absolute; top: 10px; right: 10px; background: rgba(220,53,69,0.9); border: none; color: white; font-size: 20px; font-weight: bold; width: 32px; height: 32px; border-radius: 50%; cursor: pointer; z-index: 10;" title="Удалить карточку">×</button>
                    <div class="product-overlay">
                        <h2 id="product-title">${this.escapeHtml(current.title)}</h2>
                        <div class="product-price" id="product-price">${current.price}</div>
                        <p id="product-desc">${this.escapeHtml(current.description)}</p>
                        <div>
                            <button class="btn btn-red me-3" id="product-detail-btn">Подробнее</button>
                        </div>
                    </div>
                </div>
            </div>
        `;
    }

    switchTo(key) {
        const product = this.products[key];
        if (!product) return;
        const container = document.getElementById('product-image-container');
        if (container) {
            container.style.backgroundImage = `url('${product.image}')`;
            const deleteBtn = container.querySelector('.delete-card-btn');
            if (deleteBtn) {
                deleteBtn.setAttribute('data-id', product.id);
                deleteBtn.setAttribute('data-key', key);
            }
        }
        const titleEl = document.getElementById('product-title');
        if (titleEl) titleEl.innerText = product.title;
        const priceEl = document.getElementById('product-price');
        if (priceEl) priceEl.innerText = product.price;
        const descEl = document.getElementById('product-desc');
        if (descEl) descEl.innerText = product.description;
        this.currentKey = key;
    }

    updateActiveClass(activeKey) {
        const buttons = document.querySelectorAll('.tab-btn[data-product]');
        buttons.forEach(btn => {
            if (btn.dataset.product === activeKey) btn.classList.add('active');
            else btn.classList.remove('active');
        });
    }

    async deleteCard(id, key) {
        if (!confirm(`Удалить карточку "${this.products[key].title}"?`)) return;
        ajax.delete(stockUrls.deleteStock(id), async (data, status) => {
            if (status === 204) {
                await this.loadProducts();
                this.render();
            } else {
                alert('Ошибка удаления');
            }
        });
    }

    setupListeners() {
        this.parent.addEventListener('click', (e) => {
            // Удаление через кнопку на карточке
            const deleteBtn = e.target.closest('.delete-card-btn');
            if (deleteBtn) {
                e.stopPropagation();
                const id = parseInt(deleteBtn.dataset.id);
                const key = deleteBtn.dataset.key;
                if (id && key) this.deleteCard(id, key);
                return;
            }
            // Переключение вкладок
            const tabBtn = e.target.closest('.tab-btn[data-product]');
            if (tabBtn) {
                const key = tabBtn.dataset.product;
                if (key) {
                    this.switchTo(key);
                    this.updateActiveClass(key);
                }
                return;
            }
            // Кнопка "+"
            if (e.target.closest('#add-tab-btn')) {
                new CreatePage(this.parent).render();
                return;
            }
            // Кнопка "Подробнее"
            if (e.target.closest('#product-detail-btn')) {
                const product = this.products[this.currentKey];
                if (product) {
                    new ProductPage(this.parent, product.id).render();
                }
                return;
            }
        });
    }

    async render() {
        await this.loadProducts();
        this.parent.innerHTML = this.getHTML();
        this.setupListeners(); // обработчик вешается всегда, даже если карточек нет
    }
}