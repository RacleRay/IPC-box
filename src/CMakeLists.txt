add_compile_options(-g -O3 -Wall -Werror)

link_libraries(pthread)


add_subdirectory(shm)
add_subdirectory(shm_chat)
# add_subdirectory(utils)



# for each "src/x.cpp", generate target "x"
# file(GLOB_RECURSE all_srcs CONFIGURE_DEPENDS *.c)
# foreach(v ${all_srcs})
#     string(REGEX MATCH "src/.*" relative_path ${v})
#     # message(${relative_path})
#     string(REGEX REPLACE "src/" "" target_name ${relative_path})
#     # string(REGEX REPLACE ".cpp" "" target_name ${target_name})
#     string(REGEX REPLACE ".c" "" target_name ${target_name})

#     add_executable(${target_name} ${v} utils.c shm_slave.c shm_master.c)
# endforeach()
