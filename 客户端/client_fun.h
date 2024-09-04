#ifndef _CLIENT_FUN_H
#define _CLIENT_FUN_H

#include "msg.h"

struct sockaddr_in getaddr(char *pip,char *pport);  //通过ip和端口得到要连接服务器的网络信息

int user_denglu(int sockfd); //登录

void user_jiemian(int sockfd); //用户界面

void user_zhuce(int sockfd);  //注册

void userChat(int sockfd);  //聊天界面

void userFriend(int sockfd);  //好友界面

void privateChat(int sockfd);  //私聊界面

void *dowork(void *p);  //聊天的线程工作函数--接收信息

void groupChat(int sockfd);  //群聊界面

void usermsg(int sockfd);  //用户信息界面

void english(int sockfd);  //词典界面
#endif