@echo off
echo Compiling database system...
g++ -std=c++17 src/main.cpp -o simple_db -I include/

echo.
echo Running simple test...
type simple_test.sql | simple_db

echo.
echo Test completed! 