#include <windows.h>
#define FD_SETSIZE 64
#include <winsock.h>
#include <stdlib.h>
#include <stdint.h>

#include "uMainThread.h"
#include "uProxyThread.h"

#include "..\include\uCommand.h"

#include "core\cString.h"
#include "core\cObject.h"
#include "core\uMemory.h"
#include "core\cReadWrite.h"

#include <openssl/bio.h> // BIO objects for I/O
#include <openssl/ssl.h> // SSL and SSL_CTX for SSL connections
#include <openssl/err.h> // Error reporting

static uint32_t defaultFlashPort = 80;
volatile uint32_t* atomFlashPort = &defaultFlashPort;

static uint32_t defaultFlashSSLPort = 80;
volatile uint32_t* atomFlashSSLPort = &defaultFlashSSLPort;

volatile void* atomSSLCRT = 0;
volatile void* atomSSLKEY = 0;
volatile void* atomSSLRootCA = 0;
volatile void* atomSSLIntermediate1 = 0;
volatile void* atomSSLIntermediate2 = 0;
volatile uint32_t atomSSLCRTsize = 0;
volatile uint32_t atomSSLKEYsize = 0;
volatile uint32_t atomSSLRootCAsize = 0;
volatile uint32_t atomSSLIntermediate1size = 0;
volatile uint32_t atomSSLIntermediate2size = 0;

slLock* lckSocketInfoList = 0;
volatile tSocketInfo* atomSocketInfoList = 0;

//=======================================================

static struct {
	int loaded;
	void *handle;
	DWORD (WINAPI* SendARP)(struct in_addr DestIP, struct in_addr SrcIP, PULONG pMacAddr, PULONG PhyAddrLen);
} iphlpapi;

int initIPHLPAPI() {
	if ( iphlpapi.loaded == 0 ) {
        iphlpapi.handle = (void *)LoadLibraryW(L"iphlpapi.dll");
		if ( iphlpapi.handle == 0 ) {
			return -1;
		}
		bool funcFailed = false;
		do {
			iphlpapi.SendARP =
				(DWORD (WINAPI*)(struct in_addr, struct in_addr, PULONG, PULONG))
				GetProcAddress((HMODULE)iphlpapi.handle, "SendARP");
			if ( iphlpapi.SendARP == 0 ) {
				funcFailed = true; break;
			}
		} while (false);
		if (funcFailed) {
			FreeLibrary((HMODULE)iphlpapi.handle);
			return -1;
		}
	}
	++iphlpapi.loaded;
	return 0;
}

void quitIPHLPAPI() {
	if ( iphlpapi.loaded == 0 ) {
		return;
	}
	if ( iphlpapi.loaded == 1 ) {
		FreeLibrary((HMODULE)iphlpapi.handle);
	}
	--iphlpapi.loaded;
}

