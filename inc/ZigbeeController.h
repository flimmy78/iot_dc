/*
 * ZigbeeController.h
 *
 *  Created on: Jan 2, 2018
 *      Author: zhuang
 */

#ifndef ZIGBEECONTROLLER_H_
#define ZIGBEECONTROLLER_H_

#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <stdint.h>
#include "DeviceModel.h"

class ZigbeeController {
public:
	ZigbeeController();
	virtual ~ZigbeeController();

	bool zbLampGetNew(DevBasicTypedef *dev_basic);
	int zbLampCtrl(uint64_t mac, uint8_t endpoint, uint32_t value);
	int zbLampGetValue(uint64_t mac, uint8_t endpoint);
	int zbDoorLockSetTempPassword(char *cmd, uint64_t mac, uint32_t temp_password);
	int zbPermitjoinStart(void);
	int zbPermitjoinStop(void);
};

#endif /* ZIGBEECONTROLLER_H_ */
