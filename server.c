#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <errno.h>
#include <hiredis/read.h>
#include <sys/socket.h>
#include <hiredis/hiredis.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "file/file_process.h"
#include "md5/md5.h"
#include "network/network.h"
#include "str/strUtils.h"

// thread handler function
void *pth_fun(void *arg){
	SockInfo *info = (SockInfo *)arg;

	SendProtocol sp;
	QueryBuf qb;
	RecvProtocol rp;
	size_t buf_size=0;
	int trans_flag = 1;
	char md5[33]={'\0'};
	redisContext* conn = redisConnect("127.0.0.1", 6379);  
	redisReply* reply;

    if(conn == NULL || conn->err)
	{
		printf("connection error:%s\n", conn->errstr);  
		close(info->fd);
		redisFree(conn); 
		pthread_exit(NULL);
	}

	while(trans_flag)
	{
		uint8_t* data_buf;
		memset(&sp, 0, sizeof(sp));
		memset(&qb, 0, sizeof(qb));
		memset(&rp, 0, sizeof(rp));

		if(Receive(info->fd, &sp, sizeof(sp)) == 1)
		{
			close(info->fd);
			redisFree(conn); 
			pthread_exit(NULL);
		}
		if(sp.head == 0x0000)
		{
			printf("Receive a query message.\n");
			if(Receive(info->fd, &qb, sizeof(qb)) == 1)
			{
				close(info->fd);
				redisFree(conn); 
				pthread_exit(NULL);
			}
			buf_size = sp.buf_length - sizeof(qb);
			char *file_name = (char*)malloc((buf_size+1)*sizeof(char));
			if(Receive(info->fd, file_name, buf_size) == 1)
			{
				close(info->fd);
				free(file_name);
				redisFree(conn); 
				pthread_exit(NULL);
			}
			file_name[buf_size] = '\0';
			
			Byte2Str(qb.checksum, 16, md5);
			reply = redisCommand(conn, "exists %s", md5);
			if (reply->integer == 1) 
			{
				freeReplyObject(reply);
				reply = redisCommand(conn, "hget %s index", md5);
				if (reply->type != REDIS_REPLY_STRING) 
				{
					free(file_name);
					close(info->fd);
					redisFree(conn); 
					pthread_exit(NULL);
				}
				rp.index = atol(reply->str);
				freeReplyObject(reply);
				reply = redisCommand(conn, "hget %s block_num", md5);
				uint32_t block_num = atol(reply->str);
				freeReplyObject(reply);
				if(sp.index != block_num)
				{
					rp.head = 0x0000;
					rp.size = 0xffffffff;
					send(info->fd,&rp,sizeof(rp),0);
				}
				else 
				{
					
				}
			}
			else if(reply->integer == 0)
			{
				freeReplyObject(reply);
				uint32_t block_num = GetBlockNum(qb.file_size, qb.block_size);
				reply = redisCommand(conn, "hset %s file_size %lld file_name %s block_size %d index %d block_num %d", md5, qb.file_size, file_name, qb.block_size, 0, block_num);  
    			freeReplyObject(reply);  
				reply = redisCommand(conn, "setbit %s %ld 0", md5, block_num);
				freeReplyObject(reply);
				rp.head = 0x0000;
				rp.index = 0;
				rp.size = 0xffffffff;
				send(info->fd,&rp,sizeof(rp),0);
			}

			free(file_name);
			
		}
		else if(sp.head == 0x0001)
		{
			printf("Receive a data buf.\n");
			uint8_t checksum[16] = {0}, decrypt[16]={0};
			if(Receive(info->fd, checksum, 16) == 1)
			{
				close(info->fd);
				redisFree(conn); 
				pthread_exit(NULL);
			}
			data_buf = (uint8_t*)malloc(sp.buf_length-16);
			if(Receive(info->fd, data_buf, sp.buf_length-16) == 1)
			{
				free(data_buf);
				close(info->fd);
				redisFree(conn); 
				pthread_exit(NULL);
			}
			GetStrMD5(data_buf, sp.buf_length-16, decrypt);
			if(CompareByte(checksum, decrypt, 16) == 0)
			{
				printf("Checksum is wrong.Drop %d data.\n",sp.index);
				reply = redisCommand(conn, "setbit %s %d 1",md5,sp.index);
				freeReplyObject(reply);
				free(data_buf);
				continue;
			}
			printf("Writing %d data.\n",sp.index);
			WriteData(md5, sp.index, data_buf, sp.buf_length - 16);
			reply = redisCommand(conn, "hset %s index %d", md5, sp.index+1);
			freeReplyObject(reply);
		}
	}
	// print IP and port of client
	printf("accept client IP:%s, port:%d\n", \
			inet_ntoa(info->addr.sin_addr), \
			ntohs(info->addr.sin_port));

	close(info->fd);
	pthread_exit(NULL);
	redisFree(conn); 
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