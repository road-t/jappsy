#ifndef uProxyThreadH
#define uProxyThreadH

#include <windows.h>
#include "core\uThreads.h"
#include "core\uSecureData.h"

extern struct tThread*		serverThread;
extern struct tThread*		serverThreadSSL;

extern volatile uint32_t* atomFlashPort;
extern volatile uint32_t* atomFlashSSLPort;

extern volatile void* atomSSLCRT;
extern volatile void* atomSSLKEY;
extern volatile void* atomSSLRootCA;
extern volatile void* atomSSLIntermediate1;
extern volatile void* atomSSLIntermediate2;
extern volatile uint32_t atomSSLCRTsize;
extern volatile uint32_t atomSSLKEYsize;
extern volatile uint32_t atomSSLRootCAsize;
extern volatile uint32_t atomSSLIntermediate1size;
extern volatile uint32_t atomSSLIntermediate2size;

struct tSocketInfo {
	atom_uint32_t ip;
	atom_uint64_t iMAC;
	atom_uint32_t ttl;
	atom_uint32_t ban;
	atom_uint32_t ddosTime;
	atom_uint32_t ddosCount;
	atom_uint32_t errCount;
};

#define SOCKET_INFO_LIST_SIZE		256
#define SOCKET_INFO_DDOS_MAX		5
#define SOCKET_INFO_ERR_MAX			3
#define SOCKET_INFO_BAN_TIME		(5*60*1000)

extern slLock* lckSocketInfoList;
extern volatile tSocketInfo* atomSocketInfoList;

bool socketInfoCheck(uint32_t ip, uint64_t iMAC, uint32_t timeCheck);

void thServerStart();
void thServerResume();
void thServerStop();

#endif
