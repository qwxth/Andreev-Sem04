import { ajax } from "../../modules/ajax.js";
import { stockUrls } from "../../modules/stockUrls.js";
import { MainPage } from "../main/index.js";
import { BackButtonComponent } from "../../components/back-button/index.js";

export class CreatePage {
    constructor(parent) {
        this.parent = parent;
        this.submitHandler = null;
    }

    getHTML() {
        return `
            <div class="container py-5">
                <div class="row justify-content-center">
                    <div class="col-md-6">
                        <div class="card p-4 shadow-sm">
                            <h2 class="text-center mb-4">Создать новую карточку</h2>
                            <form id="create-form">
                                <div class="mb-3">
                                    <label class="form-label fw-bold">Название *</label>
                                    <input type="text" class="form-control" id="title" required>
                                </div>
                                <div class="mb-3">
                                    <label class="form-label fw-bold">Описание *</label>
                                    <textarea class="form-control" id="description" rows="3" required></textarea>
                                </div>
                                <div class="mb-3">
                                    <label class="form-label fw-bold">Цена</label>
                                    <input type="text" class="form-control" id="price" value="от 2 490 ₽">
                                </div>
                                <div class="mb-3">
                                    <label class="form-label fw-bold">URL изображения</label>
                                    <input type="text" class="form-control" id="image" value="https://picsum.photos/id/107/400/300" placeholder="https://...">
                                </div>
                                <div class="mb-3">
                                    <label class="form-label fw-bold">Путь к 3D модели</label>
                                    <input type="text" class="form-control" id="modelPath" value="models/chevrolet.glb">
                                </div>
                                <button type="submit" class="btn btn-success w-100 py-2 fw-bold" id="submit-btn">➕ Создать карточку</button>
                            </form>
                        </div>
                    </div>
                </div>
            </div>
        `;
    }

    render() {
        this.parent.innerHTML = '';
        this.parent.insertAdjacentHTML('beforeend', this.getHTML());
        new BackButtonComponent(this.parent).render(() => new MainPage(this.parent).render());

        const form = document.getElementById('create-form');
        if (!form) return;

        if (this.submitHandler) {
            form.removeEventListener('submit', this.submitHandler);
        }

        this.submitHandler = (e) => {
            e.preventDefault();
            const submitBtn = document.getElementById('submit-btn');
            if (submitBtn.disabled) return;
            submitBtn.disabled = true;
            submitBtn.textContent = 'Создание...';

            const title = document.getElementById('title').value.trim();
            const description = document.getElementById('description').value.trim();
            const price = document.getElementById('price').value.trim();
            const image = document.getElementById('image').value.trim();
            const modelPath = document.getElementById('modelPath').value.trim();

            if (!title || !description) {
                alert('Заполните название и описание');
                submitBtn.disabled = false;
                submitBtn.textContent = '➕ Создать карточку';
                return;
            }

            ajax.post(stockUrls.createStock(), {
                title,
                description,
                price,
                image,
                modelPath
            }, (data, status) => {
                submitBtn.disabled = false;
                submitBtn.textContent = '➕ Создать карточку';
                if (status === 201 && data) {
                    alert('Карточка создана!');
                    new MainPage(this.parent).render();
                } else {
                    alert('Ошибка: ' + (data?.error || 'неизвестная ошибка'));
                }
            });
        };
        form.addEventListener('submit', this.submitHandler);
    }
}
