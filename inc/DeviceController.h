/*
 * DeviceController.h
 *
 *  Created on: 2017-6-7
 *      Author: z
 */

#ifndef DEVICE_CONTROLLER_H_
#define DEVICE_CONTROLLER_H_

#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <stdint.h>
#include "edp_c/EdpKit.h"
#include "DeviceModel.h"
#include "ZigbeeController.h"
#include "CHttpClient.h"


#define URL_REGISTER "http://47.94.6.236:81/device/index/zigbee_register"
#define POST_TRIGGER_MESSAGE_URL "http://47.94.6.236:81/device/scene/report_trigger"
#define POST_VAlUE_MESSAGE_URL "http://47.94.6.236:81/device/device/report_num_equipment"


/*******public function ****************/

extern char *getRespStr(int err);
extern char *getRespStrWithId(int id, int dev_type);
extern char *getRespStrWhithData(DevBasicTypedef *dev_basic, void *dev_struct, int err);
extern char *getRespStrWhithValue(int value, int err);

class DeviceController {
public:

	DeviceModel *device_model;
	ZigbeeController *zb_ctrl;
	CHttpClient *chttp;
	std::string source_id;
public:
	DeviceController(DeviceModel *dm, ZigbeeController *zbc, CHttpClient *chttp);
	virtual ~DeviceController();

	bool doRegisterDevice(void);

	void devTrigerScan(bool check_dev_value);

	int json_time_cmp(struct tm *local_time, cJSON *valuejson);
	int json_value_cmp(DevSensorTypedef *dev_struct,cJSON *ifjson);

	int devValueCmp(cJSON *json );
	int devTimerCmp(cJSON *json,char **resp);
	int devSensorProc(uint32_t cmd, DevDoorTypedef *dev, char **resp);
	int devDoorLockProc(uint32_t cmd, tsDevFingerPrintLockTypedef *dev, char **resp);
	int devSwitchProc(uint32_t cmd, DevSwitchTypedef *dev, char **resp);
	int devEventProc(uint32_t cmd, cJSON *json, char **resp);
	int devTrigerProc(uint32_t cmd, DevTriggerTypedef *dev, char **resp);
	int devDelayProc(uint32_t cmd, cJSON *json, char **resp);
	int devInfraredProc(uint32_t cmd, cJSON *json, char **resp);
	int devListHandle(int dev_type, char **resp);
	int devCmdHandle(int cmd_type, cJSON *pJson, char **resp);
};

#endif /* DEVICE_CONTROLLER_H_ */
