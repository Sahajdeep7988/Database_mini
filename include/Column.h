#ifndef COLUMN_H
#define COLUMN_H

#include "DataType.h"
#include <string>
#include <unordered_set>

class Column {
private:
    std::string name;
    DataType dataType;
    bool primaryKey;
    bool unique;
    bool notNull;
    std::unordered_set<std::string> uniqueValues;

public:
    Column(const std::string& colName, Type type, 
           bool isPrimaryKey = false, bool isUnique = false, bool isNotNull = false)
        : name(colName), 
          dataType(type), 
          primaryKey(isPrimaryKey), 
          unique(isUnique || isPrimaryKey), // Primary key implies unique
          notNull(isNotNull || isPrimaryKey) {} // Primary key implies not null
    
    const std::string& getName() const {
        return name;
    }
    
    DataType& getDataType() {
        return dataType;
    }
    
    const DataType& getDataType() const {
        return dataType;
    }
    
    bool isPrimaryKey() const {
        return primaryKey;
    }
    
    bool isUnique() const {
        return unique;
    }
    
    bool isNotNull() const {
        return notNull;
    }
    
    // Check if a value can be inserted (validates constraints)
    bool validateValue(const std::string& value, bool isUpdate = false) const {
        // If it's an update and the value is empty, we might be skipping this column
        if (isUpdate && value.empty()) {
            return true;
        }
        
        // Check NOT NULL constraint
        if (value.empty() && notNull) {
            return false;
        }
        
        // Validate the data type
        if (!value.empty() && !dataType.validate(value)) {
            return false;
        }
        
        return true;
    }
    
    // Check uniqueness and add value to unique set if valid
    bool checkAndAddUniqueValue(const std::string& value) {
        if (!unique || value.empty()) {
            return true;
        }
        
        if (uniqueValues.find(value) != uniqueValues.end()) {
            return false; // Value already exists, violates uniqueness
        }
        
        uniqueValues.insert(value);
        return true;
    }
    
    // Remove value from unique set (for DELETE operations)
    void removeUniqueValue(const std::string& value) {
        if (unique && !value.empty()) {
            uniqueValues.erase(value);
        }
    }
    
    // Clear unique values (when dropping a table)
    void clearUniqueValues() {
        uniqueValues.clear();
    }
};

#endif // COLUMN_H 