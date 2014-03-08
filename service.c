#include<fcntl.h>
#include<stdio.h>
//#include<iostream>
#include<arpa/inet.h> /* internet socket */
#include<string.h>
#include<stdlib.h>
//#define NDEBUG
#include<assert.h>
#include<pthread.h>
//#define MPORT 5001
#define IP_ADDR "127.0.0.1"
#define MAX_CONNECT_QUEUE 1024
#define MAX_BUF_LEN 512
#include<sys/epoll.h>
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
int main(int argc,char **argv)
{
		if(argc != 2)
		{
				printf("INPUT ERROR!\n");
				printf("Usage: command(./server) +  port number \n");
				exit(-1);
		}
		int int_port = atoi(argv[1]);
		short PORT = (short)int_port;
		printf("%s %s %hd\n",argv[0],argv[1],PORT);
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
		struct sockaddr_in serveraddr;
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(5001);
		serveraddr.sin_addr.s_addr = inet_addr(IP_ADDR);
		memset(&serveraddr.sin_zero, 0, 8);
		int ret = connect(listenfd,(struct sockaddr *)&serveraddr,sizeof(struct sockaddr));
		if(ret != 0)
		{
				printf("connect err\n");
				exit(-1);
		}
		strcpy(buf,"server connect");
		size_t len = strlen("server connect"); 
		strcpy(buf+len+1,argv[1]);
		printf("%s\n",buf);
		printf("%s\n",buf+len+1);
		int rgst_state = 0;
		printf("before send\n");
		while(rgst_state == 0)
		{
				int nc = send(listenfd,buf,sizeof(buf),0);
				if(nc < 0)
				{
						printf("send err\n");
						printf("ret of send is %d\n",nc);
				}
				else{
						printf("has sent \" %s \" \n",buf);
				}
				ret = recv(listenfd,buf,MAX_BUF_LEN,0);
				printf("recv ret = %d\n",ret);
				if( strcmp(buf,"connect ok") == 0)
				{
						rgst_state = 1;
						printf("recv %s\n",buf);
				}
		}
		printf("after registe\n");
		close(listenfd);
		listenfd = socket(PF_INET,SOCK_STREAM,0);
		ret = bind(listenfd,(struct sockaddr *)&addr,sizeof(struct sockaddr));
		if(ret == -1)
		{
				fprintf(stderr,"Bind Error,%s:%d,",__FILE__,__LINE__);
				fprintf(stderr,"%s:%d\n",(char*)inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
				close(sockfd);
				return -1;
		}		
		else if(ret == 0)
				printf("bind ok\n");
		printf("%hd\n",ntohs(addr.sin_port));

		make_socket_non_blocking (listenfd);  
		
		opened_list list;
		init_opened_list(&list); 		
		
		struct epoll_event ev;
		int epfd = epoll_create(256);
		 ev.data.fd = listenfd;
		 ev.events = EPOLLIN|EPOLLET;
		epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
		ret = listen(listenfd,MAX_CONNECT_QUEUE);
		int connfd;
		pthread_t *threadp;
		data_t *datap; 
		sockp *sdatap;
		int n;
		struct epoll_event events[20];
		while(1)
		{
		
				
				int  nfds = epoll_wait(epfd,events,20,-1);
//				printf("wait nfds == %d \n",nfds);
				int i;
				for( i = 0; i < nfds; ++i)
				{
						if(events[i].data.fd == listenfd)//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
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

								sdatap = (sockp *)malloc( sizeof(sockp));
								sdatap->fd = connfd;
								sdatap->ptr = NULL;
								ev.data.ptr =(void *)sdatap;
								ev.events = EPOLLIN|EPOLLET;										
								epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);

								printf("Finish Accept %d\n",connfd);
						}
						else if(events[i].events & EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读入。
						{
								printf("EPOLLIN\n");

								sdatap = (sockp *)events[i].data.ptr;
								datap = sdatap->ptr;
							/*	if((datap != NULL) && (datap->state == 0))
								{
										datap->state = 1;
										continue;
								}
							*/	
								printf("events[i].data.fd == %d\n",sdatap->fd);
								if ( (sockfd = sdatap->fd) < 0)
								{
										printf("continur\n");	
										continue;
								}
								printf("sockfd ==  %d\n",sockfd);

								if ( (n = read(sockfd, line, MAXLINE)) < 0) 
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
				                		//修改sockfd上要处理的事件为EPOLLOUT

				                		epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
										printf("EPOLLIN END\n");
										continue;
								}
								printf("Read %d characters\n",n);
								line[n] = '\0';							
								//printf("Has read %s \n",line);
								printf("strlen line == %d\n",strlen(line));
						//		if(strlen(line) == 0)
						//				continue;
								if(sdatap->ptr == NULL)
								{
										datap = (data_t *)malloc( sizeof(data_t));
									//	datap->buf = (char *)malloc(64*sizeof(char));
										datap->mutexp = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
										pthread_mutex_init(datap->mutexp,NULL);
								}
								else
										datap = (data_t *)(sdatap->ptr);
								
								pthread_mutex_lock(datap->mutexp);
								datap->buf = (char *)malloc(64*sizeof(char));
								strcpy(datap->buf,line);
								threadp = (pthread_t *)malloc(sizeof(pthread_t));
								datap->threadp = threadp;
								datap->list = &list;

								data_t *datapp = datap;
								pthread_create(threadp,NULL,(void *)process,(void*)datapp);
								printf("read %s \n",line);
				   				sdatap->fd = sockfd;
								sdatap->ptr = datapp;
				   				ev.data.ptr = (void *) sdatap;
								ev.events = EPOLLOUT|EPOLLET;
				                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);

								printf("datap->buf ==  %s \n",datapp->buf);
								printf("EPOLLIN End\n");
						}
			            else if(events[i].events & EPOLLOUT) // 如果有数据发送
						{
							//	printf("EPLLOUT\n");
								sdatap = events[i].data.ptr;
								sockfd = sdatap->fd;
								datap = sdatap->ptr;
							//	printf("trylock start\n");

								ret = pthread_mutex_trylock(datap->mutexp);
							//	printf("trylock end \nret == %d\n",ret);
								if(ret != 0)
								{
										sdatap->fd = sockfd;
										sdatap->ptr = datap;
										ev.data.ptr = sdatap;
								//设置用于注测的读操作事件
										ev.events = EPOLLOUT|EPOLLET;
										epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);

										continue;
								}
								else if(ret == 0)
								{
										strcpy(line,datap->buf);

										printf("After copy\n");
										//write(sockfd, line, n);
										send(sockfd,line,strlen(line)+1,0);
										printf("Send %s finish\n",line);
										//设置用于读操作的文件描述符
										free(datap->buf);
										free(datap->threadp);
										sdatap->fd = sockfd;
										datap->buf = NULL;
										datap->state = 0;
									//	memset(datap->buf,0,64);
										sdatap->ptr = datap;
										ev.data.ptr = sdatap;
								//设置用于注测的读操作事件
										ev.events = EPOLLIN|EPOLLET;
								//修改sockfd上要处理的事件为EPOLIN
										epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
										ret = pthread_mutex_unlock(datap->mutexp);
										printf("EPOLLOUT End\n");
								}	
						}
				}
		}
		close(listenfd);
		return 0;
}
