#ifndef _IPC_SIGNALS_
#define _IPC_SIGNALS_

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

sigset_t setup_signals(struct sigaction *signal_action, int flags);
sigset_t setup_parent_signals();
sigset_t setup_server_signals(struct sigaction *signal_action);
sigset_t setup_client_signals(struct sigaction *signal_action);

void notify_server();
void notify_client();

void wait_for_signal(struct sigaction *signal_action);

void client_once(int operation);
void server_once(int operation);

#endif