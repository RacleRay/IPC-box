add_executable(sem_server server.c semaph.c)
add_executable(sem_client client.c semaph.c)


target_link_libraries(sem_server ipc-utils)
target_link_libraries(sem_client ipc-utils)
