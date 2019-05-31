#ifndef BTREE_INDEX_H_
#define BTREE_INDEX_H_

#include <stdio.h>
#include <queue>
#include <iostream>
#include <assert.h>

namespace kvsvr
{

template <class k>
struct KeyIndex
{
  k key;      //key
  long offset; // the offset of the key in the file
};

template <class k, size_t M>
struct BTreeNode
{
  BTreeNode()
      : size_(0), pParent_(nullptr), leaf_(true)
  {
    for (size_t idx = 0; idx < M + 1; ++idx)
      pSub_[idx] = nullptr;
  }

  k keys_[M];              // key array
  BTreeNode *pSub_[M + 1]; // the pointers of the children
  BTreeNode *pParent_;     // the pointer of the parent
  size_t size_;            // the size of the valid keys
  bool leaf_;
};

/**
 * @brief BTree class used to store the offset of the index key in file 
 * 
 * @tparam k The type of the key
 * @tparam M The order number of the BTree. The number of children for one node. M must not be less than 3
 */
template <class k, size_t M = 3>
class BTreeIndex
{
  typedef BTreeNode<KeyIndex<k>, M> Node;

public:
  BTreeIndex()
    : pRoot_(nullptr), 
      min_keys_size_((M + 1) / 2 - 1), 
      max_child_size_(M + 1),
      size_(0)
  {
    assert(M >= 3);
  }

  ~BTreeIndex()
  {
    if (pRoot_)
    {
      std::queue<Node *> q;
      q.push(pRoot_);

      while (!q.empty())
      {
        Node *top = q.front();
        int index;
        for (index = 0; index < top->size_; ++index)
        {
          if (top->pSub_[index])
            q.push(top->pSub_[index]);
        }
        if (top->pSub_[index])
          q.push(top->pSub_[index]);
        q.pop();
        delete top;
      }
    }
  }

/**
 * @brief Find the key in BTree
 * 
 * @param key The target key
 * @return std::pair<Node *, int> The target node and its index. While not exit, return the last node and -1
 */
  std::pair<Node *, int> find(const k &key)
  {
    Node *pCur = pRoot_;
    Node *pParent = nullptr;

    while (pCur)
    {
      int index = 0;
      while (index < pCur->size_)
      {
        if (pCur->keys_[index].key > key) // Jump the to the child node
          break;
        else if (pCur->keys_[index].key < key) // to next key in this node
          index++;
        else
          return std::pair<Node *, int>(pCur, index); // return the target node and its index
      }

      pParent = pCur;
      pCur = pCur->pSub_[index]; // Jump the to the child node
    }
    return std::pair<Node *, int>(pParent, -1); // not found
  }

