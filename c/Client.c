#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include "ftp.h"

int main(int argc, char **argv)
{
    char server[20];
    int port = 5275;
    int sockfd;
    struct sockaddr_in addr;

   
    strcpy(server, "127.0.0.1");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket 创建失败\n");
        return -1;
       // exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(server);

    if (connect(sockfd, (struct sockaddr *) &addr,sizeof(struct sockaddr_in)) == -1) {
        printf("socket 连接失败\n");
        return -1;
    }
	struct FTP_msg send_msg;
    struct FTP_msg recv_msg;
	char name[50];
    char password[50];
	int flag=1;

    //--------------------尝试收发通讯--------------------
	while(flag){
	    send_msg.head.type=LOG;
	    send_msg.head.len=0;
	    send_msg.data=NULL;
	
	    if (FTP_send_msg(sockfd, &send_msg) == -1){
	        printf("与服务器连接断开！\n");
	    }
	    if(FTP_recv_msg(sockfd, &send_msg) == -1 ){
	        printf("与服务器连接断开！\n");
	    }

        //ssize_t send(int sockfd, const void *buf, size_t len, int flags);
	    printf("登陆中。。。。\n");
	    printf("输入用户名：\n");
	    scanf("%s",name);
	    send(sockfd,name,50,0);
	    printf("输入密码:\n");
        scanf("%s",password);
        send(sockfd,password,50,0);

        if (FTP_recv_msg(sockfd, &recv_msg) == -1) {
             printf("登陆失败！\n");
         }
          else if(recv_msg.head.len ==-1){
			  printf("登陆失败！\n");
         }
          else{
			  printf("登陆成功!\n");
			  flag=0;
		 }
    }


    //--------------------开始输入命令--------------------
    FILE *fp = NULL;
    char cmd[1024]="";
    char tmp[1024]="";
    

    while (1) {
        printf("ftp>");
        gets(cmd);
        if(!strcmp(cmd, "")){ }

        else if (!strncmp(cmd, "put", 3)) {
		    //strcat(tmp,"/");
		    strcat(tmp,cmd+4);
		    if((fp=fopen(tmp,"r+"))==NULL){
                printf("%s找不到！\n",tmp);
                memset(tmp,0,sizeof(tmp));
		    }else{
		        send_msg.head.type=UPLOAD;
		        send_msg.head.len=strlen(cmd+4)+1; 
		        send_msg.data=cmd+4;
                //fclose
		        if(FTP_send_msg(sockfd,&send_msg)==-1){
		            printf("与服务器连接断开！\n");
		        }

	            if (FTP_recv_msg(sockfd, &recv_msg) == -1
                        || recv_msg.head.type == ERROR) {
                    printf("与服务器连接断开！\n");
                }
                else if(FTP_upload(sockfd, fp, 0) == -1) {
                    printf("上传失败！\n");
                }else{
                    fclose(fp);
				    printf("上传文件： %s 成功！\n", tmp);
			    }
            }
            memset(tmp,0,sizeof(tmp));

        } else if (!strncmp(cmd, "get", 3)) {
	  
 	        send_msg.data = cmd + 4;
            send_msg.head.len = strlen(cmd+4)+1;
            send_msg.head.type = DOWNLOAD;
            if (FTP_send_msg(sockfd, &send_msg) == -1) {
                printf("与服务器连接断开！\n");
            }
	        if (FTP_recv_msg(sockfd, &recv_msg) == -1
                        || recv_msg.head.type == ERROR) {
                printf("与服务器连接断开！\n");\
            }
           
            strcpy(tmp, cmd + 4);
            if ((fp = fopen(tmp, "w+")) == NULL) {
                printf("%s创建失败\n",tmp);
            }
            else {
                if (FTP_download(sockfd, fp, 0) == -1) {
                	printf("下载文件 %s 失败！\n", cmd + 4);
                }else{
                    fclose(fp);
                    printf("下载文件 %s 成功！\n",tmp);
                }
            }
            memset(tmp,0,sizeof(tmp));

        } else if (!strncmp(cmd, "cd", 2)) {
            send_msg.data = cmd + 3;
            send_msg.head.len = strlen(cmd + 3) + 1;
            send_msg.head.type = CD;

            if (FTP_send_msg(sockfd, &send_msg) == -1) {
                printf("与服务器连接断开！1\n");
            }
            if (FTP_recv_msg(sockfd, &recv_msg) == -1
                    || recv_msg.head.type == ERROR) {
                printf("与服务器连接断开！2\n");
            }
	        else if(chdir(cmd+3)==-1){
		       // printf("cd 失败！\n");
		    }

        } else if (!strncmp(cmd, "pwd", 3)) {
	        send_msg.data = NULL;
            send_msg.head.len = 0;
            send_msg.head.type = PWD;
            if (FTP_send_msg(sockfd, &send_msg) == -1) {
                printf("与服务器连接断开！\n");
            }
            if (FTP_recv_msg(sockfd, &recv_msg) == -1
                    || recv_msg.head.type == ERROR) {
                printf("与服务器连接断开！\n");
            }
			else{
                printf("服务器路径:%s\n", recv_msg.data);
			}
       
	    } else if (!strncmp(cmd, "rmdir", 5)) {
			send_msg.data = cmd + 6;
            send_msg.head.len = strlen(cmd + 6)+1;
            send_msg.head.type = RMDIR;
            if (FTP_send_msg(sockfd, &send_msg) == -1) {
                printf("与服务器连接断开！\n");
            }
            if (FTP_recv_msg(sockfd, &recv_msg) == -1
                    || recv_msg.head.type == ERROR) {
                printf("与服务器连接断开！\n");
            }else{
		       printf("%s 已被删除！\n",cmd+6);
			}

        } else if (!strncmp(cmd, "mkdir", 5)) {
		    send_msg.data = cmd + 6;
            send_msg.head.len = strlen(cmd + 6)+1;
            send_msg.head.type = MKDIR;
            if (FTP_send_msg(sockfd, &send_msg) == -1) {
                printf("与服务器连接断开！\n");
            }
            if (FTP_recv_msg(sockfd, &recv_msg) == -1
                    || recv_msg.head.type == ERROR) {
                printf("与服务器连接断开！\n");
            }else{
		       printf("%s 创建成功！\n",cmd+6);
			   }

        } else if (!strncmp(cmd, "ls", 2)) {
			if (strlen(cmd) == 2) {
                strcpy(tmp, ".");
            } else {
                strcpy(tmp, cmd + 3);
            }
            send_msg.data = tmp;
            send_msg.head.len = strlen(tmp) + 1;
            send_msg.head.type = LS;
            if (FTP_send_msg(sockfd, &send_msg) == -1) {
                printf("与服务器连接断开！\n");
            }
            if (FTP_recv_msg(sockfd, &recv_msg) == -1) {
                printf("与服务器连接断开！\n");
            }
	        else{
                 printf("%s\n", recv_msg.data);
			}

        } else if (!strncmp(cmd, "lcd", 3)) {
	         if (cmd_lcd(cmd + 4) == -1) {
                printf("lcd 失败！\n");
                 }
        } else if (!strncmp(cmd, "lpwd", 4)) {
             if (cmd_lpwd(tmp) == -1) {
                printf("lpwd 失败！\n");
            }

        }else if (!strncmp(cmd, "lrmdir", 6)) {
	        if (cmd_lrmdir(cmd + 7) == -1) {
                printf("lrmdir 失败！\n");
            }

        }  else if (!strncmp(cmd, "lmkdir", 6)) {
         if (cmd_lmkdir(cmd + 7) == -1) {
                printf("lmkdir 失败！\n");
            }

        } else if (!strncmp(cmd, "dir", 3)) {
    		getcwd(cmd,1000);
		    if(cmd_dir(cmd)==-1){
		    printf("dir 失败\n");
		}

        } else if (!strcmp(cmd, "q") || !strcmp(cmd, "quit")) {
	        send_msg.data = NULL;
            send_msg.head.len = 0;
            send_msg.head.type = END;
            if (FTP_send_msg(sockfd, &send_msg) == -1) {
                printf("与服务器连接断开！\n");
                return -1;
            }
			    printf("客户端退出！\n");
            break;
            } else {
                printf("输入命令有误！\n");
            }
    }

    close(sockfd);
    return 0;
}
