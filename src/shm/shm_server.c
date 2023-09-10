#include "utils.h"
#include "shm.h"


int main(int argc, char *argv[]) {
    struct Arguments args;
    parse_arguments(&args, argc, argv);

    int shm_id;
    key_t shm_key;
    shm_key = ftok(SHNAME, IDENTIFIER);
    printf("Key: %d\n", shm_key);

    shm_id = shmget(shm_key, args.size + 1, IPC_CREAT |0666);
    if (shm_id < 0) {
        err_sys("Can`t allocate shared memory");
    }

    char* shm_memory = NULL;
    shm_memory = (char*)shmat(shm_id, NULL, 0);
    if (shm_memory == (char*)-1) {
        err_sys("Can`t attach shared memory");
    }

    // communicate
    run_server(shm_memory, args.size);

    // clean up
    shmdt(shm_memory);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}