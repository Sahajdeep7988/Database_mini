#include "../../include/storage_engine/BufferPoolManager.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>

namespace sqldb {

// Page implementation
Page::Page(int pageId, size_t pageSize)
    : pageId_(pageId), data_(pageSize, 0), isDirty_(false), pinCount_(0) {
}

int Page::getPageId() const {
    return pageId_;
}

char* Page::getData() {
    return data_.data();
}

const char* Page::getData() const {
    return data_.data();
}

bool Page::isDirty() const {
    return isDirty_;
}

void Page::setDirty(bool dirty) {
    isDirty_ = dirty;
}

int Page::getPinCount() const {
    return pinCount_;
}

void Page::incrementPinCount() {
    pinCount_++;
}

void Page::decrementPinCount() {
    if (pinCount_ > 0) {
        pinCount_--;
    }
}

// BufferPoolManager implementation
BufferPoolManager::BufferPoolManager(size_t poolSize, size_t pageSize)
    : poolSize_(poolSize), pageSize_(pageSize) {
    // Initialize the buffer pool with empty pages
    pages_.resize(poolSize, nullptr);
}

BufferPoolManager::~BufferPoolManager() {
    // Flush all dirty pages to disk
    flushAllPages();
}

std::shared_ptr<Page> BufferPoolManager::fetchPage(int pageId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if the page is already in the buffer pool
    auto iter = pageTable_.find(pageId);
    if (iter != pageTable_.end()) {
        size_t frameId = iter->second;
        std::shared_ptr<Page> page = pages_[frameId];
        
        // Update LRU list
        lruList_.erase(lruMap_[pageId]);
        lruList_.push_front(pageId);
        lruMap_[pageId] = lruList_.begin();
        
        // Increment pin count
        page->incrementPinCount();
        
        return page;
    }
    
    // Page not in buffer pool, find a frame to load it
    int frameId = findVictim();
    if (frameId == -1) {
        // No available frame
        return nullptr;
    }
    
    // If the frame contains a page, write it back to disk if dirty
    if (pages_[frameId] != nullptr) {
        int victimPageId = pages_[frameId]->getPageId();
        
        if (pages_[frameId]->isDirty()) {
            writePageToDisk(victimPageId, pages_[frameId]);
        }
        
        // Remove the victim page from the page table and LRU list
        pageTable_.erase(victimPageId);
        lruList_.erase(lruMap_[victimPageId]);
        lruMap_.erase(victimPageId);
    }
    
    // Create a new page
    std::shared_ptr<Page> page = std::make_shared<Page>(pageId, pageSize_);
    
    // Read the page from disk
    if (!readPageFromDisk(pageId, page)) {
        // Page doesn't exist on disk, initialize with zeros
        std::fill(page->getData(), page->getData() + pageSize_, 0);
    }
    
    // Add the page to the buffer pool
    pages_[frameId] = page;
    pageTable_[pageId] = frameId;
    
    // Update LRU list
    lruList_.push_front(pageId);
    lruMap_[pageId] = lruList_.begin();
    
    // Increment pin count
    page->incrementPinCount();
    
    return page;
}

std::shared_ptr<Page> BufferPoolManager::allocatePage(int pageId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if the page already exists
    auto iter = pageTable_.find(pageId);
    if (iter != pageTable_.end()) {
        return nullptr;  // Page already exists
    }
    
    // Find a frame to allocate
    int frameId = findVictim();
    if (frameId == -1) {
        // No available frame
        return nullptr;
    }
    
    // If the frame contains a page, write it back to disk if dirty
    if (pages_[frameId] != nullptr) {
        int victimPageId = pages_[frameId]->getPageId();
        
        if (pages_[frameId]->isDirty()) {
            writePageToDisk(victimPageId, pages_[frameId]);
        }
        
        // Remove the victim page from the page table and LRU list
        pageTable_.erase(victimPageId);
        lruList_.erase(lruMap_[victimPageId]);
        lruMap_.erase(victimPageId);
    }
    
    // Create a new page
    std::shared_ptr<Page> page = std::make_shared<Page>(pageId, pageSize_);
    
    // Initialize the page with zeros
    std::fill(page->getData(), page->getData() + pageSize_, 0);
    
    // Add the page to the buffer pool
    pages_[frameId] = page;
    pageTable_[pageId] = frameId;
    
    // Update LRU list
    lruList_.push_front(pageId);
    lruMap_[pageId] = lruList_.begin();
    
    // Increment pin count
    page->incrementPinCount();
    
    return page;
}

bool BufferPoolManager::unpinPage(int pageId, bool isDirty) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if the page is in the buffer pool
    auto iter = pageTable_.find(pageId);
    if (iter == pageTable_.end()) {
        return false;  // Page not in buffer pool
    }
    
