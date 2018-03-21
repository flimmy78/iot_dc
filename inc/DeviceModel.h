/*
 * DeviceModel.h
 *
 *  Created on: Dec 24, 2017
 *      Author: zhuang
 */

#ifndef DEVICEMODEL_H_
#define DEVICEMODEL_H_

#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <stdint.h>
//#include <sys/io.h>
#include "sqlite3.h"
#include "edp_c/cJSON.h"

/*****json 字段定义******/
#define CMD_STR "cmd"
//#define DEV_STR "dev"
#define DEVID_STR "devid"
#define VALUE_STR "value"
#define PREVALUE_STR "prevalue"
#define LOLMT_STR "lolmt"
#define UPLMT_STR "uplmt"
#define STA_STR "sta"
#define TYPE_STR "type"
#define IF_STR "if"
#define THEN_STR "then"
#define LINK_STR "link"  /*该JSON 对象存储和硬件相关参数*/
#define USERID_STR "userid"
#define ONENETID_STR "onenetid"
//#define ERRNO_STR "err"
//#define ERROR_STR "error"
#define APIKEY_STR "apikey"
//#define DESCRIP_STR "descrip"
#define EVENT_STR "event"
#define MAC_STR "mac"
#define COMPARE_STR "compare"
#define CH_STR "ch"
#define CODE_STR "code"
#define MSG_STR "msg"
#define DATA_STR "data"
/**********json 属性*************/
//#define ERROR_STR_S "error"
//#define SUCC_STR "succ"
#define ON_STR "on"
#define OFF_STR "off"
#define NULL_STR "null"

