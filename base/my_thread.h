/**
 * @file my_thread.h
 * @author Kingsley
 * @brief Thread Class Which Imitates the std::thread
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef MY_THREAD_H_
#define MY_THREAD_H_

#include <boost/function.hpp>
#include <boost/atomic.hpp>

namespace my_thread
{

template <typename CLASSTYPE>
void* thread_func(void* arg)
{
  CLASSTYPE *thisClass = (CLASSTYPE*)arg;
  thisClass->func_();
  CLASSTYPE::thread_num--;
  return NULL;
}

/**
 * @brief Thread class which imitates the std::thread
 * 
 */
class Thread
{
public:
  typedef std::function<void ()> ThreadFunc;
  template<typename _func, typename... _Args>
  explicit Thread(_func&& _threadFun, _Args&&... _args) 
    : started_(false), 
      joined_(false), 
      detached_(false)
  {
    func_ = std::bind(std::forward<_func>(_threadFun), std::forward<_Args>(_args)...);
    thread_num++;

  }
  void start();

  void join();

  pthread_t thread_id()
  {
    return id_;
  }

  void detach();

  ~Thread(){}

  ThreadFunc func_;

  static boost::atomic_int thread_num;
private:
  pthread_t id_;
  bool started_;
  bool joined_;
  bool detached_;

  Thread(const Thread&);
  Thread& operator=(const Thread&);
};



} // MyThread

#endif