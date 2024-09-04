#include "server_fun.h"
#include "msg.h"

// 声明全局变量  
extern int onlineCount;

void copyDictToDataBase(sqlite3 * db)  //加载词典数据
{
    //没有词典表则创建词典表
    char * sql = "create table  if not exists dict (word char,mean char)";
	sqlite3_exec(db,sql,NULL,NULL,NULL);

    //表中有数据则无需加载
    sql = "select * from dict";
	char ** presult = NULL;
	int row,column;
	sqlite3_get_table(db,sql,&presult,&row,&column,NULL);

	sqlite3_free_table(presult); //释放内存
	if(row==0)  //加载词典数据
	{
		FILE * fp = fopen("./dict.txt","r");
		if(fp==NULL)
		{
			perror("fopen");
			return;
		}
		char buf[100];
		char word[50];
		char mean[50];

		char op[128];
		char * fg = NULL;
		while(1)
		{
			bzero(buf,sizeof(buf));
			fg = fgets(buf,sizeof(buf),fp);
			if(fg==NULL)  //加载完毕
			{
				break;
			}
			bzero(word,sizeof(word));
			bzero(mean,sizeof(mean));
			sscanf(buf,"%s %[^'\n']",word,mean);
			sprintf(op,"insert into dict values(\"%s\",\"%s\")",word,mean);
			sqlite3_exec(db,op,NULL,NULL,NULL);
		}
		fclose(fp);
	}
}

void server_get_word(int fd,MSG msg,sqlite3 *udb)  //用户要查单词
{
    EnglishWord ewd = *(EnglishWord *)msg.buf;

    char s1[512];
    sprintf(s1,"SELECT * FROM dict where word='%s';",ewd.word);

    char ** ppResult;
    int row;//行
    int col;//列
    sqlite3_get_table(udb, s1, &ppResult, &row, &col,NULL);
    if(row == 0) msg.type =701;  //没查到单词
    else
    {
        strcpy(ewd.mean,ppResult[col+1]);  //把意思复制返回客户的信息
        memcpy(msg.buf,&ewd,sizeof(ewd));
        msg.type =700;

        char username[128];
        for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
        {
            if(fd == ouser[i].fd)
            {
                strcpy(username,ouser[i].name);
                break;
            }
        }

        //如果没创建就创建用户的历史记录表
        char s1[512];
        sprintf(s1,"create table  if not exists %s_history (word char,mean char)",username);
        exec_sqlite3(udb,s1);

        //存到用户的历史记录表中
        char s2[512];
        sprintf(s2,"insert into %s_history values(\"%s\",\"%s\")",username,ewd.word,ewd.mean);
        exec_sqlite3(udb,s2);
    }
    write(fd,&msg,sizeof(msg));
}

void server_history_word(int fd,MSG msg,sqlite3 *udb)  //用户要查历史记录
{
    char username[128];
    for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
    {
        if(fd == ouser[i].fd)
        {
            strcpy(username,ouser[i].name);
            break;
        }
    }
    char sql[512];
    sprintf(sql,"SELECT * FROM %s_history;",username);  //从用户历史记录数据库中得到数据
    char *msg1 = 0;
    char **ppResult;//指向字符串数组的指针，每一个元素是一个字符串
    int row;//行
    int col;//列
    int data =sqlite3_get_table(udb, sql, &ppResult, &row, &col,&msg1);
    int valCount = row*col+col;
    memcpy(msg.buf,&valCount,sizeof(int));
    write(fd,&msg,sizeof(msg));
    for (int i = col; i < valCount; i++)
    {
        strcpy(msg.buf,ppResult[i]);
        write(fd,&msg,sizeof(msg));
    }
}

