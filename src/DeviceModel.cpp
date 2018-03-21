/*
 * DeviceModel.cpp
 *
 *  Created on: Dec 24, 2017
 *      Author: zhuang
 */

#include "DeviceModel.h"
#include <stdlib.h>

#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG(format,...) do{printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__);fflush(stdout);}while(0)
#else
#define DDEBUGformat,...)
#endif


typedef struct list_node{
	uint32_t index;
	const char *str;
}ListNode;

DeviceModel::DeviceModel() {
	// TODO Auto-generated constructor stub

}

DeviceModel::~DeviceModel() {
	// TODO Auto-generated destructor stub

}


int DeviceModel::getCmdType(cJSON *json){
	static const ListNode cmd_list[]={
		{ENUM_CMD_ADD, CMD_ADD},
		{ENUM_CMD_DELETE, CMD_DELETE},
		{ENUM_CMD_EDIT, CMD_EDIT},
		{ENUM_CMD_GET, CMD_GET},
		{ENUM_CMD_TRIG, CMD_TRIG},
		{ENUM_CMD_RESP, CMD_RESP},
		{ENUM_CMD_BACK, CMD_BACK},
		{ENUM_CMD_LOGIN, CMD_LOGIN},
		{ENUM_CMD_REGISTER, CMD_REGISTER},
		{ENUM_CMD_GET_LIST, CMD_GET_LIST},
		{ENUM_CMD_ADD_LOCK, "add_lock"},
		{ENUM_CMD_DELETE_LOCK, "delete_lock"},
		{ENUM_CMD_SET_TEMP_PASSWORD, "set_temp_password"},
		{ENUM_CMD_INVALIT_TEMP_PASSWORD, "invalid_temp_password"},
		{ENUM_CMD_SET_HOLD_PASSWORD, "set_hold_password"},
		{ENUM_CMD_NEW_USER, "new_user"},
		{ENUM_CMD_DELETE_USER, "delete_user"},
		{ENUM_CMD_ADD_DIGIT_PASSWORD, "add_didit_password"},
		{ENUM_CMD_EDIT_DIGIT_PASSWORD, "edit_digit_password"},
		{ENUM_CMD_DELETE_DIGIT_PASSWORD, "delete_digit_password"},
		{ENUM_CMD_ENTER_LEARN, "enter_learn"},
		{ENUM_CMD_OUT_LEARN, "out_learn"},
		{ENUM_CMD_EDIT_PASSWORD_ATTRIBUTE, "edit_password_attribute"},
		{ENUM_CMD_SET_AGING_DATE, "set_aging_date"},
		{ENUM_CMD_SET_SETTING, "set_setting"}
	};

	uint16_t i = 0;
	cJSON *cmdSub = cJSON_GetObjectItem(json, CMD_STR);
	if(cmdSub == NULL)
		return ENUM_ERROR_UNKNOWN;
	else{
		for(i=0; i< ( sizeof(cmd_list) / sizeof(cmd_list[0]) ); i++){
			if( strcmp(cmdSub->valuestring, cmd_list[i].str ) == 0){
				return cmd_list[i].index;
			}
		}
		return ENUM_ERROR_INVALID_CMD;
	}
}


int DeviceModel::getDevType(cJSON *json){
	cJSON *devSub = cJSON_GetObjectItem(json, TYPE_STR);
	if(devSub == NULL)
		return ENUM_ERROR_UNKNOWN;
	else{
		return devSub->valueint;
	}
}

int DeviceModel::getConditionType(cJSON *json)
{
	static const ListNode dev_condition_type_list[]={
		{CONDITION_LAGER, ">"},
		{CONDITION_LAGER_EQUAL, ">="},
		{CONDITION_LITTLE, "<"},
		{CONDITION_LITTLE_EQUAL, "<="},
		{CONDOTION_EQUAL, "=="},
		{CONDITION_IN_OUT, "inout"},
		{CONDITION_CHANGE, "change"},
	};

	uint16_t i = 0;
	cJSON *devSub = cJSON_GetObjectItem(json, TYPE_STR);
	if(devSub == NULL)
		return ENUM_ERROR_UNKNOWN;
	else{
		for(i=0; i< ( sizeof(dev_condition_type_list) / sizeof(dev_condition_type_list[0]) ); i++){
			if( strcmp(devSub->valuestring, dev_condition_type_list[i].str ) == 0){
				return dev_condition_type_list[i].index;
			}
		}
		return ENUM_ERROR_INVALID_DEV;
	}
}

