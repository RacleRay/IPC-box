set(IPC_UTILS 
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/signals.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sockets.c)

add_library(ipc-utils STATIC ${IPC_UTILS})
target_compile_options(ipc-utils PUBLIC -fPIC)