//---------------------------------------------------------------------------
#include <stdint.h>
#include <mem.h>
#include <malloc.h>
#include "cSHA.h"
#include "../core/uMemory.h"
#include "../core/uMemoryManager.h"

void* cSHA160::operator new(size_t size) throw(const char*) {
    void *p = memAlloc(void, p, size); // mmalloc(size);
//    if (!p) throw "cSound::operator new() error";
    return p;
}

void* cSHA160::operator new[](size_t size) throw(const char*) {
    void *p = memAlloc(void, p, size); // mmalloc(size);
//    if (!p) throw "cSound::operator new[]() error";
    return p;
}

void cSHA160::operator delete(void *p) {
    memFree(p); // mmfree(p);
}

void cSHA160::operator delete[](void *p) {
    memFree(p); // mmfree(p);
}

void* cSHA256::operator new(size_t size) throw(const char*) {
    void *p = memAlloc(void, p, size); // mmalloc(size);
//    if (!p) throw "cSound::operator new() error";
    return p;
}

void* cSHA256::operator new[](size_t size) throw(const char*) {
    void *p = memAlloc(void, p, size); // mmalloc(size);
//    if (!p) throw "cSound::operator new[]() error";
    return p;
}

void cSHA256::operator delete(void *p) {
    memFree(p); // mmfree(p);
}

void cSHA256::operator delete[](void *p) {
    memFree(p); // mmfree(p);
}

void* cSHA512::operator new(size_t size) throw(const char*) {
    void *p = memAlloc(void, p, size); // mmalloc(size);
//    if (!p) throw "cSound::operator new() error";
    return p;
}

void* cSHA512::operator new[](size_t size) throw(const char*) {
    void *p = memAlloc(void, p, size); // mmalloc(size);
//    if (!p) throw "cSound::operator new[]() error";
    return p;
}

void cSHA512::operator delete(void *p) {
    memFree(p); // mmfree(p);
}

void cSHA512::operator delete[](void *p) {
    memFree(p); // mmfree(p);
}

cSHA160::cSHA160() {}
cSHA160::~cSHA160() {}
cSHA256::cSHA256() {}
cSHA256::~cSHA256() {}
cSHA512::cSHA512() {}
cSHA512::~cSHA512() {}

void cSHA160::init() {
	count = 0;
	state[0] = 0x67452301;
	state[1] = 0xefcdab89;
	state[2] = 0x98Badcfe;
	state[3] = 0x10325476;
	state[4] = 0xc3d2e1f0;
}

void cSHA224::init() {
	count = 0;
	state[0] = 0xc1059ed8;
	state[1] = 0x367cd507;
	state[2] = 0x3070dd17;
	state[3] = 0xf70e5939;
	state[4] = 0xffc00b31;
	state[5] = 0x68581511;
	state[6] = 0x64f98fa7;
	state[7] = 0xbefa4fa4;
}

void cSHA256::init() {
	count = 0;
	state[0] = 0x6a09e667;
	state[1] = 0xbb67ae85;
	state[2] = 0x3c6ef372;
	state[3] = 0xa54ff53a;
	state[4] = 0x510e527f;
	state[5] = 0x9b05688c;
	state[6] = 0x1f83d9ab;
	state[7] = 0x5be0cd19;
}

void cSHA384::init() {
	count[0] = 0; count[1] = 0;
	state[0] = 0xcbbb9d5dc1059ed8ULL;
	state[1] = 0x629a292a367cd507ULL;
	state[2] = 0x9159015a3070dd17ULL;
	state[3] = 0x152fecd8f70e5939ULL;
	state[4] = 0x67332667ffc00b31ULL;
	state[5] = 0x8eb44a8768581511ULL;
	state[6] = 0xdb0c2e0d64f98fa7ULL;
	state[7] = 0x47b5481dbefa4fa4ULL;
}

void cSHA512::init() {
	count[0] = 0; count[1] = 0;
	state[0] = 0x6a09e667f3bcc908ULL;
	state[1] = 0xbb67ae8584caa73bULL;
	state[2] = 0x3c6ef372fe94f82bULL;
	state[3] = 0xa54ff53a5f1d36f1ULL;
	state[4] = 0x510e527fade682d1ULL;
	state[5] = 0x9b05688c2b3e6c1fULL;
	state[6] = 0x1f83d9abfb41bd6bULL;
	state[7] = 0x5be0cd19137e2179ULL;
}

void cSHA160::update(const char* buffer, unsigned long size) {
	unsigned long index = (((unsigned long)count) >> 3) & 0x3F; // bytes mod 64
	uint64_t sizeBits = size; sizeBits <<= 3;
	count += sizeBits;
	unsigned long partLen = index; partLen ^= 0x3F; ++partLen;
	if (size >= partLen) {
		memcpy(part+index, buffer, partLen); buffer += partLen;
		transform((unsigned long*)part);
		size -= partLen; index = size >> 6; size &= 0x3F;
		while (index > 0) {
			transform((unsigned long*)buffer); buffer += 64; --index;
		}
	}
	if (size > 0)
		memcpy(part+index, buffer, size);
}

