#ifndef _IPC_SHM_
#define _IPC_SHM_

#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>

// SHNAME must refer to an existing, accessible file
#define SHNAME     "/tmp"
#define IDENTIFIER 'R'

#define LAZY_TEST_KEY 0x6666

void run_server(char *shm_memory, int msg_size, int msg_count);
void run_client(char *shm_memory, int msg_size, int msg_count);

#endif