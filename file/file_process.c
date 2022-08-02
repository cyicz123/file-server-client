/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-27 10:04:33
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-02 17:16:25
 * @FilePath: /tcp-server/file/file_process.c
 * @Description: 对文件打开，分割，合并处理
 */ 
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include "../str/strUtils.h"
#include "file_process.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILE_SIZE (off_t)(4294967295) //最大4GB
#define MERGE_FILE_READ_BUF_SIZE 1024

/**
 * @description: 以rb模式打开文件，同时对文件进行检查是否大于4GB。
 * @param {char*} file_path 欲打开的文件路径
 * @return {FILE*} fd 如果为NULL说明打开失败
 */
FILE* ReadFile(const char* file_path)
{
    off_t size=GetFileSize(file_path);
    if (size == 0) 
    {
        printf("Can't open the file.\n");
        return NULL;
    }

    if(size < 0 || size > MAX_FILE_SIZE)
    {
        printf("The file is larger than 4GB.\n");
        return NULL;
    }
    
    FILE* fd=NULL;
    fd = fopen(file_path, "rb");
    if (fd == NULL)
    {
        printf("Can't open the file.\n");
        return NULL;
    }

    return fd;
    
}

/**
 * @description: 以wb模式打开文件
 * @param {char*} file_path 欲打开的文件路径
 * @return {FILE*} fd 如果为NULL说明打开失败
 */
FILE* WriteFile(const char* file_path)
{
    FILE* fd = NULL;
    fd = fopen(file_path, "wb");
    if (fd == NULL)
    {
        printf("Can't open the file.\n");
        return NULL;
    }
    return fd;
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
        printf("Can't close the file.");
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
        printf("Can't open the file.\n");
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
        printf("Set the fd point failed.\n");
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
int WriteData(const char* prefix, const uint32_t index, const uint8_t* buf, const uint32_t length)
{
    int index_s_length = GetIntDigit(index) + 1;
    char* index_s = (char*)malloc(index_s_length * sizeof(char));
    Uint32ToStr(index_s, index);
    
    size_t path_length = (strlen(prefix) + strlen(index_s) + 2 + 1 ) * sizeof(char); //2为.%s-%s中的固定字符数量
    char* path = (char*)malloc(path_length * sizeof(char));

    snprintf(path, path_length * sizeof(char), ".%s-%s", prefix, index_s);
    free(index_s);


    FILE* file = fopen(path, "wb");

    if(file == NULL)
    {
        printf("Can't write %s\n", path);
        free(path);
        return 1;
    }

    size_t count = 0;
    count=fwrite(buf, 1, length, file);
    if(count != length)
    {
        printf("Failed to write data to %s. Expected %u bytes of data to be written, but only %lu was written.\n", path, length, count);
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
            printf("The read and write operations are inconsistent.\n");
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