bool j2sBasic(cJSON *json, DevBasicTypedef *dev){
	cJSON *cj_item = NULL;
	char *endptr=NULL;
	//
	cj_item = cJSON_GetObjectItem(json, "type");
	if(cj_item){
		dev->dev_type = cj_item->valueint;
	}
	cj_item = cJSON_GetObjectItem(json, "devid");
	if(cj_item){
		dev->dev_id = cj_item->valueint;
	}
	cj_item = cJSON_GetObjectItem(json, "mac");
	if(cj_item){
		dev->mac = strtoull(cj_item->valuestring, &endptr, 16);
	}
	return true;
}
bool s2jFingerPrintLock(cJSON *json, tsDevFingerPrintLockTypedef *dev){

}
bool j2sFingerPrintLock(cJSON *json, tsDevFingerPrintLockTypedef *dev){
	cJSON *cj_item = NULL;
	//user
	cj_item = cJSON_GetObjectItem(json, "user");
	if(cj_item){
		//user - id
		cJSON *cj_user_item = cJSON_GetObjectItem(cj_item, "id");
		if(cj_user_item){
			dev->user.id = cj_user_item->valueint;
		}
		//user - type
		cj_user_item = cJSON_GetObjectItem(cj_item, "type");
		if(cj_user_item){
			dev->user.type = cj_user_item->valueint;
		}
		//user - password - isen
		cj_user_item = cJSON_GetObjectItem(cj_user_item, "isen");
		if(cj_user_item){
			dev->user.en = (bool)cj_user_item->valueint;
		}
		//user - aging_date
		cj_user_item = cJSON_GetObjectItem(cj_item, "aging_date");
		if(cj_user_item){
			for(int i=0; i< cJSON_GetArraySize(cj_user_item); i++){
				cJSON *cj_date_item = cJSON_GetArrayItem(cj_user_item,i);
				if(cj_date_item){
					//user - aging_date - w
					cJSON *cj_date_array_item = cJSON_GetObjectItem(cj_date_item, "w");
					if(cj_date_array_item)
						strcpy(dev->user.aging_date[i].week, cj_date_array_item->valuestring);
					//user - aging_date - y
					cj_date_array_item = cJSON_GetObjectItem(cj_date_item, "y");
					if(cj_date_array_item)
						dev->user.aging_date[i].year = cj_date_array_item->valueint;
					//user - aging_date - mo
					cj_date_array_item = cJSON_GetObjectItem(cj_date_item, "mo");
					if(cj_date_array_item)
						dev->user.aging_date[i].mounth = cj_date_array_item->valueint;
					//user - aging_date - d
					cj_date_array_item = cJSON_GetObjectItem(cj_date_item, "d");
					if(cj_date_array_item)
						dev->user.aging_date[i].mday = cj_date_array_item->valueint;
					//user - aging_date - h
					cj_date_array_item = cJSON_GetObjectItem(cj_date_item, "h");
					if(cj_date_array_item)
						dev->user.aging_date[i].hour = cj_date_array_item->valueint;
					//user - aging_date - mi
					cj_date_array_item = cJSON_GetObjectItem(cj_date_item, "mi");
					if(cj_date_array_item)
						dev->user.aging_date[i].minute = cj_date_array_item->valueint;
				}
			}
		}
		//user - password
		cj_user_item = cJSON_GetObjectItem(cj_item, "password");
		if(cj_user_item){
			//user - password - id
			cJSON *cj_password_item = cJSON_GetObjectItem(cj_user_item, "id");
			if(cj_password_item){
				dev->user.password.id = cj_password_item->valueint;
			}
			//user - password - type
			cj_password_item = cJSON_GetObjectItem(cj_user_item, "type");
			if(cj_password_item){
				dev->user.password.type = cj_password_item->valueint;
			}
			//user - password - isen
			cj_password_item = cJSON_GetObjectItem(cj_user_item, "isen");
			if(cj_password_item){
				dev->user.password.en = (bool)cj_password_item->valueint;
			}
			//user - password - password
			cj_password_item = cJSON_GetObjectItem(cj_user_item, "password");
			if(cj_password_item){
				dev->user.password.password = cj_password_item->valueint;
			}
		}
	}
	//seting
	cj_item = cJSON_GetObjectItem(json, "setting");
	if(cj_item){
		//seting - upload
		cJSON *cj_setting_item = cJSON_GetObjectItem(cj_item, "upload");
		if(cj_setting_item)
			dev->setting.upload = cj_setting_item->valueint;
		//seting - batter
		cj_setting_item = cJSON_GetObjectItem(cj_item, "batter");
		if(cj_setting_item)
			dev->setting.batter = cj_setting_item->valueint;
		//seting - digit_warning
		cj_setting_item = cJSON_GetObjectItem(cj_item, "digt_warning");
		if(cj_setting_item)
			dev->setting.digt_warning = cj_setting_item->valueint;
		//seting - finger_warning
		cj_setting_item = cJSON_GetObjectItem(cj_item, "finger_warning");
		if(cj_setting_item)
			dev->setting.finger_warning = cj_setting_item->valueint;
		//seting - iccard_warning
		cj_setting_item = cJSON_GetObjectItem(cj_item, "iccard_warning");
		if(cj_setting_item)
			dev->setting.iccard_warning = cj_setting_item->valueint;
	}
	//temp_password
	cj_item = cJSON_GetObjectItem(json, "temp_password");
	if(cj_item){
		dev->temp_password = cj_item->valueint;
	}
	//hold_password
	cj_item = cJSON_GetObjectItem(json, "hold_password");
	if(cj_item){
		dev->hold_password = cj_item->valueint;
	}
	return true;
}
int DeviceModel::convertJson2Struct(cJSON *json, uint32_t dev_type, void *dev_struct){
	int err = 0;
	DevButtonTypedef *dev_button = NULL;
	DevSwitchTypedef *dev_switch = NULL;
	DevSensorTypedef *dev_sensor = NULL;
//	DevInfraredTypedef *dev_infrared = NULL;
//	DevEventTypedef *dev_event = NULL;
	DevTriggerTypedef *dev_trigger = NULL;
	cJSON *devidSub = NULL, *cj_mac = NULL,
			*statusSub, *conditionSub = NULL, *actionSub = NULL,
			*cj_ch = NULL, *cj_value=NULL;
	char *condition_str = NULL, *action_str = NULL;
	char *endptr=NULL;

	devidSub = cJSON_GetObjectItem(json, DEVID_STR);

	switch(dev_type)
  {
//  	case ENUM_DEV_BUTTON:
//		dev_button = (DevButtonTypedef*)dev_struct;
//		dev_button->dev_basic.dev_type = dev_type;
//		dev_button->dev_basic.dev_id = atoi(devidSub->valuestring);
// 		break;

	case ENUM_DEV_SWITCH_1:
	case ENUM_DEV_SWITCH_2:
	case ENUM_DEV_SWITCH_3:
	case ENUM_DEV_SWITCH_4:
	case ENUM_DEV_ON_OFF_PLUG:
	case ENUM_DEV_ON_OFF_86_PLUG:
	case ENUM_DEV_ON_OFF_CURTAIN:
	case ENUM_DEV_DIMMING_1:
	case ENUM_DEV_DIMMING_2:
	case ENUM_DEV_CURTAIN_1:
	case ENUM_DEV_CURTAIN_2:
	case ENUM_DEV_PUSH:
	case ENUM_DEV_BLINDS:
	case ENUM_DEV_COLOR_LED:
	case ENUM_DEV_CO:
	case ENUM_DEV_SMOKE:
	case ENUM_DEV_WATER:
	case ENUM_DEV_GAS:
	case ENUM_DEV_HUMAN:
	case ENUM_DEV_TEMP_HUMIDITY:
	case ENUM_DEV_DOOR_CONTACT:
	case ENUM_DEV_LUMINESCENCE:
		dev_sensor = (DevSensorTypedef*)dev_struct;
		dev_sensor->dev_basic.dev_type = dev_type;
		if(devidSub)
			dev_sensor->dev_basic.dev_id = devidSub->valueint;

		cj_mac = cJSON_GetObjectItem(json, MAC_STR);
		if(cj_mac)
			dev_sensor->dev_basic.mac = strtoull(cj_mac->valuestring, &endptr, 16);
		else
			dev_sensor->dev_basic.mac = 0;

		cj_ch = cJSON_GetObjectItem(json, CH_STR);
		if(cj_ch)
			dev_sensor->ch = cj_ch->valueint;
		else
			dev_sensor->ch=0;
		dev_sensor->ch = (dev_sensor->ch > 4) ? 4 : dev_sensor->ch;

		cj_value = cJSON_GetObjectItem(json, VALUE_STR);
		if(cj_value)
			dev_sensor->u32_value[dev_sensor->ch] = cj_value->valueint;
		break;
  	case ENUM_DEV_EVENT:
		err = ENUM_ERROR_INVALID_DEV;
		break;
  	case ENUM_DEV_TRIGGER:
		dev_trigger = (DevTriggerTypedef*)dev_struct;
		dev_trigger->dev_basic.dev_type = dev_type;
		if(devidSub)
			dev_trigger->dev_basic.dev_id = devidSub->valueint;

		//如果没有sta字段，则status = -1
		statusSub = cJSON_GetObjectItem(json, STA_STR);
		if(statusSub){
			dev_trigger->status = statusSub->valueint;
		}else{
			dev_trigger->status = -1;
		}
		//if
		conditionSub = cJSON_GetObjectItem(json, IF_STR);
		condition_str = cJSON_PrintUnformatted(conditionSub);
		if(condition_str)
			strcpy(dev_trigger->condition, condition_str);
		else
			strcpy(dev_trigger->condition, "");
		//then
		actionSub = cJSON_GetObjectItem(json, THEN_STR);
		action_str = cJSON_PrintUnformatted(actionSub);
		if(action_str)
			strcpy(dev_trigger->action, action_str);
		else
			strcpy(dev_trigger->action, "");

		if(condition_str)
			free(condition_str);
		if(action_str)
			free(action_str);
		break;
  	default:
		err = ENUM_ERROR_INVALID_DEV;
  		break;
  }
	return err;

}


