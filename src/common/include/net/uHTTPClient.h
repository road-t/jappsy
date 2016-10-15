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

#ifndef JAPPSY_UHTTPCLIENT_H
#define JAPPSY_UHTTPCLIENT_H

#include <platform.h>
#include <data/uObject.h>
#include <data/uString.h>
#include <data/uStream.h>

class HTTPClient : public CObject {
public:
	typedef bool (*onStreamCallback)(const CString& url, Stream* stream, void* userData);
	typedef int (*onErrorCallback)(const CString& url, const CString& error, void* userData);
	typedef bool (*onRetryCallback)(const CString& url, void* userData);
	typedef void (*onFatalCallback)(const CString& url, const CString& error, void* userData);
	typedef void (*onReleaseCallback)(void* userData);
	
	static void Request(const CString& url, char* post, bool threaded, int retry, int timeout, bool cache, void* userData, onStreamCallback onstream, onErrorCallback onerror, onRetryCallback onretry, onFatalCallback onfatal, onReleaseCallback onrelease) throw(const char*);
};

void uHTTPClientInit();
void uHTTPClientQuit();

#endif //JAPPSY_UHTTPCLIENT_H
