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
#include <data/uStream.h>
#include <data/uString.h>
#include <data/uVector.h>
#include <core/uSystem.h>

class Loader;
class GLRender;

struct LoaderStatus {
	jbool update = false;
	int count = 0;
	int total = 0;
	int left = 0;
	int error = 0;
};

class Loader : public CObject {
private:
	class File : public CObject {
	public:
		CString path;
		CString file;
		CString uri;
		CString ext;
		CString key;
		CString group;
		
		inline File(const CString& path, const CString& file, const CString& uri, const CString& ext, const CString& key, const CString& group) {
			this->path = path;
			this->file = file;
			this->uri = uri;
			this->ext = ext;
			this->key = key;
			this->group = group;
		}
	};
	
public:
	class Info : public CObject {
	public:
		Loader* loader = NULL;
		File* info = NULL;
		
		Info(Loader* loader, File* info);
		~Info();
	};
	
public:
	// loader callback types
	typedef void* (*onFileCallback)(const CString& url, void* object, void* userData);
	typedef void* (*onStatusCallback)(const LoaderStatus& status, void* userData);
	typedef void* (*onReadyCallback)(void* userData);
	typedef void* (*onErrorCallback)(const CString& error, void* userData);

	onFileCallback onfile = NULL;
	onStatusCallback onstatus = NULL;
	onReadyCallback onready = NULL;
	onErrorCallback onerror = NULL;
	
	void* userData = NULL;

private:
	// loader internal data
#ifdef DEBUG
	int loadSpeed = 1;
#else
	int loadSpeed = 5;
#endif
	
	Vector<File*> list;
	struct LoaderStatus status;
	volatile int32_t shutdown = 0;
	volatile jlock updating = false;
	CString lastError;
	
	CString cacheid;
	
	void checkUpdate(int time);
	static void* onUpdate(void* data);
	void update();
	void run();
	
	static bool onhttp_text(const CString& url, Stream* stream, void* userData);
	static bool onhttp_data(const CString& url, Stream* stream, void* userData);
	static void onhttp_error(const CString& url, const CString& error, void* userData);
	static bool onhttp_retry(const CString& url, void* userData);
	static void onhttp_release(void* userData);
	
	bool onText(const File* info, Stream* stream);
	bool onData(const File* info, Stream* stream);
	void onLoad(const File* info, void* object);
	void onError(const File* info, const CString& error);
	bool onRetry(const File* info);
	
private:
	// json parser callbacks and data
	CString group;
	CString subgroup;
	
	static void onjson_root_start(struct JsonContext* ctx, void* target);
	static void onjson_group(struct JsonContext* ctx, const char* key, void* target);
	static void onjson_subgroup(struct JsonContext* ctx, const char* key, void* target);
	static void onjson_subfile(struct JsonContext* ctx, const char* key, char* value, void* target);
	
public:
	GLRender* context = NULL;
	
	CString basePath;

	inline Loader(GLRender* context, void* userData) {
		this->context = context;
		this->userData = userData;
	}
	
	void release();
	~Loader();
	
	void setCallbacks(onFileCallback onfile, onStatusCallback onstatus, onReadyCallback onready, onErrorCallback onerror);
	void load(const char* jsonconfig) throw(const char*);
};

#endif //JAPPSY_ULOADER_H