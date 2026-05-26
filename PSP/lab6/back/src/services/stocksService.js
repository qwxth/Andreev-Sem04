const fileService = require('./fileService');
let dataFilePath;
const init = (p) => { dataFilePath = p; };
const findAll = (title) => {
    const stocks = fileService.readData(dataFilePath);
    if (title) return stocks.filter(s => s.title.toLowerCase().includes(title.toLowerCase()));
    return stocks;
};
const findOne = (id) => fileService.readData(dataFilePath).find(s => s.id === id);
const create = (data) => {
    const stocks = fileService.readData(dataFilePath);
    const newId = stocks.length ? Math.max(...stocks.map(s => s.id)) + 1 : 1;
    const newStock = { id: newId, ...data };
    stocks.push(newStock);
    fileService.writeData(dataFilePath, stocks);
    return newStock;
};
const update = (id, data) => {
    const stocks = fileService.readData(dataFilePath);
    const idx = stocks.findIndex(s => s.id === id);
    if (idx === -1) return null;
    stocks[idx] = { ...stocks[idx], ...data };
    fileService.writeData(dataFilePath, stocks);
    return stocks[idx];
};
const remove = (id) => {
    const stocks = fileService.readData(dataFilePath);
    const filtered = stocks.filter(s => s.id !== id);
    if (filtered.length === stocks.length) return false;
    fileService.writeData(dataFilePath, filtered);
    return true;
};
module.exports = { init, findAll, findOne, create, update, remove };
