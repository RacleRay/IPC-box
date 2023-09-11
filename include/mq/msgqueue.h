#ifndef _MSGQUEUE_H_
#define _MSGQUEUE_H_

#define CLIENT_MESSAGE 1
#define SERVER_MESSAGE 2

#define MESSAGE_BUF_LEN 4096  // not used when test

typedef struct message {
    int type;
    char buf[];  // flexible array member
} message_t;

#endif