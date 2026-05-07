import { ProductSwitchComponent } from "../../components/product-switch/index.js";

export class MainPage {
    constructor(parent) { this.parent = parent; }
    getHTML() {
        return `
            <div class="top-bar">
                <div class="container">
                    <div class="row align-items-center">
                        <div class="col-md-6"><i class="fas fa-phone-alt me-2"></i> 8 800 333 97 97 <span class="ms-3"><i class="far fa-clock me-1"></i> 24/7</span></div>
                        <div class="col-md-6 text-end"><a href="#" class="text-decoration-none me-3"><i class="fas fa-map-marker-alt me-1"></i> Офисы</a><a href="#"><i class="fas fa-user me-1"></i> Личный кабинет</a></div>
                    </div>
                </div>
            </div>
            <div class="navbar-main">
                <div class="container">
                    <div class="d-flex justify-content-between align-items-center">
                        <div class="logo"><span>Альфа</span><span>Страхование</span></div>
                        <div class="d-none d-md-flex align-items-center">
                            <a href="#" class="nav-link-custom">Страхование</a>
                            <a href="#" class="nav-link-custom">О компании</a>
                            <a href="#" class="nav-link-custom">Помощь</a>
                            <a href="#" class="nav-link-custom">Контакты</a>
                            <button class="btn btn-red ms-4">Оплатить онлайн</button>
                        </div>
                    </div>
                </div>
            </div>
            <div class="hero">
                <div class="container">
                    <h1>Надёжная защита <span>вашего автомобиля</span></h1>
                    <p class="lead mt-3">Выберите продукт и оформите полис онлайн за 5 минут</p>
                </div>
            </div>
            <div class="product-showcase">
                <div class="container">
                    <div id="product-switch-container"></div>
                </div>
            </div>
            <div class="footer">
                <div class="container">
                    <div class="row">
                        <div class="col-md-4"><div class="footer-logo">АльфаСтрахование</div><p>© 2025 Все права защищены</p><div><a href="#" class="me-3"><i class="fab fa-vk fa-lg"></i></a><a href="#" class="me-3"><i class="fab fa-telegram fa-lg"></i></a><a href="#"><i class="fab fa-youtube fa-lg"></i></a></div></div>
                        <div class="col-md-2"><h6 class="text-white">Продукты</h6><p><a href="#">ОСАГО</a></p><p><a href="#">КАСКО</a></p><p><a href="#">Зелёная карта</a></p><p><a href="#">Страхование квартир</a></p></div>
                        <div class="col-md-2"><h6 class="text-white">Компания</h6><p><a href="#">О нас</a></p><p><a href="#">Новости</a></p><p><a href="#">Вакансии</a></p></div>
                        <div class="col-md-4"><h6 class="text-white">Контакты</h6><p><i class="fas fa-phone me-2"></i> 8 800 333 97 97</p><p><i class="fas fa-envelope me-2"></i> info@alfastrah.ru</p><p><i class="fas fa-map-marker-alt me-2"></i> г. Москва, ул. Шаболовка, 31</p></div>
                    </div>
                    <div class="copyright">Демонстрационный проект. Не является официальным сайтом.</div>
                </div>
            </div>
        `;
    }
    render() {
        this.parent.innerHTML = '';
        this.parent.insertAdjacentHTML('beforeend', this.getHTML());
        const container = document.getElementById('product-switch-container');
        if (container) new ProductSwitchComponent(container).render();
    }
}
