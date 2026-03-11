#include "titandb.h"

#include <iostream>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace titandb {

// ─────────────────────────────────────────────
//  Constructor / Destructor
// ─────────────────────────────────────────────

TitanDB::TitanDB(const std::string& filepath) : filepath_(filepath) {
    // Open for read+write in binary mode; create if not exists
    file_.open(filepath_, std::ios::in | std::ios::out | std::ios::binary);
    if (!file_.is_open()) {
        // File doesn't exist yet — create it
        std::ofstream create(filepath_, std::ios::binary);
        create.close();
        file_.open(filepath_, std::ios::in | std::ios::out | std::ios::binary);
    }
    if (!file_.is_open())
        throw std::runtime_error("TitanDB: cannot open/create file: " + filepath_);

    loadIndex();
}

TitanDB::~TitanDB() {
    if (file_.is_open())
        file_.close();
}

// ─────────────────────────────────────────────
//  Private helpers
// ─────────────────────────────────────────────

void TitanDB::padString(char* buf, const std::string& s, size_t maxLen) {
    std::memset(buf, 0, maxLen);
    std::memcpy(buf, s.c_str(), std::min(s.size(), maxLen - 1));
}

bool TitanDB::writeRecord(long offset, const Record& rec) {
    file_.seekp(offset, std::ios::beg);
    if (!file_) return false;

    char alive = rec.alive ? 1 : 0;
    file_.write(&alive, 1);

    char keyBuf[KEY_SIZE]   = {};
    char valBuf[VALUE_SIZE] = {};
    padString(keyBuf, rec.key,   KEY_SIZE);
    padString(valBuf, rec.value, VALUE_SIZE);

    file_.write(keyBuf, KEY_SIZE);
    file_.write(valBuf, VALUE_SIZE);
    file_.flush();
    return file_.good();
}

Record TitanDB::readRecord(long offset) {
    file_.seekg(offset, std::ios::beg);

    char alive = 0;
    file_.read(&alive, 1);

    char keyBuf[KEY_SIZE]   = {};
    char valBuf[VALUE_SIZE] = {};
    file_.read(keyBuf, KEY_SIZE);
    file_.read(valBuf, VALUE_SIZE);

    return Record{
        alive != 0,
        std::string(keyBuf),
        std::string(valBuf)
    };
}

long TitanDB::appendRecord(const Record& rec) {
    file_.seekp(0, std::ios::end);
    long offset = static_cast<long>(file_.tellp());
    writeRecord(offset, rec);
    return offset;
}

void TitanDB::loadIndex() {
    index_.clear();
    file_.seekg(0, std::ios::end);
    long fileSize = static_cast<long>(file_.tellg());

    for (long offset = 0; offset + static_cast<long>(RECORD_SIZE) <= fileSize;
         offset += static_cast<long>(RECORD_SIZE)) {
        Record rec = readRecord(offset);
        if (rec.alive)
            index_[rec.key] = offset;
        // Deleted records are simply skipped (tombstoned)
    }
}

// ─────────────────────────────────────────────
//  Public CRUD
// ─────────────────────────────────────────────

bool TitanDB::put(const std::string& key, const std::string& value) {
    if (key.size() >= KEY_SIZE || value.size() >= VALUE_SIZE)
        return false; // overflow guard
    if (index_.count(key))
        return false; // key already exists — use update()

    Record rec{true, key, value};
    long offset = appendRecord(rec);
    index_[key] = offset;
    return true;
}

std::optional<std::string> TitanDB::get(const std::string& key) {
    auto it = index_.find(key);
    if (it == index_.end())
        return std::nullopt;

    Record rec = readRecord(it->second);
    return rec.value;
}

bool TitanDB::update(const std::string& key, const std::string& value) {
    if (value.size() >= VALUE_SIZE) return false;
    auto it = index_.find(key);
    if (it == index_.end()) return false;

    // Overwrite value in-place at the same byte offset (fixed-width FTW)
    Record rec{true, key, value};
    return writeRecord(it->second, rec);
}

bool TitanDB::remove(const std::string& key) {
    auto it = index_.find(key);
    if (it == index_.end()) return false;

    // Soft-delete: flip the alive byte to 0
    file_.seekp(it->second, std::ios::beg);
    char dead = 0;
    file_.write(&dead, 1);
    file_.flush();

    index_.erase(it);
    return true;
}

// ─────────────────────────────────────────────
//  Utility
// ─────────────────────────────────────────────

size_t TitanDB::size() const {
    return index_.size();
}

void TitanDB::printAll() const {
    std::cout << "=== TitanDB Contents (" << index_.size() << " records) ===\n";
    for (const auto& [k, offset] : index_) {
        // const_cast safe — readRecord only reads
        Record rec = const_cast<TitanDB*>(this)->readRecord(offset);
        std::cout << "  [" << rec.key << "] => " << rec.value << "\n";
    }
    std::cout << "================================================\n";
}

void TitanDB::compact() {
    // Collect all live records
    std::vector<Record> live;
    file_.seekg(0, std::ios::end);
    long fileSize = static_cast<long>(file_.tellg());

    for (long offset = 0; offset + static_cast<long>(RECORD_SIZE) <= fileSize;
         offset += static_cast<long>(RECORD_SIZE)) {
        Record rec = readRecord(offset);
        if (rec.alive) live.push_back(rec);
    }

    // Truncate and rewrite
    file_.close();
    std::ofstream trunc(filepath_, std::ios::binary | std::ios::trunc);
    trunc.close();
    file_.open(filepath_, std::ios::in | std::ios::out | std::ios::binary);

    index_.clear();
    for (const auto& rec : live) {
        long offset = appendRecord(rec);
        index_[rec.key] = offset;
    }
}

} // namespace titandb
