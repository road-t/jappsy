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
#include <opengl/uGLReader.h>

void Loader::setCallbacks(onFileCallback onfile, onStatusCallback onstatus, onReadyCallback onready, onErrorCallback onerror) {
	this->onfile = onfile;
	this->onstatus = onstatus;
	this->onready = onready;
	this->onerror = onerror;
}

void Loader::release() {
	lock();
	AtomicSet(&shutdown, 1);
	
	do {
		if (AtomicLockTry(&updating))
			break;
		
		unlock();
		usleep(1);
		lock();
	} while (true);
	
	while (AtomicGet(&status.left) > 0) {
		usleep(1);
	}
	
	AtomicUnlock(&updating);
	unlock();
}

Loader::~Loader() {
	release();

	int32_t count = list.count();
	if (count > 0) {
		File** items = list.items();
		for (int i = 0; i < count; i++) {
			delete items[i];
		}
	}
}

struct LoaderUpdateWaitData {
	Loader* loader;
	int time;
};

void Loader::checkUpdate(int time) {
	lock();
	if (AtomicGet(&shutdown) == 0) {
		if (AtomicLockTry(&updating)) {
			struct LoaderUpdateWaitData* threadData = memAlloc(struct LoaderUpdateWaitData, threadData, sizeof(struct LoaderUpdateWaitData));
			if (threadData == NULL) {
				AtomicUnlock(&updating);
				unlock();
				return;
			}
			
			threadData->loader = this;
			threadData->time = time;
			NewThreadAsync(onUpdate, NULL, threadData);
		}
	}
	unlock();
}

void* Loader::onUpdate(void* data) {
	struct LoaderUpdateWaitData* threadData = (struct LoaderUpdateWaitData*)data;
	Loader* loader = (Loader*)AtomicGetPtr(&(threadData->loader));
	systemSleep(AtomicGet(&(threadData->time)));
	memFree(threadData);
	
	loader->update();
	return NULL;
}

void Loader::update() {
	lock();
	AtomicUnlock(&updating);

	if (AtomicGet(&shutdown) == 0) {
		cacheid = currentTimeMillis();
		
		int count = list.count();
		if (AtomicGet(&(status.error)) > 0) count = 0;
		if (count > loadSpeed) count = loadSpeed;
		if (count == 0) count = -1;
		if (AtomicGet(&(status.left)) >= loadSpeed) count = 0;
		else if (AtomicGet(&(status.update))) {
			if (onstatus != NULL) onstatus(status, userData);
			AtomicSet(&(status.update), false);
		}
		while (count > 0) {
			File* info = list.pop();
			try {
				bool releaseInfo = false;
				AtomicSet(&(status.update), true);
				AtomicIncrement(&(status.left));
				if ((info->ext.compareToIgnoreCase(L"png") == 0) ||
					(info->ext.compareToIgnoreCase(L"jpg") == 0) ||
					(info->ext.compareToIgnoreCase(L"jpeg") == 0)) {
					// TODO: PNG JPG Loader not needed
					// Fake OK
					AtomicIncrement(&(status.count));
					AtomicDecrement(&(status.left));
					AtomicSet(&(status.update), true);
					releaseInfo = true;
				} else if (
					(info->ext.compareToIgnoreCase(L"mp3") == 0) ||
					(info->ext.compareToIgnoreCase(L"ogg") == 0)
				) {
					// TODO: MP3 OGG Loader
					// Fake OK
					AtomicIncrement(&(status.count));
					AtomicDecrement(&(status.left));
					AtomicSet(&(status.update), true);
					releaseInfo = true;
				} else if (
					(info->ext.compareToIgnoreCase(L"jimg") == 0) ||
					(info->ext.compareToIgnoreCase(L"jsh") == 0)) {
					Info* user = new Info(this, info);
					HTTPClient::Request(info->uri, true, 3, 5, user, onhttp_data, onhttp_error, onhttp_retry, onhttp_release);
				} else if (
					(info->ext.compareToIgnoreCase(L"json") == 0) ||
					(info->ext.compareToIgnoreCase(L"vsh") == 0) ||
					(info->ext.compareToIgnoreCase(L"fsh") == 0)) {
					Info* user = new Info(this, info);
					HTTPClient::Request(info->uri, true, 3, 5, user, onhttp_text, onhttp_error, onhttp_retry, onhttp_release);
				} else {
					// Unknown File Type
					// Fake OK
					AtomicIncrement(&(status.count));
					AtomicDecrement(&(status.left));
					AtomicSet(&(status.update), true);
					releaseInfo = true;
				}
				if (releaseInfo) {
					delete info;
				}
			} catch (...) {
				AtomicIncrement(&(status.error));
				lastError = eOutOfMemory;
				delete info;
			}
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
					(void)MainThreadSync(onready, userData);
				} catch (const char* e) {
					lastError = e;
					if (onerror != NULL) onerror(lastError, userData);
				}
			}
		}
	}
	unlock();
}

void Loader::run() {
	checkUpdate(15);
}

void Loader::onjson_root_start(struct JsonContext* ctx, void* target) {
	ctx->callbacks->onobject.onobjectstart = onjson_group;
}

void Loader::onjson_group(struct JsonContext* ctx, const char* key, void* target) {
	Loader* loader = (Loader*)target;
	loader->group = key;
	
	JsonClearCallbacks(ctx->callbacks, target);
	ctx->callbacks->onobject.onobjectstart = onjson_subgroup;
}

