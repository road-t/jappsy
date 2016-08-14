#include <stdint.h>
#include <malloc.h>
#include <time.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

#include "uSSLUtil.h"
#include "cMD5.h"
#include "cSHA.h"
#include "core\uSpinLock.h"

#include "core\uMemoryManager.h"
#include "core\uMemory.h"

#include <openssl/ssl.h>
#include <openssl/crypto.h>

static slLock** lockarray = 0;

static void openssl_lock_callback(int mode, int type, const char *file, int line) {
    (void)file;
    (void)line;
    if (mode & CRYPTO_LOCK) {
        slSingleLock(lockarray[type]);
    } else {
        slSingleUnlock(lockarray[type]);
    }
}

static unsigned long openssl_thread_id() {
    return (unsigned long)GetCurrentThreadId();
}

static void openssl_init_locks() {
    lockarray = (slLock**)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(slLock*));
    for (int i = 0; i < CRYPTO_num_locks(); i++) {
        memCall( lockarray[i] = slCreateSingleLock(0); )
    }

    CRYPTO_set_id_callback((unsigned long (*)())openssl_thread_id);
    CRYPTO_set_locking_callback(openssl_lock_callback);
}

static void openssl_kill_locks() {
    CRYPTO_set_id_callback(0);
    CRYPTO_set_locking_callback(0);
    for (int i = 0; i < CRYPTO_num_locks(); i++) {
        slCloseSingleLock(lockarray[i]);
        lockarray[i] = 0;
    }

    OPENSSL_free(lockarray);
    lockarray = 0;
}

void* OPENSSL_custom_malloc(size_t size) {
	void* ptr = memAlloc(void, ptr, size);
	return ptr;
}

void* OPENSSL_custom_realloc(void* mem, size_t size) {
	void* ptr = memRealloc(void, ptr, mem, size);
	return ptr;
}

void OPENSSL_custom_free(void* mem) {
	memFree(mem);
}

void init_OPENSSL() {
	/* Это нужно для создания HTTPS сервера (https://thunked.org/programming/openssl-tutorial-server-t12.html) */
	//CRYPTO_malloc_init(); // Initialize malloc, free, etc for OpenSSL's use
	//CRYPTO_set_mem_functions(OPENSSL_custom_malloc, OPENSSL_custom_realloc, OPENSSL_custom_free);
	//CRYPTO_set_locked_mem_functions(OPENSSL_custom_malloc, OPENSSL_custom_free);
    SSL_library_init(); // Initialize OpenSSL's SSL libraries
    SSL_load_error_strings(); // Load SSL error strings
    ERR_load_BIO_strings(); // Load BIO error strings
    OpenSSL_add_all_algorithms(); // Load all available encryption algorithms

    openssl_init_locks();
}

void uninit_OPENSSL() {
    openssl_kill_locks();
}

// возвращает требуемый размер буфера для хранения зашифрованного размером size
#define AES_decode_size(size) (((size + 0x2F) & 0xFFFFFFF0))

void sub_AES_encode(AES_KEY* aes, const void* in, unsigned long insize, unsigned long* outBuf) {
	// вычисляем первый рандом блок
	unsigned long temp[4];
	asm volatile (
        ".byte 15, 49\n\t"
        "movl %%eax, (%%edi)\n\t"
        "movl %%edx, 4(%%edi)\n\t"
    : : "D" (temp)
    : "%eax", "%edx"
    );
	time((long*)&(temp[2]));
	temp[3] = insize;
	cMD5 md5;
	md5.init();
	md5.update((char*)temp,16);
	md5.final();
	md5.state[3] = insize;
	// шифруем первый блок
	AES_encrypt((unsigned char*)(md5.state),(unsigned char*)outBuf,aes);

	// вычисляем второй блок (подтверждающий верность первого)
	md5.init();
	md5.update((char*)&(temp[3]),4);
	md5.final();
	md5.state[0] ^= outBuf[0];
	md5.state[1] ^= outBuf[1];
	md5.state[2] ^= outBuf[2];
	md5.state[3] ^= outBuf[3];
	outBuf += 4;
	// шифруем второй блок
	AES_encrypt((unsigned char*)(md5.state),(unsigned char*)outBuf,aes);

	// приступаем к шифрованию самих данных
	unsigned char* inBuf = (unsigned char*)in;
	while (insize > 16) {
		memcpy(temp,inBuf,16); insize -= 16; inBuf += 16;
		temp[0] ^= outBuf[0];
		temp[1] ^= outBuf[1];
		temp[2] ^= outBuf[2];
		temp[3] ^= outBuf[3];
		outBuf += 4;
		AES_encrypt((unsigned char*)temp,(unsigned char*)outBuf,aes);
	}

	// дополняем последний блок и шифруем его
	if (insize > 0) {
		memcpy(temp,inBuf,insize);
		md5.init();
		md5.update((char*)temp,insize);
		md5.final();
		memcpy(md5.state,temp,insize);
		md5.state[0] ^= outBuf[0];
		md5.state[1] ^= outBuf[1];
		md5.state[2] ^= outBuf[2];
		md5.state[3] ^= outBuf[3];
		outBuf += 4;
		// шифруем последний блок
		AES_encrypt((unsigned char*)(md5.state),(unsigned char*)outBuf,aes);
	}
	inBuf = 0;
	memset(&md5,0,sizeof(md5));
	memset(temp,0,16);
}

