#ifndef TABLE_H
#define TABLE_H

#include "Column.h"
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>  // For std::transform
#include <functional>
#include <iostream>
#include <cctype>     // For std::toupper
#include <cstdio>  // For std::remove

using Record = std::vector<std::string>;
using ColumnMap = std::unordered_map<std::string, size_t>;

// Condition for WHERE clauses
struct Condition {
    enum class Operator {
        EQUALS,
        NOT_EQUALS,
        GREATER_THAN,
        LESS_THAN,
        GREATER_EQUAL,
        LESS_EQUAL
    };

    enum class LogicalOperator {
        NONE,
        AND,
        OR
    };

    std::string column;
    Operator op;
    std::string value;
    LogicalOperator logicalOp;
    std::shared_ptr<Condition> nextCondition;

    Condition() : logicalOp(LogicalOperator::NONE), nextCondition(nullptr) {}

    // Convert string to operator
    static Operator stringToOperator(const std::string& opStr) {
        if (opStr == "=") return Operator::EQUALS;
        if (opStr == "!=") return Operator::NOT_EQUALS;
        if (opStr == ">") return Operator::GREATER_THAN;
        if (opStr == "<") return Operator::LESS_THAN;
        if (opStr == ">=") return Operator::GREATER_EQUAL;
        if (opStr == "<=") return Operator::LESS_EQUAL;
        
        throw std::invalid_argument("Invalid operator: " + opStr);
    }

    // Convert string to logical operator
    static LogicalOperator stringToLogicalOperator(const std::string& opStr) {
        // Create a copy of the string to transform
        std::string upperOpStr = opStr;
        
        // Trim whitespace first
        upperOpStr.erase(0, upperOpStr.find_first_not_of(" \t\n\r"));
        upperOpStr.erase(upperOpStr.find_last_not_of(" \t\n\r") + 1);
        
        // Convert to uppercase for case-insensitive comparison
        std::transform(upperOpStr.begin(), upperOpStr.end(), upperOpStr.begin(), 
                      [](unsigned char c){ return std::toupper(c); });
        
        if (upperOpStr == "AND") return LogicalOperator::AND;
        if (upperOpStr == "OR") return LogicalOperator::OR;
        
        return LogicalOperator::NONE;
    }
};

class Table {
private:
    std::string name;
    std::string dataDir;
    std::vector<Column> columns;
    ColumnMap columnMap;  // Maps column names to their index
    std::vector<Record> records;
    std::string dataFilePath;
    std::string metaFilePath;

    // Save table structure and data to file
    bool saveToFile() const {
        // Create data directory if it doesn't exist
        std::ofstream metaFile(metaFilePath);
        if (!metaFile) {
            return false;
        }

        // Save column metadata
        for (const auto& col : columns) {
            metaFile << col.getName() << ","
                    << DataType::typeToString(col.getDataType().getType()) << ","
                    << (col.isPrimaryKey() ? "PK," : ",")
                    << (col.isUnique() ? "UQ," : ",")
                    << (col.isNotNull() ? "NN" : "") << std::endl;
        }
        metaFile.close();

        // Save data records
        std::ofstream dataFile(dataFilePath);
        if (!dataFile) {
            return false;
        }

        for (const auto& record : records) {
            for (size_t i = 0; i < record.size(); ++i) {
                dataFile << record[i];
                if (i < record.size() - 1) {
                    dataFile << ",";
                }
            }
            dataFile << std::endl;
        }
        dataFile.close();

        return true;
    }

