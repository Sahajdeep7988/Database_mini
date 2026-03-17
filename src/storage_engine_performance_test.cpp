#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include "../include/storage_engine/StorageEngine.h"
#include "../include/storage_engine/BufferPoolManager.h"
#include "../include/storage_engine/BTreeIndex.h"
#include "../include/storage_engine/TransactionManager.h"
#include "../include/storage_engine/Value.h"
#include "../include/Column.h"
#include "../include/DataType.h"

using namespace sqldb;
using namespace std::chrono;

// Helper function to generate random string
std::string generateRandomString(size_t length) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);
    
    std::string str(length, 0);
    for (size_t i = 0; i < length; ++i) {
        str[i] = charset[dist(rng)];
    }
    return str;
}

// Test class for performance testing
class PerformanceTest {
private:
    std::unique_ptr<StorageEngine> storageEngine;
    std::shared_ptr<Table> table;
    std::string testDir;
    std::ofstream resultsFile;
    
    // Timer variables
    high_resolution_clock::time_point startTime;
    high_resolution_clock::time_point endTime;
    
    void startTimer() {
        startTime = high_resolution_clock::now();
    }
    
    double endTimer() {
        endTime = high_resolution_clock::now();
        return duration_cast<milliseconds>(endTime - startTime).count() / 1000.0;
    }
    
    void logResult(const std::string& operation, int recordCount, double seconds) {
        std::cout << std::left << std::setw(20) << operation 
                  << std::setw(15) << recordCount 
                  << std::setw(10) << std::fixed << std::setprecision(3) << seconds << " seconds" 
                  << std::setw(15) << (recordCount / seconds) << " ops/sec" << std::endl;
                  
        if (resultsFile.is_open()) {
            resultsFile << operation << "," << recordCount << "," << seconds << "," << (recordCount / seconds) << std::endl;
        }
    }
    
public:
    PerformanceTest(const std::string& dir = "perf_test_data") : testDir(dir) {
        // Create directory if it doesn't exist
        std::filesystem::create_directories(testDir);
        
        // Open results file
        resultsFile.open(testDir + "/results.csv");
        if (resultsFile.is_open()) {
            resultsFile << "Operation,RecordCount,Seconds,OpsPerSecond" << std::endl;
        }
        
        // Initialize storage engine
        storageEngine = std::make_unique<StorageEngine>(1000); // 1000 pages in buffer pool
        storageEngine->initialize(testDir);
        
        std::cout << "Performance test initialized with directory: " << testDir << std::endl;
        std::cout << std::left << std::setw(20) << "Operation" 
                  << std::setw(15) << "Record Count" 
                  << std::setw(10) << "Time (s)" 
                  << std::setw(15) << "Throughput" << std::endl;
        std::cout << std::string(60, '-') << std::endl;
    }
    
    ~PerformanceTest() {
        if (resultsFile.is_open()) {
            resultsFile.close();
        }
        
        // Clean up test directory
        // std::filesystem::remove_all(testDir);
    }
    
    bool createTestTable() {
        // Define columns
        std::vector<Column> columns;
        columns.emplace_back("id", Type::INT, true, true, true);
        columns.emplace_back("name", Type::STRING, false, true, true);
        columns.emplace_back("value", Type::INT, false, false, false);
        columns.emplace_back("description", Type::STRING, false, false, false);
        
        // Create table
        table = storageEngine->createTable("performance_test", columns);
        return table != nullptr;
    }
    
    void insertRecords(int count) {
        if (!table) return;
        
        startTimer();
        
        // Begin transaction
        int txnId = storageEngine->beginTransaction();
        
        // Insert records
        for (int i = 0; i < count; ++i) {
            std::vector<std::pair<std::string, std::string>> values;
            values.push_back({"id", std::to_string(i)});
            values.push_back({"name", generateRandomString(10)});
            values.push_back({"value", std::to_string(rand() % 10000)});
            values.push_back({"description", generateRandomString(50)});
            
            // Insert the record
            table->insertRecord(values);
            
            // Commit every 1000 records to avoid transaction getting too large
            if (i > 0 && i % 1000 == 0) {
                storageEngine->commitTransaction(txnId);
                txnId = storageEngine->beginTransaction();
            }
        }
        
        // Commit final transaction
        storageEngine->commitTransaction(txnId);
        
        double seconds = endTimer();
        logResult("Insert", count, seconds);
    }
    
