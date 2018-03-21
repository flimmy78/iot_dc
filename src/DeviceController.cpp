/*
 * DeviceController.cpp
 *
 *  Created on: 2017-6-7
 *      Author: z
 */
#include <stdio.h>
#include "dc_common.h"
#include "DeviceController.h"
#include "newDb.h"
#include "ZigbeeConstant.h"

#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG(format,...) do{printf("File: "__FILE__", Line: %05d: "format"", __LINE__, ##__VA_ARGS__);fflush(stdout);}while(0)
#else
#define DEBUG(format,...)
#endif

/*
 * zigbee device type to my type
 *
 *
 * */
int zbTypeToDevType(int zb_type){
	int ret = 0;
	switch(zb_type){
	case SIMPLE_DESCR_OCCUPANCY_SENSOR:
		ret = ENUM_DEV_DOOR_CONTACT;
		break;
	case 0x106:
		ret = ENUM_DEV_TEMP_HUMIDITY;
		break;
	case SIMPLE_DESCR_LAMP_DIMM:
		ret = ENUM_DEV_SWITCH_3;
		break;
	default:
		ret=zb_type;
	}
	return ret;
}

/*
*描述:返回响应消息
*参数:errno 错误编号errstr 错误信息
*返回值:消息字符串
*备注:返回字符串需要free
*/
char *getRespStr(int errno)
{
		char *str = (char *)malloc(128);

		if(str)
			sprintf(str,"{\"%s\":%d,"
						"\"%s\":\"%s\"}",
			CODE_STR,errno,
			MSG_STR,getErrorMessage(errno));
		return (str);

}

/*
*描述:返回安防设备响应消息
*参数:pJSON 设备信息
*返回值:消息字符串
*备注:返回字符串需要free
*/
char *getRespStrWithId(int id, int dev_type)
{
    cJSON *cj_resp = cJSON_CreateObject();
    if(NULL == cj_resp){
        return NULL;
    }
	cJSON_AddStringToObject(cj_resp, ONENETID_STR, (char *)g_systemcfg.onenet_devid);
//	cJSON_AddNumberToObject(cj_resp, ONENETID_STR, atoi(g_systemcfg.onenet_devid));
	cJSON_AddNumberToObject(cj_resp, DEVID_STR, id);
	cJSON_AddNumberToObject(cj_resp, TYPE_STR, dev_type);
	char * p = cJSON_PrintUnformatted(cj_resp);
	cJSON_Delete(cj_resp);
    if(NULL == p){
   		DEBUG("Error: null string\r\n");
		return NULL;
    }
    return p;
}

/*
*描述:返回响应消息
*参数:json_str 要返回的json数据
*返回值:消息字符串
*备注:返回字符串需要free
*/
char *getRespStrWhithData(DevBasicTypedef *dev_basic, void *dev_struct, int errno)
{
	char * str = NULL;
	cJSON *cj_resp = NULL,
			*cj_value=NULL;

	if(dev_basic == NULL || dev_struct == NULL)
			return NULL;

	cj_resp = cJSON_CreateObject();
	cj_value = cJSON_CreateObject();
	if(cj_resp == NULL || cj_value == NULL){
		if(cj_resp)
			cJSON_Delete(cj_resp);
		if(cj_value)
			cJSON_Delete(cj_value);
		return NULL;
	}

	cJSON_AddNumberToObject(cj_resp, CODE_STR, errno);
	cJSON_AddStringToObject(cj_resp, MSG_STR, getErrorMessage(errno));
	convertStruct2Json(cj_value, dev_basic->dev_type, dev_struct);
	cJSON_AddItemToObject(cj_resp, DATA_STR, cj_value);
	str = cJSON_PrintUnformatted(cj_resp);
	if(cj_resp)
		cJSON_Delete(cj_resp);
	return str;
}
/*
*描述:返回响应消息
*参数:json_str 要返回的json数据
*返回值:消息字符串
*备注:返回字符串需要free
*/
char *getRespStrWhithValue(int value, int errno)
{
	char * str = NULL;
	cJSON *cj_resp = NULL,
			*cj_value=NULL;

	cj_resp = cJSON_CreateObject();
	cj_value = cJSON_CreateObject();
	if(cj_resp == NULL || cj_value == NULL){
		if(cj_resp)
			cJSON_Delete(cj_resp);
		if(cj_value)
			cJSON_Delete(cj_value);
		return NULL;
	}

	cJSON_AddNumberToObject(cj_resp, CODE_STR, errno);
	cJSON_AddStringToObject(cj_resp, MSG_STR, getErrorMessage(errno));
	cJSON_AddNumberToObject(cj_value, VALUE_STR, value);
	cJSON_AddItemToObject(cj_resp, DATA_STR, cj_value);
	str = cJSON_PrintUnformatted(cj_resp);
	if(cj_resp)
		cJSON_Delete(cj_resp);
	return str;
}
/************************ member function **********************************/
DeviceController::DeviceController(DeviceModel *dm, ZigbeeController *zbc, CHttpClient *chttp) {
	// TODO Auto-generated constructor stub
	this->device_model = dm;
	this->zb_ctrl = zbc;
	this->chttp = chttp;
	this->source_id = "607467";
}

DeviceController::~DeviceController() {
	// TODO Auto-generated destructor stub
}


/*
*描述:注册主机
*参数:
*返回值:true: 注册成功
*返回值:false: 注册失败
*备注: 服务器返回消息格式{code:200,message:xxxx,data:{}}
*/
bool DeviceController::doRegisterDevice(void){
	char register_req[128];
	int err=0;
	cJSON * cj_response=NULL;
	cJSON * cj_data=NULL;
	cJSON * cj_sub=NULL;

	printf("doRegisterDevice start\r\n");
	// get mac
//	if(getIfMacStr(mac_str, "eno1"))
//		printf("mac: %s\r\n", mac_str);
//	else if(getIfMacStr(mac_str, "eth0"))
//		printf("mac: %s\r\n", mac_str);

//	strcpy(mac_str, "000c43e17629");
//	sprintf(register_req,"mac=%s", mac_str);
	sprintf(register_req,"mac=%s", g_systemcfg.mac);
	printf("%s\r\n", register_req);
	// send request and reciver response
	std::string strResponse;
	this->chttp->Post(URL_REGISTER, register_req, strResponse);
	std::cout << strResponse << std::endl;

	// parse data
	cj_response = cJSON_Parse(strResponse.c_str());
	if(cj_response == NULL){
		return false;
	}
	/* data */
	cj_data = cJSON_GetObjectItem(cj_response,"data");
	if(cj_data == NULL){
		return false;
	}
	/* onenet device id */
	cj_sub = cJSON_GetObjectItem(cj_data,"device_id");
	if(cj_sub){
		strcpy((char*)g_systemcfg.onenet_devid, cj_sub->valuestring);
	}else
		err= -1;
	/* onenet apikey */
	cj_sub = cJSON_GetObjectItem(cj_data,"api-key");
	if(cj_sub){
		strcpy((char*)g_systemcfg.onenet_apikey, cj_sub->valuestring);
	}else
		err= -1;
	/* yaokan_active_code*/
	cj_sub = cJSON_GetObjectItem(cj_data,"yaokan_active_code");
	if(cj_sub){
		strcpy((char*)g_systemcfg.ir_active_code, cj_sub->valuestring);
	}

	if(cj_response)
		cJSON_Delete(cj_response);
	if(err == -1){
//		g_systemcfg.is_register =true;
		return false;
	}else {
//		g_systemcfg.is_register =0;
		return true;
	}
}

