#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define FIFO_SERVER "/tmp/myfifo"

int main()
{
	int fd;
	char w_buf[100];
	int nread;

	printf("Preparing for reading bytes...\n");
    printf("im OK ");
	fd = open( FIFO_SERVER ,O_RDONLY|O_NONBLOCK,0);
	printf("I am OK");
	if(fd == -1)
	{
		perror("open");
		exit(1);
	}
	while(1)
	{
		memset(w_buf,0,sizeof(w_buf));
	
	if((nread = read(fd,w_buf,100))== -1)
		{
			if(errno == EAGAIN)
				printf("no data yet\n");
		}
		printf("read %s from FIFO\n",w_buf);
		sleep(1);
	}
	close(fd);
	pause();
	unlink(FIFO_SERVER);
}
