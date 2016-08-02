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

#include "uJPEG.h"
#include <core/uMemory.h>
#include <core/uString.h>
#include <io/uFileIO.h>

#include <jconfig.h>
#define JCONFIG_INCLUDED
#define HAVE_PROTOTYPES
#include <jpeglib.h>
#include <jinclude.h>
#include <jerror.h>
#include <setjmp.h>

/*
 * errorCode : eNullPointer | eInvalidFormat | eOK
 */
bool is_jpeg(const void* ptr, uint32_t insize, char** error) {
    if (ptr == NULL) {
        if (error != NULL)
            *error = (char*)eNullPointer;
        return false;
    } else if ((insize < 10)
               || ((*(uint32_t *) ptr != JPEGAPP0_HEAD)
                   && (*(uint32_t *) ptr != JPEGAPP1_HEAD))) {
        if (error != NULL)
            *error = (char*)eInvalidFormat;
        return false;
    }

    if (error != NULL)
        *error = (char*)eOK;

    return true;
}

typedef struct {
    struct jpeg_source_mgr pub;	    // public fields
    JOCTET eoi_buffer[2];			// a place to put a dummy EOI
} jappsy_jpeg_source_mgr;

typedef jappsy_jpeg_source_mgr * jappsy_jpeg_src_ptr;

METHODDEF(void) jpeg_dummyInitSource(j_decompress_ptr cinfo) { }

METHODDEF(boolean) jpeg_fillInputBuffer(j_decompress_ptr cinfo) {
    jappsy_jpeg_src_ptr src = (jappsy_jpeg_src_ptr) cinfo->src;
    WARNMS(cinfo, JWRN_JPEG_EOF);
    // Create a fake EOI marker
    src->eoi_buffer[0] = (JOCTET) 0xFF;
    src->eoi_buffer[1] = (JOCTET) JPEG_EOI;
    src->pub.next_input_byte = src->eoi_buffer;
    src->pub.bytes_in_buffer = 2;
    return TRUE;
}

