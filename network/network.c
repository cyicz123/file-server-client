/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-03 14:57:36
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-26 14:09:33
 * @FilePath: /tcp-server/network/network.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../config.h"
#include "network.h"
#include "../log/log.h"

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>


int PrepareServer(struct sockaddr_in* server_addr, pthread_mutex_t* mutex){
	int socket_fd = -1;
    uint16_t bind_port = THREAD_SERVER_START_PORT;
    int bind_ret = -1;
    int listen_ret = -1;
    
	// 1.create a socket
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0){
		log_error("Create socket fail!");
        return -1;
	}

	// 2.bind ip and port
    pthread_mutex_lock(mutex);
    memset(server_addr, 0, sizeof(struct sockaddr_in));
	server_addr->sin_family = AF_INET;                // addr family
	server_addr->sin_addr.s_addr = htonl(INADDR_ANY); // ip addr

    while (bind_ret != 0) {      
        log_info("Binding a port:%d", bind_port);
        server_addr->sin_port = htons(bind_port);
	    bind_ret = bind(socket_fd, (struct sockaddr*)server_addr, sizeof(struct sockaddr_in));
        if(bind_ret != 0) {
            log_warn("Port:%d has been used. Retry a new port.", bind_port);
        }
        if (0 == bind_port) { //说明bind_port 产生了正溢出
            log_error("All ports have been used.");
            pthread_mutex_unlock(mutex);
            return -1;
        }
        bind_port++;
    }
	pthread_mutex_unlock(mutex);

	// 3.listen
	listen_ret = listen(socket_fd, 128);
	if(listen_ret < 0){
		log_error("Listen fail!");
        return -1;
	}

	log_info("the server started...");
	log_info("listen on port: %d", ntohs(server_addr->sin_port));
	log_info("waiting for client...");

    return socket_fd;
}

/**
 * @description: fd收取size字节的数据
 * @param {int} fd 套接字fd
 * @param {uint8_t*} buf 存放数据的数组
 * @param {size_t} size 字节长度
 * @return {int} 0 成功 1 失败
 */
int Receive(int fd, void* buf, size_t size)
{
    int times = 10 ; 
    size_t recv_byte = 0;
    for(size_t i =0; i < times; i++)
    {
        recv_byte = recv_byte + recv(fd,buf,size - recv_byte,0);
        if(recv_byte == size)
            break;
    }
    if (recv_byte == size) 
    {
        return 0;
    }
    else 
    {
        printf("Expect to receive %ld bytes, only receive %ld bytes.\n",size,recv_byte);
        return 1;
    }
}