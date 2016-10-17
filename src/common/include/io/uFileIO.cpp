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

#include "uFileIO.h"
#include <core/uMemory.h>
#include <core/uSystem.h>
#include <fcntl.h>
#include <sys/stat.h>

#if defined(__JNI__)

static const char _clsFileDescriptor[] = "java/io/FileDescriptor";

const char* clsFileDescriptor = _clsFileDescriptor;

JNIEXPORT jobject JNICALL
Java_com_jappsy_io_FileIO_readFile(JNIEnv *env, jclass type, jobject fd) {
    if (fd == NULL) {
        jniThrow(env, eNullPointer, NULL);
        return JNI_FALSE;
    }

    jclass jc = env->FindClass(clsFileDescriptor);
    if (jc == NULL) {
        jniThrow(env, eUnknown, NULL);
        return JNI_FALSE;
    }

    jfieldID jf = env->GetFieldID(jc, "descriptor", "I");
    if (jf == NULL) {
        jniThrow(env, eUnknown, NULL);
        return JNI_FALSE;
    }

    int fileDescriptor = env->GetIntField(fd, jf);

    const char* error;
    uint32_t size;
    void* buffer = fio_readFile(fileDescriptor, &size, &error);
    if (buffer != NULL) {
        return env->NewDirectByteBuffer(buffer, size);
    }
    jniThrow(env, error, NULL);

    return NULL;
}

#endif

int fio_create(const char* path, const char** error) {
    int fd = open(path, O_CREAT | O_RDWR | O_NOCTTY | O_APPEND | O_NONBLOCK | O_NOFOLLOW | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd < 0) {
        if (error != NULL) {
            int err = errno;
            if (err == EBADF) {
                *error = eIOFileNotOpen;
            } else {
                *error = eIOInvalidFile;
            }
        }
        return -1;
    }

    return fd;
}

int fio_createNew(const char* path, const char** error) {
    int fd = open(path, O_CREAT | O_WRONLY | O_NOCTTY | O_TRUNC | O_NONBLOCK | O_NOFOLLOW | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd < 0) {
        if (error != NULL) {
            int err = errno;
            if (err == EBADF) {
                *error = eIOFileNotOpen;
            } else {
                *error = eIOInvalidFile;
            }
        }
        return -1;
    }

    return fd;
}

int fio_open(const char* path, const char** error) {
    int fd = open(path, O_RDONLY | O_NOCTTY | O_NOFOLLOW | O_LARGEFILE);
    if (fd < 0) {
        if (error != NULL) {
            int err = errno;
            if (err == EBADF) {
                *error = eIOFileNotOpen;
            } else {
                *error = eIOInvalidFile;
            }
        }
        return -1;
    }

    return fd;
}

bool fio_close(const int fd, const char** error) {
    int res = close(fd);
    if (res < 0) {
        if (error != NULL) {
            int err = errno;
            if (err == EBADF) {
                *error = eIOFileNotOpen;
            } else if (err == EIO) {
                *error = eIOInvalidFile;
            } else {
                return true;
            }
        }

        return false;
    }

    return true;
}

bool fio_begin(const int fd, off_t* restore, off_t* size, const char** error) {
    if (fd < 0) {
        if (error != NULL)
            *error = eIOFileNotOpen;
        return false;
    }

    #if defined(JAPPSY_IO_RESTORE_FILE_POINTER)
        if (restore != NULL) {
            *restore = lseek(fd, 0, SEEK_CUR);

            if (*restore == (off_t) -1) {
                if (error != NULL) {
                    int err = errno;
                    if (err == EBADF)
                        *error = eIOFileNotOpen;
                    else
                        *error = eIOInvalidFile;
                }
                return false;
            }
        }
    #endif

    if (size != NULL) {
        *size = lseek(fd, 0, SEEK_END);
        if (*size == (off_t) -1) {
            if (error != NULL) {
                int err = errno;
                if (err == EBADF)
                    *error = eIOFileNotOpen;
                else
                    *error = eIOInvalidFile;
            }
            return false;
        }
    }

    off_t begin = lseek(fd, 0, SEEK_SET);
    if (begin == (off_t)-1) {
        if (error != NULL) {
            int err = errno;
            if (err == EBADF)
                *error = eIOFileNotOpen;
            else
                *error = eIOInvalidFile;
        }
        return false;
    }

    return true;
}

bool fio_end(const int fd, off_t* restore, const char** error) {
    #if defined(JAPPSY_IO_RESTORE_FILE_POINTER)
        if (restore != NULL) {
            off_t result = lseek(fd, *restore, SEEK_SET);

            if (result == (off_t) -1) {
                if (error != NULL) {
                    int err = errno;
                    if (err == EBADF)
                        *error = eIOFileNotOpen;
                    else
                        *error = eIOInvalidFile;
                }
                return false;
            }
        }
    #endif

    return true;
}

bool fio_readFully(const int fd, uint8_t* buffer, size_t len, const char** error) {
    int ofs = 0;
    while (len > 0) {
        ssize_t n = read(fd, (void *) (buffer + ofs), len);
        if (n < 0) { // Read Error
            int err = errno;
            if (err == EBADF)
                *error = eIOFileNotOpen;
            else
                *error = eIOInvalidFile;
            return false;
        } else if (n == 0) { // EOF
            if (error != NULL)
                *error = eIOReadLimit;
            return false;
        }
        len -= n;
        ofs += n;
    }

    return true;
}

