//UDP服务端
/* 无连接的客户/服务器程序的在原理上和连接的客户/服务器是一样的，
    两者的区别在于:
    1.无连接的客户/服务器中的客户一般不需要建立连接，
    2.而且在发送接收数据时，需要指定远端机的地址。 */

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
#define SERVPORT 6666
#define USERNUM 2

//用户信息：用户名和密码
struct userinfo
{
    char username[32];
    char password[32];
};
struct userinfo users[] = {
    {"wsg", "123"},
    {"qqq", "456"}};

//验证用户名是否存在
int check_name(const char *name)
{
    int i;

    if (name == NULL)
    {
        printf("in check_name, NULL pointer");
        return -2;
    }
    //用户名存在，返回该用户在结构体users中的下标
    for (i = 0; i < USERNUM; i++)
        if (strcmp(name, users[i].username) == 0)
            return i;

    //用户名不存在
    return -1;
}

//向用户端发送数据
void send_data(int sockfd, struct sockaddr_in remote_addr, const char *string)
{
    if (sendto(sockfd, string, strlen(string), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
        perror("send error");
}

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;    //服务端地址信息
    struct sockaddr_in remote_addr; //客户端地址信息
    char recvline[4096], sendline[4096];
    char recv_buf[MAXLINE];
    int recvbytes;
    int flag = 0; //0: username; 1: password
    int user_num;
    bool LOGGED_IN;

    //创建socket套接字
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    //设置该套接字使之可以重新绑定端口
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(int));

    //初始化服务器端地址结构
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVPORT);

    //绑定，bind()函数把一个地址族中的特定地址赋给socket
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    socklen_t sin_size = sizeof(struct sockaddr_in);
    while (1)
    {
        LOGGED_IN = false;

        while (1)
        {
            //接受客户端发送的用户信息
            memset(recv_buf, 0, sizeof(recv_buf));
            if ((recvbytes = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&remote_addr, &sin_size)) < 0)
            {
                perror("recv error");
                exit(1);
            }
            recv_buf[recvbytes - 1] = '\0';
            printf("recv userinfo: %s\n", recv_buf);

            if (flag == 0)
            {
                user_num = check_name(recv_buf);
                switch (user_num)
                {
                case -1:
                    sendto(sockfd, "n", strlen("n"), 0, (struct sockaddr *)&remote_addr, sin_size);
                    break;
                case -2:
                    exit(1);
                    break;
                default:
                    sendto(sockfd, "y", strlen("y"), 0, (struct sockaddr *)&remote_addr, sin_size);
                    flag = 1;
                    break;
                }
            }
            else if (flag == 1)
            {
                if (strcmp(users[user_num].password, recv_buf) == 0)
                {
                    send_data(sockfd, remote_addr, "y\n");
                    send_data(sockfd, remote_addr, "Login Success!\n");
                    printf("%s login\n", users[user_num].username);
                    LOGGED_IN = true;
                    break;
                }
                else
                    sendto(sockfd, "n", strlen("n"), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
            }
        }

        if (LOGGED_IN == true)
        {
            //接受客户端发送的消息
            memset(recvline, 0, sizeof(recvline));
            if ((recvbytes = recvfrom(sockfd, recvline, sizeof(recvline), 0, (struct sockaddr *)&remote_addr, &sin_size)) < 0)
            {
                perror("recv error");
                exit(1);
            }
            recvline[recvbytes - 1] = '\0';
            printf("recv msg: %s\n", recvline);
        }
        close(sockfd);
        exit(0);
    }
    return 0;
}