void server_down_word(int fd,MSG msg,sqlite3 *udb)  //用户要离线下载
{
    FILE *pf = fopen("dict.txt", "rb"); // 用二进制的方式打开文件

    int count = 0;
    int flag = 1;
    FileTxt ft;
    while (1) // 整个文件还没有发送完毕
    {
        // fread的返回值是读了几个第二个参数那么多字节
        int ret = 0;
        while (ret != 1000) // 一直读，直到填满1000个字节
        {
            // ret相当于是作为fread多次完成任务的一个进度
            ret += fread(ft.buf + ret, 1, 1000 - ret, pf);
            if (feof(pf) == 1) // 如果文件已经到达了末尾，确实没有数据可以读了
            {
                flag = 0;
                break;
            }
        }
        ft.len = ret;
        count+=ret;
        if(flag == 1)
        {
            msg.type = 730;
            memcpy(msg.buf,&ft,sizeof(ft));
            write(fd,&msg,sizeof(msg));
        }
        else
        {
            msg.type = 731;
            memcpy(msg.buf,&ft,sizeof(ft));
            write(fd,&msg,sizeof(msg));
            printf("发送了%d个字节\n",count);
            break;
        }
    }
    fclose(pf);
}


void exec_sqlite3(sqlite3 *db,char *sql)  //执行数据库
{
    char *msg=0;
    // printf("===================hhhh===================%s\n",sql);  //调试信息
    sqlite3_exec(db, sql, 0, 0, &msg);
    if (msg != 0)
        if (strcmp("table user already exists", msg) != 0)
            printf("%s\n", msg);
}

int exec_zhuce(sqlite3 *db,USER user) //注册用户
{
    char sql1[512];
    char *words = "Hello world!"; //默认个性签名
    sprintf(sql1,"INSERT INTO user VALUES ('%s','%s','%s');",user.name,user.passwd,words);
    exec_sqlite3(db,sql1);  //调用执行函数

    //创建用户好友表格
    char sql2[512];
    sprintf(sql2,"CREATE TABLE %s(name varchar(255),words varchar(255))",user.name);
    exec_sqlite3(db,sql2);  //调用执行函数

    return 1;  //注册成功
}

void exec_zhuxiao(sqlite3 *db,char *username)  //注销用户
{
    //删除好友信息
    char sh1[512];
    sprintf(sh1,"SELECT name FROM %s",username);
    char ** result; //存放成功匹配结果的二维数组
    int row,col; //行，列
    sqlite3_get_table(db, sh1, &result, &row, &col,NULL);
    if(row != 0)
    {
        for(int i = col;i < row*col+col;i++)
        {
            char sh2[512];
            sprintf(sh2,"DELETE FROM %s WHERE name='%s';",result[i],username);
            exec_sqlite3(db, sh2);  //调用执行函数
        }
    }


    //删除用户好友表
    char s1[512];
    sprintf(s1,"DROP TABLE IF EXISTS %s;",username);
    exec_sqlite3(db,s1);

    //删除用户聊天室表
    char s2[512];
    sprintf(s2,"DROP TABLE IF EXISTS %s_addGroup;",username);
    exec_sqlite3(db,s2);

    //删除用户单词历史记录表
    char s3[512];
    sprintf(s3,"DROP TABLE IF EXISTS %s_history;",username);
    exec_sqlite3(db,s3);

    //删除已注册的信息
    char s4[512];
    sprintf(s4,"DELETE FROM user WHERE name='%s';",username);
    exec_sqlite3(db,s4);

}

int get_sqlite3(sqlite3 *db,char *name,char *pass)  //从数据库匹配该用户--用于登录
{
    char sql[512];
    sprintf(sql,"SELECT * FROM user where name='%s' and password='%s';",name,pass);
    char * msg = NULL;
    char ** result; //存放成功匹配结果的二维数组
    int row,col; //行，列
    int data =sqlite3_get_table(db, sql, &result, &row, &col,&msg);
    if(data<0)
    {
        printf("%s\n",msg);
    }
    if(row == 0) return 0;  //登录失败
    return 1;  //登录成功
}