/*
*描述:判断valjson 中的数据是否符合ifjson 中的条件
*参数:valjson 待判断的数值 ifjson 条件
*返回值:1 符合 0 不符合
*备注:
*/
int DeviceController::json_value_cmp(DevSensorTypedef *dev_struct,cJSON *ifjson)
{
		int value;
//		int prevalue;
		cJSON *pSub=NULL,*ppSub=NULL;

		value = dev_struct->u32_value[dev_struct->ch];
//		prevalue = dev_struct->pre_value_int;

		pSub = cJSON_GetObjectItem(ifjson, COMPARE_STR);
		if(NULL == pSub)
			return -1;

		ppSub = cJSON_GetObjectItem(ifjson, VALUE_STR);
		if(NULL == ppSub)
			return -1;

//		if(0 == strcmp("inout",pSub->valuestring)){
//			cJSON *pppSub = cJSON_GetObjectItem(ppSub, LOLMT_STR);
//			if(NULL == pppSub)
//			return -1;
//			int lolimt = pppSub->valueint;
//			pppSub = cJSON_GetObjectItem(ppSub, UPLMT_STR);
//			if(NULL == pppSub)
//			return -1;
//			int uplimt = pppSub->valueint;
//
//			/*首次进入闭区间*/
//			if(value >= lolimt && value <=uplimt)
//				if(prevalue <= lolimt && prevalue >=uplimt)
//					return 0;
//				/*首次离开闭区间*/
//			if(value <= lolimt || value >=uplimt)
//				if(prevalue >= lolimt && prevalue <=uplimt)
//					return 0;
//
//			return -1;
//		}	else if(0 == strcmp("in",pSub->valuestring)){
//			cJSON *pppSub = cJSON_GetObjectItem(ppSub, LOLMT_STR);
//			if(NULL == pppSub)
//			return -1;
//			int lolimt = pppSub->valueint;
//			pppSub = cJSON_GetObjectItem(ppSub, UPLMT_STR);
//			if(NULL == pppSub)
//			return -1;
//			int uplimt = pppSub->valueint;
//
//			/*首次进入闭区间*/
//			if(value >= lolimt && value <=uplimt)
//				if(prevalue <= lolimt && prevalue >=uplimt)
//					return 0;
//			return -1;
//		}else if(0 == strcmp("out",pSub->valuestring)){
//			cJSON *pppSub = cJSON_GetObjectItem(ppSub, LOLMT_STR);
//			if(NULL == pppSub)
//			return -1;
//			int lolimt = pppSub->valueint;
//			pppSub = cJSON_GetObjectItem(ppSub, UPLMT_STR);
//			if(NULL == pppSub)
//			return -1;
//			int uplimt = pppSub->valueint;
//
//				/*首次离开闭区间*/
//			if(value <= lolimt || value >=uplimt)
//				if(prevalue >= lolimt && prevalue <=uplimt)
//					return 0;
//
//			return -1;
//		}else if(0 == strcmp("change",pSub->valuestring)){
//			if(value != prevalue)
//				return 0;
//			else
//				return -1;
//		}else {

			int ifval = ppSub->valueint;
			printf("Compare %d %s %d?\r\n",value, pSub->valuestring, ifval);
			if(0 == strcmp(">",pSub->valuestring))
			{
				if(value > ifval)
					return 0;
				else
					return -1;
			}
			else if(0 == strcmp(">=",pSub->valuestring))
			{
				if(value >= ifval)
					return 0;
				else
					return -1;
			}
			else if(0 == strcmp("<",pSub->valuestring))
			{
				if(value < ifval)
					return 0;
				else
					return -1;
			}
			else if(0 == strcmp("<=",pSub->valuestring))
			{
				if(value <= ifval)
					return 0;
				else
					return -1;
			}else if(0 == strcmp("==",pSub->valuestring))
			{
				if(value == ifval)
					return 0;
				else
					return -1;
			}
//		}
	return -1;
}

/*
*描述:定时器处理函数
*参数:json 输入数据 resp 要返回的响应数据
*返回值:-1 fail 0 success
*备注:
*/
int DeviceController::json_time_cmp(struct tm *local_time, cJSON *valuejson)
{
	/*判断分钟*/
	cJSON *pSub = cJSON_GetObjectItem(valuejson, "MI");
	if(NULL == pSub){
		return -1;
	}
	if(local_time->tm_min != pSub->valueint){
		DEBUG("min %d != %d", local_time->tm_min, pSub->valueint);
		return -1;
	}
	/*判断分小时*/
	pSub = cJSON_GetObjectItem(valuejson, "H");
	if(NULL == pSub){
		return 0;
	}
	if(local_time->tm_hour != pSub->valueint){
		DEBUG("hour %d != %d", local_time->tm_hour, pSub->valueint);
		return -1;
	}
	/*判断时间的格式*/
	pSub = cJSON_GetObjectItem(valuejson, "W");
	if(NULL == pSub){
		/*判断日期*/
		pSub = cJSON_GetObjectItem(valuejson, "D");
		if(NULL == pSub){
			return 0;
		}
		if(local_time->tm_mday != pSub->valueint){
			DEBUG("day %d != %d", local_time->tm_mday, pSub->valueint);
			return -1;
		}
		/*判断分月份*/
		pSub = cJSON_GetObjectItem(valuejson, "MO");
		if(NULL == pSub){
			return 0;
		}
		if(local_time->tm_mon != pSub->valueint){
			DEBUG("month %d != %d", local_time->tm_mon, pSub->valueint);
			return -1;
		}
		/*判断年份*/
		pSub = cJSON_GetObjectItem(valuejson, "Y");
		if(NULL == pSub){
			return 0;
		}
		if(local_time->tm_year!= pSub->valueint){
			DEBUG("year %d != %d", local_time->tm_year, pSub->valueint);
			return -1;
		}
	}else{
		pSub = cJSON_GetObjectItem(valuejson, "W");
	  if(NULL == pSub){
		return -1;
	  }
		char wdaystr[2]={0,0};
		sprintf(wdaystr,"%d",local_time->tm_wday);
		if(NULL == strstr(pSub->valuestring, wdaystr)){
			DEBUG("week %s != %s", wdaystr, pSub->valuestring);
			return -1;
		}
	}
	return 0;
}



