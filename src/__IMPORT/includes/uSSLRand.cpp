#include <windows.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/err.h>

#include "uSSLRand.h"
#include "cSHA.h"
#include "../core/uRandom.h"

#define ENTROPY_NEEDED 256  /* require 256 bits = 32 bytes of randomness */
#define SHA_DIGEST_LENGTH	64
#define STATE_SIZE	1023

static int state_num=0,state_index=0;
static unsigned char state[STATE_SIZE+SHA_DIGEST_LENGTH];
static unsigned char sha[SHA_DIGEST_LENGTH];
static unsigned __int64 sha_count[2]={0,0};
double RSA_entropy=0;
static int initialized=0;

static unsigned int crypto_lock_rand = 0; /* may be set only when a thread
                                           * holds CRYPTO_LOCK_RAND
                                           * (to prevent double locking) */
/* access to lockin_thread is synchronized by CRYPTO_LOCK_RAND2 */
static unsigned long locking_thread = 0; /* valid iff crypto_lock_rand is set */

int _bytes(unsigned char *buf, int num);

int RSA_r_bytes(unsigned char *buf, int num) {
	int ret = _bytes(buf,num);
	// ожидать в течении минуты
	if (ret == 0) {
		unsigned long timepast = 0;
		while ((ret = _bytes(buf,num)) == 0) {
			Sleep(100); timepast += 100;
			if (timepast > 60000) break;
		}
	}
	return ret;
}

