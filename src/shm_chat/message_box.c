
#include "message_box.h"
#include "utils.h"


messagebox_p messagebox_open(int id) {
    char box_id[SHM_ID_LEN];
    if (sprintf(box_id, "__msgbox_%d", id) < 0) {
        err_sys("box id snprintf.");  // TODO: exit sys ?
    }

    // create shared memory
    int fd = shm_open(box_id, O_CREAT | O_RDWR, IPCMODE);
    if (fd < 0) {
        printf("%s shm_open failed.\n", box_id);
        return NULL;
    }

    __off_t size = sizeof(messagebox_t) + (MESSAGE_BUF_SIZE * sizeof(message_t));
    if (ftruncate(fd, size) < 0) {
        printf("%s ftruncate failed.\n", box_id);
        return NULL;
    }

    // mmap
    void* addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        printf("%s mmap failed.\n", box_id);
        return NULL;
    }

    return addr;
}


int messagebox_close(messagebox_p box) {
    if (munmap(box, sizeof(messagebox_t) + (MESSAGE_BUF_SIZE * sizeof(message_t))) < 0) {
        printf("munmap failed.\n");
        return -1;
    }
    free(box);
    return OK;
}


// =============================================================================
// === For shm message box ===
key_t messagebox_get_shmkey(int id) {
    // shname must be a valid pathname
    char shname[SHM_ID_LEN];
    if (snprintf(shname, SHM_ID_LEN, "/shm_%d", id) < 0) {
        printf("shname snprintf failed.\n");
        return -1;
    }
    printf("%s\n", shname);

    key_t key = ftok(shname, 'X');
    return key;
}


messagebox_p messagebox_open_shm(key_t key) {
    int shm_id = shmget(key, sizeof(messagebox_t) + (MESSAGE_BUF_SIZE * sizeof(message_t)), 0660 | IPC_CREAT);
    if (shm_id < 0) {
        err_sys("Can`t allocate shared memory.");
    }
    
    void* addr = shmat(shm_id, NULL, 0);
    if (addr == MAP_FAILED) {
        err_sys("Can`t attach shared memory.");
    }
    return addr;
}


void messagebox_close_shm(int id, messagebox_p box) {
    key_t shm_id = messagebox_get_shmkey(id);
    shmdt(box);
    shmctl(shm_id, IPC_RMID, NULL);
}
// =============================================================================


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
int messagebox_send(messagebox_p box, message_t* msg) {
    if ((box->msg_in_pos + 1) % MESSAGE_BUF_SIZE == box->msg_out_pos) {
        return -1;  // fulled
    }

    memcpy(&box->messages[box->msg_in_pos], msg, sizeof(message_t));
    // box->messages[box->head].sender_id = msg->sender_id;
    // box->messages[box->head].type = msg->type;
    // memcpy(box->messages[box->head].sender_name, msg->sender_name, USER_NAME_LEN);
    // memcpy(box->messages[box->head].message, msg->message, sizeof(msg->message));

    // update head
    box->msg_in_pos = (box->msg_in_pos + 1) % MESSAGE_BUF_SIZE;
    return OK;
}


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
int messagebox_recv(messagebox_p box, message_t* msg) {
    if (box->msg_out_pos == box->msg_in_pos) {
        return -1;  // empty
    }

    memcpy(msg, &box->messages[box->msg_out_pos], sizeof(message_t));
    // msg->type = box->messages[box->tail].type;
    // msg->sender_id = box->messages[box->tail].sender_id;
    // memcpy(msg->sender_name, box->messages[box->tail].sender_name, USER_NAME_LEN);
    // memcpy(msg->message, box->messages[box->tail].message, sizeof(msg->message));

    // update tail: tail + 1
    box->msg_out_pos = (box->msg_out_pos + 1) % MESSAGE_BUF_SIZE;

    return OK;
}


int messagebox_unlink(int id) {
    char box_id[SHM_ID_LEN];
    if (sprintf(box_id, "__msgbox_%d", id) < 0) {
        err_sys("box id snprintf.");
    }

    if (shm_unlink(box_id) < 0) {
        return -1;
    }

    return OK;
}


// message box is a ring buffer. when (msg_in_pos == msg_out_pos) the message box is empty.
int messagebox_check_empty(messagebox_p box) {
	return box->msg_in_pos == box->msg_out_pos ? 1 : 0;
}

// message box is a ring buffer. when (msg_in_pos + 1 == msg_out_pos) the message box is full.
int messagebox_check_full(messagebox_p box) {
	return ((box->msg_in_pos + 1) % MESSAGE_BUF_SIZE) == box->msg_out_pos ? 1 : 0;
}


static void get_time(char* time_str, size_t len) {
    time_t t;
    struct tm* tmp;
    
    (void)time(&t);
    tmp = localtime(&t);

    if (strftime(time_str, len, "%Y-%m-%d %H:%M:%S", tmp) == 0) {
        printf("strftime buffer too small.\n");
        return;
    }
}


void messagebox_ptime(void) {
    char time_str[MESSAGE_TIME_LEN] = { 0 };

    get_time(time_str, MESSAGE_TIME_LEN);

    printf("[ %s ] ", time_str);
}