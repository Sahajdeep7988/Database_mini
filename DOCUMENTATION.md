# RelictDB - Technical Documentation

## Overview

This document provides detailed technical information about the RelictDB system implementation. The system is designed to provide a simple but effective in-memory and disk-persistent database engine with SQL-like syntax for data storage and retrieval.

## Author
**Sahajdeep Singh**  
Email: sahajdeepsingh404@gmail.com  
Mobile: +91 7988168548

## Architecture

The system follows a modular architecture with the following major components:

### Core Components

1. **DatabaseSystem**: The top-level component that manages multiple databases
2. **DatabaseManager**: Manages a single database and its tables
3. **Table**: Represents a database table with columns and records
4. **Column**: Defines a table column with name, data type, and constraints
5. **QueryParser**: Parses and executes SQL-like queries
6. **DataType**: Defines and manages supported data types

### Directory Structure

```
├── include/                # Header files
│   ├── AggregateFunction.h # Aggregate functions (COUNT, SUM, etc.)
│   ├── Column.h            # Column class
│   ├── DataType.h          # Data type definitions
│   ├── DatabaseManager.h   # Database manager
│   ├── DatabaseSystem.h    # Database system
│   ├── QueryParser.h       # SQL query parser
│   ├── StringFunction.h    # String functions (UPPER, LOWER, etc.)
│   └── Table.h             # Table class
└── src/                    # Source files
    └── main.cpp            # Main application
```

### On-Disk Storage Format

Each database is stored as a directory structure:

```
databases/             # Base directory for all databases
├── mydatabase/        # A specific database
│   └── data/          # Data directory inside the database
│       ├── table1.meta # Table metadata (columns, types, constraints)
│       ├── table1.data # Table data records
│       ├── table2.meta # Another table's metadata
│       └── table2.data # Another table's data
└── anotherdatabase/   # Another database
    └── data/
        ├── ...
```

#### File Formats

1. **Table Metadata (.meta)**:
   Each line represents a column with comma-separated values:
   ```
   columnName,dataType,PK,UQ,NN
   ```
   Where:
   - `columnName` is the name of the column
   - `dataType` is one of: INT, BIGINT, STRING, DATE
   - `PK` is either "PK" if it's a primary key, or empty
   - `UQ` is either "UQ" if it's unique, or empty
   - `NN` is either "NN" if it's not null, or empty

2. **Table Data (.data)**:
   Each line represents a record with comma-separated values matching the columns defined in the metadata file.

## Implementation Details

### Data Types

The system supports the following data types:

1. **INT**: Integer numbers stored as `int` (32-bit signed integers)
2. **BIGINT**: Large integers stored as `long long` (64-bit signed integers)
3. **STRING**: Text data stored as `std::string`
4. **DATE**: Date values in YYYY-MM-DD format stored as `std::string`

### Constraints

The system supports the following constraints:

1. **PRIMARY KEY**: Unique identifier for records (implies NOT NULL)
2. **UNIQUE**: Values must be unique across all records
3. **NOT NULL**: Values cannot be empty/null

### Case Sensitivity

All SQL commands and identifiers (table names, column names) are case-insensitive. String data comparisons in the WHERE clause are also case-insensitive, making the system more user-friendly.

### Command Parsing

The system uses regular expressions (std::regex) to parse SQL commands, providing a flexible way to handle variations in syntax while maintaining proper validation. Commands are processed in the following order:

1. Check if it's a special dot command (.help, .exit, etc.)
2. Check if it's a database-level command (CREATE DATABASE, USE, etc.)
3. Check if it's a table-level command (CREATE TABLE, INSERT, etc.)

### Error Handling

The system implements robust error handling throughout the codebase:

1. **Input validation**: Commands are validated before execution
2. **Database operations**: Errors during file/directory operations are properly reported
3. **Data validation**: Data is validated against constraints before insertion
4. **Query execution**: Syntax and semantic errors are caught and reported

### Platform Compatibility

The system is designed to work on multiple platforms (Windows, Unix-based systems) by:

1. Using conditional compilation (`#ifdef _WIN32`) for platform-specific code
2. Handling path separators appropriately (backslash on Windows, forward slash on Unix)
3. Using proper directory and file operations for each platform

## Class Details

### DatabaseSystem

Manages multiple databases and provides operations for creating, dropping, and using databases.

**Key Methods**:
- `createDatabase(const std::string& dbName)`: Creates a new database
- `dropDatabase(const std::string& dbName)`: Deletes a database
- `useDatabase(const std::string& dbName)`: Selects a database
- `getCurrentDatabaseManager()`: Gets the current database manager
- `getAllDatabaseNames()`: Lists all databases

### DatabaseManager

Manages a single database with multiple tables.

**Key Methods**:
- `createTable(const std::string& tableName)`: Creates a new table
- `dropTable(const std::string& tableName)`: Deletes a table
- `getTable(const std::string& tableName)`: Gets a table by name
- `getAllTableNames()`: Lists all tables in the database
- `setDataDirectory(const std::string& dirPath)`: Sets the data directory path

### Table

Represents a database table with columns and records.

**Key Methods**:
- `addColumn(...)`: Adds a column to the table
- `removeColumn(...)`: Removes a column
- `insertRecord(...)`: Inserts a new record
- `selectRecords(...)`: Retrieves records based on criteria
- `updateRecords(...)`: Updates records
- `deleteRecords(...)`: Deletes records
- `drop()`: Deletes the table

### Column

Defines a table column with name, data type, and constraints.

