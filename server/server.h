/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-25 14:50:43
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-09-05 15:46:10
 * @FilePath: /tcp-server/thread/thread.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef THREAD_H
#define THREAD_H

#include "../network/network.h"

#include <stdint.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>

FILE* server_log;

typedef struct thread_arg_server{
    struct sockaddr_in addr;
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

/**
 * @description: 处理Get报文
 * @return {uint16_t} 0 成功 非零为错误码
 */
uint16_t handleGet(thread_arg_server* arg, RequestBuf* request_buf);

/**
 * @description: 处理下载
 * @param {thread_arg_server*} arg
 * @param {RequestBuf*} request_buf
 * @return {uint16_t} 0 成功 非零为错误码
 */
uint16_t handleGetDownload(thread_arg_server* arg, RequestBuf* request_buf);

/**
 * @description: 处理Post报文
 * @return {uint16_t} 0 成功 非零为错误码
 */
uint16_t handlePost(thread_arg_server* arg, RequestBuf* request_buf);

/**
 * @description: 处理上传
 * @param {thread_arg_server*} arg
 * @param {RequestBuf*} request_buf
 * @return {uint16_t} 0 成功 非零为错误码
 */
uint16_t handlePostUpload(thread_arg_server* arg, RequestBuf* request_buf);

/**
 * @description: 处理Delete报文
 * @return {uint16_t} 0 成功 非零为错误码
 */
uint16_t handleDelete(thread_arg_server* arg, RequestBuf* request_buf);

/**
 * @description: 处理Query报文
 * @return {uint16_t} 0 成功 非零为错误码
 */
uint16_t handleQuery(thread_arg_server* arg, RequestBuf* request_buf);

/**
 * @description: 处理Command报文
 * @return {uint16_t} 0 成功 非零为错误码
 */
uint16_t handleCommand(thread_arg_server* arg, RequestBuf* request_buf);

uint16_t handleCommandMergeFile(thread_arg_server* arg, RequestBuf* request_buf);

/**
 * @description: 处理Query报文中的查询文件列表功能
 * @param {thread_arg_server*} arg
 * @param {RequestBuf*} request_buf
 * @return {uint16_t} 0 成功 非零为错误码
 */
uint16_t handleQueryListPath(thread_arg_server* arg, RequestBuf* request_buf);

uint16_t handleQueryFileSize(thread_arg_server* arg, RequestBuf* request_buf);

void handleError(thread_arg_server* arg, RequestBuf* request_buf, int error_code);
#endif