bool fio_writeFully(const int fd, uint8_t* buffer, size_t len, const char** error) {
    int ofs = 0;
    while (len > 0) {
        ssize_t n = write(fd, (void *) (buffer + ofs), len);
        if (n < 0) { // Write Error
            int err = errno;
            if (err == EBADF)
                *error = eIOFileNotOpen;
            else
                *error = eIOInvalidFile;
            return false;
        } else if (n == 0) { // Nothing Written?
            systemSleep(1);
            // TODO: File Write Again? (Infinite Loop?)
        }
        len -= n;
        ofs += n;
    }

    return true;
}

bool fio_seek(const int fd, uint32_t ofs, const char** error) {
    off_t result = lseek(fd, (off_t)ofs, SEEK_SET);

    if (result == (off_t) -1) {
        if (error != NULL) {
            int err = errno;
            if (err == EBADF)
                *error = eIOFileNotOpen;
            else
                *error = eIOInvalidFile;
        }
        return false;
    }

    return true;
}

#if defined(__WINNT__)
	int fsync (int fd) {
		HANDLE h = (HANDLE) _get_osfhandle (fd);
		DWORD err;

		if (h == INVALID_HANDLE_VALUE) {
			errno = EBADF;
			return -1;
		}

		if (!FlushFileBuffers (h)) {
			/* Translate some Windows errors into rough approximations of Unix
			* errors.  MSDN is useless as usual - in this case it doesn't
			* document the full range of errors.
			*/
			err = GetLastError ();
			switch (err) {
				case ERROR_ACCESS_DENIED:
					/* For a read-only handle, fsync should succeed, even though we have
					no way to sync the access-time changes.  */
					return 0;

				/* eg. Trying to fsync a tty. */
				case ERROR_INVALID_HANDLE:
					errno = EINVAL;
					break;

				default:
					errno = EIO;
			}
			return -1;
		}

		return 0;
	}
#endif

bool fio_flush(const int fd, const char** error) {
    int res = fsync(fd);

    if (res < 0) { // Flush Error
        int err = errno;
        if (err == EBADF)
            *error = eIOFileNotOpen;
        else if (err == EIO)
            *error = eIOInvalidFile;
        else // Skip EROFS, EINVAL
            return true;

        return false;
    }

    return true;
}

bool fio_truncate(const int fd, uint32_t size, const char** error) {
    int res = ftruncate(fd, size);

    if (res < 0) { // Truncate Error
        int err = errno;
        if (err == EBADF)
            *error = eIOFileNotOpen;
        else if (err != EINVAL)
            *error = eIOInvalidFile;
        else // Skip EINVAL
            return true;

        return false;
    }

    return true;
}

void* fio_readFile(const int fd, uint32_t* size, const char** error) {
    off_t fileRestore, fileSize;
    if (!fio_begin(fd, &fileRestore, &fileSize, error))
        return NULL;

    if (fileSize > 0x7FFFFFFF) {
        if (!fio_end(fd, &fileRestore, error))
            return NULL;

        if (error != NULL)
            *error = eIOReadLimit;

        return NULL;
    }

    void* result = memAlloc(void, result, (uint32_t)fileSize);
    if (result == NULL) {
        if (!fio_end(fd, &fileRestore, error))
            return NULL;

        if (error != NULL)
            *error = eOutOfMemory;

        return NULL;
    }

    if (!fio_readFully(fd, (uint8_t*)result, (uint32_t)fileSize, error)) {
        memFree(result);

        if (!fio_end(fd, &fileRestore, error))
            return NULL;

        return NULL;
    }

    if (!fio_end(fd, &fileRestore, error)) {
        memFree(result);

        return NULL;
    }

    if (size != NULL)
        *size = (uint32_t)fileSize;

    return result;
}

bool fio_setModification(const int fd, uint64_t time, const char** error) {
    struct timespec times[2];
    times[0].tv_sec = 0;
    times[0].tv_nsec = UTIME_OMIT;

    if (time == 0) {
        times[1].tv_sec = 0;
        times[1].tv_nsec = UTIME_NOW;
    } else {
        times[1].tv_sec = time / 1000ULL;
        times[1].tv_nsec = (time % 1000ULL) * 1000000ULL;
    }

    int res = futimens(fd, times);
    if (res < 0) { // Set Time Error
        int err = errno;
        if (err == EBADF)
            *error = eIOFileNotOpen;
        else if (err != EINVAL)
            *error = eIOInvalidFile;
        else // Skip EINVAL
            return true;

        return false;
    }

    return true;
}

bool fio_getModification(const int fd, uint64_t* time, const char** error) {
    struct stat buf;

    int res = fstat(fd, &buf);
    if (res < 0) { // Get Stat Error
        int err = errno;
        if (err == EBADF)
            *error = eIOFileNotOpen;
        else
            *error = eIOInvalidFile;

        return false;
    }

    if (time != NULL) {
        *time = (uint64_t)(buf.st_mtime) * 1000ULL + ((uint64_t)(buf.st_mtime_nsec) / 1000000ULL);
    }

    return true;
}