    // Load table structure and data from file
    bool loadFromFile() {
        std::ifstream metaFile(metaFilePath);
        if (!metaFile) {
            return false;
        }

        // Clear existing column data
        columns.clear();
        columnMap.clear();

        // Load column metadata
        std::string line;
        size_t colIndex = 0;
        while (std::getline(metaFile, line)) {
            std::istringstream iss(line);
            std::string colName, typeStr, pkStr, uqStr, nnStr;
            
            std::getline(iss, colName, ',');
            std::getline(iss, typeStr, ',');
            std::getline(iss, pkStr, ',');
            std::getline(iss, uqStr, ',');
            std::getline(iss, nnStr);

            Type type = DataType::stringToType(typeStr);
            bool isPK = (pkStr == "PK");
            bool isUQ = (uqStr == "UQ");
            bool isNN = (nnStr == "NN");

            columns.emplace_back(colName, type, isPK, isUQ, isNN);
            columnMap[colName] = colIndex++;
        }
        metaFile.close();

        // Load data records
        std::ifstream dataFile(dataFilePath);
        if (!dataFile) {
            return false;
        }

        records.clear();
        while (std::getline(dataFile, line)) {
            std::istringstream iss(line);
            std::string value;
            Record record;

            for (size_t i = 0; i < columns.size(); ++i) {
                if (i < columns.size() - 1) {
                    std::getline(iss, value, ',');
                } else {
                    std::getline(iss, value);
                }
                record.push_back(value);
            }

            records.push_back(record);
        }
        dataFile.close();

        // Rebuild unique value sets
        for (size_t colIdx = 0; colIdx < columns.size(); ++colIdx) {
            if (columns[colIdx].isUnique()) {
                columns[colIdx].clearUniqueValues();
                for (const auto& record : records) {
                    columns[colIdx].checkAndAddUniqueValue(record[colIdx]);
                }
            }
        }

        return true;
    }

    // Evaluate a condition against a record
    bool evaluateCondition(const Record& record, const Condition& condition) const {
        // Find the column index
        auto colIter = columnMap.find(condition.column);
        if (colIter == columnMap.end()) {
            throw std::invalid_argument("Unknown column name: " + condition.column);
        }
        
        size_t colIndex = colIter->second;
        const std::string& recordValue = record[colIndex];
        const std::string& conditionValue = condition.value;
        
        // Get column data type
        Type colType = columns[colIndex].getDataType().getType();
        
        bool result = false;
        
        // Evaluate based on type
        switch (colType) {
            case Type::INT: {
                // Handle empty values
                if (recordValue.empty() || conditionValue.empty()) {
                    return recordValue == conditionValue;
                }
                
                int recVal = std::stoi(recordValue);
                int condVal = std::stoi(conditionValue);
                
                switch (condition.op) {
                    case Condition::Operator::EQUALS: result = (recVal == condVal); break;
                    case Condition::Operator::NOT_EQUALS: result = (recVal != condVal); break;
                    case Condition::Operator::GREATER_THAN: result = (recVal > condVal); break;
                    case Condition::Operator::LESS_THAN: result = (recVal < condVal); break;
                    case Condition::Operator::GREATER_EQUAL: result = (recVal >= condVal); break;
                    case Condition::Operator::LESS_EQUAL: result = (recVal <= condVal); break;
                }
                break;
            }
            case Type::BIGINT: {
                // Handle empty values
                if (recordValue.empty() || conditionValue.empty()) {
                    return recordValue == conditionValue;
                }
                
                long long recVal = std::stoll(recordValue);
                long long condVal = std::stoll(conditionValue);
                
                switch (condition.op) {
                    case Condition::Operator::EQUALS: result = (recVal == condVal); break;
                    case Condition::Operator::NOT_EQUALS: result = (recVal != condVal); break;
                    case Condition::Operator::GREATER_THAN: result = (recVal > condVal); break;
                    case Condition::Operator::LESS_THAN: result = (recVal < condVal); break;
                    case Condition::Operator::GREATER_EQUAL: result = (recVal >= condVal); break;
                    case Condition::Operator::LESS_EQUAL: result = (recVal <= condVal); break;
                }
                break;
            }
            case Type::STRING: {
                // Make string comparisons case-insensitive
                std::string recordValueUpper = recordValue;
                std::string conditionValueUpper = conditionValue;
                
                // Convert both strings to uppercase for case-insensitive comparison
                std::transform(recordValueUpper.begin(), recordValueUpper.end(), recordValueUpper.begin(),
                              [](unsigned char c) { return std::toupper(c); });
                std::transform(conditionValueUpper.begin(), conditionValueUpper.end(), conditionValueUpper.begin(),
                              [](unsigned char c) { return std::toupper(c); });
                
                switch (condition.op) {
                    case Condition::Operator::EQUALS: result = (recordValueUpper == conditionValueUpper); break;
                    case Condition::Operator::NOT_EQUALS: result = (recordValueUpper != conditionValueUpper); break;
                    case Condition::Operator::GREATER_THAN: result = (recordValueUpper > conditionValueUpper); break;
                    case Condition::Operator::LESS_THAN: result = (recordValueUpper < conditionValueUpper); break;
                    case Condition::Operator::GREATER_EQUAL: result = (recordValueUpper >= conditionValueUpper); break;
                    case Condition::Operator::LESS_EQUAL: result = (recordValueUpper <= conditionValueUpper); break;
                }
                break;
            }
            case Type::DATE: {
                // For date, we can compare lexicographically as they're in YYYY-MM-DD format
                switch (condition.op) {
                    case Condition::Operator::EQUALS: result = (recordValue == conditionValue); break;
                    case Condition::Operator::NOT_EQUALS: result = (recordValue != conditionValue); break;
                    case Condition::Operator::GREATER_THAN: result = (recordValue > conditionValue); break;
                    case Condition::Operator::LESS_THAN: result = (recordValue < conditionValue); break;
                    case Condition::Operator::GREATER_EQUAL: result = (recordValue >= conditionValue); break;
                    case Condition::Operator::LESS_EQUAL: result = (recordValue <= conditionValue); break;
                }
                break;
            }
        }
        
        // If there's a next condition, evaluate it recursively
        if (condition.nextCondition) {
            bool nextResult = evaluateCondition(record, *condition.nextCondition);
            
            // Combine with logical operator
            switch (condition.logicalOp) {
                case Condition::LogicalOperator::AND:
                    return result && nextResult;
                case Condition::LogicalOperator::OR:
                    return result || nextResult;
                default:
                    return result;
            }
        }
        
        return result;
    }

public:
    Table(const std::string& tableName, const std::string& dataDirectory = "data") 
        : name(tableName), dataDir(dataDirectory) {
        metaFilePath = dataDir + "/" + name + ".meta";
        dataFilePath = dataDir + "/" + name + ".data";
        // Try to load existing table data
        if (!loadFromFile()) {
            // New table - will be created when columns are added
        }
    }