  /**
 * @brief Insert KeyIndex to BTree
 * 
 * @param new_keyIndex 
 * @return true KeyIndex is not in the BTree
 * @return false KeyIndex is already in the BTree. Update its offset
 */
  bool insert(const KeyIndex<k> &new_keyIndex)
  {
    if (pRoot_ == nullptr)
    {
      pRoot_ = new Node;
      pRoot_->keys_[0] = new_keyIndex;
      pRoot_->size_ = 1;
      ++size_;
      return true;
    }

    std::pair<Node *, int> ret = find(new_keyIndex.key);
    if (ret.second != -1) // The KeyIndex is already in the BTree.
    {
      //ret.first->keys_[ret.second].offset = new_keyIndex.offset;
      return false;
    }

    ++size_;

    Node *pNode = ret.first;
    Node *pSub = nullptr;
    KeyIndex<k> valuek = new_keyIndex;

    while (true)
    {
      _insert(pNode, valuek, pSub);

      if (pNode->size_ < M) // The node is not full. Return success.
        return true;

      //Split the full node
      Node *pNewNode = new Node;
      size_t mid = M / 2;
      size_t index = 0;
      size_t idx = 0;

      size_t pNode_size = pNode->size_;
      // Move elements and child pointers to the new node
      for (idx = mid + 1; idx < pNode_size; ++idx)
      {
        pNewNode->keys_[index] = pNode->keys_[idx];
        pNewNode->pSub_[index] = pNode->pSub_[idx];
        if (pNode->pSub_[idx])
        {
          pNewNode->leaf_ = false;
          pNode->pSub_[idx]->pParent_ = pNewNode; // update the the parent node of the child
          pNode->pSub_[idx] = nullptr;    // remove the original child node
        }
        ++(pNewNode->size_);
        --(pNode->size_);
        ++index;
      }

      pNewNode->pSub_[index] = pNode->pSub_[idx]; // the last child node
      if (pNode->pSub_[idx])
      {
        pNode->pSub_[idx]->pParent_ = pNewNode;
        pNode->pSub_[idx] = NULL;
      }

      --(pNode->size_);
      if (pNode->pParent_ == nullptr) // If the node is the parent node
      {
        pRoot_ = new Node; // create a new node the store the mid value.
        pRoot_->keys_[0] = pNode->keys_[mid];
        pRoot_->pSub_[0] = pNode;
        pNode->pParent_ = pRoot_;
        pRoot_->pSub_[1] = pNewNode;
        pNewNode->pParent_ = pRoot_;
        ++(pRoot_->size_);
        pRoot_->leaf_ = false;
        return true;
      }
      else // Insert the mid value to its parent node
      {
        valuek = pNode->keys_[mid];
        pNode = pNode->pParent_;
        pSub = pNewNode;
      }
    }
  }

  bool insert(const k &key, long offset)
  {
    KeyIndex<k> new_key{key, offset};
    return insert(new_key);
  }

/**
 * @brief Erase key from tree
 * 
 * @param key 
 * @return true 
 * @return false When the key is not in the tree, return false 
 */
  bool erase(const k &key)
  {
    if(pRoot_ == nullptr)
      return false;
    
    std::pair<Node*, int> erase_node = find(key);
    if(erase_node.second == -1)
      return false;
    
    --size_;

    Node *node = erase_node.first;
    int pos = erase_node.second;

    if(!node->leaf_)  // not leaf node
    {
      Node *replace = nullptr;
      // get the last right key of the left tree
      _getLtreeRnode(node, pos, replace); 
      node->keys_[pos] = replace->keys_[replace->size_ - 1];  // replace the node
      node = replace;   // erase the leaf node
      --replace->size_;
    }
    else
    {
      for(int i = pos; i < (node->size_ - 1); ++i)
        node->keys_[i] = node->keys_[i + 1];
      --node->size_;
    }

    while(1)
    {
      if(node->pParent_ == nullptr) // root node
      {
        if(node->size_ == 0)  // empty root node
        {
          if(!node->leaf_)  // not leaf node
          {
            pRoot_ = node->pSub_[0];  // set the first child to root
            pRoot_->pParent_ = nullptr;
          }
          else
          {
            pRoot_ = nullptr;  // empty tree
          }
          delete node;
        }
        return true;
      }

      if(node->size_ < min_keys_size_) // size < (M + 1) / 2 - 1
      {
        int child_pos = _getChildPosFromParent(node->pParent_, node);
        Node *parent = node->pParent_;
        if(child_pos > 0 && parent->pSub_[child_pos - 1]->size_ > min_keys_size_)
        { // borrow the last key from left brother
          Node *left = parent->pSub_[child_pos - 1];
          for(int i = node->size_; i >= 1; --i)  // move backward
          {
            node->keys_[i] = node->keys_[i - 1];
            node->pSub_[i + 1] = node->pSub_[i];
          }
          node->pSub_[1] = node->pSub_[0];
          node->keys_[0] = parent->keys_[child_pos - 1];  // insert the parent key to the first place
          ++node->size_;
          node->pSub_[0] = left->pSub_[left->size_];  // take over the last child of the brother
          if(node->pSub_[0])
            node->pSub_[0]->pParent_ = node;  // don't forget to handle its parent
          parent->keys_[child_pos - 1] = left->keys_[left->size_ - 1];  // brother key → parent key
          --left->size_;
          return true;
        }
        else if(child_pos < parent->size_ && parent->pSub_[child_pos + 1]->size_ > min_keys_size_)
        { // borrow the first key from right brother
          Node *right = parent->pSub_[child_pos + 1];
          node->keys_[node->size_] = parent->keys_[child_pos]; // insert the parent key to the last place
          ++node->size_;
          parent->keys_[child_pos] = right->keys_[0];  // brother key → parent key
          node->pSub_[node->size_] = right->pSub_[0];  // take over the first child of the brother
          if(node->pSub_[node->size_])
            node->pSub_[node->size_]->pParent_ = node;  // don't forget to handle its parent

          for(int i = 0; i < right->size_; ++i)  // move forward
          {
            right->keys_[i] = right->keys_[i + 1];
            right->pSub_[i] = right->pSub_[i + 1];
          }
          right->pSub_[right->size_] = nullptr;
          --right->size_;
          return true;
        }
        else
        {  // can not borrow from brothers. merge node with left of right brother
          Node *merged_node = nullptr;
          if(child_pos > 0)  // merge node with left brother
          { 
            Node *left = parent->pSub_[child_pos - 1];
            //merged_node = combine(parent, left, node, child_pos - 1);
            merged_node = combine(parent, left, node, child_pos, true);
          }
          else if(child_pos < parent->size_)  // merge node with right brother
          {
            Node *right = parent->pSub_[child_pos + 1];
            //merged_node = combine(parent, right, node, child_pos);
            merged_node = combine(parent, right, node, child_pos, false);
          }
          node = merged_node->pParent_;  //  tracking back. the parent may not be legal.
        }
        
      }
      else  // size >= (M + 1) / 2 - 1
      {
        return true;
      }
      
    }
    
  }

