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

#include "uPNG.h"
#include <core/uMemory.h>
#include <io/uFileIO.h>
#include <png.h>
#include <pngstruct.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef png_jmpbuf
    #define png_jmpbuf(png_ptr) ((png_ptr)->png_jmpbuf)
#endif

/*
 * errorCode : eNullPointer | eInvalidFormat | eOK
 */
bool is_png(const void *ptr, uint32_t insize, char** error) {
    if (ptr == NULL) {
        if (error != NULL)
            *error = (char*)eNullPointer;
        return false;
    } else if ((insize < 8)
               || (*(uint32_t *) ptr != PNG_HEAD)
               || (!png_check_sig((png_byte *) ptr, 8))) {
        if (error != NULL)
            *error = (char*)eInvalidFormat;
        return false;
    }

    if (error != NULL)
        *error = (char*)eOK;

    return true;
}

struct pngMem_filep {
    uint8_t *ptr;
    uint32_t ofs;
    uint32_t size;
};

static void pngMem_readData(png_structp png_ptr, png_bytep data, png_size_t length) {
    struct pngMem_filep *file = (struct pngMem_filep *) (png_ptr->io_ptr);
    uint32_t left = file->size - file->ofs;
    if (left < length) {
        png_error(png_ptr, (char*) eIOReadLimit);
    } else {
        memcpy(data, file->ptr + file->ofs, length);
        file->ofs += length;
    }
}

static void pngMem_writeData(png_structp png_ptr, png_bytep data, png_size_t length) {
    struct pngMem_filep *file = (struct pngMem_filep *) (png_ptr->io_ptr);
    if (file->ptr != 0) {
        memcpy(file->ptr + file->ofs, data, length);
    }
    file->ofs += length;
    if (file->ofs > file->size)
        file->size = file->ofs;
}

static void pngMem_flush(png_structp png_ptr) {
}

/*
 * errorCode : eNullPointer | eInvalidFormat | eOutOfMemory | eOK
 */
bool png_getSize(const void *ptr, uint32_t insize, uint32_t *lpWidth, uint32_t *lpHeight, char** error) {
    if (!is_png(ptr, insize, error))
        return false;

    // Create PNG Structures
    struct pngMem_filep file;
    file.ptr = (uint8_t *) ptr;
    file.ofs = 8;
    file.size = insize;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, (png_error_ptr)NULL,
                                                 (png_error_ptr)NULL);
    if (png_ptr == NULL) {
        if (error != NULL)
            *error = (char*)eOutOfMemory;
        return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, NULL);
        if (error != NULL)
            *error = (char*)eOutOfMemory;
        return false;
    }

    // Error Handler
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_imagep image = (png_imagep)(png_ptr->error_ptr);
        if (image != NULL) {
            //TODO: Check LOG("PNG Error: %s", image->message);

            if (error != NULL)
                *error = is_errorPtr(image->message) ? image->message : (char*)eInvalidFormat;
        }

        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }

    png_uint_32 width;
    png_uint_32 height;
    {
        if (error != NULL)
            *error = (char*)eInvalidFormat;

        // Init Structures
        png_set_read_fn(png_ptr, (png_voidp) &file, pngMem_readData);
        png_set_sig_bytes(png_ptr, 8);

        // Read PNG Info
        png_read_info(png_ptr, info_ptr);

        // Get Image Info
        int bitDepth;
        int colorType;
        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bitDepth, &colorType, 0, 0, 0);

        // Finilize PNG
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    }

    if (lpWidth != NULL)
        *lpWidth = (uint32_t) width;

    if (lpHeight != NULL)
        *lpHeight = (uint32_t) height;

    if (error != NULL)
        *error = (char*)eOK;
    return true;
}

/*
 * errorCode : eInvalidParams | eNullPointer | eInvalidFormat | eOutOfMemory | eOK
 */
