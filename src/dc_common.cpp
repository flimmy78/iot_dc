/*
 * dc_common.c
 *
 *  Created on: Jan 31, 2018
 *      Author: zhuang
 */

#include "dc_common.h"
#include "nibbles.h"
#include "newDb.h"

//#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG(format,...) do{printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__);fflush(stdout);}while(0)
#else
#define DEBUG(format,...)
#endif


uint64_t zcbNodeGetExtendedAddress( uint16_t shortAddress ) {
    DEBUG( "Get node extended address for 0x%04x\n",
            (int)shortAddress );

    newdb_zcb_t zcb;
    if ( newDbGetZcbSaddr( shortAddress, &zcb ) ) {

        uint64_t u64mac = nibblestr2u64( zcb.mac );
        return u64mac;
    }

    return 0;
}

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

bool getIfMacStr(char mac[16], const char *if_name){

    struct   ifreq   ifreq;
    int   sock;

    if((sock=socket(AF_INET,SOCK_STREAM,0)) <0)
    {
        perror( "socket ");
        return   false;
    }
    strcpy(ifreq.ifr_name, if_name);
    if(ioctl(sock,SIOCGIFHWADDR,&ifreq) <0)
    {
        perror( "ioctl ");
        return   false;
    }
    sprintf(mac, "%02x%02x%02x%02x%02x%02x",
            (unsigned   char)ifreq.ifr_hwaddr.sa_data[0],
            (unsigned   char)ifreq.ifr_hwaddr.sa_data[1],
            (unsigned   char)ifreq.ifr_hwaddr.sa_data[2],
            (unsigned   char)ifreq.ifr_hwaddr.sa_data[3],
            (unsigned   char)ifreq.ifr_hwaddr.sa_data[4],
            (unsigned   char)ifreq.ifr_hwaddr.sa_data[5]);
    return true;
}

/*
 * {
	"is_configured":	false,
	"home_ssid":	"",
	"home_password":	"",
	"userid":	"",
	"is_register":	false,
	"onenet_devid":	"24704373",
	"onenet_apikey":	"g2HnDqPTvG2rV5vqstiC",
	"yaokan_key":	"",
	"ir_active_code":	""
	}
*/
const char *system_config_path = "/usr/bin/iot_dc/SystemConfig.json";
//const char *system_config_path = "SystemConfig.json";
bool dc_config_save(SystemConfig *config){

	FILE *fp;
	bool ret;

	fp = fopen( system_config_path, "w");
	if(fp){
		cJSON *cj_cfg = cJSON_CreateObject();
		cJSON_AddBoolToObject(cj_cfg, "is_configured", config->is_configured);
		cJSON_AddStringToObject(cj_cfg, "home_ssid", config->home_ssid);
		cJSON_AddStringToObject(cj_cfg, "home_password", config->home_password);
		cJSON_AddStringToObject(cj_cfg, "userid", config->userid);
		cJSON_AddBoolToObject(cj_cfg, "is_register", config->is_register);
		cJSON_AddStringToObject(cj_cfg, "onenet_devid", config->onenet_devid);
		cJSON_AddStringToObject(cj_cfg, "onenet_apikey", config->onenet_apikey);
		cJSON_AddStringToObject(cj_cfg, "yaokan_key", config->yaokan_key);
		cJSON_AddStringToObject(cj_cfg, "ir_active_code", config->ir_active_code);
		cJSON_AddStringToObject(cj_cfg, "mac", config->mac);

		char *str_cfg = cJSON_Print(cj_cfg);
		fseek(fp, 0, SEEK_SET );
		fwrite(str_cfg, sizeof(char), strlen(str_cfg), fp);
		fflush(fp);
		fclose(fp);

		cJSON_Delete(cj_cfg);
		cj_cfg=NULL;
		if(str_cfg){
			free(str_cfg);
			str_cfg=NULL;
		}

		ret=true;
	}else{
		printf("Error: fopen\r\n");
		ret=false;
	}
	return ret;
}

bool dc_config_read(SystemConfig *config){
	FILE *fp;
	bool ret;

	fp = fopen( system_config_path, "r");
	if(fp){

		char *str_cfg = (char *)calloc(sizeof(char),1024);
		fseek(fp, 0, SEEK_SET );
		fread(str_cfg, sizeof(char), 1024, fp);
		printf("config: %s\r\n", str_cfg);

		cJSON *cj_cfg = cJSON_Parse(str_cfg);
		cJSON *cj_item=NULL;

		cj_item = cJSON_GetObjectItem(cj_cfg, "is_configured");
		if(cj_item){
			config->is_configured = (bool)cj_item->type;
		}

		cj_item = cJSON_GetObjectItem(cj_cfg, "home_ssid");
		if(cj_item){
			strcpy(config->home_ssid, cj_item->valuestring );
		}

		cj_item = cJSON_GetObjectItem(cj_cfg, "home_password");
		if(cj_item){
			strcpy(config->home_password, cj_item->valuestring );
		}

		cj_item = cJSON_GetObjectItem(cj_cfg, "userid");
		if(cj_item){
			strcpy(config->userid, cj_item->valuestring );
		}

		cj_item = cJSON_GetObjectItem(cj_cfg, "is_register");
		if(cj_item){
			config->is_register = (bool)cj_item->type;
		}

		cj_item = cJSON_GetObjectItem(cj_cfg, "onenet_devid");
		if(cj_item){
			strcpy(config->onenet_devid, cj_item->valuestring );
		}

		cj_item = cJSON_GetObjectItem(cj_cfg, "onenet_apikey");
		if(cj_item){
			strcpy(config->onenet_apikey, cj_item->valuestring );
		}

		cj_item = cJSON_GetObjectItem(cj_cfg, "yaokan_key");
		if(cj_item){
			strcpy(config->yaokan_key, cj_item->valuestring );
		}

		cj_item = cJSON_GetObjectItem(cj_cfg, "ir_active_code");
		if(cj_item){
			strcpy(config->ir_active_code, cj_item->valuestring );
		}

		cj_item = cJSON_GetObjectItem(cj_cfg, "mac");
		if(cj_item){
			strcpy(config->mac, cj_item->valuestring );
		}

		fclose(fp);

		cJSON_Delete(cj_cfg);
		if(str_cfg){
			free(str_cfg);
			str_cfg=NULL;
		}
		ret=true;
	}else{
		printf("Error: fopen\r\n");
		ret=false;
	}
	return ret;
}

void delay_ms(int ms)
{
	usleep(ms*1000);
}
void delay_us(int us)
{
	usleep(us);
}
