#include<stdio.h>
#include<arpa/inet.h> /* internet socket */
#include<string.h>
//#define NDEBUG
#include<assert.h>
//#define PORT 5002
#define MAIN_SERVER_PORT 5001
#define IP_ADDR "127.0.0.1"
#define MAX_CONNECT_QUEUE 1024
#define MAX_BUF_LEN 512
char buf[MAX_BUF_LEN];
int main()
{
		int sockfd = -1;
		struct sockaddr_in serveraddr;

		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(MAIN_SERVER_PORT);
		serveraddr.sin_addr.s_addr = inet_addr(IP_ADDR);
		//bzero(&(serveraddr.sin_zero), 8);/* in string.h */
		memset(&serveraddr.sin_zero, 0, 8);
		sockfd = socket(PF_INET,SOCK_STREAM,0);
		assert((sockfd != -1));
		int ret = connect(sockfd,(struct sockaddr *)&serveraddr,sizeof(struct sockaddr));
		
		printf("Welcome to HDB Client!\n");
		int loginstate = 0;
		while(loginstate == 0)
		{
				if(ret == -1)
				{
						fprintf(stderr,"Connect Error,%s:%d\n",__FILE__,__LINE__);
						return -1;
				}
				char username[32];
				printf("UserName : ");

				gets(username);
				printf("\n");

				strcpy(buf,"login");
				strcpy(buf + 6,username);

				ret = send(sockfd,buf,sizeof(buf),0);
				if(ret > 0)
				{
						printf("send command \" %s \" to %s:%d\n",buf,(char*)inet_ntoa(serveraddr.sin_addr),ntohs(serveraddr.sin_port));
				}
				ret = recv(sockfd,buf,MAX_BUF_LEN,0);
				if(ret > 0)
				{		
						printf("Server rely:%s\n",buf);
				}
				int pt32 = atoi(buf);
				in_port_t port = (in_port_t)pt32;
				printf("%hd \n",port);
				serveraddr.sin_port = htons(port);
				close(sockfd);
				sockfd = socket(PF_INET,SOCK_STREAM,0);
			    ret = connect(sockfd,(struct sockaddr *)&serveraddr,sizeof(struct sockaddr));
				printf("ret of second connect is %d\n",ret);
				if(ret == 0)
				{
						loginstate = 1;
				}
		}
		
		while(1)
		{
				printf("HDB_V!.0:> ");
				gets(buf);
				ret = send(sockfd,buf,sizeof(buf),0);
	
				if(ret > 0)
				{
						printf("send command \" %s \" to %s:%d\n",buf,(char*)inet_ntoa(serveraddr.sin_addr),ntohs(serveraddr.sin_port));
				}
		
				ret = recv(sockfd,buf,MAX_BUF_LEN,0);
		
				if(ret > 0)
				{		
						printf("Server rely:%s\n",buf);
				}
		}
		close(sockfd);
		return 0;
}