void* png_getPixels(const void *ptr, uint32_t insize, uint32_t outWidth, uint32_t outHeight, int32_t x1, int32_t y1, int32_t x2, int32_t y2, char** error) {
    // Check Out Size
    if ((outWidth == 0)
        || (outHeight == 0)
        || (x1 >= x2)
        || (y1 >= y2)) {
        if (error != NULL)
            *error = (char*)eInvalidParams;
        return NULL;
    }
    if ((x2 - x1) > outWidth)
        x2 = outWidth + x1;
    if ((y2 - y1) > outHeight)
        y2 = outHeight + y1;
    if ((x2 < 0) || (y2 < 0)) {
        if (error != NULL)
            *error = (char*)eInvalidParams;
        return NULL;
    }

    // Check PNG Header
    if (!is_png(ptr, insize, error))
        return NULL;

    // Create PNG Structures
    struct pngMem_filep file;
    file.ptr = (uint8_t *) ptr;
    file.ofs = 8;
    file.size = insize;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, (png_error_ptr)NULL,
                                                 (png_error_ptr)NULL);
    if (png_ptr == NULL) {
        if (error != NULL)
            *error = (char*)eOutOfMemory;
        return NULL;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, NULL);
        if (error != NULL)
            *error = (char*)eOutOfMemory;
        return NULL;
    }

    png_uint_32 width;
    png_uint_32 height;
    png_uint_32 ulRowBytes;
    png_uint_32 ulChannels;
    png_byte *pbImageData = NULL;
    png_byte **ppbRowPointers = NULL;

    // Error Handler
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_imagep image = (png_imagep)(png_ptr->error_ptr);
        if (image != NULL) {
            //TODO: Check LOG("PNG Error: %s", image->message);

            if (error != NULL)
                *error = is_errorPtr(image->message) ? image->message : (char*)eInvalidFormat;
        }

        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        if (pbImageData != NULL) {
            memFree(pbImageData);
            pbImageData = NULL;
        }
        if (ppbRowPointers != NULL) {
            memFree(ppbRowPointers);
            ppbRowPointers = NULL;
        }
        return NULL;
    }

    {
        if (error != NULL)
            *error = (char *) eInvalidFormat;

        // Init Structures
        png_set_read_fn(png_ptr, (png_voidp) &file, pngMem_readData);
        png_set_sig_bytes(png_ptr, 8);

        // Read PNG Info
        png_read_info(png_ptr, info_ptr);

        // Get Image Info
        int bitDepth;
        int colorType;
        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bitDepth, &colorType, 0, 0, 0);

        // Configure PNG for RGBA888 or RGB888 mode
        if (bitDepth == 16)
            png_set_strip_16(png_ptr);
        if ((colorType == PNG_COLOR_TYPE_PALETTE)
            || (bitDepth < 8)
            || (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)))
            png_set_expand(png_ptr);
        if ((colorType == PNG_COLOR_TYPE_GRAY)
            || (colorType == PNG_COLOR_TYPE_GRAY_ALPHA))
            png_set_gray_to_rgb(png_ptr);

        // Set TRANSPARENT background color
        png_color_16 *pBackground;
        png_color bkgColor;
        png_color *pBkgColor;
        if (png_get_bKGD(png_ptr, info_ptr, &pBackground)) {
            png_set_background(png_ptr, pBackground, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
            bkgColor.red = (png_byte) pBackground->red;
            bkgColor.green = (png_byte) pBackground->green;
            bkgColor.blue = (png_byte) pBackground->blue;
            pBkgColor = &bkgColor;
        } else
            pBkgColor = NULL;

        // Set GAMMA translations
        double dGamma;
        if (png_get_gAMA(png_ptr, info_ptr, &dGamma))
            png_set_gamma(png_ptr, 2.2, dGamma);

        // Update Config
        png_read_update_info(png_ptr, info_ptr);

        // Get Updated Image Info
        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bitDepth, &colorType, 0, 0, 0);

        // Get PNG Text
        /* Для работы с OpenGL эти данные не нужны!!!
        png_text* text_ptr = 0;
        int num_text = 0;
        png_get_text(png_ptr, info_ptr, &text_ptr, &num_text);
        for (int i = 0; i < num_text; i++) {
            if (text_ptr[i].compression == PNG_TEXT_COMPRESSION_NONE) {
                if (strcmp(text_ptr[i].key, "Title") == 0) {
                    bmText = new cString(text_ptr[i].text, text_ptr[i].text_length);
                }
            }
        }
         */

        // Check Image Memory Size Info
        ulRowBytes = png_get_rowbytes(png_ptr, info_ptr);
        ulChannels = png_get_channels(png_ptr, info_ptr);
        if ((ulChannels != 3) && (ulChannels != 4)) {
            if (error != NULL)
                *error = (char *) eInvalidFormat;
            png_error(png_ptr, "Unsupported Format");
        }

        // Allocate PNG Memory
        pbImageData = memAlloc(png_byte, pbImageData,
                               (uint32_t) (ulRowBytes * height * sizeof(png_byte) +
                                           sizeof(uint32_t) * 2));
        if (pbImageData == NULL) {
            if (error != NULL)
                *error = (char *) eOutOfMemory;
            png_error(png_ptr, "OutOfMemory");
        }
        ppbRowPointers = memAlloc(png_bytep, ppbRowPointers,
                                  (uint32_t) (height * sizeof(png_bytep)));
        if (ppbRowPointers == NULL) {
            if (error != NULL)
                *error = (char *) eOutOfMemory;
            png_error(png_ptr, "OutOfMemory");
        }

        // Set Rows
        int i = 0;
        for (; i < height; i++)
            ppbRowPointers[i] = pbImageData + (i * ulRowBytes) + sizeof(uint32_t);

        // Read Image
        png_read_image(png_ptr, ppbRowPointers);

        // Finish Reading
        png_read_end(png_ptr, 0);

        // Free Memory
        memFree(ppbRowPointers);
        ppbRowPointers = NULL;

        // Finilize PNG
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    }

    // Allocate Out Memory
    uint8_t *outResult = memAlloc(uint8_t, outResult,
                                  (uint32_t) (outWidth * outHeight * sizeof(uint32_t)));
    if (outResult == NULL) {
        memFree(pbImageData);
        if (error != NULL)
            *error = (char*)eOutOfMemory;
        return NULL;
    }

    register uint8_t *inBuffer = (uint8_t *) (pbImageData + sizeof(uint32_t));
    register uint8_t *outBuffer = outResult;

    if (y1 < 0) {
        outBuffer -= (y1 * outWidth) * sizeof(uint32_t);
        y1 = 0;
    }
    if (x1 < 0) {
        outBuffer -= x1 * sizeof(uint32_t);
        x1 = 0;
    }
    if (x2 > width) x2 = (int32_t)width;
    if (y2 > height) y2 = (int32_t)height;

    inBuffer += (y1 * ulRowBytes + x1) * ulChannels;

    uint32_t copyWidth = (uint32_t) (x2 - x1);
    uint32_t copyHeight = (uint32_t) (y2 - y1);
    if ((copyWidth == 0)
        || (copyHeight == 0)) {
        memFree(pbImageData);
        if (error != NULL)
            *error = (char*)eInvalidParams;
        return outResult;
    }

    int x, y;
    uint32_t inLineSkip = (uint32_t) ((ulRowBytes - copyWidth * ulChannels) + x1 * ulChannels);
    uint32_t outLineSkip = (uint32_t) ((outWidth - copyWidth) * sizeof(uint32_t));
    if (ulChannels == 3) {
        //inBuffer--;
        for (y = (y2 - y1); y > 0; y--) {
            for (x = copyWidth; x > 0; x--) {
                /*
                *(uint32_t *) outBuffer = __builtin_bswap32(*(uint32_t *) inBuffer) | 0xFF000000;
                */
                *(uint32_t *) outBuffer = *(uint32_t *) inBuffer | 0xFF000000;
                inBuffer += 3;
                outBuffer += sizeof(uint32_t);
            }
            inBuffer += inLineSkip;
            outBuffer += outLineSkip;
        }
    } else {
        for (y = (y2 - y1); y > 0; y--) {
            for (x = copyWidth; x > 0; x--) {
                /*
                uint32_t color = __builtin_bswap32(*(uint32_t *) inBuffer);
                *(uint32_t *) outBuffer = (color >> 8) | (color << (32 - 8));
                */
                *(uint32_t *) outBuffer = *(uint32_t *) inBuffer;
                inBuffer += 4;
                outBuffer += sizeof(uint32_t);
            }
            inBuffer += inLineSkip;
            outBuffer += outLineSkip;
        }
    }

    memFree(pbImageData);
    if (error != NULL)
        *error = (char*)eOK;
    return outResult;
}

