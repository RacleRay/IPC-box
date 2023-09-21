add_executable(tcpsock_server server.c)
add_executable(tcpsock_client client.c)


target_link_libraries(tcpsock_server ipc-utils)
target_link_libraries(tcpsock_client ipc-utils)