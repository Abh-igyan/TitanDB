#include <iostream>
#include <cassert>
#include <cstdio>
#include <optional>
#include <chrono>
#include "TitanDB.hpp"

int passed = 0, failed = 0;

#define TEST(name, expr) do { \
    if (expr) { std::cout << "  [PASS] " << name << "\n"; ++passed; } \
    else       { std::cout << "  [FAIL] " << name << "\n"; ++failed; } \
} while(0)

void cleanup(const std::string& f) { std::remove(f.c_str()); }

// ------------------- BASIC CRUD -------------------
void testBasicCRUD() {
    std::cout << "\n--- Basic CRUD ---\n";
    const std::string file = "test_basic.db";
    cleanup(file);
    TitanDB db(file);

    TEST("put new key", db.put("name", "Abhigyan"));
    TEST("put duplicate fails", !db.put("name", "Other"));

    auto v1 = db.get("name");
    TEST("get existing", v1 && *v1 == "Abhigyan");

    TEST("get missing", !db.get("ghost").has_value());

    TEST("update existing", db.update("name", "Abhigyan Tiwari"));
    auto v2 = db.get("name");
    TEST("get after update", v2 && *v2 == "Abhigyan Tiwari");

    TEST("update missing fails", !db.update("ghost", "x"));

    TEST("delete existing", db.remove("name"));
    TEST("get after delete", !db.get("name").has_value());

    TEST("delete missing fails", !db.remove("name"));

    cleanup(file);
}

// ------------------- PERSISTENCE -------------------
void testPersistence() {
    std::cout << "\n--- Persistence ---\n";
    const std::string file = "test_persist.db";
    cleanup(file);

    {
        TitanDB db(file);
        db.put("city", "Silchar");
        db.put("college", "NIT Silchar");
    }

    {
        TitanDB db2(file);
        auto v1 = db2.get("city");
        auto v2 = db2.get("college");

        TEST("city persisted", v1 && *v1 == "Silchar");
        TEST("college persisted", v2 && *v2 == "NIT Silchar");
        TEST("size after reload", db2.size() == 2);
    }

    cleanup(file);
}

// ------------------- COMPACTION -------------------
void testCompaction() {
    std::cout << "\n--- Compaction ---\n";
    const std::string file = "test_compact.db";
    cleanup(file);
    TitanDB db(file);

    db.put("a", "1");
    db.put("b", "2");
    db.put("c", "3");
    db.remove("b");

    TEST("size before compact", db.size() == 2);

    db.compact();

    TEST("size after compact", db.size() == 2);
    TEST("a still exists", db.get("a").value() == "1");
    TEST("c still exists", db.get("c").value() == "3");
    TEST("b removed", !db.get("b").has_value());

    cleanup(file);
}

// ------------------- STALE CLEANUP -------------------
void testStaleCleanup() {
    std::cout << "\n--- Stale Record Cleanup ---\n";
    const std::string file = "test_stale.db";
    cleanup(file);
    TitanDB db(file);

    db.put("x", "1");
    db.update("x", "2");
    db.update("x", "3");

    db.compact();

    auto v = db.get("x");
    TEST("latest value kept", v && *v == "3");
    TEST("only one record remains", db.size() == 1);

    cleanup(file);
}

// ------------------- COMPACTION PERSISTENCE -------------------
void testCompactionPersistence() {
    std::cout << "\n--- Compaction Persistence ---\n";
    const std::string file = "test_compact_persist.db";
    cleanup(file);

    {
        TitanDB db(file);
        db.put("a", "1");
        db.update("a", "2");
        db.compact();
    }

    {
        TitanDB db2(file);
        auto v = db2.get("a");
        TEST("value after restart", v && *v == "2");
        TEST("size after restart", db2.size() == 1);
    }

    cleanup(file);
}

// ------------------- CORRUPTION HANDLING -------------------
void testCorruptionHandling() {
    std::cout << "\n--- Corruption Handling ---\n";
    const std::string file = "test_corrupt.db";
    cleanup(file);

    {
        TitanDB db(file);
        db.put("a", "1");
    }

    // simulate partial write
    FILE* f = fopen(file.c_str(), "ab");
    fwrite("garbage", 1, 7, f);
    fclose(f);

    TitanDB db2(file);
    auto v = db2.get("a");

    TEST("valid data survives corruption", v && *v == "1");

    cleanup(file);
}

// ------------------- EDGE CASES -------------------
void testEdgeCases() {
    std::cout << "\n--- Edge Cases ---\n";
    const std::string file = "test_edge.db";
    cleanup(file);
    TitanDB db(file);

    TEST("value with spaces", db.put("quote", "hello world from TitanDB"));
    TEST("get spaced value", db.get("quote").value() == "hello world from TitanDB");

    for (int i = 0; i < 10; ++i)
        db.put("key" + std::to_string(i), "val" + std::to_string(i));

    TEST("bulk insert size", db.size() == 11);

    for (int i = 0; i < 5; ++i)
        db.remove("key" + std::to_string(i));

    TEST("size after bulk delete", db.size() == 6);

    cleanup(file);
}

// ------------------- SIZE LIMITS -------------------
void testSizeLimits() {
    std::cout << "\n--- Size Limits ---\n";
    const std::string file = "test_limits.db";
    cleanup(file);
    TitanDB db(file);

    std::string big_key(40, 'k');
    std::string big_val(100, 'v');

    TEST("reject large key", !db.put(big_key, "x"));
    TEST("reject large value", !db.put("k", big_val));

    cleanup(file);
}

// ------------------- MAIN -------------------
int main() {

    std::cout << "|------------------------------|\n"
              << "|       TitanDB v1.0           |\n"
              << "|        Test Suite            |\n"
              << "|------------------------------|\n";

    auto start = std::chrono::high_resolution_clock::now();

    testBasicCRUD();
    testPersistence();
    testCompaction();
    testStaleCleanup();
    testCompactionPersistence();
    testCorruptionHandling();
    testEdgeCases();
    testSizeLimits();

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "\n--------------------------------\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed\n";
    std::cout << "Success Rate: "
              << (passed * 100.0 / (passed + failed)) << "%\n";

    std::cout << "Execution Time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms\n";

    std::cout << "---------------------------------\n";

    return failed == 0 ? 0 : 1;
}
