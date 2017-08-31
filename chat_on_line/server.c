#include "head.h"

#define CLIENT_LOGIN 10      //客户端登录
#define CLIENT_TALK  20      //客户端群聊
#define CLIENT_QUIT  30      //客户端退出
#define SYSTEM_TALK  40      //系统消息
#define SYSTEM_QUIT  50      //系统退出
#define CLIENT_PRIV  60      //客户端私聊
#define CLIENT_NOTIN 70      //客户端不在线
#define CLIENT_ZAN   80      //点赞

typedef struct sockaddr DataType; 

typedef struct node
{
	int n;
	char pname[15];
	DataType data;
	struct node *next;
}LinkNode;     //记录 人数 人名  用户地址 

typedef struct
{
	int type;				//消息类型
	char toname[15];		//发给对象
	char name[15];			//记录用户名字  ？？
	char buf[1024];			//消息内容
}MSG;					//消息

LinkNode *creat_empty_linklist()
{
	LinkNode *head = NULL;
	head = (LinkNode *)malloc(sizeof(LinkNode));

	head->next = NULL;
	return head;
};

int enter_linklist_head(LinkNode *head,DataType data,MSG msg)//将信息存入链表
{
	LinkNode *temp = NULL;								//动态链表
	temp = (LinkNode *)malloc(sizeof(LinkNode));  //分配空间

	(head->n)++;								//链表长度加一   ？？？待确认是否正确  
	strcpy(temp->pname,msg.name);             //存储用户名

	temp->data = data;							//IP  PORT 等
	temp->next = head->next;					//链首插入信息
	head->next = temp;

	return 0;
}

void dele_linklist(LinkNode *head,DataType data)   //用户退出时删除信息
{
	LinkNode *temp = NULL ;
	LinkNode *p = head;
	while(p->next != NULL && (memcmp(&(p->next->data),&data,sizeof(data))) !=0 )  
	//节点不在链表尾  memcmp()对结构体进行比较，相同删除（有可能出错，涉及到内存）
	{
		p = p->next;
	}

	(head->n)--;                            //链表长度减一
	temp = p->next;							//从链表中删除
	p->next = p->next->next;
	free(temp);								//释放内存
	temp = NULL;
	return;
}

void send_message(int sock_fd,struct sockaddr_in server_in) //父进程发送消息给子进程
{
	MSG msg;
	msg.type = SYSTEM_TALK;
	strcpy(msg.name,"System");
	
	while(1)
	{
		fgets(msg.buf,sizeof(msg.buf)-1,stdin);   //从标准输入中得到数据
		msg.buf[strlen(msg.buf)-1] = '\0';        //数据最后一位改为 ‘\0’

		if(strncmp(msg.buf,"quit",4) == 0 )         //比较数据 是否为退出模式，是 goto 否 继续执行
			goto next;
		sendto(sock_fd,&msg,sizeof(MSG),0,(struct sockaddr *)&server_in,sizeof(server_in));
		//     socket  数据   长度     flags                欲传送的网络地址   长度		
	}

	return;    //发送模式结束
	next:      //退出模式

	msg.type = SYSTEM_QUIT;
	sendto(sock_fd,&msg,sizeof(MSG),0,(struct sockaddr *)&server_in,sizeof(server_in));
	usleep(500);
	exit(EXIT_SUCCESS);
}

void broadcast_message(MSG msg,LinkNode *head,int sock_fd)   //广播消息给每个用户
{
	LinkNode *p = NULL;
	p = head->next;   //用户链表

	while(p)
	{
		printf("Online name %s.\n",p->pname);     //显示所有用户名   while()循环
		sendto(sock_fd,&msg,sizeof(MSG),0,&p->data,sizeof(p->data));   //将消息发送给所有用户
		p = p->next;
	}
	return;
}

void private_chat(MSG msg,LinkNode *head,int sock_fd)    //处理私聊信息
{
	LinkNode *temp = NULL;
	LinkNode *p = head->next;
	if(msg.type == CLIENT_PRIV)                     //判断消息类型
	{
		while(p != NULL && strcmp(p->pname,msg.toname) != 0)        //如果用户不为空且与目的用户不匹配，链表向后移一位
		{                                                           //直到p 为NULL 或者找到目的用户
			p = p->next;
		}
		if(p == NULL)            //没有找到指定用户
		{
			msg.type = CLIENT_NOTIN;       //消息类型为不在线
			p = head->next;
			while(p != NULL && strcmp(p->pname,msg.name) != 0)    //用户不为空且与发送用户不匹配，链表向后移一位
			{                                                     //找到发送用户
				p = p->next;
			}
		}
		sendto(sock_fd,&msg,sizeof(MSG),0,&p->data,sizeof(p->data));//找到指定用户并发送数据
	}
	return;
}

