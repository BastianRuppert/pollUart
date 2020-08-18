/*
Bastian Ruppert
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "utils.h"

#define POLLSRC_LEN 2
#define POLLSRC_STDIN 0
#define POLLSRC_UART 1

struct pollfd pfds[POLLSRC_LEN];

char uart_buffer[128];

const char * comport = "/dev/ttyUSB0";

static int need_exit = 0;

static void get_stdin(void);
static void uart_poll(void);

static void catch_sigint(int signum){
  printf("signal handler: %i\n",signum);
  need_exit = 1;
}

char tmpBuf[2048];

int main(int argc, char **argv)
{
  int uartfd = -1;
  signal (SIGINT, catch_sigint);

  uartfd = utils_open_uart(comport);
  if(uartfd<0)
  {
    printf("open uart failed: %s\n", comport);
  }

  //stdin
  pfds[POLLSRC_STDIN].fd = fileno(stdin);
  pfds[POLLSRC_STDIN].events = POLLIN | POLLERR;
  pfds[POLLSRC_STDIN].revents = 0;

  //UART
  pfds[POLLSRC_UART].fd = uartfd;
  pfds[POLLSRC_UART].events = POLLIN | POLLPRI | POLLERR;
  pfds[POLLSRC_UART].revents = 0;

  while (!need_exit)
    {

      switch (poll(pfds, POLLSRC_LEN,40))
      {
         /*
           case 0://timeout
           {
           if(ton_poll_timeout())
           need_exit = 1;
           break;
           }
         */
        case -1:{
          if (errno != EINTR) {
            need_exit = 1;
            break;
          }
          continue;
        }
        default:{
          break;
        }
      }

      if (need_exit) {
        break;
      }

      if(pfds[POLLSRC_STDIN].revents & POLLIN) {//sdtin
        get_stdin();
      }else if(pfds[POLLSRC_UART].revents &  (POLLIN | POLLPRI| POLLERR)) { //uart
        uart_poll();
      }

    }// end while (!need_exit)

    if(pfds[POLLSRC_UART].fd > 0){
      close(pfds[POLLSRC_STDIN].fd);
    }

  return 0;
}

void uart_poll()
{
  int len,i;

  len = read(pfds[POLLSRC_UART].fd,uart_buffer,sizeof(uart_buffer));
  printf("uart1_poll read len = %i i: \n",len);
#if 0
  if(len>=0){
	  for(i=0;i<len;i++){
      	printf("|%c",uart_buffer[i]);
    }
	  //printf("\n");
  }
#endif
}

static void get_stdin(void)
{
  char buf[256];
  int ret;

  if(fgets(buf,sizeof(buf),stdin)!=0){
    if(!strncmp("exit",buf,4))
    {
      need_exit = 1;
    }
    else if(!strncmp("test",buf,strlen("atTest")))
    {
      printf("Testausgabe: Test\n");
    }
    else
    {
        printf("unsupported cmd: %s",buf);
        printf("supported commands are:\n");
        printf("test, exit\n");
    }
  }//end if ()fgets)
}

