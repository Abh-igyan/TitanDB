# 🚀 TitanDB – Disk-Backed Key-Value Store (C++17)

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square&logo=c%2B%2B)
![License](https://img.shields.io/badge/license-MIT-green?style=flat-square)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey?style=flat-square)
![Status](https://img.shields.io/badge/status-active-success?style=flat-square)

A lightweight, log-structured key-value store built from scratch in C++17, inspired by Bitcask-style storage engines. TitanDB demonstrates core systems concepts like persistent storage, crash recovery, append-only design, and in-memory indexing.

---

## 📌 Features

- ⚡ **Append-Only Log Storage**
  - All writes (`put`, `update`, `delete`) are appended to disk
  - Ensures sequential I/O for better performance

- 🧠 **In-Memory Hash Index**
  - Maps keys → file offsets
  - Enables O(1) lookups without loading entire DB into memory

- 💾 **Persistent Storage**
  - Data survives across program restarts
  - Automatic index reconstruction via file scan on startup

- 🪦 **Soft Deletes (Tombstones)**
  - Deletes are handled via tombstone records
  - Avoids expensive in-place modifications

- 🛡️ **Crash Resilience (Basic)**
  - Safe record reading using `rec_read_safe`
  - Prevents corruption from partial writes

- 🔁 **Update via Append**
  - Updates create new versions instead of overwriting
  - Older versions are cleaned during compaction (planned)

---

## 🧱 Architecture Overview

```
            +----------------------+
            |   In-Memory Index    |
            |  key → file offset   |
            +----------+-----------+
                       |
                       ↓
            +----------------------+
            |   Append-Only Log    |
            |  [Record][Record]... |
            +----------------------+
```

### Record Format

```cpp
struct Record {
    char key[32];
    char val[64];
    bool alive; // tombstone flag
};
```

- Fixed-size binary serialization
- Enables direct offset-based access (`seekg`, `seekp`)

---

## ⚙️ Core Operations

### 🟢 Put
- Appends new record to file
- Updates index with latest offset

### 🔵 Get
- Looks up offset from index
- Reads record from disk
- Returns value if `alive == true`

### 🟡 Update
- Internally calls `put()`
- Follows append-only semantics

### 🔴 Delete
- Appends tombstone record (`alive = false`)
- Removes key from index

---

## 🔄 Startup Recovery

On initialization:
- File is scanned sequentially
- Index is rebuilt from records
- Latest state of each key is restored

```cpp
for (pos = 0; pos < file_end; pos += sizeof(Record)) {
    if (!rec_read_safe(...)) break;

    if (r.alive)
        ind[r.key] = pos;
    else
        ind.erase(r.key);
}
```

---

## 🛠️ Build & Run

### Compile

```bash
g++ -std=c++17 main.cpp TitanDB.cpp record.cpp -o titan
```

### Run

```bash
./titan
```

---

## 🧪 Example Usage

```cpp
TitanDB db("titan.db");

db.put("username", "abhigyan");
db.put("college", "NIT Silchar");

db.update("college", "IIT Bombay");

auto val = db.get("username");
if (val) std::cout << *val << std::endl;  // abhigyan

db.remove("username");

auto gone = db.get("username");
if (!gone) std::cout << "Key not found\n";  // Key not found
```

---

## ⚠️ Current Limitations

- Fixed-size keys/values
- No concurrency control (single-threaded)
- No checksum validation (corruption detection is basic)
- No compaction (log grows indefinitely)

---

## 🚀 Roadmap / Future Improvements

- 🔁 **Log Compaction** — Remove stale records and reclaim disk space
- 🔐 **Thread Safety** — Add mutex/shared_mutex for concurrent access
- 🧾 **Write-Ahead Logging (WAL)** — Improve crash guarantees
- 🧠 **Bloom Filters** — Faster negative lookups
- 📦 **Variable-Length Records** — More flexible storage
- 🛡️ **Checksums** — Detect corrupted/partial writes reliably

---

## 📚 Key Concepts Demonstrated

- File I/O (`seekg`, `seekp`, binary serialization)
- Log-structured storage design
- In-memory indexing
- Crash recovery techniques
- Tombstone-based deletion
- Trade-offs between in-place vs append-only updates

---

## 🧪 Test Suite

29 automated tests across 8 categories:

| Category | What it covers |
|---|---|
| Basic CRUD | put, get, update, delete, duplicates |
| Persistence | index rebuild after restart |
| Compaction | live records preserved, dead dropped |
| Stale Cleanup | latest version kept after multiple updates |
| Compaction Persistence | compacted file survives restart |
| Corruption Handling | partial writes don't corrupt valid data |
| Edge Cases | spaces in values, bulk insert/delete |
| Size Limits | keys/values exceeding fixed-width bounds |

### Run tests

```bash
g++ -std=c++17 tests/test_titandb.cpp src/TitanDB.cpp src/record.cpp -Iinclude -o run_tests
./run_tests
```

Expected output:
```
Results: 28 passed, 1 failed
Success Rate: 96.5517%
Execution Time: X ms
```

---

## 👤 Author

**Abhigyan Tiwari**
- GitHub: [@Abh-igyan](https://github.com/Abh-igyan)
- B.Tech CSE, NIT Silchar (2024–2028)

---

## 📄 License

This project is licensed under the MIT License.
