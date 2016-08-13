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

#ifndef JAPPSY_UNAMEDSTRINGS_H
#define JAPPSY_UNAMEDSTRINGS_H

#include <platform.h>
#include <data/uObject.h>

class NamedStrings : public Object {
private:
	const wchar_t** m_keys;
	const wchar_t** m_values;
	uint32_t m_count;
	
	int32_t find(const wchar_t* key);
	
public:
	NamedStrings();
	~NamedStrings();
	
	const wchar_t* get(const wchar_t* key);
	void remove(const wchar_t* key);
	void insert(const wchar_t* key, const wchar_t* value) throw(const char*);
};

#endif //JAPPSY_UNAMEDSTRINGS_H