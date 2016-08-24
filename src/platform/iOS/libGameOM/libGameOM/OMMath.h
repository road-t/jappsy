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

#ifndef OMMATH_H
#define OMMATH_H

#include <platform.h>
#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <math.h>

enum RotationState { RS_BACKWARD = -1, RS_NONE = 0, RS_FORWARD = 1};

inline static GLfloat randomf() {
    return (GLfloat)rand() / (GLfloat)(RAND_MAX);
}

inline static int randomi(int max) {
    return rand() % max;
}

class Bars;

class Rotation : public CObject {
public:
    struct {
        GLfloat maxSpeed = 720.0;
        GLfloat minSpeed = 720-180;
        GLfloat maxAccel = 720;
        GLfloat minAccel = 720-360;
    } speedup;
    
    struct {
        GLfloat maxAccel = -500;
        GLfloat minAccel = -400;
        GLfloat prevSpeed = 360; // Если больше speedup.maxSpeed то торможение не зависит от пред.барабана
    } slowdown;
    
public:
    GLfloat rotate = 0.0;
    GLfloat speed = 0.0;
    GLfloat accel = 0.0;
    GLfloat time = 0.0;
    GLfloat target = -1;
    Rotation* prev = NULL;
    bool stopped = true;
    
    bool updated = false;
    RotationState state = RS_NONE;
    
    inline static GLfloat Angle(GLfloat angle) {
        return angle - (floorf(angle / 360.0) * 360.0);
    }
    
    void start(GLfloat* target = NULL);
    void check();
    void userMove(GLfloat delta);
    void userRotate(GLfloat delta, GLfloat speed);
    bool update(uint64_t* time = NULL);
    int offset();
    
    static bool stoppedBars(Rotation* bars);
    static bool updateBars(Rotation* bars, uint64_t* time = NULL);
    static int calculateBars(Rotation* bars, int* mask);
    static void generateBars(Rotation* bars, int grace);
    static int generateGrace(Rotation* bars);
    static int generateDouble();
};

#endif //OMMATH_H