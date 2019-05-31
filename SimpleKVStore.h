#ifndef SIMPLEKVSTORE_H_
#define SIMPLEKVSTORE_H_

#include <fcntl.h>
#include <unistd.h>

#include <string>
using std::string;

#include <boost/lexical_cast.hpp>

#define LOGGER_WARN
#define LOGGER_DEBUG
#include <logger.h>

namespace kvsvr
{

template <typename K, typename V>
class SimpleKVStore
{
public:
  SimpleKVStore(string client_name);

  long set(const K &key, const V &value);

  V get(long offset);

  void erase(const K &key);

private:
  string client_name_;
  int file_fd_;
  FILE *file_;
};

template <typename K, typename V>
SimpleKVStore<K, V>::SimpleKVStore(string client_name) 
  : client_name_(client_name)
{
  string default_doc_ = "client_file/";
  string file_name = default_doc_ + client_name_ + ".aof";

  file_fd_ = open(file_name.c_str(), O_RDWR | O_CREAT | O_EXCL, S_IRWXU);
  if (file_fd_ == -1) //  file already exists
  {
    file_fd_ = open(file_name.c_str(), O_RDWR); // open again
    file_ = fopen(file_name.c_str(), "a+");
    if (file_ == nullptr)
      WARN("open file %s error !!", file_name.c_str());
  }
  else
  {
    file_ = fopen(file_name.c_str(), "a+");
    if (file_ == nullptr)
      WARN("open file %s error !!", file_name.c_str());
    INFO("Create database: %s\n", client_name_.c_str());

    string copy_right = "Simple Key-Value Server AOF\nAuthor: Kingsley\nCopyright (c) 2019\n";
    write(file_fd_, copy_right.c_str(), copy_right.size());
    string create_info = "database " + client_name_ + "\n";
    write(file_fd_, create_info.c_str(), create_info.size());
  }
}

template <typename K, typename V>
long SimpleKVStore<K, V>::set(const K &key, const V &value)
{
  string key_msg, value_msg;
  try
  {
    key_msg = boost::lexical_cast<string>(key);
    value_msg = boost::lexical_cast<string>(value);
  }
  catch(boost::bad_lexical_cast& e)
  {
    std::cerr << e.what() << '\n';
  }
  
  lseek(file_fd_, 0, SEEK_END);
  string msg_begin = "-set <" + key_msg + "> <";
  write(file_fd_, msg_begin.c_str(), msg_begin.size());
  long offset = ftell(file_);
  string msg_end = value_msg + ">\n";
  write(file_fd_, msg_end.c_str(), msg_end.size());
  return offset;
}

template <typename K, typename V>
V SimpleKVStore<K, V>::get(long offset)
{
  lseek(file_fd_, offset, SEEK_SET);
  char value[255];
  int index = 0;
  read(file_fd_, value, 1);
  while(value[index] != '>')
  {
    ++index;
    read(file_fd_, value + index, 1);
  }
  value[++index] = '\0';
  string value_msg = value;
  try
  {
    V value_ret = boost::lexical_cast<V>(value_msg);
    return value_ret;
  }
  catch(boost::bad_lexical_cast& e)
  {
    std::cerr << e.what() << '\n';
  }
}

template <typename K, typename V>
void SimpleKVStore<K, V>::erase(const K &key)
{
  string key_msg;
  try
  {
    key_msg = boost::lexical_cast<string>(key);
  }
  catch(const boost::bad_lexical_cast& e)
  {
    std::cerr << e.what() << '\n';
  }
  lseek(file_fd_, 0, SEEK_END);
  string msg = "-delete <" + key_msg + ">\n";
  write(file_fd_, msg.c_str(), msg.size());
}

} // namespace kvsvr

#endif // SIMPLEKVSTORE_H_