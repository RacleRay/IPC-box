#include "shm.h"

static inline void shm_wait(atomic_char *guard, char message) {
    while (atomic_load(guard) != message) {}
}

static inline void shm_notify(atomic_char *guard, char message) {
    atomic_store(guard, message);
}

void run_server(char *shm_memory, int size) {
    void *buf = malloc(size);

    atomic_char *guard = (atomic_char *)shm_memory;

    shm_wait(guard, 's');
    
    printf("Start shm server test loops\n");
    // while (1) {
    for (int i = 0; i < TEST_LOOPS; ++i) {
        // test
        memset(shm_memory + 1, '*', size);

        shm_notify(guard, 'c');
        shm_wait(guard, 's');

        memcpy(buf, shm_memory + 1, size);
    }

    printf("shm server test loops done\n");

    free(buf);
}

void run_client(char *shm_memory, int size) {
    void *buf = malloc(size);

    atomic_char *guard = (atomic_char *)shm_memory;
    atomic_init(guard, 's');

    // assert(sizeof(atomic_char) == 1);
    printf("Start shm client test loops\n");

    // while (1) {
    for (int i = 0; i < TEST_LOOPS; ++i) {
        shm_wait(guard, 'c');

        // test
        memcpy(buf, shm_memory + 1, size);

        memset(shm_memory + 1, '+', size);

        shm_notify(guard, 's');
    }

    printf("shm client test loops done\n");

    free(buf);
}