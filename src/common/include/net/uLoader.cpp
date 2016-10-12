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

#ifdef __IOS__
	#include <openal/uMixer.h>
#endif

Loader::File::~File() {
	if (this->query != NULL) {
		delete this->query;
		this->query = NULL;
	}
	if (this->post != NULL) {
		memFree(this->post);
		this->post = NULL;
	}
}

Loader::Info::Info(Loader* loader, File* info) {
	this->loader = loader;
	this->info = info;
	if ((info->onquery != NULL) && (info->query != NULL) && (info->post == NULL)) {
		info->post = info->onquery(loader, info->query);
	}
}

Loader::Info::~Info() {
	delete info;
}

void Loader::setCallbacks(onStatusCallback onstatus, onFileCallback onfile, onErrorCallback onerror, onRetryCallback onretry, onReadyCallback onready, onFatalCallback onfatal) {
	this->onstatus = onstatus;
	this->onfile = onfile;
	this->onerror = onerror;
	this->onretry = onretry;
	this->onready = onready;
	this->onfatal = onfatal;
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
				if ((info->ext.compareToIgnoreCase(L"html") == 0) ||
					(info->ext.compareToIgnoreCase(L"css") == 0) ||
					(info->ext.compareToIgnoreCase(L"js") == 0) ||
					(info->ext.compareToIgnoreCase(L"png") == 0) ||
					(info->ext.compareToIgnoreCase(L"jpg") == 0) ||
					(info->ext.compareToIgnoreCase(L"jpeg") == 0)) {
					Info* user = new Info(this, info);
					HTTPClient::Request(info->uri, info->post, true, -1, 5, info->cache, user, onhttp_data, onhttp_error, onhttp_retry, onhttp_fatal, onhttp_release);
				} else if (
					(info->ext.compareToIgnoreCase(L"mp3") == 0) ||
					(info->ext.compareToIgnoreCase(L"ogg") == 0)
				) {
					Info* user = new Info(this, info);
					HTTPClient::Request(info->uri, info->post, true, -1, 5, info->cache, user, onhttp_data, onhttp_error, onhttp_retry, onhttp_fatal, onhttp_release);
				} else if (
					(info->ext.compareToIgnoreCase(L"jimg") == 0) ||
					(info->ext.compareToIgnoreCase(L"jsh") == 0)) {
					Info* user = new Info(this, info);
					HTTPClient::Request(info->uri, info->post, true, -1, 5, info->cache, user, onhttp_data, onhttp_error, onhttp_retry, onhttp_fatal, onhttp_release);
				} else if (
					(info->ext.compareToIgnoreCase(L"json") == 0) ||
					(info->ext.compareToIgnoreCase(L"vsh") == 0) ||
					(info->ext.compareToIgnoreCase(L"fsh") == 0)) {
					Info* user = new Info(this, info);
					HTTPClient::Request(info->uri, info->post, true, -1, 5, info->cache, user, onhttp_text, onhttp_error, onhttp_retry, onhttp_fatal, onhttp_release);
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
			unlock();
		} else if (count == 0) {
			checkUpdate(15);
			unlock();
		} else if (AtomicGet(&(status.error)) > 0) {
			if (onfatal != NULL) onfatal(lastError, userData);
			unlock();
		} else {
			unlock();
			if (onready != NULL) {
				try {
					(void)MainThreadSync(onready, userData);
				} catch (const char* e) {
					lock();
					lastError = e;
					if (onfatal != NULL) onfatal(lastError, userData);
					unlock();
				}
			}
		}
	} else {
		unlock();
	}
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
			Loader::File* info = new Loader::File(uri->path(), uri->file(), uri->uri(), uri->ext(), key, loader->subgroup, true);
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
	lock();
	if (json == NULL)
		throw eNullPointer;
	
	struct JsonContext ctx;
	struct JsonCallbacks callbacks;
	ctx.callbacks = &callbacks;
	JsonClearCallbacks(&callbacks, this);
	callbacks.onrootstart = onjson_root_start;
	if (!JsonCall(&ctx, json)) {
		unlock();
#ifdef DEBUG
		JsonDebugError(ctx, json);
#endif
		throw eConvert;
	}
	unlock();
	
	run();
}

