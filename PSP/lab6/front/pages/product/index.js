import { stockUrls } from "../../modules/stockUrls.js";
import { BackButtonComponent } from "../../components/back-button/index.js";
import { MainPage } from "../main/index.js";
import { Car3DComponent } from "../../components/car-3d/index.js";

export class ProductPage {
    constructor(parent, id) {
        this.parent = parent;
        this.id = id;
        this.product = null;
    }

    getHTML() {
        return `
            <div class="container py-5">
                <div class="row g-5 align-items-center">
                    <div class="col-md-6">
                        <div id="car-3d-container" style="width:100%; height:400px; border-radius:20px; overflow:hidden; position:relative;"></div>
                    </div>
                    <div class="col-md-6">
                        <div id="product-detail-info"></div>
                    </div>
                </div>
            </div>
        `;
    }

    async loadData() {
        try {
            const response = await fetch(stockUrls.getStockById(this.id));
            if (!response.ok) throw new Error(`HTTP ${response.status}`);
            const data = await response.json();
            this.renderData(data);
        } catch (err) {
            console.error(err);
            alert('Карточка не найдена');
            new MainPage(this.parent).render();
        }
    }

    renderData(item) {
        this.product = item;
        const infoDiv = document.getElementById('product-detail-info');
        infoDiv.innerHTML = `
            <h1 class="display-5 fw-bold mb-3">${this.escapeHtml(item.title)}</h1>
            <div class="fs-2 text-danger fw-bold mb-3">${item.price || "от 2 490 ₽"}</div>
            <p class="mt-3 lead text-secondary">${this.escapeHtml(item.description)}</p>
        `;
        const container = document.getElementById('car-3d-container');
        if (item.modelPath) {
            new Car3DComponent(container, item.modelPath).render();
        }
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

    render() {
        this.parent.innerHTML = '';
        this.parent.insertAdjacentHTML('beforeend', this.getHTML());
        new BackButtonComponent(this.parent).render(() => new MainPage(this.parent).render());
        this.loadData();
    }
}