cString _GetMacAddress(int ip, uint64_t* pMac) {
	//if (pMac != 0) *pMac = 0xFFFFFFFFFFFFULL;
	//return cString("FF-FF-FF-FF-FF-FF");

    DWORD ret;
    unsigned char mac[6];
    struct in_addr srcip;
    struct in_addr destip;
    ULONG MacAddr[2];
    ULONG PhyAddrLen = 6;
    int i;

    srcip.s_addr=0;
    destip.S_un.S_addr=ip;

    ret = iphlpapi.SendARP(destip , srcip , MacAddr , &PhyAddrLen);

   	if (pMac != 0) *pMac = 0;

    if(PhyAddrLen) {
        BYTE *bMacAddr = (BYTE *) & MacAddr;
        for (i = 0; i < (int)PhyAddrLen; i++) {
            mac[i] = (char)bMacAddr[i];
        }
    } else {
    	if (ip == 0x0100007F) {
    		if (pMac != 0) *pMac = 0xFFFFFFFFFFFFULL;
			return cString("FF-FF-FF-FF-FF-FF");
    	}
    	return cString("00-00-00-00-00-00");
    }

    if (pMac != 0) {
		*(((uint8_t*)pMac)+0) = mac[0];
		*(((uint8_t*)pMac)+1) = mac[1];
		*(((uint8_t*)pMac)+2) = mac[2];
		*(((uint8_t*)pMac)+3) = mac[3];
		*(((uint8_t*)pMac)+4) = mac[4];
		*(((uint8_t*)pMac)+5) = mac[5];
    }

	return cString().sprintf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X", 18, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

struct mcInfo {
	uint64_t iMAC;
	uint32_t timeout;
};

slLock* slmc = 0;
mcInfo**** macCache = {0};

static inline uint64_t InterlockedCompareExchange64(volatile uint64_t *dest, uint64_t exch, uint64_t comp) {
	return __sync_val_compare_and_swap (dest, comp, exch);
}

static inline uint64_t InterlockedExchange64(volatile uint64_t *val, uint64_t new_val) {
	uint64_t old_val;
	do {
		old_val = *val;
	} while (InterlockedCompareExchange64 (val, new_val, old_val) != old_val);
	return old_val;
}

cString GetMacAddress(uint32_t ip, uint64_t* pMac, bool update) {
	cString result;
	uint64_t iMAC;
	uint32_t idx = ip;
	slSingleLock(slmc);
	mcInfo**** ptr1 = (mcInfo****)InterlockedCompareExchange((LONG*)&macCache, 0, 0);
	if (ptr1 == 0) {
		ptr1 = memAlloc(mcInfo***, ptr1, sizeof(mcInfo***) * 256);
		InterlockedExchange((LONG*)&macCache, (LONG)ptr1);
	}
	mcInfo*** ptr2 = (mcInfo***)InterlockedCompareExchange((LONG*)&(ptr1[idx & 0xFF]), 0, 0);
	if (ptr2 == 0) {
		ptr2 = memAlloc(mcInfo**, ptr2, sizeof(mcInfo**) * 256);
		InterlockedExchange((LONG*)&(ptr1[idx & 0xFF]), (LONG)ptr2);
	}
	idx >>= 8;
	mcInfo** ptr3 = (mcInfo**)InterlockedCompareExchange((LONG*)&(ptr2[idx & 0xFF]), 0, 0);
	if (ptr3 == 0) {
		ptr3 = memAlloc(mcInfo*, ptr3, sizeof(mcInfo*) * 256);
		InterlockedExchange((LONG*)&(ptr2[idx & 0xFF]), (LONG)ptr3);
	}
	idx >>= 8;
	mcInfo* ptr4 = (mcInfo*)InterlockedCompareExchange((LONG*)&(ptr3[idx & 0xFF]), 0, 0);
	if (ptr4 == 0) {
		ptr4 = memAlloc(mcInfo, ptr4, sizeof(mcInfo) * 256);
		InterlockedExchange((LONG*)&(ptr3[idx & 0xFF]), (LONG)ptr4);
	}
	idx >>= 8;
	mcInfo* info = &(ptr4[idx & 0xFF]);
	uint32_t timeout = InterlockedCompareExchange((LONG*)&(info->timeout), 0, 0);
	uint32_t timenow = GetTickCount();
	if ((update) || (timeout == 0) || (timeout <= timenow)) {
		result = _GetMacAddress(ip, &iMAC);
		InterlockedExchange((LONG*)&(info->timeout), GetTickCount() + 60000);
		InterlockedExchange64((uint64_t*)&(info->iMAC), iMAC);
		slSingleUnlock(slmc);
	} else {
		iMAC = InterlockedCompareExchange64((uint64_t*)&(info->iMAC), 0, 0);
		slSingleUnlock(slmc);
		unsigned char* mac = (uint8_t*)&iMAC;
		result = cString().sprintf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X", 18, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	if (pMac != 0) {
		*pMac = iMAC;
	}
	return result;
}

//=======================================================

bool socketInfoCheck(uint32_t ip, uint64_t iMAC, uint32_t timeCheck) {
	uint32_t now = GetTickCount();

	slSingleLock(lckSocketInfoList);

	tSocketInfo* ptr = (tSocketInfo*)InterlockedCompareExchange((long*)&(atomSocketInfoList), 0, 0);
	tSocketInfo* emptyPtr = 0;
	for (int i = SOCKET_INFO_LIST_SIZE - 1; i >= 0; i--) {
		if (ptr->iMAC == 0) {
			emptyPtr = ptr;
		} else if (int32_t(now - ptr->ttl) >= 0) {
			emptyPtr = ptr;
		} else if (ptr->iMAC == iMAC) {
			if (ptr->ban != 0) {
				slSingleUnlock(lckSocketInfoList);
				return false;
			} else if (timeCheck != 0) {
				ptr->ttl = now + SOCKET_INFO_BAN_TIME;
				if (int32_t(now - timeCheck - ptr->ddosTime) < 0) {
					ptr->ddosCount = ptr->ddosCount + 1;
					if (ptr->ddosCount >= SOCKET_INFO_DDOS_MAX) {
						ptr->ddosTime = now;
						ptr->ban = 1;
						slSingleUnlock(lckSocketInfoList);
						return false;
					}
				} else if (ptr->ddosCount > 0) {
					ptr->ddosCount = ptr->ddosCount - 1;
				}
				ptr->ddosTime = now;
				slSingleUnlock(lckSocketInfoList);
				return true;
			} else {
				ptr->ttl = now + SOCKET_INFO_BAN_TIME;
				slSingleUnlock(lckSocketInfoList);
				return true;
			}
		}

		ptr++;
	}

	if (emptyPtr == 0) {
		slSingleUnlock(lckSocketInfoList);
		return false;
	}

	emptyPtr->ip = ip;
	emptyPtr->iMAC = iMAC;
	emptyPtr->ttl = now + SOCKET_INFO_BAN_TIME;
	emptyPtr->ban = 0;
	emptyPtr->ddosTime = now;
	emptyPtr->ddosCount = 0;
	emptyPtr->errCount = 0;

	slSingleUnlock(lckSocketInfoList);

	return true;
}

bool socketInfoError(uint32_t ip, uint64_t iMAC, bool reset) {
	uint32_t now = GetTickCount();

	slSingleLock(lckSocketInfoList);

	tSocketInfo* ptr = (tSocketInfo*)InterlockedCompareExchange((long*)&(atomSocketInfoList), 0, 0);
	tSocketInfo* emptyPtr = 0;
	for (int i = SOCKET_INFO_LIST_SIZE - 1; i >= 0; i--) {
		if (ptr->iMAC == 0) {
			emptyPtr = ptr;
		} else if (int32_t(now - ptr->ttl) >= 0) {
			emptyPtr = ptr;
		} else if (ptr->iMAC == iMAC) {
			if (ptr->ban != 0) {
				slSingleUnlock(lckSocketInfoList);
				return false;
			} else {
				ptr->ttl = now + SOCKET_INFO_BAN_TIME;
				if (!reset) {
					ptr->errCount = ptr->errCount + 1;
					if (ptr->errCount >= SOCKET_INFO_ERR_MAX) {
						ptr->ban = 1;
						slSingleUnlock(lckSocketInfoList);
						return false;
					}
				} else {
					ptr->errCount = 0;
				}
				slSingleUnlock(lckSocketInfoList);
				return true;
			}
		}

		ptr++;
	}

	if (emptyPtr == 0) {
		slSingleUnlock(lckSocketInfoList);
		return false;
	}

	emptyPtr->ip = ip;
	emptyPtr->iMAC = iMAC;
	emptyPtr->ttl = now + SOCKET_INFO_BAN_TIME;
	emptyPtr->ban = 0;
	emptyPtr->ddosTime = now;
	emptyPtr->ddosCount = 0;
	if (reset) {
		emptyPtr->errCount = 0;
	} else {
		emptyPtr->errCount = 1;
	}

	slSingleUnlock(lckSocketInfoList);

	return true;
}

struct tThread*		serverThread;
struct tThread*		serverThreadSSL;

void makeValidQuery(cString& query) {
	int len = query.length();
	if ((len > 0) && (query.lpwcData != 0)) {
		int i = 0;
		wchar_t* ptr = query.lpwcData;
		while (i < len) {
			wchar_t c = *ptr;
			if (c == L'%') {
				if ((i + 2) < len) {
					c = ptr[1];
					if (((c >= L'A') && (c <= L'F')) || ((c >= L'a') && (c <= L'f')) || ((c >= L'0') || (c <= L'9'))) {
						c = ptr[2];
						if (((c >= L'A') && (c <= L'F')) || ((c >= L'a') && (c <= L'f')) || ((c >= L'0') || (c <= L'9'))) {
							ptr++; i++; continue;
						}
					}

					ptr[0] = L'+';
					ptr[1] = L'+';
					ptr[2] = L'+';
					ptr += 3;
					i += 3;
					continue;
				} else {
					int j = 0;
					while ((j < 3) && (i < len)) {
						*ptr = '+';
						ptr++; i++; j++;
					}
					break;
				}
			} else if (
				// Reserved
				(c == L';') || (c == L'/') || (c == L'?') || (c == L':') || (c == L'@') || (c == L'&') || (c == L'=') || (c == L'+') || (c == L'$') || (c == L',') ||
				// Unreserved
					// AlphaNum
				((c >= L'A') && (c <= L'Z')) || ((c >= L'a') && (c <= L'z')) || ((c >= L'0') || (c <= L'9')) ||
					// Mark
				(c == L'-') || (c == L'_') || (c == L'.') || (c == L'!') || (c == L'~') || (c == L'*') || (c == L'\'') || (c == L'(') || (c == L')')
			) {
				ptr++; i++; continue;
			}

			*ptr = L'+';
			ptr++; i++;
		}
	}
}

int selectCheckIP(uint32_t ip);
char* httpGet(const cString& request, int post, int* len, uint32_t ip, bool ssl) {
	uint32_t update_ip = 0;
	cString sMAC = L"";
	cString userAgent = L"Unknown";
	cString host = L"127.0.0.1";
	bool acceptGzip = false;

	cStrings req; req.explode(request, L"\r\n");
	req.trim();
	for (int i = req.count()-1; i >= 0; i--) {
		if (!req[i].isEmpty()) {
			int pos = req[i].pos(L": ");
			if (pos > 0) {
				cString key = req[i].subString(0, pos);
				cString value = req[i].subString(pos + 2, req[i].length() - pos - 2);

				if ((ip != 0x0100007F) && (key.like(L"Accept-Encoding")) && (value.like(L"*gzip*"))) {
					acceptGzip = true;
				} else if (key.like(L"User-Agent")) {
					userAgent = value;
				} else if (key.like(L"Host")) {
					host = value;
				} else if ((ip == 0x0100007F) && (key.like(L"X-Real-IP"))) {
					char* sip = value.toChar();
					if (sip != 0) {
						update_ip = inet_addr(sip);
						memFree(sip);
						acceptGzip = false;
					}
				}
			}
		}
	}

	// TODO: Отключаем сжатие, VirtualAlloc не может выделить память
	acceptGzip = false;

	long posStart = request.pos(L"/");
	long posEnd = request.pos(L" HTTP/1.", posStart);

	cObject* result;

	if ((posStart > 0) && (posEnd > 0)) {
		cString get = request.subString(posStart, posEnd - posStart);

		long pos = get.pos(L"?");

		cObject query;
		cString script = get.subString(0, pos);
		cObject* oGet = 0;
		cObject* oPost = 0;

		// Scripts To Update MAC
		bool update_mac = false;
		if ((script.like(L"/session.cgi")) ||
			(script.like(L"/command.js"))
		) {
			update_mac = true;
		}

		uint64_t iMAC = 0;
		cString* mac = 0;
		if (update_ip != 0) {
			sMAC = GetMacAddress(update_ip, &iMAC, update_mac);
			mac = &sMAC;

			if ((update_ip != 0x0100007F) && (iMAC == 0)) {
				return 0;
			} else if ((update_ip != 0x0100007F) && (!socketInfoCheck(update_ip, iMAC, 0))) {
				return 0;
			} else if ((ip != 0x0100007F) && (selectCheckIP(update_ip) != 0)) {
				return 0;
			}

			ip = update_ip;
		} else {
			sMAC = GetMacAddress(ip, &iMAC, update_mac);
			mac = &sMAC;

			if ((ip != 0x0100007F) && (iMAC == 0)) {
				return 0;
			} else if ((ip != 0x0100007F) && (!socketInfoCheck(ip, iMAC, 0))) {
				return 0;
			}
		}

		cString qini = L"";

		if (pos >= 0) {
			cString squery = get.subString(pos + 1, -1);
			if (squery.length() > 0) {
				makeValidQuery(squery);
				cStrings tmp; tmp.explode(squery, L"&");
				tmp.trim();
				for (int i = tmp.count()-1; i >= 0; i--) {
					if (tmp[i].isEmpty()) {
						tmp.unset(i);
					} else if (tmp[i].pos(L"=") < 0) {
						tmp[i] += L"=";
					}
				}
				squery = tmp.implode(L"&");
				if (squery.length() > 0) {
					squery += L"\r\n";
					squery.replace(L"\"", L"\\\"");
					squery.replace(L"&", L"\r\n");
					squery.replace(L"+", L" ");
					squery.replace(L"=", L"=\"");
					squery.replace(L"\r\n", L"\"\r\n");

					qini += L"[GET]\r\n";
					qini += squery;
				}
			}
		}

		if (post > 0) {
			cString squery = request.subString(post, -1);
			if (squery.length() > 0) {
				makeValidQuery(squery);
				cStrings tmp; tmp.explode(squery, L"&");
				tmp.trim();
				for (int i = tmp.count()-1; i >= 0; i--) {
					if (tmp[i].isEmpty()) {
						tmp.unset(i);
					} else if (tmp[i].pos(L"=") < 0) {
						tmp[i] += L"=";
					}
				}
				squery = tmp.implode(L"&");
				if (squery.length() > 0) {
					squery += L"\r\n";
					squery.replace(L"\"", L"\\\"");
					squery.replace(L"&", L"\r\n");
					squery.replace(L"+", L" ");
					squery.replace(L"=", L"=\"");
					squery.replace(L"\r\n", L"\"\r\n");

					qini += L"[POST]\r\n";
					qini += squery;
				}
			}
		}

		if (qini.length() > 0) {
			memCall( query.loadFromString(qini); )

			if (query.type == otArray) {
				cObject* oGet = query.isChild(L"GET");
				if ((oGet != 0) && (oGet->type != otArray)) {
					oGet = 0;
				}

				cObject* oPost = query.isChild(L"POST");
				if ((oPost != 0) && (oPost->type != otArray)) {
					oPost = 0;
				}

				result = httpCommand(&script, oGet, oPost, &userAgent, &host, acceptGzip, ip, iMAC, mac, ssl);
			} else {
				result = httpCommand(&script, 0, 0, &userAgent, &host, acceptGzip, ip, iMAC, mac, ssl);
			}
		} else {
			result = httpCommand(&script, 0, 0, &userAgent, &host, acceptGzip, ip, iMAC, mac, ssl);
		}
	} else {
		result = httpErrorCommand();
	}

	char *tmp = 0;

	if (result != 0) {
		if (result->type == otText) {
			tmp = result->lpsText->toChar();
			if (tmp) {
				*len = strlen(tmp);
			}
		} else if (result->type == otPointer) {
			tmp = memAlloc(char,tmp,result->dwMemSize);
			if (tmp) {
				memcpy(tmp, result->lpData, result->dwMemSize);
				*len = result->dwMemSize;
			}
		}

		memDelete(result);
	}

	return tmp;
}

static int iMaxUdpDg = 1024;

bool startNetwork() {
    static bool mStartNetwork = false;

    if (!mStartNetwork) {
        WORD wVersionRequested;
        WSADATA wsaData;
        wVersionRequested = MAKEWORD( 1, 1 );
        if ( WSAStartup( wVersionRequested, &wsaData ) == 0 ) {
            mStartNetwork = true;
            if (wsaData.iMaxUdpDg > 0) {
                iMaxUdpDg = wsaData.iMaxUdpDg;
            }
        }
    }
    return mStartNetwork;
}

int setTCPSocketOptions(SOCKET s) {
    int rc, on = 1, rcvbuf = 8192, sndbuf = 8192, buf, bufsize;

	bufsize = sizeof(buf);
	rc = getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&buf, &bufsize);
    if (rc < 0) {
		return rc;
    }
    //Sleep(0);
    rc = setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, sizeof(rcvbuf));
    if (rc < 0) {
		return rc;
    }
	bufsize = sizeof(buf);
	rc = getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&buf, &bufsize);
    if (rc < 0) {
		return rc;
    }
    //Sleep(0);

	bufsize = sizeof(buf);
	rc = getsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&buf, &bufsize);
    if (rc < 0) {
		return rc;
    }
    //Sleep(0);
    rc = setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, sizeof(sndbuf));
    if (rc < 0) {
		return rc;
    }
	bufsize = sizeof(buf);
	rc = getsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&buf, &bufsize);
    if (rc < 0) {
		return rc;
    }
    //Sleep(0);

	rc = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
    if (rc < 0) {
		return rc;
    }

	return rc;
}