  /**
 * @brief Set the key offset 
 * 
 * @param key 
 * @param offset 
 * @return true If the key is in the BTree, return true,
 * @return false else return false.
 */
  bool setKeyOffset(const k &key, long offset)
  {
    std::pair<Node *, int> ret = find(key);
    if (ret.second == -1)
      return false;

    ret.first->keys_[ret.second].offset = offset;
    return true;
  }

  /**
 * @brief Find the offset of the key
 * 
 * @param key 
 * @param offset return offset
 * @return true If the key is in the BTree, return true,
 * @return false else return false.
 */
  bool findKeyOffset(const k &key, long &offset)
  {
    std::pair<Node *, int> ret = find(key);
    if (ret.second == -1)
      return false;

    offset = ret.first->keys_[ret.second].offset;
    return true;
  }

  /**
 * @brief Judge whether the key is in BTree
 * 
 * @param key 
 * @return true In BTree
 * @return false Not in BTree
 */
  bool isInBTree(const k &key)
  {
    std::pair<Node *, int> ret = find(key);
    if (ret.second == -1)
      return false;
    else
      return true;
  }

  size_t size()
  {
    return size_;
  }

  /**
 * @brief Level traversal the BTree. '[]' means a node.
 * 
 */
  void show()
  {
    if (pRoot_ == nullptr)
    {
      printf("Empty tree!\n");
      return;
    }

    std::queue<Node *> q;
    q.push(pRoot_);

    while (!q.empty())
    {
      Node *top = q.front();
      int index;
      std::cout << "[ ";
      for (index = 0; index < top->size_; ++index)
      {
        std::cout << top->keys_[index].key << " ";
        if (top->pSub_[index])
          q.push(top->pSub_[index]);
      }
      std::cout << "], ";
      if (top->pSub_[index])
        q.push(top->pSub_[index]);
      q.pop();
    }
    std::cout << std::endl;
  }

private:
  /**
 * @brief Insert the key the the node.
 * 
 * @param pNode The node uesd to place the key
 * @param key 
 * @param pSub the child node of the key
 */
  void _insert(Node *&pNode, const KeyIndex<k> &key, Node *pSub)
  {
    int end = pNode->size_ - 1;
    while (end > -1)
    {
      if (pNode->keys_[end].key > key.key) // Backward shift the bigger key and its child node.
      {
        pNode->keys_[end + 1] = pNode->keys_[end];
        pNode->pSub_[end + 2] = pNode->pSub_[end + 1];
      }
      else
        break;
      --end;
    }
    pNode->keys_[end + 1] = key; // Find the correct node to put the key and child node
    pNode->pSub_[end + 2] = pSub;
    if (pSub)
      pSub->pParent_ = pNode;
    ++(pNode->size_);
  }

