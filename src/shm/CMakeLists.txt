
add_executable(shm_server shm_server.c shm.c ../utils/utils.c)
add_executable(shm_client shm_client.c shm.c ../utils/utils.c)