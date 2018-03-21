/*
 * main.c
 *
 *  Created on: 2017-6-7
 *      Author: z
 */
#include <fstream>
#include <map>
#include <cctype>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <signal.h>

#include <sqlite3.h>
#include "onenet.h"
#include "DeviceController.h"
#include "DeviceModel.h"
/*iot_gw*/
#include "socket.h"
#include "jsonCreate.h"
#include "atoi.h"
#include "newDb.h"
#include "newLog.h"
#include "gateway.h"

#include "cJSON/cJSON.h"
#include "CHttpClient.h"
#include "dc_common.h"
#include "NotifyServer.h"

/*************** global variables *********************/
DeviceController *g_device_ctrl=NULL;
DeviceModel *g_device_model=NULL;
CHttpClient *g_chttp_client=NULL;
SystemConfig g_systemcfg;
volatile bool is_login_onenet;
volatile bool g_program_runing=true;
/*************** extern function **********************/
extern bool gc_thread( void * argv );

/*************** local variables *********************/

/*************** local funcation *********************/
static void systemconfig_init(void);
void test(void);

void Stop(int signo)
{
    printf("oops! stop!!!\n");
    g_program_runing=false;
}
int main(int argc,char *argv[])
{
	CHttpClient chttp_client;
	DeviceModel device_model;
	ZigbeeController zbc;
	DeviceController device_ctrl(&device_model, &zbc, &chttp_client);
	g_device_ctrl = &device_ctrl;
	g_device_model = &device_model;
	g_chttp_client = &chttp_client;

	printf("iot_dc start\r\n");
//	sleep(10);
	signal(SIGINT, Stop);
	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);
	systemconfig_init();
    device_model.init();
    notify_server_start();
	onenet_init(g_device_ctrl);

	while(g_program_runing){
		sleep(5);
	}
	g_program_runing=false;
	notify_server_stop();
	onenet_uninit();
	device_model.uninit();
	curl_global_cleanup();
	pthread_exit(NULL);
}

static void systemconfig_init(void){
	bool is_need_save_config = false;
    // TODO read config
    if(dc_config_read(&g_systemcfg) == false){
    	// get mac
    	if(!getIfMacStr(g_systemcfg.mac, "eno1"))
    		getIfMacStr(g_systemcfg.mac, "eth0");

    	printf("mac: %s\r\n", g_systemcfg.mac);

    	g_systemcfg.is_configured = false;
    	g_systemcfg.is_register = false;
    	is_need_save_config = true;
    }
	// TODO receive config data
	if(!g_systemcfg.is_configured){
//		if(gc_thread(NULL))
//			g_systemcfg.is_configured = true;
		is_need_save_config = true;
	}

	// TODO register device
	if(!g_systemcfg.is_register){
		if(g_device_ctrl->doRegisterDevice())
			g_systemcfg.is_register = true;
		else
			printf("Error: doRegisterDevice\r\n");

		is_need_save_config = true;
	}
	if(is_need_save_config)
		dc_config_save(&g_systemcfg);
}


/****************** test func ************************/

int libcurl_test_post(void)
{
  CURL *curl;
  CURLcode res;


  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */
    curl_easy_setopt(curl, CURLOPT_URL, "http://postit.example.com/moo.cgi");
    /* Now specify the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }

  return 0;
}

int lcurl_test(void)
{
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if(curl) {
    struct curl_slist *chunk = NULL;

    /* Remove a header curl would otherwise add by itself */
    chunk = curl_slist_append(chunk, "Accept:");

    /* Add a custom header */
    chunk = curl_slist_append(chunk, "Another: yes");

    /* Modify a header curl otherwise adds differently */
    chunk = curl_slist_append(chunk, "Host: example.com");

    /* Add a header with "blank" contents to the right of the colon. Note that
       we're then using a semicolon in the string we pass to curl! */
    chunk = curl_slist_append(chunk, "X-silly-header;");

    /* set our custom set of headers */
    res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    curl_easy_setopt(curl, CURLOPT_URL, "localhost");
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);

    /* free the custom headers */
    curl_slist_free_all(chunk);
  }
  return 0;
}
void response_trigger_test(void){
	/*返回触发器的ID给服务器*/
	char *cmd_resp = getRespStrWithId(26, 31011);
//	std::string cmd_resp = "{\"onenetid\":24704373,\"devid\":26,\"type\":31011}";
	if(cmd_resp){
		std::string strResponse;
		g_device_ctrl->chttp->Post_json(POST_TRIGGER_MESSAGE_URL,
						cmd_resp,
						strResponse);
		std::cout << strResponse << std::endl;
		free(cmd_resp);
	}
}
void response_switch_test(void){
	DevSwitchTypedef dev;
	dev.dev_basic.dev_id=7;
	dev.dev_basic.dev_type = 20131;
	dev.dev_basic.endpoint=1;
	dev.dev_basic.group_addr=444;
	dev.dev_basic.mac=0x00158D000119FF57;
	dev.dev_basic.short_addr=777;
	dev.u32_value[1] = 1;
	dev.ch=1;
//    char *cmd_resp=NULL;
//    if(getNotifyString(dev.dev_basic.dev_type, (void *)&dev, &cmd_resp)){
//    				std::string strResponse;
//    				g_device_ctrl->chttp->Post_json(POST_VAlUE_MESSAGE_URL,
//    						cmd_resp,
//    						strResponse);
//    				std::cout << strResponse << std::endl;
//    				if(cmd_resp)
//    					free(cmd_resp);
//    			}
//	char *cmd_resp = "{\"dev\":\"switch\",\"devid\":5,\"type\":20131,\"onenetid\":24704373,\"mac\":\"dc4a3ee7016c\",\"value\":0,\"ch\":1}";
//	if(getNotifyString(dev.dev_basic.dev_type, (void *)&dev, &cmd_resp)){
//		std::string strResponse;
//		g_device_ctrl->chttp->Post(POST_VAlUE_MESSAGE_URL,
//				"Content-Type: application/json",
//				cmd_resp,
//				strResponse);
//		std::cout << strResponse << std::endl;
//		free(cmd_resp);
//	}
}