    // Copy constructor
    Table(const Table& other) 
        : name(other.name), dataDir(other.dataDir), columns(other.columns),
          records(other.records), dataFilePath(other.dataFilePath), metaFilePath(other.metaFilePath) {
        // Rebuild the column map
        for (size_t i = 0; i < columns.size(); ++i) {
            columnMap[columns[i].getName()] = i;
        }
    }

    const std::string& getName() const {
        return name;
    }

    // Add a column to the table
    bool addColumn(const std::string& columnName, Type type, 
                   bool isPrimaryKey = false, bool isUnique = false, bool isNotNull = false) {
        // Check if column already exists
        if (columnMap.find(columnName) != columnMap.end()) {
            return false;
        }

        // Check if trying to add a primary key when one already exists
        if (isPrimaryKey) {
            for (const auto& col : columns) {
                if (col.isPrimaryKey()) {
                    return false; // Can't have multiple primary keys
                }
            }
        }

        // Add the new column
        columnMap[columnName] = columns.size();
        columns.emplace_back(columnName, type, isPrimaryKey, isUnique, isNotNull);

        // Add empty values for this column to all existing records
        for (auto& record : records) {
            record.push_back("");
        }

        return saveToFile();
    }

    // Add a column object to the table
    bool addColumn(const Column& column) {
        // Check if column already exists
        if (columnMap.find(column.getName()) != columnMap.end()) {
            return false;
        }

        // Check if trying to add a primary key when one already exists
        if (column.isPrimaryKey()) {
            for (const auto& col : columns) {
                if (col.isPrimaryKey()) {
                    return false; // Can't have multiple primary keys
                }
            }
        }

        // Add the new column
        columnMap[column.getName()] = columns.size();
        columns.push_back(column);

        // Add empty values for this column to all existing records
        for (auto& record : records) {
            record.push_back("");
        }

        return saveToFile();
    }

    // Get all columns
    const std::vector<Column>& getColumns() const {
        return columns;
    }

