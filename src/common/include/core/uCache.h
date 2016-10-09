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

#ifndef JAPPSY_UCACHE_H
#define JAPPSY_UCACHE_H

#include <platform.h>
#include <data/uObject.h>
#include <data/uString.h>
#include <core/uAtomic.h>
#include <data/uStream.h>

#ifndef __OBJC__
class NSData;
#endif

#ifdef __cplusplus
extern "C" {
#endif
	
	Stream* NSDataToStream(NSData* data) throw(const char*);
	
#ifdef __cplusplus
}
#endif

class Cache {
private:
	uint64_t getMaxMemory();
	uint64_t getUsedMemory();
	uint64_t getFreeMemory();

private:
	bool mkdirs(const CString& createPath);
	CString getDiskCacheDir(const CString& uniqueName);

	void deleteRecursive(const CString& directory);

private:
	uint64_t m_dataCacheMaxSize;
	uint64_t m_dataCacheSize;
	
	jlock m_DiskCacheLock = false;
	
	CString m_dataCacheDir;
	
public:
	Cache(const CString& cacheDir);
	~Cache();
	
	void addData(const CString& path, const CString& file, Stream* data);
	Stream* getData(const CString& path, const CString& file);
	CString getDataPath(const CString& path, const CString& file);
};

#endif //JAPPSY_UCACHE_H
