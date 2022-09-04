/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-27 10:04:33
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-09-04 16:22:15
 * @FilePath: /tcp-server/file/file_process.c
 * @Description: 对文件打开，分割，合并处理
 */ 
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include "../str/strUtils.h"
#include "../log/log.h"
#include "file_process.h"

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include <errno.h>

#define MERGE_FILE_READ_BUF_SIZE 1024


/**
 * @description: 以rb模式打开文件，同时对文件进行检查是否大于4GB。
 * @param {char*} file_path 欲打开的文件路径
 * @return {FILE*} fd 如果为NULL说明打开失败
 */
FILE* ReadFile(const char* file_path)
{
    FILE* fd=NULL;
    fd = fopen(file_path, "rb");
    if (fd == NULL)
    {
        log_error("Can't open the file.");
        return NULL;
    }

    return fd;
    
}

/**
 * @description: 以ab模式打开文件
 * @param {char*} file_path 欲打开的文件路径
 * @return {FILE*} fd 如果为NULL说明打开失败
 */
FILE* WriteFile(const char* file_path)
{
    FILE* fd = NULL;
    fd = fopen(file_path, "ab");
    if (fd == NULL)
    {
        log_error("Can't open the file.");
        return NULL;
    }
    return fd;
}

int InitConfig(const char* file, DownloadFileInfo* file_info){
    FILE* fd = WriteFile(file);
    int ret = -1;
    uint64_t block_len = 0;
    const int fixed_head_len = sizeof(uint64_t) + sizeof(uint8_t);
    DownloadBlockInfo init_block_info;

    if (fd == NULL)
    {
        log_error("Can't open the file.");
        return 1;
    }
    if (NULL == file_info) {
        log_error("File info is NULL");
        fclose(fd);
        return 1;
    }
    if (0 == file_info->file_size || 0 == file_info->block_num) {
        log_error("File size or block number is zero.");
        fclose(fd);
        return 1;
    }
    ret = fseek(fd, 0, SEEK_SET);
    if (ret < 0) {
        log_error("Can't set the file size.");
        fclose(fd);
        return 1;
    }
    ret = fwrite(&(file_info->file_size), 1, sizeof(uint64_t), fd);
    if (sizeof(uint64_t) != ret) {
        log_error("Can't set the file size.");
        fclose(fd);

        return 1;
    }
    ret = fseek(fd, sizeof(uint64_t), SEEK_SET);
    if (ret < 0) {
        log_error("Can't set the block num.");
        fclose(fd);
        return 1;
    }
    ret = fwrite(&(file_info->block_num), 1, sizeof(uint8_t), fd);
    if (sizeof(uint8_t) != ret) {
        log_error("Can't set the block num.");
        fclose(fd);

        return 1;
    }
    ret = fseek(fd, fixed_head_len, SEEK_SET);
    if (ret < 0) {
        log_error("Can't init the block info.");
        fclose(fd);
        return 1;
    }
    // 根据block_num初始化后面的block_info
    block_len = GetBlockSize(file_info->file_size, file_info->block_num);
    for (size_t i = 0; i<file_info->block_num; i++) {
        init_block_info.index = i;
        init_block_info.head = i * block_len;
        if ( i < file_info->block_num - 1){
            init_block_info.len = block_len;
        }
        else {
            init_block_info.len = file_info->file_size - init_block_info.head;
        }
        ret = fwrite(&(init_block_info), 1, sizeof(DownloadBlockInfo), fd);
        if (sizeof(DownloadBlockInfo) != ret) {
            log_error("Can't set the file size.");
            fclose(fd);
            return 1;
        }
    }
    
    fclose(fd);
    return 0;
}

