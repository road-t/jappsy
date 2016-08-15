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

#include "uJSONObject.h"
#include <data/uJSON.h>
#include <data/uNumber.h>

#define json_is_special(ch)	(((ch) == '\t') || ((ch) == '\r') || ((ch) == '\n') || ((ch) == '\b') || ((ch) == '\f'))
#define jsonw_is_special(ch)	(((ch) == L'\t') || ((ch) == L'\r') || ((ch) == L'\n') || ((ch) == L'\b') || ((ch) == L'\f'))

void json_parse_to_array(RefJSONArray* object, const struct json_node* n) throw(const char*);

void json_parse_to_object(RefJSONObject* object, const struct json_node* n) throw(const char*) {
	if (n != 0) {
		if (n->type != json_type_object) {
			throw eConvert;
		}
		
		uint32_t cnt = n->value.o.c - 1;
		for (int i = 0; i <= cnt; i++) {
			char* key = n->value.o.k[i]->value.s;
			struct json_node* j = n->value.o.v[i];
			
			if (j != NULL) {
				switch (j->type) {
					case json_type_null:
						object->put(key, Object());
						break;
					
					case json_type_object: {
						JSONObject o = new JSONObject();
						json_parse_to_object((RefJSONObject*)(o._object), j);
						object->put(key, o);
						break;
					}
					
					case json_type_array: {
						JSONArray a = new JSONArray();
						json_parse_to_array((RefJSONArray*)(a._object), j);
						object->put(key, a);
						break;
					}
						
					case json_type_string:
						object->put(key, String(j->value.s));
						break;
						
					case json_type_number:
						if (j->value.n.is_float) {
							object->put(key, Double::valueOf(j->value.n.v.f));
						} else {
							object->put(key, Long::valueOf(j->value.n.v.i));
						}
						break;
						
					case json_type_boolean:
						object->put(key, Java::Boolean::valueOf(j->value.b));
						break;
				}
			}
		}
	}
}

void json_parse_to_array(RefJSONArray* array, const struct json_node* n) throw(const char*) {
	if (n != NULL) {
		if (n->type != json_type_array) {
			throw eConvert;
		}
		
		uint32_t cnt = n->value.a.c - 1;
		for (int i = 0; i <= cnt; i++) {
			struct json_node* j = n->value.a.v[i];

			if (j != NULL) {
				switch (j->type) {
					case json_type_null:
						array->put(Object());
						break;
						
					case json_type_object: {
						JSONObject o = new JSONObject();
						json_parse_to_object((RefJSONObject*)(o._object), j);
						array->put(o);
						break;
					}
						
					case json_type_array: {
						JSONArray a = new JSONArray();
						json_parse_to_array((RefJSONArray*)(a._object), j);
						array->put(a);
						break;
					}
						
					case json_type_string:
						array->put(String(j->value.s));
						break;
						
					case json_type_number:
						if (j->value.n.is_float) {
							array->put(Double::valueOf(j->value.n.v.f));
						} else {
							array->put(Long::valueOf(j->value.n.v.i));
						}
						break;
						
					case json_type_boolean:
						array->put(Java::Boolean::valueOf(j->value.b));
						break;
				}
			}
		}
	}
}

void jsonw_parse_to_array(RefJSONArray* object, const struct json_node* n) throw(const char*);