#if 1
void give_praise(MSG msg,LinkNode *head,int sock_fd)    
{
	LinkNode *temp = NULL;
	LinkNode *p = head->next;
	if(msg.type == CLIENT_ZAN)                     //判断消息类型
	{
		while(p != NULL && strcmp(p->pname,msg.toname) != 0)        //如果用户不为空且与目的用户不匹配，链表向后移一位
		{                                                           //直到p 为NULL 或者找到目的用户
			p = p->next;
		}
		if(p == NULL)            //没有找到指定用户
		{
			msg.type = CLIENT_NOTIN;       //消息类型为不在线
			p = head->next;
			while(p != NULL && strcmp(p->pname,msg.name) != 0)    //用户不为空且与发送用户不匹配，链表向后移一位
			{                                                     //找到发送用户
				p = p->next;
			}
		}

		sendto(sock_fd,&msg,sizeof(MSG),0,&p->data,sizeof(p->data));//找到指定用户并发送数据
	}
	return;
}
#endif

void do_client(int sock_fd)                  //与客户端交互，根据不同的消息类型做不同的事
{
	MSG msg;									//
	LinkNode *L = creat_empty_linklist();     //新建一个用户列表
	struct sockaddr peer_addr;               //创建网络端口链表  ？？？
	int addr_len = sizeof(peer_addr);         //端口长度      ？？？？

	while(1)
	{
		recvfrom(sock_fd,&msg,sizeof(MSG),0,&peer_addr,&addr_len);     //接收消息
			//   套接字   接收数据缓冲区 长度  指定欲传送的网络地址  长度

		switch(msg.type)												//判断消息类型
		{
			case CLIENT_LOGIN:											//处理登录
				enter_linklist_head(L,peer_addr,msg);                 //将信息存入链表
				//              *head 记录用户地址 msg消息（类型，私聊消息发给谁，用户名，消息内容）
				printf("Online %d.\n",L->n);                         //显示在线人数
				broadcast_message(msg,L,sock_fd);                    //广播信息给所有用户
				break;
			case CLIENT_TALK:											//处理群聊
				broadcast_message(msg,L,sock_fd);						//广播信息给所有用户
				break;
			case CLIENT_QUIT:											//处理退出
				dele_linklist(L,peer_addr);								//删除用户地址
				printf("Online %d.\n",L->n);							//显示在线人数
				broadcast_message(msg,L,sock_fd);						
				break;
			case SYSTEM_TALK:											//处理系统消息
				broadcast_message(msg,L,sock_fd);
				break;
			case CLIENT_PRIV:											//处理私聊信息
				private_chat(msg,L,sock_fd);							//找到指定用户发送消息
				break;
#if 1
			case CLIENT_ZAN:
			{
				give_praise(msg,L,sock_fd);		
				break;
			}
#endif
			case SYSTEM_QUIT:											//处理系统退出
				broadcast_message(msg,L,sock_fd);
				goto next;
		
		}

		printf("##############################\n");
		printf("TYPE:%d\n",msg.type);
		printf("NAME:%s\n",msg.name);
		printf("MTXT:%s\n",msg.buf);
		printf("##############################\n");
	}

	next:                                    //系统退出
		usleep(500);
		exit(EXIT_SUCCESS);
}

int main(int argc,const char *argv[])
{
	int sock_fd;            
	pid_t pid;    
	struct sockaddr_in server_in;   //sockaddr 与sockaddr_in可以互相转化，并无区别  指明地址信息

	if(argc < 3)                 //判断是否输入IP 地址
	{
		printf("Usage : %s IP Port\n",argv[0]);   //？？
		return -1;
	}

	sock_fd = socket(AF_INET,SOCK_DGRAM,0); 
	
	if(sock_fd < 0)    
	{
		printf("fail to soc_fd:%s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	bzero(&server_in,sizeof(server_in));//清空缓存，等待下一个端口
	server_in.sin_family = AF_INET;
	server_in.sin_port = htons(atoi(argv[2]));     //端口号
	server_in.sin_addr.s_addr = inet_addr(argv[1]);   //地址

	if(bind(sock_fd,(struct sockaddr *)&server_in,sizeof(server_in)) < 0)         //  0 成功 -1 失败
				   																  // 绑定一个协议地址
	{
		perror("fail to bind");
		exit(EXIT_FAILURE);
	}
	
	if((pid = fork())<0)   //创建一个子进程
	{
		printf("fail to fork\n");
		exit(EXIT_FAILURE);
	}
	
	if(pid == 0)        //子进程执行客户端交互
	{
		do_client(sock_fd);
	}

	if(pid > 0)         //父进程发送消息给子进程
	{
		send_message(sock_fd,server_in);
	}

	exit(EXIT_SUCCESS);
}

		



