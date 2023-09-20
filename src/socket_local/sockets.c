#include "sockets.h"
#include "utils.h"

#ifndef SEND
#define SEND 0
#endif

#ifndef RECV
#define RECV 1
#endif


void set_socket_buffer_size(int sockfd, int size, int direction) {
    // setsockopt and getsockopt are our friends.
    // clang-format off
    int ret_state = setsockopt(
        sockfd,
        SOL_SOCKET,
        (direction == SEND) ? SO_SNDBUF : SO_RCVBUF,
        &size,
        sizeof(size)
    );
    // clang-format on

    if (ret_state == -1) {
        err_sys("set socket buffer size");
    }
}


// direction : 0 for send, 1 for recv
struct timeval get_socket_timeout(int socket_fd, int direction) {
	int ret_status;
	struct timeval timeout;
	socklen_t value_size = sizeof timeout;

	// clang-format off
	ret_status = getsockopt(
		socket_fd,
		SOL_SOCKET,
		(direction == SEND) ? SO_SNDTIMEO : SO_RCVTIMEO,
		&timeout,
		&value_size
	);
	// clang-format on

	if (ret_status == -1) {
		err_sys("getting socket timeout");
	}

	return timeout;
}

double get_socket_timeout_seconds(int socket_fd, int direction) {
	struct timeval timeout = get_socket_timeout(socket_fd, direction);
	return timeout.tv_sec + (timeout.tv_usec / 1e6);
}

// struct timeval timeout = {seconds, microseconds};
void set_socket_timeout(int socket_fd, struct timeval* timeout, int direction) {
	// clang-format off
	int ret_status = setsockopt(
		socket_fd,
		SOL_SOCKET,
		(direction == SEND) ? SO_SNDTIMEO : SO_RCVTIMEO,
		timeout,
		sizeof(*timeout)
	);
	// clang-format on

	if (ret_status == -1) {
		err_sys("setting blocking timeout");
	}
}