
/******************************************************************************************
 *                      WangJiayi's simple database headfile.                             *
 *                      It's just a prototype,a lot of things need to be included         *
 *                      in the next version.                                              *
 *                      Version 0.1.                                                      *
 ******************************************************************************************/


/* The length limit of name of the hdb        */
#define MAX_NAME_LENGTH 32

/* Definition for the type of a key word      */
typedef char* key_type;

/* Definition for the type of a value         */
typedef char* value_type;

/* Definition for the type of a record        */
typedef struct record_t{
		key_type key;
		value_type value;
} *record_type;

//typedef TCHDB *hdb_t;

/*
 * Declare a struct fo some informaton about a table of database,
 * every record of this kind is stored in the file "Tables".
 * The struct has a 5 members,we mainly use 3 of them:
 * n_records tells us how many records are there in one table;
 * hdb_name tells us the name of the table;
 * pos stores the positon of this record in the "Tables".
 * We also use this struct as a handler to operate on the HDB.
*/
typedef struct my_hdb{
		void  *hdb_id;
		int   table_id;
		int   n_records;
		char  hdb_name[MAX_NAME_LENGTH];
		long  pos;
} *DB_DESCRIPTOR;
/*
 * This function uses the parameter "name" to read a record ,from the
 * file "Tables" in which there stores the information about a
 * database table,to create a handler to operate on the DB.The record
 * is returned as a DB_DESCRIPTOR type.
 */
DB_DESCRIPTOR create_hdb_handler(const char *name);
/*
 * This function helps us open a database table according to the name
 * parameter and atomatically generates a handler as the return value.
 */
DB_DESCRIPTOR open_hdb(const char *hdb_name);
/*
 * This function reads a record from the hdb descripted by the db_pointer
 * parameter,the reccord is stored in the other parameter record.
 * It retruns 0 if success.
 */
int read_hdb(DB_DESCRIPTOR db_pointer,record_type record);
/*
 * This function writess a record to the hdb descripted by the db_pointer
 * parameter,the reccord is stored in the other parameter record.
 * It retruns 0 if success.
 */
int write_hdb(DB_DESCRIPTOR db_pointer,record_type record);
int write2_hdb(DB_DESCRIPTOR db_pointer,key_type key,value_type value);
/*
 * If we want to close the database table,we can tansfer the db_pointer as
 * a parameter to this function.
 * It retruns 0 if success.
 */
int close_hdb(DB_DESCRIPTOR db_pointer);
/*
 * This function returns the value to the key in the database table respect
 * to the parameter db_pointer.
 */
value_type readby_key(DB_DESCRIPTOR db_pointer,key_type key);

