/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-30 15:05:13
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-09-17 15:55:21
 * @FilePath: /tcp-server/client/client.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "../config.h"
#include "client.h"
#include "../str/strUtils.h"
#include "../log/log.h"
#include "../network/network.h"
#include "../file/file_process.h"
#include "../progressbar/progressbar.h"
#include "../progressbar/statusbar.h"

#include <signal.h>
#include <libgen.h>


static struct option long_options[] = {
	{"help", no_argument, NULL, 'h'},
	{"ser_addr", required_argument, NULL, 's'},
	{"ls", optional_argument, NULL, 'l'},
	{"download", required_argument, NULL, 'd'},
	// {"delete", required_argument, NULL, 'D'},
	{"upload", required_argument, NULL, 'u'},
	{"version", no_argument, NULL, 'v'}
};

int StartClient(int argc, char* argv[]){
    // 声明
	char opt = -1; 
	int option_index = 0;
	int str2addrret = -1;
	char* storage_dir = NULL;
	// char* delete_file = NULL;
	char* upload_file = NULL;
	char* download_file = NULL;
	struct sockaddr_in ser_addr;
    bool query_files_flag = 0;
    int ret = -1;

	// 初始化
	memset(&ser_addr, 0, sizeof(struct sockaddr_in));
	str2addrret = Str2Addr(CLIENT_DEFAULT_ADDR, &ser_addr);
	if (1 == str2addrret) {
		log_error("Set client address failed.");
		return 1;
	}
    // 处理命令行
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
				str2addrret = Str2Addr(optarg, &ser_addr);
			}
			else {
				str2addrret = Str2Addr(CLIENT_DEFAULT_ADDR, &ser_addr);
			}
			if (1 == str2addrret) {
				log_error("Set server address failed.");
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
		// case 'D':
		// 	if (NULL != optarg) {
		// 		delete_file = optarg;
		// 	}
		// 	break;
		case 'u':
			if (NULL != optarg) {
				upload_file = optarg;
			}
			break;
		}
	}
	

    if (1 == query_files_flag) {
        ret = QueryFiles(&ser_addr, storage_dir);
        if (ret < 0) {
            log_error("Query failed");
            return -1;
        }
        return 0;
    }
	if (NULL != upload_file) {
        ret = UploadFile(&ser_addr, upload_file);
        if (0 != ret) {
            log_error("Upload failed.");
            return -1;
        }
		return 0;
	}
	if (NULL != download_file) {
        ret = DownloadFile(&ser_addr, download_file, NULL);
        if (0 != ret) {
            log_error("Download failed.");
            return -1;
        }
		return 0;
	}
	// if (NULL != delete_file) {
	// 	printf("delete_file: %s\n", delete_file);
	// 	return 0;
	// }
    return 0;
}

