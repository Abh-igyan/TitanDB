# TitanDB

A lightweight, **disk-backed key-value store** written in C++17 — built from scratch with no external dependencies.

Designed for learning systems programming concepts: fixed-width serialization, byte-offset indexing, file I/O with `seekg`/`seekp`, soft deletes, and log compaction.

---

## Architecture

```
┌─────────────────────────────────────────────┐
│                  TitanDB                    │
│                                             │
│  put/get/update/delete                      │
│         │                                   │
│         ▼                                   │
│  In-Memory Hash Index                       │
│  unordered_map<key, byte_offset>            │
│         │                                   │
│         │  O(1) seekg/seekp                 │
│         ▼                                   │
│  ┌──────────────────────────────────────┐   │
│  │         titan.db (binary file)       │   │
│  │  [flag|key(128B)|value(256B)] record │   │
│  │  [flag|key(128B)|value(256B)] record │   │
│  │  ...                                 │   │
│  └──────────────────────────────────────┘   │
└─────────────────────────────────────────────┘
```

### Key Design Decisions

| Decision | Why |
|---|---|
| **Fixed-width records (385 bytes)** | Enables O(1) in-place updates via `seekp` — no rewriting the whole file |
| **In-memory hash index** | Maps keys → byte offsets; achieves O(1) reads after startup |
| **Soft deletes (tombstone flag)** | Flip 1 byte instead of shifting the entire file |
| **Log compaction** | Reclaims disk space by rewriting only live records |
| **No external libraries** | Pure C++ stdlib: `<fstream>`, `<unordered_map>`, `<optional>` |

---

## Record Format (on disk)

```
Offset 0:   [1  byte ] alive flag (1 = live, 0 = deleted)
Offset 1:   [128 bytes] key   (null-padded)
Offset 129: [256 bytes] value (null-padded)
            ─────────────────
Total:       385 bytes per record
```

---

## Operations & Complexity

| Operation | Time | Notes |
|---|---|---|
| `put(key, val)` | O(1) amortized | Appends to end of file |
| `get(key)` | O(1) | Hash lookup → single `seekg` |
| `update(key, val)` | O(1) | In-place overwrite via `seekp` |
| `remove(key)` | O(1) | Flip alive byte to 0 (tombstone) |
| `compact()` | O(n) | Rewrites all live records |
| Startup (index load) | O(n) | Linear scan once to build hash index |

---

## Build & Run

### Requirements
- g++ with C++17 support
- `make`

### Build
```bash
git clone https://github.com/YOUR_USERNAME/TitanDB.git
cd TitanDB
make
```

### Run tests
```bash
make test
```

### Interactive CLI
```bash
make run
# or
./titandb mydata.db
```

---

## CLI Usage

```
titandb> put username abhigyan
OK - inserted 'username'

titandb> get username
username => abhigyan

titandb> update username abhigyan_tiwari
OK - updated 'username'

titandb> delete username
OK - deleted 'username'

titandb> list
=== TitanDB Contents (0 records) ===

titandb> compact
OK - compacted. Live records: 0

titandb> exit
Goodbye.
```

---

## Project Structure

```
TitanDB/
├── include/
│   └── titandb.h          # Public API + constants
├── src/
│   ├── titandb.cpp        # Core implementation
│   └── main.cpp           # Interactive CLI
├── tests/
│   └── test_titandb.cpp   # 22 unit tests
└── Makefile
```

---

## What I Learned / Concepts Demonstrated

- **Binary file I/O** in C++ (`fstream`, `seekg`, `seekp`, `read`, `write`)
- **Fixed-width serialization** for constant-time random access
- **Hash indexing** for O(1) key lookup without loading entire DB into RAM
- **Soft deletes** vs hard deletes — trade-offs in disk usage vs write speed
- **Log compaction** — a core concept in real-world KV stores (LevelDB, RocksDB)
- Memory vs disk trade-offs in systems design

---

## Inspired By

Real-world systems like [Bitcask](https://riak.com/assets/bitcask-intro.pdf) (the storage engine behind Riak), which uses the same append-only log + in-memory index architecture.

---

## License

MIT
