const express = require('express');
const path = require('path');
const stocksRouter = require('./routes/stocks');
const stocksService = require('./services/stocksService');

const app = express();
const PORT = 3000;
const DATA_FILE = path.join(__dirname, 'data/stocks.json');

stocksService.init(DATA_FILE);
app.use(express.json());

// Раздача статики из папки public (собранный фронтенд)
app.use(express.static(path.join(__dirname, '../public')));

// CORS уже не нужен, так как всё на одном порту, но оставим на всякий случай
app.use((req, res, next) => {
    res.header('Access-Control-Allow-Origin', '*');
    res.header('Access-Control-Allow-Methods', 'GET,POST,PATCH,DELETE,OPTIONS');
    res.header('Access-Control-Allow-Headers', 'Content-Type');
    if (req.method === 'OPTIONS') return res.sendStatus(204);
    next();
});

app.use('/stocks', stocksRouter);

// Все остальные запросы отправляем на index.html (для SPA)
app.get('*', (req, res) => {
    res.sendFile(path.join(__dirname, '../public/index.html'));
});

app.use((req, res) => res.status(404).json({ error: 'Not found' }));
app.use((err, req, res, next) => {
    console.error(err);
    res.status(500).json({ error: 'Server error' });
});

app.listen(PORT, () => {
    console.log(`✅ Сервер запущен на http://localhost:${PORT}`);
    console.log(`📁 Статика из папки: ${path.join(__dirname, '../public')}`);
});
