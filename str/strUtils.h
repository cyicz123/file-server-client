/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-28 13:45:30
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-12 09:52:39
 * @FilePath: /tcp-server/string/int2string.h
 * @Description: 字符串处理工具函数
 */
#ifndef STR_UTILS_H
#define STR_UTILS_H

#include <stdint.h>

/**
 * @description: 获取非负整数的位数
 * @param {uint32_t} num 一个非负整数
 * @return {int} 非负整数的位数
 */
int GetIntDigit(uint32_t num);

/**
 * @description: 将非负整数转换为字符串
 * @param {char*} str 被转换的字符串
 * @param {int} len str长度
 * @param {uint32_t} num 非负整数
 * @return {int} 转换后写入的字符串长度
 */
int Uint32ToStr(char* str,int len, uint32_t num);

/**
 * @description: 将字节转换为十六进制字符串表示 
 * @param {uint8_t*} buf 字节数组
 * @param {int} length 字节数组长度
 * @param {char*} result 存放结果的字符数组
 * @return {int} 0 成功 1 失败 
 */
int Byte2Str(const uint8_t* buf, int length, char* result);

/**
 * @description: 逐字节比较两字节数组是否相同
 * @param {uint8_t*} s1 字节数组1
 * @param {uint8_t*} s2 字节数组2
 * @param {int} size 字节长度
 * @return {int} 0 不相同 1 相同
 */
int CompareByte(const void* s1, const void* s2, int size);
#endif