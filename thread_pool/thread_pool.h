/**
 * @file thread_pool.h
 * @author Kingsley
 * @brief Thread Pool
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "work_thread.h"
#include <semaphore.h>
#include "parameters.h"

namespace kvsvr
{

class ThreadPool : boost::noncopyable
{
typedef std::function<void ()> TaskFunc;

public:
  ThreadPool(parameters::Parameters *pool_parameters);

  void start();

  void thread_routine(int index);

  void distribute_task();

  work_thread::WorkThread* get_next_work_thread();

  ~ThreadPool()
  { 
    for(int i = 0; i < work_threads_.size(); ++i)
    {
      delete work_threads_[i];
    }
  }

  status add_task_to_pool(TaskFunc new_task);

  void close_pool();

private:
  std::vector<work_thread::WorkThread*> work_threads_;
  std::shared_ptr<my_thread::Thread> distribute_thread_;
  int next_;
  int threads_num_;
  bool started;
  my_mutex::MutexLock boot_mutex_;
  my_condition::Condition boot_cond_;

  pthread_barrier_t pool_barrier_;
  bool pool_activate;

  my_mutex::MutexLock pool_mutex_;

  sem_t task_num_;

  WorkQueue<work_thread::Work::WorkPtr> pool_work_queue_;

  parameters::Parameters *pool_parameters_;

  int max_work_num_;

};



} // namespace kvsvr




#endif  // THREAD_POOL_H_