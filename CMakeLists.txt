cmake_minimum_required(VERSION 3.0.0)
project(tcp-server VERSION 0.1.0)

SET(CMAKE_BUILD_TYPE "DEBUG") 
SET(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall")

link_libraries(pthread)
# add_subdirectory(md5)
add_subdirectory(file)
add_subdirectory(str)
add_subdirectory(log)
add_subdirectory(network)
add_subdirectory(server)
add_subdirectory(client)

add_executable(cli client.c)
add_executable(ser server.c)
# add_executable(test test.c)

target_link_libraries(cli log)
target_link_libraries(cli clientLib)
target_link_libraries(ser log)
target_link_libraries(ser serverLib)
