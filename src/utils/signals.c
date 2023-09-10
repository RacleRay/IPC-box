#include "signals.h"
#include "utils.h"


// do nothing
void signal_handler(int signal) {}


sigset_t setup_server_signals(struct sigaction* new_sa) {
    sigset_t oset = setup_signals(new_sa, BLOCK_USR1 | IGNORE_USR2);
    usleep(1000);
    return oset;
}


sigset_t setup_client_signals(struct sigaction* new_sa) {
    sigset_t oset = setup_signals(new_sa, BLOCK_USR2 | IGNORE_USR1);
    usleep(1000);
    return oset;
}


void setup_ignore_signals(struct sigaction* new_sa, int flags) {
    new_sa->sa_handler = signal_handler;
    
    // will ignore SIGUSR1
    if (!(flags & BLOCK_USR1)) {
        if (sigaction(SIGUSR1, new_sa, NULL) != 0) {
            err_sys("Can`t register SIGUSR1 handler.");
        }
    }

    // will ignore SIGUSR2
    if (!(flags & BLOCK_USR2)) {
        if (sigaction(SIGUSR2, new_sa, NULL) != 0) {
            err_sys("Can`t register SIGUSR2 handler.");
        }
    }
}


sigset_t setup_block_signals(struct sigaction* new_sa, int flags) {
    new_sa->sa_handler = SIG_DFL;  // default action

    if (flags & BLOCK_USR1) {
        sigaddset(&new_sa->sa_mask, SIGUSR1);
    }

    if (flags & BLOCK_USR2) {
        sigaddset(&new_sa->sa_mask, SIGUSR2);
    }

    sigset_t oset;
    sigprocmask(SIG_BLOCK, &new_sa->sa_mask, &oset);

    return oset;
}


sigset_t setup_signals(struct sigaction* new_sa, int flags) {

    // this is not really reliable
    new_sa->sa_flags = SA_RESTART; // restart the system calls if interrupted

    sigemptyset(&new_sa->sa_mask);
    setup_ignore_signals(new_sa, flags);

    sigset_t oset;
    sigemptyset(&new_sa->sa_mask);    
    oset = setup_block_signals(new_sa, flags);

    return oset;    
}