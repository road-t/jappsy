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

#include "OMMath.h"
#include <core/uSystem.h>

void Rotation::start(GLfloat* target) {
    if (target == NULL) {
        accel = randomf() * (speedup.maxAccel - speedup.minAccel) + speedup.minAccel;
    } else {
        accel = Rotation::Angle(*target);
    }
    state = RotationState::RS_NONE;
    stopped = false;
}

void Rotation::check() {
    if (target >= 0.0) {
        // Запустить кручение если барабан тормозит или не крутится
        if ((speed < speedup.maxSpeed) && (accel <= 0.0)) {
            accel = randomf() * (speedup.maxAccel - speedup.minAccel) + speedup.minAccel;
        }
        if ((speed >= speedup.minSpeed) && (accel >= 0.0)) {
            if ((prev == NULL) || ((prev->speed < Rotation::slowdown.prevSpeed) && (prev->accel <= 0.0))) {
                accel = randomf() * (Rotation::slowdown.maxAccel - Rotation::slowdown.minAccel) + Rotation::slowdown.minAccel;
                
                // Вычисляем примерное время на торможение и расстояние
                GLfloat resultTime = -speed / accel;
                GLfloat resultRotate = speed * resultTime + accel * resultTime * resultTime / 2.0;
                GLfloat absRotate = (resultRotate + rotate) / 360.0;
                absRotate = (absRotate - floorf(absRotate)) * 360.0;
                // Вычисляем куда ближе цель (чуть медленнее или чуть быстрее)
                GLfloat forwardRotate = 0.0;
                GLfloat backwardRotate = 0.0;
                if (absRotate > target) {
                    forwardRotate = 360.0 - (absRotate - target);
                    backwardRotate = absRotate - target;
                } else {
                    forwardRotate = target - absRotate;
                    backwardRotate = 360.0 - (target - absRotate);
                }
                if ((forwardRotate > backwardRotate) && (resultRotate > backwardRotate)) {
                    resultRotate -= backwardRotate;
                } else {
                    resultRotate += forwardRotate;
                }
                // Вычисляем окончательное ускорение для торможения
                GLfloat resultAccel = -speed * speed / (resultRotate * 2.0);
                
                accel = resultAccel;
                target = -1.0;
            }
        }
    }
}

void Rotation::userMove(GLfloat delta) {
    rotate += delta;
    speed = 0.0;
    accel = 0.0;
    updated = true;
}

void Rotation::userRotate(GLfloat delta, GLfloat speed) {
    rotate += delta;
    speed = fabsf(speed);
    if (speed > speedup.maxSpeed)
        speed = speedup.maxSpeed;
    accel = randomf() * (Rotation::slowdown.maxAccel - Rotation::slowdown.minAccel) + Rotation::slowdown.minAccel;
    state = (speed > 0.0) ? RS_BACKWARD : RS_FORWARD;
    updated = true;
}

bool Rotation::update(uint64_t* time) {
    int64_t currentTime = (time != NULL) ? *time : currentTimeMillis();
    if (this->time == 0) {
        this->time = currentTime;
    } else {
        GLfloat deltaTime = (GLfloat)(currentTime - this->time) / 1000.0;
        if (deltaTime > 0) {
            check();
            
            // Определяем текущую скорость, время на ускорение и время с постоянной скоростью
            GLfloat accelTime = deltaTime;
            GLfloat leftTime = 0.0;
            GLfloat resultSpeed = speed + accel * deltaTime;
            GLfloat resultAccel = accel;
            if (resultSpeed > speedup.maxSpeed) { // Предельная скорость
                resultSpeed = speedup.maxSpeed;
                // Вычисляем время до максимальной скорости
                if (accel != 0.0)
                    accelTime = (resultSpeed - speed) / accel;
                else
                    accelTime = 0.0;
                leftTime = deltaTime - accelTime;
                resultAccel = 0.0; // отрубаем ускорение
            } else if (resultSpeed < 0.0) { // Предельное торможение
                resultSpeed = 0.0;
                // Вычисляем время до полного торможения
                if (accel != 0.0)
                    accelTime = (resultSpeed - speed) / accel;
                else
                    accelTime = 0.0;
                leftTime = deltaTime - accelTime;
                resultAccel = 0.0; // отрубаем ускорение
            }
            GLfloat resultRotate = speed * accelTime + accel * accelTime * accelTime / 2.0 + resultSpeed * leftTime;
            if (state != RS_BACKWARD) {
                rotate = Rotation::Angle(rotate + resultRotate);
            } else {
                rotate = Rotation::Angle(rotate - resultRotate);
            }
            speed = resultSpeed;
            accel = resultAccel;
            this->time = currentTime;
            if (speed != 0) {
                stopped = false;
            }
            if (updated) {
                updated = false;
                return true;
            }
            return resultRotate != 0.0;
        }
    }
    if (updated) {
        updated = false;
        return true;
    }
    return false;
}

