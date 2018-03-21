/*
 * NotifyServer.cpp
 *
 *  Created on: Jan 10, 2018
 *      Author: zhuang
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>

#include "dc_common.h"
#include "NotifyServer.h"
#include "socket.h"
#include "newDb.h"
#include "DeviceModel.h"
#include "DeviceController.h"
#include "ZigbeeConstant.h"

/* Public variables ----------------------------------------------------------*/
extern DeviceController *g_device_ctrl;
volatile bool is_need_post_msg=false;

/* Public functions ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
struct _tsAttributeReport {
	uint8_t     u8SequenceNo;
	uint16_t    u16ShortAddress;
	uint8_t     u8Endpoint;
	uint16_t    u16ClusterID;
	uint16_t    u16AttributeID;
	uint8_t     u8AttributeStatus;
	uint8_t     u8Type;
	uint16_t    u16SizeOfAttributesInBytes;
	union {
		uint8_t     u8Data;
		uint16_t    u16Data;
		uint32_t    u32Data;
		uint64_t    u64Data;
	} uData;
} __attribute__((__packed__));

/* Private define ------------------------------------------------------------*/
#define SOCKET_HOST         "0.0.0.0"
#define SOCKET_PORT         "12313"//"3001"

#define INPUTBUFFERLEN      200
/* Private macro -------------------------------------------------------------*/
#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG(format,...) do{printf("File: "__FILE__", Line: %05d: "format"", __LINE__, ##__VA_ARGS__);fflush(stdout);}while(0)
#else
#define DEBUG(format,...)
#endif

/* Private variables ---------------------------------------------------------*/
static char socketInputBuffer[INPUTBUFFERLEN + 2];
static int serverSocketHandle;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static bool doPostValueMessage(const char *message);
static bool getNotifyString(uint32_t dev_type, void *dev_struct, char **resp);
/*
*描述:返回响应消息
*参数:json_str 要返回的json数据
*返回值:消息字符串
*备注:返回字符串需要free
*/
static bool getNotifyString(uint32_t dev_type, void *dev_struct, char **resp)
{
	cJSON *cj_value=NULL;

	if(dev_struct == NULL)
			return false;

	cj_value = cJSON_CreateObject();
	if(cj_value == NULL){
		return false;
	}
	cJSON_AddStringToObject(cj_value, ONENETID_STR, (char *)g_systemcfg.onenet_devid);
	convertStruct2Json(cj_value, dev_type, dev_struct);
	if(resp)
		*resp = cJSON_PrintUnformatted(cj_value);
	if(cj_value)
		cJSON_Delete(cj_value);
	return true;
}
/*
*描述:返回响应消息
*参数:json_str 要返回的json数据
*返回值:消息字符串
*备注:返回字符串需要free
*/
static bool doPostValueMessage(const char *message)
{
	if(message){
		std::string strResponse;
		g_device_ctrl->chttp->Post_json(POST_VAlUE_MESSAGE_URL,
				message,
				strResponse);
		std::cout << strResponse << std::endl;
		return true;
	}
	return false;
}
/**
 * \brief Handles an incoming attribute message
 * \param u16ShortAddress Short address of the device
 * \param u16ClusterID Cluster ID of the attribute
 * \param u16AttributeID Attribute ID
 * \param u64Data Data containes
 * \param u8Endpoint Originating endpoint
 */