void cSHA256::update(const char* buffer, unsigned long size) {
	unsigned long index = (((unsigned long)count) >> 3) & 0x3F; // bytes mod 64
	uint64_t sizeBits = size; sizeBits <<= 3;
	count += sizeBits;
	unsigned long partLen = index; partLen ^= 0x3F; ++partLen;
	if (size >= partLen) {
		memcpy(part+index, buffer, partLen); buffer += partLen;
		transform((unsigned long*)part);
		size -= partLen; index = size >> 6; size &= 0x3F;
		while (index > 0) {
			transform((unsigned long*)buffer); buffer += 64; --index;
		}
	}
	if (size > 0)
		memcpy(part+index, buffer, size);
}

void cSHA512::update(const char* buffer, unsigned long size) {
	unsigned long index = (((unsigned long)(count[0])) >> 3) & 0x7F; // bytes mod 128
	uint64_t sizeBits = size; sizeBits <<= 3;
	count[0] += sizeBits;
	if (count[0] < sizeBits) ++count[1];
	unsigned long partLen = index; partLen ^= 0x7F; ++partLen;
	if (size >= partLen) {
		memcpy(part+index, buffer, partLen); buffer += partLen;
		transform((unsigned uint64_t*)part);
		size -= partLen; index = size >> 7; size &= 0x7F;
		while (index > 0) {
			transform((unsigned uint64_t*)buffer); buffer += 128; --index;
		}
	}
	if (size > 0)
		memcpy(part+index, buffer, size);
}

