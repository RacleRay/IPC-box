#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include "utils.h"
#include "msgqueue.h"
#include "signals.h"


int create_message_queue(const char *path);
void run_server(int mq, int msg_size, int msg_count);


int main(int argc, char *argv[]) {
    arguments_t args;
    parse_arguments(&args, argc, argv);

    // create message queue
    int mq = create_message_queue("/tmp/msgqueue");

    // notify the client message queue is created
    struct sigaction sa;
    sigset_t oset = setup_server_signals(&sa);
    kill(0, SIGUSR2);  // signal for all processes in same group

    // run server
    run_server(mq, args.msg_size, args.msg_count);

    // recover signalmask
    sigprocmask(SIG_SETMASK, &oset, NULL);

    return 0;
}


int create_message_queue(const char *path) {
    // make sure path is valid
    int tmp_descriptor = open(path, O_RDONLY | O_CREAT, 0666);
    if (tmp_descriptor < 0) { err_sys("msgqueue file open"); }
    close(tmp_descriptor);

    // create message queue
    key_t key_t = ftok(path, 'R');

    int mq = msgget(key_t, IPC_CREAT | 0666);
    if (mq < 0) { err_sys("msgget"); }

    return mq;
}


void run_server(int mq, int msg_size, int msg_count) {
    // msg memory allocate
    message_t* msg = malloc(sizeof(message_t) + msg_size * sizeof(msg->buf[0]));

    for (int i = 0; i < msg_count ; i++) {
        // construct message
        msg->type = SERVER_MESSAGE;
        memset(msg->buf, '*', msg_size);

        // IPC_NOWAIT: Return error on wait
        if (msgsnd(mq, msg, msg_size, IPC_NOWAIT) < 0) {
            err_sys("msgsnd");
        }

        if (msgrcv(mq, msg, msg_size, CLIENT_MESSAGE, 0) < 0) {
            err_sys("msgrcv");
        }
    }

    // cleanup
    if (msgctl(mq, IPC_RMID, NULL) < 0) {
        err_sys("msgctl");
    }
    free(msg);
}