#ifndef DATABASE_SYSTEM_H
#define DATABASE_SYSTEM_H

#include "DatabaseManager.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>    // For errno
#ifdef _WIN32
    #include <direct.h>   // For _mkdir on Windows
    #define MKDIR(dir) _mkdir(dir)
#else
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>   // For rmdir on POSIX systems
    #define MKDIR(dir) mkdir(dir, 0777)
#endif

class DatabaseSystem {
private:
    std::unordered_map<std::string, std::unique_ptr<DatabaseManager>> databases;
    std::string currentDatabase;
    std::string baseDirectory;
    
    // Helper function to create a directory
    bool createDir(const std::string& path) {
        int result = MKDIR(path.c_str());
        
        // If directory was created OR it already exists, return true
        return (result == 0 || errno == EEXIST);
    }
    
    // Helper function to remove a file
    bool removeFile(const std::string& path) {
        return std::remove(path.c_str()) == 0;
    }
    
    // Helper function to recursively remove a directory
    bool removeDir(const std::string& path) {
        DIR* dir = opendir(path.c_str());
        if (!dir) {
            return false;
        }
        
        struct dirent* entry;
        bool success = true;
        
        // First, remove all files and subdirectories
        while ((entry = readdir(dir)) != NULL) {
            std::string name = entry->d_name;
            
            // Skip . and ..
            if (name == "." || name == "..") {
                continue;
            }
            
            // Build full path
            std::string fullPath;
            #ifdef _WIN32
                fullPath = path + "\\" + name;
            #else
                fullPath = path + "/" + name;
            #endif
            
            // Check if it's a directory
            struct stat statbuf;
            if (stat(fullPath.c_str(), &statbuf) != 0) {
                success = false;
                continue;
            }
            
            if (S_ISDIR(statbuf.st_mode)) {
                // Recursively remove subdirectory
                if (!removeDir(fullPath)) {
                    success = false;
                }
            } else {
                // Remove file
                if (!removeFile(fullPath)) {
                    success = false;
                }
            }
        }
        
        closedir(dir);
        
        // Now remove the directory itself
        #ifdef _WIN32
            success = _rmdir(path.c_str()) == 0 && success;
        #else
            success = rmdir(path.c_str()) == 0 && success;
        #endif
        
        return success;
    }
    
    // Helper function to create a database directory
    bool createDatabaseDirectory(const std::string& dbName) {
        // First ensure the base directory exists
        if (!createDir(baseDirectory)) {
            std::cerr << "Error creating base directory: " << baseDirectory << std::endl;
            return false;
        }
        
        // Construct database path
        std::string dbPath;
        std::string dataPath;
        #ifdef _WIN32
            dbPath = baseDirectory + "\\" + dbName;
            dataPath = dbPath + "\\data";
        #else
            dbPath = baseDirectory + "/" + dbName;
            dataPath = dbPath + "/data";
        #endif
        
        // Create database directory
        if (!createDir(dbPath)) {
            std::cerr << "Error creating database directory: " << dbPath << std::endl;
            return false;
        }
        
        // Create data directory inside database
        if (!createDir(dataPath)) {
            std::cerr << "Error creating data directory: " << dataPath << std::endl;
            return false;
        }
        
        // Ensure transaction logs directory exists
        if (!createDir("test_transactions")) {
            std::cerr << "Warning: Failed to create transaction logs directory" << std::endl;
        }
        
        return true;
    }
    