void* AES_encode(const char* key, const void* in, unsigned long insize, unsigned long* outsize) {
	// генерируем ключ для шифрования
	cSHA256 sha;
	sha.init();
	sha.update(key,strlen(key));
	sha.final();
	AES_KEY aes;
	AES_set_encrypt_key((const unsigned char*)(sha.state),256,&aes);
	// подчищаем секретную информацию
	memset(&sha,0,sizeof(sha));

	// вычисляем необходимый размер памяти и выделяем
	unsigned long resSize = AES_decode_size(insize);
	if (outsize) *outsize = resSize;
	unsigned long* outBuf = memAlloc(unsigned long,outBuf,resSize);
	if (!outBuf) throw "Out of memory";
	resSize = 0;

	sub_AES_encode(&aes,in,insize,outBuf);

	// подчищаем секретную информацию
	memset(&aes,0,sizeof(aes));

	return outBuf;
}

void* sub_AES_decode(AES_KEY* aes, const void* in, unsigned long* outsize) {
	// дешифруем первые 2 блока для выявления размера буфера
	unsigned long temp[8];
	unsigned long* inBuf = (unsigned long*)in;
	AES_decrypt((unsigned char*)inBuf,(unsigned char*)temp,aes); inBuf += 4;
	AES_decrypt((unsigned char*)inBuf,(unsigned char*)&(temp[4]),aes);
	temp[4] ^= inBuf[-4];
	temp[5] ^= inBuf[-3];
	temp[6] ^= inBuf[-2];
	temp[7] ^= inBuf[-1];
	inBuf += 4;
	cMD5 md5;
	md5.init();
	md5.update((char*)&(temp[3]),4);
	md5.final();
	// сравниваем temp[4-7] с md5.state - должны быть одинаковы
	if (memcmp(&(temp[4]),md5.state,16) == 0)  {
		// подчищаем секретную информацию
		memset(&md5,0,sizeof(md5));
		unsigned long size = temp[3];
		memset(temp,0,32);
		// запоминаем размер
		if (outsize) *outsize = size;
		// выделяем память
        unsigned long* outBuf = memAlloc(unsigned long,outBuf,size);
		if (!outBuf) throw "Out of memory";
		unsigned long* out = outBuf;
		// дешифруем блоки
		while (size > 16) {
			AES_decrypt((unsigned char*)inBuf,(unsigned char*)outBuf,aes);
			outBuf[0] ^= inBuf[-4];
			outBuf[1] ^= inBuf[-3];
			outBuf[2] ^= inBuf[-2];
			outBuf[3] ^= inBuf[-1];
			inBuf += 4; outBuf += 4; size -= 16;
		}
		// дешифруем последний блок
		if (size > 0) {
			AES_decrypt((unsigned char*)inBuf,(unsigned char*)temp,aes);
			temp[0] ^= inBuf[-4];
			temp[1] ^= inBuf[-3];
			temp[2] ^= inBuf[-2];
			temp[3] ^= inBuf[-1];
			memcpy(outBuf,temp,size);
		}
		// подчищаем секретную информацию
		inBuf = 0; outBuf = 0;
		memset(temp,0,16);
		return out;
	}
	// подчищаем секретную информацию
	inBuf = 0;
	memset(&md5,0,sizeof(md5));
	memset(temp,0,32);
	return 0;
}

void* AES_decode(const char* key, const void* in, unsigned long* outsize) {
	// генерируем ключ для шифрования
	cSHA256 sha;
	sha.init();
	sha.update(key,strlen(key));
	sha.final();
	AES_KEY aes;
	AES_set_decrypt_key((const unsigned char*)(sha.state),256,&aes);
	// подчищаем секретную информацию
	memset(&sha,0,sizeof(sha));

	void* res = sub_AES_decode(&aes,in,outsize);

	memset(&aes,0,sizeof(aes));

	return res;
}

