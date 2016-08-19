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

#include "uLoader.h"
#include <data/uJSON.h>
#include <net/uURI.h>
#include <core/uSystem.h>
#include <net/uHttpClient.h>
#include <opengl/uGLRender.h>

RefLoader::RefLoader(GLRender* context, onFileCallback onfile, onStatusCallback onstatus, onReadyCallback onready, onErrorCallback onerror, Object& userData) {
	initialize();
	THIS.context = context;
	THIS.onfile = onfile;
	THIS.onstatus = onstatus;
	THIS.onready = onready;
	THIS.onerror = onerror;
	THIS.userData = userData;
}

void RefLoader::release() {
	THIS.wait();
	AtomicSet(&shutdown, 1);
	handler.release();
	
	while (AtomicGet(&status.left) > 0) {
		sleep(1);
	}
	
	context = NULL;
	THIS.notifyAll();
}

RefLoader::~RefLoader() {
	release();
	// TODO: RELEASE LOADER
}

void RefLoader::checkUpdate(int time) {
	THIS.wait();
	if (AtomicGet(&shutdown) == 0) {
		if (AtomicLockTry(&updating)) {
			handler.postDelayed(onUpdate, time, this);
		}
	}
	THIS.notifyAll();
}

void RefLoader::onUpdate(const Object& data) {
	(*(Loader*)(&data)).ref().update();
}

void RefLoader::update() {
	THIS.wait();
	AtomicUnlock(&updating);

	if (AtomicGet(&shutdown) == 0) {
		cacheid = currentTimeMillis();
		
		int count = hasDownloads();
		if (AtomicGet(&(status.error)) > 0) count = 0;
		if (count > loadSpeed) count = loadSpeed;
		if (count == 0) count = -1;
		if (AtomicGet(&(status.left)) >= loadSpeed) count = 0;
		else if (AtomicGet(&(status.update))) {
			if (onstatus != NULL) onstatus(status, userData);
			AtomicSet(&(status.update), false);
		}
		while (count > 0) {
			File* info = &(lastDownload());
			String ext = (*info).ref().ext;
			
			AtomicSet(&(status.update), true);
			AtomicIncrement(&(status.left));
			if ((ext.compareToIgnoreCase(L"png") == 0) ||
				(ext.compareToIgnoreCase(L"jpg") == 0) ||
				(ext.compareToIgnoreCase(L"jpeg") == 0)) {
				// TODO: PNG JPG Loader not needed
				// Fake OK
				AtomicIncrement(&(status.count));
				AtomicDecrement(&(status.left));
				AtomicSet(&(status.update), true);
			} else if (
				(ext.compareToIgnoreCase(L"mp3") == 0) ||
				(ext.compareToIgnoreCase(L"ogg") == 0)
			) {
				// TODO: MP3 OGG Loader
				// Fake OK
				AtomicIncrement(&(status.count));
				AtomicDecrement(&(status.left));
				AtomicSet(&(status.update), true);
			} else if (
				(ext.compareToIgnoreCase(L"jimg") == 0) ||
				(ext.compareToIgnoreCase(L"jsh") == 0)) {
				Info user = new Info(*this, info->ref());
				HTTPClient::Request(info->ref().file, true, 0, 5, user, onhttp_data, onhttp_error);
			} else if (
				(ext.compareToIgnoreCase(L"json") == 0) ||
				(ext.compareToIgnoreCase(L"vsh") == 0) ||
				(ext.compareToIgnoreCase(L"fsh") == 0)) {
				Info user = new Info(*this, info->ref());
				HTTPClient::Request(info->ref().file, true, 0, 5, user, onhttp_text, onhttp_error);
			} else {
				// Unknown File Type
				// Fake OK
				AtomicIncrement(&(status.count));
				AtomicDecrement(&(status.left));
				AtomicSet(&(status.update), true);
			}
			doneDownload();
			count--;
		}
		if (AtomicGet(&(status.left)) > 0) {
			checkUpdate(15);
		} else if (count == 0) {
			checkUpdate(15);
		} else if (AtomicGet(&(status.error)) > 0) {
			if (onerror != NULL) onerror(lastError, userData);
		} else {
			if (onready != NULL) {
				try {
					onready(result, userData);
				} catch (const char* e) {
					lastError = e;
					if (onerror != NULL) onerror(lastError, userData);
				}
			}
		}
	}
	THIS.notifyAll();
}

void RefLoader::run() {
	checkUpdate(15);
}

void RefLoader::onjson_root(struct json_context* ctx, void* target) {
	ctx->callbacks->onobject.onobject = onjson_group;
}

void RefLoader::onjson_group(struct json_context* ctx, const char* key, void* target) {
	RefLoader* loader = (RefLoader*)target;
	loader->group = key;
	
	json_clear_callbacks(ctx->callbacks, target);
	ctx->callbacks->onobject.onobject = onjson_subgroup;
}

