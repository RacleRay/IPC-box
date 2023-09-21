#ifndef _IPC_SHM_SYNC_H_
#define _IPC_SHM_SYNC_H_

#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>

#define LAZY_TEST_KEY 0x6666

// for synchronization
struct sync {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

#endif