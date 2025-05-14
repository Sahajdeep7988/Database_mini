#ifndef STORAGE_ENGINE_H
#define STORAGE_ENGINE_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "../Table.h"

namespace sqldb {

/**
 * @class StorageEngine
 * @brief Main class for the database storage engine
 * 
 * This class manages the physical storage of data, including:
 * - B-Tree indexes for efficient data retrieval
 * - Buffer pool management for caching
 * - Transaction management
 * - Disk I/O operations
 */
class StorageEngine {
public:
    /**
     * @brief Constructor for StorageEngine
     * @param bufferPoolSize Size of the buffer pool in pages
     */
    StorageEngine(size_t bufferPoolSize = 1000);
    
    /**
     * @brief Destructor for StorageEngine
     */
    ~StorageEngine();
    
    /**
     * @brief Initialize the storage engine
     * @param dbPath Path to the database files
     * @return true if initialization was successful, false otherwise
     */
    bool initialize(const std::string& dbPath);
    
    /**
     * @brief Load a table into the storage engine
     * @param tableName Name of the table to load
     * @return Pointer to the loaded table, or nullptr if loading failed
     */
    std::shared_ptr<Table> loadTable(const std::string& tableName);
    
    /**
     * @brief Create a new table in the storage engine
     * @param tableName Name of the table to create
     * @param columns Column definitions for the new table
     * @return Pointer to the created table, or nullptr if creation failed
     */
    std::shared_ptr<Table> createTable(const std::string& tableName, const std::vector<Column>& columns);
    
    /**
     * @brief Delete a table from the storage engine
     * @param tableName Name of the table to delete
     * @return true if deletion was successful, false otherwise
     */
    bool deleteTable(const std::string& tableName);
    
    /**
     * @brief Begin a new transaction
     * @return Transaction ID
     */
    int beginTransaction();
    
    /**
     * @brief Commit a transaction
     * @param transactionId ID of the transaction to commit
     * @return true if commit was successful, false otherwise
     */
    bool commitTransaction(int transactionId);
    
    /**
     * @brief Rollback a transaction
     * @param transactionId ID of the transaction to rollback
     * @return true if rollback was successful, false otherwise
     */
    bool rollbackTransaction(int transactionId);
    
    /**
     * @brief Flush all dirty pages to disk
     * @return true if flush was successful, false otherwise
     */
    bool flushBufferPool();

private:
    // Buffer pool size in pages
    size_t bufferPoolSize_;
    
    // Path to the database files
    std::string dbPath_;
    
    // Map of table names to table objects
    std::unordered_map<std::string, std::shared_ptr<Table>> tables_;
    
    // Current transaction ID
    int currentTransactionId_;
};

} // namespace sqldb

#endif // STORAGE_ENGINE_H 