#include "../include/DatabaseSystem.h"
#include "../include/QueryParser.h"
#include "../include/StringFunction.h"
#include "../include/AggregateFunction.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <regex>

// Display results in a tabular format
void displayResults(const QueryResult& result) {
    if (!result.success) {
        std::cout << "Error: " << result.message << std::endl;
        return;
    }
    
    if (!result.message.empty()) {
        std::cout << result.message << std::endl;
    }
    
    // If we have data to display
    if (!result.columnNames.empty() && !result.rows.empty()) {
        // Determine column widths (minimum 10 characters)
        const size_t MIN_WIDTH = 10;
        std::vector<size_t> colWidths(result.columnNames.size(), MIN_WIDTH);
        
        // Check column names length
        for (size_t i = 0; i < result.columnNames.size(); ++i) {
            colWidths[i] = std::max(colWidths[i], result.columnNames[i].length() + 2);
        }
        
        // Check data length
        for (const auto& row : result.rows) {
            for (size_t i = 0; i < row.size() && i < colWidths.size(); ++i) {
                colWidths[i] = std::max(colWidths[i], row[i].length() + 2);
            }
        }
        
        // Print header
        std::cout << std::string(4, ' ');
        for (size_t i = 0; i < result.columnNames.size(); ++i) {
            std::cout << "| " << std::left << std::setw(colWidths[i] - 2) 
                      << result.columnNames[i] << " ";
        }
        std::cout << "|" << std::endl;
        
        // Print separator
        std::cout << std::string(4, ' ') << "+";
        for (size_t width : colWidths) {
            std::cout << std::string(width, '-') << "+";
        }
        std::cout << std::endl;
        
        // Print rows
        for (const auto& row : result.rows) {
            std::cout << std::string(4, ' ');
            for (size_t i = 0; i < row.size() && i < colWidths.size(); ++i) {
                std::cout << "| " << std::left << std::setw(colWidths[i] - 2) 
                          << row[i] << " ";
            }
            std::cout << "|" << std::endl;
        }
        
        // Print row count
        std::cout << std::endl << result.rows.size() << " row(s) returned." << std::endl;
    }
}

// Print welcome message and help
void printHelp() {
    std::cout << "====================================================" << std::endl;
    std::cout << "  Simple SQL-like Database System" << std::endl;
    std::cout << "====================================================" << std::endl;
    std::cout << "IMPORTANT: DO NOT use semicolons at the end of commands!" << std::endl;
    std::cout << "They will result in syntax errors." << std::endl;
    std::cout << std::endl;
    std::cout << "Database commands:" << std::endl;
    std::cout << "  CREATE DATABASE dbName" << std::endl;
    std::cout << "  DROP DATABASE dbName" << std::endl;
    std::cout << "  USE dbName" << std::endl;
    std::cout << "  EXIT" << std::endl;
    std::cout << std::endl;
    std::cout << "Table commands (requires database selection):" << std::endl;
    std::cout << "  CREATE TABLE tableName (col1 TYPE1 [CONSTRAINTS], col2 TYPE2 [CONSTRAINTS], ...)" << std::endl;
    std::cout << "  DROP TABLE tableName" << std::endl;
    std::cout << "  ALTER TABLE tableName ADD columnName TYPE [CONSTRAINTS]" << std::endl;
    std::cout << "  ALTER TABLE tableName DROP columnName" << std::endl;
    std::cout << "  INSERT INTO tableName (col1, col2, ...) VALUES (val1, val2, ...)" << std::endl;
    std::cout << "  SELECT col1, col2, ... FROM tableName [WHERE conditions]" << std::endl;
    std::cout << "  UPDATE tableName SET col1=val1, col2=val2, ... [WHERE conditions]" << std::endl;
    std::cout << "  DELETE FROM tableName [WHERE conditions]" << std::endl;
    std::cout << "  DESC tableName" << std::endl;
    std::cout << "  SHOW TABLES" << std::endl;
    std::cout << std::endl;
    std::cout << "Transaction commands:" << std::endl;
    std::cout << "  BEGIN TRANSACTION (or BEGIN)" << std::endl;
    std::cout << "  COMMIT" << std::endl;
    std::cout << "  ROLLBACK" << std::endl;
    std::cout << std::endl;
    std::cout << "Available data types: INT, BIGINT, STRING, DATE" << std::endl;
    std::cout << "Available constraints: PRIMARY KEY, UNIQUE, NOT NULL" << std::endl;
    std::cout << std::endl;
    std::cout << "Special commands:" << std::endl;
    std::cout << "  .help       - Display this help message" << std::endl;
    std::cout << "  .databases  - List all databases" << std::endl;
    std::cout << "  .tables     - List all tables in the current database" << std::endl;
    std::cout << "  .flush      - Flush all data to disk" << std::endl;
    std::cout << "  .exit       - Exit the program" << std::endl;
    std::cout << "====================================================" << std::endl;
}

