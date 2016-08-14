#ifndef uSSLUtilH
#define uSSLUtilH

#include <openssl/rsa.h>
//---------------------------------------------------------------------------
void init_OPENSSL();
void uninit_OPENSSL();

// возвращает зашифрованный блок размером outsize из in по ключу key
void* AES_encode(const char* key, const void* in, unsigned long insize, unsigned long* outsize);

// возвращает разшифрованный блок размером outsize из in по ключу key
// для уничтожения блока используйте AES_free(mem)
void* AES_decode(const char* key, const void* in, unsigned long* outsize);

void* AES_free(void* mem);

void* RSA_private_encode(RSA* key, const void* in, unsigned long insize, unsigned long* outsize);
void* RSA_private_decode(RSA* key, const void* in, unsigned long insize, unsigned long* outsize);
void* RSA_public_encode(RSA* key, const void* in, unsigned long insize, unsigned long* outsize);
void* RSA_public_decode(RSA* key, const void* in, unsigned long insize, unsigned long* outsize);

void* RSA_free(void* mem);

char* RSA_store_private_key(RSA* key);
char* RSA_store_public_key(RSA* key);
RSA* RSA_restore_key(const char* key);

#endif
