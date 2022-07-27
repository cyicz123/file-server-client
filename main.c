/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-27 09:56:12
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-07-27 20:15:57
 * @FilePath: /tcp-server/main.c
 */
#include "md5/md5.h"
#include "file/file_process.h"
#include <string.h>

#define DATA_SIZE 65536

int main(int argc, char* argv[])
{
    char *path;
 
    if (argc < 2) {
        printf("usage: %s 'path'\n", argv[0]);
        return 1;
    }
    path = argv[1];
 
    FILE *fd=NULL;
    fd=ReadFile(path);
    if(fd == NULL)
    {
        return 1;
    }

    uint64_t size = GetFileSize(path);
    uint16_t read_byte_size = 0;
    uint8_t buf[DATA_SIZE]={0};
    for (uint16_t index=1 ; (index-1)*DATA_SIZE<size ; index++) {
        memset((uint8_t*)buf, 0, DATA_SIZE);
        read_byte_size = ReadData(fd, buf, DATA_SIZE, index, size);
        printf("index: %d, size: %d\n",index,read_byte_size);
    }
    CloseFile(fd);
    return 0;
}