int _bytes(unsigned char *buf, int num) {
	static volatile int stirred_pool = 0;
	int i,j,k,st_num,st_idx;
	int num_ceil;
	int ok;
	unsigned __int64 sha_c[2];
	unsigned char local_sha[SHA_DIGEST_LENGTH];
	cSHA512 sha512;
	int do_stir_pool = 0;

#ifdef PREDICT
	if (rand_predictable)
		{
		static unsigned char val=0;

		for (i=0; i<num; i++)
			buf[i]=val++;
		return(1);
		}
#endif

	if (num <= 0)
		return 1;

	/* round upwards to multiple of MD_DIGEST_LENGTH/2 */
	num_ceil = (1 + (num-1)/(SHA_DIGEST_LENGTH/2)) * (SHA_DIGEST_LENGTH/2);

	/*
	 * (Based on the rand(3) manpage:)
	 *
	 * For each group of 10 bytes (or less), we do the following:
	 *
	 * Input into the hash function the local 'md' (which is initialized from
	 * the global 'md' before any bytes are generated), the bytes that are to
	 * be overwritten by the random bytes, and bytes from the 'state'
	 * (incrementing looping index). From this digest output (which is kept
	 * in 'md'), the top (up to) 10 bytes are returned to the caller and the
	 * bottom 10 bytes are xored into the 'state'.
	 *
	 * Finally, after we have finished 'num' random bytes for the
	 * caller, 'count' (which is incremented) and the local and global 'md'
	 * are fed into the hash function and the results are kept in the
	 * global 'md'.
	 */

	CRYPTO_w_lock(CRYPTO_LOCK_RAND);

	/* prevent ssleay_rand_bytes() from trying to obtain the lock again */
	CRYPTO_w_lock(CRYPTO_LOCK_RAND2);
	locking_thread = CRYPTO_thread_id();
	CRYPTO_w_unlock(CRYPTO_LOCK_RAND2);
	crypto_lock_rand = 1;

	if (!initialized) {
//		RAND_poll();
		initialized = 1;
//		return 0;
	}

	if (!stirred_pool)
		do_stir_pool = 1;

	ok = (RSA_entropy >= ENTROPY_NEEDED);
	if (!ok) {
		/* If the PRNG state is not yet unpredictable, then seeing
		 * the PRNG output may help attackers to determine the new
		 * state; thus we have to decrease the entropy estimate.
		 * Once we've had enough initial seeding we don't bother to
		 * adjust the entropy count, though, because we're not ambitious
		 * to provide *information-theoretic* randomness.
		 *
		 * NOTE: This approach fails if the program forks before
		 * we have enough entropy. Entropy should be collected
		 * in a separate input pool and be transferred to the
		 * output pool only when the entropy limit has been reached.
		 */
//		entropy -= num;
		if (RSA_entropy < 0)
			RSA_entropy = 0;
	}

	if (do_stir_pool) {
		/* In the output function only half of 'md' remains secret,
		 * so we better make sure that the required entropy gets
		 * 'evenly distributed' through 'state', our randomness pool.
		 * The input function (ssleay_rand_add) chains all of 'md',
		 * which makes it more suitable for this purpose.
		 */

		int n = STATE_SIZE; /* so that the complete pool gets accessed */
		while (n > 0) {
#if SHA_DIGEST_LENGTH > 64
# error "Please adjust DUMMY_SEED."
#endif
#define DUMMY_SEED "................................................................" /* at least MD_DIGEST_LENGTH */
			/* Note that the seed does not matter, it's just that
			 * ssleay_rand_add expects to have something to hash. */
			RSA_r_add(DUMMY_SEED, SHA_DIGEST_LENGTH, 0.0);
			n -= SHA_DIGEST_LENGTH;
		}
		if (ok)
			stirred_pool = 1;
	}

	st_idx=state_index;
	st_num=state_num;
	sha_c[0] = sha_count[0];
	sha_c[1] = sha_count[1];
	memcpy(local_sha, sha, sizeof sha);

	state_index+=num_ceil;
	if (state_index > state_num)
		state_index %= state_num;

	/* state[st_idx], ..., state[(st_idx + num_ceil - 1) % st_num]
	 * are now ours (but other threads may use them too) */

	sha_count[0] += 1;

	/* before unlocking, we must clear 'crypto_lock_rand' */
	crypto_lock_rand = 0;
	CRYPTO_w_unlock(CRYPTO_LOCK_RAND);

	while (num > 0) {
		/* num_ceil -= MD_DIGEST_LENGTH/2 */
		j=(num >= SHA_DIGEST_LENGTH/2)?SHA_DIGEST_LENGTH/2:num;
		num-=j;
		sha512.init();
		sha512.update((char*)local_sha,SHA_DIGEST_LENGTH);
		sha512.update((char*)&(sha_c[0]),sizeof(sha_c));
#ifndef PURIFY
		sha512.update((char*)buf,j); /* purify complains */
#endif
		k=(st_idx+SHA_DIGEST_LENGTH/2)-st_num;
		if (k > 0) {
			sha512.update((char*)&(state[st_idx]),SHA_DIGEST_LENGTH/2-k);
			sha512.update((char*)&(state[0]),k);
		} else
			sha512.update((char*)&(state[st_idx]),SHA_DIGEST_LENGTH/2);
		sha512.final();
		memcpy(local_sha,sha512.state,SHA_DIGEST_LENGTH);

		for (i=0; i<SHA_DIGEST_LENGTH/2; i++) {
			state[st_idx++]^=local_sha[i]; /* may compete with other threads */
			if (st_idx >= st_num)
				st_idx=0;
			if (i < j)
				*(buf++)=local_sha[i+SHA_DIGEST_LENGTH/2];
		}
	}

	sha512.init();
	sha512.update((char *)&(sha_c[0]),sizeof(sha_c));
	sha512.update((char*)local_sha,SHA_DIGEST_LENGTH);
	CRYPTO_w_lock(CRYPTO_LOCK_RAND);
	sha512.update((char*)sha,SHA_DIGEST_LENGTH);
	sha512.final();
	memcpy(sha,sha512.state,SHA_DIGEST_LENGTH);
	CRYPTO_w_unlock(CRYPTO_LOCK_RAND);

	memset(&sha512,0,sizeof(sha512));
	if (ok)
		return(1);
	else {
		RANDerr(RAND_F_SSLEAY_RAND_BYTES,RAND_R_PRNG_NOT_SEEDED);
		ERR_add_error_data(1, "You need to read the OpenSSL FAQ, "
			"http://www.openssl.org/support/faq.html");
		return(0);
	}
}

