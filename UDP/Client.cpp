//UDP客户端
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 4096
using namespace std;

//获取用户信息：用户名和密码
void input_userinfo(int sockfd, struct sockaddr_in servaddr,const char *prompt)
{
    char input_buf[32];
    char recv_buf[MAXLINE];
    int flag;
    int count = 0;
    do
    {
        
        printf("%s:", prompt);

        //获取用户输入存放到input_buf，并做相应处理
        cin >> input_buf;
        int len = strlen(input_buf);
        input_buf[len++] = '\n';
        input_buf[len] = '\0';

        socklen_t sin_size = sizeof(struct sockaddr_in);        
        
        //将读取到的用户信息发送给服务器
        if (sendto(sockfd, input_buf, strlen(input_buf), 0,(struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        }

        //从套接字上读取一次数据
        if (recvfrom(sockfd, recv_buf, sizeof(recv_buf),0,(struct sockaddr *)&servaddr, &sin_size) < 0)
        {
            printf("recv msg error: %s(errno: %d)\n", strerror(errno), errno);
        }

        if (recv_buf[0] == 'y')
            flag = 0;
        else
        {
            count++;
            if (count >= 3)
            {
                printf("input error for 3 times, exit!\n");
                exit(-1);
            }
            printf("%s error, input again!\n", prompt);
            flag = 1;
        }
    } while (flag == 1);
}

//给服务端发送消息
void send_msg(int sockfd,struct sockaddr_in servaddr)
{
    char input_buf[MAXLINE];
    cout<<"send msg:";
    getchar();
    int i=0;
    //获取用户输入存放到input_buf，并做相应处理
    while((input_buf[i]=getchar())!='\n')
        i++;
    input_buf[i]='\n';
    input_buf[++i]='\0';
    int len = strlen(input_buf);

    //将读取到的用户信息发送给服务器
    if (sendto(sockfd, input_buf, strlen(input_buf), 0,(struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
    }
}

int main(int argc, char **argv)
{

    int sockfd, recvbytes;
    char recvline[4096], sendline[4096];
    struct sockaddr_in servaddr;

    if (argc != 2)
    {
        printf("usage: ./c.out <ipaddress>\n");
        exit(0);
    }

    char *serv_sinaddr = argv[1];

    //初始化服务器段地址结构
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    if (inet_pton(AF_INET, serv_sinaddr, &servaddr.sin_addr) <= 0)
    {
        printf("inet_pton error for %s\n", argv[1]);
        exit(0);
    }

    //创建UDP套接字
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    //输入用户名和密码
    input_userinfo(sockfd, servaddr, "username");
    input_userinfo(sockfd, servaddr, "password");

    //读取登录成功信息并打印出来 
    socklen_t sin_size = sizeof(struct sockaddr_in);        
    if((recvbytes = recvfrom(sockfd, recvline, sizeof(recvline),0,(struct sockaddr *)&servaddr, &sin_size))<0)
    {
        perror("recv error");
        exit(1);
    }
    for(int i=0;i<recvbytes;i++)
        printf("%c",recvline[i]);
    printf("\n");

    //登录成功后发送消息
    send_msg(sockfd,servaddr);

    close(sockfd);

    return 0;
}