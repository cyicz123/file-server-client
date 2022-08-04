#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h> //pthread header

#include "network/network.h"

// define SockInfo struct
typedef struct SockInfo{
	int fd;
	struct sockaddr_in addr;
	pthread_t tid;
}SockInfo;

// thread handler function
void *pth_fun(void *arg){
	SockInfo *info = (SockInfo *)arg;

	SendProtocol sp;
	QueryBuf qb;
	size_t buf_size=0;

	char buff[512*1024+16*sizeof(uint8_t)+sizeof(uint32_t)]; //32个字节表示协议长度，16个字节装md5校验码，剩下512k装数据。
	memset(&sp, 0, sizeof(sp));
	memset(buff, 0, sizeof(buff));
	memset(&qb, 0, sizeof(qb));

	int debug_1 = recv(info->fd, &sp, sizeof(sp), 0);
	if(sp.head == 0x0000)
	{
		printf("Receive a query message.\n");
		int debug_2 = recv(info->fd, &qb, sizeof(qb), 0);
		buf_size = sp.buf_length - sizeof(qb);
		char *file_name = (char*)malloc((buf_size+1)*sizeof(char));
		int debug_3 = recv(info->fd, file_name, buf_size, 0);
		file_name[buf_size] = '\0';
		printf("File name is %s\n",file_name);
		free(file_name);
	}

	// print IP and port of client
	printf("accept client IP:%s, port:%d\n", \
			inet_ntoa(info->addr.sin_addr), \
			ntohs(info->addr.sin_port));

	// transfer
	// while(1){
	// 	memset(buff, 0, sizeof(buff));
	// 	int read_ret = read(info->fd, buff, sizeof(buff));
	// 	if(read_ret == -1 ){
	// 		printf("err: read fail! caused by %s\n", strerror(errno));
	// 		pthread_exit(NULL);
	// 	}else if(read_ret == 0){
	// 		printf("The client has closed!\n");
	// 		close(info->fd);
	// 		break;
	// 	}

	// 	printf("[IP:%s, port:%d] recv data:%s\n", \
	// 			inet_ntoa(info->addr.sin_addr), \
	// 			ntohs(info->addr.sin_port), buff);

	// 	write(info->fd, buff, read_ret); // send buff to client
	// }
	return NULL;
}

int main(int argc, char* argv[])
{
	// check user input port
	if(argc<2){
		printf("eg: ./server port\n");
		exit(1);
	}

	int port_nu = atoi(argv[1]); //	user input port number

	// 1.create a socket
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0){
		printf("err: create socket fail! caused by %s\n", strerror(errno));
	}

	// 2.bind ip and port
	struct sockaddr_in server_addr;
	bzero((char*)& server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;                // addr family
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // ip addr
	server_addr.sin_port = htons(port_nu);	         // port num

	int bind_ret = bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(bind_ret < 0){
		printf("err: bind fail! caused by %s\n", strerror(errno));
	}

	// 3.listen
	int listen_ret = listen(socket_fd, 128);
	if(listen_ret < 0){
		printf("err: listen fail! caused by %s", strerror(errno));
	}

	printf("the server started...\n");
	printf("listen on port:%d\n", port_nu);
	printf("waiting for client...\n");


	// 4.wait and connect, pthread_creat 
	int i = 0;              // thread number
	SockInfo info[1024]; // maximum number of threads
	// set fd=-1
	for(;i < sizeof(info)/sizeof(info[0]); ++i){
		info[i].fd = -1;
	}
	socklen_t len = sizeof(struct sockaddr_in);
	while(1){

		// select minimum value of i, and fd[i]==-1
		for(i=0; i<sizeof(info)/sizeof(info[0]); ++i){
			if(info[i].fd == -1) break;
		}
		if(i == 256) break;

		// main thread: wait and connection
		info[i].fd = accept(socket_fd, (struct sockaddr*)&info[i].addr, &len);

		// create pthread to transfer
		pthread_create(&info[i].tid, NULL, pth_fun, &info[i]);

		// detach a thread
		pthread_detach(info[i].tid);

	}

	// 6.close
	close(socket_fd);

	// terminate main thread
	pthread_exit(NULL);
}