#include "sh.h"
#include <signal.h>
#include <stdio.h>

void sig_handler(int signal);

int main(int argc, char **argv, char **envp)
{
  /* put signal set up stuff here */
  if ((int)getpid() != 0)
  {
    signal(SIGINT, SIG_IGN); // ignores CTRL C
    signal(SIGTSTP, SIG_IGN); // ignores CTRL Z
  }
  return sh(argc, argv, envp);
}

// handles CTRL C and CTRL D signals
void sig_handler(int signal)
{
  return;
}
