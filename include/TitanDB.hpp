#pragma once
#include <fstream>
#include <unordered_map>
#include <string>
#include <optional>
#include "record.hpp"

class TitanDB {
private:
    std::fstream file;
    std::unordered_map<std::string, std::streampos> ind;
    bool rec_read_safe(std::fstream& fo, std::streampos offset, Record& r);

public:
    TitanDB(const std::string& fname);
    bool put(const std::string& key, const std::string& val);
    std::optional<std::string> get(const std::string& key);
    bool update(const std::string& key, const std::string& val);
    bool remove(const std::string& key);
    int size();
    void printAll();
    void compact();
};
