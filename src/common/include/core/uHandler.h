/*
 * Copyright (C) 2016 The Jappsy Open Source Project (http://jappsy.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef JAPPSY_HANDLER_H
#define JAPPSY_HANDLER_H

#include <platform.h>
#include <data/uObject.h>

extern const wchar_t TypeHandler[];

class RefHandler : public RefObject {
public:
	typedef void (*Callback)(void* userData);
	
private:
	class HandlerCallback {
	public:
		volatile int32_t m_lock;
		
		Callback m_callback;
		volatile int m_delay;
		volatile bool m_enabled;
		volatile void* m_userData;
		
		HandlerCallback(Callback callback, int delay, void* userData);
	};

private:
	volatile int32_t m_retain;
	volatile int32_t m_lock;

	HandlerCallback** m_queue;
	volatile uint32_t m_count;
	volatile uint32_t m_memorySize;
	
	void resize(uint32_t count) throw(const char*);
	void push(HandlerCallback* runner);
	void remove(HandlerCallback* runner);
	
public:
	RefHandler();
	~RefHandler();

	void run(void* runnerid);

	void* postDelayed(Callback callback, int delay, void* userData);
	void* post(Callback callback, void* userData);
	void removeCallbacks(Callback callback);
	void remove(void* runnerid);
};

class Handler : public Object {
public:
	RefClass(Handler, Handler);
	
	inline void* postDelayed(RefHandler::Callback callback, int delay, void* userData) throw(const char*) { CHECKTHIS; return THIS->postDelayed(callback, delay, userData); }
	inline void* post(RefHandler::Callback callback, void* userData) throw(const char*) { CHECKTHIS; return THIS->post(callback, userData); }
	inline void removeCallbacks(RefHandler::Callback callback) throw(const char*) { CHECKTHIS; THIS->removeCallbacks(callback); }
	inline void remove(void* runnerid) throw(const char*) { CHECKTHIS; THIS->remove(runnerid); }
};

#endif //JAPPSY_HANDLER_H