/*
 * onenet.h
 *
 *  Created on: 2017-6-7
 *      Author: z
 */

#ifndef ONENET_H_
#define ONENET_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <getopt.h>
#include <libgen.h>
#include <time.h>

#include "edp_c/EdpKit.h"
#ifdef _ENCRYPT
#include "Openssl.h"
#endif

#include "DeviceController.h"

int onenet_init(DeviceController *dev_ctr);
void onenet_uninit(void);
void hexdump(const unsigned char *buf, uint32 num);
int32 Open(const uint8 *addr, int16 portno);
int32 DoSend(int32 sockfd, const char* buffer, uint32 len);
void *recv_thread_func(void* arg);
void *ping_thread_func(void* arg);

#endif /* ONENET_H_ */