    // Remove a column from the table
    bool removeColumn(const std::string& columnName) {
        auto colIter = columnMap.find(columnName);
        if (colIter == columnMap.end()) {
            return false;
        }

        size_t colIndex = colIter->second;

        // Can't remove primary key column
        if (columns[colIndex].isPrimaryKey()) {
            return false;
        }

        // Remove the column and update records
        columns.erase(columns.begin() + colIndex);
        columnMap.erase(columnName);

        // Update column indices in map
        columnMap.clear();
        for (size_t i = 0; i < columns.size(); ++i) {
            columnMap[columns[i].getName()] = i;
        }

        // Remove column values from all records
        for (auto& record : records) {
            record.erase(record.begin() + colIndex);
        }

        return saveToFile();
    }

    // Insert a new record
    bool insertRecord(const std::vector<std::pair<std::string, std::string>>& colValues) {
        if (columns.empty()) {
            return false; // No columns defined
        }

        // Initialize a record with empty values
        Record newRecord(columns.size(), "");

        // Fill in provided values
        for (const auto& [colName, value] : colValues) {
            auto colIter = columnMap.find(colName);
            if (colIter == columnMap.end()) {
                return false; // Column doesn't exist
            }

            size_t colIndex = colIter->second;
            
            // Validate the value against column constraints
            if (!columns[colIndex].validateValue(value)) {
                return false; // Value violates constraints
            }
            
            newRecord[colIndex] = value;
        }

        // Check constraints for all columns
        for (size_t i = 0; i < columns.size(); ++i) {
            // Check NOT NULL constraint
            if (columns[i].isNotNull() && newRecord[i].empty()) {
                return false; // NOT NULL column has no value
            }
            
            // Check UNIQUE constraint
            if (!columns[i].checkAndAddUniqueValue(newRecord[i])) {
                // Roll back any unique values added
                for (size_t j = 0; j < i; ++j) {
                    if (columns[j].isUnique()) {
                        columns[j].removeUniqueValue(newRecord[j]);
                    }
                }
                return false; // Unique constraint violated
            }
        }

        records.push_back(newRecord);
        return saveToFile();
    }
    
    // Select records based on conditions and column selection
    std::vector<std::vector<std::string>> selectRecords(
        const std::vector<std::string>& selectedColumns,
        const Condition* condition = nullptr) const {
        
        std::vector<std::vector<std::string>> result;
        
        // Process column selection
        std::vector<size_t> colIndices;
        
        // Handle SELECT * case
        if (selectedColumns.size() == 1 && selectedColumns[0] == "*") {
            for (size_t i = 0; i < columns.size(); ++i) {
                colIndices.push_back(i);
            }
        } else {
            // Handle specific columns
            for (const auto& colName : selectedColumns) {
                auto colIter = columnMap.find(colName);
                if (colIter == columnMap.end()) {
                    throw std::invalid_argument("Unknown column name: " + colName);
                }
                colIndices.push_back(colIter->second);
            }
        }
        
        // Process records
        for (const auto& record : records) {
            // Skip if condition exists and is not met
            if (condition && !evaluateCondition(record, *condition)) {
                continue;
            }
            
            // Extract selected columns from record
            std::vector<std::string> selectedRecord;
            for (size_t colIdx : colIndices) {
                selectedRecord.push_back(record[colIdx]);
            }
            
            result.push_back(selectedRecord);
        }
        
        return result;
    }
    
