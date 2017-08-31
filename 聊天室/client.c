#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<signal.h>
#define CLIENT_LOGIN 10
#define CLIENT_TALK 20
#define CLIENT_QUIT 30
#define SYSTEM_TALK 40
#define SYSTEM_QUIT 50
#define CLIENT_PRIV 60
#define CLIENT_NOTIN 70


typedef struct
{
	int type;
	char toname[15];   //目标名称
	char name[15];     //发送者名称
	char buf[1024];    //消息内容
}MSG;

void recv_message(int sock_fd,const char *pname)    //子进程接受消息
{
	MSG msg;
	while(1)
	{
		bzero(&msg,(sizeof(msg)));    //清空消息缓存
		recvfrom(sock_fd,&msg,sizeof(MSG),0,NULL,NULL);    //接受消息
		if(strcmp(msg.name,pname) != 0)       //发送者名称与自己名称不一致（即不是自己），便打印显示   
		{
			printf("#################\n");
			printf("TYPE:%d\n",msg.type);
			printf("NAME:%s\n",msg.name);
			printf("MTXT:%s\n",msg.buf);
		}
		if(msg.type == CLIENT_NOTIN)				//客户不在线
													//打印不在线的消息
		{
			printf("%s not online \n",msg.toname);
		}
		if(msg.type == SYSTEM_QUIT)			//处理退出消息
			goto next;                 
		}
		next:
		kill(getppid(),SIGKILL);          //杀死父进程   ？？？getppid()取得父进程识别码
		usleep(200);
		exit(EXIT_SUCCESS);
}


void send_message(int sock_fd,struct sockaddr_in server_in,const char * pname,int pid)   //父进程发送消息
{
	MSG msg;
	msg.type = CLIENT_LOGIN;   //登录模式
	strcpy(msg.name,pname);    //本客户端名
	strcpy(msg.buf,"i am Login");  //消息内容

	sendto(sock_fd,&msg,sizeof(MSG),0,(struct sockaddr *)&server_in,sizeof(server_in));//父进程发送登录消息

	while(1)
	{
		msg.type = CLIENT_TALK;                  //聊天模式
		fgets(msg.buf,sizeof(msg.buf),stdin);    //标准输入    消息内容
		msg.buf[strlen(msg.buf)-1] = '\0';       //转换文本格式（确保有\0）
		
		if(strncmp(msg.buf,"CHAT",4) == 0)       //检测是否为私聊
		{
			printf("Who do you want to chat with?\n");
			fgets(msg.toname,sizeof(msg.toname),stdin);  //标准输入  目标客户
			msg.toname[strlen(msg.toname)-1] = '\0';      
			while(1)
			{
				usleep(500);
				printf("Input msg.buf\n");
				bzero(msg.buf,sizeof(msg.buf));    //清空缓存
				fgets(msg.buf,sizeof(msg.buf),stdin);  //标准输入    消息内容
				msg.buf[strlen(msg.buf)-1] = '\0';
				msg.type = CLIENT_PRIV;            // 私聊模式
				sendto(sock_fd,&msg,sizeof(MSG),0,(struct sockaddr *)&server_in,sizeof(server_in));  //发送消息
				if(strncmp(msg.buf,"OUT",4) == 0)    //退出私聊   （聊天时输入OUT  ）
				break;
			}
		}

		if(strncmp(msg.buf,"quit",4) == 0)         //退出客户端   （聊天时输入quit）
			goto next;
			sendto(sock_fd,&msg,sizeof(MSG),0,(struct sockaddr *)&server_in,sizeof(server_in));//群聊发送消息
	}
	next:
	kill(pid,SIGKILL);   //退出时杀死子进程
	wait(NULL);
	msg.type = CLIENT_QUIT;    //退出模式

	sendto(sock_fd,&msg,sizeof(MSG),0,(struct sockaddr*)&server_in,sizeof(server_in));  //发送退出消息
	exit(EXIT_SUCCESS);
}

int main(int argc,const char *argv[])
{
	int sock_fd;
	pid_t pid;
	struct sockaddr_in server_in;
	
	if(argc < 4)      
	{
		printf("Usage: %s IP Port",argv[0]);
		return -1;
	}

	sock_fd = socket(AF_INET,SOCK_DGRAM,0);   //端口设置

	if(sock_fd < 0)
	{
		printf("fail to sock_fd: %s",strerror(errno));
		exit(EXIT_FAILURE);
	}
	bzero(&server_in,sizeof(server_in));     //清空缓存
	server_in.sin_family =AF_INET;                       //设置协议族
	server_in.sin_port = htons(atoi(argv[2]));             //设置端口PORT 
	server_in.sin_addr.s_addr = inet_addr(argv[1]);        //设置IP 

	if((pid = fork()) < 0)   //创建子进程
	{
		printf("fail to fork");
		exit(EXIT_FAILURE);
	}
	if(pid == 0)            
	{
		recv_message(sock_fd,argv[3]);  //子进程接受消息（）客户端交互
	}

	if(pid > 0)
	{
		send_message(sock_fd,server_in,argv[3],pid); //父进程发送消息
	}
	exit(EXIT_SUCCESS);
}

