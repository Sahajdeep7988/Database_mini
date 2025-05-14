@echo off
REM ===================================================
REM SQL-like Database System Test Script
REM ===================================================
REM This script demonstrates the functionality of the 
REM database system, including transaction support.
REM 
REM IMPORTANT NOTE: Do not use semicolons at the end of commands!
REM ===================================================

echo ===================================================
echo Creating test script file...
echo ===================================================

REM Create the test commands file
echo CREATE DATABASE testdb > test_commands.txt
echo USE testdb >> test_commands.txt

echo. >> test_commands.txt
echo REM Creating tables... >> test_commands.txt
echo CREATE TABLE employees (id INT PRIMARY KEY, name STRING NOT NULL, department STRING, salary BIGINT) >> test_commands.txt

echo. >> test_commands.txt
echo REM Inserting initial data... >> test_commands.txt
echo INSERT INTO employees (id, name, department, salary) VALUES (1, 'John Doe', 'Engineering', 90000) >> test_commands.txt
echo INSERT INTO employees (id, name, department, salary) VALUES (2, 'Jane Smith', 'Marketing', 85000) >> test_commands.txt
echo INSERT INTO employees (id, name, department, salary) VALUES (3, 'Bob Johnson', 'Engineering', 92000) >> test_commands.txt

echo. >> test_commands.txt
echo REM Querying data... >> test_commands.txt
echo SELECT * FROM employees >> test_commands.txt

echo. >> test_commands.txt
echo REM Testing transaction with COMMIT... >> test_commands.txt
echo BEGIN TRANSACTION >> test_commands.txt
echo INSERT INTO employees (id, name, department, salary) VALUES (4, 'Alice Brown', 'HR', 78000) >> test_commands.txt
echo UPDATE employees SET salary = 95000 WHERE id = 1 >> test_commands.txt
echo COMMIT >> test_commands.txt
echo SELECT * FROM employees >> test_commands.txt

echo. >> test_commands.txt
echo REM Testing transaction with ROLLBACK... >> test_commands.txt
echo BEGIN TRANSACTION >> test_commands.txt
echo INSERT INTO employees (id, name, department, salary) VALUES (5, 'Chris Wilson', 'Finance', 88000) >> test_commands.txt
echo UPDATE employees SET department = 'Research' WHERE id = 2 >> test_commands.txt
echo ROLLBACK >> test_commands.txt
echo SELECT * FROM employees >> test_commands.txt

echo. >> test_commands.txt
echo REM Testing data persistence... >> test_commands.txt
echo .flush >> test_commands.txt

echo. >> test_commands.txt
echo REM Clean up and exit... >> test_commands.txt
echo EXIT >> test_commands.txt
echo .exit >> test_commands.txt

echo ===================================================
echo Running test script...
echo ===================================================
echo Running SQL commands through sqldb.exe...
echo Results will be saved to test_results.txt
echo.

REM Run the test script through sqldb.exe
type test_commands.txt | .\sqldb.exe > test_results.txt

echo ===================================================
echo Test completed. Results saved to test_results.txt
echo ===================================================
echo NOTE: Due to how terminal input works in this application,
echo       some commands may not be recognized correctly when
echo       piped in. For best results, run commands interactively.
echo ===================================================

REM Display a portion of the results
echo Showing beginning of results:
type test_results.txt | findstr /B /C:">" /C:"Error:" /C:"Welcome" /C:"Using"

echo.
echo To use the database interactively, run:
echo   .\sqldb.exe 