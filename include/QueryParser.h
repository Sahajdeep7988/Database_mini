#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "DatabaseManager.h"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>
#include <stdexcept>
#include <memory>
#include <functional>

// Structure to hold query results
struct QueryResult {
    bool success;
    std::string message;
    std::vector<std::string> columnNames;
    std::vector<std::vector<std::string>> rows;
    
    QueryResult(bool s = true, const std::string& msg = "") 
        : success(s), message(msg) {}
};

class QueryParser {
private:
    DatabaseManager& dbManager;
    
    // Helper: Trim whitespace from a string
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }
    
    // Helper: Convert string to uppercase
    std::string toUpper(const std::string& str) {
        std::string upper = str;
        std::transform(upper.begin(), upper.end(), upper.begin(), 
            [](unsigned char c){ return std::toupper(c); });
        return upper;
    }
    
    // Helper: Split string by delimiter
    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(trim(token));
        }
        return tokens;
    }
    
    // Helper: Find closing parenthesis
    size_t findClosingParen(const std::string& str, size_t openPos) {
        int count = 1;
        for (size_t i = openPos + 1; i < str.size(); i++) {
            if (str[i] == '(') count++;
            else if (str[i] == ')') {
                count--;
                if (count == 0) return i;
            }
        }
        return std::string::npos;
    }
    
    // Parse CREATE TABLE statement
    QueryResult parseCreateTable(const std::string& query) {
        std::regex createPattern(R"(CREATE\s+TABLE\s+(\w+)\s*\((.*)\))", std::regex_constants::icase);
        std::smatch matches;
        
        if (!std::regex_search(query, matches, createPattern)) {
            return QueryResult(false, "Invalid CREATE TABLE syntax");
        }
        
        std::string tableName = matches[1].str();
        std::string columnsStr = matches[2].str();
        
        // Check if table already exists before attempting to create it
        if (dbManager.tableExists(tableName)) {
            return QueryResult(false, "Table '" + tableName + "' already exists");
        }
        
        // Parse column definitions first to validate before creating the table
        auto columnDefs = split(columnsStr, ',');
        bool hasPrimaryKey = false;
        
        // Store column definitions for later creation
        struct ColumnDefinition {
            std::string name;
            Type type;
            bool isPrimaryKey;
            bool isUnique;
            bool isNotNull;
        };
        std::vector<ColumnDefinition> columns;
        
        // Validate all column definitions before creating anything
        for (const auto& colDef : columnDefs) {
            std::string trimmedColDef = trim(colDef);
            // Improved regex pattern to better handle spaces
            std::regex colPattern(R"((\w+)\s*,?\s*(\w+)\s*(.*)?)", std::regex_constants::icase);
            std::smatch colMatches;
            
            if (!std::regex_search(trimmedColDef, colMatches, colPattern)) {
                return QueryResult(false, "Invalid column definition: " + trimmedColDef);
            }
            
            std::string colName = colMatches[1].str();
            std::string colTypeStr = toUpper(colMatches[2].str());
            std::string constraintsStr = colMatches[3].matched ? toUpper(colMatches[3].str()) : "";
            
            // Convert string to DataType
            Type colType;
            try {
                if (colTypeStr == "INT") colType = Type::INT;
                else if (colTypeStr == "BIGINT") colType = Type::BIGINT;
                else if (colTypeStr == "STRING") colType = Type::STRING;
                else if (colTypeStr == "DATE") colType = Type::DATE;
                else {
                    return QueryResult(false, "Invalid data type '" + colTypeStr + "' for column '" + colName + "'");
                }
            } catch (const std::exception& e) {
                return QueryResult(false, "Error parsing data type for column '" + colName + "': " + std::string(e.what()));
            }
            
            // Parse constraints - handle both with and without brackets/quotes
            bool isPrimaryKey = constraintsStr.find("PRIMARY KEY") != std::string::npos;
            bool isUnique = constraintsStr.find("UNIQUE") != std::string::npos;
            bool isNotNull = constraintsStr.find("NOT NULL") != std::string::npos;
            
            if (isPrimaryKey) {
                if (hasPrimaryKey) {
                    return QueryResult(false, "Table cannot have multiple PRIMARY KEYs");
                }
                hasPrimaryKey = true;
            }
            
            // Store column for later creation
            columns.push_back({colName, colType, isPrimaryKey, isUnique, isNotNull});
        }
        
        // All column definitions are valid, now create the table
        if (!dbManager.createTable(tableName)) {
            return QueryResult(false, "Failed to create table '" + tableName + "'");
        }
        
        Table* table = dbManager.getTable(tableName);
        if (!table) {
            return QueryResult(false, "Failed to create table '" + tableName + "'");
        }
        
        // Add all columns to the table
        bool success = true;
        for (const auto& col : columns) {
            if (!table->addColumn(col.name, col.type, col.isPrimaryKey, col.isUnique, col.isNotNull)) {
                success = false;
                break;
            }
        }
        
        // If any column addition failed, drop the table and return error
        if (!success) {
            dbManager.dropTable(tableName);
            return QueryResult(false, "Failed to create columns for table '" + tableName + "'");
        }
        
        return QueryResult(true, "Table '" + tableName + "' created successfully");
    }
    
    // Parse DROP TABLE statement
    QueryResult parseDropTable(const std::string& query) {
        std::regex dropPattern(R"(DROP\s+TABLE\s+(\w+))", std::regex_constants::icase);
        std::smatch matches;
        
        if (!std::regex_search(query, matches, dropPattern)) {
            return QueryResult(false, "Invalid DROP TABLE syntax");
        }
        
        std::string tableName = matches[1].str();
        
        if (!dbManager.tableExists(tableName)) {
            return QueryResult(false, "Table '" + tableName + "' does not exist");
        }
        
        if (!dbManager.dropTable(tableName)) {
            return QueryResult(false, "Failed to drop table '" + tableName + "'");
        }
        
        return QueryResult(true, "Table '" + tableName + "' dropped successfully");
    }
    
    // Parse ALTER TABLE statement
    QueryResult parseAlterTable(const std::string& query) {
        std::regex alterPattern(R"(ALTER\s+TABLE\s+(\w+)\s+(ADD|DROP)\s+(\w+)(?:\s+(\w+)(?:\s+(.*))?)?)", std::regex_constants::icase);
        std::smatch matches;
        
        if (!std::regex_search(query, matches, alterPattern)) {
            return QueryResult(false, "Invalid ALTER TABLE syntax");
        }
        
        std::string tableName = matches[1].str();
        std::string operation = toUpper(matches[2].str());
        std::string columnName = matches[3].str();
        
        Table* table = dbManager.getTable(tableName);
        if (!table) {
            return QueryResult(false, "Table '" + tableName + "' does not exist");
        }
        
        if (operation == "ADD") {
            if (matches.size() < 5) {
                return QueryResult(false, "Missing data type for ADD operation");
            }
            
            std::string colTypeStr = toUpper(matches[4].str());
            std::string constraintsStr = matches[5].matched ? toUpper(matches[5].str()) : "";
            
            // Convert string to DataType
            Type colType;
            try {
                if (colTypeStr == "INT") colType = Type::INT;
                else if (colTypeStr == "BIGINT") colType = Type::BIGINT;
                else if (colTypeStr == "STRING") colType = Type::STRING;
                else if (colTypeStr == "DATE") colType = Type::DATE;
                else {
                    return QueryResult(false, "Invalid data type: " + colTypeStr);
                }
            } catch (const std::exception& e) {
                return QueryResult(false, "Error parsing data type: " + std::string(e.what()));
            }
            
            // Parse constraints
            bool isPrimaryKey = constraintsStr.find("PRIMARY KEY") != std::string::npos;
            bool isUnique = constraintsStr.find("UNIQUE") != std::string::npos;
            bool isNotNull = constraintsStr.find("NOT NULL") != std::string::npos;
            
            // Add column to table
            if (!table->addColumn(columnName, colType, isPrimaryKey, isUnique, isNotNull)) {
                return QueryResult(false, "Failed to add column: " + columnName);
            }
            
            return QueryResult(true, "Column '" + columnName + "' added to table '" + tableName + "'");
        } else if (operation == "DROP") {
            // Remove column from table
            if (!table->removeColumn(columnName)) {
                return QueryResult(false, "Failed to drop column: " + columnName);
            }
            
            return QueryResult(true, "Column '" + columnName + "' dropped from table '" + tableName + "'");
        }
        
        return QueryResult(false, "Unknown ALTER TABLE operation: " + operation);
    }
    
    // Parse INSERT statement
    QueryResult parseInsert(const std::string& query) {
        std::regex insertPattern(R"(INSERT\s+INTO\s+(\w+)\s*\(([^)]*)\)\s*VALUES\s*\(([^)]*)\))", std::regex_constants::icase);
        std::smatch matches;
        
        if (!std::regex_search(query, matches, insertPattern)) {
            return QueryResult(false, "Invalid INSERT syntax");
        }
        
        std::string tableName = matches[1].str();
        std::string columnsStr = matches[2].str();
        std::string valuesStr = matches[3].str();
        
        Table* table = dbManager.getTable(tableName);
        if (!table) {
            return QueryResult(false, "Table '" + tableName + "' does not exist");
        }
        
        // Parse column names
        auto columnNames = split(columnsStr, ',');
        
        // Parse values
        std::vector<std::string> values;
        std::string currentValue;
        bool inQuotes = false;
        
        for (char c : valuesStr) {
            if (c == '\'' || c == '"') {
                inQuotes = !inQuotes;
                continue;
            }
            
            if (c == ',' && !inQuotes) {
                values.push_back(trim(currentValue));
                currentValue.clear();
            } else {
                currentValue += c;
            }
        }
        
        if (!currentValue.empty()) {
            values.push_back(trim(currentValue));
        }
        
        if (columnNames.size() != values.size()) {
            return QueryResult(false, "Number of columns does not match number of values");
        }
        
        // Prepare column-value pairs
        std::vector<std::pair<std::string, std::string>> colValues;
        for (size_t i = 0; i < columnNames.size(); ++i) {
            colValues.emplace_back(columnNames[i], values[i]);
        }
        
        // Insert record
        if (!table->insertRecord(colValues)) {
            return QueryResult(false, "Failed to insert record");
        }
        
        return QueryResult(true, "Record inserted successfully");
    }
    
    // Parse condition from a WHERE clause
    std::unique_ptr<Condition> parseCondition(const std::string& conditionStr) {
        if (conditionStr.empty()) {
            return nullptr;
        }
        
        // Check for AND/OR operators
        size_t andPos = conditionStr.find(" AND ");
        size_t orPos = conditionStr.find(" OR ");
        
        if (andPos != std::string::npos || orPos != std::string::npos) {
            size_t opPos = (andPos != std::string::npos && (orPos == std::string::npos || andPos < orPos)) ? andPos : orPos;
            std::string logicalOp = conditionStr.substr(opPos + 1, (opPos == andPos) ? 3 : 2);
            
            // Parse the first condition
            std::string firstConditionStr = conditionStr.substr(0, opPos);
            auto firstCondition = parseCondition(firstConditionStr);
            
            // Parse the next condition
            std::string nextConditionStr = conditionStr.substr(opPos + logicalOp.length() + 1);
            auto nextCondition = parseCondition(nextConditionStr);
            
            // Link the conditions
            firstCondition->logicalOp = Condition::stringToLogicalOperator(logicalOp);
            firstCondition->nextCondition = std::move(nextCondition);
            
            return firstCondition;
        }
        
        // Parse single condition: column operator value
        std::regex condPattern(R"((\w+)\s*([=!<>][=]?)\s*([^=!<>]*))", std::regex_constants::icase);
        std::smatch condMatches;
        
        if (!std::regex_search(conditionStr, condMatches, condPattern)) {
            throw std::invalid_argument("Invalid condition: " + conditionStr);
        }
        
        std::string columnName = trim(condMatches[1].str());
        std::string opStr = trim(condMatches[2].str());
        std::string valueStr = trim(condMatches[3].str());
        
        // Remove quotes from value if present
        if ((valueStr.front() == '\'' && valueStr.back() == '\'') ||
            (valueStr.front() == '"' && valueStr.back() == '"')) {
            valueStr = valueStr.substr(1, valueStr.length() - 2);
        }
        
        // Create condition
        auto condition = std::make_unique<Condition>();
        condition->column = columnName;
        condition->op = Condition::stringToOperator(opStr);
        condition->value = valueStr;
        
        return condition;
    }
    
    // Parse SELECT statement
    QueryResult parseSelect(const std::string& query) {
        // Parse: SELECT columns FROM table [WHERE conditions]
        std::regex selectPattern(R"(SELECT\s+(.*?)\s+FROM\s+(\w+)(?:\s+WHERE\s+(.*))?)", std::regex_constants::icase);
        std::smatch matches;
        
        if (!std::regex_search(query, matches, selectPattern)) {
            return QueryResult(false, "Invalid SELECT syntax");
        }
        
        std::string columnsStr = matches[1].str();
        std::string tableName = matches[2].str();
        std::string whereClause = matches[3].matched ? matches[3].str() : "";
        
        Table* table = dbManager.getTable(tableName);
        if (!table) {
            return QueryResult(false, "Table '" + tableName + "' does not exist");
        }
        
        // Parse column names
        auto columnNames = split(columnsStr, ',');
        
        // Parse WHERE clause if present
        std::unique_ptr<Condition> condition;
        if (!whereClause.empty()) {
            try {
                condition = parseCondition(whereClause);
            } catch (const std::exception& e) {
                return QueryResult(false, "Error parsing WHERE clause: " + std::string(e.what()));
            }
        }
        
        try {
            // Execute SELECT
            auto result = table->selectRecords(columnNames, condition.get());
            
            // Create result set
            QueryResult queryResult(true, "Query executed successfully");
            
            // Get column names for result header
            std::vector<size_t> colIndices = table->getColumnIndices(columnNames);
            queryResult.columnNames = table->getColumnNames(colIndices);
            queryResult.rows = result;
            
            return queryResult;
        } catch (const std::exception& e) {
            return QueryResult(false, "Error executing SELECT: " + std::string(e.what()));
        }
    }
    
    // Parse UPDATE statement
    QueryResult parseUpdate(const std::string& query) {
        // Parse: UPDATE table SET col1=val1, col2=val2, ... [WHERE conditions]
        std::regex updatePattern(R"(UPDATE\s+(\w+)\s+SET\s+(.*?)(?:\s+WHERE\s+(.*))?$)", std::regex_constants::icase);
        std::smatch matches;
        
        if (!std::regex_search(query, matches, updatePattern)) {
            return QueryResult(false, "Invalid UPDATE syntax");
        }
        
        std::string tableName = matches[1].str();
        std::string setClause = matches[2].str();
        std::string whereClause = matches[3].matched ? matches[3].str() : "";
        
        Table* table = dbManager.getTable(tableName);
        if (!table) {
            return QueryResult(false, "Table '" + tableName + "' does not exist");
        }
        
        // Parse SET clause
        auto assignments = split(setClause, ',');
        std::vector<std::pair<std::string, std::string>> colValues;
        
        for (const auto& assignment : assignments) {
            size_t eqPos = assignment.find('=');
            if (eqPos == std::string::npos) {
                return QueryResult(false, "Invalid SET clause: " + assignment);
            }
            
            std::string colName = trim(assignment.substr(0, eqPos));
            std::string value = trim(assignment.substr(eqPos + 1));
            
            // Remove quotes from value if present
            if ((value.front() == '\'' && value.back() == '\'') ||
                (value.front() == '"' && value.back() == '"')) {
                value = value.substr(1, value.length() - 2);
            }
            
            colValues.emplace_back(colName, value);
        }
        
        // Parse WHERE clause if present
        std::unique_ptr<Condition> condition;
        if (!whereClause.empty()) {
            try {
                condition = parseCondition(whereClause);
            } catch (const std::exception& e) {
                return QueryResult(false, "Error parsing WHERE clause: " + std::string(e.what()));
            }
        }
        
        try {
            // Execute UPDATE
            int updatedCount = table->updateRecords(colValues, condition.get());
            
            // Create result
            return QueryResult(true, "Updated " + std::to_string(updatedCount) + " record(s)");
        } catch (const std::exception& e) {
            return QueryResult(false, "Error executing UPDATE: " + std::string(e.what()));
        }
    }
    
    // Parse DELETE statement
    QueryResult parseDelete(const std::string& query) {
        // Parse: DELETE FROM table [WHERE conditions]
        std::regex deletePattern(R"(DELETE\s+FROM\s+(\w+)(?:\s+WHERE\s+(.*))?)", std::regex_constants::icase);
        std::smatch matches;
        
        if (!std::regex_search(query, matches, deletePattern)) {
            return QueryResult(false, "Invalid DELETE syntax");
        }
        
        std::string tableName = matches[1].str();
        std::string whereClause = matches[2].matched ? matches[2].str() : "";
        
        Table* table = dbManager.getTable(tableName);
        if (!table) {
            return QueryResult(false, "Table '" + tableName + "' does not exist");
        }
        
        // Parse WHERE clause if present
        std::unique_ptr<Condition> condition;
        if (!whereClause.empty()) {
            try {
                condition = parseCondition(whereClause);
            } catch (const std::exception& e) {
                return QueryResult(false, "Error parsing WHERE clause: " + std::string(e.what()));
            }
        }
        
        try {
            // Execute DELETE
            int deletedCount = table->deleteRecords(condition.get());
            
            // Create result
            return QueryResult(true, "Deleted " + std::to_string(deletedCount) + " record(s)");
        } catch (const std::exception& e) {
            return QueryResult(false, "Error executing DELETE: " + std::string(e.what()));
        }
    }
    