void jsonw_parse_to_object(RefJSONObject* object, const struct json_node* n) throw(const char*) {
	if (n != 0) {
		if (n->type != json_type_object) {
			throw eConvert;
		}
		
		uint32_t cnt = n->value.o.c - 1;
		for (int i = 0; i <= cnt; i++) {
			wchar_t* key = n->value.o.k[i]->value.ws;
			struct json_node* j = n->value.o.v[i];
			
			if (j != NULL) {
				switch (j->type) {
					case json_type_null:
						object->put(key, Object());
						break;
						
					case json_type_object: {
						JSONObject o = new JSONObject();
						json_parse_to_object((RefJSONObject*)(o._object), j);
						object->put(key, o);
						break;
					}
						
					case json_type_array: {
						JSONArray a = new JSONArray();
						json_parse_to_array((RefJSONArray*)(a._object), j);
						object->put(key, a);
						break;
					}
						
					case json_type_string:
						object->put(key, String(j->value.ws));
						break;
						
					case json_type_number:
						if (j->value.n.is_float) {
							object->put(key, Double::valueOf(j->value.n.v.f));
						} else {
							object->put(key, Long::valueOf(j->value.n.v.i));
						}
						break;
						
					case json_type_boolean:
						object->put(key, Java::Boolean::valueOf(j->value.b));
						break;
				}
			}
		}
	}
}

void jsonw_parse_to_array(RefJSONArray* array, const struct json_node* n) throw(const char*) {
	if (n != NULL) {
		if (n->type != json_type_array) {
			throw eConvert;
		}
		
		uint32_t cnt = n->value.a.c - 1;
		for (int i = 0; i <= cnt; i++) {
			struct json_node* j = n->value.a.v[i];
			
			if (j != NULL) {
				switch (j->type) {
					case json_type_null:
						array->put(Object());
						break;
						
					case json_type_object: {
						JSONObject o = new JSONObject();
						json_parse_to_object((RefJSONObject*)(o._object), j);
						array->put(o);
						break;
					}
						
					case json_type_array: {
						JSONArray a = new JSONArray();
						json_parse_to_array((RefJSONArray*)(a._object), j);
						array->put(a);
						break;
					}
						
					case json_type_string:
						array->put(String(j->value.ws));
						break;
						
					case json_type_number:
						if (j->value.n.is_float) {
							array->put(Double::valueOf(j->value.n.v.f));
						} else {
							array->put(Long::valueOf(j->value.n.v.i));
						}
						break;
						
					case json_type_boolean:
						array->put(Java::Boolean::valueOf(j->value.b));
						break;
				}
			}
		}
	}
}

#ifdef DEBUG

void json_debug_error(const struct json_context& ctx, const char* json) {
	uint32_t size = 0;
	utf8_strlen(json, &size);
	int errofs = (int)((intptr_t)(ctx.error.ptr) - (intptr_t)(json));
	int prefix = 4;
	int start = errofs - 8; if (start < 0) { start = 0; prefix = 0; }
	int postfix = 4;
	int end = errofs + 8; if (end > size) { end = size; postfix = 0; }
	int len = (end - start);
	char* line1 = (char*)mmalloc((len + prefix + postfix + 1)*sizeof(char));
	char* line2 = (char*)mmalloc((len + prefix + postfix + 1)*sizeof(char));
	char* p1 = line1;
	char* p2 = line2;
	char* e = (char*)json;
	while (prefix-- > 1) {
		*p1 = ' ';
		*p2 = '.';
		p1++; p2++;
	}
	if (prefix-- > 0) {
		*p1 = ' ';
		*p2 = ' ';
		p1++; p2++;
	}
	while (len-- > 0) {
		if (e == ctx.error.ptr) *p1 = 'v'; else *p1 = ' ';
		char ch = *e;
		if (json_is_special(ch)) *p2 = ' '; else *p2 = *e;
		p1++; p2++; e++;
	}
	if (postfix-- > 0) {
		*p1 = ' ';
		*p2 = ' ';
		p1++; p2++;
	}
	while (postfix-- > 0) {
		*p1 = ' ';
		*p2 = '.';
		p1++; p2++;
	}
	*p1 = '\0';
	*p2 = '\0';
	
	LOG("JSON Parse Error:")
	LOG("%s", line1);
	LOG("%s", line2);
	
	mmfree(line1);
	mmfree(line2);
}

void jsonw_log(const wchar_t* wstr) {
	if (wstr == NULL)
		return;
	
	uint32_t size = wcs_toutf8_size(wstr);
	if (size == 0)
		return;

	char* str = (char*)mmalloc(size);
	if (str == NULL)
		return;
	
	wcs_toutf8(wstr, str, size);
	
	LOG("%s", str);
	
	mmfree(str);
}

