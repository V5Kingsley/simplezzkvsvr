#ifndef KVSVR_POOL_H_
#define KVSVR_POOL_H_

#include <sys/socket.h>
#include <thread_pool.h>
#include "SimpleKVIndex.h"
#include <map>
#include <boost/thread/thread.hpp>
using std::string;

namespace kvsvr
{

#define BTREE_ORDER 5

typedef boost::shared_lock<boost::shared_mutex> read_lock;
typedef boost::unique_lock<boost::shared_mutex> write_lock;

struct clientInfo
{
  int fd;
  string client_name;
  std::shared_ptr<SimpleKVIndex<string, string, BTREE_ORDER>> client_file;
  //my_mutex::MutexLock client_mutex;
  boost::shared_mutex client_mutex;
  
  bool reform;
};

class KVSVRPool : public ThreadPool
{

public:
  KVSVRPool(parameters::Parameters *pool_parameters) 
    : ThreadPool(pool_parameters),
      succeed_response_("+OK"),
      fail_response_("+ERROR")
  {
  }

  virtual void thread_routine(int index) override;

  virtual void distribute_task() override;

  virtual ~KVSVRPool(){}

  virtual void close_pool() override;

  status add_kv_task(int client_fd, string client_name, string kv_task);

  void get_kv_task(struct clientInfo *client, const string &key);

  void set_kv_task(struct clientInfo *client, const string &key, const string &value);

  void delete_kv_task(struct clientInfo *client, const string &key);

private:
  std::map<string, struct clientInfo*> client_map_;

  string succeed_response_;
  string fail_response_;

};


} // namespace kvsvr


#endif // KVSVR_POOL_H_