static unsigned long PADDING[32] =
	{ 0x00000080, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void cSHA160::final() {
	unsigned long index = (((unsigned long)count) >> 3) & 0x3F; // bytes mod 64
	unsigned long padLen = (index < 56) ? (56 - index) : (120 - index);
	uint64_t cnt;
	{	// FLIP64 (destination,source)
		void* dest = (void*)(&cnt);
		void* src = (void*)(&count);
		__asm__ __volatile__ (
            "movl (%%edi), %%eax\n\t"
            "movl 4(%%edi), %%edx\n\t"
            "bswap %%eax\n\t"
            "bswap %%edx\n\t"
            "movl %%edx, (%%esi)\n\t"
            "movl %%eax, 4(%%esi)\n\t"
        : : "S" (dest), "D" (src)
        : "%eax", "%edx"
        );
	}
	update((char*)PADDING, padLen);
	update((char*)&cnt, 8);
	count = 0;
	{	// VALFLIP160 (destination)
		void* dest = (void*)state;
		__asm__ __volatile__ (
            "movl (%%esi), %%eax\n\t"
            "movl 4(%%esi), %%edx\n\t"
            "movl 12(%%esi), %%ecx\n\t"
            "movl 16(%%esi), %%ebx\n\t"
            "movl %%ebx, (%%esi)\n\t"
            "movl %%ecx, 4(%%esi)\n\t"
            "movl %%edx, 12(%%esi)\n\t"
            "movl %%eax, 16(%%esi)\n\t"
            "xorl %%eax, %%eax\n\t"
            "movl %%eax, 20(%%esi)\n\t"
        : : "S" (dest)
        : "%eax", "%ebx", "%ecx", "%edx"
        );
	}
	memset(part,0,64);
}

/*
cString cSHA160::toString() {
	unsigned char* sha = (unsigned char*)state;
	cString res;
	for (int i = 19; i >= 0; i--) {
		unsigned char ch = sha[i];
		unsigned char chu = ch >> 4;
		unsigned char chl = ch & 0x0F;
		chu = (chu >= 10) ? chu + 'A' - 10 : chu + '0';
		chl = (chl >= 10) ? chl + 'A' - 10 : chl + '0';
		res +=((char)chu);
		res +=((char)chl);
	}
	return res;
}
*/

void cSHA224::final() {
	unsigned long index = (((unsigned long)count) >> 3) & 0x3F; // bytes mod 64
	unsigned long padLen = (index < 56) ? (56 - index) : (120 - index);
	uint64_t cnt;
	{	// FLIP64 (destination,source)
		void* dest = (void*)(&cnt);
		void* src = (void*)(&count);
		__asm__ __volatile__ (
            "movl (%%edi), %%eax\n\t"
            "movl 4(%%edi), %%edx\n\t"
            "bswap %%eax\n\t"
            "bswap %%edx\n\t"
            "movl %%edx, (%%esi)\n\t"
            "movl %%eax, 4(%%esi)\n\t"
        : : "S" (dest), "D" (src)
        : "%eax", "%edx"
        );
	}
	update((char*)PADDING, padLen);
	update((char*)&cnt, 8);
	count = 0;
	{	// VALFLIP224 (destination)
		void* dest = (void*)state;
		__asm__ __volatile__ (
            "movl (%%esi), %%eax\n\t"
            "movl 4(%%esi), %%edx\n\t"
            "movl 20(%%esi), %%ecx\n\t"
            "movl 24(%%esi), %%ebx\n\t"
            "movl %%ebx, (%%esi)\n\t"
            "movl %%ecx, 4(%%esi)\n\t"
            "movl %%edx, 20(%%esi)\n\t"
            "movl %%eax, 24(%%esi)\n\t"
            "movl 8(%%esi), %%eax\n\t"
            "movl 16(%%esi), %%edx\n\t"
            "movl %%edx, 8(%%esi)\n\t"
            "movl %%eax, 16(%%esi)\n\t"
            "xorl %%eax, %%eax\n\t"
            "movl %%eax, 28(%%esi)\n\t"
        : : "S" (dest)
        : "%eax", "%ebx", "%ecx", "%edx"
        );
	}
	memset(part,0,64);
}

void cSHA256::final() {
	unsigned long index = (((unsigned long)count) >> 3) & 0x3F; // bytes mod 64
	unsigned long padLen = (index < 56) ? (56 - index) : (120 - index);
	uint64_t cnt;
	{	// FLIP64 (destination,source)
		void* dest = (void*)(&cnt);
		void* src = (void*)(&count);
		__asm__ __volatile__ (
            "movl (%%edi), %%eax\n\t"
            "movl 4(%%edi), %%edx\n\t"
            "bswap %%eax\n\t"
            "bswap %%edx\n\t"
            "movl %%edx, (%%esi)\n\t"
            "movl %%eax, 4(%%esi)\n\t"
        : : "S" (dest), "D" (src)
        : "%eax", "%edx"
        );
	}
	update((char*)PADDING, padLen);
	update((char*)&cnt, 8);
	count = 0;
	{	// VALFLIP256 (destination)
		void* dest = (void*)state;
		__asm__ __volatile__ (
            "movl (%%esi), %%eax\n\t"
            "movl 4(%%esi), %%edx\n\t"
            "movl 24(%%esi), %%ecx\n\t"
            "movl 28(%%esi), %%ebx\n\t"
            "movl %%ebx, (%%esi)\n\t"
            "movl %%ecx, 4(%%esi)\n\t"
            "movl %%edx, 24(%%esi)\n\t"
            "movl %%eax, 28(%%esi)\n\t"
            "movl 8(%%esi), %%eax\n\t"
            "movl 12(%%esi), %%edx\n\t"
            "movl 16(%%esi), %%ecx\n\t"
            "movl 20(%%esi), %%ebx\n\t"
            "movl %%ebx, 8(%%esi)\n\t"
            "movl %%ecx, 12(%%esi)\n\t"
            "movl %%edx, 16(%%esi)\n\t"
            "movl %%eax, 20(%%esi)\n\t"
        : : "S" (dest)
        : "%eax", "%ebx", "%ecx", "%edx"
        );
	}
	memset(part,0,64);
}

void cSHA384::final() {
	unsigned long index = (((unsigned long)(count[0])) >> 3) & 0x7F; // bytes mod 128
	unsigned long padLen = (index < 112) ? (112 - index) : (240 - index);
	uint64_t cnt[2];
	{	// FLIP128 (destination,source)
		void* dest = (void*)(cnt);
		void* src = (void*)(count);
		__asm__ __volatile__ (
            "movl (%%edi), %%eax\n\t"
            "movl 4(%%edi), %%edx\n\t"
            "movl 8(%%edi), %%ecx\n\t"
            "movl 12(%%edi), %%ebx\n\t"
            "bswap %%eax\n\t"
            "bswap %%edx\n\t"
            "bswap %%ecx\n\t"
            "bswap %%ebx\n\t"
            "movl %%ebx, (%%esi)\n\t"
            "movl %%ecx, 4(%%esi)\n\t"
            "movl %%edx, 8(%%esi)\n\t"
            "movl %%eax, 12(%%esi)\n\t"
        : : "S" (dest), "D" (src)
        : "%eax", "%edx", "%ecx", "%ebx"
        );
	}
	update((char*)PADDING, padLen);
	update((char*)cnt, 16);
	count[0] = 0; count[1] = 0;
	{	// VALFLIP384 (destination)
		void* dest = (void*)state;
		__asm__ __volatile__ (
            "movl (%%esi), %%eax\n\t"
            "movl 4(%%esi), %%edx\n\t"
            "movl 40(%%esi), %%ecx\n\t"
            "movl 44(%%esi), %%ebx\n\t"
            "movl %%ecx, (%%esi)\n\t"
            "movl %%ebx, 4(%%esi)\n\t"
            "movl %%eax, 40(%%esi)\n\t"
            "movl %%edx, 44(%%esi)\n\t"
            "movl 8(%%esi), %%eax\n\t"
            "movl 12(%%esi), %%edx\n\t"
            "movl 32(%%esi), %%ecx\n\t"
            "movl 36(%%esi), %%ebx\n\t"
            "movl %%ecx, 8(%%esi)\n\t"
            "movl %%ebx, 12(%%esi)\n\t"
            "movl %%eax, 32(%%esi)\n\t"
            "movl %%edx, 36(%%esi)\n\t"
            "movl 16(%%esi), %%eax\n\t"
            "movl 20(%%esi), %%edx\n\t"
            "movl 24(%%esi), %%ecx\n\t"
            "movl 28(%%esi), %%ebx\n\t"
            "movl %%ecx, 16(%%esi)\n\t"
            "movl %%ebx, 20(%%esi)\n\t"
            "movl %%eax, 24(%%esi)\n\t"
            "movl %%edx, 28(%%esi)\n\t"
            "movl $0, 48(%%esi)\n\t"
            "movl $0, 52(%%esi)\n\t"
            "movl $0, 56(%%esi)\n\t"
            "movl $0, 60(%%esi)\n\t"
        : : "S" (dest)
        : "%eax", "%ebx", "%ecx", "%edx"
        );
	}
	memset(part,0,128);
}

void cSHA512::final() {
	unsigned long index = (((unsigned long)(count[0])) >> 3) & 0x7F; // bytes mod 128
	unsigned long padLen = (index < 112) ? (112 - index) : (240 - index);
	uint64_t cnt[2];
	{	// FLIP128 (destination,source)
		void* dest = (void*)(cnt);
		void* src = (void*)(count);
		__asm__ __volatile__ (
            "movl (%%edi), %%eax\n\t"
            "movl 4(%%edi), %%edx\n\t"
            "movl 8(%%edi), %%ecx\n\t"
            "movl 12(%%edi), %%ebx\n\t"
            "bswap %%eax\n\t"
            "bswap %%edx\n\t"
            "bswap %%ecx\n\t"
            "bswap %%ebx\n\t"
            "movl %%ebx, (%%esi)\n\t"
            "movl %%ecx, 4(%%esi)\n\t"
            "movl %%edx, 8(%%esi)\n\t"
            "movl %%eax, 12(%%esi)\n\t"
        : : "S" (dest), "D" (src)
        : "%eax", "%edx", "%ecx", "%ebx"
        );
	}
	update((char*)PADDING, padLen);
	update((char*)cnt, 16);
	count[0] = 0; count[1] = 0;
	{	// VALFLIP512 (destination)
		void* dest = (void*)state;
		__asm__ __volatile__ (
            "movl (%%esi), %%eax\n\t"
            "movl 4(%%esi), %%edx\n\t"
            "movl 56(%%esi), %%ecx\n\t"
            "movl 60(%%esi), %%ebx\n\t"
            "movl %%ecx, (%%esi)\n\t"
            "movl %%ebx, 4(%%esi)\n\t"
            "movl %%eax, 56(%%esi)\n\t"
            "movl %%edx, 60(%%esi)\n\t"
            "movl 8(%%esi), %%eax\n\t"
            "movl 12(%%esi), %%edx\n\t"
            "movl 48(%%esi), %%ecx\n\t"
            "movl 52(%%esi), %%ebx\n\t"
            "movl %%ecx, 8(%%esi)\n\t"
            "movl %%ebx, 12(%%esi)\n\t"
            "movl %%eax, 48(%%esi)\n\t"
            "movl %%edx, 52(%%esi)\n\t"
            "movl 16(%%esi), %%eax\n\t"
            "movl 20(%%esi), %%edx\n\t"
            "movl 40(%%esi), %%ecx\n\t"
            "movl 44(%%esi), %%ebx\n\t"
            "movl %%ecx, 16(%%esi)\n\t"
            "movl %%ebx, 20(%%esi)\n\t"
            "movl %%eax, 40(%%esi)\n\t"
            "movl %%edx, 44(%%esi)\n\t"
            "movl 24(%%esi), %%eax\n\t"
            "movl 28(%%esi), %%edx\n\t"
            "movl 32(%%esi), %%ecx\n\t"
            "movl 36(%%esi), %%ebx\n\t"
            "movl %%ecx, 24(%%esi)\n\t"
            "movl %%ebx, 28(%%esi)\n\t"
            "movl %%eax, 32(%%esi)\n\t"
            "movl %%edx, 36(%%esi)\n\t"
        : : "S" (dest)
        : "%eax", "%ebx", "%ecx", "%edx"
        );
	}
	memset(part,0,128);
}

#define LITTLE_ENDIAN

#define ROTATE_LEFT32(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define F160(x,y,z) (((x) & (y)) | ((~x) & (z)))
#define G160(x,y,z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define H160(x,y,z) ((x) ^ (y) ^ (z))

#define X160(x, i) \
	(x)[(i)&0xF] = ROTATE_LEFT32 ( \
		(x)[((i)-3)&0xF] ^ (x)[((i)-8)&0xF] ^ (x)[((i)-14)&0xF] ^ (x)[((i)-16)&0xF] \
		,1 )

#define R0F160(a, b, c, d, e, x, i) { \
	(e) += F160((b), (c), (d)) + (x[i]) + 0x5A827999 + ROTATE_LEFT32((a),5); \
	(b) = ROTATE_LEFT32((b), 30); \
}
#define R1F160(a, b, c, d, e, x, i)  { \
	(e) += F160((b), (c), (d)) + (X160(x, i)) + 0x5A827999 + ROTATE_LEFT32((a),5); \
	(b) = ROTATE_LEFT32((b), 30); \
}
#define R2H160(a, b, c, d, e, x, i) { \
	(e) += H160((b), (c), (d)) + (X160(x, i)) + 0x6ED9EBA1 + ROTATE_LEFT32((a),5); \
	(b) = ROTATE_LEFT32((b), 30); \
}
#define R3G160(a, b, c, d, e, x, i)  { \
	(e) += G160((b), (c), (d)) + (X160(x, i)) + 0x8F1BBCDC + ROTATE_LEFT32((a),5); \
	(b) = ROTATE_LEFT32((b), 30); \
}
#define R4H160(a, b, c, d, e, x, i)  { \
	(e) += H160((b), (c), (d)) + (X160(x, i)) + 0xCA62C1D6 + ROTATE_LEFT32((a),5); \
	(b) = ROTATE_LEFT32((b), 30); \
}

