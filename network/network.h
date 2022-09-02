/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-03 14:57:30
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-09-02 14:50:32
 * @FilePath: /tcp-server/network/network.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#ifndef NET_H
#define NET_H


#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>

#define SINGLE_TRANSMISSION_LEN 1073741824

#define TYPE_GET 1
#define TYPE_POST 2
#define TYPE_DELETE 3
#define TYPE_QUERY 4
#define TYPE_COMMAND 5

#define NET_PROTOCOL_GET 100
#define NET_PROTOCOL_POST 200
#define NET_PROTOCOL_DELETE 300
#define NET_PROTOCOL_QUERY 400
#define NET_PROTOCOL_COMMAND 500

#define REQUEST_OK 200
#define BAD_REQUEST 400
#define NOT_FOUND 404
#define INTERNAL_SERVER_ERROR 500

#define QUERY_MODE_LIST 0 //默认路径查询
#define QUERY_MODE_LIST_WITH_PATH 1  //带有路径的查询
#define QUERY_MODE_FILE_SIZE 2
#define QUERY_MODE_FILE_SIZE_WITH_PATH 3

#define GET_MODE_DOWNLOAD 0


/**
 * @description: 通过提取百位的数字来得到控制命令
 * @param {uint16_t}
 * @return {*} 
 */
#define getMode(x) (x / 100 % 10)



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
 * @param {sockaddr_in*} server_addr 服务器地址
 * @param {pthread_mutex_t} mutex 多线程锁
 * @return {int} socket_fd 
 */
int PrepareServer(struct sockaddr_in* server_addr, pthread_mutex_t* mutex);

/**
 * @description: 连接服务器
 * @param {sockaddr_in*} server_addr 服务器地址
 * @return {int} socket_fd -1 错误
 */
int ConnectServer(struct sockaddr_in* server_addr);

/**
 * @description: fd收取size字节的数据
 * @param {int} fd 套接字fd
 * @param {uint8_t*} buf 存放数据的数组
 * @param {size_t} size 字节长度
 * @return {int} 0 成功 -1 连接断开 大于零表示断开连接导致未收到想要收到的字节
 */
int Receive(int fd, void* buf, size_t size);

/**
 * @description: 读取size个字节，不清除缓冲区
 * @param {int} fd 套接字fd
 * @param {void*} buf 存放数据的数组
 * @param {size_t} size 字节长度
 * @return {int}  接受的字节数 -1 连接断开 
 */
int recvPeekLine(int fd, size_t size);

/**
 * @description: 读到'\n'就返回，将最后一位的\n更改为\0
 * @param {int} fd 套接字fd
 * @param {void*} buf 存放数据的数组
 * @param {size_t} size 每次支持最多读取size个字节
 * @return {int}  接受的字节数 -1 连接断开 
 */
int ReadLine(int fd, void* buf, size_t size);

/**
 * @description: 发送size字节的数据
 * @param {int} fd 套接字fd
 * @param {void*} buf 存放数据的数组
 * @param {size_t} size 字节长度
 * @return {int} 0 成功 -1 连接断开 大于零表示断开连接导致未发送想要发送的字节
 */
int Send(int fd, void* buf, size_t size);

/**
 * @description: 发送一行数据，末尾发送一个\n
 * @param {int} fdk 套接字fd
 * @param {void*} buf 存放数据的数组
 * @param {size_t} size 最多发送的字节
 * @return {int}  发送的字节数 -1 连接断开 
 */
int WriteLine(int fd, const void* buf, size_t size);

#endif