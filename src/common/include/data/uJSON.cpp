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

#include "uJSON.h"
#include <core/uMemory.h>
#include <data/uString.h>

#ifdef __cplusplus
extern "C" {
#endif

	#define json_is_space(ch)	(((ch) == ' ') || ((ch) == '\t') || ((ch) == '\r') || ((ch) == '\n') || ((ch) == '\b') || ((ch) == '\f'))
	#define json_is_hex(ch)		((((ch) >= '0') && ((ch) <= '9')) || (((ch) >= 'a') && ((ch) <= 'f')) || (((ch) >= 'A') && ((ch) <= 'F')))
	#define json_cur			*(*ptrptr)
	#define json_next			(ch = *(++(*ptrptr)))
	#define json_rev			(*ptrptr)--

	#define jsonw_is_space(ch)	(((ch) == L' ') || ((ch) == L'\t') || ((ch) == L'\r') || ((ch) == L'\n') || ((ch) == L'\b') || ((ch) == L'\f'))
	#define jsonw_is_hex(ch)	((((ch) >= L'0') && ((ch) <= L'9')) || (((ch) >= L'a') && ((ch) <= L'f')) || (((ch) >= L'A') && ((ch) <= L'F')))
	#define jsonw_cur			*(*ptrptr)
	#define jsonw_next			(ch = *(++(*ptrptr)))
	#define jsonw_rev			(*ptrptr)--

	bool json_check_number(char** ptrptr, int32_t level) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		char ch = json_cur;
		if (ch == '-') {
			json_next;
		}
		if (ch == '\0') {
			return false; // Неожиданный конец
		} else if ((ch == 'i') || (ch == 'I')) {
			json_next;
			if ((ch == 'n') || (ch == 'N')) {
				json_next;
				if ((ch == 'f') || (ch == 'F')) {
					return true;
				}
			}
			if (ch == '\0') {
				return false; // Неожиданный конец
			}
			return false; // Ошибка синтаксиса
		} else if (ch == '0') {
			json_next;
			goto json_check_number_point;
		} else if ((ch >= '1') && (ch <= '9')) {
			do {
				if (json_next == '\0') {
					return false; // Неожиданный конец
				} else if ((ch < '0') || (ch > '9')) {
					break;
				}
			} while (true);
		}
	json_check_number_point:
		if (ch == '.') {
			json_next;
			if ((ch >= '0') && (ch <= '9')) {
				do {
					if (json_next == '\0') {
						return false; // Неожиданный конец
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				return false; // Ошибка синтаксиса
			}
		}
		if ((ch == 'e') || (ch == 'E')) {
			json_next;
			if ((ch == '-') || (ch == '+')) {
				json_next;
			}
			if ((ch >= '0') && (ch <= '9')) {
				do {
					if (json_next == '\0') {
						return false; // Неожиданный конец
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				return false; // Ошибка синтаксиса
			}
		}
		json_rev;
		return true;
	}

	bool jsonw_check_number(wchar_t** ptrptr, int32_t level) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		wchar_t ch = jsonw_cur;
		if (ch == L'-') {
			jsonw_next;
		}
		if (ch == L'\0') {
			return false; // Неожиданный конец
		} else if ((ch == L'i') || (ch == L'I')) {
			jsonw_next;
			if ((ch == L'n') || (ch == L'N')) {
				jsonw_next;
				if ((ch == L'f') || (ch == L'F')) {
					return true;
				}
			}
			if (ch == L'\0') {
				return false; // Неожиданный конец
			}
			return false; // Ошибка синтаксиса
		} else if (ch == L'0') {
			jsonw_next;
			goto jsonw_check_number_point;
		} else if ((ch >= L'1') && (ch <= L'9')) {
			do {
				if (jsonw_next == L'\0') {
					return false; // Неожиданный конец
				} else if ((ch < L'0') || (ch > L'9')) {
					break;
				}
			} while (true);
		}
	jsonw_check_number_point:
		if (ch == L'.') {
			jsonw_next;
			if ((ch >= L'0') && (ch <= L'9')) {
				do {
					if (jsonw_next == L'\0') {
						return false; // Неожиданный конец
					} else if ((ch < L'0') || (ch > L'9')) {
						break;
					}
				} while (true);
			} else {
				return false; // Ошибка синтаксиса
			}
		}
		if ((ch == L'e') || (ch == L'E')) {
			jsonw_next;
			if ((ch == L'-') || (ch == L'+')) {
				jsonw_next;
			}
			if ((ch >= L'0') && (ch <= L'9')) {
				do {
					if (jsonw_next == L'\0') {
						return false; // Неожиданный конец
					} else if ((ch < L'0') || (ch > L'9')) {
						break;
					}
				} while (true);
			} else {
				return false; // Ошибка синтаксиса
			}
		}
		jsonw_rev;
		return true;
	}
	
	bool json_check_string(char** ptrptr, int32_t level) {
		char ch;
		while (json_next != '\0') {
			if (ch == '\"') {
				return true;
			} else if ((uint8_t)ch & 0x80) { // found UTF8
				int cnt;
				if ((uint8_t)(ch & 0xE0) == 0xC0) { // 2 byte UTF8
					cnt = 1;
				} else if ((uint8_t)(ch & 0xF0) == 0xE0) { // 3 byte UTF8
					cnt = 2;
				} else if ((uint8_t)(ch & 0xF8) == 0xF0) { // 4 byte UTF8
					cnt = 3;
				} else if ((uint8_t)(ch & 0xFC) == 0xF8) { // 5 byte UTF8
					cnt = 4;
				} else if ((uint8_t)(ch & 0xFE) == 0xFC) { // 6 byte UTF8
					cnt = 5;
				} else {
					return false; // Not UTF8
				}
				while ((cnt > 0) && (json_next != '\0')) {
					if ((uint8_t)(ch & 0xC0) != 0x80) {
						return false; // Not UTF8
					}
					cnt--;
				}
				if (cnt > 0) {
					if (ch == '\0') {
						return false; // Неожиданный конец
					} else {
						return false; // Ошибка синтаксиса
					}
				}
			} else if (ch == '\\') {
				json_next;
				if ((ch == '\"') || (ch == '\\') || (ch == '\/') || (ch == 'b') || (ch == 'f') || (ch == 'n') || (ch == 'r') || (ch == 't')) {
				} else if (ch == 'u') {
					int cnt = 4;
					while ((cnt > 0) && (json_next != '\0')) {
						if (!json_is_hex(ch)) {
							return false; // Ошибка синтаксиса
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == '\0') {
							return false; // Неожиданный конец
						} else {
							return false; // Ошибка синтаксиса
						}
					}
				} else if (ch == '\0') {
					return false; // Неожиданный конец
				} else {
					return false; // Ошибка синтаксиса
				}
			}
		}
		return false; // Неожиданный конец
	}

	bool jsonw_check_string(wchar_t** ptrptr, int32_t level) {
		wchar_t ch;
		while (jsonw_next != L'\0') {
			if (ch == L'\"') {
				return true;
#if __WCHAR_MAX__ <= 0x10000
			} else if ((uint16_t)(ch & 0xFC00) == 0xD800) { // found UTF16
				if (jsonw_next == L'\0') {
					return false; // Неожиданный конец
				}
				if ((uint16_t)(ch & 0xFC00) != 0xDC00) {
					return false; // Not UTF16
				}
#endif
			} else if (ch == L'\\') {
				jsonw_next;
				if ((ch == L'\"') || (ch == L'\\') || (ch == L'\/') || (ch == L'b') || (ch == L'f') || (ch == L'n') || (ch == L'r') || (ch == L't')) {
				} else if (ch == L'u') {
					int cnt = 4;
					while ((cnt > 0) && (jsonw_next != L'\0')) {
						if (!jsonw_is_hex(ch)) {
							return false; // Ошибка синтаксиса
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == L'\0') {
							return false; // Неожиданный конец
						} else {
							return false; // Ошибка синтаксиса
						}
					}
				} else if (ch == L'\0') {
					return false; // Неожиданный конец
				} else {
					return false; // Ошибка синтаксиса
				}
			}
		}
		return false; // Неожиданный конец
	}
	
	bool json_check_object(char** ptrptr, int32_t level);
	bool json_check_array(char** ptrptr, int32_t level);
	
	bool json_check_value(char** ptrptr, int32_t level) {
		char ch;
		while (json_next != '\0') {
			if (ch == '\"') {
				return json_check_string(ptrptr, level);
			} else if ((ch == '-') || ((ch >= '0') && (ch <= '9'))) {
				return json_check_number(ptrptr, level);
			} else if (ch == '{') {
				return json_check_object(ptrptr, level);
			} else if (ch == '[') {
				return json_check_array(ptrptr, level);
			} else if ((ch == 't') || (ch == 'T')) {
				json_next;
				if ((ch == 'r') || (ch == 'R')) {
					json_next;
					if ((ch == 'u') || (ch == 'U')) {
						json_next;
						if ((ch == 'e') || (ch == 'E')) {
							return true;
						}
					}
				}
				return false; // Ошибка синтаксиса
			} else if ((ch == 'f') || (ch == 'F')) {
				json_next;
				if ((ch == 'a') || (ch == 'A')) {
					json_next;
					if ((ch == 'l') || (ch == 'L')) {
						json_next;
						if ((ch == 's') || (ch == 'S')) {
							json_next;
							if ((ch == 'e') || (ch == 'E')) {
								return true;
							}
						}
					}
				}
				return false; // Ошибка синтаксиса
			} else if ((ch == 'n') || (ch == 'N')) {
				json_next;
				if ((ch == 'u') || (ch == 'U')) {
					json_next;
					if ((ch == 'l') || (ch == 'L')) {
						json_next;
						if ((ch == 'l') || (ch == 'L')) {
							return true;
						}
					}
				} else if ((ch == 'a') || (ch == 'A')) {
					json_next;
					if ((ch == 'n') || (ch == 'N')) {
						return true;
					}
				}
				return false; // Ошибка синтаксиса
			} else if ((ch == 'i') || (ch == 'I')) {
				json_next;
				if ((ch == 'n') || (ch == 'N')) {
					json_next;
					if ((ch == 'f') || (ch == 'F')) {
						return true;
					}
				}
				if (ch == '\0') {
					return false; // Неожиданный конец
				}
				return false; // Ошибка синтаксиса
			} else if (!json_is_space(ch)) {
				return false; // Ошибка синтаксиса
			}
		}
		return false; // Неожиданный конец
	}

	bool jsonw_check_object(wchar_t** ptrptr, int32_t level);
	bool jsonw_check_array(wchar_t** ptrptr, int32_t level);
	
	bool jsonw_check_value(wchar_t** ptrptr, int32_t level) {
		wchar_t ch;
		while (jsonw_next != L'\0') {
			if (ch == L'\"') {
				return jsonw_check_string(ptrptr, level);
			} else if ((ch == L'-') || ((ch >= L'0') && (ch <= L'9'))) {
				return jsonw_check_number(ptrptr, level);
			} else if (ch == L'{') {
				return jsonw_check_object(ptrptr, level);
			} else if (ch == L'[') {
				return jsonw_check_array(ptrptr, level);
			} else if ((ch == L't') || (ch == L'T')) {
				jsonw_next;
				if ((ch == L'r') || (ch == L'R')) {
					jsonw_next;
					if ((ch == L'u') || (ch == L'U')) {
						jsonw_next;
						if ((ch == L'e') || (ch == L'E')) {
							return true;
						}
					}
				}
				return false; // Ошибка синтаксиса
			} else if ((ch == L'f') || (ch == L'F')) {
				jsonw_next;
				if ((ch == L'a') || (ch == L'A')) {
					jsonw_next;
					if ((ch == L'l') || (ch == L'L')) {
						jsonw_next;
						if ((ch == L's') || (ch == L'S')) {
							jsonw_next;
							if ((ch == L'e') || (ch == L'E')) {
								return true;
							}
						}
					}
				}
				return false; // Ошибка синтаксиса
			} else if ((ch == L'n') || (ch == L'N')) {
				jsonw_next;
				if ((ch == L'u') || (ch == L'U')) {
					jsonw_next;
					if ((ch == L'l') || (ch == L'L')) {
						jsonw_next;
						if ((ch == L'l') || (ch == L'L')) {
							return true;
						}
					}
				} else if ((ch == L'a') || (ch == L'A')) {
					jsonw_next;
					if ((ch == L'n') || (ch == L'N')) {
						return true;
					}
				}
				return false; // Ошибка синтаксиса
			} else if ((ch == L'i') || (ch == L'I')) {
				jsonw_next;
				if ((ch == L'n') || (ch == L'N')) {
					jsonw_next;
					if ((ch == L'f') || (ch == L'F')) {
						return true;
					}
				}
				if (ch == L'\0') {
					return false; // Неожиданный конец
				}
				return false; // Ошибка синтаксиса
			} else if (!jsonw_is_space(ch)) {
				return false; // Ошибка синтаксиса
			}
		}
		return false; // Неожиданный конец
	}
	
	bool json_check_array(char** ptrptr, int32_t level) {
		char ch;
		level++;
	json_check_array_repeat:
		while (json_next != '\0') {
			if (ch == ']') {
				return true;
			} else if (!json_is_space(ch)) {
				json_rev;
				if (!json_check_value(ptrptr, level)) {
					return false;
				}
				while (json_next != '\0') {
					if (ch == ',') {
						goto json_check_array_repeat;
					} else if (ch == ']') {
						return true;
					} else if (!json_is_space(ch)) {
						return false; // Ошибка синтаксиса
					}
				}
				return false; // Неожиданный конец
			}
		}
		return false; // Неожиданный конец
	}

	bool jsonw_check_array(wchar_t** ptrptr, int32_t level) {
		wchar_t ch;
		level++;
	jsonw_check_array_repeat:
		while (jsonw_next != L'\0') {
			if (ch == L']') {
				return true;
			} else if (!jsonw_is_space(ch)) {
				jsonw_rev;
				if (!jsonw_check_value(ptrptr, level)) {
					return false;
				}
				while (jsonw_next != L'\0') {
					if (ch == L',') {
						goto jsonw_check_array_repeat;
					} else if (ch == L']') {
						return true;
					} else if (!jsonw_is_space(ch)) {
						return false; // Ошибка синтаксиса
					}
				}
				return false; // Неожиданный конец
			}
		}
		return false; // Неожиданный конец
	}
	
	bool json_check_object(char** ptrptr, int32_t level) {
		char ch;
		level++;
	json_check_object_repeat:
		while (json_next != '\0') {
			if (ch == '}') {
				return true;
			} else if (ch == '\"') {
				if (!json_check_string(ptrptr, level)) {
					return false;
				}
				while (json_next != '\0') {
					if (ch == ':') {
						if (!json_check_value(ptrptr, level)) {
							return false;
						}
						while (json_next != '\0') {
							if (ch == ',') {
								goto json_check_object_repeat;
							} else if (ch == '}') {
								return true;
							} else if (!json_is_space(ch)) {
								return false; // Ошибка синтаксиса
							}
						}
						return false; // Неожиданный конец
					} else if (!json_is_space(ch)) {
						return false; // Ошибка синтаксиса
					}
				}
				return false; // Неожиданный конец
			} else if (!json_is_space(ch)) {
				return false; // Ошибка синтаксиса
			}
		}
		return false; // Неожиданный конец
	}

	bool jsonw_check_object(wchar_t** ptrptr, int32_t level) {
		wchar_t ch;
		level++;
	jsonw_check_object_repeat:
		while (jsonw_next != L'\0') {
			if (ch == L'}') {
				return true;
			} else if (ch == L'\"') {
				if (!jsonw_check_string(ptrptr, level)) {
					return false;
				}
				while (jsonw_next != L'\0') {
					if (ch == L':') {
						if (!jsonw_check_value(ptrptr, level)) {
							return false;
						}
						while (jsonw_next != L'\0') {
							if (ch == L',') {
								goto jsonw_check_object_repeat;
							} else if (ch == L'}') {
								return true;
							} else if (!jsonw_is_space(ch)) {
								return false; // Ошибка синтаксиса
							}
						}
						return false; // Неожиданный конец
					} else if (!jsonw_is_space(ch)) {
						return false; // Ошибка синтаксиса
					}
				}
				return false; // Неожиданный конец
			} else if (!jsonw_is_space(ch)) {
				return false; // Ошибка синтаксиса
			}
		}
		return false; // Неожиданный конец
	}
	
	bool json_check(const char* json) {
		char* ptr = (char*)json;
		char** ptrptr = &ptr;
		char ch;
		while ((ch = *ptr) != '\0') {
			if (json_is_space(ch)) {
				ptr++;
			} else if (ch == '{') {
				if (!json_check_object(ptrptr, 0)) {
					return false;
				}
				while (json_next != '\0') {
					if (!json_is_space(ch)) {
						return false; // Ошибка синтаксиса
					}
				}
				return true;
			} else {
				return false; // Ошибка синтаксиса
			}
		}
		return false; // Неожиданный конец
	}

	bool jsonw_check(const wchar_t* json) {
		wchar_t* ptr = (wchar_t*)json;
		wchar_t** ptrptr = &ptr;
		wchar_t ch;
		while ((ch = *ptr) != L'\0') {
			if (jsonw_is_space(ch)) {
				ptr++;
			} else if (ch == L'{') {
				if (!jsonw_check_object(ptrptr, 0)) {
					return false;
				}
				while (jsonw_next != L'\0') {
					if (!jsonw_is_space(ch)) {
						return false; // Ошибка синтаксиса
					}
				}
				return true;
			} else {
				return false; // Ошибка синтаксиса
			}
		}
		return false; // Неожиданный конец
	}
	
	struct json_node* json_create(struct json_node* parent, json_type type, uint32_t level, const char* data, uint32_t size) {
		json_node* j = memAlloc(struct json_node, j, sizeof(struct json_node));
		if (j == NULL) {
			return NULL;
		}
		
		j->parent = parent;
		j->type = type;
		j->level = level;
		j->data = (char*)data;
		j->size = size;
		j->value.n.v.i = 0;
		
		return j;
	}

	struct json_node* jsonw_create(struct json_node* parent, json_type type, uint32_t level, const wchar_t* data, uint32_t size) {
		json_node* j = memAlloc(struct json_node, j, sizeof(struct json_node));
		if (j == NULL) {
			return NULL;
		}
		
		j->parent = parent;
		j->type = type;
		j->level = level;
		j->wdata = (wchar_t*)data;
		j->size = size;
		j->value.n.v.i = 0;
		
		return j;
	}

	void json_destroy(struct json_node* j) {
		if (j != NULL) {
			switch (j->type) {
				case json_type_object: {
					for (int i = j->value.o.c - 1; i >= 0; i--) {
						json_destroy(j->value.o.k[i]);
						json_destroy(j->value.o.v[i]);
					}
					memFree(j->value.o.k);
					memFree(j->value.o.v);
					break;
				}
					
				case json_type_array: {
					for (int i = j->value.a.c - 1; i >= 0; i--) {
						json_destroy(j->value.a.v[i]);
					}
					memFree(j->value.a.v);
					break;
				}
					
				case json_type_string: {
					if (j->value.s != NULL) {
						memFree(j->value.s);
					}
					break;
				}
			}
			
			memFree(j);
		}
	}
	
	bool json_object_add(struct json_node* node, struct json_node* vkey, struct json_node* value) {
		if ((node != NULL) && (node->type == json_type_object)) {
			struct json_node** jk = memRealloc(struct json_node*, jk, node->value.o.k, sizeof(struct json_node*) * (node->value.o.c + 1));
			struct json_node** jv = memRealloc(struct json_node*, jv, node->value.o.v, sizeof(struct json_node*) * (node->value.o.c + 1));
			if ((jk != NULL) && (jv != NULL)) {
				node->value.o.k = jk;
				node->value.o.v = jv;
				jk[node->value.o.c] = vkey;
				jv[node->value.o.c] = value;
				node->value.o.c++;
				
				return true;
			} else {
				if (jk != NULL) {
					for (int i = node->value.o.c - 1; i >= 0; i--) {
						json_destroy(jk[i]);
					}
					memFree(jk);
				} else {
					for (int i = node->value.o.c - 1; i >= 0; i--) {
						json_destroy(node->value.o.k[i]);
					}
					memFree(node->value.o.k);
				}
				node->value.o.k = NULL;
				
				if (jv != NULL) {
					for (int i = node->value.o.c - 1; i >= 0; i--) {
						json_destroy(jv[i]);
					}
					memFree(jv);
				} else {
					for (int i = node->value.o.c - 1; i >= 0; i--) {
						json_destroy(node->value.o.v[i]);
					}
					memFree(node->value.o.v);
				}
				node->value.o.v = NULL;
				
				node->value.o.c = 0;
			}
		}
		
		return false;
	}
	
	bool json_array_add(struct json_node* node, struct json_node* value) {
		if ((node != NULL) && (node->type == json_type_array)) {
			struct json_node** jv = memRealloc(struct json_node*, jv, node->value.a.v, sizeof(struct json_node*) * (node->value.a.c + 1));
			if (jv != 0) {
				node->value.a.v = jv;
				jv[node->value.a.c] = value;
				node->value.a.c++;
				
				return true;
			} else {
				for (int i = node->value.a.c - 1; i >= 0; i--) {
					json_destroy(node->value.a.v[i]);
				}
				memFree(node->value.a.v);
				node->value.a.c = 0;
			}
		}
		
		return false;
	}
	
#include <math.h>
#include <wchar.h>
	
	bool json_parse_number(char** ptrptr, struct json_node* node) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		char ch = *(*ptrptr);
		bool minus = false;
		if (ch == '-') {
			minus = true;
			json_next;
		}
		if (ch == '\0') {
			return false; // Неожиданный конец
		} else if ((ch == 'i') || (ch == 'I')) {
			json_next;
			if ((ch == 'n') || (ch == 'N')) {
				json_next;
				if ((ch == 'f') || (ch == 'F')) {
					node->value.n.is_float = true;
					node->value.n.v.f = (minus ? -INFINITY : INFINITY);
					node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data)) + sizeof(char);
					return true;
				}
			}
			if (ch == '\0') {
				return false; // Неожиданный конец
			}
			return false; // Ошибка синтаксиса
		} else if (ch == '0') {
			json_next;
			goto json_parse_number_point;
		} else if ((ch >= '1') && (ch <= '9')) {
			do {
				if (json_next == '\0') {
					return false; // Неожиданный конец
				} else if ((ch < '0') || (ch > '9')) {
					break;
				}
			} while (true);
		}
	json_parse_number_point:
		bool is_float = false;
		if (ch == '.') {
			is_float = true;
			json_next;
			if ((ch >= '0') && (ch <= '9')) {
				do {
					if (json_next == '\0') {
						return false; // Неожиданный конец
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				return false; // Ошибка синтаксиса
			}
		}
		if ((ch == 'e') || (ch == 'E')) {
			is_float = true;
			json_next;
			if ((ch == '-') || (ch == '+')) {
				json_next;
			}
			if ((ch >= '0') && (ch <= '9')) {
				do {
					if (json_next == '\0') {
						return false; // Неожиданный конец
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				return false; // Ошибка синтаксиса
			}
		}
		
		node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data));
		*(*ptrptr) = '\0';
		if (is_float) {
			node->value.n.is_float = true;
			node->value.n.v.f = atof(node->data);
		} else {
			node->value.n.is_float = false;
			node->value.n.v.i = atoll(node->data);
		}
		*(*ptrptr) = ch;
		
		json_rev;
		return true;
	}
	
	bool jsonw_parse_number(wchar_t** ptrptr, struct json_node* node) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		wchar_t ch = *(*ptrptr);
		bool minus = false;
		if (ch == L'-') {
			minus = true;
			jsonw_next;
		}
		if (ch == L'\0') {
			return false; // Неожиданный конец
		} else if ((ch == L'i') || (ch == L'I')) {
			jsonw_next;
			if ((ch == L'n') || (ch == L'N')) {
				jsonw_next;
				if ((ch == L'f') || (ch == L'F')) {
					node->value.n.is_float = true;
					node->value.n.v.f = (minus ? -INFINITY : INFINITY);
					node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata)) + sizeof(wchar_t);
					return true;
				}
			}
			if (ch == L'\0') {
				return false; // Неожиданный конец
			}
			return false; // Ошибка синтаксиса
		} else if (ch == L'0') {
			jsonw_next;
			goto jsonw_parse_number_point;
		} else if ((ch >= L'1') && (ch <= L'9')) {
			do {
				if (jsonw_next == L'\0') {
					return false; // Неожиданный конец
				} else if ((ch < L'0') || (ch > L'9')) {
					break;
				}
			} while (true);
		}
	jsonw_parse_number_point:
		bool is_float = false;
		if (ch == L'.') {
			is_float = true;
			jsonw_next;
			if ((ch >= L'0') && (ch <= L'9')) {
				do {
					if (jsonw_next == L'\0') {
						return false; // Неожиданный конец
					} else if ((ch < L'0') || (ch > L'9')) {
						break;
					}
				} while (true);
			} else {
				return false; // Ошибка синтаксиса
			}
		}
		if ((ch == L'e') || (ch == L'E')) {
			is_float = true;
			jsonw_next;
			if ((ch == L'-') || (ch == L'+')) {
				jsonw_next;
			}
			if ((ch >= L'0') && (ch <= L'9')) {
				do {
					if (json_next == L'\0') {
						return false; // Неожиданный конец
					} else if ((ch < L'0') || (ch > L'9')) {
						break;
					}
				} while (true);
			} else {
				return false; // Ошибка синтаксиса
			}
		}
		
		node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata));
		*(*ptrptr) = L'\0';
		if (is_float) {
			node->value.n.is_float = true;
			node->value.n.v.f = wcstold(node->wdata, NULL);
		} else {
			node->value.n.is_float = false;
			node->value.n.v.i = wcstoll(node->wdata, NULL, 10);
		}
		*(*ptrptr) = ch;
		
		jsonw_rev;
		return true;
	}
	
	bool json_parse_string(char** ptrptr, struct json_node* node) {
		char ch;
		uint32_t len = 0;
		char* save = *ptrptr;
		while (json_next != '\0') {
			if (ch == '\"') {
				node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data)) + sizeof(char);
				*ptrptr = save;
				goto json_parse_string_alloc;
			} else if (ch & 0x80) { // found UTF8
				len++;
				int cnt;
				if ((uint8_t)(ch & 0xE0) == 0xC0) { // 2 byte UTF8
					cnt = 1;
				} else if ((uint8_t)(ch & 0xF0) == 0xE0) { // 3 byte UTF8
					cnt = 2;
				} else if ((uint8_t)(ch & 0xF8) == 0xF0) { // 4 byte UTF8
					cnt = 3;
				} else if ((uint8_t)(ch & 0xFC) == 0xF8) { // 5 byte UTF8
					cnt = 4;
				} else if ((uint8_t)(ch & 0xFE) == 0xFC) { // 6 byte UTF8
					cnt = 5;
				} else {
					return false; // Not UTF8
				}
				while ((cnt > 0) && (json_next != '\0')) {
					len++;
					if ((uint8_t)(ch & 0xC0) != 0x80) {
						return false; // Not UTF8
					}
					cnt--;
				}
				if (cnt > 0) {
					if (ch == '\0') {
						return false; // Неожиданный конец
					} else {
						return false; // Ошибка синтаксиса
					}
				}
			} else if (ch == '\\') {
				json_next;
				if ((ch == '\"') || (ch == '\\') || (ch == '\/') || (ch == 'b') || (ch == 'f') || (ch == 'n') || (ch == 'r') || (ch == 't')) {
					len++;
				} else if (ch == 'u') {
					uint32_t utf16 = 0;
					int cnt = 4;
					while ((cnt > 0) && (json_next != '\0')) {
						utf16 <<= 4;
						if ((ch >= '0') && (ch <= '9')) {
							utf16 |= (uint16_t)(ch - '0');
						} else if ((ch >= 'A') && (ch <= 'F')) {
							utf16 |= (uint16_t)(ch - 'A' + 10);
						} else if ((ch >= 'a') && (ch <= 'a')) {
							utf16 |= (uint16_t)(ch - 'a' + 10);
						} else {
							return false; // Ошибка синтаксиса
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == '\0') {
							return false; // Неожиданный конец
						} else {
							return false; // Ошибка синтаксиса
						}
					}
					
					if ((utf16 & 0xFC00) == 0xD800) { // found UTF16
						json_next;
						if (ch == '\\') {
							json_next;
							if (ch == 'u') {
								uint32_t utf16_2 = 0;
								int cnt = 4;
								while ((cnt > 0) && (json_next != '\0')) {
									utf16_2 <<= 4;
									if ((ch >= '0') && (ch <= '9')) {
										utf16_2 |= (uint16_t)(ch - '0');
									} else if ((ch >= 'A') && (ch <= 'F')) {
										utf16_2 |= (uint16_t)(ch - 'A' + 10);
									} else if ((ch >= 'a') && (ch <= 'a')) {
										utf16_2 |= (uint16_t)(ch - 'a' + 10);
									} else {
										return false; // Ошибка синтаксиса
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == '\0') {
										return false; // Неожиданный конец
									} else {
										return false; // Ошибка синтаксиса
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									return false; // Ошибка синтаксиса
								}
							} else if (ch == '\0') {
								return false; // Неожиданный конец
							} else {
								return false; // Ошибка синтаксиса
							}
						} else if (ch == '\0') {
							return false; // Неожиданный конец
						} else {
							return false; // Ошибка синтаксиса
						}
					}
					
					if (utf16 <= 0x7F) { // 1 byte UTF8
						len++;
					} else if (utf16 <= 0x7FF) { // 2 byte UTF8
						len += 2;
					} else if (utf16 <= 0xFFFF) { // 3 byte UTF8
						len += 3;
					} else if (utf16 <= 0x1FFFFF) { // 4 byte UTF8
						len += 4;
					} else if (utf16 <= 0x3FFFFFF) { // 5 byte UTF8
						len += 5;
					} else if (utf16 <= 0x7FFFFFFF) { // 6 byte UTF8
						len += 6;
					} else {
						return false;
					}
				} else if (ch == '\0') {
					return false; // Неожиданный конец
				} else {
					return false; // Ошибка синтаксиса
				}
			} else {
				len++;
			}
		}
		return false; // Неожиданный конец
		
	json_parse_string_alloc:
		char* ptr = memAlloc(char, ptr, (len + 1) * sizeof(char));
		if (ptr == NULL) {
			return false;
		}
		node->value.s = ptr;
		
		while (json_next != '\0') {
			if (ch == '\"') {
				*ptr = '\0';
				return true;
			} else if (ch & 0x80) { // found UTF8
				*ptr = ch; ptr++;
				int cnt;
				if ((ch & 0xE0) == 0xC0) { // 2 byte UTF8
					cnt = 1;
				} else if ((ch & 0xF0) == 0xE0) { // 3 byte UTF8
					cnt = 2;
				} else if ((ch & 0xF8) == 0xF0) { // 4 byte UTF8
					cnt = 3;
				} else if ((ch & 0xFC) == 0xF8) { // 5 byte UTF8
					cnt = 4;
				} else if ((ch & 0xFE) == 0xFC) { // 6 byte UTF8
					cnt = 5;
				} else {
					return false; // Not UTF8
				}
				while ((cnt > 0) && (json_next != '\0')) {
					if ((ch & 0xC0) != 0x80) {
						return false; // Not UTF8
					}
					*ptr = ch; ptr++;
					cnt--;
				}
				if (cnt > 0) {
					if (ch == '\0') {
						return false; // Неожиданный конец
					} else {
						return false; // Ошибка синтаксиса
					}
				}
			} else if (ch == '\\') {
				json_next;
				if ((ch == '\"') || (ch == '\\') || (ch == '\/') || (ch == 'b') || (ch == 'f') || (ch == 'n') || (ch == 'r') || (ch == 't')) {
					*ptr = ch; ptr++;
				} else if (ch == 'u') {
					uint32_t utf16 = 0;
					int cnt = 4;
					while ((cnt > 0) && (json_next != '\0')) {
						utf16 <<= 4;
						if ((ch >= '0') && (ch <= '9')) {
							utf16 |= (uint16_t)(ch - '0');
						} else if ((ch >= 'A') && (ch <= 'F')) {
							utf16 |= (uint16_t)(ch - 'A' + 10);
						} else if ((ch >= 'a') && (ch <= 'a')) {
							utf16 |= (uint16_t)(ch - 'a' + 10);
						} else {
							return false; // Ошибка синтаксиса
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == '\0') {
							return false; // Неожиданный конец
						} else {
							return false; // Ошибка синтаксиса
						}
					}
					
					if ((utf16 & 0xFC00) == 0xD800) { // found UTF16
						json_next;
						if (ch == '\\') {
							json_next;
							if (ch == 'u') {
								uint32_t utf16_2 = 0;
								int cnt = 4;
								while ((cnt > 0) && (json_next != '\0')) {
									utf16_2 <<= 4;
									if ((ch >= '0') && (ch <= '9')) {
										utf16_2 |= (uint16_t)(ch - '0');
									} else if ((ch >= 'A') && (ch <= 'F')) {
										utf16_2 |= (uint16_t)(ch - 'A' + 10);
									} else if ((ch >= 'a') && (ch <= 'a')) {
										utf16_2 |= (uint16_t)(ch - 'a' + 10);
									} else {
										return false; // Ошибка синтаксиса
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == '\0') {
										return false; // Неожиданный конец
									} else {
										return false; // Ошибка синтаксиса
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									return false; // Ошибка синтаксиса
								}
							} else if (ch == '\0') {
								return false; // Неожиданный конец
							} else {
								return false; // Ошибка синтаксиса
							}
						} else if (ch == '\0') {
							return false; // Неожиданный конец
						} else {
							return false; // Ошибка синтаксиса
						}
					}
					
					if (utf16 <= 0x7F) { // 1 byte UTF8
						*ptr = utf16; ptr++;
					} else if (utf16 <= 0x7FF) { // 2 byte UTF8
						*ptr = 0xC0 | (utf16 >> 6); ptr++;
						*ptr = 0x80 | (utf16 & 0x3F); ptr++;
					} else if (utf16 <= 0xFFFF) { // 3 byte UTF8
						*ptr = 0xE0 | (utf16 >> 12); ptr++;
						*ptr = 0x80 | ((utf16 >> 6) & 0x3F); ptr++;
						*ptr = 0x80 | (utf16 & 0x3F); ptr++;
					} else if (utf16 <= 0x1FFFFF) { // 4 byte UTF8
						*ptr = 0xF0 | (utf16 >> 18); ptr++;
						*ptr = 0x80 | ((utf16 >> 12) & 0x3F); ptr++;
						*ptr = 0x80 | ((utf16 >> 6) & 0x3F); ptr++;
						*ptr = 0x80 | (utf16 & 0x3F); ptr++;
					} else if (utf16 <= 0x3FFFFFF) { // 5 byte UTF8
						*ptr = 0xF8 | (utf16 >> 24); ptr++;
						*ptr = 0x80 | ((utf16 >> 18) & 0x3F); ptr++;
						*ptr = 0x80 | ((utf16 >> 12) & 0x3F); ptr++;
						*ptr = 0x80 | ((utf16 >> 6) & 0x3F); ptr++;
						*ptr = 0x80 | (utf16 & 0x3F); ptr++;
					} else if (utf16 <= 0x7FFFFFFF) { // 6 byte UTF8
						*ptr = 0xFC | (utf16 >> 30); ptr++;
						*ptr = 0x80 | ((utf16 >> 24) & 0x3F); ptr++;
						*ptr = 0x80 | ((utf16 >> 18) & 0x3F); ptr++;
						*ptr = 0x80 | ((utf16 >> 12) & 0x3F); ptr++;
						*ptr = 0x80 | ((utf16 >> 6) & 0x3F); ptr++;
						*ptr = 0x80 | (utf16 & 0x3F); ptr++;
					} else {
						return false;
					}
				} else if (ch == '\0') {
					return false; // Неожиданный конец
				} else {
					return false; // Ошибка синтаксиса
				}
			} else {
				*ptr = ch; ptr++;
			}
		}
		return false; // Неожиданный конец
	}
	
	bool jsonw_parse_string(wchar_t** ptrptr, struct json_node* node) {
		wchar_t ch;
		uint32_t len = 0;
		wchar_t* save = *ptrptr;
		while (jsonw_next != L'\0') {
			if (ch == L'\"') {
				node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata)) + sizeof(wchar_t);
				*ptrptr = save;
				goto jsonw_parse_string_alloc;
#if __WCHAR_MAX__ <= 0x10000
			} else if ((uint16_t)(ch & 0xFC00) == 0xD800) { // found UTF16
				if (jsonw_next == L'\0') {
					return false; // Неожиданный конец
				}
				if ((uint16_t)(ch & 0xFC00) != 0xDC00) {
					return false; // Not UTF16
				}
				len += 2;
#endif
			} else if (ch == L'\\') {
				jsonw_next;
				if ((ch == L'\"') || (ch == L'\\') || (ch == L'\/') || (ch == L'b') || (ch == L'f') || (ch == L'n') || (ch == L'r') || (ch == L't')) {
					len++;
				} else if (ch == L'u') {
					uint32_t utf16 = 0;
					int cnt = 4;
					while ((cnt > 0) && (jsonw_next != L'\0')) {
						utf16 <<= 4;
						if ((ch >= L'0') && (ch <= L'9')) {
							utf16 |= (uint16_t)(ch - L'0');
						} else if ((ch >= L'A') && (ch <= L'F')) {
							utf16 |= (uint16_t)(ch - L'A' + 10);
						} else if ((ch >= L'a') && (ch <= L'a')) {
							utf16 |= (uint16_t)(ch - L'a' + 10);
						} else {
							return false; // Ошибка синтаксиса
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == L'\0') {
							return false; // Неожиданный конец
						} else {
							return false; // Ошибка синтаксиса
						}
					}
					
					if ((utf16 & 0xFC00) == 0xD800) { // found UTF16
						jsonw_next;
						if (ch == L'\\') {
							jsonw_next;
							if (ch == L'u') {
								uint32_t utf16_2 = 0;
								int cnt = 4;
								while ((cnt > 0) && (jsonw_next != L'\0')) {
									utf16_2 <<= 4;
									if ((ch >= L'0') && (ch <= L'9')) {
										utf16_2 |= (uint16_t)(ch - L'0');
									} else if ((ch >= L'A') && (ch <= L'F')) {
										utf16_2 |= (uint16_t)(ch - L'A' + 10);
									} else if ((ch >= L'a') && (ch <= L'a')) {
										utf16_2 |= (uint16_t)(ch - L'a' + 10);
									} else {
										return false; // Ошибка синтаксиса
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == L'\0') {
										return false; // Неожиданный конец
									} else {
										return false; // Ошибка синтаксиса
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									return false; // Ошибка синтаксиса
								}
							} else if (ch == L'\0') {
								return false; // Неожиданный конец
							} else {
								return false; // Ошибка синтаксиса
							}
						} else if (ch == L'\0') {
							return false; // Неожиданный конец
						} else {
							return false; // Ошибка синтаксиса
						}
					}
					
#if __WCHAR_MAX__ <= 0x10000
					if (utf16 <= 0xFFFF) { // 2 byte UTF16
						len++;
					} else if (utf16 <= 0xFFFFF) { // 4 byte UTF16
						len += 2;
					} else {
						return false; // No UTF32 to UTF16 conversion
					}
#else
					len++;
#endif
				} else if (ch == L'\0') {
					return false; // Неожиданный конец
				} else {
					return false; // Ошибка синтаксиса
				}
			} else {
				len++;
			}
		}
		return false; // Неожиданный конец
		
	jsonw_parse_string_alloc:
		wchar_t* ptr = memAlloc(wchar_t, ptr, (len + 1) * sizeof(wchar_t));
		if (ptr == NULL) {
			return false;
		}
		node->value.ws = ptr;
		
		while (jsonw_next != L'\0') {
			if (ch == L'\"') {
				*ptr = L'\0';
				return true;
#if __WCHAR_MAX__ <= 0x10000
			} else if ((uint16_t)(ch & 0xFC00) == 0xD800) { // found UTF16
				*ptr = ch; ptr++;
				if (jsonw_next == L'\0') {
					return false; // Неожиданный конец
				}
				if ((uint16_t)(ch & 0xFC00) != 0xDC00) {
					return false; // Not UTF16
				}
				*ptr = ch; ptr++;
#endif
			} else if (ch == L'\\') {
				jsonw_next;
				if ((ch == L'\"') || (ch == L'\\') || (ch == L'\/') || (ch == L'b') || (ch == L'f') || (ch == L'n') || (ch == L'r') || (ch == L't')) {
					*ptr = ch; ptr++;
				} else if (ch == L'u') {
					uint32_t utf16 = 0;
					int cnt = 4;
					while ((cnt > 0) && (jsonw_next != L'\0')) {
						utf16 <<= 4;
						if ((ch >= L'0') && (ch <= L'9')) {
							utf16 |= (uint16_t)(ch - L'0');
						} else if ((ch >= L'A') && (ch <= L'F')) {
							utf16 |= (uint16_t)(ch - L'A' + 10);
						} else if ((ch >= L'a') && (ch <= L'a')) {
							utf16 |= (uint16_t)(ch - L'a' + 10);
						} else {
							return false; // Ошибка синтаксиса
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == L'\0') {
							return false; // Неожиданный конец
						} else {
							return false; // Ошибка синтаксиса
						}
					}
					
					if ((utf16 & 0xFC00) == 0xD800) { // found UTF16
						jsonw_next;
						if (ch == L'\\') {
							jsonw_next;
							if (ch == L'u') {
								uint32_t utf16_2 = 0;
								int cnt = 4;
								while ((cnt > 0) && (jsonw_next != L'\0')) {
									utf16_2 <<= 4;
									if ((ch >= L'0') && (ch <= L'9')) {
										utf16_2 |= (uint16_t)(ch - L'0');
									} else if ((ch >= L'A') && (ch <= L'F')) {
										utf16_2 |= (uint16_t)(ch - L'A' + 10);
									} else if ((ch >= L'a') && (ch <= L'a')) {
										utf16_2 |= (uint16_t)(ch - L'a' + 10);
									} else {
										return false; // Ошибка синтаксиса
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == L'\0') {
										return false; // Неожиданный конец
									} else {
										return false; // Ошибка синтаксиса
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									return false; // Ошибка синтаксиса
								}
							} else if (ch == L'\0') {
								return false; // Неожиданный конец
							} else {
								return false; // Ошибка синтаксиса
							}
						} else if (ch == L'\0') {
							return false; // Неожиданный конец
						} else {
							return false; // Ошибка синтаксиса
						}
					}
					
#if __WCHAR_MAX__ <= 0x10000
					if (utf16 <= 0xFFFF) { // 2 byte UTF16
						*ptr = utf16; ptr++;
					} else if (utf16 <= 0xFFFFF) { // 4 byte UTF16
						*ptr = (wchar_t)((utf16 >> 10) + 0xD7C0); ptr++;
						*dst = (wchar_t)((utf16 & 0x3FF) + 0xDC00); ptr++;
					} else {
						return false; // No UTF32 to UTF16 conversion
					}
#else
					*ptr = (wchar_t)utf16; ptr++;
#endif
				} else if (ch == L'\0') {
					return false; // Неожиданный конец
				} else {
					return false; // Ошибка синтаксиса
				}
			} else {
				*ptr = ch; ptr++;
			}
		}
		return false; // Неожиданный конец
	}
	
	bool json_parse_object(char** ptrptr, struct json_node* node, int32_t level);
	bool json_parse_array(char** ptrptr, struct json_node* node, int32_t level);
	
	struct json_node* json_parse_node(char** ptrptr, struct json_node* parent, int32_t level) {
		char ch;
		struct json_node* node = NULL;
		while (json_next != '\0') {
			if (ch == '\"') {
				node = json_create(parent, json_type_string, level, *ptrptr, 0);
				if (node == NULL) {
					return NULL;
				}
				if (!json_parse_string(ptrptr, node)) {
					json_destroy(node);
					return NULL;
				}
				return node;
			} else if ((ch == '-') || ((ch >= '0') && (ch <= '9'))) {
				node = json_create(parent, json_type_number, level, *ptrptr, 0);
				if (node == NULL) {
					return NULL;
				}
				if (!json_parse_number(ptrptr, node)) {
					json_destroy(node);
					return NULL;
				}
				return node;
			} else if (ch == '{') {
				node = json_create(parent, json_type_object, level, *ptrptr, 0);
				if (node == NULL) {
					return NULL;
				}
				if (!json_parse_object(ptrptr, node, level)) {
					json_destroy(node);
					return NULL;
				}
				return node;
			} else if (ch == '[') {
				node = json_create(parent, json_type_array, level, *ptrptr, 0);
				if (node == NULL) {
					return NULL;
				}
				if (!json_parse_array(ptrptr, node, level)) {
					json_destroy(node);
					return NULL;
				}
				return node;
			} else if ((ch == 't') || (ch == 'T')) {
				json_next;
				if ((ch == 'r') || (ch == 'R')) {
					json_next;
					if ((ch == 'u') || (ch == 'U')) {
						json_next;
						if ((ch == 'e') || (ch == 'E')) {
							node = json_create(parent, json_type_boolean, level, *ptrptr - 3, 4);
							if (node == NULL) {
								return NULL;
							}
							node->value.b = true;
							return node;
						}
					}
				}
				if (ch == '\0') {
					return NULL; // Неожиданный конец
				}
				return NULL; // Ошибка синтаксиса
			} else if ((ch == 'f') || (ch == 'F')) {
				json_next;
				if ((ch == 'a') || (ch == 'A')) {
					json_next;
					if ((ch == 'l') || (ch == 'L')) {
						json_next;
						if ((ch == 's') || (ch == 'S')) {
							json_next;
							if ((ch == 'e') || (ch == 'E')) {
								node = json_create(parent, json_type_boolean, level, *ptrptr - 4, 5);
								if (node == NULL) {
									return NULL;
								}
								node->value.b = false;
								return node;
							}
						}
					}
				}
				if (ch == '\0') {
					return NULL; // Неожиданный конец
				}
				return NULL; // Ошибка синтаксиса
			} else if ((ch == 'n') || (ch == 'N')) {
				json_next;
				if ((ch == 'u') || (ch == 'U')) {
					json_next;
					if ((ch == 'l') || (ch == 'L')) {
						json_next;
						if ((ch == 'l') || (ch == 'L')) {
							node = json_create(parent, json_type_null, level, *ptrptr - 3, 4);
							if (node == NULL) {
								return NULL;
							}
							return node;
						}
					}
				} else if ((ch == 'a') || (ch == 'A')) {
					json_next;
					if ((ch == 'n') || (ch == 'N')) {
						node = json_create(parent, json_type_number, level, *ptrptr - 2, 3);
						if (node == NULL) {
							return NULL;
						}
						node->value.n.is_float = true;
						node->value.n.v.f = NAN;
						return node;
					}
				}
				if (ch == '\0') {
					return NULL; // Неожиданный конец
				}
				return NULL; // Ошибка синтаксиса
			} else if ((ch == 'i') || (ch == 'I')) {
				json_next;
				if ((ch == 'n') || (ch == 'N')) {
					json_next;
					if ((ch == 'f') || (ch == 'F')) {
						node = json_create(parent, json_type_number, level, *ptrptr - 2, 3);
						if (node == NULL) {
							return NULL;
						}
						node->value.n.is_float = true;
						node->value.n.v.f = INFINITY;
						return node;
					}
				}
				if (ch == '\0') {
					return NULL; // Неожиданный конец
				}
				return NULL; // Ошибка синтаксиса
			} else if (!json_is_space(ch)) {
				return NULL; // Ошибка синтаксиса
			}
		}
		return NULL; // Неожиданный конец
	}
	
	bool jsonw_parse_object(wchar_t** ptrptr, struct json_node* node, int32_t level);
	bool jsonw_parse_array(wchar_t** ptrptr, struct json_node* node, int32_t level);
	
	struct json_node* jsonw_parse_node(wchar_t** ptrptr, struct json_node* parent, int32_t level) {
		wchar_t ch;
		struct json_node* node = NULL;
		while (json_next != L'\0') {
			if (ch == L'\"') {
				node = jsonw_create(parent, json_type_string, level, *ptrptr, 0);
				if (node == NULL) {
					return NULL;
				}
				if (!jsonw_parse_string(ptrptr, node)) {
					jsonw_destroy(node);
					return NULL;
				}
				return node;
			} else if ((ch == L'-') || ((ch >= L'0') && (ch <= L'9'))) {
				node = jsonw_create(parent, json_type_number, level, *ptrptr, 0);
				if (node == NULL) {
					return NULL;
				}
				if (!jsonw_parse_number(ptrptr, node)) {
					jsonw_destroy(node);
					return NULL;
				}
				return node;
			} else if (ch == L'{') {
				node = jsonw_create(parent, json_type_object, level, *ptrptr, 0);
				if (node == NULL) {
					return NULL;
				}
				if (!jsonw_parse_object(ptrptr, node, level)) {
					jsonw_destroy(node);
					return NULL;
				}
				return node;
			} else if (ch == L'[') {
				node = jsonw_create(parent, json_type_array, level, *ptrptr, 0);
				if (node == NULL) {
					return NULL;
				}
				if (!jsonw_parse_array(ptrptr, node, level)) {
					jsonw_destroy(node);
					return NULL;
				}
				return node;
			} else if ((ch == L't') || (ch == L'T')) {
				jsonw_next;
				if ((ch == L'r') || (ch == L'R')) {
					jsonw_next;
					if ((ch == L'u') || (ch == L'U')) {
						jsonw_next;
						if ((ch == L'e') || (ch == L'E')) {
							node = jsonw_create(parent, json_type_boolean, level, *ptrptr - 3, 4 * sizeof(wchar_t));
							if (node == NULL) {
								return NULL;
							}
							node->value.b = true;
							return node;
						}
					}
				}
				if (ch == L'\0') {
					return NULL; // Неожиданный конец
				}
				return NULL; // Ошибка синтаксиса
			} else if ((ch == L'f') || (ch == L'F')) {
				jsonw_next;
				if ((ch == L'a') || (ch == L'A')) {
					jsonw_next;
					if ((ch == L'l') || (ch == L'L')) {
						jsonw_next;
						if ((ch == L's') || (ch == L'S')) {
							jsonw_next;
							if ((ch == L'e') || (ch == L'E')) {
								node = jsonw_create(parent, json_type_boolean, level, *ptrptr - 4, 5 * sizeof(wchar_t));
								if (node == NULL) {
									return NULL;
								}
								node->value.b = false;
								return node;
							}
						}
					}
				}
				if (ch == L'\0') {
					return NULL; // Неожиданный конец
				}
				return NULL; // Ошибка синтаксиса
			} else if ((ch == L'n') || (ch == L'N')) {
				jsonw_next;
				if ((ch == L'u') || (ch == L'U')) {
					jsonw_next;
					if ((ch == L'l') || (ch == L'L')) {
						jsonw_next;
						if ((ch == L'l') || (ch == L'L')) {
							node = jsonw_create(parent, json_type_null, level, *ptrptr - 3, 4 * sizeof(wchar_t));
							if (node == NULL) {
								return NULL;
							}
							return node;
						}
					}
				} else if ((ch == L'a') || (ch == L'A')) {
					jsonw_next;
					if ((ch == L'n') || (ch == L'N')) {
						node = jsonw_create(parent, json_type_number, level, *ptrptr - 2, 3 * sizeof(wchar_t));
						if (node == NULL) {
							return NULL;
						}
						node->value.n.is_float = true;
						node->value.n.v.f = NAN;
						return node;
					}
				}
				if (ch == L'\0') {
					return NULL; // Неожиданный конец
				}
				return NULL; // Ошибка синтаксиса
			} else if ((ch == L'i') || (ch == L'I')) {
				jsonw_next;
				if ((ch == L'n') || (ch == L'N')) {
					jsonw_next;
					if ((ch == L'f') || (ch == L'F')) {
						node = jsonw_create(parent, json_type_number, level, *ptrptr - 2, 3 * sizeof(wchar_t));
						if (node == NULL) {
							return NULL;
						}
						node->value.n.is_float = true;
						node->value.n.v.f = INFINITY;
						return node;
					}
				}
				if (ch == L'\0') {
					return NULL; // Неожиданный конец
				}
				return NULL; // Ошибка синтаксиса
			} else if (!jsonw_is_space(ch)) {
				return NULL; // Ошибка синтаксиса
			}
		}
		return NULL; // Неожиданный конец
	}
	
	bool json_parse_array(char** ptrptr, struct json_node* node, int32_t level) {
		char ch;
		level++;
	json_parse_array_repeat:
		while (json_next != '\0') {
			if (ch == ']') {
				node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data)) + sizeof(char);
				return true;
			} else if (!json_is_space(ch)) {
				json_rev;
				struct json_node* value = json_parse_node(ptrptr, node, level);
				if (value == NULL) {
					return false;
				}
				if (!json_array_add(node, value)) {
					json_destroy(value);
					return false;
				}
				while (json_next != '\0') {
					if (ch == ',') {
						goto json_parse_array_repeat;
					} else if (ch == ']') {
						node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data)) + sizeof(char);
						return true;
					} else if (!json_is_space(ch)) {
						return false; // Ошибка синтаксиса
					}
				}
				return false; // Неожиданный конец
			}
		}
		return false; // Неожиданный конец
	}
	
	bool jsonw_parse_array(wchar_t** ptrptr, struct json_node* node, int32_t level) {
		wchar_t ch;
		level++;
	jsonw_parse_array_repeat:
		while (jsonw_next != L'\0') {
			if (ch == L']') {
				node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata)) + sizeof(wchar_t);
				return true;
			} else if (!jsonw_is_space(ch)) {
				jsonw_rev;
				struct json_node* value = jsonw_parse_node(ptrptr, node, level);
				if (value == NULL) {
					return false;
				}
				if (!jsonw_array_add(node, value)) {
					jsonw_destroy(value);
					return false;
				}
				while (jsonw_next != L'\0') {
					if (ch == L',') {
						goto jsonw_parse_array_repeat;
					} else if (ch == L']') {
						node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata)) + sizeof(wchar_t);
						return true;
					} else if (!jsonw_is_space(ch)) {
						return false; // Ошибка синтаксиса
					}
				}
				return false; // Неожиданный конец
			}
		}
		return false; // Неожиданный конец
	}
	
	bool json_parse_object(char** ptrptr, struct json_node* node, int32_t level) {
		char ch;
		level++;
	json_parse_object_repeat:
		while (json_next != '\0') {
			if (ch == '}') {
				node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data)) + sizeof(char);
				return true;
			} else if (ch == '\"') {
				struct json_node* vkey = json_create(node, json_type_string, level, *ptrptr, 0);
				if (vkey == NULL) {
					return false;
				}
				if (!json_parse_string(ptrptr, vkey)) {
					json_destroy(vkey);
					return false;
				}
				while (json_next != '\0') {
					if (ch == ':') {
						struct json_node* value = json_parse_node(ptrptr, node, level);
						if (value == NULL) {
							json_destroy(vkey);
							return false;
						}
						if (!json_object_add(node, vkey, value)) {
							json_destroy(vkey);
							json_destroy(value);
							return false;
						}
						while (json_next != '\0') {
							if (ch == ',') {
								goto json_parse_object_repeat;
							} else if (ch == '}') {
								node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data)) + sizeof(char);
								return true;
							} else if (!json_is_space(ch)) {
								return false; // Ошибка синтаксиса
							}
						}
						return false; // Неожиданный конец
					} else if (!json_is_space(ch)) {
						return false; // Ошибка синтаксиса
					}
				}
				return false; // Неожиданный конец
			} else if (!json_is_space(ch)) {
				return false; // Ошибка синтаксиса
			}
		}
		return false; // Неожиданный конец
	}
	
	bool jsonw_parse_object(wchar_t** ptrptr, struct json_node* node, int32_t level) {
		wchar_t ch;
		level++;
	jsonw_parse_object_repeat:
		while (jsonw_next != L'\0') {
			if (ch == L'}') {
				node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata)) + sizeof(wchar_t);
				return true;
			} else if (ch == L'\"') {
				struct json_node* vkey = jsonw_create(node, json_type_string, level, *ptrptr, 0);
				if (vkey == NULL) {
					return false;
				}
				if (!jsonw_parse_string(ptrptr, vkey)) {
					jsonw_destroy(vkey);
					return false;
				}
				while (jsonw_next != L'\0') {
					if (ch == L':') {
						struct json_node* value = jsonw_parse_node(ptrptr, node, level);
						if (value == NULL) {
							jsonw_destroy(vkey);
							return false;
						}
						if (!jsonw_object_add(node, vkey, value)) {
							json_destroy(vkey);
							json_destroy(value);
							return false;
						}
						while (jsonw_next != L'\0') {
							if (ch == L',') {
								goto jsonw_parse_object_repeat;
							} else if (ch == L'}') {
								node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata)) + sizeof(wchar_t);
								return true;
							} else if (!jsonw_is_space(ch)) {
								return false; // Ошибка синтаксиса
							}
						}
						return false; // Неожиданный конец
					} else if (!jsonw_is_space(ch)) {
						return false; // Ошибка синтаксиса
					}
				}
				return false; // Неожиданный конец
			} else if (!jsonw_is_space(ch)) {
				return false; // Ошибка синтаксиса
			}
		}
		return false; // Неожиданный конец
	}
	
	struct json_node* json_parse(const char* json) {
		char* ptr = (char*)json;
		char** ptrptr = &ptr;
		char ch;
		while ((ch = *ptr) != '\0') {
			if (json_is_space(ch)) {
				ptr++;
			} else if (ch == '{') {
				json_node* node = json_create(0, json_type_object, 0, ptr, 0);
				if (node == NULL) {
					return NULL;
				}
				if (!json_parse_object(ptrptr, node, 0)) {
					json_destroy(node);
					return NULL;
				}
				while (json_next != '\0') {
					if (!json_is_space(ch)) {
						json_destroy(node);
						return NULL; // Ошибка синтаксиса
					}
				}
				return node;
			} else {
				return NULL; // Ошибка синтаксиса
			}
		}
		return NULL; // Неожиданный конец
	}
	
	struct json_node* jsonw_parse(const wchar_t* json) {
		wchar_t* ptr = (wchar_t*)json;
		wchar_t** ptrptr = &ptr;
		wchar_t ch;
		while ((ch = *ptr) != L'\0') {
			if (jsonw_is_space(ch)) {
				ptr++;
			} else if (ch == L'{') {
				json_node* node = jsonw_create(0, json_type_object, 0, ptr, 0);
				if (node == NULL) {
					return NULL;
				}
				if (!jsonw_parse_object(ptrptr, node, 0)) {
					jsonw_destroy(node);
					return NULL;
				}
				while (jsonw_next != L'\0') {
					if (!jsonw_is_space(ch)) {
						jsonw_destroy(node);
						return NULL; // Ошибка синтаксиса
					}
				}
				return node;
			} else {
				return NULL; // Ошибка синтаксиса
			}
		}
		return NULL; // Неожиданный конец
	}
	