void cSHA160::transform(unsigned long* x) {
	unsigned long buf[16] = {0};
	#ifdef LITTLE_ENDIAN
	// на x86 компах требуется другой порядок байт
	{	// FLIPDWORD16 (destination,source)
		void* dest = (void*)(buf);
		void* src = (void*)(x);
		__asm__ __volatile__ (
            "movl $16, %%ecx\n\t"
            "xorl %%edx, %%edx\n\t"
        "1:"
            "movl (%%edi,%%edx), %%eax\n\t"
            "bswap %%eax\n\t"
            "movl %%eax, (%%esi,%%edx)\n\t"
            "addl $4, %%edx\n\t"
        "loop 1b\n\t"
        : : "S" (dest), "D" (src)
        : "%eax", "%ecx", "%edx"
		);
	}
	#else
	CopyMemory(buf,x,64);
	#endif

	unsigned long a = state[0], b = state[1],
	c = state[2], d = state[3], e = state[4];

	R0F160(a,b,c,d,e,buf, 0); R0F160(e,a,b,c,d,buf, 1);
	R0F160(d,e,a,b,c,buf, 2); R0F160(c,d,e,a,b,buf, 3);
	R0F160(b,c,d,e,a,buf, 4); R0F160(a,b,c,d,e,buf, 5);
	R0F160(e,a,b,c,d,buf, 6); R0F160(d,e,a,b,c,buf, 7);
	R0F160(c,d,e,a,b,buf, 8); R0F160(b,c,d,e,a,buf, 9);
	R0F160(a,b,c,d,e,buf,10); R0F160(e,a,b,c,d,buf,11);
	R0F160(d,e,a,b,c,buf,12); R0F160(c,d,e,a,b,buf,13);
	R0F160(b,c,d,e,a,buf,14); R0F160(a,b,c,d,e,buf,15);
	R1F160(e,a,b,c,d,buf,16); R1F160(d,e,a,b,c,buf,17);
	R1F160(c,d,e,a,b,buf,18); R1F160(b,c,d,e,a,buf,19);
	R2H160(a,b,c,d,e,buf,20); R2H160(e,a,b,c,d,buf,21);
	R2H160(d,e,a,b,c,buf,22); R2H160(c,d,e,a,b,buf,23);
	R2H160(b,c,d,e,a,buf,24); R2H160(a,b,c,d,e,buf,25);
	R2H160(e,a,b,c,d,buf,26); R2H160(d,e,a,b,c,buf,27);
	R2H160(c,d,e,a,b,buf,28); R2H160(b,c,d,e,a,buf,29);
	R2H160(a,b,c,d,e,buf,30); R2H160(e,a,b,c,d,buf,31);
	R2H160(d,e,a,b,c,buf,32); R2H160(c,d,e,a,b,buf,33);
	R2H160(b,c,d,e,a,buf,34); R2H160(a,b,c,d,e,buf,35);
	R2H160(e,a,b,c,d,buf,36); R2H160(d,e,a,b,c,buf,37);
	R2H160(c,d,e,a,b,buf,38); R2H160(b,c,d,e,a,buf,39);
	R3G160(a,b,c,d,e,buf,40); R3G160(e,a,b,c,d,buf,41);
	R3G160(d,e,a,b,c,buf,42); R3G160(c,d,e,a,b,buf,43);
	R3G160(b,c,d,e,a,buf,44); R3G160(a,b,c,d,e,buf,45);
	R3G160(e,a,b,c,d,buf,46); R3G160(d,e,a,b,c,buf,47);
	R3G160(c,d,e,a,b,buf,48); R3G160(b,c,d,e,a,buf,49);
	R3G160(a,b,c,d,e,buf,50); R3G160(e,a,b,c,d,buf,51);
	R3G160(d,e,a,b,c,buf,52); R3G160(c,d,e,a,b,buf,53);
	R3G160(b,c,d,e,a,buf,54); R3G160(a,b,c,d,e,buf,55);
	R3G160(e,a,b,c,d,buf,56); R3G160(d,e,a,b,c,buf,57);
	R3G160(c,d,e,a,b,buf,58); R3G160(b,c,d,e,a,buf,59);
	R4H160(a,b,c,d,e,buf,60); R4H160(e,a,b,c,d,buf,61);
	R4H160(d,e,a,b,c,buf,62); R4H160(c,d,e,a,b,buf,63);
	R4H160(b,c,d,e,a,buf,64); R4H160(a,b,c,d,e,buf,65);
	R4H160(e,a,b,c,d,buf,66); R4H160(d,e,a,b,c,buf,67);
	R4H160(c,d,e,a,b,buf,68); R4H160(b,c,d,e,a,buf,69);
	R4H160(a,b,c,d,e,buf,70); R4H160(e,a,b,c,d,buf,71);
	R4H160(d,e,a,b,c,buf,72); R4H160(c,d,e,a,b,buf,73);
	R4H160(b,c,d,e,a,buf,74); R4H160(a,b,c,d,e,buf,75);
	R4H160(e,a,b,c,d,buf,76); R4H160(d,e,a,b,c,buf,77);
	R4H160(c,d,e,a,b,buf,78); R4H160(b,c,d,e,a,buf,79);

	state[0] += a; state[1] += b; state[2] += c;
	state[3] += d; state[4] += e;
}