void RefLoader::onjson_subgroup(struct json_context* ctx, const char* key, void* target) {
	RefLoader* loader = (RefLoader*)target;
	loader->subgroup = key;
	
	json_clear_callbacks(ctx->callbacks, target);
	ctx->callbacks->onobject.onstring = onjson_subfile;
}

void RefLoader::onjson_subfile(struct json_context* ctx, const char* key, char* value, void* target) {
	RefLoader* loader = (RefLoader*)target;
	
	String path = value;
	URI* uri = new URI((wchar_t*)path);
	uri->absolutePath((wchar_t*)(loader->basePath));
	
	if (!loader->subgroup.startsWith(L"disable/") && (strstr(key, "disable/") == NULL)) {
		loader->list.add(new RefLoader::RefFile(uri->uri(), uri->ext(), key, loader->subgroup));
		loader->status.total++;
	}
	
	delete uri;
}

void RefLoader::load(const char* json) throw(const char*) {
	if (json == NULL)
		throw eNullPointer;
	
	struct json_context ctx;
	struct json_callbacks callbacks;
	ctx.callbacks = &callbacks;
	json_clear_callbacks(&callbacks, this);
	callbacks.onroot = onjson_root;
	if (!json_call(&ctx, json)) {
#ifdef DEBUG
		json_debug_error(ctx, json);
#endif
		throw eConvert;
	}
	
	run();
}

RefLoader::RefInfo::RefInfo(const RefLoader& loader, const RefFile& info) {
	this->loader = new Loader(&loader);
	this->info = &info;
}

RefLoader::RefInfo::~RefInfo() {
	delete loader;
}

bool RefLoader::onhttp_text(const String& url, Stream& stream, const Object& userData) {
	RefInfo* info = (RefInfo*)&(userData.ref());
	return info->loader->ref().onText(info->info, stream);
}

bool RefLoader::onhttp_data(const String& url, Stream& stream, const Object& userData) {
	RefInfo* info = (RefInfo*)&(userData.ref());
	return info->loader->ref().onData(info->info, stream);
}

void RefLoader::onhttp_error(const String& url, const String& error, const Object& userData) {
	String err = String::format(L"Unable to load %ls - %ls", (wchar_t*)url, (wchar_t*)error);
	err.log();

	RefInfo* info = (RefInfo*)&(userData.ref());
	info->loader->ref().onError(info->info, error);
}

bool RefLoader::onText(const File& info, Stream& stream) {
	if (AtomicGet(&shutdown) != 0)
		return false;
	
	String ext = info.ref().ext;
	if (ext.compareToIgnoreCase(L"vsh") == 0) {
		try {
			GLShader* shader = &(context->shaders->createVertexShader((wchar_t*)(info.ref().key), (char*)stream.getBuffer()));
			onLoad(info, *shader);
			if (onfile != NULL) onfile(info.ref().file, *shader, userData);
		} catch (...) {
			return false;
		}
	} else if (ext.compareToIgnoreCase(L"fsh") == 0) {
		try {
			GLShader* shader = &(context->shaders->createFragmentShader((wchar_t*)(info.ref().key), (char*)stream.getBuffer()));
			onLoad(info, *shader);
			if (onfile != NULL) onfile(info.ref().file, *shader, userData);
		} catch (...) {
			return false;
		}
	} else {
		onLoad(info, stream);
		if (onfile != NULL) onfile(info.ref().file, stream, userData);
	}
	return true;
}

bool RefLoader::onData(const File& info, Stream& stream) {
	if (AtomicGet(&shutdown) != 0)
		return false;
	
	String ext = info.ref().ext;
	if (ext.compareToIgnoreCase(L"jimg") == 0) {
		try {
			GLTexture* texture = &(GLReader::createTexture(context, (wchar_t*)(info.ref().key), stream));
			onLoad(info, *texture);
			if (onfile != NULL) onfile(info.ref().file, *texture, userData);
		} catch (...) {
			return false;
		}
	} else if (ext.compareToIgnoreCase(L"jsh") == 0) {
		try {
			GLShader* shader = &(GLReader::createShader(context, (wchar_t*)(info.ref().key), stream));
			onLoad(info, *shader);
			if (onfile != NULL) onfile(info.ref().file, *shader, userData);
		} catch (...) {
			return false;
		}
	} else {
		onLoad(info, stream);
		if (onfile != NULL) onfile(info.ref().file, stream, userData);
	}
	return true;
}

void RefLoader::onError(const File& info, const String& error) {
	AtomicDecrement(&(status.left));
	AtomicIncrement(&(status.error));
	lastError = info.ref().file;
}

void RefLoader::onLoad(const File& info, const Object& object) {
	__sync_synchronize();

	JSONObject group;
	try {
		group = result.getJSONObject(info.ref().group);
	} catch (...) {
		group = new JSONObject();
		result.put(info.ref().group, group);
	}
	
	group.put(info.ref().key, object);

	AtomicIncrement(&(status.count));
	AtomicDecrement(&(status.left));
	AtomicSet(&(status.update), true);
}

