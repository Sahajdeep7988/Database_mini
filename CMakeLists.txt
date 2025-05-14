cmake_minimum_required(VERSION 3.10)
project(SQLDatabase VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Add source files for main application
file(GLOB MAIN_SOURCES 
    "src/*.cpp"
    "src/storage_engine/*.cpp"
)

# Remove test files from main sources
list(FILTER MAIN_SOURCES EXCLUDE REGEX ".*storage_engine_test\\.cpp$")
list(FILTER MAIN_SOURCES EXCLUDE REGEX ".*storage_engine_performance_test\\.cpp$")

# Add main executable
add_executable(sqldb ${MAIN_SOURCES})

# Add test executable
add_executable(storage_engine_test 
    "src/storage_engine_test.cpp"
    "src/storage_engine/StorageEngine.cpp"
    "src/storage_engine/BufferPoolManager.cpp"
    "src/storage_engine/BTreeIndex.cpp"
    "src/storage_engine/TransactionManager.cpp"
)

# Add performance test executable
add_executable(storage_engine_perf_test
    "src/storage_engine_performance_test.cpp"
    "src/storage_engine/StorageEngine.cpp"
    "src/storage_engine/BufferPoolManager.cpp"
    "src/storage_engine/BTreeIndex.cpp"
    "src/storage_engine/TransactionManager.cpp"
)

# Set output directory
set_target_properties(sqldb PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

set_target_properties(storage_engine_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

set_target_properties(storage_engine_perf_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Enable warnings
if(MSVC)
    target_compile_options(sqldb PRIVATE /W4)
    target_compile_options(storage_engine_test PRIVATE /W4)
    target_compile_options(storage_engine_perf_test PRIVATE /W4)
else()
    target_compile_options(sqldb PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(storage_engine_test PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(storage_engine_perf_test PRIVATE -Wall -Wextra -pedantic)
endif() 