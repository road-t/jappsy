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

package com.jappsy.cipher;

import java.nio.Buffer;

public class CRC {
    public static native int crc7(final Buffer data, long offset, long len);
    public static native int crc16(final Buffer data, long offset, long len);
    public static native int crc32(final Buffer data, long offset, long len);

    static {
        System.loadLibrary("jappsy-jni");
    }
}
