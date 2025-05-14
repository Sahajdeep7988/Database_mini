#include "../../include/storage_engine/BTreeIndex.h"
#include <algorithm>
#include <fstream>
#include <iostream>

namespace sqldb {

// BTreeNode implementation for int keys
template<>
BTreeNode<int>::BTreeNode(bool isLeaf, int order)
    : isLeaf_(isLeaf), order_(order) {
}

template<>
bool BTreeNode<int>::isLeaf() const {
    return isLeaf_;
}

template<>
bool BTreeNode<int>::insert(const int& key, int recordId) {
    // Find the position to insert
    auto it = std::lower_bound(keys_.begin(), keys_.end(), key);
    size_t pos = it - keys_.begin();
    
    // Check if key already exists
    if (it != keys_.end() && *it == key) {
        return false;  // Key already exists
    }
    
    // Insert the key
    keys_.insert(keys_.begin() + pos, key);
    
    // If this is a leaf node, insert the record ID
    if (isLeaf_) {
        recordIds_.insert(recordIds_.begin() + pos, recordId);
    } else {
        // If this is an internal node, adjust children pointers
        children_.insert(children_.begin() + pos + 1, nullptr);
    }
    
    return true;
}

template<>
int BTreeNode<int>::search(const int& key) const {
    // Find the position of the key
    auto it = std::lower_bound(keys_.begin(), keys_.end(), key);
    size_t pos = it - keys_.begin();
    
    // Check if key exists
    if (it != keys_.end() && *it == key) {
        // If this is a leaf node, return the record ID
        if (isLeaf_) {
            return recordIds_[pos];
        } else {
            // If this is an internal node, search in the appropriate child
            return children_[pos + 1]->search(key);
        }
    } else {
        // If this is a leaf node, key not found
        if (isLeaf_) {
            return -1;
        } else {
            // If this is an internal node, search in the appropriate child
            if (pos < children_.size()) {
                return children_[pos]->search(key);
            } else {
                return -1;  // No child to search
            }
        }
    }
}

template<>
bool BTreeNode<int>::split(std::shared_ptr<BTreeNode<int>> parent, int childIndex) {
    // Create a new node
    std::shared_ptr<BTreeNode<int>> newNode = std::make_shared<BTreeNode<int>>(isLeaf_, order_);
    
    // Calculate the middle index
    int midIndex = keys_.size() / 2;
    
    // Move half of the keys to the new node
    newNode->keys_.assign(keys_.begin() + midIndex + 1, keys_.end());
    keys_.resize(midIndex);
    
    // If this is a leaf node, move half of the record IDs to the new node
    if (isLeaf_) {
        newNode->recordIds_.assign(recordIds_.begin() + midIndex + 1, recordIds_.end());
        recordIds_.resize(midIndex);
        
        // Update the next leaf pointer
        newNode->nextLeaf_ = nextLeaf_;
        nextLeaf_ = newNode;
    } else {
        // If this is an internal node, move half of the children to the new node
        newNode->children_.assign(children_.begin() + midIndex + 1, children_.end());
        children_.resize(midIndex + 1);
    }
    
    // Insert the middle key into the parent
    int midKey = keys_[midIndex];
    parent->keys_.insert(parent->keys_.begin() + childIndex, midKey);
    
    // Insert the new node as a child of the parent
    parent->children_.insert(parent->children_.begin() + childIndex + 1, newNode);
    
    return true;
}

// BTreeIndex implementation for int keys
template<>
BTreeIndex<int>::BTreeIndex(const std::string& columnName, Type type, int order)
    : columnName_(columnName), dataType_(type), order_(order) {
    // Create the root node as a leaf node
    root_ = std::make_shared<BTreeNode<int>>(true, order);
}

template<>
bool BTreeIndex<int>::insert(const int& key, int recordId) {
    if (!root_) {
        root_ = std::make_shared<BTreeNode<int>>(true, order_);
    }
    
    // If the root is full, split it
    if (root_->keys_.size() >= static_cast<size_t>(order_ - 1)) {
        std::shared_ptr<BTreeNode<int>> newRoot = std::make_shared<BTreeNode<int>>(false, order_);
        newRoot->children_.push_back(root_);
        root_->split(newRoot, 0);
        root_ = newRoot;
    }
    
    // Insert the key
    std::vector<std::shared_ptr<BTreeNode<int>>> path;
    std::shared_ptr<BTreeNode<int>> node = searchNode(key, root_, path);
    
    if (!node) {
        std::cerr << "Failed to find node for key: " << key << std::endl;
        return false;
    }
    
    // Insert the key into the node
    if (!node->insert(key, recordId)) {
        std::cerr << "Failed to insert key: " << key << std::endl;
        return false;
    }
    
    // If the node is full, split it
    if (node->keys_.size() >= static_cast<size_t>(order_ - 1)) {
        // Find the parent node
        std::shared_ptr<BTreeNode<int>> parent = path.empty() ? nullptr : path.back();
        
        // If there's no parent, this is the root node
        if (!parent) {
            parent = std::make_shared<BTreeNode<int>>(false, order_);
            parent->children_.push_back(node);
            root_ = parent;
        }
        
        // Find the child index
        int childIndex = 0;
        for (size_t i = 0; i < parent->children_.size(); ++i) {
            if (parent->children_[i] == node) {
                childIndex = static_cast<int>(i);
                break;
            }
        }
        
        // Split the node
        node->split(parent, childIndex);
    }
    
    return true;
}

template<>
int BTreeIndex<int>::search(const int& key) const {
    if (!root_) {
        return -1;
    }
    
    return root_->search(key);
}

template<>
std::vector<int> BTreeIndex<int>::rangeSearch(const int& startKey, const int& endKey, 
                                             std::function<bool(const int&, const int&)> comparator) const {
    std::vector<int> results;
    
    if (!root_) {
        return results;
    }
    
    // Find the leaf node containing the start key
    std::vector<std::shared_ptr<BTreeNode<int>>> path;
    std::shared_ptr<BTreeNode<int>> node = searchNode(startKey, root_, path);
    
    if (!node || !node->isLeaf()) {
        return results;
    }
    
    // Traverse the leaf nodes
    while (node) {
        // Check each key in the node
        for (size_t i = 0; i < node->keys_.size(); ++i) {
            int key = node->keys_[i];
            
            // Check if the key is in the range
            if (comparator(key, endKey)) {
                results.push_back(node->recordIds_[i]);
            } else if (key > endKey) {
                // If we've passed the end key, we're done
                return results;
            }
        }
        
        // Move to the next leaf node
        node = node->nextLeaf_;
    }
    
    return results;
}

template<>
bool BTreeIndex<int>::remove(const int& key) {
    // TODO: Implement key removal
    return false;
}

template<>
bool BTreeIndex<int>::saveToFile(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        return false;
    }
    
    // Write index metadata
    file.write(columnName_.c_str(), columnName_.size() + 1);
    file.write(reinterpret_cast<const char*>(&dataType_), sizeof(dataType_));
    file.write(reinterpret_cast<const char*>(&order_), sizeof(order_));
    
    // TODO: Write the B-Tree structure
    
    return true;
}

template<>
bool BTreeIndex<int>::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        return false;
    }
    
    // Read index metadata
    char columnNameBuffer[256];
    file.getline(columnNameBuffer, 256, '\0');
    columnName_ = columnNameBuffer;
    
    file.read(reinterpret_cast<char*>(&dataType_), sizeof(dataType_));
    file.read(reinterpret_cast<char*>(&order_), sizeof(order_));
    
    // TODO: Read the B-Tree structure
    
    return true;
}

} // namespace sqldb 