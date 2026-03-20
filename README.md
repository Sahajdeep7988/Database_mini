![C++17](https://img.shields.io/badge/C++-17-blue?logo=cplusplus) ![CMake](https://img.shields.io/badge/Build-CMake-green) ![License](https://img.shields.io/badge/License-MIT-yellow) ![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-lightgrey)

# RelictDB
> A lightweight relational database engine built from scratch in C++ — featuring a hand-written SQL parser, B-Tree indexing, LRU buffer pool, and ACID-compliant transactions with file-based persistence.

## Motivation

I built RelictDB to deeply understand how database systems work internally — not just how to use SQL, but how storage engines, query parsers, indexing structures, and transaction managers are actually implemented. This project started as a question: "what does MySQL actually do when I run a query?" and turned into a full implementation.

Key internal systems I implemented from scratch:
- A hand-written recursive-descent SQL parser (no libraries)
- B-Tree index structure for O(log n) key lookups
- LRU Buffer Pool Manager to minimize disk I/O
- Write-ahead style transaction manager for ACID guarantees
- Binary file-based persistence layer

## Features

### Data Types
- **INT**: Integer values
- **BIGINT**: Large integer values
- **STRING**: Text values
- **DATE**: Date values in `YYYY-MM-DD` format

### Database Operations
- Create, use, and drop databases
- Create, alter, and drop tables
- Insert, update, delete, and select data
- Transaction support (`BEGIN`/`COMMIT`/`ROLLBACK`)

### Engine Capabilities
- Case-insensitive SQL commands
- Constraint validation (`PRIMARY KEY`, `UNIQUE`, `NOT NULL`)
- Advanced `WHERE` conditions with `AND`/`OR`
- B-Tree indexing on `PRIMARY KEY` columns
- LRU buffer pool page caching
- File-based persistence for data durability

### Important Syntax Rule
Unlike standard SQL, commands in this system must **not** end with semicolons.

## Architecture

RelictDB is structured in three layers:

**Query Layer** — The SQL parser tokenizes and parses input commands into an internal AST representation. Supports SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, ALTER, and transaction commands.

**Engine Layer** — The database manager routes parsed queries to the correct table handler. Each table maintains its own B-Tree index on the primary key column for fast lookups.

**Storage Layer** — The storage engine handles serialization of rows to binary page files. The buffer pool manager keeps recently used pages in memory using an LRU eviction policy, reducing disk reads.

## Build & Installation

### Prerequisites
- C++17 compatible compiler (`GCC 7+`, `Clang 5+`, `MSVC 2017+`)
- CMake `3.10+` (recommended)

### Build From Source

#### Using CMake (Recommended)
```bash
# Clone the repository
git clone https://github.com/Sahajdeep7988/RelictDB.git
cd RelictDB

# Configure
mkdir build && cd build
cmake ..

# Build
cmake --build .
```

#### Manual Build
```bash
# Linux/macOS
g++ -std=c++17 src/main.cpp src/storage_engine/*.cpp -o relictdb -I include/

# Windows (MinGW)
g++ -std=c++17 src/main.cpp src/storage_engine/*.cpp -o relictdb.exe -I include/

# Windows (MSVC)
cl /std:c++17 /EHsc /Iinclude src/main.cpp src/storage_engine/*.cpp /Ferelictdb.exe
```

### Project Structure
```text
├── CMakeLists.txt
├── include/
│   ├── AggregateFunction.h
│   ├── Column.h
│   ├── DataType.h
│   ├── DatabaseManager.h
│   ├── DatabaseSystem.h
│   ├── QueryParser.h
│   ├── StringFunction.h
│   ├── Table.h
│   └── storage_engine/
│       ├── BTreeIndex.h
│       ├── BufferPoolManager.h
│       ├── StorageEngine.h
│       ├── TransactionManager.h
│       └── Value.h
└── src/
    ├── main.cpp
    ├── storage_engine_test.cpp
    ├── storage_engine_performance_test.cpp
    └── storage_engine/
        ├── BTreeIndex.cpp
        ├── BufferPoolManager.cpp
        ├── StorageEngine.cpp
        └── TransactionManager.cpp
```

## Usage / SQL Reference

### Start the Application
```bash
# Linux/macOS
./relictdb

# Windows
relictdb.exe
```

### Quickstart Example
```sql
CREATE DATABASE school
USE school

CREATE TABLE students (id INT PRIMARY KEY, name STRING NOT NULL, age INT)
INSERT INTO students (id, name, age) VALUES (1, 'John Doe', 20)
SELECT * FROM students

UPDATE students SET age = 21 WHERE id = 1
DELETE FROM students WHERE id = 1

EXIT
.exit
```

### SQL Command Reference

#### Database Commands
| Command | Description | Example |
|---------|-------------|---------|
| `CREATE DATABASE` | Creates a new database | `CREATE DATABASE school` |
| `DROP DATABASE` | Deletes a database | `DROP DATABASE school` |
| `USE` | Switches to a database | `USE school` |
| `EXIT` | Exits the current database | `EXIT` |

#### Table Commands
| Command | Description | Example |
|---------|-------------|---------|
| `CREATE TABLE` | Creates a table | `CREATE TABLE students (id INT PRIMARY KEY, name STRING)` |
| `DROP TABLE` | Deletes a table | `DROP TABLE students` |
| `ALTER TABLE ... ADD` | Adds a column | `ALTER TABLE students ADD email STRING` |
| `ALTER TABLE ... DROP` | Drops a column | `ALTER TABLE students DROP email` |
| `DESC` | Shows table schema | `DESC students` |
| `SHOW TABLES` | Lists all tables | `SHOW TABLES` |

#### Data Manipulation Commands
| Command | Description | Example |
|---------|-------------|---------|
| `INSERT INTO` | Inserts data | `INSERT INTO students (id, name) VALUES (1, 'John')` |
| `SELECT` | Queries data | `SELECT * FROM students WHERE age > 20` |
| `UPDATE` | Updates data | `UPDATE students SET age = 21 WHERE id = 1` |
| `DELETE FROM` | Deletes data | `DELETE FROM students WHERE id = 1` |

#### Transaction Commands
| Command | Description | Example |
|---------|-------------|---------|
| `BEGIN TRANSACTION` | Starts a transaction | `BEGIN TRANSACTION` |
| `COMMIT` | Commits current transaction | `COMMIT` |
| `ROLLBACK` | Reverts current transaction | `ROLLBACK` |

#### WHERE Clause Operators
Comparison operators:
- `=`
- `!=` or `<>`
- `<`
- `>`
- `<=`
- `>=`

Logical operators:
- `AND`
- `OR`

Example:
```sql
SELECT * FROM students
WHERE (age > 20 AND department = 'Engineering') OR name = 'John'
```

#### Special Dot Commands
| Command | Description |
|---------|-------------|
| `.help` | Displays help |
| `.databases` | Lists all databases |
| `.tables` | Lists tables in the current database |
| `.flush` | Flushes in-memory changes to disk |
| `.exit` | Exits the program |

#### Constraints
- **PRIMARY KEY**: Unique row identifier
- **UNIQUE**: Column-wide uniqueness
- **NOT NULL**: Prevents empty/null values

Constraint example:
```sql
CREATE TABLE employees (
    id INT PRIMARY KEY,
    email STRING UNIQUE,
    name STRING NOT NULL,
    department STRING
)
```

#### Transaction Example
```sql
BEGIN TRANSACTION
INSERT INTO employees (id, name, department) VALUES (1, 'John Doe', 'Engineering')
UPDATE departments SET employee_count = employee_count + 1 WHERE name = 'Engineering'
COMMIT
```

### Troubleshooting
1. **Permission errors**: Ensure write permissions in the runtime directory.
2. **Database not found**: Create it first with `CREATE DATABASE`.
3. **Syntax errors**: Remove trailing semicolons.
4. **Complex conditions**: Use parentheses to group `AND`/`OR` logic.
5. **Build failures**: Confirm C++17 compiler and toolchain availability.

For deeper implementation notes, see [`help.md`](help.md).

## Known Limitations & Future Work

This is an educational implementation — not production-ready. Known limitations:
- No support for JOINs across tables (planned)
- Single-user only — no concurrent transaction isolation
- WHERE clause does not support subqueries
- Index is only created on PRIMARY KEY columns

These are acknowledged design boundaries, not bugs.

## Author
**Sahajdeep Singh** — B.Tech AI & ML, Amity University Punjab    
sahajdeepsingh404@gmail.com