int closesocket_h(SOCKET s) {
	int rc;

	linger l;
	l.l_onoff = 1;
	l.l_linger = 0;

	rc = setsockopt(s, SOL_SOCKET, SO_LINGER, (char *)&l, sizeof(l));
	rc = closesocket(s);

	return rc;
}

SOCKET createSimpleTCPSocket() {
    int rc, on = 1;

    if (!startNetwork()) {
        return INVALID_SOCKET;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        return INVALID_SOCKET;
    }

    rc = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0) {
        closesocket_h(s);
        return INVALID_SOCKET;
    }

	rc = setTCPSocketOptions(s);
	if (rc < 0) {
        closesocket_h(s);
        return INVALID_SOCKET;
	}

    rc = ioctlsocket(s, FIONBIO, (u_long *)&on);
    if (rc < 0) {
        closesocket_h(s);
        return INVALID_SOCKET;
    }

    return s;
}

SOCKET sServer = INVALID_SOCKET;
SOCKET sFlash = INVALID_SOCKET;
sockaddr_in saHost = {0};
fd_set fdListen;
fd_set fdMaster;
cObject* oSockets = 0;
fd_set fdFlashListen;
fd_set fdFlash;
cObject* oFlash = 0;

bool selectStart() {
	if (sServer != INVALID_SOCKET) {
		return true;
	}

	sServer = createSimpleTCPSocket();
	sFlash = createSimpleTCPSocket();

	if (sServer != INVALID_SOCKET) {
		FD_ZERO(&fdMaster);
		FD_ZERO(&fdListen);
		FD_ZERO(&fdFlashListen);
		FD_ZERO(&fdFlash);

		oSockets = memNew(oSockets, cObject());
		oFlash = memNew(oFlash, cObject());

		ZeroMemory(&saHost, sizeof(sockaddr_in));
		saHost.sin_family = AF_INET;

//#ifdef DEBUG
        saHost.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("192.168.3.5");//INADDR_ANY;
//#else
//        saHost.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
//#endif
		// Напоминание по флешу (сервер выдающий флеш должен еще работать и на 843 порту http://www.adobe.com/devnet/flashplayer/articles/socket_policy_files.html)
        //saHost.sin_port = htons(atoi("80"));
        saHost.sin_port = htons(InterlockedCompareExchange((long*)atomFlashPort, 0, 0));

        int rc = bind(sServer, (const sockaddr*)&saHost, sizeof(sockaddr_in));
        if (rc >= 0) {
            rc = listen(sServer, 32);
            if (rc >= 0) {
				FD_SET(sServer, &fdMaster);
				FD_SET(sServer, &fdListen);

				if (sFlash != INVALID_SOCKET) {
					saHost.sin_port = htons(atoi("843"));
					rc = bind(sFlash, (const sockaddr*)&saHost, sizeof(sockaddr_in));
					if (rc >= 0) {
						rc = listen(sFlash, 32);
						if (rc >= 0) {
							FD_SET(sFlash, &fdMaster);
							FD_SET(sFlash, &fdFlashListen);
						} else {
							closesocket_h(sFlash);
							sFlash = INVALID_SOCKET;
						}
					} else {
						closesocket_h(sFlash);
						sFlash = INVALID_SOCKET;
					}
				}

#ifdef LOG_ENABLE
    gameLog(L"Socket", L"Done");
#endif
				return true;
            }
        }

		if (sFlash != INVALID_SOCKET) {
			closesocket_h(sFlash);
			sFlash = INVALID_SOCKET;
		}
        closesocket_h(sServer);
        sServer = INVALID_SOCKET;
	}

	return false;
}

