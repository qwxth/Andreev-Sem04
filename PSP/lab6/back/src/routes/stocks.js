const express = require('express');
const router = express.Router();
const ctrl = require('../controllers/stocksController');
router.get('/', ctrl.getAllStocks);
router.get('/:id', ctrl.getStockById);
router.post('/', ctrl.createStock);
router.patch('/:id', ctrl.updateStock);
router.delete('/:id', ctrl.deleteStock);
module.exports = router;