int get_name_sqlite3(sqlite3 *db,char *name)  //从数据库匹配该用户--用于注册
{
    char sql[512];
    sprintf(sql,"SELECT * FROM user where name='%s';",name);
    char * msg = 0;
    char ** result; //存放成功匹配结果的二维数组
    int row,col; //行，列
    int data =sqlite3_get_table(db, sql, &result, &row, &col,&msg);
    if(data<0)
    {
        printf("%s\n",msg);
    }
    if(row == 0) return 0;  //匹配失败
    return 1;  //匹配成功
}

int get_friend_sqlite3(sqlite3 *db,char *username,char *friendname)  //从好友列表数据库匹配该用户--用于添加/删除好友
{
    char sql[512];
    sprintf(sql,"SELECT * FROM %s where name='%s';",username,friendname);
    char * msg = 0;
    char ** result; //存放成功匹配结果的二维数组
    int row,col; //行，列
    int data =sqlite3_get_table(db, sql, &result, &row, &col,&msg);
    if(data<0)
    {
        printf("%s\n",msg);
    }
    if(row == 0) return 0;  //匹配失败
    return 1;  //匹配成功
}

int get_group_sqlite3(sqlite3 *db,char *groupname)  //从聊天室列表中匹配是否存在该聊天室--用于加入/退出聊天室
{
    char sql[512];
    sprintf(sql,"SELECT * FROM 聊天室列表 where groupname='%s';",groupname);
    char * msg = 0;
    char ** result; //存放成功匹配结果的二维数组
    int row,col; //行，列
    int data =sqlite3_get_table(db, sql, &result, &row, &col,&msg);
    if(data<0)
    {
        printf("%s\n",msg);
    }
    if(row == 0) return 0;  //匹配失败
    return 1;  //匹配成功
}

int get_member_sqlite3(sqlite3 *db,char *groupname,char *username)  //从聊天室中匹配是否有该成员--用于加入聊天室/退出--用于广播
{
    char sql[512];
    sprintf(sql,"SELECT * FROM %s where member='%s';",groupname,username);
    char * msg = 0;
    char ** result; //存放成功匹配结果的二维数组
    int row,col; //行，列
    int data =sqlite3_get_table(db, sql, &result, &row, &col,&msg);
    if(data<0)
    {
        printf("%s\n",msg);
    }
    if(row == 0) return 0;  //匹配失败
    return 1;  //匹配成功
}



void server_denglu(int fd,MSG msg,sqlite3 *udb)
{
    USER *user = (USER *)msg.buf;  //得到客户发来的账号密码

    //回复客户端
    if(get_sqlite3(udb,user->name,user->passwd)) 
    {
        if(onlineCount == onlinemax)  msg.type = 99;
        else
        {
            if(online_user(user->name)==1)  //其他客户端没有登录该用户
            {
                msg.type = 100; //登录成功
                ouser[onlineCount].fd = fd;  //存在在线人数的数组中
                strcpy(ouser[onlineCount].name,user->name);
                onlineCount++;//在线人数加1
                printf("服务器当前在线人数 %d\n",onlineCount);
                printf("欢迎%d号客户端 %s用户进入服务器^-^\n",ouser[onlineCount-1].fd,ouser[onlineCount-1].name);
            }
            else msg.type = 98;
        }
    }
    else msg.type = 101;
    write(fd,&msg,sizeof(msg));
}

int online_user(char *name) //检查用户登录时是否已经在别的客户端登录 返回1说明没有
{
    for(int i=0;i<onlineCount;i++)
    {
        if(strcmp(name,ouser[i].name)==0)
        {
            return 0;
        }
    }
    return 1;
}

void server_zhuce(int fd,MSG msg,sqlite3 *udb)  //处理用户注册
{
    USER *user = (USER *)msg.buf;  //得到客户发来的账号密码

    if(get_name_sqlite3(udb,user->name)) msg.type = 104;     //已经有该用户名
    else 
    {
        //调用数据库的注册函数
        if(exec_zhuce(udb,*user)) msg.type = 103;  //注册成功;  
        else msg.type = 105; //加入数据库失败
    }
    write(fd,&msg,sizeof(msg));
}

