CREATE DATABASE testdb;
USE testdb;
CREATE TABLE users (id INT PRIMARY KEY, name STRING, age INT, active BIGINT);
INSERT INTO users (id, name, age, active) VALUES (1, 'User1', 25, 1);
INSERT INTO users (id, name, age, active) VALUES (2, 'User2', 30, 0);
INSERT INTO users (id, name, age, active) VALUES (3, 'User3', 22, 1);
INSERT INTO users (id, name, age, active) VALUES (4, 'User4', 35, 1);

-- Simple select to show all records
SELECT * FROM users;

-- Test AND logical operator
SELECT * FROM users WHERE age > 25 AND active = 1;

-- Test OR logical operator
SELECT * FROM users WHERE age < 25 OR active = 0;

-- Test complex condition with AND and OR
SELECT * FROM users WHERE (age > 25 AND active = 1) OR name = 'User3';

EXIT; 