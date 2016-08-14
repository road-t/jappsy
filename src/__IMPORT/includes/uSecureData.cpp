#include <windows.h>

#include "uMemory.h"
#include "uSecureData.h"

/* CRC-TABLE CRCINIT=0xFFFFFFFF POLY=0xEDB88320 ANSI X3.66  */
const uint32_t crc32_lookup[256] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t atom_crc32(void* data, uint32_t len) {
	uint32_t crc = 0xFFFFFFFF;
	uint32_t* buf = (uint32_t*)data;
    len = (len + 3) >> 2;
	for (int i = len; i > 0; i--) {
	    uint32_t val = InterlockedCompareExchange((volatile LONG*)buf, 0, 0);
		crc = (uint32_t)(crc << 8) ^ (uint32_t)(crc32_lookup[ ((crc >> 24) ^ val) & 0xFF ]);
		val >>= 8;
		crc = (uint32_t)(crc << 8) ^ (uint32_t)(crc32_lookup[ ((crc >> 24) ^ val) & 0xFF ]);
		val >>= 8;
		crc = (uint32_t)(crc << 8) ^ (uint32_t)(crc32_lookup[ ((crc >> 24) ^ val) & 0xFF ]);
		val >>= 8;
		crc = (uint32_t)(crc << 8) ^ (uint32_t)(crc32_lookup[ ((crc >> 24) ^ val) & 0xFF ]);
		buf++;
	}
	return crc;
}

void atom_bzero(void* data, uint32_t len) {
	uint32_t* buf = (uint32_t*)data;
	uint32_t vxor = (uint32_t)data ^ 0x1C8CFBFF;
    len = (len + 3) >> 2;
	for (int i = len; i > 0; i--) {
	    InterlockedExchange((volatile LONG*)buf, vxor);
	    buf++;
	}
}

void atom_set(void* dest, const void* src, uint32_t len) {
	uint32_t* d = (uint32_t*)dest;
	uint32_t* s = (uint32_t*)src;
	uint32_t dxor = (uint32_t)dest ^ 0x1C8CFBFF;
    len = (len + 3) >> 2;
	for (int i = len; i > 0; i--) {
	    dxor ^= InterlockedCompareExchange((volatile LONG*)s, 0, 0);
	    InterlockedExchange((volatile LONG*)d, dxor);
	    s++; d++;
	}
}

void atom_get(void* dest, const void* src, uint32_t len) {
	uint32_t* d = (uint32_t*)dest;
	uint32_t* s = (uint32_t*)src;
	uint32_t sxor = (uint32_t)src ^ 0x1C8CFBFF;
    len = (len + 3) >> 2;
	for (int i = len; i > 0; i--) {
	    uint32_t val = InterlockedCompareExchange((volatile LONG*)s, 0, 0);
	    InterlockedExchange((volatile LONG*)d, val ^ sxor);
	    sxor = val;
	    s++; d++;
	}
}

void atom_memcpy(void* dest, const void* src, uint32_t len) {
	uint32_t* d = (uint32_t*)dest;
	uint32_t* s = (uint32_t*)src;
	uint32_t dxor = (uint32_t)dest ^ 0x1C8CFBFF;
	uint32_t sxor = (uint32_t)src ^ 0x1C8CFBFF;
    len = (len + 3) >> 2;
	for (int i = len; i > 0; i--) {
	    uint32_t val = InterlockedCompareExchange((volatile LONG*)s, 0, 0);
	    dxor ^= (val ^ sxor);
	    InterlockedExchange((volatile LONG*)d, dxor);
	    sxor = val;
	    s++; d++;
	}
}

///============================

void sec_abstract::update_crc() {
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t len = InterlockedCompareExchange((volatile LONG*)&(p->len), 0, 0);
        uint32_t crc = atom_crc32(data, len);
        InterlockedExchange((volatile LONG*)&(p->crc), crc);
    }
}

