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

#ifndef OMGAME_H
#define OMGAME_H

#include <opengl/uGLEngine.h>
#include <event/uMotionEvent.h>
#include <data/uStream.h>
#include <net/uLoader.h>
#include <opengl/uGLPaint.h>
#include "OMBars.h"

class OMGame : public GLEngine {
public:
    OMGame();
    
    GLPaint m_paint;
    Bars bars;
    
    bool ready = false;
    
    void onFrame(GLRender* context);
    void onTouch(const wchar_t* event);
    void onFile(const CString& url, void* object);
    void onStatus(const LoaderStatus& status);
    void onReady();
    void onError(const CString& error);
};

#endif //OMGAME_H
