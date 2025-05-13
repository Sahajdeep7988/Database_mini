# SQL-like Database System - Comprehensive Guide

## Introduction

This document provides a detailed guide to the SQL-like database system implemented in C++. This project demonstrates a wide range of C++ programming concepts and practical applications, from basic object-oriented principles to advanced features like regular expressions and cross-platform compatibility.

The system implements a simple SQL-like database that:
- Supports multiple databases and tables
- Handles basic SQL commands (CREATE, DROP, ALTER, SELECT, INSERT, UPDATE, DELETE)
- Provides data persistence through file storage
- Implements a command-line interface
- Includes data type validation and constraint enforcement
- Works on both Windows and Unix-based systems

This guide will help you understand how the codebase works, what C++ concepts are used, and how you could implement a similar system yourself.

## Table of Contents

1. [System Architecture](#system-architecture)
2. [Core C++ Concepts Used](#core-cpp-concepts)
3. [Database Management](#database-management)
4. [Table Operations](#table-operations)
5. [SQL Command Parsing](#sql-command-parsing)
6. [Data Types and Validation](#data-types-and-validation)
7. [File I/O and Persistence](#file-io-and-persistence)
8. [Command-Line Interface](#command-line-interface)
9. [Cross-Platform Compatibility](#cross-platform-compatibility)
10. [Implementation Details](#implementation-details)
11. [Building and Running the System](#building-and-running)

## System Architecture

The system is designed with a hierarchical architecture that mimics real-world database management systems. It consists of several key components:

### Overall Structure

The project structure follows a logical hierarchy:

```
├── CMakeLists.txt          # Build configuration
├── README.md               # Project documentation
├── src/                    # Source files
│   └── main.cpp            # Main application entry point
├── include/                # Header files
│   ├── AggregateFunction.h # Aggregate functions (COUNT, SUM, etc.)
│   ├── Column.h            # Column class definition
│   ├── DataType.h          # Data type definitions
│   ├── DatabaseManager.h   # Database manager
│   ├── DatabaseSystem.h    # Top-level database system
│   ├── QueryParser.h       # SQL query parser
│   ├── StringFunction.h    # String functions (UPPER, LOWER, etc.)
│   └── Table.h             # Table class definition
└── databases/              # Data storage directory (created at runtime)
```

### Component Hierarchy

1. **DatabaseSystem**: The top-level component that manages multiple databases.
   - Responsible for creating, dropping, and switching between databases
   - Maintains a collection of DatabaseManager instances
   - Handles directory operations for database storage

2. **DatabaseManager**: Manages a single database and its tables.
   - Responsible for creating, dropping, and accessing tables
   - Maintains a collection of Table instances
   - Manages database-specific data directory

3. **Table**: Manages a database table and its data.
   - Responsible for storing and retrieving records
   - Manages columns, data types, and constraints
   - Implements data operations (SELECT, INSERT, UPDATE, DELETE)
   - Handles file I/O for table persistence

4. **Column**: Represents a table column.
   - Manages column metadata (name, data type, constraints)
   - Validates data against constraints (PRIMARY KEY, UNIQUE, NOT NULL)
   - Enforces uniqueness constraints

5. **DataType**: Represents and validates data types.
   - Implements data type conversion and validation
   - Supports INT, BIGINT, STRING, and DATE types

6. **QueryParser**: Parses and executes SQL commands.
   - Uses regular expressions to parse query syntax
   - Dispatches queries to appropriate handlers
   - Returns query results to the caller

### Data Flow

1. User enters a SQL command in the command-line interface
2. The main application passes the command to the DatabaseSystem
3. If it's a database-level command (CREATE DATABASE, USE, etc.), DatabaseSystem handles it
4. If it's a table command, it's passed to the current DatabaseManager
5. The DatabaseManager passes the command to the QueryParser
6. The QueryParser parses the command and performs the requested operation
7. Results are returned back up the chain to the user

### Storage Structure

- Each database is represented as a directory
- Each database has a "data" subdirectory for storing tables
- Each table has two files:
  - `.meta`: Stores table schema (columns, data types, constraints)
  - `.data`: Stores table records in a CSV-like format

This architecture provides clear separation of concerns, with each component responsible for a specific aspect of the database system.

## Core C++ Concepts

This project demonstrates many core C++ concepts. Understanding these concepts is essential for implementing a similar system.

### 1. Object-Oriented Programming

#### Classes and Objects

The system is built using classes to represent database components. Each class encapsulates data and related functionality.

**Example (`Column.h`):**
```cpp
class Column {
private:
    std::string name;
    DataType dataType;
    bool primaryKey;
    bool unique;
    bool notNull;
    std::unordered_set<std::string> uniqueValues;

public:
    // Constructor and methods
    Column(const std::string& colName, Type type, 
           bool isPrimaryKey = false, bool isUnique = false, bool isNotNull = false);
    
    const std::string& getName() const;
    DataType& getDataType();
    const DataType& getDataType() const;
    bool isPrimaryKey() const;
    bool isUnique() const;
    bool isNotNull() const;
    // ...
};
```

#### Encapsulation

Private member variables with public accessor methods control access to object data.

**Example (`Column.h`):**
```cpp
// Private data
private:
    std::string name;
    bool primaryKey;

// Public accessors
public:
    const std::string& getName() const {
        return name;
    }
    
    bool isPrimaryKey() const {
        return primaryKey;
    }
```

#### Constructor Initialization Lists

Initializing member variables in constructor initialization lists improves efficiency.

**Example (`Column.h`):**
```cpp
Column(const std::string& colName, Type type, 
       bool isPrimaryKey = false, bool isUnique = false, bool isNotNull = false)
    : name(colName), 
      dataType(type), 
      primaryKey(isPrimaryKey), 
      unique(isUnique || isPrimaryKey), // Primary key implies unique
      notNull(isNotNull || isPrimaryKey) {} // Primary key implies not null
```

#### Composition

Building complex objects from simpler ones.

**Example (`Table.h`):**
```cpp
class Table {
private:
    std::string name;
    std::vector<Column> columns; // Table is composed of Columns
    std::vector<Record> records; // Table contains Records
    // ...
};
```

### 2. Memory Management

#### Smart Pointers

The project uses `std::unique_ptr` for automatic memory management.

**Example (`DatabaseSystem.h`):**
```cpp
// Each database is managed by a unique_ptr to a DatabaseManager
std::unordered_map<std::string, std::unique_ptr<DatabaseManager>> databases;
```

#### Move Semantics

Transferring ownership of resources efficiently.

**Example (`DatabaseSystem.h`):**
```cpp
bool createDatabase(const std::string& dbName) {
    // ...
    auto dbManager = std::make_unique<DatabaseManager>();
    // ...
    databases[dbName] = std::move(dbManager); // Transfer ownership
    return true;
}
```

#### RAII (Resource Acquisition Is Initialization)

Resources are tied to object lifetimes.

**Example (`Table.h`):**
```cpp
bool saveToFile() const {
    std::ofstream metaFile(metaFilePath);
    if (!metaFile) {
        return false;
    }
    // Use file...
    metaFile.close(); // File is closed when function exits or throws
}
```

### 3. STL (Standard Template Library)

#### Containers

The project uses various STL containers.

**Example:**
```cpp
// Vector for dynamic array of records
std::vector<Record> records;

// Unordered map for efficient lookups
std::unordered_map<std::string, size_t> columnMap;

// Unordered set for unique value tracking
std::unordered_set<std::string> uniqueValues;
```

#### Algorithms

STL algorithms for performing operations on containers.

**Example (`QueryParser.h`):**
```cpp
std::string toUpper(const std::string& str) {
    std::string upper = str;
    std::transform(upper.begin(), upper.end(), upper.begin(), 
        [](unsigned char c){ return std::toupper(c); });
    return upper;
}
```

### 4. Type Safety

#### Enum Classes

Type-safe enumerations that don't implicitly convert to integers.

**Example (`DataType.h`):**
```cpp
enum class Type {
    INT,
    BIGINT,
    STRING,
    DATE
};
```

#### Const Correctness

Methods that don't modify object state are marked as `const`.

**Example (`Column.h`):**
```cpp
const std::string& getName() const {
    return name;
}

bool isPrimaryKey() const {
    return primaryKey;
}
```

### 5. Error Handling

#### Exception Handling

Using exceptions for reporting and handling errors.

**Example (`DataType.h`):**
```cpp
void setValue(const std::string& val) {
    if (!validate(val)) {
        throw std::invalid_argument("Invalid value for the specified data type");
    }
    value = val;
}
```

#### Graceful Error Reporting

Using return values to indicate success or failure.

**Example (`QueryParser.h`):**
```cpp
return QueryResult(false, "Failed to create table '" + tableName + "'");
```

### 6. Modern C++ Features

#### Regular Expressions

Using regex for complex string pattern matching.

**Example (`QueryParser.h`):**
```cpp
std::regex createPattern(R"(CREATE\s+TABLE\s+(\w+)\s*\((.*)\))", std::regex_constants::icase);
std::smatch matches;

if (!std::regex_search(query, matches, createPattern)) {
    return QueryResult(false, "Invalid CREATE TABLE syntax");
}
```

#### Lambda Functions

Anonymous inline functions.

**Example (`QueryParser.h`):**
```cpp
std::transform(upper.begin(), upper.end(), upper.begin(), 
    [](unsigned char c){ return std::toupper(c); });
```

#### Conditional Compilation

Platform-specific code using preprocessor directives.

**Example (`DatabaseSystem.h`):**
```cpp
#ifdef _WIN32
    #include <direct.h>   // For _mkdir on Windows
    #define MKDIR(dir) _mkdir(dir)
#else
    #include <sys/types.h>
    #include <sys/stat.h>
    #define MKDIR(dir) mkdir(dir, 0777)
#endif
```

## Database Management

This section explains how databases are managed within the system, including database creation, deletion, and selection.

### Database Storage

Each database is stored as a directory on the filesystem with a "data" subdirectory for table files. This approach provides a simple and intuitive way to organize database data while maintaining persistence.

### Database Creation

The `CREATE DATABASE` command creates a new database by:
1. Checking if the database already exists
2. Creating a new directory for the database
3. Creating a "data" subdirectory for table storage
4. Creating a DatabaseManager object to manage the database

**Implementation (`DatabaseSystem.h`):**
```cpp
bool createDatabase(const std::string& dbName) {
    // Check if database already exists
    if (databases.find(dbName) != databases.end()) {
        return false;
    }
    
    // Create database directory
    if (!createDatabaseDirectory(dbName)) {
        return false;
    }
    
    // Create database manager
    auto dbManager = std::make_unique<DatabaseManager>();
    
    // Set the correct data path based on platform
    std::string dataPath;
    #ifdef _WIN32
        dataPath = baseDirectory + "\\" + dbName + "\\data";
    #else
        dataPath = baseDirectory + "/" + dbName + "/data";
    #endif
    
    dbManager->setDataDirectory(dataPath);
    
    // Add to databases map
    databases[dbName] = std::move(dbManager);
    
    return true;
}
```

**Usage Example:**
```sql
CREATE DATABASE school;
```

### Database Deletion

The `DROP DATABASE` command deletes a database by:
1. Checking if the database exists
2. Removing all files and directories associated with the database
3. Removing the database from the database collection

**Implementation (`DatabaseSystem.h`):**
```cpp
bool dropDatabase(const std::string& dbName) {
    // Check if database exists
    auto it = databases.find(dbName);
    if (it == databases.end()) {
        return false;
    }
    
    // If it's the current database, unselect it
    if (currentDatabase == dbName) {
        currentDatabase.clear();
    }
    
    // Construct the database path
    std::string dbPath;
    #ifdef _WIN32
        dbPath = baseDirectory + "\\" + dbName;
    #else
        dbPath = baseDirectory + "/" + dbName;
    #endif
    
    // Remove database from map
    databases.erase(it);
    
    // Remove database directory and all its contents
    return removeDir(dbPath);
}
```

**Usage Example:**
```sql
DROP DATABASE school;
```

### Database Selection

The `USE` command selects a database to work with by:
1. Checking if the database exists
2. Setting it as the current database

**Implementation (`DatabaseSystem.h`):**
```cpp
bool useDatabase(const std::string& dbName) {
    // Check if database exists
    if (databases.find(dbName) == databases.end()) {
        return false;
    }
    
    // Set as current database
    currentDatabase = dbName;
    return true;
}
```

**Usage Example:**
```sql
USE school;
```

### Exiting a Database

The `EXIT` command deselects the current database by clearing the current database name:

**Implementation:**
```cpp
bool exitDatabase() {
    currentDatabase.clear();
    return true;
}
```

**Usage Example:**
```sql
EXIT;
```

### Listing Databases

The special `.databases` command lists all available databases:

**Implementation:**
```cpp
std::vector<std::string> getAllDatabaseNames() const {
    std::vector<std::string> dbNames;
    for (const auto& [name, _] : databases) {
        dbNames.push_back(name);
    }
    return dbNames;
}
```

**Usage Example:**
```sql
.databases
```

### Database Initialization

When the system starts, it automatically scans the base directory to load existing databases:

**Implementation (`DatabaseSystem.h`):**
```cpp
void loadExistingDatabases() {
    // Ensure base directory exists
    if (!createDir(baseDirectory)) {
        std::cerr << "Failed to create base directory: " << baseDirectory << std::endl;
        return;
    }
    
    DIR* dir = opendir(baseDirectory.c_str());
    if (!dir) {
        std::cerr << "Failed to open base directory: " << baseDirectory << std::endl;
        return;
    }
    
    // Read directory entries
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string dbName = entry->d_name;
        
        // Skip . and ..
        if (dbName == "." || dbName == "..") {
            continue;
        }
        
        // Check if it's a directory
        std::string fullPath;
        #ifdef _WIN32
            fullPath = baseDirectory + "\\" + dbName;
        #else
            fullPath = baseDirectory + "/" + dbName;
        #endif
        
        struct stat statbuf;
        if (stat(fullPath.c_str(), &statbuf) != 0) {
            continue;
        }
        
        if (S_ISDIR(statbuf.st_mode)) {
            // Create data directory if it doesn't exist
            std::string dataPath;
            #ifdef _WIN32
                dataPath = fullPath + "\\data";
            #else
                dataPath = fullPath + "/data";
            #endif
            
            // Ensure data directory exists
            createDir(dataPath);
            
            // Create database manager for this database
            auto dbManager = std::make_unique<DatabaseManager>();
            dbManager->setDataDirectory(dataPath);
            databases[dbName] = std::move(dbManager);
        }
    }
    
    closedir(dir);
}
```

This approach allows the database system to maintain persistence across multiple runs, with all databases and their data preserved between program executions.

## Table Operations

This section explains how tables are managed within the system, including creation, modification, and manipulation of table data.

### Table Creation

Tables are created using the `CREATE TABLE` command, which specifies columns with their data types and constraints:

**Implementation (QueryParser):**
```cpp
QueryResult parseCreateTable(const std::string& query) {
    std::regex createPattern(R"(CREATE\s+TABLE\s+(\w+)\s*\((.*)\))", std::regex_constants::icase);
    std::smatch matches;
    
    if (!std::regex_search(query, matches, createPattern)) {
        return QueryResult(false, "Invalid CREATE TABLE syntax");
    }
    
    std::string tableName = matches[1].str();
    std::string columnsStr = matches[2].str();
    
    // Check if table already exists
    if (dbManager.tableExists(tableName)) {
        return QueryResult(false, "Table '" + tableName + "' already exists");
    }
    
    // Parse column definitions
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
    
    // Validate columns before creating anything
    for (const auto& colDef : columnDefs) {
        // Parse column name, type, and constraints
        // ...
        
        // Check for PRIMARY KEY
        if (isPrimaryKey) {
            if (hasPrimaryKey) {
                return QueryResult(false, "Table cannot have multiple PRIMARY KEYs");
            }
            hasPrimaryKey = true;
        }
        
        // Store column definition
        columns.push_back({colName, colType, isPrimaryKey, isUnique, isNotNull});
    }
    
    // Create the table
    if (!dbManager.createTable(tableName)) {
        return QueryResult(false, "Failed to create table '" + tableName + "'");
    }
    
    // Add all columns to the table
    Table* table = dbManager.getTable(tableName);
    for (const auto& col : columns) {
        if (!table->addColumn(col.name, col.type, col.isPrimaryKey, col.isUnique, col.isNotNull)) {
            // If any column addition fails, drop the table
            dbManager.dropTable(tableName);
            return QueryResult(false, "Failed to create columns for table '" + tableName + "'");
        }
    }
    
    return QueryResult(true, "Table '" + tableName + "' created successfully");
}
```

**Usage Example:**
```sql
CREATE TABLE students (
    id INT PRIMARY KEY,
    name STRING NOT NULL,
    age INT,
    enrollment_date DATE
);
```

### Table Modification

Tables can be modified using the `ALTER TABLE` command to add or remove columns:

**Implementation (QueryParser):**
```cpp
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
        // Add a column
        std::string colTypeStr = toUpper(matches[4].str());
        std::string constraintsStr = matches[5].matched ? toUpper(matches[5].str()) : "";
        
        // Convert string to DataType
        Type colType = DataType::stringToType(colTypeStr);
        
        // Parse constraints
        bool isPrimaryKey = constraintsStr.find("PRIMARY KEY") != std::string::npos;
        bool isUnique = constraintsStr.find("UNIQUE") != std::string::npos;
        bool isNotNull = constraintsStr.find("NOT NULL") != std::string::npos;
        
        // Add column to table
        if (!table->addColumn(columnName, colType, isPrimaryKey, isUnique, isNotNull)) {
            return QueryResult(false, "Failed to add column: " + columnName);
        }
        
        return QueryResult(true, "Column '" + columnName + "' added to table '" + tableName + "'");
    } 
    else if (operation == "DROP") {
        // Remove a column
        if (!table->removeColumn(columnName)) {
            return QueryResult(false, "Failed to drop column: " + columnName);
        }
        
        return QueryResult(true, "Column '" + columnName + "' dropped from table '" + tableName + "'");
    }
    
    return QueryResult(false, "Unknown ALTER TABLE operation: " + operation);
}
```

**Usage Examples:**
```sql
-- Add a column
ALTER TABLE students ADD email STRING UNIQUE;

-- Drop a column
ALTER TABLE students DROP email;
```

### Table Deletion

Tables can be deleted using the `DROP TABLE` command:

**Implementation (QueryParser):**
```cpp
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
```

**Usage Example:**
```sql
DROP TABLE students;
```

### Data Insertion

Data is inserted into tables using the `INSERT INTO` command:

**Implementation (QueryParser):**
```cpp
QueryResult parseInsert(const std::string& query) {
    std::regex insertPattern(R"(INSERT\s+INTO\s+(\w+)\s*\((.*?)\)\s*VALUES\s*\((.*?)\))", std::regex_constants::icase);
    std::smatch matches;
    
    if (!std::regex_search(query, matches, insertPattern)) {
        return QueryResult(false, "Invalid INSERT INTO syntax");
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
    for (auto& name : columnNames) {
        name = trim(name);
    }
    
    // Parse values
    auto values = split(valuesStr, ',');
    for (auto& value : values) {
        value = trim(value);
        
        // Remove quotes from string values
        if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
            value = value.substr(1, value.size() - 2);
        }
    }
    
    // Check if column count matches value count
    if (columnNames.size() != values.size()) {
        return QueryResult(false, "Column count doesn't match value count");
    }
    
    // Insert the record
    if (!table->insert(columnNames, values)) {
        return QueryResult(false, "Failed to insert record");
    }
    
    return QueryResult(true, "Record inserted successfully");
}
```

**Usage Example:**
```sql
INSERT INTO students (id, name, age, enrollment_date) 
VALUES (1, 'John Smith', 20, '2023-09-15');
```

### Data Querying

Data is queried using the `SELECT` command, which can include column selection and WHERE conditions:

**Implementation (QueryParser):**
```cpp
QueryResult parseSelect(const std::string& query) {
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
    
    // Parse column names (* means all columns)
    std::vector<std::string> columnNames;
    if (trim(columnsStr) == "*") {
        columnNames = table->getColumnNames();
    } else {
        columnNames = split(columnsStr, ',');
        for (auto& name : columnNames) {
            name = trim(name);
        }
    }
    
    // Parse WHERE clause if present
    std::shared_ptr<Condition> condition = nullptr;
    if (!whereClause.empty()) {
        condition = parseCondition(whereClause);
    }
    
    // Execute the query
    auto result = table->select(columnNames, condition);
    
    // Create the result structure
    QueryResult queryResult(true, "Query executed successfully");
    queryResult.columnNames = columnNames;
    queryResult.rows = result;
    
    return queryResult;
}
```

**Usage Examples:**
```sql
-- Select all columns and rows
SELECT * FROM students;

-- Select specific columns
SELECT name, age FROM students;

-- Select with WHERE condition
SELECT * FROM students WHERE age > 21;
```

### Data Update

Data is updated using the `UPDATE` command:

**Implementation (QueryParser):**
```cpp
QueryResult parseUpdate(const std::string& query) {
    std::regex updatePattern(R"(UPDATE\s+(\w+)\s+SET\s+(.*?)(?:\s+WHERE\s+(.*))?)", std::regex_constants::icase);
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
    
    // Parse SET clause to get column-value pairs
    auto assignments = split(setClause, ',');
    std::unordered_map<std::string, std::string> columnValues;
    
    for (const auto& assignment : assignments) {
        auto parts = split(assignment, '=');
        if (parts.size() != 2) {
            return QueryResult(false, "Invalid SET clause: " + assignment);
        }
        
        std::string colName = trim(parts[0]);
        std::string value = trim(parts[1]);
        
        // Remove quotes from string values
        if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
            value = value.substr(1, value.size() - 2);
        }
        
        columnValues[colName] = value;
    }
    
    // Parse WHERE clause if present
    std::shared_ptr<Condition> condition = nullptr;
    if (!whereClause.empty()) {
        condition = parseCondition(whereClause);
    }
    
    // Execute the update
    int updatedCount = table->update(columnValues, condition);
    
    if (updatedCount < 0) {
        return QueryResult(false, "Failed to update records");
    }
    
    return QueryResult(true, std::to_string(updatedCount) + " record(s) updated");
}
```

**Usage Example:**
```sql
UPDATE students SET age = 21 WHERE id = 1;
```

### Data Deletion

Data is deleted using the `DELETE FROM` command:

**Implementation (QueryParser):**
```cpp
QueryResult parseDelete(const std::string& query) {
    std::regex deletePattern(R"(DELETE\s+FROM\s+(\w+)(?:\s+WHERE\s+(.*))?)", std::regex_constants::icase);
    std::smatch matches;
    
    if (!std::regex_search(query, matches, deletePattern)) {
        return QueryResult(false, "Invalid DELETE FROM syntax");
    }
    
    std::string tableName = matches[1].str();
    std::string whereClause = matches[2].matched ? matches[2].str() : "";
    
    Table* table = dbManager.getTable(tableName);
    if (!table) {
        return QueryResult(false, "Table '" + tableName + "' does not exist");
    }
    
    // Parse WHERE clause if present
    std::shared_ptr<Condition> condition = nullptr;
    if (!whereClause.empty()) {
        condition = parseCondition(whereClause);
    }
    
    // Execute the delete
    int deletedCount = table->deleteRecords(condition);
    
    if (deletedCount < 0) {
        return QueryResult(false, "Failed to delete records");
    }
    
    return QueryResult(true, std::to_string(deletedCount) + " record(s) deleted");
}
```

**Usage Example:**
```sql
DELETE FROM students WHERE id = 2;
```

## SQL Command Parsing

This section explains how SQL commands are parsed and executed in the system.

### Query Parsing Architecture

The QueryParser class is responsible for parsing SQL commands and dispatching them to appropriate handlers. It uses regular expressions to match command patterns and extract relevant information.

**Key Components:**

1. **QueryParser Class**: Parses and executes SQL commands
2. **QueryResult Structure**: Holds the results of query execution
3. **Condition Structure**: Represents WHERE conditions for filtering

### Command Identification

The system first identifies the type of SQL command using regular expressions:

```cpp
QueryResult parseQuery(const std::string& query) {
    // Normalize the query (remove extra whitespace, convert to uppercase for pattern matching)
    std::string normalizedQuery = normalizeQuery(query);
    
    // Check for different query types
    bool isCreateTable = std::regex_search(normalizedQuery, std::regex("^CREATE\\s+TABLE", std::regex_constants::icase));
    bool isDropTable = std::regex_search(normalizedQuery, std::regex("^DROP\\s+TABLE", std::regex_constants::icase));
    bool isAlterTable = std::regex_search(normalizedQuery, std::regex("^ALTER\\s+TABLE", std::regex_constants::icase));
    bool isInsert = std::regex_search(normalizedQuery, std::regex("^INSERT\\s+INTO", std::regex_constants::icase));
    bool isSelect = std::regex_search(normalizedQuery, std::regex("^SELECT", std::regex_constants::icase));
    bool isUpdate = std::regex_search(normalizedQuery, std::regex("^UPDATE", std::regex_constants::icase));
    bool isDelete = std::regex_search(normalizedQuery, std::regex("^DELETE\\s+FROM", std::regex_constants::icase));
    
    // Dispatch to appropriate handler
    if (isCreateTable) {
        return parseCreateTable(query);
    } else if (isDropTable) {
        return parseDropTable(query);
    } else if (isAlterTable) {
        return parseAlterTable(query);
    } else if (isInsert) {
        return parseInsert(query);
    } else if (isSelect) {
        return parseSelect(query);
    } else if (isUpdate) {
        return parseUpdate(query);
    } else if (isDelete) {
        return parseDelete(query);
    }
    
    // Command not recognized
    return QueryResult(false, "Unrecognized SQL command");
}
```

### Query Result Structure

Results are returned using the QueryResult structure:

```cpp
struct QueryResult {
    bool success;
    std::string message;
    std::vector<std::string> columnNames;
    std::vector<std::vector<std::string>> rows;
    
    QueryResult(bool s = true, const std::string& msg = "") 
        : success(s), message(msg) {}
};
```

### Condition Parsing

WHERE conditions are parsed and represented using a linked structure of Condition objects:

```cpp
std::shared_ptr<Condition> parseCondition(const std::string& conditionStr) {
    // Regular expression to match conditions with logical operators
    std::regex conditionPattern(R"((.+?)(?:\s+(AND|OR)\s+(.+))?$)", std::regex_constants::icase);
    std::smatch matches;
    
    if (!std::regex_match(conditionStr, matches, conditionPattern)) {
        throw std::invalid_argument("Invalid condition: " + conditionStr);
    }
    
    std::string firstConditionStr = matches[1].str();
    std::string logicalOpStr = matches[2].matched ? matches[2].str() : "";
    std::string nextConditionStr = matches[3].matched ? matches[3].str() : "";
    
    // Parse the first condition (format: column operator value)
    std::regex atomicPattern(R"(\s*(\w+)\s*([=!<>]+)\s*([^=!<>]+)\s*)");
    std::smatch atomicMatches;
    
    if (!std::regex_match(firstConditionStr, atomicMatches, atomicPattern)) {
        throw std::invalid_argument("Invalid atomic condition: " + firstConditionStr);
    }
    
    auto condition = std::make_shared<Condition>();
    condition->column = trim(atomicMatches[1].str());
    condition->op = Condition::stringToOperator(atomicMatches[2].str());
    condition->value = trim(atomicMatches[3].str());
    
    // Remove quotes from string values
    if (condition->value.size() >= 2 && 
        condition->value.front() == '\'' && condition->value.back() == '\'') {
        condition->value = condition->value.substr(1, condition->value.size() - 2);
    }
    
    // Handle logical operators (AND, OR) for complex conditions
    if (!logicalOpStr.empty() && !nextConditionStr.empty()) {
        condition->logicalOp = Condition::stringToLogicalOperator(logicalOpStr);
        condition->nextCondition = parseCondition(nextConditionStr);
    }
    
    return condition;
}
```

### String Utilities

Several string utility functions help with parsing:

```cpp
// Trim whitespace from a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

// Convert string to uppercase
std::string toUpper(const std::string& str) {
    std::string upper = str;
    std::transform(upper.begin(), upper.end(), upper.begin(), 
        [](unsigned char c){ return std::toupper(c); });
    return upper;
}

// Split string by delimiter
std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}
```

### Regular Expressions for Parsing

Regular expressions are used extensively for parsing SQL commands:

| Command Type | Regular Expression Pattern |
|--------------|----------------------------|
| CREATE TABLE | `CREATE\s+TABLE\s+(\w+)\s*\((.*)\)` |
| DROP TABLE   | `DROP\s+TABLE\s+(\w+)` |
| ALTER TABLE  | `ALTER\s+TABLE\s+(\w+)\s+(ADD\|DROP)\s+(\w+)(?:\s+(\w+)(?:\s+(.*))?)?` |
| INSERT INTO  | `INSERT\s+INTO\s+(\w+)\s*\((.*?)\)\s*VALUES\s*\((.*?)\)` |
| SELECT       | `SELECT\s+(.*?)\s+FROM\s+(\w+)(?:\s+WHERE\s+(.*))?` |
| UPDATE       | `UPDATE\s+(\w+)\s+SET\s+(.*?)(?:\s+WHERE\s+(.*))?` |
| DELETE FROM  | `DELETE\s+FROM\s+(\w+)(?:\s+WHERE\s+(.*))?` |

These patterns allow the system to parse and extract relevant components from SQL commands, such as table names, column definitions, conditions, and values.

## Data Types and Validation

This section explains how data types are implemented and validated in the system.

### Supported Data Types

The system supports four basic data types, defined in an enum class:

```cpp
enum class Type {
    INT,     // Integer values
    BIGINT,  // Large integer values (>9 digits)
    STRING,  // Text values
    DATE     // Date values (YYYY-MM-DD format)
};
```

### DataType Class

The DataType class encapsulates a type and a value, providing validation methods:

```cpp
class DataType {
private:
    Type type;
    std::string value;
    
    bool validateInt(const std::string& val) const;
    bool validateBigInt(const std::string& val) const;
    bool validateDate(const std::string& val) const;

public:
    DataType(Type t, const std::string& val = "");
    
    bool validate(const std::string& val) const;
    void setValue(const std::string& val);
    std::string getValue() const;
    Type getType() const;
    
    static std::string typeToString(Type t);
    static Type stringToType(const std::string& typeStr);
};
```

### Type Validation

Each data type has specific validation rules:

#### INT Validation

Validates that a string can be converted to an integer:

```cpp
bool validateInt(const std::string& val) const {
    try {
        std::stoi(val);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}
```

#### BIGINT Validation

Validates that a string can be converted to a long long integer:

```cpp
bool validateBigInt(const std::string& val) const {
    try {
        std::stoll(val);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}
```

#### STRING Validation

All strings are considered valid (no validation required).

#### DATE Validation

Validates that a string represents a valid date in YYYY-MM-DD format:

```cpp
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
```

### Type Conversion

The class provides methods for converting between string representations and Type enum values:

```cpp
std::string typeToString(Type t) {
    switch (t) {
        case Type::INT: return "INT";
        case Type::BIGINT: return "BIGINT";
        case Type::STRING: return "STRING";
        case Type::DATE: return "DATE";
        default: return "UNKNOWN";
    }
}

Type stringToType(const std::string& typeStr) {
    if (typeStr == "INT") return Type::INT;
    if (typeStr == "BIGINT") return Type::BIGINT;
    if (typeStr == "STRING") return Type::STRING;
    if (typeStr == "DATE") return Type::DATE;
    throw std::invalid_argument("Unknown data type: " + typeStr);
}
```

### Value Setting with Validation

When setting a value, the class validates it against the data type:

```cpp
void setValue(const std::string& val) {
    if (!validate(val)) {
        throw std::invalid_argument("Invalid value for the specified data type");
    }
    value = val;
}
```

### General Validation Method

The main validate method dispatches to type-specific validators:

```cpp
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
```

### Column Constraints

Beyond data type validation, columns can have additional constraints:

1. **PRIMARY KEY**: A unique identifier for a row
2. **UNIQUE**: Values must be unique within the column
3. **NOT NULL**: Values cannot be null (empty)

These constraints are implemented in the Column class:

```cpp
class Column {
private:
    std::string name;
    DataType dataType;
    bool primaryKey;
    bool unique;
    bool notNull;
    std::unordered_set<std::string> uniqueValues;

public:
    // Constructor sets up constraints
    Column(const std::string& colName, Type type, 
           bool isPrimaryKey = false, bool isUnique = false, bool isNotNull = false)
        : name(colName), 
          dataType(type), 
          primaryKey(isPrimaryKey), 
          unique(isUnique || isPrimaryKey), // Primary key implies unique
          notNull(isNotNull || isPrimaryKey) {} // Primary key implies not null
          
    // Value validation against constraints
    bool validateValue(const std::string& value, bool isUpdate = false) const {
        // For updates, empty value might mean "skip this column"
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
    
    // Enforcing uniqueness
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
};
```

The combination of data type validation and constraint enforcement ensures that only valid data is stored in the database tables.

## File I/O and Persistence

This section explains how the database system persists data to disk.

### Storage Structure

Database data is stored in a hierarchical file structure:

```
databases/                      # Base directory for all databases
├── example_db/                 # A database directory
│   └── data/                   # Data directory for tables
│       ├── students.meta       # Metadata for the students table
│       └── students.data       # Data records for the students table
└── another_db/                 # Another database directory
    └── data/                   # Data directory for tables
        ├── employees.meta      # Metadata for the employees table
        └── employees.data      # Data records for the employees table
```

### File Types

Each table is represented by two files:

1. **Meta File (.meta)**: Contains table structure information (columns, data types, constraints)
2. **Data File (.data)**: Contains the actual data records in a CSV-like format

### Table Metadata Storage

Table metadata is stored in a simple text format, with one line per column:

```cpp
bool saveToFile() const {
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
    
    // Save data records...
    return true;
}
```

Sample .meta file content:
```
id,INT,PK,UQ,NN
name,STRING,,UQ,NN
age,INT,,,
enrollment_date,DATE,,,
```

### Table Data Storage

Table data is stored in a CSV-like format, with one record per line:

```cpp
bool saveToFile() const {
    // Save metadata first...
    
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
```

Sample .data file content:
```
1,John Smith,20,2023-09-15
2,Jane Doe,22,2023-08-10
3,Bob Johnson,21,2023-09-01
```

### Loading Data from Files

When the system starts, it loads existing databases and tables from disk:

```cpp
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
    records.clear();
    std::ifstream dataFile(dataFilePath);
    if (dataFile) {
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
    }

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
```

### Directory Operations

The system includes cross-platform functions for directory operations:

```cpp
// Create a directory
bool createDir(const std::string& path) {
    int result = MKDIR(path.c_str());
    
    // If directory was created OR it already exists, return true
    return (result == 0 || errno == EEXIST);
}

// Remove a directory and its contents
bool removeDir(const std::string& path) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return false;
    }
    
    struct dirent* entry;
    bool success = true;
    
    // First, remove all files and subdirectories
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        
        // Skip . and ..
        if (name == "." || name == "..") {
            continue;
        }
        
        // Build full path
        std::string fullPath;
        #ifdef _WIN32
            fullPath = path + "\\" + name;
        #else
            fullPath = path + "/" + name;
        #endif
        
        // Check if it's a directory
        struct stat statbuf;
        if (stat(fullPath.c_str(), &statbuf) != 0) {
            success = false;
            continue;
        }
        
        if (S_ISDIR(statbuf.st_mode)) {
            // Recursively remove subdirectory
            if (!removeDir(fullPath)) {
                success = false;
            }
        } else {
            // Remove file
            if (std::remove(fullPath.c_str()) != 0) {
                success = false;
            }
        }
    }
    
    closedir(dir);
    
    // Now remove the directory itself
    #ifdef _WIN32
        success = _rmdir(path.c_str()) == 0 && success;
    #else
        success = rmdir(path.c_str()) == 0 && success;
    #endif
    
    return success;
}
```

### Persistence Strategy

The database system ensures data persistence by:

1. **Loading existing data at startup**: The system scans the base directory for existing databases and loads their metadata
2. **Saving data after modifications**: After operations that modify data (INSERT, UPDATE, DELETE), the data is saved to disk
3. **Atomic file operations**: When possible, file operations are performed in a way that prevents data corruption

### Error Handling in File Operations

The system includes error handling for file operations:

```cpp
bool saveToFile() const {
    std::ofstream metaFile(metaFilePath);
    if (!metaFile) {
        // Handle error opening meta file
        return false;
    }
    
    // Write metadata...
    metaFile.close();
    
    std::ofstream dataFile(dataFilePath);
    if (!dataFile) {
        // Handle error opening data file
        return false;
    }
    
    // Write data...
    dataFile.close();
    
    return true;
}
```

This persistence approach provides a simple yet effective way to store and retrieve database data across program executions.

## Command-Line Interface

This section explains how the command-line interface (CLI) is implemented in the system.

### Main Command Loop

The main application provides an interactive prompt where users can enter SQL commands:

```cpp
int main() {
    DatabaseSystem dbSystem("databases");
    std::string query;
    
    printHelp();  // Display welcome and help message
    
    while (true) {
        // Display prompt based on current database
        std::string prompt = "sql";
        if (!dbSystem.getCurrentDatabaseName().empty()) {
            prompt += "(" + dbSystem.getCurrentDatabaseName() + ")";
        }
        prompt += "> ";
        
        std::cout << prompt;
        
        // Get user input
        std::getline(std::cin, query);
        
        // Check for special commands
        if (query == ".exit") {
            std::cout << "Exiting..." << std::endl;
            break;
        } else if (query == ".help") {
            printHelp();
            continue;
        } else if (query == ".databases") {
            listDatabases(dbSystem);
            continue;
        } else if (query == ".tables") {
            listTables(dbSystem);
            continue;
        }
        
        // Process SQL commands
        if (processSqlCommand(dbSystem, query)) {
            // Command was processed successfully
            continue;
        }
        
        std::cout << "Unknown command. Type .help for help." << std::endl;
    }
    
    return 0;
}
```

### Welcome Message and Help

The system displays a welcome message and help information at startup:

```cpp
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
```

### Command Processing

The system categorizes commands as either database-level or table-level:

```cpp
bool processSqlCommand(DatabaseSystem& dbSystem, const std::string& query) {
    // Check if it's a database-level command
    if (isDatabaseCommand(query)) {
        return processDatabaseCommand(dbSystem, query);
    }
    
    // If it's a table command, check if a database is selected
    if (dbSystem.getCurrentDatabaseName().empty()) {
        std::cout << "Error: No database selected. Use 'USE dbName' first." << std::endl;
        return true;  // Command was recognized but failed due to no database
    }
    
    // Process table command using the current database
    DatabaseManager* dbManager = dbSystem.getCurrentDatabaseManager();
    if (!dbManager) {
        std::cout << "Error: Failed to access database." << std::endl;
        return true;  // Command was recognized but failed due to database access error
    }
    
    // Create a query parser and process the command
    QueryParser parser(*dbManager);
    QueryResult result = parser.parseQuery(query);
    
    // Display the results
    displayResults(result);
    
    return true;  // Command was processed
}
```

### Result Display

Query results are displayed in a formatted tabular layout:

```cpp
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
```

### Special Commands

The system implements special dot commands for common operations:

```cpp
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

// List all tables in the current database
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

// Show table schema
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
```

### User Experience

The command-line interface is designed to be user-friendly with:

1. Context-aware prompt that shows the current database
2. Clear error messages
3. Formatted tabular output for query results
4. Helpful commands for exploring databases and tables

This interface provides an intuitive way for users to interact with the database system, making it accessible even for those with limited SQL knowledge.

## Cross-Platform Compatibility

This section explains how the system is designed to work across different operating systems.

### Platform Detection

The system uses preprocessor directives to detect the operating system:

```cpp
#ifdef _WIN32
    // Windows-specific code
#else
    // Unix/Linux/macOS code
#endif
```

### Platform-Specific Headers

Different header files are included based on the detected platform:

```cpp
#include <errno.h>    // Common to all platforms
#ifdef _WIN32
    #include <direct.h>   // For _mkdir on Windows
#else
    #include <sys/types.h>
    #include <sys/stat.h>
#endif
```

### Directory Operations

Directory creation is handled differently on each platform:

```cpp
#ifdef _WIN32
    #define MKDIR(dir) _mkdir(dir)
#else
    #define MKDIR(dir) mkdir(dir, 0777)
#endif

bool createDir(const std::string& path) {
    int result = MKDIR(path.c_str());
    
    // If directory was created OR it already exists, return true
    return (result == 0 || errno == EEXIST);
}
```

### Path Handling

File paths are constructed differently on each platform:

```cpp
std::string getTablePath(const std::string& tableName) {
    #ifdef _WIN32
        return dataDirectory + "\\" + tableName;
    #else
        return dataDirectory + "/" + tableName;
    #endif
}
```

### Directory Traversal

The system uses the POSIX `dirent.h` interface for directory traversal on all platforms:

```cpp
#include <dirent.h>

void loadExistingDatabases() {
    DIR* dir = opendir(baseDirectory.c_str());
    if (!dir) {
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string dbName = entry->d_name;
        
        // Skip . and ..
        if (dbName == "." || dbName == "..") {
            continue;
        }
        
        // Process directory entry
        // ...
    }
    
    closedir(dir);
}
```

### Directory Removal

Directory removal is performed recursively with platform-specific code:

```cpp
bool removeDir(const std::string& path) {
    // ... recursive file removal code ...
    
    // Remove the directory itself
    #ifdef _WIN32
        success = _rmdir(path.c_str()) == 0 && success;
    #else
        success = rmdir(path.c_str()) == 0 && success;
    #endif
    
    return success;
}
```

### Build Configuration

The system's CMakeLists.txt file enables cross-platform building:

```cmake
cmake_minimum_required(VERSION 3.10)
project(SQLDatabase)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Source files
set(SOURCE_FILES src/main.cpp)

# Main executable
add_executable(sqldb ${SOURCE_FILES})

# Platform-specific libraries
if(WIN32)
    target_link_libraries(sqldb)
else()
    target_link_libraries(sqldb pthread)
endif()
```

### Terminal I/O

The command-line interface works across platforms using standard C++ I/O streams:

```cpp
// Display prompt
std::cout << prompt;

// Get user input
std::getline(std::cin, query);
```

### File I/O

File operations use C++ standard library functions that work across platforms:

```cpp
// Writing to a file
std::ofstream metaFile(metaFilePath);
if (metaFile) {
    // Write data
    metaFile.close();
}

// Reading from a file
std::ifstream dataFile(dataFilePath);
if (dataFile) {
    // Read data
    dataFile.close();
}
```

### Character Case Handling

The system handles case insensitivity in a platform-independent way:

```cpp
std::string toUpper(const std::string& str) {
    std::string upper = str;
    std::transform(upper.begin(), upper.end(), upper.begin(), 
        [](unsigned char c){ return std::toupper(c); });
    return upper;
}
```

These cross-platform considerations ensure that the database system works consistently across Windows, Linux, and macOS environments, providing a unified experience for users regardless of their operating system.

## Building and Running the System

This section provides instructions for building and running the database system.

### Prerequisites

To build the system, you'll need:

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10 or higher
- Git (optional, for version control)

### Obtaining the Source Code

Clone the repository or download the source code:

```bash
git clone https://github.com/username/sql-database.git
cd sql-database
```

### Building with CMake

Create a build directory and compile the project:

```bash
# Create a build directory
mkdir build
cd build

# Generate build files with CMake
cmake ..

# Build the project
cmake --build .
```

### Building Manually

If you prefer to build without CMake:

#### On Linux/Mac

```bash
g++ -std=c++17 src/main.cpp -o sqldb -I include/
```

#### On Windows with MSVC

```bash
cl /std:c++17 /EHsc /Iinclude src/main.cpp /Fesqldb.exe
```

#### On Windows with MinGW

```bash
g++ -std=c++17 src/main.cpp -o sqldb.exe -I include/
```

### Running the Program

Once built, you can run the program:

```bash
# On Linux/Mac
./sqldb

# On Windows
sqldb.exe
```

### Command Examples

Here are some example commands to get started:

```sql
-- Create a database
CREATE DATABASE school;

-- Use the database
USE school;

-- Create a table
CREATE TABLE students (
    id INT PRIMARY KEY,
    name STRING NOT NULL,
    age INT,
    enrollment_date DATE
);

-- Insert data
INSERT INTO students (id, name, age, enrollment_date) 
VALUES (1, 'John Smith', 20, '2023-09-15');

INSERT INTO students (id, name, age, enrollment_date) 
VALUES (2, 'Jane Doe', 22, '2023-08-10');

-- Query data
SELECT * FROM students;
SELECT name, age FROM students WHERE age > 20;

-- Update data
UPDATE students SET age = 21 WHERE id = 1;

-- Delete data
DELETE FROM students WHERE id = 2;

-- Show tables
SHOW TABLES;

-- Describe table schema
DESC students;

-- Exit the current database
EXIT;

-- Exit the program
.exit
```

### Troubleshooting

Common issues and solutions:

1. **Compilation Errors**: 
   - Ensure your C++ compiler supports C++17
   - Check that all header files are in the include directory
   - Use the `-I` flag to specify the include directory

2. **Runtime Errors**:
   - Ensure you have write permissions in the directory for database creation
   - Check for syntax errors in SQL commands
   - Make sure to select a database with `USE` before running table commands

3. **Cross-Platform Issues**:
   - Use forward slashes in paths on all platforms (`path/to/file`) for consistency
   - Avoid OS-specific functions not covered by the compatibility layer

## Conclusion

This comprehensive guide has covered the key aspects of the SQL-like database system implemented in C++. The project demonstrates a wide range of programming concepts and provides a functional database system with persistence, SQL command support, and a user-friendly interface.

### Key Takeaways

1. **Architecture**: The system uses a hierarchical design with clear separation of concerns between components.

2. **Object-Oriented Design**: Classes like DatabaseSystem, DatabaseManager, Table, and Column encapsulate specific functionality.

3. **Data Management**: The system handles data types, validation, and constraints for database integrity.

4. **SQL Support**: A complete set of SQL commands is supported for database and data manipulation.

5. **Persistence**: File I/O enables data to persist between program executions.

6. **Cross-Platform**: Careful design allows the system to work on multiple operating systems.

7. **User Interface**: A command-line interface provides user-friendly interaction with the database.

### Extending the System

The modular design makes it easy to extend the system with additional features such as:

- Additional data types (FLOAT, BOOLEAN, etc.)
- Support for JOINs and more complex queries
- Indexes for improved query performance
- Transactions and concurrency control
- Network support for client-server architecture

### Learning Value

This project provides an excellent learning opportunity for:

- C++ programming concepts and best practices
- Database system architecture and implementation
- SQL parsing and execution
- File I/O and persistence
- Cross-platform development

By understanding the concepts and implementation details covered in this guide, you should now have the knowledge to implement a similar database system or extend this one with your own features.