static void devSensorHandle( struct dev_basic_typedef *dev_basic,
					  uint16_t u16ShortAddress,
                      uint16_t u16ClusterID,
                      uint16_t u16AttributeID,
                      uint64_t u64Data,
                      uint8_t  u8Endpoint ){
	DevSensorTypedef dev_sensor;
	char *cmd_resp = NULL;

	memcpy((void *)&dev_sensor.dev_basic, (void *)dev_basic, sizeof(struct dev_basic_typedef));
	//update database
	if(u16ClusterID == 0x0400)
		dev_sensor.ch = 2;
	else if(u16ClusterID == 0x0400 || u16ClusterID == 0x0406 || u16ClusterID == 0x0405)
		dev_sensor.ch = 1;
	else
		dev_sensor.ch = u8Endpoint;
//	dev_sensor.ch = (uint32_t)(u16ClusterID == 0x0402 ? 1 : 2);//0x0402 temp clsterid,0x0405 hum clusterid.
	dev_sensor.u32_value[dev_sensor.ch] = (uint32_t)u64Data;
	g_device_ctrl->device_model->updateSensorDev(&dev_sensor);

	//post data to server
	if(getNotifyString(dev_sensor.dev_basic.dev_type, (void *)&dev_sensor, &cmd_resp)){
		doPostValueMessage(cmd_resp);
		if(cmd_resp)
			free(cmd_resp);
	}
}
//static void devSwitchHandle( struct dev_basic_typedef *dev_basic,
//					  uint16_t u16ShortAddress,
//                      uint16_t u16ClusterID,
//                      uint16_t u16AttributeID,
//                      uint64_t u64Data,
//                      uint8_t  u8Endpoint ){
//	DevSwitchTypedef dev_switch;
//	char *cmd_resp = NULL;
//
//	memcpy((void *)&dev_switch.dev_basic, (void *)dev_basic, sizeof(struct dev_basic_typedef));
//	//update database
//	dev_switch.ch = (uint32_t)u8Endpoint;
//	dev_switch.u32_value[dev_switch.ch] = (uint32_t)u64Data;
//	g_device_ctrl->device_model->updateSwitchDev(&dev_switch);
//
//	//post data to server
//	if(getNotifyString(dev_switch.dev_basic.dev_type, (void *)&dev_switch, &cmd_resp)){
//		doPostValueMessage(cmd_resp);
//		if(cmd_resp)
//			free(cmd_resp);
//	}
//}
static void notify_server_handle(void * msg, int len){
	struct _tsAttributeReport *psMessage = (struct _tsAttributeReport *)msg;
    uint64_t u64IEEEAddress = 0;


	newDbOpen();
	u64IEEEAddress = zcbNodeGetExtendedAddress(psMessage->u16ShortAddress);
	newDbClose();

    uint64_t u64Data = 0;
    volatile int32_t  i32Data;

    switch(psMessage->u8Type) {
        case(E_ZCL_GINT8):
        case(E_ZCL_UINT8):
        case(E_ZCL_INT8):
        case(E_ZCL_ENUM8):
        case(E_ZCL_BMAP8):
        case(E_ZCL_BOOL):
        case(E_ZCL_OSTRING):
        case(E_ZCL_CSTRING):
            u64Data = (uint64_t )psMessage->uData.u8Data;
            break;

        case(E_ZCL_LOSTRING):
        case(E_ZCL_LCSTRING):
        case(E_ZCL_STRUCT):
        case(E_ZCL_INT16):
        case(E_ZCL_UINT16):
        case(E_ZCL_ENUM16):
        case(E_ZCL_CLUSTER_ID):
        case(E_ZCL_ATTRIBUTE_ID):
            u64Data = (uint64_t )ntohs(psMessage->uData.u16Data);
            break;

        case(E_ZCL_UINT24):
        case(E_ZCL_UINT32):
        case(E_ZCL_TOD):
        case(E_ZCL_DATE):
        case(E_ZCL_UTCT):
        case(E_ZCL_BACNET_OID):
            u64Data = (uint64_t )ntohl(psMessage->uData.u32Data);
            break;

        case E_ZCL_INT24:
            i32Data = (int32_t)ntohl(psMessage->uData.u32Data);
            // REPAIR/EXTEND SIGN
            i32Data <<= 8;
            i32Data >>= 8;
            u64Data = (uint64_t )i32Data;
            break;

        case(E_ZCL_UINT40):
        case(E_ZCL_UINT48):
        case(E_ZCL_UINT56):
        case(E_ZCL_UINT64):
        case(E_ZCL_IEEE_ADDR):
            u64Data = (uint64_t )be64toh(psMessage->uData.u64Data);
            break;

        default:
            printf( "Unknown attribute data type (%d)\n", psMessage->u8Type );
            printf( "-  8: Data = %llx\n", (uint64_t )psMessage->uData.u8Data );
            printf( "- 16: Data = %llx\n", (uint64_t )ntohs( psMessage->uData.u16Data ) );
            printf( "- 32: Data = %llx\n", (uint64_t )ntohl( psMessage->uData.u32Data ) );
            printf( "- 64: Data = %llx\n", (uint64_t )be64toh( psMessage->uData.u64Data ) );
            break;
    }

    printf( "Attribute report from 0x%04X - Endpoint %d, cluster 0x%04X, attribute 0x%04X.\n",
                    psMessage->u16ShortAddress,
                    psMessage->u8Endpoint,
                    psMessage->u16ClusterID,
                    psMessage->u16AttributeID
                );
	printf("data:%d\r\n", (uint32_t)u64Data);

//    if(is_need_post_msg){
//    	struct notifyMessage msg;
//    	memcpy(&msg, psMessage, sizeof(struct _tsAttributeReport));
//    	msg.u64IEEEAddress = u64IEEEAddress;
//        /* 发送消息队列 */
//        int ret_value = msgsnd(notify_msg_id,&msg,sizeof(struct notifyMessage),IPC_NOWAIT);
//        if ( ret_value < 0 ) {
//    	   printf("msgsnd() write msg failed,err=%d[%s]\n",errno,strerror(errno));
//        }
//    }

    //
	struct dev_basic_typedef dev_basic;
    dev_basic.mac = u64IEEEAddress;
    dev_basic.dev_id = 0;

    if(u64IEEEAddress){
        DEBUG("selectBasicDev");
        if(g_device_ctrl->device_model->isExistDev(u64IEEEAddress, "device_basic_tb") &&
        		g_device_ctrl->device_model->selectBasicDev(u64IEEEAddress, &dev_basic) == SQLITE_OK){
        	switch(dev_basic.dev_type){

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
        		devSensorHandle( &dev_basic,
								psMessage->u16ShortAddress,
								psMessage->u16ClusterID,
								psMessage->u16AttributeID,
								u64Data,
								psMessage->u8Endpoint );
        		break;
        	default:
        		break;
        	}
        	g_device_ctrl->devTrigerScan(true);
        }
    }
}


