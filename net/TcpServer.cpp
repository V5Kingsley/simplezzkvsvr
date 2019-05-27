/**
 * @file TcpServer.cpp
 * @author Kingsley
 * @brief 
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include "TcpServer.h"
#include "Socket.h"
#include "thread_pool.h"
#include <fcntl.h>

namespace kvsvr
{

TcpServer::TcpServer(ThreadPool* thread_pool, int listen_port)
  : thread_pool_(thread_pool),
    socket_(new Socket(listen_port))
{
  socket_->set_reuseaddr();
  socket_->bind();
  socket_->listen();
}

/**
 * @brief Add task to pool
 * 
 * @param new_job 
 */
status TcpServer::add_task_to_pool(std::function<void ()> new_job)
{
  thread_pool_->add_task_to_pool(new_job);
}

/**
 * @brief Set fd to no block.
 * 
 * @param fd 
 */
void TcpServer::setNoBlock(int fd)
{
  int old_option = fcntl(fd, F_GETFL);
  int new_option = old_option | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_option);
}

}