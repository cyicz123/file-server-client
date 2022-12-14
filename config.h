/*
 * @Author: cyicz123 cyicz123@outlook.com
 * @Date: 2022-08-25 14:02:28
 * @LastEditors: cyicz123 cyicz123@outlook.com
 * @LastEditTime: 2022-09-17 15:49:36
 * @FilePath: /tcp-server/config.h
 * @Description: 项目配置选项
 */
#ifndef CONFIG_H
#define CONFIG_H

#define SERVER_START_PORT 8080
#define SERVER_MAXIUM_THREADS 1024
#define SERVER_STORAGE_FILE "./storage"

#define CLIENT_DEFAULT_ADDR "127.0.0.1:8080"
#define CLIENT_DOWNLOADS_THREADS_NUM 1
#define CLIENT_UPLOADS_THREADS_NUM 1

#define CLIENT_LOG "client.log"
#define CLIENT_LOG_LEVEL LOG_INFO
#define SERVER_LOG "server.log"
#define SERVER_LOG_LEVEL LOG_DEBUG

#endif