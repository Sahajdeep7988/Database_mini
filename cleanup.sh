#!/bin/bash

echo "Cleaning up unnecessary files from repository..."

# Remove test scripts
rm -f run_test.bat
rm -f run_test.ps1
rm -f run_test.sh
rm -f simple_test.sql
rm -f test_clean.sql
rm -f test_semicolons.bat
rm -f test_semicolons.sql
rm -f test_transactions.sql
rm -f test_db.bat

# Remove test directory
rm -rf test_transactions

# Remove build artifacts
rm -f *.o
rm -f *.exe

# Remove other unnecessary files
rm -f cmake.msi

# Remove directories
rm -rf build
rm -rf obj
rm -rf data

echo "Cleanup complete."
echo "Use 'git status' to see changes and 'git add .' to stage them." 