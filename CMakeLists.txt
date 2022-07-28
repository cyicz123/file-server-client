cmake_minimum_required(VERSION 3.0.0)
project(tcp-server VERSION 0.0.1)

SET(CMAKE_BUILD_TYPE "Debug") 
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall")

add_subdirectory(md5)
add_subdirectory(file)
add_subdirectory(str)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} md5)
target_link_libraries(${PROJECT_NAME} file_process)
target_link_libraries(${PROJECT_NAME} str)