/*
*描述:定时器处理函数
*参数:json 输入数据 resp 要返回的响应数据
*返回值:-1 fail 0 success
*备注:
*/
int DeviceController::devTimerCmp(cJSON *json,char **resp)
{
	#define IS_BETWWEN_INT(value,max,min) (value >= min && value <= max)
	static const char *time_keyword[] = {"Y","MO","D","H","MI"};
	uint16_t i = 0, array_size = 0, result = 0;
	cJSON *value_json = NULL,
			*time_item_0 = NULL,
			*time_item_1 = NULL,
			*item_0_sub = NULL,
			*item_1_sub = NULL,
			*week_0_sub = NULL,
			*week_1_sub = NULL;
	struct tm *local_tm;
	time_t ttCurrentTime;
	int current_time[6];
	char time_str[32];
	char wdaystr[2]={0,0};

	ttCurrentTime = time(NULL);
	local_tm = localtime(&ttCurrentTime);
	printf("%s", asctime_r(local_tm, time_str));
	local_tm->tm_year += 1900;
	local_tm->tm_mon += 1;
	current_time[0] = local_tm->tm_year;
	current_time[1] = local_tm->tm_mon;
	current_time[2] = local_tm->tm_mday;
	current_time[3] = local_tm->tm_hour;
	current_time[4] = local_tm->tm_min;
	current_time[5] = local_tm->tm_wday;//星期
  // get time value json
	value_json = cJSON_GetObjectItem(json, VALUE_STR);
	if(NULL == value_json){
		result = -1;
		return result;
	}
	array_size = cJSON_GetArraySize(value_json);
	time_item_0 = cJSON_GetArrayItem(value_json,0);
	time_item_1 = cJSON_GetArrayItem(value_json,1);
	if(array_size > 0 && array_size < 2){
		result = json_time_cmp(local_tm, time_item_0);
	}else{
		for(i = 0;i < sizeof(time_keyword)/sizeof(time_keyword[0]); i++){
			item_0_sub = cJSON_GetObjectItem(time_item_0, time_keyword[i]);
			item_1_sub = cJSON_GetObjectItem(time_item_1, time_keyword[i]);
			if(item_0_sub == NULL || item_1_sub == NULL)
				continue;
			if(!IS_BETWWEN_INT(current_time[i],item_1_sub->valueint, item_0_sub->valueint)){
				DEBUG("time %d !<= %d !<= %d ", item_0_sub->valueint,  current_time[i], item_1_sub->valueint);
				result = -1*i;
				return result;
			}
		}
		week_0_sub = cJSON_GetObjectItem(time_item_0, "W");
		week_1_sub = cJSON_GetObjectItem(time_item_1, "W");
		if( week_0_sub && week_1_sub){
			sprintf(wdaystr,"%d",current_time[5]);
			if(NULL == strstr(week_0_sub->valuestring, wdaystr) || NULL == strstr(week_1_sub->valuestring, wdaystr)){
				DEBUG("week %s != %s != %s", wdaystr, week_0_sub->valuestring, week_1_sub->valuestring);
				result = -1*i;
				return result;
			}
		}
	}

	return result;
}


/*----------------------------------------------------------------*/


/********************************设备操作函数******************************************/

int DeviceController::devValueCmp(cJSON *json )
{
	cJSON *cj_item=NULL;
	int err=0;
	DevSensorTypedef dev_struct;

	cj_item = cJSON_GetObjectItem(json, DEVID_STR);
	if(cj_item){
		dev_struct.dev_basic.dev_id = cj_item->valueint;
	}else{
		err= -1;
		return err;
	}
	err = device_model->selectSensorDev(&dev_struct);

	dev_struct.dev_basic.dev_type = device_model->getDevType(json);
	cj_item = cJSON_GetObjectItem(json, CH_STR);
	if(cj_item)
		dev_struct.ch = cj_item->valueint;
	else
		dev_struct.ch = 1;

	if(err >= 0)
		err = json_value_cmp(&dev_struct , json);/*判断获得的的数据是否符合条件*/
	return err;

}

