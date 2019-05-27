#ifndef SIMPLEKVINDEX_H_
#define SIMPLEKVINDEX_H_

#include "LRU.h"
#include "BTree_index.h"
#include "SimpleKVStore.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>

#define LOGGER_WARN
#define LOGGER_DEBUG
#include <logger.h>

using std::string;

namespace kvsvr  
{

template <typename K, typename V, size_t M>
class SimpleKVIndex
{
public:
  SimpleKVIndex(string client_name, unsigned int cache_size)
    : BTree_(),
      lru_(cache_size),
      KVStore_(new SimpleKVStore<K, V>(client_name))
  {
  }

  void set(const K &key, const V &value);

  bool get(const K &key, V &value);

private:
  BTreeIndex<K, M> BTree_;
  LRUCache<K, V> lru_;
  FILE *file_;
  int file_fd_;
  std::shared_ptr<SimpleKVStore<K, V>> KVStore_;
};


template <typename K, typename V, size_t M>
void SimpleKVIndex<K, V, M>::set(const K &key, const V &value)
{
  long offset = KVStore_->set(key, value);
  long last_offset;
  if (BTree_.findKeyOffset(key, last_offset) == false)  // not exit
  {
    std::cout << key << " is not in tree" << std::endl;
    BTree_.insert(key, offset);
  }
  else
    BTree_.setKeyOffset(key, offset);
  
  lru_.set(key, value);
}

template <typename K, typename V, size_t M>
bool SimpleKVIndex<K, V, M>::get(const K &key, V &value)
{
  if (lru_.get(key, value) == true)
    return true;

  long offset;
  if (BTree_.findKeyOffset(key, offset) == false)
    return false;

  // get value from file
  value = KVStore_->get(offset);
  lru_.set(key, value);
  return true;
}


} // namespace kvsvr

#endif // SIMPLEKVINDEX_H_