void server_get_friend(int fd,MSG msg,sqlite3 *udb)  ////用户要查看他的全部好友
{
    char name[128];
    for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
    {
        if(fd == ouser[i].fd)
        {
            strcpy(name,ouser[i].name);
            break;
        }
    }
    char sql[512];
    sprintf(sql,"SELECT * FROM %s;",name);  //从name数据库中得到好友数据
    char *msg1 = 0;
    char **ppResult;//指向字符串数组的指针，每一个元素是一个字符串
    int row;//行
    int col;//列
    int data =sqlite3_get_table(udb, sql, &ppResult, &row, &col,&msg1);
    int valCount = row*col+col;
    memcpy(msg.buf,&valCount,sizeof(int));
    write(fd,&msg,sizeof(msg));
    for (int i = col; i < valCount; i++)
    {
        strcpy(msg.buf,ppResult[i]);
        write(fd,&msg,sizeof(msg));
    }
}

void server_add_friend(int fd,MSG msg,sqlite3 *udb)  //用户要添加好友
{
    char username[128],userwords[128];
    for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
    {
        if(fd == ouser[i].fd)
        {
            strcpy(username,ouser[i].name);
            break;
        }
    }

    //得到个性签名
    char sql1[512];
    sprintf(sql1,"SELECT * FROM user WHERE name='%s';",username);  //从数据库中得到数据
    char *msg1 = 0;
    char **ppResult;//指向字符串数组的指针，每一个元素是一个字符串
    int row;//行
    int col;//列
    sqlite3_get_table(udb, sql1, &ppResult, &row, &col,&msg1);
    strcpy(userwords,ppResult[5]);


    char friendname[128],friendwords[128];
    memcpy(friendname,msg.buf,sizeof(friendname));

    //得到个性签名
    char sql2[512];
    sprintf(sql2,"SELECT * FROM user WHERE name='%s';",friendname);  //从数据库中得到数据
    sqlite3_get_table(udb, sql2, &ppResult, &row, &col,&msg1);
    strcpy(friendwords,ppResult[5]);

    if(get_name_sqlite3(udb,friendname))  //找到要添加的用户
    {
        if(get_friend_sqlite3(udb,username,friendname)) msg.type = 312;  //已经加好友了
        else
        {
            char sql4[512];
            sprintf(sql4,"INSERT INTO %s VALUES ('%s','%s');",friendname,username,userwords);
            exec_sqlite3(udb,sql4);  //调用执行函数

            char sql5[512];
            sprintf(sql5,"INSERT INTO %s VALUES ('%s','%s');",username,friendname,friendwords);
            exec_sqlite3(udb,sql5);  //调用执行函数
            msg.type = 310;  //添加成功
        }
    }     
    else msg.type = 311;  //没找到
    write(fd,&msg,sizeof(msg));
}

void server_del_friend(int fd,MSG msg,sqlite3 *udb)  //用户要删除好友
{
    char username[128],friendname[128];
    for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
    {
        if(fd == ouser[i].fd)
        {
            strcpy(username,ouser[i].name);
            break;
        }
    }

    memcpy(friendname,msg.buf,sizeof(friendname));  //得到好友名

    if(get_friend_sqlite3(udb,username,friendname))  //说明是好友，可以删除
    {
        //删除user好友列表的friend
        char sql1[512];
        sprintf(sql1,"DELETE FROM %s WHERE name='%s';",username,friendname);
        exec_sqlite3(udb, sql1);  //调用执行函数

        //删除friend好友列表的user
        char sql2[512];
        sprintf(sql2,"DELETE FROM %s WHERE name='%s';",friendname,username);
        exec_sqlite3(udb, sql2);  //调用执行函数
        msg.type = 320;  //返回删除成功信息
    }
    else    msg.type = 321;  //说明好友列表没有该好友
    write(fd,&msg,sizeof(msg));
}

