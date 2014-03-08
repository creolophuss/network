#include<fcntl.h>
#include<stdio.h>
//#include<iostream>
#include<arpa/inet.h> /* internet socket */
#include<string.h>
#include<stdlib.h>
//#define NDEBUG
#include<assert.h>
#include<pthread.h>
#define PORT 5001
#define IP_ADDR "127.0.0.1"
#define MAX_CONNECT_QUEUE 1024
#define MAX_BUF_LEN 512
#include<sys/epoll.h>
//#include"myhdb.h"
#include"cmd_handler.h"
#include<errno.h>
#define MAXLINE 512
static int  
make_socket_non_blocking (int sfd)  
{  
		int flags, s;  

		flags = fcntl (sfd, F_GETFL/*, 0*/);  
		if (flags == -1)  
		{  
				perror ("fcntl");  
				return -1;  
		}  

		flags |= O_NONBLOCK;  
		s = fcntl (sfd, F_SETFL, flags);  
		if (s == -1)  
		{  
				perror ("fcntl");  
				return -1;  
		}  

		return 0;  
}  
int main()
{
		char *server_port[16];
		int n_server = 0;
		char line[MAX_BUF_LEN];
		int sockfd = -1;
		char buf[MAX_BUF_LEN];
		struct sockaddr_in clientaddr;
		socklen_t clientaddr_len = sizeof(struct sockaddr_in);
		struct sockaddr_in addr;

		addr.sin_family = AF_INET;
		addr.sin_port = htons(PORT);
		addr.sin_addr.s_addr = inet_addr(IP_ADDR);
		//bzero(&(addr.sin_zero), 8);/* in string.h */
		memset(&addr.sin_zero, 0, 8);

		int listenfd = socket(PF_INET,SOCK_STREAM,0);
	//	assert((sockfd != -1));

		int ret = bind(listenfd,(struct sockaddr *)&addr,sizeof(struct sockaddr));
		if(ret == -1)
		{
				fprintf(stderr,"Bind Error,%s:%d,",__FILE__,__LINE__);
				fprintf(stderr,"%s:%d\n",(char*)inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
				close(sockfd);
				return -1;
		}		
		make_socket_non_blocking (listenfd);  
		
		
		struct epoll_event ev;
		int epfd = epoll_create(256);
		ev.data.fd = listenfd;
		ev.events = EPOLLIN|EPOLLET;

		epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
		ret = listen(listenfd,MAX_CONNECT_QUEUE);
		if(ret != 0)
				printf("listen err\n");
		int connfd;
		pthread_t *threadp;
		data_t *datap; 
		typedef struct sdata_t
		{
				int fd;
				void *ptr;
		} *sdata_p;
		sdata_p sdatap;
		DB_DESCRIPTOR db_ptr;
		db_ptr = open_hdb("users");
		char login_name[32];
		int n;
		struct epoll_event events[20];
		while(1)
		{
				printf("epoll\n");
				int  nfds = epoll_wait(epfd,events,20,-1);
				printf("wait nfds == %d \n",nfds);
				int i;
	//			for(i = 0; i < n_server; i++)
	//					printf("port %d %hd \n",i,server_port[i]);
				for( i = 0; i < nfds; ++i)
				{
						if(events[i].data.fd == listenfd)
						{
								printf("Begin Accept\n");
								connfd = accept(listenfd,(struct sockaddr *)&clientaddr, &clientaddr_len);
								printf("listenfd == %d\n",listenfd);
								printf("connfd fd == %d\n",connfd);
								if(connfd < 0){					
									   	perror("connfd<0");
										exit(1);
						  		}
								make_socket_non_blocking (connfd);  
								char *str = inet_ntoa(clientaddr.sin_addr);
								printf("Accept a connection from %s\n",str);
								sdatap = (struct sdata_t *)malloc( sizeof(struct sdata_t));
								sdatap->fd = connfd;
								sdatap->ptr = NULL;
								ev.data.ptr = (void *) sdatap;
								ev.events = EPOLLIN|EPOLLET;										
								epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
								printf("Finish Accept %d\n",connfd);
						}
						else if(events[i].events & EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读入。
						{
								printf("EPOLLIN\n");
								sdatap = (struct sdata_t *)events[i].data.ptr;
							//	printf("events[i].data.fd == %d\n",sdatap->fd);
								if ( (sockfd = sdatap->fd) < 0)
								{
										printf("continur\n");	
										continue;
								}
								printf("sockfd ==  %d\n",sockfd);

								if ( (n = read(sockfd, line,sizeof(line))) < 0) 
								{
										printf("read < n\n");
										if (errno == ECONNRESET) 
										{																			
											   	close(sockfd);											
												sdatap->fd = -1;
										} else								
												printf("readline error.\n");
										close(sockfd);									
									   	sdatap->fd = -1;
										continue;
							  	}
								else if(n == 0)
								{
										printf("n == 0\n");
										
										ev.data.ptr = (void *) sdatap;	
					   					ev.events = EPOLLIN|EPOLLET;
				                		epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
										printf("EPOLLIN END\n");
										continue;
								}
								printf("Has read %s \n",line);
								n = strlen(line);
								if(strcmp(line,"login") == 0)
								{
										char *p ;
										p =(char *) readby_key(db_ptr,line+n+1);
										int client_id = atoi(p);
										printf("client_id ==  %d \n",client_id);
								//		printf("n_server == %d\n",n_server);
										if(n_server == 0)
												continue;
										int index = client_id % n_server;
										printf("assignment\n");
//										sdatap->ptr = (void *) (server_port+index);
										sdatap->ptr =  server_port[index];
										printf("get port from index %d is %s\n",index,server_port[index]);
										printf("server port == %s\n",(char *)sdatap->ptr);
										printf("login end\n");
								}
								else if(strcmp(line,"server connect") == 0)
								{
										size_t len = strlen(line);
										size_t plen = strlen(line+len+1);
										printf("len == %lu,plen == %lu \n",len,plen);
										char *pt = (char *)malloc(plen+1);
										strcpy(pt,line+len+1);
										server_port[n_server] = pt;
										printf("add new port %s\n",server_port[n_server]);
										n_server++;
										sdatap->ptr = "connect ok";
										printf("the address of \" connect ok \" is %lu \n",sdatap->ptr);
								}
								
				   				sdatap->fd = sockfd;
				   				ev.data.ptr = (void *) sdatap;
								ev.events = EPOLLOUT|EPOLLET;
				                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
								printf("EPOLLIN End\n");
						}
			            else if(events[i].events & EPOLLOUT) // 如果有数据发送
						{
								printf("EPLLOUT\n");
								sdatap = events[i].data.ptr;
								sockfd = sdatap->fd;
								printf("sockfd == %d\n",sockfd);
							//	printf("the address in sdatap->ptr is %lu \n",sdatap->ptr);
							//	printf("1\n");

								strcpy(buf,sdatap->ptr);
							//	printf("2\n");

								write(sockfd, buf, sizeof(buf));
								printf("has sent \"%s\" \n",buf);
								sdatap->fd = sockfd;
									//	memset(datap->buf,0,64);
								//		sdatap->ptr = datap;
								ev.data.ptr = sdatap;
								//设置用于注测的读操作事件
								ev.events = EPOLLIN|EPOLLET;
								//修改sockfd上要处理的事件为EPOLIN
								epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,&ev);
								close(sockfd);
								printf("EPOLLOUT End\n");
						}	
				}
		}
		close(listenfd);
		return 0;
}
