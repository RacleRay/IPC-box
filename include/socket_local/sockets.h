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

#endif