//客户端
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

int my_recv(int conn_fd, char *data_buf, int len)
{
    static char recv_buf[MAXLINE]; // 自定义缓冲区，MAXLINE定义在my_recv.h中
    static char *pread;            // 指向下一次读取数据的位置
    static int len_remain = 0;     // 自定义缓冲区中剩余字节数
    int i;

    // 如果自定义缓冲区中没有数据，则从套接字读取数据
    if (len_remain <= 0)
    {
        if ((len_remain = recv(conn_fd, recv_buf, sizeof(recv_buf), 0)) < 0)
        {
            perror("recv");
        }
        else if (len_remain == 0)
        { // 目的计算机端的socket连接关闭
            return 0;
        }
        pread = recv_buf; // 重新初始化pread指针
    }

    // 从自定义缓冲区中读取一次数据
    for (i = 0; *pread != '\n'; i++)
    {
        if (i > len)
        { // 防止指针越界
            return -1;
        }
        data_buf[i] = *pread++;
        len_remain--;
    }

    // 去除结束标志
    len_remain--;
    pread++;

    return i; // 读取成功
}

void input_userinfo(int sockfd, const char *prompt)
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

        //将读取到的用户信息发送给服务器
        if (send(sockfd, input_buf, strlen(input_buf), 0) < 0)
        {
            printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        }

        //从套接字上读取一次数据
        if (my_recv(sockfd, recv_buf, sizeof(recv_buf)) < 0)
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

void send_msg(int sockfd)
{
    char input_buf[MAXLINE];
    printf("send msg:");

    //获取用户输入存放到input_buf，并做相应处理
    cin >> input_buf;
    int len = strlen(input_buf);
    input_buf[len++] = '\n';
    input_buf[len] = '\0';

    //将读取到的用户信息发送给服务器
    if (send(sockfd, input_buf, strlen(input_buf), 0) < 0)
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
        printf("usage: ./client <ipaddress>\n");
        exit(0);
    }

    char *serv_sinaddr = argv[1];
    // while (true)
    // {

    //初始化服务器段地址结构
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    if (inet_pton(AF_INET, serv_sinaddr, &servaddr.sin_addr) <= 0)
    {
        printf("inet_pton error for %s\n", argv[1]);
        exit(0);
    }

    //创建TCP套接字
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    //向服务端发送连接请求
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    //输入用户名和密码
    input_userinfo(sockfd, "username");
    input_userinfo(sockfd, "password");

    //读取登录成功信息并打印出来

    if((recvbytes = my_recv(sockfd, recvline, sizeof(recvline)))<0)
    {
        perror("recv error");
        exit(1);
    }
    for(int i=0;i<recvbytes;i++)
        printf("%c",recvline[i]);
    printf("\n");

    //登录成功后发送消息
    send_msg(sockfd);

    close(sockfd);
    // }
    // exit(0);

    return 0;
}