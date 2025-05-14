#include "../../include/storage_engine/StorageEngine.h"
#include "../../include/storage_engine/BufferPoolManager.h"
#include "../../include/storage_engine/TransactionManager.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace sqldb {

StorageEngine::StorageEngine(size_t bufferPoolSize)
    : bufferPoolSize_(bufferPoolSize), currentTransactionId_(0) {
}

StorageEngine::~StorageEngine() {
    // Flush all dirty pages to disk
    flushBufferPool();
}

bool StorageEngine::initialize(const std::string& dbPath) {
    dbPath_ = dbPath;
    
    // Create the database directory if it doesn't exist
    if (!std::filesystem::exists(dbPath_)) {
        std::filesystem::create_directories(dbPath_);
    }
    
    // Load existing tables
    for (const auto& entry : std::filesystem::directory_iterator(dbPath_)) {
        if (entry.is_regular_file() && entry.path().extension() == ".table") {
            std::string tableName = entry.path().stem().string();
            std::shared_ptr<Table> table = loadTable(tableName);
            if (table) {
                tables_[tableName] = table;
            }
        }
    }
    
    return true;
}

std::shared_ptr<Table> StorageEngine::loadTable(const std::string& tableName) {
    std::string tablePath = dbPath_ + "/" + tableName + ".table";
    
    if (!std::filesystem::exists(tablePath)) {
        std::cerr << "Table file does not exist: " << tablePath << std::endl;
        return nullptr;
    }
    
    std::ifstream file(tablePath, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open table file: " << tablePath << std::endl;
        return nullptr;
    }
    
    // Read table metadata
    std::vector<Column> columns;
    size_t columnCount;
    file.read(reinterpret_cast<char*>(&columnCount), sizeof(columnCount));
    
    for (size_t i = 0; i < columnCount; ++i) {
        std::string name;
        DataType type;
        bool primaryKey;
        bool notNull;
        
        // Read column name
        size_t nameLength;
        file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
        name.resize(nameLength);
        file.read(&name[0], nameLength);
        
        // Read column type
        file.read(reinterpret_cast<char*>(&type), sizeof(type));
        
        // Read column constraints
        file.read(reinterpret_cast<char*>(&primaryKey), sizeof(primaryKey));
        file.read(reinterpret_cast<char*>(&notNull), sizeof(notNull));
        
        columns.emplace_back(name, type, primaryKey, notNull);
    }
    
    // Create the table
    std::shared_ptr<Table> table = std::make_shared<Table>(tableName, columns);
    
    // Read table rows
    size_t rowCount;
    file.read(reinterpret_cast<char*>(&rowCount), sizeof(rowCount));
    
    for (size_t i = 0; i < rowCount; ++i) {
        std::vector<Value> values;
        
        for (size_t j = 0; j < columnCount; ++j) {
            DataType type = columns[j].getType();
            Value value;
            
            // Read value type
            file.read(reinterpret_cast<char*>(&value.type), sizeof(value.type));
            
            // Read value data
            switch (value.type) {
                case DataType::INT: {
                    int intValue;
                    file.read(reinterpret_cast<char*>(&intValue), sizeof(intValue));
                    value.intValue = intValue;
                    break;
                }
                case DataType::DOUBLE: {
                    double doubleValue;
                    file.read(reinterpret_cast<char*>(&doubleValue), sizeof(doubleValue));
                    value.doubleValue = doubleValue;
                    break;
                }
                case DataType::STRING: {
                    size_t stringLength;
                    file.read(reinterpret_cast<char*>(&stringLength), sizeof(stringLength));
                    std::string stringValue;
                    stringValue.resize(stringLength);
                    file.read(&stringValue[0], stringLength);
                    value.stringValue = stringValue;
                    break;
                }
                case DataType::BOOL: {
                    bool boolValue;
                    file.read(reinterpret_cast<char*>(&boolValue), sizeof(boolValue));
                    value.boolValue = boolValue;
                    break;
                }
                default:
                    break;
            }
            
            values.push_back(value);
        }
        
        table->insertRow(values);
    }
    
    return table;
}

std::shared_ptr<Table> StorageEngine::createTable(const std::string& tableName, const std::vector<Column>& columns) {
    if (tables_.find(tableName) != tables_.end()) {
        std::cerr << "Table already exists: " << tableName << std::endl;
        return nullptr;
    }
    
    std::shared_ptr<Table> table = std::make_shared<Table>(tableName, columns);
    tables_[tableName] = table;
    
    // Save table metadata to disk
    std::string tablePath = dbPath_ + "/" + tableName + ".table";
    std::ofstream file(tablePath, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to create table file: " << tablePath << std::endl;
        tables_.erase(tableName);
        return nullptr;
    }
    
    // Write column metadata
    size_t columnCount = columns.size();
    file.write(reinterpret_cast<const char*>(&columnCount), sizeof(columnCount));
    
    for (const auto& column : columns) {
        std::string name = column.getName();
        DataType type = column.getType();
        bool primaryKey = column.isPrimaryKey();
        bool notNull = column.isNotNull();
        
        // Write column name
        size_t nameLength = name.size();
        file.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        file.write(name.c_str(), nameLength);
        
        // Write column type
        file.write(reinterpret_cast<const char*>(&type), sizeof(type));
        
        // Write column constraints
        file.write(reinterpret_cast<const char*>(&primaryKey), sizeof(primaryKey));
        file.write(reinterpret_cast<const char*>(&notNull), sizeof(notNull));
    }
    
    // Write empty row count
    size_t rowCount = 0;
    file.write(reinterpret_cast<const char*>(&rowCount), sizeof(rowCount));
    
    return table;
}

bool StorageEngine::deleteTable(const std::string& tableName) {
    auto it = tables_.find(tableName);
    if (it == tables_.end()) {
        std::cerr << "Table does not exist: " << tableName << std::endl;
        return false;
    }
    
    tables_.erase(it);
    
    // Delete table file
    std::string tablePath = dbPath_ + "/" + tableName + ".table";
    if (std::filesystem::exists(tablePath)) {
        std::filesystem::remove(tablePath);
    }
    
    return true;
}

int StorageEngine::beginTransaction() {
    // TODO: Implement transaction management
    return ++currentTransactionId_;
}

bool StorageEngine::commitTransaction(int transactionId) {
    // TODO: Implement transaction management
    return true;
}

bool StorageEngine::rollbackTransaction(int transactionId) {
    // TODO: Implement transaction management
    return true;
}

bool StorageEngine::flushBufferPool() {
    // TODO: Implement buffer pool management
    return true;
}

} // namespace sqldb 