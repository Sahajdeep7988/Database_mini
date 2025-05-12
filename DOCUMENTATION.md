# SQL-like Database System - Implementation Documentation

## Overview

This document provides detailed information about the implementation of our SQL-like database system, created as an end-semester OOPs project. The system is implemented in C++17 and provides a command-line interface for SQL-like operations on data stored in plain text files.

## Core Components

### 1. Database System (DatabaseSystem.h)

The database system manages multiple databases:

- Creates, deletes, and switches between databases
- Maintains a current active database
- Organizes databases in directories on the file system

Implementation details:
- Uses a hash map to store database managers
- Each database has its own data directory
- Scans the base directory to load existing databases

### 2. Data Types (DataType.h)

The database supports four data types with validation:

- **INT**: Integer values
  - Validated using `std::stoi()`
- **BIGINT**: Large integer values (more than 9 digits)
  - Validated using `std::stoll()`
- **STRING**: Text values (no specific validation required)
- **DATE**: Date values in YYYY-MM-DD format
  - Validated using regex pattern and range checks for months and days
  - Includes leap year validation for February

Implementation details:
- The `Type` enum class defines the available types
- The `DataType` class provides validation methods for each type
- Type conversion methods between string representations and enum values

### 3. Columns (Column.h)

Columns represent the structure of tables with the following features:

- Support for constraints: PRIMARY KEY, UNIQUE, NOT NULL
- Validation of values against data types and constraints
- Tracking of unique values to enforce uniqueness

Implementation details:
- Constraints are tracked using boolean flags
- A set of unique values is maintained for columns with UNIQUE or PRIMARY KEY constraints
- Value validation is performed against both data type and constraint rules

### 4. Tables (Table.h)

Tables manage collections of columns and records with the following operations:

- Creation, deletion, and modification of table structure
- Insertion, selection, updating, and deletion of records
- Persistence to text files in the data/ directory

Implementation details:
- Tables are stored in two files:
  - `tableName.meta`: Stores column definitions and constraints
  - `tableName.data`: Stores the actual records as comma-separated values
- Records are stored in memory as vectors of strings
- Conditions for WHERE clauses are represented as a linked structure of condition objects

### 5. Database Manager (DatabaseManager.h)

The database manager handles:

- Creation and management of tables within a database
- Loading tables from disk on startup
- Providing access to tables for query operations

Implementation details:
- Uses a hash map to store and retrieve tables by name
- Scans the data directory to load existing tables
- Manages table lifecycle including creation and deletion

### 6. Query Parser (QueryParser.h)

The query parser interprets SQL-like commands:

- Parses and validates SQL syntax
- Extracts parameters and conditions
- Forwards requests to the appropriate table methods

Implementation details:
- Regular expressions are used for parsing SQL commands
- Recursive descent for parsing complex conditions with AND/OR operators
- Results are returned through a `QueryResult` structure

### 7. String and Aggregate Functions

The system includes utility functions:

- **String Functions** (StringFunction.h):
  - UPPER(): Convert text to uppercase
  - LOWER(): Convert text to lowercase
  - LENGTH(): Get string length
  - SUBSTRING(): Extract part of a string

- **Aggregate Functions** (AggregateFunction.h):
  - COUNT(): Count non-empty values
  - SUM(): Add numeric values
  - AVG(): Calculate average of numeric values
  - MIN(): Find minimum value
  - MAX(): Find maximum value

## SQL Command Implementation

### Database Commands

#### 1. CREATE DATABASE

```sql
CREATE DATABASE dbName;
```

Implementation:
- Creates a new directory structure for the database
- Initializes a DatabaseManager for the new database

#### 2. DROP DATABASE

```sql
DROP DATABASE dbName;
```