/*
*描述:检查触发器 对满足的条件的触发器执行相应动作
*参数:devtype 指定要检测的设备类型 有 timer valuedev
*返回值:
*备注:设备信息登记才触发器表格中，要用时从表格中取出
*/
void DeviceController::devTrigerScan(bool check_dev_value)
{
	#define ID_COL 0
	#define DEV_TYPE_COL 1
	#define STATUS_COL 2
	#define CONDITION_COL 3
	#define ACTION_COL 4
	#define IS_TRIGGERED_COL 5

	bool is_triggered;
	int action_err = -1;
	int id=0;
	int dev_type=0;
	uint8_t succ=0;
	uint16_t condition_arraysize,action_arraysize,i;
	cJSON *pSub = NULL, *if_json = NULL, *if_item_json = NULL, *action_json = NULL, *action_item_json = NULL;
	const unsigned char *condition_str=NULL;
	const unsigned char *action_str=NULL;
	char sql_query[128]={0};
	sprintf(sql_query,"select * from %s","device_trigger_tb");
	sqlite3_stmt *pstmt;
	sqlite3_prepare(device_model->sqlite3_db, sql_query, strlen(sql_query), &pstmt, NULL);
	while(1){
//		condition_str=NULL;
//		action_str=NULL;
		sqlite3_step(pstmt);
		id=sqlite3_column_int(pstmt, ID_COL);
		DEBUG("trigger id=%d\r\n", id);
		if(id <= 0){
			break;
		}
		dev_type = sqlite3_column_int(pstmt, DEV_TYPE_COL);
		/* status */
		int status = sqlite3_column_int(pstmt, STATUS_COL);
		if(status != 1){
			DEBUG("trigger sta=%d\r\n", status);
			continue;
		}

		/* condition */
		condition_str = sqlite3_column_text(pstmt, CONDITION_COL);
		if(condition_str){
//			DEBUG("condition_str:%s\r\n", condition_str);
			/*检查所有的条件是否成立*/
			if_json = cJSON_Parse((const char *)condition_str);
			condition_arraysize = cJSON_GetArraySize(if_json);/*获得条件的个数*/
			succ = 0;
			for(i=0;i<condition_arraysize;i++){
				if_item_json = cJSON_GetArrayItem(if_json, i);/*逐个获取条件内容*/
				pSub = cJSON_GetObjectItem(if_item_json, TYPE_STR);
				/*对定时器类型进行判断*/
				if(pSub->valueint == ENUM_DEV_TIME){
					if(0 == devTimerCmp(if_item_json,NULL))
						succ++;
					else {
						DEBUG("condition %d false\r\n", i);
						break;
					}
				}else if(check_dev_value){ /*其它数值类设备进行判断*/
					if(0 == devValueCmp(if_item_json))
						succ++;
					else {
						DEBUG("condition %d false\r\n", i);
						break;
					}
				}
			}
			cJSON_Delete(if_json);
		}

		/* action */
		// TODO is trigged?
		is_triggered = (bool)sqlite3_column_int(pstmt, IS_TRIGGERED_COL);
		action_str = sqlite3_column_text(pstmt, ACTION_COL);
		if(action_str && succ == condition_arraysize && condition_arraysize > 0 && (!is_triggered)){
			action_err = 0;
			action_json = cJSON_Parse((const char *)action_str);
			action_arraysize = cJSON_GetArraySize(action_json);/*获得条件的个数*/
			if(action_json != NULL && action_arraysize != 0){
				for(i=0; i < action_arraysize; i++){
					action_item_json = cJSON_GetArrayItem(action_json, i);/*逐个获取条件内容*/
					pSub = cJSON_GetObjectItem(action_item_json, TYPE_STR);
					if(pSub == NULL){
						action_err = -1;
						continue;
					}
					if(pSub->valueint == ENUM_DEV_BUTTON
						|| pSub->valueint == ENUM_DEV_EVENT
						|| pSub->valueint == ENUM_DEV_DELAY){			//判断类型，添加不同指令
						action_err |= this->devCmdHandle(ENUM_CMD_TRIG, action_item_json, NULL);
					}else{
						action_err |= this->devCmdHandle(ENUM_CMD_EDIT, action_item_json, NULL);
					}
				}
			}
			cJSON_Delete(action_json);
			this->device_model->updateTriggerDev(id, 1);
		}
		/* put message to server */
		if(0 == action_err){
			/*返回触发器的ID给服务器*/
			char *cmd_resp = getRespStrWithId(id, dev_type);
			if(cmd_resp){
				std::string strResponse;
				this->chttp->Post_json(POST_TRIGGER_MESSAGE_URL, cmd_resp, strResponse);
				std::cout << strResponse << std::endl;
				free(cmd_resp);
			}
		}else
			DEBUG("action false:%d\r\n", action_err);

		// TODO if condition is false and is_triggered is true change is_triggered to false
		if(succ != condition_arraysize && condition_arraysize > 0 && is_triggered){
			this->device_model->updateTriggerDev(id, 0);
		}
	}
	sqlite3_finalize(pstmt);


}
/*
*描述:门磁传感器处理函数
*参数:json 输入数据 resp 要返回的响应数据
*返回值:-1 fail 0 success
*备注:
*/
int DeviceController::devSensorProc(uint32_t cmd, DevSensorTypedef *dev, char **resp)
{
	int err=0;
	DevSensorTypedef dev_for_read;

	switch (cmd)
  {
  	case ENUM_CMD_ADD://新增设备
  		if(dev->dev_basic.dev_id == 0){
			err = DCI_ERROR_DEV_INSERT;
			break;
		}
		//检查设备是否已经存在
  		if(device_model->isExistDev(dev->dev_basic.dev_id, "device_sensor_tb")){
  			err = DCI_ERROR_DEV_EXIST;
  			break;
  		}

		err = device_model->insterSensorDev(dev);
		if(err != SQLITE_OK){
			DEBUG("Error: device insert\r\n");
			err = DCI_ERROR_DEV_INSERT;
			break;
		}
		//插入设备总表
		err = device_model->insterBasicDev(&dev->dev_basic);
		if(err != SQLITE_OK){
			DEBUG("Error: basic device insert\r\n");
			err = DCI_ERROR_DEV_INSERT;
			break;
		}
		//获得响应数据
		if(resp != NULL)
			*resp = getRespStr(err);
		err = ENUM_ERROR_NONE;
  		break;

  	case ENUM_CMD_DELETE:
		//删除设备数据
  		err = device_model->deleteDev(dev->dev_basic.dev_id, "device_sensor_tb");
		if(err < 0){
			err = DCI_ERROR_DEV_DELETE;
//			break;
		}
		//从设备总表中删除
		err = device_model->deleteDev(dev->dev_basic.dev_id, "device_basic_tb");
		if(err < 0){
			err = DCI_ERROR_DEV_DELETE;
			break;
		}
		//获得响应数据
		if(resp != NULL)
			*resp = getRespStr(err);
		err = ENUM_ERROR_NONE;
  		break;
  	case ENUM_CMD_GET:
  		if(!device_model->isExistDev(dev->dev_basic.dev_id, "device_sensor_tb")){
			err = DCI_ERROR_DEV_NOT_EXIST;
			break;
		}
		//读取设备数据
		dev_for_read.dev_basic.dev_type = dev->dev_basic.dev_type;
		dev_for_read.dev_basic.dev_id = dev->dev_basic.dev_id;
		device_model->selectSensorDev(&dev_for_read);
		if(err < 0){
			err = ENUM_ERROR_DEV_READ;
			break;
		}
		//获得响应数据
		if(resp != NULL)
			*resp = getRespStrWhithValue(dev_for_read.u32_value[dev->ch], err);
		if(*resp == NULL){
			err = ENUM_ERROR_MALLOC_FALSE;
			break;
		}
		err = ENUM_ERROR_NONE;
  		break;
  	default:
  		err = ENUM_ERROR_INVALID_CMD;
  		break;
  }
	if(resp != NULL && err != ENUM_ERROR_NONE)
		*resp = getRespStr(err);
	return err;
}

