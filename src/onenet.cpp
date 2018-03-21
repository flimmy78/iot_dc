/*
 * onenet.cpp
 *
 *  Created on: 2017-6-7
 *      Author: z
 */

#include "onenet.h"
#include <sys/time.h>    //struct itimerval, setitimer()
#include <signal.h>        //signal()
#include "dc_common.h"

#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG(format,...) do{printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__);fflush(stdout);}while(0)
#else
#define DDEBUG(format,...)
#endif

/*----------------------------错误码-----------------------------------------*/
#define ERR_CREATE_SOCKET   -1
#define ERR_HOSTBYNAME      -2
#define ERR_CONNECT         -3
#define ERR_SEND            -4
#define ERR_TIMEOUT         -5
#define ERR_RECV            -6
/*---------------统一linux和windows上的Socket api----------------------------*/
#ifndef htonll
#ifdef _BIG_ENDIAN
#define htonll(x)   (x)
#define ntohll(x)   (x)
#else
#define htonll(x)   ((((uint64)htonl(x)) << 32) + htonl(x >> 32))
#define ntohll(x)   ((((uint64)ntohl(x)) << 32) + ntohl(x >> 32))
#endif
#endif
/* linux程序需要定义_LINUX */
#ifdef _LINUX
#define Socket(a,b,c)          socket(a,b,c)
#define Connect(a,b,c)         connect(a,b,c)
#define Close(a)               close(a)
#define Read(a,b,c)            read(a,b,c)
#define Recv(a,b,c,d)          recv(a, (void *)b, c, d)
#define Select(a,b,c,d,e)      select(a,b,c,d,e)
#define Send(a,b,c,d)          send(a, (const int8 *)b, c, d)
#define Write(a,b,c)           write(a,b,c)
#define GetSockopt(a,b,c,d,e)  getsockopt((int)a,(int)b,(int)c,(void *)d,(socklen_t *)e)
#define SetSockopt(a,b,c,d,e)  setsockopt((int)a,(int)b,(int)c,(const void *)d,(int)e)
#define GetHostByName(a)       gethostbyname((const char *)a)
#endif

static int onenet_sockfd;
static pthread_t tid_recv;//,tid_ping;
static pthread_t tid_trigger_sacn;
static DeviceController *device_ctr=NULL;

/*
 * buffer按十六进制输出
 */
void hexdump(const unsigned char *buf, uint32 num)
{
    uint32 i = 0;
    for (; i < num; i++)
    {
        printf("%02X", buf[i]);
//        if ((i+1)%8 == 0)
//            printf("\n");
    }
    printf("\n");
}
/*
 * 函数名:  Open
 * 功能:    创建socket套接字并连接服务端
 * 参数:    addr    ip地址
 *          protno  端口号
 * 说明:    这里只是给出一个创建socket连接服务端的例子, 其他方式请查询相关socket api
 * 相关socket api:
 *          socket, gethostbyname, connect
 * 返回值:  类型 (int32)
 *          <=0     创建socket失败
 *          >0      socket描述符
 */
int32 Open(const uint8 *addr, int16 portno)
{
    int32 sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    /* 创建socket套接字 */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "ERROR opening socket\n");
        return ERR_CREATE_SOCKET;
    }
    server = GetHostByName(addr);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        return ERR_HOSTBYNAME;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    /* 客户端 建立与TCP服务器的连接 */
    if (Connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "ERROR connecting\n");
        return ERR_CONNECT;
    }
#ifdef _DEBUG
    printf("[%s] connect to server %s:%d succ!...\n", __func__, addr, portno);
#endif
    return sockfd;
}
/*
 * 函数名:  DoSend
 * 功能:    将buffer中的len字节内容写入(发送)socket描述符sockfd, 成功时返回写的(发送的)字节数.
 * 参数:    sockfd  socket描述符
 *          buffer  需发送的字节
 *          len     需发送的长度
 * 说明:    这里只是给出了一个发送数据的例子, 其他方式请查询相关socket api
 *          一般来说, 发送都需要循环发送, 是因为需要发送的字节数 > socket的写缓存区时, 一次send是发送不完的.
 * 相关socket api:
 *          send
 * 返回值:  类型 (int32)
 *          <=0     发送失败
 *          >0      成功发送的字节数
 */