Implementation:
- Removes the database from the system
- (Note: Currently doesn't physically delete database files)

#### 3. USE DATABASE

```sql
USE dbName;
```

Implementation:
- Sets the current active database
- All subsequent table operations apply to this database

#### 4. EXIT

```sql
EXIT;
```

Implementation:
- Exits the current database, returning to the main SQL prompt
- Clears the current database selection

### Table Commands

#### 1. CREATE TABLE

```sql
CREATE TABLE tableName (
    column1 TYPE1 [CONSTRAINTS],
    column2 TYPE2 [CONSTRAINTS],
    ...
);
```

Implementation:
- Parses column definitions using regex
- Validates data types and constraints
- Creates table in memory and initializes data files

#### 2. DROP TABLE

```sql
DROP TABLE tableName;
```

Implementation:
- Removes table from memory
- Deletes associated data files

#### 3. ALTER TABLE

```sql
ALTER TABLE tableName ADD columnName TYPE [CONSTRAINTS];
ALTER TABLE tableName DROP columnName;
```

Implementation:
- For ADD: Adds new column to table structure and updates records with empty values
- For DROP: Removes column from table structure and updates records

#### 4. INSERT

```sql
INSERT INTO tableName (column1, column2, ...) VALUES (value1, value2, ...);
```

Implementation:
- Validates column names
- Validates values against data types and constraints
- Adds record to table and updates data file

#### 5. SELECT

```sql
SELECT column1, column2, ... FROM tableName [WHERE conditions];
```

Implementation:
- Supports column selection or * for all columns
- WHERE clause with comparison operators (=, !=, <, >, <=, >=)
- Logical operators (AND, OR) for complex conditions

#### 6. UPDATE

```sql
UPDATE tableName SET column1=value1, column2=value2, ... [WHERE conditions];
```

Implementation:
- Updates specified columns in records matching the WHERE condition
- Validates new values against data types and constraints

#### 7. DELETE

```sql
DELETE FROM tableName [WHERE conditions];
```

Implementation:
- Removes records matching the WHERE condition
- Updates unique value tracking for unique columns

#### 8. DESC

```sql
DESC tableName;
```

Implementation:
- Displays detailed information about the table's structure
- Shows column names, data types, and constraints

#### 9. SHOW TABLES

```sql
SHOW TABLES;
```

Implementation:
- Lists all tables in the current database
- Equivalent to the special command `.tables`

## Object-Oriented Design Principles

The following OOP principles are applied in the implementation:

### 1. Encapsulation

- Each class has well-defined responsibilities
- Private members are used to hide implementation details
- Public interfaces provide controlled access to functionality

### 2. Inheritance

- The class hierarchy reflects the is-a relationship between concepts

### 3. Polymorphism

- Type-specific behavior is implemented through virtual methods

### 4. Abstraction

- Complex implementation details are hidden behind simple interfaces
- Classes represent abstract concepts (tables, columns, etc.)

### 5. Modularity

- The system is divided into separate modules with specific responsibilities
- Each header file focuses on a single concept

## Error Handling

The system provides error handling through:

- Validation of input values
- Checking of constraints before modifications
- Exception handling for runtime errors
- Meaningful error messages through the QueryResult structure

## File Structure

```
├── CMakeLists.txt          # Build configuration
├── README.md               # User documentation
├── DOCUMENTATION.md        # Implementation details (this file)
├── databases/              # Base directory for all databases
│   ├── [dbName]/           # Database directory
│   │   └── data/           # Data storage directory
│   │       ├── [tableName].meta  # Table metadata
│   │       └── [tableName].data  # Table records
├── include/                # Header files
│   ├── AggregateFunction.h # Aggregate functions (COUNT, SUM, etc.)
│   ├── Column.h            # Column class
│   ├── DataType.h          # Data type definitions
│   ├── DatabaseManager.h   # Database manager
│   ├── DatabaseSystem.h    # Database system for managing databases
│   ├── QueryParser.h       # SQL query parser
│   ├── StringFunction.h    # String functions (UPPER, LOWER, etc.)
│   └── Table.h             # Table class
└── src/                    # Source files
    └── main.cpp            # Main application and command-line interface
```

## Data Persistence

### Database Organization

Databases are organized as directories:

1. **Base Directory** (`databases/`):
   - Contains subdirectories for each database

2. **Database Directories** (`databases/dbName/`):
   - Contains a `data/` subdirectory for tables

3. **Table Files**:
   - **Metadata Files** (`databases/dbName/data/tableName.meta`):
     - Each line contains one column definition
     - Format: `columnName,dataType,primaryKey,unique,notNull`
     - PK = Primary Key, UQ = Unique, NN = Not Null

   - **Data Files** (`databases/dbName/data/tableName.data`):
     - Each line contains one record
     - Values are comma-separated
     - Order matches the column order in the metadata file

## Performance Considerations

This implementation is focused on educational purposes rather than performance:

- In-memory storage with persistence to text files
- Linear search for record operations
- No indexing for faster retrieval
- No optimization for large datasets

## Example Use Cases

### Multiple Database Management

```sql
-- Create databases
CREATE DATABASE school;
CREATE DATABASE store;

-- Switch to school database
USE school;

-- Create tables in school database
CREATE TABLE students (
    id INT PRIMARY KEY,
    name STRING NOT NULL,
    age INT
);

-- View tables in current database
SHOW TABLES;

-- Get details of a table
DESC students;

-- Exit the school database
EXIT;

-- Switch to store database
USE store;

-- Create tables in store database
CREATE TABLE products (
    product_id INT PRIMARY KEY,
    name STRING NOT NULL,
    price INT
);
```

### Student Database

```sql
-- Create a table for students
CREATE TABLE students (
    id INT PRIMARY KEY,
    name STRING NOT NULL,
    age INT,
    email STRING UNIQUE,
    enrollment_date DATE
);

-- Insert student records
INSERT INTO students (id, name, age, email, enrollment_date)
VALUES (1, 'John Doe', 20, 'john@example.com', '2023-01-15');

INSERT INTO students (id, name, age, email, enrollment_date)
VALUES (2, 'Jane Smith', 22, 'jane@example.com', '2023-02-10');

-- Query students
SELECT * FROM students WHERE age > 21;

-- Update student information
UPDATE students SET age = 21 WHERE id = 1;

-- Delete student
DELETE FROM students WHERE id = 2;
```

### Product Inventory with BIGINT

```sql
-- Create a product table
CREATE TABLE products (
    product_id BIGINT PRIMARY KEY,
    name STRING NOT NULL,
    category STRING,
    price INT NOT NULL,
    stock_quantity INT
);

-- Insert products with large IDs
INSERT INTO products (product_id, name, category, price, stock_quantity)
VALUES (9876543210, 'Laptop', 'Electronics', 1200, 10);

-- Query products
SELECT name, price FROM products WHERE stock_quantity < 5;

-- Update stock
UPDATE products SET stock_quantity = 8 WHERE product_id = 9876543210;
```

## Special Commands

The system provides several special dot commands:

- `.help`: Display help message
- `.databases`: List all databases in the system
- `.tables`: List all tables in the current database
- `.exit`: Exit the program

## Future Enhancements

Possible enhancements for future versions:

1. Support for JOIN operations between tables
2. Indexing for faster data retrieval
3. Transaction support with COMMIT/ROLLBACK
4. Foreign key constraints
5. More advanced data types (FLOAT, BOOLEAN, etc.)
6. Support for user-defined functions
7. Better error handling and reporting
8. Physical deletion of dropped databases
9. Database backup and restore commands
10. Query optimization

## Conclusion

This SQL-like database system demonstrates the application of object-oriented programming principles in C++. It provides a solid foundation for understanding database concepts and can be extended for more complex functionality. The multi-database support allows for better organization of data and more closely resembles professional database management systems like MySQL. 