bool png_create(const void* pixels, uint32_t inWidth, uint32_t inHeight, bool alpha, void** outPtr, uint32_t* outSize, char** error) {
    if ((pixels == NULL) || (outPtr == NULL) || (outSize == NULL)) {
        if (error != NULL)
            *error = (char*)eNullPointer;
        return false;
    }

    if ((inWidth == 0) || (inHeight == 0)) {
        if (error != NULL)
            *error = (char*)eInvalidParams;
        return false;
    }

    png_uint_32 ulChannels = (alpha ? 4 : 3);
    png_uint_32 width = (png_uint_32)inWidth;
    png_uint_32 height = (png_uint_32)inHeight;
    png_uint_32 ulRowBytes = (((((width*(ulChannels<<3))+7)>>3)+3)&0xFFFFFFFC);
    png_byte *pbImageData = NULL;
    png_byte **ppbRowPointers = NULL;

    // Allocate Temporary PNG Memory
    pbImageData = memAlloc(png_byte, pbImageData,
                           (uint32_t) (ulRowBytes * height * sizeof(png_byte) +
                                       sizeof(uint32_t) * 2));
    if (pbImageData == NULL) {
        if (error != NULL)
            *error = (char *) eOutOfMemory;
        return false;
    }
    ppbRowPointers = memAlloc(png_bytep, ppbRowPointers,
                              (uint32_t) (height * sizeof(png_bytep)));
    if (ppbRowPointers == NULL) {
        memFree(pbImageData);

        if (error != NULL)
            *error = (char *) eOutOfMemory;
        return false;
    }

    // Set Rows
    int i = 0;
    for (; i < height; i++)
        ppbRowPointers[i] = pbImageData + (i * ulRowBytes) + sizeof(uint32_t);

    // Convert OpenGL RGBA Pixels to PNG Pixels
    register uint8_t *inBuffer = (uint8_t*)pixels;
    register uint8_t *outBuffer = (uint8_t *)(pbImageData + sizeof(uint32_t));

    int x, y;
    uint32_t outLineSkip = (uint32_t) (ulRowBytes - width * ulChannels);
    if (ulChannels == 3) {
        for (y = (int)height; y > 0; y--) {
            for (x = (int)width; x > 0; x--) {
                *(uint32_t *) outBuffer = *(uint32_t *) inBuffer;
                inBuffer += sizeof(uint32_t);
                outBuffer += 3;
            }
            outBuffer += outLineSkip;
        }
    } else {
        for (y = (int)height; y > 0; y--) {
            for (x = (int)width; x > 0; x--) {
                *(uint32_t *) outBuffer = *(uint32_t *) inBuffer;
                inBuffer += sizeof(uint32_t);
                outBuffer += 4;
            }
            outBuffer += outLineSkip;
        }
    }

    struct pngMem_filep file;
    file.ptr = NULL;
    file.ofs = 0;
    file.size = 0;

    // Two Pass Loop
    // 1st pass: Calculating Result Memory Size
    // 2nd pass: Creating Memory PNG File
    do {
        // Create PNG Structures
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, (png_error_ptr)NULL,
                                                     (png_error_ptr)NULL);
        if (png_ptr == NULL) {
            memFree(ppbRowPointers);
            memFree(pbImageData);

            if (error != NULL)
                *error = (char*)eOutOfMemory;
            return false;
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) {
            png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
            memFree(ppbRowPointers);
            memFree(pbImageData);

            if (error != NULL)
                *error = (char*)eOutOfMemory;
            return false;
        }

        // Error Handler
        if (setjmp(png_jmpbuf(png_ptr))) {
            png_imagep image = (png_imagep)(png_ptr->error_ptr);
            if (image != NULL) {
                //TODO: Check LOG("PNG Error: %s", image->message);

                if (error != NULL)
                    *error = image->message;
            }

            png_destroy_write_struct(&png_ptr, &info_ptr);
            memFree(ppbRowPointers);
            memFree(pbImageData);

            if (file.ptr != NULL)
                memFree(file.ptr);

            return false;
        }

        {
            if (error != NULL)
                *error = (char *) eInvalidFormat;

            // Init Structures
            png_set_write_fn(png_ptr, (png_voidp)&file, pngMem_writeData, pngMem_flush);

            // Write PNG Header
            if (alpha)
                png_set_IHDR(png_ptr, info_ptr, width, height, 8 /* bitDepth */,
                             PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_ADAM7, PNG_COMPRESSION_TYPE_BASE,
                             PNG_FILTER_TYPE_BASE);
            else
                png_set_IHDR(png_ptr, info_ptr, width, height, 8 /* bitDepth */,
                             PNG_COLOR_TYPE_RGB, PNG_INTERLACE_ADAM7, PNG_COMPRESSION_TYPE_BASE,
                             PNG_FILTER_TYPE_BASE);

            /* Для работы с OpenGL эти данные не нужны!!!
            // Задаём текстовые блоки (тест)
            if (num_text > 0) {
                libpng.png_set_text(png_ptr, info_ptr, text_ptr, num_text);
            }
             */

            png_write_info(png_ptr, info_ptr);

            // Write PNG Image
            png_write_image(png_ptr, ppbRowPointers);
            png_write_end(png_ptr, info_ptr);

            // Finalize PNG
            png_destroy_write_struct(&png_ptr, &info_ptr);
        }

        if (file.ptr == NULL) {
            // Allocate Result Memory
            file.ptr = memAlloc(uint8_t, file.ptr, file.size);
            if (file.ptr == NULL) {
                memFree(ppbRowPointers);
                memFree(pbImageData);

                if (error != NULL)
                    *error = (char*)eOutOfMemory;
                return false;
            }

            file.ofs = 0;
        } else {
            memFree(ppbRowPointers);
            memFree(pbImageData);

            break;
        }
    } while (true);

    *outSize = file.size;
    *outPtr = file.ptr;

    if (error != NULL)
        *error = (char*)eOK;
    return true;
}

