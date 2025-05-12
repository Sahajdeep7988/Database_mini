#ifndef AGGREGATE_FUNCTION_H
#define AGGREGATE_FUNCTION_H

#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <limits>

class AggregateFunction {
private:
    // Convert a string to int (with validation)
    static int toInt(const std::string& str) {
        if (str.empty()) {
            throw std::invalid_argument("Empty string cannot be converted to int");
        }
        
        try {
            return std::stoi(str);
        } catch (const std::exception& e) {
            throw std::invalid_argument("'" + str + "' is not a valid integer");
        }
    }

public:
    // Count the number of non-empty values
    static int count(const std::vector<std::string>& values) {
        return std::count_if(values.begin(), values.end(), 
            [](const std::string& str) { return !str.empty(); });
    }
    
    // Sum of numeric values
    static int sum(const std::vector<std::string>& values) {
        int result = 0;
        
        for (const auto& value : values) {
            if (!value.empty()) {
                result += toInt(value);
            }
        }
        
        return result;
    }
    
    // Average of numeric values
    static double avg(const std::vector<std::string>& values) {
        int total = 0;
        int count = 0;
        
        for (const auto& value : values) {
            if (!value.empty()) {
                total += toInt(value);
                count++;
            }
        }
        
        if (count == 0) {
            return 0.0;
        }
        
        return static_cast<double>(total) / count;
    }
    
    // Minimum value
    static std::string min(const std::vector<std::string>& values) {
        if (values.empty() || std::all_of(values.begin(), values.end(), 
                                         [](const std::string& str) { return str.empty(); })) {
            return "";
        }
        
        // Find first non-empty value
        auto firstNonEmpty = std::find_if(values.begin(), values.end(),
                                        [](const std::string& str) { return !str.empty(); });
        
        if (firstNonEmpty == values.end()) {
            return "";
        }
        
        // Try numeric comparison first
        try {
            int minVal = toInt(*firstNonEmpty);
            
            for (const auto& value : values) {
                if (!value.empty()) {
                    int val = toInt(value);
                    if (val < minVal) {
                        minVal = val;
                    }
                }
            }
            
            return std::to_string(minVal);
        } catch (const std::exception&) {
            // If numeric conversion fails, do string comparison
            std::string minVal = *firstNonEmpty;
            
            for (const auto& value : values) {
                if (!value.empty() && value < minVal) {
                    minVal = value;
                }
            }
            
            return minVal;
        }
    }
    
    // Maximum value
    static std::string max(const std::vector<std::string>& values) {
        if (values.empty() || std::all_of(values.begin(), values.end(), 
                                         [](const std::string& str) { return str.empty(); })) {
            return "";
        }
        
        // Find first non-empty value
        auto firstNonEmpty = std::find_if(values.begin(), values.end(),
                                        [](const std::string& str) { return !str.empty(); });
        
        if (firstNonEmpty == values.end()) {
            return "";
        }
        
        // Try numeric comparison first
        try {
            int maxVal = toInt(*firstNonEmpty);
            
            for (const auto& value : values) {
                if (!value.empty()) {
                    int val = toInt(value);
                    if (val > maxVal) {
                        maxVal = val;
                    }
                }
            }
            
            return std::to_string(maxVal);
        } catch (const std::exception&) {
            // If numeric conversion fails, do string comparison
            std::string maxVal = *firstNonEmpty;
            
            for (const auto& value : values) {
                if (!value.empty() && value > maxVal) {
                    maxVal = value;
                }
            }
            
            return maxVal;
        }
    }
};

#endif // AGGREGATE_FUNCTION_H 