void selectStop() {
	if (sServer != INVALID_SOCKET) {
		memDelete(oFlash);
		memDelete(oSockets);

		if (sFlash != INVALID_SOCKET) {
			closesocket(sFlash);
			sFlash = INVALID_SOCKET;
		}
        closesocket(sServer);
        sServer = INVALID_SOCKET;
    }
}

char selectBuffer[16384];

bool checkIp(uint32_t v1, uint32_t v2) {
	uint8_t ip1[4]; *((uint32_t*)ip1) = v1;
	uint8_t ip2[4]; *((uint32_t*)ip2) = v2;
	for (int i = 0; i < 4; i++) {
		if ((ip1[i] != 255) && (ip1[i] != ip2[i])) {
			return false;
		}
	}
	return true;
}

int selectCheckIP(uint32_t ip) {
	if (ip == 0x0100007F) {
		return 1;
	} else if (mIS2(MODE2_ENABLE_REMOTE)) {
		return 1; // 0 - only http, 1 - http and https
	}

	return -1;
}

bool selectLoop() {
    int rc;
    timeval timeout;
    sockaddr_in sa;
    int addr_len;
    SOCKET ws;

    timeout.tv_sec = 5; // Устанавливаем таймаут 30 секунд
    timeout.tv_usec = 0;

    fd_set fdWorking;        // Рабочие сокеты

	CopyMemory(&fdWorking, &fdMaster, sizeof(fd_set));

	rc = select(0, &fdWorking, 0, 0, &timeout);

    if (rc < 0) {
		// Ошибка сокетов
		return false;
    } else if (rc == 0) {
    	// timeout
    } else {
    	for (int i = fdWorking.fd_count-1; i >= 0; i--) {
			ws = fdWorking.fd_array[i];
    		if (FD_ISSET(ws,&fdListen)) { // Прослушиваемый сокет принимает соединение
    			addr_len = sizeof(sockaddr_in);
    			SOCKET s = accept(ws, (struct sockaddr*)&sa, &addr_len);
				int ip = sa.sin_addr.S_un.S_addr;
				//uint64_t iMAC;
				//cString sMAC = GetMacAddress(ip, &iMAC, false);
				//if ((ip != 0x0100007F) && (iMAC == 0)) {
				//	closesocket_h(s);
				//} else if ((ip != 0x0100007F) && (!socketInfoCheck(ip, iMAC, 0))) {
				//	closesocket_h(s);
				//} else
				if ((ip == 0x0100007F) || (selectCheckIP(ip) >= 0)) {
					if (setTCPSocketOptions(s) < 0) {
						closesocket_h(s);
					} else {
						FD_SET(s, &fdMaster);
						cObject* oSocket;
						memCallRet(oSocket, oSockets->getChild((int)s); )
						memCall( oSocket->getChild("Data")->setText(sNULL); )
						memCall( oSocket->getChild("IP")->setNumber(ip); )
						memCall( oSocket->getChild("Post")->setNumber(0); )
					}
				} else {
					closesocket_h(s);
				}
    		} else if (FD_ISSET(ws,&fdFlashListen)) {
    			addr_len = sizeof(sockaddr_in);
    			SOCKET s = accept(ws, (struct sockaddr*)&sa, &addr_len);

				if (setTCPSocketOptions(s) < 0) {
					closesocket_h(s);
				} else {
					FD_SET(s, &fdMaster);
					FD_SET(s, &fdFlash);
					memCall( oFlash->getChild((int)s)->setText(sNULL); )
				}
    		} else { // Соединение принимает данные
    			rc = recv(ws, selectBuffer, sizeof(selectBuffer) - 1, 0);

    			if (rc < 0) { // Ошибка получаемых данных?
    				rc = WSAGetLastError();
					if (rc != WSAEWOULDBLOCK) {
						closesocket_h(ws);

						FD_CLR(ws, &fdMaster);
						if (FD_ISSET(ws, &fdFlash)) {
							FD_CLR(ws, &fdFlash);
							memDelete(oFlash->getChild((int)ws));
						} else {
							memDelete(oSockets->getChild((int)ws));
						}
					}
    			} else if (rc == 0) { // Соединение оборванно клиентом
    				rc = WSAGetLastError();

					closesocket_h(ws);

    				FD_CLR(ws, &fdMaster);
					if (FD_ISSET(ws, &fdFlash)) {
						FD_CLR(ws, &fdFlash);
						memDelete(oFlash->getChild((int)ws));
					} else {
						memDelete(oSockets->getChild((int)ws));
					}
				} else if (FD_ISSET(ws, &fdFlash)) {
    				cObject* o = oFlash->isChild((int)(ws));
    				if ((o != 0) && (o->type == otText)) {
						selectBuffer[rc] = 0;
						*(o->lpsText) += selectBuffer;

						closesocket_h(ws);
						FD_CLR(ws, &fdMaster);
						FD_CLR(ws, &fdFlash);
						memDelete(oFlash->getChild((int)ws));
    				}
    			} else {
    				cObject* oSocket = oSockets->isChild((int)(ws));
    				cObject* oData = 0;
    				cObject* oIP = 0;
    				cObject* oPost = 0;
    				if (oSocket != 0) {
						oData = oSocket->isChild("Data");
						oIP = oSocket->isChild("IP");
						oPost = oSocket->isChild("Post");
    				}
    				if ((oData != 0) && (oData->type == otText)) {
						selectBuffer[rc] = 0;
						*(oData->lpsText) += selectBuffer;

						long queryEnd = oData->lpsText->pos(L"\r\n\r\n");
						if (queryEnd >= 0) {
							int len = 0;
							char* response = 0;
							bool gotData = false;
							bool closeConn = false;
							bool closeHard = true;
							if (oData->lpsText->like(L"GET /* HTTP/1.?\r\n*")) {
								response = httpGet(*(oData->lpsText), 0, &len, oIP->toInt(), false);
								gotData = true;
							} else if (oData->lpsText->like(L"POST /* HTTP/1.?\r\n*")) {
								int postSize = oPost->toInt();
								if (postSize == 0) {
									if ((oData->lpsText->pos(L"Content-Type: application/x-www-form-urlencoded\r\n") > 0) ||
										(oData->lpsText->pos(L"Content-Type: application/x-www-form-urlencoded;") > 0))
									{
										long posStart = oData->lpsText->pos(L"Content-Length: ");
										long posEnd = oData->lpsText->pos(L"\r\n", posStart);

										if ((posStart > 0) && (posEnd > 0)) {
											postSize = (int)(oData->lpsText->subString(posStart + 16, posEnd - posStart - 16));
										} else {
											postSize = -1;
										}
									} else {
										postSize = -1;
									}
									oPost->setNumber(postSize);
								}
								if (postSize < 0) {
									closeConn = true;
								} else if (postSize > 0) {
									if ((oData->lpsText->strLength - queryEnd - 4) == postSize) {
										//cString post = oData->lpsText->subString(queryEnd + 4, oData->lpsText->strLength - queryEnd - 4);
										response = httpGet(*(oData->lpsText), queryEnd + 4, &len, oIP->toInt(), false);
										gotData = true;
									}
								}
							} else {
								closeConn = true;
							}

							if (gotData) {
								if (response) {
									rc = send(ws, response, len, 0);

									memFree(response);

									if (rc < 0) { // Ошибка передаваемых данных?
										rc = WSAGetLastError();
										if (rc != WSAEWOULDBLOCK) {
											closeConn = true;
										}
									} else if (rc == 0) { // Соединение оборванно
										rc = WSAGetLastError();
										closeConn = true;
									} else {
										closeConn = true;
										closeHard = false;
									}
								} else {
									closeConn = true;
								}
							}

							if (closeConn) {
								// Левые данные
								if (closeHard) {
									closesocket_h(ws);
								} else {
									closesocket(ws);
								}

								FD_CLR(ws, &fdMaster);
								memDelete(oSockets->getChild((int)ws));

							}

						}
    				}

    			}
    		}
    	}
    }

	return true;
}

