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

OMEngine::OMEngine() {
    context = new GLRender(this, 1920, 1080, ::onFrame, ::onTouch);
}

OMEngine::~OMEngine() {
    delete context;
}