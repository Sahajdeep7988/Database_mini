#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "Table.h"
#include "storage_engine/StorageEngine.h"
#include <unordered_map>
#include <memory>
#include <dirent.h>
#include <set>
#include <string.h>

#ifdef _WIN32
    #include <direct.h>
#else
    #include <sys/types.h>
    #include <sys/stat.h>
#endif

class DatabaseManager {
private:
    std::unordered_map<std::string, std::unique_ptr<Table>> tables;
    std::string dataDirectory;
    std::unique_ptr<sqldb::StorageEngine> storageEngine;
    
    // Load existing tables from data directory
    void loadExistingTables() {
        // Try to open the data directory
        DIR* dir = opendir(dataDirectory.c_str());
        if (!dir) {
            // Create data directory if it doesn't exist
            #ifdef _WIN32
                mkdir(dataDirectory.c_str());
            #else
                mkdir(dataDirectory.c_str(), 0777);
            #endif
            return;
        }
        
        // Set to keep track of table names
        std::set<std::string> tableNames;
        
        // Read directory entries
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            std::string filename = entry->d_name;
            
            // Skip . and ..
            if (filename == "." || filename == "..") {
                continue;
            }
            
            // Only look for .table extension (storage engine format)
            size_t pos = filename.find(".table");
            if (pos != std::string::npos) {
                // Extract table name
                std::string tableName = filename.substr(0, pos);
                tableNames.insert(tableName);
            }
        }
        
        closedir(dir);
        
        // Load tables from storage engine
        for (const auto& tableName : tableNames) {
            if (storageEngine) {
                auto loadedTable = storageEngine->loadTable(tableName);
                if (loadedTable) {
                    // Convert shared_ptr to unique_ptr
                    tables[tableName] = std::make_unique<Table>(*loadedTable);
                }
            }
        }
    }
    
public:
    DatabaseManager() : dataDirectory("data"), storageEngine(std::make_unique<sqldb::StorageEngine>(1000)) {
        // Initialize storage engine with default buffer pool size (1000 pages)
        storageEngine->initialize(dataDirectory);
        loadExistingTables();
    }
    
    // Set the data directory path
    void setDataDirectory(const std::string& directory) {
        dataDirectory = directory;
        // Clear existing tables
        tables.clear();
        
        // Initialize storage engine with the new directory
        if (storageEngine) {
            storageEngine->initialize(directory);
        }
        
        // Load tables from the new directory
        loadExistingTables();
    }
    
    // Get the current data directory
    const std::string& getDataDirectory() const {
        return dataDirectory;
    }
    
    // Create a new table (legacy version, prefer createTableWithColumns)
    bool createTable(const std::string& tableName) {
        // Check if table already exists
        if (tables.find(tableName) != tables.end()) {
            return false;
        }
        
        // Create empty columns list for storage engine
        std::vector<Column> emptyColumns;
        
        // Use storage engine to create table
        if (storageEngine) {
            auto newTable = storageEngine->createTable(tableName, emptyColumns);
            if (newTable) {
                // Convert shared_ptr to unique_ptr
                tables[tableName] = std::make_unique<Table>(*newTable);
                return true;
            }
        }
        
        // Failed to create table
        return false;
    }
    
    // Create a new table with columns using the storage engine
    bool createTableWithColumns(const std::string& tableName, const std::vector<Column>& columns) {
        // Check if table already exists
        if (tables.find(tableName) != tables.end()) {
            return false;
        }
        
        // Use storage engine to create table
        if (storageEngine) {
            auto newTable = storageEngine->createTable(tableName, columns);
            if (newTable) {
                // Convert shared_ptr to unique_ptr
                tables[tableName] = std::make_unique<Table>(*newTable);
        return true;
            }
        }
        
        // Failed to create table
        return false;
    }
    
    // Get a table by name
    Table* getTable(const std::string& tableName) {
        auto it = tables.find(tableName);
        if (it == tables.end()) {
            return nullptr;
        }
        return it->second.get();
    }
    
    // Drop a table
    bool dropTable(const std::string& tableName) {
        auto it = tables.find(tableName);
        if (it == tables.end()) {
            return false;
        }
        
        // Drop the table using storage engine
        if (storageEngine) {
            if (storageEngine->deleteTable(tableName)) {
        tables.erase(it);
        return true;
            }
        }
        
        // Failed to drop table
        return false;
    }
    
    // Check if a table exists
    bool tableExists(const std::string& tableName) const {
        return tables.find(tableName) != tables.end();
    }
    
    // Get list of all tables
    std::vector<std::string> getAllTableNames() const {
        std::vector<std::string> tableNames;
        tableNames.reserve(tables.size());
        
        for (const auto& [name, _] : tables) {
            tableNames.push_back(name);
        }
        
        return tableNames;
    }
    
    // Get the storage engine
    sqldb::StorageEngine* getStorageEngine() {
        return storageEngine.get();
    }
    
    // Begin a transaction
    int beginTransaction() {
        if (storageEngine) {
            return storageEngine->beginTransaction();
        }
        return -1;
    }
    
    // Commit a transaction
    bool commitTransaction(int transactionId) {
        if (storageEngine) {
            return storageEngine->commitTransaction(transactionId);
        }
        return false;
    }
    
    // Rollback a transaction
    bool rollbackTransaction(int transactionId) {
        if (storageEngine) {
            return storageEngine->rollbackTransaction(transactionId);
        }
        return false;
    }
    
    // Flush all data to disk
    bool flushData() {
        if (storageEngine) {
            return storageEngine->flushBufferPool();
        }
        return false;
    }
};

#endif // DATABASE_MANAGER_H 