void* RSA_private_encode(RSA* key, const void* in, unsigned long insize, unsigned long* outsize) {
	unsigned long rsaSize = RSA_size(key);

	// генерируем ключ для шифрования
	cSHA256 sha;
	RAND_pseudo_bytes((unsigned char*)(sha.state),32);
	AES_KEY aes;
	AES_set_encrypt_key((const unsigned char*)(sha.state),256,&aes);
	// подчищаем секретную информацию

	// вычисляем необходимый размер памяти и выделяем
	unsigned long resSize = AES_decode_size(insize);
	if (outsize) (*outsize) = (resSize + rsaSize);
	char* outBuf = memAlloc(char,outBuf,resSize + rsaSize);
	if (!outBuf) return 0;
	resSize = 0;

	// шифруем сессионный ключ
	RSA_private_encrypt(32,(const unsigned char*)(sha.state),(unsigned char*)outBuf,key,RSA_PKCS1_PADDING);
	memset(&sha,0,sizeof(sha));

	// шифруем данные сессионным ключём
	sub_AES_encode(&aes,in,insize,(unsigned long*)(outBuf+rsaSize));

	// подчищаем секретную информацию
	memset(&aes,0,sizeof(aes));

	return outBuf;
}

void* RSA_public_encode(RSA* key, const void* in, unsigned long insize, unsigned long* outsize) {
	unsigned long rsaSize = RSA_size(key);

	// генерируем ключ для шифрования
	cSHA256 sha;
	RAND_pseudo_bytes((unsigned char*)(sha.state),32);
	AES_KEY aes;
	AES_set_encrypt_key((const unsigned char*)(sha.state),256,&aes);
	// подчищаем секретную информацию

	// вычисляем необходимый размер памяти и выделяем
	unsigned long resSize = AES_decode_size(insize);
	if (outsize) (*outsize) = (resSize + rsaSize);
	char* outBuf = memAlloc(char,outBuf,resSize + rsaSize);
	if (!outBuf) return 0;
	resSize = 0;

	// шифруем сессионный ключ
	RSA_public_encrypt(32,(const unsigned char*)(sha.state),(unsigned char*)outBuf,key,RSA_PKCS1_PADDING);
	memset(&sha,0,sizeof(sha));

	// шифруем данные сессионным ключём
	sub_AES_encode(&aes,in,insize,(unsigned long*)(outBuf+rsaSize));

	// подчищаем секретную информацию
	memset(&aes,0,sizeof(aes));

	return outBuf;
}

void* RSA_private_decode(RSA* key, const void* in, unsigned long insize, unsigned long* outsize) {
	unsigned long rsaSize = RSA_size(key);
	unsigned char* sha = memAlloc(unsigned char,sha,rsaSize-11);
	if (!sha) return 0;

	unsigned char* inBuf = (unsigned char*)in;
	// получаем сессионный ключ
	RSA_private_decrypt(rsaSize,inBuf,sha,key,RSA_PKCS1_PADDING);

	// генерируем ключ для шифрования
	AES_KEY aes;
	AES_set_decrypt_key(sha,256,&aes);
	// подчищаем секретную информацию
	memset(sha,0,rsaSize-11);
	memFree(sha);

	void* res = sub_AES_decode(&aes,(inBuf+rsaSize),outsize);

	memset(&aes,0,sizeof(aes));

	return res;
}

void* RSA_public_decode(RSA* key, const void* in, unsigned long insize, unsigned long* outsize) {
	unsigned long rsaSize = RSA_size(key);
	unsigned char* sha = memAlloc(unsigned char,sha,rsaSize-11);
	if (!sha) return 0;

	unsigned char* inBuf = (unsigned char*)in;
	// получаем сессионный ключ
	RSA_public_decrypt(rsaSize,inBuf,sha,key,RSA_PKCS1_PADDING);

	// генерируем ключ для шифрования
	AES_KEY aes;
	AES_set_decrypt_key(sha,256,&aes);
	// подчищаем секретную информацию
	memset(sha,0,rsaSize-11);
	memFree(sha);

	void* res = sub_AES_decode(&aes,(inBuf+rsaSize),outsize);

	memset(&aes,0,sizeof(aes));

	return res;
}

void* AES_free(void* mem) {
    memFree(mem);
    return 0;
}

