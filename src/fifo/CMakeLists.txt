add_executable(fifo_server server.c)
add_executable(fifo_client client.c)


target_link_libraries(fifo_server ipc-utils)
target_link_libraries(fifo_client ipc-utils)
