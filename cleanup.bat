@echo off
echo Cleaning up unnecessary files from repository...

REM Remove test scripts
del /q run_test.bat
del /q run_test.ps1
del /q run_test.sh
del /q simple_test.sql
del /q test_clean.sql
del /q test_semicolons.bat
del /q test_semicolons.sql
del /q test_transactions.sql
del /q test_db.bat

REM Remove test directory
rmdir /s /q test_transactions

REM Remove build artifacts
del /q *.o
del /q *.exe

REM Remove other unnecessary files
del /q cmake.msi

REM Remove directories
rmdir /s /q build
rmdir /s /q obj
rmdir /s /q data

echo Cleanup complete.
echo Use 'git status' to see changes and 'git add .' to stage them. 