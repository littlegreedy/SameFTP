#include <stdio.h>
#include <stdint.h>
#include<string.h>
#include<stdlib.h>
#include "ftp.h"
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

int LoginCheck(char *name,char *password)
{
	if((strcmp(name,"cap")==0)&&(strcmp(password,"cap")==0))
		return 0;
	else if((strcmp(name,"csd")==0)&&(strcmp(password,"csd")==0))
		return 0;
	else
        return -1;
}

static int send_until_all(int sockfd, void *data, int len)
{
	if(sockfd==-1)
		return -1;
	
	int bytes_left;
	int send_bytes;
	char *ptr;
	
	bytes_left=len;
	ptr=data;
	while(bytes_left>0)
	{
		send_bytes=send(sockfd,ptr,bytes_left,0);
		if(send_bytes<=0)
			return -1;
		bytes_left=bytes_left-send_bytes;
		ptr=ptr+send_bytes;
	}
	return 0;
}

static int send_head(int sockfd, struct FTP_head *head)
{
	if(sockfd==-1)
		return -1;
	
    uint32_t data[2];
	data[0]=htonl(head->type);
	data[1]=htonl(head->len);
	
	int length=sizeof(uint32_t)*2;
	return send_until_all(sockfd,data,length);
}

int FTP_send_msg(int sockfd, struct FTP_msg *msg)
{
	if(sockfd ==-1)
		return -1;
		
	
    if(send_head(sockfd,&msg->head)==-1)
		return -1;
	
	if(msg->head.len!=0)
		if(send_until_all(sockfd, msg->data,msg->head.len)==-1)
			return -1;

	return 0;
}

static int recv_until_all(int sockfd, void *buf, int len)
{
	if(sockfd==-1)
		return -1;
	int bytes_left;
	int recv_bytes;
	char *ptr;
	
	bytes_left=len;
	ptr=buf;
	while(bytes_left>0)
	{
		recv_bytes=recv(sockfd,ptr,bytes_left,0);
		if(recv_bytes<0)
			return -1;
		else if(recv_bytes==0)
			break;
		bytes_left=bytes_left-recv_bytes;
		ptr=ptr+recv_bytes;
	}
	return 0;
}

//recv的封装
int FTP_recv_msg(int sockfd, struct FTP_msg *msg)
{
	uint32_t data[2];
	if(sockfd==-1){
		return -1;
	}
	msg->data = NULL;
	int length=sizeof(uint32_t)*2;
	if(recv_until_all(sockfd, data,length)==-1){
			return -1;
	}
	msg->head.type=ntohl(data[0]);
	msg->head.len=ntohl(data[1]);
	
	if(msg->head.len==0){
		msg->data=NULL;
	}
	else{
		msg->data=malloc(msg->head.len);
		if(!msg->data){
			return -1;
			}
		if(recv_until_all(sockfd, msg->data,msg->head.len)==-1){
			free(msg->data);
			return -1;
		}
	}
	memset(data,0,sizeof(data));

	return 0;
}

static int send_end(int sockfd) {
	struct FTP_msg msg;

	msg.head.type = END;
	msg.head.len = 0;
	msg.data = NULL;

	if (FTP_send_msg(sockfd, &msg) == -1) {
		printf("连接断开\n");
		return -1;
	}

	return 0;
}

int FTP_upload(int sockfd, FILE *fp, int mode)
{
	
	if (fp == NULL || sockfd == -1) {
        	printf("文件句柄错误或套接字句柄错误\n");
        	return -1;
	}

	int MAXLEN = 1024;
	char * buf= malloc(MAXLEN);
	int rc;

	if (buf == NULL) {
        printf("malloc 分配空间失败\n");
       	return -1;
    } 
	struct FTP_msg msg;

    switch (mode) {
        case 0:
			while ((rc = fread(buf, sizeof(char), MAXLEN, fp)) != 0) {
                msg.data = buf;
                msg.head.len = rc;

                FTP_send_msg(sockfd, &msg);
			}
            
			if (send_end(sockfd) != 0){
						return -1;
			}else{
				printf("文件发送结束\n");
			}

			break;
        
		default:
			printf("mode 参数错误！\n");

			return -1;

	}
	return 0;
}


int FTP_download(int sockfd, FILE * fp, int mode) {

	if (fp == NULL || sockfd == -1) {
		fprintf(stderr, "文件句柄错误或套接字句柄错误\n");
		return -1;
	}
	int MAXLEN = 1024;
	char * buf= malloc(MAXLEN);
	int length;
	struct FTP_msg msg;
	switch (mode) {
	case 0:

		for(;;){
			int count;
        	        
			if(FTP_recv_msg(sockfd,&msg)==-1){
				printf("连接断开。。。\n");
				return -1;
			}	
			
			length=msg.head.len;
			buf=msg.data;
			count = fwrite(buf,sizeof(char),length,fp);
			if(msg.head.type == END){
				return 0;
			}
		}
		fclose(fp);
		break;

	default:
		fprintf(stderr, "mode参数错误\n");
		return -1;
	}
	free(buf);
	return 0;
}
