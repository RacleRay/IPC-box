#ifndef _IPC_SIGNALS_H_
#define _IPC_SIGNALS_H_

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#define IGNORE_USR1 0x0
#define IGNORE_USR2 0x0
#define BLOCK_USR1  0x1
#define BLOCK_USR2  0x2

#define WAIT   0x0
#define NOTIFY 0x1

void signal_handler(int signal);

sigset_t setup_signals(struct sigaction *new_sa, int flags);
sigset_t setup_parent_signals();
sigset_t setup_server_signals(struct sigaction *new_sa);
sigset_t setup_client_signals(struct sigaction *new_sa);

void client_once(int operation);
void server_once(int operation);

#endif