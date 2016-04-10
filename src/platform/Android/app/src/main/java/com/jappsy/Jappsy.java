/*
 * Copyright (C) 2016 The Jappsy Open Source Project
 *
 * Project Web Page http://jappsy.com
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

package com.jappsy;

public class Jappsy {

    public static native boolean initialize();
    public static native void free();

    public static native void malinfo();

    public static boolean m_initialized = false;

    static {
        System.loadLibrary("jappsy-jni");
    }
}
