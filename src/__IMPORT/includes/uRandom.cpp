#include <windows.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "uRandom.h"
#include "uMemory.h"
#include "uCPU.h"

/****************************
    Linear                  */

static volatile uint32_t linear_seed = GetTickCount();

void linear_init(uint32_t seed) {
    linear_seed = seed;
}

uint32_t linear_random() {
    uint64_t r;
    r = int64_t(linear_seed)*1664525+1013904223;
    linear_seed = int32_t(r);
    return linear_seed;
}

/****************************
    MotherOfAll             */

static volatile long double mother_seed[5];

void mother_init(uint32_t seed) {
    uint64_t s;
    s = seed;
    for (int i = 0; i <= 4; i++) {
        s = uint32_t(s*1664525+1013904223);
        mother_seed[i] = s/4294967296LL;
    }
}

long double mother_random() {
    long double b,c;
    b = 2111111111.0;
    c = b*mother_seed[3] + 1492*mother_seed[2] + 1776*mother_seed[1] + 5115*mother_seed[0] + mother_seed[4];
	mother_seed[4] = floorl(c);
	mother_seed[3] = mother_seed[2];
	mother_seed[2] = mother_seed[1];
	mother_seed[1] = mother_seed[0];
	mother_seed[0] = c - mother_seed[4];
	mother_seed[4] = mother_seed[4]/4294967296LL;
	return mother_seed[0];
}

/*********************************
    Kiss (Keep It Simple Stupid) */

static volatile uint32_t kiss_x = 123456789;
static volatile uint32_t kiss_y = 362436000;
static volatile uint32_t kiss_z = 521288629;
static volatile uint32_t kiss_c = 7654321;

void kiss_init(uint32_t seed) {
    linear_init(seed);
    kiss_x = linear_random();
    kiss_y = linear_random();
    kiss_z = linear_random();
    kiss_c = linear_random();
}

uint32_t kiss_random() {
    uint64_t t;
    kiss_x = uint64_t(69069)*kiss_x + 12345;
	kiss_y = kiss_y^(kiss_y<<13);
	kiss_y = kiss_y^(kiss_y>>17);
	kiss_y = kiss_y^(kiss_y<<5);
	t = uint64_t(698769069)*kiss_z + kiss_c;
	kiss_c = t>>32;
	kiss_z = t;
	return kiss_x + kiss_y + kiss_z;
}

/********************************
    Random                      */

void rndSeed(uint32_t seed) {
    kiss_init(seed);
}

uint32_t rndRand() {
    return kiss_random();
}

uint32_t rndRand(uint32_t max) {
//    return kiss_random() % max;
    uint64_t t = kiss_random();
    return ((t*max)>>32);
}

///*******************************************************

void rndSeed(uint32_t seed, struct tKissSeed* kissSeed) {
    uint64_t r;
    r = ((int64_t)seed)*1664525+1013904223;
    kissSeed->kiss_x = seed = ((int32_t)r);
    r = ((int64_t)seed)*1664525+1013904223;
    kissSeed->kiss_y = seed = ((int32_t)r);
    r = ((int64_t)seed)*1664525+1013904223;
    kissSeed->kiss_z = seed = ((int32_t)r);
    r = ((int64_t)seed)*1664525+1013904223;
    kissSeed->kiss_c = seed = ((int32_t)r);
}

uint32_t rndRand(struct tKissSeed* kissSeed) {
    uint64_t t;
    kissSeed->kiss_x = ((uint64_t)69069)*kissSeed->kiss_x + 12345;
	kissSeed->kiss_y = kissSeed->kiss_y^(kissSeed->kiss_y<<13);
	kissSeed->kiss_y = kissSeed->kiss_y^(kissSeed->kiss_y>>17);
	kissSeed->kiss_y = kissSeed->kiss_y^(kissSeed->kiss_y<<5);
	t = ((uint64_t)698769069)*kissSeed->kiss_z + kissSeed->kiss_c;
	kissSeed->kiss_c = t>>32;
	kissSeed->kiss_z = t;
	return kissSeed->kiss_x + kissSeed->kiss_y + kissSeed->kiss_z;
}

int rndMD5(const wchar_t* ptr) {
    int i;
    wchar_t* tmp = (wchar_t*)ptr;
    for (i = 31; i >= 0; i--) {
        wchar_t wch = *(tmp+i);
        if (((wch >= L'0') && (wch <= L'9')) ||
            ((wch >= L'a') && (wch <= L'f')) ||
            ((wch >= L'A') && (wch <= L'F')))
        {
//          tmp++;
        } else
            return 0;
    }
    return 1;
}

void setSeed(const wchar_t* seedHash, struct tKissSeed* kissSeed) {
    uint16_t h,l;
    uint16_t* src = (uint16_t*)seedHash;
    uint8_t* dst = (uint8_t*)kissSeed;

    if ((seedHash == 0) || (rndMD5(seedHash) == 0)) {
        rndSeed(time(0) + GetTickCount() + RDTSC(), kissSeed);
    } else {
        int len = wcslen(seedHash);
        while (len > 0) {
            h = *src; len--; src++;
            l = *src; len--; src++;
            if ((h >= L'0') && (h <= L'9')) h -= L'0';
            else if ((h >= L'A') && (h <= L'F')) h -= (L'A'-10);
            else if ((h >= L'a') && (h <= L'f')) h -= (L'a'-10);
            else h = 0;
            if ((l >= L'0') && (l <= L'9')) l -= L'0';
            else if ((l >= L'A') && (l <= L'F')) l -= (L'A'-10);
            else if ((l >= L'a') && (l <= L'f')) l -= (L'a'-10);
            else l = 0;
            l |= (h<<4);

            *dst = l;

            dst++;
        }
    }
}

void getSeed(wchar_t* seedHash, struct tKissSeed* kissSeed) {
    uint16_t h,l;
    uint8_t* src = (uint8_t*)kissSeed;
    uint16_t* dst = (uint16_t*)seedHash;
    int len = 32;

    while (len > 0) {
        l = *src; src++;

        h = (l >> 4);
        if (h < 10) h += L'0';
        else h += (L'A'-10);
        *dst = h; dst++; len--;
        l &= 0x0F;
        if (l < 10) l += L'0';
        else l += (L'A'-10);
        *dst = l; dst++; len--;
    }
    *dst = 0;
}
