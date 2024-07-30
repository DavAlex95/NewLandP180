#ifndef _TOOL_H_
#define _TOOL_H_

#include "lcomm.h"
#define COMM_UNUSED(x) (void)x
#define TCP_CONNECT_TIMEOUT	20*50
#define TCP_WRITE_TIMEOUT	20*50

#define MAX_PACK_SIZE 	8*2048
#define MAX_SEND_SIZE MAX_PACK_SIZE
#define MAX_RECV_SIZE MAX_SEND_SIZE

#define COMM_TCPIP_HEAD_LEN 2
#define STX     2
#define ETX     3

#define MAXWIFISSIDNUM 32

enum EM_HANGUPMODE
{
	HANGUP_SOCKET=0,	 				/**<Socket*/
	HANGUP_PPP,							/**<PPP*/
	HANGUP_RESET,						/**<SocketReset*/
};

typedef enum {
	EM_CONNECT = 0,  					/**<*/
	EM_PRECONNECT,	    				/**<*/
}EM_PRECONNECT_FALG;

typedef enum
{
	SSL_FORBIDDEN = 0,					/**<SSL*/
	SSL_SINGLE_AUTH = 1,				/**<SSL*/
	SSL_MUTUAL_AUTH = 2,				/**<SSL*/
}EM_SSLMODE;

/**
 *@brief LED control
*/
typedef enum {
        LED_RFID_RED_ON       = 0x01,  /**<Red on*/
        LED_RFID_RED_OFF      = 0x02,  /**<Red off*/
        LED_RFID_RED_FLICK    = 0x03,  /**<Red blink*/
        LED_RFID_YELLOW_ON    = 0x04,  /**<Yellow on*/
        LED_RFID_YELLOW_OFF   = 0x08,  /**<Yellow off*/
        LED_RFID_YELLOW_FLICK = 0x0c,  /**<Yellow blink*/
        LED_RFID_GREEN_ON     = 0x10,  /**<Green on*/
        LED_RFID_GREEN_OFF    = 0x20,  /**<Green off*/
        LED_RFID_GREEN_FLICK  = 0x30,  /**<Green blink*/
        LED_RFID_BLUE_ON      = 0x40,  /**<Blue on*/
        LED_RFID_BLUE_OFF     = 0x80,  /**<Blue off*/
        LED_RFID_BLUE_FLICK   = 0xc0,  /**<Blue blink*/
        LED_COM_ON            = 0x100, /**<Communication led on*/
        LED_COM_OFF           = 0x200, /**<Communication led off*/
        LED_COM_FLICK         = 0x300, /**<Communication led blink*/
        LED_ONL_ON            = 0x400, /**<Online led on*/
        LED_ONL_OFF           = 0x800, /**<Online led off*/
        LED_ONL_FLICK         = 0xc00  /**<Online led blink*/
} EM_LED;

extern int PubDebugSelectly(char cLevel, char* lpszFormat, ...);

#define KEEP_PPP_TIME	(1200)
#define LCP_PPP_KEEP    (0x03 << 4)  /**<PPP long link state defined in ndk.h*/

typedef struct
{
	char cLedComm;						/***/
	char cLedOnline;					/***/
}STLEDFLAG;// 01

extern int GetTimeDif(uint unFirstDatetime,uint unSecondDatetime);
extern int TcpOpen(int *pnHandle, char cSslFlag);
extern int TcpConnect(int nHandle, char cSslFlag, const char *pszRemoteIp, ushort usRemotePort, uint unTimeout);
extern int TcpGetStatus(int nHandle, int index);
extern int TcpWrite(int nHandle, char cSslFlag, const char *psData,int nDataLen);
extern int TcpRead(int nIsNonBlock, int nHandle, char cSslFlag, char *psOutData, uint nLen, uint *punReadLen, int nTimeOut);
extern int TcpHangUp(int *pnHandle, char cSslFlag, int nFlag);
extern int TcpGetHandle(int nHandle, int *pnDestHandle);
extern void SetLedComFlick();
extern void CommserverLedShine(EM_LED emStatus);
extern int ParseDnsIp(char cCommType, const STSERVERADDRESS *pstServerAddress, int *pnTimeout, char *pszIp, char cIsQuery);
extern void SetCommLedFlag();
#endif