METHODDEF(void) jpeg_skipInputData(j_decompress_ptr cinfo, long num_bytes) {
    jappsy_jpeg_src_ptr src = (jappsy_jpeg_src_ptr) cinfo->src;
    if (num_bytes > 0) {
        while (num_bytes > (long) src->pub.bytes_in_buffer) {
            num_bytes -= (long) src->pub.bytes_in_buffer;
            (void) jpeg_fillInputBuffer(cinfo);
        }
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}

METHODDEF(void) jpeg_dummyTermSource(j_decompress_ptr cinfo) { }

GLOBAL(void) jpeg_initMemorySource(j_decompress_ptr cinfo, const JOCTET * buffer, size_t bufsize) {
    jappsy_jpeg_src_ptr src;
    if (cinfo->src == 0) {	// first time for this JPEG object?
        cinfo->src = (struct jpeg_source_mgr *)
                (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                            SIZEOF(jappsy_jpeg_source_mgr));
    }

    src = (jappsy_jpeg_src_ptr) cinfo->src;
    src->pub.init_source = jpeg_dummyInitSource;
    src->pub.fill_input_buffer = jpeg_fillInputBuffer;
    src->pub.skip_input_data = jpeg_skipInputData;
    src->pub.resync_to_restart = jpeg_resync_to_restart; // use default method
    src->pub.term_source = jpeg_dummyTermSource;

    src->pub.next_input_byte = buffer;
    src->pub.bytes_in_buffer = bufsize;
}

struct jappsy_jpeg_error_mgr {
    struct jpeg_error_mgr pub;	// "public" fields
    jmp_buf setjmp_buffer;		// for return to caller
};

typedef struct jappsy_jpeg_error_mgr * jappsy_jpeg_error_ptr;

unsigned char MJPGDHTSeg[0x1A4] = {
        /* JPEG DHT Segment for YCrCb omitted from MJPG data */
        0xFF,0xC4,0x01,0xA2,
        0x00,0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x01,0x00,0x03,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,0x10,0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05,0x05,0x04,0x04,0x00,
        0x00,0x01,0x7D,0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,
        0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,0x24,
        0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,0x29,0x2A,0x34,
        0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,
        0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,
        0x79,0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,
        0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,
        0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,
        0xDA,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,
        0xF8,0xF9,0xFA,0x11,0x00,0x02,0x01,0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,0x04,0x00,0x01,
        0x02,0x77,0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,
        0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xA1,0xB1,0xC1,0x09,0x23,0x33,0x52,0xF0,0x15,0x62,
        0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,0xF1,0x17,0x18,0x19,0x1A,0x26,0x27,0x28,0x29,0x2A,
        0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,
        0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,
        0x79,0x7A,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,
        0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,
        0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,
        0xD9,0xDA,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,
        0xF9,0xFA
};

void jpeg_set_mjpeg_dht(j_decompress_ptr cinfo) {
    INT32 length;
    UINT8 bits[17];
    UINT8 huffval[256];
    int i, index, count;
    JHUFF_TBL **htblptr;

    uint8_t* next_input_byte = (uint8_t*)MJPGDHTSeg;
    //size_t bytes_in_buffer = 0x1A4 - 2;

    next_input_byte += 2;
    length = (*next_input_byte)<<8; next_input_byte++;
    length |= (*next_input_byte); next_input_byte++;
    length -= 2;

    while (length > 16) {
        index = *(next_input_byte); next_input_byte++;

        bits[0] = 0;
        count = 0;
        for (i = 1; i <= 16; i++) {
            bits[i] = *(next_input_byte); next_input_byte++;
            count += bits[i];
        }

        length -= 1 + 16;

        if (count > 256 || ((INT32) count) > length)
            ERREXIT(cinfo, JERR_BAD_HUFF_TABLE);

        for (i = 0; i < count; i++) {
            huffval[i] = *(next_input_byte); next_input_byte++;
        }

        length -= count;

        if (index & 0x10) {		/* AC table definition */
            index -= 0x10;
            htblptr = &cinfo->ac_huff_tbl_ptrs[index];
        } else {			/* DC table definition */
            htblptr = &cinfo->dc_huff_tbl_ptrs[index];
        }

        if (index < 0 || index >= NUM_HUFF_TBLS)
            ERREXIT1(cinfo, JERR_DHT_INDEX, index);

        if (*htblptr == 0)
            *htblptr = jpeg_alloc_huff_table((j_common_ptr) cinfo);

        memcpy((*htblptr)->bits, bits, SIZEOF((*htblptr)->bits));
        memcpy((*htblptr)->huffval, huffval, SIZEOF((*htblptr)->huffval));
    }

    if (length != 0)
        ERREXIT(cinfo, JERR_BAD_LENGTH);
}

METHODDEF(void) jpeg_errorExit(j_common_ptr cinfo) {
    jappsy_jpeg_error_ptr err = (jappsy_jpeg_error_ptr) cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    longjmp(err->setjmp_buffer, 1);
}

bool jpeg_getSize(const void* ptr, uint32_t insize, uint32_t* lpWidth, uint32_t* lpHeight, char** error) {
    if (!is_jpeg(ptr, insize, error))
        return false;

    struct jpeg_decompress_struct cinfo = {0};
    struct jappsy_jpeg_error_mgr jerr;

    uint32_t outWidth = 0;
    uint32_t outHeight = 0;

    // Init JPEG Error Handling
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = jpeg_errorExit;
    int motion_jpeg_dht = (*((uint32_t*)(((uint8_t*)ptr) + 6)) == MJPEG_CHUNK) ? 1 : 0;

    if (setjmp(jerr.setjmp_buffer)) {
        if (jerr.pub.msg_code == JERR_NO_HUFF_TABLE) {
            motion_jpeg_dht++;
        } else if (motion_jpeg_dht != 0) {
            motion_jpeg_dht++;
        } else if (jerr.pub.msg_code == JERR_OUT_OF_MEMORY) {
            if (error != NULL)
                *error = (char*)eOutOfMemory;

            jpeg_destroy_decompress(&cinfo);
            return false;
        }

        jpeg_destroy_decompress(&cinfo);
        if (motion_jpeg_dht != 1) {
            if (error != NULL)
                *error = (char*)eInvalidFormat;

            return false;
        }
    }

    // Decompress JPEG
    {
        jpeg_create_decompress(&cinfo);
        jpeg_initMemorySource(&cinfo, (const JOCTET *) ptr, insize);

        // Read JPEG Header
        (void) jpeg_read_header(&cinfo, TRUE);

        // Start JPEG Decompress to calculate dimensions
        if (motion_jpeg_dht == 1) jpeg_set_mjpeg_dht(&cinfo);
        (void) jpeg_start_decompress(&cinfo);

        outWidth = cinfo.output_width;
        outHeight = cinfo.output_height;

        jpeg_destroy_decompress(&cinfo);
    }

    if (lpWidth != NULL)
        *lpWidth = (uint32_t) outWidth;

    if (lpHeight != NULL)
        *lpHeight = (uint32_t) outHeight;

    if (error != NULL)
        *error = (char*)eOK;
    return true;
}

void* jpeg_getPixels(const void* ptr, uint32_t insize, uint32_t outWidth, uint32_t outHeight, int32_t x1, int32_t y1, int32_t x2, int32_t y2, char** error) {
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

    // Check JPEG Header
    if (!is_jpeg(ptr, insize, error))
        return NULL;

    struct jpeg_decompress_struct cinfo = {0};
    struct jappsy_jpeg_error_mgr jerr;

    uint8_t *outResult = NULL;
    uint32_t width = 0;
    uint32_t height = 0;

    // Init JPEG Error Handling
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = jpeg_errorExit;
    int motion_jpeg_dht = (*((uint32_t*)(((uint8_t*)ptr) + 6)) == MJPEG_CHUNK) ? 1 : 0;

    if (setjmp(jerr.setjmp_buffer)) {
        if (outResult != NULL) {
            memFree(outResult);
            outResult = NULL;
        }

        if (jerr.pub.msg_code == JERR_NO_HUFF_TABLE) {
            motion_jpeg_dht++;
        } else if (motion_jpeg_dht != 0) {
            motion_jpeg_dht++;
        } else if (jerr.pub.msg_code == JERR_OUT_OF_MEMORY) {
            if (error != NULL)
                *error = (char*)eOutOfMemory;

            jpeg_destroy_decompress(&cinfo);
            return NULL;
        }

        jpeg_destroy_decompress(&cinfo);
        if (motion_jpeg_dht != 1) {
            if (error != NULL)
                *error = (char*)eInvalidFormat;

            return NULL;
        }
    }

    // Decompress JPEG
    {
        if (error != NULL)
            *error = (char *) eInvalidFormat;

        jpeg_create_decompress(&cinfo);
        jpeg_initMemorySource(&cinfo, (const JOCTET *) ptr, insize);

        // Read JPEG Header
        (void) jpeg_read_header(&cinfo, TRUE);

        // Start JPEG Decompress to calculate dimensions
        if (motion_jpeg_dht == 1) jpeg_set_mjpeg_dht(&cinfo);
        (void) jpeg_start_decompress(&cinfo);

        width = cinfo.output_width;
        height = cinfo.output_height;

        // Обработка маркеров
        /* Для работы с OpenGL эти данные не нужны!!!
        jpeg_saved_marker_ptr mptr = cinfo.marker_list;
        while (mptr != 0) {
            if (mptr->marker == JPEG_COM) {
                int len = utf8_strlen_nzt(mptr->data, mptr->data_length, &outTextSize);
                outText = memAlloc(char, outText, outTextSize);
                memcpy(outText, mptr->data, outTextSize - 1);
                outText[outTextSize - 1] = 0;
                break;
            }
            mptr = mptr->next;
        }
         */

        JDIMENSION row_stride = (JDIMENSION)(cinfo.output_width * cinfo.output_components + sizeof(uint32_t));
        JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

        // Allocate Out Memory
        outResult = memAlloc(uint8_t, outResult,
                             (uint32_t) (outWidth * outHeight * sizeof(uint32_t)));
        if (outResult == NULL) {
            (void) jpeg_finish_decompress(&cinfo);
            jpeg_destroy_decompress(&cinfo);

            return false;
        }

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

        uint32_t copyWidth = (uint32_t) (x2 - x1);
        uint32_t copyHeight = (uint32_t) (y2 - y1);
        if ((copyWidth == 0)
            || (copyHeight == 0)) {
            (void) jpeg_finish_decompress(&cinfo);
            jpeg_destroy_decompress(&cinfo);

            if (error != NULL)
                *error = (char*)eInvalidParams;
            return outResult;
        }

        // Skip Lines
        while (cinfo.output_scanline < y1) {
            if (jpeg_read_scanlines(&cinfo, buffer, 1) != 1) {
                if (outResult != NULL) {
                    memFree(outResult);
                    outResult = NULL;
                }

                jpeg_destroy_decompress(&cinfo);

                if (error != NULL)
                    *error = (char*)eInvalidFormat;
                return NULL;
            }
        }

        uint32_t inLineSkip = (uint32_t) (x1 * cinfo.num_components);
        uint32_t outLineSkip = (uint32_t) ((outWidth - copyWidth) * sizeof(uint32_t));

        // Process Rows
        if (cinfo.num_components == 4) {
            // YMCK to RGBA
            while (cinfo.output_scanline < y2/*cinfo.output_height*/) {
                if (jpeg_read_scanlines(&cinfo, buffer, 1) != 1) {
                    if (outResult != NULL) {
                        memFree(outResult);
                        outResult = NULL;
                    }

                    jpeg_destroy_decompress(&cinfo);

                    if (error != NULL)
                        *error = (char*)eInvalidFormat;
                    return NULL;
                }
                register uint8_t *inBuffer = (uint8_t*)(buffer[0]) + inLineSkip;
                int x = copyWidth;
                for (; x > 0; x--) {
                    register uint32_t color = *((uint32_t *)inBuffer);
                    uint32_t k = (color >> 24);
                    *(uint32_t*)outBuffer = (color + k + (k << 8) + (k << 16)) ^ 0x00FFFFFF | 0xFF000000;
                    inBuffer += 4;
                    outBuffer += 4;
                }
                outBuffer += outLineSkip;
            }
        } else {
            // RGB to RGBA
            while (cinfo.output_scanline < y2/*cinfo.output_height*/) {
                if (jpeg_read_scanlines(&cinfo, buffer, 1) != 1) {
                    if (outResult != NULL) {
                        memFree(outResult);
                        outResult = NULL;
                    }

                    jpeg_destroy_decompress(&cinfo);

                    if (error != NULL)
                        *error = (char*)eInvalidFormat;
                    return NULL;
                }
                register uint8_t *inBuffer = (uint8_t*)(buffer[0]) + inLineSkip;
                int x = copyWidth;
                for (; x > 0; x--) {
                    *(uint32_t*)outBuffer = *((uint32_t *)inBuffer) | 0xFF000000;
                    inBuffer += 3;
                    outBuffer += 4;
                }
                outBuffer += outLineSkip;
            }
        }

        // Завершение
        (void) jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
    }

    if (error != NULL)
        *error = (char*)eOK;
    return outResult;
}

bool jpeg_create(const void* pixels, uint32_t inWidth, uint32_t inHeight, int quality, int smooth, void** outPtr, uint32_t* outSize, char** error) {
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

    unsigned char* outResult = NULL;
    unsigned long outResultSize = 0;

    struct jpeg_compress_struct cinfo = {0};
    struct jappsy_jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1] = {0};

    // Инициализация
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = jpeg_errorExit;
    if (setjmp(jerr.setjmp_buffer)) {
        if (outResult != NULL) {
            free(outResult);
            outResult = NULL;
        }

        if (row_pointer[0] != NULL) {
            memFree(row_pointer[0]);
            row_pointer[0] = NULL;
        }

        if (jerr.pub.msg_code == JERR_OUT_OF_MEMORY) {
            if (error != NULL)
                *error = (char*)eOutOfMemory;

            jpeg_destroy_compress(&cinfo);
            return NULL;
        }

        jpeg_destroy_compress(&cinfo);
        return false;
    }

    {
        if (error != NULL)
            *error = (char *) eInvalidFormat;

        jpeg_CreateCompress(&cinfo, JPEG_LIB_VERSION, sizeof(struct jpeg_compress_struct));

        jpeg_mem_dest(&cinfo, &outResult, &outResultSize);

        cinfo.image_width = inWidth;
        cinfo.image_height = inHeight;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;

        jpeg_set_defaults(&cinfo);

        cinfo.dct_method = JDCT_IFAST;
        cinfo.optimize_coding = TRUE;
        cinfo.smoothing_factor = smooth;

        jpeg_set_quality(&cinfo, quality, TRUE);

        jpeg_start_compress(&cinfo, TRUE);

        /* Для работы с OpenGL эти данные не нужны!!!
        if (bmText != 0) {
            char* text = bmText->toChar();
            if (text != 0) {
                libjpeg.jpeg_write_marker(&cinfo, JPEG_COM, (const JOCTET *)text, strlen(text));
                memFree(text);
            }
        }
         */

        row_pointer[0] = memAlloc(JSAMPLE, row_pointer[0], (uint32_t)(inWidth * 3 + sizeof(uint32_t)));
        register uint8_t* inBuffer = (uint8_t*)pixels;
        // Convert RGBA to RGB
        while (cinfo.next_scanline < cinfo.image_height) {
            register uint8_t* outBuffer = (uint8_t*)(row_pointer[0]);
            int x = inWidth;
            for (; x > 0; x--) {
                *(uint32_t*)outBuffer = *(uint32_t*)inBuffer;
                inBuffer += 4;
                outBuffer += 3;
            }
            (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);

        jpeg_destroy_compress(&cinfo);

        memFree(row_pointer[0]);
    }

    void* copyResult = memAlloc(void, copyResult, (uint32_t)outResultSize);
    if (copyResult == NULL) {
        free(outResult);

        if (error != NULL)
            *error = (char*)eOutOfMemory;

        return false;
    }

    memcpy(copyResult, outResult, outResultSize);
    free(outResult); // Удаление объекта созданного библиотекой JPEG

    *outPtr = copyResult;
    *outSize = (uint32_t)outResultSize;

    if (error != NULL)
        *error = (char*)eOK;

    return true;
}

#if defined(__JNI__)

JNIEXPORT jboolean JNICALL
Java_com_jappsy_io_image_JPEG_isJPEG(JNIEnv *env, jclass type, jobject memoryFile) {
    if (memoryFile == NULL) {
        jniThrow(env, eNullPointer, NULL);
        return JNI_FALSE;
    }

    void* memory = (*env)->GetDirectBufferAddress(env, memoryFile);
    jlong memorySize = (*env)->GetDirectBufferCapacity(env, memoryFile);

    char* error;
    if (memory != NULL) {
        if (is_jpeg(memory, (uint32_t)memorySize, &error))
            return JNI_TRUE;
    } else {
        error = (char*)eInvalidParams;
    }

    jniThrow(env, error, NULL);
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_jappsy_io_image_JPEG_getSize(JNIEnv *env, jclass type, jobject memoryFile,
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
            if (jpeg_getSize(memory, (uint32_t)memorySize, &width, &height, &error)) {
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

    return result;}

JNIEXPORT jobject JNICALL
Java_com_jappsy_io_image_JPEG_getPixels(JNIEnv *env, jclass type, jobject memoryFile, jint outWidth,
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
        void* pixels = jpeg_getPixels(memory, (uint32_t)memorySize, (uint32_t)outWidth, (uint32_t)outHeight, x1, y1, x2, y2, &error);
        if (pixels != NULL) {
            return (*env)->NewDirectByteBuffer(env, pixels, (uint32_t)(outWidth * outHeight * sizeof(uint32_t)));
        }
        jniThrow(env, error, NULL);
    } else {
        jniThrow(env, eNullPointer, NULL);
    }

    return NULL;
}

#endif