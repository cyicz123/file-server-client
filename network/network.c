/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-03 14:57:36
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-09-01 14:45:47
 * @FilePath: /tcp-server/network/network.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../config.h"
#include "network.h"
#include "../log/log.h"

#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int PrepareServer(struct sockaddr_in* server_addr, pthread_mutex_t* mutex){
	int socket_fd = -1;
    uint16_t bind_port = SERVER_START_PORT;
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

int ConnectServer(struct sockaddr_in* server_addr){
    int socket_fd = -1;
    server_addr->sin_family = AF_INET;
    int connect_ret = -1;
    
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0){
		log_error("Create socket fail!");
        return -1;
	}
    
    connect_ret = connect(socket_fd, (struct sockaddr*)server_addr, sizeof(struct sockaddr_in));
    if (0 != connect_ret) {
        log_error("Connect the server failed.");
        return -1;
    }
    
    return socket_fd;
}

/**
 * @description: fd收取size字节的数据
 * @param {int} fd 套接字fd
 * @param {uint8_t*} buf 存放数据的数组
 * @param {size_t} size 字节长度
 * @return {int} 0 成功 -1 连接断开 大于零表示断开连接导致未收到想要收到的字节
 */
int Receive(int fd, void* buf, size_t size)
{
    int max_times = 10; 
    size_t recv_byte = 0;
    size_t recv_once = 0;
    for(size_t i =0; i < max_times; i++)
    {
        // 接收缓冲区指针需要偏移已接收字节数，接收字节数为总字节数-已接收字节数
        recv_once = recv(fd, buf + recv_byte, size - recv_byte,0);
        if (-1 == recv_once) {
            log_error("Read fail! caused by %s.", strerror(errno));
            return -1;
        }
        else if (0 == recv_once) {
            log_info("Disconnect the connection.");
            return -1;
        }
        recv_byte = recv_byte + recv_once;
        if(recv_byte == size)
            break;
    }
    if (recv_byte == size) 
    {
        return 0;
    }
    else 
    {
        log_warn("Expect to receive %ld bytes, only receive %ld bytes.", size, recv_byte);
        return recv_byte;
    }
}

// TODO recv有陷入死等的可能
int recvPeekLine(int fd, size_t size){
    char* temp = (char*)malloc(size);
    memset(temp, 0, size);
    int recv_byte = 0;
    int recv_once = 0;

    while (recv_byte<size) {
        recv_once = recv(fd, temp + recv_byte, size - recv_byte, MSG_PEEK);
        if (-1 == recv_once) {
            log_error("Read fail! caused by %s.", strerror(errno));
            free(temp);
            return -1;
        }
        else if (0 == recv_once) {
            log_info("Disconnect the connection.");
            free(temp);
            return -1;
        }
        
        for (size_t i=0; i<recv_once; i++) {
            if ('\n' == temp[recv_byte + i]) {
                recv_byte = recv_byte + i + 1;
                free(temp);
                return recv_byte;
            }
        }
        recv_byte = recv_byte + recv_once;
    }
    free(temp);
    return size;
}

int ReadLine(int fd, void* buf, size_t size){
    char* s_buf = (char*)buf;
    int line_len = 0;
    int recv_ret = -1;

    line_len = recvPeekLine(fd, size);
    recv_ret = Receive(fd, buf, line_len);
    if (-1 == recv_ret) {
        log_error("Read one line failed.");
        return -1;
    }
    if (size == line_len) {
        log_warn("Produces a truncation.");
    }
    s_buf[line_len-1] = '\0';
    return line_len;
}

int Send(int fd, void* buf, size_t size){
    int max_times = 10; 
    size_t send_byte = 0;
    size_t send_once = 0;
    for(size_t i =0; i < max_times; i++)
    {
        // 发送缓冲区指针需要偏移已发送字节数，发送字节数为总字节数-已发送字节数
        send_once = send(fd, buf + send_byte, size - send_byte, 0);
        if (-1 == send_once) {
            log_error("Read fail! caused by %s.", strerror(errno));
            return -1;
        }
        else if (0 == send_once) {
            log_warn("Disconnect the connection.");
            return -1;
        }
        send_byte = send_byte + send_once;
        if(send_byte == size)
            break;
    }
    if (send_byte == size) 
    {
        return 0;
    }
    else 
    {
        log_warn("Expect to send %ld bytes, only send %ld bytes.", size, send_byte);
        return send_byte;
    }
}

int WriteLine(int fd, const void* buf, size_t size){
    char* s_buf = (char*)malloc(size+1);
    memset(s_buf, 0, size+1);
    memcpy(s_buf, buf, size);
    int send_ret = -1;
    int send_max_len = 0;
    int buf_len = 0;

    buf_len = strlen(s_buf);
    send_max_len = buf_len < size ? buf_len : size;
    s_buf[send_max_len] = '\n';
    send_ret = Send(fd, s_buf, send_max_len+1);
    if (send_ret < 0) {
        log_error("Send one line failed.");
        free(s_buf);
        return -1;
    }
    free(s_buf);
    return send_max_len + 1;
}