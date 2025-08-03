// index.js
const mmap = require('./build/Release/mmap-io.node');

module.exports = {
  PROT_READ: mmap.PROT_READ,
  PROT_WRITE: mmap.PROT_WRITE,
  PROT_EXEC: mmap.PROT_EXEC,
  PROT_NONE: mmap.PROT_NONE,

  MAP_SHARED: mmap.MAP_SHARED,
  MAP_PRIVATE: mmap.MAP_PRIVATE,
  MAP_NONBLOCK: mmap.MAP_NONBLOCK,
  MAP_POPULATE: mmap.MAP_POPULATE,

  MADV_NORMAL: mmap.MADV_NORMAL,
  MADV_RANDOM: mmap.MADV_RANDOM,
  MADV_SEQUENTIAL: mmap.MADV_SEQUENTIAL,
  MADV_WILLNEED: mmap.MADV_WILLNEED,
  MADV_DONTNEED: mmap.MADV_DONTNEED,

  map: mmap.map,
  advise: mmap.advise,
  incore: mmap.incore,
  sync: mmap.sync_lib_private__ // wrapped inside addon
};

console.log("✅ mmap-io-latest successfully installed! Happy Coding! 🎉 by Karan Vallecha 🥳");