//int DeviceModel::convertStruct2Json(cJSON *json, uint32_t dev_type, void *dev_struct){
int convertStruct2Json(cJSON *json, uint32_t dev_type, void *dev_struct){
	int err = 0;
//	DevButtonTypedef *dev_button = NULL;
//	DevSwitchTypedef *dev_switch = NULL;
	DevSensorTypedef *dev_sensor = NULL;
//	DevInfraredTypedef *dev_infrared = NULL;
//	DevEventTypedef *dev_event = NULL;
	DevTriggerTypedef *dev_trigger = NULL;
	char str_buf[24];
	cJSON *cj_condition = NULL;
	cJSON *cj_action = NULL;

	switch(dev_type)
  {
//  	case ENUM_DEV_BUTTON:
//		dev_button = (DevButtonTypedef*)dev_struct;
//		cJSON_AddNumberToObject(json, TYPE_STR, dev_type);
//		sprintf(str_buf, "%d", dev_button->dev_basic.dev_id);
//		cJSON_AddStringToObject(json, DEVID_STR, str_buf);
// 		break;
	case ENUM_DEV_SWITCH_1:
	case ENUM_DEV_SWITCH_2:
	case ENUM_DEV_SWITCH_3:
	case ENUM_DEV_SWITCH_4:
	case ENUM_DEV_ON_OFF_PLUG:
	case ENUM_DEV_ON_OFF_86_PLUG:
	case ENUM_DEV_ON_OFF_CURTAIN:
	case ENUM_DEV_DIMMING_1:
	case ENUM_DEV_DIMMING_2:
	case ENUM_DEV_CURTAIN_1:
	case ENUM_DEV_CURTAIN_2:
	case ENUM_DEV_PUSH:
	case ENUM_DEV_BLINDS:
	case ENUM_DEV_COLOR_LED:
	case ENUM_DEV_CO:
	case ENUM_DEV_SMOKE:
	case ENUM_DEV_WATER:
	case ENUM_DEV_GAS:
	case ENUM_DEV_HUMAN:
	case ENUM_DEV_TEMP_HUMIDITY:
	case ENUM_DEV_DOOR_CONTACT:
	case ENUM_DEV_LUMINESCENCE:
		dev_sensor = (DevSensorTypedef*)dev_struct;
		cJSON_AddNumberToObject(json, TYPE_STR, dev_type);
		cJSON_AddNumberToObject(json, DEVID_STR, dev_sensor->dev_basic.dev_id);
		sprintf(str_buf, "%llx", dev_sensor->dev_basic.mac);
		cJSON_AddStringToObject(json, MAC_STR, (char *)str_buf);
		cJSON_AddNumberToObject(json, VALUE_STR, dev_sensor->u32_value[dev_sensor->ch]);
		cJSON_AddNumberToObject(json, CH_STR, dev_sensor->ch);
		break;
  	case ENUM_DEV_EVENT:
		err = ENUM_ERROR_INVALID_DEV;
		break;
  	case ENUM_DEV_TRIGGER:
  		dev_trigger = (DevTriggerTypedef*)dev_struct;

  		cJSON_AddNumberToObject(json, TYPE_STR, dev_type);
  		cJSON_AddNumberToObject(json, DEVID_STR, dev_trigger->dev_basic.dev_id);

  		cJSON_AddNumberToObject(json, STA_STR, dev_trigger->status);

		cj_condition = cJSON_Parse(dev_trigger->condition);
		cj_action = cJSON_Parse(dev_trigger->action);

		if(cj_condition)
			cJSON_AddItemToObject(json, IF_STR, cj_condition);
		if(cj_action)
			cJSON_AddItemToObject(json, THEN_STR, cj_action);

		break;
  	default:
		err = ENUM_ERROR_INVALID_DEV;
  		break;
  }
	return err;
}

