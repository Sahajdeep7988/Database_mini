# SQL-like Database System

A comprehensive SQL-like database system implemented in C++ that handles database operations with persistence. This system provides a lightweight, standalone database solution with support for multiple databases, tables, SQL commands, and ACID-compliant transactions.

## Author
**Sahajdeep Singh**  
Email: sahajdeepsingh404@gmail.com  
Mobile: +91 7988168548

## Overview

This C++ database system provides SQL-like functionality with the following features:

- Multiple database support
- Table creation and management
- Data types with validation
- SQL command processing
- ACID-compliant transactions
- B-Tree indexing
- Buffer pool management
- File-based persistence

> **⚠️ IMPORTANT:** Unlike standard SQL, commands in this system should NOT end with semicolons.  
> Adding semicolons will result in syntax errors.

## Features

### Data Types
- **INT**: Integer values
- **BIGINT**: Large integer values
- **STRING**: Text values
- **DATE**: Date values in YYYY-MM-DD format

### Database Operations
- Create, use, and drop databases
- Create, alter, and drop tables
- Insert, update, delete, and select data
- Transaction support (BEGIN/COMMIT/ROLLBACK)

### Special Features
- Case-insensitive commands
- Persistent data storage
- Constraint validation (PRIMARY KEY, UNIQUE, NOT NULL)
- Transaction support for atomicity and consistency
- Advanced WHERE clause conditions with logical operators

## Installation

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10+ (recommended for building)

### Building from Source

#### Using CMake (Recommended)

```bash
# Clone the repository
git clone https://github.com/Sahajdeep7988/Database_mini.git
cd Database_mini

# Create build directory
mkdir build && cd build

# Generate build files
cmake ..

# Build the project
cmake --build .
```

#### Manual Build

```bash
# For Linux/macOS
g++ -std=c++17 src/main.cpp src/storage_engine/*.cpp -o sqldb -I include/

# For Windows (MinGW)
g++ -std=c++17 src/main.cpp src/storage_engine/*.cpp -o sqldb.exe -I include/

# For Windows (MSVC)
cl /std:c++17 /EHsc /Iinclude src/main.cpp src/storage_engine/*.cpp /Fesqldb.exe
```

## Usage

### Starting the Application

```bash
# Linux/macOS
./sqldb

# Windows
sqldb.exe
```

### Command Syntax

> **⚠️ IMPORTANT:** Unlike standard SQL, commands in this system should NOT end with semicolons.

### Quickstart Example

```
# Create a database
CREATE DATABASE school

# Use the database
USE school

# Create a table
CREATE TABLE students (id INT PRIMARY KEY, name STRING NOT NULL, age INT)

# Insert data
INSERT INTO students (id, name, age) VALUES (1, 'John Doe', 20)

# Query data
SELECT * FROM students

# Update data
UPDATE students SET age = 21 WHERE id = 1

# Exit database
EXIT

# Exit application
.exit
```

## SQL Commands Reference

### Data Types

| Type | Description | Example |
|------|-------------|---------|
| INT | Integer values | 42 |
| BIGINT | Large integer values | 9876543210 |
| STRING | Text values | 'Hello World' |
| DATE | Date values (YYYY-MM-DD) | '2023-10-15' |

### Database Commands

| Command | Description | Example |
|---------|-------------|---------|
| CREATE DATABASE | Creates a new database | CREATE DATABASE school |
| DROP DATABASE | Deletes a database | DROP DATABASE school |
| USE | Switches to a database | USE school |
| EXIT | Exits the current database | EXIT |

### Table Commands

| Command | Description | Example |
|---------|-------------|---------|
| CREATE TABLE | Creates a new table | CREATE TABLE students (id INT PRIMARY KEY, name STRING) |
| DROP TABLE | Deletes a table | DROP TABLE students |
| ALTER TABLE ADD | Adds a column | ALTER TABLE students ADD email STRING |
| ALTER TABLE DROP | Removes a column | ALTER TABLE students DROP email |
| DESC | Shows table schema | DESC students |
| SHOW TABLES | Lists all tables | SHOW TABLES |

### Data Manipulation Commands

| Command | Description | Example |
|---------|-------------|---------|
| INSERT INTO | Inserts data | INSERT INTO students (id, name) VALUES (1, 'John') |
| SELECT | Retrieves data | SELECT * FROM students WHERE age > 20 |
| UPDATE | Updates data | UPDATE students SET age = 21 WHERE id = 1 |
| DELETE FROM | Deletes data | DELETE FROM students WHERE id = 1 |

