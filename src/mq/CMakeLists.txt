add_executable(mq_server server.c)
add_executable(mq_client client.c)

target_link_libraries(mq_server ipc-utils)
target_link_libraries(mq_client ipc-utils)