int ReadConfigFileInfo(const char* file, DownloadFileInfo* file_info){
    FILE* fd;
    int ret = -1;

    if (NULL == file) {
        log_error("Config file is NULL.");
        return 1;
    }
    if (NULL == file_info) {
        log_error("Config file info is NULL.");
        return 1;
    }
    if (0 == ExistFile(file)) {
        log_error("Config file doesn't exist.");
        return 1;
    }

    fd = ReadFile(file);
    if (NULL == fd) {
        return 1;
    }
    ret = fseek(fd, 0, SEEK_SET);
    if (ret < 0) {
        log_error("Can't read the file size.");
        fclose(fd);
        return 1;
    }
    ret = fread(&(file_info->file_size), 1, sizeof(uint64_t), fd);
    if (sizeof(uint64_t) != ret) {
        log_error("Can't read the file size.");
        fclose(fd);
        return 1;
    }
    ret = fseek(fd, sizeof(uint64_t), SEEK_SET);
    if (ret < 0) {
        log_error("Can't read the block num.");
        fclose(fd);
        return 1;
    }
    ret = fread(&(file_info->block_num), 1, sizeof(uint8_t), fd);
    if (sizeof(uint8_t) != ret) {
        log_error("Can't read the block num.");
        fclose(fd);
        return 1;
    }
    return 0;
}

int ReadConfigDownloadInfo(const char* file, DownloadBlockInfo* block_info){
    FILE* fd;
    int ret = -1;
    int block_info_off = 0;
    const int fixed_head_len = sizeof(uint64_t) + sizeof(uint8_t);
    
    if (NULL == file) {
        log_error("Config file is NULL.");
        return 1;
    }
    if (NULL == block_info) {
        log_error("Config info is NULL.");
        return 1;
    }
    if (0 == ExistFile(file)) {
        log_error("Config file doesn't exist.");
        return 1;
    }

    fd = ReadFile(file);
    if (NULL == fd) {
        return 1;
    }

    block_info_off = fixed_head_len + sizeof(DownloadBlockInfo) * block_info->index;
    ret = fseek(fd, block_info_off, SEEK_SET);
    if (ret < 0) {
        log_error("Can't read the block info.");
        fclose(fd);
        return 1;
    }
    ret = fread(block_info, 1, sizeof(DownloadBlockInfo), fd);
    if (sizeof(DownloadBlockInfo) != ret) {
        log_error("Can't read the block info. Caused by %s.", strerror(errno));
        fclose(fd);
        return 1;
    }
    fclose(fd);
    return 0;
}

int WriteConfigDownloadInfo(const char* file, DownloadBlockInfo* block_info){
    FILE* fd;
    int ret = -1;
    int block_info_off = 0;
    const int fixed_head_len = sizeof(uint64_t) + sizeof(uint8_t);
    
    if (NULL == file) {
        log_error("Config file is NULL.");
        return 1;
    }
    if (NULL == block_info) {
        log_error("Config info is NULL.");
        return 1;
    }
    if (0 == ExistFile(file)) {
        log_error("Config file doesn't exist.");
        return 1;
    }

    fd = fopen(file, "rb+");
    if (NULL == fd) {
        return 1;
    }
    
    block_info_off = fixed_head_len + sizeof(DownloadBlockInfo) * block_info->index;
    ret = fseek(fd, block_info_off, SEEK_SET);
    if (ret < 0) {
        log_error("Can't write the block info.");
        fclose(fd);
        return 1;
    }
    ret = fwrite(block_info, 1, sizeof(DownloadBlockInfo), fd);
    if (sizeof(DownloadBlockInfo) != ret) {
        log_error("Can't write the block info. Caused by %s.", strerror(errno));
        fclose(fd);
        return 1;
    }
    fclose(fd);
    return 0;
}

int CheckDownloadStatus(const char* file){
    FILE* fd;
    int ret = -1;
    uint8_t block_num = 0;
    int block_info_off = 0;
    const int fixed_head_len = sizeof(uint64_t) + sizeof(uint8_t);
    DownloadBlockInfo block_info;
    int download_complete_flag = 1;

    
    if (NULL == file) {
        log_error("Config file is NULL.");
        return -1;
    }
    
    if (0 == ExistFile(file)) {
        log_error("Config file doesn't exist.");
        return -1;
    }

    fd = ReadFile(file);
    if (NULL == fd) {
        return -1;
    }
    
    ret = fseek(fd, sizeof(uint64_t), SEEK_SET);
    if (ret < 0) {
        log_error("Can't set the block num.");
        fclose(fd);
        return -1;
    }
    ret = fread(&block_num, 1, sizeof(uint8_t), fd);
    if (sizeof(uint8_t) != ret) {
        log_error("Can't set the block num.");
        fclose(fd);
        return -1;
    }
    
    for (size_t i = 0; i<block_num; i++) {
        block_info_off = fixed_head_len + sizeof(DownloadBlockInfo) * i;
        ret = fseek(fd, block_info_off, SEEK_SET);
        if (ret < 0) {
            log_error("Can't read the block info.");
            fclose(fd);
            return -1;
        }
        memset(&block_info, 0, sizeof(DownloadBlockInfo));
        ret = fread(&block_info, 1, sizeof(DownloadBlockInfo), fd);
        if (sizeof(DownloadBlockInfo) != ret) {
            log_error("Can't read the block info.");
            fclose(fd);
            return -1;
        }
        if (0 != block_info.len) {
            download_complete_flag = 0;
        }
    }
    fclose(fd);
    return download_complete_flag;
}

