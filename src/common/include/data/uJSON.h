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

#ifndef JAPPSY_UJSON_H
#define JAPPSY_UJSON_H

#include <data/uObject.h>
#include <data/uString.h>

class RefJSON : public RefObject {
public:
	inline RefJSON() { TYPE = TypeJSON; }
};

class JSON : public Object {
private:
	inline static String key(const String& value) throw(const char*) { return String(L"\"").concat(value).concat(L"\""); }
	
public:
	RefClass(JSON, JSON);

	static String encode(const String& value) throw(const char*);
	
	inline static String keyify(const RefObject& object) throw(const char*) { return JSON::key(JSON::encode(object.toJSON())); }
	inline static String keyify(const Object& object) throw(const char*) { return JSON::key(JSON::encode(object.toJSON())); }
	inline static String keyify(const RefObject* object) throw(const char*) { return JSON::key(JSON::encode(object->toJSON())); }
	inline static String keyify(const Object* object) throw(const char*) { return JSON::key(JSON::encode(object->toJSON())); }
	inline static String keyify(const void* ptr) throw(const char*) { return JSON::key(JSON::encode(L"null")); }
	inline static String keyify(const wchar_t* string) throw(const char*) { return JSON::key(JSON::encode(string)); }
	inline static String keyify(const char* string) throw(const char*) { return JSON::key(JSON::encode(string)); }
	inline static String keyify(const char character) throw(const char*) { return JSON::key(JSON::encode(character)); }
	inline static String keyify(const wchar_t character) throw(const char*) { return JSON::key(JSON::encode(character)); }
	inline static String keyify(const bool value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static String keyify(const int8_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static String keyify(const uint8_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static String keyify(const int16_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static String keyify(const uint16_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static String keyify(const int32_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static String keyify(const uint32_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static String keyify(const int64_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static String keyify(const uint64_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static String keyify(const float value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static String keyify(const double value) throw(const char*) { return JSON::key(JSON::encode(value)); }

	inline static String stringify(const RefObject& object) throw(const char*) { return object.toJSON(); }
	inline static String stringify(const Object& object) throw(const char*) { return object.toJSON(); }
	inline static String stringify(const RefObject* object) throw(const char*) { return object->toJSON(); }
	inline static String stringify(const Object* object) throw(const char*) { return object->toJSON(); }
	inline static String stringify(const void* ptr) throw(const char*) { return L"null"; }
	inline static String stringify(const wchar_t* string) throw(const char*) { return JSON::key(JSON::encode(string)); }
	inline static String stringify(const char* string) throw(const char*) { return JSON::key(JSON::encode(string)); }
	inline static String stringify(const char character) throw(const char*) { return JSON::key(JSON::encode(character)); }
	inline static String stringify(const wchar_t character) throw(const char*) { return JSON::key(JSON::encode(character)); }
	inline static String stringify(const bool value) throw(const char*) { return value; }
	inline static String stringify(const int8_t value) throw(const char*) { return value; }
	inline static String stringify(const uint8_t value) throw(const char*) { return value; }
	inline static String stringify(const int16_t value) throw(const char*) { return value; }
	inline static String stringify(const uint16_t value) throw(const char*) { return value; }
	inline static String stringify(const int32_t value) throw(const char*) { return value; }
	inline static String stringify(const uint32_t value) throw(const char*) { return value; }
	inline static String stringify(const int64_t value) throw(const char*) { return value; }
	inline static String stringify(const uint64_t value) throw(const char*) { return value; }
	inline static String stringify(const float value) throw(const char*) { return value; }
	inline static String stringify(const double value) throw(const char*) { return value; }
};

#endif //JAPPSY_UJSON_H