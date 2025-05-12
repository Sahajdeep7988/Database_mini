# SQL-like Database System

A simple SQL-like database system implemented in C++ that handles basic database operations with persistence. The system supports multiple databases, tables, and various SQL commands.

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
g++ -std=c++17 src/main.cpp -o sqldb -I include/
```

### Run
```bash
./sqldb
```

## Example Usage

```sql
CREATE DATABASE testdb;
USE testdb;
CREATE TABLE users (id INT PRIMARY KEY, name STRING, age INT, salary BIGINT);
INSERT INTO users (id, name, age, salary) VALUES (1, 'John', 30, 75000);
INSERT INTO users (id, name, age, salary) VALUES (2, 'Jane', 28, 85000);
SELECT * FROM users WHERE age > 25;
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
├── data/                   # Data storage directory
├── include/                # Header files
│   ├── AggregateFunction.h # Aggregate functions (COUNT, SUM, etc.)
│   ├── Column.h            # Column class
│   ├── DataType.h          # Data type definitions
│   ├── DatabaseManager.h   # Database manager
│   ├── QueryParser.h       # SQL query parser
│   ├── StringFunction.h    # String functions (UPPER, LOWER, etc.)
│   └── Table.h             # Table class
└── src/                    # Source files
    └── main.cpp            # Main application
```

## Limitations

- No support for foreign keys or joins
- No transaction support
- Limited error handling
- Basic SQL syntax support only

## License

This project is provided for educational purposes only. 