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

class Handler;

class RefHandler : public RefObject {
public:
	typedef void (*Callback)(const Object& userData);
	
private:
	class HandlerCallback : public RefObject {
	public:
		volatile Callback callback;
		volatile int32_t delay;
		Object userData;
		
		volatile int32_t shutdown = 0;
		
		inline HandlerCallback() throw(const char*) { throw eInvalidParams; }
		HandlerCallback(Callback callback, int delay, const Object& userData);
	};

private:
	volatile int32_t shutdown = 0;
	
	HandlerCallback** queue = NULL;
	volatile int32_t count = 0;
	volatile int32_t memorySize = 0;
	
	HandlerCallback** resize(uint32_t count) throw(const char*);
	void push(HandlerCallback* runner);
	void remove(HandlerCallback* runner);
	
public:
	RefHandler() { TYPE = TypeHandler; }
	~RefHandler();

	static void onthread(Handler* handler, void* runnerid);
	static void onrun(Handler* handler, void* runnerid);

	void* postDelayed(Callback callback, int delay, const Object& userData);
	void* post(Callback callback, const Object& userData);
	void removeCallbacks(Callback callback);
	void remove(void* runnerid);
	
	void release();
};

class Handler : public Object {
public:
	RefClass(Handler, Handler);
	
	inline void* postDelayed(RefHandler::Callback callback, int delay, const Object& userData = null) throw(const char*) { CHECKTHIS; return THIS->postDelayed(callback, delay, userData); }
	inline void* post(RefHandler::Callback callback, const Object& userData = null) throw(const char*) { CHECKTHIS; return THIS->post(callback, userData); }
	inline void removeCallbacks(RefHandler::Callback callback) throw(const char*) { CHECKTHIS; THIS->removeCallbacks(callback); }
	inline void remove(void* runnerid) throw(const char*) { CHECKTHIS; THIS->remove(runnerid); }
	
	inline void release() throw(const char*) { CHECKTHIS; THIS->release(); }
};

#endif //JAPPSY_HANDLER_H