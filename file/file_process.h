/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-27 10:04:22
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-09-03 11:02:12
 * @FilePath: /tcp-server/file/file_process.h
 * @Description: 对文件打开，分割，合并处理
 */
#ifndef FILE_H
#define FILE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define MAX_DIR_FILES_NUM 255
#define MAX_FILE_NAME_LENGTH 255

#ifdef _POSIX_PATH_MAX
#define PATHNAME_MAX _POSIX_PATH_MAX
#else
#define PATHNAME_MAX 1000
#endif

#define HERE "./"

typedef struct DownloadBlockInfo{
    uint8_t index;
    uint64_t head;
    uint64_t len;
}DownloadBlockInfo;

typedef struct DownloadFileInfo{
    uint64_t file_size;
    uint8_t block_num;
}DownloadFileInfo;

/**
 * @description: 以rb模式打开文件，同时对文件进行检查是否大于4GB。
 * @param {char*} file_path 欲打开的文件路径
 * @return {FILE*} fd 如果为NULL说明打开失败
 */
FILE* ReadFile(const char* file_path);

/**
 * @description: 以ab模式打开文件
 * @param {char*} file_path 欲打开的文件路径
 * @return {FILE*} fd 如果为NULL说明打开失败
 */
FILE* WriteFile(const char* file_path);

/**
 * @description: 将配置写入二进制配置文件，如果不存在则创建
 * @param {char*} file 配置文件名
 * @param {DownloadFileInfo*} file_info 配置文件参数
 * @return {int} 0 成功 1 失败
 */
int InitConfig(const char* file, DownloadFileInfo* file_info);

/**
 * @description: 根据block_info中的index在配置文件file中读取下载信息
 * @param {char*} file 配置文件的文件名
 * @param {DownloadBlockInfo*} block_info 要获取的下载块信息
 * @return {int} 0 成功 1 失败
 */
int ReadConfigDownloadInfo(const char* file, DownloadBlockInfo* block_info);

/**
 * @description: 写入配置文件对应的块
 * @param {char*} file 配置文件文件名
 * @param {DownloadBlockInfo*} block_info 待写入的块
 * @return {int} 0 成功 1 失败
 */
int WriteConfigDownloadInfo(const char* file, DownloadBlockInfo* block_info);

/**
 * @description: 检查下载是否完成
 * @param {char*} file 配置文件文件名
 * @return {int} 0 未完成 1 完成 -1 出错
 */
int CheckDownloadStatus(const char* file);

/**
 * @description: 关闭文件
 * @param {FILE*} fd
 * @return {int} 0 成功关闭 1 出现错误
 */
int CloseFile(FILE* fd);

/**
 * @description: 获取文件的大小，单位Byte
 * @param {char*} file_path 文件路径
 * @return {uint64_t} 文件大小
 */
uint64_t GetFileSize(const char* file_path);

/**
 * @description: 读取第n块数据放入buf中
 * @param {FILE*} fd 文件描述符
 * @param {uint8_t*} buf 存放数据的数组
 * @param {uint32_t} length 数组长度
 * @param {uint32_t} index 读取数据的序号
 * @return {uint32_t} 已经读取的字节数，0代表失败
 */
uint32_t ReadData(FILE* fd,uint8_t* buf,const uint32_t length, const uint32_t index);

/**
 * @description: 将二进制字节写入一个文件，一般文件名为：.文件前缀{md5校验值}-序号
 * @param {char*} prefix 文件前缀
 * @param {uint32_t} index 报文序号
 * @param {uint8_t*} buf 二进制数据数组
 * @param {uint32_t} length 数组长度
 * @return {int} 0 成功 1 失败
 */
int WriteData(const char* prefix, const uint8_t index, const char* buf, const uint64_t length);

/**
 * @description: 打开路径为path的小文件，写入fd末尾。
 * @param {FILE*} fd 待写入的文件描述符
 * @param {char*} path 待合并的小文件的路径
 * @return {int} 0 成功 1 失败 
 */
int MergeFile(FILE* fd, const char* path);

/**
 * @description: 判断文件是否存在
 * @param {char*} path 文件路径
 * @return {int} 0 不存在 1 存在
 */
int ExistFile(const char* path);

/**
 * @description: 根据每个分割块大小和总文件大小计算得到分割块总数
 * @param {uint64_t} file_size 总文件大小
 * @param {uint32_t} block_size 分割块大小
 * @return {uint32_t} block_num 分割块总数
 */
uint32_t GetBlockNum(uint64_t file_size, uint32_t block_size);

/**
 * @description: 根据总文件大小和分割块数目得到分割块大小
 * @param {uint64_t} file_size 总文件大小
 * @param {uint8_t} block_num 分割块总数
 * @return {uint64_t} block_size 分割块大小
 */
uint64_t GetBlockSize(uint64_t file_size, uint8_t block_num);

/**
 * @description: 创建一个同级下的目录，如果目录已存在则不创建
 * @param {char*} path 文件路径
 * @return {*} 0 失败 1 成功
 */
int CreateFile(const char* path);

/**
 * @description: 显示目录内文件名，存放进files二级数组内
 * @param {char*} path 目录路径
 * @param {char*} files 存放文件名的字符串数组，需要调用者释放
 * @return {int} 文件数目 -1表示路径不存在或者错误
 */
int ShowDirFiles(const char* path, char* files[]);

/**
 * @description: 释放ShowDirFiles中产生的files二级动态数组
 * @param {char**} files
 * @param {int} num
 * @return {int} 0 成功 1 失败
 */
int FreeFiles(char** files, int num);

/**
 * @description: 移动到目标路径，并返回移动前的路径
 * @param {char*} target_dir 目标路径
 * @param {char*} current_dir 当前路径
 * @param {int} max_dir_len 字符串的最大长度
 * @return {*} 0 成功 1 失败
 */
int ChangeDir(const char* target_dir, char* current_dir, int max_dir_len);


#endif