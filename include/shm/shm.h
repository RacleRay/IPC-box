#ifndef _IPC_SHM_
#define _IPC_SHM_

#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>


#define SHNAME "shm"
#define IDENTIFIER 'R'

#define TEST_LOOPS 1000

void run_server(char* shm_memory, int size);
void run_client(char *shm_memory, int size);

#endif 