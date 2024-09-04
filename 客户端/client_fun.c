#include "client_fun.h"
#include "msg.h"

struct sockaddr_in getaddr(char *pip,char *pport)  //通过ip和端口得到要连接服务器的网络信息
{
     struct sockaddr_in addr;  //填充服务器的网络信息
    addr.sin_addr.s_addr = inet_addr(pip);
    addr.sin_family = AF_INET;

    int port;
    sscanf(pport,"%d",&port);
    addr.sin_port = htons(port);
    return addr;
}

int user_denglu(int sockfd)
{
    MSG msg;
    msg.type = 1;
    USER user;
    printf("请输入用户名：\n");
    scanf("%s",user.name);
    printf("请输入密码：\n");
    scanf("%s",user.passwd);
    memcpy(msg.buf,&user,sizeof(user));
    write(sockfd,&msg,sizeof(msg));  //发送给服务器账号密码

    read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
    if(msg.type == 100) 
    {
        printf("登录成功！\n");
        return 1;
    }
    else if(msg.type == 101)
    {
        printf("用户名或密码错误！\n");
        return 0;
    }
    else if(msg.type == 99)
    {
        printf("服务器在线人数已满！\n");
        return 0;
    }
    else if(msg.type == 98)
    {
        printf("用户已在别的客户端登录\n");
        return 0;
    }    
}

void user_jiemian(int sockfd)
{
    while (1)
    {
        int choose =-1;
        printf("\t---------------------------用户界面---------------------------\n");
        printf("\t  1.词典查询 2.聊天界面 3.个人信息 4. 退出\n");
        printf("\t--------------------------------------------------------------\n");
        
        scanf("%d",&choose);
        switch (choose)
        {
        case 1:
            english(sockfd);
            break;
        case 2:
            userChat(sockfd);  //聊天界面
            break;
        case 3:
            usermsg(sockfd); //用户信息界面
            break;
        default:
            exit(0);
        }
    }
}

void user_zhuce(int sockfd)  //注册
{
    MSG msg;
    msg.type = 2;
    USER user;
    printf("请输入用户名：\n");
    scanf("%s",user.name);
    printf("请输入密码：\n");
    scanf("%s",user.passwd);
    memcpy(msg.buf,&user,sizeof(user));
    write(sockfd,&msg,sizeof(msg));  //发送给服务器账号密码

    read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
    if(msg.type == 103) 
        printf("注册成功!\n");
    else if(msg.type == 104)
        printf("已经存在该用户名\n");
    else if(msg.type == 105)
        printf("上传服务器失败\n");
}

void userChat(int sockfd)  //聊天界面
{
    while (1)
    {
        int choose =-1;
        printf("\t---------聊天界面---------\n");
        printf("\t  1.好友 2.私聊 3.群聊 4.返回\n");
        printf("\t--------------------------\n");
        
        scanf("%d",&choose);
        switch (choose)
        {
        case 1:
            userFriend(sockfd);
            break;
        case 2:
            privateChat(sockfd);
            break;
        case 3:
            groupChat(sockfd);
            break;
        default:
            return;
        }
    }
}

void userFriend(int sockfd)  //好友界面
{
    MSG msg;
    while (1)
    {
        int choose =-1;
        printf("\t---------好友界面---------\n");
        printf("\t  1.好友列表 2.加好友 3.删除好友 4.返回\n");
        printf("\t--------------------------\n");
        
        scanf("%d",&choose);
        switch (choose)
        {
        case 1:
            msg.type = 30;
            write(sockfd,&msg,sizeof(msg));  //发送给服务器
            read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
            int valCount = *(int *)msg.buf;
            int j=0;
            printf("\t===================================\n");
            printf("\t好友用户名\t\t个性签名\n");
            for (int i = 2; i < valCount; i++)
            {
                read(sockfd,&msg,sizeof(msg));
                printf("\t%-20s",msg.buf);
                j++;
                if(j%2==0) printf("\n");
            }
            printf("\t===================================\n");
            break;
        case 2:
        {
            msg.type = 31;
            char name[128];
            printf("输入好友用户名添加：\n");
            scanf("%s",name);
            memcpy(msg.buf,name,sizeof(name));
            write(sockfd,&msg,sizeof(msg));  //发送给服务器
            read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
            if(msg.type == 310) 
                printf("添加成功!\n");
            else if(msg.type == 311)
                printf("没找到该用户\n");
            else if(msg.type == 312)
                printf("已经添加好友\n");
            break;
        }
        case 3:
        {
            msg.type = 32;
            char name[128];
            printf("输入好友用户名删除：\n");
            scanf("%s",name);
            memcpy(msg.buf,name,sizeof(name));
            write(sockfd,&msg,sizeof(msg));  //发送给服务器
            read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
            if(msg.type == 320) 
                printf("删除成功!\n");
            else if(msg.type == 321)
                printf("没有该好友\n");
            break;
        }
        default:
            return;
        }
    }
}

