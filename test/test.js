const mmap = require('../build/Release/mmap-io.node');
const fs = require('fs');

fs.writeFileSync('testfile.txt', 'Hello mmap world!');
const fd = fs.openSync('testfile.txt', 'r+');

const size = fs.statSync('testfile.txt').size;
const buf = mmap.map(size, mmap.PROT_READ | mmap.PROT_WRITE, mmap.MAP_SHARED, fd);

console.log('Buffer from mmap:', buf.toString());

// 🔹 Test advise (random read)
mmap.advise(buf, mmap.MADV_RANDOM);
console.log('advise() called successfully');

// 🔹 Test incore (pages in memory)
const incoreInfo = mmap.incore(buf);
console.log('incore() result:', incoreInfo); // [unmappedPages, mappedPages]

// 🔹 Test sync (flush changes)
buf.write('Hi!!!');
mmap.sync_lib_private__(buf, 0, buf.length, true, false);
console.log('sync() called successfully');

fs.closeSync(fd);
