#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <mutex>
#include <fstream>

namespace sqldb {

/**
 * @enum TransactionState
 * @brief Represents the state of a transaction
 */
enum class TransactionState {
    RUNNING,
    COMMITTED,
    ABORTED
};

/**
 * @enum LockMode
 * @brief Represents the mode of a lock
 */
enum class LockMode {
    SHARED,
    EXCLUSIVE
};

/**
 * @class Lock
 * @brief Represents a lock on a resource
 */
class Lock {
public:
    /**
     * @brief Constructor for Lock
     * @param resourceId The ID of the resource
     * @param transactionId The ID of the transaction
     * @param mode The mode of the lock
     */
    Lock(int resourceId, int transactionId, LockMode mode);
    
    /**
     * @brief Get the ID of the resource
     * @return The ID of the resource
     */
    int getResourceId() const;
    
    /**
     * @brief Get the ID of the transaction
     * @return The ID of the transaction
     */
    int getTransactionId() const;
    
    /**
     * @brief Get the mode of the lock
     * @return The mode of the lock
     */
    LockMode getMode() const;
    
private:
    // The ID of the resource
    int resourceId_;
    
    // The ID of the transaction
    int transactionId_;
    
    // The mode of the lock
    LockMode mode_;
};

/**
 * @class LogRecord
 * @brief Represents a log record in the write-ahead log
 */
class LogRecord {
public:
    /**
     * @enum LogType
     * @brief Represents the type of a log record
     */
    enum class LogType {
        BEGIN,
        COMMIT,
        ABORT,
        UPDATE,
        INSERT,
        DELETE
    };
    
    /**
     * @brief Constructor for LogRecord
     * @param transactionId The ID of the transaction
     * @param type The type of the log record
     * @param pageId The ID of the page (for UPDATE, INSERT, DELETE)
     * @param offset The offset in the page (for UPDATE, INSERT, DELETE)
     * @param beforeImage The before image (for UPDATE)
     * @param afterImage The after image (for UPDATE, INSERT)
     */
    LogRecord(int transactionId, LogType type, int pageId = -1, int offset = -1,
              const std::string& beforeImage = "", const std::string& afterImage = "");
    
    /**
     * @brief Get the ID of the transaction
     * @return The ID of the transaction
     */
    int getTransactionId() const;
    
    /**
     * @brief Get the type of the log record
     * @return The type of the log record
     */
    LogType getType() const;
    
    /**
     * @brief Get the ID of the page
     * @return The ID of the page
     */
    int getPageId() const;
    
    /**
     * @brief Get the offset in the page
     * @return The offset in the page
     */
    int getOffset() const;
    
    /**
     * @brief Get the before image
     * @return The before image
     */
    const std::string& getBeforeImage() const;
    
    /**
     * @brief Get the after image
     * @return The after image
     */
    const std::string& getAfterImage() const;
    
    /**
     * @brief Serialize the log record to a string
     * @return The serialized log record
     */
    std::string serialize() const;
    
    /**
     * @brief Deserialize a log record from a string
     * @param data The serialized log record
     * @return The deserialized log record
     */
    static LogRecord deserialize(const std::string& data);
    
private:
    // The ID of the transaction
    int transactionId_;
    
    // The type of the log record
    LogType type_;
    
    // The ID of the page
    int pageId_;
    
    // The offset in the page
    int offset_;
    
    // The before image
    std::string beforeImage_;
    
    // The after image
    std::string afterImage_;
};

/**
 * @class Transaction
 * @brief Represents a transaction
 */
class Transaction {
public:
    /**
     * @brief Constructor for Transaction
     * @param transactionId The ID of the transaction
     */
    Transaction(int transactionId);
    
    /**
     * @brief Get the ID of the transaction
     * @return The ID of the transaction
     */
    int getTransactionId() const;
    
    /**
     * @brief Get the state of the transaction
     * @return The state of the transaction
     */
    TransactionState getState() const;
    
    /**
     * @brief Set the state of the transaction
     * @param state The state to set
     */
    void setState(TransactionState state);
    
    /**
     * @brief Add a lock to the transaction
     * @param lock The lock to add
     */
    void addLock(std::shared_ptr<Lock> lock);
    
    /**
     * @brief Get the locks held by the transaction
     * @return The locks held by the transaction
     */
    const std::vector<std::shared_ptr<Lock>>& getLocks() const;
    
private:
    // The ID of the transaction
    int transactionId_;
    
    // The state of the transaction
    TransactionState state_;
    
    // The locks held by the transaction
    std::vector<std::shared_ptr<Lock>> locks_;
};

/**
 * @class TransactionManager
 * @brief Manages transactions and concurrency control
 */
class TransactionManager {
public:
    /**
     * @brief Constructor for TransactionManager
     * @param logPath The path to the log file
     */
    TransactionManager(const std::string& logPath = "");
    
    /**
     * @brief Destructor for TransactionManager
     */
    ~TransactionManager();
    
    /**
     * @brief Begin a new transaction
     * @return The ID of the new transaction
     */
    int beginTransaction();
    
    /**
     * @brief Commit a transaction
     * @param transactionId The ID of the transaction to commit
     * @return true if commit was successful, false otherwise
     */
    bool commitTransaction(int transactionId);
    
    /**
     * @brief Abort a transaction
     * @param transactionId The ID of the transaction to abort
     * @return true if abort was successful, false otherwise
     */
    bool abortTransaction(int transactionId);
    
    /**
     * @brief Acquire a lock on a resource
     * @param transactionId The ID of the transaction
     * @param resourceId The ID of the resource
     * @param mode The mode of the lock
     * @return true if lock acquisition was successful, false otherwise
     */
    bool acquireLock(int transactionId, int resourceId, LockMode mode);
    
    /**
     * @brief Release a lock on a resource
     * @param transactionId The ID of the transaction
     * @param resourceId The ID of the resource
     * @return true if lock release was successful, false otherwise
     */
    bool releaseLock(int transactionId, int resourceId);
    
    /**
     * @brief Write a log record to the write-ahead log
     * @param record The log record to write
     * @return true if write was successful, false otherwise
     */
    bool writeLog(const LogRecord& record);
    
    /**
     * @brief Recover the database from the write-ahead log
     * @return true if recovery was successful, false otherwise
     */
    bool recover();
    
private:
    // The next transaction ID
    int nextTransactionId_;
    
    // The path to the log file
    std::string logPath_;
    
    // The log file
    std::ofstream logFile_;
    
    // Map from transaction ID to transaction
    std::unordered_map<int, std::shared_ptr<Transaction>> transactions_;
    
    // Map from resource ID to locks
    std::unordered_map<int, std::vector<std::shared_ptr<Lock>>> resourceLocks_;
    
    // Mutex for thread safety
    std::mutex mutex_;
    
    /**
     * @brief Check if a lock can be granted
     * @param transactionId The ID of the transaction
     * @param resourceId The ID of the resource
     * @param mode The mode of the lock
     * @return true if the lock can be granted, false otherwise
     */
    bool canGrantLock(int transactionId, int resourceId, LockMode mode);
    
    /**
     * @brief Release all locks held by a transaction
     * @param transactionId The ID of the transaction
     */
    void releaseAllLocks(int transactionId);
};

} // namespace sqldb

#endif // TRANSACTION_MANAGER_H 