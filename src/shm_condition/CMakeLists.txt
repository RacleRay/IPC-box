add_executable(sync_shm_server server.c)
add_executable(sync_shm_client client.c)


target_link_libraries(sync_shm_server ipc-utils)
target_link_libraries(sync_shm_client ipc-utils)