  bool _getLtreeRnode(Node* start, int keypos, Node *&res)
  {
    if(start == nullptr)
      return false;
    
    start = start->pSub_[keypos];
    while(start->pSub_[0] != nullptr)
      start = start->pSub_[start->size_];
    res = start;
    return true;
  }


/**
 * @brief Get the index of the child node in parent node
 * 
 * @param parent 
 * @param child 
 * @return int the index of the child node in parent node
 */
  int _getChildPosFromParent(Node *parent, Node *child)
  {
    for(int i = 0; i < max_child_size_; ++i)
    {
      if(child == parent->pSub_[i])
        return i;
    }
    return -1;
  }

  bool isLeafNode(Node *node)
  {
    for(int i = 0; i < node->size_ + 1; ++i)
      if(node->pSub_[i] != nullptr)
        return false;
    
    return true;
  }

  /*Node* combine(Node *parent, Node *brother, Node *node, int parent_key_pos)
  {
    auto key = parent->keys_[parent_key_pos];
    for(int i = parent_key_pos; i < parent->size_; ++i)  // parent node moves forward
    {
      parent->keys_[i] = parent->keys_[i + 1];
      parent->pSub_[i + 1] = parent->pSub_[i + 2];
    }
    --parent->size_;

    // put the parent key to the right place.
    if(node->size_ == 0 || key.key >= node->keys_[node->size_ - 1].key)
    { 
      node->keys_[node->size_] = key;
      node->pSub_[node->size_ + 1] = brother->pSub_[0];  // take over the first child of the brother node
      ++node->size_;
    }
    else
    {
      brother->keys_[brother->size_] = key;
      brother->pSub_[brother->size_ + 1] = node->pSub_[0];   // take over the first child of the node
      ++brother->size_;
    }

    Node *merged_node = new Node;
    merged_node->pParent_ = parent;
    merged_node->leaf_ = node->leaf_;
    if(node->keys_[0].key > brother->keys_[brother->size_ - 1].key)  // merged  node starts from the brother
    {
      merged_node->pSub_[0] = brother->pSub_[0];
      if(merged_node->pSub_[0])
        merged_node->pSub_[0]->pParent_ = merged_node;
      for(int i = 0; i < brother->size_; ++i)
      {
        merged_node->keys_[i] = brother->keys_[i];
        merged_node->pSub_[i + 1] = brother->pSub_[i + 1];
        if(merged_node->pSub_[i + 1])
          merged_node->pSub_[i + 1]->pParent_ = merged_node;
      }

      for(int i = brother->size_; i < node->size_ + brother->size_; ++i)
      {
        merged_node->keys_[i] = node->keys_[i - brother->size_];
        merged_node->pSub_[i + 1] = node->pSub_[i - brother->size_ + 1];
        if(merged_node->pSub_[i + 1])
          merged_node->pSub_[i + 1]->pParent_ = merged_node;
      }
    }
    else  // merged  node starts from the node
    {
      merged_node->pSub_[0] = node->pSub_[0];
      if(merged_node->pSub_[0])
        merged_node->pSub_[0]->pParent_ = merged_node;
      for(int i = 0; i < node->size_; ++i)
      {
        merged_node->keys_[i] = node->keys_[i];
        merged_node->pSub_[i + 1] = node->pSub_[i + 1];
        if(merged_node->pSub_[i + 1])
          merged_node->pSub_[i + 1]->pParent_ = merged_node;
      }

      for(int i = node->size_; i < node->size_ + brother->size_; ++i)
      {
        merged_node->keys_[i] = brother->keys_[i - node->size_];
        merged_node->pSub_[i + 1] = brother->pSub_[i - node->size_ + 1];
        if(merged_node->pSub_[i + 1])
          merged_node->pSub_[i + 1]->pParent_ = merged_node;
      }
    }
    merged_node->size_ = node->size_ + brother->size_;
    parent->pSub_[parent_key_pos] = merged_node;
    delete node;
    delete brother;
    return merged_node;
  }*/


/**
 * @brief Combine the parent key, brother and node to a new node
 * 
 * @param parent 
 * @param brother 
 * @param node 
 * @param parent_key_pos 
 * @return Node* Return the merge node
 */
  Node* combine(Node *parent, Node *brother, Node *node, int child_pos, bool left_brother)
  {
    int parent_key_pos = left_brother ? child_pos - 1 : child_pos;
    auto key = parent->keys_[parent_key_pos];
    for(int i = parent_key_pos; i < parent->size_; ++i)  // parent node moves forward
    {
      parent->keys_[i] = parent->keys_[i + 1];
      parent->pSub_[i + 1] = parent->pSub_[i + 2];
    }
    --parent->size_;

    if(left_brother)  // brother key node
    {
      brother->keys_[brother->size_] = key;
      brother->pSub_[brother->size_ + 1] = node->pSub_[0];   // take over the first child of the node
      ++brother->size_;
    }
    else  // node key brother
    {
      node->keys_[node->size_] = key;
      node->pSub_[node->size_ + 1] = brother->pSub_[0];  // take over the first child of the brother node
      ++node->size_;
    }

    Node *merged_node = new Node;
    merged_node->pParent_ = parent;
    merged_node->leaf_ = node->leaf_;

    if(left_brother)  // merged node starts with brother. brother key node
    {
      merged_node->pSub_[0] = brother->pSub_[0];
      if(merged_node->pSub_[0])
        merged_node->pSub_[0]->pParent_ = merged_node;
      for(int i = 0; i < brother->size_; ++i)
      {
        merged_node->keys_[i] = brother->keys_[i];
        merged_node->pSub_[i + 1] = brother->pSub_[i + 1];
        if(merged_node->pSub_[i + 1])
          merged_node->pSub_[i + 1]->pParent_ = merged_node;
      }

      for(int i = brother->size_; i < node->size_ + brother->size_; ++i)
      {
        merged_node->keys_[i] = node->keys_[i - brother->size_];
        merged_node->pSub_[i + 1] = node->pSub_[i - brother->size_ + 1];
        if(merged_node->pSub_[i + 1])
          merged_node->pSub_[i + 1]->pParent_ = merged_node;
      }
    }
    else  // merged node starts with node. node key brother
    {
      merged_node->pSub_[0] = node->pSub_[0];
      if(merged_node->pSub_[0])
        merged_node->pSub_[0]->pParent_ = merged_node;
      for(int i = 0; i < node->size_; ++i)
      {
        merged_node->keys_[i] = node->keys_[i];
        merged_node->pSub_[i + 1] = node->pSub_[i + 1];
        if(merged_node->pSub_[i + 1])
          merged_node->pSub_[i + 1]->pParent_ = merged_node;
      }

      for(int i = node->size_; i < node->size_ + brother->size_; ++i)
      {
        merged_node->keys_[i] = brother->keys_[i - node->size_];
        merged_node->pSub_[i + 1] = brother->pSub_[i - node->size_ + 1];
        if(merged_node->pSub_[i + 1])
          merged_node->pSub_[i + 1]->pParent_ = merged_node;
      }
    }
    merged_node->size_ = node->size_ + brother->size_;
    parent->pSub_[parent_key_pos] = merged_node;
    delete node;
    delete brother;
    return merged_node;
  }

private:
  Node *pRoot_;
  size_t min_keys_size_;
  size_t max_child_size_;
  size_t size_;
};

} // namespace kvsvr

#endif // BTREE_INDEX_H_