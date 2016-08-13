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

#ifndef JAPPSY_ULOADER_H
#define JAPPSY_ULOADER_H

#include <platform.h>
#include <data/uObject.h>
#include <data/uArray.h>
#include <data/uNamedArray.h>
#include <data/uNamedStrings.h>
#include <data/uStream.h>
#include <data/uString.h>

class Loader : public Object {
private:
	NamedArray<NamedStrings>* config;
	
public:
	struct File {
		
	};
	
	
	struct Info {
		
	};
	
	struct Status {
		
	};
	
	typedef void (*onFileCallback)(const Info& info, const Stream& stream, void* userData);
	typedef void (*onStatusCallback)(const Status& status, void* userData);
	typedef void (*onReadyCallback)(const Array<Stream>& result, void* userData);
	typedef void (*onErrorCallback)(const char* error, void* userData);
	
	Loader(onFileCallback onfile, onStatusCallback onstatus, onReadyCallback onready, onErrorCallback onerror, void* userData);
	~Loader();
	
	void load(NamedArray<NamedStrings>* config);
};

#endif //JAPPSY_ULOADER_H