// List all databases
void listDatabases(DatabaseSystem& dbSystem) {
    auto dbNames = dbSystem.getAllDatabaseNames();
    
    if (dbNames.empty()) {
        std::cout << "No databases found." << std::endl;
        return;
    }
    
    std::cout << "Databases:" << std::endl;
    for (const auto& name : dbNames) {
        if (name == dbSystem.getCurrentDatabaseName()) {
            std::cout << "  - " << name << " (current)" << std::endl;
        } else {
            std::cout << "  - " << name << std::endl;
        }
    }
}

// List all tables
void listTables(DatabaseSystem& dbSystem) {
    if (dbSystem.getCurrentDatabaseName().empty()) {
        std::cout << "Error: No database selected. Use 'USE dbName' first." << std::endl;
        return;
    }
    
    DatabaseManager* dbManager = dbSystem.getCurrentDatabaseManager();
    if (!dbManager) {
        std::cout << "Error: Failed to access database." << std::endl;
        return;
    }
    
    auto tableNames = dbManager->getAllTableNames();
    
    if (tableNames.empty()) {
        std::cout << "No tables found in the current database." << std::endl;
        return;
    }
    
    std::cout << "Tables in database '" << dbSystem.getCurrentDatabaseName() << "':" << std::endl;
    for (const auto& name : tableNames) {
        std::cout << "  - " << name << std::endl;
    }
}

// Show table schema (used by DESC command)
void showTableSchema(DatabaseSystem& dbSystem, const std::string& tableName) {
    if (dbSystem.getCurrentDatabaseName().empty()) {
        std::cout << "Error: No database selected. Use 'USE dbName' first." << std::endl;
        return;
    }
    
    DatabaseManager* dbManager = dbSystem.getCurrentDatabaseManager();
    if (!dbManager) {
        std::cout << "Error: Failed to access database." << std::endl;
        return;
    }
    
    Table* table = dbManager->getTable(tableName);
    if (!table) {
        std::cout << "Error: Table '" << tableName << "' does not exist." << std::endl;
        return;
    }
    
    const auto& columns = table->getColumns();
    if (columns.empty()) {
        std::cout << "Table '" << tableName << "' has no columns." << std::endl;
        return;
    }
    
    std::cout << "Schema for table '" << tableName << "':" << std::endl;
    std::cout << "+----------------+----------+-------------+" << std::endl;
    std::cout << "| Column Name    | Type     | Constraints |" << std::endl;
    std::cout << "+----------------+----------+-------------+" << std::endl;
    
    for (const auto& column : columns) {
        std::string constraints;
        if (column.isPrimaryKey()) constraints += "PRIMARY KEY ";
        else if (column.isUnique()) constraints += "UNIQUE ";
        if (column.isNotNull() && !column.isPrimaryKey()) constraints += "NOT NULL ";
        
        std::cout << "| " << std::left << std::setw(14) << column.getName() << " | "
                  << std::setw(8) << DataType::typeToString(column.getDataType().getType()) << " | "
                  << std::setw(11) << constraints << " |" << std::endl;
    }
    
    std::cout << "+----------------+----------+-------------+" << std::endl;
}

