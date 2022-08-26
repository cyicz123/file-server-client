/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-25 14:01:45
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-08-26 11:12:52
 * @FilePath: /tcp-server/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "config.h"
#include "log/log.h"
#include "thread/thread.h"
#include <pthread.h>


int main(int argc, char* argv[]){
    pthread_t server_tid;
    pthread_create(&server_tid, NULL, StartServer, NULL);
    pthread_join(server_tid, NULL);
    return 0;
}

