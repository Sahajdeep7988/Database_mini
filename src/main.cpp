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
    std::cout << "Available data types: INT, BIGINT, STRING, DATE" << std::endl;
    std::cout << "Available constraints: PRIMARY KEY, UNIQUE, NOT NULL" << std::endl;
    std::cout << std::endl;
    std::cout << "Special commands:" << std::endl;
    std::cout << "  .help       - Display this help message" << std::endl;
    std::cout << "  .databases  - List all databases" << std::endl;
    std::cout << "  .tables     - List all tables in the current database" << std::endl;
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
    std::regex createTablePattern(R"(CREATE\s+TABLE)", std::regex_constants::icase);
    std::regex dropTablePattern(R"(DROP\s+TABLE)", std::regex_constants::icase);
    std::regex alterTablePattern(R"(ALTER\s+TABLE)", std::regex_constants::icase);
    std::regex insertPattern(R"(INSERT\s+INTO)", std::regex_constants::icase);
    std::regex selectPattern(R"(SELECT)", std::regex_constants::icase);
    std::regex updatePattern(R"(UPDATE\s+\w+\s+SET)", std::regex_constants::icase);
    std::regex deletePattern(R"(DELETE\s+FROM)", std::regex_constants::icase);
    std::regex descPattern(R"(DESC\s+\w+)", std::regex_constants::icase);
    std::regex showTablesPattern(R"(SHOW\s+TABLES)", std::regex_constants::icase);
    
    return std::regex_search(query, createTablePattern) ||
           std::regex_search(query, dropTablePattern) ||
           std::regex_search(query, alterTablePattern) ||
           std::regex_search(query, insertPattern) ||
           std::regex_search(query, selectPattern) ||
           std::regex_search(query, updatePattern) ||
           std::regex_search(query, deletePattern) ||
           std::regex_search(query, descPattern) ||
           std::regex_search(query, showTablesPattern);
}

// Check if command is a database command
bool isDatabaseCommand(const std::string& query) {
    std::regex createDbPattern(R"(CREATE\s+DATABASE\s+\w+)", std::regex_constants::icase);
    std::regex dropDbPattern(R"(DROP\s+DATABASE\s+\w+)", std::regex_constants::icase);
    std::regex useDbPattern(R"(USE\s+\w+)", std::regex_constants::icase);
    std::regex exitPattern(R"(EXIT)", std::regex_constants::icase);
    
    return std::regex_match(query, createDbPattern) ||
           std::regex_match(query, dropDbPattern) ||
           std::regex_match(query, useDbPattern) ||
           std::regex_match(query, exitPattern);
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
    // Create the database system
    DatabaseSystem dbSystem;
    
    // Print welcome message
    printHelp();
    
    // Main command loop
    std::string line;
    std::string query;
    bool multiline = false;
    
    std::cout << std::endl << "Enter a SQL command (or .help, .databases, .tables, .exit):" << std::endl;
    
    while (true) {
        // Print prompt with current database
        if (!multiline) {
            if (dbSystem.getCurrentDatabaseName().empty()) {
                std::cout << "sql> ";
            } else {
                std::cout << dbSystem.getCurrentDatabaseName() << "> ";
            }
        } else {
            std::cout << "...> ";
        }
        
        // Get line of input
        std::getline(std::cin, line);
        
        // Check for special commands (only in single-line mode)
        if (!multiline) {
            // Convert the line to lowercase for case-insensitive comparison
            std::string lineLower = line;
            std::transform(lineLower.begin(), lineLower.end(), lineLower.begin(), 
                [](unsigned char c) { return std::tolower(c); });
                
            if (lineLower == ".exit") {
                break;
            } else if (lineLower == ".help") {
                printHelp();
                continue;
            } else if (lineLower == ".databases") {
                listDatabases(dbSystem);
                continue;
            } else if (lineLower == ".tables") {
                listTables(dbSystem);
                continue;
            }
        }
        
        // Add line to query
        query += line;
        
        // Check if query is complete (ends with semicolon)
        if (!query.empty() && query.back() == ';') {
            // Remove semicolon
            query.pop_back();
            
            // Process the command
            bool commandHandled = false;
            
            // First check if it's a database command
            commandHandled = processDatabaseCommand(dbSystem, query);
            
            // If it's not a database command, check if it's a table command
            if (!commandHandled && isTableCommand(query)) {
                // Table commands require a database to be selected
                DatabaseManager* dbManager = dbSystem.getCurrentDatabaseManager();
                if (!dbManager) {
                    std::cout << "Error: No database selected. Use 'USE dbName' first." << std::endl;
                    commandHandled = true; // Mark as handled to avoid unknown command error
                } else {
                    // Create query parser for the current database
                    QueryParser queryParser(*dbManager);
                    
                    // Parse and execute query
                    QueryResult result = queryParser.parseQuery(query);
                    
                    // Display results
                    displayResults(result);
                    commandHandled = true;
                }
            }
            
            // If it's neither a database nor a recognized table command
            if (!commandHandled) {
                std::cout << "Error: Unknown command. Type .help for assistance." << std::endl;
            }
            
            // Reset for next query
            query.clear();
            multiline = false;
        } else {
            // Not complete, add newline and continue
            query += " ";
            multiline = true;
        }
    }
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
} 