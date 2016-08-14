//---------------------------------------------------------------------------
#ifndef cSHAH
#define cSHAH
//---------------------------------------------------------------------------
#define cSHA1	cSHA160
#define cSHA2	cSHA256
//---------------------------------------------------------------------------
#include <stdint.h>

class cSHA160 {
private:
	uint64_t count;
	unsigned char part[64];
	void transform(unsigned long* x);
public:
    void* operator new(size_t) throw(const char*);
    void* operator new[](size_t) throw(const char*);
    void operator delete(void*);
    void operator delete[](void*);

	unsigned long state[5];
	cSHA160();
	~cSHA160();
	void init();
	void update(const char* buffer, unsigned long size);
	void final();
};
//---------------------------------------------------------------------------
class cSHA256 {
protected:
	uint64_t count;
	unsigned char part[64];
	void transform(unsigned long* x);
public:
    void* operator new(size_t) throw(const char*);
    void* operator new[](size_t) throw(const char*);
    void operator delete(void*);
    void operator delete[](void*);

	unsigned long state[8];
	cSHA256();
	~cSHA256();
	void init();
	void update(const char* buffer, unsigned long size);
	void final();
};
//---------------------------------------------------------------------------
class cSHA224 : public cSHA256 {
public:
	void init();
	void final();
};
//---------------------------------------------------------------------------
class cSHA512 {
protected:
	uint64_t count[2];
	unsigned char part[128];
	void transform(unsigned __int64* x);
public:
    void* operator new(size_t) throw(const char*);
    void* operator new[](size_t) throw(const char*);
    void operator delete(void*);
    void operator delete[](void*);

	uint64_t state[8];
	cSHA512();
	~cSHA512();
	void init();
	void update(const char* buffer, unsigned long size);
	void final();
};
//---------------------------------------------------------------------------
class cSHA384 : public cSHA512 {
public:
	void init();
	void final();
};
//---------------------------------------------------------------------------
#endif
