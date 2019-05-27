#ifndef SIMPLEKVSTORE_H_
#define SIMPLEKVSTORE_H_

#include <fcntl.h>
#include <unistd.h>
#include <string>
using std::string;

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

}

template <typename K, typename V>
V SimpleKVStore<K, V>::get(long offset)
{
  
}

} // namespace kvsvr

#endif // SIMPLEKVSTORE_H_