void devBasicTest(DeviceModel *device_model){
	DevBasicTypedef dev_basic;
	dev_basic.dev_id=111;
	dev_basic.dev_type = 20131;
	dev_basic.endpoint=333;
	dev_basic.group_addr=444;
	dev_basic.mac=666666;
	dev_basic.short_addr=777;

	device_model->insterBasicDev(&dev_basic);
	memset(&dev_basic, 0, sizeof(DevBasicTypedef));
	dev_basic.mac=666666;
	device_model->selectBasicDev(&dev_basic);

	printf("dev_id=%d\r\n", dev_basic.dev_id);

}

void newDbtest(void){
	char buf[512];
	if(newDbOpen() == 1)
		printf("newDbOpen success\r\n");
	else
		printf("newDbOpen fail\r\n");
	char * table = NULL;
	table = newDbSerializeDevs( 512, buf );
	printf( "    <table>%s</table>\n", ( table ) ? table : "-" );
	newDbClose();
}


void list_test(void){
	using namespace std;
	string table="id,mac,dev,ty,par,nm,heat,cool,tmp,hum,prs,co2,bat,batl,als,xloc,yloc,zloc,sid,cmd,lvl,rgb,kelvin,act,sum,flags,lastupdate;"
			"0,00158D000119FF57,6,dim,0,,0,0,0,0,0,0,0,0,0,0,0,0,0,off,0,0,0,0,0,9,1516522037";
//	char *resp=NULL;
//	    g_device_ctrl->devListHandle(ENUM_DEV_NEW, &resp);
//	    if(resp){
//	    	printf("resp:%s\r\n", resp);
//	    	free(resp);
//	    }

//	std::vector<std::string> row_str = split(table, ';');
//
//	for (size_t i =1; i < row_str.size(); i ++) {
//
//		std::vector<std::string> col_str = split(row_str[i], ',');
//		for (size_t j =0; j < col_str.size(); j ++)
//			cout << col_str[j] << endl;
//
//	}
	/*
	 *
		sub_buf = table;
		i=0;
		j=0;
		while ((p[i][j] = strtok_r(sub_buf, ";", &outer_ptr)) != NULL){
			sub_buf = p[i][j];
			j=0;
			while ((p[i][j++] = strtok_r(sub_buf, ",", &inner_ptr)) != NULL){
			  in++;
			  sub_buf = NULL;
			}
			i++;
			row++;
			sub_buf = NULL;
		}
		newDbClose();

		printf("Here we have %d strings\n", in+1);
		for (i = 0; i < row; i++){
			for (j = 0; j < 27; j++)
				printf("%s, ", p[i][j]);
			printf("\r\n");
		}
		fflush(stdout);
	 *
	 * */
}


void routine_test(void *arg){

	printf("routine_test\r\n");
}
void *thread_test(void *arg){
	pthread_cleanup_push(routine_test,NULL);
	char *str = (char *)malloc(128);
	strcpy(str, "hello");
	printf("thread_test\r\n");
	printf("%s",str);
	free(str);
	fflush(stdout);
	pthread_cleanup_pop(0);
	pthread_exit(NULL);
}
void test(void){
	libcurl_test_post();
//	response_switch_test();
}

//     set up output stream for line buffering using space that
//     will be obtained through an indirect call to malloc
//    if (setvbuf(stdout, NULL, _IOLBF, 132) != 0)
//        printf("failed to set up buffer for output file\n");
//    else
//        printf("buffer set up for output file\n");