typedef struct error_message_t{
	int err;
	const char *message;
}ErrotMessage;
static ErrotMessage error_message[]={
	{0, "success"},
	{ENUM_ERROR_INVALID_CMD,"invalid cmd"},
	{ENUM_ERROR_INVALID_DEV,"invalid dev"},
	{ENUM_ERROR_DEV_EXIST,"dev exist"},
	{ENUM_ERROR_MALLOC_FALSE, "malloc"},
	{DCI_ERROR_DEV_INSERT, "insert"},
	{DCI_ERROR_DEV_DELETE, "delete"},
	{DCI_ERROR_DEV_UPDATE, "update"},
	{DCI_ERROR_DEV_SELECT, "select"},
	{DCI_ERROR_DEV_EXIST, "is exist"},
	{DCI_ERROR_DEV_NOT_EXIST, "not to exist"}
};
#define ERROR_MESSAGE_NUM (sizeof(error_message)/sizeof(error_message[0]))
/**
 * @brief return error message
 * @param derrno
 * @return return error message
 * @warning
 */
const char* getErrorMessage(int err){
	unsigned int i;
	for(i=0; i < ERROR_MESSAGE_NUM; i++){
		if(error_message[i].err == err)
			return error_message[i].message;
	}
	return "unknow error";
}

/*------------------------------ common funcation ----------------------------*/
static char g_sql_cmd[1024];

/**
 * @brief sql delete
 * @param dev_inf:device data
 * @return success SQLITE_OK,false SQLITE_ERROR
 * @warning
 */
int DeviceModel::deleteDev(uint32_t id, const char *tb){
	char *errmsg = 0;
	int ret = 0;

	sprintf( g_sql_cmd, "DELETE FROM %s WHERE dev_id=%d;", tb, id );

  //select data
  ret = sqlite3_exec(sqlite3_db, g_sql_cmd, NULL, (void *)"delete", &errmsg);
  if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
  }
  sqlite3_free(errmsg);
  return (-1*ret);
}
/**
 * @brief sql delete
 * @param dev_inf:device data
 * @return success SQLITE_OK,false SQLITE_ERROR
 * @warning
 */
int DeviceModel::deleteDev(uint64_t mac, const char *tb){
	char *errmsg = 0;
	int ret = 0;

	sprintf( g_sql_cmd, "DELETE FROM %s WHERE mac=%llu;", tb, mac );

  //select data
  ret = sqlite3_exec(sqlite3_db, g_sql_cmd, NULL, (void *)"delete", &errmsg);
  if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
  }
  sqlite3_free(errmsg);
  return (-1*ret);
}
/**
 * @brief isExistDev
 * @param
 * @return success SQLITE_OK,false SQLITE_ERROR
 * @warning
 */
bool DeviceModel::isExistDev(uint32_t id, const char *tb){
	sprintf(g_sql_cmd,"SELECT COUNT(*) FROM %s WHERE dev_id=%d", tb, id);
	sqlite3_stmt *pstmt;
	sqlite3_prepare(sqlite3_db, g_sql_cmd, strlen(g_sql_cmd), &pstmt, NULL);
	sqlite3_step(pstmt);
	int count=sqlite3_column_int(pstmt,0);
	sqlite3_finalize(pstmt);

	if(count > 0)
		return true;
	return false;
}
/**
 * @brief isExistDev
 * @param
 * @return success SQLITE_OK,false SQLITE_ERROR
 * @warning
 */
bool DeviceModel::isExistDev(uint64_t mac, const char *tb){
	sprintf(g_sql_cmd,"SELECT COUNT(*) FROM %s WHERE mac=%llu", tb, mac);
	sqlite3_stmt *pstmt;
	sqlite3_prepare(sqlite3_db, g_sql_cmd, strlen(g_sql_cmd), &pstmt, NULL);
	sqlite3_step(pstmt);
	int count=sqlite3_column_int(pstmt,0);
	sqlite3_finalize(pstmt);

	if(count > 0)
		return true;
	return false;
//	sprintf(g_sql_cmd,"SELECT COUNT(*) FROM %s WHERE mac=%llu", tb, mac);
//	sqlite3_stmt *pstmt;
//	sqlite3_prepare(sqlite3_db, g_sql_cmd, strlen(g_sql_cmd), &pstmt, NULL);
//	sqlite3_step(pstmt);
//	int count=sqlite3_column_int(pstmt,0);
//	sqlite3_finalize(pstmt);
//
//	if(count > 0)
//		return true;
//	return false;
}
/*------------------------------basic dev-------------------------------------*/
static const char *db_name = "/usr/bin/iot_dc/iot_dc.db";
//static const char *db_name = "/iot_dc.db";
#define DEV_BASIC_TB "device_basic_tb"
#define DEV_BASIC_TB_COL_NAME_FOR_CREATE "(id INTEGER PRIMARY KEY AUTOINCREMENT,"\
"mac INTEGER,"\
"dev_type INTEGER,"\
"dev_id INTEGER,"\
"short_addr INTEGER,"\
"endpoint INTEGER)";

