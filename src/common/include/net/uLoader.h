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

class RefLoader : public JRefObject {
private:
	class RefFile : public JRefObject {
	public:
		JString file;
		JString ext;
		JString key;
		JString group;
		
		inline RefFile() {}
		
		inline RefFile(const JString& file, const JString& ext, const JString& key, const JString& group) {
			THIS.file = file;
			THIS.ext = ext;
			THIS.key = key;
			THIS.group = group;
		}
	};
	
	class File : public JObject {
	public:
		JRefClass(File, RefFile);
	};
	
public:
	class RefInfo : public JRefObject {
	public:
		Loader* loader = NULL;
		File info;
		
		inline RefInfo() {}

		RefInfo(const RefLoader& loader, const RefFile& info);
		~RefInfo();
	};
	
	class Info : public JObject {
	public:
		JRefClass(Info, RefInfo);

		inline Info(const RefLoader& loader, const RefFile& info) {
			RefInfo* o = new RefInfo(loader, info);
			if (o == NULL) throw eOutOfMemory;
			THIS.setRef(o);
		}
	};
	
public:
	// loader callback types
	typedef void (*onFileCallback)(const JString& url, const JObject& object, const JObject userData);
	typedef void (*onStatusCallback)(const LoaderStatus& status, const JObject userData);
	typedef void (*onReadyCallback)(const JSONObject& result, const JObject userData);
	typedef void (*onErrorCallback)(const JString& error, const JObject userData);

	onFileCallback onfile = NULL;
	onStatusCallback onstatus = NULL;
	onReadyCallback onready = NULL;
	onErrorCallback onerror = NULL;
	
	JObject userData;

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
	JString lastError;
	
	JString cacheid;
	
	inline int hasDownloads() { return list.size(); }
	
	inline File& lastDownload() throw(const char*) {
		return *(File*)&(list.peek());
	}
	
	inline void doneDownload() throw(const char*) {
		list.pop();
	}
	
	Handler handler;
	void checkUpdate(int time);
	static void onUpdate(const JObject& data);
	void update();
	void run();
	
	static bool onhttp_text(const JString& url, Stream& stream, const JObject& userData);
	static bool onhttp_data(const JString& url, Stream& stream, const JObject& userData);
	static void onhttp_error(const JString& url, const JString& error, const JObject& userData);
	static bool onhttp_retry(const JString& url, const JObject& userData);
	
	bool onText(const File& info, Stream& stream);
	bool onData(const File& info, Stream& stream);
	void onLoad(const File& info, const JObject& object);
	void onError(const File& info, const JString& error);
	bool onRetry(const File& info);
	
private:
	// json parser callbacks and data
	JString group;
	JString subgroup;
	
	static void onjson_root_start(struct json_context* ctx, void* target);
	static void onjson_group(struct json_context* ctx, const char* key, void* target);
	static void onjson_subgroup(struct json_context* ctx, const char* key, void* target);
	static void onjson_subfile(struct json_context* ctx, const char* key, char* value, void* target);

private:
	GLRender* context;
	
public:
	
	JString basePath;

	inline void initialize() {
		result = new JSONObject();
		list = new List<File>();
		handler = new Handler();
	}

	inline RefLoader() { throw eInvalidParams; }
	inline RefLoader(GLRender* context, JObject& userData) { initialize(); THIS.context = context; THIS.userData = userData; }
	~RefLoader();
	
	void setCallbacks(onFileCallback onfile, onStatusCallback onstatus, onReadyCallback onready, onErrorCallback onerror);
	void load(const char* jsonconfig) throw(const char*);
	void release();
};

class Loader : public JObject {
public:
	JRefClass(Loader, RefLoader);
	
	inline Loader(GLRender* context, JObject& userData) {
		RefLoader* o = new RefLoader(context, userData);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	inline void release() throw(const char*) { THIS.ref().release(); }
};

#endif //JAPPSY_ULOADER_H