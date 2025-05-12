#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include <string>
#include <stdexcept>
#include <regex>
#include <sstream>
#include <iomanip>

enum class Type {
    INT,
    BIGINT,
    STRING,
    DATE
};

class DataType {
private:
    Type type;
    std::string value;
    
    bool validateInt(const std::string& val) const {
        try {
            std::stoi(val);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    bool validateBigInt(const std::string& val) const {
        try {
            std::stoll(val);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    bool validateDate(const std::string& val) const {
        // Basic date format validation (YYYY-MM-DD)
        std::regex datePattern(R"(\d{4}-\d{2}-\d{2})");
        if (!std::regex_match(val, datePattern)) {
            return false;
        }
        
        // Parse the date components
        int year, month, day;
        char delimiter;
        std::istringstream ss(val);
        ss >> year >> delimiter >> month >> delimiter >> day;
        
        // Basic validation for month and day ranges
        if (month < 1 || month > 12) return false;
        
        // Check days in month (simplified)
        const int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int maxDays = daysInMonth[month];
        
        // Adjust for February in leap years
        if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
            maxDays = 29;
        }
        
        return (day >= 1 && day <= maxDays);
    }

public:
    DataType(Type t, const std::string& val = "") : type(t), value(val) {
        if (!val.empty() && !validate(val)) {
            throw std::invalid_argument("Invalid value for the specified data type");
        }
    }
    
    bool validate(const std::string& val) const {
        switch (type) {
            case Type::INT:
                return validateInt(val);
            case Type::BIGINT:
                return validateBigInt(val);
            case Type::STRING:
                return true; // All strings are valid
            case Type::DATE:
                return validateDate(val);
            default:
                return false;
        }
    }
    
    void setValue(const std::string& val) {
        if (!validate(val)) {
            throw std::invalid_argument("Invalid value for the specified data type");
        }
        value = val;
    }
    
    std::string getValue() const {
        return value;
    }
    
    Type getType() const {
        return type;
    }
    
    static std::string typeToString(Type t) {
        switch (t) {
            case Type::INT: return "INT";
            case Type::BIGINT: return "BIGINT";
            case Type::STRING: return "STRING";
            case Type::DATE: return "DATE";
            default: return "UNKNOWN";
        }
    }
    
    static Type stringToType(const std::string& typeStr) {
        if (typeStr == "INT") return Type::INT;
        if (typeStr == "BIGINT") return Type::BIGINT;
        if (typeStr == "STRING") return Type::STRING;
        if (typeStr == "DATE") return Type::DATE;
        throw std::invalid_argument("Unknown data type: " + typeStr);
    }
};

#endif // DATA_TYPE_H 