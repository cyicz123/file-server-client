/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-25 14:50:50
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-09-04 15:51:59
 * @FilePath: /tcp-server/thread/thread.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "server.h"
#include "../config.h"
#include "../log/log.h"
#include "../file/file_process.h"
#include "../str/strUtils.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>


pthread_mutex_t _server_mutex;


void StartServer(){
    struct sockaddr_in server_addr;
    int socket_fd = -1;
    int available_thread_id = -1;
	socklen_t len = sizeof(struct sockaddr_in);
    int storage_ret = -1;
	thread_arg_server arg[SERVER_MAXIUM_THREADS]; 
    // 初始化 arg数组
    memset(&arg, 0, sizeof(thread_arg_server) * SERVER_MAXIUM_THREADS);
    for (size_t i = 0; i < SERVER_MAXIUM_THREADS; i++) {
        arg[i].fd = -1;
        arg[i].tid = 0;
    }

    storage_ret = CreateFile(SERVER_STORAGE_FILE);
    if (0 == storage_ret) {
        log_error("Set the storage file file. Please check the path: %s", SERVER_STORAGE_FILE);
        exit(1);
    }

    socket_fd = PrepareServer(&server_addr, &_server_mutex);
    if (socket_fd < 0) {
        exit(1);
    }

	
	while(1){
        // 选择一个可用的线程
        for (available_thread_id = 0; available_thread_id < SERVER_MAXIUM_THREADS; available_thread_id++) {
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
    if (NULL == thread_arg) {
       return NULL; 
    }
    memset(&request_buf, 0, sizeof(RequestBuf));

    while (true) {
        ret = Receive(thread_arg->fd, &request_buf, sizeof(RequestBuf));
        if (-1 == ret ) {
            log_info("Connect interrupt.");
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
    uint16_t ret = 0;
    if (GET_MODE_DOWNLOAD == request_buf->cmd || GET_MODE_DOWNLOAD_WITH_PATH == request_buf->cmd) {
        ret = handleGetDownload(arg, request_buf);
    }
    
    return ret;
}

uint16_t handleGetDownload(thread_arg_server* arg, RequestBuf* request_buf){
    ReplyBuf reply_buf;
    int ret = -1;
    char dir_path[MAX_FILE_NAME_LENGTH] = SERVER_STORAGE_FILE;
    char file_name[MAX_FILE_NAME_LENGTH] = {'\0'};
    char file_path[MAX_FILE_NAME_LENGTH + MAX_FILE_NAME_LENGTH] = {'\0'}; // 两倍，防止拼接溢出
    char* buf = NULL;
    uint64_t once_send_byte = 0;
    DownloadBlockInfo download_info;
    FILE* read_fd = NULL;

    // 条件检查
    if(GET_MODE_DOWNLOAD_WITH_PATH == request_buf->cmd){
        // 接收可选的下载路径选项
        log_info("Download the file size in the specified directory.");
        memset(dir_path, 0, MAX_FILE_NAME_LENGTH);
        ret = ReadLine(arg->fd, dir_path, MAX_FILE_NAME_LENGTH);
        if (ret < 0) {
            return INTERNAL_SERVER_ERROR;
        }
    }
    ret = ExistFile(dir_path);
    if (false == ret) {
        return NOT_FOUND;
    }
    // 接收文件名
    ret = ReadLine(arg->fd, file_name, MAX_FILE_NAME_LENGTH);
    if (ret < 0) {
        log_error("Receive the file name failed during handing the query about file size.");
        return INTERNAL_SERVER_ERROR;
    }
    // 接收下载信息
    ret = Receive(arg->fd, &download_info, sizeof(DownloadBlockInfo));
    if (0 != ret) {
        return INTERNAL_SERVER_ERROR;
    }
    // 根据路径和文件名合成url路径
    Combine(file_path, dir_path, file_name);
    ret = ExistFile(file_path);
    if (false == ret) {
        return NOT_FOUND;
    }
    // 回复成功请求报文
    reply_buf.type = request_buf->type;
    reply_buf.status_code = REQUEST_OK;
    ret = Send(arg->fd, &reply_buf, sizeof(ReplyBuf));
    if (0 != ret) {
        return INTERNAL_SERVER_ERROR;
    }
    // 发送下载数据
    read_fd = ReadFile(file_path);
    buf = (char*)malloc(sizeof(char) * SINGLE_TRANSMISSION_LEN);
    while (0 != download_info.len) {
        buf = memset(buf, 0, sizeof(char) * SINGLE_TRANSMISSION_LEN);
        if (download_info.len < SINGLE_TRANSMISSION_LEN) {
            once_send_byte = download_info.len;
        }
        else {
            once_send_byte = SINGLE_TRANSMISSION_LEN;
        }
        // 文件可能超过4G，需要用fseeko替代fseek
        ret = fseeko(read_fd, download_info.head, SEEK_SET);
        if (ret < 0) {
            log_error("Can't read the data.");
            CloseFile(read_fd);
            free(buf);
            return INTERNAL_SERVER_ERROR; 
        }
        // 读取文件数据
        memset(buf, 0, SINGLE_TRANSMISSION_LEN);
        ret = fread(buf, 1, once_send_byte, read_fd);
        if (once_send_byte != ret) {
            log_error("Can't read the data.");
            free(buf);
            CloseFile(read_fd);
            return INTERNAL_SERVER_ERROR;
        }
        // 发送文件数据
        ret = Send(arg->fd, buf, once_send_byte);
        if (0 != ret) {
            free(buf);
            return INTERNAL_SERVER_ERROR;
        }
        // 更新下载信息
        download_info.head = download_info.head + once_send_byte;
        download_info.len = download_info.len - once_send_byte;
    }
    free(buf);
    CloseFile(read_fd);
    return 0;
}


uint16_t handlePost(thread_arg_server* arg, RequestBuf* request_buf){
    return 0;
}


uint16_t handleDelete(thread_arg_server* arg, RequestBuf* request_buf){
    return 0;
}


uint16_t handleQuery(thread_arg_server* arg, RequestBuf* request_buf){
    uint16_t query_ret = 0;
    if (QUERY_MODE_LIST == request_buf->cmd || QUERY_MODE_LIST_WITH_PATH == request_buf->cmd) {
        query_ret = handleQueryListPath(arg, request_buf);
    }
    else if (QUERY_MODE_FILE_SIZE == request_buf->cmd || QUERY_MODE_FILE_SIZE_WITH_PATH == request_buf->cmd) {
        query_ret = handleQueryFileSize(arg, request_buf);
    }
    return query_ret;
    
}

uint16_t handleQueryListPath(thread_arg_server* arg, RequestBuf* request_buf){
    int ret = -1;
    char **files;
    int file_num = 0;
    uint32_t net_file_num = 0;
    char prefix[MAX_FILE_NAME_LENGTH] = SERVER_STORAGE_FILE;
    ReplyBuf reply_buf;


    // 初始化files
    files = (char**)malloc(sizeof(char*) * MAX_DIR_FILES_NUM);

    // 设置prefix
    if(QUERY_MODE_LIST_WITH_PATH == request_buf->cmd){
        log_info("Query the files in the specified directory.");
        ret = ReadLine(arg->fd, prefix, MAX_FILE_NAME_LENGTH);
        if (ret < 0) {
            free(files);
            return INTERNAL_SERVER_ERROR;
        }
    }
    
    

    file_num = ShowDirFiles(prefix, files);
    if (file_num < 0) {
        free(files);
        return NOT_FOUND;
    }
    // 到了此步，说明查询成功，需要回复相应成功报文
    reply_buf.type = request_buf->type;
    reply_buf.status_code = REQUEST_OK;
    ret = Send(arg->fd, &reply_buf, sizeof(ReplyBuf));
    if (ret < 0) {
        FreeFiles(files, file_num);
        log_error("Send reply buf error.");
        return INTERNAL_SERVER_ERROR;
    }
    // 发送文件数量
    net_file_num = htonl(file_num);
    ret = Send(arg->fd, &net_file_num, sizeof(uint32_t));
    if (ret < 0) {
        FreeFiles(files, file_num);
        log_error("Send file numbers error.");
        return INTERNAL_SERVER_ERROR;
    }
    //发送文件
    for (size_t i=0; i<file_num; i++) {
        ret = WriteLine(arg->fd, files[i], MAX_FILE_NAME_LENGTH);
        if (ret < 0) {
            FreeFiles(files, file_num);
            log_error("Send file name error.");
            return INTERNAL_SERVER_ERROR;
        }
    }
    log_info("Send files completely.");
    FreeFiles(files, file_num);
    return 0;
}

uint16_t handleQueryFileSize(thread_arg_server* arg, RequestBuf* request_buf){
    char dir_path[MAX_FILE_NAME_LENGTH] = SERVER_STORAGE_FILE;
    char file_name[MAX_FILE_NAME_LENGTH] = {'\0'};
    char file_path[MAX_FILE_NAME_LENGTH + MAX_FILE_NAME_LENGTH] = {'\0'}; // 两倍，防止拼接溢出
    int ret = -1;
    int exist_file = false;
    uint64_t file_size = 0;
    ReplyBuf reply_buf;
    
    if(QUERY_MODE_FILE_SIZE_WITH_PATH == request_buf->cmd){
        log_info("Query the file size in the specified directory.");
        memset(dir_path, 0, MAX_FILE_NAME_LENGTH);
        ret = ReadLine(arg->fd, dir_path, MAX_FILE_NAME_LENGTH);
        if (ret < 0) {
            return INTERNAL_SERVER_ERROR;
        }
    }
    ret = ExistFile(dir_path);
    if (false == ret) {
        return NOT_FOUND;
    }

    ret = ReadLine(arg->fd, file_name, MAX_FILE_NAME_LENGTH);
    if (ret < 0) {
        log_error("Receive the file name failed during handing the query about file size.");
        return INTERNAL_SERVER_ERROR;
    }

    Combine(file_path, dir_path, file_name);
    exist_file = ExistFile(file_path);
    if (false == exist_file) {
        return NOT_FOUND;
    }
    file_size = GetFileSize(file_path);
    
    reply_buf.type = request_buf->type;
    reply_buf.status_code = REQUEST_OK;
    ret = Send(arg->fd, &reply_buf, sizeof(ReplyBuf));
    if (ret < 0) {
        log_error("Send reply buf error.");
        return INTERNAL_SERVER_ERROR;
    }

    ret = Send(arg->fd, &file_size, sizeof(uint64_t));
    if (ret < 0) {
        log_error("Send reply buf error.");
        return INTERNAL_SERVER_ERROR;
    }
    return 0;
}

uint16_t handleCommand(thread_arg_server* arg, RequestBuf* request_buf){
    return 0;
}


void handleError(thread_arg_server* arg, RequestBuf* request_buf, int error_code){
    size_t send_ret = -1;
    ReplyBuf reply_buf;
    
    if (error_code < 0) {
        reply_buf.status_code = BAD_REQUEST;
    }
    reply_buf.type = request_buf->type;
    reply_buf.status_code = error_code;
    send_ret = Send(arg->fd, &reply_buf, sizeof(reply_buf));
    if (0 != send_ret) {
        log_error("Send buf failed. Caused by %s.", strerror(errno));
    }
    return;
}

