#ifndef BTREE_INDEX_H
#define BTREE_INDEX_H

#include <vector>
#include <memory>
#include <string>
#include <functional>
#include "../DataType.h"

namespace sqldb {

/**
 * @class BTreeNode
 * @brief A node in the B-Tree index
 */
template <typename KeyType>
class BTreeNode {
public:
    /**
     * @brief Constructor for BTreeNode
     * @param isLeaf Whether this node is a leaf node
     * @param order The order of the B-Tree
     */
    BTreeNode(bool isLeaf = true, int order = 3);
    
    /**
     * @brief Check if the node is a leaf node
     * @return true if the node is a leaf node, false otherwise
     */
    bool isLeaf() const;
    
    /**
     * @brief Insert a key-value pair into the node
     * @param key The key to insert
     * @param recordId The record ID associated with the key
     * @return true if insertion was successful, false otherwise
     */
    bool insert(const KeyType& key, int recordId);
    
    /**
     * @brief Search for a key in the node
     * @param key The key to search for
     * @return The record ID associated with the key, or -1 if not found
     */
    int search(const KeyType& key) const;
    
    /**
     * @brief Split the node when it's full
     * @param parent The parent node
     * @param childIndex The index of this node in the parent
     * @return true if split was successful, false otherwise
     */
    bool split(std::shared_ptr<BTreeNode<KeyType>> parent, int childIndex);
    
private:
    // Whether this node is a leaf node
    bool isLeaf_;
    
    // The order of the B-Tree
    int order_;
    
    // The keys stored in this node
    std::vector<KeyType> keys_;
    
    // The record IDs associated with the keys (for leaf nodes)
    std::vector<int> recordIds_;
    
    // The child nodes (for non-leaf nodes)
    std::vector<std::shared_ptr<BTreeNode<KeyType>>> children_;
    
    // The next leaf node (for leaf nodes)
    std::shared_ptr<BTreeNode<KeyType>> nextLeaf_;
};

/**
 * @class BTreeIndex
 * @brief B-Tree index implementation for efficient data retrieval
 */
template <typename KeyType>
class BTreeIndex {
public:
    /**
     * @brief Constructor for BTreeIndex
     * @param columnName The name of the column to index
     * @param dataType The data type of the column
     * @param order The order of the B-Tree
     */
    BTreeIndex(const std::string& columnName, DataType dataType, int order = 3);
    
    /**
     * @brief Insert a key-value pair into the index
     * @param key The key to insert
     * @param recordId The record ID associated with the key
     * @return true if insertion was successful, false otherwise
     */
    bool insert(const KeyType& key, int recordId);
    
    /**
     * @brief Search for a key in the index
     * @param key The key to search for
     * @return The record ID associated with the key, or -1 if not found
     */
    int search(const KeyType& key) const;
    
    /**
     * @brief Range search in the index
     * @param startKey The start key of the range
     * @param endKey The end key of the range
     * @param comparator The comparator function for the range search
     * @return A vector of record IDs in the range
     */
    std::vector<int> rangeSearch(const KeyType& startKey, const KeyType& endKey, 
                                std::function<bool(const KeyType&, const KeyType&)> comparator) const;
    
    /**
     * @brief Delete a key from the index
     * @param key The key to delete
     * @return true if deletion was successful, false otherwise
     */
    bool remove(const KeyType& key);
    
    /**
     * @brief Save the index to disk
     * @param filename The filename to save to
     * @return true if save was successful, false otherwise
     */
    bool saveToFile(const std::string& filename) const;
    
    /**
     * @brief Load the index from disk
     * @param filename The filename to load from
     * @return true if load was successful, false otherwise
     */
    bool loadFromFile(const std::string& filename);
    
private:
    // The name of the column to index
    std::string columnName_;
    
    // The data type of the column
    DataType dataType_;
    
    // The order of the B-Tree
    int order_;
    
    // The root node of the B-Tree
    std::shared_ptr<BTreeNode<KeyType>> root_;
    
    /**
     * @brief Search for a node containing the key
     * @param key The key to search for
     * @param node The current node
     * @param path A vector to store the path from root to the node
     * @return The node containing the key, or nullptr if not found
     */
    std::shared_ptr<BTreeNode<KeyType>> searchNode(const KeyType& key, 
                                                  std::shared_ptr<BTreeNode<KeyType>> node,
                                                  std::vector<std::shared_ptr<BTreeNode<KeyType>>>& path) const;
};

} // namespace sqldb

#endif // BTREE_INDEX_H 