void RSA_r_cleanup() {

	memset(state,0,sizeof(state));

//	OPENSSL_cleanse(state,sizeof(state));
	state_num=0;
	state_index=0;
	memset(sha,0,sizeof(sha));
//	OPENSSL_cleanse(sha,SHA_DIGEST_LENGTH);
	sha_count[0]=0;
	sha_count[1]=0;
	RSA_entropy=0;
	initialized=0;
}

void RSA_r_add(const void *buf, int num, double add) {

	int i,j,k,st_idx;
	unsigned __int64 sha_c[2];
	unsigned char local_sha[SHA_DIGEST_LENGTH];
	cSHA512 sha512;
	int do_not_lock;

	/*
	 * (Based on the rand(3) manpage)
	 *
	 * The input is chopped up into units of 20 bytes (or less for
	 * the last block).  Each of these blocks is run through the hash
	 * function as follows:  The data passed to the hash function
	 * is the current 'md', the same number of bytes from the 'state'
	 * (the location determined by in incremented looping index) as
	 * the current 'block', the new key data 'block', and 'count'
	 * (which is incremented after each use).
	 * The result of this is kept in 'md' and also xored into the
	 * 'state' at the same locations that were used as input into the
		 * hash function.
	 */

	/* check if we already have the lock */
	if (crypto_lock_rand) {
		CRYPTO_r_lock(CRYPTO_LOCK_RAND2);
		do_not_lock = (locking_thread == CRYPTO_thread_id());
		CRYPTO_r_unlock(CRYPTO_LOCK_RAND2);
	} else
		do_not_lock = 0;

	if (!do_not_lock) CRYPTO_w_lock(CRYPTO_LOCK_RAND);
	st_idx=state_index;

	/* use our own copies of the counters so that even
	 * if a concurrent thread seeds with exactly the
	 * same data and uses the same subarray there's _some_
	 * difference */
	sha_c[0] = sha_count[0];
	sha_c[1] = sha_count[1];

	memcpy(local_sha, sha, sizeof sha);

	/* state_index <= state_num <= STATE_SIZE */
	state_index += num;
	if (state_index >= STATE_SIZE) {
		state_index%=STATE_SIZE;
		state_num=STATE_SIZE;
	} else if (state_num < STATE_SIZE) {
		if (state_index > state_num)
			state_num=state_index;
	}
	/* state_index <= state_num <= STATE_SIZE */

	/* state[st_idx], ..., state[(st_idx + num - 1) % STATE_SIZE]
	 * are what we will use now, but other threads may use them
	 * as well */

	sha_count[1] += (num / SHA_DIGEST_LENGTH) + (num % SHA_DIGEST_LENGTH > 0);

	if (!do_not_lock) CRYPTO_w_unlock(CRYPTO_LOCK_RAND);

	for (i=0; i<num; i+=SHA_DIGEST_LENGTH) {
		j=(num-i);
		j=(j > SHA_DIGEST_LENGTH)?SHA_DIGEST_LENGTH:j;

		sha512.init();
		sha512.update((char*)local_sha,SHA_DIGEST_LENGTH);
		k=(st_idx+j)-STATE_SIZE;
		if (k > 0) {
			sha512.update((char*)&(state[st_idx]),j-k);
			sha512.update((char*)&(state[0]),k);
		} else
			sha512.update((char*)&(state[st_idx]),j);

		sha512.update((char*)buf,j);
		sha512.update((char *)&(sha_c[0]),sizeof(sha_c));
		sha512.final();
		memcpy(local_sha,sha512.state,SHA_DIGEST_LENGTH);
		sha_c[1]++;

		buf=(const char *)buf + j;

		for (k=0; k<j; k++) {
			/* Parallel threads may interfere with this,
			 * but always each byte of the new state is
			 * the XOR of some previous value of its
			 * and local_md (itermediate values may be lost).
			 * Alway using locking could hurt performance more
			 * than necessary given that conflicts occur only
			 * when the total seeding is longer than the random
			 * state. */
			state[st_idx++]^=local_sha[k];
			if (st_idx >= STATE_SIZE)
				st_idx=0;
		}
	}
	memset(&sha512,0,sizeof(sha512));

	if (!do_not_lock) CRYPTO_w_lock(CRYPTO_LOCK_RAND);
	/* Don't just copy back local_md into md -- this could mean that
	 * other thread's seeding remains without effect (except for
	 * the incremented counter).  By XORing it we keep at least as
	 * much entropy as fits into md. */
	for (k = 0; k < (int)sizeof(sha); k++) {
		sha[k] ^= local_sha[k];
	}
	if (RSA_entropy < ENTROPY_NEEDED) /* stop counting when we have enough */
		RSA_entropy += add;
	if (!do_not_lock) CRYPTO_w_unlock(CRYPTO_LOCK_RAND);

#if !defined(OPENSSL_THREADS) && !defined(OPENSSL_SYS_WIN32)
	assert(sha_c[1] == sha_count[1]);
#endif
}

