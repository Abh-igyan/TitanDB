#include <iostream>
#include <optional>
#include <cstring>
#include "TitanDB.hpp"

TitanDB::TitanDB(const std::string &fname)
: file(fname, std::ios::binary | std::ios::in | std::ios::out){
    if(!file.is_open()){
        file.clear();
        file.open(fname, std::ios::binary | std::ios::out);
        file.close();
        file.open(fname, std::ios::binary | std::ios::in | std::ios::out);
    }
    file.seekg(0, std::ios::end);
    std::streampos end = file.tellg();
    for(std::streampos pos = 0; pos < end; pos += sizeof(Record)){
        Record r;
        if(!rec_read_safe(file, pos, r)){
            std::cout << "Corrupt record at offset " << pos << "\n";
            break;
        }
        if(r.alive) ind[r.key] = pos;
        else ind.erase(r.key);
    }
}

bool TitanDB::rec_read_safe(std::fstream &fo, std::streampos offset, Record &r){
    fo.seekg(offset);
    fo.read(reinterpret_cast<char*>(&r), sizeof(r));
    return fo.gcount() == sizeof(r);
}

std::optional<std::string> TitanDB::get(const std::string &key){
    if(ind.find(key) == ind.end()) return std::nullopt;
    Record r;
    if(!rec_read_safe(file, ind[key], r)) return std::nullopt;
    if(!r.alive) return std::nullopt;
    return std::string(r.val);
}

bool TitanDB::put(const std::string &key, const std::string &val){
    if(ind.find(key) != ind.end()) return false;  // key exists, use update
    if(key.size() >= key_sz || val.size() >= val_sz) return false;
    
    Record r;
    strncpy(r.key, key.c_str(), key_sz - 1);
    r.key[key_sz - 1] = '\0';
    strncpy(r.val, val.c_str(), val_sz - 1);
    r.val[val_sz - 1] = '\0';
    r.alive = true;
    
    file.seekp(0, std::ios::end);
    std::streampos ofs = file.tellp();
    rec_write(file, r);
    file.flush();
    ind[key] = ofs;
    return true;
}

bool TitanDB::update(const std::string &key, const std::string &val){
    if(ind.find(key) == ind.end()) return false;
    if(val.size() >= val_sz) return false;
    
    Record r;
    strncpy(r.key, key.c_str(), key_sz - 1);
    r.key[key_sz - 1] = '\0';
    strncpy(r.val, val.c_str(), val_sz - 1);
    r.val[val_sz - 1] = '\0';
    r.alive = true;
    
    file.seekp(0, std::ios::end);
    std::streampos ofs = file.tellp();
    rec_write(file, r);
    file.flush();
    ind[key] = ofs;  // update index to new offset
    return true;
}

bool TitanDB::remove(const std::string &key){
    if(ind.find(key) == ind.end()) return false;
    
    Record r;
    strncpy(r.key, key.c_str(), key_sz - 1);
    r.key[key_sz - 1] = '\0';
    r.val[0] = '\0';
    r.alive = false;
    
    file.seekp(0, std::ios::end);
    rec_write(file, r);
    file.flush();
    ind.erase(key);
    return true;
}

int TitanDB::size(){
    return ind.size();
}

void TitanDB::printAll(){
    if(ind.empty()){
        std::cout << "=== TitanDB is empty ===\n";
        return;
    }
    std::cout << "=== TitanDB Contents (" << ind.size() << " records) ===\n";
    for(auto &[key, offset] : ind){
        Record r;
        if(rec_read_safe(file, offset, r) && r.alive)
            std::cout << "  " << r.key << " => " << r.val << "\n";
    }
}
