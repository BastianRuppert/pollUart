#ifndef __UTILS_H__
#define __UTILS_H__


extern int utils_open_uart(const char * tty);


/**
 *  init log ublox modem traffic to file
 */
extern void utils_log_init(const char * path);

/**
 *  deinit log ublox modem traffic to file
 */
extern void utils_log_deinit();

/**
 *  log ublox modem traffic to file
 */
extern void utils_log_c(char c);

/**
 *  log ublox modem traffic to file
 */
extern void utils_log_buf(char * buf,int len);

#endif // Ende Includewächter