// Create a new database
bool createDatabase(DatabaseSystem& dbSystem, const std::string& dbName) {
    if (dbName.empty()) {
        std::cout << "Error: Database name cannot be empty." << std::endl;
        return false;
    }
    
    if (dbSystem.databaseExists(dbName)) {
        std::cout << "Error: Database '" << dbName << "' already exists." << std::endl;
        return false;
    }
    
    std::cout << "Creating database '" << dbName << "'..." << std::endl;
    
    if (dbSystem.createDatabase(dbName)) {
        std::cout << "Database '" << dbName << "' created successfully." << std::endl;
        return true;
    } else {
        std::cout << "Error: Failed to create database '" << dbName << "'." << std::endl;
        return false;
    }
}

// Drop a database
bool dropDatabase(DatabaseSystem& dbSystem, const std::string& dbName) {
    if (dbName.empty()) {
        std::cout << "Error: Database name cannot be empty." << std::endl;
        return false;
    }
    
    if (!dbSystem.databaseExists(dbName)) {
        std::cout << "Error: Database '" << dbName << "' does not exist." << std::endl;
        return false;
    }
    
    if (dbSystem.dropDatabase(dbName)) {
        std::cout << "Database '" << dbName << "' dropped successfully." << std::endl;
        return true;
    } else {
        std::cout << "Error: Failed to drop database '" << dbName << "'." << std::endl;
        return false;
    }
}

// Use a database
bool useDatabase(DatabaseSystem& dbSystem, const std::string& dbName) {
    if (dbName.empty()) {
        std::cout << "Error: Database name cannot be empty." << std::endl;
        return false;
    }
    
    if (!dbSystem.databaseExists(dbName)) {
        std::cout << "Error: Database '" << dbName << "' does not exist." << std::endl;
        return false;
    }
    
    if (dbSystem.useDatabase(dbName)) {
        std::cout << "Using database '" << dbName << "'." << std::endl;
        return true;
    } else {
        std::cout << "Error: Failed to use database '" << dbName << "'." << std::endl;
        return false;
    }
}

// Exit the current database
bool exitDatabase(DatabaseSystem& dbSystem) {
    if (dbSystem.getCurrentDatabaseName().empty()) {
        std::cout << "Error: No database currently selected." << std::endl;
        return false;
    }
    
    std::string currentDb = dbSystem.getCurrentDatabaseName();
    dbSystem.useDatabase(""); // Set current database to empty
    std::cout << "Exited database '" << currentDb << "'." << std::endl;
    return true;
}

// Check if command is a table command that requires database selection
bool isTableCommand(const std::string& query) {
    // Clean query by removing trailing semicolon for pattern matching
    std::string cleanQuery = query;
    if (!cleanQuery.empty() && cleanQuery.back() == ';') {
        cleanQuery.pop_back();
    }
    
    std::regex createTablePattern(R"(CREATE\s+TABLE)", std::regex_constants::icase);
    std::regex dropTablePattern(R"(DROP\s+TABLE)", std::regex_constants::icase);
    std::regex alterTablePattern(R"(ALTER\s+TABLE)", std::regex_constants::icase);
    std::regex insertPattern(R"(INSERT\s+INTO)", std::regex_constants::icase);
    std::regex selectPattern(R"(SELECT)", std::regex_constants::icase);
    std::regex updatePattern(R"(UPDATE\s+\w+\s+SET)", std::regex_constants::icase);
    std::regex deletePattern(R"(DELETE\s+FROM)", std::regex_constants::icase);
    std::regex descPattern(R"(DESC\s+\w+)", std::regex_constants::icase);
    std::regex showTablesPattern(R"(SHOW\s+TABLES)", std::regex_constants::icase);
    std::regex beginPattern(R"(^BEGIN(\s+TRANSACTION)?$)", std::regex_constants::icase);
    std::regex commitPattern(R"(^COMMIT$)", std::regex_constants::icase);
    std::regex rollbackPattern(R"(^ROLLBACK$)", std::regex_constants::icase);
    
    return std::regex_search(cleanQuery, createTablePattern) ||
           std::regex_search(cleanQuery, dropTablePattern) ||
           std::regex_search(cleanQuery, alterTablePattern) ||
           std::regex_search(cleanQuery, insertPattern) ||
           std::regex_search(cleanQuery, selectPattern) ||
           std::regex_search(cleanQuery, updatePattern) ||
           std::regex_search(cleanQuery, deletePattern) ||
           std::regex_search(cleanQuery, descPattern) ||
           std::regex_search(cleanQuery, showTablesPattern) ||
           std::regex_match(cleanQuery, beginPattern) ||
           std::regex_match(cleanQuery, commitPattern) ||
           std::regex_match(cleanQuery, rollbackPattern);
}

