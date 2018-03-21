/*
 * NotifyServer.h
 *
 *  Created on: Jan 10, 2018
 *      Author: zhuang
 */
//#ifdef __cplusplus
//extern "C" {
//#endif

#ifndef NOTIFYSERVER_H_
#define NOTIFYSERVER_H_



    struct notifyMessage {
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
        uint64_t u64IEEEAddress;
    }__attribute__((__packed__));

//	#define NOTIFY_PATHNAME "notify_server"
//	#define NOTIFY_PROJ_ID 1

	extern volatile bool is_need_post_msg;
	void notify_server_start(void);
	void notify_server_stop(void);
#endif /* NOTIFYSERVER_H_ */

//#ifdef __cplusplus
//}
//#endif