uint32_t tNetworkSelectThreadProc(struct tThread* thread) {

	while (WaitForSingleObject(evThreadExitRequest, 0) != WAIT_OBJECT_0) {
		if (selectStart()) {
			if (!selectLoop()) {
				selectStop();
			}
		} else {
			Sleep(1000);
		}
	}

	selectStop();

#ifdef LOG_ENABLE
    gameLog(L"Socket", L"Exit");
#endif

    return 0;
}

uint32_t tNetworkSelectKillThreadProc(struct tThread* thread) {
    WSACancelBlockingCall();

	selectStop();

    return 0;
}

/// SSL SERVER

SOCKET sServerSSL = INVALID_SOCKET;
sockaddr_in saHostSSL = {0};
fd_set fdListenSSL;
fd_set fdMasterSSL;
fd_set fdWriteSSL;
cObject* oSocketsSSL = 0;

SSL_CTX *ctx;

#define PASSWORD "certificate_password_here"

int password_callback(char* buffer, int num, int rwflag, void* userdata) {
    if (num < (strlen(PASSWORD) + 1)) {
		return 0;
    }
    strcpy(buffer, PASSWORD);
    return strlen(PASSWORD);
}

int verify_callback(int ok, X509_STORE_CTX* store) {
	char data[255];

	if (!ok) {
		X509* cert = X509_STORE_CTX_get_current_cert(store);
		int depth = X509_STORE_CTX_get_error_depth(store);
		int err = X509_STORE_CTX_get_error(store);

		//printf("Error with certificate at depth: %d!\n", depth);
		X509_NAME_oneline(X509_get_issuer_name(cert), data, 255);
		//printf("\tIssuer: %s\n", data);
		X509_NAME_oneline(X509_get_subject_name(cert), data, 255);
		//printf("\tSubject: %s\n", data);
		//printf("\tError %d: %s\n", err, X509_verify_cert_error_string(err));
	}

	return ok;
}

void saveDER(wchar_t* fileName, void* buffer, int size) {
	cReadWrite* file = memNew(file, cReadWrite());
	if (file->openFile(fileName,O_RDWR | O_BINARY | O_CREAT | O_TRUNC)) {
		file->write((unsigned char*)buffer,size);
		file->close(false);
	}
	memDelete(file);
}

