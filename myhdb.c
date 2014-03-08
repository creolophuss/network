#include <tcutil.h>
#include <tchdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "myhdb.h"
#define  MAX_NAME_LENGTH 32

/*
 This function uses the parameter "name" to read a record ,from the
 file "Tables" in which there stores the information about a
 database table,to create a handler to operate on the DB.The record
 is returned as a DB_DESCRIPTOR type.
*/
DB_DESCRIPTOR create_hdb_handler(const char *name)
{
		FILE *fp;
        int n_tables = 0;
		int n = 0;
		fp  = fopen("Tables","r+");
		if(fp == NULL)
		{
				printf("Can not open Tables.\n");
				exit(-1);
		}
        DB_DESCRIPTOR ptr = (DB_DESCRIPTOR)malloc(sizeof(struct my_hdb));
        printf("begin to create handler.\n");
		do{
				//fgetpos(fp,ptr->pos);
                ptr->pos = ftell(fp);
				n = fread(ptr,sizeof(struct my_hdb),1,fp);
                printf("fread returns %d.\n",n);
				if((strcmp(ptr->hdb_name,name) == 0) && (strcmp(name,ptr->hdb_name) == 0))
						break;
		}while(n);
		if (n == 0)
		{
				ptr->n_records = 0;
				strncpy(ptr->hdb_name,name,MAX_NAME_LENGTH);
                printf("table name is %s\n",ptr->hdb_name);
				fwrite(ptr,sizeof(struct my_hdb),1,fp);
		}
		ptr->hdb_id = tchdbnew();
		return ptr;
}
/*
 This function helps us open a database table according to the name
 parameter and atomatically generates a handler as the return value.
*/
DB_DESCRIPTOR open_hdb(const char *hdb_name)
{
		int ecode;
        printf("begin to open hdb.\n");
		DB_DESCRIPTOR db_pointer = create_hdb_handler(hdb_name);
		if(!tchdbopen(db_pointer->hdb_id, db_pointer->hdb_name, HDBOWRITER | HDBOCREAT))
		{
				ecode = tchdbecode(db_pointer->hdb_id);
				fprintf(stderr, "open error: %s\n", tchdberrmsg(ecode));
				exit(-1);
		}
		tchdbiterinit(db_pointer->hdb_id);
		return db_pointer;
}
/*
 This function reads a record from the hdb descripted by the db_pointer
 parameter,the reccord is stored in the other parameter record.
 It retruns 0 if success.
*/
int read_hdb(DB_DESCRIPTOR db_pointer,record_type record)
{
		if((record->key = tchdbiternext2(db_pointer->hdb_id)) != NULL)
		{
				record->value = tchdbget2(db_pointer->hdb_id, record->key);
				return 0;
		}
		else 
				return -1;
}


/*
 This function writess a record to the hdb descripted by the db_pointer
 parameter,the reccord is stored in the other parameter record.
 It retruns 0 if success.
*/
int write_hdb(DB_DESCRIPTOR db_pointer,record_type record)
{
		int ecode;
		if(!tchdbput2(db_pointer->hdb_id, record->key, record->value))
		{
				ecode = tchdbecode(db_pointer->hdb_id);
				fprintf(stderr, "put error: %s\n", tchdberrmsg(ecode));
				return -1;
		}
		db_pointer->n_records++;
		return 0;
}
int write2_hdb(DB_DESCRIPTOR db_pointer,key_type key,value_type value)
{
		int ecode;
		if(!tchdbput2(db_pointer->hdb_id, key , value))
		{
				ecode = tchdbecode(db_pointer->hdb_id);
				fprintf(stderr, "put error: %s\n", tchdberrmsg(ecode));
				return -1;
		}
		db_pointer->n_records++;
		return 0;
}
/*
 If we want to close the database table,we can tansfer the db_pointer as
 a parameter to this function.
 It retruns 0 if success.
*/
int close_hdb(DB_DESCRIPTOR db_pointer)
{
		int ecode;
		FILE *fp;
        int n;         
		fp  = fopen("Tables","r+");
		//fsetpos(fp,db_pointer->pos);
        fseek(fp,db_pointer->pos,SEEK_SET);
		fwrite(db_pointer,sizeof(struct my_hdb),1,fp);
		if(!tchdbclose(db_pointer->hdb_id)){
				ecode = tchdbecode(db_pointer->hdb_id);
				fprintf(stderr, "close error: %s\n", tchdberrmsg(ecode));
				return -1;
		}
		/* delete the object */
		tchdbdel(db_pointer->hdb_id);
		return 0;
}
/*
 This function returns the value to the key in the database table respect
 to the parameter db_pointer.
*/
value_type readby_key(DB_DESCRIPTOR db_pointer,key_type key)
{
		return tchdbget2(db_pointer->hdb_id,key);
}
