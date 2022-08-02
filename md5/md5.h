/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-26 14:45:06
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-02 15:34:09
 * @FilePath: /tcp-server/md5/md5.h
 * @Description: 计算文件或者字符串的MD5
 */
#ifndef MD5_H
#define MD5_H

#include <stdint.h>
#include <stdio.h>

/**
 * @description: 计算文件的MD5
 * @param {FILE*} fd 文件描述符
 * @param {uint8_t*} decrypt 16字节的数组存放MD5校验结果，结果为二进制
 * @return {int} 0 成功 1 失败
 */
int GetFileMD5(FILE* fd, uint8_t* decrypt);

/**
 * @description: 计算字符串的MD5
 * @param {unsigned char*} str 字符串
 * @param {size_t} length 字符串长度
 * @param {uint8_t*} decrypt 16字节的数组存放MD5校验结果，结果为二进制
 * @return {int} 0 成功 1 失败
 */
int GetStrMD5(unsigned char* str, size_t length, uint8_t* decrypt);
#endif