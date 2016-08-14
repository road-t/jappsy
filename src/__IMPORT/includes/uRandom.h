#ifndef uRandomH
#define uRandomH

#include <stdint.h>

void rndSeed(uint32_t seed);
uint32_t rndRand();
uint32_t rndRand(uint32_t max);

struct tKissSeed {
    volatile uint32_t kiss_x;
    volatile uint32_t kiss_y;
    volatile uint32_t kiss_z;
    volatile uint32_t kiss_c;
};

void rndSeed(uint32_t seed, struct tKissSeed* kissSeed);
uint32_t rndRand(struct tKissSeed* kissSeed);
void setSeed(const wchar_t* seedHash, struct tKissSeed* kissSeed);
void getSeed(wchar_t* seedHash, struct tKissSeed* kissSeed);

#endif