int Rotation::offset() {
    GLfloat ofs = roundf(Rotation::Angle(rotate) / 45.0);
    ofs -= floorf(ofs / 8) * 8;
    return (int)ofs;
}

bool Rotation::stoppedBars(Rotation* bars) {
    if ((bars[0].speed == 0) && (bars[1].speed == 0) && (bars[2].speed == 0)) {
        if ((!bars[0].stopped) && (!bars[1].stopped) && (!bars[2].stopped)) {
            bars[0].stopped = bars[1].stopped = bars[2].stopped = true;
            if ((bars[0].state == RS_NONE) && (bars[1].state == RS_NONE) && (bars[2].state == RS_NONE)) {
                return true;
            }
        }
    }
    return false;
}

bool Rotation::updateBars(Rotation* bars, uint64_t* time) {
    bool res1 = bars[0].update(time);
    bool res2 = bars[1].update(time);
    bool res3 = bars[2].update(time);
    return res1 || res2 || res3;
}

int Rotation::calculateBars(Rotation* bars, int* mask) {
    int ofs1 = bars[0].offset();
    int ofs2 = bars[1].offset();
    int ofs3 = bars[2].offset();
    if (ofs1 == 1) { // S1 ? ?
        if (ofs2 == 1) { // S1 S1 ?
            if (ofs3 == 1) { // S1 S1 S1
                mask[0] = mask[1] = mask[2] = 1;
                return 888;
            } else { // S1 S1 *
                mask[0] = mask[1] = 1; mask[2] = 0;
                return 88;
            }
        } else { // S1 * *
            mask[0] = 1; mask[1] = mask[2] = 0;
            return 8;
        }
    } else if (ofs1 == 3) { // S2 ? ?
        if (ofs2 == 3) { // S2 S2 ?
            if (ofs3 == 3) { // S2 S2 S2
                mask[0] = mask[1] = mask[2] = 2;
                return 888;
            } else { // S2 S2 *
                mask[0] = mask[1] = 2; mask[2] = 0;
                return 88;
            }
        } else if (ofs2 == 1) { // S2 S1 ?
            if (ofs3 == 3) { // S2 S1 S2
                mask[0] = mask[2] = 2; mask[1] = 1;
                return -8;
            } else { // S2 * *
                mask[0] = 2; mask[1] = mask[2] = 0;
                return 8;
            }
        } else { // S2 * *
            mask[0] = 2; mask[1] = mask[2] = 0;
            return 8;
        }
    } else if (ofs1 == 5) { // S3 ? ?
        if (ofs2 == 5) { // S3 S3 ?
            if (ofs3 == 5) { // S3 S3 S3
                mask[0] = mask[1] = mask[2] = 3;
                return 888;
            } else { // S3 S3 *
                mask[0] = mask[1] = 3; mask[2] = 0;
                return 88;
            }
        } else if (ofs2 == 3) { // S3 S2 ?
            if (ofs3 == 5) { // S3 S2 S3
                mask[0] = mask[2] = 3; mask[1] = 2;
                return -8;
            } else { // S3 * *
                mask[0] = 3; mask[1] = mask[2] = 0;
                return 8;
            }
        } else { // S3 * *
            mask[0] = 3; mask[1] = mask[2] = 0;
            return 8;
        }
    } else if (ofs1 == 6) { // M1 ? ?
        if (ofs2 == 6) { // M1 M1 ?
            if (ofs3 == 6) { // M1 M1 M1
                mask[0] = mask[1] = mask[2] = 1;
                return 444;
            } else { // M1 M1 *
                mask[0] = mask[1] = 1; mask[2] = 0;
                return 44;
            }
        } else { // M1 * *
            mask[0] = 1; mask[1] = mask[2] = 0;
            return 4;
        }
    } else if (ofs1 == 0) { // M2 ? ?
        if (ofs2 == 0) { // M2 M2 ?
            if (ofs3 == 0) { // M2 M2 M2
                mask[0] = mask[1] = mask[2] = 2;
                return 444;
            } else {
                mask[0] = mask[1] = 2; mask[2] = 0;
                return 44;
            }
        } else if (ofs2 == 6) { // M2 M1 ?
            if (ofs3 == 0) { // M2 M1 M2
                mask[0] = mask[2] = 2; mask[1] = 1;
                return -4;
            } else { // M2 * *
                mask[0] = 2; mask[1] = mask[2] = 0;
                return 4;
            }
        } else { // M2 * *
            mask[0] = 2; mask[1] = mask[2] = 0;
            return 4;
        }
    } else if (ofs1 == 2) { // M3 ? ?
        if (ofs2 == 2) { // M3 M3 ?
            if (ofs3 == 2) { // M3 M3 M3
                mask[0] = mask[1] = mask[2] = 3;
                return 444;
            } else { // M3 M3 *
                mask[0] = mask[1] = 3; mask[2] = 0;
                return 44;
            }
        } else if (ofs2 == 0) { // M3 M2 ?
            if (ofs3 == 2) { // M3 M2 M3
                mask[0] = mask[2] = 3; mask[1] = 2;
                return -4;
            } else { // M3 * *
                mask[0] = 3; mask[1] = mask[2] = 0;
                return 4;
            }
        } else { // M3 * *
            mask[0] = 3; mask[1] = mask[2] = 0;
            return 4;
        }
    }
    mask[0] = mask[1] = mask[2] = 0;
    return 0;
}