#define ROTATE_RIGHT32(x, n) (((x) >> (n)) | ((x) << (32-(n))))

#define CH32(x,y,z) ((z) ^ ((x) & ((y) ^ (z))))
#define MAJ32(x,y,z) (((x) & (y)) | ((z) & ((x) | (y))))

#define E032(x) (ROTATE_RIGHT32(x, 2) ^ ROTATE_RIGHT32(x,13) ^ ROTATE_RIGHT32(x,22))
#define E132(x) (ROTATE_RIGHT32(x, 6) ^ ROTATE_RIGHT32(x,11) ^ ROTATE_RIGHT32(x,25))
#define S032(x) (ROTATE_RIGHT32(x, 7) ^ ROTATE_RIGHT32(x,18) ^ ((x) >> 3))
#define S132(x) (ROTATE_RIGHT32(x,17) ^ ROTATE_RIGHT32(x,19) ^ ((x) >> 10))

#define BLEND32(x, i) (x)[i] = S132((x)[i-2]) + (x)[i-7] + S032((x)[i-15]) + (x)[i-16]

#define R32(a,b,c,d,e,f,g,h,t1,t2,x,ac) \
	(t1) = (h) + E132(e) + CH32(e,f,g) + ac + x; \
	(t2) = E032(a) + MAJ32(a,b,c); (d)+=(t1); (h)=(t1)+(t2);

