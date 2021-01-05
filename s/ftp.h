#include <stdio.h>
#include<pthread.h>
enum FTP_msg_type {
    OK,
    ERROR,
    LOG,
    PWD,
    CD,
    LS,
    MKDIR,
    UPLOAD,
    DOWNLOAD,
    END,
    RMDIR
};

struct FTP_head {
    enum FTP_msg_type type;
    int len;
};

struct FTP_msg {
    struct FTP_head head;
    char *data;
};

struct user_info{
    int id;
    char *name;
    int tid;
    pthread_t pid;
};

int LoginCheck(char *name,char *password);

int FTP_send_msg(int sockfd, struct FTP_msg *msg);

int FTP_recv_msg(int sockfd, struct FTP_msg *msg);

int FTP_send_cmd(int sockfd, enum FTP_msg_type type, char *cmd_str,
        struct FTP_msg *msg);


int FTP_upload(int sockfd, FILE * fp, int mode);


int FTP_download(int sockfd, FILE * fp, int mode);
