# SQL-like Database System - Usage Notes

## Important Syntax Notes

### SQL Command Syntax

This system uses a SQL-like syntax but with an important difference: commands should NOT end with semicolons. Adding semicolons at the end of commands will result in syntax errors.

**Examples:**
```
CREATE DATABASE testdb
USE testdb
CREATE TABLE users (id INT PRIMARY KEY, name STRING)
BEGIN TRANSACTION
INSERT INTO users (id, name) VALUES (1, 'User1')
COMMIT
```

### Transaction Support

The system supports full ACID-compliant transactions:

1. **BEGIN TRANSACTION** (or simply **BEGIN**): Start a new transaction
2. **COMMIT**: Save all changes made in the current transaction 
3. **ROLLBACK**: Discard all changes made in the current transaction

When a transaction is active, the prompt will show "(TRANSACTION)" to indicate this state.

### Storage Engine

The system uses a B-Tree based storage engine with the following features:

- Tables are stored in a single `.table` file format
- Buffer pool caching for improved performance
- Transaction logging for durability and recovery
- Data flushing with the `.flush` command

### Special Commands

- `.help`: Display help information
- `.databases`: List all databases
- `.tables`: List tables in the current database
- `.flush`: Manually flush all data to disk
- `.exit`: Exit the program

## Troubleshooting

If you encounter issues with commands not being recognized:

1. Make sure you haven't included semicolons at the end of your commands
2. Check that you are connected to a database (use the `USE dbname` command)
3. Verify command spelling and capitalization (though commands are case-insensitive)
4. If a transaction fails, use `ROLLBACK` to reset the transaction state

## Example Session

```
> CREATE DATABASE testdb
Creating database 'testdb'...
Database 'testdb' created successfully.

> USE testdb
Using database 'testdb'.

testdb> CREATE TABLE users (id INT PRIMARY KEY, name STRING)
Table 'users' created successfully

testdb> BEGIN TRANSACTION
Transaction started

testdb (TRANSACTION)> INSERT INTO users (id, name) VALUES (1, 'User1')
Record inserted successfully

testdb (TRANSACTION)> COMMIT
Transaction committed

testdb> SELECT * FROM users
    | id       | name      |
    +----------+-----------+
    | 1        | User1     |

1 row(s) returned.

testdb> .flush
Data flushed to disk successfully

testdb> EXIT
Exited database 'testdb'.

> .exit
Goodbye! 