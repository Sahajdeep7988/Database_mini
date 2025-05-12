cmake_minimum_required(VERSION 3.10)
project(SQLDatabase VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(sqldb
    src/main.cpp
)

# Include directories
target_include_directories(sqldb PRIVATE include)

# Set output directory
set_target_properties(sqldb PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Enable warnings
if(MSVC)
    target_compile_options(sqldb PRIVATE /W4)
else()
    target_compile_options(sqldb PRIVATE -Wall -Wextra -pedantic)
endif() 