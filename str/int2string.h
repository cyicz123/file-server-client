/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-28 13:45:30
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-07-28 14:46:39
 * @FilePath: /tcp-server/string/int2string.h
 * @Description: 整数转字符串
 */
#ifndef __INT_TO_STR__
#define __INT_TO_STR__

#include <stdint.h>

/**
 * @description: 获取正整数的位数
 * @param {uint32_t} num 一个非负整数
 * @return {int} 非负整数的位数
 */
int GetIntDigit(uint32_t num);

/**
 * @description: 将非负整数转换为字符串
 * @param {char*} str 被转换的字符串
 * @param {uint32_t} num 非负整数
 * @return {int} 转换后写入的字符串长度
 */
int Uint32ToStr(char* str,uint32_t num);

#endif