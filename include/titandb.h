#pragma once

#include <string>
#include <unordered_map>
#include <fstream>
#include <cstdint>
#include <optional>

namespace titandb {

// Fixed-width record layout on disk:
// [1 byte: alive flag][128 bytes: key][256 bytes: value] = 385 bytes per record
constexpr size_t KEY_SIZE   = 128;
constexpr size_t VALUE_SIZE = 256;
constexpr size_t RECORD_SIZE = 1 + KEY_SIZE + VALUE_SIZE; // 385 bytes

struct Record {
    bool        alive;
    std::string key;
    std::string value;
};

class TitanDB {
public:
    explicit TitanDB(const std::string& filepath);
    ~TitanDB();

    // CRUD
    bool        put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool        update(const std::string& key, const std::string& value);
    bool        remove(const std::string& key);

    // Utility
    void        compact();           // reclaim space from deleted records
    size_t      size() const;        // number of live records
    void        printAll() const;    // debug dump

private:
    std::string                           filepath_;
    std::fstream                          file_;
    std::unordered_map<std::string, long> index_; // key -> byte offset in file

    void     loadIndex();
    bool     writeRecord(long offset, const Record& rec);
    Record   readRecord(long offset);
    long     appendRecord(const Record& rec);
    void     padString(char* buf, const std::string& s, size_t maxLen);
};

} // namespace titandb
