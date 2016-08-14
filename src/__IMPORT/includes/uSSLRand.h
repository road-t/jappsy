#ifndef uSSLRandH
#define uSSLRandH

void RSA_r_start();
void RSA_r_stop();

void RSA_r_add(const void *buf, int num, double entropy);
int RSA_r_status();
int RSA_r_bytes(unsigned char *buf, int num);
void RSA_r_cleanup();
void RSA_r_add(const void *buf, int num, double add);
void RSA_r_seed(const void *buf, int num);
int RSA_r_pseudorand(unsigned char *buf, int num);

extern double RSA_entropy;

#include <openssl/rsa.h>

RSA* RSA_keygen(RSA* key, int bits = 1024, bool interactive = false);
RSA* RSA_keyfree(RSA* key);

#endif
