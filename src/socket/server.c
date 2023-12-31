#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sockets.h"
#include "utils.h"

#define HOST "localhost"
#define PORT "8080"

int create_listen_socket();
void run_communicate(int connfd, int size, int count, int nonblock);


int main(int argc, char *argv[]) {
    struct arguments args;
    parse_arguments(&args, argc, argv);

    int lsockfd = create_listen_socket();

    // === accept the connections adapted to IPv6
    struct sockaddr_storage client_addr;  // big enough for IPv6 and IPv4
    socklen_t sin_size = sizeof(client_addr);

    // block until a client connects
    int connfd = accept(lsockfd, (struct sockaddr *)&client_addr, &sin_size);
    if (connfd < 0) {
        err_sys("server accept.");
    }

    // set socket buffer size.
    set_socket_buffer_size(connfd, SOCKET_BUFFER_SIZE, RECV);
    set_socket_buffer_size(connfd, SOCKET_BUFFER_SIZE, SEND);

    // set io flag
    if (NONBLOCK) {
        if (set_io_flag(connfd, O_NONBLOCK) < 0) {
            err_sys("server set_io_flag.");
        }
    }

    // maybe close the lsockfd, because we don't need it anymore for only one connection.
    
    // === communication 
    run_communicate(connfd, args.msg_size, args.msg_count, NONBLOCK);

    // clean up
    close(lsockfd);
    close(connfd);

    return 0;
}


/**
 * @brief The first file descriptor is for the main listening socket.
 *        For every client connection, we will get a new file descriptor for
 *        communication.
 * 
 * @return int 
 */
int create_listen_socket() {
    // === Get address info ===
    // Hints passed to getaddrinfo()
    struct addrinfo hints;
    memset(&(hints), 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;  // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP
    // hints.ai_flags = AI_PASSIVE;  // For wildcard IP address, then HOST can be NULL.

    // full the addrinfo structure.
    struct addrinfo *server_info = NULL;
    int ret_code = getaddrinfo(HOST, PORT, &hints, &server_info);
    if (server_info == NULL) {
        err_sys("Can`t find valid address.");
    }
    if (ret_code != 0) {
        (void)fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(ret_code));
        freeaddrinfo(server_info);
        exit(EXIT_FAILURE);
    }

    // === Get valid address ===
    int sockfd;
    // loop through all the address linked list 
    for (struct addrinfo *p = server_info; p != NULL; p = p->ai_next) {
        // try to establish a socket
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) {
            continue;
        }

        // set socket option
        ret_code = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){SOL_SOCKET}, sizeof(int));
        if (ret_code < 0) {
            close(sockfd);
            err_sys("server setsockopt.");
        }

        // bind the socket file descriptor to the address
        ret_code = bind(sockfd, p->ai_addr, p->ai_addrlen);
        if (ret_code == -1) {
            close(sockfd);
            err_sys("server bind.");            
        }

        break;
    }    
    
    // === Listen ===
    ret_code = listen(sockfd, 10);
    if (ret_code < 0) {
        err_sys("server listen.");
    }

    return sockfd;
}


void run_communicate(int connfd, int size, int count, int nonblock) {
    void *buf = malloc(size);
    memset(buf, 0, size);

    printf("tcp socket server start.\n");

    for (int i = 0; i < count; i++) {
        if (send(connfd, buf, size, 0) < 0) {
            err_sys("server send.");
        }

        if (nonblock) {
            while (recv(connfd, buf, size, 0) < size) {
                if (errno != EAGAIN || errno != EWOULDBLOCK) {
                    err_sys("nonblock server recv.");
                }
            }
        } else if (recv(connfd, buf, size, 0) < 0) {
            err_sys("server recv.");
        }
    }

    printf("tcp socket server end.\n");

    free(buf);
}