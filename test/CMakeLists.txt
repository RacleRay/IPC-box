# # for each "test/x.cpp", generate target "x"
# file(GLOB_RECURSE all_tests *.cpp *.c)
# foreach(v ${all_tests})
#     string(REGEX MATCH "test/.*" relative_path ${v})
#     # message(${relative_path})
#     string(REGEX REPLACE "test/" "" target_name ${relative_path})
#     string(REGEX REPLACE ".cpp" "" target_name ${target_name})
#     string(REGEX REPLACE ".c" "" target_name ${target_name})

#     add_executable(${target_name} ${v})
# endforeach()


# 下面的方法可能更好，更稳定，不容易出现找不到对象的问题

# # 显式列出每个源文件
# set(all_tests
#     file1.cpp
#     file2.cpp
#     file3.c
#     # ...
# )

# # 添加可执行文件并包含源文件
# add_executable(my_executable ${all_tests})