static void readtimer(void) {
	DWORD w;
	LARGE_INTEGER l;
	static int have_perfc = 1;
#if defined(_MSC_VER) && defined(_M_X86)
	static int have_tsc = 1;
	DWORD cyclecount;

	if (have_tsc) {
		__try {
			__asm {
				_emit 0x0f
				_emit 0x31
				mov cyclecount, eax
			}
			add(&cyclecount, sizeof(cyclecount), 1);
		} __except(EXCEPTION_EXECUTE_HANDLER) {
			have_tsc = 0;
		}
	}
#else
# define have_tsc 0
#endif

	if (have_perfc) {
		if (QueryPerformanceCounter(&l) == 0)
			have_perfc = 0;
		else
			RSA_r_add(&l, sizeof(l), 0);
	}

	if (!have_tsc && !have_perfc) {
		w = GetTickCount();
		RSA_r_add(&w, sizeof(w), 0);
	}
}

int r_event(UINT iMsg, WPARAM wParam, LPARAM lParam) {
	double add_entropy=0;
	switch (iMsg) {
		case WM_KEYDOWN:
			{
				static WPARAM key;
				if (key != wParam)
					add_entropy = 0.05;
				key = wParam;
			}
			break;
		case WM_MOUSEMOVE:
			{
				static int lastx,lasty,lastdx,lastdy;
				int x,y,dx,dy;
				x=LOWORD(lParam);
				y=HIWORD(lParam);
				dx=lastx-x;
				dy=lasty-y;
				if (dx != 0 && dy != 0 && dx-lastdx != 0 && dy-lastdy != 0)
					add_entropy=.2;
				lastx=x, lasty=y;
				lastdx=dx, lastdy=dy;
			}
			break;
	}

	readtimer();
	RSA_r_add(&iMsg, sizeof(iMsg), add_entropy);
	RSA_r_add(&wParam, sizeof(wParam), 0);
	RSA_r_add(&lParam, sizeof(lParam), 0);
	return (RSA_r_status());
}

void RSA_r_seed(const void *buf, int num) {
	RSA_r_add(buf, num, (double)num);
}

int RSA_r_pseudorand(unsigned char *buf, int num) {

	int ret;
	unsigned long err;

	ret = RSA_r_bytes(buf, num);
	if (ret == 0) {
		err = ERR_peek_error();
		if (ERR_GET_LIB(err) == ERR_LIB_RAND &&
			ERR_GET_REASON(err) == RAND_R_PRNG_NOT_SEEDED)
			ERR_clear_error();
	}
	return (ret);
}

