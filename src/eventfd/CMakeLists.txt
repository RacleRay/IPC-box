add_executable(eventfd eventfd.c)

target_link_libraries(eventfd ipc-utils)