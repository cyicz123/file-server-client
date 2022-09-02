/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-30 14:06:50
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-09-02 15:31:46
 * @FilePath: /tcp-server/client.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "config.h"
#include "client/client.h"
#include "log/log.h"

int main(int argc, char* argv[]){
	client_log = fopen(CLIENT_LOG, "a");
	log_set_level(CLIENT_LOG_LEVEL);
    log_set_quiet(true);
	StartClient(argc, argv);
	fclose(client_log);
	return 0;
}