#define CMD_ADD "add"
#define CMD_DELETE "delete"
#define CMD_EDIT "edit"
#define CMD_GET "get"
#define CMD_TRIG "trig"
#define CMD_RESP "resp"
#define CMD_BACK "back"
#define CMD_LOGIN "login"
#define CMD_REGISTER "register"
#define CMD_GET_LIST "get_list"

	typedef enum {
		ENUM_ERROR_NONE = 0,
		ENUM_ERROR_UNKNOWN = -1,
		ENUM_ERROR_INVALID_CMD = -2,
		ENUM_ERROR_INVALID_DEV = -3,
		ENUM_ERROR_DEV_EXIST = -4,
		ENUM_ERROR_MALLOC_FALSE = -5,

		ENUM_ERROR_LIST_ADD = -10,
		ENUM_ERROR_LIST_DELETE = -11,
		ENUM_ERROR_DEV_WRITE = -12,
		ENUM_ERROR_DEV_DELETE = -13,
		ENUM_ERROR_DEV_UPDATE = -14,
		ENUM_ERROR_DEV_READ = -15,
		ENUM_ERROR_JSON_PARSE = -16,
		ENUM_ERROR_LIST_UPDATE = -17,

		DCI_ERROR_DEV_INSERT = -100,
		DCI_ERROR_DEV_DELETE = -101,
		DCI_ERROR_DEV_UPDATE = -102,
		DCI_ERROR_DEV_SELECT = -103,
		DCI_ERROR_DEV_EXIST = -104,
		DCI_ERROR_DEV_NOT_EXIST = -105,
	}DevErrorEnum;

	typedef enum {
		ENUM_CMD_ADD = 0,
		ENUM_CMD_DELETE,
		ENUM_CMD_EDIT,
		ENUM_CMD_GET,
		ENUM_CMD_TRIG,
		ENUM_CMD_RESP,
		ENUM_CMD_BACK,

		ENUM_CMD_LOGIN,
		ENUM_CMD_REGISTER,
		ENUM_CMD_GET_LIST,
		//finger prints lock
		ENUM_CMD_ADD_LOCK,
		ENUM_CMD_DELETE_LOCK,
		ENUM_CMD_SET_TEMP_PASSWORD,
		ENUM_CMD_INVALIT_TEMP_PASSWORD,
		ENUM_CMD_SET_HOLD_PASSWORD,
		ENUM_CMD_NEW_USER,
		ENUM_CMD_DELETE_USER,
		ENUM_CMD_ADD_DIGIT_PASSWORD,
		ENUM_CMD_EDIT_DIGIT_PASSWORD,
		ENUM_CMD_DELETE_DIGIT_PASSWORD,
		ENUM_CMD_ENTER_LEARN,
		ENUM_CMD_OUT_LEARN,
		ENUM_CMD_EDIT_PASSWORD_ATTRIBUTE,
		ENUM_CMD_SET_AGING_DATE,
		ENUM_CMD_SET_SETTING,
	}DevCmdEnum;
	typedef enum {
		ENUM_DEV_NEW = 0,
		ENUM_DEV_BUTTON = 20000,
		ENUM_DEV_INFRARED = 22111,
		ENUM_DEV_SWITCH_1 = 20111,
		ENUM_DEV_SWITCH_2 = 20121,
		ENUM_DEV_SWITCH_3 = 20131,
		ENUM_DEV_SWITCH_4 = 20141,
		ENUM_DEV_DIMMING_1 = 20211,
		ENUM_DEV_DIMMING_2 = 20221,
		ENUM_DEV_CURTAIN_1 = 20311,
		ENUM_DEV_CURTAIN_2 = 20321,
		ENUM_DEV_ON_OFF_CURTAIN = 20411,
		ENUM_DEV_ON_OFF_PLUG = 20811,
		ENUM_DEV_ON_OFF_86_PLUG = 20821,
		ENUM_DEV_PUSH = 20511,
		ENUM_DEV_BLINDS = 20611,
		ENUM_DEV_COLOR_LED = 20711,
		ENUM_DEV_DOOR_CONTACT = 25111,
		ENUM_DEV_HUMAN = 25211,
		ENUM_DEV_CO = 25311,
		ENUM_DEV_SMOKE = 25411,
		ENUM_DEV_WATER = 25511,
		ENUM_DEV_GAS = 25611,
		ENUM_DEV_TEMP_HUMIDITY = 25711,
		ENUM_DEV_LUMINESCENCE = 25811,
		ENUM_DEV_FINGER_PRINT_LOCK = 25911,
		ENUM_DEV_TRIGGER = 31011,
		ENUM_DEV_EVENT = 32011,
		ENUM_DEV_DELAY = 33011,
		ENUM_DEV_TIME = 33111,
		ENUM_DEV_ALL = 99999,
	}DevTypeEnum;

	typedef enum {
		CONDITION_LAGER = 0,
		CONDITION_LAGER_EQUAL,
		CONDITION_LITTLE,
		CONDITION_LITTLE_EQUAL,
		CONDOTION_EQUAL,
		CONDITION_IN_OUT,
		CONDITION_CHANGE,
	}ConditionTypeEnum;

	typedef enum {
		E_FL_WARNING_NONE = 0,
		E_FL_WARNING_HOLD,
		E_FL_WARNING_DIGIT_PASSWORD,
		E_FL_WARNING_FINGER_PRINTS,
		E_FL_WARNING_IC_CARD
	}teFLWarning;
	/*-------------------- device model ----------------------*/
	//basic
	typedef struct dev_basic_typedef{
		uint32_t dev_type;
		uint32_t dev_id;
		uint64_t mac;
		uint16_t short_addr;
		uint16_t group_addr;
		uint16_t endpoint;
	}DevBasicTypedef;

	//button
	typedef struct dev_button_typedef{
		struct dev_basic_typedef dev_basic;
		uint32_t dev_id[10];
		uint8_t dev_id_num;
	}DevButtonTypedef;

	//switch
