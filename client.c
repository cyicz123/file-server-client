/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-26 10:45:59
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-26 15:40:24
 * @FilePath: /tcp-server/client.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
// demo: socket tcp client
#include "log/log.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
	// input check
	if(argc < 2){
		printf("eg: ./client port\n");
		exit(1);
	}
	int port = atoi(argv[1]);

	// 1.create a socket
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);

	// 2.connect server
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	// inet_pton(AF_INET, "localhost", &server_addr.sin_addr.s_addr);

	// 3.connect
	int ret = connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret < 0) {
		log_error("Connect the server failed");
		close(client_fd);
		exit(0);
	}

	// 4.communication
	while(1){
		
		printf("input data:");
		// send data
		char buf[1024];
		fgets(buf, sizeof(buf), stdin);
		write(client_fd, buf, 1024);

		// wait and receive data
		int read_ret = read(client_fd, buf, sizeof(buf));
		if(read_ret == -1){
			perror("read error");
			exit(1);
		}else if(read_ret == 0){
			printf("server has closed connection.\n");
			break;
		}else{
			printf("received data:%s\n", buf);
		}
	}

	close(client_fd);
	return 0;
}
