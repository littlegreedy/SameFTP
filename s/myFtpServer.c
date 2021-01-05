#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<sys/un.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/stat.h>
#include<pthread.h>

#include <string.h>
#include <arpa/inet.h>
#include<netinet/in.h>
#include <dirent.h>
#include <fcntl.h>
#include<errno.h>

#include <shadow.h>
#include<pwd.h>
#include<pthread.h>
#include"ftp.h"
void *run(void *arg);
void *server_run(void *arg);

int sum = 0;
int count=0;

struct user_info user[50];

int main(int argc, char **argv)
{
    
    int sockfd;
    int port = 5275;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton("127.0.0.1", &addr.sin_addr);//inet_aton()是一个改进的方法来将一个字符串IP地址转换为一个32位的网络序列IP地址。
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("server socket creates error\n");
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr))== -1){
        printf("server binds error\n");
        return -1;
    }
    else{
    	printf("server starts successfully\n\n");
    }
    //用于面向连接服务器，表明愿意接收连接
    if (listen(sockfd, 100) == -1) {
        printf("server listens error\n");
        return -1;
    }
    //将一个IP字符串转换成Dotted Decimal Notation
    printf("server is running, ip: %s, port: %d\n",inet_ntoa(addr.sin_addr), port);

	int len;     //指向存有addr地址长度的整形数。
    int cli_sockfd;
    struct sockaddr_in cli_addr;
	len = sizeof(cli_addr);
    unsigned client_port=0;  
    char client_ip[20];  
    pthread_t tid;

	printf("server listenning at port\n");

//创建新的服务器线程
    pthread_create(&tid,NULL,server_run,(void *)sockfd);


	while(1){

	     if(cli_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &len)){
             sum++;
             count++;
         }
            
        if (cli_sockfd < 0) {
            printf("server acception error\n");
		    close(sockfd);//关闭通信的结果待测！
            return -1;
         }
		    //获得客户机ip和端口
		    inet_ntop(AF_INET,&cli_addr.sin_addr.s_addr,client_ip,sizeof(client_ip));
		    client_port = ntohs(cli_addr.sin_port);
		    printf("用户IP:%d : 端口: %d 连接，用户套接字sockfd:%d\n",client_ip,client_port,sockfd);
		
            /*
            * run（）即为
            * { 
                read(...); 
                process(...); 
                write(...); 
              }
            *
            */


		    //创建新的用户线程
             //rc=0 p create error
		    pthread_create(&tid,NULL,run,(void *)cli_sockfd);
           
	    }
}


void *server_run(void *arg){
    int sockfd=(int)arg;
    char cmd[1000]="";
    char tmp[1000]="";
    int uid=0;
    int i=0;
    int j=0;

    while (1) {
        printf("server>");
        gets(cmd);
        if(!strcmp(cmd, "")){ }
        else if (!strncmp(cmd, "list", 4)) {
            printf("-----------------------User Information-----------------------\n");

            for(i=0;i<count;i++){
	            printf("|ID:%d    name:%s                                             |\n",user[i].id,user[i].name);
	            printf("--------------------------------------------------------------\n");
            }
        }else if (!strncmp(cmd, "kill", 4)) {
            memset(tmp,0,sizeof(tmp));
            strcat(tmp,cmd+5);
            uid=atoi(tmp);
            for(j=0;j<count;j++)
            {
	            if(user[j].id==uid){
		            if(pthread_cancel(user[j].pid)!=0)
			            printf("%d thread not stop!\n",user[j].id);
		            for(i=j;i<count;i++){
			            user[i].id=user[i+1].id;
			            user[i].name=user[i+1].name;
			            user[i].tid=user[i+1].tid;
			            user[i].pid=user[i+1].pid;
		            }
		            count--;
            close(user[j].tid);
	            }	
            }
        }else if (!strncmp(cmd, "sum", 3)) {
            printf("The history number of User:%d\n",sum);
        }else if (!strncmp(cmd, "count", 5)) {
            printf("The present number of User:%d\n",count);
        }else if (!strcmp(cmd, "q") || !strcmp(cmd, "quit")) {
	        printf("服务器端退出！\n");
            break;
        } else {
            printf("输入命令有误！\n");
        }
     }
    close(sockfd);
       
    return 0;
}



