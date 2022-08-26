/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-25 14:01:45
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-26 19:11:08
 * @FilePath: /tcp-server/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "config.h"
#include "log/log.h"
#include "thread/thread.h"

int main(int argc, char* argv[]){
    StartServer();
    return 0;
}