static void *notify_server_run(void *arg){
//	pthread_cleanup_push(notify_server_routine,NULL);
//	notify_msg_id = create_msg();
//	printf("msqid: %d\r\n", notify_msg_id);
    serverSocketHandle = socketOpen( (char*)SOCKET_HOST, (char*)SOCKET_PORT, 1 );
    if ( serverSocketHandle >= 0 ) {
    	 while ( g_program_runing ) {
    		 int len;
    		 int clientSocketHandle = socketAccept( serverSocketHandle );
    		 if ( clientSocketHandle >= 0 ) {
				 // len = socketRead( socketHandle, socketInputBuffer, INPUTBUFFERLEN );
				 len = socketReadWithTimeout( clientSocketHandle,
						   socketInputBuffer, INPUTBUFFERLEN, 10 );
				 if ( len >= 0 ) {
					 notify_server_handle(socketInputBuffer, len);
				 }
				 socketClose( clientSocketHandle );
    		 }
    	}
    	 socketClose( serverSocketHandle );
    }

//    msgctl(notify_msg_id,IPC_RMID,0); //删除消息队列
//    pthread_cleanup_pop(0);
    pthread_exit(NULL);
}
int make_socket_non_blocking (int sfd) {
    int flags = fcntl (sfd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }
    if (fcntl(sfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}
#define BACKLOG 5
int fd_A[BACKLOG];
int conn_amount;
#define BUF_SIZE 256
void *xnotify_server_run(void *arg)
{
    int nSocktfd, nNewSocktfd;
    int i,nRet,nMaxSock;
    socklen_t nSockaddrLen;
    struct sockaddr_in client_addr;
    char szBuf[BUF_SIZE];
    fd_set fdRead;
    struct timeval tv;

    serverSocketHandle = socketOpen( (char*)SOCKET_HOST, (char*)SOCKET_PORT, 1 );
    if (serverSocketHandle < 0)
    {
        DEBUG("ServerBuildSocket faile.\n");
        exit(0);
    }

    conn_amount = 0;
    nSockaddrLen = sizeof(client_addr);
    nMaxSock = serverSocketHandle;

    while(g_program_runing)
    {
        FD_ZERO(&fdRead);
        FD_SET(nSocktfd,&fdRead);

        tv.tv_sec = 3;
        tv.tv_usec = 0;

        //add active connect to fd set
        for (i = 0; i < BACKLOG; i++) {
            if (fd_A[i] != 0)
            {
                FD_SET(fd_A[i],&fdRead);
            }
        }

        nRet = select(nMaxSock + 1, &fdRead, NULL, NULL, &tv);
        if (nRet < 0)
        {
            DEBUG("select failed.\n");
            break;
        }
        else if(nRet == 0)
        {
//            DEBUG("timeout.\n");
            continue;
        }

        //check whether the connection have data to recieve
        for (i = 0; i < BACKLOG; i++)
        {
            if (fd_A[i] && FD_ISSET(fd_A[i],&fdRead))
            {
                nRet = recv(fd_A[i],szBuf,sizeof(szBuf),0);
                if (nRet <= 0)
                {
                    DEBUG("Client %d close\n",i);
                    close(fd_A[i]);
                    FD_CLR(fd_A[i],&fdRead);
                    fd_A[i] = 0;
                    conn_amount --;
                }
                else
                {
                	notify_server_handle(szBuf, nRet);
//                    if (nRet < BUF_SIZE)
//                        memset(&szBuf[nRet],'\0',1);
//                        DEBUG("Client[%d] send: %s.\n",i,szBuf);
                }
            }
         }//for

        //check whether a new connection comes
        if (FD_ISSET(nSocktfd,&fdRead))
        {
            nNewSocktfd = accept(serverSocketHandle, (struct sockaddr *)&client_addr,&nSockaddrLen);
            if (nNewSocktfd <= 0)
            {
                DEBUG("accept failed.\n");
                continue;
            }

            if (conn_amount < BACKLOG -1)
            {
                for(i = 0; i < BACKLOG; i++)
                {
                    if(fd_A[i] == 0)
                    {
                        fd_A[i] = nNewSocktfd;
                        conn_amount ++;
                        break;
                    }
                }
                DEBUG("new connection client[%d] %s:%d\n", conn_amount,
                        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                if (nNewSocktfd > nMaxSock)
                {
                    nMaxSock = nNewSocktfd;
                }
            }
            else
            {
                DEBUG("max connections arrive,exit\n");
                close(nNewSocktfd);
                continue;
            }
        }
       }//while

    //close all connections
    for (i = 0; i < BACKLOG; i++)
    {
        if (fd_A[i] != 0)
        {
            close(fd_A[i]);
        }
    }

    pthread_exit(NULL);
}
static pthread_t notify_server_thread_id;
void notify_server_start(void){
	 pthread_create(&notify_server_thread_id,NULL, notify_server_run, NULL);
}
void notify_server_stop(void){
	pthread_cancel(notify_server_thread_id);
//	DEBUG("wait notify server exit\r\n");
	pthread_join(notify_server_thread_id,NULL);
}


// -------------------------------------------------------------
// dcSendNotify
// return 0:succ
// return 888:sockt error
// author zhuangqs
// -------------------------------------------------------------

int notify_server_send_msg( void * msg, int len ) {

    int handle = socketOpen( (char*)"localhost", (char*)"3001", 0 );

    if ( handle >= 0 ) {
    	socketWrite( handle, (char *)msg, len );
        socketClose( handle );

        return( 0 );
    } else {
        printf("Control socket problem\n" );
        return( 888 );
    }
    return( 123 );
}

/********************************* END OF FILE *********************************/