void server_onl_friend(int fd,MSG msg,sqlite3 *udb)  //用户要查看在线好友
{
    char username[128];
    for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
    {
        if(fd == ouser[i].fd)
        {
            strcpy(username,ouser[i].name);
            break;
        }
    }

    char sql[512];
    sprintf(sql,"SELECT name FROM %s;",username);  //从name数据库中得到好友用户名
    char *msg1 = 0;
    char **ppResult;//指向字符串数组的指针，每一个元素是一个字符串
    int row;//行
    int col;//列
    int data =sqlite3_get_table(udb, sql, &ppResult, &row, &col,&msg1);

    for (int i = col; i < row*col+col; i++)
    {
        //调用判断是否在线 在线会返回0
        if(online_user(ppResult[i])==0)
        {
            strcpy(msg.buf,ppResult[i]);
            write(fd,&msg,sizeof(msg));
        } 
    }
    //发送完毕后发送信号让客户端停止接收
    strcpy(msg.buf,"#-#");
    write(fd,&msg,sizeof(msg));
}

void server_cht_friend(int fd,MSG msg,sqlite3 *udb)  //用户要私聊好友
{
    ChatMsg cmsg = *(ChatMsg *)msg.buf;

    for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
    {
        if(fd == ouser[i].fd)
        {
            strcpy(cmsg.username,ouser[i].name);
            break;
        }
    }

    if(get_friend_sqlite3(udb,cmsg.username,cmsg.friendname))  //判断是否是好友
    {
        if(online_user(cmsg.friendname)==0)  //判断好友是否在线
        {
            int friendfd;
            for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户的套接字
            {
                if(strcmp(cmsg.friendname,ouser[i].name)==0)
                {
                    friendfd = ouser[i].fd;
                    break;
                }
            }
            //发送给好友
            strcpy(cmsg.friendname,cmsg.username);
            memcpy(msg.buf,&cmsg,sizeof(cmsg));
            msg.type = 340;

            write(friendfd,&msg,sizeof(msg));
            return;
        }
        else //好友不在线
        {
            //加入留言表格，回复客户端 
            msg.type = 341;

            //留言功能
            char *msg = 0;
            char sql[] = "CREATE TABLE 留言表(send varchar(255),recv varchar(255),txt varchar(255))";
            exec_sqlite3(udb,sql);  //调用执行函数，创建留言表

            //添加到留言表中
            char sql1[512];
            sprintf(sql1,"INSERT INTO 留言表 VALUES ('%s','%s','%s');",cmsg.username,cmsg.friendname,cmsg.txt);
            exec_sqlite3(udb,sql1);  //调用执行函数，插入留言表
        }
    }
    else msg.type = 342; //不是好友
    write(fd,&msg,sizeof(msg));
}

void server_look_words(int fd,MSG msg,sqlite3 *udb)  //用户要查看留言
{
    char username[128];
    for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
    {
        if(fd == ouser[i].fd)
        {
            strcpy(username,ouser[i].name);
            break;
        }
    }

    char sql[512];
    sprintf(sql,"SELECT * FROM 留言表 where recv='%s';",username);
    char ** ppResult;
    int row;//行
    int col;//列
    sqlite3_get_table(udb, sql, &ppResult, &row, &col,NULL);
    if(row == 0) 
    {
        msg.type = 350;  //没有新的留言
        write(fd,&msg,sizeof(msg));
    }
    else
    {
        msg.type = 351;
        ChatMsg cmsg;
        for (size_t i = col; i < row*col+col; i+=3)
        {
            strcpy(cmsg.friendname,ppResult[i]); //谁发给我的
            strcpy(cmsg.txt,ppResult[i+2]);  //内容
            memcpy(msg.buf,&cmsg,sizeof(cmsg));
            write(fd,&msg,sizeof(msg));  //发送给我

            //发送之后删除留言表中的记录
            char sql1[512];
            sprintf(sql1,"DELETE FROM 留言表 WHERE send='%s' and recv='%s' and txt='%s';",ppResult[i],ppResult[i+1],ppResult[i+2]);
            exec_sqlite3(udb,sql1);  //调用执行函数，删除留言表
        }
        //告诉客户端发完了
        strcpy(cmsg.username,"&-&");
        memcpy(msg.buf,&cmsg,sizeof(cmsg));
        write(fd,&msg,sizeof(msg));  //发送给我
    }
}

