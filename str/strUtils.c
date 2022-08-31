/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-28 13:45:30
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-30 15:17:29
 * @FilePath: /tcp-server/string/int2string.h
 * @Description: 字符串处理工具函数
 */
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strUtils.h"
#include "../log/log.h"


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
 * @param {int} len str长度
 * @param {uint32_t} num 非负整数
 * @return {int} 转换后写入的字符串长度
*/
int Uint32ToStr(char* str, int len, uint32_t num)
{
    return snprintf(str, len, "%u", num);
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

/**
 * @description: 逐字节比较两字节数组是否相同
 * @param {uint8_t*} s1 字节数组1
 * @param {uint8_t*} s2 字节数组2
 * @param {int} size 字节长度
 * @return {int} 0 不相同 1 相同
 */
int CompareByte(const void* s1, const void* s2, int size)
{
    uint8_t* _s1 = (uint8_t*)s1;
    uint8_t* _s2 = (uint8_t*)s2;
    for(size_t i=0; i<size; i++)
    {
        if(_s1[i] != _s2[i])
            return 0;
    }
    return 1;
}

int Str2Addr(const char* str_addr, struct sockaddr_in* addr){
	const char* str_port = strchr(str_addr, ':'); // str_port结果为":port"
	int convert_ret = -1;
	
	if (NULL != str_port && str_port > str_addr) { // str_port非空且str_port在str_addr右边，则可认为str_port有效
		int port = atoi(str_port + 1);
		if(port < 1 || port > 65535){
			log_error("The address %s port is illegal!", str_addr);
			return 1;
		}
		addr->sin_port = htons(port);

		char str_ip[255] = {'\0'};
		memcpy(str_ip, str_addr, str_port-str_addr);
		convert_ret = inet_aton(str_ip, &addr->sin_addr);
		if (1 != convert_ret) {
			log_error("The address %s ip is illegal!", str_addr);
			return 1;
		}
		return 0;
	}
	else {
		log_error("The address %s is illegal!", str_addr);
		return 1;
	}
}