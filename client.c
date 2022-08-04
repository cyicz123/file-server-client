/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-03 10:45:06
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-04 11:18:12
 * @FilePath: /tcp-server/client.c
 * @Description: 客户端
 */
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "network/network.h"
#include "file/file_process.h"
#include "md5/md5.h"


#define DATA_SIZE 1048576

int main(int argc, char* argv[])
{
	char *path;
    if (argc < 2) {
        printf("usage: %s 'path'\n", argv[0]);
        return 1;
    }
    path = argv[1];
    uint64_t file_size = GetFileSize(path);
    char* file_name = basename(path);
    FILE* file_fd = ReadFile(path);
    if (file_fd == NULL) 
    {
        return 1;
    }

    const char* s_port = "8080"; 
	int port = atoi(s_port);
	// 创建套接字
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);

	// 连接服务器
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	//server_addr.sin_addr.s_addr = htonl();
	if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr) != 1)
    {
        printf("Set the IP address failed.\n");
        return 1;
    }

	// 3.connect
	if(connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)
    {
        printf("Connect the server failed.\n");
        return 1;
    }
    
    uint8_t data_buf[DATA_SIZE]; 
    SendProtocol send_protocol;
    RecvProtocol recv_protocol;
    QueryBuf query_buf;
    
    recv_protocol.head = 0x0000;
    
	while(recv_protocol.head != 0x0001)
    {
		// 查询
        send_protocol.head = 0x0000;
        send_protocol.index = 0;

        query_buf.block_size = DATA_SIZE;
        query_buf.file_size = file_size;
        if(GetFileMD5(file_fd, query_buf.checksum) == 1)
        {
            printf("Calculate %s md5 failed.\n",file_name);
            return 1;
        }

        send_protocol.buf_length = sizeof(query_buf) + strlen(file_name);
        int debug_1 = send(client_fd,&send_protocol,sizeof(send_protocol),0);
        int debug_2 = send(client_fd,&query_buf,sizeof(query_buf),0);
        int debug_3 = send(client_fd,file_name,strlen(file_name),0);

        
        sleep(10);


		// printf("input data:");
		// // send data
		// char buf[1024];
		// fgets(buf, sizeof(buf), stdin);
		// write(client_fd, buf, strlen(buf));

		// // wait and receive data
		// int read_ret = read(client_fd, buf, sizeof(buf));
		// if(read_ret == -1){
		// 	perror("read error");
		// 	exit(1);
		// }else if(read_ret == 0){
		// 	printf("server has closed connection.\n");
		// 	break;
		// }else{
		// 	printf("received data:%s\n", buf);
		// }
	}

    CloseFile(file_fd);
	close(client_fd);
	return 0;
}