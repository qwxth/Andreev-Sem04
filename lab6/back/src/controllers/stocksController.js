const stocksService = require('../services/stocksService');
const getAllStocks = (req, res) => {
    const { title } = req.query;
    res.json(stocksService.findAll(title));
};
const getStockById = (req, res) => {
    const id = parseInt(req.params.id);
    const stock = stocksService.findOne(id);
    if (!stock) return res.status(404).json({ error: "Not found" });
    res.json(stock);
};
const createStock = (req, res) => {
    const { src, title, text, ...rest } = req.body;
    if (!title) {
        return res.status(400).json({ error: 'Поле title обязательно' });
    }
    // Если нет src или text, можно использовать значения по умолчанию
    const newStock = stocksService.create({ 
        src: src || '', 
        title, 
        text: text || '', 
        ...rest 
    });
    res.status(201).json(newStock);
};
const updateStock = (req, res) => {
    const id = parseInt(req.params.id);
    const updated = stocksService.update(id, req.body);
    if (!updated) return res.status(404).json({ error: "Not found" });
    res.json(updated);
};
const deleteStock = (req, res) => {
    const id = parseInt(req.params.id);
    const success = stocksService.remove(id);
    if (!success) return res.status(404).json({ error: "Not found" });
    res.status(204).send();
};
module.exports = { getAllStocks, getStockById, createStock, updateStock, deleteStock };
