#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<math.h>

int main()
{
	pid_t child;
	if((child = vfork()) == -1)
	{
		printf("Fork Error: %s\n",strerror(errno));
		exit(1);
	}
	else
		if(child == 0)
		{
			sleep(1);
			printf("I am the child:%d\n",getpid());
			exit(0);
		}
		else
		{
			printf("I am the father:%d\n",getpid());
			exit(0);
		}	
}