public:
    QueryParser(DatabaseManager& manager) : dbManager(manager) {}
    
    // Parse and execute a query
    QueryResult parseQuery(const std::string& query) {
        std::string trimmedQuery = trim(query);
        
        // Determine query type by the first keyword
        std::istringstream iss(trimmedQuery);
        std::string firstWord;
        iss >> firstWord;
        firstWord = toUpper(firstWord);
        
        try {
            if (firstWord == "CREATE") {
                return parseCreateTable(trimmedQuery);
            } else if (firstWord == "DROP") {
                return parseDropTable(trimmedQuery);
            } else if (firstWord == "ALTER") {
                return parseAlterTable(trimmedQuery);
            } else if (firstWord == "INSERT") {
                return parseInsert(trimmedQuery);
            } else if (firstWord == "SELECT") {
                return parseSelect(trimmedQuery);
            } else if (firstWord == "UPDATE") {
                return parseUpdate(trimmedQuery);
            } else if (firstWord == "DELETE") {
                return parseDelete(trimmedQuery);
            } else {
                return QueryResult(false, "Unknown command: " + firstWord);
            }
        } catch (const std::exception& e) {
            return QueryResult(false, "Error executing query: " + std::string(e.what()));
        }
    }
};

#endif // QUERY_PARSER_H 