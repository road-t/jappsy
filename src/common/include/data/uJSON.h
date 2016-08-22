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

#include <platform.h>

#include <data/uString.h>

#ifdef __cplusplus
extern "C" {
#endif
	
	enum JsonType {
		JSON_TYPE_NULL = 0,
		JSON_TYPE_OBJECT = 1,
		JSON_TYPE_ARRAY = 2,
		JSON_TYPE_STRING = 3,
		JSON_TYPE_NUMBER = 4,
		JSON_TYPE_BOOLEAN = 5
	};
	
	enum JsonErrorType {
		JSON_ERROR_NONE = 0,
		JSON_ERROR_OUTOFMEMORY = 1,
		JSON_ERROR_UNEXPECTEDEND = 2,
		JSON_ERROR_SYNTAX = 3,
	};

	struct JsonNumber {
		bool is_float;
		union {
			int64_t	 i;
			double	 f;
		} v;
	};
	
	struct JsonContext;
	struct JsonNode;
	
	struct JsonObjectCallbacks {
		void (*onobjectstart)(struct JsonContext* ctx, const char* key, void* target);
		void (*onobjectend)(struct JsonContext* ctx, const char* key, void* parenttarget, void* target, bool noerror);
		void (*onarraystart)(struct JsonContext* ctx, const char* key, void* target);
		void (*onarrayend)(struct JsonContext* ctx, const char* key, void* parenttarget, void* target, bool noerror);
		void (*onstring)(struct JsonContext* ctx, const char* key, char* value, void* target);
		void (*onnumber)(struct JsonContext* ctx, const char* key, const struct JsonNumber& number, void* target);
		void (*onboolean)(struct JsonContext* ctx, const char* key, bool value, void* target);
		void (*onnull)(struct JsonContext* ctx, const char* key, void* target);
	};

	struct JsonArrayCallbacks {
		void (*onobjectstart)(struct JsonContext* ctx, const int index, void* target);
		void (*onobjectend)(struct JsonContext* ctx, const int index, void* parenttarget, void* target, bool noerror);
		void (*onarraystart)(struct JsonContext* ctx, const int index, void* target);
		void (*onarrayend)(struct JsonContext* ctx, const int index, void* parenttarget, void* target, bool noerror);
		void (*onstring)(struct JsonContext* ctx, const int index, const char* value, void* target);
		void (*onnumber)(struct JsonContext* ctx, const int index, const struct JsonNumber& number, void* target);
		void (*onboolean)(struct JsonContext* ctx, const int index, const bool value, void* target);
		void (*onnull)(struct JsonContext* ctx, const int index, void* target);
	};
	
	struct JsonCallbacks {
		void* target;
		
		void (*onrootstart)(struct JsonContext* ctx, void* target);
		void (*onrootend)(struct JsonContext* ctx, void* target, bool noerror);
		
		struct JsonObjectCallbacks onobject;
		struct JsonArrayCallbacks onarray;
	};
	
	inline void JsonCopyCallbacks(struct JsonCallbacks* dst, const struct JsonCallbacks* src) { memcpy(dst, src, sizeof(struct JsonCallbacks)); }
	inline void JsonClearCallbacks(struct JsonCallbacks* dst, void* newtarget) { memset(dst, 0, sizeof(struct JsonCallbacks)); dst->target = newtarget; }
	
	struct JsonContext {
		union {
			char buffer[64];
			wchar_t wbuffer[64];
		};
		
		struct {
			JsonErrorType type;
			union {
				const char* ptr;
				const wchar_t* wptr;
			};
		
			union {
				const char* expected;
				const wchar_t* wexpected;
			};
		} error;
		
		inline void seterror(JsonErrorType type, const char* ptr, const char* expected = NULL) {
			error.type = type;
			error.ptr = ptr;
			error.expected = expected;
		}

		inline void wseterror(JsonErrorType type, const wchar_t* ptr, const wchar_t* expected = NULL) {
			error.type = type;
			error.wptr = ptr;
			error.wexpected = expected;
		}
		
		struct JsonCallbacks* callbacks;
	};
	
	struct JsonNode {
		struct JsonNode*	parent;
		JsonType	type;
		bool		wide;
		int32_t		level;
		union {
			char*		data;
			wchar_t*	wdata;
		};
		uint32_t	size;
		
		union {
			struct {
				struct JsonNode** k;
				struct JsonNode** v;
				uint32_t 	 c;
			} o;
			
			struct {
				struct JsonNode** v;
				uint32_t	 c;
			} a;
			
			CString*	cs;
			
			struct JsonNumber n;
			
			bool		b;
		} value;
		
		struct JsonNode*	it;
		
		int32_t count();
		JsonNode** keys() throw(const char*);
		JsonNode** items();
		
		JsonNode* get(int index) throw(const char*);
		JsonNode* getKey(int index) throw(const char*);
		JsonNode* get(const CString& key) throw(const char*);

		inline bool isNull() { return type == JSON_TYPE_NULL; }
		inline bool isObject() { return type == JSON_TYPE_OBJECT; }
		inline bool isArray() { return type == JSON_TYPE_ARRAY; }
		
		CString toString() throw(const char*);
		double toDouble() throw(const char*);
		int64_t toInt() throw(const char*);
		bool toBoolean() throw(const char*);
	};

	bool JsonCheck(struct JsonContext* ctx, const char* json);
	bool JsonCheckW(struct JsonContext* ctx, const wchar_t* json);
	
	struct JsonNode* JsonCreate(struct JsonNode* parent, JsonType type, uint32_t level, const char* data, uint32_t size);
	struct JsonNode* JsonCreateW(struct JsonNode* parent, JsonType type, uint32_t level, const wchar_t* data, uint32_t size);
	void JsonDestroy(struct JsonNode* j);
	#define JsonDestroyW JsonDestroy
	bool JsonObjectAdd(struct JsonNode* node, struct JsonNode* vkey, struct JsonNode* value);
	#define JsonObjectAddW JsonObjectAdd
	bool JsonArrayAdd(struct JsonNode* node, struct JsonNode* value);
	#define JsonArrayAddW JsonArrayAdd
	
	struct JsonNode* JsonParse(struct JsonContext* ctx, const char* json);
	struct JsonNode* JsonParseW(struct JsonContext* ctx, const wchar_t* json);

	bool JsonCall(struct JsonContext* ctx, const char* json);

	struct JsonNode* JsonArrayGet(const struct JsonNode* node, uint32_t index);
	struct JsonNode* JsonObjectGet(const struct JsonNode* node, const CString& key);
	struct JsonNode* JsonObjectGetIC(const struct JsonNode* node, const CString& key);
	
	const CString* JsonParentKey(const struct JsonNode* value, const CString* def = 0);
	
	void JsonDebugError(const struct JsonContext& ctx, const char* json);
	void JsonDebugErrorW(const struct JsonContext& ctx, const wchar_t* json);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <data/uObject.h>
#include <data/uString.h>

class JSONObject : public CObject {
public:
	JsonNode* root = NULL;
	
	JSONObject(const CString& json);
	JSONObject(const wchar_t* json);
	JSONObject(const char* json);
	
	~JSONObject();
};

class JSON {
private:
	inline static CString key(const CString& value) throw(const char*) {
		if ((value.m_length > 0) && (value[0] == L'\"'))
			return value;

		return CString(L"\"").concat(value).concat(L"\"");
	}
	
	
public:
	static CString encode(const CString& value) throw(const char*);
	
//	inline static CString keyify(const CObject& object) throw(const char*) { return JSON::key(JSON::encode(object.toJSON())); }
//	inline static CString keyify(const CObject* object) throw(const char*) { return JSON::key(JSON::encode(object->toJSON())); }
	inline static CString keyify(const void* ptr) throw(const char*) { return JSON::key(JSON::encode(L"null")); }
	inline static CString keyify(const wchar_t* string) throw(const char*) { return JSON::key(JSON::encode(string)); }
	inline static CString keyify(const char* string) throw(const char*) { return JSON::key(JSON::encode(string)); }
	inline static CString keyify(const char character) throw(const char*) { return JSON::key(JSON::encode(character)); }
	inline static CString keyify(const wchar_t character) throw(const char*) { return JSON::key(JSON::encode(character)); }
	inline static CString keyify(const bool value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static CString keyify(const int8_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static CString keyify(const uint8_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static CString keyify(const int16_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static CString keyify(const uint16_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static CString keyify(const int32_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static CString keyify(const uint32_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static CString keyify(const int64_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static CString keyify(const uint64_t value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static CString keyify(const float value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	inline static CString keyify(const double value) throw(const char*) { return JSON::key(JSON::encode(value)); }
	
//	inline static CString stringify(const CObject& object) throw(const char*) { return object.toJSON(); }
//	inline static CString stringify(const CObject* object) throw(const char*) { return object->toJSON(); }
	inline static CString stringify(const void* ptr) throw(const char*) { return L"null"; }
	inline static CString stringify(const wchar_t* string) throw(const char*) { return JSON::key(JSON::encode(string)); }
	inline static CString stringify(const char* string) throw(const char*) { return JSON::key(JSON::encode(string)); }
	inline static CString stringify(const char character) throw(const char*) { return JSON::key(JSON::encode(character)); }
	inline static CString stringify(const wchar_t character) throw(const char*) { return JSON::key(JSON::encode(character)); }
	inline static CString stringify(const bool value) throw(const char*) { return value; }
	inline static CString stringify(const int8_t value) throw(const char*) { return value; }
	inline static CString stringify(const uint8_t value) throw(const char*) { return value; }
	inline static CString stringify(const int16_t value) throw(const char*) { return value; }
	inline static CString stringify(const uint16_t value) throw(const char*) { return value; }
	inline static CString stringify(const int32_t value) throw(const char*) { return value; }
	inline static CString stringify(const uint32_t value) throw(const char*) { return value; }
	inline static CString stringify(const int64_t value) throw(const char*) { return value; }
	inline static CString stringify(const uint64_t value) throw(const char*) { return value; }
	inline static CString stringify(const float value) throw(const char*) { return value; }
	inline static CString stringify(const double value) throw(const char*) { return value; }
};

#endif

#endif //JAPPSY_UJSON_H