void server_create_group(int fd,MSG msg,sqlite3 *udb)  //用户要创建聊天室
{
    //首先创建一个聊天室列表，存放已经创建的聊天室
    char sql[] = "CREATE TABLE 聊天室列表(groupname varchar(255),count int)";
    exec_sqlite3(udb,sql);

    //创建聊天室成员表，并将聊天室名字加入聊天室列表，如果已经存在则不可以创建新的
    char groupsname[128];
    memcpy(groupsname,msg.buf,sizeof(groupsname)); //得到用户的聊天室名字

    char sql1[512],errbuf[128],*errmsg;
    sprintf(sql1,"CREATE TABLE %s(member varchar(255))",groupsname); //创建聊天室成员列表
    sprintf(errbuf,"table %s already exists",groupsname);  //检查是否已经创建

    sqlite3_exec(udb,sql1,0,0,&errmsg);
    if(errmsg != 0)
    {
        if (strcmp(errbuf, errmsg) != 0)  //说明已经被创建，返回客户提示
        {
            msg.type = 401;
        }
        else //有别的错误
        {
            msg.type = 402;
            printf("%s\n",errmsg);
        }
    }
    else  //可以创建，加入聊天室列表，并且把创建者加入聊天室
    {
        //加入聊天室列表
        char s1[512];
        sprintf(s1,"INSERT INTO 聊天室列表 VALUES ('%s','1');",groupsname);
        exec_sqlite3(udb,s1);

        //得到创建者名字
        char username[128];
        for(int i=0;i<onlineCount;i++)
        {
            if(fd == ouser[i].fd)
            {
                strcpy(username,ouser[i].name);
                break;
            }
        }

        //创建者加入聊天室
        char s2[512];
        sprintf(s1,"INSERT INTO %s VALUES ('%s');",groupsname,username);
        exec_sqlite3(udb,s1);

        //创建用户的聊天室列表
        char s3[512];
        sprintf(s3,"CREATE TABLE %s_addGroup(groupname varchar(255))",username); //创建聊天室成员列表
        exec_sqlite3(udb,s3); //执行数据库语句

        //聊天室加入创建者的聊天室表
        //插入用户的聊天室列表
        char s4[512];
        sprintf(s4,"INSERT INTO %s_addGroup VALUES ('%s');",username,groupsname);
        exec_sqlite3(udb,s4); //执行数据库语句

        msg.type = 400; //创建成功
    }
    write(fd,&msg,sizeof(msg));
}

void server_look_group(int fd,MSG msg,sqlite3 *udb)  //用户要浏览聊天室
{
    //先把存在的聊天室列表发给用户
    char sql[]="SELECT * FROM 聊天室列表;";  //从聊天室列表中得到聊天室数据
    char *msg1 = 0;
    char **ppResult;//指向字符串数组的指针，每一个元素是一个字符串
    int row;//行
    int col;//列
    int data =sqlite3_get_table(udb, sql, &ppResult, &row, &col,&msg1);
    int valCount = row*col+col;  //发送数量
    memcpy(msg.buf,&valCount,sizeof(int));
    write(fd,&msg,sizeof(msg));
    for (int i = col; i < valCount; i++)
    {
        strcpy(msg.buf,ppResult[i]);
        write(fd,&msg,sizeof(msg));
    }
}

