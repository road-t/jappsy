#ifndef UHufCodeH
#define UHufCodeH

// Реализация зжатия данных методом Хуфмана
void* HUF_encode(const void* in, unsigned long insize, unsigned long* outsize);
void* HUF_encode(const void* in, unsigned long insize, void* out, unsigned long* outsize);
void* HUF_decode(const void* in, unsigned long insize, unsigned long* outsize);
void* HUF_decode(const void* in, unsigned long insize, void* out, unsigned long* outsize);

#endif

