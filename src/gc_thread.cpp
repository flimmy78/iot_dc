// ------------------------------------------------------------------
// GW config socket program
// ------------------------------------------------------------------
// IoT program that handles the GW config from the LAN
// ------------------------------------------------------------------
// Author:    zhuangqs
// Copyright:
// ------------------------------------------------------------------

/** \addtogroup gc
 * \file
 * \section GW config socket program
 * \brief Program that handles the GW config from the LAN
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "atoi.h"
#include "jsonCreate.h"
#include "dump.h"
#include "gateway.h"
#include "newLog.h"
#include "DeviceController.h"
#include "dc_common.h"

#define DISCOVERY_PORT      2999

#define INPUTBUFFERLEN      256

// #define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MAIN_DEBUG */

// -------------------------------------------------------------
// Globals
// -------------------------------------------------------------

static int port = DISCOVERY_PORT;
static int running = 1;

static const char * response_succ="{\"msg\":\"success\",\"code\":\"0000\"}";
static const char * response_false="{\"msg\":\"fail\",\"code\":\"1002\"}";

// ------------------------------------------------------------------------
// Main
// ------------------------------------------------------------------------

/**
 * \brief GwDiscovery's main entry point: opens a UDP socket on port <port> and listens
 * for incoming broadcast messages. Reports hostname and API version to discovery clients.
 * \param argc Number of command-line parameters
 * \param argv Parameter list (-h = help, -p <port> = listen to UDP port <port>)
 */

bool gc_thread( void * argv ) {
	char home_ssid[16];
	char home_password[8];
	char userid[16];
	char * message = (char * )malloc(256);
	bool result = false;

    printf( "\n\nGW Discovery Started\n" );


    DEBUG_PRINTF("GW Discovery Started" );

    int                sock;
    struct sockaddr_in LAddr; // connector's address information
    struct sockaddr_in TargetAddr; // connector's address information
    int                NumBytes;
    unsigned char      buf[INPUTBUFFERLEN];
    int                Len;
    int                requestCount = 0;
    int                client_sock;

    if ( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) > 0 ) {
//        setsockopt( sock, SOL_SOCKET, SO_BROADCAST, &Opt, sizeof(Opt) );
//        ioctl(sock, FIONBIO, 1);
    
        LAddr.sin_family      = AF_INET;
        LAddr.sin_port        = htons( port );
        LAddr.sin_addr.s_addr = INADDR_ANY;
        bind( sock, (struct sockaddr *)&LAddr, sizeof(LAddr) );

        while( running ) {
                
            // Wait for incoming UDP discover packets
            memset( &TargetAddr, 0, sizeof( TargetAddr ) );
            TargetAddr.sin_family      = AF_INET;
            TargetAddr.sin_port        = htons( DISCOVERY_PORT );
            TargetAddr.sin_addr.s_addr = htonl( INADDR_BROADCAST );
    
            memset( buf, 0, sizeof( buf ) );
              

            listen(sock, 1);
            Len = sizeof( TargetAddr );
            client_sock = accept (sock, (struct sockaddr *)&TargetAddr, (socklen_t *)&Len);
            NumBytes = read(client_sock, buf, sizeof(buf));
              
            // buf contains identification data
            if ( NumBytes > 0 ) {
                // Get IP address out of data packet
                dump( (char *)buf, sizeof(buf) );

                // parse data
                char *param_str = strstr((char*)buf, "/?");
                if(param_str){
                    int count = sscanf((const char *)param_str,"%*[/?]SSID=%[^&]%*[&]PSW=%[^&]%*[&]USERID=%[^ ]",
                            				home_ssid,
                            				home_password,
                            				userid);
            		if(count == 3){
            			printf("gc: config success\r\n");
            			printf("home_ssid:%s\r\n", home_ssid);
            			printf("home_password:%s\r\n", home_password);
            			printf("home_userid:%s\r\n", userid);
            			sprintf(message,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s",
            					(int)strlen(response_succ),response_succ);

            			strcpy(g_systemcfg.home_ssid, home_ssid);
            			strcpy(g_systemcfg.home_password, home_password);
            			strcpy(g_systemcfg.userid, userid);
            			g_systemcfg.is_configured = true;
            			running = 0;
            			result = true;
            		}else{
            			printf("gc: config false\r\n");
            			sprintf(message,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s",
            					(int)strlen(response_false),response_false);
            			g_systemcfg.is_configured = false;
//            			running = 0;
            			result = false;
            		}
                }

                // Send response packet
                NumBytes = write(client_sock, message, strlen(message));

            }

            requestCount++;
            char * ip = (char *)&(TargetAddr.sin_addr);

            sprintf( logbuffer, "GW discovery client %d.%d.%d.%d (%d)",
                     ip[0], ip[1], ip[2], ip[3], requestCount );
            DEBUG_PRINTF("%s", logbuffer );
        }
        close(sock);
    } else {
        sprintf( logbuffer, "Error opening GW discovery socket %d", port );
        DEBUG_PRINTF( "%s",  logbuffer );
    }
    if(message)
    	free(message);
    return result;
}
