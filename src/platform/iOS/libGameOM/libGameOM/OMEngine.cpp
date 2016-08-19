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

void onFile(const String& url, const Object& object, const Object& userData) {
    String::format(L"FILE: %ls", (wchar_t*)url).log();
}

void onStatus(const RefLoader::Status& status, const Object& userData) {
    String::format(L"STATUS: %d / %d", status.count, status.total).log();
}

void onReady(const HashMap<String, Stream>& result, const Object& userData) {
    String::format(L"READY!").log();
}

void onError(const String& error, const Object& userData) {
    String::format(L"ERROR: %ls", (wchar_t*)error).log();
}

OMEngine::OMEngine() {
    GLEngine engine = this;
    context = memNew(context, GLRender(engine, 1920, 1080, ::onFrame, ::onTouch));
    
    const char *sOMLoadRes =
        #include "OMLoad.res"
    ;
 
    //sOMLoadRes = "{\"groups\":{\"shaders\":{\"vsh_main\":\"shaders/vsh_main.jsh\"}}}";
    
    RefLoader* loader = &(context->loader.ref());
    loader->basePath = L"https://www.cox.ru/res/om/";
    loader->onfile = onFile;
    loader->onstatus = onStatus;
    loader->onready = onReady;
    loader->onerror = onError;
    loader->userData = engine;
    loader->load(sOMLoadRes);
}

void OMEngine::release() {
    if (context != NULL) {
        memDelete(context);
        context = NULL;
    }
}

OMEngine::~OMEngine() {
    release();
}