/**
 * @file my_thread.cpp
 * @author Kingsley
 * @brief 
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include "my_thread.h"

namespace my_thread
{

boost::atomic_int Thread::thread_num(0); // Record the number of threads created.

/**
 * @brief Must be called after the thread class is built. Otherwise the new thread will not be created.
 * 
 */
void Thread::start()
{
  if (!started_)
  {
    pthread_create(&id_, NULL, thread_func<Thread>, this);
    started_ = true;
  }
  else
    return;
}

void Thread::join()
{
  if (started_ && !joined_)
    pthread_join(id_, NULL);
  return;
}

void Thread::detach()
{
  if (started_ && !detached_ && !joined_)
    pthread_detach(id_);
  else
    return;
}

} // namespace my_thread
