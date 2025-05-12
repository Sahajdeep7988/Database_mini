#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "Table.h"
#include <unordered_map>
#include <memory>
#include <dirent.h>
#include <set>
#include <string.h>

class DatabaseManager {
private:
    std::unordered_map<std::string, std::unique_ptr<Table>> tables;
    std::string dataDirectory;
    
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
        
        // Set to keep track of table names (we'll have .meta and .data files)
        std::set<std::string> tableNames;
        
        // Read directory entries
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            std::string filename = entry->d_name;
            
            // Skip . and ..
            if (filename == "." || filename == "..") {
                continue;
            }
            
            // Check if file has .meta extension
            size_t pos = filename.find(".meta");
            if (pos != std::string::npos) {
                // Extract table name
                std::string tableName = filename.substr(0, pos);
                tableNames.insert(tableName);
            }
        }
        
        closedir(dir);
        
        // Load tables
        for (const auto& tableName : tableNames) {
            tables[tableName] = std::make_unique<Table>(tableName, dataDirectory);
        }
    }
    
public:
    DatabaseManager() : dataDirectory("data") {
        loadExistingTables();
    }
    
    // Set the data directory path
    void setDataDirectory(const std::string& directory) {
        dataDirectory = directory;
        // Clear existing tables
        tables.clear();
        // Load tables from the new directory
        loadExistingTables();
    }
    
    // Get the current data directory
    const std::string& getDataDirectory() const {
        return dataDirectory;
    }
    
    // Create a new table
    bool createTable(const std::string& tableName) {
        // Check if table already exists
        if (tables.find(tableName) != tables.end()) {
            return false;
        }
        
        // Create new table
        tables[tableName] = std::make_unique<Table>(tableName, dataDirectory);
        return true;
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
        
        // Drop the table
        if (!it->second->drop()) {
            return false;
        }
        
        // Remove from map
        tables.erase(it);
        return true;
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
};

#endif // DATABASE_MANAGER_H 