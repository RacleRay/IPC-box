#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#include "semaph.h"
#include "utils.h"


void run_client(char *shm_addr, int sem_id, int msg_size, int msg_count);


int main(int argc, char *argv[]) {
    struct arguments args;
    parse_arguments(&args, argc, argv);

    // manunal shm key
    int shm_id = shmget(SHM_KEY, getpagesize(), IPC_CREAT | 0666);
    if (shm_id < 0) {
        err_sys("shmget");
    }

    // manunal sem key
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (sem_id < 0) {
        err_sys("semget");
    }

    if (semaphores_init(sem_id) < 0) {
        err_sys("semaphores_init");
    }

    char *shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (char *)-1) {
        err_sys("shmat");
    }

    // communicate
    run_client(shm_addr, sem_id, args.msg_size, args.msg_count);

    // clean up
    shmdt(shm_addr);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 1, IPC_RMID);

    return 0;
}


void run_client(char *shm_addr, int sem_id, int msg_size, int msg_count) {
    void *buf = malloc(msg_size);

    semaphores_post(sem_id);

    printf("Start semaphores shm client test \n");
    for (int i = 0; i < msg_count; i++) {
        // wait for server
        semaphores_wait(sem_id);

        memcpy(buf, shm_addr, msg_size);

        memset(shm_addr, '@', msg_size);

        // notify server
        semaphores_post(sem_id);
    }

    printf("End semaphores shm client test \n");

    free(buf);
}