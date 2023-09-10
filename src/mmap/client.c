#include <fcntl.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "utils.h"

void run_client(char *file_memory, int msg_size, int msg_count);

int main(int argc, char *argv[]) {
    arguments_t args;
    parse_arguments(&args, argc, argv);

    // open file for mmap
    int file_descriptor = open("/tmp/mmap", O_RDWR | O_CREAT, 0666);
    if (file_descriptor < 0) { err_sys("mmap file open"); }
    ftruncate(file_descriptor, args.msg_size);

    // mmap file
    //  MAP_SHARED: flush modified data to the underlying file immediately,
    //  instead of buffering. necessary for IPC.
    //  Note: if use MAP_FILE, you must use msync() to flush the data to the file.
    void *file_memory =
        mmap(NULL, args.msg_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
    if (file_memory == MAP_FAILED) { err_sys("mmap file"); }

    if (close(file_descriptor) < 0) { err_sys("close file"); }

    // run client
    run_client(file_memory, args.msg_size, args.msg_count);

    // unmap
    if (munmap(file_memory, args.msg_size) < 0) { err_sys("unmap file"); }

    (void)remove("/tmp/mmap");

    return 0;
}

void run_client(char *file_memory, int msg_size, int msg_count) {
    atomic_char *guard = (atomic_char *)file_memory;

    // notify the server
    atomic_store(guard, 's');

    void *buf = malloc(msg_size);
    for (int i = 0; i < msg_count; i++) {
        // wait for server
        while (atomic_load(guard) != 'c') {}

        // read
        memcpy(buf, file_memory, msg_size);
        // write
        memset(file_memory, '@', msg_size);

        // notify the server
        atomic_store(guard, 's');
    }

    free(buf);
}