**Key Properties**:
- Name
- Data type
- Primary key flag
- Unique constraint flag
- Not null constraint flag

**Key Methods**:
- `validateValue(...)`: Validates a value against the column's constraints
- `checkAndAddUniqueValue(...)`: Checks and enforces uniqueness

### QueryParser

Parses and executes SQL-like queries.

**Key Methods**:
- `parseQuery(...)`: Main entry point for query parsing
- Specialized parsers for each type of query (CREATE TABLE, SELECT, etc.)

## SQL Command Syntax

> **⚠️ IMPORTANT:** SQL commands in this system should NOT end with semicolons. If a command includes a semicolon, it will result in a syntax error.

### Database Operations

```sql
-- Create a new database
CREATE DATABASE dbName

-- Drop a database
DROP DATABASE dbName

-- Use a database
USE dbName

-- Exit current database
EXIT
```

### Table Operations

```sql
-- Create a table
CREATE TABLE tableName (
    column1 TYPE1 [PRIMARY KEY] [UNIQUE] [NOT NULL],
    column2 TYPE2 [CONSTRAINTS],
    ...
)

-- Drop a table
DROP TABLE tableName

-- Add a column
ALTER TABLE tableName ADD columnName TYPE [CONSTRAINTS]

-- Drop a column
ALTER TABLE tableName DROP columnName

-- Show table schema
DESC tableName

-- Show all tables
SHOW TABLES
```

### Data Operations

```sql
-- Insert data
INSERT INTO tableName (column1, column2, ...) VALUES (value1, value2, ...)

-- Select data
SELECT column1, column2, ... FROM tableName [WHERE conditions]
SELECT * FROM tableName [WHERE conditions]

-- Update data
UPDATE tableName SET column1=value1, column2=value2, ... [WHERE conditions]

-- Delete data
DELETE FROM tableName [WHERE conditions]
```

### WHERE Clause Syntax

The WHERE clause supports the following operators:

- `=`: Equal to
- `!=`: Not equal to
- `<>`: Not equal to (standard SQL syntax)
- `>`: Greater than
- `<`: Less than
- `>=`: Greater than or equal to
- `<=`: Less than or equal to
- `AND`: Logical AND
- `OR`: Logical OR
- `NOT`: Logical NOT

Examples:
```sql
-- Simple comparison
WHERE age > 25

-- Logical operators with proper precedence
WHERE age > 25 AND salary <= 75000

-- Complex conditions
WHERE (age > 25 AND department = 'Sales') OR (experience >= 5 AND status = 'Active')

-- NOT operator
WHERE NOT (age < 18)

-- NULL checks
WHERE email IS NULL
WHERE phone IS NOT NULL
```

#### Logical Operator Behavior

- `AND`: Returns true (1) if both operands are non-zero and not NULL, false (0) if one or more operands are 0, otherwise NULL.
- `OR`: Returns true (1) if any operand is non-zero, false (0) if both are 0, NULL if one operand is NULL and the other is 0.
- `NOT`: Negates a condition. Returns true (1) if operand is false (0), false (0) if operand is true (non-zero), NOT NULL returns NULL.

Expressions are evaluated according to operator precedence, with AND having higher precedence than OR. Use parentheses to explicitly control evaluation order in complex conditions.

## Recent Improvements

The system has recently been enhanced with:

1. **BIGINT Data Type**: Support for large integers exceeding the 32-bit INT limit
2. **Case-Insensitive Behavior**: All commands and string comparisons are now case-insensitive
3. **EXIT Command**: Added to exit the current database without closing the application
4. **Platform Fixes**: 
   - Improved Windows path handling
   - Fixed directory creation/deletion on all platforms
   - Better error handling for file operations
5. **Robustness**: Added additional validation and error handling
6. **No Semicolons**: Updated to reject commands containing semicolons to improve syntax consistency

## Limitations and Future Work

Current limitations:

1. **No Foreign Keys**: The system doesn't support foreign key relationships between tables
2. **No Joins**: No support for table JOIN operations
3. **Limited Data Types**: Only basic data types are supported (no FLOAT, DECIMAL, etc.)
4. **No Indexing**: No index support for faster queries

Potential future enhancements:

1. Add support for more data types (FLOAT, DECIMAL, BOOLEAN)
2. Implement basic JOIN operations
3. Implement indexes for faster queries
4. Add support for aggregate functions (COUNT, SUM, AVG)
5. Implement more SQL commands (GROUP BY, HAVING, ORDER BY, etc.)

## Troubleshooting and Debugging

### Common Issues

1. **Syntax Errors with Semicolons**:
   - Cause: Using semicolons at the end of commands
   - Solution: Remove all semicolons from commands

2. **Permission Errors**:
   - Cause: Insufficient permissions to create/write files
   - Solution: Run the application with appropriate permissions or change the target directory

3. **Path Issues**:
   - Cause: Invalid or inaccessible paths
   - Solution: Ensure paths are valid and accessible

4. **Data Validation Errors**:
   - Cause: Data violates constraints (primary key, unique, not null)
   - Solution: Ensure data complies with table constraints

### Debugging Tips

1. Check error messages for specific details
2. Verify file and directory permissions
3. Examine the database directories to ensure files are created/updated correctly
4. For development: Add debug output to track command execution flow

## Conclusion

This RelictDB system provides a lightweight yet functional implementation of database operations with persistent storage. It's designed for educational purposes and small applications. While it lacks some features of full-fledged database systems, it demonstrates the core concepts of database management systems and SQL query processing. 