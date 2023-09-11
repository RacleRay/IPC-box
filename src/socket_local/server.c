#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>

#include "sockets.h"
#include "utils.h"


int main(int argc, char *argv[]) {
    struct arguments args;
    parse_arguments(&args, argc, argv);

    // create socket 
    int sockfd;

    // accept connection

    // run server

    return 0;
}


int create_socket() {
    // UNIX domain socket; SOCK_STREAM means TCP; 0 means protocol picked by system
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        err_sys("open socket on server-side");
    }

    // UNIX domain socket
    struct 
}