#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#define SHM_KEY 0x666
#define SEM_KEY 0x777

union semun {
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO
                              (Linux-specific) */
};

int semaphores_init(int sem_id);

int semaphores_wait(int sem_id);
int semaphores_post(int sem_id);

#endif