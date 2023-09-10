add_executable(pipe_test pipe.c)

target_link_libraries(pipe_test ipc-utils)