/**
 * @description: 关闭文件
 * @param {FILE*} fd
 * @return {int} 0 成功关闭 1 出现错误
 */
int CloseFile(FILE* fd)
{
    if(fclose(fd) != 0)
    {
        log_error("Can't close the file.");
        return 1;
    }
    else
        return 0;
}

/**
 * @description: 获取文件的大小，单位Byte
 * @param {char*} file_path 文件路径
 * @return {uint64_t} 文件大小
 */
uint64_t GetFileSize(const char* file_path)
{
    struct stat64 buf;
    if (stat64(file_path,&buf)) 
    {
        log_error("Can't open the file.");
        return 0;
    }
    return buf.st_size;
}


/**
 * @description: 读取第n块数据放入buf中
 * @param {FILE*} fd 文件描述符
 * @param {uint8_t*} buf 存放数据的数组
 * @param {uint32_t} length 数组长度
 * @param {uint32_t} index 读取数据的序号
 * @return {uint32_t} 已经读取的字节数,0代表失败
 */
uint32_t ReadData(FILE* fd,uint8_t* buf,const uint32_t length, const uint32_t index)
{
    off_t offset = index*length;  
    if(fseeko(fd, offset, SEEK_SET) == -1)
    {
        log_error("Set the fd point failed.");
        return 0;
    }
    uint32_t read_size=fread(buf, 1,length, fd);

    return read_size;

}

/**
 * @description: 将二进制字节写入一个文件，一般文件名为：.文件前缀{md5校验值}-序号
 * @param {char*} prefix 文件前缀
 * @param {uint32_t} index 报文序号
 * @param {uint8_t*} buf 二进制数据数组
 * @param {uint32_t} length 数组长度
 * @return {int} 0 成功 1 失败
 */
int WriteData(const char* prefix, const uint8_t index, const char* buf, const uint64_t length)
{
    int s_index_length = GetIntDigit(index) + 1;
    char* s_index = (char*)malloc(s_index_length * sizeof(char));
    Uint32ToStr(s_index, s_index_length, index);
    
    size_t path_length = (strlen(prefix) + strlen(s_index) + 2 + 1 ) * sizeof(char); //2为.%s-%s中的固定字符数量
    char* path = (char*)malloc(path_length * sizeof(char));

    snprintf(path, path_length * sizeof(char), ".%s-%s", prefix, s_index);
    free(s_index);

    FILE* file = fopen(path, "ab+");

    if(file == NULL)
    {
        log_error("Can't write %s", path);
        free(path);
        return 1;
    }

    // fseek(file, 0, SEEK_END);
    size_t count = 0;
    count=fwrite(buf, 1, length, file);
    if(count != length)
    {
        log_error("Failed to write data to %s. Expected %u bytes of data to be written, but only %lu was written.", path, length, count);
        free(path);
        fclose(file);
        return 1;
    }
    
    free(path);
    fclose(file);
    return 0;
}
    
/**
 * @description: 打开路径为path的小文件，写入fd末尾。
 * @param {FILE*} fd 待写入的文件描述符
 * @param {char*} path 待合并的小文件的路径
 * @return {int} 0 成功 1 失败 
 */
