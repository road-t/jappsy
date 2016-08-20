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
#include <core/uSystem.h>

static int color = 0;

void OMEngine::onFrame(GLRender* context) {
    float c = (float)color / 255.0f;
    glClearColor(0.0f, c, 0.0f, 1.0f);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    color++;
    if (color >= 256) color = 0;
    
    if (ready) {
        context->drawRect(0, 0, 500, 500, m_paint);
        
        context->drawTexture(100, 100, 400, 400, L"mramor.png");
        
        uint64_t time = currentTimeMillis();
        GLfloat worldTime = (GLfloat)(time % 86400000) / 86400000.0;
        context->drawEffectMobile(0, 0, 1000, 1000, L"mobile_effect_torsion", 0.5, worldTime);
    }
}

void OMEngine::onTouch(const wchar_t* event) {
    
}

void OMEngine::onFile(const String& url, const Object& object) {
    String::format(L"FILE: %ls", (wchar_t*)url).log();
}

void OMEngine::onStatus(const LoaderStatus& status) {
    String::format(L"STATUS: %d / %d", status.count, status.total).log();
}

void OMEngine::onReady(const HashMap<String, Stream>& result) {
    String::format(L"READY!").log();
    
    const char *sOMShadersRes =
        #include "OMShaders.res"
    ;
    
    if (!context->createShaders(sOMShadersRes)) {
        shutdown();
    }
    
    m_paint.setColor(0xFF4080FF);
    ready = true;
}

void OMEngine::onError(const String& error) {
    String::format(L"ERROR: %ls", (wchar_t*)error).log();
}

OMEngine::OMEngine() {
    const char *sOMLoadRes =
        #include "OMLoad.res"
    ;
 
    //sOMLoadRes = "{\"groups\":{\"shaders\":{\"vsh_main\":\"shaders/vsh_main.jsh\"}}}";
    
    THIS.setBasePath(L"https://www.cox.ru/res/om/");
    THIS.load(sOMLoadRes);
}
