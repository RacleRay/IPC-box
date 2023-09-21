add_compile_options(-g -O3 -Wall -Werror)

link_libraries(pthread)


add_subdirectory(utils)
add_subdirectory(shm)
add_subdirectory(shm_chat)
add_subdirectory(pipe)
add_subdirectory(fifo)
add_subdirectory(mmap)
add_subdirectory(mq)
add_subdirectory(semaphore)
add_subdirectory(socket_local)
add_subdirectory(socket)
add_subdirectory(shm_condition)


# for each "src/x.cpp", generate target "x"
# file(GLOB_RECURSE all_srcs CONFIGURE_DEPENDS *.c)
# foreach(v ${all_srcs})
#     string(REGEX MATCH "src/.*" relative_path ${v})
#     # message(${relative_path})
#     string(REGEX REPLACE "src/" "" target_name ${relative_path})
#     # string(REGEX REPLACE ".cpp" "" target_name ${target_name})
#     string(REGEX REPLACE ".c" "" target_name ${target_name})

#     add_executable(${target_name} ${v})
# endforeach()
