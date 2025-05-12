#ifndef STRING_FUNCTION_H
#define STRING_FUNCTION_H

#include <string>
#include <algorithm>
#include <cctype>
#include <stdexcept>

class StringFunction {
public:
    // Convert a string to uppercase
    static std::string upper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c) { return std::toupper(c); });
        return result;
    }
    
    // Convert a string to lowercase
    static std::string lower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        return result;
    }
    
    // Get the length of a string
    static size_t length(const std::string& str) {
        return str.length();
    }
    
    // Get a substring
    static std::string substring(const std::string& str, size_t start, size_t length = std::string::npos) {
        if (start >= str.length()) {
            throw std::out_of_range("Start index out of range");
        }
        
        return str.substr(start, length);
    }
};

#endif // STRING_FUNCTION_H 