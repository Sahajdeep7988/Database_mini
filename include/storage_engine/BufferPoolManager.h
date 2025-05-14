#ifndef BUFFER_POOL_MANAGER_H
#define BUFFER_POOL_MANAGER_H

#include <vector>
#include <unordered_map>
#include <list>
#include <memory>
#include <string>
#include <mutex>

namespace sqldb {

/**
 * @class Page
 * @brief Represents a page in memory
 */
class Page {
public:
    /**
     * @brief Constructor for Page
     * @param pageId The ID of the page
     * @param pageSize The size of the page in bytes
     */
    Page(int pageId, size_t pageSize = 4096);
    
    /**
     * @brief Get the ID of the page
     * @return The ID of the page
     */
    int getPageId() const;
    
    /**
     * @brief Get the data of the page
     * @return Pointer to the data of the page
     */
    char* getData();
    
    /**
     * @brief Get the data of the page (const version)
     * @return Pointer to the data of the page
     */
    const char* getData() const;
    
    /**
     * @brief Check if the page is dirty
     * @return true if the page is dirty, false otherwise
     */
    bool isDirty() const;
    
    /**
     * @brief Set the dirty flag of the page
     * @param dirty The dirty flag to set
     */
    void setDirty(bool dirty);
    
    /**
     * @brief Get the pin count of the page
     * @return The pin count of the page
     */
    int getPinCount() const;
    
    /**
     * @brief Increment the pin count of the page
     */
    void incrementPinCount();
    
    /**
     * @brief Decrement the pin count of the page
     */
    void decrementPinCount();
    
private:
    // The ID of the page
    int pageId_;
    
    // The data of the page
    std::vector<char> data_;
    
    // Whether the page is dirty
    bool isDirty_;
    
    // The pin count of the page
    int pinCount_;
};

/**
 * @class BufferPoolManager
 * @brief Manages the buffer pool for caching pages in memory
 */
class BufferPoolManager {
public:
    /**
     * @brief Constructor for BufferPoolManager
     * @param poolSize The size of the buffer pool in pages
     * @param pageSize The size of each page in bytes
     */
    BufferPoolManager(size_t poolSize = 1000, size_t pageSize = 4096);
    
    /**
     * @brief Destructor for BufferPoolManager
     */
    ~BufferPoolManager();
    
    /**
     * @brief Fetch a page from the buffer pool
     * @param pageId The ID of the page to fetch
     * @return Pointer to the page, or nullptr if not found
     */
    std::shared_ptr<Page> fetchPage(int pageId);
    
    /**
     * @brief Allocate a new page in the buffer pool
     * @param pageId The ID of the new page
     * @return Pointer to the new page, or nullptr if allocation failed
     */
    std::shared_ptr<Page> allocatePage(int pageId);
    
    /**
     * @brief Unpin a page in the buffer pool
     * @param pageId The ID of the page to unpin
     * @param isDirty Whether the page is dirty
     * @return true if unpin was successful, false otherwise
     */
    bool unpinPage(int pageId, bool isDirty);
    
    /**
     * @brief Flush a page to disk
     * @param pageId The ID of the page to flush
     * @return true if flush was successful, false otherwise
     */
    bool flushPage(int pageId);
    
    /**
     * @brief Flush all pages in the buffer pool to disk
     * @return true if flush was successful, false otherwise
     */
    bool flushAllPages();
    
    /**
     * @brief Delete a page from the buffer pool
     * @param pageId The ID of the page to delete
     * @return true if deletion was successful, false otherwise
     */
    bool deletePage(int pageId);
    
    /**
     * @brief Set the path to the database files
     * @param dbPath The path to the database files
     */
    void setDbPath(const std::string& dbPath);
    
private:
    // The size of the buffer pool in pages
    size_t poolSize_;
    
    // The size of each page in bytes
    size_t pageSize_;
    
    // The path to the database files
    std::string dbPath_;
    
    // The pages in the buffer pool
    std::vector<std::shared_ptr<Page>> pages_;
    
    // Map from page ID to frame ID
    std::unordered_map<int, size_t> pageTable_;
    
    // LRU list for page replacement
    std::list<int> lruList_;
    
    // Map from page ID to LRU iterator
    std::unordered_map<int, std::list<int>::iterator> lruMap_;
    
    // Mutex for thread safety
    std::mutex mutex_;
    
    /**
     * @brief Find a victim page to evict from the buffer pool
     * @return The frame ID of the victim page, or -1 if no victim found
     */
    int findVictim();
    
    /**
     * @brief Read a page from disk
     * @param pageId The ID of the page to read
     * @param page The page to read into
     * @return true if read was successful, false otherwise
     */
    bool readPageFromDisk(int pageId, std::shared_ptr<Page> page);
    
    /**
     * @brief Write a page to disk
     * @param pageId The ID of the page to write
     * @param page The page to write from
     * @return true if write was successful, false otherwise
     */
    bool writePageToDisk(int pageId, std::shared_ptr<Page> page);
};

} // namespace sqldb

#endif // BUFFER_POOL_MANAGER_H 