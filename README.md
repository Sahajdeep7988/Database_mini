# SQL-like Database System

A simple SQL-like database system implemented in C++ that handles basic database operations with persistence. The system supports multiple databases, tables, and various SQL commands.

## Author
**Sahajdeep Singh**  
Email: sahajdeepsingh404@gmail.com  
Mobile: +91 7988168548

## Features

### Data Types
- **INT**: For integer values
- **BIGINT**: For large integer values (>9 digits)
- **STRING**: For text values
- **DATE**: For date values (YYYY-MM-DD format)

### Database Commands
- **CREATE DATABASE dbName**: Creates a new database
- **DROP DATABASE dbName**: Deletes an existing database
- **USE dbName**: Selects a database to work with
- **EXIT**: Exits the current database (returns to no database selected)
- **SHOW TABLES**: Lists all tables in the current database

### Table Commands
- **CREATE TABLE tableName (col1 TYPE1 [CONSTRAINTS], ...)**: Creates a new table
- **DROP TABLE tableName**: Deletes a table
- **ALTER TABLE tableName ADD columnName TYPE [CONSTRAINTS]**: Adds a column
- **ALTER TABLE tableName DROP columnName**: Removes a column
- **DESC tableName**: Shows table schema

### Data Manipulation
- **INSERT INTO tableName (col1, col2, ...) VALUES (val1, val2, ...)**: Inserts data
- **SELECT col1, col2, ... FROM tableName [WHERE conditions]**: Retrieves data
- **UPDATE tableName SET col1=val1, col2=val2, ... [WHERE conditions]**: Updates data
- **DELETE FROM tableName [WHERE conditions]**: Deletes data

### Constraints
- **PRIMARY KEY**: Unique identifier for a row
- **UNIQUE**: Values must be unique
- **NOT NULL**: Values cannot be null

### Special Commands
- **.help**: Displays help information
- **.databases**: Lists all databases
- **.tables**: Lists tables in the current database
- **.exit**: Exits the program

## Key Features
- **Case-insensitive commands**: All SQL commands are case-insensitive
- **Persistent storage**: Databases and tables are stored on disk
- **Multiple database support**: Create and manage multiple databases
- **Error handling**: Robust error handling and reporting
- **Cross-platform**: Works on Windows and Unix-based systems

## Building and Running

### Prerequisites
- C++17 compatible compiler (GCC, Clang, MSVC)

### Build
```bash
# On Linux/Mac
g++ -std=c++17 src/main.cpp -o sqldb -I include/

# On Windows with MSVC
cl /std:c++17 /EHsc /Iinclude src/main.cpp /Fesqldb.exe

# On Windows with MinGW
g++ -std=c++17 src/main.cpp -o sqldb.exe -I include/
```

### Run
```bash
# On Linux/Mac
./sqldb

# On Windows
sqldb.exe
```

## Detailed Usage Guide

### Getting Started
1. Start the application by running the executable
2. The welcome screen will display available commands
3. Create a database with `CREATE DATABASE mydatabase;`
4. Select the database with `USE mydatabase;`
5. Create tables and manipulate data using SQL-like commands

### Database Operations
```sql
-- Create a new database
CREATE DATABASE school;

-- Switch to the database
USE school;

-- List all databases
.databases

-- Exit the current database (return to main prompt)
EXIT;

-- Delete a database
DROP DATABASE school;
```

### Table Operations
```sql
-- Create a table with various data types and constraints
CREATE TABLE students (
    id INT PRIMARY KEY,
    name STRING NOT NULL,
    age INT,
    enrollment_date DATE,
    tuition_fee BIGINT
);

-- Show all tables in current database
SHOW TABLES;

-- View table structure
DESC students;

-- Add a column to an existing table
ALTER TABLE students ADD email STRING UNIQUE;

-- Remove a column
ALTER TABLE students DROP email;

-- Delete a table
DROP TABLE students;
```

### Data Operations
```sql
-- Insert data
INSERT INTO students (id, name, age, enrollment_date, tuition_fee) 
VALUES (1, 'John Smith', 20, '2023-09-15', 125000);

-- Insert multiple records (run as separate commands)
INSERT INTO students (id, name, age, enrollment_date, tuition_fee) 
VALUES (2, 'Jane Doe', 22, '2023-08-10', 125000);

-- Query data
SELECT * FROM students;
SELECT name, age FROM students;
SELECT * FROM students WHERE age > 21;
SELECT * FROM students WHERE name = 'John Smith';

-- Update data
UPDATE students SET age = 21 WHERE id = 1;

-- Delete data
DELETE FROM students WHERE id = 2;
```

### Advanced Queries
```sql
-- Combine conditions with AND/OR
SELECT * FROM students WHERE age > 20 AND tuition_fee = 125000;

-- Using comparison operators
SELECT * FROM students WHERE enrollment_date < '2023-09-01';
```

## Implementation Details

- Databases are stored as directories with a "data" subdirectory
- Tables are stored as pairs of files: .meta (schema) and .data (records)
- Commands are parsed using regex patterns for flexibility
- Cross-platform path handling for Windows and Unix systems

## Project Structure

```
├── CMakeLists.txt          # Build configuration
├── README.md               # Project documentation
├── DOCUMENTATION.md        # Detailed technical documentation
├── databases/              # Data storage directory (created at runtime)
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

## Troubleshooting

1. **Permission Errors**: If you encounter permission errors when creating databases, ensure you have write permissions in the directory.

2. **Database Not Found**: Make sure you create a database before trying to use it with the `USE` command.

3. **Invalid Command**: All SQL commands must end with a semicolon (;). Special dot commands like `.help` do not require a semicolon.

4. **Syntax Errors**: Make sure your SQL commands follow the correct syntax as shown in the examples.

## See Also

For more detailed information about the implementation and architecture, please see the [DOCUMENTATION.md](DOCUMENTATION.md) file. 