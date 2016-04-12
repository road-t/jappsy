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

package com.jappsy.core.util;

import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.LinkedHashMap;

public class PngText {

    // "image/png", "image/apng", "image/mng"

    private static byte[] pngHeader = {-119, 80, 78, 71, 13, 10, 26, 10};
    private static byte[] mngHeader = {-119, 77, 78, 71, 13, 10, 26, 10};
    private static byte[] iendChunk = {73, 69, 78, 68};
    private static byte[] tEXtChunk = {116, 69, 88, 116};

    private static int findVal(byte[] data, byte val) {
        for (int i = 0; i < data.length; i++) {
            if (data[i] == val) {
                return i;
            }
        }

        return -1;
    }

    public static LinkedHashMap<String,String> extract(InputStream is) throws IOException {
        LinkedHashMap<String,String> keyMap;

        try {
            byte[] header = new byte[8];
            is.read(header);

            if (!Arrays.equals(pngHeader, header) && (!Arrays.equals(mngHeader, header))) {
                return null;
            }

            keyMap = new LinkedHashMap<String,String>();

            byte[] length = new byte[4];
            byte[] chunkType = new byte[4];

            is.read(length);
            is.read(chunkType);

            while (!Arrays.equals(chunkType, iendChunk)) {
                int bytesLen = ((0x00FF & length[0]) << 24) | ((0x00FF & length[1]) << 16) | ((0x00FF & length[2]) << 8) | (0x00FF & length[3]);
                if (Arrays.equals(chunkType, tEXtChunk)) {
                    byte[] textBytes = new byte[bytesLen];
                    is.read(textBytes);
                    int nullPos = findVal(textBytes, (byte)0);
                    String key = new String(textBytes, 0, nullPos, "ISO-8859-1");
                    String value = new String(textBytes, nullPos + 1, textBytes.length - (nullPos + 1), "ISO-8859-1");

                    keyMap.put(key, value);
                } else {
                    long skipped = 0;
                    do {
                        skipped += is.skip(bytesLen);
                    } while (skipped < bytesLen);
                }

                long skipped = 0;
                do {
                    skipped += is.skip(4);
                } while (skipped < 4);
                is.read(length);
                is.read(chunkType);
            }
        } catch (IOException e) {
            keyMap = null;
        } finally {
            is.close();
        }

        return keyMap;
    }

}
