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

#include "OMGame.h"
#include "OMView.h"

//#include <data/uObject.h>
//#include <data/uString.h>
//#include <opengl/uOpenGL.h>
#include <opengl/uGLRender.h>
//#include <core/uSystem.h>

static int color = 0;

void OMGame::onFrame(GLRender* context) {
    float c = (float)color / 255.0f;
    glClearColor(0.0f, c, 0.0f, 1.0f);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    color++;
    if (color >= 256) color = 0;
    
    /*
    if (ready) {
        context->drawRect(0, 0, 500, 500, m_paint);
        
        context->drawTexture(100, 100, 400, 400, L"mramor.png");
        
        uint64_t time = currentTimeMillis();
        GLfloat worldTime = (GLfloat)(time % 86400000) / 86400000.0;
        context->drawEffectMobile(0, 0, 500, 500, L"mobile_effect_sun1", 0.5, worldTime);
    }
     */
}

void OMGame::onTouch(const wchar_t* event) {
    
}

void OMGame::onFile(const CString& url, void* object) {
    CString::format(L"FILE: %ls", (wchar_t*)url).log();
}

void OMGame::onStatus(const LoaderStatus& status) {
    CString::format(L"STATUS: %d / %d", status.count, status.total).log();
}

void OMGame::onReady() {
    CString::format(L"READY!").log();
    
    // Подготавливаем шейдеры
    
    const char *sOMShadersRes =
        #include "OMShaders.res"
    ;
    
    //NSBundle* bundle = [NSBundle bundleWithIdentifier:@"com.jappsy.libGameOM"];
    NSBundle* bundle = [NSBundle mainBundle];
    void* library = (__bridge void*)bundle;
    
    if (!context->createShaders(sOMShadersRes, library)) {
        shutdown();
    }
    
    if (!context->createModels("{}", library)) {
        shutdown();
    }
    
    const char *sOMSpritesRes =
        #include "OMSprites.res"
    ;
    
    if (!context->createSprites(sOMSpritesRes)) {
        shutdown();
    }
    
    m_paint.setColor(0xFF4080FF);
    ready = true;
}

void OMGame::onError(const CString& error) {
    CString::format(L"ERROR: %ls", (wchar_t*)error).log();
}

OMGame::OMGame() {
    const char *sOMLoadRes =
        #include "OMLoad.res"
    ;
    
    this->setBasePath(L"https://www.cox.ru/res/om/");
    this->load(sOMLoadRes);
}
