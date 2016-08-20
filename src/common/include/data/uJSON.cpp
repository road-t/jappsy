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
	#define json_is_special(ch)	(((ch) == '\t') || ((ch) == '\r') || ((ch) == '\n') || ((ch) == '\b') || ((ch) == '\f'))
	#define json_cur			*(*ptrptr)
	#define json_next			(ch = *(++(*ptrptr)))
	#define json_rev			(*ptrptr)--

	#define jsonw_is_space(ch)	(((ch) == L' ') || ((ch) == L'\t') || ((ch) == L'\r') || ((ch) == L'\n') || ((ch) == L'\b') || ((ch) == L'\f'))
	#define jsonw_is_hex(ch)	((((ch) >= L'0') && ((ch) <= L'9')) || (((ch) >= L'a') && ((ch) <= L'f')) || (((ch) >= L'A') && ((ch) <= L'F')))
	#define jsonw_is_special(ch)	(((ch) == L'\t') || ((ch) == L'\r') || ((ch) == L'\n') || ((ch) == L'\b') || ((ch) == L'\f'))
	#define jsonw_cur			*(*ptrptr)
	#define jsonw_next			(ch = *(++(*ptrptr)))
	#define jsonw_rev			(*ptrptr)--
	
	#define json_err(type, ...)		ctx->seterror(type, *ptrptr, ## __VA_ARGS__)
	#define jsonw_err(type, ...)	ctx->wseterror(type, *ptrptr, ## __VA_ARGS__)

	bool json_check_number(struct json_context* ctx, char** ptrptr, int32_t level) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		char ch = json_cur;
		if (ch == '-') {
			json_next;
		}
		if (ch == '\0') {
			json_err(json_error_eof);
			return false;
		} else if ((ch == 'i') || (ch == 'I')) {
			json_next;
			if ((ch == 'n') || (ch == 'N')) {
				json_next;
				if ((ch == 'f') || (ch == 'F')) {
					return true;
				}
			}
			if (ch == '\0') {
				json_err(json_error_eof);
				return false;
			}
			json_err(json_error_syntax);
			return false;
		} else if (ch == '0') {
			json_next;
			goto json_check_number_point;
		} else if ((ch >= '1') && (ch <= '9')) {
			do {
				if (json_next == '\0') {
					json_err(json_error_eof);
					return false;
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
						json_err(json_error_eof);
						return false;
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				json_err(json_error_syntax);
				return false;
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
						json_err(json_error_eof);
						return false;
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				json_err(json_error_syntax);
				return false;
			}
		}
		json_rev;
		return true;
	}

	bool jsonw_check_number(struct json_context* ctx, wchar_t** ptrptr, int32_t level) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		wchar_t ch = jsonw_cur;
		if (ch == L'-') {
			jsonw_next;
		}
		if (ch == L'\0') {
			jsonw_err(json_error_eof);
			return false;
		} else if ((ch == L'i') || (ch == L'I')) {
			jsonw_next;
			if ((ch == L'n') || (ch == L'N')) {
				jsonw_next;
				if ((ch == L'f') || (ch == L'F')) {
					return true;
				}
			}
			if (ch == L'\0') {
				jsonw_err(json_error_eof);
				return false;
			}
			jsonw_err(json_error_syntax);
			return false;
		} else if (ch == L'0') {
			jsonw_next;
			goto jsonw_check_number_point;
		} else if ((ch >= L'1') && (ch <= L'9')) {
			do {
				if (jsonw_next == L'\0') {
					jsonw_err(json_error_eof);
					return false;
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
						jsonw_err(json_error_eof);
						return false;
					} else if ((ch < L'0') || (ch > L'9')) {
						break;
					}
				} while (true);
			} else {
				jsonw_err(json_error_syntax);
				return false;
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
						jsonw_err(json_error_eof);
						return false;
					} else if ((ch < L'0') || (ch > L'9')) {
						break;
					}
				} while (true);
			} else {
				jsonw_err(json_error_syntax);
				return false;
			}
		}
		jsonw_rev;
		return true;
	}
	
	bool json_check_string(struct json_context* ctx, char** ptrptr, int32_t level) {
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
						json_err(json_error_eof);
						return false;
					} else {
						json_err(json_error_syntax);
						return false;
					}
				}
			} else if (ch == '\\') {
				json_next;
				if ((ch == '\"') || (ch == '\\') || (ch == '/') || (ch == 'b') || (ch == 'f') || (ch == 'n') || (ch == 'r') || (ch == 't')) {
				} else if (ch == 'u') {
					int cnt = 4;
					while ((cnt > 0) && (json_next != '\0')) {
						if (!json_is_hex(ch)) {
							json_err(json_error_syntax);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == '\0') {
							json_err(json_error_eof);
							return false;
						} else {
							json_err(json_error_syntax);
							return false;
						}
					}
				} else if (ch == '\0') {
					json_err(json_error_eof);
					return false;
				} else {
					json_err(json_error_syntax);
					return false;
				}
			}
		}
		json_err(json_error_eof);
		return false;
	}

	bool jsonw_check_string(struct json_context* ctx, wchar_t** ptrptr, int32_t level) {
		wchar_t ch;
		while (jsonw_next != L'\0') {
			if (ch == L'\"') {
				return true;
#if __WCHAR_MAX__ <= 0x10000
			} else if ((uint16_t)(ch & 0xFC00) == 0xD800) { // found UTF16
				if (jsonw_next == L'\0') {
					json_err(json_error_eof);
					return false;
				}
				if ((uint16_t)(ch & 0xFC00) != 0xDC00) {
					return false; // Not UTF16
				}
#endif
			} else if (ch == L'\\') {
				jsonw_next;
				if ((ch == L'\"') || (ch == L'\\') || (ch == L'/') || (ch == L'b') || (ch == L'f') || (ch == L'n') || (ch == L'r') || (ch == L't')) {
				} else if (ch == L'u') {
					int cnt = 4;
					while ((cnt > 0) && (jsonw_next != L'\0')) {
						if (!jsonw_is_hex(ch)) {
							jsonw_err(json_error_syntax);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == L'\0') {
							jsonw_err(json_error_eof);
							return false;
						} else {
							jsonw_err(json_error_syntax);
							return false;
						}
					}
				} else if (ch == L'\0') {
					jsonw_err(json_error_eof);
					return false;
				} else {
					jsonw_err(json_error_syntax);
					return false;
				}
			}
		}
		jsonw_err(json_error_eof);
		return false;
	}
	
	bool json_skip_comment(struct json_context* ctx, char** ptrptr) {
		char ch;
		if (json_next == '\0') {
			json_err(json_error_eof);
			return false;
		}
		if (ch == '/') {
			while (json_next != '\0') {
				if ((ch == '\n') || (ch == '\r')) {
					break;
				}
			}
			json_rev;
			return true;
		} else if (ch == '*') {
			json_next;
			while (ch != '\0') {
				if (ch == '*') {
					if (json_next == '\0') {
						break;
					} else if (ch == '/') {
						return true;
					}
				}
				json_next;
			}
			json_rev;
			return true;
		}
		json_err(json_error_syntax);
		return false;
	}
	
	bool jsonw_skip_comment(struct json_context* ctx, wchar_t** ptrptr) {
		wchar_t ch;
		if (jsonw_next == L'\0') {
			jsonw_err(json_error_eof);
			return false;
		}
		if (ch == L'/') {
			while (jsonw_next != L'\0') {
				if ((ch == L'\n') || (ch == L'\r')) {
					break;
				}
			}
			jsonw_rev;
			return true;
		} else if (ch == L'*') {
			jsonw_next;
			while (ch != L'\0') {
				if (ch == L'*') {
					if (jsonw_next == L'\0') {
						break;
					} else if (ch == L'/') {
						return true;
					}
				}
				jsonw_next;
			}
			jsonw_rev;
			return true;
		}
		jsonw_err(json_error_syntax);
		return false;
	}
	
	bool json_check_object(struct json_context* ctx, char** ptrptr, int32_t level);
	bool json_check_array(struct json_context* ctx, char** ptrptr, int32_t level);
	
	bool json_check_value(struct json_context* ctx, char** ptrptr, int32_t level) {
		char ch;
		while (json_next != '\0') {
			if (ch == '\"') {
				return json_check_string(ctx, ptrptr, level);
			} else if ((ch == '-') || ((ch >= '0') && (ch <= '9'))) {
				return json_check_number(ctx, ptrptr, level);
			} else if (ch == '{') {
				return json_check_object(ctx, ptrptr, level);
			} else if (ch == '[') {
				return json_check_array(ctx, ptrptr, level);
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
				json_err(json_error_syntax);
				return false;
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
				json_err(json_error_syntax);
				return false;
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
				json_err(json_error_syntax);
				return false;
			} else if ((ch == 'i') || (ch == 'I')) {
				json_next;
				if ((ch == 'n') || (ch == 'N')) {
					json_next;
					if ((ch == 'f') || (ch == 'F')) {
						return true;
					}
				}
				if (ch == '\0') {
					json_err(json_error_eof);
					return false;
				}
				json_err(json_error_syntax);
				return false;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (!json_is_space(ch)) {
				json_err(json_error_syntax);
				return false;
			}
		}
		json_err(json_error_eof);
		return false;
	}

	bool jsonw_check_object(struct json_context* ctx, wchar_t** ptrptr, int32_t level);
	bool jsonw_check_array(struct json_context* ctx, wchar_t** ptrptr, int32_t level);
	
	bool jsonw_check_value(struct json_context* ctx, wchar_t** ptrptr, int32_t level) {
		wchar_t ch;
		while (jsonw_next != L'\0') {
			if (ch == L'\"') {
				return jsonw_check_string(ctx, ptrptr, level);
			} else if ((ch == L'-') || ((ch >= L'0') && (ch <= L'9'))) {
				return jsonw_check_number(ctx, ptrptr, level);
			} else if (ch == L'{') {
				return jsonw_check_object(ctx, ptrptr, level);
			} else if (ch == L'[') {
				return jsonw_check_array(ctx, ptrptr, level);
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
				jsonw_err(json_error_syntax);
				return false;
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
				jsonw_err(json_error_syntax);
				return false;
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
				jsonw_err(json_error_syntax);
				return false;
			} else if ((ch == L'i') || (ch == L'I')) {
				jsonw_next;
				if ((ch == L'n') || (ch == L'N')) {
					jsonw_next;
					if ((ch == L'f') || (ch == L'F')) {
						return true;
					}
				}
				if (ch == L'\0') {
					jsonw_err(json_error_eof);
					return false;
				}
				jsonw_err(json_error_syntax);
				return false;
			} else if (ch == '/') {
				if (!jsonw_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (!jsonw_is_space(ch)) {
				jsonw_err(json_error_syntax);
				return false;
			}
		}
		jsonw_err(json_error_eof);
		return false;
	}
	
	bool json_check_array(struct json_context* ctx, char** ptrptr, int32_t level) {
		char ch;
		level++;
	json_check_array_repeat:
		while (json_next != '\0') {
			if (ch == ']') {
				return true;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (!json_is_space(ch)) {
				json_rev;
				if (!json_check_value(ctx, ptrptr, level)) {
					return false;
				}
				while (json_next != '\0') {
					if (ch == ',') {
						goto json_check_array_repeat;
					} else if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							return false;
						}
					} else if (ch == ']') {
						return true;
					} else if (!json_is_space(ch)) {
						json_err(json_error_syntax);
						return false;
					}
				}
				json_err(json_error_eof);
				return false;
			}
		}
		json_err(json_error_eof);
		return false;
	}

	bool jsonw_check_array(struct json_context* ctx, wchar_t** ptrptr, int32_t level) {
		wchar_t ch;
		level++;
	jsonw_check_array_repeat:
		while (jsonw_next != L'\0') {
			if (ch == L']') {
				return true;
			} else if (ch == '/') {
				if (!jsonw_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (!jsonw_is_space(ch)) {
				jsonw_rev;
				if (!jsonw_check_value(ctx, ptrptr, level)) {
					return false;
				}
				while (jsonw_next != L'\0') {
					if (ch == L',') {
						goto jsonw_check_array_repeat;
					} else if (ch == '/') {
						if (!jsonw_skip_comment(ctx, ptrptr)) {
							return false;
						}
					} else if (ch == L']') {
						return true;
					} else if (!jsonw_is_space(ch)) {
						jsonw_err(json_error_syntax);
						return false;
					}
				}
				jsonw_err(json_error_eof);
				return false;
			}
		}
		jsonw_err(json_error_eof);
		return false;
	}
	
	bool json_check_object(struct json_context* ctx, char** ptrptr, int32_t level) {
		char ch;
		level++;
	json_check_object_repeat:
		while (json_next != '\0') {
			if (ch == '}') {
				return true;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (ch == '\"') {
				if (!json_check_string(ctx, ptrptr, level)) {
					return false;
				}
				while (json_next != '\0') {
					if (ch == ':') {
						if (!json_check_value(ctx, ptrptr, level)) {
							return false;
						}
						while (json_next != '\0') {
							if (ch == ',') {
								goto json_check_object_repeat;
							} else if (ch == '/') {
								if (!json_skip_comment(ctx, ptrptr)) {
									return false;
								}
							} else if (ch == '}') {
								return true;
							} else if (!json_is_space(ch)) {
								json_err(json_error_syntax);
								return false;
							}
						}
						json_err(json_error_eof);
						return false;
					} else if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							return false;
						}
					} else if (!json_is_space(ch)) {
						json_err(json_error_syntax);
						return false;
					}
				}
				json_err(json_error_eof);
				return false;
			} else if (!json_is_space(ch)) {
				json_err(json_error_syntax);
				return false;
			}
		}
		json_err(json_error_eof);
		return false;
	}

	bool jsonw_check_object(struct json_context* ctx, wchar_t** ptrptr, int32_t level) {
		wchar_t ch;
		level++;
	jsonw_check_object_repeat:
		while (jsonw_next != L'\0') {
			if (ch == L'}') {
				return true;
			} else if (ch == '/') {
				if (!jsonw_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (ch == L'\"') {
				if (!jsonw_check_string(ctx, ptrptr, level)) {
					return false;
				}
				while (jsonw_next != L'\0') {
					if (ch == L':') {
						if (!jsonw_check_value(ctx, ptrptr, level)) {
							return false;
						}
						while (jsonw_next != L'\0') {
							if (ch == L',') {
								goto jsonw_check_object_repeat;
							} else if (ch == '/') {
								if (!jsonw_skip_comment(ctx, ptrptr)) {
									return false;
								}
							} else if (ch == L'}') {
								return true;
							} else if (!jsonw_is_space(ch)) {
								jsonw_err(json_error_syntax);
								return false;
							}
						}
						jsonw_err(json_error_eof);
						return false;
					} else if (ch == '/') {
						if (!jsonw_skip_comment(ctx, ptrptr)) {
							return false;
						}
					} else if (!jsonw_is_space(ch)) {
						jsonw_err(json_error_syntax);
						return false;
					}
				}
				jsonw_err(json_error_eof);
				return false;
			} else if (!jsonw_is_space(ch)) {
				jsonw_err(json_error_syntax);
				return false;
			}
		}
		jsonw_err(json_error_eof);
		return false;
	}
	
	bool json_check(struct json_context* ctx, const char* json) {
		ctx->seterror(json_error_none, NULL);
		char* ptr = (char*)json;
		char** ptrptr = &ptr;
		char ch;
		while ((ch = *ptr) != '\0') {
			if (json_is_space(ch)) {
				ptr++;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (ch == '{') {
				if (!json_check_object(ctx, ptrptr, 0)) {
					return false;
				}
				while (json_next != '\0') {
					if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							return false;
						}
					} else if (!json_is_space(ch)) {
						json_err(json_error_syntax);
						return false;
					}
				}
				return true;
			} else {
				json_err(json_error_syntax);
				return false;
			}
		}
		json_err(json_error_eof);
		return false;
	}

	bool jsonw_check(struct json_context* ctx, const wchar_t* json) {
		ctx->wseterror(json_error_none, NULL);
		wchar_t* ptr = (wchar_t*)json;
		wchar_t** ptrptr = &ptr;
		wchar_t ch;
		while ((ch = *ptr) != L'\0') {
			if (jsonw_is_space(ch)) {
				ptr++;
			} else if (ch == L'/') {
				if (!jsonw_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (ch == L'{') {
				if (!jsonw_check_object(ctx, ptrptr, 0)) {
					return false;
				}
				while (jsonw_next != L'\0') {
					if (ch == L'/') {
						if (!jsonw_skip_comment(ctx, ptrptr)) {
							return false;
						}
					} else if (!jsonw_is_space(ch)) {
						jsonw_err(json_error_syntax);
						return false;
					}
				}
				return true;
			} else {
				jsonw_err(json_error_syntax);
				return false;
			}
		}
		jsonw_err(json_error_eof);
		return false;
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
	
	bool json_parse_number(struct json_context* ctx, char** ptrptr, struct json_node* node) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		char ch = *(*ptrptr);
		bool minus = false;
		if (ch == '-') {
			minus = true;
			json_next;
		}
		if (ch == '\0') {
			json_err(json_error_eof);
			return false;
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
				json_err(json_error_eof);
				return false;
			}
			json_err(json_error_syntax);
			return false;
		} else if (ch == '0') {
			json_next;
			goto json_parse_number_point;
		} else if ((ch >= '1') && (ch <= '9')) {
			do {
				if (json_next == '\0') {
					json_err(json_error_eof);
					return false;
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
						json_err(json_error_eof);
						return false;
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				json_err(json_error_syntax);
				return false;
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
						json_err(json_error_eof);
						return false;
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				json_err(json_error_syntax);
				return false;
			}
		}
		
		node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data));
		if (node->size > 63 * sizeof(char)) {
			json_err(json_error_syntax);
			return false;
		}
		
		memcpy(ctx->buffer, node->data, node->size);
		ctx->buffer[node->size / sizeof(char)] = 0;
		if (is_float) {
			node->value.n.is_float = true;
			node->value.n.v.f = atof(ctx->buffer);
		} else {
			node->value.n.is_float = false;
			node->value.n.v.i = atoll(ctx->buffer);
		}
		
		json_rev;
		return true;
	}
	
	bool jsonw_parse_number(struct json_context* ctx, wchar_t** ptrptr, struct json_node* node) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		wchar_t ch = *(*ptrptr);
		bool minus = false;
		if (ch == L'-') {
			minus = true;
			jsonw_next;
		}
		if (ch == L'\0') {
			jsonw_err(json_error_eof);
			return false;
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
				jsonw_err(json_error_eof);
				return false;
			}
			jsonw_err(json_error_syntax);
			return false;
		} else if (ch == L'0') {
			jsonw_next;
			goto jsonw_parse_number_point;
		} else if ((ch >= L'1') && (ch <= L'9')) {
			do {
				if (jsonw_next == L'\0') {
					jsonw_err(json_error_eof);
					return false;
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
						jsonw_err(json_error_eof);
						return false;
					} else if ((ch < L'0') || (ch > L'9')) {
						break;
					}
				} while (true);
			} else {
				jsonw_err(json_error_syntax);
				return false;
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
						jsonw_err(json_error_eof);
						return false;
					} else if ((ch < L'0') || (ch > L'9')) {
						break;
					}
				} while (true);
			} else {
				jsonw_err(json_error_syntax);
				return false;
			}
		}
		
		node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata));
		if (node->size > 63 * sizeof(wchar_t)) {
			jsonw_err(json_error_syntax);
			return false;
		}
		
		memcpy(ctx->wbuffer, node->wdata, node->size);
		ctx->wbuffer[node->size / sizeof(wchar_t)] = 0;
		if (is_float) {
			node->value.n.is_float = true;
			node->value.n.v.f = wcstold(ctx->wbuffer, NULL);
		} else {
			node->value.n.is_float = false;
			node->value.n.v.i = wcstoll(ctx->wbuffer, NULL, 10);
		}
		
		jsonw_rev;
		return true;
	}
	
	bool json_parse_string(struct json_context* ctx, char** ptrptr, struct json_node* node) {
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
						json_err(json_error_eof);
						return false;
					} else {
						json_err(json_error_syntax);
						return false;
					}
				}
			} else if (ch == '\\') {
				json_next;
				if ((ch == '\"') || (ch == '\\') || (ch == '/') || (ch == 'b') || (ch == 'f') || (ch == 'n') || (ch == 'r') || (ch == 't')) {
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
							json_err(json_error_syntax);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == '\0') {
							json_err(json_error_eof);
							return false;
						} else {
							json_err(json_error_syntax);
							return false;
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
										json_err(json_error_syntax);
										return false;
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == '\0') {
										json_err(json_error_eof);
										return false;
									} else {
										json_err(json_error_syntax);
										return false;
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									json_err(json_error_syntax);
									return false;
								}
							} else if (ch == '\0') {
								json_err(json_error_eof);
								return false;
							} else {
								json_err(json_error_syntax);
								return false;
							}
						} else if (ch == '\0') {
							json_err(json_error_eof);
							return false;
						} else {
							json_err(json_error_syntax);
							return false;
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
					json_err(json_error_eof);
					return false;
				} else {
					json_err(json_error_syntax);
					return false;
				}
			} else {
				len++;
			}
		}
		json_err(json_error_eof);
		return false;
		
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
						json_err(json_error_eof);
						return false;
					} else {
						json_err(json_error_syntax);
						return false;
					}
				}
			} else if (ch == '\\') {
				json_next;
				if ((ch == '\"') || (ch == '\\') || (ch == '/') || (ch == 'b') || (ch == 'f') || (ch == 'n') || (ch == 'r') || (ch == 't')) {
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
							json_err(json_error_syntax);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == '\0') {
							json_err(json_error_eof);
							return false;
						} else {
							json_err(json_error_syntax);
							return false;
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
										json_err(json_error_syntax);
										return false;
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == '\0') {
										json_err(json_error_eof);
										return false;
									} else {
										json_err(json_error_syntax);
										return false;
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									json_err(json_error_syntax);
									return false;
								}
							} else if (ch == '\0') {
								json_err(json_error_eof);
								return false;
							} else {
								json_err(json_error_syntax);
								return false;
							}
						} else if (ch == '\0') {
							json_err(json_error_eof);
							return false;
						} else {
							json_err(json_error_syntax);
							return false;
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
					json_err(json_error_eof);
					return false;
				} else {
					json_err(json_error_syntax);
					return false;
				}
			} else {
				*ptr = ch; ptr++;
			}
		}
		json_err(json_error_eof);
		return false;
	}
	
	bool jsonw_parse_string(struct json_context* ctx, wchar_t** ptrptr, struct json_node* node) {
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
					json_err(json_error_eof);
					return false;
				}
				if ((uint16_t)(ch & 0xFC00) != 0xDC00) {
					return false; // Not UTF16
				}
				len += 2;
