#include <arpa/inet.h>
#include <errno.h>
#include <hiredis/read.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <hiredis/hiredis.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "file/file_process.h"
#include "md5/md5.h"
#include "network/network.h"
#include "str/strUtils.h"
#include "log/log.h"

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
	log_info("[%u]: Connecting redis database.", info->tid);
    if(conn == NULL || conn->err)
	{
		log_error("[%u]: connection error:%s", info->tid,  conn->errstr);  
		close(info->fd);
		redisFree(conn); 
		pthread_exit(NULL);
	}

	while(1)
	{
		if(trans_flag == 0)
		{
			// 服务端接收完全部数据收尾工作。合并所有文件，redis数据库标志位置为true
			log_info("[%u]: Received all datas.Ready to merge file.", info->tid);
			reply = redisCommand(conn, "hget File-%s block_num", md5);
			uint32_t block_num = atol(reply->str);
			freeReplyObject(reply);

			reply = redisCommand(conn, "hget File-%s index", md5);
			size_t index = atol(reply->str);
			freeReplyObject(reply);

			FILE* write_fd = WriteFile(md5);
			for(; index<block_num; index++)
			{
				// 合并小数据包
				int index_s_length = GetIntDigit(index) + 1;
				char* index_s = (char*)malloc(index_s_length * sizeof(char));
				Uint32ToStr(index_s, index_s_length, index);
				
				size_t path_length = (strlen(md5) + strlen(index_s) + 2 + 1 ) * sizeof(char); //2为.%s-%s中的固定字符数量
				char* data_path = (char*)malloc(path_length * sizeof(char));

				snprintf(data_path, path_length * sizeof(char), ".%s-%s", md5, index_s);
				free(index_s);
				
				if(!ExistFile(data_path))
				{
					// 此序号数据包不存在，让客户端重传此包。并将合并文件序号置为此序号
					log_error("[%u]: The %d index data doesn't exist.Want to resend it.", info->tid, index);
					reply = redisCommand(conn, "hset File-%s index %d",md5,index);
					freeReplyObject(reply);
					reply = redisCommand(conn, "setbit BitMap-%s %d 0",md5,index);
					freeReplyObject(reply);
					// 数据包传输标志位重新置为1
					trans_flag = 1;
					free(data_path);
					// 退出合并数据包循环
					rp.index = index;
					rp.head = 0x0000;
					rp.size = 1;
					log_info("[%u]: Send the resend message.");
					send(info->fd,&rp,sizeof(rp),0);
					break;
				}
				else
				{
					MergeFile(write_fd, data_path);
					remove(data_path);
					free(data_path);
				}
			}
			CloseFile(write_fd);
			if(trans_flag == 0)
			{
				// 表示合并成功。进入校验文件部分。
				log_info("[%u]: Merge successful! Check it md5 checksum.", info->tid);
				FILE* rd_fd = ReadFile(md5);
				uint8_t decrypt[16] = {0};
				char s_decrypt[33] = {'\0'};
				GetFileMD5(rd_fd, decrypt);
				Byte2Str(decrypt, 16, s_decrypt);
				if (CompareByte(md5,s_decrypt,32) == 0) 
				{
					// 校验失败，需要重传所有数据包，重置BitMap为全0
					log_error("[%u]: Inconsistent checksum. Need to resend all.", info->tid);
					reply = redisCommand(conn, "del BitMap-%s", md5);
					freeReplyObject(reply);
					reply = redisCommand(conn, "setbit BitMap-%s %d 0",md5 , block_num - 1 );
					freeReplyObject(reply);
					rp.index = 0;
					rp.head = 0x0000;
					rp.size = block_num;
					send(info->fd,&rp,sizeof(rp),0);
					trans_flag = 1;
					CloseFile(write_fd);
					// 删除此错误文件
					remove(md5);
				}
				else 
				{
					// 校验成功，发送确认报文
					log_info("[%u]: Receive successful! Confirm it to client.", info->tid);
					rp.head = 0x0001;
					send(info->fd,&rp,sizeof(rp),0);
					break;
				}
			}
			else 
			{
				// 合并失败，重新循环，进入接收程序。
				log_warn("[%u]: Merge failed.Receive them again.", info->tid);
				continue;
			}
			
		}
		// 接收程序部分
		uint8_t* data_buf;
		memset(&sp, 0, sizeof(sp));
		memset(&qb, 0, sizeof(qb));
		memset(&rp, 0, sizeof(rp));

		log_info("[%u]: Receiving messages.", info->tid);
		// 接收报文头部
		log_info("[%u]: Receiving message head.", info->tid);
		if(Receive(info->fd, &sp, sizeof(sp)) == 1)
		{
            log_error("[%u]: Received an incorrect message.", info->tid) ;
			close(info->fd);
			redisFree(conn); 
			pthread_exit(NULL);
		}
		if(sp.head == 0x0000)
		{
			// 查询报文
			// log_info("Receive a query message.");
			log_info("[%u]: Receive a query message.", info->tid) ;
			if(Receive(info->fd, &qb, sizeof(qb)) == 1)
			{
            	log_error("[%u]: Received an incorrect message.", info->tid) ;
				close(info->fd);
				redisFree(conn); 
				pthread_exit(NULL);
			}
			buf_size = sp.buf_length - sizeof(qb);
			char *file_name = (char*)malloc((buf_size+1)*sizeof(char));
			if(Receive(info->fd, file_name, buf_size) == 1)
			{
            	log_error("[%u]: Received an incorrect message.", info->tid) ;
				close(info->fd);
				free(file_name);
				redisFree(conn); 
				pthread_exit(NULL);
			}
			file_name[buf_size] = '\0';
		
			Byte2Str(qb.checksum, 16, md5);
			md5[32] = '\0';
			reply = redisCommand(conn, "exists File-%s", md5);
			if (reply->integer == 1) 
			{	
				// 该文件部分或全部已经传输过了。
				log_info("[%u]: %s has existed.", info->tid,  file_name);
				freeReplyObject(reply);
				reply = redisCommand(conn, "hget File-%s file_flag", md5);
				
				if (CompareByte(reply->str, "true", 4) == 1)
				{
					// 标志位为true，说明要传输的文件已存在。设置传输数据标志位为0,跳出socket连接循环
					log_info("[%u]: The file has been received.", info->tid) ;
					free(file_name);
					freeReplyObject(reply);
					trans_flag = 0;
					rp.head = 0x0001;
					send(info->fd,&rp,sizeof(rp),0);
					break;
				}
				freeReplyObject(reply);
				reply = redisCommand(conn, "hget File-%s block_num", md5);
				uint32_t block_num = atol(reply->str);
				freeReplyObject(reply);
				reply = redisCommand(conn, "bitpos BitMap-%s 0 0 %d bit", md5, block_num-1);
				long long pos = reply->integer;
				freeReplyObject(reply);

				if (pos == -1)
				{
					// 说明报文全部收到，需要进入合并文件程序。传输数据标志为0，重新开始循环。
					log_info("[%u]: The file's all datas have been received, but not been merged. Ready to merge them.", info->tid) ;
					free(file_name);
					trans_flag = 0;
					continue;
				}
				
				reply = redisCommand(conn, "bitpos BitMap-%s 1 %d %d bit", md5, pos+1, block_num-1);
				long long new_pos = reply->integer;
				freeReplyObject(reply);
				
				uint32_t window_size = 0;
				if (new_pos != -1) 
				{
					// 说明从pos到末尾有已经接收了的数据包。
					window_size = new_pos - pos;
				}
				else 
				{
					// 说明后面全都没被接收。
					window_size = block_num - pos;	
				}
				
				rp.head = 0x0000;
				rp.index = pos;
				rp.size = window_size;
				log_info("[%u]: Send the result message.", info->tid) ;
				send(info->fd,&rp,sizeof(rp),0);
			}
			else
			{
				// 文件没有被传输过。
				log_info("[%u]: A new file will be transmitted.", info->tid) ;
				freeReplyObject(reply);
				uint32_t block_num = GetBlockNum(qb.file_size, qb.block_size);
				reply = redisCommand(conn, "hset File-%s file_size %lld file_name %s block_size %d block_num %d file_flag %s index 0", md5, qb.file_size, file_name, qb.block_size, block_num, "false");  
    			freeReplyObject(reply);  
				reply = redisCommand(conn, "setbit BitMap-%s %d 0", md5, block_num);
				freeReplyObject(reply);
				rp.head = 0x0000;
				rp.index = 0;
				rp.size = block_num;
				log_info("[%u]: Send the result.", info->tid) ;
				send(info->fd,&rp,sizeof(rp),0);
			}

			free(file_name);
			
		}
		else if(sp.head == 0x0001)
		{
			// 数据报文
			log_info("[%u]: Receive a data buf. Length is %d.", info->tid,  sp.buf_length);
			uint8_t checksum[16] = {0}, decrypt[16]={0};
			if(Receive(info->fd, checksum, 16) == 1)
			{
            	log_error("[%u]: Received an incorrect message.", info->tid) ;

				close(info->fd);
				redisFree(conn); 
				pthread_exit(NULL);
			}
			data_buf = (uint8_t*)malloc(sp.buf_length-16);
			if(Receive(info->fd, data_buf, sp.buf_length-16) == 1)
			{
            	log_error("[%u]: Received an incorrect message.", info->tid) ;
				free(data_buf);
				close(info->fd);
				redisFree(conn); 
				pthread_exit(NULL);
			}
			GetStrMD5(data_buf, sp.buf_length-16, decrypt);
			if(CompareByte(checksum, decrypt, 16) == 0)
			{
				log_warn("[%u]: Checksum is wrong.Drop %d data.", info->tid, sp.index);
				free(data_buf);
				continue;
			}
			log_info("[%u]: Writing the %d index data.", info->tid, sp.index);
			WriteData(md5, sp.index, data_buf, sp.buf_length - 16);
			reply = redisCommand(conn, "setbit BitMap-%s %d 1", md5, sp.index);
			freeReplyObject(reply);
		}

		
	}
	
	// 说明已经成功收到文件，设置文件接收标志位为true
	reply = redisCommand(conn, "hset File-%s file_flag true",md5);
	freeReplyObject(reply);

	close(info->fd);
	redisFree(conn);
	
	return NULL;
}

int main(int argc, char* argv[])
{

	int port_nu = atoi("8080"); //	user input port number
	
	// 1.create a socket
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0){
		log_error("err: create socket fail! caused by %s", strerror(errno));
	}

	// 2.bind ip and port
	struct sockaddr_in server_addr;
	bzero((char*)& server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;                // addr family
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // ip addr
	server_addr.sin_port = htons(port_nu);	         // port num

	int bind_ret = bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(bind_ret < 0){
		log_error("err: bind fail! caused by %s", strerror(errno));
	}

	// 3.listen
	int listen_ret = listen(socket_fd, 128);
	if(listen_ret < 0){
		log_error("err: listen fail! caused by %s", strerror(errno));
	}

	log_info("the server started...");
	log_info("listen on port:%d", port_nu);
	log_info("waiting for client...");


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