// Check if command is a database command
bool isDatabaseCommand(const std::string& query) {
    // Clean query by removing trailing semicolon for pattern matching
    std::string cleanQuery = query;
    if (!cleanQuery.empty() && cleanQuery.back() == ';') {
        cleanQuery.pop_back();
    }
    
    std::regex createDbPattern(R"(CREATE\s+DATABASE\s+\w+)", std::regex_constants::icase);
    std::regex dropDbPattern(R"(DROP\s+DATABASE\s+\w+)", std::regex_constants::icase);
    std::regex useDbPattern(R"(USE\s+\w+)", std::regex_constants::icase);
    std::regex exitPattern(R"(EXIT)", std::regex_constants::icase);
    
    return std::regex_match(cleanQuery, createDbPattern) ||
           std::regex_match(cleanQuery, dropDbPattern) ||
           std::regex_match(cleanQuery, useDbPattern) ||
           std::regex_match(cleanQuery, exitPattern);
}

// Process database commands
bool processDatabaseCommand(DatabaseSystem& dbSystem, const std::string& query) {
    // CREATE DATABASE command
    std::regex createDbPattern(R"(CREATE\s+DATABASE\s+(\w+))", std::regex_constants::icase);
    std::smatch createDbMatches;
    if (std::regex_search(query, createDbMatches, createDbPattern)) {
        std::string dbName = createDbMatches[1].str();
        return createDatabase(dbSystem, dbName);
    }
    
    // DROP DATABASE command
    std::regex dropDbPattern(R"(DROP\s+DATABASE\s+(\w+))", std::regex_constants::icase);
    std::smatch dropDbMatches;
    if (std::regex_search(query, dropDbMatches, dropDbPattern)) {
        std::string dbName = dropDbMatches[1].str();
        return dropDatabase(dbSystem, dbName);
    }
    
    // USE DATABASE command
    std::regex useDbPattern(R"(USE\s+(\w+))", std::regex_constants::icase);
    std::smatch useDbMatches;
    if (std::regex_search(query, useDbMatches, useDbPattern)) {
        std::string dbName = useDbMatches[1].str();
        return useDatabase(dbSystem, dbName);
    }
    
    // EXIT command to exit current database
    std::regex exitDbPattern(R"(EXIT)", std::regex_constants::icase);
    if (std::regex_match(query, exitDbPattern)) {
        return exitDatabase(dbSystem);
    }
    
    // SHOW TABLES command
    std::regex showTablesPattern(R"(SHOW\s+TABLES)", std::regex_constants::icase);
    if (std::regex_match(query, showTablesPattern)) {
        listTables(dbSystem);
        return true;
    }
    
    // DESC command
    std::regex descPattern(R"(DESC\s+(\w+))", std::regex_constants::icase);
    std::smatch descMatches;
    if (std::regex_search(query, descMatches, descPattern)) {
        std::string tableName = descMatches[1].str();
        showTableSchema(dbSystem, tableName);
        return true;
    }
    
    // Not a database command
    return false;
}