void Rotation::generateBars(Rotation* bars, int grace) {
    int ofs1, ofs2, ofs3;
    
    if (grace == 888) {
        ofs1 = ofs2 = ofs3 = randomi(3)*2+1; // S1 S1 S1 | S2 S2 S2 | S3 S3 S3
    } else if (grace == 444) {
        ofs1 = ofs2 = ofs3 = (randomi(3)*2-2+8) % 8; // M1 M1 M1 | M2 M2 M2 | M3 M3 M3
    } else if (grace == 88) {
        ofs1 = ofs2 = randomi(3)*2+1; // S1 S1 ? | S2 S2 ? | S3 S3 ?
        ofs3 = (randomi(7)+1+ofs1) % 8; // S1 S1 !S1 | S2 S2 !S2 | S3 S3 !S3
    } else if (grace == 44) {
        ofs1 = ofs2 = (randomi(3)*2-2+8) % 8; // M1 M1 ? | M2 M2 ? | M3 M3 ?
        ofs3 = (randomi(7)+1+ofs1) % 8; // M1 M1 !M1 | M2 M2 !M2 | M3 M3 !M3
    } else if (grace == 8) {
        ofs1 = randomi(3)*2+1; // S1 ? ? | S2 ? ? | S3 ? ?
        ofs2 = (randomi(7)+1+ofs1) % 8; // S1 !S1 ? | S2 !S2 ? | S3 !S3 ?
        ofs3 = (randomi(7)+1+ofs1) % 8; // S1 !S1 !S1 | S2 !S2 !S2 | S3 !S3 !S3
    } else if (grace == 4) {
        ofs1 = (randomi(3)*2-2+8) % 8; // M1 ? ? | M2 ? ? | M3 ? ?
        ofs2 = (randomi(7)+1+ofs1) % 8; // M1 !M1 ? | M2 !M2 ? | M3 !M3 ?
        ofs3 = (randomi(7)+1+ofs1) % 8; // M1 !M1 !M1 | M2 !M2 !M2 | M3 !M3 !M3
    } else if (grace == -8) {
        ofs1 = ofs3 = randomi(2)*2+3; // S2 ? S2 | S3 ? S3
        ofs2 = ofs1 - 2; // S2 S1 S2 | S3 S2 S3
    } else if (grace == -4) {
        ofs1 = ofs3 = randomi(2)*2; // M2 ? M2 | M3 ? M3
        ofs2 = (ofs1 - 2+8) % 8; // M2 M1 M2 | M3 M2 M3
    } else {
        ofs1 = randomi(2)*3+4; // !S!M ? ?
        ofs2 = randomi(8);
        ofs3 = randomi(8);
    }
    
    GLfloat target[3];
    target[0] = ofs1 * 45;
    target[1] = ofs2 * 45;
    target[2] = ofs3 * 45;
    bars[0].start(&(target[0]));
    bars[1].start(&(target[1]));
    bars[2].start(&(target[2]));
}

#define tempseqlen  18
static int tempseq[tempseqlen] = { 0, -4, 0, 4, 0, -8, 0, 0, 0, 0, 0, 8, 0, 0, 44, 88, 444, 888 };
static int tempofs = 0;
#define graceslen   9
static int graces[graceslen] = { 0, 4, 8, -4, -8, 44, 88, 444, 888 };
int Rotation::generateGrace(Rotation* bars) {
    int grace = 0;
    grace = tempseq[tempofs]; tempofs++; if (tempofs >= tempseqlen) tempofs = 0;
    /*
     if (randomi(4) == 0) {
     grace = graces[randomi(8)+1];
     }
     */
    generateBars(bars, grace);
    return grace;
}

#define tempdblseqlen   4
static int tempdblseq[tempdblseqlen] = { 0, 1, 1, 0 };
static int tempdblofs = 0;
int Rotation::generateDouble() {
    int index = 0;
    /*
     index = randomi(2);
     */
    index = tempdblseq[tempdblofs]; tempdblofs++; if (tempdblofs >= tempdblseqlen) tempdblofs = 0;
    return index;
}