    void pointQueries(int count) {
        if (!table) return;
        
        startTimer();
        
        // Begin transaction
        int txnId = storageEngine->beginTransaction();
        
        // Perform random point queries
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<> dist(0, count - 1);
        
        for (int i = 0; i < count; ++i) {
            int id = dist(rng);
            
            // Build condition: id = X
            Condition condition;
            condition.column = "id";
            condition.op = Condition::Operator::EQUALS;
            condition.value = std::to_string(id);

            // Execute the query
            table->selectRecords({"id", "name", "value"}, &condition);
        }
        
        // Commit transaction
        storageEngine->commitTransaction(txnId);
        
        double seconds = endTimer();
        logResult("Point Query", count, seconds);
    }
    
    void rangeQueries(int count, int rangeSize) {
        if (!table) return;
        
        startTimer();
        
        // Begin transaction
        int txnId = storageEngine->beginTransaction();
        
        // Perform random range queries
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<> dist(0, count - rangeSize);
        
        for (int i = 0; i < count / rangeSize; ++i) {
            int startId = dist(rng);
            int endId = startId + rangeSize - 1;
            
            // Build condition: id >= startId AND id <= endId
            Condition firstCondition;
            firstCondition.column = "id";
            firstCondition.op = Condition::Operator::GREATER_EQUAL;
            firstCondition.value = std::to_string(startId);
            firstCondition.logicalOp = Condition::LogicalOperator::AND;

            firstCondition.nextCondition = std::make_shared<Condition>();
            firstCondition.nextCondition->column = "id";
            firstCondition.nextCondition->op = Condition::Operator::LESS_EQUAL;
            firstCondition.nextCondition->value = std::to_string(endId);

            // Execute the query
            table->selectRecords({"id", "name", "value"}, &firstCondition);
        }
        
        // Commit transaction
        storageEngine->commitTransaction(txnId);
        
        double seconds = endTimer();
        logResult("Range Query", count / rangeSize, seconds);
    }
    
    void updateRecords(int count) {
        if (!table) return;
        
        startTimer();
        
        // Begin transaction
        int txnId = storageEngine->beginTransaction();
        
        // Perform random updates
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<> dist(0, count - 1);
        
        for (int i = 0; i < count; ++i) {
            int id = dist(rng);
            
            // Create update values
            std::vector<std::pair<std::string, std::string>> updateValues;
            updateValues.push_back({"value", std::to_string(rand() % 10000)});
            updateValues.push_back({"description", generateRandomString(50)});

            // Build condition: id = X
            Condition condition;
            condition.column = "id";
            condition.op = Condition::Operator::EQUALS;
            condition.value = std::to_string(id);

            // Execute the update
            table->updateRecords(updateValues, &condition);
            
            // Commit every 1000 updates
            if (i > 0 && i % 1000 == 0) {
                storageEngine->commitTransaction(txnId);
                txnId = storageEngine->beginTransaction();
            }
        }
        
        // Commit final transaction
        storageEngine->commitTransaction(txnId);
        
        double seconds = endTimer();
        logResult("Update", count, seconds);
    }
    
    void deleteRecords(int count) {
        if (!table) return;
        
        startTimer();
        
        // Begin transaction
        int txnId = storageEngine->beginTransaction();
        
        // Perform random deletes
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<> dist(0, count - 1);
        
        for (int i = 0; i < count / 10; ++i) { // Delete 10% of records
            int id = dist(rng);
            
            // Build condition: id = X
            Condition condition;
            condition.column = "id";
            condition.op = Condition::Operator::EQUALS;
            condition.value = std::to_string(id);

            // Execute the delete
            table->deleteRecords(&condition);
            
            // Commit every 100 deletes
            if (i > 0 && i % 100 == 0) {
                storageEngine->commitTransaction(txnId);
                txnId = storageEngine->beginTransaction();
            }
        }
        
        // Commit final transaction
        storageEngine->commitTransaction(txnId);
        
        double seconds = endTimer();
        logResult("Delete", count / 10, seconds);
    }
    
    void runFullTest(int recordCount) {
        // Create test table
        if (!createTestTable()) {
            std::cerr << "Failed to create test table!" << std::endl;
            return;
        }
        
        // Run tests
        insertRecords(recordCount);
        pointQueries(recordCount);
        rangeQueries(recordCount, 100); // Range size of 100
        updateRecords(recordCount);
        deleteRecords(recordCount);
        
        // Flush buffer pool
        storageEngine->flushBufferPool();
    }
};

int main(int argc, char* argv[]) {
    // Default record count
    int recordCount = 10000;
    
    // Parse command line arguments
    if (argc > 1) {
        recordCount = std::stoi(argv[1]);
    }
    
    std::cout << "Running storage engine performance test with " << recordCount << " records..." << std::endl;
    
    // Run performance test
    PerformanceTest test;
    test.runFullTest(recordCount);
    
    std::cout << "Performance test completed. Results saved to perf_test_data/results.csv" << std::endl;
    return 0;
} 