bool selectStartSSL() {
	int SSLCRTsize;
	int SSLKEYsize;
	int SSLRootCAsize;
	int SSLIntermediate1size;
	int SSLIntermediate2size;
	void* SSLCRT;
	void* SSLKEY;
	void* SSLRootCA;
	void* SSLIntermediate1;
	void* SSLIntermediate2;

	if (sServerSSL != INVALID_SOCKET) {
		return true;
	}

	SSLCRTsize = InterlockedCompareExchange((long*)(&atomSSLCRTsize), 0, 0);
	SSLKEYsize = InterlockedCompareExchange((long*)(&atomSSLKEYsize), 0, 0);
	SSLRootCAsize = InterlockedCompareExchange((long*)(&atomSSLRootCAsize), 0, 0);
	SSLIntermediate1size = InterlockedCompareExchange((long*)(&atomSSLIntermediate1size), 0, 0);
	SSLIntermediate2size = InterlockedCompareExchange((long*)(&atomSSLIntermediate2size), 0, 0);

	if ((SSLCRTsize == 0) || (SSLKEYsize == 0) || (atomSSLRootCAsize == 0) || (atomSSLIntermediate1size == 0) || (atomSSLIntermediate2size == 0)) {
		return false;
	}

	SSLCRT = (void*)(InterlockedCompareExchange((long*)(&atomSSLCRT), 0, 0));
	SSLKEY = (void*)(InterlockedCompareExchange((long*)(&atomSSLKEY), 0, 0));
	SSLRootCA = (void*)(InterlockedCompareExchange((long*)(&atomSSLRootCA), 0, 0));
	SSLIntermediate1 = (void*)(InterlockedCompareExchange((long*)(&atomSSLIntermediate1), 0, 0));
	SSLIntermediate2 = (void*)(InterlockedCompareExchange((long*)(&atomSSLIntermediate2), 0, 0));

	if ((SSLCRT == 0) || (SSLKEY == 0) || (SSLRootCA == 0) || (SSLIntermediate1 == 0) || (SSLIntermediate2 == 0)) {
		return false;
	}

	void* ptr;
	X509* cert_rootca = 0; ptr = SSLRootCA;
	cert_rootca = d2i_X509(0, (const unsigned char**)&ptr, SSLRootCAsize);
	X509* cert_intermediate1 = 0; ptr = SSLIntermediate1;
	cert_intermediate1 = d2i_X509(0, (const unsigned char**)&ptr, SSLIntermediate1size);
	X509* cert_intermediate2 = 0; ptr = SSLIntermediate2;
	cert_intermediate2 = d2i_X509(0, (const unsigned char**)&ptr, SSLIntermediate2size);

#ifdef DEBUG
	saveDER(L"./SSL/test.crt", SSLCRT, SSLCRTsize);
	saveDER(L"./SSL/test.key", SSLKEY, SSLKEYsize);
#endif

	sServerSSL = createSimpleTCPSocket();

#ifdef LOG_ENABLE
    gameLog(L"SSL",cString().swprintf(L"sServerSSL = %d", 1024, sServerSSL));
#endif
	if (sServerSSL != INVALID_SOCKET) {
		FD_ZERO(&fdMasterSSL);
		FD_ZERO(&fdListenSSL);
		FD_ZERO(&fdWriteSSL);

		oSocketsSSL = memNew(oSocketsSSL, cObject());

		ZeroMemory(&saHostSSL, sizeof(sockaddr_in));
		saHostSSL.sin_family = AF_INET;

        saHostSSL.sin_addr.S_un.S_addr = INADDR_ANY;
		// Напоминание по флешу (сервер выдающий флеш должен еще работать и на 843 порту http://www.adobe.com/devnet/flashplayer/articles/socket_policy_files.html)
        //saHostSSL.sin_port = htons(atoi("443"));
        int port = InterlockedCompareExchange((long*)atomFlashSSLPort, 0, 0);
        saHostSSL.sin_port = htons(port);

#ifdef LOG_ENABLE
    gameLog(L"SSL",cString().swprintf(L"bind(%08X : %d)", 1024, saHostSSL.sin_addr.S_un.S_addr, port));
#endif
        int rc = bind(sServerSSL, (const sockaddr*)&saHostSSL, sizeof(sockaddr_in));
        if (rc >= 0) {
#ifdef LOG_ENABLE
    gameLog(L"SSL",cString().swprintf(L"listen(32)", 1024));
#endif
            rc = listen(sServerSSL, 32);
            if (rc >= 0) {
				FD_SET(sServerSSL, &fdMasterSSL);
				FD_SET(sServerSSL, &fdListenSSL);

				/// OpenSSL Init
				do {
					ctx = SSL_CTX_new(SSLv23_server_method());

#ifdef LOG_ENABLE
    gameLog(L"SSL",cString().swprintf(L"SSL_CTX_new = 0x%08X", 1024, (uint32_t)ctx));
#endif
					//SSL_CTX_ctrl(ctx, SSL_CTRL_SET_SESS_CACHE_MODE, SSL_SESS_CACHE_OFF, 0);

					//C:\OpenSSL\bin>openssl x509 -inform PEM -in terminal.pem -outform DER -out cert.der
					//C:\OpenSSL\bin>openssl rsa -inform PEM -in terminal.key -outform DER -out key.der

					SSL_CTX_set_default_passwd_cb(ctx, password_callback);
					//SSL_CTX_use_certificate_file(ctx, "./SSL/terminal.pem", SSL_FILETYPE_PEM);
					//rc = SSL_CTX_use_certificate_file(ctx, "./SSL/cert.der", SSL_FILETYPE_ASN1);
					SSL_CTX_use_certificate_ASN1(ctx, SSLCRTsize, (unsigned char*)SSLCRT);

					if (cert_intermediate1 != 0) {
						X509_STORE_add_cert(SSL_CTX_get_cert_store(ctx), cert_intermediate1);
						SSL_CTX_add_extra_chain_cert(ctx, cert_intermediate1);
					}
					if (cert_intermediate2 != 0) {
						X509_STORE_add_cert(SSL_CTX_get_cert_store(ctx), cert_intermediate2);
						SSL_CTX_add_extra_chain_cert(ctx, cert_intermediate2);
					}

					//SSL_CTX_use_PrivateKey_file(ctx, "./SSL/terminal.key", SSL_FILETYPE_PEM);
					//rc = SSL_CTX_use_PrivateKey_file(ctx, "./SSL/key.der", SSL_FILETYPE_ASN1);
					SSL_CTX_use_RSAPrivateKey_ASN1(ctx, (unsigned char*)SSLKEY, SSLKEYsize);

					if (cert_rootca != 0) {
						X509_STORE_add_cert(SSL_CTX_get_cert_store(ctx), cert_rootca);
						SSL_CTX_add_extra_chain_cert(ctx, cert_rootca);
					}

					// Попробовать это разкоментировать с реальным сертификатом
					//SSL_CTX_load_verify_locations(ctx, "rootcas.pem", 0);
					if (cert_rootca != 0) {
						SSL_CTX_set_verify_depth(ctx, 9);
						SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, verify_callback);
						//SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_callback);
					}
					//RAND_load_file("random.pem", 1024 << 10);
					//

					/*
					BIO* bio = BIO_new_file("./SSL/dh1024.pem", "r");
					if (bio == 0) break;
					DH* ret = PEM_read_bio_DHparams(bio, 0, 0, 0);
					BIO_free(bio);
					if (SSL_CTX_set_tmp_dh(ctx, ret) < 0) break;
					*/

					RSA* rsa = RSA_generate_key(1024, RSA_F4, 0, 0);
					SSL_CTX_set_tmp_rsa(ctx, rsa);
					RSA_free(rsa);

					SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"); // ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH

#ifdef LOG_ENABLE
    gameLog(L"SSL",cString().swprintf(L"Done", 1024, (uint32_t)ctx));
#endif
					return true;

				} while (false);
            }
        }

        closesocket_h(sServerSSL);
        sServerSSL = INVALID_SOCKET;
	}

	return false;
}

