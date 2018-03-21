
#include "sql_ops.h"

using namespace std;

sql_ops::sql_ops()
{
	
}
/**
 * @brief sql init
 * @param name database name
 * @return success SQLITE_OK,false SQLITE_ERROR
 * @warning
 */
int sql_ops::sql_init (const char *name)
{
	const char *sql_create_table="CREATE TABLE devive_data_tb(id INTEGER PRIMARY KEY AUTOINCREMENT,dev_type INTEGER,dev_id VARCHAR(32),dev_link VARCHAR(32),dev_all_data TEXT)";
	char *errmsg = 0;
	int ret = 0;
  
  ret = sqlite3_open(name,&sqlite3_db);
  if(ret != SQLITE_OK){
      fprintf(stderr,"Cannot open db: %s\n",sqlite3_errmsg(sqlite3_db));
      return ret;
  }

  ret = sqlite3_exec( sqlite3_db, sql_create_table, NULL, NULL, &errmsg );
  if(ret != SQLITE_OK){
      fprintf(stderr,"create table fail: %s\n",errmsg);
  }
  sqlite3_free(errmsg);
    
  return ret;
}
/**
 * @brief sql insert
 * @param dev_inf:device data
 * @return success SQLITE_OK,false SQLITE_ERROR
 * @warning
 */
int sql_ops::sql_insert (device_typedef *dev_inf)
{
	const char *sql_instert ="INSERT INTO devive_data_tb(dev_type,dev_id,dev_link,dev_all_data)";
	char sql[128];
	char *errmsg = 0;
	int ret = 0;
	
	sprintf( sql, "%s values(%d,'%s','%s')", sql_instert, dev_inf->type, dev_inf->id.data(), dev_inf->all_data.data() );
	//printf("%s\r\n", sql);
	ret = sqlite3_exec(sqlite3_db, sql, NULL, NULL, &errmsg);
	if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %s\n",errmsg);
	}
	sqlite3_free(errmsg);
	return ret;
}
/**
 * @brief sql select
 * @param dev_inf:device data
 * @return success SQLITE_OK,false SQLITE_ERROR
 * @warning
 */
int sql_ops::sql_select (device_typedef *dev_inf)
{
	const char *sql_instert ="SELECT * FROM devive_data_tb";
	char sql[128];
	char *errmsg = 0;
	int ret = 0;
	
	if(!dev_inf->id.empty())
		sprintf(sql, "%s WHERE dev_type=%d AND dev_id='%s'", sql_instert, dev_inf->type, dev_inf->id.data());
	else
		sprintf(sql, "%s WHERE dev_type=%d AND dev_link='%s'", sql_instert, dev_inf->type, dev_inf->link.data());
  //select data
  ret = sqlite3_exec(sqlite3_db,sql,dev_inf->callback_func,NULL,&errmsg);
  if(ret != SQLITE_OK){
      fprintf(stderr,"query SQL error: %s\n",errmsg);
  }
  sqlite3_free(errmsg);
  return ret;	
}
/**
 * @brief sql delete
 * @param dev_inf:device data
 * @return success SQLITE_OK,false SQLITE_ERROR
 * @warning
 */
int sql_ops::sql_delete (device_typedef *dev_inf)
{
	const char *sql_delete ="DELETE FROM devive_data_tb";
	char sql[128];
	char *errmsg = 0;
	int ret = 0;
	
	sprintf( sql, "%s WHERE dev_type=%d AND dev_id='%s'", sql_delete, dev_inf->type, dev_inf->id.data() );

  //select data
  ret = sqlite3_exec(sqlite3_db,sql,dev_inf->callback_func,NULL,&errmsg);
  if(ret != SQLITE_OK){
      fprintf(stderr,"query SQL error: %s\n",errmsg);
  }
  sqlite3_free(errmsg);
  return ret;	
}