int32 DoSend(int32 sockfd, const char* buffer, uint32 len)
{
    int32 total  = 0;
    int32 n = 0;
    while (len != (uint32)total)
    {
        /* 试着发送len - total个字节的数据 */
        n = Send(sockfd,buffer + total,len - total,MSG_NOSIGNAL);
        if (n <= 0)
        {
            fprintf(stderr, "ERROR writing to socket\n");
            return n;
        }
        /* 成功发送了n个字节的数据 */
        total += n;
    }
    /* wululu test print send bytes */
//    hexdump((const unsigned char *)buffer, len);
    return total;
}
/*
 * 函数名:  recv_thread_func
 * 功能:    接收线程函数
 * 参数:    arg     socket描述符
 * 说明:    这里只是给出了一个从socket接收数据的例子, 其他方式请查询相关socket api
 *          一般来说, 接收都需要循环接收, 是因为需要接收的字节数 > socket的读缓存区时, 一次recv是接收不完的.
 * 相关socket api:
 *          recv
 * 返回值:  无
 */
void *recv_thread_func(void* arg)
{
    int sockfd = *(int*)arg;
    int error = 0;
    int n, rtn;
    uint8 mtype, jsonorbin;
    char buffer[1024];
    RecvBuffer* recv_buf = NewBuffer();
    EdpPacket* pkg;

    char* src_devid;
    char* push_data;
    uint32 push_datalen;

    //cJSON* save_json;
    //char* save_json_str;

    cJSON* desc_json;
    char* desc_json_str;
    char* save_bin;
    uint32 save_binlen;
    unsigned short msg_id;
    unsigned char save_date_ret;

    char* cmdid;
    uint16 cmdid_len;
    char*  cmd_req;
    uint32 cmd_req_len;
    EdpPacket* send_pkg;
    char* ds_id;
    double dValue = 0;
    //int iValue = 0;
    //char* cValue = NULL;

    char* simple_str = NULL;
    char* cmd_resp;
    unsigned cmd_resp_len = 0;

	DataTime stTime = {0};

    FloatDPS* float_data = NULL;
    int count = 0;
    int i = 0;

    struct UpdateInfoList* up_info = NULL;

	int cmd_type=0;
#ifdef _DEBUG
    printf("[%s] recv thread start ...\n", __func__);
#endif

    while (error == 0 && g_program_runing)
    {
        /* 试着接收1024个字节的数据 */
//        n = recv(sockfd, buffer, 1024, MSG_WAITALL | MSG_NOSIGNAL);
        n = recv(sockfd, buffer, 1024, MSG_NOSIGNAL);
        if (n <= 0)
            continue;
        printf("recv from server, bytes: %d\n", n);
        /* wululu test print send bytes */
//        hexdump((const unsigned char *)buffer, n);
        /* 成功接收了n个字节的数据 */
        WriteBytes(recv_buf, buffer, n);
        while (1)
        {
            /* 获取一个完成的EDP包 */
            if ((pkg = GetEdpPacket(recv_buf)) == 0)
            {
                printf("need more bytes...\n");
                break;
            }
            /* 获取这个EDP包的消息类型 */
            mtype = EdpPacketType(pkg);
#ifdef _ENCRYPT
            if (mtype != ENCRYPTRESP){
                if (g_is_encrypt){
                    SymmDecrypt(pkg);
                }
            }
#endif
            /* 根据这个EDP包的消息类型, 分别做EDP包解析 */
            switch(mtype)
            {
#ifdef _ENCRYPT
            case ENCRYPTRESP:
                UnpackEncryptResp(pkg);
                break;
#endif
            case CONNRESP:
                /* 解析EDP包 - 连接响应 */
                rtn = UnpackConnectResp(pkg);
                printf("recv connect resp, rtn: %d\n", rtn);
                break;
            case PUSHDATA:
                /* 解析EDP包 - 数据转发 */
                UnpackPushdata(pkg, &src_devid, &push_data, &push_datalen);
                printf("recv push data, src_devid: %s, push_data: %s, len: %d\n",
                       src_devid, push_data, push_datalen);
                free(src_devid);
                free(push_data);
                break;
            case UPDATERESP:
                UnpackUpdateResp(pkg, &up_info);
                while (up_info){
                    printf("name = %s\n", up_info->name);
                    printf("version = %s\n", up_info->version);
                    printf("url = %s\nmd5 = ", up_info->url);
                    for (i=0; i<32; ++i){
                        printf("%c", (char)up_info->md5[i]);
                    }
                    printf("\n");
                    up_info = up_info->next;
                }
                FreeUpdateInfolist(up_info);
                break;

            case SAVEDATA:
                /* 解析EDP包 - 数据存储 */
                if (UnpackSavedata(pkg, &src_devid, &jsonorbin) == 0)
                {
                    if (jsonorbin == kTypeFullJson
                        || jsonorbin == kTypeSimpleJsonWithoutTime
                        || jsonorbin == kTypeSimpleJsonWithTime)
                    {
                        printf("json type is %d\n", jsonorbin);
                        /* 解析EDP包 - json数据存储 */
                        /* UnpackSavedataJson(pkg, &save_json); */
                        /* save_json_str=cJSON_Print(save_json); */
                        /* printf("recv save data json, src_devid: %s, json: %s\n", */
                        /*     src_devid, save_json_str); */
                        /* free(save_json_str); */
                        /* cJSON_Delete(save_json); */

                        /* UnpackSavedataInt(jsonorbin, pkg, &ds_id, &iValue); */
                        /* printf("ds_id = %s\nvalue= %d\n", ds_id, iValue); */

                        UnpackSavedataDouble((SaveDataType)jsonorbin, pkg, &ds_id, &dValue);
                        printf("ds_id = %s\nvalue = %f\n", ds_id, dValue);

                        /* UnpackSavedataString(jsonorbin, pkg, &ds_id, &cValue); */
                        /* printf("ds_id = %s\nvalue = %s\n", ds_id, cValue); */
                        /* free(cValue); */

                        free(ds_id);

                    }
                    else if (jsonorbin == kTypeBin)
                    {/* 解析EDP包 - bin数据存储 */
                        UnpackSavedataBin(pkg, &desc_json, (uint8**)&save_bin, &save_binlen);
                        desc_json_str=cJSON_Print(desc_json);
                        printf("recv save data bin, src_devid: %s, desc json: %s, bin: %s, binlen: %d\n",
                               src_devid, desc_json_str, save_bin, save_binlen);
                        free(desc_json_str);
                        cJSON_Delete(desc_json);
                        free(save_bin);
                    }
                    else if (jsonorbin == kTypeString ){
                        UnpackSavedataSimpleString(pkg, &simple_str);

                        printf("%s\n", simple_str);
                        free(simple_str);
                    }else if (jsonorbin == kTypeStringWithTime){
						UnpackSavedataSimpleStringWithTime(pkg, &simple_str, &stTime);

                        printf("time:%u-%02d-%02d %02d-%02d-%02d\nstr val:%s\n",
							stTime.year, stTime.month, stTime.day, stTime.hour, stTime.minute, stTime.second, simple_str);
                        free(simple_str);
					}else if (jsonorbin == kTypeFloatWithTime){
                        if(UnpackSavedataFloatWithTime(pkg, &float_data, &count, &stTime)){
                            printf("UnpackSavedataFloatWithTime failed!\n");
                        }

                        printf("read time:%u-%02d-%02d %02d-%02d-%02d\n",
                            stTime.year, stTime.month, stTime.day, stTime.hour, stTime.minute, stTime.second);
                        printf("read float data count:%d, ptr:[%p]\n", count, float_data);

                        for(i = 0; i < count; ++i){
                            printf("ds_id=%u,value=%f\n", float_data[i].ds_id, float_data[i].f_data);
                        }

                        free(float_data);
                        float_data = NULL;
                    }
                    free(src_devid);
                }else{
                    printf("error\n");
                }
                break;
            case SAVEACK:
                UnpackSavedataAck(pkg, &msg_id, &save_date_ret);
                printf("save ack, msg_id = %d, ret = %d\n", msg_id, save_date_ret);
                break;
            case CMDREQ:
                if (UnpackCmdReq(pkg, &cmdid, &cmdid_len,
                                 &cmd_req, &cmd_req_len) == 0){
                    /*
                     * 用户按照自己的需求处理并返回，响应消息体可以为空.
                     * 处理完后需要释放
                     */
                	printf("cmd_req:%s\r\n", cmd_req);
					cJSON * pJson = cJSON_Parse(cmd_req);
					if(NULL == pJson){
						DEBUG("cJSON_Parse error");
					}else{
						cmd_type = device_ctr->device_model->getCmdType(pJson);
						device_ctr->devCmdHandle(cmd_type, pJson,&cmd_resp);//数据处理
					}
					/*生成响应包*/
					if(cmd_resp != NULL){
						printf("cmd_resp:%s\r\n", cmd_resp);
						cmd_resp_len = strlen(cmd_resp);
						send_pkg = PacketCmdResp(cmdid, cmdid_len, cmd_resp, cmd_resp_len);
					}

#ifdef _ENCRYPT
if (g_is_encrypt){
    SymmEncrypt(send_pkg);
}
#endif
					if(send_pkg != NULL){
						DoSend(sockfd, (const char*)send_pkg->_data, send_pkg->_write_pos);
						DeleteBuffer(&send_pkg);
					}
					if(pJson)
						cJSON_Delete(pJson);
					if(cmd_resp)
						free(cmd_resp);
					if(cmdid)
						free(cmdid);
					if(cmd_req)
						free(cmd_req);
                }
                break;
            case PINGRESP:
                /* 解析EDP包 - 心跳响应 */
                UnpackPingResp(pkg);
                printf("recv ping resp\n");
                break;

            default:
                /* 未知消息类型 */
                error = 1;
                printf("recv failed...\n");
                break;
            }
            DeleteBuffer(&pkg);
        }
//        usleep(100);
    }
    DeleteBuffer(&recv_buf);
    pthread_exit(NULL);
}

