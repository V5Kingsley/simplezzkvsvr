#include "TcpEpollServer.h"
#include <parameters.h>
#include <algorithm>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/stat.h>  

//#define LOGGER_DEBUG
#define LOGGER_WARN
#include <logger.h>

#include "Socket.h"

namespace kvsvr
{

TcpEpollServer::TcpEpollServer(ThreadPool *pool, parameters::Parameters *parameters)
  : TcpServer(pool, parameters->getListenPort()),
    http_parameters_(parameters)
{
  
}

int TcpEpollServer::efd_ = eventfd(0, 0); // event fd. used in sigint to quit the handle_request loop.



TcpEpollServer::~TcpEpollServer()
{
  
}


/**
 * @brief Sigint signal call back function. Event fd is used to notice the main loop to quit
 * 
 * @param sig 
 */
void TcpEpollServer::sig_int_handle(int sig)
{
  uint64_t u = 1;
  ssize_t rc;
  rc = write(efd_, &u, sizeof(uint64_t));
  if(rc != sizeof(uint64_t))
    WARN("sig int eventfd write error");
}

/**
 * @brief add event to epoll fd
 * 
 * @param fd 
 * @param event_type 
 */
void TcpEpollServer::add_event(int fd, int event_type)
{
  epoll_event e;
  e.data.fd = fd;
  e.events = event_type;
  epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &e);
}

/**
 * @brief delete event from epoll fd
 * 
 * @param fd 
 * @param event_type 
 */
void TcpEpollServer::del_event(int fd, int event_type)
{
  epoll_event e;
  e.data.fd = fd;
  e.events = event_type;
  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &e);
}

/**
 * @brief Close fd and delete its timer from queue
 * 
 * @param fd 
 */
void TcpEpollServer::close_client(int fd)
{
  client_timers_queue_.del_timer(client_fd_array_[fd]);
  delete client_fd_array_[fd];
  client_fd_array_[fd] = nullptr;
  close(fd);
}

/**
 * @brief Read data from client fd. Get the request and give response to client
 * 
 * @param client_fd 
 */
void TcpEpollServer::client_service(int client_fd)
{
  DEBUG("handling client request... client fd: %d\n", client_fd);


}

/**
 * @brief Handle request loop with epoll method
 * 
 */
void TcpEpollServer::handle_request()
{
  signal(SIGPIPE, SIG_IGN);  // ignore sigpipe
  signal(SIGINT, sig_int_handle); 

  if(socket_->fd() == -1)
  {
    WARN("Create listen fd failed!\n");
    return;
  }

  assert(efd_ != -1);

  epoll_fd_ = epoll_create(5);
  assert(epoll_fd_ != -1);

  int time_fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);  //create timer fd to check the timer queue at intervals
  assert(time_fd != -1);
  struct itimerspec new_value;
  struct timespec now;
  uint64_t exp;
  ssize_t s;
  assert(clock_gettime(CLOCK_REALTIME, &now) != -1);
  new_value.it_value.tv_sec = 2;
  new_value.it_value.tv_nsec = now.tv_nsec;
  new_value.it_interval.tv_sec = 2;
  new_value.it_interval.tv_nsec = 0;
  assert(timerfd_settime(time_fd, 0, &new_value, NULL) != -1);  // interval 2s
  bool timer_tick = false;

  add_event(socket_->fd(), EPOLLIN);
  add_event(efd_, EPOLLIN);
  add_event(time_fd, EPOLLIN);

  int overtime_ms = -1;
  bool run = true;
  epoll_event events[MAXEVENTS];
  while(run == true)
  {
    int ret = epoll_wait(epoll_fd_, events, MAXEVENTS, overtime_ms);
    if(ret < 0 && errno != EINTR)
    {
      WARN("epoll wait failed!\n");
      break;
    }

    for(int i = 0; i < ret; ++i)
    {
      if(events[i].data.fd == socket_->fd())   // a new client
      {
        int client_fd = socket_->accept();
        if(client_fd == -1)
        {
          continue;
        }
        setNoBlock(client_fd);
        
        add_event(client_fd, EPOLLIN); 
        
        timer_tick::Timer *new_timer = new timer_tick::Timer(
          client_fd, std::bind(&TcpEpollServer::client_overtime_cb, this, std::placeholders::_1), time(NULL) + CLIENT_LIFE_TIME);    // create the timer of the client fd
        assert(client_fd < MAX_FD);
        client_fd_array_[client_fd] = new_timer;   // record the fd and its timer
        client_timers_queue_.add_timer(new_timer);  // add to queue

        DEBUG("accept a new client[%d]\n", client_fd);
      }
      else if(events[i].data.fd == time_fd)  // received the timer fd tick. set timer_tick to true to check the timer queue
      {
        s = read(time_fd, &exp, sizeof(uint64_t));
        assert(s == sizeof(uint64_t));
        timer_tick = true;
        DEBUG("timer tick!!!\n");
      }
      else if((events[i].data.fd == efd_ ) && (events[i].events & EPOLLIN))  // got the event fd signal to quit the loop.
      {
        INFO("Got a sigint signal. Exiting...\n");
        run = false;
        break;
      }
      else if(events[i].events & EPOLLIN) // a client send request
      {
        DEBUG("receive a request from client[%d]\n", events[i].data.fd);
        status r = 
          add_task_to_pool(std::bind(
            &TcpEpollServer::client_service, this, static_cast<int>(events[i].data.fd)));  // add the task to pool
        /*if(r == FAILED)
        {
          continue;  // How to handle overflowed task?
        }*/
        del_event(events[i].data.fd, EPOLLIN);
      }
      else if(events[i].events & EPOLLRDHUP) // a client close the fd. Never got the signal??
      {
        DEBUG("EPOLLRDHUP!\n");
        close_client(events[i].data.fd);
        del_event(events[i].data.fd, EPOLLIN);
      }
      else
      {
        WARN("unexpected things happened! \n");
      }
    }

    if(timer_tick)   // timer ticking. Remove redundant client links.
    {
      time_t current_time = time(NULL);
      while(!client_timers_queue_.empty())
      {
        timer_tick::Timer* top_timer = client_timers_queue_.top();
        if(top_timer->overtime() < current_time)
        {
          top_timer->overtime_callback(top_timer);   // If overtime, call the callback function to handle.
        }
        else
        {
          break;
        }   
      }
      DEBUG("client queue size: %d\n", client_timers_queue_.size());
    }
  }
  socket_->close();
}

/**
 * @brief Client overtime callback function. 
 * 
 * @param overtime_timer 
 */
void TcpEpollServer::client_overtime_cb(timer_tick::Timer* overtime_timer)
{
  close_client(overtime_timer->fd());
}

} // namespace kvsvr

