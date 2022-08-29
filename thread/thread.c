/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-25 14:50:50
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-29 14:58:40
 * @FilePath: /tcp-server/thread/thread.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "thread.h"
#include "../config.h"
#include "../log/log.h"
#include "../file/file_process.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/**
 * @description: 通过提取百位的数字来得到控制命令
 * @param {uint16_t}
 * @return {*} 
 */
#define getMode(x) (x / 100 % 10)

#define TYPE_GET 1
#define TYPE_POST 2
#define TYPE_DELETE 3
#define TYPE_QUERY 4
#define TYPE_COMMAND 5

pthread_mutex_t _server_mutex;


void StartServer(){
    struct sockaddr_in server_addr;
    int socket_fd = -1;
    int available_thread_id = -1;
	socklen_t len = sizeof(struct sockaddr_in);
    int storage_ret = -1;
	thread_arg_server arg[THREAD_SERVER_MAXIUM_THREADS]; 
    // 初始化 arg数组
    memset(&arg, 0, sizeof(thread_arg_server) * THREAD_SERVER_MAXIUM_THREADS);
    for (size_t i = 0; i < THREAD_SERVER_MAXIUM_THREADS; i++) {
        arg[i].fd = -1;
        arg[i].tid = 0;
    }

    storage_ret = CreateFile(THREAD_SERVER_STORAGE_FILE);
    if (0 == storage_ret) {
        log_error("Set the storage file file. Please check the path: %s", THREAD_SERVER_STORAGE_FILE);
        exit(1);
    }

    socket_fd = PrepareServer(&server_addr, &_server_mutex);
    if (socket_fd < 0) {
        exit(1);
    }

	
	while(1){
        // 选择一个可用的线程
        for (available_thread_id = 0; available_thread_id < THREAD_SERVER_MAXIUM_THREADS; available_thread_id++) {
            if(arg[available_thread_id].fd == -1) break;
        }
		if(available_thread_id == 256) break;

		arg[available_thread_id].fd = accept(socket_fd, (struct sockaddr*)&arg[available_thread_id].addr, &len);

		// 开启处理客户端的分离线程
		pthread_create(&arg[available_thread_id].tid, NULL, HandleClient, &arg[available_thread_id]);

		// 分离线程
		pthread_detach(arg[available_thread_id].tid);

	}

	close(socket_fd);
    
    exit(0);
}


void *HandleClient(void* arg){
    //声明
    thread_arg_server* thread_arg = (thread_arg_server*)(arg);
    RequestBuf request_buf;
    int ret = -1;
    int request_ret = -1;

    //初始化
    memset(&request_buf, 0, sizeof(RequestBuf));

    while (true) {
        ret = Receive(thread_arg->fd, &request_buf, sizeof(RequestBuf));
        if (-1 == ret ) {
            log_warn("Connect interrupt.");
            break;
        }
        else if (0 != ret) {
            log_info("[IP: %s, Port: %d] The client is disconnected.", inet_ntoa(thread_arg->addr.sin_addr), ntohs(thread_arg->addr.sin_port));
            break;
        }

        switch (getMode(request_buf.type)) {
            case TYPE_GET:
            log_info("Handing a GET request.");
            request_ret = handleGet(thread_arg, &request_buf);
            break;

            case TYPE_POST:
            log_info("Handing a POST request.");
            request_ret = handlePost(thread_arg, &request_buf);
            break;
            
            case TYPE_DELETE:
            log_info("Handing a DELETE request.");
            request_ret = handleDelete(thread_arg, &request_buf);
            break;
            
            case TYPE_QUERY:
            log_info("Handing a QUERY request.");
            request_ret = handleQuery(thread_arg, &request_buf);
            break;

            case TYPE_COMMAND:
            log_info("Handing a COMMAND request.");
            request_ret = handleCommand(thread_arg, &request_buf);
            break;

            default:
            log_info("Unknow request.");
            request_ret = 404;
        }
        
        if (0 != request_ret) {
            handleError(thread_arg, &request_buf, request_ret);
        }
    }
    // close 一定要放在fd = -1 前面，因为重置连接没加锁，如果放在下面可能遇到先释放线程后，被另一个线程启用，最后fd没关闭。
    close(thread_arg->fd);     
    thread_arg->fd = -1;
    thread_arg->tid = 0;
    return NULL;
}

uint16_t handleGet(thread_arg_server* arg, RequestBuf* request_buf){
    return 0;
}


uint16_t handlePost(thread_arg_server* arg, RequestBuf* request_buf){
    return 0;
}


uint16_t handleDelete(thread_arg_server* arg, RequestBuf* request_buf){
    return 0;
}


uint16_t handleQuery(thread_arg_server* arg, RequestBuf* request_buf){
    return 0;
}


uint16_t handleCommand(thread_arg_server* arg, RequestBuf* request_buf){
    return 0;
}


void handleError(thread_arg_server* arg, RequestBuf* request_buf, int error_code){
    return;
}