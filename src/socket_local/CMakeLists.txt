add_executable(unsock_server server.c)
add_executable(unsock_client client.c)


target_link_libraries(unsock_server ipc-utils)
target_link_libraries(unsock_client ipc-utils)
