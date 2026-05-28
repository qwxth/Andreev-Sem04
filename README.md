# Лабораторная работа №5. Альфа-Страхование (XHR + два сервера + CORS)

## Оглавление
1. [Цель лабораторной работы](#1-цель-лабораторной-работы)
2. [Часть 1. Работа с API через XHR](#2-часть-1-работа-с-api-через-xhr)
3. [Часть 2. Структура проекта](#3-часть-2-структура-проекта)
4. [Скриншоты работающего приложения](#4-скриншоты-работающего-приложения)
5. [Важные части кода](#5-важные-части-кода)
6. [Вывод](#6-вывод)

---

## 1. Цель лабораторной работы
- Научиться выполнять HTTP‑запросы к бэкенду с помощью `XMLHttpRequest`.
- Реализовать форму для создания новой страховой карточки (POST).
- Организовать взаимодействие фронтенда (Live Server, порт 5500) и бэкенда (Express, порт 3000) с преодолением CORS.

---

## 2. Часть 1. Работа с API через XHR

### modules/stockUrls.js
```javascript
class StockUrls {
    constructor() {
        this.baseUrl = 'http://localhost:3000';
    }
    getStocks() { return `${this.baseUrl}/stocks`; }
    getStockById(id) { return `${this.baseUrl}/stocks/${id}`; }
    createStock() { return `${this.baseUrl}/stocks`; }
    deleteStock(id) { return `${this.baseUrl}/stocks/${id}`; }
}
export const stockUrls = new StockUrls();

class Ajax {
    get(url, callback) {
        const xhr = new XMLHttpRequest();
        xhr.open('GET', url);
        xhr.send();
        xhr.onreadystatechange = () => {
            if (xhr.readyState === 4) this._handleResponse(xhr, callback);
        };
    }
    post(url, data, callback) {
        const xhr = new XMLHttpRequest();
        xhr.open('POST', url);
        xhr.setRequestHeader('Content-Type', 'application/json');
        xhr.send(JSON.stringify(data));
        xhr.onreadystatechange = () => {
            if (xhr.readyState === 4) this._handleResponse(xhr, callback);
        };
    }
    delete(url, callback) {
        const xhr = new XMLHttpRequest();
        xhr.open('DELETE', url);
        xhr.send();
        xhr.onreadystatechange = () => {
            if (xhr.readyState === 4) this._handleResponse(xhr, callback);
        };
    }
    _handleResponse(xhr, callback) {
        try {
            const data = xhr.responseText ? JSON.parse(xhr.responseText) : null;
            callback(data, xhr.status);
        } catch (e) {
            console.error(e);
            callback(null, xhr.status);
        }
    }
}
export const ajax = new Ajax();

import { ajax } from "../../modules/ajax.js";
import { stockUrls } from "../../modules/stockUrls.js";
import { ProductPage } from "../../pages/product/index.js";

export class ProductSwitchComponent {
    // ... другие методы

    loadProducts() {
        ajax.get(stockUrls.getStocks(), (data) => {
            this.products = {};
            data.forEach(p => {
                const key = `product_${p.id}`;
                this.products[key] = {
                    key, id: p.id,
                    title: p.title,
                    description: p.description,
                    price: p.price,
                    image: p.image,
                    modelPath: p.modelPath
                };
            });
            this.currentKey = Object.keys(this.products)[0];
            this.render();
        });
    }
}

import { ajax } from "../../modules/ajax.js";
import { stockUrls } from "../../modules/stockUrls.js";
import { MainPage } from "../main/index.js";

export class CreatePage {
    // ... другие методы

    setupForm() {
        const form = document.getElementById('create-form');
        form.addEventListener('submit', (e) => {
            e.preventDefault();
            const payload = {
                title: document.getElementById('title').value.trim(),
                description: document.getElementById('description').value.trim(),
                price: document.getElementById('price').value.trim(),
                image: document.getElementById('image').value.trim(),
                modelPath: document.getElementById('modelPath').value.trim()
            };
            ajax.post(stockUrls.createStock(), payload, (data, status) => {
                if (status === 201) {
                    alert('Карточка создана!');
                    new MainPage(this.parent).render();
                } else {
                    alert('Ошибка: ' + (data?.error || 'неизвестная ошибка'));
                }
            });
        });
    }
}

lab5/
├── index.html
├── main.js
├── modules/
│   ├── ajax.js
│   └── stockUrls.js
├── pages/
│   ├── main/index.js
│   ├── product/index.js
│   └── create/index.js
├── components/
│   ├── product-switch/index.js
│   ├── back-button/index.js
│   └── car-3d/index.js
├── images/
├── models/
└── styles.css

4. **Скриншоты работающего приложения**
https:///PSP/lab6/home.png

5. **Важные части кода**
modules/ajax.js – обёртка над XMLHttpRequest для GET, POST, DELETE.

components/product-switch/index.js – загрузка карточек и удаление через XHR.

pages/create/index.js – форма создания карточки (POST-запрос).

back/src/index.js – бэкенд Express, запускаемый на порту 3000.

6. **Вывод**
Реализовано полноценное CRUD-взаимодействие с бэкендом через XMLHttpRequest.

Добавлена возможность создания новых карточек (POST /stocks).

Фронтенд запускается отдельно (Live Server, порт 5500), бэкенд – на порту 3000. Для работы требуется расширение CORS или настройка заголовков на сервере.

Весь интерфейс сохранил красно‑белую стилистику, 3D‑модели загружаются через Car3DComponent.

Лабораторная работа №5 (вариант 2) выполнена полностью.