void jsonw_debug_error(const struct json_context& ctx, const wchar_t* json) {
	uint32_t size = wcs_strlen(json, NULL);
	int errofs = (int)((intptr_t)(ctx.error.ptr) - (intptr_t)(json)) / sizeof(wchar_t);
	int prefix = 4;
	int start = errofs - 8; if (start < 0) { start = 0; prefix = 0; }
	int postfix = 4;
	int end = errofs + 8; if (end > size) { end = size; postfix = 0; }
	int len = (end - start);
	wchar_t* line1 = (wchar_t*)mmalloc((len + prefix + postfix + 1)*sizeof(wchar_t));
	wchar_t* line2 = (wchar_t*)mmalloc((len + prefix + postfix + 1)*sizeof(wchar_t));
	wchar_t* p1 = line1;
	wchar_t* p2 = line2;
	wchar_t* e = (wchar_t*)json;
	while (prefix-- > 1) {
		*p1 = L' ';
		*p2 = L'.';
		p1++; p2++;
	}
	if (prefix-- > 0) {
		*p1 = L' ';
		*p2 = L' ';
		p1++; p2++;
	}
	while (len-- > 0) {
		if (e == ctx.error.wptr) *p1 = L'v'; else *p1 = L' ';
		char ch = *e;
		if (jsonw_is_special(ch)) *p2 = L' '; else *p2 = *e;
		p1++; p2++; e++;
	}
	if (postfix-- > 0) {
		*p1 = L' ';
		*p2 = L' ';
		p1++; p2++;
	}
	while (postfix-- > 0) {
		*p1 = L' ';
		*p2 = L'.';
		p1++; p2++;
	}
	*p1 = L'\0';
	*p2 = L'\0';
	
	LOG("JSON Parse Error:")
	jsonw_log(line1);
	jsonw_log(line2);
	
	mmfree(line1);
	mmfree(line2);
}

#endif

RefJSONObject::RefJSONObject(const String& json) throw(const char*) {
	TYPE = TypeJSONObject;

	if (json._object == NULL)
		return;
	
	struct json_context ctx;
	struct json_node* root = jsonw_parse(&ctx, (wchar_t*)json);
	if (root == NULL) {
#ifdef DEBUG
		jsonw_debug_error(ctx, (wchar_t*)json);
#endif
		throw eConvert;
	} else {
		try {
			jsonw_parse_to_object(this, root);
		} catch (...) {
			jsonw_destroy(root);
			throw;
		}
		jsonw_destroy(root);
	}
}

RefJSONObject::RefJSONObject(const wchar_t* json) throw(const char*) {
	TYPE = TypeJSONObject;

	if (json == NULL)
		return;
	
	struct json_context ctx;
	struct json_node* root = jsonw_parse(&ctx, json);
	if (root == NULL) {
#ifdef DEBUG
		jsonw_debug_error(ctx, json);
#endif
		throw eConvert;
	} else {
		try {
			jsonw_parse_to_object(this, root);
		} catch (...) {
			jsonw_destroy(root);
			throw;
		}
		jsonw_destroy(root);
	}
}

RefJSONObject::RefJSONObject(const char* json) throw(const char*) {
	TYPE = TypeJSONObject;
	
	if (json == NULL)
		return;
	
	struct json_context ctx;
	struct json_node* root = json_parse(&ctx, json);
	if (root == NULL) {
#ifdef DEBUG
		json_debug_error(ctx, json);
#endif
		throw eConvert;
	} else {
		try {
			json_parse_to_object(this, root);
		} catch (...) {
			json_destroy(root);
			throw;
		}
		json_destroy(root);
	}
}

const String RefJSONObject::toString(int indentSpaces) const throw(const char*) {
	// TODO: json to string
	return L"";
}
