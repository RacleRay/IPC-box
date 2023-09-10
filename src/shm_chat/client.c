#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>    /* Defines O_* constants  */
#include <sys/stat.h> /* Defines mode constants */
#include <sys/mman.h>
#include <sys/types.h>
#include <pthread.h>

#include "message_box.h"
#include "utils.h"

static volatile int runflag = 1;

int client_id;
char *name;

// "ctrl + c"
void handler() {
    runflag = 0;
}

void *recv_message(void *arg) {
    messagebox_p client_box = (messagebox_p)arg;

    message_p msg = malloc(sizeof(message_t));
    if (msg == NULL) {
        perror("recv malloc");
        return NULL;
    };
    memset(msg, 0, sizeof(message_t));

    (void)signal(SIGINT, handler);
    while (runflag) {
        while (messagebox_check_empty(client_box) && runflag) { usleep(100); }
        if (!runflag) { break; }

        messagebox_recv(client_box, msg);
        switch (msg->type) {
            case SERVER:
                // messagebox_ptime();
                printf("[Server] %s\n", msg->message);
                break;
            case WHISPER:
                // messagebox_ptime();
                printf("[Private] %s: %s\n", msg->sender_name, msg->message);
                break;
            case BROADCAST:
                // messagebox_ptime();
                printf("> %s: %s\n", msg->sender_name, msg->message);
                break;
            default:
                printf("Unknnwn message type\n");
                break;
        }
    }

    return NULL;
}

void *send_message(void *arg) {
    // get server box
    messagebox_p server_box = (messagebox_p)arg;

    message_p msg = malloc(sizeof(message_t));
    if (msg == NULL) {
        perror("recv malloc");
        return NULL;
    };
    memset(msg, 0, sizeof(message_t));
    msg->sender_id = client_id;
    strcpy(msg->sender_name, name);

    (void)signal(SIGINT, handler);
    while (runflag) {
        // 1. input message
        if (fgets(msg->message, MESSAGE_LEN, stdin) < 0) {
            perror("message fgets");
            return NULL;
        }
        size_t len = strlen(msg->message);
        msg->message[len - 1] = '\0'; // remove \n

        // 2. check message type
        if (strcmp(msg->message, "/quit") == 0) {
            msg->type = QUIT;

            // wait if the message box is full
            while (messagebox_check_full(server_box)) { usleep(100); }

            messagebox_send(server_box, msg);
            runflag = 0;

        } else if (strcmp(msg->message, "/whisper") == 0) {
            printf("@ whisper to (enter the user name): \n");

            // show the active user list.
            msg->type = LIST;
            while (messagebox_check_full(server_box)) { usleep(100); }
            messagebox_send(server_box, msg);

            // construct whisper message
            msg->type = WHISPER;
            char to[USER_NAME_LEN] = {0};
            if (fgets(to, USER_NAME_LEN, stdin) < 0) {
                perror("whisper to fgets");
                return NULL;
            };
            to[strlen(to) - 1] = '\0';
            memcpy(msg->sender_name, to, USER_NAME_LEN); // reuse the name

            printf("input whisper message: \n");
            if (fgets(msg->message, MESSAGE_LEN, stdin) < 0) {
                perror("whisper message fgets");
                return NULL;
            }
            msg->message[strlen(msg->message) - 1] = '\0'; // remove \n

            while (messagebox_check_full(server_box)) { usleep(100); }
            messagebox_send(server_box, msg);

        } else if (strcmp(msg->message, "/list") == 0) {
            msg->type = LIST;

            while (messagebox_check_full(server_box)) { usleep(100); }
            messagebox_send(server_box, msg);

        } else if (strcmp(msg->message, "/help") == 0) {
            printf("\n>>> Option:\n"
                   "/list    : List the chat room members\n"
                   "/quit    : Leave the chat room\n"
                   "/whisper : Send a private message\n"
                   "/help    : Get help information\n\n");

        } else {
            msg->type = BROADCAST;

            // if message box is full, wait
            while (messagebox_check_full(server_box)) { usleep(100); }
            messagebox_send(server_box, msg);
        }
    }

    return NULL;
}

void client_join_chat(int client_id, const char *name, messagebox_p server_box) {
    // create first message
    message_p msg = malloc(sizeof(message_t));
    memset(msg, 0, sizeof(message_t));

    strcpy(msg->sender_name, name);
    msg->sender_id = client_id;
    msg->type = JOIN;

    // actually dont need to check if message box is full.
    messagebox_send(server_box, msg);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./client \"user_id[0-100]\" \"user_name\" \n");
        return -1;
    }

    messagebox_ptime();
    printf("Client started. \n");

    // (void)signal(SIGINT, handler);

    // process arguments
    client_id = (int)strtol(argv[1], NULL, 10);
    // client_id = atoi(argv[1]);
    name = argv[2];
    messagebox_ptime();
    printf("Client name: %s, client id: %d\n", name, client_id);

    // 1. === get mmap server box ===
    messagebox_p server_box;
    server_box = messagebox_open(SERVER_ID);
    // ===========================
    // === get shm server box  ===
    // key_t server_key = messagebox_get_shmkey(SERVER_ID);
    // printf("server key: %d\n", server_key);
    // messagebox_p server_box = messagebox_open_shm(server_key);
    // ===========================

    // 2. join chat
    client_join_chat(client_id, name, server_box);

    // ===========================
    // // get client id from server is better
    // int client_id = msg->sender_id;
    // printf("Client id: %d\n", client_id);
    // ===========================

    // 3. === create mmap client box ===
    messagebox_p client_box = messagebox_open(client_id);
    client_box->msg_in_pos = 0;
    client_box->msg_out_pos = 0;
    // ===============================
    // === create shm client box ===
    // key_t client_key = messagebox_get_shmkey(client_id);
    // printf("client key: %d\n", client_key);
    // messagebox_p client_box = messagebox_open_shm(client_key);
    // client_box->head = 0;
    // client_box->tail = 0;
    // ===============================

    // 4. create thread dealing with message recv and send
    pthread_t server_thread, client_thread;
    pthread_create(&server_thread, NULL, send_message, (void *)server_box);
    pthread_create(&client_thread, NULL, recv_message, (void *)client_box);

    pthread_join(server_thread, NULL);
    pthread_join(client_thread, NULL);

    printf("\n");
    messagebox_ptime();
    printf("Client stopped.\n");

    return 0;
}