void ping_thread_func(int signal)
{
	if(signal == SIGALRM){
//	int sockfd = *(int*)arg;
	EdpPacket* send_pkg;
//    while (g_program_runing)
//    {
        send_pkg = PacketPing();
        printf("send bytes: %d\n", send_pkg->_write_pos);
        DoSend(onenet_sockfd, (const char*)send_pkg->_data, send_pkg->_write_pos);
        DeleteBuffer(&send_pkg);
        alarm(120);
//        sleep(2*60);
//    }
	}
//    pthread_exit(NULL);
}


//void trigger_sacn_thread(void* arg){
void *trigger_sacn_thread(void* arg){
	int sleep_count=0;
	while(g_program_runing){
		if(sleep_count > 30){
			sleep_count = 0;
			printf("trigger san...\n");
//			device_ctr->devTrigerScan(false);
			device_ctr->devTrigerScan(true);
		}
		sleep_count++;
	    sleep(1);
	}
	pthread_exit(NULL);
}
int onenet_init(DeviceController *dev_ctr)
{
    int ret;
    EdpPacket* send_pkg;
    struct timeval timeout = {3,0};

    if(dev_ctr == NULL){
    	fprintf(stderr, "DeviceController is null");
    	return -1;
    }
    device_ctr = dev_ctr;
//    const char* ip = "192.168.31.160";
    const char* ip = "183.230.40.39";
    const char* port = "876";

    const char* src_dev = g_systemcfg.onenet_devid;
    const char* src_api_key = g_systemcfg.onenet_apikey;

//    const char* src_dev = "607467";
//    const char* src_api_key = "rdeeUnZB1vuBg4ZLZQcZMVzVBv0A";
//    const char* src_dev = "24704373";//g_systemcfg.onenet_devid;
//    const char* src_api_key = "g2HnDqPTvG2rV5vqstiCMokBUPQ=";//g_systemcfg.onenet_apikey;

    /* create a socket and connect to server */
    onenet_sockfd = Open((const uint8*)ip, atoi(port));
    if (onenet_sockfd < 0)
        return -1;
    setsockopt(onenet_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
    /* create a recv thread */
    ret = pthread_create(&tid_recv,NULL, recv_thread_func,&onenet_sockfd);

    /* connect to server */
    /*    send_pkg = PacketConnect1(src_dev, "Bs04OCJioNgpmvjRphRak15j7Z8=");*/
    send_pkg = PacketConnect1(src_dev, src_api_key);
    /* send_pkg = PacketConnect2("433223", "{ \"SYS\" : \"0DEiuApATHgLurKNEl6vY4bLwbQ=\" }");*/
    /* send_pkg = PacketConnect2("433223", "{ \"13982031959\" : \"888888\" }");*/

    /* 向设备云发送连接请求 */
    printf("send connect to server, bytes: %d\n", send_pkg->_write_pos);
    ret=DoSend(onenet_sockfd, (const char*)send_pkg->_data, send_pkg->_write_pos);
    DeleteBuffer(&send_pkg);
    signal(SIGALRM, ping_thread_func); //注册安装信号
    alarm(120);
//    if(ret > 0)
//    	pthread_create(&tid_ping,NULL, ping_thread_func, &onenet_sockfd);

    //tirgger
    pthread_create(&tid_trigger_sacn,NULL, trigger_sacn_thread, NULL);

    /* close socket */
    //Close(onenet_sockfd);

    //pthread_join(tid_recv,NULL);
    return ret;
}
void onenet_uninit(void){
	DEBUG("onenet_uninit\r\n");
	DEBUG("wait tid_recv exit\r\n");
	pthread_join(tid_recv,NULL);
	DEBUG("wait tid_trigger_sacn exit\r\n");
	pthread_join(tid_trigger_sacn,NULL);

	close(onenet_sockfd);
}
