/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-25 14:01:45
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-09-02 15:30:50
 * @FilePath: /tcp-server/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "config.h"
#include "log/log.h"
#include "server/server.h"

int main(int argc, char* argv[]){
    server_log = fopen(CLIENT_LOG, "a");
	log_set_level(SERVER_LOG_LEVEL);
    StartServer();
    fclose(server_log);
    return 0;
}

