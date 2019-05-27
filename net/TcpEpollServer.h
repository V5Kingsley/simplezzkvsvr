/**
 * @file TcpEpollServer.h
 * @author Kingsley
 * @brief Use epoll to handle client request.
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef TCPEPOLLSERVER_H_
#define TCPEPOLLSERVER_H_
#include "TcpServer.h"
#include <parameters.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <map>
#include <string>
#include <queue>
#include "timer_tick.h"
#include "timer_queue.h"

namespace kvsvr
{

class ThreadPool;

class TcpEpollServer : public TcpServer
{
public:
  TcpEpollServer(ThreadPool* pool, parameters::Parameters* parameters);

  virtual void handle_request() override;

  virtual void add_event(int fd, int event_type) override;

  virtual void del_event(int fd, int event_type) override;

  static void sig_int_handle(int sig);

  virtual void client_service(int client_fd) override;

  virtual ~TcpEpollServer();

  void close_client(int fd);

  void client_overtime_cb(timer_tick::Timer* overtime_timer);

  static const int MAXEVENTS = 255;
  static const int CLIENT_LIFE_TIME = 5;
  static const int MAX_FD = 10000;

private:
  int epoll_fd_;
  
  parameters::Parameters *http_parameters_;

  static int efd_; // event_fd

  timer_tick::TimerQueue client_timers_queue_;  // client timer queue
  timer_tick::Timer* client_fd_array_[MAX_FD];  // client fd and its timer

};



} // namespace kvsvr


#endif // TCPEPOLLSERVER_H_