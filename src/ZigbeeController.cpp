/*
 * ZigbeeController.cpp
 *
 *  Created on: Jan 2, 2018
 *      Author: zhuang
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include "ZigbeeController.h"
/*iot_gw*/
#include "socket.h"
#include "jsonCreate.h"
#include "atoi.h"
#include "newDb.h"
#include "newLog.h"
#include "gateway.h"

#include "NotifyServer.h"

#define CONTROL_PORT    "2001"
#define PERMIT_DURATION       180  // 3 minutes = 180 secs

// -------------------------------------------------------------
// Write control
// -------------------------------------------------------------

static int writeControl( char * cmd ) {

    int handle = socketOpen( (char *)"localhost", (char *)CONTROL_PORT, 0 );

    if ( handle >= 0 ) {

        socketWriteString( handle, cmd );
        socketClose( handle );

        return( 0 );
    } else {
        newLogAdd( NEWLOG_FROM_CGI, (char *)"Control socket problem" );
        return( 888 );
    }
    return( 123 );
}

ZigbeeController::ZigbeeController() {
	// TODO Auto-generated constructor stub

}

ZigbeeController::~ZigbeeController() {
	// TODO Auto-generated destructor stub
}

bool ZigbeeController::zbLampGetNew(DevBasicTypedef *dev_basic){
//	int msgid;
//	struct notifyMessage msg;
//
//	key_t msg_key = ftok(NOTIFY_PATHNAME, NOTIFY_PROJ_ID);
//	msgid = msgget(msg_key,IPC_EXCL );/*检查消息队列是否存在 */
//	if(msgid < 0){
//		printf("msq not existed! errno=%d [%s]\n",errno,strerror(errno));
//		is_need_post_msg = false;
//		return (false);
//	}
//	/*接收消息队列*/
//    is_need_post_msg = true;
//    msgrcv(msgid,&msg,sizeof(struct notifyMessage),0,0);
//	is_need_post_msg = false;
//	dev_basic->mac = msg.u64IEEEAddress;
//	dev_basic->endpoint = msg.u8Endpoint;
//	dev_basic->short_addr = msg.u16ShortAddress;
	return true;
}
int ZigbeeController::zbLampCtrl(uint64_t mac, uint8_t endpoint, uint32_t value){

	int err=0;
	char mac_buf[17];
	char cmd_buf[8];

	sprintf(mac_buf, "%016llX", mac);
	sprintf(cmd_buf, "%s", value == 1 ? "on" : "off");

    if ( strlen( mac_buf ) > 0 ) {
    	writeControl(jsonControl( mac_buf, cmd_buf, -1, -1, -1, -1, -1, -1, endpoint ));
    } else {
    	err = -1;
    	printf("Error: mac\r\n");
    }
    return err;
}
int ZigbeeController::zbLampGetValue(uint64_t mac, uint8_t endpoint){

	return 0;
}

int ZigbeeController::zbDoorLockSetTempPassword(char *cmd, uint64_t mac, uint32_t temp_password){

	int err=0;
	char mac_buf[17];

	sprintf(mac_buf, "%016llX", mac);

    if ( strlen( mac_buf ) > 0 ) {
    	writeControl(jsonDoorLockControl( mac_buf, cmd, temp_password, -1));
    } else {
    	err = -1;
    	printf("Error: mac\r\n");
    }
    return err;
}

int ZigbeeController::zbPermitjoinStart(void){
	newLogAdd( NEWLOG_FROM_CGI, (char *)"Start permit-join" );
    writeControl( jsonCmdSetPermitJoin( NULL, PERMIT_DURATION ) );
	return 0;
}

int ZigbeeController::zbPermitjoinStop(void){
	newLogAdd( NEWLOG_FROM_CGI, (char *)"Stop permit-join" );
	writeControl( jsonCmdSetPermitJoin( NULL, 0 ) );
	return 0;
}
