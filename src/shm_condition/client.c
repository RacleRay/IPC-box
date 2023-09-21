#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shmsync.h"
#include "utils.h"


void init_sync(struct sync* sync);
void run_sync_client(void* shm_addr, struct sync* sync, int msg_size, int msg_count);


int main(int argc, char *argv[]) {

    struct arguments args;
    parse_arguments(&args, argc, argv);

    // create shared memory
    size_t shm_size = args.msg_size + sizeof(struct sync);
    int shm_id = shmget((key_t)LAZY_TEST_KEY, shm_size, IPC_CREAT | 0666);
    if (shm_id < 0) {
        err_sys("shm client shm get");
    }

    void* shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr < (void*)-1) {
        err_sys("shm client shmat");
    }
    memset(shm_addr, 0, shm_size);

    // condition variables
    struct sync* sync = (struct sync*)(shm_addr + args.msg_size);
    init_sync(sync);

    // run
    run_sync_client(shm_addr, sync, args.msg_size, args.msg_count);

    // clean up
    shmdt(shm_addr);

    return 0;
}


void init_sync(struct sync* sync) {
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;

    if (pthread_mutexattr_init(&mutex_attr) != 0) {
        err_sys("initialize mutex attribute");
    }
    if (pthread_condattr_init(&cond_attr) != 0) {
        err_sys("initialize condition variable attribute");
    }

    // multiple processes may access these objects. The default is PTHREAD_PROCESS_PRIVATE.
    if (pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) != 0) {
        err_sys("set mutex attribute");
    }
    if (pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED) != 0) {
        err_sys("set condition variable attribute");
    }

    // initialize 
    if (pthread_mutex_init(&sync->mutex, &mutex_attr) != 0) {
        err_sys("initialize mutex");
    }
    if (pthread_cond_init(&sync->cond, &cond_attr) != 0) {
        err_sys("initialize condition variable");
    }

    // destroy attributes
    if (pthread_mutexattr_destroy(&mutex_attr) != 0) {
        err_sys("destroy mutex attribute");
    }
    if (pthread_condattr_destroy(&cond_attr) != 0) {
        err_sys("destroy condition variable attribute");
    }
}


void run_sync_client(void* shm_addr, struct sync* sync, int msg_size, int msg_count) {
    void *buf = malloc(msg_size);

    //notify server
    pthread_cond_signal(&sync->cond);

    printf("sync shm client start.\n");

    for (int i = 0; i < msg_count; i++) {
        // wait for server
        pthread_mutex_lock(&sync->mutex);
        pthread_cond_wait(&sync->cond, &sync->mutex);

        // recevie message
        memcpy(buf, shm_addr, msg_size);

        // send message
        memset(shm_addr, 's', msg_size);

        // notify server
        pthread_cond_signal(&sync->cond);
    }    

    printf("sync shm client end.\n");

    free(buf);
}