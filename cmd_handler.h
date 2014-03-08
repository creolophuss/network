#include "myhdb.h"
/*
 *  The max space to store a command string.
 */ 
#define MAX_CMD_LENGTH 128
/*
 * The limit of the max number of arguments
 * a command can have.
 */
#define MAX_ARG_NO 16
/*
 * The max space of a key can have.
 */   
#define MAX_KEY_LENGTH 64
/*
 * The max space of a value can have.
 */   
#define MAX_VALUE_LENGTH 64
/*
 * A struct node hold the input command,
 * content[0] points to operater , 
 * content[i] (i > 0) points to other 
 * arguments.
 * argc indicates the total number 
 * of arguments.
 */

typedef struct opened_node
{
		DB_DESCRIPTOR db_ptr;
		char dbname[32];
		int counter;
		struct opened_node *next;
		pthread_mutex_t list_mutex;
} opened_list;
#define ACCEPT_END 0
#define READ_END 1
#define WRITE_end 2
typedef struct data
{
//		int sockfd;
		char dbname[32];
		DB_DESCRIPTOR db_ptr;
		pthread_mutex_t *mutexp;
		pthread_t *threadp;
		opened_list *list;
		char *buf;
		int state;
} data_t;
typedef struct sock
{
		int fd;
		data_t *ptr;
} sockp;
typedef struct cmd_arg{
		char *content[MAX_ARG_NO];
		int argc;
} *cmd_arr;

//void data_t_destroy(data_t *datap);
void init_opened_list(opened_list *list);
opened_list *search_list(opened_list *list,char *dbname);
void insert_list(opened_list *list,opened_list *node);
/*
 * Extract each argument from a string 
 * pointed by command, and store the 
 * arguments into the command array.
 * Return 0 if success.
 */ 
int make_cmd2(char *command,cmd_arr array);
/*
 * Compare the operater of the input string
 * with one particular command.
 * Return 1 if the two match each other, and 
 * return 0 otherwise.
 */ 
int equal(char *input,char *command);
/*
 * Release the memory allocated
 * to a command array after the command has 
 * been done, lest too manys commands
 * will make the program run out of memory.
 */
void free_cmd2(cmd_arr cmd_list);
/*
 * Process the command string,and return the
 * result as a string.
 */
char *process(data_t *datap);
