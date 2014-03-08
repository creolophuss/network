#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
//#include "myhdb.h"
#include "cmd_handler.h"
//typedef extern DB_DESCRIPTOR db_handler;
void init_opened_list(opened_list *list)
{
		opened_list *head = list;
		head->db_ptr = NULL;
		head->next = NULL;
		head->counter = 0;	
		pthread_mutex_init(&(list->list_mutex),NULL);
}

opened_list *search_list(opened_list *list,char *dbname)
{
//		pthread_mutex_lock(&(list->list_mutex));
		
		opened_list *head = list;
		opened_list *lp = head->next;
		while(lp != NULL)
		{
				if(equal(lp->dbname,dbname))
				{
						printf("DB:%s has already been opened.\n",dbname);
//						pthread_mutex_unlock(&(list->list_mutex));
						return lp;
				}

				lp = lp->next;
		}
//		pthread_mutex_unlock(&(list->list_mutex));
		return NULL;
}

void insert_list(opened_list *list,opened_list *node)
{
		opened_list *p,*q;
		p = list;
		q = p->next;
		p->next = node;
		node->next = q;
}

char *process(data_t *datap)
{
		printf("process %s\n",datap->buf);
		opened_list *list;
		list = datap->list;
		char *command ;
		command = datap->buf;
		DB_DESCRIPTOR db_ptr = datap->db_ptr;

		struct cmd_arg cmd_list;
		printf("command is %s",command);	
//		char *datap->buf ;
//		data->buf = (char *)malloc(MAX_CMD_LENGTH*sizeof(char));
		if( datap->buf == NULL)
		{
				printf("no command\n");
				exit(0);
		}
		/* construct the command string. */			
		make_cmd2(command,&cmd_list);
		int argc = cmd_list.argc;	
		int k;
		for(k = 0; k < argc; k++)
		{
				printf("%s\n",cmd_list.content[k]);
		}

			
		/*
		 * The matching procedure of the command.
		 */
/*				
		if(equal(cmd_list.content[0],"login"))
		{
		}
		else */
		if(equal(cmd_list.content[0],"exit"))
		{
				close_hdb(db_ptr);
				strcpy(datap->buf,"Successly Closed.");
				//return datap->buf;
		}
		else if(equal(cmd_list.content[0],"open"))
		{
				printf("\n");
				printf("start open()\n");

				opened_list *listp ;
				pthread_mutex_lock(&(list->list_mutex));
				listp = search_list(datap->list,cmd_list.content[1]);
				pthread_mutex_unlock(&(list->list_mutex));

				if(listp == NULL)
				{
						listp =	(opened_list *)malloc(sizeof(opened_list));
						listp->counter = 1;
						pthread_mutex_init(&(listp->list_mutex),NULL);
						strcpy(listp->dbname, cmd_list.content[1]);
						strcpy(datap->dbname, cmd_list.content[1]);
						db_ptr = open_hdb(cmd_list.content[1]);
						listp->db_ptr = db_ptr;
						datap->db_ptr = db_ptr;
						
						opened_list *again;
						pthread_mutex_lock(&(list->list_mutex));
					    again = search_list(datap->list,cmd_list.content[1]);
						if(again == NULL)
								insert_list(list,listp);
						else{
								pthread_mutex_lock(&(again->list_mutex));
								again->counter++;
								pthread_mutex_unlock(&(again->list_mutex));
								datap->db_ptr = again->db_ptr;
								strcpy(datap->dbname,again->dbname);
								free(listp);
						}
						pthread_mutex_unlock(&(list->list_mutex));
				}
				else{
						pthread_mutex_lock(&(listp->list_mutex));
						listp->counter++;
						pthread_mutex_unlock(&(listp->list_mutex));
						datap->db_ptr = listp->db_ptr;
						strcpy(datap->dbname,listp->dbname);
				}
				printf("open complete\n");
				printf("\n");
				strcpy(datap->buf,"Open complete.");

		}
		else if(equal(cmd_list.content[0],"write"))
		{
				printf("\n");
				printf("start to write\n");
				write2_hdb(db_ptr,cmd_list.content[1],cmd_list.content[2]);
				printf("Write complete.\n");
				printf("\n");
				strcpy(datap->buf,"Write complete.");
				//return datap->buf;
		}
		else if(equal(cmd_list.content[0],"read"))
		{
				printf("\n");
				value_type value;;
				value = readby_key(db_ptr,cmd_list.content[1]);
				printf("%s\n",value);
				printf("\n");
				strcpy(datap->buf,value);
				free(value);
		}
		else 
		{
				strcpy(datap->buf,"Unknow command.");
		}
		free_cmd2(&cmd_list);
		
		pthread_mutex_unlock(datap->mutexp);
		return datap->buf;
}		

void free_cmd2(cmd_arr cmd_list)
{
		int i;
		for(i = 0; i < cmd_list->argc; i++)
				free(cmd_list->content[i]);
}

int equal(char *input,char *command)
{
		char *p,*q;
		p = input;
		q = command;
		if( strlen(input) != strlen(command))
				return 0;
		while(*q != '\0' && *p != '\0')
		{
				if(*q != *p)
						return 0;
				p++;
				q++;
		}
		return 1;
}


int make_cmd2(char *command,cmd_arr array)
{
		int i,n;
		int len;
		(array->argc) = 0;
		char *first,*last;
		char *p;
		p = command;
		n = strlen(command);
		printf("%s LEN: %d\n",command,n);

		do{
				while(*p == ' ' || *p == '\t' || *p == '\n')
						p++;
				if(*p != '\0')
				{
						first = last = p;
				}
				while(*p != ' ' && *p != '\t' && *p != '\n' && *p != '\0')
				{
						p++;
						last++;
				}
				*p = '\0';
				len = last - first + 1;
				array->content[array->argc] = (char *)malloc(len*sizeof(char));
				strcpy(array->content[array->argc],first);
				(array->argc)++;
				p++;
		}while(p <= (command + n));
}
