/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-25 14:50:43
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-25 17:25:56
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
 * @description: 开启一个服务器线程
 * @param {void*} arg sockaddr_in传递一个全局变量用于回传服务器创建后的地址和端口等信息
 * @return {*}
 */
void *StartServer();

void *HandleClient(void* arg);

#endif