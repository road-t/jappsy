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

#ifndef OMBARS_H
#define OMBARS_H

#include <platform.h>
#include <data/uObject.h>
#include <data/uVector.h>
#include <opengl/uGLRender.h>

#include "OMMath.h"

class Effect : public CObject {
public:
    CString key;
    GLfloat x1;
    GLfloat y1;
    GLfloat x2;
    GLfloat y2;
    int64_t start;
    int64_t pause;
    int64_t duration;
    bool mobile;
    
    inline Effect(const CString& key, GLfloat x, GLfloat y, GLfloat d, int64_t start = 0, int64_t pause = -1, int64_t duration = -1, bool mobile = true) {
        this->key = key;
        this->x1 = x - d;
        this->y1 = y - d;
        this->x2 = x + d;
        this->y2 = y + d;
        this->start = start;
        this->pause = pause;
        this->duration = duration;
        this->mobile = mobile;
    }
    
    bool render(GLRender* context, uint64_t worldTime) {
        int64_t localTime = 0;
        if (start < worldTime) {
            localTime = worldTime - start;
            if (duration >= 0) {
                if (localTime > duration) {
                    localTime = 0;
                    return false;
                } else if ((pause >= 0) && (localTime > (duration - pause))) {
                    localTime = duration - localTime;
                }
            }
            if ((pause >= 0) && (localTime > pause)) {
                localTime = pause;
            }
        }
        context->drawEffectMobile(x1, y1, x2, y2, key, (GLfloat)(localTime % 86400000) / 86400000.0, (GLfloat)(worldTime % 86400000) / 86400000.0);
        return true;
    }
};

class Bars : public CObject {
public:
    Rotation rotation[3];
    int mask[3];
    Vector<Effect*> effects;
    
    Bars();
    ~Bars();
    
    void clear();
    
    bool update();
    bool stopped();
    void start();
    int stopRandom();
    int stop(wchar_t* ofs); // 3 числа в строке от 0 до 7
    int stopCancel();
    int calculate();
    
    void renderEffects(GLRender* context);
};

#endif //OMBARS_H
