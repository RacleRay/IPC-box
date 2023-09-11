#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>

#include "signals.h"
#include "sockets.h"
#include "utils.h"


int create_socket(const char *socket_path);
int accept_connection(int sockfd, int nonblock);
void run_server(int connfd, int msg_size, int msg_count, int nonblock);


int main(int argc, char *argv[]) {
    struct arguments args;
    parse_arguments(&args, argc, argv);

    // create socket 
    int sockfd = create_socket(SOCKET_PATH);

    // signal once to all in the same group
    struct sigaction sa;
    sigset_t oset = setup_server_signals(&sa);
    kill(0, SIGUSR2);

    // accept connection
    int connfd = accept_connection(sockfd, NONBLOCK);
    close(sockfd); // don`t need it anymore

    // run server
    run_server(connfd, args.msg_size, args.msg_count, NONBLOCK);

    // recover sigmask
    sigprocmask(SIG_SETMASK, &oset, NULL);

    // clean up
    close(connfd);
    (void)remove(SOCKET_PATH);

    return 0;
}


int create_socket(const char *socket_path) {
    // UNIX domain socket; SOCK_STREAM means TCP; 0 means protocol picked by system
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        err_sys("open socket on server-side");
    }

    // UNIX domain socket
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);
    // in case socket file already exists
    (void)unlink(addr.sun_path);

    // bind socket to address
    if (bind(sockfd, (struct sockaddr *)&addr, SUN_LEN(&addr)) < 0) {
        err_sys("bind socket on server-side");
    }

    // listen for connections
    if (listen(sockfd, 10) < 0) {
        err_sys("listen on server-side");
    }

    return sockfd;
}


int accept_connection(int sockfd, int nonblock) {
    struct sockaddr_un client_addr;

    socklen_t addr_len = sizeof(client_addr);
    int connfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
    if (connfd < 0) {
        err_sys("accept on server-side");
    }

    set_unsocket_buffer_size(connfd, SOCKET_BUFFER_SIZE, SEND);
    set_unsocket_buffer_size(connfd, SOCKET_BUFFER_SIZE, RECV);

    if (nonblock) {
        if (set_io_flag(connfd, nonblock) == -1) {
            err_sys("set nonblock on server-side");
        }
    }

    return connfd;
}


void run_server(int connfd, int msg_size, int msg_count, int nonblock) {
    void* buf = malloc(msg_size);

    printf("Start UNIX domain socket server test\n");
    for (int i = 0; i < msg_count; i++) {
        // construct message for test
        memset(buf, '*', msg_size);

        // send message
        if (send(connfd, buf, msg_size, 0) < msg_size) {
            err_sys("send on server-side");
        }

        // receive message
        if (nonblock) {
            while (recv(connfd, buf, msg_size, 0) < msg_size) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    err_sys("recv on server-side");
                }
            }
        } else {
            if (recv(connfd, buf, msg_size, 0) < msg_size) {
                err_sys("recv on server-side");
            }
        }
    }
    printf("End UNIX domain socket server test\n");

    free(buf);
}