void privateChat(int sockfd)  //私聊界面
{
    MSG msg;
    pthread_t pthid;
    while (1)
    {
        int choose =-1;
        printf("\t---------私聊界面---------\n");
        printf("\t  1.查看在线好友 2.好友聊天 3.查看留言 4.返回\n");
        printf("\t--------------------------\n");
        
        scanf("%d",&choose);
        switch (choose)
        {
        case 1:
            msg.type = 33;
            write(sockfd,&msg,sizeof(msg));  //发送给服务器
            int j=0;
            printf("\t===================================\n");
            printf("\t好友用户名\n");
            while (1)
            {
                read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
                if(strcmp(msg.buf,"#-#")==0) break;  //停止接受
                printf("%-20s\n",msg.buf);
            }
            printf("\t===================================\n");
            break;
        case 2:
        {
            //接收消息
            pthread_create(&pthid,NULL,dowork,&sockfd);

            msg.type = 34;
            ChatMsg cmsg;
            char name[128],txt[512];
            printf("发给谁：\n");
            scanf("%s",name);
            strcpy(cmsg.friendname,name);

            while (1)
            {
                printf("发什么(输入'*-*'改变聊天好友,'quit'退出):\n");
                scanf("%s",txt);
                memcpy(cmsg.txt,txt,sizeof(txt));

                if(strcmp(txt,"*-*")==0)
                {
                    printf("发给谁：\n");
                    scanf("%s",name);
                    strcpy(cmsg.friendname,name);
                    continue;
                }
                if(strcmp(txt,"quit")==0) break;
                memcpy(msg.buf,&cmsg,sizeof(cmsg));
                write(sockfd,&msg,sizeof(msg));  //发送给服务器
            }
            break;
        }
        case 3:
        {
            msg.type = 35;
            ChatMsg cmsg;
            write(sockfd,&msg,sizeof(msg));  //发送给服务器

            printf("\t-------留言--------\n");
            while (1)
            {           
                read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
                if(msg.type == 350)  //说明没有留言
                {
                    printf("\t-------没有新的留言--------\n");
                    break;
                }
                else if(msg.type == 351)
                {
                    cmsg = *(ChatMsg *)msg.buf;
                    if(strcmp(msg.buf,"&-&")==0) break;  //停止接受
                    printf("%s 说:%s\n",cmsg.friendname,cmsg.txt);
                }
            }
            printf("\n");
            break;
        }
        default:
            pthread_cancel(pthid); //退出并关闭线程
            return;
        }
    }
}

void *dowork(void *p)  //私聊的线程工作函数--接收信息
{
    int fd = *(int *)p;
    MSG msg1;
    while (1)
    {
        read(fd,&msg1,sizeof(msg1)); //接收消息
        if(msg1.type == 341)
        {
            printf("好友不在线,已经给好友留言\n");
        }
        else if(msg1.type == 342)
        {
            printf("你没这个好友\n");
        }
        else if(msg1.type == 340)
        {
            ChatMsg cmsg = *(ChatMsg *)msg1.buf;
            printf("%s 说:%s\n",cmsg.friendname,cmsg.txt);
        }
        else if(msg1.type == 430)
        {
            printf("聊天室不存在\n");
        }
        else if(msg1.type == 431)
        {
            printf("你不是该聊天室成员\n");
        }
        else if(msg1.type == 432)
        {
            ChatGroup cmsg = *(ChatGroup *)msg1.buf;
            printf("%s 聊天室的 %s 说:%s\n",cmsg.groupname,cmsg.username,cmsg.txt);
        }

        usleep(1000);
    }
}