void* RSA_free(void* mem) {
    memFree(mem);
    return 0;
}

#include <openssl/bn.h>

char* RSA_store_private_key(RSA* key) {
	char *strN, *strE, *strD, *strP, *strQ, *strDmp1, *strDmq1, *strIqmp;
	char *res = 0;
	if (key) {
        strN = BN_bn2hex(key->n);
        strE = BN_bn2hex(key->e);
        strD = BN_bn2hex(key->d);
        strP = BN_bn2hex(key->p);
        strQ = BN_bn2hex(key->q);
        strDmp1 = BN_bn2hex(key->dmp1);
        strDmq1 = BN_bn2hex(key->dmq1);
        strIqmp = BN_bn2hex(key->iqmp);
        int len = strlen(strN)+strlen(strE)+strlen(strD)+strlen(strP)+strlen(strQ)+strlen(strDmp1)+strlen(strDmq1)+strlen(strIqmp);
        res = memAlloc(char,res,len+8);
        if (res != 0) {
            strcpy(res, strN);
            strcat(res, "\n");
            strcat(res, strE);
            strcat(res, "\n");
            strcat(res, strD);
            strcat(res, "\n");
            strcat(res, strP);
            strcat(res, "\n");
            strcat(res, strQ);
            strcat(res, "\n");
            strcat(res, strDmp1);
            strcat(res, "\n");
            strcat(res, strDmq1);
            strcat(res, "\n");
            strcat(res, strIqmp);
        }
        OPENSSL_free(strN);
        OPENSSL_free(strE);
        OPENSSL_free(strD);
        OPENSSL_free(strP);
        OPENSSL_free(strQ);
        OPENSSL_free(strDmp1);
        OPENSSL_free(strDmq1);
        OPENSSL_free(strIqmp);
	}
    return res;
}

char* RSA_store_public_key(RSA* key) {
	char *strN, *strE;
	char *res = 0;
	if (key) {
        strN = BN_bn2hex(key->n);
        strE = BN_bn2hex(key->e);
        int len = strlen(strN)+strlen(strE);
        res = memAlloc(char,res,len+2);
        if (res != 0) {
            strcpy(res, strN);
            strcat(res, "\n");
            strcat(res, strE);
        }
        OPENSSL_free(strN);
        OPENSSL_free(strE);
	}
	return res;
}

char* RSA_readln(char** source) {
    char* ptr = (*source);
    char ch;
    int len = 0;
    bool stop = false;
    while ((ch = (*ptr)) != '\0') {
        if ((ch == '\n') || (ch == '\r')) {
            ptr++; stop = true; continue;
        } else if (stop) break;
        ptr++; len++;
    }
    char* dest = 0;
    if (len > 0) {
        dest = memAlloc(char,dest,len+1);
        if (dest) {
            memcpy(dest,(*source),len);
            dest[len] = 0;
        }
    }
    (*source) = ptr;
    return dest;
}

RSA* RSA_restore_key(const char* data) {
    RSA* key = 0;
    if (data) {
        key = RSA_new();
        if (key) {
            char *strN, *strE, *strD, *strP, *strQ, *strDmp1, *strDmq1, *strIqmp;
            char* ptr = (char*)data;
            strN = RSA_readln(&ptr);
            strE = RSA_readln(&ptr);
            strD = RSA_readln(&ptr);
            strP = RSA_readln(&ptr);
            strQ = RSA_readln(&ptr);
            strDmp1 = RSA_readln(&ptr);
            strDmq1 = RSA_readln(&ptr);
            strIqmp = RSA_readln(&ptr);
            if (strN) { BN_hex2bn(&(key->n),strN); memFree(strN); }
            if (strE) { BN_hex2bn(&(key->e),strE); memFree(strE); }
            if (strD) {
                BN_hex2bn(&(key->d),strD); memFree(strD);
                if (strP) { BN_hex2bn(&(key->p),strP); memFree(strP); }
                if (strQ) { BN_hex2bn(&(key->q),strQ); memFree(strQ); }
                if (strDmp1) { BN_hex2bn(&(key->dmp1),strDmp1); memFree(strDmp1); }
                if (strDmq1) { BN_hex2bn(&(key->dmq1),strDmq1); memFree(strDmq1); }
                if (strIqmp) { BN_hex2bn(&(key->iqmp),strIqmp); memFree(strIqmp); }
                if (RSA_check_key(key) == 1) {
                    return key;
                }
            } else if ((key->n) && (key->e)) {
                return key;
            }
            RSA_free(key);
            key = 0;
        }
    }
    return key;
}