/*
*描述:指纹锁处理函数
*参数:cmd 命令类型
*参数:dev 指纹锁数据
*参数:resp 响应数据
*返回值:-1 fail 0 success
*备注:
*/
int DeviceController::devDoorLockProc(uint32_t cmd, tsDevFingerPrintLockTypedef *dev, char **resp)
{
	int err = ENUM_ERROR_NONE;
	switch (cmd){
	case ENUM_CMD_ADD_LOCK:
		if(dev->dev_basic.dev_id == 0){
			err = DCI_ERROR_DEV_INSERT;
			break;
		}
		//检查设备是否已经存在
		if(device_model->isExistDev(dev->dev_basic.dev_id, "device_basic_tb")){
			err = DCI_ERROR_DEV_EXIST;
			break;
		}
		//插入设备总表
		err = device_model->insterBasicDev(&dev->dev_basic);
		if(err != SQLITE_OK){
			DEBUG("Error: basic device insert\r\n");
			err = DCI_ERROR_DEV_INSERT;
			break;
		}
		//获得响应数据
		if(resp != NULL)
			*resp = getRespStr(err);
		err = ENUM_ERROR_NONE;
		break;
	case ENUM_CMD_DELETE_LOCK:
		//从设备总表中删除
		err = device_model->deleteDev(dev->dev_basic.dev_id, "device_basic_tb");
		if(err < 0){
			err = DCI_ERROR_DEV_DELETE;
			break;
		}
		//获得响应数据
		if(resp != NULL)
			*resp = getRespStr(err);
		err = ENUM_ERROR_NONE;
		break;
	case ENUM_CMD_SET_TEMP_PASSWORD:
	{
		if(!device_model->isExistDev(dev->dev_basic.dev_id, "device_basic_tb")){
			err = DCI_ERROR_DEV_NOT_EXIST;
			break;
		}
		//get mac
		if((err = device_model->selectBasicDev(dev->dev_basic.dev_id, &dev->dev_basic)) == 0){
			//send message
			err = this->zb_ctrl->zbDoorLockSetTempPassword("stp", dev->dev_basic.mac, dev->temp_password);
		}
		if(err < 0){
			err = ENUM_ERROR_UNKNOWN;
			break;
		}
		//获得响应数据
		if(resp != NULL)
			*resp = getRespStr(err);
		err = ENUM_ERROR_NONE;
	}
	break;
	case ENUM_CMD_INVALIT_TEMP_PASSWORD:
	{
			if(!device_model->isExistDev(dev->dev_basic.dev_id, "device_basic_tb")){
				err = DCI_ERROR_DEV_NOT_EXIST;
				break;
			}
			//get mac
			if((err = device_model->selectBasicDev(dev->dev_basic.dev_id, &dev->dev_basic)) == 0){
				//send message
				err = this->zb_ctrl->zbDoorLockSetTempPassword("itp", dev->dev_basic.mac, 0xffffffff);
			}
			if(err < 0){
				err = ENUM_ERROR_UNKNOWN;
				break;
			}
			//获得响应数据
			if(resp != NULL)
				*resp = getRespStr(err);
			err = ENUM_ERROR_NONE;
	}
	break;
	case ENUM_CMD_SET_HOLD_PASSWORD:
	break;
	case ENUM_CMD_NEW_USER:
	break;
	case ENUM_CMD_DELETE_USER:
	break;
	case ENUM_CMD_ADD_DIGIT_PASSWORD:
	break;
	case ENUM_CMD_EDIT_DIGIT_PASSWORD:
	break;
	case ENUM_CMD_DELETE_DIGIT_PASSWORD:
	break;
	case ENUM_CMD_ENTER_LEARN:
	break;
	case ENUM_CMD_OUT_LEARN:
	break;
	case ENUM_CMD_EDIT_PASSWORD_ATTRIBUTE:
	break;
	case ENUM_CMD_SET_AGING_DATE:
	break;
	case ENUM_CMD_SET_SETTING:
	break;

	default:
		err = ENUM_ERROR_INVALID_CMD;
		break;
	}
	if(resp != NULL && err != ENUM_ERROR_NONE)
		*resp = getRespStr(err);
	return err;
}
/*
*描述:开关设备处理函数
*参数:json 输入数据 resp 要返回的响应数据
*返回值:-1 fail 0 success
*备注:
*/
int DeviceController::devSwitchProc(uint32_t cmd, DevSwitchTypedef *dev, char **resp)
{
	int err = ENUM_ERROR_NONE;
	DevSwitchTypedef dev_for_read;

	switch (cmd)
	{
  	case ENUM_CMD_ADD://新增设备
  		if(dev->dev_basic.dev_id == 0){
			err = DCI_ERROR_DEV_INSERT;
			break;
		}
		//检查设备是否已经存在
  		if(device_model->isExistDev(dev->dev_basic.dev_id, "device_sensor_tb")){
  			err = DCI_ERROR_DEV_EXIST;
  			break;
  		}

		err = device_model->insterSensorDev(dev);
		if(err != SQLITE_OK){
			DEBUG("Error: device insert\r\n");
			err = DCI_ERROR_DEV_INSERT;
			break;
		}
		//插入设备总表
		err = device_model->insterBasicDev(&dev->dev_basic);
		if(err != SQLITE_OK){
			DEBUG("Error: basic device insert\r\n");
			err = DCI_ERROR_DEV_INSERT;
			break;
		}
		//获得响应数据
		if(resp != NULL)
			*resp = getRespStr(err);
		err = ENUM_ERROR_NONE;
  		break;

  	case ENUM_CMD_DELETE:
		//删除设备数据
  		err = device_model->deleteDev(dev->dev_basic.dev_id, "device_sensor_tb");
		if(err < 0){
			err = DCI_ERROR_DEV_DELETE;
//			break;
		}
		//从设备总表中删除
		err = device_model->deleteDev(dev->dev_basic.dev_id, "device_basic_tb");
		if(err < 0){
			err = DCI_ERROR_DEV_DELETE;
			break;
		}
		//获得响应数据
		if(resp != NULL)
			*resp = getRespStr(err);
		err = ENUM_ERROR_NONE;
  		break;

  	case ENUM_CMD_EDIT:
  		if(dev->dev_basic.dev_id != 0){
  	 		if(!device_model->isExistDev(dev->dev_basic.dev_id, "device_sensor_tb")){
  	  			err = DCI_ERROR_DEV_NOT_EXIST;
  	  			break;
  	  		}
  			//读取设备数据
  			dev_for_read.dev_basic.dev_type = dev->dev_basic.dev_type;
  			dev_for_read.dev_basic.dev_id = dev->dev_basic.dev_id;
  			device_model->selectSensorDev(&dev_for_read);
  			if(err < 0){
  				err = DCI_ERROR_DEV_SELECT;
  				break;
  			}
  			dev_for_read.ch = dev->ch;
//  			dev_for_read.pre_value_int = dev_for_read.value_int[dev->ch];
  			dev_for_read.u32_value[dev->ch] = dev->u32_value[dev->ch];
  			zb_ctrl->zbLampCtrl(dev_for_read.dev_basic.mac, dev->ch, dev_for_read.u32_value[dev->ch]);
  			//保存设备数据
  			device_model->updateSensorDev(&dev_for_read);
  			if(err < 0){
  				err = DCI_ERROR_DEV_UPDATE;
  				break;
  			}
  		}else{
  			zb_ctrl->zbLampCtrl(dev->dev_basic.mac, dev->ch, dev->u32_value[dev->ch]);
  		}

		//获得响应数据
		if(resp != NULL)
			*resp = getRespStrWhithValue(dev->u32_value[dev->ch], err);
		err = ENUM_ERROR_NONE;
  		break;

  	case ENUM_CMD_GET:
  		if(!device_model->isExistDev(dev->dev_basic.dev_id, "device_sensor_tb")){
  			err = DCI_ERROR_DEV_NOT_EXIST;
  			break;
  		}
		//读取设备数据
		dev_for_read.dev_basic.dev_type = dev->dev_basic.dev_type;
		dev_for_read.dev_basic.dev_id = dev->dev_basic.dev_id;
		device_model->selectSensorDev(&dev_for_read);
		if(err < 0){
			err = ENUM_ERROR_DEV_READ;
			break;
		}
		//从zigbee数据库读取value
//			dev_for_read.u32_value = zb_ctrl->zbLampGetValue(dev->dev_basic.mac, dev->dev_basic.endpoint);
		//获得响应数据
		if(resp != NULL)
			*resp = getRespStrWhithValue(dev_for_read.u32_value[dev->ch], err);
		if(*resp == NULL){
			err = ENUM_ERROR_MALLOC_FALSE;
			break;
		}
		err = ENUM_ERROR_NONE;
  		break;
  	default:
		err = ENUM_ERROR_INVALID_CMD;
  		break;
  }
	if(resp != NULL && err != ENUM_ERROR_NONE)
		*resp = getRespStr(err);
	return err;

}