static int cb_select_basic(void *data, int argc, char **argv, char **azColName){
	int i;
	DevBasicTypedef *dev = (DevBasicTypedef *)data;
	char *endptr;

	for(i=0; i<argc; i++){
	  printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

	  if(strcmp(azColName[i], "dev_type") == 0)
		  dev->dev_type = atoi(argv[i]);
	  else if(strcmp(azColName[i], "dev_id") == 0)
		  dev->dev_id = atoi(argv[i]);
	  else if(strcmp(azColName[i], "mac") == 0)
		  dev->mac = strtoull(argv[i], &endptr, 10);//sscanf(argv[i], "%llu", &dev->mac);
	  else if(strcmp(azColName[i], "short_addr") == 0)
		  dev->short_addr = atoi(argv[i]);
	  else if(strcmp(azColName[i], "endpoint") == 0)
		  dev->endpoint = atoi(argv[i]);

	}
	printf("\n");
	return 0;
}

/**
 * @brief sql insert
 * @param dev_inf:device data
 * @return success SQLITE_OK,false SQLITE_ERROR
 * @warning
 */
int DeviceModel::insterBasicDev(DevBasicTypedef *dev_basic){
	char *errmsg = 0;
	int ret = 0;

	sprintf(g_sql_cmd, "INSERT INTO %s values(NULL, %llu, %d, %d, %d, %d);",
			DEV_BASIC_TB,
			dev_basic->mac,
			dev_basic->dev_type,
			dev_basic->dev_id,
			dev_basic->short_addr,
			dev_basic->endpoint);

	ret = sqlite3_exec(sqlite3_db, g_sql_cmd, NULL, NULL, &errmsg);
	if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
	}
	sqlite3_free(errmsg);
	return (-1*ret);
}




int DeviceModel::updateBasicDev(DevBasicTypedef *dev_basic){
//	char sql[128];
//	char *errmsg = 0;
//	int ret = 0;
//
//	sprintf( sql, "UPDATE %s SET dev_id=%d, short_addr=%d, endpoint=%d WHERE mac=%llu;",
//			DEV_BASIC_TB,
//			dev_basic->dev_id,
//			dev_basic->short_addr,
//			dev_basic->endpoint,
//			dev_basic->mac);
//
//  //select data
//  ret = sqlite3_exec(sqlite3_db, sql, NULL, NULL, &errmsg);
//  if(ret != SQLITE_OK){
//	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
//  }
//  sqlite3_free(errmsg);
//  return (-1*ret);
	return -1;
}

int DeviceModel::selectBasicDev(uint32_t id, DevBasicTypedef *dev_basic){
	char *errmsg = 0;
	int ret = 0;

	sprintf( g_sql_cmd, "SELECT * FROM %s WHERE dev_id=%d", DEV_BASIC_TB, id);
  //select data
  ret = sqlite3_exec(sqlite3_db, g_sql_cmd, cb_select_basic, (void *)dev_basic, &errmsg);
  if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
  }
  sqlite3_free(errmsg);
  return (-1*ret);
}
int DeviceModel::selectBasicDev(uint64_t mac, DevBasicTypedef *dev_basic){
	char *errmsg = 0;
	int ret = 0;

	sprintf( g_sql_cmd, "SELECT * FROM %s WHERE mac=%llu", DEV_BASIC_TB, mac);
	//select data
	ret = sqlite3_exec(sqlite3_db, g_sql_cmd, cb_select_basic, (void *)dev_basic, &errmsg);
	if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
	}
	sqlite3_free(errmsg);
	return (-1*ret);
}
int DeviceModel::selectBasicDev(DevBasicTypedef *dev_basic){
	char *errmsg = 0;
	int ret = 0;

//	sprintf( sql, "SELECT * FROM %s WHERE mac=%llu AND endpoint=%d", DEV_BASIC_TB, dev_basic->mac, dev_basic->endpoint);
	sprintf( g_sql_cmd, "SELECT * FROM %s WHERE mac=%llu", DEV_BASIC_TB, dev_basic->mac);
  //select data
  ret = sqlite3_exec(sqlite3_db, g_sql_cmd, cb_select_basic, (void *)dev_basic, &errmsg);
  if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
  }
  sqlite3_free(errmsg);
  return (-1*ret);
}


/*---------------------------------- switch ------------------------------------------*/
#define DEV_SWITCH_TB "device_switch_tb"
#define DEV_SWITCH_TB_COL_NAME_FOR_CREATE "(dev_id INTEGER PRIMARY KEY,"\
"dev_type INTEGER,"\
"mac INTEGER,"\
"short_addr INTEGER,"\
"group_addr INTEGER,"\
"endpoint INTEGER,"\
"value_int_1 INTEGER,value_int_2 INTEGER,value_int_3 INTEGER,value_int_4 INTEGER,"\
"pre_value_int INTEGER)"

