/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-03 14:57:30
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-29 16:35:02
 * @FilePath: /tcp-server/network/network.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#ifndef NET_H
#define NET_H


#include <netinet/in.h>
#include <stdint.h>
#include <pthread.h> 
#include <sys/socket.h>

// typedef struct SockInfo{
// 	int fd;
// 	struct sockaddr_in addr;
// 	pthread_t tid;
// }SockInfo;


typedef struct RequestBuf
{
    uint16_t type;
    uint16_t cmd;
}RequestBuf;

typedef struct ReplyBuf{
    uint16_t type;
    uint16_t status_code;
}ReplyBuf;


/**
 * @description: 启动服务器，进入等待连接状态
 * @param {uint16_t} port 端口号
 * @return {int} socket_fd 
 */
int PrepareServer(struct sockaddr_in* server_addr, pthread_mutex_t* mutex);

/**
 * @description: fd收取size字节的数据
 * @param {int} fd 套接字fd
 * @param {uint8_t*} buf 存放数据的数组
 * @param {size_t} size 字节长度
 * @return {size_t} 0 成功 -1 连接断开 大于零表示断开连接导致未收到想要收到的字节
 */
size_t Receive(int fd, void* buf, size_t size);

/**
 * @description: 发送size字节的数据
 * @param {int} fd 套接字fd
 * @param {void*} buf 存放数据的数组
 * @param {size_t} size 字节长度
 * @return {size_t} 0 成功 -1 连接断开 大于零表示断开连接导致未收到想要收到的字节
 */
size_t Send(int fd, void* buf, size_t size);

#endif