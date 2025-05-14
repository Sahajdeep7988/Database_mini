CREATE DATABASE transactiondb;
USE transactiondb;

CREATE TABLE employees (
    id INT PRIMARY KEY,
    name STRING NOT NULL,
    department STRING,
    salary BIGINT
);

INSERT INTO employees (id, name, department, salary) VALUES (1, 'John Doe', 'Engineering', 90000);
INSERT INTO employees (id, name, department, salary) VALUES (2, 'Jane Smith', 'Marketing', 85000);
INSERT INTO employees (id, name, department, salary) VALUES (3, 'Bob Johnson', 'Engineering', 92000);

SELECT * FROM employees;

BEGIN TRANSACTION;

INSERT INTO employees (id, name, department, salary) VALUES (4, 'Alice Brown', 'HR', 78000);

UPDATE employees SET salary = 95000 WHERE id = 1;

COMMIT;

SELECT * FROM employees;

BEGIN TRANSACTION;

INSERT INTO employees (id, name, department, salary) VALUES (5, 'Chris Wilson', 'Finance', 88000);

UPDATE employees SET department = 'Research' WHERE id = 2;

ROLLBACK;

SELECT * FROM employees;

.flush

EXIT;
.exit 