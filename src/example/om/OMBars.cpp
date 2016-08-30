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

#include "OMBars.h"
#include <core/uSystem.h>

Bars::Bars() {
    
}

Bars::~Bars() {
    clear();
}

void Bars::clear() {
    int32_t count = effects.count();
    Effect** items = effects.items();
    for (int i = 0; i < count; i++) {
        delete items[i];
    }
    effects.clear();
}

bool Bars::update() {
    uint64_t currentTime = currentTimeMillis();
    return Rotation::updateBars(rotation, &currentTime);
}

bool Bars::stopped() {
    return Rotation::stoppedBars(rotation);
}

void Bars::start() {
    rotation[0].start();
    rotation[1].start();
    rotation[2].start();
    clear();
}

int Bars::stopRandom() {
    int sum = Rotation::generateGrace(rotation);
    clear();
    return sum;
}

int Bars::calculate() {
    int sum = Rotation::calculateBars(rotation, mask);
    
    bool mobile = true;
    uint64_t currentTime = currentTimeMillis();
    
    for (int i = 0; i < 3; i++) {
        if (mask[i] != 0) {
            int ofs = rotation[i].offset();
            CString key;
            if ((ofs == 0) || (ofs == 2) || (ofs == 6)) {
                key = mobile ? L"mobile_effect_moon1" : L"effect_moon1";
            } else if ((ofs == 1) || (ofs == 3) || (ofs == 5)) {
                key = mobile ? L"mobile_effect_sun1" : L"effect_sun1";
            }
            int x = 960+(i-1)*456;
            int y = 572;
            switch (ofs) {
                case 1:
                case 6:
                    y -= 242;
                    break;
                case 2:
                case 5:
                    y += 236;
                    break;
            }
            Effect* effect = new Effect(key, x, y, 150, currentTime, -1, -1, mobile);
            effects.push( effect );
        }
    }
    
    return sum;
}

void Bars::renderEffects(GLRender* context) {
    int32_t count = effects.count();
    Effect** items = effects.items();
    uint64_t worldTime = currentTimeMillis();
    for (int i = count-1; i >= 0; i--) {
        if (!items[i]->render(context, worldTime)) {
            delete items[i];
            effects.remove(i);
            items = effects.items();
        }
    }
}
