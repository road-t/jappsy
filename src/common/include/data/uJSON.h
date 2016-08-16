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

#ifdef __cplusplus
extern "C" {
#endif
	
	enum json_type {
		json_type_null = 0,
		json_type_object = 1,
		json_type_array = 2,
		json_type_string = 3,
		json_type_number = 4,
		json_type_boolean = 5
	};
	
	enum json_error_type {
		json_error_none = 0,
		json_error_oom = 1,
		json_error_eof = 2,
		json_error_syntax = 3,
	};

	struct json_number {
		bool is_float;
		union {
			int64_t	 i;
			double	 f;
		} v;
	};
	
	struct json_context;
	struct json_node;
	
	struct json_object_callbacks {
		typedef void (*object_callback)(struct json_context* ctx, const char* key, void* target);
		typedef void (*array_callback)(struct json_context* ctx, const char* key, void* target);
		typedef void (*string_callback)(struct json_context* ctx, const char* key, char* value, void* target);
		typedef void (*number_callback)(struct json_context* ctx, const char* key, struct json_number& number, void* target);
		typedef void (*boolean_callback)(struct json_context* ctx, const char* key, bool value, void* target);
		typedef void (*null_callback)(struct json_context* ctx, const char* key, void* target);
		
		object_callback onobject;
		array_callback onarray;
		string_callback onstring;
		number_callback onnumber;
		boolean_callback onboolean;
		null_callback onnull;
	};

	struct json_array_callbacks {
		typedef void (*object_callback)(struct json_context* ctx, const int index, void* target);
		typedef void (*array_callback)(struct json_context* ctx, const int index, void* target);
		typedef void (*string_callback)(struct json_context* ctx, const int index, const char* value, void* target);
		typedef void (*number_callback)(struct json_context* ctx, const int index, const struct json_number& number, void* target);
		typedef void (*boolean_callback)(struct json_context* ctx, const int index, const bool value, void* target);
		typedef void (*null_callback)(struct json_context* ctx, const int index, void* target);
		
		object_callback onobject;
		array_callback onarray;
		string_callback onstring;
		number_callback onnumber;
		boolean_callback onboolean;
		null_callback onnull;
	};
	
	struct json_callbacks {
		typedef void (*root_callback)(struct json_context* ctx, void* target);
		
		void* target;
		
		root_callback onroot;
		struct json_object_callbacks onobject;
		struct json_array_callbacks onarray;
	};
	
	inline void json_store_callbacks(struct json_callbacks* dst, const struct json_callbacks* src) { memcpy(dst, src, sizeof(struct json_callbacks)); }
	inline void json_clear_callbacks(struct json_callbacks* dst, void* newtarget) { memset(dst, 0, sizeof(struct json_callbacks)); dst->target = newtarget; }
	
	struct json_context {
		union {
			char buffer[64];
			wchar_t wbuffer[64];
		};
		
		struct {
			json_error_type type;
			union {
				const char* ptr;
				const wchar_t* wptr;
			};
		
			union {
				const char* expected;
				const wchar_t* wexpected;
			};
		} error;
		
		inline void seterror(json_error_type type, const char* ptr, const char* expected = NULL) {
			this->error.type = type;
			this->error.ptr = ptr;
			this->error.expected = expected;
		}

		inline void wseterror(json_error_type type, const wchar_t* ptr, const wchar_t* expected = NULL) {
			this->error.type = type;
			this->error.wptr = ptr;
			this->error.wexpected = expected;
		}
		
		json_callbacks* callbacks;
	};
	
	struct json_node {
		struct json_node*	parent;
		json_type	type;
		int32_t		level;
		union {
			char*		data;
			wchar_t*	wdata;
		};
		uint32_t	size;
		
		union {
			struct {
				struct json_node** k;
				struct json_node** v;
				uint32_t 	 c;
			} o;
			
			struct {
				struct json_node** v;
				uint32_t	 c;
			} a;
			
			char*		s;
			wchar_t*	ws;
			
			struct json_number n;
			
			bool		b;
		} value;
	};

	bool json_check(struct json_context* ctx, const char* json);
	bool jsonw_check(struct json_context* ctx, const wchar_t* json);
	
	struct json_node* json_create(struct json_node* parent, json_type type, uint32_t level, const char* data, uint32_t size);
	struct json_node* jsonw_create(struct json_node* parent, json_type type, uint32_t level, const wchar_t* data, uint32_t size);
	void json_destroy(struct json_node* j);
	#define jsonw_destroy json_destroy
	bool json_object_add(struct json_node* node, struct json_node* vkey, struct json_node* value);
	#define jsonw_object_add json_object_add
	bool json_array_add(struct json_node* node, struct json_node* value);
	#define jsonw_array_add json_array_add
	
	struct json_node* json_parse(struct json_context* ctx, const char* json);
	struct json_node* jsonw_parse(struct json_context* ctx, const wchar_t* json);

	bool json_call(struct json_context* ctx, const char* json);

	struct json_node* json_array_get(const struct json_node* node, uint32_t index);
	struct json_node* json_object_get(const struct json_node* node, const char* key);
	struct json_node* json_object_geti(const struct json_node* node, const char* key);
	struct json_node* jsonw_object_get(const struct json_node* node, const wchar_t* key);
	struct json_node* jsonw_object_geti(const struct json_node* node, const wchar_t* key);
	
	const char* json_key(const struct json_node* value, const char* def = 0);
	const wchar_t* jsonw_key(const struct json_node* value, const wchar_t* def = 0);
	const char* json_value(const struct json_node* value, const char* def = 0);
	const wchar_t* jsonw_value(const struct json_node* value, const wchar_t* def = 0);
	
	void json_debug_error(const struct json_context& ctx, const char* json);
	void jsonw_debug_error(const struct json_context& ctx, const wchar_t* json);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <data/uString.h>

class JSON {
private:
	inline static String key(const String& value) throw(const char*) {
		if ((value.length > 0) && (value[0] == L'\"'))
			return value;

		return String(L"\"").concat(value).concat(L"\"");
	}
	
	
public:
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

#endif

#endif //JAPPSY_UJSON_H