void *run(void *arg){
    int cli_sockfd=(int)arg;
    int count_id=0;
    //让线程分离  ----自动退出,无系统残留资源
    pthread_t id=pthread_self();
    pthread_detach(id);
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    struct FTP_msg send_msg;
    struct FTP_msg recv_msg;
    char tmp[1000];
	char name[50];
	char password[50];
    FILE *fp = NULL;

    while (1) {
        if (FTP_recv_msg(cli_sockfd, &recv_msg) == -1) {
            printf("server未接收到数据\n");
            return -1;
        }

        switch (recv_msg.head.type) {
            case LOG:
				printf("用户正在登陆。。。\n");
				
                send_msg.head.type=OK;
                send_msg.head.len=0;
                send_msg.data=NULL;
				
				if(FTP_send_msg(cli_sockfd, &send_msg)==-1){
                    printf("sending LOG answer error\n");
                }
				
				recv(cli_sockfd,name,50,0);
				recv(cli_sockfd,password,50,0);
				if(LoginCheck(name,password)==0){
					printf("ID:%d    用户名:%s\n",sum,name);
					printf("登陆成功！\n");

					user[sum-1].id=sum;
					user[sum-1].name=name;
					user[sum-1].tid=cli_sockfd;
					count_id=sum;

					send_msg.head.type=OK;
					send_msg.head.len=0;
					send_msg.data=NULL;
				}
				else{
					printf("登录失败！\n");
					send_msg.head.type=ERROR;
					send_msg.head.len=-1;
					send_msg.data=NULL;	
				}
				if(FTP_send_msg(cli_sockfd, &send_msg)==-1){
					printf("sending LOGing error\n");
				}
                break;
            case CD:
				printf("command is CD\n");
                
				if (cmd_lcd(recv_msg.data) == -1) {
                    send_msg.head.type = ERROR;
                } 
                else {
                    send_msg.head.type = OK;
                }
                send_msg.head.len = 0;
                send_msg.data = NULL;
				
                if(FTP_send_msg(cli_sockfd, &send_msg)==-1){
                    printf("sending CD answer error\n");
                }
                break;
            case PWD:
                printf("command is PWD\n");
				
		        memset(tmp,0,sizeof(tmp));
                //getcwd(path,1000)
				if (cmd_lpwd(tmp) == -1) {
                    send_msg.head.type = ERROR;
                    send_msg.head.len = 0;
                    send_msg.data = NULL;
                } else {
                    send_msg.head.type = OK;
                    send_msg.data = tmp;
                    send_msg.head.len = strlen(tmp) + 1;
                }
				
                if(FTP_send_msg(cli_sockfd, &send_msg)==-1){
                    printf("sending PWD answer error\n");
                }	
                break;
            case LS:
               printf("command is LS\n");
                pthread_mutex_lock(&mutex);
		        char info[200];
 		        DIR *pDir;
   		        struct dirent  *ent;
   		        if(pDir = opendir(recv_msg.data)){
   		            while((ent=readdir(pDir))!=NULL){
		                strcat(info,ent->d_name);
		                strcat(info,"    ");
		            }
                    pthread_mutex_unlock(&mutex);
		            printf("\n");
		            closedir(pDir);
	 	        }
		        if (cmd_dir(recv_msg.data) == -1) { 
                    send_msg.head.type = ERROR;
                    send_msg.data = NULL;
                    send_msg.head.len = 0;
                } else {
                    send_msg.head.type = OK;
                    send_msg.data = info;
                    send_msg.head.len = strlen(info) + 1;
                }
				
                if(FTP_send_msg(cli_sockfd, &send_msg)==-1){
                    printf("sending LS answer error\n");
                }
		        memset(info,0,sizeof(info));
                break;

            case MKDIR:
                printf("command is MKDIR\n");
		
		        memset(tmp,0,sizeof(tmp));	
		        if (cmd_lmkdir(recv_msg.data) == -1) {
                    send_msg.head.type = ERROR;
                } else {
                    send_msg.head.type = OK;
                }
                send_msg.head.len = 0;
                send_msg.data = NULL;
				
                if(FTP_send_msg(cli_sockfd, &send_msg)==-1){
                    printf("sending MKDIR answer error\n");
                }

                break;
	    case RMDIR:
                printf("command is RMDIR\n");

		        memset(tmp,0,sizeof(tmp));
		        if (cmd_lrmdir(recv_msg.data) == -1) {
                    send_msg.head.type = ERROR;
                } 
                else {
                    send_msg.head.type = OK;
                }
                send_msg.head.len = 0;
                send_msg.data = NULL;
				
                if(FTP_send_msg(cli_sockfd, &send_msg)==-1){

                    printf("sending RMDIR answer error\n");
                }

                break;

            case UPLOAD:
                printf("command is UPLOAD\n");
                
		        memset(tmp,0,sizeof(tmp));
				send_msg.head.type=OK;
				send_msg.head.len=0;
				send_msg.data=NULL;
				if(FTP_send_msg(cli_sockfd, &send_msg)==-1){
                    printf("PUT return error\n");
                }
				else{
					strcat(tmp,recv_msg.data);
					if((fp=fopen(tmp,"w+"))==NULL)
					{
						printf("%s 创建失败\n", tmp);
						return -1;
					}
					if (FTP_download(cli_sockfd, fp, 0) == -1) {
						printf("上传文件 %s 失败\n", tmp);	
					}
				}
				fclose(fp);
                break;

            case DOWNLOAD:
                printf("command is DOWNLOAD\n");
                
				memset(tmp,0,sizeof(tmp));
		        send_msg.head.len = 0;
                send_msg.data = NULL;
                strcpy(tmp, recv_msg.data);
                if ((fp = fopen(tmp, "r")) == NULL) {
                    printf("打开文件%s失败\n", tmp);
                    send_msg.head.type = ERROR;
                    send_msg.head.len = 0;
                    send_msg.data = NULL;
                    if (FTP_send_msg(cli_sockfd, &send_msg) == -1) {
                        printf("GET return error\n");
                    }
                } else {
                    send_msg.head.type = OK;
                    send_msg.head.len = 0;
                    send_msg.data = NULL;
                    if (FTP_send_msg(cli_sockfd, &send_msg) == -1) {
                        printf("GET return error\n");
                       
                    }
                    if (FTP_upload(cli_sockfd, fp, 0) == -1) {
                        printf("文件下载%s失败\n", tmp);
                    }else{
			
					printf("文件%s下载成功\n", tmp);
					}
                }
				
                break;
            case END:
                printf("User_ID:%d exit\n",count_id);
				close(cli_sockfd);
				
                break;
        }

    }
}
