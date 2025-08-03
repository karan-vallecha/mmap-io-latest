# mmap-io-latest

Updated Node.js/C++ **mmap** bindings with TypeScript support and Node 20+ compatibility.
This library provides a simple interface to memory-map files in Node.js using native bindings.

## 🚀 Features

* ✅ **Memory-mapped file support** using native `mmap` system calls
* ✅ **Node.js 20+ and N-API support**
* ✅ **TypeScript definitions included**
* ✅ **Cross-platform** (Linux, macOS, Windows – limited support on Windows)

---

## 📦 Installation

```bash
npm install mmap-io-latest
```

---

## 📖 Usage Example

```js
const fs = require("fs");
const mmap = require("mmap-io-latest");

const fd = fs.openSync("testfile.txt", "r+");
const stats = fs.fstatSync(fd);

// Map the entire file into memory
const buffer = mmap.map(
  stats.size,
  mmap.PROT_READ | mmap.PROT_WRITE,
  mmap.MAP_SHARED,
  fd,
  0,
  mmap.MADV_NORMAL
);

console.log("File contents:", buffer.toString());
fs.closeSync(fd);
```

---

## 🔑 API Reference

### Constants

| Constant          | Description                       |
| ----------------- | --------------------------------- |
| `PROT_READ`       | Pages may be read                 |
| `PROT_WRITE`      | Pages may be written              |
| `PROT_EXEC`       | Pages may be executed             |
| `PROT_NONE`       | Pages cannot be accessed          |
| `MAP_SHARED`      | Changes are shared                |
| `MAP_PRIVATE`     | Changes are private               |
| `MADV_NORMAL`     | No special treatment              |
| `MADV_RANDOM`     | Expect random page references     |
| `MADV_SEQUENTIAL` | Expect sequential page references |
| `MADV_WILLNEED`   | Expect access in the near future  |
| `MADV_DONTNEED`   | Do not expect access              |

---

### `map(size, protection, flags, fd, [offset], [advise]) → Buffer`

Maps a file into memory and returns a Node.js Buffer.

| Param      | Type   | Description                          |               |
| ---------- | ------ | ------------------------------------ | ------------- |
| size       | number | Size of the mapping                  |               |
| protection | number | e.g., \`PROT\_READ                   | PROT\_WRITE\` |
| flags      | number | e.g., `MAP_SHARED`                   |               |
| fd         | number | File descriptor                      |               |
| offset     | number | Offset in file (default: 0)          |               |
| advise     | number | Usage hint (e.g., `MADV_SEQUENTIAL`) |               |

---

### `advise(buffer, [offset], [length], advise) → void`

Provides advice about memory usage to the kernel.

---

### `incore(buffer) → [number, number]`

Returns `[unmappedPages, mappedPages]` for the buffer.

---

### `sync(buffer, offset?, length?, blocking?, invalidate?) → void`

Synchronizes changes to disk.

---

## 📦 TypeScript Support

```ts
import { map, PROT_READ, PROT_WRITE, MAP_SHARED } from "mmap-io-latest";
```

Type definitions are included out of the box.

---

## ⚠️ Notes

* On **Windows**, `madvise` and `mincore` are not fully supported.
* Ensure the file descriptor is opened with the correct permissions.
* Always close the file descriptor after use.

---

## 📜 License

MIT License © 2025

[Updated by Karan Vallecha on August 2, 2025]