#endif
			} else if (ch == L'\\') {
				jsonw_next;
				if ((ch == L'\"') || (ch == L'\\') || (ch == L'/') || (ch == L'b') || (ch == L'f') || (ch == L'n') || (ch == L'r') || (ch == L't')) {
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
							jsonw_err(json_error_syntax);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == L'\0') {
							jsonw_err(json_error_eof);
							return false;
						} else {
							jsonw_err(json_error_syntax);
							return false;
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
										jsonw_err(json_error_syntax);
										return false;
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == L'\0') {
										jsonw_err(json_error_eof);
										return false;
									} else {
										jsonw_err(json_error_syntax);
										return false;
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									jsonw_err(json_error_syntax);
									return false;
								}
							} else if (ch == L'\0') {
								jsonw_err(json_error_eof);
								return false;
							} else {
								jsonw_err(json_error_syntax);
								return false;
							}
						} else if (ch == L'\0') {
							jsonw_err(json_error_eof);
							return false;
						} else {
							jsonw_err(json_error_syntax);
							return false;
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
					jsonw_err(json_error_eof);
					return false;
				} else {
					jsonw_err(json_error_syntax);
					return false;
				}
			} else {
				len++;
			}
		}
		jsonw_err(json_error_eof);
		return false;
		
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
					json_err(json_error_eof);
					return false;
				}
				if ((uint16_t)(ch & 0xFC00) != 0xDC00) {
					return false; // Not UTF16
				}
				*ptr = ch; ptr++;
