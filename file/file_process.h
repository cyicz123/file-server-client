/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-27 10:04:22
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-07-28 09:38:21
 * @FilePath: /tcp-server/file/file_process.h
 * @Description: 对文件打开，分割，合并处理
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @description: 打开文件，同时对文件进行检查是否大于4GB。
 * @param {char*} file_path 欲打开的文件路径
 * @return {FILE*} fd 如果为NULL说明打开失败
 */
FILE* ReadFile(const char* file_path);

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
 * @param {uint16_t} length 数组长度
 * @param {uint16_t} index 读取数据的序号
 * @param {uint32_t} file_size 整个文件的大小，单位Byte
 * @return {uint16_t} 已经读取的字节数，0代表失败
 */
uint16_t ReadData(FILE* fd,uint8_t* buf,const size_t length, const uint16_t index, const uint32_t file_size);