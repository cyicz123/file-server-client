/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-03 14:57:36
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-11 17:13:37
 * @FilePath: /tcp-server/network/network.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "network.h"
#include <stdio.h>

/**
 * @description: fd收取size字节的数据
 * @param {int} fd 套接字fd
 * @param {uint8_t*} buf 存放数据的数组
 * @param {size_t} size 字节长度
 * @return {int} 0 成功 1 失败
 */
int Receive(int fd, void* buf, size_t size)
{
    int times = 10 ; 
    size_t recv_byte = 0;
    for(size_t i =0; i < times; i++)
    {
        recv_byte = recv_byte + recv(fd,buf,size - recv_byte,0);
        if(recv_byte == size)
            break;
    }
    if (recv_byte == size) 
    {
        return 0;
    }
    else 
    {
        printf("Expect to receive %ld bytes, only receive %ld bytes.\n",size,recv_byte);
        return 1;
    }
}