#ifndef _MSG_H_
#define _MSG_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

//打印错误信息，文件，函数，行数
#define ERR_LOG(msg)                                        \
    do                                                      \
    {                                                       \
        printf("%s %s %d\n", __FILE__, __func__, __LINE__); \
        perror(msg);                                        \
        exit(-1);                                           \
    } while (0)

//服务器  事件的最大值
#define EVENTSMAX 1000

//通用的信息结构体，用来客户端和服务器通信
//type: 1:登录 2：注册          词典    个人信息
//      100 登录成功 101 登录失败  99 服务器在线人数已满 98 该用户已经在别的客户端登录
//      103 注册成功 104 注册时已经存在该用户名 105 上传服务器失败
//      聊天界面 ：
//          好友    30 查看所有好友信息 31 加好友 32 删除好友
//                      310 加好友成功 311 没找到该用户 312 已经添加好友
//                      320 删除成功  321 没加该好友
//          私聊    33 查看在线好友  34 在线聊天 35 查看留言
//                      340 别人发的消息 341 好友不在线 342 你没这个好友 
//                      350 没有新的留言 351 好友的留言
//          群聊    40 创建聊天室 41 浏览全部聊天室 45 查看加入的聊天室  42 加入聊天室  43 聊天室聊天 44 退出聊天室 
//                      400 创建成功 401 创建失败，已有该聊天室 402 服务器出错
//                      420 加入成功 421 没有该聊天室 422 不能重复加入聊天室
//                      430 聊天室不存在 431 你不是该聊天室成员 432 群聊发来的消息
//                      440 聊天室不存在 441 你根本就没加入聊天室 442 退出成功
//      个人信息：  50 查看个人信息 51 修改密码 52 修改个性签名 53 注销账号
//                  61 修改个性签名 62 修改密码
//                  610 成功修改个性签名
//                  620 成功修改密码
//                  530 注销时密码错误 540 注销成功
//  
//       词典       70 查单词 71 查看历史记录   73 离线下载
//                      700 查到单词  701 没查到单词     
//                      730 传输 731 最后一次传输                
typedef struct MSG{
    short type;
    char buf[1022];
}MSG;

typedef struct USER  //注册和登录  当type为1和2时
{
    char name[128];
    char passwd[128];
}USER;

typedef struct ChatMsg{  //聊天信息    当type为34时
    char username[128];
    char friendname[128];
    char txt[512];
}ChatMsg;

typedef struct ChatGroup{  //群聊信息   当type为43时
    char username[128];
    char groupname[128];
    char txt[512];
}ChatGroup;

typedef struct UserMess{   //用户信息
    char username[128];
    char words[128];
}UserMess;

typedef struct EnglishWord{  //英语单词结构体
    char word[50];
    char mean[50];
}EnglishWord;

typedef struct FileTxt{   //用于传输文件
    int len;
    char buf[1000];
}FileTxt;
#endif