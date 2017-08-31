#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<math.h>

int main(void)
{
	pid_t child;

	if((child = fork()) == -1)
	{
		printf("Fork Error:%s\n",strerror(errno));
		exit(1);
	}
	else
	{
		if(child == 0)
		{
			printf("the child process is run \n");
			sleep(1);
			printf("I am the child:%d\n",getpid());
			exit(0);
		}
		else
		{
			wait(NULL);
			printf("the Father process is run \n");
			printf("I am the father:%d\n",getpid());
			return 0;
		}
	}
}