    void loadExistingDatabases() {
        // Ensure base directory exists
        if (!createDir(baseDirectory)) {
            std::cerr << "Failed to create base directory: " << baseDirectory << std::endl;
            return;
        }
        
        DIR* dir = opendir(baseDirectory.c_str());
        if (!dir) {
            std::cerr << "Failed to open base directory: " << baseDirectory << std::endl;
            return;
        }
        
        // Read directory entries
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            std::string dbName = entry->d_name;
            
            // Skip . and ..
            if (dbName == "." || dbName == "..") {
                continue;
            }
            
            // Check if it's a directory
            std::string fullPath;
            #ifdef _WIN32
                fullPath = baseDirectory + "\\" + dbName;
            #else
                fullPath = baseDirectory + "/" + dbName;
            #endif
            
            struct stat statbuf;
            if (stat(fullPath.c_str(), &statbuf) != 0) {
                continue;
            }
            
            if (S_ISDIR(statbuf.st_mode)) {
                // Create data directory if it doesn't exist
                std::string dataPath;
                #ifdef _WIN32
                    dataPath = fullPath + "\\data";
                #else
                    dataPath = fullPath + "/data";
                #endif
                
                // Ensure data directory exists
                createDir(dataPath);
                
                // Create database manager for this database
                auto dbManager = std::make_unique<DatabaseManager>();
                dbManager->setDataDirectory(dataPath);
                databases[dbName] = std::move(dbManager);
            }
        }
        
        closedir(dir);
    }
    
public:
    DatabaseSystem(const std::string& baseDir = "databases") : baseDirectory(baseDir) {
        loadExistingDatabases();
    }
    
    // Create a new database
    bool createDatabase(const std::string& dbName) {
        // Check if database already exists
        if (databases.find(dbName) != databases.end()) {
            return false;
        }
        
        // Create database directory
        if (!createDatabaseDirectory(dbName)) {
            return false;
        }
        
        // Create database manager
        auto dbManager = std::make_unique<DatabaseManager>();
        
        // Set the correct data path based on platform
        std::string dataPath;
        #ifdef _WIN32
            dataPath = baseDirectory + "\\" + dbName + "\\data";
        #else
            dataPath = baseDirectory + "/" + dbName + "/data";
        #endif
        
        dbManager->setDataDirectory(dataPath);
        
        // Add to databases map
        databases[dbName] = std::move(dbManager);
        
        return true;
    }
    
    // Drop a database
    bool dropDatabase(const std::string& dbName) {
        // Check if database exists
        auto it = databases.find(dbName);
        if (it == databases.end()) {
            return false;
        }
        
        // If it's the current database, unselect it
        if (currentDatabase == dbName) {
            currentDatabase.clear();
        }
        
        // Construct the database path
        std::string dbPath;
        #ifdef _WIN32
            dbPath = baseDirectory + "\\" + dbName;
        #else
            dbPath = baseDirectory + "/" + dbName;
        #endif
        
        // Remove from databases map - do this before physical deletion
        // to release any files that might be open
        databases.erase(it);
        
        // Physically remove the database directory and all its contents
        bool deletionSuccess = removeDir(dbPath);
        
        if (!deletionSuccess) {
            std::cerr << "Warning: Could not completely delete database directory: " << dbPath << std::endl;
            std::cerr << "Some files may remain on disk." << std::endl;
        }
        
        return true; // Return true even if physical deletion had issues (database is logically removed)
    }
    
    // Switch to a database
    bool useDatabase(const std::string& dbName) {
        // Allow empty string to unselect database
        if (dbName.empty()) {
            currentDatabase.clear();
            return true;
        }
        
        // Check if database exists
        if (databases.find(dbName) == databases.end()) {
            return false;
        }
        
        // Set current database
        currentDatabase = dbName;
        return true;
    }
    
    // Get current database manager
    DatabaseManager* getCurrentDatabaseManager() {
        if (currentDatabase.empty() || databases.find(currentDatabase) == databases.end()) {
            return nullptr;
        }
        
        return databases[currentDatabase].get();
    }
    
    // Get current database name
    const std::string& getCurrentDatabaseName() const {
        return currentDatabase;
    }
    
    // Check if a database exists
    bool databaseExists(const std::string& dbName) const {
        return databases.find(dbName) != databases.end();
    }
    
    // Get list of all databases
    std::vector<std::string> getAllDatabaseNames() const {
        std::vector<std::string> dbNames;
        dbNames.reserve(databases.size());
        
        for (const auto& [name, _] : databases) {
            dbNames.push_back(name);
        }
        
        return dbNames;
    }
};

#endif // DATABASE_SYSTEM_H 