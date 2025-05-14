cmake_minimum_required(VERSION 3.10)
project(SQLDatabase VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Add source files
file(GLOB SOURCES 
    "src/*.cpp"
    "src/storage_engine/*.cpp"
)

# Add executable
add_executable(sqldb ${SOURCES})

# Set output directory
set_target_properties(sqldb PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Enable warnings
if(MSVC)
    target_compile_options(sqldb PRIVATE /W4)
else()
    target_compile_options(sqldb PRIVATE -Wall -Wextra -pedantic)
endif() 