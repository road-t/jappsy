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
#include <data/uJSONObject.h>
#include <data/uList.h>
#include <core/uHandler.h>

class Loader;
class GLRender;

struct LoaderStatus {
	jbool update = false;
	int count = 0;
	int total = 0;
	int left = 0;
	int error = 0;
};

class RefLoader : public RefObject {
private:
	class RefFile : public RefObject {
	public:
		String file;
		String ext;
		String key;
		String group;
		
		inline RefFile() {}
		
		inline RefFile(const String& file, const String& ext, const String& key, const String& group) {
			THIS.file = file;
			THIS.ext = ext;
			THIS.key = key;
			THIS.group = group;
		}
	};
	
	class File : public Object {
	public:
		RefClass(File, RefFile);
	};
	
public:
	class RefInfo : public RefObject {
	public:
		Loader* loader = NULL;
		File info;
		
		inline RefInfo() {}

		RefInfo(const RefLoader& loader, const RefFile& info);
		~RefInfo();
	};
	
	class Info : public Object {
	public:
		RefClass(Info, RefInfo);

		inline Info(const RefLoader& loader, const RefFile& info) {
			RefInfo* o = new RefInfo(loader, info);
			if (o == NULL) throw eOutOfMemory;
			THIS.setRef(o);
		}
	};
	
public:
	// loader callback types
	typedef void (*onFileCallback)(const String& url, const Object& object, const Object userData);
	typedef void (*onStatusCallback)(const LoaderStatus& status, const Object userData);
	typedef void (*onReadyCallback)(const HashMap<String, Stream>& result, const Object userData);
	typedef void (*onErrorCallback)(const String& error, const Object userData);

	onFileCallback onfile = NULL;
	onStatusCallback onstatus = NULL;
	onReadyCallback onready = NULL;
	onErrorCallback onerror = NULL;
	
	Object userData;

private:
	// loader internal data
#ifdef DEBUG
	int loadSpeed = 1;
#else
	int loadSpeed = 5;
#endif
	
	JSONObject result;
	List<File> list;
	struct LoaderStatus status;
	volatile int32_t shutdown = 0;
	volatile bool updating = false;
	String lastError;
	
	String cacheid;
	
	inline int hasDownloads() { return list.size(); }
	
	inline File& lastDownload() throw(const char*) {
		return *(File*)&(list.peek());
	}
	
	inline void doneDownload() throw(const char*) {
		list.pop();
	}
	
	Handler handler;
	void checkUpdate(int time);
	static void onUpdate(const Object& data);
	void update();
	void run();
	
	static bool onhttp_text(const String& url, Stream& stream, const Object& userData);
	static bool onhttp_data(const String& url, Stream& stream, const Object& userData);
	static void onhttp_error(const String& url, const String& error, const Object& userData);
	static bool onhttp_retry(const String& url, const Object& userData);
	
	bool onText(const File& info, Stream& stream);
	bool onData(const File& info, Stream& stream);
	void onLoad(const File& info, const Object& object);
	void onError(const File& info, const String& error);
	bool onRetry(const File& info);
	
private:
	// json parser callbacks and data
	String group;
	String subgroup;
	
	static void onjson_root(struct json_context* ctx, void* target);
	static void onjson_group(struct json_context* ctx, const char* key, void* target);
	static void onjson_subgroup(struct json_context* ctx, const char* key, void* target);
	static void onjson_subfile(struct json_context* ctx, const char* key, char* value, void* target);

private:
	GLRender* context;
	
public:
	
	String basePath;

	inline void initialize() {
		result = new JSONObject();
		list = new List<File>();
		handler = new Handler();
	}

	inline RefLoader() { throw eInvalidParams; }
	inline RefLoader(GLRender* context, Object& userData) { initialize(); THIS.context = context; THIS.userData = userData; }
	~RefLoader();
	
	void setCallbacks(onFileCallback onfile, onStatusCallback onstatus, onReadyCallback onready, onErrorCallback onerror);
	void load(const char* jsonconfig) throw(const char*);
	void release();
};

class Loader : public Object {
public:
	RefClass(Loader, RefLoader);
	
	inline Loader(GLRender* context, Object& userData) {
		RefLoader* o = new RefLoader(context, userData);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	inline void release() throw(const char*) { THIS.ref().release(); }
};

#endif //JAPPSY_ULOADER_H