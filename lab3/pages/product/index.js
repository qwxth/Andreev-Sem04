import { BackButtonComponent } from "../../components/back-button/index.js";
import { MainPage } from "../main/index.js";
import { Car3DComponent } from "../../components/car-3d/index.js";

export class ProductPage {
    constructor(parent, id, modelPath) {
        this.parent = parent;
        this.id = id;
        this.modelPath = modelPath;
    }
    getHTML() {
        return `<div class="container py-5"><div class="row g-5 align-items-center"><div class="col-md-6"><div id="car-3d-container" style="width:100%; height:400px; border-radius:20px; overflow:hidden; position:relative;"></div></div><div class="col-md-6"><div id="product-detail-info"></div></div></div></div>`;
    }
    getData() {
        const map = {
            1: { title: "ОСАГО", fullText: "Обязательное страхование автогражданской ответственности. Покрытие до 500 000 руб. за вред здоровью, до 400 000 руб. за имущество. Оформите полис онлайн за 5 минут.", price: "от 2 490 ₽" },
            2: { title: "КАСКО", fullText: "Полная защита от угона, повреждений. Включает ремонт на СТО дилеров, эвакуатор, помощь на дороге. Гибкая франшиза.", price: "от 15 800 ₽" },
            3: { title: "Зелёная карта", fullText: "Международный полис для поездок за границу. Покрытие до 1 млн евро. Действует в 44 странах.", price: "от 800 ₽/день" },
            4: { title: "Страхование квартир", fullText: "Защита стен, отделки, имущества от огня, воды, кражи. Выплаты за 3 дня.", price: "от 500 ₽/мес" }
        };
        return map[this.id];
    }
    clickBack() { new MainPage(this.parent).render(); }
    render() {
        this.parent.innerHTML = '';
        this.parent.insertAdjacentHTML('beforeend', this.getHTML());
        new BackButtonComponent(this.parent).render(this.clickBack.bind(this));
        const data = this.getData();
        const infoDiv = document.getElementById('product-detail-info');
        infoDiv.innerHTML = `<h1 class="display-5 fw-bold">${data.title}</h1><div class="fs-2 text-danger fw-bold mt-3">${data.price}</div><p class="mt-4 lead">${data.fullText}</p><button class="btn btn-red btn-lg mt-3" id="detail-order-btn">Оформить онлайн</button>`;
        document.getElementById('detail-order-btn')?.addEventListener('click', () => alert('Демо. Оформление на официальном сайте.'));
        const container = document.getElementById('car-3d-container');
        const car3d = new Car3DComponent(container, this.modelPath);
        car3d.render();
    }
}