void Loader::onjson_subgroup(struct JsonContext* ctx, const char* key, void* target) {
	Loader* loader = (Loader*)target;
	loader->subgroup = key;
	
	JsonClearCallbacks(ctx->callbacks, target);
	ctx->callbacks->onobject.onstring = onjson_subfile;
}

void Loader::onjson_subfile(struct JsonContext* ctx, const char* key, char* value, void* target) {
	Loader* loader = (Loader*)target;
	
	CString path = value;
	URI* uri = new URI((wchar_t*)path);
	
	try {
		uri->absolutePath((wchar_t*)(loader->basePath));
	
		if (!loader->subgroup.startsWith(L"disable/") && (strstr(key, "disable/") == NULL)) {
			Loader::File* info = new Loader::File(uri->path(), uri->file(), uri->uri(), uri->ext(), key, loader->subgroup);
			loader->list.push(info);
			AtomicIncrement(&(loader->status.total));
		}
	} catch (...) {
		delete uri;
		throw;
	}
	
	delete uri;
}

void Loader::load(const char* json) throw(const char*) {
	if (json == NULL)
		throw eNullPointer;
	
	struct JsonContext ctx;
	struct JsonCallbacks callbacks;
	ctx.callbacks = &callbacks;
	JsonClearCallbacks(&callbacks, this);
	callbacks.onrootstart = onjson_root_start;
	if (!JsonCall(&ctx, json)) {
#ifdef DEBUG
		JsonDebugError(ctx, json);
#endif
		throw eConvert;
	}
	
	run();
}

Loader::Info::Info(Loader* loader, File* info) {
	this->loader = loader;
	this->info = info;
}

Loader::Info::~Info() {
	delete info;
}

void Loader::onhttp_release(void* userData) {
	Info* info = (Info*)userData;
	delete info;
}

bool Loader::onhttp_text(const CString& url, Stream* stream, void* userData) {
	Info* info = (Info*)userData;
	
	bool result = false;
	try {
		result = info->loader->onText(info->info, stream);
	} catch (...) {
	}
	
	return result;
}

bool Loader::onhttp_data(const CString& url, Stream* stream, void* userData) {
	Info* info = (Info*)userData;

	bool result = false;
	try {
		result = info->loader->onData(info->info, stream);
	} catch (...) {
	}
	
	return result;
}

void Loader::onhttp_error(const CString& url, const CString& error, void* userData) {
	Info* info = (Info*)userData;
	
	try {
		info->loader->onError(info->info, error);
	} catch (...) {
	}
}

bool Loader::onhttp_retry(const CString& url, void* userData) {
	Info* info = (Info*)userData;
	
	bool result = false;
	try {
		result = info->loader->onRetry(info->info);
	} catch (...) {
	}
	
	return result;
}

bool Loader::onText(const File* info, Stream* stream) {
	if (AtomicGet(&shutdown) != 0)
		return false;
	
	if (info->ext.compareToIgnoreCase(L"vsh") == 0) {
		try {
			GLShader* shader = context->shaders->createVertexShader((wchar_t*)info->key, (char*)stream->getBuffer());
			onLoad(info, shader);
			if (onfile != NULL) onfile(info->uri, shader, userData);
		} catch (...) {
			return false;
		}
	} else if (info->ext.compareToIgnoreCase(L"fsh") == 0) {
		try {
			GLShader* shader = context->shaders->createFragmentShader((wchar_t*)info->key, (char*)stream->getBuffer());
			onLoad(info, shader);
			if (onfile != NULL) onfile(info->uri, shader, userData);
		} catch (...) {
			return false;
		}
	} else if ((info->ext.compareToIgnoreCase(L"json") == 0) && (info->group.compareToIgnoreCase(L"models") == 0)) {
		try {
			GLModel* model = context->models->createModel((wchar_t*)info->key, (char*)stream->getBuffer());
			onLoad(info, model);
			if (onfile != NULL) onfile(info->uri, model, userData);
		} catch (...) {
			return false;
		}
	} else {
		onLoad(info, stream);
		if (onfile != NULL) onfile(info->uri, stream, userData);
	}
	return true;
}

bool Loader::onData(const File* info, Stream* stream) {
	if (AtomicGet(&shutdown) != 0)
		return false;
	
	if (info->ext.compareToIgnoreCase(L"jimg") == 0) {
		try {
			GLTexture* texture = GLReader::createTexture(context, info->key, stream);
			onLoad(info, texture);
			if (onfile != NULL) onfile(info->uri, texture, userData);
		} catch (...) {
			return false;
		}
	} else if (info->ext.compareToIgnoreCase(L"jsh") == 0) {
		try {
			GLShader* shader = GLReader::createShader(context, info->key, stream);
			onLoad(info, shader);
			if (onfile != NULL) onfile(info->uri, shader, userData);
		} catch (...) {
			return false;
		}
	} else {
		onLoad(info, stream);
		if (onfile != NULL) onfile(info->uri, stream, userData);
	}
	return true;
}

void Loader::onError(const File* info, const CString& error) {
	AtomicDecrement(&(status.left));
	AtomicIncrement(&(status.error));
	lastError = info->uri;
}

bool Loader::onRetry(const File* info) {
	if (AtomicGet(&shutdown) != 0)
		return false;
	
	return true;
}

void Loader::onLoad(const File* info, void* object) {
	__sync_synchronize();

	// Тут можно добавить result как в джаве, но он не нужен в си

	AtomicIncrement(&(status.count));
	AtomicDecrement(&(status.left));
	AtomicSet(&(status.update), true);
}

