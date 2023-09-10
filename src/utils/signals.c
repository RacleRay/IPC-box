#include "signals.h"
#include "utils.h"


// do nothing
void signal_handler(int signal) {}


sigset_t setup_sever_signal(struct sigaction* signal_action) {
    sigset_t oset = setup_signals(signal_action, BLOCK_USR1 | IGNORE_USR2);
    usleep(1000);
    return oset;
}


sigset_t setup_client_signal(struct sigaction* signal_action) {
    sigset_t oset = setup_signals(signal_action, BLOCK_USR2 | IGNORE_USR1);
    usleep(1000);
    return oset;
}


void setup_ignore_signals(struct sigaction* signal_action, int flags) {
    signal_action->sa_handler = signal_handler;
    
    // will ignore SIGUSR1
    if (!(flags & BLOCK_USR1)) {
        if (sigaction(SIGUSR1, signal_action, NULL) != 0) {
            err_sys("Can`t register SIGUSR1 handler.");
        }
    }

    // will ignore SIGUSR2
    if (!(flags & BLOCK_USR2)) {
        if (sigaction(SIGUSR2, signal_action, NULL) != 0) {
            err_sys("Can`t register SIGUSR2 handler.");
        }
    }
}


sigset_t setup_block_signals(struct sigaction* signal_action, int flags) {
    signal_action->sa_handler = SIG_DFL;  // default action

    if (flags & BLOCK_USR1) {
        sigaddset(&signal_action->sa_mask, SIGUSR1);
    }

    if (flags & BLOCK_USR2) {
        sigaddset(&signal_action->sa_mask, SIGUSR2);
    }

    sigset_t oset;
    sigprocmask(SIG_BLOCK, &signal_action->sa_mask, &oset);

    return oset;
}


sigset_t setup_signals(struct sigaction* signal_action, int flags) {
    signal_action->sa_flags = SA_RESTART; // restart the system calls if interrupted

    sigemptyset(&signal_action->sa_mask);
    setup_ignore_signals(signal_action, flags);

    sigset_t oset;
    sigemptyset(&signal_action->sa_mask);    
    oset = setup_block_signals(signal_action, flags);

    return oset;    
}