class LoadThreadData {
public:
	Loader* loader;
	CString path;
	CString group;
	CString key;
	Loader::File::onLoad onload;
	Loader::File::onError onerror;
	void* userData;
};

void* Loader::loadThread(void* userData) {
	LoadThreadData* thread = (LoadThreadData*)userData;
	
	try {
		thread->loader->lock();
	
		URI* uri = new URI((wchar_t*)thread->path);
		try {
			uri->absolutePath((wchar_t*)(thread->loader->basePath));
		
			Loader::File* info = new Loader::File(uri->path(), uri->file(), uri->uri(), uri->ext(), thread->key, thread->group, true);
			info->onload = thread->onload;
			info->onerror = thread->onerror;
			info->userData = thread->userData;
			thread->loader->list.push(info);
			AtomicIncrement(&(thread->loader->status.total));
		} catch (...) {
		}
	
		delete uri;
		thread->loader->unlock();
	
		thread->loader->run();
	} catch (...) {
	}
	
	memDelete(thread);
	
	return NULL;
}

void Loader::load(const CString& path, const CString& group, const CString& key, File::onLoad onload, File::onError onerror, void* userData) {
	LoadThreadData* thread = memNew(thread, LoadThreadData);
	thread->loader = this;
	thread->path = path;
	thread->group = group;
	thread->key = key;
	thread->onload = onload;
	thread->onerror = onerror;
	thread->userData = userData;
	MainThreadAsync(loadThread, NULL, thread);
}

