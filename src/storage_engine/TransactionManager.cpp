#include "../../include/storage_engine/TransactionManager.h"
#include <iostream>
#include <algorithm>
#include <cstring>

namespace sqldb {

// Lock implementation
Lock::Lock(int resourceId, int transactionId, LockMode mode)
    : resourceId_(resourceId), transactionId_(transactionId), mode_(mode) {
}

int Lock::getResourceId() const {
    return resourceId_;
}

int Lock::getTransactionId() const {
    return transactionId_;
}

LockMode Lock::getMode() const {
    return mode_;
}

// LogRecord implementation
LogRecord::LogRecord(int transactionId, LogType type, int pageId, int offset,
                   const std::string& beforeImage, const std::string& afterImage)
    : transactionId_(transactionId), type_(type), pageId_(pageId), offset_(offset),
      beforeImage_(beforeImage), afterImage_(afterImage) {
}

int LogRecord::getTransactionId() const {
    return transactionId_;
}

LogRecord::LogType LogRecord::getType() const {
    return type_;
}

int LogRecord::getPageId() const {
    return pageId_;
}

int LogRecord::getOffset() const {
    return offset_;
}

const std::string& LogRecord::getBeforeImage() const {
    return beforeImage_;
}

const std::string& LogRecord::getAfterImage() const {
    return afterImage_;
}

std::string LogRecord::serialize() const {
    std::string result;
    
    // Serialize transaction ID
    result.append(reinterpret_cast<const char*>(&transactionId_), sizeof(transactionId_));
    
    // Serialize log type
    int type = static_cast<int>(type_);
    result.append(reinterpret_cast<const char*>(&type), sizeof(type));
    
    // Serialize page ID
    result.append(reinterpret_cast<const char*>(&pageId_), sizeof(pageId_));
    
    // Serialize offset
    result.append(reinterpret_cast<const char*>(&offset_), sizeof(offset_));
    
    // Serialize before image
    size_t beforeImageSize = beforeImage_.size();
    result.append(reinterpret_cast<const char*>(&beforeImageSize), sizeof(beforeImageSize));
    result.append(beforeImage_);
    
    // Serialize after image
    size_t afterImageSize = afterImage_.size();
    result.append(reinterpret_cast<const char*>(&afterImageSize), sizeof(afterImageSize));
    result.append(afterImage_);
    
    return result;
}

LogRecord LogRecord::deserialize(const std::string& data) {
    size_t pos = 0;
    
    // Deserialize transaction ID
    int transactionId;
    std::memcpy(&transactionId, data.data() + pos, sizeof(transactionId));
    pos += sizeof(transactionId);
    
    // Deserialize log type
    int type;
    std::memcpy(&type, data.data() + pos, sizeof(type));
    pos += sizeof(type);
    LogType logType = static_cast<LogType>(type);
    
    // Deserialize page ID
    int pageId;
    std::memcpy(&pageId, data.data() + pos, sizeof(pageId));
    pos += sizeof(pageId);
    
    // Deserialize offset
    int offset;
    std::memcpy(&offset, data.data() + pos, sizeof(offset));
    pos += sizeof(offset);
    
    // Deserialize before image
    size_t beforeImageSize;
    std::memcpy(&beforeImageSize, data.data() + pos, sizeof(beforeImageSize));
    pos += sizeof(beforeImageSize);
    std::string beforeImage(data.data() + pos, beforeImageSize);
    pos += beforeImageSize;
    
    // Deserialize after image
    size_t afterImageSize;
    std::memcpy(&afterImageSize, data.data() + pos, sizeof(afterImageSize));
    pos += sizeof(afterImageSize);
    std::string afterImage(data.data() + pos, afterImageSize);
    
    return LogRecord(transactionId, logType, pageId, offset, beforeImage, afterImage);
}

// Transaction implementation
Transaction::Transaction(int transactionId)
    : transactionId_(transactionId), state_(TransactionState::RUNNING) {
}

int Transaction::getTransactionId() const {
    return transactionId_;
}

TransactionState Transaction::getState() const {
    return state_;
}

void Transaction::setState(TransactionState state) {
    state_ = state;
}

void Transaction::addLock(std::shared_ptr<Lock> lock) {
    locks_.push_back(lock);
}

const std::vector<std::shared_ptr<Lock>>& Transaction::getLocks() const {
    return locks_;
}

// TransactionManager implementation
TransactionManager::TransactionManager(const std::string& logPath)
    : nextTransactionId_(1), logPath_(logPath) {
    // Don't open the log file immediately to avoid file locking issues
}

TransactionManager::~TransactionManager() {
    if (logFile_.is_open()) {
        logFile_.close();
    }
}

int TransactionManager::beginTransaction() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    int transactionId = nextTransactionId_++;
    transactions_[transactionId] = std::make_shared<Transaction>(transactionId);
    
