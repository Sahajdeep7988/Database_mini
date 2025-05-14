-- Test file for semicolon support
CREATE DATABASE semicolondb;
USE semicolondb;

-- Create a simple table
CREATE TABLE users (id INT PRIMARY KEY, name STRING);

-- Insert data with semicolons
INSERT INTO users (id, name) VALUES (1, 'User1');
INSERT INTO users (id, name) VALUES (2, 'User2');

-- Select with semicolon
SELECT * FROM users;

-- Test transaction with semicolons
BEGIN TRANSACTION;
INSERT INTO users (id, name) VALUES (3, 'User3');
COMMIT;

-- Verify transaction
SELECT * FROM users;

-- Clean up
EXIT; 