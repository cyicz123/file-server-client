/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-25 14:50:50
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-26 14:09:47
 * @FilePath: /tcp-server/thread/thread.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "thread.h"
#include "../config.h"
#include "../network/network.h"
#include "../log/log.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t _server_mutex;


void *StartServer(){
    struct sockaddr_in server_addr;
    int socket_fd = -1;
    int available_thread_id = -1;
	socklen_t len = sizeof(struct sockaddr_in);
	thread_arg_server arg[THREAD_SERVER_MAXIUM_THREADS]; 
    // 初始化 arg数组
    memset(&arg, 0, sizeof(thread_arg_server) * THREAD_SERVER_MAXIUM_THREADS);
    for (size_t i = 0; i < THREAD_SERVER_MAXIUM_THREADS; i++) {
        arg[i].fd = -1;
        arg[i].tid = -1;
    }


    socket_fd = PrepareServer(&server_addr, &_server_mutex);
    if (socket_fd < 0) {
        return NULL;
    }

	
	while(1){
        // 选择一个可用的线程
        for (available_thread_id = 0; available_thread_id < THREAD_SERVER_MAXIUM_THREADS; available_thread_id++) {
            if(arg[available_thread_id].fd == -1) break;
        }
		if(available_thread_id == 256) break;

		arg[available_thread_id].fd = accept(socket_fd, (struct sockaddr*)&arg[available_thread_id].server_addr, &len);

		// 开启处理客户端的分离线程
		pthread_create(&arg[available_thread_id].tid, NULL, HandleClient, &arg[available_thread_id]);

		// 分离线程
		pthread_detach(arg[available_thread_id].tid);

	}

	close(socket_fd);
    
    return NULL;
}


void *HandleClient(void* arg){
    thread_arg_server* thread_arg = (thread_arg_server*)(arg);

    char buff[1024]; //32个字节表示协议长度，16个字节装md5校验码，剩下512k装数据。
    memset(buff, 0, sizeof(char) * 1024);

    // print IP and port of client
    log_info("accept client IP:%s, port:%d", inet_ntoa(thread_arg->server_addr.sin_addr), ntohs(thread_arg->server_addr.sin_port));

    // transfer
    while(1){
            memset(buff, 0, sizeof(buff));
            int read_ret = read(thread_arg->fd, buff, 1024);
            if(read_ret == -1 ){
                    log_error("err: read fail!");
                    thread_arg->fd = -1;
                    pthread_exit(NULL);
            }else if(read_ret == 0){
                    log_error("The client has closed!");
                    close(thread_arg->fd);
                    break;
            }

            log_info("recv data:%s", buff);

            write(thread_arg->fd, buff, read_ret); // send buff to client
    }
    thread_arg->fd = -1;
    return NULL;
}