void groupChat(int sockfd)  //群聊界面
{
    MSG msg;
    pthread_t pthid = 0;
    while (1)
    {
        int choose =-1;
        printf("\t---------群聊界面---------\n");
        printf("\t  1.创建聊天室 2 浏览聊天室 3.加入聊天室 4.聊天室聊天 5.退出聊天室 6.返回\n");
        printf("\t--------------------------\n");
        
        scanf("%d",&choose);
        switch (choose)
        {
            case 1:
            {
                char groupname[128];
                msg.type = 40;
                printf("给聊天室取个名:\n");
                scanf("%s",groupname);
                memcpy(msg.buf,groupname,sizeof(groupname));
                write(sockfd,&msg,sizeof(msg));  //发送给服务器
                read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息 400 创建成功 401 创建失败，已有该聊天室 402 服务器出错
                if(msg.type == 400) printf("创建成功\n");
                else if(msg.type == 401) printf("创建失败，已有该聊天室\n");
                else if(msg.type == 402) printf("服务器出错\n");
                break;
            }
            case 2:
            {   int choo;  //选择
                while (1)
                {
                    printf("\t----------------------------------------\n");
                    printf("\t1.列出全部聊天室 2.我加入的聊天室 3.返回\n");
                    printf("\t----------------------------------------\n");
                    scanf("%d",&choo);
                    if(choo == 1)
                    {
                        msg.type = 41;
                        write(sockfd,&msg,sizeof(msg));  //发送给服务器
                        read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
                        int valCount = *(int *)msg.buf;
                        int j=0;
                        if(valCount == 0)
                        {
                            printf("\t当前没有聊天室\n");
                        }
                        else
                        {
                            printf("\t===================================\n");
                            printf("\t聊天室列表\t\t聊天室人数\n");
                            for (int i = 2; i < valCount; i++)
                            {
                                read(sockfd,&msg,sizeof(msg));
                                printf("\t%-20s",msg.buf);
                                j++;
                                if(j%2==0) printf("\n");
                            }
                            printf("\t===================================\n");
                        }
                    }
                    else if(choo ==2)
                    {
                        msg.type = 45;
                        write(sockfd,&msg,sizeof(msg));  //发送给服务器
                        read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
                        int valCount = *(int *)msg.buf;
                        if(valCount == 0)
                        {
                            printf("\t没有加入聊天室\n");
                        }
                        else
                        {
                            printf("\t===================================\n");
                            printf("\t聊天室名字\n");
                            for (int i = 1; i < valCount; i++)
                            {
                                read(sockfd,&msg,sizeof(msg));
                                printf("\t%s\n",msg.buf);
                            }
                            printf("\t===================================\n");
                        }
                    }
                    else if (choo >= 3 || choo <= 0) break;
                }
                break;
            }
            case 3:
            {
                msg.type = 42;
                char groupname[128];
                printf("你想加入哪个聊天室:\n");
                scanf("%s",groupname);
                memcpy(msg.buf,groupname,sizeof(groupname));
                write(sockfd,&msg,sizeof(msg));  //发送给服务器
                read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息 420 加入成功 421 没有该聊天室 422 不能重复加入聊天室
                if(msg.type == 420) printf("加入成功\n");
                else if(msg.type == 421) printf("没有该聊天室\n");
                else if(msg.type == 422) printf("不能重复加入聊天室\n");
                break;
            }
            case 4:
            {
                //接收群聊消息
                pthread_create(&pthid,NULL,dowork,&sockfd);

                msg.type = 43;
                ChatGroup cmsg;
                char groupname[128],txt[512];
                printf("发送给哪个聊天室:\n");
                scanf("%s",groupname);
                strcpy(cmsg.groupname,groupname);

                while (1)
                {
                    printf("发什么(输入'^-^'改变聊天室,'quit'退出):\n");
                    scanf("%s",txt);
                    memcpy(cmsg.txt,txt,sizeof(txt));

                    if(strcmp(txt,"^-^")==0)
                    {
                        printf("发送给哪个聊天室:\n");
                        scanf("%s",groupname);
                        strcpy(cmsg.groupname,groupname);
                        continue;
                    }
                    if(strcmp(txt,"quit")==0) break;
                    memcpy(msg.buf,&cmsg,sizeof(cmsg));
                    write(sockfd,&msg,sizeof(msg));  //发送给服务器
                }
                break;
            }
            case 5:
            {
                msg.type = 44;
                char groupname[128];
                printf("你想退出哪个聊天室:\n");
                scanf("%s",groupname);
                memcpy(msg.buf,groupname,sizeof(groupname));
                write(sockfd,&msg,sizeof(msg));  //发送给服务器
                read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息 440 聊天室不存在 441 你根本就没加入聊天室 442 退出成功
                if(msg.type == 440) printf("聊天室不存在\n");
                else if(msg.type == 441) printf("你根本就没加入聊天室\n");
                else if(msg.type == 442) printf("退出成功\n");
                break;
            }
            default:
                if(pthid != 0) pthread_cancel(pthid); //退出并关闭线程
                return;
            }
    }
}