    // Update records based on condition
    int updateRecords(
        const std::vector<std::pair<std::string, std::string>>& colValues,
        const Condition* condition = nullptr) {
        
        int updatedCount = 0;
        
        // Prepare column indices and values for update
        std::vector<std::pair<size_t, std::string>> updates;
        for (const auto& [colName, value] : colValues) {
            auto colIter = columnMap.find(colName);
            if (colIter == columnMap.end()) {
                throw std::invalid_argument("Unknown column name: " + colName);
            }
            
            size_t colIndex = colIter->second;
            
            // Validate the value against column constraints
            if (!columns[colIndex].validateValue(value, true)) {
                throw std::invalid_argument("Invalid value for column: " + colName);
            }
            
            updates.emplace_back(colIndex, value);
        }
        
        // Process records
        for (auto& record : records) {
            // Skip if condition exists and is not met
            if (condition && !evaluateCondition(record, *condition)) {
                continue;
            }
            
            // Apply updates
            for (const auto& [colIndex, value] : updates) {
                // Skip update if value is empty (for partial updates)
                if (value.empty()) {
                    continue;
                }
                
                // Remove old unique value
                if (columns[colIndex].isUnique()) {
                    columns[colIndex].removeUniqueValue(record[colIndex]);
                }
                
                // Set new value
                record[colIndex] = value;
                
                // Add new unique value
                if (columns[colIndex].isUnique() && 
                    !columns[colIndex].checkAndAddUniqueValue(value)) {
                    // Roll back
                    throw std::runtime_error("Unique constraint violated");
                }
            }
            
            updatedCount++;
        }
        
        if (updatedCount > 0) {
            saveToFile();
        }
        
        return updatedCount;
    }
    
    // Delete records based on condition
    int deleteRecords(const Condition* condition = nullptr) {
        size_t initialSize = records.size();
        
        // Find records to delete
        std::vector<Record> newRecords;
        
        for (const auto& record : records) {
            // Skip record (don't include in newRecords) if condition is met
            if (!condition || evaluateCondition(record, *condition)) {
                // Remove unique values for this record
                for (size_t i = 0; i < columns.size(); ++i) {
                    if (columns[i].isUnique()) {
                        columns[i].removeUniqueValue(record[i]);
                    }
                }
            } else {
                newRecords.push_back(record);
            }
        }
        
        int deletedCount = initialSize - newRecords.size();
        
        // Replace records with filtered ones
        records = std::move(newRecords);
        
        if (deletedCount > 0) {
            saveToFile();
        }
        
        return deletedCount;
    }
    
    // Drop the table (delete all data)
    bool drop() {
        bool success = true;
        
        // Try to remove the data file
        if (std::remove(dataFilePath.c_str()) != 0) {
            // Check if the file exists first
            std::ifstream checkDataFile(dataFilePath);
            if (checkDataFile) {
                // File exists but couldn't be deleted
                checkDataFile.close();
                success = false;
            }
        }
        
        // Try to remove the meta file
        if (std::remove(metaFilePath.c_str()) != 0) {
            // Check if the file exists first
            std::ifstream checkMetaFile(metaFilePath);
            if (checkMetaFile) {
                // File exists but couldn't be deleted
                checkMetaFile.close();
                success = false;
            }
        }
        
        // Clear in-memory data even if file deletion failed
        columns.clear();
        columnMap.clear();
        records.clear();
        
        return success;
    }
    
    // Get column names (for query result headers)
    std::vector<std::string> getColumnNames() const {
        std::vector<std::string> names;
        names.reserve(columns.size());
        
        for (const auto& col : columns) {
            names.push_back(col.getName());
        }
        
        return names;
    }
    
    // Get column names for specific indices
    std::vector<std::string> getColumnNames(const std::vector<size_t>& indices) const {
        std::vector<std::string> names;
        names.reserve(indices.size());
        
        for (size_t idx : indices) {
            if (idx < columns.size()) {
                names.push_back(columns[idx].getName());
            }
        }
        
        return names;
    }
    
    // Get column indices for names
    std::vector<size_t> getColumnIndices(const std::vector<std::string>& colNames) const {
        std::vector<size_t> indices;
        
        // Handle * case
        if (colNames.size() == 1 && colNames[0] == "*") {
            for (size_t i = 0; i < columns.size(); ++i) {
                indices.push_back(i);
            }
            return indices;
        }
        
        // Handle specific columns
        for (const auto& name : colNames) {
            auto it = columnMap.find(name);
            if (it == columnMap.end()) {
                throw std::invalid_argument("Unknown column: " + name);
            }
            indices.push_back(it->second);
        }
        
        return indices;
    }
    
    // Check if table is empty
    bool isEmpty() const {
        return records.empty();
    }
    
    // Get record count
    size_t getRecordCount() const {
        return records.size();
    }
};

#endif // TABLE_H 