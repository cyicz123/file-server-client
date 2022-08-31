/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-30 15:05:13
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-31 17:29:56
 * @FilePath: /tcp-server/client/client.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../config.h"
#include "client.h"
#include "../str/strUtils.h"
#include "../log/log.h"
#include "../network/network.h"
#include "../file/file_process.h"

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

int StartClient(int argc, char* argv[]){
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
    bool query_files_flag = 0;
    int query_ret = -1;

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
            query_files_flag = 1;
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

    if (1 == query_files_flag) {
        query_ret = QueryFiles(&ser_addr, storage_dir);
        if (query_ret < 0) {
            log_error("Query failed");
            return -1;
        }
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
    return 0;
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

int QueryFiles(struct sockaddr_in* ser_addr, const char* storage_dir){
    // 声明
    RequestBuf request_buf;
    ReplyBuf reply_buf;
    int client_fd = -1;
    int send_ret = -1;
    int recv_ret = -1;
    int dir_len = 0;
    char* send_dir = NULL;
    uint32_t file_num =0;
    char** files = NULL;

    //初始化
    request_buf.type = NET_PROTOCOL_QUERY;
    if (NULL == storage_dir) {
        request_buf.cmd = QUERY_MODE_LIST;
    }
    else {
        request_buf.cmd = QUERY_MODE_LIST_WITH_PATH;
        dir_len = strlen(storage_dir);
        send_dir = (char*)malloc(sizeof(char) * (dir_len + 1));
        memset(send_dir, '\n', sizeof(char) * (dir_len + 1));
        strncpy(send_dir, storage_dir, sizeof(char) * (dir_len + 1));
    }
    client_fd = ConnectServer(ser_addr);
    if (client_fd < 0) {
        return -1;
    }

    // 查询
    send_ret = Send(client_fd, &request_buf, sizeof(RequestBuf));
    if (0 != send_ret) {
        log_error("Query buf send failed.");
        return -1;
    }
    if (QUERY_MODE_LIST_WITH_PATH == request_buf.cmd) {
        send_ret = Send(client_fd, send_dir, sizeof(char) * (dir_len + 1));
        if (0 != send_ret) {
            log_error("Directory name send failed.");
            return -1;
        }
    }

    // 接收回复报文
    recv_ret = Receive(client_fd, &reply_buf, sizeof(ReplyBuf));
    if (0 != recv_ret) {
        log_error("Receive the query reply buf failed.");
        return -1;
    }
    if (NET_PROTOCOL_QUERY != reply_buf.type) {
        log_error("Receive reply buf's type is incorrect.");
        return -1;
    }
    if (REQUEST_OK != reply_buf.status_code) {
        log_error("Occur an error. Reply status code: %d", reply_buf.status_code);
        return -1;
    }

    // 接收文件的数量
    recv_ret = Receive(client_fd, &file_num, sizeof(uint32_t));
    if (0 != recv_ret) {
        log_error("Receive the query reply buf failed.");
        return -1;
    }
    file_num = ntohl(file_num);
    if (0 == file_num) {
        return 0;
    }

    //接收文件名，并打印
    files = (char**)malloc(sizeof(char*) * file_num);
    for (size_t i=0; i<file_num; i++) {
        files[i] = (char*)malloc(sizeof(char) * MAX_FILE_NAME_LENGTH);
        memset(files[i], '\0', sizeof(char) * MAX_FILE_NAME_LENGTH);

        recv_ret = ReadLine(client_fd, files[i], MAX_FILE_NAME_LENGTH);
        
        if (recv_ret < 0) {
            free(files[i]);
            free(files);
            return -1;
        }
        printf("File %zu: %s\n", i+1, files[i]);
        free(files[i]);
    }
    free(files);

    close(client_fd);
    return file_num;
}