void selectStopSSL() {
	if (sServerSSL != INVALID_SOCKET) {
		memDelete(oSocketsSSL);

		SSL_CTX_free(ctx);

        closesocket(sServerSSL);
        sServerSSL = INVALID_SOCKET;
    }
}

char selectBufferSSL[16384];

#define SSL_QUERY_NONE		0
#define SSL_QUERY_ACCEPT	1
#define SSL_QUERY_READ		2
#define SSL_QUERY_WRITE		3

int selectErrorSSL(SSL* ssl, int ret_code) {
	int rc = SSL_get_error(ssl, ret_code);

	if ((rc != SSL_ERROR_WANT_READ) && (rc != SSL_ERROR_WANT_WRITE)) {
		if (rc == SSL_ERROR_ZERO_RETURN) {
			rc = 0; // DISCONNECT
		} else if (rc == SSL_ERROR_SYSCALL) {
			int err = ERR_get_error();
			if (err == 0) {
				if (rc == 0) {
					// EOF
					rc = -1; // DISCONNECT ON ERROR
				} else {
					err = WSAGetLastError();
					if (err == WSAECONNABORTED) {
						rc = -2; // DISCONNECT (NO FREE)
					} else {
						rc = -1; // DISCONNECT ON ERROR
					}
				}
			} else {
				rc = -1; // DISCONNECT ON ERROR
			}
		} else {
			rc = -1; // DISCONNECT ON ERROR
		}
	} else {
		rc = 1; // CONTINUE
	}

	return rc;
}

void selectDataSSL(SOCKET ws) {
	cObject* oSocket;
	memCallRet(oSocket, oSocketsSSL->getChild((int)ws); )
	cObject* oDataIn = 0;
	cObject* oDataOut = 0;
	cObject* oIP = 0;
	cObject* oPost = 0;
	cObject* oSSL = 0;
	cObject* oSSLStatus = 0;
	cObject* oUnsafe = 0;
	SSL* ssl = 0;
	int status = 0;
	if (oSocket != 0) {
		oDataIn = oSocket->isChild("DataIn");
		oDataOut = oSocket->isChild("DataOut");
		oIP = oSocket->isChild("IP");
		oPost = oSocket->isChild("Post");
		oSSL = oSocket->isChild("SSL");
		if (oSSL) {
			ssl = (SSL*)(oSSL->lpData);
			memCallRet( oSSLStatus, oSocket->getChild("SSLStatus"); )
			status = oSSLStatus->toInt();
		}
		oUnsafe = oSocket->isChild("SSLUnsafe");
	}

	int rc = 0; // DISCONNECT
	bool closeHard = true;

	if (ssl != 0) {
		if (status == SSL_QUERY_ACCEPT) {
			do {
				rc = SSL_accept(ssl);
				if (rc == 1) {
					oSSLStatus->setNumber((int)SSL_QUERY_NONE);

					if (SSL_get_verify_result(ssl) == X509_V_OK) {
						break; /// OK
					}
				} else {
					rc = selectErrorSSL(ssl, rc);
					if (rc > 0) {
						oSSLStatus->setNumber(SSL_QUERY_ACCEPT);
						break;
					}
				}

				if (rc > 0) {
					memCall( oSocket->getChild("SSLUnsafe")->setNumber(true); )
				}
			} while (false);
		} else if (status != SSL_QUERY_WRITE) {
			rc = SSL_read(ssl, selectBufferSSL, sizeof(selectBufferSSL) - 1);

			if (rc <= 0) {
				rc = selectErrorSSL(ssl, rc);
				if (rc > 0) {
					oSSLStatus->setNumber(SSL_QUERY_READ);
				}
			} else {
				if (oDataOut == 0) {
					if ((oDataIn != 0) && (oDataIn->type == otText)) {
						selectBufferSSL[rc] = 0;
						*(oDataIn->lpsText) += selectBufferSSL;

						long queryEnd = oDataIn->lpsText->pos(L"\r\n\r\n");
						if (queryEnd >= 0) {
							if (oUnsafe == 0) {
								int len = 0;
								char* response = 0;
								bool gotData = false;
								if (oDataIn->lpsText->like(L"GET /* HTTP/1.?\r\n*")) {
									response = httpGet(*(oDataIn->lpsText), 0, &len, oIP->toInt(), true);
									gotData = true;
								} else if (oDataIn->lpsText->like(L"POST /* HTTP/1.?\r\n*")) {
									int postSize = oPost->toInt();
									if (postSize == 0) {
										if ((oDataIn->lpsText->pos(L"Content-Type: application/x-www-form-urlencoded\r\n") > 0) ||
											(oDataIn->lpsText->pos(L"Content-Type: application/x-www-form-urlencoded;") > 0))
										{
											long posStart = oDataIn->lpsText->pos(L"Content-Length: ");
											long posEnd = oDataIn->lpsText->pos(L"\r\n", posStart);

											if ((posStart > 0) && (posEnd > 0)) {
												postSize = (int)(oDataIn->lpsText->subString(posStart + 16, posEnd - posStart - 16));
											} else {
												postSize = -1;
											}
										} else {
											postSize = -1;
										}
										oPost->setNumber(postSize);
									}
									if (postSize < 0) {
										rc = 0; // DISCONNECT
									} else if (postSize > 0) {
										if ((oDataIn->lpsText->strLength - queryEnd - 4) == postSize) {
											//cString post = oDataIn->lpsText->subString(queryEnd + 4, oDataIn->lpsText->strLength - queryEnd - 4);
											response = httpGet(*(oDataIn->lpsText), queryEnd + 4, &len, oIP->toInt(), true);
											gotData = true;
										}
									}
								} else {
									rc = 0; // DISCONNECT
								}

								if (gotData) {
									if (response) {
										rc = SSL_write(ssl, response, len);

										if (rc <= 0) { // None Transfered
											rc = selectErrorSSL(ssl, rc);
											if (rc > 0) {
												memCallRet( oDataOut, oSocket->getChild("DataOut"); )
												oDataOut->setData(response, len);
												memCall( oSocket->getChild("DataOutOfs")->setNumber(0); )

												oSSLStatus->setNumber(SSL_QUERY_WRITE);
												FD_SET(ws, &fdWriteSSL);
											}
										} else if (rc < len) { // Partial Transfered
											memCallRet( oDataOut, oSocket->getChild("DataOut"); )
											oDataOut->setData(response + rc, len - rc);
											memCall( oSocket->getChild("DataOutOfs")->setNumber(0); )

											oSSLStatus->setNumber(SSL_QUERY_WRITE);
											FD_SET(ws, &fdWriteSSL);

											rc = 1; // CONTINUE
										} else { // All Transfered
											closeHard = false;
											memFree(response);

											rc = 0; // DISCONNECT
										}
									} else {
										rc = 0; // DISCONNECT
									}
								}
							} else {
								rc = 0; // DISCONNECT
							}
						}
					}
				}
			}
		} else {
			cObject* oDataOutOfs;
			memCallRet( oDataOutOfs, oSocket->getChild("DataOutOfs"); )
			int ofs = oDataOutOfs->toInt();
			int len = oDataOut->dwMemSize - ofs;
			char* response = (char*)(oDataOut->lpData) + ofs;

			rc = SSL_write(ssl, response, len);

			if (rc <= 0) { // None Transfered
				rc = selectErrorSSL(ssl, rc);
				if (rc > 0) {
					oSSLStatus->setNumber(SSL_QUERY_WRITE);
				}
			} else if (rc < len) { // Partial Transfered
				memCall( oSocket->getChild("DataOutOfs")->setNumber(ofs + rc); )

				oSSLStatus->setNumber(SSL_QUERY_WRITE);

				rc = 1; // CONTINUE
			} else { // All Transfered
				closeHard = false;
				rc = 0; // DISCONNECT
			}

			if (rc <= 0) {
				void* ptr = oDataOut->lpData;
				memDelete(oDataOut); oDataOut = 0;
				memFree(ptr);
			}
		}

		if (rc <= 0) {
			SSL_shutdown(ssl);
			SSL_free(ssl);
		}
	}

	if (rc <= 0) {
		if (closeHard) {
			closesocket_h(ws);
		} else {
			closesocket(ws);
		}

		FD_CLR(ws, &fdMasterSSL);
		FD_CLR(ws, &fdWriteSSL);
		memDelete(oSocketsSSL->getChild((int)ws));
	}
}