int main() {
    // Initialize database system
    DatabaseSystem dbSystem;
    
    // Print welcome message
    std::cout << "Welcome to the SQL-like Database System" << std::endl;
    std::cout << "Type '.help' for usage information or '.exit' to quit" << std::endl;
    
    // Main command loop
    std::string input;
    bool exit = false;
    
    // Keep track of the current query parser (one per database)
    std::unique_ptr<QueryParser> queryParser;
    
    while (!exit) {
        // Show prompt
            if (dbSystem.getCurrentDatabaseName().empty()) {
            std::cout << "> ";
            } else {
            std::cout << dbSystem.getCurrentDatabaseName();
            
            // Show transaction status
            if (queryParser && queryParser->isInTransaction()) {
                std::cout << " (TRANSACTION)";
            }
            
            std::cout << "> ";
        }
        
        // Get user input
        std::getline(std::cin, input);
        
        // Trim input
        input.erase(0, input.find_first_not_of(" \t\r\n"));
        
        // Remove only trailing whitespace, keeping semicolons
        if (!input.empty()) {
            size_t end = input.find_last_not_of(" \t\r\n");
            if (end != std::string::npos) {
                input.erase(end + 1);
            }
        }
        
        // Skip empty input
        if (input.empty()) {
            continue;
        }
        
        // Check for semicolons and reject if found (except in special dot commands)
        if (input[0] != '.' && input.find(';') != std::string::npos) {
            std::cout << "Error: Semicolons are not allowed in commands. Please remove all semicolons." << std::endl;
            continue;
        }
        
        // Handle special commands (starting with a dot)
        if (input[0] == '.') {
            std::string command = input.substr(1);
            std::transform(command.begin(), command.end(), command.begin(), 
                [](unsigned char c) { return std::tolower(c); });
                
            if (command == "exit" || command == "quit") {
                exit = true;
            } else if (command == "help") {
                printHelp();
            } else if (command == "databases") {
                listDatabases(dbSystem);
            } else if (command == "tables") {
                listTables(dbSystem);
            } else if (command == "flush") {
                if (queryParser) {
                    auto result = queryParser->flushData();
                    std::cout << result.message << std::endl;
                } else {
                    std::cout << "Error: No database selected. Use 'USE dbName' first." << std::endl;
                }
            } else {
                std::cout << "Unknown command: " << command << std::endl;
            }
            
            continue;
        }
        
        // Handle "DESC tableName" command
        std::regex descPattern(R"(^\s*DESC\s+(\w+)\s*$)", std::regex_constants::icase);
        std::smatch descMatches;
        if (std::regex_search(input, descMatches, descPattern)) {
            std::string tableName = descMatches[1].str();
            showTableSchema(dbSystem, tableName);
            continue;
        }
        
        // Handle "SHOW TABLES" command
        std::regex showTablesPattern(R"(^\s*SHOW\s+TABLES\s*$)", std::regex_constants::icase);
        if (std::regex_search(input, showTablesPattern)) {
            listTables(dbSystem);
            continue;
        }
        
        // Check if this is a database-level command
        if (isDatabaseCommand(input)) {
            processDatabaseCommand(dbSystem, input);
            
            // Update query parser if database changed
            if (!dbSystem.getCurrentDatabaseName().empty()) {
                DatabaseManager* dbManager = dbSystem.getCurrentDatabaseManager();
                if (dbManager) {
                    queryParser = std::make_unique<QueryParser>(*dbManager);
                }
                } else {
                queryParser.reset();
            }
            
            continue;
        }
        
        // Check if this is a table-level command
        if (isTableCommand(input)) {
            // Ensure a database is selected
            if (dbSystem.getCurrentDatabaseName().empty()) {
                std::cout << "Error: No database selected. Use 'USE dbName' first." << std::endl;
                continue;
            }
            
            // Get the current database manager
            DatabaseManager* dbManager = dbSystem.getCurrentDatabaseManager();
            if (!dbManager) {
                std::cout << "Error: Failed to access database." << std::endl;
                continue;
            }
            
            // Create query parser if it doesn't exist
            if (!queryParser) {
                queryParser = std::make_unique<QueryParser>(*dbManager);
            }
            
            // Execute the query, using the processed input without semicolon
            auto result = queryParser->parseQuery(input);
                    displayResults(result);
            
            continue;
            }
            
        // If we get here, the command is not recognized
        std::cout << "Unrecognized command: " << input << std::endl;
            }
            
    // Ensure data is flushed before exiting
    if (queryParser) {
        queryParser->flushData();
    }
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
} 