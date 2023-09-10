# IPC box

- shm
- pipe


- shm_chat: is a local multi-user chatroom based on shm and mmap.
```sh
# server compiled and run in the build directory.
./server

# client compiled and run in the build directory.
./client [client_id](1-100) [user_name]
```