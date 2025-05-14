#ifndef COLUMN_H
#define COLUMN_H

#include "DataType.h"
#include <string>
#include <unordered_set>

class Column {
private:
    std::string name;
    DataType dataType;
    bool isPrimaryKeyFlag;
    bool isUniqueFlag;
    bool isNotNullFlag;
    std::unordered_set<std::string> uniqueValues; // For checking unique constraint

public:
    Column(const std::string& colName, Type type, bool primaryKey = false, bool unique = false, bool notNull = false)
        : name(colName), dataType(type), isPrimaryKeyFlag(primaryKey), 
          isUniqueFlag(unique || primaryKey), isNotNullFlag(notNull || primaryKey) {
    }

    Column(const std::string& colName, const DataType& type, bool primaryKey = false, bool unique = false, bool notNull = false)
        : name(colName), dataType(type), isPrimaryKeyFlag(primaryKey), 
          isUniqueFlag(unique || primaryKey), isNotNullFlag(notNull || primaryKey) {
    }

    const std::string& getName() const {
        return name;
    }

    const DataType& getDataType() const {
        return dataType;
    }

    bool isPrimaryKey() const {
        return isPrimaryKeyFlag;
    }

    bool isUnique() const {
        return isUniqueFlag;
    }

    bool isNotNull() const {
        return isNotNullFlag;
    }

    // Check and add a value to the unique value set
    bool checkAndAddUniqueValue(const std::string& value) {
        // If this column is not unique, always return true
        if (!isUniqueFlag) {
            return true;
        }
        
        // Empty values are allowed for non-NOT NULL columns
        if (value.empty() && !isNotNullFlag) {
            return true;
        }
        
        // Check if the value already exists in the unique set
        if (uniqueValues.find(value) != uniqueValues.end()) {
            return false; // Value already exists, violates unique constraint
        }
        
        // Add the value to the set
        uniqueValues.insert(value);
        return true;
    }

    // Remove a value from the unique value set
    void removeUniqueValue(const std::string& value) {
        if (isUniqueFlag && !value.empty()) {
            uniqueValues.erase(value);
        }
    }

    // Clear all unique values
    void clearUniqueValues() {
        uniqueValues.clear();
    }

    // Validate a value according to column constraints
    bool validateValue(const std::string& value, bool isUpdate = false) const {
        // Check NOT NULL constraint for non-update operations
        if (!isUpdate && isNotNullFlag && value.empty()) {
            return false; // NOT NULL constraint violated
        }
        
        // If empty value is being set during update, it's allowed
        if (isUpdate && value.empty()) {
            return true;
        }
        
        // Validate type
        return dataType.validateValue(value);
    }
};

#endif // COLUMN_H 