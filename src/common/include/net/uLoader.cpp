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
	this->onfile = onfile;
	this->onstatus = onstatus;
	this->onready = onready;
	this->onerror = onerror;
	this->userData = userData;
}

void RefLoader::release() {
	this->wait();
	AtomicSet(&shutdown, 1);
	handler.release();
	this->notifyAll();
}

RefLoader::~RefLoader() {
	release();
	// TODO: RELEASE LOADER
}

void RefLoader::checkUpdate(int time) {
	this->wait();
	if (AtomicGet(&shutdown) == 0) {
		if (AtomicLockTry(&updating)) {
			handler->postDelayed(onUpdate, time, this);
		}
	}
	this->notifyAll();
}

void RefLoader::onUpdate(const Object& data) {
	(*(Loader*)(&data))->update();
}

void RefLoader::update() {
	this->wait();
	AtomicUnlock(&updating);

	if (AtomicGet(&shutdown) == 0) {
		cacheid = currentTimeMillis();
		
		
		checkUpdate(15);
	}
	this->notifyAll();
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
