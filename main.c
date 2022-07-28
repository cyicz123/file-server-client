/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-28 09:49:30
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-07-28 09:58:57
 * @FilePath: /tcp-server/main.c
 * @Description: 主函数
 */
#include "md5/md5.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
    char* path=NULL;
    char result[128]={0};
 
    if (argc < 2) {
        printf("usage: %s 'string'\n", argv[0]);
        return 1;
    }
    path = argv[1];
 
 
    // benchmark
    for (size_t i = 0; i < 1000000; i++) {
        GetFileMD5(path, result);
    }
 
    printf("%s's md5 is %s\n",path,result);
 
    return 0;
}