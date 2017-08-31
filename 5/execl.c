#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>

int main(int argc,char *argv[])
{
	if(argc<2)
	{
		perror("you haven't input the filename,please try again!\n");
		exit(EXIT_FAILURE);
	}

	if(execl("./file_cp","file_cp",argv[1],argv[2],NULL)<0)
		perror("execl error!");
}