#endif
			} else if (ch == L'\\') {
				jsonw_next;
				if ((ch == L'\"') || (ch == L'\\') || (ch == L'/') || (ch == L'b') || (ch == L'f') || (ch == L'n') || (ch == L'r') || (ch == L't')) {
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
							jsonw_err(json_error_syntax);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == L'\0') {
							jsonw_err(json_error_eof);
							return false;
						} else {
							jsonw_err(json_error_syntax);
							return false;
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
										jsonw_err(json_error_syntax);
										return false;
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == L'\0') {
										jsonw_err(json_error_eof);
										return false;
									} else {
										jsonw_err(json_error_syntax);
										return false;
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									jsonw_err(json_error_syntax);
									return false;
								}
							} else if (ch == L'\0') {
								jsonw_err(json_error_eof);
								return false;
							} else {
								jsonw_err(json_error_syntax);
								return false;
							}
						} else if (ch == L'\0') {
							jsonw_err(json_error_eof);
							return false;
						} else {
							jsonw_err(json_error_syntax);
							return false;
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
					jsonw_err(json_error_eof);
					return false;
				} else {
					jsonw_err(json_error_syntax);
					return false;
				}
			} else {
				*ptr = ch; ptr++;
			}
		}
		jsonw_err(json_error_eof);
		return false;
	}
	
	bool json_parse_object(struct json_context* ctx, char** ptrptr, struct json_node* node, int32_t level);
	bool json_parse_array(struct json_context* ctx, char** ptrptr, struct json_node* node, int32_t level);
	
	struct json_node* json_parse_node(struct json_context* ctx, char** ptrptr, struct json_node* parent, int32_t level) {
		char ch;
		struct json_node* node = NULL;
		while (json_next != '\0') {
			if (ch == '\"') {
				node = json_create(parent, json_type_string, level, *ptrptr, 0);
				if (node == NULL) {
					json_err(json_error_oom);
					return NULL;
				}
				if (!json_parse_string(ctx, ptrptr, node)) {
					json_destroy(node);
					return NULL;
				}
				return node;
			} else if ((ch == '-') || ((ch >= '0') && (ch <= '9'))) {
				node = json_create(parent, json_type_number, level, *ptrptr, 0);
				if (node == NULL) {
					json_err(json_error_oom);
					return NULL;
				}
				if (!json_parse_number(ctx, ptrptr, node)) {
					json_destroy(node);
					return NULL;
				}
				return node;
			} else if (ch == '{') {
				node = json_create(parent, json_type_object, level, *ptrptr, 0);
				if (node == NULL) {
					json_err(json_error_oom);
					return NULL;
				}
				if (!json_parse_object(ctx, ptrptr, node, level)) {
					json_destroy(node);
					return NULL;
				}
				return node;
			} else if (ch == '[') {
				node = json_create(parent, json_type_array, level, *ptrptr, 0);
				if (node == NULL) {
					json_err(json_error_oom);
					return NULL;
				}
				if (!json_parse_array(ctx, ptrptr, node, level)) {
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
								json_err(json_error_oom);
								return NULL;
							}
							node->value.b = true;
							return node;
						}
					}
				}
				if (ch == '\0') {
					json_err(json_error_eof);
					return NULL;
				}
				json_err(json_error_syntax);
				return NULL;
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
									json_err(json_error_oom);
									return NULL;
								}
								node->value.b = false;
								return node;
							}
						}
					}
				}
				if (ch == '\0') {
					json_err(json_error_eof);
					return NULL;
				}
				json_err(json_error_syntax);
				return NULL;
			} else if ((ch == 'n') || (ch == 'N')) {
				json_next;
				if ((ch == 'u') || (ch == 'U')) {
					json_next;
					if ((ch == 'l') || (ch == 'L')) {
						json_next;
						if ((ch == 'l') || (ch == 'L')) {
							node = json_create(parent, json_type_null, level, *ptrptr - 3, 4);
							if (node == NULL) {
								json_err(json_error_oom);
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
							json_err(json_error_oom);
							return NULL;
						}
						node->value.n.is_float = true;
						node->value.n.v.f = NAN;
						return node;
					}
				}
				if (ch == '\0') {
					json_err(json_error_eof);
					return NULL;
				}
				json_err(json_error_syntax);
				return NULL;
			} else if ((ch == 'i') || (ch == 'I')) {
				json_next;
				if ((ch == 'n') || (ch == 'N')) {
					json_next;
					if ((ch == 'f') || (ch == 'F')) {
						node = json_create(parent, json_type_number, level, *ptrptr - 2, 3);
						if (node == NULL) {
							json_err(json_error_oom);
							return NULL;
						}
						node->value.n.is_float = true;
						node->value.n.v.f = INFINITY;
						return node;
					}
				}
				if (ch == '\0') {
					json_err(json_error_eof);
					return NULL;
				}
				json_err(json_error_syntax);
				return NULL;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return NULL;
				}
			} else if (!json_is_space(ch)) {
				json_err(json_error_syntax);
				return NULL;
			}
		}
		json_err(json_error_eof);
		return NULL;
	}
	
	bool jsonw_parse_object(struct json_context* ctx, wchar_t** ptrptr, struct json_node* node, int32_t level);
	bool jsonw_parse_array(struct json_context* ctx, wchar_t** ptrptr, struct json_node* node, int32_t level);
	
	struct json_node* jsonw_parse_node(struct json_context* ctx, wchar_t** ptrptr, struct json_node* parent, int32_t level) {
		wchar_t ch;
		struct json_node* node = NULL;
		while (json_next != L'\0') {
			if (ch == L'\"') {
				node = jsonw_create(parent, json_type_string, level, *ptrptr, 0);
				if (node == NULL) {
					jsonw_err(json_error_oom);
					return NULL;
				}
				if (!jsonw_parse_string(ctx, ptrptr, node)) {
					jsonw_destroy(node);
					return NULL;
				}
				return node;
			} else if ((ch == L'-') || ((ch >= L'0') && (ch <= L'9'))) {
				node = jsonw_create(parent, json_type_number, level, *ptrptr, 0);
				if (node == NULL) {
					jsonw_err(json_error_oom);
					return NULL;
				}
				if (!jsonw_parse_number(ctx, ptrptr, node)) {
					jsonw_destroy(node);
					return NULL;
				}
				return node;
			} else if (ch == L'{') {
				node = jsonw_create(parent, json_type_object, level, *ptrptr, 0);
				if (node == NULL) {
					jsonw_err(json_error_oom);
					return NULL;
				}
				if (!jsonw_parse_object(ctx, ptrptr, node, level)) {
					jsonw_destroy(node);
					return NULL;
				}
				return node;
			} else if (ch == L'[') {
				node = jsonw_create(parent, json_type_array, level, *ptrptr, 0);
				if (node == NULL) {
					jsonw_err(json_error_oom);
					return NULL;
				}
				if (!jsonw_parse_array(ctx, ptrptr, node, level)) {
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
								jsonw_err(json_error_oom);
								return NULL;
							}
							node->value.b = true;
							return node;
						}
					}
				}
				if (ch == L'\0') {
					jsonw_err(json_error_eof);
					return NULL;
				}
				jsonw_err(json_error_syntax);
				return NULL;
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
									jsonw_err(json_error_oom);
									return NULL;
								}
								node->value.b = false;
								return node;
							}
						}
					}
				}
				if (ch == L'\0') {
					jsonw_err(json_error_eof);
					return NULL;
				}
				jsonw_err(json_error_syntax);
				return NULL;
			} else if ((ch == L'n') || (ch == L'N')) {
				jsonw_next;
				if ((ch == L'u') || (ch == L'U')) {
					jsonw_next;
					if ((ch == L'l') || (ch == L'L')) {
						jsonw_next;
						if ((ch == L'l') || (ch == L'L')) {
							node = jsonw_create(parent, json_type_null, level, *ptrptr - 3, 4 * sizeof(wchar_t));
							if (node == NULL) {
								jsonw_err(json_error_oom);
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
							jsonw_err(json_error_oom);
							return NULL;
						}
						node->value.n.is_float = true;
						node->value.n.v.f = NAN;
						return node;
					}
				}
				if (ch == L'\0') {
					jsonw_err(json_error_eof);
					return NULL;
				}
				jsonw_err(json_error_syntax);
				return NULL;
			} else if ((ch == L'i') || (ch == L'I')) {
				jsonw_next;
				if ((ch == L'n') || (ch == L'N')) {
					jsonw_next;
					if ((ch == L'f') || (ch == L'F')) {
						node = jsonw_create(parent, json_type_number, level, *ptrptr - 2, 3 * sizeof(wchar_t));
						if (node == NULL) {
							jsonw_err(json_error_oom);
							return NULL;
						}
						node->value.n.is_float = true;
						node->value.n.v.f = INFINITY;
						return node;
					}
				}
				if (ch == L'\0') {
					jsonw_err(json_error_eof);
					return NULL;
				}
				jsonw_err(json_error_syntax);
				return NULL;
			} else if (ch == '/') {
				if (!jsonw_skip_comment(ctx, ptrptr)) {
					return NULL;
				}
			} else if (!jsonw_is_space(ch)) {
				jsonw_err(json_error_syntax);
				return NULL;
			}
		}
		jsonw_err(json_error_eof);
		return NULL;
	}
	
	bool json_parse_array(struct json_context* ctx, char** ptrptr, struct json_node* node, int32_t level) {
		char ch;
		level++;
	json_parse_array_repeat:
		while (json_next != '\0') {
			if (ch == ']') {
				node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data)) + sizeof(char);
				return true;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (!json_is_space(ch)) {
				json_rev;
				struct json_node* value = json_parse_node(ctx, ptrptr, node, level);
				if (value == NULL) {
					json_err(json_error_oom);
					return false;
				}
				if (!json_array_add(node, value)) {
					json_destroy(value);
					return false;
				}
				while (json_next != '\0') {
					if (ch == ',') {
						goto json_parse_array_repeat;
					} else if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							return false;
						}
					} else if (ch == ']') {
						node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data)) + sizeof(char);
						return true;
					} else if (!json_is_space(ch)) {
						json_err(json_error_syntax);
						return false;
					}
				}
				json_err(json_error_eof);
				return false;
			}
		}
		json_err(json_error_eof);
		return false;
	}
	
	bool jsonw_parse_array(struct json_context* ctx, wchar_t** ptrptr, struct json_node* node, int32_t level) {
		wchar_t ch;
		level++;
	jsonw_parse_array_repeat:
		while (jsonw_next != L'\0') {
			if (ch == L']') {
				node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata)) + sizeof(wchar_t);
				return true;
			} else if (ch == '/') {
				if (!jsonw_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (!jsonw_is_space(ch)) {
				jsonw_rev;
				struct json_node* value = jsonw_parse_node(ctx, ptrptr, node, level);
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
					} else if (ch == '/') {
						if (!jsonw_skip_comment(ctx, ptrptr)) {
							return false;
						}
					} else if (ch == L']') {
						node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata)) + sizeof(wchar_t);
						return true;
					} else if (!jsonw_is_space(ch)) {
						jsonw_err(json_error_syntax);
						return false;
					}
				}
				jsonw_err(json_error_eof);
				return false;
			}
		}
		jsonw_err(json_error_eof);
		return false;
	}
	
	bool json_parse_object(struct json_context* ctx, char** ptrptr, struct json_node* node, int32_t level) {
		char ch;
		level++;
	json_parse_object_repeat:
		while (json_next != '\0') {
			if (ch == '}') {
				node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data)) + sizeof(char);
				return true;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (ch == '\"') {
				struct json_node* vkey = json_create(node, json_type_string, level, *ptrptr, 0);
				if (vkey == NULL) {
					json_err(json_error_oom);
					return false;
				}
				if (!json_parse_string(ctx, ptrptr, vkey)) {
					json_destroy(vkey);
					return false;
				}
				while (json_next != '\0') {
					if (ch == ':') {
						struct json_node* value = json_parse_node(ctx, ptrptr, node, level);
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
							} else if (ch == '/') {
								if (!json_skip_comment(ctx, ptrptr)) {
									json_destroy(vkey);
									return false;
								}
							} else if (ch == '}') {
								node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data)) + sizeof(char);
								return true;
							} else if (!json_is_space(ch)) {
								json_err(json_error_syntax);
								json_destroy(vkey);
								return false;
							}
						}
						json_err(json_error_eof);
						json_destroy(vkey);
						return false;
					} else if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							json_destroy(vkey);
							return false;
						}
					} else if (!json_is_space(ch)) {
						json_err(json_error_syntax);
						json_destroy(vkey);
						return false;
					}
				}
				json_err(json_error_eof);
				json_destroy(vkey);
				return false;
			} else if (!json_is_space(ch)) {
				json_err(json_error_syntax);
				return false;
			}
		}
		json_err(json_error_eof);
		return false;
	}
	
	bool jsonw_parse_object(struct json_context* ctx, wchar_t** ptrptr, struct json_node* node, int32_t level) {
		wchar_t ch;
		level++;
	jsonw_parse_object_repeat:
		while (jsonw_next != L'\0') {
			if (ch == L'}') {
				node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata)) + sizeof(wchar_t);
				return true;
			} else if (ch == '/') {
				if (!jsonw_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (ch == L'\"') {
				struct json_node* vkey = jsonw_create(node, json_type_string, level, *ptrptr, 0);
				if (vkey == NULL) {
					jsonw_err(json_error_oom);
					return false;
				}
				if (!jsonw_parse_string(ctx, ptrptr, vkey)) {
					jsonw_destroy(vkey);
					return false;
				}
				while (jsonw_next != L'\0') {
					if (ch == L':') {
						struct json_node* value = jsonw_parse_node(ctx, ptrptr, node, level);
						if (value == NULL) {
							jsonw_destroy(vkey);
							return false;
						}
						if (!jsonw_object_add(node, vkey, value)) {
							jsonw_destroy(vkey);
							jsonw_destroy(value);
							return false;
						}
						while (jsonw_next != L'\0') {
							if (ch == L',') {
								goto jsonw_parse_object_repeat;
							} else if (ch == L'/') {
								if (!jsonw_skip_comment(ctx, ptrptr)) {
									jsonw_destroy(vkey);
									return false;
								}
							} else if (ch == L'}') {
								node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata)) + sizeof(wchar_t);
								return true;
							} else if (!jsonw_is_space(ch)) {
								jsonw_err(json_error_syntax);
								jsonw_destroy(vkey);
								return false;
							}
						}
						jsonw_err(json_error_eof);
						jsonw_destroy(vkey);
						return false;
					} else if (ch == '/') {
						if (!jsonw_skip_comment(ctx, ptrptr)) {
							jsonw_destroy(vkey);
							return false;
						}
					} else if (!jsonw_is_space(ch)) {
						jsonw_err(json_error_syntax);
						jsonw_destroy(vkey);
						return false;
					}
				}
				jsonw_err(json_error_eof);
				json_destroy(vkey);
				return false;
			} else if (!jsonw_is_space(ch)) {
				jsonw_err(json_error_syntax);
				return false;
			}
		}
		jsonw_err(json_error_eof);
		return false;
	}
	
	struct json_node* json_parse(struct json_context* ctx, const char* json) {
		ctx->seterror(json_error_none, NULL);
		char* ptr = (char*)json;
		char** ptrptr = &ptr;
		char ch;
		while ((ch = *ptr) != '\0') {
			if (json_is_space(ch)) {
				ptr++;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return NULL;
				}
			} else if (ch == '{') {
				json_node* node = json_create(0, json_type_object, 0, ptr, 0);
				if (node == NULL) {
					json_err(json_error_oom);
					return NULL;
				}
				if (!json_parse_object(ctx, ptrptr, node, 0)) {
					json_destroy(node);
					return NULL;
				}
				while (json_next != '\0') {
					if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							json_destroy(node);
							return NULL;
						}
					} else if (!json_is_space(ch)) {
						json_destroy(node);
						json_err(json_error_syntax);
						return NULL;
					}
				}
				return node;
			} else {
				json_err(json_error_syntax);
				return NULL;
			}
		}
		json_err(json_error_eof);
		return NULL;
	}
	
	struct json_node* jsonw_parse(struct json_context* ctx, const wchar_t* json) {
		ctx->wseterror(json_error_none, NULL);
		wchar_t* ptr = (wchar_t*)json;
		wchar_t** ptrptr = &ptr;
		wchar_t ch;
		while ((ch = *ptr) != L'\0') {
			if (jsonw_is_space(ch)) {
				ptr++;
			} else if (ch == L'/') {
				if (!jsonw_skip_comment(ctx, ptrptr)) {
					return NULL;
				}
			} else if (ch == L'{') {
				json_node* node = jsonw_create(0, json_type_object, 0, ptr, 0);
				if (node == NULL) {
					jsonw_err(json_error_oom);
					return NULL;
				}
				if (!jsonw_parse_object(ctx, ptrptr, node, 0)) {
					jsonw_destroy(node);
					return NULL;
				}
				while (jsonw_next != L'\0') {
					if (ch == L'/') {
						if (!jsonw_skip_comment(ctx, ptrptr)) {
							jsonw_destroy(node);
							return NULL;
						}
					} else if (!jsonw_is_space(ch)) {
						jsonw_destroy(node);
						jsonw_err(json_error_syntax);
						return NULL;
					}
				}
				return node;
			} else {
				jsonw_err(json_error_syntax);
				return NULL;
			}
		}
		jsonw_err(json_error_eof);
		return NULL;
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
		char* e = (char*)(ctx.error.ptr);
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
		bool prevspace = false;
		while (len > 0) {
			char ch = *e;
			if (ch == '\0')
				break;
			if (json_is_space(ch)) {
				if (!prevspace) {
					if (e == ctx.error.ptr) *p1 = 'v'; else *p1 = ' ';
					*p2 = ' ';
					prevspace = true;
					p1++; p2++;
					len--;
				}
			} else {
				if (e == ctx.error.ptr) *p1 = 'v'; else *p1 = ' ';
				*p2 = *e;
				prevspace = false;
				p1++; p2++;
				len--;
			}
			e++;
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
		
		switch (ctx.error.type) {
			case json_error_none:
				LOG("JSON Parse: No Error!");
				break;
			case json_error_oom:
				LOG("JSON Parse: Out Of Memory!");
				break;
			case json_error_eof:
				LOG("JSON Parse: Unexpected End!");
				break;
			case json_error_syntax:
				LOG("JSON Parse: Syntax Error!");
				break;
		}
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
		int errofs = (int)((intptr_t)(ctx.error.wptr) - (intptr_t)(json)) / sizeof(wchar_t);
		int prefix = 4;
		int start = errofs - 8; if (start < 0) { start = 0; prefix = 0; }
		int postfix = 4;
		int end = errofs + 8; if (end > size) { end = size; postfix = 0; }
		int len = (end - start);
		wchar_t* line1 = (wchar_t*)mmalloc((len + prefix + postfix + 1)*sizeof(wchar_t));
		wchar_t* line2 = (wchar_t*)mmalloc((len + prefix + postfix + 1)*sizeof(wchar_t));
		wchar_t* p1 = line1;
		wchar_t* p2 = line2;
		wchar_t* e = (wchar_t*)(ctx.error.wptr);
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
		
		switch (ctx.error.type) {
			case json_error_none:
				LOG("JSON Parse: No Error!");
				break;
			case json_error_oom:
				LOG("JSON Parse: Out Of Memory!");
				break;
			case json_error_eof:
				LOG("JSON Parse: Unexpected End!");
				break;
			case json_error_syntax:
				LOG("JSON Parse: Syntax Error!");
				break;
		}
		jsonw_log(line1);
		jsonw_log(line2);
		
		mmfree(line1);
		mmfree(line2);
	}
	
#endif
	
	//////
	
	bool json_call_number(struct json_context* ctx, char** ptrptr, struct json_number* number) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		char* nodedata = *ptrptr;
		char ch = *(*ptrptr);
		bool minus = false;
		if (ch == '-') {
			minus = true;
			json_next;
		}
		if (ch == '\0') {
			json_err(json_error_eof);
			return false;
		} else if ((ch == 'i') || (ch == 'I')) {
			json_next;
			if ((ch == 'n') || (ch == 'N')) {
				json_next;
				if ((ch == 'f') || (ch == 'F')) {
					number->is_float = true;
					number->v.f = (minus ? -INFINITY : INFINITY);
					return true;
				}
			}
			if (ch == '\0') {
				json_err(json_error_eof);
				return false;
			}
			json_err(json_error_syntax);
			return false;
		} else if (ch == '0') {
			json_next;
			goto json_call_number_point;
		} else if ((ch >= '1') && (ch <= '9')) {
			do {
				if (json_next == '\0') {
					json_err(json_error_eof);
					return false;
				} else if ((ch < '0') || (ch > '9')) {
					break;
				}
			} while (true);
		}
	json_call_number_point:
		bool is_float = false;
		if (ch == '.') {
			is_float = true;
			json_next;
			if ((ch >= '0') && (ch <= '9')) {
				do {
					if (json_next == '\0') {
						json_err(json_error_eof);
						return false;
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				json_err(json_error_syntax);
				return false;
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
						json_err(json_error_eof);
						return false;
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				json_err(json_error_syntax);
				return false;
			}
		}
		
		int nodesize = (int)((intptr_t)(*ptrptr) - (intptr_t)(nodedata));
		if (nodesize > 63 * sizeof(char)) {
			json_err(json_error_syntax);
			return false;
		}
		
		memcpy(ctx->buffer, nodedata, nodesize);
		ctx->buffer[nodesize / sizeof(char)] = 0;
		if (is_float) {
			number->is_float = true;
			number->v.f = atof(ctx->buffer);
		} else {
			number->is_float = false;
			number->v.i = atoll(ctx->buffer);
		}
		
		json_rev;
		return true;
	}
	
	bool json_call_string(struct json_context* ctx, char** ptrptr, char** string) {
		char* nodedata = *ptrptr;
		int nodesize = 0;
		char ch;
		uint32_t len = 0;
		char* save = *ptrptr;
		while (json_next != '\0') {
			if (ch == '\"') {
				nodesize = (int)((intptr_t)(*ptrptr) - (intptr_t)(nodedata)) + sizeof(char);
				*ptrptr = save;
				goto json_call_string_alloc;
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
						json_err(json_error_eof);
						return false;
					} else {
						json_err(json_error_syntax);
						return false;
					}
				}
			} else if (ch == '\\') {
				json_next;
				if ((ch == '\"') || (ch == '\\') || (ch == '/') || (ch == 'b') || (ch == 'f') || (ch == 'n') || (ch == 'r') || (ch == 't')) {
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
							json_err(json_error_syntax);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == '\0') {
							json_err(json_error_eof);
							return false;
						} else {
							json_err(json_error_syntax);
							return false;
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
										json_err(json_error_syntax);
										return false;
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == '\0') {
										json_err(json_error_eof);
										return false;
									} else {
										json_err(json_error_syntax);
										return false;
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									json_err(json_error_syntax);
									return false;
								}
							} else if (ch == '\0') {
								json_err(json_error_eof);
								return false;
							} else {
								json_err(json_error_syntax);
								return false;
							}
						} else if (ch == '\0') {
							json_err(json_error_eof);
							return false;
						} else {
							json_err(json_error_syntax);
							return false;
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
					json_err(json_error_eof);
					return false;
				} else {
					json_err(json_error_syntax);
					return false;
				}
			} else {
				len++;
			}
		}
		json_err(json_error_eof);
		return false;
		
	json_call_string_alloc:
		char* ptr = memAlloc(char, ptr, (len + 1) * sizeof(char));
		if (ptr == NULL) {
			return false;
		}
		*string = ptr;
		
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
						json_err(json_error_eof);
						return false;
					} else {
						json_err(json_error_syntax);
						return false;
					}
				}
			} else if (ch == '\\') {
				json_next;
				if ((ch == '\"') || (ch == '\\') || (ch == '/') || (ch == 'b') || (ch == 'f') || (ch == 'n') || (ch == 'r') || (ch == 't')) {
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
							json_err(json_error_syntax);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == '\0') {
							json_err(json_error_eof);
							return false;
						} else {
							json_err(json_error_syntax);
							return false;
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
										json_err(json_error_syntax);
										return false;
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == '\0') {
										json_err(json_error_eof);
										return false;
									} else {
										json_err(json_error_syntax);
										return false;
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									json_err(json_error_syntax);
									return false;
								}
							} else if (ch == '\0') {
								json_err(json_error_eof);
								return false;
							} else {
								json_err(json_error_syntax);
								return false;
							}
						} else if (ch == '\0') {
							json_err(json_error_eof);
							return false;
						} else {
							json_err(json_error_syntax);
							return false;
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
					json_err(json_error_eof);
					return false;
				} else {
					json_err(json_error_syntax);
					return false;
				}
			} else {
				*ptr = ch; ptr++;
			}
		}
		json_err(json_error_eof);
		return false;
	}

	bool json_call_object(struct json_context* ctx, char** ptrptr);
	bool json_call_array(struct json_context* ctx, char** ptrptr);
	
	bool json_call_array_node(struct json_context* ctx, char** ptrptr, int index) {
		char ch;
		struct json_node* node = NULL;
		while (json_next != '\0') {
			if (ch == '\"') {
				if (ctx->callbacks->onarray.onstring != NULL) {
					char* value = NULL;
					if (!json_call_string(ctx, ptrptr, &value)) {
						return false;
					}
					ctx->callbacks->onarray.onstring(ctx, index, value, ctx->callbacks->target);
					memFree(value);
				} else {
					if (!json_check_string(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if ((ch == '-') || ((ch >= '0') && (ch <= '9'))) {
				if (ctx->callbacks->onarray.onnumber != NULL) {
					struct json_number number;
					if (!json_call_number(ctx, ptrptr, &number)) {
						return false;
					}
					ctx->callbacks->onarray.onnumber(ctx, index, number, ctx->callbacks->target);
				} else {
					if (!json_check_number(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if (ch == '{') {
				if (ctx->callbacks->onarray.onobjectstart != NULL) {
					struct json_callbacks store;
					json_store_callbacks(&store, ctx->callbacks);
					ctx->callbacks->onarray.onobjectstart(ctx, index, ctx->callbacks->target);
					bool result = json_call_object(ctx, ptrptr);
					if (ctx->callbacks->onarray.onobjectend != NULL) {
						ctx->callbacks->onarray.onobjectend(ctx, index, store.target, ctx->callbacks->target, result);
					}
					if (!result) {
						json_store_callbacks(ctx->callbacks, &store);
						return false;
					}
					json_store_callbacks(ctx->callbacks, &store);
				} else {
					if (!json_check_object(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if (ch == '[') {
				if (ctx->callbacks->onarray.onarraystart != NULL) {
					struct json_callbacks store;
					json_store_callbacks(&store, ctx->callbacks);
					ctx->callbacks->onarray.onarraystart(ctx, index, ctx->callbacks->target);
					bool result = json_call_array(ctx, ptrptr);
					if (ctx->callbacks->onarray.onarrayend != NULL) {
						ctx->callbacks->onarray.onarrayend(ctx, index, store.target, ctx->callbacks->target, result);
					}
					if (!result) {
						json_store_callbacks(&store, ctx->callbacks);
						return false;
					}
					json_store_callbacks(&store, ctx->callbacks);
				} else {
					if (!json_check_array(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if ((ch == 't') || (ch == 'T')) {
				json_next;
				if ((ch == 'r') || (ch == 'R')) {
					json_next;
					if ((ch == 'u') || (ch == 'U')) {
						json_next;
						if ((ch == 'e') || (ch == 'E')) {
							if (ctx->callbacks->onarray.onboolean != NULL) {
								ctx->callbacks->onarray.onboolean(ctx, index, true, ctx->callbacks->target);
							}
							return true;
						}
					}
				}
				if (ch == '\0') {
					json_err(json_error_eof);
					return false;
				}
				json_err(json_error_syntax);
				return false;
			} else if ((ch == 'f') || (ch == 'F')) {
				json_next;
				if ((ch == 'a') || (ch == 'A')) {
					json_next;
					if ((ch == 'l') || (ch == 'L')) {
						json_next;
						if ((ch == 's') || (ch == 'S')) {
							json_next;
							if ((ch == 'e') || (ch == 'E')) {
								if (ctx->callbacks->onarray.onboolean != NULL) {
									ctx->callbacks->onarray.onboolean(ctx, index, false, ctx->callbacks->target);
								}
								return true;
							}
						}
					}
				}
				if (ch == '\0') {
					json_err(json_error_eof);
					return false;
				}
				json_err(json_error_syntax);
				return NULL;
			} else if ((ch == 'n') || (ch == 'N')) {
				json_next;
				if ((ch == 'u') || (ch == 'U')) {
					json_next;
					if ((ch == 'l') || (ch == 'L')) {
						json_next;
						if ((ch == 'l') || (ch == 'L')) {
							if (ctx->callbacks->onarray.onnull != NULL) {
								ctx->callbacks->onarray.onnull(ctx, index, ctx->callbacks->target);
							}
							return true;
						}
					}
				} else if ((ch == 'a') || (ch == 'A')) {
					json_next;
					if ((ch == 'n') || (ch == 'N')) {
						if (ctx->callbacks->onarray.onnumber != NULL) {
							struct json_number number;
							number.is_float = true;
							number.v.f = NAN;
							ctx->callbacks->onarray.onnumber(ctx, index, number, ctx->callbacks->target);
						}
						return true;
					}
				}
				if (ch == '\0') {
					json_err(json_error_eof);
					return false;
				}
				json_err(json_error_syntax);
				return false;
			} else if ((ch == 'i') || (ch == 'I')) {
				json_next;
				if ((ch == 'n') || (ch == 'N')) {
					json_next;
					if ((ch == 'f') || (ch == 'F')) {
						if (ctx->callbacks->onarray.onnumber != NULL) {
							struct json_number number;
							number.is_float = true;
							number.v.f = INFINITY;
							ctx->callbacks->onarray.onnumber(ctx, index, number, ctx->callbacks->target);
						}
						return true;
					}
				}
				if (ch == '\0') {
					json_err(json_error_eof);
					return false;
				}
				json_err(json_error_syntax);
				return false;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (!json_is_space(ch)) {
				json_err(json_error_syntax);
				return false;
			}
		}
		json_err(json_error_eof);
		return false;
	}
	
	bool json_call_object_node(struct json_context* ctx, char** ptrptr, const char* key) {
		char ch;
		struct json_node* node = NULL;
		while (json_next != '\0') {
			if (ch == '\"') {
				if (ctx->callbacks->onobject.onstring != NULL) {
					char* value = NULL;
					if (!json_call_string(ctx, ptrptr, &value)) {
						return false;
					}
					ctx->callbacks->onobject.onstring(ctx, key, value, ctx->callbacks->target);
					memFree(value);
				} else {
					if (!json_check_string(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if ((ch == '-') || ((ch >= '0') && (ch <= '9'))) {
				if (ctx->callbacks->onobject.onnumber != NULL) {
					struct json_number number;
					if (!json_call_number(ctx, ptrptr, &number)) {
						return false;
					}
					ctx->callbacks->onobject.onnumber(ctx, key, number, ctx->callbacks->target);
				} else {
					if (!json_check_number(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if (ch == '{') {
				if (ctx->callbacks->onobject.onobjectstart != NULL) {
					struct json_callbacks store;
					json_store_callbacks(&store, ctx->callbacks);
					ctx->callbacks->onobject.onobjectstart(ctx, key, ctx->callbacks->target);
					bool result = json_call_object(ctx, ptrptr);
					if (ctx->callbacks->onobject.onobjectend != NULL) {
						ctx->callbacks->onobject.onobjectend(ctx, key, store.target, ctx->callbacks->target, result);
					}
					if (!result) {
						json_store_callbacks(ctx->callbacks, &store);
						return false;
					}
					json_store_callbacks(ctx->callbacks, &store);
				} else {
					if (!json_check_object(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if (ch == '[') {
				if (ctx->callbacks->onobject.onarraystart != NULL) {
					struct json_callbacks store;
					json_store_callbacks(&store, ctx->callbacks);
					ctx->callbacks->onobject.onarraystart(ctx, key, ctx->callbacks->target);
					bool result = json_call_array(ctx, ptrptr);
					if (ctx->callbacks->onobject.onarrayend != NULL) {
						ctx->callbacks->onobject.onarrayend(ctx, key, store.target, ctx->callbacks->target, result);
					}
					if (!result) {
						json_store_callbacks(ctx->callbacks, &store);
						return false;
					}
					json_store_callbacks(ctx->callbacks, &store);
				} else {
					if (!json_check_array(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if ((ch == 't') || (ch == 'T')) {
				json_next;
				if ((ch == 'r') || (ch == 'R')) {
					json_next;
					if ((ch == 'u') || (ch == 'U')) {
						json_next;
						if ((ch == 'e') || (ch == 'E')) {
							if (ctx->callbacks->onobject.onboolean != NULL) {
								ctx->callbacks->onobject.onboolean(ctx, key, true, ctx->callbacks->target);
							}
							return true;
						}
					}
				}
				if (ch == '\0') {
					json_err(json_error_eof);
					return false;
				}
				json_err(json_error_syntax);
				return false;
			} else if ((ch == 'f') || (ch == 'F')) {
				json_next;
				if ((ch == 'a') || (ch == 'A')) {
					json_next;
					if ((ch == 'l') || (ch == 'L')) {
						json_next;
						if ((ch == 's') || (ch == 'S')) {
							json_next;
							if ((ch == 'e') || (ch == 'E')) {
								if (ctx->callbacks->onobject.onboolean != NULL) {
									ctx->callbacks->onobject.onboolean(ctx, key, false, ctx->callbacks->target);
								}
								return true;
							}
						}
					}
				}
				if (ch == '\0') {
					json_err(json_error_eof);
					return false;
				}
				json_err(json_error_syntax);
				return NULL;
			} else if ((ch == 'n') || (ch == 'N')) {
				json_next;
				if ((ch == 'u') || (ch == 'U')) {
					json_next;
					if ((ch == 'l') || (ch == 'L')) {
						json_next;
						if ((ch == 'l') || (ch == 'L')) {
							if (ctx->callbacks->onobject.onnull != NULL) {
								ctx->callbacks->onobject.onnull(ctx, key, ctx->callbacks->target);
							}
							return true;
						}
					}
				} else if ((ch == 'a') || (ch == 'A')) {
					json_next;
					if ((ch == 'n') || (ch == 'N')) {
						if (ctx->callbacks->onobject.onnumber != NULL) {
							struct json_number number;
							number.is_float = true;
							number.v.f = NAN;
							ctx->callbacks->onobject.onnumber(ctx, key, number, ctx->callbacks->target);
						}
						return true;
					}
				}
				if (ch == '\0') {
					json_err(json_error_eof);
					return false;
				}
				json_err(json_error_syntax);
				return false;
			} else if ((ch == 'i') || (ch == 'I')) {
				json_next;
				if ((ch == 'n') || (ch == 'N')) {
					json_next;
					if ((ch == 'f') || (ch == 'F')) {
						if (ctx->callbacks->onobject.onnumber != NULL) {
							struct json_number number;
							number.is_float = true;
							number.v.f = INFINITY;
							ctx->callbacks->onobject.onnumber(ctx, key, number, ctx->callbacks->target);
						}
						return true;
					}
				}
				if (ch == '\0') {
					json_err(json_error_eof);
					return false;
				}
				json_err(json_error_syntax);
				return false;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (!json_is_space(ch)) {
				json_err(json_error_syntax);
				return false;
			}
		}
		json_err(json_error_eof);
		return false;
	}
	
	bool json_call_array(struct json_context* ctx, char** ptrptr) {
		char ch;
		int index = 0;
	json_call_array_repeat:
		while (json_next != '\0') {
			if (ch == ']') {
				return true;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (!json_is_space(ch)) {
				json_rev;
				if (!json_call_array_node(ctx, ptrptr, index)) {
					return false;
				}
				index++;
				while (json_next != '\0') {
					if (ch == ',') {
						goto json_call_array_repeat;
					} else if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							return false;
						}
					} else if (ch == ']') {
						return true;
					} else if (!json_is_space(ch)) {
						json_err(json_error_syntax);
						return false;
					}
				}
				json_err(json_error_eof);
				return false;
			}
		}
		json_err(json_error_eof);
		return false;
	}

	bool json_call_object(struct json_context* ctx, char** ptrptr) {
		char ch;
	json_call_object_repeat:
		while (json_next != '\0') {
			if (ch == '}') {
				return true;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (ch == '\"') {
				char* vkey = NULL;
				if (!json_call_string(ctx, ptrptr, &vkey)) {
					return false;
				}
				while (json_next != '\0') {
					if (ch == ':') {
						if (!json_call_object_node(ctx, ptrptr, vkey)) {
							memFree(vkey);
							return false;
						}
						memFree(vkey);
						while (json_next != '\0') {
							if (ch == ',') {
								goto json_call_object_repeat;
							} else if (ch == '/') {
								if (!json_skip_comment(ctx, ptrptr)) {
									return false;
								}
							} else if (ch == '}') {
								return true;
							} else if (!json_is_space(ch)) {
								json_err(json_error_syntax);
								return false;
							}
						}
						json_err(json_error_eof);
						return false;
					} else if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							memFree(vkey);
							return false;
						}
					} else if (!json_is_space(ch)) {
						memFree(vkey);
						json_err(json_error_syntax);
						return false;
					}
				}
				memFree(vkey);
				json_err(json_error_eof);
				return false;
			} else if (!json_is_space(ch)) {
				json_err(json_error_syntax);
				return false;
			}
		}
		json_err(json_error_eof);
		return false;
	}

	bool json_call(struct json_context* ctx, const char* json) {
		if (ctx->callbacks == NULL)
			return false;
		
		ctx->seterror(json_error_none, NULL);
		char* ptr = (char*)json;
		char** ptrptr = &ptr;
		char ch;
		while ((ch = *ptr) != '\0') {
			if (json_is_space(ch)) {
				ptr++;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
				ptr++;
			} else if (ch == '{') {
				if (ctx->callbacks != NULL) {
					if (ctx->callbacks->onroot != NULL) {
						ctx->callbacks->onroot(ctx, ctx->callbacks->target);
					}
				}
				if (!json_call_object(ctx, ptrptr)) {
					return false;
				}
				while (json_next != '\0') {
					if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							return false;
						}
					} else if (!json_is_space(ch)) {
						json_err(json_error_syntax);
						return false;
					}
				}
				return true;
			} else {
				json_err(json_error_syntax);
				return false;
			}
		}
		json_err(json_error_eof);
		return false;
	}
	

#ifdef __cplusplus
}
#endif

String JSON::encode(const String& value) throw(const char*) {
	// TODO: encode JSON string
	return value;
}
