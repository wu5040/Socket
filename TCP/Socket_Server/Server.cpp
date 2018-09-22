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

int main(int argc, char **argv)
{
    int listenfd, connfd;           //listenfd:监听socket;connfd: 数据传输socket
    struct sockaddr_in servaddr;    //服务端地址信息
    struct sockaddr_in remote_addr; //客户端地址信息
    char recvline[4096], sendline[4096];
    int recvbytes;

    //创建socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    else
        printf("create socket success\n");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVPORT);

    //bind()函数把一个地址族中的特定地址赋给socket
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    else
        printf("bind socket success\n");

    //调用listen()来监听这个socket
    //如果客户端这时调用connect()发出连接请求，服务器端就会接收到这个请求。
    if (listen(listenfd, 10) == -1)
    {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    else
        printf("listen socket......\n");

    printf("======waiting for client's request======\n");
    while (1)
    {
        socklen_t sin_sinze = sizeof(struct sockaddr_in);
        if ((connfd = accept(listenfd, (struct sockaddr *)&remote_addr, &sin_sinze) == -1))
        {
            printf("accept socket error: %s(errno: %d)", strerror(errno), errno);
            continue;
        }

        printf("received a connection from %s\n", inet_ntoa(remote_addr.sin_addr));

        recvbytes = recv(connfd, recvline, MAXLINE, 0);
        if (recvline != "")
        {
            recvline[recvbytes] = '\0';
            printf("recv msg from client: %s\n", recvline);
        }

        close(connfd);
    }

    close(listenfd);

    getchar();

    return 0;
}