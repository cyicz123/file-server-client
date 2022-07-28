/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-28 09:49:30
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-07-28 19:25:00
 * @FilePath: /tcp-server/main.c
 * @Description: 主函数
 */
#include "md5/md5.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char* argv[])
{
    char* msg=NULL;
    char* path=NULL;
    uint8_t decrypt1[16],decrypt2[16];
 
    if (argc < 3) {
        printf("usage: %s 'string' 'file'\n", argv[0]);
        return 1;
    }
    msg = argv[1];
    path = argv[2];

    FILE* fd = NULL;
    fd = fopen(path, "rb");
    if(fd == NULL) 
    {
        fprintf(stderr, "File %s not exists, errno = %d, error = %s\n", path, errno, strerror(errno));
        return 1;
    }
 
    GetStrMD5((unsigned char*)msg, strlen(msg), decrypt1);
    GetFileMD5(fd, decrypt2);
    
 
    for(size_t i=0; i<16; i++)
    {
        printf("%02x", decrypt1[i]);
    }
    printf("\n");

    for(size_t i=0; i<16; i++)
    {
        printf("%02x", decrypt2[i]);
    }
    printf("\n");
 
    fclose(fd);
    return 0;
}