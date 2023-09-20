#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>

#include "signals.h"
#include "sockets.h"
#include "utils.h"


int create_connection(const char *socket_path, int nonblock);
void run_client(int connfd, int msg_size, int msg_count, int nonblock);

int main(int argc, char *argv[]) {
    struct arguments args;
    parse_arguments(&args, argc, argv);

    // wati for server
    struct sigaction sa;
    sigset_t oset = setup_client_signals(&sa);
    int signo;
    sigwait(&(sa.sa_mask), &signo);;

    // create connection
    // NOTE: when using UNIX domain sockets, nonblocking mode is not much faster than blocking mode.
    int connfd = create_connection(SOCKET_PATH, NONBLOCK);

    // run client
    run_client(connfd, args.msg_size, args.msg_count, NONBLOCK);

    // recover sigmask
    sigprocmask(SIG_SETMASK, &oset, NULL);

    // clean up
    close(connfd);

    return 0;
}


int create_connection(const char *socket_path, int nonblock) {
    int connfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (connfd < 0) {
        err_sys("create connection on client side");
    }

    set_socket_buffer_size(connfd, SOCKET_BUFFER_SIZE, SEND);
    set_socket_buffer_size(connfd, SOCKET_BUFFER_SIZE, RECV);

    if (nonblock) {
        if (set_io_flag(connfd, nonblock) < 0) {
            err_sys("set nonblock on client side");
        }
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);
    if (connect(connfd, (struct sockaddr *)&addr, SUN_LEN(&addr)) < 0) {
        err_sys("connet to server");
    }

    return connfd;
}


void run_client(int connfd, int msg_size, int msg_count, int nonblock) {
    void *buf = malloc(msg_size);

    printf("Start UNXI domain sockets client test\n");
    for (int i = 0; i < msg_count; i++) {
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

        // construct message
        memset(buf, '@', msg_size);

        // send message
        if (send(connfd, buf, msg_size, 0) < msg_size) {
            err_sys("send on server-side");
        }
    }
    printf("End UNXI domain sockets client test\n");

    free(buf);
}