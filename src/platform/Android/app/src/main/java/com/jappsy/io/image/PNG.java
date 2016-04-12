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

package com.jappsy.io.image;

import com.jappsy.exceptions.EIOInvalidFormat;
import com.jappsy.exceptions.EInvalidParams;
import com.jappsy.exceptions.ENullPointer;
import com.jappsy.exceptions.EOutOfMemory;

import java.nio.ByteBuffer;

// TODO: Temporary Class for Testing

public class PNG {

    public static native boolean isPNG(final ByteBuffer memoryFile) throws ENullPointer;
    public static native boolean getSize(final ByteBuffer memoryFile, int[] size) throws ENullPointer, EInvalidParams;
    public static native ByteBuffer getPixels(final ByteBuffer memoryFile, final int outWidth, final int outHeight, int x1, int y1, int x2, int y2) throws ENullPointer, EOutOfMemory, EIOInvalidFormat, EInvalidParams;

    static {
        System.loadLibrary("jappsy-jni");
    }
}