    // Write a BEGIN log record
    LogRecord record(transactionId, LogRecord::LogType::BEGIN);
    writeLog(record);
    
    return transactionId;
}

bool TransactionManager::commitTransaction(int transactionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto iter = transactions_.find(transactionId);
    if (iter == transactions_.end()) {
        return false;  // Transaction not found
    }
    
    std::shared_ptr<Transaction> transaction = iter->second;
    
    // Write a COMMIT log record
    LogRecord record(transactionId, LogRecord::LogType::COMMIT);
    writeLog(record);
    
    // Set transaction state to COMMITTED
    transaction->setState(TransactionState::COMMITTED);
    
    // Release all locks held by the transaction
    releaseAllLocks(transactionId);
    
    // Remove the transaction from the active transactions
    transactions_.erase(transactionId);
    
    return true;
}

bool TransactionManager::abortTransaction(int transactionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto iter = transactions_.find(transactionId);
    if (iter == transactions_.end()) {
        return false;  // Transaction not found
    }
    
    std::shared_ptr<Transaction> transaction = iter->second;
    
    // Write an ABORT log record
    LogRecord record(transactionId, LogRecord::LogType::ABORT);
    writeLog(record);
    
    // Set transaction state to ABORTED
    transaction->setState(TransactionState::ABORTED);
    
    // TODO: Undo all changes made by the transaction
    
    // Release all locks held by the transaction
    releaseAllLocks(transactionId);
    
    // Remove the transaction from the active transactions
    transactions_.erase(transactionId);
    
    return true;
}

bool TransactionManager::acquireLock(int transactionId, int resourceId, LockMode mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto txnIter = transactions_.find(transactionId);
    if (txnIter == transactions_.end()) {
        return false;  // Transaction not found
    }
    
    std::shared_ptr<Transaction> transaction = txnIter->second;
    
    // Check if the transaction already holds a lock on the resource
    for (const auto& existingLock : transaction->getLocks()) {
        if (existingLock->getResourceId() == resourceId) {
            // If the transaction already holds a lock on the resource
            if (existingLock->getMode() == mode || existingLock->getMode() == LockMode::EXCLUSIVE) {
                // Already holds the requested lock or a stronger one
                return true;
            } else if (mode == LockMode::EXCLUSIVE) {
                // Upgrade from shared to exclusive lock
                // First, check if the upgrade is possible
                if (!canGrantLock(transactionId, resourceId, mode)) {
                    return false;
                }
                
                // Remove the existing lock
                auto& locks = resourceLocks_[resourceId];
                locks.erase(std::remove_if(locks.begin(), locks.end(),
                                         [transactionId](const std::shared_ptr<Lock>& l) {
                                             return l->getTransactionId() == transactionId;
                                         }),
                          locks.end());
                
                // Create a new exclusive lock
                std::shared_ptr<Lock> newLock = std::make_shared<Lock>(resourceId, transactionId, mode);
                resourceLocks_[resourceId].push_back(newLock);
                transaction->addLock(newLock);
                
                return true;
            }
        }
    }
    
    // Check if the lock can be granted
    if (!canGrantLock(transactionId, resourceId, mode)) {
        return false;
    }
    
    // Create a new lock
    std::shared_ptr<Lock> newLock = std::make_shared<Lock>(resourceId, transactionId, mode);
    resourceLocks_[resourceId].push_back(newLock);
    transaction->addLock(newLock);
    
    return true;
}

bool TransactionManager::releaseLock(int transactionId, int resourceId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto txnIter = transactions_.find(transactionId);
    if (txnIter == transactions_.end()) {
        return false;  // Transaction not found
    }
    
    std::shared_ptr<Transaction> transaction = txnIter->second;
    
    // Remove the lock from the transaction
    auto& txnLocks = const_cast<std::vector<std::shared_ptr<Lock>>&>(transaction->getLocks());
    txnLocks.erase(std::remove_if(txnLocks.begin(), txnLocks.end(),
                                [resourceId](const std::shared_ptr<Lock>& l) {
                                    return l->getResourceId() == resourceId;
                                }),
                  txnLocks.end());
    
    // Remove the lock from the resource
    auto& resourceLocksList = resourceLocks_[resourceId];
    resourceLocksList.erase(std::remove_if(resourceLocksList.begin(), resourceLocksList.end(),
                                         [transactionId](const std::shared_ptr<Lock>& l) {
                                             return l->getTransactionId() == transactionId;
                                         }),
                          resourceLocksList.end());
    
    return true;
}

