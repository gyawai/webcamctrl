#ifndef __UTILS_H__
#define __UTILS_H__

#include <netinet/in.h>

/*
 * wrappers for system-dependent functions
 */
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
extern int closesocket(SOCKET fd);

/*
 * utilities for socket connection
 */
extern int setup_server(in_port_t port);
extern int setup_client(char *hostname, in_port_t port);
extern int mserver_socket(in_port_t port, int num);
extern int mserver_maccept(SOCKET soc, int limit, void (*func)());

/*
 * miscellaneous utilities
 */
extern char *chop_newline(char *buf, int len);
extern void print_buf(char *buf, int nbyte);

#endif // __UTILS_H__