void helpInfo(char opt, const char* version){
	if ('v' == opt) {
		printf("Hi-Hik version %s\n", version);
		return;
	}
	printf("Usage: file-client [OPTION]... [FILE]...\n");
	printf("A file transfer tool, support upload and download.\n");
	printf("\n");
	printf("Mandatory arguments to long options are mandatory for short options too.\n");
	switch (opt) {
	case 's':
		printf("\t-s, --ser_addr IP:Port\t\t\tSet the server address to IP :port. The default address is 127.0.0.1:8080.\n");
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
		printf("\t-l, --ls [FILE-DIRECTORY]\t\t\tView remote server downloadable files.\n");
		printf("\t-d, --download FILE\t\t\tDownload the file.\n");
		// printf("\t-D, --delete FILE\t\t\tDelete the file.\n");
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
    int ret = -1;
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
        log_error("Connect the server failed.");
        return -1;
    }

    // 查询
    ret = Send(client_fd, &request_buf, sizeof(RequestBuf));
    if (0 != ret) {
        log_error("Query buf send failed.");
        return -1;
    }
    if (QUERY_MODE_LIST_WITH_PATH == request_buf.cmd) {
        ret = Send(client_fd, send_dir, sizeof(char) * (dir_len + 1));
        if (0 != ret) {
            log_error("Directory name send failed.");
            return -1;
        }
    }

    // 接收回复报文
    ret = Receive(client_fd, &reply_buf, sizeof(ReplyBuf));
    if (0 != ret) {
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
    ret = Receive(client_fd, &file_num, sizeof(uint32_t));
    if (0 != ret) {
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

        ret = ReadLine(client_fd, files[i], MAX_FILE_NAME_LENGTH);
        
        if (ret < 0) {
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

uint64_t QueryFileSize(struct sockaddr_in* ser_addr, const char* file){
    RequestBuf request_buf;
    ReplyBuf reply_buf;
    int ret = -1;
    int fd = -1;
    uint64_t file_size = 0;

    if (NULL == file) {
        log_error("Fail to request the file size. Cause by the file is NULL.");
        return 0;
    }

    fd = ConnectServer(ser_addr);
    if (fd < 0) {
        log_error("Connect the server failed.");
        return 0;
    }

    request_buf.type = NET_PROTOCOL_QUERY;
    request_buf.cmd = QUERY_MODE_FILE_SIZE;

    ret = Send(fd, &request_buf, sizeof(RequestBuf));
    if (0 != ret) {
        log_error("Query buf send failed.");
        close(fd);
        return 0;
    }
    ret = WriteLine(fd, file, MAX_FILE_NAME_LENGTH);
    if (ret < 0) {
        log_error("Can't send the query file name.");
        close(fd);
        return 0;
    }
    
    ret = Receive(fd, &reply_buf, sizeof(ReplyBuf));
    if (0 != ret) {
        log_error("Receive the query reply buf failed.");
        close(fd);
        return 0;
    }
    if (NET_PROTOCOL_QUERY != reply_buf.type) {
        log_error("Receive reply buf's type is incorrect.");
        close(fd);
        return 0;
    }
    if (REQUEST_OK != reply_buf.status_code) {
        log_error("Occur an error. Reply status code: %d", reply_buf.status_code);
        close(fd);
        return 0;
    }
    
    ret = Receive(fd, &file_size, sizeof(uint64_t));
    if (0 != ret) {
        log_error("Receive the query reply buf failed.");
        close(fd);
        return 0;
    }
    close(fd);
    return file_size;
}

int DownloadFile(struct sockaddr_in* ser_addr, char* file, char* storage_file){
    // 声明
    char* path = (NULL == storage_file) ? HERE : storage_file; // 为空则存放在同级目录下
    char download_config_file[MAX_FILE_NAME_LENGTH] = {'\0'};
    char start_path[MAX_FILE_NAME_LENGTH] = {'\0'};
    char block_file[MAX_FILE_NAME_LENGTH] = {'\0'};
    int ret = -1;
    DownloadBlockInfo block_info;
    DownloadFileInfo file_info;
    int exist_config_file_flag = false;
    FILE* write_fd = NULL;

    // 初始化
    memset(&block_info, 0, sizeof(DownloadBlockInfo));
    memset(&file_info, 0, sizeof(DownloadFileInfo));

    // 条件检查
    if (NULL == file) {
        log_error("Transmit the file is NULL.");
        return 1;
    }

    // 切换到下载文件存放路径
    ret = ChangeDir(path, start_path, MAX_FILE_NAME_LENGTH);
    if (0 != ret) {
        return 1;
    }
    
    ConfigNameGen(download_config_file, file, MAX_FILE_NAME_LENGTH);
    exist_config_file_flag = ExistFile(download_config_file);


    // 初始化下载
    if (false == exist_config_file_flag) {
        file_info.block_num = CLIENT_DOWNLOADS_THREADS_NUM;
        file_info.file_size = QueryFileSize(ser_addr, file);
        if (0 == file_info.file_size) {
            log_error("Occur an error in getting file size.");
            ChangeDir(start_path, NULL, MAX_FILE_NAME_LENGTH);
            return 1;
        }
        ret = InitConfig(download_config_file, &file_info);
        if (1 == ret) {
            log_error("Set config failed.");
            ChangeDir(start_path, NULL, MAX_FILE_NAME_LENGTH);
            return 1;
        }
    }
    else {
        ret = ReadConfigFileInfo(download_config_file, &file_info);
        if (0 != ret) {
            return 1;
        }
    }
    // 下载
    ClientThreadArg args[CLIENT_DOWNLOADS_THREADS_NUM];
    for (size_t i=0; i<file_info.block_num; i++) {
        args[i].file = file;
        args[i].addr = ser_addr;
        args[i].index = i;
        pthread_create(&args[i].tid, NULL, dowloadFile, &args[i]);
    }
    for (size_t i=0; i<file_info.block_num; i++) {
        pthread_join(args[i].tid, NULL);
    }
    if (1 == CheckDownloadStatus(download_config_file)) {
        // 合并数据包
        write_fd = fopen(file, "wb");
        progressbar* bar = progressbar_new("Merging the file", file_info.block_num);
        signal(SIGINT, SIG_IGN);
        for (size_t i=0; i<file_info.block_num; i++) {
            progressbar_inc(bar);
            // 合并小数据包
            BlockNameGen(block_file, file, i, MAX_FILE_NAME_LENGTH);

            MergeFile(write_fd, block_file);
            remove(block_file);
        }
        signal(SIGINT, SIG_DFL);
        progressbar_finish(bar);
        fclose(write_fd);
        remove(download_config_file);
    }
    ChangeDir(start_path, NULL, MAX_FILE_NAME_LENGTH);
    // 回到初始路径
    return 0;
}

void* dowloadFile(void* arg){
    ClientThreadArg* client_arg = (ClientThreadArg*)arg;
    DownloadBlockInfo block_info;
    RequestBuf request_buf;
    ReplyBuf reply_buf;
    uint64_t start_head = 0;
    uint64_t once_recv_byte = 0;
    char config_file[MAX_FILE_NAME_LENGTH];
    int ret = -1;
    int fd = -1;
    char* recv_buf = NULL;
    char bar_label[MAX_LABEL_LEN];

    // 条件检查
    if (NULL == client_arg) {
        log_error("Argument is NULL.");
        return NULL;
    }
    if (NULL == client_arg->addr) {
        log_error("Address is NULL.");
        return NULL;
    }
    if (NULL == client_arg->file) {
        log_error("Config file is NULL.");
        return NULL;
    }
    ConfigNameGen(config_file, client_arg->file, MAX_FILE_NAME_LENGTH);
    if (0 == ExistFile(config_file)) {
        log_error("Config file doesn't exist.");
        return NULL;
    }

    // 获取下载数据
    block_info.index = client_arg->index;
    ret = ReadConfigDownloadInfo(config_file, &block_info);
    if (1 == ret) {
        return NULL;
    }
    snprintf(bar_label, MAX_LABEL_LEN, "Threads %d", block_info.index);
    // 连接服务器
    fd = ConnectServer(client_arg->addr);
    if (fd < 0) {
        return NULL;
    }
    // 发送下载报文
    request_buf.type = NET_PROTOCOL_GET;
    request_buf.cmd = GET_MODE_DOWNLOAD;
    ret = Send(fd, &request_buf, sizeof(RequestBuf));
    if (0 != ret) {
        log_error("Query buf send failed.");
        close(fd);
        return NULL;
    }
    // 发送下载文件名 TODO: 需要增加可选的下载路径选项
    ret = WriteLine(fd, client_arg->file, MAX_FILE_NAME_LENGTH);
    if (ret <= 0) {
        return NULL;
    }
    // 发送下载信息
    ret = Send(fd, &block_info, sizeof(DownloadBlockInfo));

    // 接收回复报文
    ret = Receive(fd, &reply_buf, sizeof(ReplyBuf));
    if (0 != ret) {
        log_error("Receive the query reply buf failed.");
        close(fd);
        return NULL;
    }
    if (NET_PROTOCOL_GET != reply_buf.type) {
        log_error("Receive reply buf's type is incorrect.");
        close(fd);
        return NULL;
    }
    if (REQUEST_OK != reply_buf.status_code) {
        log_error("Occur an error. Reply status code: %d", reply_buf.status_code);
        close(fd);
        return NULL;
    }
    // 接收数据
    progressbar* bar = progressbar_new(bar_label, block_info.len);
    progressbar_inc(bar);
    recv_buf = (char*)malloc(sizeof(char) * SINGLE_TRANSMISSION_LEN);
    start_head = block_info.head;
    while (0 != block_info.len) {
        recv_buf = memset(recv_buf, 0, sizeof(char) * SINGLE_TRANSMISSION_LEN);
        if (block_info.len < SINGLE_TRANSMISSION_LEN) {
            once_recv_byte = block_info.len;
        }
        else {
            once_recv_byte = SINGLE_TRANSMISSION_LEN;
        }   
        ret = Receive(fd, recv_buf, once_recv_byte);
        if (0 != ret) {
            log_error("Occur an error during downloading.");
            close(fd);
            progressbar_free(bar);
            free(recv_buf);
            return NULL;
        }
        signal(SIGINT, SIG_IGN);
        ret = WriteData(client_arg->file, client_arg->index, recv_buf, once_recv_byte);
        if (0 != ret) {
            log_error("Fail to write the download data into disk.");
            progressbar_free(bar);
            free(recv_buf);
            close(fd);
            signal(SIGINT, SIG_DFL);
            return NULL;
        }

        block_info.head = block_info.head + once_recv_byte;
        block_info.len = block_info.len - once_recv_byte;
        ret = WriteConfigDownloadInfo(config_file, &block_info);
        signal(SIGINT, SIG_DFL);
        if (0 != ret) {
            log_error("Fail to write the download config into disk.");
            progressbar_free(bar);
            free(recv_buf);
            close(fd);
            return NULL;
        }
        progressbar_update(bar, block_info.head - start_head);
    }
    close(fd);
    progressbar_free(bar);
    free(recv_buf);
    printf("%s downloading successful.\n", bar_label);
    return NULL;
}

int UploadFile(struct sockaddr_in* ser_addr, char* file){
    // 声明
    DownloadFileInfo file_info;
    DownloadBlockInfo block_info;
    int ret = -1;
    char upload_config_file[MAX_FILE_NAME_LENGTH] = {0};
    char* base_file = NULL;
    bool exist_config_file_flag = false;

    // 初始化
    memset(&block_info, 0, sizeof(DownloadBlockInfo));
    memset(&file_info, 0, sizeof(DownloadFileInfo));

    // 条件检查
    if (NULL == file) {
        log_error("Transmit the file is NULL.");
        return 1;
    }
    if (0 == ExistFile(file)) {
        log_error("The file doesn't exist in the path:%s.", file);
        return 1;
    }

    base_file = basename(file);
    ConfigNameGen(upload_config_file, base_file, MAX_FILE_NAME_LENGTH);
    exist_config_file_flag = ExistFile(upload_config_file);


    // 初始化上传
    if (false == exist_config_file_flag) {
        file_info.block_num = CLIENT_UPLOADS_THREADS_NUM;
        file_info.file_size = GetFileSize(file);
        if (0 == file_info.file_size) {
            log_error("Occur an error in getting file size.");
            return 1;
        }
        ret = InitConfig(upload_config_file, &file_info);
        if (1 == ret) {
            log_error("Set config failed.");
            return 1;
        }
    }
    else {
        ret = ReadConfigFileInfo(upload_config_file, &file_info);
        if (0 != ret) {
            return 1;
        }
    }
    // 上传
    ClientThreadArg args[CLIENT_DOWNLOADS_THREADS_NUM];
    for (size_t i=0; i<file_info.block_num; i++) {
        args[i].file = file;
        args[i].addr = ser_addr;
        args[i].index = i;
        pthread_create(&args[i].tid, NULL, uploadFile, &args[i]);
    }
    for (size_t i=0; i<file_info.block_num; i++) {
        pthread_join(args[i].tid, NULL);
    }
    if (1 == CheckDownloadStatus(upload_config_file)) {
        // 合并数据包
        ret = CommandServerMergeFile(ser_addr, base_file);
        if (0 != ret) {
            log_error("Server merge the file failed.");
            return 1;
        }
        remove(upload_config_file);
    }
    return 0;
}

void* uploadFile(void* arg){
    ClientThreadArg* client_arg = (ClientThreadArg*)arg;
    DownloadBlockInfo block_info;
    RequestBuf request_buf;
    ReplyBuf reply_buf;
    uint64_t start_head = 0;
    uint64_t once_send_byte = 0;
    FILE* read_fd;
    char config_file[MAX_FILE_NAME_LENGTH] = {0};
    char* base_name;
    int ret = -1;
    int fd = -1;
    char* send_buf = NULL;
    char bar_label[MAX_LABEL_LEN] = {0};

    // 条件检查
    if (NULL == client_arg) {
        log_error("Argument is NULL.");
        return NULL;
    }
    if (NULL == client_arg->addr) {
        log_error("Address is NULL.");
        return NULL;
    }
    if (NULL == client_arg->file) {
        log_error("File is NULL.");
        return NULL;
    }
   
    base_name = basename(client_arg->file);
    ConfigNameGen(config_file, base_name, MAX_FILE_NAME_LENGTH);
    if (0 == ExistFile(config_file)) {
        log_error("Config file doesn't exist.");
        return NULL;
    }

    // 获取上传数据
    block_info.index = client_arg->index;
    ret = ReadConfigDownloadInfo(config_file, &block_info);
    if (1 == ret) {
        return NULL;
    }
    snprintf(bar_label, MAX_LABEL_LEN, "Threads %d", block_info.index);
    // 连接服务器
    fd = ConnectServer(client_arg->addr);
    if (fd < 0) {
        return NULL;
    }
    // 发送上传报文
    request_buf.type = NET_PROTOCOL_POST;
    request_buf.cmd = POST_MODE_UPLOAD;
    ret = Send(fd, &request_buf, sizeof(RequestBuf));
    if (0 != ret) {
        log_error("Query buf send failed.");
        close(fd);
        return NULL;
    }
    // 发送上传文件名 TODO: 需要增加可选的下载路径选项
    ret = WriteLine(fd, base_name, MAX_FILE_NAME_LENGTH);
    if (ret <= 0) {
        return NULL;
    }
    // 发送上传信息
    ret = Send(fd, &block_info, sizeof(DownloadBlockInfo));

    // 接收回复报文
    ret = Receive(fd, &reply_buf, sizeof(ReplyBuf));
    if (0 != ret) {
        log_error("Receive the query reply buf failed.");
        close(fd);
        return NULL;
    }
    if (NET_PROTOCOL_POST != reply_buf.type) {
        log_error("Receive reply buf's type is incorrect.");
        close(fd);
        return NULL;
    }
    if (REQUEST_OK != reply_buf.status_code) {
        log_error("Occur an error. Reply status code: %d", reply_buf.status_code);
        close(fd);
        return NULL;
    }
    // 上传数据
    progressbar* bar = progressbar_new(bar_label, block_info.len);
    progressbar_inc(bar);
    read_fd = ReadFile(client_arg->file);
    send_buf = (char*)malloc(sizeof(char) * SINGLE_TRANSMISSION_LEN);
    start_head = block_info.head;
    while (0 != block_info.len) {
        send_buf = memset(send_buf, 0, sizeof(char) * SINGLE_TRANSMISSION_LEN);
        if (block_info.len < SINGLE_TRANSMISSION_LEN) {
            once_send_byte = block_info.len;
        }
        else {
            once_send_byte = SINGLE_TRANSMISSION_LEN;
        }   
        ret = fseeko(read_fd, block_info.head, SEEK_SET);
        if (ret < 0) {
            log_error("Can't read the data.");
            CloseFile(read_fd);
            close(fd);
            free(send_buf);
            progressbar_free(bar);
            return NULL; 
        }
        ret = fread(send_buf, 1, once_send_byte, read_fd);
        if (once_send_byte != ret) {
            log_error("Can't read the data.");
            free(send_buf);
            CloseFile(read_fd);
            close(fd);
            progressbar_free(bar);
            return NULL;
        }
        signal(SIGINT, SIG_IGN);
        ret = Send(fd, send_buf, once_send_byte);
        if (0 != ret) {
            log_error("Occur an error during uploading.");
            CloseFile(read_fd);
            close(fd);
            progressbar_free(bar);
            free(send_buf);
            return NULL;
        }
        block_info.head = block_info.head + once_send_byte;
        block_info.len = block_info.len - once_send_byte;
        ret = WriteConfigDownloadInfo(config_file, &block_info);
        signal(SIGINT, SIG_DFL);
        if (0 != ret) {
            log_error("Fail to write the download config into disk.");
            progressbar_free(bar);
            free(send_buf);
            CloseFile(read_fd);
            close(fd);
            return NULL;
        }
        progressbar_update(bar, block_info.head - start_head);
    }
    close(fd);
    CloseFile(read_fd);
    progressbar_free(bar);
    free(send_buf);
    printf("\n%s uploading successful.\n", bar_label);
    return NULL;
}

int CommandServerMergeFile(struct sockaddr_in* ser_addr, char* file){
    RequestBuf request_buf;
    ReplyBuf reply_buf;
    DownloadFileInfo file_info;
    char upload_config_file[MAX_FILE_NAME_LENGTH] = {'\0'};
    int ret = -1;
    int fd = -1;

    if (NULL == file) {
        log_error("File is NULL.");
        return 1;
    }
    // 读取同级目录下配置文件
    ConfigNameGen(upload_config_file, file, MAX_FILE_NAME_LENGTH);
    if (0 == ExistFile(upload_config_file)) {
        log_error("Config file doesn't exist.");
        return 1;
    }
    ret = ReadConfigFileInfo(upload_config_file, &file_info);
    if (0 != ret) {
        log_error("Read file info failed.");
        return 1;
    }
    // 连接服务器
    fd = ConnectServer(ser_addr);
    if (fd < 0) {
        log_error("Connect the server failed.");
        return 1;
    }
    // 发送命令报文
    request_buf.type = NET_PROTOCOL_COMMAND;
    request_buf.cmd = COMMAND_MODE_MERGE_FILE;
    ret = Send(fd, &request_buf, sizeof(RequestBuf));
    if (0 != ret) {
        log_error("Command buf send failed.");
        close(fd);
        return 1;
    }
    // 发送合并文件名
    ret = WriteLine(fd, file, MAX_FILE_NAME_LENGTH);
    if (ret < 0) {
        log_error("Can't send the command buf.");
        close(fd);
        return 1;
    }
    // 发送合并文件信息
    ret = Send(fd, &file_info, sizeof(DownloadFileInfo));
    if (0 != ret) {
        log_error("Send file info failed");
        close(fd);
        return 1;
    }
    // 接收结果
    ret = Receive(fd, &reply_buf, sizeof(ReplyBuf));
    if (0 != ret) {
        log_error("Receive the command reply buf failed.");
        close(fd);
        return 1;
    }
    if (NET_PROTOCOL_COMMAND != reply_buf.type) {
        log_error("Receive reply buf's type is incorrect.");
        close(fd);
        return 1;
    }
    if (REQUEST_OK != reply_buf.status_code) {
        log_error("Occur an error. Reply status code: %d", reply_buf.status_code);
        close(fd);
        return 1;
    }
    
    close(fd);
    return 0;
}