/*
*描述:事件设备处理函数
*参数:json 输入数据 resp 要返回的响应数据
*返回值:-1 fail 0 success
*备注:
*/
int DeviceController::devEventProc(uint32_t cmd, cJSON *json, char **resp)
{
	int err=0;
	switch (cmd)
  {
  	case ENUM_CMD_ADD://新增设备
  		break;

  	case ENUM_CMD_DELETE:
		break;

  	case ENUM_CMD_EDIT:
		break;

  	case ENUM_CMD_GET:
  		break;
  	case ENUM_CMD_TRIG:

  		break;
  	default :
  		break;
  }
	return err;
}

/*
*描述:触发器处理函数
*参数:json 输入数据 resp 要返回的响应数据
*返回值:-1 fail 0 success
*备注:
*/
int DeviceController::devTrigerProc(uint32_t cmd, DevTriggerTypedef *dev, char **resp){
	DevTriggerTypedef dev_for_read;
	int i=0, err=0, arraysize = 0;
	cJSON *pSub = NULL, *action_json = NULL, *action_item_json = NULL;
	switch (cmd)
  {
  	case ENUM_CMD_ADD://新增设备
  		if(dev->dev_basic.dev_id == 0){
  			err = DCI_ERROR_DEV_INSERT;
  			break;
  		}
		err = device_model->insterTriggerDev(dev);
		if(err < 0){
			err = DCI_ERROR_DEV_INSERT;
			break;
		}
  		if(resp != NULL)
				*resp = getRespStr(err);
  		err = ENUM_ERROR_INVALID_CMD;
  		break;

  	case ENUM_CMD_DELETE:
  		err = device_model->deleteDev(dev->dev_basic.dev_id, "device_trigger_tb");
  		if(err < 0){
			err = DCI_ERROR_DEV_DELETE;
			break;
		}
		if(resp != NULL)
			*resp = getRespStr(err);
		err = ENUM_ERROR_INVALID_CMD;
		break;

  	case ENUM_CMD_EDIT:
  		if(!device_model->isExistDev(dev->dev_basic.dev_id, "device_trigger_tb")){
  			err = DCI_ERROR_DEV_NOT_EXIST;
  			break;
  		}
  		err = device_model->updateTriggerDev(dev);
  		if(err < 0){
			err = DCI_ERROR_DEV_UPDATE;
			break;
		}
  		if(resp != NULL)
  			*resp = getRespStr(err);
  		err = ENUM_ERROR_INVALID_CMD;
		break;

  	case ENUM_CMD_GET:
  		if(!device_model->isExistDev(dev->dev_basic.dev_id, "device_trigger_tb")){
  			err = DCI_ERROR_DEV_NOT_EXIST;
  			break;
  		}
		//读取设备数据
		dev_for_read.dev_basic.dev_type = dev->dev_basic.dev_type;
		dev_for_read.dev_basic.dev_id = dev->dev_basic.dev_id;
		err = device_model->selectTriggerDev(&dev_for_read);
		if(err < 0){
			err = DCI_ERROR_DEV_SELECT;
			break;
		}
		//获得响应数据
		if(resp != NULL)
			*resp = getRespStrWhithData(&dev_for_read.dev_basic, (void *)&dev_for_read, err);
		if(*resp == NULL){
			err = ENUM_ERROR_MALLOC_FALSE;
			break;
		}
		err = ENUM_ERROR_INVALID_CMD;
  		break;

  	case ENUM_CMD_TRIG:
  		if(!device_model->isExistDev(dev->dev_basic.dev_id, "device_trigger_tb")){
  			err = DCI_ERROR_DEV_NOT_EXIST;
  			break;
  		}
		//读取设备数据
		dev_for_read.dev_basic.dev_type = dev->dev_basic.dev_type;
		dev_for_read.dev_basic.dev_id = dev->dev_basic.dev_id;
		err = device_model->selectTriggerDev(&dev_for_read);
		if(err < 0){
			err = DCI_ERROR_DEV_SELECT;
			break;
		}
		action_json = cJSON_Parse(dev_for_read.action);
		arraysize = cJSON_GetArraySize(action_json);/*获得条件的个数*/
		for(i=0;i<arraysize;i++)
		{
			action_item_json = cJSON_GetArrayItem(action_json, i);/*逐个获取条件内容*/
			pSub = cJSON_GetObjectItem(action_item_json, TYPE_STR);
			if(pSub == NULL){
				continue;
			}
			if(pSub->valueint == ENUM_DEV_BUTTON
				|| pSub->valueint == ENUM_DEV_EVENT
				|| pSub->valueint == ENUM_DEV_DELAY){			//判断类型，添加不同指令
				devCmdHandle(ENUM_CMD_TRIG, action_item_json, NULL);
			}else{
				devCmdHandle(ENUM_CMD_EDIT, action_item_json, NULL);
			}
		}
		if(action_json){
			cJSON_Delete(action_json);
			action_json = NULL;
		}
		if(resp != NULL)
			*resp = getRespStr(err);
		err = ENUM_ERROR_INVALID_CMD;
  		break;

  	default:
		err = ENUM_ERROR_INVALID_CMD;
  		break;
  }
	if(resp != NULL && err != ENUM_ERROR_INVALID_CMD)
		*resp = getRespStr(err);
	return err;
}

