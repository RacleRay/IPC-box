#ifndef _MESSAGE_BOX_
#define _MESSAGE_BOX_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/types.h>

#define MESSAGE_BUF_SIZE 128
#define MESSAGE_TIME_LEN 64

#define MESSAGE_LEN      4096
#define SESSION_NAME_LEN 32
#define SHM_ID_LEN       16
#define USER_NAME_LEN    32
#define SHM_ID_LEN       16

#define OK 0

// message type
#define SERVER    0
#define JOIN      1
#define BROADCAST 2
#define QUIT      3
#define LIST      4
#define WHISPER   5

#define MAX_CLIENT 100

#define SERVER_ID 0

#define IPCMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

typedef struct Message {
    int type;
    int sender_id;
    char sender_name[USER_NAME_LEN];
    char message[MESSAGE_LEN];
} message_t, *message_p;

/**
 * message box
 * Working schdule :
 * 1. message send to the message box will add at index msg_in_pos of the message buf
 * array.
 * 2. message receive from the message box will remove at index msg_out_pos of the
 * message buf array.
 * 3. message box is a ring buffer.
 * 4. message box is a shared memory.
 * 5. when (msg_in_pos + 1 == msg_out_pos) the message box is full.
 * 6. when (msg_in_pos == msg_out_pos) the message box is empty.
 */
typedef struct Messagebox {
    message_t messages[MESSAGE_BUF_SIZE];
    int msg_in_pos;
    int msg_out_pos;
} messagebox_t, *messagebox_p;

typedef struct chatsession {
    char session_owner[SESSION_NAME_LEN];
    messagebox_p client_box;
} chatsession_t;

messagebox_p messagebox_open(int id);
int messagebox_close(messagebox_p mbox);

key_t messagebox_get_shmkey(int id);
messagebox_p messagebox_open_shm(key_t key);
void messagebox_close_shm(int id, messagebox_p box);

int messagebox_send(messagebox_p box, message_t *msg);
int messagebox_recv(messagebox_p box, message_t *msg);

int messagebox_unlink(int id);

int messagebox_check_full(messagebox_p box);
int messagebox_check_empty(messagebox_p box);

void messagebox_ptime(void);

#endif