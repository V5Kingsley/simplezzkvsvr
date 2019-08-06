#ifndef SIMPLEKVINDEX_H_
#define SIMPLEKVINDEX_H_

#include "LRU.h"
#include "BTree_index.h"
#include "SimpleKVStore.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <boost/lexical_cast.hpp>

#define LOGGER_WARN
#define LOGGER_DEBUG
#include <logger.h>

using std::string;

namespace kvsvr  
{

namespace task_type
{
enum taskType{GET, SET, DELETE, STATS, QUIT, ERROR};
}


static task_type::taskType get_task_type(const string & task_str, string &key, string &value)
{
  int length = task_str.length();
  int i = 0;
  if(task_str[i++] != '-')
    return task_type::ERROR;

  string task_type;

  while (i != length && task_str[i] != ' ')
    task_type.push_back(task_str[i++]);

  if (task_type == "get")
  {
    string key_str;
    if(task_str[i++] != ' ')
      return task_type::ERROR;
    if(task_str[i++] != '<')
      return task_type::ERROR;
    while(i != length && task_str[i] != '>')
      key_str.push_back(task_str[i++]);
    if(i == length - 1 && task_str[i] == '>')
    {
      key = key_str;
      return task_type::GET;
    }
    else
    {
      return task_type::ERROR;
    }
  }
  else if (task_type == "set")
  {
    string key_str, value_str;
    if(task_str[i++] != ' ')
      return task_type::ERROR;
    if(task_str[i++] != '<')
      return task_type::ERROR;
    while(i != length && task_str[i] != '>')
      key_str.push_back(task_str[i++]);
    if(i < length - 4 && task_str[i] == '>' && task_str[i + 1] == ' ' && task_str[i + 2] == '<')
    {
      i += 3;
      while(i != length && task_str[i] != '>')
        value_str.push_back(task_str[i++]);
      if(i == length - 1 && task_str[i] == '>')
      {
        key = key_str;
        value = value_str;
        return task_type::SET;
      }
      else
      {
        return task_type::ERROR;
      }
    }
    else
    {
      return task_type::ERROR;
    }
  }
  else if (task_type == "delete")
  {
    string key_str;
    if(task_str[i++] != ' ')
      return task_type::ERROR;
    if(task_str[i++] != '<')
      return task_type::ERROR;
    while(i != length && task_str[i] != '>')
      key_str.push_back(task_str[i++]);
    if(i == length - 1 && task_str[i] == '>')
    {
      key = key_str;
      return task_type::DELETE;
    }
    else
    {
      return task_type::ERROR;
    }
  }
  else if (task_type == "stats")
  {
  }
  else if (task_type == "quit")
  {
  }
  else
  {
    DEBUG("Invalid request!");
    return task_type::ERROR;
  }
}

template <typename K, typename V, size_t M>
class SimpleKVIndex
{
public:
  SimpleKVIndex(string client_name, unsigned int cache_size)
    : BTree_(),
      lru_(cache_size),
      KVStore_(new SimpleKVStore<K, V>(client_name))
  {
    reloadFile();
  }

  void set(const K &key, const V &value);

  bool get(const K &key, V &value);

  bool erase(const K &key);

  bool reloadFile();

private:
  BTreeIndex<K, M> BTree_;
  LRUCache<K, V> lru_;
  std::shared_ptr<SimpleKVStore<K, V>> KVStore_;
};


template <typename K, typename V, size_t M>
void SimpleKVIndex<K, V, M>::set(const K &key, const V &value)
{
  long offset = KVStore_->set(key, value);
  long last_offset;
  if (BTree_.findKeyOffset(key, last_offset) == false)  // not exit
  {
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

template <typename K, typename V, size_t M>
bool SimpleKVIndex<K, V, M>::erase(const K &key)
{
  if(BTree_.erase(key) == false)
    return false;
  
  lru_.erase(key);
  KVStore_->erase(key);
  return true;
}

template <typename K, typename V, size_t M>
bool SimpleKVIndex<K, V, M>::reloadFile()
{
  FILE* file = KVStore_->getFile();

  if(file == NULL)
  {
    LOGGER_WARN("empty file!\n");
    return false;
  }

  char buf[BUFSIZ];
  for(int i = 0; i < 4; ++i)
  {
    if(feof(file))
    {
      return true;
    }
    fgets(buf, sizeof(buf), file);
  }

  while(!feof(file))
  {
    char command[100];
    long offset = ftell(file);
    bool offset_find = false;
    int flag_count = 0;
    fgets(command, sizeof(command), file);
    for(int i = 0; i < sizeof(command); ++i)
    {
      if(command[i] == '\0')
        break;
      if(command[i] == '\r' || command[i] == '\n')
      {
        command[i] = '\0';
        break;
      }
      if(command[i] == '<')
        ++flag_count;
      if(flag_count == 2)
      {
        offset_find = true;
        offset += i;
      }
    }
    std::string key, value;
    switch(get_task_type(command, key, value))
    {
      case task_type::GET:
      {
        break;
      }
      case task_type::SET:
      {
        K key_ret;
        V value_ret;
        try
        {
          value_ret = boost::lexical_cast<V>(value);
          key_ret = boost::lexical_cast<K>(key);
        }
        catch(boost::bad_lexical_cast& e)
        {
          std::cerr << e.what() << '\n';
          continue;
        }
        if(offset_find == true && flag_count == 2)
        {
          long last_offset;
          if (BTree_.findKeyOffset(key, last_offset) == false)  // not exit
          {
            BTree_.insert(key_ret, offset);
          }
          else
          {
            BTree_.setKeyOffset(key_ret, offset);
          }
          lru_.set(key_ret, value_ret);
        }
        break;
      }
      case task_type::DELETE:
      {
        K key_ret;
        V value_ret;
        try
        {
          key_ret = boost::lexical_cast<K>(key);
        }
        catch(boost::bad_lexical_cast& e)
        {
          std::cerr << e.what() << '\n';
          break;
        }
        if(BTree_.erase(key_ret) == true)
        {
          lru_.erase(key_ret);
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }

  return true;

}


} // namespace kvsvr

#endif // SIMPLEKVINDEX_H_