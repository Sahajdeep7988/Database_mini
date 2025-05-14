#ifndef VALUE_H
#define VALUE_H

#include <string>
#include "../DataType.h"

namespace sqldb {

/**
 * @struct Value
 * @brief Represents a value in the database
 */
struct Value {
    Type type;
    union {
        int intValue;
        long long bigintValue;
        double doubleValue;
        bool boolValue;
    };
    std::string stringValue; // Not in union because it has a non-trivial destructor

    // Default constructor
    Value() : type(Type::INT), intValue(0) {}

    // Constructor for INT
    static Value createInt(int val) {
        Value v;
        v.type = Type::INT;
        v.intValue = val;
        return v;
    }

    // Constructor for BIGINT
    static Value createBigInt(long long val) {
        Value v;
        v.type = Type::BIGINT;
        v.bigintValue = val;
        return v;
    }

    // Constructor for STRING
    static Value createString(const std::string& val) {
        Value v;
        v.type = Type::STRING;
        v.stringValue = val;
        return v;
    }

    // Constructor for DATE (stored as string)
    static Value createDate(const std::string& val) {
        Value v;
        v.type = Type::DATE;
        v.stringValue = val;
        return v;
    }

    // Convert to string representation
    std::string toString() const {
        switch (type) {
            case Type::INT:
                return std::to_string(intValue);
            case Type::BIGINT:
                return std::to_string(bigintValue);
            case Type::STRING:
            case Type::DATE:
                return stringValue;
            default:
                return "";
        }
    }

    // Create value from string and type
    static Value fromString(const std::string& str, Type type) {
        Value v;
        v.type = type;
        
        switch (type) {
            case Type::INT:
                v.intValue = std::stoi(str);
                break;
            case Type::BIGINT:
                v.bigintValue = std::stoll(str);
                break;
            case Type::STRING:
            case Type::DATE:
                v.stringValue = str;
                break;
        }
        
        return v;
    }
};

} // namespace sqldb

#endif // VALUE_H 