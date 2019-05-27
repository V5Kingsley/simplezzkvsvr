/**
 * @file work_thread.cpp
 * @author Kingsley
 * @brief 
 * @version 0.1
 * @date 2019-05-08
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include "work_thread.h"

#define LOGGER_WARN
#include <logger.h>

#include <signal.h>
//#include <setjmp.h>

namespace kvsvr
{
namespace work_thread
{

/*static jmp_buf jmpbuffer;

void sig_alarm(int sigs)
{
  longjmp(jmpbuffer, 1);
}*/

void Work::execute_work()
{
  /*if(setjmp(jmpbuffer) != 0)
  {
    printf("This work is overtime. Quit it!!!\n");
    alarm(0);
    return;
  }*/

  if (work_ == nullptr)
  {
    WARN("No work to execute !!! ");
  }
  else
  {
    //signal(SIGALRM, sig_alarm);
    //alarm(1);
    work_();
   // alarm(0);
  }
}

} // namespace work_thread

} // namespace kvsvr