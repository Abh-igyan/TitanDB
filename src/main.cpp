#include <iostream>
#include <string>
#include <sstream>
#include "titandb.h"

void printHelp() {
    std::cout << "\nTitanDB CLI Commands:\n"
              << "  put <key> <value>    - Insert a new key-value pair\n"
              << "  get <key>            - Retrieve value by key\n"
              << "  update <key> <value> - Update existing key\n"
              << "  delete <key>         - Delete a key\n"
              << "  list                 - Show all records\n"
              << "  size                 - Show record count\n"
              << "  compact              - Reclaim space from deleted records\n"
              << "  help                 - Show this menu\n"
              << "  exit                 - Quit\n\n";
}

int main(int argc, char* argv[]) {
    std::string dbFile = (argc > 1) ? argv[1] : "titan.db";

    std::cout << "╔══════════════════════════════╗\n"
              << "║       TitanDB v1.0           ║\n"
              << "║  High-Performance KV Store   ║\n"
              << "╚══════════════════════════════╝\n";
    std::cout << "Database file: " << dbFile << "\n";

    titandb::TitanDB db(dbFile);
    std::cout << "Loaded " << db.size() << " existing records.\n";
    printHelp();

    std::string line;
    while (true) {
        std::cout << "titandb> ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "exit" || cmd == "quit") {
            std::cout << "Goodbye.\n";
            break;

        } else if (cmd == "put") {
            std::string key, value;
            iss >> key;
            std::getline(iss >> std::ws, value);
            if (key.empty() || value.empty()) {
                std::cout << "Usage: put <key> <value>\n"; continue;
            }
            if (db.put(key, value))
                std::cout << "OK - inserted '" << key << "'\n";
            else
                std::cout << "ERROR - key already exists or too long. Use 'update'.\n";

        } else if (cmd == "get") {
            std::string key; iss >> key;
            auto val = db.get(key);
            if (val) std::cout << key << " => " << *val << "\n";
            else     std::cout << "NOT FOUND: '" << key << "'\n";

        } else if (cmd == "update") {
            std::string key, value;
            iss >> key;
            std::getline(iss >> std::ws, value);
            if (db.update(key, value))
                std::cout << "OK - updated '" << key << "'\n";
            else
                std::cout << "ERROR - key not found or value too long.\n";

        } else if (cmd == "delete") {
            std::string key; iss >> key;
            if (db.remove(key))
                std::cout << "OK - deleted '" << key << "'\n";
            else
                std::cout << "NOT FOUND: '" << key << "'\n";

        } else if (cmd == "list") {
            db.printAll();

        } else if (cmd == "size") {
            std::cout << "Records: " << db.size() << "\n";

        } else if (cmd == "compact") {
            db.compact();
            std::cout << "OK - compacted. Live records: " << db.size() << "\n";

        } else if (cmd == "help") {
            printHelp();

        } else {
            std::cout << "Unknown command: '" << cmd << "'. Type 'help'.\n";
        }
    }
    return 0;
}