void cSHA256::transform(unsigned long* x) {
	unsigned long buf[64] = {0};
	#ifdef LITTLE_ENDIAN
	// на x86 компах требуется другой порядок байт
	{	// FLIPDWORD16 (destination,source)
		void* dest = (void*)(buf);
		void* src = (void*)(x);
		__asm__ __volatile__ (
            "movl $16, %%ecx\n\t"
            "xorl %%edx, %%edx\n\t"
        "1:"
			"movl (%%edi,%%edx), %%eax\n\t"
			"bswap %%eax\n\t"
			"movl %%eax, (%%esi,%%edx)\n\t"
			"addl $4, %%edx\n\t"
		"loop 1b\n\t"
		: : "S" (dest), "D" (src)
		: "%eax", "%ecx", "%edx"
		);
	}
	#else
	CopyMemory(buf,x,64);
	#endif

	for (int i = 16; i < 64; i++)
		BLEND32(buf, i);

	unsigned long a = state[0], b = state[1],
	c = state[2], d = state[3], e = state[4],
	f = state[5], g = state[6], h = state[7];
	unsigned long t1,t2;

	R32(a,b,c,d,e,f,g,h,t1,t2,buf[ 0],0x428a2f98);
	R32(h,a,b,c,d,e,f,g,t1,t2,buf[ 1],0x71374491);
	R32(g,h,a,b,c,d,e,f,t1,t2,buf[ 2],0xb5c0fbcf);
	R32(f,g,h,a,b,c,d,e,t1,t2,buf[ 3],0xe9b5dba5);
	R32(e,f,g,h,a,b,c,d,t1,t2,buf[ 4],0x3956c25b);
	R32(d,e,f,g,h,a,b,c,t1,t2,buf[ 5],0x59f111f1);
	R32(c,d,e,f,g,h,a,b,t1,t2,buf[ 6],0x923f82a4);
	R32(b,c,d,e,f,g,h,a,t1,t2,buf[ 7],0xab1c5ed5);

	R32(a,b,c,d,e,f,g,h,t1,t2,buf[ 8],0xd807aa98);
	R32(h,a,b,c,d,e,f,g,t1,t2,buf[ 9],0x12835b01);
	R32(g,h,a,b,c,d,e,f,t1,t2,buf[10],0x243185be);
	R32(f,g,h,a,b,c,d,e,t1,t2,buf[11],0x550c7dc3);
	R32(e,f,g,h,a,b,c,d,t1,t2,buf[12],0x72be5d74);
	R32(d,e,f,g,h,a,b,c,t1,t2,buf[13],0x80deb1fe);
	R32(c,d,e,f,g,h,a,b,t1,t2,buf[14],0x9bdc06a7);
	R32(b,c,d,e,f,g,h,a,t1,t2,buf[15],0xc19bf174);

	R32(a,b,c,d,e,f,g,h,t1,t2,buf[16],0xe49b69c1);
	R32(h,a,b,c,d,e,f,g,t1,t2,buf[17],0xefbe4786);
	R32(g,h,a,b,c,d,e,f,t1,t2,buf[18],0x0fc19dc6);
	R32(f,g,h,a,b,c,d,e,t1,t2,buf[19],0x240ca1cc);
	R32(e,f,g,h,a,b,c,d,t1,t2,buf[20],0x2de92c6f);
	R32(d,e,f,g,h,a,b,c,t1,t2,buf[21],0x4a7484aa);
	R32(c,d,e,f,g,h,a,b,t1,t2,buf[22],0x5cb0a9dc);
	R32(b,c,d,e,f,g,h,a,t1,t2,buf[23],0x76f988da);

	R32(a,b,c,d,e,f,g,h,t1,t2,buf[24],0x983e5152);
	R32(h,a,b,c,d,e,f,g,t1,t2,buf[25],0xa831c66d);
	R32(g,h,a,b,c,d,e,f,t1,t2,buf[26],0xb00327c8);
	R32(f,g,h,a,b,c,d,e,t1,t2,buf[27],0xbf597fc7);
	R32(e,f,g,h,a,b,c,d,t1,t2,buf[28],0xc6e00bf3);
	R32(d,e,f,g,h,a,b,c,t1,t2,buf[29],0xd5a79147);
	R32(c,d,e,f,g,h,a,b,t1,t2,buf[30],0x06ca6351);
	R32(b,c,d,e,f,g,h,a,t1,t2,buf[31],0x14292967);

	R32(a,b,c,d,e,f,g,h,t1,t2,buf[32],0x27b70a85);
	R32(h,a,b,c,d,e,f,g,t1,t2,buf[33],0x2e1b2138);
	R32(g,h,a,b,c,d,e,f,t1,t2,buf[34],0x4d2c6dfc);
	R32(f,g,h,a,b,c,d,e,t1,t2,buf[35],0x53380d13);
	R32(e,f,g,h,a,b,c,d,t1,t2,buf[36],0x650a7354);
	R32(d,e,f,g,h,a,b,c,t1,t2,buf[37],0x766a0abb);
	R32(c,d,e,f,g,h,a,b,t1,t2,buf[38],0x81c2c92e);
	R32(b,c,d,e,f,g,h,a,t1,t2,buf[39],0x92722c85);

	R32(a,b,c,d,e,f,g,h,t1,t2,buf[40],0xa2bfe8a1);
	R32(h,a,b,c,d,e,f,g,t1,t2,buf[41],0xa81a664b);
	R32(g,h,a,b,c,d,e,f,t1,t2,buf[42],0xc24b8b70);
	R32(f,g,h,a,b,c,d,e,t1,t2,buf[43],0xc76c51a3);
	R32(e,f,g,h,a,b,c,d,t1,t2,buf[44],0xd192e819);
	R32(d,e,f,g,h,a,b,c,t1,t2,buf[45],0xd6990624);
	R32(c,d,e,f,g,h,a,b,t1,t2,buf[46],0xf40e3585);
	R32(b,c,d,e,f,g,h,a,t1,t2,buf[47],0x106aa070);

	R32(a,b,c,d,e,f,g,h,t1,t2,buf[48],0x19a4c116);
	R32(h,a,b,c,d,e,f,g,t1,t2,buf[49],0x1e376c08);
	R32(g,h,a,b,c,d,e,f,t1,t2,buf[50],0x2748774c);
	R32(f,g,h,a,b,c,d,e,t1,t2,buf[51],0x34b0bcb5);
	R32(e,f,g,h,a,b,c,d,t1,t2,buf[52],0x391c0cb3);
	R32(d,e,f,g,h,a,b,c,t1,t2,buf[53],0x4ed8aa4a);
	R32(c,d,e,f,g,h,a,b,t1,t2,buf[54],0x5b9cca4f);
	R32(b,c,d,e,f,g,h,a,t1,t2,buf[55],0x682e6ff3);

	R32(a,b,c,d,e,f,g,h,t1,t2,buf[56],0x748f82ee);
	R32(h,a,b,c,d,e,f,g,t1,t2,buf[57],0x78a5636f);
	R32(g,h,a,b,c,d,e,f,t1,t2,buf[58],0x84c87814);
	R32(f,g,h,a,b,c,d,e,t1,t2,buf[59],0x8cc70208);
	R32(e,f,g,h,a,b,c,d,t1,t2,buf[60],0x90befffa);
	R32(d,e,f,g,h,a,b,c,t1,t2,buf[61],0xa4506ceb);
	R32(c,d,e,f,g,h,a,b,t1,t2,buf[62],0xbef9a3f7);
	R32(b,c,d,e,f,g,h,a,t1,t2,buf[63],0xc67178f2);

	state[0] += a; state[1] += b; state[2] += c;
	state[3] += d; state[4] += e; state[5] += f;
	state[6] += g; state[7] += h;
}

