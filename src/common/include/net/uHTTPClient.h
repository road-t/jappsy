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
#include <data/uAtomicObject.h>
#include <data/uStream.h>

class HTTPClient {
public:
	typedef bool (*onStreamCallback)(const JString& url, Stream& stream, const JObject& userData);
	typedef void (*onErrorCallback)(const JString& url, const JString& error, const JObject& userData);
	typedef bool (*onRetryCallback)(const JString& url, const JObject& userData);
	
	static void Request(const JString& url, bool threaded, int retry, int timeout, onStreamCallback onstream, onErrorCallback onerror, onRetryCallback onretry) throw(const char*);
	static void Request(const JString& url, bool threaded, int retry, int timeout, const JObject& userData, onStreamCallback onstream, onErrorCallback onerror, onRetryCallback onretry) throw(const char*);
};

#endif //JAPPSY_UHTTPCLIENT_H