#if defined(__JNI__)

JNIEXPORT jboolean JNICALL
Java_com_jappsy_io_image_PNG_isPNG(JNIEnv *env, jclass type, jobject memoryFile) {
    if (memoryFile == NULL) {
        jniThrow(env, eNullPointer, NULL);
        return JNI_FALSE;
    }

    void* memory = (*env)->GetDirectBufferAddress(env, memoryFile);
    jlong memorySize = (*env)->GetDirectBufferCapacity(env, memoryFile);

    char* error;
    if (memory != NULL) {
        if (is_png(memory, (uint32_t)memorySize, &error))
            return JNI_TRUE;
    } else {
        error = (char*)eInvalidParams;
    }

    jniThrow(env, error, NULL);
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_jappsy_io_image_PNG_getSize(JNIEnv *env, jclass type, jobject memoryFile,
                                     jintArray size_) {
    if (memoryFile == NULL) {
        jniThrow(env, eNullPointer, NULL);
        return JNI_FALSE;
    }

    if (size_ == NULL) {
        jniThrow(env, eInvalidParams, NULL);
        return JNI_FALSE;
    }

    jsize count = (*env)->GetArrayLength(env, size_);
    jboolean result = JNI_FALSE;

    if (count >= 2) {
        void* memory = (*env)->GetDirectBufferAddress(env, memoryFile);
        jlong memorySize = (*env)->GetDirectBufferCapacity(env, memoryFile);

        if (memory != NULL) {
            uint32_t width;
            uint32_t height;
            char* error;
            if (png_getSize(memory, (uint32_t)memorySize, &width, &height, &error)) {
                jint newSize[] = { width, height };
                (*env)->SetIntArrayRegion(env, size_, 0, 2, newSize);
                result = JNI_TRUE;
            }
            jniThrow(env, error, NULL);
        } else {
            jniThrow(env, eNullPointer, NULL);
        }
    } else {
        jniThrow(env, eInvalidParams, NULL);
    }

    return result;
}

JNIEXPORT jobject JNICALL
Java_com_jappsy_io_image_PNG_getPixels(JNIEnv *env, jclass type, jobject memoryFile, jint outWidth,
                                       jint outHeight, jint x1, jint y1, jint x2, jint y2) {
    if (memoryFile == NULL) {
        jniThrow(env, eNullPointer, NULL);
        return NULL;
    }

    if ((outWidth < 0) || (outHeight < 0)) {
        jniThrow(env, eInvalidParams, NULL);
        return NULL;
    }

    void* memory = (*env)->GetDirectBufferAddress(env, memoryFile);
    jlong memorySize = (*env)->GetDirectBufferCapacity(env, memoryFile);

    if (memory != NULL) {
        char* error;
        void* pixels = png_getPixels(memory, (uint32_t)memorySize, (uint32_t)outWidth, (uint32_t)outHeight, x1, y1, x2, y2, &error);
        if (pixels != NULL) {
            jobject directBuffer = (*env)->NewDirectByteBuffer(env, pixels, (uint32_t)(outWidth * outHeight * sizeof(uint32_t)));
            jobject memoryRef = (*env)->NewGlobalRef(env, directBuffer);

            return memoryRef;
        }
        jniThrow(env, error, NULL);
    } else {
        jniThrow(env, eNullPointer, NULL);
    }

    return NULL;
}

#endif

#ifdef __cplusplus
}
#endif
