const express = require('express');
const path = require('path');
const fs = require('fs');
const app = express();
const PORT = 3000;
const DATA_FILE = path.join(__dirname, 'data/stocks.json');

// CORS middleware (разрешаем любые запросы)
app.use((req, res, next) => {
    res.header('Access-Control-Allow-Origin', '*');
    res.header('Access-Control-Allow-Methods', 'GET, POST, PATCH, DELETE, OPTIONS');
    res.header('Access-Control-Allow-Headers', 'Content-Type');
    if (req.method === 'OPTIONS') {
        return res.sendStatus(204);
    }
    next();
});

app.use(express.json());

// Функции чтения/записи
const readData = () => {
    try {
        const data = fs.readFileSync(DATA_FILE, 'utf8');
        return JSON.parse(data);
    } catch (err) {
        console.error(err);
        return [];
    }
};
const writeData = (data) => {
    try {
        fs.writeFileSync(DATA_FILE, JSON.stringify(data, null, 2));
    } catch (err) {
        console.error(err);
    }
};

// Маршруты
app.get('/stocks', (req, res) => {
    const stocks = readData();
    res.json(stocks);
});

app.get('/stocks/:id', (req, res) => {
    const id = parseInt(req.params.id);
    const stocks = readData();
    const stock = stocks.find(s => s.id === id);
    if (!stock) return res.status(404).json({ error: 'Not found' });
    res.json(stock);
});

app.post('/stocks', (req, res) => {
    const newData = req.body;
    const stocks = readData();
    const newId = stocks.length ? Math.max(...stocks.map(s => s.id)) + 1 : 1;
    const newStock = { id: newId, ...newData };
    stocks.push(newStock);
    writeData(stocks);
    res.status(201).json(newStock);
});

app.patch('/stocks/:id', (req, res) => {
    const id = parseInt(req.params.id);
    const stocks = readData();
    const index = stocks.findIndex(s => s.id === id);
    if (index === -1) return res.status(404).json({ error: 'Not found' });
    stocks[index] = { ...stocks[index], ...req.body };
    writeData(stocks);
    res.json(stocks[index]);
});

app.delete('/stocks/:id', (req, res) => {
    const id = parseInt(req.params.id);
    let stocks = readData();
    const filtered = stocks.filter(s => s.id !== id);
    if (filtered.length === stocks.length) return res.status(404).json({ error: 'Not found' });
    writeData(filtered);
    res.status(204).send();
});

// Обработка 404
app.use((req, res) => res.status(404).json({ error: 'Not found' }));
app.use((err, req, res, next) => {
    console.error(err);
    res.status(500).json({ error: 'Server error' });
});

app.listen(PORT, () => console.log(`Server running at http://localhost:${PORT}`));
