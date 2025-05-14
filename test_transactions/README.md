# Transaction Testing

This directory contains transaction-related test files and logs for the SQL-like Database System.

## How Transactions Work

Transactions in this database system follow the ACID properties (Atomicity, Consistency, Isolation, Durability) to ensure data integrity.

### Using Transactions

1. Begin a transaction with the `BEGIN TRANSACTION` or `BEGIN` command
2. Execute your operations (INSERT, UPDATE, DELETE)
3. Commit changes with the `COMMIT` command, or discard them with the `ROLLBACK` command

### Example

```
BEGIN TRANSACTION
INSERT INTO employees (id, name, department, salary) VALUES (1, 'John Doe', 'Engineering', 90000)
UPDATE departments SET employee_count = employee_count + 1 WHERE name = 'Engineering'
COMMIT
```

### Notes

- Remember that SQL commands in this system should NOT end with semicolons
- The transaction manager creates log files in this directory to ensure durability
- Use the `.flush` command to ensure all data is written to disk

## Implementation Details

The transaction system is implemented using a write-ahead logging (WAL) mechanism, with the following components:

1. **TransactionManager**: Handles transaction states and operations
2. **BufferPoolManager**: Manages in-memory page caching with proper locking
3. **StorageEngine**: Coordinates transaction operations across tables

Refer to the main documentation for more details on the implementation. 