bool selectLoopSSL() {
    int rc;
    timeval timeout;
    sockaddr_in sa;
    int addr_len;
    SOCKET ws;

    timeout.tv_sec = 5; // Устанавливаем таймаут 30 секунд
    timeout.tv_usec = 0;

    fd_set fdWorkingSSL;        // Рабочие сокеты
    fd_set fdCheckSSL;			// Сокеты требующие повторной записи

	CopyMemory(&fdWorkingSSL, &fdMasterSSL, sizeof(fd_set));
	CopyMemory(&fdCheckSSL, &fdWriteSSL, sizeof(fd_set));

	rc = select(0, &fdWorkingSSL, &fdCheckSSL, 0, &timeout);

    if (rc < 0) {
		// Ошибка сокетов
		return false;
    } else if (rc == 0) {
    	// timeout
    } else {
    	for (int i = fdWorkingSSL.fd_count-1; i >= 0; i--) {
			ws = fdWorkingSSL.fd_array[i];
    		if (!FD_ISSET(ws,&fdListenSSL)) { // Соединение принимает данные
				selectDataSSL(ws);
    		}
    	}
    	for (int i = fdWorkingSSL.fd_count-1; i >= 0; i--) {
			ws = fdWorkingSSL.fd_array[i];
    		if (FD_ISSET(ws,&fdListenSSL)) { // Прослушиваемый сокет принимает соединение
    			addr_len = sizeof(sockaddr_in);
    			SOCKET s = accept(ws, (struct sockaddr*)&sa, &addr_len);
				int ip = sa.sin_addr.S_un.S_addr;

				if ((ip == 0x0100007F) || (selectCheckIP(ip) == 1)) {
					if (setTCPSocketOptions(s) < 0) {
						closesocket_h(s);
					} else {
						FD_SET(s, &fdMasterSSL);
						cObject* oSocket;
						memCallRet( oSocket, oSocketsSSL->getChild((int)s); )
						memCall( oSocket->getChild("DataIn")->setText(sNULL); )
						memCall( oSocket->getChild("IP")->setNumber(ip); )
						memCall( oSocket->getChild("Post")->setNumber(0); )

						/// OpenSSL Create Socket
						SSL* ssl = 0;

						do {
							rc = 0;
							ssl = SSL_new(ctx);

							if (ssl != 0) {
								BIO* sslclient = BIO_new_socket(s, BIO_NOCLOSE);
								SSL_set_bio(ssl, sslclient, sslclient);
								//SSL_set_fd(ssl, s);

								memCall( oSocket->getChild("SSL")->setData(ssl, sizeof(SSL)); )
								cObject* oSSLStatus;
								memCallRet( oSSLStatus, oSocket->getChild("SSLStatus"); )

								rc = SSL_accept(ssl);
								if (rc == 1) {
									oSSLStatus->setNumber((int)SSL_QUERY_NONE);

									if (SSL_get_verify_result(ssl) == X509_V_OK) {
										break; /// OK
									}
								} else {
									rc = selectErrorSSL(ssl, rc);
									if (rc > 0) {
										oSSLStatus->setNumber(SSL_QUERY_ACCEPT);
										break;
									}
								}

								if (rc <= 0) {
									SSL_shutdown(ssl);
									SSL_free(ssl);
								}
							}

							if (rc <= 0) {
								closesocket_h(s);

								FD_CLR(s, &fdMasterSSL);
								memDelete(oSocketsSSL->getChild((int)s));
							} else {
								memCall( oSocket->getChild("SSLUnsafe")->setNumber(true); )
							}
						} while (false);
					}
				} else {
					closesocket_h(s);
				}
    		}
    	}
    	for (int i = fdCheckSSL.fd_count-1; i >= 0; i--) {
    		ws = fdCheckSSL.fd_array[i];
    		if (FD_ISSET(ws,&fdWriteSSL)) { // Сокет доступен для записи
				selectDataSSL(ws);
    		}
    	}
    }

	return true;
}

uint32_t tNetworkSelectSSLThreadProc(struct tThread* thread) {
	while (WaitForSingleObject(evThreadExitRequest, 0) != WAIT_OBJECT_0) {
		if (selectStartSSL()) {
			if (!selectLoopSSL()) {
				selectStopSSL();
			}
		} else {
			Sleep(1000);
		}
	}

	selectStopSSL();

#ifdef LOG_ENABLE
    gameLog(L"SSL", L"Exit");
#endif

    return 0;
}

uint32_t tNetworkSelectSSLKillThreadProc(struct tThread* thread) {
    WSACancelBlockingCall();

	selectStopSSL();

    return 0;
}

void thServerStart() {
	slmc = slCreateSingleLock(false);
	initIPHLPAPI();

    // Создаём поток главного окна
    serverThread = thAlloc(threadList, mainThread, THREAD_SELECT, INFINITE, tNetworkSelectThreadProc, 0, tNetworkSelectKillThreadProc);
    serverThread->data = 0;
    thPriority(serverThread, THREAD_PRIORITY_HIGHEST);
    //thPriority(serverThread, THREAD_PRIORITY_IDLE);

    serverThreadSSL = thAlloc(threadList, mainThread, THREAD_SELECT, INFINITE, tNetworkSelectSSLThreadProc, 0, tNetworkSelectSSLKillThreadProc);
    serverThreadSSL->data = 0;
    thPriority(serverThreadSSL, THREAD_PRIORITY_HIGHEST);
    //thPriority(serverThreadSSL, THREAD_PRIORITY_IDLE);
}

void thServerResume() {
    // Запускаем поток select
    thResume(serverThread);
    thResume(serverThreadSSL);
}

void thServerStop() {
	quitIPHLPAPI();
	slCloseSingleLock(slmc);
}
