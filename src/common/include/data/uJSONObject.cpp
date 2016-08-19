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

void json_parse_to_array(RefJSONArray* object, const struct json_node* n) throw(const char*);

void json_parse_to_object(RefJSONObject* object, const struct json_node* n) throw(const char*) {
	if (n != 0) {
		if (n->type != json_type_object) {
			throw eConvert;
		}
		
		uint32_t cnt = n->value.o.c;
		for (int i = 0; i < cnt; i++) {
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
		
		uint32_t cnt = n->value.a.c;
		for (int i = 0; i < cnt; i++) {
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
		
		uint32_t cnt = n->value.o.c;
		for (int i = 0; i < cnt; i++) {
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
		
		uint32_t cnt = n->value.a.c;
		for (int i = 0; i < cnt; i++) {
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

RefJSONObject::RefJSONObject(const String& json) throw(const char*) {
	initialize();

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
	initialize();

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
	initialize();
	
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
