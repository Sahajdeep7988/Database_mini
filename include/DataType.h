#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include <string>
#include <regex>
#include <stdexcept>
#include <iostream>

enum class Type {
    INT,
    BIGINT,
    STRING,
    DATE
};

class DataType {
private:
    Type type;

public:
    DataType(Type t = Type::STRING) : type(t) {}

    Type getType() const {
        return type;
    }

    bool validateValue(const std::string& value) const {
        if (value.empty()) {
            return true;  // Empty values are handled by the NOT NULL constraint
        }

        switch (type) {
            case Type::INT: {
                std::regex pattern(R"(^-?\d+$)");
                if (!std::regex_match(value, pattern)) {
                    return false;
                }
                
                try {
                    int val = std::stoi(value);
                    return true;
                } catch (const std::exception&) {
                    return false;  // Out of range
                }
            }
            case Type::BIGINT: {
                std::regex pattern(R"(^-?\d+$)");
                if (!std::regex_match(value, pattern)) {
                    return false;
                }
                
                try {
                    long long val = std::stoll(value);
                    return true;
                } catch (const std::exception&) {
                    return false;  // Out of range
                }
            }
            case Type::STRING:
                return true;  // All strings are valid
            case Type::DATE: {
                // Format: YYYY-MM-DD
                std::regex pattern(R"(^\d{4}-\d{2}-\d{2}$)");
                if (!std::regex_match(value, pattern)) {
                    return false;
                }
                
                // Basic validation of year, month, day
                int year, month, day;
                sscanf(value.c_str(), "%d-%d-%d", &year, &month, &day);
                
                if (year < 1 || month < 1 || month > 12 || day < 1 || day > 31) {
                    return false;
                }
                
                // Check days in month
                if (month == 2) {
                    bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
                    if (day > (isLeapYear ? 29 : 28)) {
                        return false;
                    }
                } else if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) {
                    return false;
                }
                
                return true;
            }
            default:
                return false;
        }
    }

    static Type stringToType(const std::string& typeStr) {
        if (typeStr == "INT") return Type::INT;
        if (typeStr == "BIGINT") return Type::BIGINT;
        if (typeStr == "STRING") return Type::STRING;
        if (typeStr == "DATE") return Type::DATE;
        
        throw std::invalid_argument("Unknown data type: " + typeStr);
    }

    static std::string typeToString(Type type) {
        switch (type) {
            case Type::INT: return "INT";
            case Type::BIGINT: return "BIGINT";
            case Type::STRING: return "STRING";
            case Type::DATE: return "DATE";
            default: return "UNKNOWN";
        }
    }
};

#endif // DATA_TYPE_H 