/*
*描述:触发器处理函数
*参数:json 输入数据 resp 要返回的响应数据
*返回值:-1 fail 0 success
*备注:
*/
int DeviceController::devDelayProc(uint32_t cmd, cJSON *json, char **resp)
{
	int err=0;
	cJSON * pSub = cJSON_GetObjectItem(json, VALUE_STR);
	if(NULL == pSub)
		return -1;
	delay_ms(pSub->valueint * 1000);
	if(resp != NULL)
		*resp = getRespStr(err);
	return err;
}

/*
*描述:红外设备处理函数
*参数:
*返回值:
*备注:
*/
int DeviceController::devInfraredProc(uint32_t cmd, cJSON *json, char **resp)
{
	return 0;
}

static cJSON *cj_dev_list_data=NULL;

/**
 *描述:zcb 回调函数
 *参数:
 *返回值：
 * */
int zcbCbGetNewDevice( newdb_zcb_t * pzcb ){
	cJSON *cj_item = cJSON_CreateObject();
	if(cj_item && cj_dev_list_data){
		int dev_type = zbTypeToDevType(pzcb->type);
		uint64_t mac;
		sscanf(pzcb->mac, "%llX", &mac);
		if(dev_type && !g_device_model->isExistDev(mac, "device_basic_tb")){
			cJSON_AddNumberToObject(cj_item, TYPE_STR, dev_type);
			cJSON_AddStringToObject(cj_item, MAC_STR, pzcb->mac);
			cJSON_AddItemToArray(cj_dev_list_data, cj_item);
		}
	}
	return 1;
}
/**
 *描述:zcb 回调函数
 *参数:
 *返回值：
 * */
