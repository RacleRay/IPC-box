#include "shm.h"

static inline void shm_wait(atomic_char *guard, char message) {
    while (atomic_load(guard) != message) {}
}

static inline void shm_notify(atomic_char *guard, char message) {
    atomic_store(guard, message);
}

void run_server(char *shm_memory, int msg_size, int msg_count) {
    void *buf = malloc(msg_size);

    atomic_char *guard = (atomic_char *)shm_memory;

    shm_wait(guard, 's');
    
    printf("Start shm server test loops\n");
    // while (1) {
    for (int i = 0; i < msg_count; ++i) {
        // test
        memset(shm_memory + 1, '*', msg_size);

        shm_notify(guard, 'c');
        shm_wait(guard, 's');

        memcpy(buf, shm_memory + 1, msg_size);
    }

    printf("shm server test loops done\n");

    free(buf);
}

void run_client(char *shm_memory, int msg_size, int msg_count) {
    void *buf = malloc(msg_size);

    atomic_char *guard = (atomic_char *)shm_memory;
    atomic_init(guard, 's');

    // assert(sizeof(atomic_char) == 1);
    printf("Start shm client test loops\n");

    // while (1) {
    for (int i = 0; i < msg_count; ++i) {
        shm_wait(guard, 'c');

        // test
        memcpy(buf, shm_memory + 1, msg_size);

        memset(shm_memory + 1, '+', msg_size);

        shm_notify(guard, 's');
    }

    printf("shm client test loops done\n");

    free(buf);
}