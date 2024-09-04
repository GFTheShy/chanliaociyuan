#ifndef _SERVER_FUN_H
#define _SERVER_FUN_H

#include <sqlite3.h>
#include "msg.h"

#define onlinemax 1000

//定义在线的用户，保存在数组中
typedef struct onlineUser{
    int fd;
    char name[128];
}Ouser;

//在线人数的数组
Ouser ouser[onlinemax];

//词典
void copyDictToDataBase(sqlite3 * db);  //加载词典数据

void server_get_word(int fd,MSG msg,sqlite3 *udb);  //用户要查单词

void server_history_word(int fd,MSG msg,sqlite3 *udb);  //用户要查历史记录

void server_down_word(int fd,MSG msg,sqlite3 *udb);  //用户要离线下载
//词典


//数据库操作
void exec_sqlite3(sqlite3 *db,char *sql);  //执行数据库

int exec_zhuce(sqlite3 *db,USER user); //注册用户

void exec_zhuxiao(sqlite3 *db,char *username);  //注销用户

int get_sqlite3(sqlite3 *db,char *name,char *pass);  //从数据库匹配该用户--用于登录

int get_name_sqlite3(sqlite3 *db,char *name);  //从数据库匹配该用户--用于注册--用于添加好友--不能重复添加

int get_friend_sqlite3(sqlite3 *db,char *username,char *friendname);  //从好友列表数据库匹配该用户--用于添加/删除好友

int get_group_sqlite3(sqlite3 *db,char *groupname);  //从聊天室列表中匹配是否存在该聊天室--用于加入/退出聊天室

int get_member_sqlite3(sqlite3 *db,char *groupname,char *username);  //从聊天室中匹配是否有该成员--用于加入聊天室/退出--用于广播
//数据库操作


//业务
void server_denglu(int fd,MSG msg,sqlite3 *udb); //处理用户登录

int online_user(char *name); //检查用户登录时是否已经在别的客户端登录

void server_zhuce(int fd,MSG msg,sqlite3 *udb);  //处理用户注册

void server_get_friend(int fd,MSG msg,sqlite3 *udb);  ////用户要查看他的全部好友

void server_add_friend(int fd,MSG msg,sqlite3 *udb);  //用户要添加好友

void server_del_friend(int fd,MSG msg,sqlite3 *udb);  //用户要删除好友

void server_onl_friend(int fd,MSG msg,sqlite3 *udb);  //用户要查看在线好友

void server_cht_friend(int fd,MSG msg,sqlite3 *udb);  //用户要私聊好友

void server_look_words(int fd,MSG msg,sqlite3 *udb);  //用户要查看留言

void server_create_group(int fd,MSG msg,sqlite3 *udb);  //用户要创建聊天室

void server_look_group(int fd,MSG msg,sqlite3 *udb);  //用户要浏览聊天室

void server_join_group(int fd,MSG msg,sqlite3 *udb);  //用户要加入聊天室

void server_useradd_group(int fd,MSG msg,sqlite3 *udb);  //用户要查看加入的聊天室

void server_send_group(int fd,MSG msg,sqlite3 *udb);  //用户要聊天室聊天

void server_quit_group(int fd,MSG msg,sqlite3 *udb);  //用户要退出聊天室

void server_get_user(int fd,MSG msg,sqlite3 *udb);  //用户要查看个人信息

void server_change_words(int fd,MSG msg,sqlite3 *udb);  //用户要修改个性签名

void server_change_pass(int fd,MSG msg,sqlite3 *udb);  //用户要修改密码

void server_user_zhuxiao(int fd,MSG msg,sqlite3 *udb);  //用户要注销账号
//业务

#endif