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
#include "include/OMGame.h"

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong JNICALL
Java_com_jappsy_OMEngine_onCreate(JNIEnv *env, jclass type) {
	LOG("onCreate");

	try {
		OMGame *engine = new OMGame(
				L"e994a237491a85ff72b9f737bbf47047cfbc6dbb0897ea1eea5e75338a4b13c3",
				L"8ea5f70b15263872760d7e14ce8e579a",
				L"",
				L"EN"
		);
		return (jlong) (intptr_t) (engine);
	} catch (...) {
		return (jlong) NULL;
	}

	return (jlong) NULL;
}

#ifdef __cplusplus
}
#endif
