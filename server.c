/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: 2016年06月02日 星期四 17时00分22秒
 ************************************************************************/

#include<stdio.h>
#include <string.h>
#include "stdlib.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "unistd.h"
#include "pthread.h"

#define PORT 8080
#define client_maxnu 8
int connect_client[client_maxnu];
int connect_number;

void *client_connect_handle(void *vargp);
void *data_handle_func(void *vargp);
void *data_send_handle_func(void *vargp);
void client_info_translate(int *info);

struct user_t{
    char user_name[20];
    char who[20];
    char msg_content[256];
};

struct user_t user;

void server_creat_func(void)
{
    int server_fd;
    //int *clientfdp;
    int res;
    socklen_t addrlen = sizeof(struct sockaddr);
    struct sockaddr_in server_addr;
   // struct sockaddr_in client_addr;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    res = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    if(res < 0)
    {
        printf("bind error!\n");
    }
    
    res = listen(server_fd, 8);
    if(res < 0)
    {
        printf("listen error!\n");
    }

    char val = 1;
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&val,1);

    pthread_t client_handle;
    pthread_t data_handle;
    pthread_t data_send;
    pthread_create(&client_handle, NULL, client_connect_handle, &server_fd);
    pthread_create(&data_handle, NULL, data_handle_func, NULL);
    pthread_create(&data_send, NULL, data_send_handle_func, NULL);
    while(1);
}


void *client_connect_handle(void *vargp)
{
    int connfd = *((int *)vargp);
    struct sockaddr_in client_addr;
    while(1)
    {
        int i;
        int sc;
        int size = sizeof(client_addr);
        sc = accept(connfd, (struct sockaddr *)&client_addr, &size);
        if(sc >= 0)
        {
            printf("A client connect:%d\n", sc);
        }
        for(i = 0; i<client_maxnu; i++)
        {
            if(connect_client[i] == -1)
            {
                connect_client[i] = sc;
                connect_number++;
                printf("%d client has been connected\n", connect_number);
                break;
            }
        }
    }
}

int data_length;
char data_recv_buff[256];
char sender_name[20];

void *data_handle_func(void *vargp)
{
    int i;
    int maxfd;
    fd_set scanfd;
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int err = -1;
    while(1)
    {
        maxfd = -1;
        FD_ZERO(&scanfd);
        for(i = 0; i< client_maxnu; i++)
        {
            if(connect_client[i] != -1)
            {
                FD_SET(connect_client[i], &scanfd);
                if(maxfd < connect_client[i])
                {
                    maxfd = connect_client[i];
                }
            }
        }

        err = select(maxfd + 1, &scanfd, NULL, NULL, &timeout);
        switch(err)
        {
            case 0:
            break;
            case -1:
            break;
            default :
            if(connect_number <= 0)
            {
                break;
            }
            for(i = 0; i < client_maxnu; i++)
            {
                if(connect_client[i] != -1)
                {
                    if(FD_ISSET(connect_client[i], &scanfd))
                    {
                        data_length = recv(connect_client[i], &user,sizeof(struct user_t), 0);
                        if(data_length > 0)
                        {
                            strcpy(data_recv_buff, user.msg_content);
                            strcpy(sender_name, user.user_name);
                            printf("msg from %s:\n%s\n", sender_name,data_recv_buff);
                        }                     
                    }
                }    // send(connect_client[i], data_recv_buff, 256, 0);
            }
            break;
        }
        
    }

}

void *data_send_handle_func(void *arpa)
{
    int i;
    while(1)
    {
        if(data_length <= 0)
        {
            continue;
        }
        if(data_length > 0)
        {
            for(i = 0; i < client_maxnu; i++)
            {
                if(connect_client[i] != -1)
                {
                    send(connect_client[i], &user, sizeof(struct user_t), 0);
                    printf("转发成功\n");
                }
            }
            data_length = 0;
        }
    }
}

int main(void)
{
    memset(connect_client, -1, client_maxnu);   
    server_creat_func();
}

