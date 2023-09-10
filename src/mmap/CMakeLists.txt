add_executable(mmap_server server.c)
add_executable(mmap_client client.c)


target_link_libraries(mmap_server ipc-utils)
target_link_libraries(mmap_client ipc-utils)
