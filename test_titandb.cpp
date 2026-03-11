#include <iostream>
#include <cassert>
#include <cstdio>
#include "titandb.h"

// Simple test harness
int passed = 0, failed = 0;

#define TEST(name, expr) do { \
    if (expr) { std::cout << "  [PASS] " << name << "\n"; ++passed; } \
    else       { std::cout << "  [FAIL] " << name << "\n"; ++failed; } \
} while(0)

void cleanup(const std::string& f) { std::remove(f.c_str()); }

void testBasicCRUD() {
    std::cout << "\n--- Basic CRUD ---\n";
    const std::string file = "test_basic.db";
    cleanup(file);
    titandb::TitanDB db(file);

    TEST("put new key",          db.put("name", "Abhigyan"));
    TEST("put duplicate fails",  !db.put("name", "Other"));
    TEST("get existing",         db.get("name") == "Abhigyan");
    TEST("get missing",          !db.get("ghost").has_value());
    TEST("update existing",      db.update("name", "Abhigyan Tiwari"));
    TEST("get after update",     db.get("name") == "Abhigyan Tiwari");
    TEST("update missing fails", !db.update("ghost", "x"));
    TEST("delete existing",      db.remove("name"));
    TEST("get after delete",     !db.get("name").has_value());
    TEST("delete missing fails", !db.remove("name"));

    cleanup(file);
}

void testPersistence() {
    std::cout << "\n--- Persistence (data survives restart) ---\n";
    const std::string file = "test_persist.db";
    cleanup(file);

    {
        titandb::TitanDB db(file);
        db.put("city",  "Silchar");
        db.put("college", "NIT Silchar");
    } // destructor closes file

    {
        titandb::TitanDB db2(file);
        TEST("city persisted",    db2.get("city")    == "Silchar");
        TEST("college persisted", db2.get("college") == "NIT Silchar");
        TEST("size after reload", db2.size() == 2);
    }

    cleanup(file);
}

void testCompaction() {
    std::cout << "\n--- Compaction ---\n";
    const std::string file = "test_compact.db";
    cleanup(file);
    titandb::TitanDB db(file);

    db.put("a", "1");
    db.put("b", "2");
    db.put("c", "3");
    db.remove("b");
    TEST("size before compact", db.size() == 2);
    db.compact();
    TEST("size after compact",  db.size() == 2);
    TEST("a still accessible",  db.get("a") == "1");
    TEST("c still accessible",  db.get("c") == "3");
    TEST("b gone after compact",!db.get("b").has_value());

    cleanup(file);
}

void testEdgeCases() {
    std::cout << "\n--- Edge Cases ---\n";
    const std::string file = "test_edge.db";
    cleanup(file);
    titandb::TitanDB db(file);

    // Value with spaces
    TEST("value with spaces", db.put("quote", "hello world from titandb"));
    TEST("get spaced value",  db.get("quote") == "hello world from titandb");

    // Multiple puts and deletes
    for (int i = 0; i < 10; ++i)
        db.put("key" + std::to_string(i), "val" + std::to_string(i));
    TEST("bulk insert size", db.size() == 11);
    for (int i = 0; i < 5; ++i)
        db.remove("key" + std::to_string(i));
    TEST("size after bulk delete", db.size() == 6);

    cleanup(file);
}

int main() {
    std::cout << "╔══════════════════════════════╗\n"
              << "║    TitanDB Test Suite        ║\n"
              << "╚══════════════════════════════╝\n";

    testBasicCRUD();
    testPersistence();
    testCompaction();
    testEdgeCases();

    std::cout << "\n══════════════════════════════\n"
              << "Results: " << passed << " passed, " << failed << " failed\n"
              << "══════════════════════════════\n";

    return failed == 0 ? 0 : 1;
}