#define ROTATE_RIGHT64(x, n) (((x) >> (n)) | ((x) << (64-(n))))

#define CH64(x,y,z) ((z) ^ ((x) & ((y) ^ (z))))
#define MAJ64(x,y,z) (((x) & (y)) | ((z) & ((x) | (y))))

#define E064(x) (ROTATE_RIGHT64(x,28) ^ ROTATE_RIGHT64(x,34) ^ ROTATE_RIGHT64(x,39))
#define E164(x) (ROTATE_RIGHT64(x,14) ^ ROTATE_RIGHT64(x,18) ^ ROTATE_RIGHT64(x,41))
#define S064(x) (ROTATE_RIGHT64(x, 1) ^ ROTATE_RIGHT64(x, 8) ^ ((x) >> 7))
#define S164(x) (ROTATE_RIGHT64(x,19) ^ ROTATE_RIGHT64(x,61) ^ ((x) >> 6))

#define BLEND64(x, i) (x)[i] = S164((x)[i-2]) + (x)[i-7] + S064((x)[i-15]) + (x)[i-16]

#define R64(a,b,c,d,e,f,g,h,t1,t2,x,ac) \
	(t1) = (h) + E164(e) + CH64(e,f,g) + ac + x; \
	(t2) = E064(a) + MAJ64(a,b,c); (d)+=(t1); (h)=(t1)+(t2);

