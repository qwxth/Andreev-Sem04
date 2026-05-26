const express = require('express');
const path = require('path');
const stocksRouter = require('./routes/stocks');
const stocksService = require('./services/stocksService');
const app = express();
const PORT = 3000;
const DATA_FILE = path.join(__dirname, 'data/stocks.json');
stocksService.init(DATA_FILE);
app.use(express.json());
app.use((req, res, next) => {
    console.log(`[${new Date().toISOString()}] ${req.method} ${req.url}`);
    next();
});
app.use('/stocks', stocksRouter);
app.use((req, res) => res.status(404).json({ error: 'Not found' }));
app.use((err, req, res, next) => {
    console.error(err);
    res.status(500).json({ error: 'Server error' });
});
app.listen(PORT, () => console.log(`Server running at http://localhost:${PORT}`));
