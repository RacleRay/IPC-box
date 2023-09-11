#include <sys/sem.h>

#include "semaph.h"


int semaphores_init(int sem_id) {
    unsigned short values[1] = {1};
    union semun args;
    args.array = values;
    // set semval to 0-th val of values array
    return semctl(sem_id, 0, SETVAL, values);
}

int semaphores_wait(int sem_id) {
    struct sembuf sb[1];
    sb[0].sem_num = 0;  // use 0-th semaphore
    sb[0].sem_op = -1;  // decrement
    sb[0].sem_flg = SEM_UNDO;  // undo when unaccessible
    
    // acquire resource
    return semop(sem_id, sb, 1);
}

int semaphores_post(int sem_id) {
    struct sembuf sb[1];
    sb[0].sem_num = 0;  // use 0-th semaphore
    sb[0].sem_op = 1;  // increment
    sb[0].sem_flg = SEM_UNDO;  // undo when unaccessible

    // release resource
    return semop(sem_id, sb, 1);
}