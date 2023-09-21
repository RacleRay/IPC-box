#ifndef _IPC_SOCKETS_H_
#define _IPC_SOCKETS_H_

#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#define SOCKET_PATH "/tmp/ipc_socket"

#define SOCKET_BUFFER_SIZE 8192

#define SEND 0
#define RECV 1

#define NONBLOCK 0

void set_socket_buffer_size(int sockfd, int size, int direction);
struct timeval get_socket_timeout(int socket_fd, int direction);
double get_socket_timeout_seconds(int socket_fd, int direction);
void set_socket_timeout(int socket_fd, struct timeval* timeout, int direction);

#endif