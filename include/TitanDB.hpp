#include <fstream>
#include <unordered_map>
#include "record.hpp"

class TitanDB{
private:
    std::fstream file; //f obj declared
    std::unordered_map<std::string,std::streampos> ind;

public:
    TitanDB(const std::string & fname);
    bool rec_read_safe(std::fstream &fo, std::streampos offset, Record &r) ;
    std::string get(const std::string &key);
    void put(const std::string &key, const std::string &val);
    void update(const std::string &key, const std::string &newval);
    void remove(const std::string &key);
};