bool TransactionManager::writeLog(const LogRecord& record) {
    if (logPath_.empty()) {
        return false;
    }
    
    // Open the file if it's not already open
    if (!logFile_.is_open()) {
        logFile_.open(logPath_, std::ios::binary | std::ios::app);
        if (!logFile_.is_open()) {
            return false;
        }
    }
    
    std::string serializedRecord = record.serialize();
    size_t recordSize = serializedRecord.size();
    
    // Write record size
    logFile_.write(reinterpret_cast<const char*>(&recordSize), sizeof(recordSize));
    
    // Write record data
    logFile_.write(serializedRecord.data(), recordSize);
    
    // Flush to disk
    logFile_.flush();
    
    // Close the file to avoid locking issues
    logFile_.close();
    
    return true;
}

bool TransactionManager::recover() {
    if (logPath_.empty()) {
        return false;
    }
    
    std::ifstream file(logPath_, std::ios::binary);
    if (!file) {
        return false;
    }
    
    // Read all log records
    while (file) {
        // Read record size
        size_t recordSize;
        file.read(reinterpret_cast<char*>(&recordSize), sizeof(recordSize));
        
        if (file.eof()) {
            break;
        }
        
        // Read record data
        std::string serializedRecord(recordSize, '\0');
        file.read(&serializedRecord[0], recordSize);
        
        // Deserialize the record
        LogRecord record = LogRecord::deserialize(serializedRecord);
        
        // Process the record based on its type
        switch (record.getType()) {
            case LogRecord::LogType::BEGIN:
                // Create a new transaction
                transactions_[record.getTransactionId()] = std::make_shared<Transaction>(record.getTransactionId());
                break;
                
            case LogRecord::LogType::COMMIT:
                // Mark the transaction as committed
                if (transactions_.find(record.getTransactionId()) != transactions_.end()) {
                    transactions_[record.getTransactionId()]->setState(TransactionState::COMMITTED);
                }
                break;
                
            case LogRecord::LogType::ABORT:
                // Mark the transaction as aborted
                if (transactions_.find(record.getTransactionId()) != transactions_.end()) {
                    transactions_[record.getTransactionId()]->setState(TransactionState::ABORTED);
                }
                break;
                
            case LogRecord::LogType::UPDATE:
            case LogRecord::LogType::INSERT:
            case LogRecord::LogType::DELETE:
                // TODO: Apply the operation during recovery
                break;
        }
    }
    
    // Close the file to avoid locking issues
    file.close();
    
    // Determine which transactions need to be rolled back
    std::vector<int> rollbackTransactions;
    for (const auto& entry : transactions_) {
        if (entry.second->getState() == TransactionState::RUNNING) {
            rollbackTransactions.push_back(entry.first);
        }
    }
    
    // Rollback uncommitted transactions
    for (int transactionId : rollbackTransactions) {
        // TODO: Undo all changes made by the transaction
        transactions_[transactionId]->setState(TransactionState::ABORTED);
    }
    
    return true;
}

bool TransactionManager::canGrantLock(int transactionId, int resourceId, LockMode mode) {
    // If no locks exist for the resource, the lock can be granted
    if (resourceLocks_.find(resourceId) == resourceLocks_.end() || resourceLocks_[resourceId].empty()) {
        return true;
    }
    
    // Check existing locks
    for (const auto& existingLock : resourceLocks_[resourceId]) {
        // If the lock is held by the same transaction, it can be granted
        if (existingLock->getTransactionId() == transactionId) {
            continue;
        }
        
        // If the existing lock is exclusive, or the requested lock is exclusive,
        // the lock cannot be granted
        if (existingLock->getMode() == LockMode::EXCLUSIVE || mode == LockMode::EXCLUSIVE) {
            return false;
        }
    }
    
    return true;
}

void TransactionManager::releaseAllLocks(int transactionId) {
    auto txnIter = transactions_.find(transactionId);
    if (txnIter == transactions_.end()) {
        return;  // Transaction not found
    }
    
    std::shared_ptr<Transaction> transaction = txnIter->second;
    
    // Get all locks held by the transaction
    const auto& locks = transaction->getLocks();
    
    // Remove each lock from the resource
    for (const auto& lock : locks) {
        int resourceId = lock->getResourceId();
        auto& resourceLocksList = resourceLocks_[resourceId];
        resourceLocksList.erase(std::remove_if(resourceLocksList.begin(), resourceLocksList.end(),
                                             [transactionId](const std::shared_ptr<Lock>& l) {
                                                 return l->getTransactionId() == transactionId;
                                             }),
                              resourceLocksList.end());
    }
    
    // Clear the transaction's locks
    auto& txnLocks = const_cast<std::vector<std::shared_ptr<Lock>>&>(transaction->getLocks());
    txnLocks.clear();
}

} // namespace sqldb 