//static int cb_select_switch(void *data, int argc, char **argv, char **azColName){
//	int i;
//	DevSwitchTypedef *dev = (DevSwitchTypedef *)data;
//	char *endptr;
//
//	for(i=0; i<argc; i++){
//	  printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//	  if(strcmp(azColName[i], "mac") == 0)
//		  dev->dev_basic.mac = strtoull(argv[i], &endptr, 10);//sscanf(argv[i], "%llu", &dev->dev_basic.mac);
//	  else if(strcmp(azColName[i], "short_addr") == 0)
//		  dev->dev_basic.short_addr = atoi(argv[i]);
//	  else if(strcmp(azColName[i], "group_addr") == 0)
//		  dev->dev_basic.group_addr = atoi(argv[i]);
//	  else if(strcmp(azColName[i], "endpoint") == 0)
//		  dev->dev_basic.endpoint = atoi(argv[i]);
//	  else if(strcmp(azColName[i], "value_int_1") == 0)
//		  dev->u32_value[1] = atoi(argv[i]);
//	  else if(strcmp(azColName[i], "value_int_2") == 0)
//	  		 dev->u32_value[2] = atoi(argv[i]);
//	  else if(strcmp(azColName[i], "value_int_3") == 0)
//	  		  dev->u32_value[3] = atoi(argv[i]);
//	  else if(strcmp(azColName[i], "value_int_4") == 0)
//		  dev->u32_value[4] = atoi(argv[i]);
////	  else if(strcmp(azColName[i], "pre_value_int") == 0)
////		  dev->pre_value_int = atoi(argv[i]);
//
//	}
//	printf("\n");
//	return 0;
//}
//
//int DeviceModel::insterSwitchDev(DevSwitchTypedef *dev){
//#define DEV_SWITCH_TB_COL_NAME "(dev_id,"\
//"dev_type,"\
//"mac,"\
//"short_addr,"\
//"group_addr,"\
//"endpoint,"\
//"value_int_1,value_int_2,value_int_3,value_int_4,"\
//"pre_value_int)"
//
//	char *errmsg = 0;
//	int ret = 0;
//
//	sprintf(g_sql_cmd, "INSERT INTO %s %s values(%d, %d, %llu, %d, %d, %d, %d, %d, %d, %d, %d);",
//			DEV_SWITCH_TB,
//			DEV_SWITCH_TB_COL_NAME,
//			dev->dev_basic.dev_id,
//			dev->dev_basic.dev_type,
//			dev->dev_basic.mac,
//			dev->dev_basic.short_addr,
//			dev->dev_basic.group_addr,
//			dev->dev_basic.endpoint,
//			dev->u32_value[1],dev->u32_value[2],dev->u32_value[3],dev->u32_value[4]
////			dev->pre_value_int
//			);
//
//	ret = sqlite3_exec(sqlite3_db, g_sql_cmd, NULL, (void *)"insert", &errmsg);
//	if(ret != SQLITE_OK){
//	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
//	}
//	sqlite3_free(errmsg);
//	return (-1*ret);
//}
//
//
//int DeviceModel::deleteSwitchDev(DevSwitchTypedef *dev){
//	char *errmsg = 0;
//	int ret = 0;
//	if(dev->dev_basic.mac != 0)
//		sprintf( g_sql_cmd, "DELETE FROM %s WHERE mac=%llu;", DEV_SWITCH_TB, dev->dev_basic.mac );
//	else
//		sprintf( g_sql_cmd, "DELETE FROM %s WHERE dev_id=%d;", DEV_SWITCH_TB, dev->dev_basic.dev_id );
//
//  ret = sqlite3_exec(sqlite3_db, g_sql_cmd, NULL, (void *)"delete", &errmsg);
//  if(ret != SQLITE_OK){
//	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
//  }
//  sqlite3_free(errmsg);
//  return (-1*ret);
//}
//
//
//int DeviceModel::updateSwitchDev(DevSwitchTypedef *dev){
//	char *errmsg = 0;
//	int ret = 0;
//
//	if(dev->dev_basic.mac != 0)
//		sprintf( g_sql_cmd, "UPDATE %s SET value_int_%d=%d, WHERE mac=%llu;",
//				DEV_SWITCH_TB,
//				dev->ch,
//				dev->u32_value[dev->ch],
////				dev->pre_value_int,
//				dev->dev_basic.mac );
//	else
//		sprintf( g_sql_cmd, "UPDATE %s SET value_int_%d=%d, WHERE dev_id=%d;",
//				DEV_SWITCH_TB,
//				dev->ch,
//				dev->u32_value[dev->ch],
////				dev->pre_value_int,
//				dev->dev_basic.dev_id );
//
//  //select data
//  ret = sqlite3_exec(sqlite3_db, g_sql_cmd, NULL, (void *)"update", &errmsg);
//  if(ret != SQLITE_OK){
//	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
//  }
//  sqlite3_free(errmsg);
//  return (-1*ret);
//}
//
//
//int DeviceModel::selectSwitchDev(DevSwitchTypedef *dev){
//	char *errmsg = 0;
//	int ret = 0;
//
//	if(dev->dev_basic.dev_id != 0)
//		sprintf( g_sql_cmd, "SELECT * FROM %s WHERE dev_id=%d", DEV_SWITCH_TB, dev->dev_basic.dev_id );
//	else
//		sprintf( g_sql_cmd, "SELECT * FROM %s WHERE mac=%llu", DEV_SWITCH_TB, dev->dev_basic.mac );
//
//  //select data
//  ret = sqlite3_exec(sqlite3_db, g_sql_cmd, cb_select_switch, (void *)dev, &errmsg);
//  if(ret != SQLITE_OK){
//	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
//  }
//  sqlite3_free(errmsg);
//  return (-1*ret);
//}
//
//
//int DeviceModel::isExistSwitchDev(DevSwitchTypedef *dev){
//	return 0;
//}