### Transaction Commands

| Command | Description | Example |
|---------|-------------|---------|
| BEGIN TRANSACTION | Starts a transaction | BEGIN TRANSACTION |
| COMMIT | Commits changes | COMMIT |
| ROLLBACK | Discards changes | ROLLBACK |

### WHERE Clause and Operators

The system supports various operators in WHERE clauses:

#### Comparison Operators
- `=` (equals)
- `!=` or `<>` (not equals)
- `<` (less than)
- `>` (greater than)
- `<=` (less than or equal)
- `>=` (greater than or equal)

#### Logical Operators
- `AND`: Returns true when both conditions are true
- `OR`: Returns true when either condition is true

#### Examples with Logical Operators

```sql
-- Single condition
SELECT * FROM students WHERE age > 20

-- AND operator
SELECT * FROM students WHERE age > 20 AND department = 'Engineering'

-- OR operator
SELECT * FROM students WHERE age < 20 OR department = 'Art'

-- Combined operators
SELECT * FROM students WHERE (age > 20 AND department = 'Engineering') OR name = 'John'
```

### Special Commands

| Command | Description |
|---------|-------------|
| .help | Displays help |
| .databases | Lists all databases |
| .tables | Lists tables in the current database |
| .flush | Flushes data to disk |
| .exit | Exits the program |

## Constraints

The following constraints can be applied to columns:

- **PRIMARY KEY**: Unique identifier for a row
- **UNIQUE**: Ensures all values in a column are unique
- **NOT NULL**: Prevents NULL/empty values

Example:
```sql
CREATE TABLE employees (
    id INT PRIMARY KEY,
    email STRING UNIQUE,
    name STRING NOT NULL,
    department STRING
)
```

## Advanced Features

### B-Tree Indexing

The system uses B-Tree indexing for efficient data retrieval with logarithmic time complexity, supporting:
- Automatic indexing of PRIMARY KEY columns
- Efficient point queries
- Range queries with optimized performance

### Buffer Pool Management

An LRU-based buffer pool caches database pages in memory, providing:
- Reduced disk I/O
- Efficient page replacement
- Dirty page tracking

### ACID Transactions

Transaction support with ACID properties:
- **Atomicity**: All operations succeed or all fail
- **Consistency**: Data remains in a valid state
- **Isolation**: Transactions don't interfere with each other
- **Durability**: Committed changes persist

Example transaction:
```sql
BEGIN TRANSACTION
INSERT INTO employees (id, name, department) VALUES (1, 'John Doe', 'Engineering')
UPDATE departments SET employee_count = employee_count + 1 WHERE name = 'Engineering'
COMMIT
```

## Project Structure

```
├── CMakeLists.txt          # Build configuration
├── include/                # Header files
│   ├── Column.h            # Column class
│   ├── DataType.h          # Data type definitions
│   ├── DatabaseManager.h   # Database manager
│   ├── DatabaseSystem.h    # Database system
│   ├── QueryParser.h       # SQL query parser
│   ├── Table.h             # Table class
│   └── storage_engine/     # Storage engine components
│       ├── StorageEngine.h
│       ├── BufferPoolManager.h
│       ├── BTreeIndex.h
│       ├── TransactionManager.h
│       └── Value.h         # Value representation
└── src/                    # Source files
    ├── main.cpp            # Application entry point
    └── storage_engine/     # Storage engine implementations
        ├── StorageEngine.cpp
        ├── BufferPoolManager.cpp
        ├── BTreeIndex.cpp
        └── TransactionManager.cpp
```

## Troubleshooting

1. **Permission Errors**: Ensure you have write permissions in the directory where the program is running.

2. **Database Not Found**: Make sure you've created a database with `CREATE DATABASE` before trying to use it.

3. **Command Syntax**: Remember that commands should NOT end with semicolons.

4. **Logical Operator Issues**: When using complex conditions with multiple AND/OR operators, use parentheses to explicitly group conditions.

5. **Build Issues**: Ensure you have a C++17 compatible compiler and all necessary development tools installed.

## License

This project is available under the MIT License. See the LICENSE file for more details.

## Detailed Documentation

For comprehensive documentation including implementation details, architectural decisions, and internal workings, refer to the [help.md](help.md) file included in this repository. 