int RSA_r_status() {

	int ret;
	int do_not_lock;

	/* check if we already have the lock
	 * (could happen if a RAND_poll() implementation calls RAND_status()) */
	if (crypto_lock_rand) {
		CRYPTO_r_lock(CRYPTO_LOCK_RAND2);
		do_not_lock = (locking_thread == CRYPTO_thread_id());
		CRYPTO_r_unlock(CRYPTO_LOCK_RAND2);
	} else
		do_not_lock = 0;

	if (!do_not_lock) {
		CRYPTO_w_lock(CRYPTO_LOCK_RAND);

		/* prevent bytes() from trying to obtain the lock again */
		CRYPTO_w_lock(CRYPTO_LOCK_RAND2);
		locking_thread = CRYPTO_thread_id();
		CRYPTO_w_unlock(CRYPTO_LOCK_RAND2);
		crypto_lock_rand = 1;
	}

	if (!initialized) {
//		RAND_poll();
		initialized = 1;
	}

	ret = RSA_entropy >= ENTROPY_NEEDED;

	if (!do_not_lock) {
		/* before unlocking, we must clear 'crypto_lock_rand' */
		crypto_lock_rand = 0;

		CRYPTO_w_unlock(CRYPTO_LOCK_RAND);
	}

	// ожидать в течении минуты
/*	if (ret == 0) {
		unsigned long timepast = 0;
		while ((ret = entropy >= ENTROPY_NEEDED) == 0) {
			Sleep(100); timepast += 100;
			if (timepast > 60000) break;
		}
	}
*/
	return ret;
}

double RSA_getTime() {
	SYSTEMTIME time;
	FILETIME ftime;
	GetSystemTime(&time);
	SystemTimeToFileTime(&time,&ftime);
	double t = double(ftime.dwLowDateTime/10000)/1000+double(ftime.dwHighDateTime)*429.4967296;
	return t;
}

HHOOK RSA_kbhook;
static double RSA_lastkbtime;
LRESULT CALLBACK RSA_KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
static bool RSA_kbhooked = false;
void RSA_kbHook() {
	if (!RSA_kbhooked) {
		RSA_lastkbtime = RSA_getTime();
		RSA_kbhook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)RSA_KeyboardProc, (HINSTANCE)GetCurrentProcess(), 0);
		RSA_kbhooked = true;
	}
}
void RSA_kbUnHook() {
	if (RSA_kbhooked) {
		if (RSA_kbhook) { UnhookWindowsHookEx(RSA_kbhook); RSA_kbhook = 0; }
		RSA_kbhooked = false;
	}
}

HHOOK RSA_mshook;
double RSA_lastmstime;
LRESULT CALLBACK RSA_MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
bool RSA_mshooked = false;
void RSA_msHook() {
	if (!RSA_mshooked) {
		RSA_lastmstime = RSA_getTime();
		RSA_mshook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)RSA_MouseProc, (HINSTANCE)GetCurrentProcess(), 0);
		RSA_mshooked = true;
	}
}
void RSA_msUnHook() {
	if (RSA_mshooked) {
		if (RSA_mshook) { UnhookWindowsHookEx(RSA_mshook); RSA_mshook = 0; }
		RSA_mshooked = false;
	}
}

LRESULT CALLBACK RSA_KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	RSA_lastkbtime = RSA_getTime();
	HWND hwnd = GetForegroundWindow();
	if ((nCode == HC_ACTION) && (hwnd)) {
		KBDLLHOOKSTRUCT hooked = *((KBDLLHOOKSTRUCT*)lParam);

		double add_entropy=0;
		static WPARAM key;
		if (key != hooked.vkCode)
			add_entropy = 0.05;
		key = hooked.vkCode;
		readtimer();
		RSA_r_add(&wParam, sizeof(wParam), add_entropy);
		RSA_r_add(&hooked, sizeof(KBDLLHOOKSTRUCT), 0);

	}
	return CallNextHookEx(RSA_kbhook, nCode, wParam, lParam);
}