int zcbCbGetAllDevice( newdb_zcb_t * pzcb ){
	cJSON *cj_item = cJSON_CreateObject();
	if(cj_item && cj_dev_list_data){
		int dev_type = zbTypeToDevType(pzcb->type);
		if(dev_type){
			cJSON_AddNumberToObject(cj_item, TYPE_STR, dev_type);
			cJSON_AddStringToObject(cj_item, MAC_STR, pzcb->mac);
			cJSON_AddItemToArray(cj_dev_list_data, cj_item);
		}
	}
	return 1;
}
/*
*描述:dev list process
*参数:
*返回值:
*备注:
*/
int DeviceController::devListHandle(int dev_type, char **resp)
{
	int ret=0;
	cJSON *cj_resp=NULL;

	//允许设备加入ZigBee网络
	this->zb_ctrl->zbPermitjoinStart();

	switch(dev_type){
	case ENUM_DEV_NEW:
		cj_resp = cJSON_CreateObject();
		cJSON_AddNumberToObject(cj_resp, CODE_STR, 0);
		cJSON_AddStringToObject(cj_resp, MSG_STR, "sucess");
		cj_dev_list_data = cJSON_CreateArray();

		//get devices list
		newDbOpen();
		newDbLoopZcb( zcbCbGetNewDevice );
		newDbClose();
		//
		cJSON_AddItemToObject(cj_resp, DATA_STR, cj_dev_list_data);
		if(resp != NULL)
			*resp = cJSON_PrintUnformatted(cj_resp);
		cJSON_Delete(cj_resp);
		break;
	case ENUM_DEV_ALL:
		cj_resp = cJSON_CreateObject();
		cJSON_AddNumberToObject(cj_resp, CODE_STR, 0);
		cJSON_AddStringToObject(cj_resp, MSG_STR, "sucess");
		cj_dev_list_data = cJSON_CreateArray();

		//get devices list
		newDbOpen();
		newDbLoopZcb( zcbCbGetAllDevice );
		newDbClose();
		//
		cJSON_AddItemToObject(cj_resp, DATA_STR, cj_dev_list_data);
		if(resp != NULL)
			*resp = cJSON_PrintUnformatted(cj_resp);
		cJSON_Delete(cj_resp);
		break;

	}
	return ret;
}
//int DeviceController::devListHandle(int dev_type, char **resp)
//{
//	#define MAXBUF    10000
//	size_t i=0;
//	int ret=0;
//	char *buf=(char *)malloc(MAXBUF);
//	char * table = NULL;
//	cJSON *cj_resp=NULL;
//	cJSON *cj_data=NULL;
//	std::vector<std::string> row_str;
//	std::vector<std::string> col_str;
//
//	buf[0] = '\0';
//	switch(dev_type){
//	case ENUM_DEV_NEW:
//		cj_resp = cJSON_CreateObject();
//		cJSON_AddNumberToObject(cj_resp, CODE_STR, 0);
//		cJSON_AddStringToObject(cj_resp, MSG_STR, "sucess");
//		cj_data = cJSON_CreateArray();
//
//		//get devices list
//		newDbOpen();
//		table = newDbSerializeDevs( MAXBUF, buf );
//		newDbClose();
//		row_str = split(table, ';');
//		for (i =1; i < row_str.size(); i ++) {
//			col_str = split(row_str[i], ',');
//			//is exist in database these mac
//			uint64_t mac;
//			sscanf(col_str[1].c_str(), "%llu", &mac);
//			if(!device_model->isExistDev(mac, "device_basic_tb")){
//				//if not than append to response json
//				cJSON *cj_item = cJSON_CreateObject();
//				cJSON_AddNumberToObject(cj_item, TYPE_STR, zbTypeToDevType(atoi(col_str[2].c_str())) );
//				cJSON_AddStringToObject(cj_item, MAC_STR, col_str[1].c_str());
//				cJSON_AddItemToArray(cj_data, cj_item);
//			}
//		}
//		//
//		cJSON_AddItemToObject(cj_resp, DATA_STR, cj_data);
//		if(resp != NULL)
//			*resp = cJSON_PrintUnformatted(cj_resp);
//		cJSON_Delete(cj_resp);
//		break;
//	case ENUM_DEV_ALL:
//		cj_resp = cJSON_CreateObject();
//		cJSON_AddNumberToObject(cj_resp, CODE_STR, 0);
//		cJSON_AddStringToObject(cj_resp, MSG_STR, "sucess");
//		cj_data = cJSON_CreateArray();
//
//		newDbOpen();
//		table = newDbSerializeDevs( MAXBUF, buf );
//		newDbClose();
////		printf("\r\ntable:%s\r\n", table);
//		row_str = split(table, ';');
//		for (i =1; i < row_str.size(); i ++) {
//			col_str = split(row_str[i], ',');
////			for (j =0; j < col_str.size(); j ++){
////				cout << col_str[j] << ", ";
////			}
//			cJSON *cj_item = cJSON_CreateObject();
//			cJSON_AddNumberToObject(cj_item, TYPE_STR, zbTypeToDevType(atoi(col_str[2].c_str())) );
//			cJSON_AddStringToObject(cj_item, MAC_STR, col_str[1].c_str());
//			cJSON_AddItemToArray(cj_data, cj_item);
//		}
//				//
//		cJSON_AddItemToObject(cj_resp, DATA_STR, cj_data);
//		if(resp != NULL)
//			*resp = cJSON_PrintUnformatted(cj_resp);
//		cJSON_Delete(cj_resp);
//		break;
//	}
//	free(buf);
//	return ret;
//}
/*
*描述:cmd handle
*参数:
*返回值:
*备注:
*/
int DeviceController::devCmdHandle(int cmd_type, cJSON *pJson, char **resp)
{
	int err=0;
	int dev_type=0;
	void *dev_struct = NULL;

	if(NULL == pJson){
		return -1;
	}
	//解析数据，填充结构体
	dev_type = device_model->getDevType(pJson);

	//根据设备类型处理数据
	DEBUG("process device %d\r\n", dev_type);
	switch(dev_type)
	{
	case ENUM_DEV_NEW:
	case ENUM_DEV_ALL:
		if(cmd_type == ENUM_CMD_GET_LIST)
			devListHandle(dev_type, resp);
		else if(resp)
			*resp=getRespStr(err);
		break;
	case ENUM_DEV_SWITCH_1:
	case ENUM_DEV_SWITCH_2:
	case ENUM_DEV_SWITCH_3:
	case ENUM_DEV_SWITCH_4:
	case ENUM_DEV_ON_OFF_PLUG:
	case ENUM_DEV_ON_OFF_86_PLUG:
	case ENUM_DEV_ON_OFF_CURTAIN:
	case ENUM_DEV_CURTAIN_1:
	case ENUM_DEV_CURTAIN_2:
		dev_struct = (DevSwitchTypedef *)calloc(1, sizeof(DevSwitchTypedef));
		err = device_model->convertJson2Struct(pJson, dev_type, dev_struct);
		err = devSwitchProc(cmd_type, (DevSwitchTypedef *)dev_struct, resp);
		break;
	case ENUM_DEV_DIMMING_1:
	case ENUM_DEV_DIMMING_2:
	case ENUM_DEV_PUSH:
	case ENUM_DEV_BLINDS:
	case ENUM_DEV_COLOR_LED:
		break;

	case ENUM_DEV_CO:
	case ENUM_DEV_SMOKE:
	case ENUM_DEV_WATER:
	case ENUM_DEV_GAS:
	case ENUM_DEV_HUMAN:
	case ENUM_DEV_TEMP_HUMIDITY:
	case ENUM_DEV_DOOR_CONTACT:
	case ENUM_DEV_LUMINESCENCE:
		dev_struct = (DevSensorTypedef *)calloc(1, sizeof(DevSensorTypedef));
		err = device_model->convertJson2Struct(pJson, dev_type, dev_struct);
		err = devSensorProc(cmd_type, (DevSensorTypedef *)dev_struct, resp);
		break;
	case ENUM_DEV_FINGER_PRINT_LOCK:
	{
		tsDevFingerPrintLockTypedef sDevFingerPrint;
		j2sBasic(pJson, &sDevFingerPrint.dev_basic);
		if(j2sFingerPrintLock(pJson, &sDevFingerPrint))
			err = devDoorLockProc(cmd_type, &sDevFingerPrint, resp);
	}
	break;
  	case ENUM_DEV_INFRARED:
		err = devInfraredProc(cmd_type, pJson, resp);
  		break;
	case ENUM_DEV_TRIGGER:
		dev_struct = (DevTriggerTypedef *)calloc(1, sizeof(DevTriggerTypedef));
		err = device_model->convertJson2Struct(pJson, dev_type, dev_struct);
		err = devTrigerProc(cmd_type, (DevTriggerTypedef *)dev_struct, resp);
		break;
	case ENUM_DEV_EVENT:
		err = devEventProc(cmd_type, pJson, resp);
		break;
	case ENUM_DEV_DELAY:
		err = devDelayProc(cmd_type, pJson, resp);
		break;
	case ENUM_DEV_TIME:
		break;
  	case ENUM_DEV_BUTTON:
//		dev_struct = (DevButtonTypedef *)calloc(1, sizeof(DevButtonTypedef));
//		err = device_model->convertJson2Struct(pJson, dev_type, dev_struct);
  		break;
  	default:
		err = ENUM_ERROR_INVALID_DEV;
		if(resp)
			*resp=getRespStr(err);
  		break;
	}
	if(dev_struct)
		free(dev_struct);
	return err;
}