void server_join_group(int fd,MSG msg,sqlite3 *udb)  //用户要加入聊天室
{
    char username[128];
    for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
    {
        if(fd == ouser[i].fd)
        {
            strcpy(username,ouser[i].name);
            break;
        }
    }

    char groupname[128]; //得到要加入的聊天室名字
    memcpy(groupname,msg.buf,sizeof(groupname));

    //先判断要加入的聊天室是否存在
    if(get_group_sqlite3(udb,groupname))
    {
        if(get_member_sqlite3(udb,groupname,username)==0)  //判断是否已经加入聊天室，不能重复加入
        {
            //把用户插入聊天室成员表
            char s1[512],s2[512],s3[512],s4[512];
            sprintf(s1,"INSERT INTO %s VALUES ('%s');",groupname,username);
            exec_sqlite3(udb,s1); //执行数据库语句

            //更新聊天室列表
            sprintf(s2,"UPDATE 聊天室列表 set count = count+1 where groupname = '%s'",groupname);
            exec_sqlite3(udb,s2); //执行数据库语句

            //创建用户的聊天室列表
            sprintf(s3,"CREATE TABLE %s_addGroup(groupname varchar(255))",username); //创建聊天室成员列表
            exec_sqlite3(udb,s3); //执行数据库语句

            //插入用户的聊天室列表
            sprintf(s4,"INSERT INTO %s_addGroup VALUES ('%s');",username,groupname);
            exec_sqlite3(udb,s4); //执行数据库语句

            msg.type = 420;
        }
        else msg.type = 422;
    }
    else msg.type = 421;
    write(fd,&msg,sizeof(msg));
}

void server_useradd_group(int fd,MSG msg,sqlite3 *udb)  //用户要查看加入的聊天室
{
    char username[128];
    for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
    {
        if(fd == ouser[i].fd)
        {
            strcpy(username,ouser[i].name);
            break;
        }
    }
    //先把存在的聊天室列表发给用户
    char sql[512];
    sprintf(sql,"SELECT * FROM %s_addGroup;",username);  //用户的聊天室表
    char *msg1 = 0;
    char **ppResult;//指向字符串数组的指针，每一个元素是一个字符串
    int row;//行
    int col;//列
    int data =sqlite3_get_table(udb, sql, &ppResult, &row, &col,&msg1);
    int valCount = row*col+col;  //发送数量
    memcpy(msg.buf,&valCount,sizeof(int));
    write(fd,&msg,sizeof(msg));
    for (int i = col; i < valCount; i++)
    {
        strcpy(msg.buf,ppResult[i]);
        write(fd,&msg,sizeof(msg));
    }
}

void server_send_group(int fd,MSG msg,sqlite3 *udb)  //用户要聊天室聊天
{
    ChatGroup cmsg = *(ChatGroup *)msg.buf;
    char username[128];

    for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
    {
        if(fd == ouser[i].fd)
        {
            strcpy(username,ouser[i].name);
            break;
        }
    }

    //判断聊天室是否存在
    if(get_group_sqlite3(udb,cmsg.groupname)==1)
    {
        //判断是否是聊天室成员
        if(get_member_sqlite3(udb,cmsg.groupname,username)==1)
        {
            char s1[512];
            sprintf(s1,"SELECT * FROM %s;",cmsg.groupname);//查询语句

            char ** ppResult;
            int row;//行
            int col;//列
            sqlite3_get_table(udb, s1, &ppResult, &row, &col,NULL);
            for (int i = col; i < row*col+col; i++)
            {
                if(online_user(ppResult[i])==0)  //判断成员是否在线
                {
                    int friendfd;
                    for(int j=0;j<onlineCount;j++)  //找到用户的信息，得到用户的套接字
                    {
                        if(strcmp(ppResult[i],ouser[j].name)==0)
                        {
                            friendfd = ouser[j].fd;
                            break;
                        }
                    }
                    if(friendfd != fd)
                    {
                        //发送给其他在线成员
                        msg.type = 432;
                        strcpy(cmsg.username,username);
                        memcpy(msg.buf,&cmsg,sizeof(cmsg));
                        write(friendfd,&msg,sizeof(msg));
                    }
                }
            }
        }
        else  //返回用户消息
        {
            msg.type = 431;
            write(fd,&msg,sizeof(msg));
        }
    }
    else  //返回用户消息
    {
        msg.type = 430;
        write(fd,&msg,sizeof(msg));
    }
}