LRESULT CALLBACK RSA_MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	RSA_lastmstime = RSA_getTime();
	HWND hwnd = GetForegroundWindow();
	if ((nCode == HC_ACTION) && (hwnd)) {
		MSLLHOOKSTRUCT hooked = *((MSLLHOOKSTRUCT*)lParam);

		double add_entropy=0;
		static int lastx,lasty,lastdx,lastdy;
		int x,y,dx,dy;
		x=hooked.pt.x;
		y=hooked.pt.y;
		dx=lastx-x;
		dy=lasty-y;
		if (dx != 0 && dy != 0 && dx-lastdx != 0 && dy-lastdy != 0)
			add_entropy=.2;
		lastx=x, lasty=y;
		lastdx=dx, lastdy=dy;

		readtimer();
		RSA_r_add(&wParam, sizeof(wParam), add_entropy);
		RSA_r_add(&hooked, sizeof(MSLLHOOKSTRUCT), 0);

	}
	return CallNextHookEx(RSA_mshook, nCode, wParam, lParam);
}

HANDLE RSA_hKeybdThread;
DWORD RSA_idKeybdThread;
HANDLE RSA_hKeybdThreadExit;
DWORD WINAPI RSA_KeybdThreadFunc(LPVOID lpData) {
	RSA_kbHook();
	DWORD result; MSG msg;
	while (true) {
		if ((RSA_getTime()-RSA_lastkbtime) > 60) { // 1 Minute
			RSA_kbUnHook(); RSA_kbHook();
		}
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				PostMessage(msg.hwnd,msg.message,msg.wParam,msg.lParam);
				RSA_kbUnHook();
				ExitThread(-1);
				return -1;
			}
//			event(msg.message,msg.wParam,msg.lParam);
			DispatchMessage(&msg);
		}
		result = MsgWaitForMultipleObjects(1,&RSA_hKeybdThreadExit,FALSE,1000,QS_ALLINPUT);
		if (result == (WAIT_OBJECT_0 + 1)) continue;
		else if (result != WAIT_TIMEOUT) break;
	}
	RSA_kbUnHook();
	ExitThread(0);
	return 0;
}

HANDLE RSA_hMouseThread;
DWORD RSA_idMouseThread;
HANDLE RSA_hMouseThreadExit;
DWORD WINAPI RSA_MouseThreadFunc(LPVOID lpData) {
	RSA_msHook();
	DWORD result; MSG msg;
	while (true) {
		if ((RSA_getTime()-RSA_lastmstime) > 60) { // 1 Minute
			RSA_msUnHook(); RSA_msHook();
		}
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				PostMessage(msg.hwnd,msg.message,msg.wParam,msg.lParam);
				RSA_msUnHook();
				ExitThread(-1);
				return -1;
			}
//			event(msg.message,msg.wParam,msg.lParam);
			DispatchMessage(&msg);
		}
		result = MsgWaitForMultipleObjects(1,&RSA_hMouseThreadExit,FALSE,1000,QS_ALLINPUT);
		if (result == (WAIT_OBJECT_0 + 1)) continue;
		else if (result != WAIT_TIMEOUT) break;
	}
	RSA_msUnHook();
	ExitThread(0);
	return 0;
}

static RAND_METHOD *prev_rm;
static RAND_METHOD rm;

void RSA_r_start() {
	prev_rm = (RAND_METHOD*)RAND_get_rand_method();
	rm.seed = RSA_r_seed;
	rm.bytes = RSA_r_bytes;
	rm.cleanup = RSA_r_cleanup;
	rm.add = RSA_r_add;
	rm.pseudorand = RSA_r_pseudorand;
	rm.status = RSA_r_status;
	RAND_set_rand_method(&rm);

	RSA_hKeybdThreadExit = CreateEvent(0,true,false,0);
//	hKeybdThread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)&KeybdThreadFunc,0,0,&idKeybdThread);
	RSA_hMouseThreadExit = CreateEvent(0,true,false,0);
	RSA_hMouseThread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)&RSA_MouseThreadFunc,0,0,&RSA_idMouseThread);
}

