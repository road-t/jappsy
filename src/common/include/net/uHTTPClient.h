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
	typedef bool (*onStreamCallback)(const String& url, Stream& stream, const Object& userData);
	typedef void (*onErrorCallback)(const String& url, const String& error, const Object& userData);
	
	static void Request(const String& url, bool threaded, int retry, int timeout, onStreamCallback onstream, onErrorCallback onerror) throw(const char*);
	static void Request(const String& url, bool threaded, int retry, int timeout, const Object& userData, onStreamCallback onstream, onErrorCallback onerror) throw(const char*);
};

#endif //JAPPSY_UHTTPCLIENT_H