/*---------------------------------- sensor ------------------------------------------*/
#define DEV_SENSOR_TB "device_sensor_tb"
#define DEV_SENSOR_TB_COL_NAME_FOR_CREATE "(dev_id INTEGER PRIMARY KEY,"\
"dev_type INTEGER,"\
"mac INTEGER,"\
"value_int_1 INTEGER,value_int_2 INTEGER,value_int_3 INTEGER,value_int_4 INTEGER"\
")"

static int cb_select_sensor(void *data, int argc, char **argv, char **azColName){
	int i;
	DevSensorTypedef *dev = (DevSensorTypedef *)data;
	char *endptr;

	for(i=0; i<argc; i++){
	  printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	  if(strcmp(azColName[i], "mac") == 0)
		  dev->dev_basic.mac = strtoull(argv[i], &endptr, 10);
	  else if(strcmp(azColName[i], "value_int_1") == 0)
		  dev->u32_value[1] = atoi(argv[i]);
	  else if(strcmp(azColName[i], "value_int_2") == 0)
	  		 dev->u32_value[2] = atoi(argv[i]);
	  else if(strcmp(azColName[i], "value_int_3") == 0)
	  		  dev->u32_value[3] = atoi(argv[i]);
	  else if(strcmp(azColName[i], "value_int_4") == 0)
		  dev->u32_value[4] = atoi(argv[i]);

	}
	printf("\n");
	return 0;
}

int DeviceModel::insterSensorDev(DevSensorTypedef *dev){
#define DEV_SENSOR_TB_COL_NAME "(dev_id,"\
"dev_type,"\
"mac,"\
"value_int_1,value_int_2,value_int_3,value_int_4"\
")"

	char *errmsg = 0;
	int ret = 0;

	sprintf(g_sql_cmd, "INSERT INTO %s %s values(%d, %d, %llu, %d, %d, %d, %d);",
			DEV_SENSOR_TB,
			DEV_SENSOR_TB_COL_NAME,
			dev->dev_basic.dev_id,
			dev->dev_basic.dev_type,
			dev->dev_basic.mac,
			dev->u32_value[1],dev->u32_value[2],dev->u32_value[3],dev->u32_value[4]);

	ret = sqlite3_exec(sqlite3_db, g_sql_cmd, NULL, (void *)"insert", &errmsg);
	if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
	}
	sqlite3_free(errmsg);
	return (-1*ret);
}


int DeviceModel::deleteSensorDev(DevSensorTypedef *dev){
  return deleteDev(dev->dev_basic.dev_id, DEV_SENSOR_TB);
}


int DeviceModel::updateSensorDev(DevSensorTypedef *dev){
	char *errmsg = 0;
	int ret = 0;

	if(dev->dev_basic.dev_id != 0)
		sprintf( g_sql_cmd, "UPDATE %s SET value_int_%d=%d WHERE dev_id=%d;",
						DEV_SENSOR_TB,
						dev->ch,
						dev->u32_value[dev->ch],
						dev->dev_basic.dev_id );
	else
		sprintf( g_sql_cmd, "UPDATE %s SET value_int_%d=%d WHERE mac=%llu;",
				DEV_SENSOR_TB,
				dev->ch,
				dev->u32_value[dev->ch],
				dev->dev_basic.mac );


  //select data
  ret = sqlite3_exec(sqlite3_db, g_sql_cmd, NULL, (void *)"update", &errmsg);
  if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
  }
  sqlite3_free(errmsg);
  return (-1*ret);
}


int DeviceModel::selectSensorDev(DevSensorTypedef *dev){
	char *errmsg = 0;
	int ret = 0;

	if(dev->dev_basic.dev_id != 0)
		sprintf( g_sql_cmd, "SELECT * FROM %s WHERE dev_id=%d", DEV_SENSOR_TB, dev->dev_basic.dev_id );
	else
		sprintf( g_sql_cmd, "SELECT * FROM %s WHERE mac=%llu", DEV_SENSOR_TB, dev->dev_basic.mac );

  //select data
  ret = sqlite3_exec(sqlite3_db, g_sql_cmd, cb_select_sensor, (void *)dev, &errmsg);
  if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
  }
  sqlite3_free(errmsg);
  return (-1*ret);
}

/*---------------------------------- trigger ------------------------------------------*/
#define DEV_TRIGGER_TB "device_trigger_tb"
#define DEV_TRIGGER_TB_COL_NAME_FOR_CREATE "("\
"dev_id INTEGER PRIMARY KEY,"\
"dev_type INTEGER,"\
"status INTEGER,"\
"condition TEXT,"\
"action TEXT,"\
"is_triggered INTEGER)"

static int cb_select_trigger(void *data, int argc, char **argv, char **azColName){
	int i;
	DevTriggerTypedef *dev = (DevTriggerTypedef *)data;
	for(i=0; i<argc; i++){
	  printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	  fflush(stdout);
	  if(strcmp(azColName[i], "status") == 0)
		  dev->status= atoi(argv[i]);
	  else if(strcmp(azColName[i], "condition") == 0)
		  strcpy(dev->condition, argv[i]);
	  else if(strcmp(azColName[i], "action") == 0)
		  strcpy(dev->action, argv[i]);
	  else if(strcmp(azColName[i], "is_triggered") == 0)
		  dev->is_triggered= atoi(argv[i]);
	}
	printf("\n");
	return 0;
}