void RSA_r_stop() {
	SetEvent(RSA_hMouseThreadExit);
	WaitForSingleObject(RSA_hMouseThread,INFINITE);
	CloseHandle(RSA_hMouseThreadExit);
	CloseHandle(RSA_hMouseThread);
	SetEvent(RSA_hKeybdThreadExit);
//	WaitForSingleObject(hKeybdThread,INFINITE);
	CloseHandle(RSA_hKeybdThreadExit);
	CloseHandle(RSA_hKeybdThread);

	RAND_set_rand_method(prev_rm);
}

/******************/

#include <openssl/bn.h>
#include <openssl/rsa.h>

typedef void fKEYGENCB(int percent, void* param);

struct skgp {
	int stage;
	int pos;
	fKEYGENCB* random_cb;
	fKEYGENCB* keygen_cb;
	void* param;
};

static int RSA_genrsa_cb(int p, int n, BN_GENCB *cb) {
	skgp* lpkgp = (skgp*)(cb->arg);
	if (p == 0) {
		++(lpkgp->pos);
		if ((lpkgp->pos) > (((lpkgp->stage)+1) * 49)) (lpkgp->pos) -= 5;
	}
	if (p == 1) {
		(lpkgp->pos)++;
	}
	if (p == 2) {
		(lpkgp->pos) = (lpkgp->stage) * 49;
	}
	if (p == 3) {
		++(lpkgp->stage);
		(lpkgp->pos) = (lpkgp->stage) * 49;
	}
	if (lpkgp->keygen_cb) lpkgp->keygen_cb(lpkgp->pos,lpkgp->param);
	return 1;
}

RSA* RSA_generate_key(RSA* rsa, int bits, bool interactive, fKEYGENCB* random_cb, fKEYGENCB* keygen_cb, void* param) {
    if (interactive) RSA_r_start();

	skgp kgp = { 0, 0, random_cb, keygen_cb, param };
	if (keygen_cb) keygen_cb(0, param);

	BN_GENCB cb;
	BN_GENCB_set(&cb, RSA_genrsa_cb, &kgp);

    if (interactive) {
        RSA_r_cleanup();

        while (RSA_r_status() == 0) {
            if (random_cb) random_cb(RSA_entropy / 2.56, param);
            ::Sleep(100);
        }
        if (random_cb) random_cb(-1, param);
    }

	kgp.pos = 0;

	BIGNUM* e = BN_new();
	BN_set_word(e,0x10001);

	if (rsa) RSA_free(rsa);
	rsa = RSA_new();

	RSA_generate_key_ex(rsa,bits,e,&cb);
//	char* s;
//	s = BN_bn2hex(rsa->n);
//	OPENSSL_free(s);
//	s = BN_bn2hex(rsa->e);
//	OPENSSL_free(s);
//	s = BN_bn2hex(rsa->d);
//	OPENSSL_free(s);

//	RSA_free(rsa);
	BN_clear_free(e);

    if (interactive) RSA_r_stop();

    return rsa;
}

void RSA_random_cb(int percent, void* param) {
	if (percent == -1) {
		// Пожалуйста ждите. Идёт генерация секретного ключа
	} else {
		// Процент
	}
}

void RSA_keygen_cb(int percent, void* param) {
	// Процент
}

RSA* RSA_keygen(RSA* key, int bits, bool interactive) {
	return RSA_generate_key(key,bits,interactive,RSA_random_cb,RSA_keygen_cb,0);
}

RSA* RSA_keyfree(RSA* key) {
	if (key) {
	    RSA_free(key);
        key = 0;
	}
	return key;
}


