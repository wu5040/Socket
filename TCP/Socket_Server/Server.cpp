//服务器端
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
    for (i = 0; i < 2; i++)
        if (strcmp(name, users[i].username) == 0)
            return i;

    //用户名不存在
    return -1;
}

//发送数据
void send_data(int sockfd,const char* string)
{
    if(send(sockfd,string,strlen(string),0)<0)
        perror("send error");
}

int main(int argc, char **argv)
{
    int listenfd, connfd;           //listenfd:监听socket;connfd: 数据传输socket
    struct sockaddr_in servaddr;    //服务端地址信息
    struct sockaddr_in remote_addr; //客户端地址信息
    char recvline[4096], sendline[4096];
    char recv_buf[MAXLINE];
    int recvbytes;
    int flag = 0; //0: username; 1: password
    int user_num;
    bool LOGGED_IN;
    pid_t pid;

    //创建socket套接字
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    //设置该套接字使之可以重新绑定端口
    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(int));

    //初始化服务器端地址结构
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVPORT);

    //绑定，bind()函数把一个地址族中的特定地址赋给socket
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    //调用listen()来监听这个socket
    //如果客户端这时调用connect()发出连接请求，服务器端就会接收到这个请求。
    if (listen(listenfd, 10) == -1)
    {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    socklen_t sin_sinze = sizeof(struct sockaddr_in);
    while (1)
    {
        LOGGED_IN=false;
        //accept接受客户端的连接请求，并返回套接字用于收发数据
        if ((connfd = accept(listenfd, (struct sockaddr *)&remote_addr, &sin_sinze)) == -1)
        {
            printf("accept socket error: %s(errno: %d)", strerror(errno), errno);
            continue;
        }

        printf("accept a connection from %s\n", inet_ntoa(remote_addr.sin_addr));

        //创建一个子进程
        if ((pid = fork()) == 0)
        {
            while (1)
            {
                //接受客户端发送的用户信息
                memset(recv_buf, 0, sizeof(recv_buf));
                if ((recvbytes = recv(connfd, recv_buf, sizeof(recv_buf), 0)) < 0)
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
                        send(connfd, "n", strlen("n"), 0);
                        break;
                    case -2:
                        exit(1);
                        break;
                    default:
                        send(connfd, "y", strlen("y"), 0);
                        flag = 1;
                        break;
                    }
                }
                else if (flag == 1)
                {
                    if (strcmp(users[user_num].password, recv_buf) == 0)
                    {
                        send_data(connfd,"y\n");
                        send_data(connfd,"Login Success!\n");
                        // send(connfd, "y", strlen("y"), 0);
                        // send(connfd, "Login Success!\n", strlen("Login Success!\n"), 0);
                        printf("%s login\n", users[user_num].username);
                        LOGGED_IN = true;
                        break;
                    }
                    else
                        send(connfd, "n", strlen("n"), 0);
                }
            }

            if (LOGGED_IN == true)
            {
                //接受客户端发送的消息
                memset(recvline, 0, sizeof(recvline));
                if ((recvbytes = recv(connfd, recvline, sizeof(recvline), 0)) < 0)
                {
                    perror("recv error");
                    exit(1);
                }
                recvline[recvbytes - 1] = '\0';
                printf("recv msg: %s\n", recvline);
            }
            close(listenfd);
            close(connfd);
            exit(0);
        }
        else
            close(connfd);
    }

    return 0;
}