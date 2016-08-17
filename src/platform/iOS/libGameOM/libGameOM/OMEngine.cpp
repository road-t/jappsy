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

#include "OMEngine.h"

#include <opengl/uOpenGL.h>
#include <opengl/uGLRender.h>
#include <net/uLoader.h>

static int color = 0;

void onFrame(GLRender* context) {
    float c = (float)color / 255.0f;
    glClearColor(0.0f, c, 0.0f, 1.0f);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    color++;
    if (color >= 256) color = 0;
}

void onTouch(const wchar_t* event) {
    
}

#include <net/uURI.h>
#include <data/uAtomicObject.h>
#include <data/uStack.h>
#include <data/uIterator.h>
#include <data/uListIterator.h>
#include <data/uCollection.h>
#include <data/uList.h>
#include <data/uSet.h>
#include <data/uHashSet.h>
#include <data/uLinkedHashSet.h>
#include <data/uMap.h>
#include <data/uHashMap.h>
#include <data/uLinkedHashMap.h>
#include <data/uSparseArray.h>
#include <data/uNumber.h>
#include <data/uJSONArray.h>
#include <data/uJSONObject.h>

#include <net/uHTTPClient.h>
#include <net/uLoader.h>

bool onStream(const String& url, Stream& stream, const Object& userData) {
//    JSONObject json = new JSONObject( String((const char*)stream.getBuffer(), stream.getSize()) );
//    json.log();
    
    return true;
}

void onError(const String& url, const String& error, const Object& userData) {
    sleep(0);
}

OMEngine::OMEngine() {
    context = memNew(context, GLRender(this, 1920, 1080, ::onFrame, ::onTouch));
    
    const char *sOMLoadRes =
        #include "OMLoad.res"
    ;
    
    context->loader.ref().basePath = L"https://www.cox.ru/res/om/";
    context->loader.ref().load(sOMLoadRes);
    
    HTTPClient::Request(L"https://www.cox.ru/res/om/models/ball.json", false, 0, 5, onStream, onError);
}

OMEngine::~OMEngine() {
    memDelete(context);
    //loader->release();
}