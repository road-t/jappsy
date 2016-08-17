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

RefLoader::RefLoader(onFileCallback onfile, onStatusCallback onstatus, onReadyCallback onready, onErrorCallback onerror, Object& userData) {
	initialize();
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
		if (AtomicGet(&(status.left)) > loadSpeed)
			count = 0;
		else if (AtomicGet(&(status.update))) {
			if (onstatus != NULL) onstatus(status, userData);
			AtomicSet(&(status.update), false);
		}
		while (count > 0) {
			File* info = &(lastDownload());
			String ext = (*info).ref().ext;
			
			AtomicSet(&(status.update), true);
			AtomicIncrement(&(status.left));
			if ((ext.compareToIgnoreCase(L".png") == 0) ||
				(ext.compareToIgnoreCase(L".jpg") == 0) ||
				(ext.compareToIgnoreCase(L".jpeg") == 0)) {
				createImageLoader(*info);
			} else if (
				(ext.compareToIgnoreCase(L".mp3") == 0) ||
				(ext.compareToIgnoreCase(L".ogg") == 0)
			) {
				createSoundLoader(*info);
			} else if (
				(ext.compareToIgnoreCase(L".jimg") == 0) ||
				(ext.compareToIgnoreCase(L".jsh") == 0) ||
				(ext.compareToIgnoreCase(L".json") == 0)) {
				createDataLoader(*info);
			} else if (ext.compareToIgnoreCase(L".json") == 0) {
				createJsonLoader(*info);
			} else {
				createDataLoader(*info);
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

void RefLoader::onroot(struct json_context* ctx, void* target) {
	ctx->callbacks->onobject.onobject = ongroup;
}

void RefLoader::ongroup(struct json_context* ctx, const char* key, void* target) {
	RefLoader* loader = (RefLoader*)target;
	loader->group = key;
	
	json_clear_callbacks(ctx->callbacks, target);
	ctx->callbacks->onobject.onobject = onsubgroup;
}

void RefLoader::onsubgroup(struct json_context* ctx, const char* key, void* target) {
	RefLoader* loader = (RefLoader*)target;
	loader->subgroup = key;
	
	json_clear_callbacks(ctx->callbacks, target);
	ctx->callbacks->onobject.onstring = onsubfile;
}

void RefLoader::onsubfile(struct json_context* ctx, const char* key, char* value, void* target) {
	RefLoader* loader = (RefLoader*)target;
	
	String path = value;
	URI* uri = new URI((wchar_t*)path);
	uri->absolutePath((wchar_t*)(loader->basePath));
	
	loader->list.add(new RefLoader::RefFile(uri->uri(), uri->ext(), key, loader->subgroup));
	loader->status.total++;
	
	delete uri;
}

void RefLoader::load(const char* json) throw(const char*) {
	if (json == NULL)
		throw eNullPointer;
	
	struct json_context ctx;
	struct json_callbacks callbacks;
	ctx.callbacks = &callbacks;
	json_clear_callbacks(&callbacks, this);
	callbacks.onroot = onroot;
	if (!json_call(&ctx, json)) {
#ifdef DEBUG
		json_debug_error(ctx, json);
#endif
		throw eConvert;
	}
	
	run();
}

void RefLoader::createImageLoader(const File& info) {
	
}

void RefLoader::createSoundLoader(const File& info) {
	
}

void RefLoader::createJsonLoader(const File& info) {
	
}

void RefLoader::createDataLoader(const File& info) {
	
}

