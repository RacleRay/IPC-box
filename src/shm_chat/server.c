#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <pthread.h>

#include "message_box.h"

static volatile int runflag = 1;
// static volatile int client_count = 0;

chatsession_t room[MAX_CLIENT + 1];

// "ctrl + c"
void handler() {
    runflag = 0;
}

// check flag
int client_boardcast(message_p msg, int sender) {
    for (int i = 1; i <= MAX_CLIENT; i++) {
        // if (room[i].client_box != NULL && i != sender) { 
        if (room[i].client_box != NULL) { 
            messagebox_send(room[i].client_box, msg);
        }
    }
    return OK;
}

int main(int argc, char *argv[]) {
    // init chat room
    strcpy(room[0].session_owner, "SERVER");
    for (int i = 1; i <= MAX_CLIENT; i++) { room[i].client_box = NULL; }

    // === create server mmap box ===
    messagebox_p server_box = messagebox_open(SERVER_ID);
    server_box->msg_in_pos = 0;
    server_box->msg_out_pos = 0;
    // ==============================

    // === create server shm box ===
    // key_t server_key = messagebox_get_shmkey(SERVER_ID);
    // printf("server key: %d\n", server_key);
    // messagebox_p server_box = messagebox_open_shm(server_key);
    // server_box->head = 0;
    // server_box->tail = 0;
    // =============================
    messagebox_ptime();
    printf("server started, Ctrl + C to stop\n");

    // For temp use
    message_p msg = (message_p)malloc(sizeof(message_t));
    memset(msg, 0, sizeof(message_t));

    (void)signal(SIGINT, handler);
    while (runflag) {
#ifdef DEBUG
        printf("waiting for client\n");
#endif
        // 1. Ring buffer check server message box
        while (messagebox_check_empty(server_box) && runflag) { 
            usleep(100); 
        }
        if (!runflag) { break; }

        // 2. receive message from client
        messagebox_recv(server_box, msg);

        // ==============================
        // // TODO:
        // // dealing with client id
        // if (msg->sender_id == -1) {
        //     ++client_count;
        //     if (client_count >= MAX_CLIENT && msg->type == JOIN) {
        //         // // 0 is the server id
        //         // client_count = (client_count % MAX_CLIENT) + 1;
        //         printf("skipping message, too many clients\n");
        //         continue; // next message
        //     }
        //     msg->sender_id = client_count;
        // }
        // ==============================

        // 3. get mmap client box
        messagebox_p client_box = messagebox_open(msg->sender_id);
        // ===== get shm client box =====
        // key_t client_key = messagebox_get_shmkey(msg->sender_id);
        // messagebox_p client_box = messagebox_open_shm(client_key);
        // printf("client key: %d\n", client_key);
        // ==============================

        // 4. deal with message received from server box
        switch (msg->type) {
            case JOIN: {
                messagebox_ptime();
                printf("client id %d, client name %s, joined\n", msg->sender_id, msg->sender_name);

                // config the current session with new user
                room[msg->sender_id].client_box = client_box;
                memcpy(room[msg->sender_id].session_owner, msg->sender_name, USER_NAME_LEN);

                // message to all client
                msg->type = SERVER;
                if (snprintf(msg->message, MESSAGE_LEN, "%s has joined the room", msg->sender_name) < 0) {
                    perror("message snprintf error.");
                    goto out;
                }

                client_boardcast(msg, msg->sender_id);
                break;
            }

            case BROADCAST: {
                messagebox_ptime();
                printf("client %d broadcast a message\n", msg->sender_id);

                client_boardcast(msg, msg->sender_id);
                break;
            }

            case QUIT: {
                messagebox_ptime();
                printf("client %d left\n", msg->sender_id);

                // mmap unlink
                messagebox_unlink(msg->sender_id);
                // shm cleanup
                // messagebox_close_shm(msg->sender_id, room[msg->sender_id].client_box);

                room[msg->sender_id].client_box = NULL;

                // message to all client
                msg->type = SERVER;
                if (snprintf(msg->message, MESSAGE_LEN, "%s has left the room", msg->sender_name) < 0) {
                    perror("message snprintf error.");
                    goto out;
                }
                // sprintf(msg->message, "%s has left the room", msg->sender_name);

                client_boardcast(msg, msg->sender_id);
                break;
            }

            case LIST: {
                messagebox_ptime();
                printf("client %d requested user list\n", msg->sender_id);

                msg->type = SERVER;
                for (int i = 1; i <= MAX_CLIENT; i++) {
                    if (room[i].client_box != NULL && i != msg->sender_id) {
                        // memset(msg->message, 0, MESSAGE_LEN);
                        if (sprintf(msg->message, "%s is inside the room", room[i].session_owner) < 0) {
                            perror("message snprintf error.");
                            goto out;
                        }

                        while (messagebox_check_full(client_box)) { usleep(100); }

                        messagebox_send(client_box, msg);
                    }
                }
                break;
            }

            case WHISPER: {
                messagebox_ptime();
                printf("client %d sent a whisper\n", msg->sender_id);

                // find the receiver
                messagebox_p receiver_box;
                for (int i = 1; i <= MAX_CLIENT; i++) {
                    if (strcmp(room[i].session_owner, msg->sender_name) == 0) {
                        receiver_box = room[i].client_box;
                        break;
                    }
                }

                // use the session owner to find the receiver
                memcpy(msg->sender_name, room[msg->sender_id].session_owner, USER_NAME_LEN);

                while (messagebox_check_full(client_box)) { usleep(100); }

                messagebox_send(receiver_box, msg);
                break;
            }
        }
    }

out:
    messagebox_close_shm(SERVER_ID, server_box);
    messagebox_ptime();
    printf("server stopped\n");

    return 0;
}
