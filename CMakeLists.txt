cmake_minimum_required(VERSION 3.0.0)
project(tcp-server VERSION 0.0.1)

add_subdirectory(md5)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} md5)