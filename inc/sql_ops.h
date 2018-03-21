/*
 * sql_ops.h
 *
 *  Created on: 2017-6-7
 *      Author: z
 */

#ifndef SQL_OPS_H_
#define SQL_OPS_H_

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <cctype>
#include <sqlite3.h>

class sql_ops {

public:
	typedef struct {
		int type;
		std::string id;
		std::string link;
		std::string value;
		std::string all_data;
		sqlite3_callback callback_func;
		//int (*cb_select)(void *params,int n_column,char **column_value,char **column_name);
	}device_typedef;

	sqlite3 *sqlite3_db;

	sql_ops();

	int sql_init (const char *name);
	int sql_insert (device_typedef *dev_inf);
	int sql_select (device_typedef *dev_inf);
	int sql_delete (device_typedef *dev_inf);
};


#endif /* SQL_OPS_H_ */