void server_quit_group(int fd,MSG msg,sqlite3 *udb)  //用户要退出聊天室
{
    char username[128];
    for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
    {
        if(fd == ouser[i].fd)
        {
            strcpy(username,ouser[i].name);
            break;
        }
    }

    char groupname[128]; //得到要退出的聊天室名字
    memcpy(groupname,msg.buf,sizeof(groupname));

    //先判断要退出的聊天室是否存在
    if(get_group_sqlite3(udb,groupname))
    {
        if(get_member_sqlite3(udb,groupname,username)==1)  //判断是否已经加入聊天室
        {
            //更新聊天室成员表
            char s1[512],s2[512],s4[512];
            sprintf(s1,"DELETE FROM %s WHERE member='%s';",groupname,username);
            exec_sqlite3(udb,s1); //执行数据库语句

            //更新聊天室列表
            sprintf(s2,"UPDATE 聊天室列表 set count = count-1 where groupname = '%s'",groupname);
            exec_sqlite3(udb,s2); //执行数据库语句

            //更新用户的聊天室列表
            sprintf(s4,"DELETE FROM %s_addGroup WHERE groupname='%s';",username,groupname);
            exec_sqlite3(udb,s4); //执行数据库语句

            msg.type = 442;
        }
        else msg.type = 441;  //根本就没加入聊天室
    }
    else msg.type = 440;
    write(fd,&msg,sizeof(msg));
}

void server_get_user(int fd,MSG msg,sqlite3 *udb)  //用户要查看个人信息
{
    UserMess umsg;
    for(int i=0;i<onlineCount;i++)
    {
        if(fd == ouser[i].fd)
        {
            strcpy(umsg.username,ouser[i].name);
            break;
        }
    }
    char s1[512];
    sprintf(s1,"SELECT * FROM user where name='%s';",umsg.username);
    char ** ppResult;
    int row;//行
    int col;//列
    sqlite3_get_table(udb, s1, &ppResult, &row, &col,NULL);
    strcpy(umsg.words,ppResult[col+2]);
    memcpy(msg.buf,&umsg,sizeof(umsg));
    write(fd,&msg,sizeof(msg));
}

void server_change_words(int fd,MSG msg,sqlite3 *udb)  //用户要修改个性签名
{
    char username[128];
    for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
    {
        if(fd == ouser[i].fd)
        {
            strcpy(username,ouser[i].name);
            break;
        }
    }
    char words[128];
    memcpy(words,msg.buf,sizeof(words)); //得到新的个性签名
    char s1[512];
    sprintf(s1,"UPDATE user SET words='%s' WHERE name='%s';",words,username);
    exec_sqlite3(udb,s1);  //执行数据库操作
    msg.type = 610;
    write(fd,&msg,sizeof(msg));
}

void server_change_pass(int fd,MSG msg,sqlite3 *udb)  //用户要修改密码
{
    char username[128];
    for(int i=0;i<onlineCount;i++)  //找到用户的信息，得到用户名
    {
        if(fd == ouser[i].fd)
        {
            strcpy(username,ouser[i].name);
            break;
        }
    }
    char pass[128];
    memcpy(pass,msg.buf,sizeof(pass)); //得到新的密码
    char s1[512];
    sprintf(s1,"UPDATE user SET password='%s' WHERE name='%s';",pass,username);
    exec_sqlite3(udb,s1);  //执行数据库操作
    msg.type = 620;
    write(fd,&msg,sizeof(msg));
}

void server_user_zhuxiao(int fd,MSG msg,sqlite3 *udb)  //用户要注销账号
{
    USER umsg = *(USER *)msg.buf;
    for(int i=0;i<onlineCount;i++)
    {
        if(fd == ouser[i].fd)
        {
            strcpy(umsg.name,ouser[i].name);
            break;
        }
    }
    if(get_sqlite3(udb,umsg.name,umsg.passwd)==1)  //说明账号密码正确
    {
        //调用数据库注销函数
        exec_zhuxiao(udb,umsg.name);
        msg.type = 540;
    }
    else msg.type = 530;
    write(fd,&msg,sizeof(msg));
}