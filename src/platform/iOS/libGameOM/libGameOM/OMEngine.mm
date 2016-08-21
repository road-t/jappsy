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
        context->drawEffectMobile(0, 0, 500, 500, L"mobile_effect_sun1", 0.5, worldTime);
    }
}

void OMEngine::onTouch(const wchar_t* event) {
    
}

void OMEngine::onFile(const JString& url, const JObject& object) {
    JString::format(L"FILE: %ls", (wchar_t*)url).log();
}

void OMEngine::onStatus(const LoaderStatus& status) {
    JString::format(L"STATUS: %d / %d", status.count, status.total).log();
}

void OMEngine::onReady(const JSONObject& result) {
    JString::format(L"READY!").log();
    
    JSONObject fallback = new JSONObject();
    
    // Подготавливаем шейдеры
    
    const char *sOMShadersRes =
        #include "OMShaders.res"
    ;
    
    if (!context->createShaders(sOMShadersRes)) {
        shutdown();
    }
    
    // Подготавливаем модели
    JSONObject models = result.optJSONObject(L"models", fallback);
    context->createModels(models);
    
    m_paint.setColor(0xFF4080FF);
    ready = true;
}

void OMEngine::onError(const JString& error) {
    JString::format(L"ERROR: %ls", (wchar_t*)error).log();
}

#include <data/uStack.h>
#include <data/uIterator.h>
#include <data/uListIterator.h>

OMEngine::OMEngine() {
    CStack<int> stack;
    stack.push(1);
    stack.pop();
    CIterator<int> it;
    it.push(1);
    CListIterator<int> lit;
    lit.push(1);
    
    const char *sOMLoadRes =
        #include "OMLoad.res"
    ;
 
    //sOMLoadRes = "{\"groups\":{\"shaders\":{\"vsh_main\":\"shaders/vsh_main.jsh\"}}}";
    
    THIS.setBasePath(L"https://www.cox.ru/res/om/");
    THIS.load(sOMLoadRes);
}