//	typedef struct dev_switch_typedef{
//		struct dev_basic_typedef dev_basic;
//		uint32_t ch;
//		uint32_t value_int[5];
//		uint32_t pre_value_int;
//	}DevSwitchTypedef;

	//sensor
	typedef struct dev_sensor_typedef{
		struct dev_basic_typedef dev_basic;
		uint32_t ch;
		uint32_t u32_value[5];
	}DevSensorTypedef, DevSwitchTypedef, DevHumanTypedef, DevDoorTypedef, DevSmokeTypedef, DevGasTypedef;

	//infrared
	typedef struct dev_infrared_typedef{
		struct dev_basic_typedef dev_basic;
		uint16_t value[512];
	}DevInfraredTypedef;


	//passward
	typedef struct fl_password_typedef{
		int id;
		int type;
		bool en;
		int password;
	}tsFLPasswordType;
	typedef struct fl_aging_date_typedef{
		char week[8];
		int year;
		int mounth;
		int mday;
		int hour;
		int minute;
	}tsFLAgingDateType;
	//user
	typedef struct fl_user_typedef{
		int id;
		int type;
		bool en;
		tsFLAgingDateType aging_date[2];
		tsFLPasswordType password;
	}tsFLUserType;
	//setting
	typedef struct fl_setting_typedef{
		int batter;
		int digt_warning;
		int finger_warning;
		int iccard_warning;
		bool upload;
	}tsFLSettingType;

	//finger_print_lock
	typedef struct dev_finger_print_lock_typedef{
		struct dev_basic_typedef dev_basic;
		int temp_password;
		int hold_password;
		teFLWarning warning;
		tsFLUserType user;
		tsFLSettingType setting;

	}tsDevFingerPrintLockTypedef;

	//event
	typedef struct dev_event_typedef{
		struct dev_basic_typedef dev_basic;
		uint16_t event[512];
	}DevEventTypedef;

	//trigger
	typedef struct dev_trigger_typedef{
		struct dev_basic_typedef dev_basic;
		int status;
		char condition[512];
		char action[512];
		int is_triggered;
	}DevTriggerTypedef;

	extern bool j2sBasic(cJSON *json, DevBasicTypedef *dev);
extern	bool s2jFingerPrintLock(cJSON *json, tsDevFingerPrintLockTypedef *dev);
extern	bool j2sFingerPrintLock(cJSON *json, tsDevFingerPrintLockTypedef *dev);
extern int convertStruct2Json(cJSON *json, uint32_t dev_type, void *dev_struct);
extern const char* getErrorMessage(int err);

class DeviceModel {
public:
	sqlite3 *sqlite3_db;

	DeviceModel();
	virtual ~DeviceModel();

	int init(void);
	int uninit(void);
	/*----------------------common fun----------------------------*/
	int getCmdType(cJSON *json);
	int getDevType(cJSON *json);
	int getConditionType(cJSON *json);
	const char *getDevTypeStr(uint32_t dev_type);
	int convertJson2Struct(cJSON *json, uint32_t dev_type, void *dev_struct);

	/*---------------------common fun----------------------------*/
	int deleteDev(uint32_t id, const char *tb);
	int deleteDev(uint64_t mac, const char *tb);
	bool isExistDev(uint32_t id, const char *tb);
	bool isExistDev(uint64_t mac, const char *tb);

	/*----------------------basic---------------------------*/
	int insterBasicDev(DevBasicTypedef *dev_basic);
	int updateBasicDev(DevBasicTypedef *dev_basic);
	int selectBasicDev(DevBasicTypedef *dev_basic);
	int selectBasicDev(uint32_t id, DevBasicTypedef *dev_basic);
	int selectBasicDev(uint64_t mac, DevBasicTypedef *dev_basic);
	/*----------------------switch---------------------------*/
//	int insterSwitchDev(DevSwitchTypedef *dev);
//	int deleteSwitchDev(DevSwitchTypedef *dev);
//	int updateSwitchDev(DevSwitchTypedef *dev);
//	int selectSwitchDev(DevSwitchTypedef *dev);
//	int isExistSwitchDev(DevSwitchTypedef *dev);

	/*---------------------- sensor ---------------------------*/
	int insterSensorDev(DevSensorTypedef *dev);
	int deleteSensorDev(DevSensorTypedef *dev);
	int updateSensorDev(DevSensorTypedef *dev);
	int selectSensorDev(DevSensorTypedef *dev);

	/*----------------------trigger---------------------------*/
	int insterTriggerDev(DevTriggerTypedef *dev);
	int deleteTriggerDev(DevTriggerTypedef *dev);
	int updateTriggerDev(DevTriggerTypedef *dev);
	int updateTriggerDev(int dev_id, int is_triggered);
	int selectTriggerDev(DevTriggerTypedef *dev);
	int isExistTriggerDev(DevTriggerTypedef *dev);
};


#endif /* DEVICEMODEL_H_ */
