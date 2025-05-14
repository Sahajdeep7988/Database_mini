# SQL-like Database System - Usage Guide

## Important Syntax Note

> **⚠️ IMPORTANT:** This database system does NOT support semicolons at the end of SQL commands. Adding semicolons will result in syntax errors. This differs from standard SQL, so please remember to omit all semicolons.

## Getting Started

### Installing and Running

1. **Installation**:
   ```bash
   # Clone the repository
   git clone https://github.com/Sahajdeep7988/Database_mini.git
   cd Database_mini

   # Compile the program
   g++ -std=c++17 src/main.cpp src/storage_engine/*.cpp -o sqldb -I include/
   ```

2. **Running the Application**:
   ```bash
   # Linux/macOS
   ./sqldb

   # Windows
   sqldb.exe
   ```

## Basic Usage

When you start the application, you'll see a prompt:
```
Welcome to the SQL-like Database System
Type '.help' for usage information or '.exit' to quit
>
```

### Special Commands

Special commands start with a dot (`.`):

| Command | Description |
|---------|-------------|
| `.help` | Display help information |
| `.databases` | List all databases |
| `.tables` | List tables in the current database |
| `.flush` | Flush all data to disk |
| `.exit` | Exit the program |

## Working with Databases

### Creating a Database

```
> CREATE DATABASE school
Creating database 'school'...
Database 'school' created successfully.
```

### Selecting a Database

```
> USE school
Using database 'school'.
```

The prompt will change to show the current database:
```
school>
```

### Exiting a Database

To exit the current database (but stay in the application):
```
school> EXIT
Exited database 'school'.
>
```

### Dropping a Database

```
> DROP DATABASE school
Database 'school' dropped successfully.
```

## Working with Tables

### Creating a Table

```
school> CREATE TABLE students (
  id INT PRIMARY KEY,
  name STRING NOT NULL,
  age INT,
  grade FLOAT
)
Table 'students' created successfully
```

### Listing Tables

```
school> SHOW TABLES
Tables in database 'school':
  - students
```

### Viewing Table Structure

```
school> DESC students
Schema for table 'students':
+----------------+----------+-------------+
| Column Name    | Type     | Constraints |
+----------------+----------+-------------+
| id             | INT      | PRIMARY KEY |
| name           | STRING   | NOT NULL    |
| age            | INT      |             |
| grade          | FLOAT    |             |
+----------------+----------+-------------+
```

### Modifying Tables

```
# Adding a column
school> ALTER TABLE students ADD email STRING UNIQUE

# Removing a column
school> ALTER TABLE students DROP email
```

### Dropping a Table

```
school> DROP TABLE students
Table 'students' dropped successfully
```

## Data Operations

### Inserting Data

```
school> INSERT INTO students (id, name, age) VALUES (1, 'John Doe', 18)
Record inserted successfully
```

### Querying Data

```
# Select all columns and rows
school> SELECT * FROM students

# Select specific columns
school> SELECT name, age FROM students

# Select with conditions
school> SELECT * FROM students WHERE age > 18
```

### Updating Data

```
school> UPDATE students SET age = 19 WHERE id = 1
Updated 1 record(s)
```

### Deleting Data

```
school> DELETE FROM students WHERE id = 1
Deleted 1 record(s)
```

## Using WHERE Conditions

The WHERE clause supports these operators:

| Operator | Description | Example |
|----------|-------------|---------|
| `=` | Equals | `WHERE name = 'John'` |
| `!=`, `<>` | Not equals | `WHERE age != 20` |
| `>` | Greater than | `WHERE age > 18` |
| `<` | Less than | `WHERE grade < 3.5` |
| `>=` | Greater than or equal | `WHERE age >= 21` |
| `<=` | Less than or equal | `WHERE grade <= 4.0` |

### Logical Operators

You can combine conditions with:

| Operator | Description | Example |
|----------|-------------|---------|
| `AND` | Both conditions must be true | `WHERE age > 18 AND grade > 3.0` |
| `OR` | Either condition can be true | `WHERE grade > 3.5 OR attendance > 90` |

Example of complex conditions:
```
school> SELECT * FROM students WHERE (age >= 18 AND grade > 3.0) OR name = 'John Smith'
```

## Transaction Support

The system supports transactions for grouping operations:

```
school> BEGIN TRANSACTION
Transaction started

school (TRANSACTION)> INSERT INTO students (id, name, age) VALUES (2, 'Jane Smith', 19)
Record inserted successfully

school (TRANSACTION)> UPDATE courses SET enrolled = enrolled + 1 WHERE id = 101
Updated 1 record(s)

school (TRANSACTION)> COMMIT
Transaction committed
```

If something goes wrong, you can discard all changes:
```
school (TRANSACTION)> ROLLBACK
Transaction rolled back
```

## Error Handling

### Common Errors

1. **Syntax Error (Semicolon)**:
   ```
   school> SELECT * FROM students;
   Unrecognized command: SELECT * FROM students;
   ```
   Solution: Remove the semicolon.

2. **Table Doesn't Exist**:
   ```
   school> SELECT * FROM non_existent_table
   Error: Table 'non_existent_table' does not exist
   ```

3. **Constraint Violation**:
   ```
   school> INSERT INTO students (id, name) VALUES (1, NULL)
   Error: Failed to insert record - NOT NULL constraint violation for column 'name'
   ```

## Best Practices

1. **Always back up your data**: Use the `.flush` command regularly
2. **Use transactions** for operations that should be atomic
3. **Avoid semicolons** at the end of commands
4. **Check constraints** before inserting or updating data
5. **Use quotes** for string values: `'string value'`

## Troubleshooting

If you encounter issues:

1. Check that you're connected to a database (use `USE dbname`)
2. Verify your command syntax (NO semicolons!)
3. Check error messages for specific details
4. If in doubt, use `.help` to see available commands 