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

#include <android/log.h>
#include <platform.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <malloc.h>
#include <jappsy.h>
#include <opengl/uGLEngine.h>

static int color = 0;

class OMEngine : public GLEngine {
public:
	void onFrame(GLRender* context) {
		float c = (float) color / 255.0f;
		glClearColor(0.0f, c, 0.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		color++;
		if (color >= 256) color = 0;
	}
};

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL
Java_com_jappsy_OMEngine_onCreate(JNIEnv *env, jclass type) {
	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "onCreate");

	try {
		OMEngine *engine = new OMEngine();
		return (jlong) (intptr_t) (engine);
	} catch (...) {
		return (jlong) NULL;
	}

	return (jlong) NULL;
}

#ifdef __cplusplus
}
#endif
