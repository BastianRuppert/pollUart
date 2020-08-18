/*
 infoFile.c Debug Informationen in ein File schreiben
*/

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "utils.h"

static void set_mincount(int fd, int mcount)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }

    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = 5;        /* half second timer */

    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        printf("Error tcsetattr: %s\n", strerror(errno));
}

static int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    //tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */
    tty.c_cflag |= CRTSCTS;

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int utils_open_uart(const char * portname)
{
  int fd;

  fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) {
      printf("Error opening %s: %s\n", portname, strerror(errno));
      return -1;
  }
  /*baudrate 115200, 8 bits, no parity, 1 stop bit */
  //set_interface_attribs(fd, B115200);
  set_interface_attribs(fd, B9600);
  //set_mincount(fd, 0);                /* set to pure timed read */
  return fd;
}

static FILE * utils_log_FILE = 0;

void utils_log_init(const char * path)
{
    if(0==path){
        return;
    }

    if(0!=utils_log_FILE){
        fclose(utils_log_FILE);
    }

    utils_log_FILE = fopen(path,"w");

    if(0==utils_log_FILE){
        perror("utils_log_init cannot open path \n");
    }
}

void utils_log_deinit()
{
    int ret=0;
    if(0!=utils_log_FILE){
        ret = fflush(utils_log_FILE);
        if(0!=ret){
            perror("utils_log_deinit fflush error \n");
        }
        ret = fclose(utils_log_FILE);
        if(0!=ret){
            perror("utils_log_deinit fclose error \n");
        }
        utils_log_FILE = 0;
    }
}

void utils_log_c(char c)
{
    if(0!=utils_log_FILE){
        int ret = fwrite(&c,1,1,utils_log_FILE);
        if(ret!=1){
            perror("utils_log_c fwrite errro\n");
        }
    }
}

void utils_log_buf(char * buf,int len)
{
    if(0==buf||0==len){
        return;
    }

    if(0!=utils_log_FILE){
        int ret = fwrite(buf,len,1,utils_log_FILE);
        if(ret!=1){
            perror("utils_log_buf fwrite errro\n");
        }
    }
}