bool sec_abstract::check_crc() {
    bool mark[SEC_DATA_CNT];
    int normal_id = 0;
    uint32_t check = 0;
    uint32_t fail = 0;
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t len = InterlockedCompareExchange((volatile LONG*)&(p->len), 0, 0);
        uint32_t crc = atom_crc32(data, len);
        uint32_t data_crc = InterlockedCompareExchange((volatile LONG*)&(p->crc), 0, 0);
        if (data_crc != crc) {
            mark[i] = true;
            fail++;
        } else {
            mark[i] = false;
            if (check == 0) {
                normal_id = i;
                check = crc;
            }
        }
    }
    // Данные испорчены во всех хранилищах - критическая ошибка без восстановления
    if (fail > (SEC_DATA_CNT-2)) {
        return false;
    } else
    // Данные испорчены не во всех хранилищах - подлежат восстановлению
    if (fail > 0) {
        sec_data* n_p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[normal_id]), 0, 0);
        uint32_t n_len = InterlockedCompareExchange((volatile LONG*)&(n_p->len), 0, 0);
        void* n_data = (void*)InterlockedCompareExchange((volatile LONG*)&(n_p->data), 0, 0);
        for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
            if ((i != normal_id) && (mark[i])) {
                sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
                void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
                uint32_t len = InterlockedCompareExchange((volatile LONG*)&(p->len), 0, 0);
                if (len != n_len) { // вероятно испорчена ссылка на данные - игнорируем старый указатель и выделяем новую память
                    data = memAlloc(void, data, n_len);
                    InterlockedExchange((volatile LONG*)&(p->len), n_len);
                    InterlockedExchange((volatile LONG*)&(p->data), (uint32_t)data);
                }
                atom_memcpy(data, n_data, n_len);
            }
        }
        update_crc();
    }
    return true;
}

void sec_abstract::constructor(int size) {
    memCall( lock = slCreateSingleLock(false); )
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        // Выделяем память
        sec_data* p = memAlloc(sec_data, p, sizeof(sec_data));
        size = (size + 3) & (~3);
        void* data = memAlloc(void, data, size);
        // Заполняем данными
        InterlockedExchange((volatile LONG*)&(ptr[i]), (uint32_t)p);
        InterlockedExchange((volatile LONG*)&(p->len), size);
        InterlockedExchange((volatile LONG*)&(p->data), (uint32_t)data);
        atom_bzero(data, size);
    }
    update_crc();
}

void sec_abstract::destructor() {
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t len = InterlockedCompareExchange((volatile LONG*)&(p->len), 0, 0);
        atom_bzero(data, len);
        memFree(data);
        memFree(p);
    }
    slCloseSingleLock(lock);
}

///============================

sec_int32_t::sec_int32_t() {
    memCall( constructor(sizeof(uint32_t)); )
}

sec_int32_t::~sec_int32_t() {
    destructor();
}

sec_int32_t::sec_int32_t(const int val) {
    memCall( constructor(sizeof(uint32_t)); )
    this->operator =(val);
}