    size_t frameId = iter->second;
    std::shared_ptr<Page> page = pages_[frameId];
    
    // Decrement pin count
    page->decrementPinCount();
    
    // Set dirty flag if requested
    if (isDirty) {
        page->setDirty(true);
    }
    
    return true;
}

bool BufferPoolManager::flushPage(int pageId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if the page is in the buffer pool
    auto iter = pageTable_.find(pageId);
    if (iter == pageTable_.end()) {
        return false;  // Page not in buffer pool
    }
    
    size_t frameId = iter->second;
    std::shared_ptr<Page> page = pages_[frameId];
    
    // Write the page to disk if dirty
    if (page->isDirty()) {
        if (!writePageToDisk(pageId, page)) {
            return false;
        }
        page->setDirty(false);
    }
    
    return true;
}

bool BufferPoolManager::flushAllPages() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    bool success = true;
    
    // Flush all dirty pages
    for (const auto& entry : pageTable_) {
        int pageId = entry.first;
        size_t frameId = entry.second;
        std::shared_ptr<Page> page = pages_[frameId];
        
        if (page->isDirty()) {
            if (!writePageToDisk(pageId, page)) {
                success = false;
            } else {
                page->setDirty(false);
            }
        }
    }
    
    return success;
}

bool BufferPoolManager::deletePage(int pageId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if the page is in the buffer pool
    auto iter = pageTable_.find(pageId);
    if (iter == pageTable_.end()) {
        return true;  // Page not in buffer pool, nothing to do
    }
    
    size_t frameId = iter->second;
    std::shared_ptr<Page> page = pages_[frameId];
    
    // Cannot delete a pinned page
    if (page->getPinCount() > 0) {
        return false;
    }
    
    // Remove the page from the buffer pool
    pageTable_.erase(pageId);
    lruList_.erase(lruMap_[pageId]);
    lruMap_.erase(pageId);
    pages_[frameId] = nullptr;
    
    // Delete the page file from disk
    std::string pageFile = dbPath_ + "/page_" + std::to_string(pageId) + ".dat";
    std::remove(pageFile.c_str());
    
    return true;
}

void BufferPoolManager::setDbPath(const std::string& dbPath) {
    dbPath_ = dbPath;
}

int BufferPoolManager::findVictim() {
    // First, look for an empty frame
    for (size_t i = 0; i < poolSize_; ++i) {
        if (pages_[i] == nullptr) {
            return static_cast<int>(i);
        }
    }
    
    // If no empty frame, use LRU to find a victim
    for (auto it = lruList_.rbegin(); it != lruList_.rend(); ++it) {
        int pageId = *it;
        size_t frameId = pageTable_[pageId];
        
        if (pages_[frameId]->getPinCount() == 0) {
            return static_cast<int>(frameId);
        }
    }
    
    // If all pages are pinned, return -1
    return -1;
}

bool BufferPoolManager::readPageFromDisk(int pageId, std::shared_ptr<Page> page) {
    std::string pageFile = dbPath_ + "/page_" + std::to_string(pageId) + ".dat";
    
    std::ifstream file(pageFile, std::ios::binary);
    if (!file) {
        return false;  // Page doesn't exist on disk
    }
    
    file.read(page->getData(), pageSize_);
    return true;
}

bool BufferPoolManager::writePageToDisk(int pageId, std::shared_ptr<Page> page) {
    std::string pageFile = dbPath_ + "/page_" + std::to_string(pageId) + ".dat";
    
    std::ofstream file(pageFile, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open page file for writing: " << pageFile << std::endl;
        return false;
    }
    
    file.write(page->getData(), pageSize_);
    return true;
}

} // namespace sqldb 