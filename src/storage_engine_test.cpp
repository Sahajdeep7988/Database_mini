#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cassert>
#include <filesystem>
#include <cstring>
#include "../include/storage_engine/StorageEngine.h"
#include "../include/storage_engine/BufferPoolManager.h"
#include "../include/storage_engine/BTreeIndex.h"
#include "../include/storage_engine/TransactionManager.h"
#include "../include/storage_engine/Value.h"
#include "../include/Column.h"
#include "../include/DataType.h"

using namespace sqldb;

// Helper function to print test results
void printTestResult(const std::string& testName, bool success) {
    std::cout << testName << ": " << (success ? "PASSED" : "FAILED") << std::endl;
}

// Test Value class
bool testValue() {
    // Test int value
    Value intValue = Value::createInt(42);
    assert(intValue.type == Type::INT);
    assert(intValue.intValue == 42);
    assert(intValue.toString() == "42");
    
    // Test string value
    Value stringValue = Value::createString("hello");
    assert(stringValue.type == Type::STRING);
    assert(stringValue.stringValue == "hello");
    assert(stringValue.toString() == "hello");
    
    // Test fromString
    Value parsedInt = Value::fromString("123", Type::INT);
    assert(parsedInt.type == Type::INT);
    assert(parsedInt.intValue == 123);
    
    return true;
}

// Test BufferPoolManager
bool testBufferPoolManager() {
    // Create a temporary directory for testing
    std::string testDir = "test_buffer_pool";
    std::filesystem::create_directory(testDir);
    
    // Create a buffer pool with 10 pages
    BufferPoolManager bufferPool(10, 4096);
    bufferPool.setDbPath(testDir);
    
    // Allocate a new page
    std::shared_ptr<Page> page1 = bufferPool.allocatePage(1);
    assert(page1 != nullptr);
    assert(page1->getPageId() == 1);
    
    // Write data to the page
    char* data = page1->getData();
    std::string testData = "Hello, world!";
    std::copy(testData.begin(), testData.end(), data);
    page1->setDirty(true);
    
    // Unpin the page
    bool unpinResult = bufferPool.unpinPage(1, true);
    assert(unpinResult);
    
    // Fetch the page again
    std::shared_ptr<Page> page1Again = bufferPool.fetchPage(1);
    assert(page1Again != nullptr);
    assert(page1Again->getPageId() == 1);
    
    // Check the data
    std::string fetchedData(page1Again->getData(), testData.size());
    assert(fetchedData == testData);
    
    // Clean up
    bufferPool.flushAllPages();
    std::filesystem::remove_all(testDir);
    
    return true;
}

// Test StorageEngine
bool testStorageEngine() {
    // Create a temporary directory for testing
    std::string testDir = "test_storage_engine";
    std::filesystem::create_directory(testDir);
    
    // Create a storage engine
    StorageEngine engine(10);
    bool initResult = engine.initialize(testDir);
    assert(initResult);
    
    // Create a table
    std::vector<Column> columns;
    columns.emplace_back("id", Type::INT, true, true, true);
    columns.emplace_back("name", Type::STRING, false, true, true);
    
    std::shared_ptr<Table> table = engine.createTable("test_table", columns);
    assert(table != nullptr);
    
    // Load the table
    std::shared_ptr<Table> loadedTable = engine.loadTable("test_table");
    assert(loadedTable != nullptr);
    
    // Delete the table
    bool deleteResult = engine.deleteTable("test_table");
    assert(deleteResult);
    
    // Clean up
    std::filesystem::remove_all(testDir);
    
    return true;
}

// Test BTreeIndex
bool testBTreeIndex() {
    // Create a B-Tree index
    BTreeIndex<int> index("id", Type::INT, 5);
    
    // Insert a few keys
    index.insert(1, 100);
    index.insert(2, 200);
    index.insert(3, 300);
    
    // Search for keys
    assert(index.search(1) == 100);
    assert(index.search(2) == 200);
    assert(index.search(3) == 300);
    
    return true;
}

int main() {
    std::cout << "Running storage engine tests..." << std::endl;
    
    // Run tests
    printTestResult("Value Test", testValue());
    printTestResult("BufferPoolManager Test", testBufferPoolManager());
    printTestResult("StorageEngine Test", testStorageEngine());
    printTestResult("BTreeIndex Test", testBTreeIndex());
    
    std::cout << "All tests completed." << std::endl;
    return 0;
} 