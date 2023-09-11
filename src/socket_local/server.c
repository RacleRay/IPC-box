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

    int sockfd;
    
    

    return 0;
}