#ifndef uSecureDataH
#define uSecureDataH

#include <stdint.h>
#include "uSpinLock.h"

struct sec_data {
    volatile uint32_t len;
    volatile void* data;
    volatile uint32_t crc;
};

#define SEC_DATA_CNT    3

class sec_abstract {
public:
    slLock* lock;
    volatile sec_data* ptr[SEC_DATA_CNT];
    void constructor(int size);
    void destructor();
    void update_crc();
    bool check_crc();

//    sec_abstract();
//    ~sec_abstract();
};

class sec_int32_t : public sec_abstract {
public:
    sec_int32_t();
    ~sec_int32_t();

    sec_int32_t(const int val);
	sec_int32_t& operator =(const int val);
	sec_int32_t& operator +=(const int val);
	sec_int32_t& operator ++();
	sec_int32_t& operator ++(const int val);
	sec_int32_t& operator -=(const int val);
	sec_int32_t& operator --();
	sec_int32_t& operator --(const int val);
	operator int();
};

class sec_int64_t : public sec_abstract {
public:
    sec_int64_t();
    ~sec_int64_t();

    sec_int64_t(const int64_t val);
	sec_int64_t& operator =(const int64_t val);
	sec_int64_t& operator +=(const int64_t val);
	sec_int64_t& operator ++();
	sec_int64_t& operator ++(const int val);
	sec_int64_t& operator -=(const int64_t val);
	sec_int64_t& operator --();
	sec_int64_t& operator --(const int val);
	operator int64_t();
};

class sec_md5_t : public sec_abstract {
public:
    sec_md5_t();
    ~sec_md5_t();

    sec_md5_t(const wchar_t* val);
	void set(const wchar_t* val);
	void get(wchar_t* val);
};

class sec_text_t : public sec_abstract {
public:
    sec_text_t();
    ~sec_text_t();

	int length;

    sec_text_t(const wchar_t* val);
	void set(const wchar_t* val);
	wchar_t* get();
	void free(wchar_t* getResult);
	wchar_t operator [](const int index);
};

struct atom_int32_t {
    volatile int32_t atom;
    atom_int32_t(int val);
    int operator=(int val);
    operator int() const;
};

struct atom_uint32_t {
    volatile uint32_t atom;
    atom_uint32_t(unsigned int val);
    unsigned int operator=(unsigned int val);
    operator unsigned int() const;

    bool NOT(unsigned int flags);
    bool IS(unsigned int flags);
    unsigned int OR(unsigned int flags);
    unsigned int XOR(unsigned int flags);
    unsigned int NOR(unsigned int flags);
    unsigned int AND(unsigned int flags);
    bool SET(unsigned int flags);
    bool RESET(unsigned int flags);
};

struct atom_uint64_t {
    volatile uint64_t atom;
    atom_uint64_t(unsigned long long val);
    unsigned long long operator=(unsigned long long val);
    operator unsigned long long() const;

    bool NOT(unsigned long long flags);
    bool IS(unsigned long long flags);
    unsigned long long OR(unsigned long long flags);
    unsigned long long XOR(unsigned long long flags);
    unsigned long long NOR(unsigned long long flags);
    unsigned long long AND(unsigned long long flags);
    bool SET(unsigned long long flags);
    bool RESET(unsigned long long flags);
};

struct atom_bool {
    volatile int32_t atom;
    atom_bool(bool val);
    bool operator=(bool val);
    operator bool() const;
};

#endif // uSecureDataH
