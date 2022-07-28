/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-27 10:04:33
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-07-28 15:26:50
 * @FilePath: /tcp-server/file/file_process.c
 * @Description: 对文件打开，分割，合并处理
 */ 
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#include <stdint.h>
#include <sys/types.h>
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include "../str/int2string.h"
#include "file_process.h"
#include <stdio.h>
#include <sys/stat.h>

#define MAX_FILE_SIZE (off_t)(4294967295) //最大4GB

/**
 * @description: 打开文件，同时对文件进行检查是否大于4GB。
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
 * @param {uint64_t} file_size 整个文件的大小，单位Byte
 * @return {uint32_t} 已经读取的字节数,0代表失败
 */
uint32_t ReadData(FILE* fd,uint8_t* buf,const uint32_t length, const uint32_t index, const uint64_t file_size)
{
    off_t offset = (index-1)*length;  
    if(fseeko(fd, offset, SEEK_SET) == -1)
    {
        printf("Set the fd point failed.\n");
        return 0;
    }
    uint32_t read_size=fread(buf, 1,length, fd);
    if (read_size == length) 
    {
        return read_size;
    }

    if(read_size == (file_size-offset)) //最后一个报文的长度
    {
        return read_size;
    }

    return 0;

}

/**
 * @description: 将二进制字节写入一个文件，一般文件名为：.文件前缀{md5校验值}-序号.tmp
 * @param {char*} prefix 文件前缀
 * @param {uint32_t} index 报文序号
 * @param {uint8_t*} buf 二进制数据数组
 * @param {uint32_t} length 数组长度
 * @return {int} 0 成功 1 失败
 */
int WriteData(const char* prefix, const uint32_t index, const uint8_t* buf, const uint32_t length)
{
    char* path = (char*)malloc(33 * sizeof(char));
    char* index_s = (char*)malloc(10 * sizeof(char));
    Uint32ToStr(index_s, index);
    snprintf(path, 33 * sizeof(char), ".%s-%s.tmp", prefix, index_s);
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
    
    return 0;
}
    