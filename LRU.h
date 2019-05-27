#ifndef LRR_H_
#define LRU_H_

#include <map>

namespace kvsvr
{

template <typename K, typename V>
struct CacheNode
{
  K key;
  V value;
  CacheNode *pre, *next;
  CacheNode(K k, V v) : key(k), value(v), pre(nullptr), next(nullptr) {}
};

template <typename K, typename V>
class LRUCache
{
  typedef CacheNode<K, V> Node;
  static const unsigned int MAX_SIZE = 10;

public:
  LRUCache(unsigned int capacity) : max_size_(capacity), head_(nullptr), tail_(nullptr)
  {
  }

  LRUCache() : max_size_(MAX_SIZE), head_(nullptr), tail_(nullptr)
  {
  }

  ~LRUCache()
  {
    while(!m_.empty())
    {
      auto iter = m_.begin();
      delete iter->second;
      m_.erase(iter);
    }
  }

/**
 * @brief Get the value with key
 * 
 * @param key 
 * @param value Return value
 * @return true Return true if exit
 * @return false Return false if not exit
 */
  bool get(const K &key, V &value)
  {
    auto iter = m_.find(key);
    if(iter != m_.end())
    {
      Node *node = iter->second;
      remove(node);
      setHead(node);
      value = node->value;
      return true;
    }
    return false;
  }


/**
 * @brief Set the value of the key 
 * 
 * @param key 
 * @param value 
 */
  void set(const K &key, const V &value)
  {
    auto iter = m_.find(key);
    if(iter != m_.end())  // If in the cache, move the node to the head
    {
      Node *node = iter->second;
      node->value = value;
      remove(node);
      setHead(node);
    }
    else  // If not in the cache, create a new node.
    {
      Node *new_node = new Node(key, value);
      if(m_.size() >= max_size_)  // if the cache is full, remove the tail and set the new node to the head.
      {
        auto iter = m_.find(tail_->key);
        Node *del_tail = tail_;
        remove(tail_);
        delete del_tail;
        m_.erase(iter);
      }
      setHead(new_node);
      m_[key] = new_node;
    }
  }

private:
  void remove(Node *node)
  {
    if(node->pre != nullptr)
      node->pre->next = node->next;
    else
      head_ = node->next;
    
    if(node->next != nullptr)
      node->next->pre = node->pre;
    else
      tail_ = node->pre;

  }

  void setHead(Node* node)
  {
    node->next = head_;
    node->pre = nullptr;

    if(head_ != nullptr)
      head_->pre = node;
    
    head_ = node;

    if(tail_ == nullptr)
      tail_  = head_;
  }

private:
  unsigned int max_size_;  // the LRU cache max size
  Node *head_, *tail_;
  std::map<K, Node *> m_;  // store the node of the key
};

} // namespace kvsvr

#endif // namespace LRU_H_