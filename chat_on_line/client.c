#include "head.h"

#define CLIENT_LOGIN 10
#define CLIENT_TALK  20
#define CLIENT_QUIT  30
#define SYSTEM_TALK  40
#define SYSTEM_QUIT  50
#define CLIENT_PRIV  60
#define CLIENT_NOTIN 70
#define CLIENT_ZAN   80

typedef struct
{
	int type;
	char toname[15];
	char name[15];
	char buf[1024];
}MSG;
void recv_message(int sock_fd,const char *pname)  //子进程接收消息
{
	MSG msg;
	while(1)
	{
		bzero(&msg,(sizeof(msg)));
		recvfrom(sock_fd,&msg,sizeof(MSG),0,NULL,NULL);
#if 1
		if(msg.type == CLIENT_ZAN)
		{
			printf("%s praised you!\n",msg.name);
		//	continue;
		//  goto next;
		//	printf("a");
		//	msg.type = CLIENT_TALK;
		}
#endif
		else if(strcmp(msg.name,pname) != 0)     //自己发的消息不会显示出来
		{
			printf("++++++++++++++++++++\n");
			//printf("TYPE : %d \n",msg.type);
			printf("NAME : %s \n",msg.name);
			printf("MTXT : %s \n",msg.buf);
			printf("++++++++++++++++++++\n");
		}
		if(msg.type == CLIENT_NOTIN)      //打印不在线的消息
		{
			printf("%s NOT Online\n",msg.toname);
		}
		if(msg.type == SYSTEM_QUIT)//处理系统退出
		{
    		goto next;
		}

	}
	next:
	kill(getppid(),SIGUSR1);     //杀死父进程
	usleep(200);

	exit(EXIT_SUCCESS);
}
//父进程发送消息
void send_message(int sock_fd,struct sockaddr_in server_in,const char *pname,int pid)
{
	MSG msg;
	msg.type = CLIENT_LOGIN;
	strcpy(msg.name,pname);
	strcpy(msg.buf,"I am Login");
//发送登录消息
    sendto(sock_fd,&msg,sizeof(MSG),0,(struct sockaddr *)&server_in,sizeof(server_in));
	while(1)
	{
		msg.type = CLIENT_TALK;
	    fgets(msg.buf,sizeof(msg.buf),stdin);
		msg.buf[strlen(msg.buf) - 1] = '\0';

#if 1
		if(strncmp(msg.buf,"zan",3) == 0)
		{
            printf("Please input the name!\n");
			fgets(msg.toname,sizeof(msg.toname),stdin);
			msg.toname[strlen(msg.toname) - 1] = '\0';
#if 1
			usleep(500);
			msg.type = CLIENT_ZAN;
		//	sendto(sock_fd,&msg,sizeof(MSG),0,(struct sockaddr *)&server_in,sizeof(server_in));
			if(strncmp(msg.buf,"OUT",3) == 0)
			break;
#endif
#if 0
				while(1)
			{
				usleep(500);
				//printf("Input msg.buf\n");
				//bzero(msg.buf,sizeof(msg.buf));
				//fgets(msg.buf,sizeof(msg.buf),stdin);
		        //msg.buf[strlen(msg.buf) - 1] = '\0';
				msg.type = CLIENT_ZAN;
				sendto(sock_fd,&msg,sizeof(MSG),0,(struct sockaddr *)&server_in,sizeof(server_in));
				if(strncmp(msg.buf,"OUT",4) == 0)
			       break;
			}
#endif
		}
#endif
		if(strncmp(msg.buf,"CHAT",4) == 0)
		{
            printf("Who do you want to chat with ?\n");
			fgets(msg.toname,sizeof(msg.toname),stdin);
			msg.toname[strlen(msg.toname) - 1] = '\0';
			while(1)
			{
				usleep(500);
				printf("Input msg.buf\n");
				bzero(msg.buf,sizeof(msg.buf));
				fgets(msg.buf,sizeof(msg.buf),stdin);
		        msg.buf[strlen(msg.buf) - 1] = '\0';
				msg.type = CLIENT_PRIV;
				sendto(sock_fd,&msg,sizeof(MSG),0,(struct sockaddr *)&server_in,sizeof(server_in));
             //退出私聊
				if(strncmp(msg.buf,"OUT",4) == 0)
			       break;
			}
		}
//退出客户端
		if(strncmp(msg.buf,"quit",4) == 0)
		goto next;
	    sendto(sock_fd,&msg,sizeof(MSG),0,(struct sockaddr *)&server_in,sizeof(server_in));
	}
	next:
	kill(pid,SIGUSR1); //退出时杀死子进程
	wait(NULL);
	msg.type = CLIENT_QUIT;    
	sendto(sock_fd,&msg,sizeof(MSG),0,(struct sockaddr *)&server_in,sizeof(server_in));
	exit(EXIT_SUCCESS);
}

int main(int argc, const char *argv[])
{
	int sock_fd;
	pid_t pid;
	struct sockaddr_in server_in;

	if(argc < 4)
	{
		printf("Usage :%s IP Port \n",argv[0]);
		return -1;
	}
	sock_fd = socket(AF_INET,SOCK_DGRAM,0);
	if(sock_fd < 0)
	{
		printf("fail to sock_fd:%s \n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	bzero(&server_in,sizeof(server_in));
    server_in.sin_family = AF_INET;
	server_in.sin_port = htons(atoi(argv[2]));
	server_in.sin_addr.s_addr = inet_addr(argv[1]);
	if((pid = fork()) < 0)
	{
		printf("fail to fork");
		exit(EXIT_FAILURE);
	}
	if(pid == 0)
	{
		recv_message(sock_fd,argv[3]);
	}
	if(pid > 0)
	{
		send_message(sock_fd,server_in,argv[3],pid);
	}
	exit(EXIT_SUCCESS);
}
