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

#ifndef JAPPSY_UGLENGINE_H
#define JAPPSY_UGLENGINE_H

#include <data/uObject.h>
#include <event/uMotionEvent.h>
#include <net/uLoader.h>
#include <core/uCache.h>

class GLRender;

class GLEngine : public CObject {
public:
	typedef void (*onMinimizeCallback)(bool minimize, bool animate, void* userData);
	
protected:
	GLRender* context = NULL;
	
	bool minimized = false;

	onMinimizeCallback onminimize;
	void* onminimizeUserData = NULL;

public:
	Cache* cache = NULL;

	GLEngine();
	~GLEngine();
	
	void shutdown();
	
	void setBasePath(const CString& basePath);
	void preload(const char* json);
	void load(const char* json);
	
	void minimize(bool minimize, bool animate);
	void setOnMinimize(onMinimizeCallback onminimize = NULL, void* userData = NULL);
	
	void onRender();
	void onUpdate(int width, int height);
	void onTouch(MotionEvent* event);

	virtual void onFrame(GLRender* context) {}
	virtual void onResize(int width, int height, bool minimized) {}
	virtual void onTouch(const CString& event) {}
	
	virtual void onStatus(const LoaderStatus& status) {}
	virtual void onFile(const Loader::File& info, void* object) {}
	virtual void onError(const CString& error) {}
	virtual void onRetry() {}
	
	virtual void onPreload() {}
	virtual void onLoad() {}
	virtual void onFatal(const CString& error) {}
};

#endif //JAPPSY_UGLENGINE_H