void Loader::query(const CString& path, const CString& message, File::onQueryCallback onquery) {
	URI* uri = new URI((wchar_t*)path);
	
	lock();
	try {
		uri->absolutePath((wchar_t*)(basePath));
		CString file = uri->uri();

		int count = list.count();
		File** items = list.items();
		for (int i = 0; i < count; i++) {
			if (items[i]->uri.equals(file)) {
				if (items[i]->query == NULL) {
					items[i]->query = new Vector<CString&>();
				}
				items[i]->onquery = onquery;
				items[i]->query->push(message);
				unlock();
				delete uri;
				return;
			}
		}
	
		Loader::File* info = new Loader::File(uri->path(), uri->file(), uri->uri(), uri->ext(), "query", "api", false);
		if (info->query == NULL) {
			info->query = new Vector<CString&>();
		}
		info->onquery = onquery;
		info->query->push(message);
		list.push(info);
		AtomicIncrement(&(status.total));
	} catch(...) {
		unlock();
		delete uri;
		throw;
	}
	unlock();
	
	delete uri;
	
	run();
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

int Loader::onhttp_error(const CString& url, const CString& error, void* userData) {
	Info* info = (Info*)userData;
	
	try {
		return info->loader->onError(info->info, error);
	} catch (...) {
	}
	
	return 30000;
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

void Loader::onhttp_fatal(const CString& url, const CString& error, void* userData) {
	Info* info = (Info*)userData;
	
	try {
		info->loader->onFatal(info->info, error);
	} catch (...) {
	}
}

bool Loader::onText(const File* info, Stream* stream) {
	if (AtomicGet(&shutdown) != 0)
		return false;
	
	if (info->ext.compareToIgnoreCase(L"vsh") == 0) {
		try {
			GLShader* shader = context->shaders->createVertexShader((wchar_t*)info->key, (char*)stream->getBuffer());
			onLoad(info, shader);
		} catch (...) {
			return false;
		}
	} else if (info->ext.compareToIgnoreCase(L"fsh") == 0) {
		try {
			GLShader* shader = context->shaders->createFragmentShader((wchar_t*)info->key, (char*)stream->getBuffer());
			onLoad(info, shader);
		} catch (...) {
			return false;
		}
	} else if (info->ext.compareToIgnoreCase(L"json") == 0) {
		if (info->group.compareToIgnoreCase(L"models") == 0) {
			try {
				GLModel* model = context->models->createModel((wchar_t*)info->key, (char*)stream->getBuffer());
				onLoad(info, model);
			} catch (...) {
				return false;
			}
		} else {
			try {
				JSONObject* json = memNew(json, JSONObject((char*)stream->getBuffer()));
				try {
					onLoad(info, json);
				} catch (...) {
					memDelete(json);
					return false;
				}
				memDelete(json);
			} catch (...) {
				onLoad(info, NULL);
			}
		}
	} else {
		onLoad(info, stream);
	}
	return true;
}

struct PrepareAudioThread {
	void* audio;
	uint32_t size;
	bool ready;
	
	GLSoundMixer* mixer;
	const Loader::File* info;
};

void onAudioReadyCallback(void* audioHandle, uint32_t bufferSize, uint64_t audioSize, void* userData) {
	PrepareAudioThread* thread = (PrepareAudioThread*)userData;
	
	thread->audio = audioHandle;
	thread->size = bufferSize;
	thread->ready = true;
}

void onAudioErrorCallback(void* userData) {
	PrepareAudioThread* thread = (PrepareAudioThread*)userData;

	thread->ready = false;
}

void* onCreateSoundCallback(void* userData) {
	PrepareAudioThread* thread = (PrepareAudioThread*)userData;

	try {
		GLSound* sound = thread->mixer->createSound(thread->info->key, thread->audio, thread->size);
		return sound;
	} catch (...) {
		destroyAudio(thread->audio);
		throw;
	}
}

bool Loader::onData(const File* info, Stream* stream) {
	if (AtomicGet(&shutdown) != 0)
		return false;
	
	if ((info->ext.compareToIgnoreCase(L"mp3") == 0) ||
		(info->ext.compareToIgnoreCase(L"ogg") == 0)) {
		try {
#if defined(__IOS__)
			PrepareAudioThread thread;
			if (prepareAudio((NSString*)stream->getSourcePath(), &thread, false, onAudioReadyCallback, onAudioErrorCallback) && (thread.ready)) {
				thread.mixer = context->mixer;
				thread.info = info;
				GLSound* sound = (GLSound*)MainThreadSync(onCreateSoundCallback, &thread);
				onLoad(info, sound);
			} else {
				return false;
			}
#elif defined(__JNI__)
	#error PrepareAudio
#endif
		} catch (...) {
			return false;
		}
	} else if (info->ext.compareToIgnoreCase(L"jimg") == 0) {
		try {
			GLTexture* texture = GLReader::createTexture(context, info->key, stream);
			onLoad(info, texture);
		} catch (...) {
			return false;
		}
	} else if (info->ext.compareToIgnoreCase(L"jsh") == 0) {
		try {
			GLShader* shader = GLReader::createShader(context, info->key, stream);
			onLoad(info, shader);
		} catch (...) {
			return false;
		}
	} else {
		onLoad(info, stream);
	}
	return true;
}

void Loader::onLoad(const File* info, void* object) {
	__sync_synchronize();
	
	if (info->onload != NULL) {
		info->onload(info, object, info->userData);
	}
	
	// Тут можно добавить result как в джаве, но он не нужен в си
	
	AtomicIncrement(&(status.count));
	AtomicDecrement(&(status.left));
	if ((info->retry != 0) && (AtomicSub(&(status.error), info->retry) == info->retry)) {
		if (onretry != NULL) onretry(userData);
	}
	AtomicSet(&(status.update), true);

	if (onfile != NULL) onfile(*info, object, userData);
}

int Loader::onError(const File* info, const CString& error) {
	AtomicIncrement(&(status.error));
	((File*)info)->retry++;
	lastError = info->uri;

	if (onerror != NULL) onerror(lastError, userData);
	
	int timeout = info->retry * 500;
	if (timeout > 60000) timeout = 60000;
	
	return timeout;
}

void Loader::onFatal(const File* info, const CString& error) {
	if (info->onerror != NULL) {
		if (info->onerror(info, info->userData)) {
			AtomicDecrement(&(status.left));
			return;
		}
	}
	
	AtomicDecrement(&(status.left));
	AtomicIncrement(&(status.error));
	lastError = info->uri;
}

bool Loader::onRetry(const File* info) {
	if (AtomicGet(&shutdown) != 0)
		return false;
	
	return true;
}
