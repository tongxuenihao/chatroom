/*************************************************************************
	> File Name: clent.c
	> Author: 
	> Mail: 
	> Created Time: 2016年06月01日 星期三 11时27分44秒
 ************************************************************************/

#include<stdio.h>
#include "stdlib.h"
#include "string.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "unistd.h"
#include "pthread.h"


#define PORT 8080

void *data_send_handle(void *arpa);
void *data_recv_handle(void *arpa);
void data_handle_func(int *info);

struct user_t{
    char user_name[20];
    char who[20];
    char content[256];
};

char name[20];
static struct user_t user;

void user_rejister(void)
{
    int res;
    printf("enter your nickname:\n");
    fgets(name, 20, stdin);
    res = strlen(name);
    if (name[res - 1] == '\n') {
        name[res - 1] = '\0';
    }
    printf("your nickname is %s\n", name);
    strcpy(user.user_name, name);
}

void server_connect_func(void)
{
    int *clientfdp;
    int res;
    struct sockaddr_in server_addr;
    clientfdp = (int *)malloc(sizeof(int)); 
    *clientfdp = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);
   
    res = connect(*clientfdp, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    printf("loading...\n");
    if(res < 0)
    {
        printf("loading error!\n");
    }
    else{
        printf("loading success!\n");
    }
    data_handle_func(clientfdp);
    while(1);
 //   sleep(100);
}


void data_handle_func(int *info)
{
    pthread_t data_send;
    pthread_t data_recv;
    pthread_create(&data_send, NULL, data_send_handle, info);
    pthread_create(&data_recv, NULL, data_recv_handle, info);
}

void *data_send_handle(void *vargp)
{
    int connfd = *((int *)vargp); 
    int data_length;
    char data_send_buff[256];
    char send_to_who[20];
    printf("who do you want to chat:\n");
    fgets(send_to_who, 20, stdin);

    int res;

    res = strlen(send_to_who);
    if (send_to_who[res - 1] == EOF) {
        send_to_who[res - 1] = '\0';
    }
    strcpy(user.who, send_to_who);
    while(1)
    {
       // printf("%s:", user.user_name);
       // fflush(stdout);
        fgets(data_send_buff, 256, stdin);
        res = strlen(data_send_buff);
        if (data_send_buff[res - 1] == EOF) 
        {
            data_send_buff[res - 1] = '\0';
        }
        strcpy(user.content, data_send_buff);
        data_length = send(connfd, &user,sizeof(struct user_t), 0);
        if(data_length < 0)
        {
            printf("已断开连接，请重新登陆\n");
        }
    }
}

void *data_recv_handle(void *vargp)
{
    int connfd = *((int *)vargp); 
    char data_recv_buff[256];
    int data_length;
    struct user_t p_user;
    int ret;
    while(1)
    {
        data_length = recv(connfd, &p_user, sizeof(struct user_t), 0);
        //printf("%s\n",user.user_name);
        //printf("%s\n",p_user.user_name);
        if(data_length > 0)
        {
            if((ret = strcmp(user.user_name, p_user.user_name)) != 0)
            {
                printf("\t\t\t%s @  ", p_user.user_name);
                printf("%s",p_user.content);
            }

            printf("cmp ret : %d\n",ret);
        }
        data_length = 0;
    }
}

int main(void)
{
    user_rejister();
    server_connect_func();
}
