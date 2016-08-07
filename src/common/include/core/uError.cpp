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

#include "uError.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__JNI__)
    static const char _eOK[] = "OK";
    static const char _eOutOfMemory[] = "Out Of Memory";
    static const char _eNullPointer[] = "Null Pointer";
    static const char _eInvalidParams[] = "Invalid Params";
    static const char _eInvalidFormat[] = "Invalid Format";

    static const char _eIOFileNotOpen[] = "File Not Open";
    static const char _eIOInvalidFile[] = "Invalid File";
    static const char _eIOReadLimit[] = "Read Limit Exceed";

	static const char _eInvalidPointer[] = "Invalid Pointer";
	static const char _eConvert[] = "Unable to Convert";
    static const char _eOutOfRange[] = "Out Of Range";
	static const char _eOpenGL[] = "OpenGL";

    static const char _eUnknown[] = "Unknown";
#else
    static const char *_eOK = NULL;
    static const char _eOutOfMemory[] = "com/jappsy/exceptions/EOutOfMemory";
    static const char _eNullPointer[] = "com/jappsy/exceptions/ENullPointer";
    static const char _eInvalidParams[] = "com/jappsy/exceptions/EInvalidParams";
    static const char _eInvalidFormat[] = "com/jappsy/exceptions/EIOInvalidFormat";

    static const char _eIOFileNotOpen[] = "com/jappsy/exceptions/EIOFileNotOpen";
    static const char _eIOInvalidFile[] = "com/jappsy/exceptions/EIOInvalidFile";
    static const char _eIOReadLimit[] = "com/jappsy/exceptions/EIOReadLimit";

	static const char _eInvalidPointer[] = "com/jappsy/exceptions/EInvalidPointer";
	static const char _eConvert[] = "com/jappsy/exceptions/EConvert";
    static const char _eOutOfRange[] = "com/jappsy/exceptions/EOutOfRange";
	static const char _eOpenGL[] = "com/jappsy/exceptions/EOpenGL"; // TODO

    static const char _eUnknown[] = "java/lang/Error";
#endif

const char *eOK = _eOK;
const char *eOutOfMemory = _eOutOfMemory;
const char *eNullPointer = _eNullPointer;
const char *eInvalidParams = _eInvalidParams;
const char *eInvalidFormat = _eInvalidFormat;

const char* eIOFileNotOpen = _eIOFileNotOpen;
const char* eIOInvalidFile = _eIOInvalidFile;
const char* eIOReadLimit = _eIOReadLimit;

const char* eInvalidPointer = _eInvalidPointer;
const char* eConvert = _eConvert;
const char* eOutOfRange = _eOutOfRange;
const char* eOpenGL = _eOpenGL;

const char *eUnknown = _eUnknown;

#if defined(__JNI__)

    void jniThrow(JNIEnv *env, const char* eString, const char* message) {
        if (eString != eOK) {
            jclass jc = env->FindClass(eString);
            if (jc == NULL)
                jc = env->FindClass(eUnknown);
            env->ThrowNew(jc, message);
        }
    }

#endif

bool is_errorPtr(const char* ptr) {
    return (ptr == eOK)
        || (ptr == eOutOfMemory)
        || (ptr == eNullPointer)
        || (ptr == eInvalidParams)
        || (ptr == eInvalidFormat)
        || (ptr == eIOFileNotOpen)
        || (ptr == eIOInvalidFile)
        || (ptr == eIOReadLimit)

        || (ptr == eInvalidPointer)
        || (ptr == eConvert)
        || (ptr == eOutOfRange)
		|| (ptr == eOpenGL);
}

#ifdef __cplusplus
}
#endif
