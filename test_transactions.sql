-- Test Transaction Support in SQL-like Database System
-- This script tests the basic functionality of the database system with transaction support

-- Create and use a test database
CREATE DATABASE transactiondb;
USE transactiondb;

-- Create a sample table
CREATE TABLE employees (
    id INT PRIMARY KEY,
    name STRING NOT NULL,
    department STRING,
    salary BIGINT
);

-- Insert some initial records
INSERT INTO employees (id, name, department, salary) VALUES (1, 'John Doe', 'Engineering', 90000);
INSERT INTO employees (id, name, department, salary) VALUES (2, 'Jane Smith', 'Marketing', 85000);
INSERT INTO employees (id, name, department, salary) VALUES (3, 'Bob Johnson', 'Engineering', 92000);

-- Query the data to verify insertion
SELECT * FROM employees;

-- Test transaction with COMMIT
-- Begin a transaction
BEGIN TRANSACTION;

-- Insert a new record
INSERT INTO employees (id, name, department, salary) VALUES (4, 'Alice Brown', 'HR', 78000);

-- Update an existing record
UPDATE employees SET salary = 95000 WHERE id = 1;

-- Commit the transaction
COMMIT;

-- Verify that changes were committed
SELECT * FROM employees;

-- Test transaction with ROLLBACK
-- Begin a transaction
BEGIN TRANSACTION;

-- Insert a record that will be rolled back
INSERT INTO employees (id, name, department, salary) VALUES (5, 'Chris Wilson', 'Finance', 88000);

-- Update a record that will be rolled back
UPDATE employees SET department = 'Research' WHERE id = 2;

-- Rollback the transaction
ROLLBACK;

-- Verify that changes were rolled back
SELECT * FROM employees;

-- Test data persistence with flush command
.flush

-- Exit the database and program
EXIT;
.exit 