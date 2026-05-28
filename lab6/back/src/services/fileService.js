const fs = require('fs');
const readData = (filePath) => {
    try { return JSON.parse(fs.readFileSync(filePath, 'utf8')); }
    catch (err) { console.error(err); return []; }
};
const writeData = (filePath, data) => {
    try { fs.writeFileSync(filePath, JSON.stringify(data, null, 2)); }
    catch (err) { console.error(err); }
};
module.exports = { readData, writeData };