int MergeFile(FILE* fd, const char* path)
{
    FILE* rd_fd = ReadFile(path);
    if (fd == NULL) 
    {
        return 1;
    }
    int read_buf_size = 0, write_buf_size = 0;
    uint8_t buf[MERGE_FILE_READ_BUF_SIZE];
    while (!feof(rd_fd)) 
    {
        read_buf_size = fread((uint8_t*)buf, 1, MERGE_FILE_READ_BUF_SIZE, rd_fd); 
        write_buf_size = fwrite((uint8_t*)buf, 1, read_buf_size, fd);
        if (write_buf_size != read_buf_size) 
        {
            log_error("The read and write operations are inconsistent.");
            CloseFile(rd_fd);
            return 1;
        } 
    }
    return 0;
}

/**
 * @description: 判断文件是否存在
 * @param {char*} path 文件路径
 * @return {int} 0 不存在 1 存在
 */
int ExistFile(const char* path)
{
    if(!access(path, F_OK))
        return 1;
    return 0;
}

/**
 * @description: 根据每个分割块大小和总文件大小计算得到分割块总数
 * @param {uint64_t} file_size 总文件大小
 * @param {uint32_t} block_size 分割块大小
 * @return {uint32_t} block_num 分割块总数
 */
uint32_t GetBlockNum(uint64_t file_size, uint32_t block_size)
{
    if (0 == block_size) {
        log_error("Block size is zero.");
        return 0;
    }
    uint32_t block_num = 0;
    block_num = ceil((double)file_size/block_size);
    return block_num;
}

uint64_t GetBlockSize(uint64_t file_size, uint8_t block_num){
    uint64_t block_size = 0;
    if (0 == block_num) {
        log_error("Block number is zero.");
        return 0;
    }
    block_size = ceil((double)file_size/block_num);
    return block_size;
}

int CreateFile(const char* path){
    int ret =-1;
    if (ExistFile(path)) {
        return 1;
    }
    else {
        ret = mkdir(path, 0777);
    }
    // 判断创建目录是否成功
    if (0 == ret) {
        return 1;
    }
    else {
        return 0;
    }
}


int ShowDirFiles(const char* path, char* files[]){
    char** all_files;
    struct dirent **entry_list = NULL;
    struct dirent *entry = NULL;
    int count = 0;
    int file_num = 0;
    int ret = -1;

    ret = ExistFile(path);
    if (0 == ret) {
        return -1;
    }

    count = scandir(path, &entry_list, 0, alphasort);
    if (count < 0) {
        return -1;
    }
    all_files = (char**)malloc(sizeof(char*) * count); 
    for (size_t i=0; i<count; i++) {
        entry = entry_list[i];
        if (DT_REG == entry->d_type) {
            all_files[file_num] = (char*)malloc(MAX_FILE_NAME_LENGTH);
            memset(all_files[file_num], '\0', MAX_FILE_NAME_LENGTH);
            strncpy(all_files[file_num], entry->d_name, MAX_FILE_NAME_LENGTH); 
            file_num++;
        }
        free(entry);
    }
    free(entry_list);
    
    // files = (char**)malloc(sizeof(char*) * file_num);
    for (size_t i=0; i<file_num; i++) {
        files[i] = (char*)malloc(MAX_FILE_NAME_LENGTH);
        memset(files[i], '\0', MAX_FILE_NAME_LENGTH);
        strncpy(files[i], all_files[i], MAX_FILE_NAME_LENGTH);
        free(all_files[i]);
    }
    free(all_files);
    return file_num;
}


int FreeFiles(char** files, int num){
    for (size_t i=0; i<num; i++) {
        free(files[i]);
    }
    free(files);
    return 0;
}

int ChangeDir(const char* target_dir, char* current_dir, int max_dir_len){
    if (NULL == target_dir) {
        log_error("Target directory is NULL.");
        return 1;
    }
    if (0 == ExistFile(target_dir)) {
        log_error("Target directory does't exist.");
        return 1;
    }
    if (NULL != current_dir) {
        if (NULL == getcwd(current_dir, max_dir_len)) {
            log_error("Get current path error. Caused by %s.", strerror(errno));
            return 1;
        }
    }
    if (0 > chdir(target_dir)) {
        log_error("Change to target directory failed. Caused by %s.", strerror(errno));
        return 1;
    }
    return 0;
}