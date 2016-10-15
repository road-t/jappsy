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

package com.jappsy.core;

import com.jappsy.exceptions.ENullPointer;
import com.jappsy.exceptions.EOutOfMemory;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;

public class Memory {

    static {
        System.loadLibrary("JappsyEngine-jni");
    }

    public static native ByteBuffer alloc(int size) throws EOutOfMemory;
    public static native void free(ByteBuffer memory) throws ENullPointer;
    public static native int[] toIntArray(ByteBuffer memory) throws ENullPointer, EOutOfMemory;

    // Streams

    private static final int BUFFER_IO_SIZE = 8000;

    private static long copyStream(final InputStream is, final OutputStream os) throws IOException {
        int size = 0;
        byte[] buf = new byte[256];
        int l;
        while ((l = is.read(buf)) >= 0) {
            os.write(buf, 0, l);
            size += l;
        }
        return size;
    }

    public static byte[] readStream(final InputStream is) throws IOException {
        byte[] result = null;
        BufferedInputStream bis = null;
        BufferedOutputStream bos = null;
        try {
            if (is instanceof BufferedInputStream) {
                bis = (BufferedInputStream)is;
            } else {
                bis = new BufferedInputStream(is);
            }
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            bos = new BufferedOutputStream(baos, BUFFER_IO_SIZE);

			copyStream(bis, bos);
			bos.flush();

			if (baos.size() > 0) {
				result = baos.toByteArray();
			}
		} finally {
            if (bos != null)
                try {
                    bos.close();
                } catch (IOException e) {
					e.printStackTrace();
                }
            if (bis != null)
                try {
                    bis.close();
                } catch (IOException e) {
					e.printStackTrace();
                }
        }
        return result;
    }

}