int DeviceModel::insterTriggerDev(DevTriggerTypedef *dev){
#define DEV_TRIGGER_TB_COL_NAME "("\
"dev_id,"\
"dev_type,"\
"status,"\
"condition,"\
"action,"\
"is_triggered)"
	char *errmsg = 0;
	int ret = 0;

	sprintf(g_sql_cmd, "INSERT INTO %s %s values(%d, %d, %d, '%s', '%s', %d);",
			DEV_TRIGGER_TB,
			DEV_TRIGGER_TB_COL_NAME,
			dev->dev_basic.dev_id,
			dev->dev_basic.dev_type,
			dev->status,
			dev->condition,
			dev->action,
			dev->is_triggered);

	ret = sqlite3_exec(sqlite3_db, g_sql_cmd, NULL, (void *)"insert", &errmsg);
	if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
	}
	sqlite3_free(errmsg);
	return (-1*ret);
}


int DeviceModel::deleteTriggerDev(DevTriggerTypedef *dev){
	char *errmsg = 0;
	int ret = 0;

	sprintf( g_sql_cmd, "DELETE FROM %s WHERE dev_id=%d;", DEV_TRIGGER_TB, dev->dev_basic.dev_id );

  ret = sqlite3_exec(sqlite3_db, g_sql_cmd, NULL, (void *)"delete", &errmsg);
  if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
  }
  sqlite3_free(errmsg);
  return (-1*ret);
}


int DeviceModel::updateTriggerDev(DevTriggerTypedef *dev){
	char *errmsg = 0;
	int ret = 0;
	char update_format[512];

	memset(update_format, 0,sizeof(update_format));
	memset(g_sql_cmd, 0,sizeof(g_sql_cmd));


	if(dev->status != -1){
		sprintf(update_format, "UPDATE %s SET status=%d WHERE dev_id=%d;",
				DEV_TRIGGER_TB,
				dev->status,
				dev->dev_basic.dev_id);
		strcat(g_sql_cmd, update_format);
	}
	if(strlen(dev->condition) > 0){
		sprintf(update_format, "UPDATE %s SET condition='%s' WHERE dev_id=%d;",
				DEV_TRIGGER_TB,
				dev->condition,
				dev->dev_basic.dev_id);
		strcat(g_sql_cmd, update_format);
	}
	if(strlen(dev->action) > 0){
		sprintf(update_format, "UPDATE %s SET action='%s' WHERE dev_id=%d;",
				DEV_TRIGGER_TB,
				dev->action,
				dev->dev_basic.dev_id);
		strcat(g_sql_cmd, update_format);
	}

  //select data
  ret = sqlite3_exec(sqlite3_db, g_sql_cmd, NULL, (void *)"update", &errmsg);
  if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
  }
  sqlite3_free(errmsg);
  return (-1*ret);
}

int DeviceModel::updateTriggerDev(int dev_id, int is_triggered){
	char *errmsg = 0;
	int ret = 0;

	memset(g_sql_cmd, 0,sizeof(g_sql_cmd));

	sprintf(g_sql_cmd, "UPDATE %s SET is_triggered=%d WHERE dev_id=%d;",
			DEV_TRIGGER_TB,
			is_triggered,
			dev_id);

  //select data
  ret = sqlite3_exec(sqlite3_db, g_sql_cmd, NULL, (void *)"update", &errmsg);
  if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
  }
  sqlite3_free(errmsg);
  return (-1*ret);
}

int DeviceModel::selectTriggerDev(DevTriggerTypedef *dev){
	char *errmsg = 0;
	int ret = 0;

	sprintf( g_sql_cmd, "SELECT * FROM %s WHERE dev_id=%d", DEV_TRIGGER_TB, dev->dev_basic.dev_id );

  //select data
  ret = sqlite3_exec(sqlite3_db, g_sql_cmd, cb_select_trigger, (void *)dev, &errmsg);
  if(ret != SQLITE_OK){
	  fprintf(stderr,"query SQL error: %d %s\n",ret, errmsg);
  }
  sqlite3_free(errmsg);
  return (-1*ret);
}


int DeviceModel::isExistTriggerDev(DevTriggerTypedef *dev){

	return 0;
}

/*---------------------------------------------------------------------------*/
/**
 * @brief sql init
 * @param name database name
 * @return success SQLITE_OK,false SQLITE_ERROR
 * @warning
 */
int DeviceModel::init(void) {

	const char *sql_create_basic_tb="CREATE TABLE "
			DEV_BASIC_TB
			DEV_BASIC_TB_COL_NAME_FOR_CREATE;
	const char *sql_create_switch_tb = "CREATE TABLE "
			DEV_SWITCH_TB
			DEV_SWITCH_TB_COL_NAME_FOR_CREATE;
	const char *sql_create_sensor_tb = "CREATE TABLE "
			DEV_SENSOR_TB
			DEV_SENSOR_TB_COL_NAME_FOR_CREATE;
	const char *sql_create_trigger_tb = "CREATE TABLE "
			DEV_TRIGGER_TB
			DEV_TRIGGER_TB_COL_NAME_FOR_CREATE;
	const char *sql_create_tb_cmd[] = {sql_create_basic_tb,
			sql_create_switch_tb,
			sql_create_sensor_tb,
			sql_create_trigger_tb};
	char *errmsg = 0;
	int ret = 0;
	uint16_t i;

	ret = sqlite3_open(db_name, &sqlite3_db);
	if(ret != SQLITE_OK){
	  fprintf(stderr,"Cannot open db: %s\n",sqlite3_errmsg(sqlite3_db));
	  return (-1*ret);
	}

	for(i=0; i < sizeof(sql_create_tb_cmd) / sizeof(sql_create_tb_cmd[0]); i++){
		ret = sqlite3_exec( sqlite3_db, sql_create_tb_cmd[i], NULL, NULL, &errmsg );
		if(ret != SQLITE_OK){
		  fprintf(stderr,"create table fail: %s\n",errmsg);
		}
		sqlite3_free(errmsg);
	}

	return (-1*ret);

}

int DeviceModel::uninit(void){
	return sqlite3_close(sqlite3_db);
}