void usermsg(int sockfd)  //用户信息界面
{
    MSG msg;
    UserMess umsg;
    while (1)
    {
        int choose =-1;
        printf("\t---------聊天界面---------\n");
        printf("\t  1.查看个人信息 2.修改个人信息 3.注销账号 4.返回\n");
        printf("\t--------------------------\n");
        
        scanf("%d",&choose);
        switch (choose)
        {
        case 1:
            msg.type = 50;
            write(sockfd,&msg,sizeof(msg));  //发送给服务器
            read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
            umsg = *(UserMess *)msg.buf;
            printf("\t*********************\n");
            printf("\t用户名:%s\t个性签名:%s\n",umsg.username,umsg.words);
            printf("\t*********************\n");
            break;
        case 2:
        {
            int choo = -1;
            while (1)
            {
                printf("\t-----------------------------\n");
                printf("\t1.修改个性签名 2.修改密码 3.返回\n");
                printf("\t-----------------------------\n");
                scanf("%d",&choo);
                if(choo == 1)
                {
                    msg.type = 61;
                    char words[128];
                    printf("输入新的个性签名：\n");
                    scanf("%s",words);
                    strcpy(msg.buf,words);
                    write(sockfd,&msg,sizeof(msg));  //发送给服务器
                    read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
                    if(msg.type == 610) printf("成功修改个性签名\n");
                }
                else if(choo == 2)
                {
                    msg.type = 62;
                    char pass[128];
                    printf("输入新密码：\n");
                    scanf("%s",pass);
                    strcpy(msg.buf,pass);
                    write(sockfd,&msg,sizeof(msg));  //发送给服务器
                    read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
                    if(msg.type == 620) printf("成功修改密码\n");
                }
                else break;
            }
            break;
        }
        case 3:
        {
            msg.type = 53;
            USER umsg1,umsg2;
            printf("\t注销账号需要输入密码确认操作!\n");
            printf("请输入第一次密码:\n");
            scanf("%s",umsg1.passwd);
            printf("请输入第二次密码:\n");
            scanf("%s",umsg2.passwd);

            if(strcmp(umsg1.passwd,umsg2.passwd)==0)
            {
                memcpy(msg.buf,&umsg2,sizeof(USER));
                write(sockfd,&msg,sizeof(msg));  //发送给服务器
                read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
                if(msg.type == 540)  //注销成功退出客户端
                {
                    printf("注销成功！\n");
                    exit(0);
                }
                else if(msg.type == 530) printf("密码错误\n");
            }
            else printf("两次密码输入不一样！\n");
            break;
        }
        default:
            return;
        }
    }
}

void english(int sockfd)  //词典界面
{
    MSG msg;
    while (1)
    {
        int choose =-1;
        printf("\t---------词典界面---------\n");
        printf("\t  1.查单词 2.历史记录 3.离线下载 4.返回\n");
        printf("\t--------------------------\n");
        
        scanf("%d",&choose);
        switch (choose)
        {
        case 1:
        {
            EnglishWord ewd,newwd;
            printf("要查的单词:\n");
            scanf("%s",ewd.word);
            
            msg.type = 70;
            memcpy(msg.buf,&ewd,sizeof(ewd));
            write(sockfd,&msg,sizeof(msg));

            read(sockfd,&msg,sizeof(msg));
            if(msg.type == 700)
            {
                newwd = *(EnglishWord *)msg.buf;
                printf("\n\t===========查询结果===========\n");
                printf("\t== %s\t%s ==\n",newwd.word,newwd.mean);
                printf("\t===============================\n\n");
            }
            else if(msg.type == 701) printf("没有查到该单词\n");
            break;
        }
        case 2:
        {
            msg.type = 71;
            write(sockfd,&msg,sizeof(msg));
            read(sockfd,&msg,sizeof(msg));//接受服务器的返回信息
            int valCount = *(int *)msg.buf;
            int j=0;
            printf("\t===================================\n");
            printf("\t单词\t\t\t意思\n");
            for (int i = 2; i < valCount; i++)
            {
                read(sockfd,&msg,sizeof(msg));
                printf("\t%-20s",msg.buf);
                j++;
                if(j%2==0) printf("\n");
            }
            printf("\t===================================\n");
            break;
        }
        case 3:
        {
            FILE *pf = fopen("dict.txt", "wb"); // 用二进制的方式打开文件
            FileTxt ft;
            msg.type = 73;
            int count = 0;
            write(sockfd,&msg,sizeof(msg));
            while (1)
            {
                read(sockfd,&msg,sizeof(msg));
                ft = *(FileTxt *)msg.buf;
                if(msg.type == 730)
                {
                    fwrite(ft.buf,ft.len, 1, pf);
                    count += ft.len;
                }
                else if(msg.type == 731) //最后一次接收
                {
                    fwrite(ft.buf,ft.len, 1, pf);
                    count += ft.len;
                    printf("传输完成，总共接收了%d个字节\n",count);
                    break;
                }
            }
            fclose(pf);
            break;
        }
        default:
            return;
        }
    }
}