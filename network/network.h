/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-03 14:57:30
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-03 18:46:58
 * @FilePath: /tcp-server/network/network.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <stdint.h>

typedef struct SendProtocol
{
    uint16_t head;
    uint32_t buf_length;
    uint32_t index;
}SendProtocol;

typedef struct QueryBuf
{
    uint64_t file_size;
    uint32_t block_size;
    uint8_t checksum[16];
}QueryBuf;



typedef struct RecvProtocol
{
    uint16_t head;
    uint32_t index;
    uint32_t length;
}RecvProtocol;