/*
	void json_debug_tab(uint32_t level) {
		for (int i = 0; i < level; i++) {
			printf(" ");
		}
	}
	
	void json_debug(const struct json_node* j) {
		if (j != 0) {
			switch (j->type) {
				case json_type_null: {
					printf("null");
					break;
				}
					
				case json_type_object: {
					printf("{\r\n");
					uint32_t cnt = j->value.o.c - 1;
					for (int i = 0; i <= cnt; i++) {
						json_debug_tab(j->level+1);
						printf("\"%s\": ", j->value.o.k[i]->value.s);
						json_debug(j->value.o.v[i]);
						if (i != cnt) {
							printf(",\r\n");
						}
					}
					printf("\r\n");
					json_debug_tab(j->level);
					printf("}");
					break;
				}
					
				case json_type_array: {
					printf("[");
					uint32_t cnt = j->value.a.c - 1;
					for (int i = 0; i <= cnt; i++) {
						json_debug(j->value.a.v[i]);
						if (i != cnt) {
							printf(", ");
						}
					}
					printf("]");
					break;
				}
					
				case json_type_string: {
					printf("\"%s\"", j->value.s);
					break;
				}
					
				case json_type_number: {
					if (j->value.n.is_float) {
						printf("%lf", j->value.n.v.f);
					} else {
						printf("%lld", j->value.n.v.i);
					}
					break;
				}
					
				case json_type_boolean: {
					if (j->value.b) {
						printf("true");
					} else {
						printf("false");
					}
					break;
				}
			}
		}
	}
 */
	
	struct json_node* json_array_get(const struct json_node* node, uint32_t index) {
		if (node != 0) {
			if (node->type == json_type_object) {
				if (node->value.o.c > index) {
					return node->value.o.v[index];
				}
			} else if (node->type == json_type_array) {
				if (node->value.a.c > index) {
					return node->value.a.v[index];
				}
			}
		}
		return NULL;
	}
	
	struct json_node* json_object_get(const struct json_node* node, const char* key) {
		if (node != 0) {
			if (node->type == json_type_object) {
				for (int i = node->value.o.c - 1; i >= 0; i--) {
					if (node->value.o.k[i]->type == json_type_string) {
						if (strcmp(node->value.o.k[i]->value.s, key) == 0) {
							return node->value.o.v[i];
						}
					}
				}
			}
		}
		return 0;
	}
	
	struct json_node* json_object_geti(const struct json_node* node, const char* key) {
		if (node != 0) {
			if (node->type == json_type_object) {
				for (int i = node->value.o.c - 1; i >= 0; i--) {
					if (node->value.o.k[i]->type == json_type_string) {
						if (utf8_cmpi(node->value.o.k[i]->value.s, key) == 0) {
							return node->value.o.v[i];
						}
					}
				}
			}
		}
		return 0;
	}
	
	struct json_node* jsonw_object_get(const struct json_node* node, const wchar_t* key) {
		if (node != 0) {
			if (node->type == json_type_object) {
				for (int i = node->value.o.c - 1; i >= 0; i--) {
					if (node->value.o.k[i]->type == json_type_string) {
						if (wcscmp(node->value.o.k[i]->value.ws, key) == 0) {
							return node->value.o.v[i];
						}
					}
				}
			}
		}
		return 0;
	}
	
	struct json_node* jsonw_object_geti(const struct json_node* node, const wchar_t* key) {
		if (node != 0) {
			if (node->type == json_type_object) {
				for (int i = node->value.o.c - 1; i >= 0; i--) {
					if (node->value.o.k[i]->type == json_type_string) {
						if (wcs_cmpi(node->value.o.k[i]->value.ws, key) == 0) {
							return node->value.o.v[i];
						}
					}
				}
			}
		}
		return 0;
	}
	
	const char* json_key(const struct json_node* value, const char* def) {
		if ((value != 0) && (value->parent != 0) && (value->parent->type == json_type_object)) {
			for (int i = value->parent->value.o.c - 1; i >= 0; i--) {
				if (value->parent->value.o.v[i] == value) {
					return value->parent->value.o.k[i]->value.s;
				}
			}
		}
		return def;
	}
	
	const wchar_t* jsonw_key(const struct json_node* value, const wchar_t* def) {
		if ((value != 0) && (value->parent != 0) && (value->parent->type == json_type_object)) {
			for (int i = value->parent->value.o.c - 1; i >= 0; i--) {
				if (value->parent->value.o.v[i] == value) {
					return value->parent->value.o.k[i]->value.ws;
				}
			}
		}
		return def;
	}
	
	const char* json_string(const struct json_node* value, const char* def) {
		if (value != 0) {
			if (value->type == json_type_string) {
				return value->value.s;
			}
		}
		return def;
	}

	const wchar_t* jsonw_string(const struct json_node* value, const wchar_t* def) {
		if (value != 0) {
			if (value->type == json_type_string) {
				return value->value.ws;
			}
		}
		return def;
	}

#ifdef __cplusplus
}
#endif


String JSON::encode(const String& value) throw(const char*) {
	// TODO: encode JSON string
	return value;
}
