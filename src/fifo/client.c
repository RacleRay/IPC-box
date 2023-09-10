#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"
#include "signals.h"

#define FIFO_PATH "/tmp/ipc_fifo"


void run_client(FILE* stream, int msg_size, int msg_count, struct sigaction *sa);
FILE *open_fifo(const char *path, struct sigaction *sa);


int main(int argc, char **argv) {
    arguments_t args;
    parse_arguments(&args, argc, argv);

    struct sigaction sa;
    // block SIGUSR2, ignore SIGUSR1
    sigset_t oset = setup_client_signals(&sa);

    // open fifo
    FILE *stream = open_fifo(FIFO_PATH, &sa);

    run_client(stream, args.msg_size, args.msg_count, &sa);

    sigprocmask(SIG_SETMASK, &oset, NULL);

    return 0;
}


void run_client(FILE* stream, int msg_size, int msg_count, struct sigaction *sa) {
    void* buf = malloc(msg_size);

    // notify server: which will send to all process in same group
    kill(0, SIGUSR1);

    for (int i = 0; i < msg_count; i++) {
        // wait server signal
        int signo;
        sigwait(&(sa->sa_mask), &signo);

        if (fread(buf, msg_size, 1, stream) != 1) {
            err_sys("client read buf");
        }

        (void) fflush(stream);

        kill(0, SIGUSR1);  // notify client
    }

    // clean up
    free(buf);
    (void)fclose(stream);
}

FILE *open_fifo(const char *path, struct sigaction *sa) {
    int signo;
    sigwait(&(sa->sa_mask), &signo);;

    FILE *stream = fopen(path, "r");
    if (stream == NULL) {
        err_sys("fifo fopen");
    }

    return stream;
}