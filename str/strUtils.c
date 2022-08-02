/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-28 13:45:30
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-02 15:48:57
 * @FilePath: /tcp-server/string/int2string.h
 * @Description: 整数转字符串
 */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "strUtils.h"


/**
 * @description: 获取非负整数的位数
 * @param {uint32_t} num 一个非负正整数
 * @return {int} 非负整数的位数
 */
int GetIntDigit(uint32_t num)
{
    if(num == 0) return 1;
    return (int)ceil(log10(num));
}

/**
 * @description: 将非负整数转换为字符串
 * @param {char*} str 被转换的字符串
 * @param {uint32_t} num 非负整数
 * @return {int} 转换后写入的字符串长度
*/
int Uint32ToStr(char* str,uint32_t num)
{
    return snprintf(str, sizeof(str), "%u", num);
}

/**
 * @description: 将字节转换为十六进制字符串表示 
 * @param {uint8_t*} buf 字节数组
 * @param {int} length 字节数组长度
 * @param {char*} result 存放结果的字符数组
 * @return {int} 0 成功 1 失败 
 */
int Byte2Str(const uint8_t* buf, int length, char* result)
{
    const char hex_buf[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    uint8_t high=0,low=0;
    for (size_t i=0; i<length; i++) 
    {
        high = (buf[i] >> 4) & 0x0f;
        low = buf[i] & 0x0f;    
        result[2*i] = hex_buf[high];
        result[2*i+1] = hex_buf[low];
    }
    result[2*length]='\0';
    return 0;
}