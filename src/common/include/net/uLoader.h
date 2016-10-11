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
public:
	class File : public CObject {
	public:
		// Вызывается при формировании пост запроса
		typedef char* (*onQueryCallback)(const Vector<CString&>* query);
		
		typedef void (*onLoad)(const File* info, void* object, void* userData);
		typedef bool (*onError)(const File* info, void* userData);

		CString path;
		CString file;
		CString uri;
		CString ext;
		CString key;
		CString group;
		bool cache;
		
		onQueryCallback onquery;
		Vector<CString&>* query;
		char* post;
		int retry;
		
		onLoad onload;
		onError onerror;
		void* userData;
		
		inline File(const CString& path, const CString& file, const CString& uri, const CString& ext, const CString& key, const CString& group, bool cache) {
			this->path = path;
			this->file = file;
			this->uri = uri;
			this->ext = ext;
			this->key = key;
			this->group = group;
			this->cache = cache;
			this->onquery = NULL;
			this->query = NULL;
			this->post = NULL;
			this->retry = 0;
			
			onload = NULL;
			onerror = NULL;
			userData = NULL;
		}
		
		~File();
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
	
	// Вызывается периодически при изменении статуса загрузки
	typedef void* (*onStatusCallback)(const LoaderStatus& status, void* userData);
	// Вызывается после загрузки ресурса
	typedef void* (*onFileCallback)(const File& info, void* object, void* userData);
	// Вызывается при ошибке загрузки ресурса
	typedef void* (*onErrorCallback)(const CString& error, void* userData);
	// Вызывается при устранении всех ошибок загрузки (после повторных попыток)
	typedef void* (*onRetryCallback)(void* userData);
	// Вызывается после загрузки всех ресурсов
	typedef void* (*onReadyCallback)(void* userData);
	// Вызывается при фатальной ошибке загрузки
	typedef void* (*onFatalCallback)(const CString& error, void* userData);

	onStatusCallback onstatus = NULL;
	onFileCallback onfile = NULL;
	onErrorCallback onerror = NULL;
	onRetryCallback onretry = NULL;
	onReadyCallback onready = NULL;
	onFatalCallback onfatal = NULL;
	
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
	static int onhttp_error(const CString& url, const CString& error, void* userData);
	static bool onhttp_retry(const CString& url, void* userData);
	static void onhttp_fatal(const CString& url, const CString& error, void* userData);
	static void onhttp_release(void* userData);
	
	bool onText(const File* info, Stream* stream);
	bool onData(const File* info, Stream* stream);
	void onLoad(const File* info, void* object);
	int onError(const File* info, const CString& error);
	void onFatal(const File* info, const CString& error);
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
	
	void setCallbacks(onStatusCallback onstatus, onFileCallback onfile, onErrorCallback onerror, onRetryCallback onretry, onReadyCallback onready, onFatalCallback onfatal);
	void load(const char* jsonconfig) throw(const char*);
	void load(const CString& path, const CString& group, const CString& key, File::onLoad onload = NULL, File::onError onerror = NULL, void* userData = NULL);
	void query(const CString& path, const CString& message, File::onQueryCallback onquery);
};

#endif //JAPPSY_ULOADER_H
