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

#include "uCache.h"

#if defined(__IOS__)
#import <mach/mach.h>

#ifdef __cplusplus
extern "C" {
#endif

	Stream* NSDataToStream(NSData* data) throw(const char*) {
	NSUInteger size = [data length];
	uint8_t* streamptr = memAlloc(uint8_t, streamptr, size + sizeof(wchar_t));
	if (streamptr != NULL) {
		memcpy(streamptr, [data bytes], size);
		*((wchar_t*)(streamptr + size)) = L'\0';
		try {
			return new Stream(streamptr, size, true);
		} catch (...) {
			memFree(streamptr);
			throw;
		}
	} else
		throw eOutOfMemory;
}

#ifdef __cplusplus
}
#endif

#endif

uint64_t Cache::getMaxMemory() {
#if defined(__IOS__)
	return [NSProcessInfo processInfo].physicalMemory;
#elif defined(__JNI__)
	struct mallinfo info = mallinfo();
	return (uint64_t)info.usmblks;
#else
	#error Unsupported platform!
#endif
}

uint64_t Cache::getUsedMemory() {
#if defined(__IOS__)
	struct mach_task_basic_info info;
	mach_msg_type_number_t size = MACH_TASK_BASIC_INFO_COUNT;
	kern_return_t kerr = task_info(mach_task_self(),
								   MACH_TASK_BASIC_INFO,
								   (task_info_t)&info,
								   &size);
	if( kerr == KERN_SUCCESS ) {
		return info.resident_size;
	} else {
		return 0;
	}
#elif defined(__JNI__)
	struct mallinfo info = mallinfo();
	return (uint64_t)info.uordblks;
#else
	#error Unsupported platform!
#endif
}

uint64_t Cache::getFreeMemory() {
#if defined(__IOS__)
	return getMaxMemory() - getUsedMemory();
#elif defined(__JNI__)
	struct mallinfo info = mallinfo();
	return (uint64_t)info.fordblks;
#else
	#error Unsupported platform!
#endif
}

#if defined(__JNI__)
	#include <sys/stat.h>
#endif

bool Cache::mkdirs(const CString& createPath) {
#if defined(__IOS__)
	NSError * error = nil;
	[[NSFileManager defaultManager] createDirectoryAtPath:(NSString*)createPath
							  withIntermediateDirectories:YES
											   attributes:nil
													error:&error];
	if (error != nil) {
		return false;
	}
	
	return true;
#elif defined(__JNI__)
	wchar_t* wstr = (wchar_t*)createPath;

	uint32_t size = wcs_toutf8_size(wstr);
	if (size == 0)
		return true;

	char* str = (char*)mmalloc(size);
	if (str == NULL)
		throw eOutOfMemory;

	wcs_toutf8(wstr, str, size);

	size_t len = size - 1;
	if (len == 0) {
		mmfree(str);
		return true;
	}

	if (str[len - 1] == '/')
		str[len - 1] = '\0';

	char* p = NULL;
	for (p = str + 1; *p; p++) {
		if (*p == '/') {
			*p = 0;
			mkdir(str, S_IRWXU);
			*p = '/';
		}
	}

	mkdir(str, S_IRWXU);

	mmfree(str);
	return true;
#else
	#error Unsupported platform!
#endif
}

CString Cache::getDiskCacheDir(const CString& uniqueName) {
#if defined(__IOS__)
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
	NSString *cacheDirectory = [paths objectAtIndex:0];
	NSString *createPath = [cacheDirectory stringByAppendingPathComponent:uniqueName];
	
	return mkdirs(createPath) ? CString(createPath) : L"";
#elif defined(__JNI__)
	#warning TODO
#else
	#error Unsupported platform!
#endif
}

void Cache::deleteRecursive(const CString& directory) {
#if defined(__IOS__)
	NSFileManager *fm = [NSFileManager defaultManager];
	NSError *error = nil;
	NSString *path = (NSString*)directory;
	for (NSString *file in [fm contentsOfDirectoryAtPath:directory error:&error]) {
		BOOL success = [fm removeItemAtPath:[NSString stringWithFormat:@"%@%@", path, file] error:&error];
		if (!success || error) {
			// it failed.
		}
	}
#elif defined(__JNI__)
	#warning TODO
#else
	#error Unsupported platform!
#endif
}

Cache::Cache(const CString& cacheDir) {
	uint64_t maxMemory = getMaxMemory() / 10 - getUsedMemory();
	uint64_t cacheSize = 128 * 1024 * 1024; // maxMemory?
	
	m_dataCacheMaxSize = cacheSize / 2;
	m_dataCacheSize = 0;
	
	m_dataCacheDir = getDiskCacheDir(cacheDir);
}

Cache::~Cache() {
}

void Cache::addData(const CString& path, const CString& file, Stream* data) {
#if defined(__IOS__)
	AtomicLock(&m_DiskCacheLock);
	
	NSString *fileDir = [(NSString*)m_dataCacheDir stringByAppendingPathComponent:(NSString*)path];
	mkdirs(fileDir);
	NSString *filePath = [fileDir stringByAppendingPathComponent:(NSString*)file];
	
	NSData* fileData = [[NSData alloc] initWithBytes:data->getBuffer() length:data->getSize()];
	[fileData writeToFile:filePath atomically:YES];
	
	AtomicUnlock(&m_DiskCacheLock);
#elif defined(__JNI__)
	#warning TODO
#else
	#error Unsupported platform!
#endif
}

Stream* Cache::getData(const CString& path, const CString& file) {
#if defined(__IOS__)
	AtomicLock(&m_DiskCacheLock);
	
	NSString *filePath = [[(NSString*)m_dataCacheDir stringByAppendingPathComponent:(NSString*)path] stringByAppendingPathComponent:(NSString*)file];
	if ([[NSFileManager defaultManager] fileExistsAtPath:filePath]) {
		NSData* fileData = [[NSData alloc] initWithContentsOfFile:filePath];
		
		AtomicUnlock(&m_DiskCacheLock);
		
		return NSDataToStream(fileData);
	}

	AtomicUnlock(&m_DiskCacheLock);
	
	return NULL;
#elif defined(__JNI__)
	#warning TODO

	return NULL;
#else
	#error Unsupported platform!
#endif
}
