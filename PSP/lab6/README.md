
# Лабораторная работа №6. Moon Reports Frontend (fetch + async/await + Bundler)

## Оглавление
1. [Цель лабораторной работы](#1-цель-лабораторной-работы)
2. [Часть 1. Замена XHR на fetch](#2-часть-1-замена-xhr-на-fetch)
3. [Часть 2. Сборка фронтенда через Vite](#3-часть-2-сборка-фронтенда-через-vite)
4. [Часть 3. Раздача статики через бэкенд](#4-часть-3-раздача-статики-через-бэкенд)
5. [Скриншоты работающего приложения](#5-скриншоты-работающего-приложения)
6. [Важные части кода](#6-важные-части-кода)
7. [Структура проекта](#7-структура-проекта)
8. [Вывод](#8-вывод)

---

## 1. Цель лабораторной работы
- Переписать фронтенд с XHR на fetch + async/await.
- Использовать Promise для асинхронной работы.
- Добавить кнопку «Сохранить» для добавления новых лунных отчётов.
- Собрать фронтенд через bundler (Vite).
- Раздача фронтенда через бэкенд для устранения проблем с CORS.

---

## 2. Часть 1. Замена XHR на fetch

### main.js
```javascript
import { getReports } from './modules/reportApi.js';

async function loadReports() {
  const reports = await getReports();
  const container = document.getElementById('reportsContainer');
  container.innerHTML = '';
  reports.forEach(r => {
    const div = document.createElement('div');
    div.className = 'report-card';
    div.innerHTML = `<strong>${r.title}</strong><br>${r.description}<br>${r.category}`;
    container.appendChild(div);
  });
}

document.addEventListener('DOMContentLoaded', loadReports);
````

### modules/reportApi.js

```javascript
import { API_BASE_URL } from './reportUrls.js';

export async function getReports(query = '') {
  try {
    const res = await fetch(`${API_BASE_URL}?title=${encodeURIComponent(query)}`);
    if (!res.ok) throw new Error(res.status);
    return await res.json();
  } catch (err) {
    console.error('Ошибка получения отчётов:', err);
    return [];
  }
}

export async function addReport(reportData) {
  try {
    const res = await fetch(API_BASE_URL, {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify(reportData)
    });
    if (!res.ok) throw new Error(res.status);
    return await res.json();
  } catch (err) {
    console.error('Ошибка добавления отчёта:', err);
  }
}
```

### pages/add.html

```html
<form id="addReportForm">
  <input id="titleInput" placeholder="Название" required>
  <textarea id="descriptionInput" placeholder="Описание"></textarea>
  <input id="categoryInput" placeholder="Категория">
  <button type="submit" id="saveButton">Сохранить</button>
</form>

<script type="module">
import { addReport } from '../modules/reportApi.js';

document.getElementById('saveButton').addEventListener('click', async (e) => {
  e.preventDefault();
  const newReport = {
    title: document.getElementById('titleInput').value,
    description: document.getElementById('descriptionInput').value,
    category: document.getElementById('categoryInput').value,
    status: 'Черновик'
  };
  const result = await addReport(newReport);
  if (result) alert(`Добавлен отчёт с ID: ${result.id}`);
});
</script>
```

---

## 3. Часть 2. Сборка фронтенда через Vite

**vite.config.js**

```javascript
export default {
  build: {
    outDir: './public',
    emptyOutDir: true
  }
};
```

**package.json**

```json
{
  "scripts": {
    "dev": "vite",
    "build": "vite build",
    "preview": "vite preview"
  },
  "devDependencies": {
    "vite": "^5.0.0"
  }
}
```

**Команды:**

* `npm run dev` – запуск dev сервера
* `npm run build` – сборка bundle
* `npm run preview` – просмотр собранного проекта

---

## 4. Часть 3. Раздача статики через бэкенд

```javascript
const path = require('path');
app.use(express.static(path.join(__dirname, 'public')));
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});
```

**Процесс сборки и раздачи:**

```bash
cd lab_6
npm run build
cp -r public/* ../lab_4_bundle/public/
cd ../lab_4_bundle
npm run dev
```

---

## 5. Скриншоты работающего приложения


![Главная страница](labs/lab_6/screenshots/home.png)



---

## 6. Важные части кода

* main.js – загрузка списка карточек через fetch
* modules/reportApi.js – методы getReports и addReport
* add.html – форма добавления с кнопкой «Сохранить»

---

## 7. Структура проекта

```
lab_6/
├── index.html
├── main.js
├── modules/
│   ├── reportApi.js
│   └── reportUrls.js
├── pages/
│   ├── add.html
│   ├── list.html
│   └── view.html
├── components/
│   └── reportCard.js
├── styles.css
└── screenshots/
    ├── home.png
    ├── add.png
    └── list.png
```

---

## 8. Вывод

* XHR заменён на fetch с async/await.
* Кнопка «Сохранить» добавляет отчёты на backend.
* Фронтенд собран через Vite в bundle.
* Раздача фронтенда через бэкенд устранила проблему CORS.

---


