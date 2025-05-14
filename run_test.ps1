# PowerShell script to test the SQL database with transaction support
Write-Host "Testing SQL database transaction functionality..."

# Create a temporary file with SQL commands - WITHOUT semicolons
$commandsFile = "commands.txt"

@"
CREATE DATABASE transactiondb
USE transactiondb
CREATE TABLE employees (id INT PRIMARY KEY, name STRING NOT NULL, department STRING, salary BIGINT)
INSERT INTO employees (id, name, department, salary) VALUES (1, 'John Doe', 'Engineering', 90000)
INSERT INTO employees (id, name, department, salary) VALUES (2, 'Jane Smith', 'Marketing', 85000)
INSERT INTO employees (id, name, department, salary) VALUES (3, 'Bob Johnson', 'Engineering', 92000)
SELECT * FROM employees
BEGIN TRANSACTION
INSERT INTO employees (id, name, department, salary) VALUES (4, 'Alice Brown', 'HR', 78000)
UPDATE employees SET salary = 95000 WHERE id = 1
COMMIT
SELECT * FROM employees
BEGIN TRANSACTION
INSERT INTO employees (id, name, department, salary) VALUES (5, 'Chris Wilson', 'Finance', 88000)
UPDATE employees SET department = 'Research' WHERE id = 2
ROLLBACK
SELECT * FROM employees
.flush
EXIT
.exit
"@ | Out-File -FilePath $commandsFile -Encoding ascii

Write-Host "Running SQL commands through sqldb.exe..."

# Get the content of the commands file and pipe it to sqldb.exe
$output = Get-Content -Path $commandsFile | ForEach-Object { 
    Write-Host "Executing: $_"
    $_
    Start-Sleep -Milliseconds 100  # small delay between commands
} | .\sqldb.exe

# Write output to file
$output | Out-File -FilePath "test_output.txt"

Write-Host "Test completed. Check test_output.txt for results."

Write-Host "Compiling database system..." -ForegroundColor Green
g++ -std=c++17 src/main.cpp -o simple_db.exe -I include/

Write-Host ""
Write-Host "Running simple test..." -ForegroundColor Cyan
Get-Content simple_test.sql | .\simple_db.exe

Write-Host ""
Write-Host "Test completed!" -ForegroundColor Green 