/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-30 15:05:20
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-09-03 10:59:27
 * @FilePath: /tcp-server/client/client.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%Av
 */



#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>

#ifndef CLI_VERSION
#define CLI_VERSION "2.0.0"
#endif

FILE* client_log;

typedef struct ClientThreadArg{
    struct sockaddr_in* addr;
    char* storage_file;
    char* file;
    uint8_t index;
    pthread_t tid;
}ClientThreadArg;

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

/**
 * @description: 启动客户端
 * @param {int} argc 参数个数
 * @param {char*} argv 参数数组
 * @return {*}
 */
int StartClient(int argc, char* argv[]);

/**
 * @description: 打印帮助信息
 * @param {char} opt 帮助选项，'a'打印所有
 * @param {char*} version 版本号
 * @return {*}
 */
void helpInfo(char opt, const char* version);

/**
 * @description: 查询服务器storage_dir目录下的文件，如果为空，则查询服务器默认目录下文件，最后打印。
 * @param {sockaddr_in*} ser_addr 服务器地址
 * @param {char*} storage_dir 目录路径
 * @return {int} 文件数目 -1 表示出错
 */
int QueryFiles(struct sockaddr_in* ser_addr, const char* storage_dir);

uint64_t QueryFileSize(struct sockaddr_in* ser_addr, const char* file);

/**
 * @description: 下载服务器文件
 * @param {sockaddr_in*} ser_addr 服务器地址
 * @param {char*} file 文件名
 * @param {char*} storage_file 文件存放路径
 * @return {*} 0 成功 1 失败 -1 中断
 */
int DownloadFile(struct sockaddr_in* ser_addr, char* file, char* storage_file);

/**
 * @description: 根据参数下载对应服务器对应内容
 * @param {void*} (ClientThreadArg*)arg 下载所需的一些参数
 * @return {*}
 */
void* dowloadFile(void* arg);

int UploadFile();
