#include "server_fun.h"

//在线人数
int onlineCount = 0;

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // 创建套接字

    struct sockaddr_in addr; // 填充服务器的网络信息
    addr.sin_addr.s_addr = inet_addr("192.168.50.179");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);

    int val = 1;
    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)))
        ERR_LOG("setsockopt"); // 设置地址端口复用

    if (-1 == bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)))
        ERR_LOG("bind"); // 绑定服务器

    if (-1 == listen(sockfd, 10))
        ERR_LOG("listen"); // 监听

    int epofd = epoll_create1(0); // 创建epoll红黑树

    // 把监听套接字添加到红黑树
    struct epoll_event ev, events[EVENTSMAX];
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    if (-1 == epoll_ctl(epofd, EPOLL_CTL_ADD, sockfd, &ev))
        ERR_LOG("epoll_ctl");
    printf("畅聊词苑服务器运行中............\n");
    
    //打开用户数据库
    sqlite3 *udb = NULL;
    int data = sqlite3_open("user.db",&udb);
    if(data < 0) ERR_LOG("sqlite");

    //创建用户表格
    char *sql = "CREATE TABLE user(name varchar(255),password varchar(255),words varchar(255))";
    exec_sqlite3(udb,sql);  //调用执行函数

    copyDictToDataBase(udb);  //加载词典数据

    while (1)
    {
        // 等待红黑树上的事件发生
        int Ecount = epoll_wait(epofd, events, EVENTSMAX, -1);
        if (Ecount == -1)
            ERR_LOG("epoll_wait");

        for (int i = 0; i < Ecount; i++)
        {
            if (events->events == EPOLLIN) // 发生了可读事件
            {
                if (events[i].data.fd == sockfd) // 监听套接字
                {
                    struct sockaddr_in newaddr;
                    socklen_t newaddr_len = sizeof(newaddr);
                    int newfd = accept(sockfd, (struct sockaddr *)&newaddr, &newaddr_len); // 连接
                    if (newfd == -1)
                        ERR_LOG("accept");

                    ev.data.fd = newfd;
                    ev.events = EPOLLIN;
                    if (-1 == epoll_ctl(epofd, EPOLL_CTL_ADD, newfd, &ev))
                        ERR_LOG("epoll_ctl"); // 连接的新套接字上树
                }
                else // 有客户要发信息
                {
                    MSG msg;
                    int ret = read(events[i].data.fd, &msg, sizeof(msg));
                    if (ret == -1)
                        ERR_LOG("read");
                    else if (ret == 0) // 客户端退出
                    {
                        //从在线用户的数组中删除
                        for(int j=0;j<onlineCount;j++)
                        {
                            if(events[i].data.fd == ouser[j].fd)
                            {
                                printf("%d号客户端 %s用户已经退出服务器\n",ouser[j].fd,ouser[j].name);
                                for(;j<onlineCount-1;j++)
                                {
                                    ouser[j].fd = ouser[j+1].fd;
                                    strcpy(ouser[j].name,ouser[j+1].name); 
                                }
                                onlineCount--;//在线人数-1
                                break;
                            }
                        }
                        if (-1 == epoll_ctl(epofd, EPOLL_CTL_DEL, events[i].data.fd, NULL))
                            ERR_LOG("epoll_ctl"); // 从树上删除
                        close(events[i].data.fd);
                    }
                    else //业务代码
                    {
                        if (msg.type == 1)  //说明客户要登录
                        {
                            server_denglu(events[i].data.fd,msg,udb); 
                        }
                        else if(msg.type == 2)  //说明客户要注册
                        {
                            server_zhuce(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 30)  //用户要查看他的全部好友
                        {
                            server_get_friend(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 31)  //用户要添加好友
                        {
                            server_add_friend(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 32)  //用户要删除好友
                        {
                            server_del_friend(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 33)  //用户要查看在线好友
                        {
                            server_onl_friend(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 34)  //用户要私聊好友
                        {
                            server_cht_friend(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 35)  //用户要查看留言
                        {
                            server_look_words(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 40)  //用户要创建聊天室 
                        {
                            server_create_group(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 41)  //用户要浏览聊天室 
                        {
                            server_look_group(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 42)  //用户要加入聊天室 
                        {
                            server_join_group(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 45)  //用户要查看加入的聊天室 
                        {
                            server_useradd_group(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 43)  //用户要聊天室聊天
                        {
                            server_send_group(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 44)  //用户要退出聊天室
                        {
                            server_quit_group(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 50)  //用户要查看个人信息
                        {
                            server_get_user(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 61)  //61 修改个性签名 62 修改密码
                        {
                            server_change_words(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 62) 
                        {
                            server_change_pass(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 53)   //用户要注销账号
                        {
                            server_user_zhuxiao(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 70)   //用户要查单词
                        {
                            server_get_word(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 71)   //用户要查历史记录
                        {
                            server_history_word(events[i].data.fd,msg,udb);
                        }
                        else if(msg.type == 73)   //用户要离线下载
                        {
                            server_down_word(events[i].data.fd,msg,udb);
                        }
                    }
                }
            }
        }
    }

    // 服务器退出：完成下树，将退出的文件描述符listenfd及其事件从epoll实例中删除
    ev.data.fd = sockfd;
    if (-1 == epoll_ctl(epofd, EPOLL_CTL_DEL, sockfd, &ev))
        ERR_LOG("epoll_ctl");

    sqlite3_close(udb);  //关闭用户数据库
    close(sockfd);  //关闭监听套接字
    close(epofd);  //关闭红黑树
    return 0;
}