static const uint64_t sha512key[80] = {
	0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL,
	0xe9b5dba58189dbbcULL, 0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
	0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL, 0xd807aa98a3030242ULL,
	0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
	0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL,
	0xc19bf174cf692694ULL, 0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
	0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL, 0x2de92c6f592b0275ULL,
	0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
	0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL,
	0xbf597fc7beef0ee4ULL, 0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
	0x06ca6351e003826fULL, 0x142929670a0e6e70ULL, 0x27b70a8546d22ffcULL,
	0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
	0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL,
	0x92722c851482353bULL, 0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
	0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL, 0xd192e819d6ef5218ULL,
	0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
	0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL,
	0x34b0bcb5e19b48a8ULL, 0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
	0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL, 0x748f82ee5defb2fcULL,
	0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
	0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL,
	0xc67178f2e372532bULL, 0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
	0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL, 0x06f067aa72176fbaULL,
	0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
	0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL,
	0x431d67c49c100d4cULL, 0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
	0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

void cSHA512::transform(uint64_t* x) {
	uint64_t buf[80] = {0};
	#ifdef LITTLE_ENDIAN
	// на x86 компах требуется другой порядок байт
	{	// FLIPQWORD16 (destination,source)
		void* dest = (void*)(buf);
		void* src = (void*)(x);
		__asm__ __volatile__ (
            "movl $16, %%ecx\n\t"
            "xorl %%edx, %%edx\n\t"
        "1:"
			"movl (%%edi,%%edx), %%eax\n\t"
			"movl 4(%%edi,%%edx), %%ebx\n\t"
			"bswap %%eax\n\t"
			"bswap %%ebx\n\t"
			"movl %%ebx, (%%esi,%%edx)\n\t"
			"movl %%eax, 4(%%esi,%%edx)\n\t"
			"addl $8, %%edx\n\t"
		"loop 1b\n\t"
		: : "S" (dest), "D" (src)
		: "%eax", "%ecx", "%edx", "%ebx"
		);
	}
	#else
	CopyMemory(buf,x,128);
	#endif

	for (int i = 16; i < 80; i++)
		BLEND64(buf, i);

    uint64_t a = state[0], b = state[1],
	c = state[2], d = state[3], e = state[4],
	f = state[5], g = state[6], h = state[7];
	uint64_t t1,t2;

	for (int i=0; i<80; i+=8) {
		R64(a,b,c,d,e,f,g,h,t1,t2,buf[i  ],sha512key[i  ]);
		R64(h,a,b,c,d,e,f,g,t1,t2,buf[i+1],sha512key[i+1]);
		R64(g,h,a,b,c,d,e,f,t1,t2,buf[i+2],sha512key[i+2]);
		R64(f,g,h,a,b,c,d,e,t1,t2,buf[i+3],sha512key[i+3]);
		R64(e,f,g,h,a,b,c,d,t1,t2,buf[i+4],sha512key[i+4]);
		R64(d,e,f,g,h,a,b,c,t1,t2,buf[i+5],sha512key[i+5]);
		R64(c,d,e,f,g,h,a,b,t1,t2,buf[i+6],sha512key[i+6]);
		R64(b,c,d,e,f,g,h,a,t1,t2,buf[i+7],sha512key[i+7]);
	}

	state[0] += a; state[1] += b; state[2] += c;
	state[3] += d; state[4] += e; state[5] += f;
	state[6] += g; state[7] += h;
}
