/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-26 10:45:59
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-30 14:04:27
 * @FilePath: /tcp-server/client.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
// demo: socket tcp client
#include "config.h"
#include "log/log.h"
#include "network/network.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#ifndef CLI_VERSION
#define CLI_VERSION "2.0.0"
#endif

/*
 * h 帮助 
 * s server address 后跟服务器 ip:port
 * c client address 后跟客户端 ip:port
 * l ls查看远程文件，可跟要查看的路径
 * d download 下载 后跟文件名
 * D Delete 删除 后跟文件名
 * u upload 上传 后跟文件名
*/
#define SHORT_OPTS "hvs:c:l::d:D:u:"

static struct option long_options[] = {
	{"help", no_argument, NULL, 'h'},
	{"ser_addr", required_argument, NULL, 's'},
	{"cli_addr", required_argument, NULL, 'c'},
	{"ls", optional_argument, NULL, 'l'},
	{"download", required_argument, NULL, 'd'},
	{"delete", required_argument, NULL, 'D'},
	{"upload", required_argument, NULL, 'u'},
	{"version", no_argument, NULL, 'v'}
};


/**
 * @description: 打印帮助信息
 * @param {char} opt 帮助选项，'a'打印所有
 * @param {char*} version 版本号
 * @return {*}
 */
void helpInfo(char opt, const char* version);

/**
 * @description: 将字符串IP:Port转换为sockaddr_in类型
 * @param {char*} str_addr
 * @param {sockaddr_in*} addr
 * @return {int} 0 成功 1 失败
 */
int Str2Addr(const char* str_addr, struct sockaddr_in* addr);


int main(int argc, char* argv[])
{ 
	// 声明
	char opt = -1; 
	int option_index = 0;
	int str2addr_ret = -1;
	char* storage_dir = NULL;
	char* delete_file = NULL;
	char* upload_file = NULL;
	char* download_file = NULL;
	struct sockaddr_in ser_addr;
	struct sockaddr_in cli_addr;

	// 初始化
	memset(&ser_addr, 0, sizeof(struct sockaddr_in));
	memset(&cli_addr, 0, sizeof(struct sockaddr_in));
	str2addr_ret = Str2Addr(CLIENT_DEFAULT_ADDR, &ser_addr);
	if (1 == str2addr_ret) {
		log_error("Set client address failed.");
		return 1;
	}
	str2addr_ret = Str2Addr(CLIENT_DEFAULT_ADDR, &cli_addr);
	if (1 == str2addr_ret) {
		log_error("Set client address failed.");
		return 1;
	}


	
	while (true) {
		opt = getopt_long(argc, argv, SHORT_OPTS, long_options, &option_index);
		if (-1 == opt) {
			break;
		}

		switch (opt) {
		case 'h':
			helpInfo('a', CLI_VERSION);
			break;
		case 'v':
			helpInfo('v', CLI_VERSION);
			break;
		case 's':
			if (NULL != optarg) {
				str2addr_ret = Str2Addr(optarg, &ser_addr);
			}
			else {
				str2addr_ret = Str2Addr(CLIENT_DEFAULT_ADDR, &ser_addr);
			}
			if (1 == str2addr_ret) {
				log_error("Set server address failed.");
				return 1;
			}
			break;
		case 'c':
			if (NULL != optarg) {
				str2addr_ret = Str2Addr(optarg, &cli_addr);
			}
			else {
				str2addr_ret = Str2Addr(CLIENT_DEFAULT_ADDR, &cli_addr);
			}
			if (1 == str2addr_ret) {
				log_error("Set client address failed.");
				return 1;
			}
			break;
		case 'l':
			if (NULL != optarg) {
				storage_dir = optarg;
			}
			break;
		case 'd':
			if (NULL != optarg) {
				download_file = optarg;
			}
			break;
		case 'D':
			if (NULL != optarg) {
				delete_file = optarg;
			}
			break;
		case 'u':
			if (NULL != optarg) {
				upload_file = optarg;
			}
			break;
		}
	}
	log_info("[Server IP: %s, Port: %d]", inet_ntoa(ser_addr.sin_addr), ntohs(ser_addr.sin_port));
	log_info("[Client IP: %s, Port: %d]", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

	if (NULL != storage_dir) {
		printf("storage_dir: %s\n", storage_dir);
		return 0;
	}
	if (NULL != upload_file) {
		printf("upload_file: %s\n", upload_file);
		return 0;
	}
	if (NULL != download_file) {
		printf("download_file: %s\n", download_file);
		return 0;
	}
	if (NULL != delete_file) {
		printf("delete_file: %s\n", delete_file);
		return 0;
	}
}

void helpInfo(char opt, const char* version){
	if ('v' == opt) {
		printf("Hi-Hik version %s\n", version);
		return;
	}
	printf("Usage: Hi-Hik [OPTION]... [FILE]...\n");
	printf("A file transfer tool, support upload and download.\n");
	printf("\n");
	printf("Mandatory arguments to long options are mandatory for short options too.\n");
	switch (opt) {
	case 's':
		printf("\t-s, --ser_addr IP:Port\t\t\tSet the server address to IP :port. The default address is 127.0.0.1:8080.\n");
		break;
	case 'c':
		printf("\t-c, --cli_addr IP:Port\t\t\tSet the client address to IP :port. The default address is 127.0.0.1:8080. \
			If both client servers are enabled in the same address, the port number is automatically incremented.\n");
		break;
	case 'l':
		printf("\t-l, --ls [FILE-DIRECTORY]\t\t\tView remote server downloadable files.\n");
		break;
	case 'd':
		printf("\t-d, --download FILE\t\t\tDownload the file.\n");
		break;
	case 'D':
		printf("\t-D, --delete FILE\t\t\tDelete the file.\n");
		break;
	case 'u':
		printf("\t-u, --upload FILE\t\t\tUpload the file.\t");
		break;
	default:
		printf("\t-s, --ser_addr IP:Port\t\t\tSet the server address to IP :port. The default address is 127.0.0.1:8080.\n");
		printf("\t-c, --cli_addr IP:Port\t\t\tSet the client address to IP :port. The default address is 127.0.0.1:8080. If both client servers are enabled in the same address, the port number is automatically incremented.\n");
		printf("\t-l, --ls [FILE-DIRECTORY]\t\t\tView remote server downloadable files.\n");
		printf("\t-d, --download FILE\t\t\tDownload the file.\n");
		printf("\t-D, --delete FILE\t\t\tDelete the file.\n");
		printf("\t-u, --upload FILE\t\t\tUpload the file.\n");
		printf("\n");
		printf("\t-h, --help\t\t\tPrint the help info.\n");
		printf("\t-v, --version\t\t\tPrint the version.\n");
		break;
	}
	
	return;
}


int Str2Addr(const char* str_addr, struct sockaddr_in* addr){
	const char* str_port = strchr(str_addr, ':'); // str_port结果为":port"
	int convert_ret = -1;
	
	if (NULL != str_port && str_port > str_addr) { // str_port非空且str_port在str_addr右边，则可认为str_port有效
		int port = atoi(str_port + 1);
		if(port < 1 || port > 65535){
			log_error("The address %s port is illegal!", str_addr);
			return 1;
		}
		addr->sin_port = htons(port);

		char str_ip[255] = {'\0'};
		memcpy(str_ip, str_addr, str_port-str_addr);
		convert_ret = inet_aton(str_ip, &addr->sin_addr);
		if (1 != convert_ret) {
			log_error("The address %s ip is illegal!", str_addr);
			return 1;
		}
		return 0;
	}
	else {
		log_error("The address %s is illegal!", str_addr);
		return 1;
	}
}