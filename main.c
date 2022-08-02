/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-07-28 09:49:30
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-02 17:06:21
 * @FilePath: /tcp-server/main.c
 * @Description: 主函数
 */
#include "md5/md5.h"
#include "file/file_process.h"
#include "str/strUtils.h"
#include <libgen.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 1048576

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
    int size_digit = GetIntDigit(size);
    char* str_size = (char*)malloc(sizeof(char)*(size_digit+1));
    Uint32ToStr(str_size, size);
    printf("size is %s , digit is %d\n",str_size,size_digit);
    free(str_size);

    uint32_t read_byte_size = 0;
    uint8_t buf[DATA_SIZE]={0};
    uint8_t decrypt[16];
    char ret[33]={0};

    uint32_t index =0;
    while (!feof(fd)) 
    {
        memset((uint8_t*)buf, 0, DATA_SIZE);
        read_byte_size = ReadData(fd, buf, DATA_SIZE, index);
        printf("index: %u, size: %u\n",index,read_byte_size);
        
        GetStrMD5(buf, read_byte_size, decrypt);
        Byte2Str(decrypt, 16, ret);
        printf("%s\n",ret);

        int result=WriteData(basename(path), index, buf, read_byte_size);

        index++;
    }
    CloseFile(fd);

    FILE* write_fd = WriteFile("test_merge_file");
    if(write_fd == NULL)
    {
        return 1;
    }

    index = 0;
    while (1) 
    {
        int index_s_length = GetIntDigit(index) + 1;
        char* index_s = (char*)malloc(index_s_length * sizeof(char));
        Uint32ToStr(index_s, index);
        
        char* prefix = basename(path);
        size_t path_length = (strlen(prefix) + strlen(index_s) + 2 + 1 ) * sizeof(char); //2为.%s-%s中的固定字符数量
        char* data_path = (char*)malloc(path_length * sizeof(char));

        snprintf(data_path, path_length * sizeof(char), ".%s-%s", prefix, index_s);
        free(index_s);

        if(!ExistFile(data_path))
        {
            CloseFile(write_fd);
            free(data_path);
            break;
        }
        MergeFile(write_fd, data_path);
        index++;
    }
    
    return 0;
}