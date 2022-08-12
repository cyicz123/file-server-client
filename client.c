/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-03 10:45:06
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-11 16:02:18
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
#include "log/log.h"
#include "str/strUtils.h"


#define DATA_SIZE 1048576 //1MB 2^20B

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
    log_info("Set connect port %d.", port);
	// 创建套接字
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    log_info("Create a socket.");

	// 连接服务器
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	//server_addr.sin_addr.s_addr = htonl();
	if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr) != 1)
    {
        // printf("Set the IP address failed.\n");
        log_error("Set the IP address failed.");
        return 1;
    }

	// 3.connect
	if(connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)
    {
        // printf("Connect the server failed.\n");
        log_error("Connect the server failed.");
        return 1;
    }
    
    uint8_t data_buf[DATA_SIZE]; 
    SendProtocol send_protocol;
    RecvProtocol recv_protocol;
    QueryBuf query_buf;

    uint8_t byte_md5[16];
    
    recv_protocol.head = 0x0000;
    
    query_buf.block_size = DATA_SIZE;
    query_buf.file_size = file_size;
    log_info("Calculating %s's md5...", file_name);
    GetFileMD5(file_fd, query_buf.checksum);
    char s_md5[33] = {'\0'};
    Byte2Str(query_buf.checksum, 16, s_md5);
    log_info("%s's md5 is %s", file_name, s_md5);

	while(1)
    {
		// 查询
        send_protocol.head = 0x0000;
        send_protocol.index = 0;
        send_protocol.buf_length = sizeof(query_buf) + strlen(file_name);
        
        // printf("Sending a query message.\n");
        log_info("Sending a query message.");
        send(client_fd,&send_protocol,sizeof(send_protocol),0);
        send(client_fd,&query_buf,sizeof(query_buf),0);
        send(client_fd,file_name,strlen(file_name),0);
        log_info("Send complete.");

        if(Receive(client_fd, &recv_protocol, sizeof(recv_protocol)) == 1)
        {
            // printf("Receive the reply message error.\n");
            log_error("Received an incorrect message.");
            CloseFile(file_fd);
            close(client_fd);
            return 1;
        }
        
        if (recv_protocol.head == 0x0001) {
            log_info("Success!");
            break;
        }

        uint64_t index_ceiling = recv_protocol.index+recv_protocol.size;
        uint32_t block_num = GetBlockNum(file_size, DATA_SIZE);
        for(uint32_t i = recv_protocol.index; i < index_ceiling||i < block_num; i++ )
        {
            uint32_t read_byte_size = 0;
            memset(data_buf, 0, DATA_SIZE*sizeof(uint8_t));
            memset(&send_protocol,0,sizeof(send_protocol));
            memset(byte_md5, 0, sizeof(byte_md5));
            read_byte_size = ReadData(file_fd, data_buf, DATA_SIZE, i);
            if(read_byte_size == 0)
            {
                CloseFile(file_fd);
                close(client_fd);
                return 1;
            }

            GetStrMD5(data_buf, read_byte_size, byte_md5);
            send_protocol.head = 0x0001;
            send_protocol.index = i;
            send_protocol.buf_length = read_byte_size + 16;
            log_info("Sending %d block. Block size is %d.",i,read_byte_size);
            send(client_fd,&send_protocol,sizeof(send_protocol),0);
            send(client_fd,byte_md5,16,0);
            send(client_fd,data_buf,read_byte_size,0);
        }
	}
    
    CloseFile(file_fd);
	close(client_fd);
	return 0;
}