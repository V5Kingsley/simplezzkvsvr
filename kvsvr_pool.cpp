#include "kvsvr_pool.h"

#define LOGGER_DEBUG
#include "logger.h"

namespace kvsvr
{

void KVSVRPool::distribute_task()
{
  pthread_detach(pthread_self());
  INFO("distribute_task function started. thread id: %lu\n", pthread_self())

  while(pool_activate)
  {
    sem_wait(&task_num_);
    if(pool_activate != true)
      break;
    work_thread::WorkThread* selected_thread = get_next_work_thread();
    assert(!pool_work_queue_.empty());
    work_thread::Work::WorkPtr work_to_past = pool_work_queue_.pop_work();
    selected_thread->add_work(work_to_past);
    {
      my_mutex::MutexLockGuard mlg(selected_thread->get_mutex());
      if(selected_thread->state_ == IDLE)
      {
        selected_thread->state_ = BUSY;
        selected_thread->get_condition().notify();  // notify the thread to execute the work
      }
    }
  }
  INFO("Distrubute task thread exits.\n");
}


void KVSVRPool::thread_routine(int index)
{
  pthread_detach(pthread_self());
  INFO("Generated a work thread. thread id: %lu\n", pthread_self())

  assert(index < work_threads_.size());
  work_thread::WorkThread* this_work_thread = work_threads_[index];

  while(this_work_thread->state_ == BOOTING)
  {
    my_mutex::MutexLockGuard mlg(boot_mutex_);
    boot_cond_.notify();
  }

  pthread_barrier_wait(&pool_barrier_);

  while(pool_activate)
  {
    if(this_work_thread->work_empty())
    {
      {
        my_mutex::MutexLockGuard mlg(this_work_thread->get_mutex());
        this_work_thread->state_ = IDLE;
      }
      this_work_thread->get_condition().wait();
    }
    if(pool_activate == false)
      break;
    while(!this_work_thread->work_empty())
    {
      DEBUG("get a job. thread id: %lu\n", pthread_self());
      (this_work_thread->pop_work())->execute_work();
    }
  }
  INFO("Work thread %d exits.\n", index + 1);
}

status KVSVRPool::add_kv_task(int client_fd, string client_name, string kv_task)
{
  string key, value;
  if(client_map_.find(client_name) == client_map_.end())
  {
    struct clientInfo *new_client = new struct clientInfo;
    new_client->fd = client_fd;
    new_client->client_name = client_name;
    new_client->client_file = std::shared_ptr<SimpleKVIndex<string, string, BTREE_ORDER>>(new SimpleKVIndex<string, string, BTREE_ORDER>(client_name, 10));
    new_client->reform = false;
    client_map_[client_name] = new_client;
  }

  switch (get_task_type(kv_task, key, value))
  {
  case task_type::GET:
    {
      std::shared_ptr<work_thread::Work> new_work = work_thread::Work::create_work(std::bind(&KVSVRPool::get_kv_task, this, client_map_[client_name], key));
      pool_work_queue_.push_work(new_work);
      sem_post(&task_num_);
    }
    break;
  case task_type::SET:
    {
      std::shared_ptr<work_thread::Work> new_work = work_thread::Work::create_work(std::bind(&KVSVRPool::set_kv_task, this, client_map_[client_name], key, value));
      pool_work_queue_.push_work(new_work);
      sem_post(&task_num_);
    }
    break;
  case task_type::DELETE:
    {
      std::shared_ptr<work_thread::Work> new_work = work_thread::Work::create_work(std::bind(&KVSVRPool::delete_kv_task, this, client_map_[client_name], key));
      pool_work_queue_.push_work(new_work);
      sem_post(&task_num_);
    }
    break;
  case task_type::STATS:
    {
    }
    break;
  case task_type::QUIT:
    {
    }
    break;
  default:
    {
      send(client_fd, fail_response_.c_str(), fail_response_.size(), MSG_DONTWAIT);
    }
    break;
  }
  return SUCCESS;
}

task_type::taskType KVSVRPool::get_task_type(const string & task_str, string &key, string &value)
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

void KVSVRPool::get_kv_task(struct clientInfo *client, const string &key)
{
  my_mutex::MutexLockGuard mlg(client->client_mutex);
  string value;
  if (client->client_file->get(key, value))
  {
    string response_msg = "+<" + value + ">";
    send(client->fd, response_msg.c_str(), response_msg.size(), MSG_DONTWAIT);
  }
  else
  {
    send(client->fd, fail_response_.c_str(), fail_response_.size(), MSG_DONTWAIT);
  }
}

void KVSVRPool::set_kv_task(struct clientInfo *client, const string &key, const string &value)
{
  my_mutex::MutexLockGuard mlg(client->client_mutex);
  if(client->reform == true)
  {
    // reform
  }
  else
  {
    client->client_file->set(key, value);
    send(client->fd, succeed_response_.c_str(), succeed_response_.size(), MSG_DONTWAIT);
  }
}

void KVSVRPool::delete_kv_task(struct clientInfo *client, const string &key)
{
  my_mutex::MutexLockGuard mlg(client->client_mutex);
  if(client->reform == true)
  {
    // reform
  }
  else
  {
    client->client_file->erase(key);
    send(client->fd, succeed_response_.c_str(), succeed_response_.size(), MSG_DONTWAIT);
  }
}


void KVSVRPool::close_pool()
{}


} // namespace kvsvr