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

#ifndef JAPPSY_UGLSCENEOBJECT_H
#define JAPPSY_UGLSCENEOBJECT_H

#include <platform.h>
#include <data/uObject.h>

class GLObject;

class GLSceneObject : public CObject {
public:
	virtual inline void render(GLObject* object, const GLfloat time = NAN) {}
};

#endif //JAPPSY_UGLSCENEOBJECT_H
