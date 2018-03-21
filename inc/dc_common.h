/*
 * dc_common.h
 *
 *  Created on: Jan 8, 2018
 *      Author: zhuang
 */

#ifndef DC_COMMON_H_
#define DC_COMMON_H_

#include <fstream>
#include <cctype>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <unistd.h>
#include <curl/curl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>

#include "DeviceController.h"
#include "DeviceController.h"
#include "DeviceModel.h"
#include "cJSON/cJSON.h"

typedef struct{
	bool is_configured;
	char home_ssid[16];
	char home_password[8];
	char userid[16];
	char onenet_devid[16];
	char onenet_apikey[48];
	bool is_register;
	char yaokan_key[32];
	char ir_active_code[50];
	char mac[16];
}SystemConfig;

extern DeviceController *g_device_ctrl;
extern DeviceModel *g_device_model;
extern CHttpClient *g_chttp_client;
extern SystemConfig g_systemcfg;
extern volatile bool is_login_onenet;
extern volatile bool g_program_runing;

uint64_t zcbNodeGetExtendedAddress( uint16_t shortAddress );
template<typename Out>
extern void split(const std::string &s, char delim, Out result);
extern std::vector<std::string> split(const std::string &s, char delim);
extern bool getIfMacStr(char mac[16], const char *if_name);
extern bool dc_config_save(SystemConfig *config);
extern bool dc_config_read(SystemConfig *config);
void delay_ms(int ms);
void delay_us(int us);
#endif /* DC_COMMON_H_ */
