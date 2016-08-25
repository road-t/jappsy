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

#ifndef JAPPSY_UURI_H
#define JAPPSY_UURI_H

#include <platform.h>
#include <data/uObject.h>
#include <data/uString.h>

class URI : public CObject {
private:
	wchar_t* m_scheme;
	wchar_t* m_user;
	wchar_t* m_password;
	wchar_t* m_host;
	wchar_t* m_port;
	wchar_t* m_path;
	wchar_t* m_file;
	wchar_t* m_ext;
	wchar_t* m_query;
	wchar_t* m_anchor;
	
	CString* _scheme;
	CString* _server;
	CString* _fullPath;
	CString* _absolutePath;
	CString* _uri;
	
	const CString& scheme();
	const CString& server();
public:
	static const wchar_t* basePath;
	
	URI(const wchar_t* uri);
	~URI();
	
	const CString& fullPath();
	const CString& absolutePath(const wchar_t* basePath);
	inline const CString& absolutePath() { return absolutePath(NULL); }
	const CString& uri();
	inline const CString ext() { return m_ext; }
	inline const CString path() { return m_path; }
	inline const CString file() { return m_file; }
};

#endif //JAPPSY_UURI_H