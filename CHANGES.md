# Changelog

## [1.2.0] - Current
- Added LRU Buffer Pool Manager for reduced disk I/O
- Implemented B-Tree indexing on PRIMARY KEY columns
- Added ACID transaction support (BEGIN/COMMIT/ROLLBACK)
- Added BIGINT and DATE data types

## [1.1.0]
- Added ALTER TABLE (ADD/DROP column) support
- Added UNIQUE and NOT NULL constraint validation
- Added .flush command for manual disk sync
- Improved WHERE clause to support AND/OR logical operators

## [1.0.0] - Initial Release
- Core SQL parser (SELECT, INSERT, UPDATE, DELETE)
- CREATE/DROP DATABASE and TABLE commands
- File-based data persistence
- PRIMARY KEY constraint support