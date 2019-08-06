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

  void init();

  long set(const K &key, const V &value);

  V get(long offset);

  void erase(const K &key);

  FILE* getFile() { return file_; }

private:
  string client_name_;
  int file_fd_;
  FILE *file_;
};

#include "SimpleKVStore.hpp"

} // namespace kvsvr

#endif // SIMPLEKVSTORE_H_