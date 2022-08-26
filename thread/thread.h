/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-25 14:50:43
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-26 19:18:03
 * @FilePath: /tcp-server/thread/thread.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef THREAD_H
#define THREAD_H

#include <netinet/in.h>
#include <pthread.h>

typedef struct thread_arg_server{
    struct sockaddr_in server_addr;
    int fd;
    pthread_t tid;
}thread_arg_server;

/**
 * @description: 启动服务器
 * @return {*} 
 */
void StartServer();

/**
 * @description: 处理客户端请求的thread回调函数
 * @param {void*} thread_arg_server* 传送一个此结构体指针
 * @return {*}
 */
void *HandleClient(void* arg);

#endif