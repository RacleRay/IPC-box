#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "utils.h"
#include "signals.h"

#define TEST_LOOPS 1000


// open pipe
FILE* open_stream(int file_descriptor[2], char mode, int endian) {
    FILE* stream;

    close(file_descriptor[1 - endian]);

    stream = fdopen(file_descriptor[endian], &mode);
    if (stream == NULL) {
        err_sys("Can`t open stream for pipe.");
    }

    return stream;
}


void run_server(int file_descriptor[2], int msg_size, int msg_count, pid_t client_pid) {
    struct sigaction signal_action;
    void* buf;
    FILE* stream;

    stream = open_stream(file_descriptor, 'w', 1);
    buf = malloc(msg_size);

    // block SIGUSR1, ignore SIGUSR2
    sigset_t oset = setup_server_signals(&signal_action);

    int signo;
    sigwait(&signal_action.sa_mask, &signo);

    printf("start pipe server test\n");
    // while (1) {
    for (int i = 0; i < msg_count; i++) {
        if (fwrite(buf, msg_size, 1, stream) == -1) {
            err_sys("Can`t write to pipe.");
        }
        (void)fflush(stream);

        // notify client
        kill(client_pid, SIGUSR2);
        // wait for signal
        sigwait(&signal_action.sa_mask, &signo);
    }

    printf("pipe server test finished\n");

    sigprocmask(SIG_SETMASK, &oset, NULL);
    close(file_descriptor[1]);
    free(buf);
}


void run_client(int file_descriptor[2], int msg_size, int msg_count, pid_t server_pid) {
    struct sigaction signal_action;
    FILE* stream;
    void* buf;

    stream = open_stream(file_descriptor, 'r', 0);
    buf = malloc(msg_size);

    sigset_t oset = setup_client_signals(&signal_action);

    // notify server
    kill(server_pid, SIGUSR1);

    printf("start pipe client test\n");

    int signo;
    // while (1) {
    for (int i = 0; i < msg_count; i++) {
        sigwait(&signal_action.sa_mask, &signo);

        if (fwrite(buf, msg_size, 1, stream) == -1) {
            err_sys("Can`t read from pipe.");
        }

        kill(server_pid, SIGUSR1);
    }
    
    printf("pipe client test finished\n");

    sigprocmask(SIG_SETMASK, &oset, NULL);
    close(file_descriptor[0]);
    free(buf);
}


int main(int argc, char *argv[]) {
    int pipefd[2];

    struct arguments args;
    parse_arguments(&args, argc, argv);

    if (pipe(pipefd) < 0) {
        err_sys("Can`t open pipe.");
    }

    // communitcate
    pid_t pid = fork();
    if (pid < 0) {
        err_sys("Can`t fork process");
    }

    if (pid == 0) { // child
        run_client(pipefd, args.msg_size, args.msg_count, getppid());
    }

    // parent
    run_server(pipefd, args.msg_size, args.msg_count, pid);

    wait(NULL);
    return 0;
}