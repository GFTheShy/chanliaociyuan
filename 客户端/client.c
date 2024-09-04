#include "client_fun.h"

int main(int argc,char *argv[])
{
    if(argc<3)  //从命令行得到要连接的服务器
    {
        printf("请正确输入要连接的服务器和端口号\n");
        exit(0);
    } 
    int sockfd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in addr = getaddr(argv[1],argv[2]);  //填充服务器的网络信息

    if(-1 == connect(sockfd,(struct sockaddr *)&addr,sizeof(addr))) ERR_LOG("connect");

    while (1)
    {
        int choose =-1;
        printf("\t---------畅聊词苑---------\n");
        printf("\t  1.登录 2.注册 3.退出\n");
        printf("\t--------------------------\n");
        
        scanf("%d",&choose);
        switch (choose)
        {
        case 1:
            if(user_denglu(sockfd)) user_jiemian(sockfd);  //登录成功进入用户界面
            break;
        case 2:
            user_zhuce(sockfd);  //注册
            break;
        default:
            return 0;
        }
    }
    
    return 0;
}