sec_int32_t& sec_int32_t::operator =(const int val) {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        InterlockedExchange((volatile LONG*)data, val ^ dxor);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int32_t& sec_int32_t::operator +=(const int val) {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        uint32_t dval = InterlockedCompareExchange((volatile LONG*)data, 0, 0);
        dval ^= dxor;
        dval += val;
        dval ^= dxor;
        InterlockedExchange((volatile LONG*)data, dval);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int32_t& sec_int32_t::operator ++() {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        uint32_t dval = InterlockedCompareExchange((volatile LONG*)data, 0, 0);
        dval ^= dxor;
        dval++;
        dval ^= dxor;
        InterlockedExchange((volatile LONG*)data, dval);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int32_t& sec_int32_t::operator ++(const int val) {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        uint32_t dval = InterlockedCompareExchange((volatile LONG*)data, 0, 0);
        dval ^= dxor;
        dval += (val + 1);
        dval ^= dxor;
        InterlockedExchange((volatile LONG*)data, dval);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int32_t& sec_int32_t::operator -=(const int val) {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        uint32_t dval = InterlockedCompareExchange((volatile LONG*)data, 0, 0);
        dval ^= dxor;
        dval -= val;
        dval ^= dxor;
        InterlockedExchange((volatile LONG*)data, dval);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int32_t& sec_int32_t::operator --() {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        uint32_t dval = InterlockedCompareExchange((volatile LONG*)data, 0, 0);
        dval ^= dxor;
        dval--;
        dval ^= dxor;
        InterlockedExchange((volatile LONG*)data, dval);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int32_t& sec_int32_t::operator --(const int val) {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        uint32_t dval = InterlockedCompareExchange((volatile LONG*)data, 0, 0);
        dval ^= dxor;
        dval -= (val + 1);
        dval ^= dxor;
        InterlockedExchange((volatile LONG*)data, dval);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int32_t::operator int() {
    int32_t val = 0;
    slSingleLock(lock);
    if (check_crc()) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[0]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        val = dxor ^ InterlockedCompareExchange((volatile LONG*)data, 0, 0);
    }
    slSingleUnlock(lock);
    return val;
}

///============================

sec_int64_t::sec_int64_t() {
    memCall( constructor(sizeof(int64_t)); )
}

sec_int64_t::~sec_int64_t() {
    destructor();
}

sec_int64_t::sec_int64_t(const int64_t val) {
    memCall( constructor(sizeof(int64_t)); )
    this->operator =(val);
}

sec_int64_t& sec_int64_t::operator =(const int64_t val) {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        uint32_t* data = (uint32_t*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        dxor ^= (val & 0xFFFFFFFF);
        InterlockedExchange((volatile LONG*)data, dxor);
        dxor ^= (val >> 32);
        InterlockedExchange((volatile LONG*)(data+1), dxor);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int64_t& sec_int64_t::operator +=(const int64_t val) {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        uint32_t* data = (uint32_t*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        uint32_t dval1 = InterlockedCompareExchange((volatile LONG*)data, 0, 0);
        uint32_t dval2 = InterlockedCompareExchange((volatile LONG*)(data+1), 0, 0);
        dval2 ^= dval1;
        dval1 ^= dxor;
        uint64_t dval = (((uint64_t)dval2) << 32) | dval1;

        dval += val;

        dxor ^= (dval & 0xFFFFFFFF);
        InterlockedExchange((volatile LONG*)data, dxor);
        dxor ^= (dval >> 32);
        InterlockedExchange((volatile LONG*)(data+1), dxor);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int64_t& sec_int64_t::operator ++() {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        uint32_t* data = (uint32_t*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        uint32_t dval1 = InterlockedCompareExchange((volatile LONG*)data, 0, 0);
        uint32_t dval2 = InterlockedCompareExchange((volatile LONG*)(data+1), 0, 0);
        dval2 ^= dval1;
        dval1 ^= dxor;
        uint64_t dval = (((uint64_t)dval2) << 32) | dval1;

        dval++;

        dxor ^= (dval & 0xFFFFFFFF);
        InterlockedExchange((volatile LONG*)data, dxor);
        dxor ^= (dval >> 32);
        InterlockedExchange((volatile LONG*)(data+1), dxor);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int64_t& sec_int64_t::operator ++(const int val) {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        uint32_t* data = (uint32_t*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        uint32_t dval1 = InterlockedCompareExchange((volatile LONG*)data, 0, 0);
        uint32_t dval2 = InterlockedCompareExchange((volatile LONG*)(data+1), 0, 0);
        dval2 ^= dval1;
        dval1 ^= dxor;
        uint64_t dval = (((uint64_t)dval2) << 32) | dval1;

        dval += (val+1);

        dxor ^= (dval & 0xFFFFFFFF);
        InterlockedExchange((volatile LONG*)data, dxor);
        dxor ^= (dval >> 32);
        InterlockedExchange((volatile LONG*)(data+1), dxor);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int64_t& sec_int64_t::operator -=(const int64_t val) {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        uint32_t* data = (uint32_t*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        uint32_t dval1 = InterlockedCompareExchange((volatile LONG*)data, 0, 0);
        uint32_t dval2 = InterlockedCompareExchange((volatile LONG*)(data+1), 0, 0);
        dval2 ^= dval1;
        dval1 ^= dxor;
        uint64_t dval = (((uint64_t)dval2) << 32) | dval1;

        dval -= val;

        dxor ^= (dval & 0xFFFFFFFF);
        InterlockedExchange((volatile LONG*)data, dxor);
        dxor ^= (dval >> 32);
        InterlockedExchange((volatile LONG*)(data+1), dxor);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int64_t& sec_int64_t::operator --() {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        uint32_t* data = (uint32_t*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        uint32_t dval1 = InterlockedCompareExchange((volatile LONG*)data, 0, 0);
        uint32_t dval2 = InterlockedCompareExchange((volatile LONG*)(data+1), 0, 0);
        dval2 ^= dval1;
        dval1 ^= dxor;
        uint64_t dval = (((uint64_t)dval2) << 32) | dval1;

        dval--;

        dxor ^= (dval & 0xFFFFFFFF);
        InterlockedExchange((volatile LONG*)data, dxor);
        dxor ^= (dval >> 32);
        InterlockedExchange((volatile LONG*)(data+1), dxor);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int64_t& sec_int64_t::operator --(const int val) {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        uint32_t* data = (uint32_t*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        uint32_t dval1 = InterlockedCompareExchange((volatile LONG*)data, 0, 0);
        uint32_t dval2 = InterlockedCompareExchange((volatile LONG*)(data+1), 0, 0);
        dval2 ^= dval1;
        dval1 ^= dxor;
        uint64_t dval = (((uint64_t)dval2) << 32) | dval1;

        dval -= (val+1);

        dxor ^= (dval & 0xFFFFFFFF);
        InterlockedExchange((volatile LONG*)data, dxor);
        dxor ^= (dval >> 32);
        InterlockedExchange((volatile LONG*)(data+1), dxor);
    }
    update_crc();
    slSingleUnlock(lock);
    return *this;
}

sec_int64_t::operator int64_t() {
    int64_t val = 0;
    slSingleLock(lock);
    if (check_crc()) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[0]), 0, 0);
        uint32_t* data = (uint32_t*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
        uint32_t dval1 = InterlockedCompareExchange((volatile LONG*)data, 0, 0);
        uint32_t dval2 = InterlockedCompareExchange((volatile LONG*)(data+1), 0, 0);
        dval2 ^= dval1;
        dval1 ^= dxor;
        val = (((uint64_t)dval2) << 32) | dval1;
    }
    slSingleUnlock(lock);
    return val;
}

///============================

sec_md5_t::sec_md5_t() {
    memCall( constructor(sizeof(wchar_t) * 32); )
}

sec_md5_t::~sec_md5_t() {
    destructor();
}

sec_md5_t::sec_md5_t(const wchar_t* val) {
    memCall( constructor(sizeof(wchar_t) * 32); )
    this->set(val);
}

void sec_md5_t::set(const wchar_t* val) {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t len = sizeof(wchar_t) * 32;
        atom_set(data, val, len);
    }
    update_crc();
    slSingleUnlock(lock);
}

void sec_md5_t::get(wchar_t* val) {
    slSingleLock(lock);
    if (check_crc()) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[0]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t len = sizeof(wchar_t) * 32;
        atom_get(val, data, len);
    }
    slSingleUnlock(lock);
}

///==========================

sec_text_t::sec_text_t() {
    length = 0;
    constructor(sizeof(wchar_t));
}

sec_text_t::~sec_text_t() {
    destructor();
}

sec_text_t::sec_text_t(const wchar_t* val) {
    length = wcslen(val);
    uint32_t len = sizeof(wchar_t) * (length + 1);
    constructor(len);
    this->set(val);
}

void sec_text_t::set(const wchar_t* val) {
    slSingleLock(lock);
    for (int i = SEC_DATA_CNT-1; i >= 0; i--) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[i]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t len = InterlockedCompareExchange((volatile LONG*)&(p->len), 0, 0);
        length = wcslen(val);
        uint32_t data_len = length * sizeof(wchar_t);
        uint32_t data_size = (data_len + 3) & (~3);
        uint32_t data_zsize = (data_len + 5) & (~3);
        if (data_zsize != len) {
            atom_bzero(data, len);
            void* newData = memAlloc(void, newData, data_zsize);
            if (newData != 0) {
                data = newData;
                len = data_zsize;
                InterlockedExchange((volatile LONG*)&(p->len), len);
                InterlockedExchange((volatile LONG*)&(p->data), (uint32_t)data);
            }
        }
        if (data_zsize <= len) {
            if (data_size < data_zsize) {
                uint32_t* d = (uint32_t*)data;
                uint32_t* s = (uint32_t*)val;
                uint32_t dxor = (uint32_t)data ^ 0x1C8CFBFF;
                for (int i = (data_size >> 2); i > 0; i--) {
                    dxor ^= InterlockedCompareExchange((volatile LONG*)s, 0, 0);
                    InterlockedExchange((volatile LONG*)d, dxor);
                    s++; d++;
                }
                InterlockedExchange((volatile LONG*)d, dxor);
            } else {
                atom_set(data, val, data_zsize);
            }
        } else {
            atom_set(data, val, len);
        }
    }
    update_crc();
    slSingleUnlock(lock);
}

wchar_t* sec_text_t::get() {
    wchar_t* res = 0;
    slSingleLock(lock);
    if (check_crc()) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[0]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t len = InterlockedCompareExchange((volatile LONG*)&(p->len), 0, 0);
        res = memAlloc(wchar_t, res, len);
        atom_get(res, data, len);
    }
    slSingleUnlock(lock);
    return res;
}

void sec_text_t::free(wchar_t* getResult) {
    if (getResult != 0) {
        uint32_t len = (wcslen(getResult) + 1) * sizeof(wchar_t);
        len = (len + 3) & (~3);
        atom_bzero(getResult, len);
        memFree(getResult);
    }
}

wchar_t sec_text_t::operator [](const int index) {
    wchar_t res = L'\0';
    slSingleLock(lock);
    if (check_crc()) {
        sec_data* p = (sec_data*)InterlockedCompareExchange((volatile LONG*)&(ptr[0]), 0, 0);
        void* data = (void*)InterlockedCompareExchange((volatile LONG*)&(p->data), 0, 0);
        uint32_t len = InterlockedCompareExchange((volatile LONG*)&(p->len), 0, 0);
        len >>= 1;

        if (index < 2) {
            uint32_t* s = (uint32_t*)data;
            uint32_t sxor = (uint32_t)data ^ 0x1C8CFBFF;
            uint32_t val = InterlockedCompareExchange((volatile LONG*)s, 0, 0);
            val ^= sxor;
            res = (val >> (index << 4));
        } else if (index < (int)len) {
            uint32_t* s = (uint32_t*)data;
            s += ((index >> 1) - 1);
            uint32_t sxor = InterlockedCompareExchange((volatile LONG*)s, 0, 0);
            s++;
            uint32_t val = InterlockedCompareExchange((volatile LONG*)s, 0, 0);
            val ^= sxor;
            res = (val >> ((index & 1) << 4));
        }
    }
    slSingleUnlock(lock);
    return res;
}

atom_int32_t::atom_int32_t(int val) {
    InterlockedExchange((volatile LONG*)&atom, val);
}

int atom_int32_t::operator=(int val) {
    InterlockedExchange((volatile LONG*)&atom, val);
    return val;
}

atom_int32_t::operator int() const {
    return InterlockedCompareExchange((volatile LONG*)&atom, 0, 0);
}

atom_uint32_t::atom_uint32_t(unsigned int val) {
    InterlockedExchange((volatile LONG*)&atom, val);
}

unsigned int atom_uint32_t::operator=(unsigned int val) {
    InterlockedExchange((volatile LONG*)&atom, val);
    return val;
}

atom_uint32_t::operator unsigned int() const {
    return InterlockedCompareExchange((volatile LONG*)&atom, 0, 0);
}

#if !defined (InterlockedOr)

#define InterlockedOr InterlockedOr_Inline

long __inline__ InterlockedOr_Inline (long volatile *Target, long Set) {
    long i,j;
    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange((long*)Target, i | Set, i);
    } while (i != j);
    return j;
}

#endif

#if !defined (InterlockedXor)

#define InterlockedXor InterlockedXor_Inline

long __inline__ InterlockedXor_Inline (long volatile *Target, long Set) {
    long i,j;
    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange((long*)Target, i ^ Set, i);
    } while (i != j);
    return j;
}

#endif

#if !defined (InterlockedNor)

#define InterlockedNor InterlockedNor_Inline

long __inline__ InterlockedNor_Inline (long volatile *Target, long Set) {
    long i,j;
    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange((long*)Target, i & (~Set), i);
    } while (i != j);
    return j;
}

#endif

#if !defined (InterlockedAnd)

#define InterlockedAnd InterlockedAnd_Inline

long __inline__ InterlockedAnd_Inline (long volatile *Target, long Set) {
    long i,j;
    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange((long*)Target, i & Set, i);
    } while (i != j);
    return j;
}

#endif

bool atom_uint32_t::NOT(unsigned int flags) {
	return ((InterlockedCompareExchange((volatile LONG*)&atom,0,0) & (flags)) == 0);
}

bool atom_uint32_t::IS(unsigned int flags) {
	return ((InterlockedCompareExchange((volatile LONG*)&atom,0,0) & (flags)) != 0);
}

unsigned int atom_uint32_t::OR(unsigned int flags) {
	return static_cast<uint32_t>( InterlockedOr((volatile LONG*)&atom, (flags)) );
}

unsigned int atom_uint32_t::XOR(unsigned int flags) {
	return static_cast<uint32_t>( InterlockedXor((volatile LONG*)&atom, (flags)) );
}

unsigned int atom_uint32_t::NOR(unsigned int flags) {
	return static_cast<uint32_t>( InterlockedNor((volatile LONG*)&atom, (flags)) );
}

unsigned int atom_uint32_t::AND(unsigned int flags) {
	return static_cast<uint32_t>( InterlockedAnd((volatile LONG*)&atom, (flags)) );
}

bool atom_uint32_t::SET(unsigned int flags) {
	return (OR(flags) & flags) == 0;
}

bool atom_uint32_t::RESET(unsigned int flags) {
	return (NOR(flags) & flags) != 0;
}

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

atom_uint64_t::atom_uint64_t(unsigned long long val) {
    InterlockedExchange64((volatile uint64_t*)&atom, val);
}

unsigned long long atom_uint64_t::operator=(unsigned long long val) {
    InterlockedExchange64((volatile uint64_t*)&atom, val);
    return val;
}

atom_uint64_t::operator unsigned long long() const {
    return InterlockedCompareExchange64((volatile uint64_t*)&atom, 0, 0);
}

#if !defined (InterlockedOr64)

#define InterlockedOr64 InterlockedOr64_Inline

long long __inline__ InterlockedOr64_Inline (long long volatile *Target, long long Set) {
    long long i,j;
    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange64((long long*)Target, i | Set, i);
    } while (i != j);
    return j;
}

#endif

#if !defined (InterlockedXor64)

#define InterlockedXor64 InterlockedXor64_Inline

long long __inline__ InterlockedXor64_Inline (long long volatile *Target, long long Set) {
    long long i,j;
    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange64((long long*)Target, i ^ Set, i);
    } while (i != j);
    return j;
}

#endif

#if !defined (InterlockedNor64)

#define InterlockedNor64 InterlockedNor64_Inline

long long __inline__ InterlockedNor64_Inline (long long volatile *Target, long long Set) {
    long long i,j;
    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange64((long long*)Target, i & (~Set), i);
    } while (i != j);
    return j;
}

#endif

#if !defined (InterlockedAnd64)

#define InterlockedAnd64 InterlockedAnd64_Inline

long long __inline__ InterlockedAnd64_Inline (long long volatile *Target, long long Set) {
    long long i,j;
    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange64((long long*)Target, i & Set, i);
    } while (i != j);
    return j;
}

#endif

bool atom_uint64_t::NOT(unsigned long long flags) {
	return ((InterlockedCompareExchange64((volatile uint64_t*)&atom,0,0) & (flags)) == 0);
}

bool atom_uint64_t::IS(unsigned long long flags) {
	return ((InterlockedCompareExchange64((volatile uint64_t*)&atom,0,0) & (flags)) != 0);
}

unsigned long long atom_uint64_t::OR(unsigned long long flags) {
	return static_cast<uint64_t>( InterlockedOr64((volatile long long*)&atom, (flags)) );
}

unsigned long long atom_uint64_t::XOR(unsigned long long flags) {
	return static_cast<uint64_t>( InterlockedXor64((volatile long long*)&atom, (flags)) );
}

unsigned long long atom_uint64_t::NOR(unsigned long long flags) {
	return static_cast<uint64_t>( InterlockedNor64((volatile long long*)&atom, (flags)) );
}

unsigned long long atom_uint64_t::AND(unsigned long long flags) {
	return static_cast<uint64_t>( InterlockedAnd64((volatile long long*)&atom, (flags)) );
}

bool atom_uint64_t::SET(unsigned long long flags) {
	return (OR(flags) & flags) == 0;
}

bool atom_uint64_t::RESET(unsigned long long flags) {
	return (NOR(flags) & flags) != 0;
}

atom_bool::atom_bool(bool val) {
    InterlockedExchange((volatile LONG*)&atom, val ? 0 : 1);
}

bool atom_bool::operator=(bool val) {
    InterlockedExchange((volatile LONG*)&atom, val ? 0 : 1);
    return val;
}

atom_bool::operator bool() const {
    return InterlockedCompareExchange((volatile LONG*)&atom, 0, 0) == 0;
}
