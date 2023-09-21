#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "utils.h"

#define SERVER_TOKEN 1
#define CLIENT_TOKEN 2


// msgsize is always 1 at this program.
void run_server(int fd, int msgcount) {
    printf("eventfd server start\n");
    uint64_t event_val = 0;

    for (int i = 0; i < msgcount; i++) {
        
        // wait for client event
        while (true) {
            // read will return the value of the eventfd, unless the eventfd flag
            // is EFD_SEMAPHORE, in which case it will return 1 and decrement it 
            // by 1.
            if (read(fd, &event_val, 8) < 0) {
                err_sys("eventfd server read");
            }
            if (event_val == SERVER_TOKEN) {
                break;
            }

            // write it back
            if (write(fd, &event_val, 8) < 0) {
                err_sys("eventfd server write");
            }
        }

        // notify client
        uint64_t client_token = CLIENT_TOKEN;
        if (write(fd, &(client_token), 8) < 0) {
            err_sys("eventfd server write");
        }
    }
}


void run_client(int fd, int msgcount) {
    uint64_t event_val = 0;
    uint64_t server_token = SERVER_TOKEN;

    for (int i = 0; i < msgcount; i++) {
        // notify server
        if (write(fd, &(server_token), 8) < 0) {
            err_sys("eventfd client write");
        }

        // wait for server
        while (true) {
            // read will return the value of the eventfd, unless the eventfd flag
            // is EFD_SEMAPHORE, in which case it will return 1 and decrement it 
            // by 1.
            if (read(fd, &event_val, 8) < 0) {
                err_sys("eventfd server read");
            }
            if (event_val == CLIENT_TOKEN) {
                break;
            }

            // write it back
            if (write(fd, &event_val, 8) < 0) {
                err_sys("eventfd server write");
            }
        }
    }
}


int main(int argc, char **argv) {
    struct arguments args;
    parse_arguments(&args, argc, argv);

    /*
        An eventfd object is simply a file in the file-system, that can
        be used as a wait/notify-mechanism similar to a semaphore.
        Stored in the eventfd itself is a simple 64-bit/8-Byte integer.
    */
    // flags you must check.
    int efd = eventfd(0, 0);

    // inter process communication
    pid_t pid = fork();
    if (pid < 0) {
        err_sys("eventfd fork");
    }

    if (pid == (pid_t)0) {
        // child process
        run_client(efd, args.msg_count);
    } else {
        // parent process
        run_server(efd, args.msg_count);
    }

    // clean up
    close(efd);

    return 0;
}