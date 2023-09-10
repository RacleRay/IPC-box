#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"
#include "signals.h"

#define FIFO_PATH "/tmp/ipc_fifo"

void run_server(FILE* stream, int msg_size, int msg_count, struct sigaction *sa);
FILE *open_fifo(const char *path);


int main(int argc, char **argv) {
    arguments_t args;
    parse_arguments(&args, argc, argv);

    // open fifo
    FILE *stream = open_fifo(FIFO_PATH);

    struct sigaction sa;
    // block SIGUSR1, ignore SIGUSR2
    sigset_t oset = setup_server_signals(&sa);

    run_server(stream, args.msg_size, args.msg_count, &sa);

    unlink(FIFO_PATH);
    sigprocmask(SIG_SETMASK, &oset, NULL);

    return 0;
}


void run_server(FILE* stream, int msg_size, int msg_count, struct sigaction *sa) {
    void* buf = malloc(msg_size);

    // wait client signal
    int signo;
    sigwait(&(sa->sa_mask), &signo);

    for (int i = 0; i < msg_count; i++) {
        if (fwrite(buf, msg_size, 1, stream) != 1) {
            err_sys("server write buf");
        }

        (void) fflush(stream);

        kill(0, SIGUSR2);  // notify client
        sigwait(&(sa->sa_mask), &signo);  // wait client signal
    }

    // clean up
    free(buf);
    (void)fclose(stream);
}

FILE *open_fifo(const char *path) {

    if (mkfifo(path, 0666) > 0) {
        err_sys("mkfifo");
    }

    // notify_client: which will send to all process in same group
    kill(0, SIGUSR2);

    FILE *stream = fopen(path, "w");
    if (stream == NULL) {
        err_sys("fifo fopen");
    }

    return stream;
}