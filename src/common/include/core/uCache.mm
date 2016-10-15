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
#include <jappsy.h>

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

#elif defined(__JNI__)

#ifdef __cplusplus
extern "C" {
#endif

	Stream* jbyteArrayToStream(JNIEnv* env, jbyteArray data) throw(const char*) {
		jsize size = data == NULL ? 0 : env->GetArrayLength(data);
		uint8_t* streamptr = memAlloc(uint8_t, streamptr, size + sizeof(wchar_t));
		if (streamptr != NULL) {
			if (data != NULL) {
				env->GetByteArrayRegion(data, 0, size, (jbyte *)streamptr);
			}
			*((wchar_t*)(streamptr + size)) = L'\0';
			try {
				return new Stream(streamptr, (uint32_t)size, true);
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
	#include <fts.h>
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
			LOG("mkdir %s", str);
			mkdir(str, S_IRWXU | S_IRWXG | S_IXOTH);
			*p = '/';
		}
	}

	LOG("mkdir %s", str);
	mkdir(str, S_IRWXU | S_IRWXG | S_IXOTH);

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
#elif defined(__JNI__)
	CString createPath = *jappsyCacheDir;
	if (createPath.endsWith(L"/")) {
		if (uniqueName.startsWith(L"/")) {
			createPath.setLength(createPath.m_length - 1);
			createPath += uniqueName;
		} else {
			createPath += uniqueName;
		}
	} else {
		if (uniqueName.startsWith(L"/")) {
			createPath += uniqueName;
		} else {
			createPath += L"/";
			createPath += uniqueName;
		}
	}
	if (!createPath.endsWith(L"/")) {
		createPath += L"/";
	}
#else
	#error Unsupported platform!
#endif
	return mkdirs(createPath) ? CString(createPath) : L"";
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
	wchar_t* wstr = (wchar_t*)directory;

	uint32_t size = wcs_toutf8_size(wstr);
	if (size == 0)
		return;

	char* str = (char*)mmalloc(size);
	if (str == NULL)
		throw eOutOfMemory;

	wcs_toutf8(wstr, str, size);

	size_t len = size - 1;
	if (len == 0) {
		mmfree(str);
		return;
	}

	if (str[len - 1] == '/')
		str[len - 1] = '\0';

	LOG("rmdir %s", str);

	FTS *ftsp = NULL;
	FTSENT *curr;

	char *files[] = { (char *) str, NULL };

	// FTS_NOCHDIR  - Avoid changing cwd, which could cause unexpected behavior
	//                in multithreaded programs
	// FTS_PHYSICAL - Don't follow symlinks. Prevents deletion of files outside
	//                of the specified directory
	// FTS_XDEV     - Don't cross filesystem boundaries
	ftsp = fts_open(files, FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV, NULL);
	if (!ftsp) {
		LOG("%s: fts_open failed: %s", str, strerror(errno));
		goto deleteRecursive_finish;
	}

	while ((curr = fts_read(ftsp))) {
		switch (curr->fts_info) {
			case FTS_NS:
			case FTS_DNR:
			case FTS_ERR:
				LOG("%s: fts_read error: %s", curr->fts_accpath, strerror(curr->fts_errno));
				break;

			case FTS_DC:
			case FTS_DOT:
			case FTS_NSOK:
				// Not reached unless FTS_LOGICAL, FTS_SEEDOT, or FTS_NOSTAT were
				// passed to fts_open()
				break;

			case FTS_D:
				// Do nothing. Need depth-first search, so directories are deleted
				// in FTS_DP
				break;

			case FTS_DP:
			case FTS_F:
			case FTS_SL:
			case FTS_SLNONE:
			case FTS_DEFAULT:
				if (remove(curr->fts_accpath) < 0) {
					LOG("%s: failed to remove: %s",	curr->fts_path, strerror(errno));
				} else {
					LOG("%s: remove", curr->fts_accpath);
				}
				break;

			default:;
		}
	}

deleteRecursive_finish:
	if (ftsp) {
		fts_close(ftsp);
	}

	mmfree(str);
#else
	#error Unsupported platform!
#endif
}

Cache::Cache(const CString& cacheDir) {
	LOG("Cache > Create %ls", (wchar_t*)cacheDir);
	/*uint64_t maxMemory = getMaxMemory() / 10 - getUsedMemory();*/
	uint64_t cacheSize = 128 * 1024 * 1024; // maxMemory?
	LOG("Cache > Size %lld", cacheSize);
	
	m_dataCacheMaxSize = cacheSize / 2;
	m_dataCacheSize = 0;
	
	m_dataCacheDir = getDiskCacheDir(cacheDir);
}

Cache::~Cache() {
}

void Cache::addData(const CString& path, const CString& file, Stream* data) {
#if defined(__IOS__)
	AtomicLock(&m_DiskCacheLock);
	
	NSString* fileDir = [(NSString*)m_dataCacheDir stringByAppendingPathComponent:(NSString*)path];
	mkdirs(fileDir);
	NSString* filePath = [fileDir stringByAppendingPathComponent:(NSString*)file];
	
	NSData* fileData = [[NSData alloc] initWithBytes:data->getBuffer() length:data->getSize()];
	
	uint64_t modificationDate = data->getModificationDate();
	if (modificationDate == 0) {
		[fileData writeToFile:filePath atomically:YES];
	} else {
		NSDate* date = [[NSDate alloc] initWithTimeIntervalSince1970:(NSTimeInterval)((double)(modificationDate)/1000.0)];
		NSDictionary* attr = [NSDictionary dictionaryWithObjectsAndKeys: date, NSFileModificationDate, NULL];
		NSFileManager* manager = [NSFileManager defaultManager];
		[manager createFileAtPath:filePath contents:fileData attributes:attr];
	}
	
	data->setSourcePath(filePath);
	
	AtomicUnlock(&m_DiskCacheLock);
#elif defined(__JNI__)
	AtomicLock(&m_DiskCacheLock);

	#warning TODO

	CString fileDir = m_dataCacheDir; fileDir.concat(path);
	CString::format(L"Cache::addData(%ls) fileDir: %ls", (wchar_t*)path, (wchar_t*)fileDir).log();

	AtomicUnlock(&m_DiskCacheLock);
#else
	#error Unsupported platform!
#endif
}

Stream* Cache::getData(const CString& path, const CString& file) {
#if defined(__IOS__)
	AtomicLock(&m_DiskCacheLock);
	
	NSString* filePath = [[(NSString*)m_dataCacheDir stringByAppendingPathComponent:(NSString*)path] stringByAppendingPathComponent:(NSString*)file];
	NSFileManager* manager = [NSFileManager defaultManager];
	if ([manager fileExistsAtPath:filePath]) {
		//NSData* fileData = [[NSData alloc] initWithContentsOfFile:filePath];
		NSData* fileData = [manager contentsAtPath:filePath];

		NSError* error = nil;
		NSDictionary* attr = [manager attributesOfItemAtPath:filePath error:&error];
		NSDate* date = [attr fileModificationDate];
		
		AtomicUnlock(&m_DiskCacheLock);
		
		Stream* stream = NSDataToStream(fileData);
		if (stream != NULL) {
			stream->setModificationDate((uint64_t)floor([date timeIntervalSince1970] * 1000));
			stream->setSourcePath(filePath);
		}
		return stream;
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

CString Cache::getDataPath(const CString& path, const CString& file) {
#if defined(__IOS__)
	AtomicLock(&m_DiskCacheLock);
	
	NSString* filePath = [[(NSString*)m_dataCacheDir stringByAppendingPathComponent:(NSString*)path] stringByAppendingPathComponent:(NSString*)file];
	
	AtomicUnlock(&m_DiskCacheLock);
	
	return filePath;
#elif defined(__JNI__)
	#warning TODO
	
	return NULL;
#else
	#error Unsupported platform!
#endif
}

