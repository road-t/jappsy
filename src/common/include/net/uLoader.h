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
#include <data/uHashMap.h>
#include <core/uHandler.h>

class Loader;

class RefLoader : public RefObject {
public:
	struct Status {
		bool update = false;
		int count = 0;
		int total = 0;
		int left = 0;
		int error = 0;
	};

private:
	class RefFile : public RefObject {
	public:
		String file;
		String ext;
		String key;
		String group;
		
		inline RefFile() {}
		
		inline RefFile(const String& file, const String& ext, const String& key, const String& group) {
			this->file = file;
			this->ext = ext;
			this->key = key;
			this->group = group;
		}
	};
	
	class File : public Object {
	public:
		RefClass(File, File);
	};
	
public:
	class RefInfo : public RefObject {
	public:
		Loader* loader;
		String file;
		String ext;
		String key;
		String group;
		String type;
		
		void onLoad(const Stream& stream);
		void onError(const String& error);
		
		~RefInfo();
	};
	
	class Info : public Object {
	public:
		RefClass(Info, Info);
	};
	
public:
	// loader callback types
	typedef void (*onFileCallback)(const Info& info, const Stream& stream, const Object& userData);
	typedef void (*onStatusCallback)(const Status& status, const Object& userData);
	typedef void (*onReadyCallback)(const HashMap<String, Stream>& result, const Object& userData);
	typedef void (*onErrorCallback)(const String& error, const Object& userData);

private:
	// loader internal data
	int loadSpeed = 5;
	
	HashMap<String, Stream> result = new HashMap<String, Stream>();
	List<File> list = new List<File>();
	struct Status status;
	volatile int32_t shutdown = 0;
	volatile bool updating = false;
	String lastError;
	
	String cacheid;

	onFileCallback onfile = NULL;
	onStatusCallback onstatus = NULL;
	onReadyCallback onready = NULL;
	onErrorCallback onerror = NULL;
	
	Object userData;
	
	inline int hasDownloads() { return list.size(); }
	
	inline File& lastDownload() throw(const char*) {
		return *(File*)&(list.peek());
	}
	
	inline void doneDownload() throw(const char*) {
		list.pop();
	}
	
	Handler handler = new Handler();
	void checkUpdate(int time);
	static void onUpdate(const Object& data);
	void update();
	void run();
	
	void createImageLoader(const File& info);
	void createSoundLoader(const File& info);
	void createJsonLoader(const File& info);
	void createDataLoader(const File& info);
	
private:
	// json parser callbacks and data
	String group;
	String subgroup;
	
	static void onroot(struct json_context* ctx, void* target);
	static void ongroup(struct json_context* ctx, const char* key, void* target);
	static void onsubgroup(struct json_context* ctx, const char* key, void* target);
	static void onsubfile(struct json_context* ctx, const char* key, char* value, void* target);

public:
	
	String basePath;
	
	inline RefLoader() {}
	RefLoader(onFileCallback onfile, onStatusCallback onstatus, onReadyCallback onready, onErrorCallback onerror, Object& userData);
	~RefLoader();
	
	void load(const char* jsonconfig) throw(const char*);
	void release();
};

class Loader : public Object {
public:
	RefClass(Loader, Loader);
};

#endif //JAPPSY_ULOADER_H