#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "command.h"

int cmd_lpwd(char *path){
	if(getcwd(path,1000)==NULL){

		return -1;
	}
	printf("当前路径：%s\n",path);
	return 0;
}

int cmd_lcd(char *dir){
	return chdir(dir);
}

int cmd_lmkdir(char *dir){
return mkdir(dir, S_IRWXU | S_IRWXG | S_IRWXO);
}

int cmd_dir(char *path){
   
   DIR *pDir;
   struct dirent  *ent;
   if(pDir = opendir(path)){

   while((ent=readdir(pDir))!=NULL)
		printf("%s     ",ent->d_name);
		printf("\n");
		closedir(pDir);
	 }
	 else{
	   return -1;
	 }
      return 0;
}

/*

int cmd_lrmdir(char *path){
	struct stat stbuf; 
	char filepath[1024]; 
	if(stat(path,&stbuf) == -1)            //stat获取文件信息放入stbuf中
	{ 
		fprintf(stderr,"Can't Access %s\n",path);   //标准错误输出
		return; 
	} 
	if(S_ISDIR(stbuf.st_mode))  //表示是否是一个目录
	{
		DIR *dir; 
		struct dirent *ptr;         //dirent用于获取文件目录内容
		if((dir = opendir(path)) == NULL)
		{ 
			fprintf(stderr,"不能打开 %s\n",path); 
		} 
		while((ptr = readdir(dir)) != NULL)
		{
		   if(strcmp(ptr->d_name,".") == 0 ||strcmp(ptr->d_name,"..") == 0)
			  { 
				  continue; 
			  }
			strcpy(filepath,path); 
			strcat(filepath,"/"); 
			strcat(filepath,ptr->d_name); 
			rmdir(filepath);
		} 
		rmdir(path);
		closedir(dir);
		chdir("..");
 		printf("%s 已被删除\n",path);
	}
	else
	{
		unlink(path);             //对文件的删除
		printf("%s 已被删除\n",path);
	} 

	return 0;
}

*/




void dfs_remove_dir1()
{
	DIR *cur_dir = opendir(".");
	struct dirent *ptr = NULL;
	struct stat st;

	if (!cur_dir)
	{
		perror("opendir:");
		return;
	}

	while ((ptr = readdir(cur_dir)) != NULL)
	{
		stat(ptr->d_name, &st);
	
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
		{
			continue;
		}

		if (S_ISDIR(st.st_mode))
		{
			chdir(ptr->d_name);
			dfs_remove_dir1();
			chdir("..");
		}

		remove(ptr->d_name);
	}
	
	closedir(cur_dir);
}

void remove_dir1(const char *path_raw)
{
	char old_path[100];

	if (!path_raw)
	{
		return;
	}

	getcwd(old_path, 100);
	
	if (chdir(path_raw) == -1)
	{
		fprintf(stderr, "not a dir or access error\n");
		return;
	}

	dfs_remove_dir1();
	chdir(old_path);
        unlink(old_path); 
	
}
int cmd_lrmdir(char *filename)
{
	remove_dir1(filename);
	rmdir(filename);

	return 0;
}

