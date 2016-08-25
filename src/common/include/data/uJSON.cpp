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

	bool json_check_number(struct JsonContext* ctx, char** ptrptr, int32_t level) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		char ch = json_cur;
		if (ch == '-') {
			json_next;
		}
		if (ch == '\0') {
			json_err(JSON_ERROR_UNEXPECTEDEND);
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
				json_err(JSON_ERROR_UNEXPECTEDEND);
				return false;
			}
			json_err(JSON_ERROR_SYNTAX);
			return false;
		} else if (ch == '0') {
			json_next;
			goto json_check_number_point;
		} else if ((ch >= '1') && (ch <= '9')) {
			do {
				if (json_next == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
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
						json_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				json_err(JSON_ERROR_SYNTAX);
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
						json_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				json_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		json_rev;
		return true;
	}

	bool jsonw_check_number(struct JsonContext* ctx, wchar_t** ptrptr, int32_t level) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		wchar_t ch = jsonw_cur;
		if (ch == L'-') {
			jsonw_next;
		}
		if (ch == L'\0') {
			jsonw_err(JSON_ERROR_UNEXPECTEDEND);
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
				jsonw_err(JSON_ERROR_UNEXPECTEDEND);
				return false;
			}
			jsonw_err(JSON_ERROR_SYNTAX);
			return false;
		} else if (ch == L'0') {
			jsonw_next;
			goto jsonw_check_number_point;
		} else if ((ch >= L'1') && (ch <= L'9')) {
			do {
				if (jsonw_next == L'\0') {
					jsonw_err(JSON_ERROR_UNEXPECTEDEND);
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
						jsonw_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else if ((ch < L'0') || (ch > L'9')) {
						break;
					}
				} while (true);
			} else {
				jsonw_err(JSON_ERROR_SYNTAX);
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
						jsonw_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else if ((ch < L'0') || (ch > L'9')) {
						break;
					}
				} while (true);
			} else {
				jsonw_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		jsonw_rev;
		return true;
	}
	
	bool json_check_string(struct JsonContext* ctx, char** ptrptr, int32_t level) {
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
						json_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else {
						json_err(JSON_ERROR_SYNTAX);
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
							json_err(JSON_ERROR_SYNTAX);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == '\0') {
							json_err(JSON_ERROR_UNEXPECTEDEND);
							return false;
						} else {
							json_err(JSON_ERROR_SYNTAX);
							return false;
						}
					}
				} else if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				} else {
					json_err(JSON_ERROR_SYNTAX);
					return false;
				}
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}

	bool jsonw_check_string(struct JsonContext* ctx, wchar_t** ptrptr, int32_t level) {
		wchar_t ch;
		while (jsonw_next != L'\0') {
			if (ch == L'\"') {
				return true;
#if __WCHAR_MAX__ <= 0x10000
			} else if ((uint16_t)(ch & 0xFC00) == 0xD800) { // found UTF16
				if (jsonw_next == L'\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
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
							jsonw_err(JSON_ERROR_SYNTAX);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == L'\0') {
							jsonw_err(JSON_ERROR_UNEXPECTEDEND);
							return false;
						} else {
							jsonw_err(JSON_ERROR_SYNTAX);
							return false;
						}
					}
				} else if (ch == L'\0') {
					jsonw_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				} else {
					jsonw_err(JSON_ERROR_SYNTAX);
					return false;
				}
			}
		}
		jsonw_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	
	bool json_skip_comment(struct JsonContext* ctx, char** ptrptr) {
		char ch;
		if (json_next == '\0') {
			json_err(JSON_ERROR_UNEXPECTEDEND);
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
		json_err(JSON_ERROR_SYNTAX);
		return false;
	}
	
	bool jsonw_skip_comment(struct JsonContext* ctx, wchar_t** ptrptr) {
		wchar_t ch;
		if (jsonw_next == L'\0') {
			jsonw_err(JSON_ERROR_UNEXPECTEDEND);
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
		jsonw_err(JSON_ERROR_SYNTAX);
		return false;
	}
	
	bool json_check_object(struct JsonContext* ctx, char** ptrptr, int32_t level);
	bool json_check_array(struct JsonContext* ctx, char** ptrptr, int32_t level);
	
	bool json_check_value(struct JsonContext* ctx, char** ptrptr, int32_t level) {
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
				json_err(JSON_ERROR_SYNTAX);
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
				json_err(JSON_ERROR_SYNTAX);
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
				json_err(JSON_ERROR_SYNTAX);
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
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				}
				json_err(JSON_ERROR_SYNTAX);
				return false;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (!json_is_space(ch)) {
				json_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}

	bool jsonw_check_object(struct JsonContext* ctx, wchar_t** ptrptr, int32_t level);
	bool jsonw_check_array(struct JsonContext* ctx, wchar_t** ptrptr, int32_t level);
	
	bool jsonw_check_value(struct JsonContext* ctx, wchar_t** ptrptr, int32_t level) {
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
				jsonw_err(JSON_ERROR_SYNTAX);
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
				jsonw_err(JSON_ERROR_SYNTAX);
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
				jsonw_err(JSON_ERROR_SYNTAX);
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
					jsonw_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				}
				jsonw_err(JSON_ERROR_SYNTAX);
				return false;
			} else if (ch == '/') {
				if (!jsonw_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (!jsonw_is_space(ch)) {
				jsonw_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		jsonw_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	
	bool json_check_array(struct JsonContext* ctx, char** ptrptr, int32_t level) {
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
						json_err(JSON_ERROR_SYNTAX);
						return false;
					}
				}
				json_err(JSON_ERROR_UNEXPECTEDEND);
				return false;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}

	bool jsonw_check_array(struct JsonContext* ctx, wchar_t** ptrptr, int32_t level) {
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
						jsonw_err(JSON_ERROR_SYNTAX);
						return false;
					}
				}
				jsonw_err(JSON_ERROR_UNEXPECTEDEND);
				return false;
			}
		}
		jsonw_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	
	bool json_check_object(struct JsonContext* ctx, char** ptrptr, int32_t level) {
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
								json_err(JSON_ERROR_SYNTAX);
								return false;
							}
						}
						json_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							return false;
						}
					} else if (!json_is_space(ch)) {
						json_err(JSON_ERROR_SYNTAX);
						return false;
					}
				}
				json_err(JSON_ERROR_UNEXPECTEDEND);
				return false;
			} else if (!json_is_space(ch)) {
				json_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}

	bool jsonw_check_object(struct JsonContext* ctx, wchar_t** ptrptr, int32_t level) {
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
								jsonw_err(JSON_ERROR_SYNTAX);
								return false;
							}
						}
						jsonw_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else if (ch == '/') {
						if (!jsonw_skip_comment(ctx, ptrptr)) {
							return false;
						}
					} else if (!jsonw_is_space(ch)) {
						jsonw_err(JSON_ERROR_SYNTAX);
						return false;
					}
				}
				jsonw_err(JSON_ERROR_UNEXPECTEDEND);
				return false;
			} else if (!jsonw_is_space(ch)) {
				jsonw_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		jsonw_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	
	bool JsonCheck(struct JsonContext* ctx, const char* json) {
		ctx->seterror(JSON_ERROR_NONE, NULL);
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
						json_err(JSON_ERROR_SYNTAX);
						return false;
					}
				}
				return true;
			} else {
				json_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}

	bool JsonCheckW(struct JsonContext* ctx, const wchar_t* json) {
		ctx->wseterror(JSON_ERROR_NONE, NULL);
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
						jsonw_err(JSON_ERROR_SYNTAX);
						return false;
					}
				}
				return true;
			} else {
				jsonw_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		jsonw_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	
	struct JsonNode* JsonCreate(struct JsonNode* parent, JsonType type, uint32_t level, const char* data, uint32_t size) {
		JsonNode* j = memAlloc(struct JsonNode, j, sizeof(struct JsonNode));
		if (j == NULL) {
			return NULL;
		}
		bzero(j, sizeof(struct JsonNode));
		
		j->parent = parent;
		j->type = type;
		j->wide = false;
		j->level = level;
		j->data = (char*)data;
		j->size = size;
		j->value.n.v.i = 0;
		j->it = j;
		
		return j;
	}

	struct JsonNode* JsonCreateW(struct JsonNode* parent, JsonType type, uint32_t level, const wchar_t* data, uint32_t size) {
		JsonNode* j = memAlloc(struct JsonNode, j, sizeof(struct JsonNode));
		if (j == NULL) {
			return NULL;
		}
		bzero(j, sizeof(struct JsonNode));
		
		j->parent = parent;
		j->type = type;
		j->wide = true;
		j->level = level;
		j->wdata = (wchar_t*)data;
		j->size = size;
		j->value.n.v.i = 0;
		j->it = j;
		
		return j;
	}

	void JsonDestroy(struct JsonNode* j) {
		if (j != NULL) {
			switch (j->type) {
				case JSON_TYPE_OBJECT: {
					for (int i = j->value.o.c - 1; i >= 0; i--) {
						JsonDestroy(j->value.o.k[i]);
						JsonDestroy(j->value.o.v[i]);
					}
					memFree(j->value.o.k);
					memFree(j->value.o.v);
					break;
				}
					
				case JSON_TYPE_ARRAY: {
					for (int i = j->value.a.c - 1; i >= 0; i--) {
						JsonDestroy(j->value.a.v[i]);
					}
					memFree(j->value.a.v);
					break;
				}
					
				case JSON_TYPE_STRING: {
					if (j->value.cs != NULL) {
						memDelete(j->value.cs);
					}
					break;
				}
			}
			
			memFree(j);
		}
	}
	
	bool JsonObjectAdd(struct JsonNode* node, struct JsonNode* vkey, struct JsonNode* value) {
		if ((node != NULL) && (node->type == JSON_TYPE_OBJECT)) {
			struct JsonNode** jk = memRealloc(struct JsonNode*, jk, node->value.o.k, sizeof(struct JsonNode*) * (node->value.o.c + 1));
			struct JsonNode** jv = memRealloc(struct JsonNode*, jv, node->value.o.v, sizeof(struct JsonNode*) * (node->value.o.c + 1));
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
						JsonDestroy(jk[i]);
					}
					memFree(jk);
				} else {
					for (int i = node->value.o.c - 1; i >= 0; i--) {
						JsonDestroy(node->value.o.k[i]);
					}
					memFree(node->value.o.k);
				}
				node->value.o.k = NULL;
				
				if (jv != NULL) {
					for (int i = node->value.o.c - 1; i >= 0; i--) {
						JsonDestroy(jv[i]);
					}
					memFree(jv);
				} else {
					for (int i = node->value.o.c - 1; i >= 0; i--) {
						JsonDestroy(node->value.o.v[i]);
					}
					memFree(node->value.o.v);
				}
				node->value.o.v = NULL;
				
				node->value.o.c = 0;
			}
		}
		
		return false;
	}
	
	bool JsonArrayAdd(struct JsonNode* node, struct JsonNode* value) {
		if ((node != NULL) && (node->type == JSON_TYPE_ARRAY)) {
			struct JsonNode** jv = memRealloc(struct JsonNode*, jv, node->value.a.v, sizeof(struct JsonNode*) * (node->value.a.c + 1));
			if (jv != 0) {
				node->value.a.v = jv;
				jv[node->value.a.c] = value;
				node->value.a.c++;
				
				return true;
			} else {
				for (int i = node->value.a.c - 1; i >= 0; i--) {
					JsonDestroy(node->value.a.v[i]);
				}
				memFree(node->value.a.v);
				node->value.a.c = 0;
			}
		}
		
		return false;
	}
	
#include <math.h>
#include <wchar.h>
	
	bool json_parse_number(struct JsonContext* ctx, char** ptrptr, struct JsonNode* node) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		char ch = *(*ptrptr);
		bool minus = false;
		if (ch == '-') {
			minus = true;
			json_next;
		}
		if (ch == '\0') {
			json_err(JSON_ERROR_UNEXPECTEDEND);
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
				json_err(JSON_ERROR_UNEXPECTEDEND);
				return false;
			}
			json_err(JSON_ERROR_SYNTAX);
			return false;
		} else if (ch == '0') {
			json_next;
			goto json_parse_number_point;
		} else if ((ch >= '1') && (ch <= '9')) {
			do {
				if (json_next == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
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
						json_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				json_err(JSON_ERROR_SYNTAX);
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
						json_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				json_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		
		node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data));
		if (node->size > 63 * sizeof(char)) {
			json_err(JSON_ERROR_SYNTAX);
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
	
	bool jsonw_parse_number(struct JsonContext* ctx, wchar_t** ptrptr, struct JsonNode* node) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		wchar_t ch = *(*ptrptr);
		bool minus = false;
		if (ch == L'-') {
			minus = true;
			jsonw_next;
		}
		if (ch == L'\0') {
			jsonw_err(JSON_ERROR_UNEXPECTEDEND);
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
				jsonw_err(JSON_ERROR_UNEXPECTEDEND);
				return false;
			}
			jsonw_err(JSON_ERROR_SYNTAX);
			return false;
		} else if (ch == L'0') {
			jsonw_next;
			goto jsonw_parse_number_point;
		} else if ((ch >= L'1') && (ch <= L'9')) {
			do {
				if (jsonw_next == L'\0') {
					jsonw_err(JSON_ERROR_UNEXPECTEDEND);
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
						jsonw_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else if ((ch < L'0') || (ch > L'9')) {
						break;
					}
				} while (true);
			} else {
				jsonw_err(JSON_ERROR_SYNTAX);
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
						jsonw_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else if ((ch < L'0') || (ch > L'9')) {
						break;
					}
				} while (true);
			} else {
				jsonw_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		
		node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata));
		if (node->size > 63 * sizeof(wchar_t)) {
			jsonw_err(JSON_ERROR_SYNTAX);
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
	
	bool json_parse_string(struct JsonContext* ctx, char** ptrptr, struct JsonNode* node) {
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
						json_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else {
						json_err(JSON_ERROR_SYNTAX);
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
							json_err(JSON_ERROR_SYNTAX);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == '\0') {
							json_err(JSON_ERROR_UNEXPECTEDEND);
							return false;
						} else {
							json_err(JSON_ERROR_SYNTAX);
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
										json_err(JSON_ERROR_SYNTAX);
										return false;
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == '\0') {
										json_err(JSON_ERROR_UNEXPECTEDEND);
										return false;
									} else {
										json_err(JSON_ERROR_SYNTAX);
										return false;
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									json_err(JSON_ERROR_SYNTAX);
									return false;
								}
							} else if (ch == '\0') {
								json_err(JSON_ERROR_UNEXPECTEDEND);
								return false;
							} else {
								json_err(JSON_ERROR_SYNTAX);
								return false;
							}
						} else if (ch == '\0') {
							json_err(JSON_ERROR_UNEXPECTEDEND);
							return false;
						} else {
							json_err(JSON_ERROR_SYNTAX);
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
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				} else {
					json_err(JSON_ERROR_SYNTAX);
					return false;
				}
			} else {
				len++;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
		
	json_parse_string_alloc:
		char* ptr = memAlloc(char, ptr, (len + 1) * sizeof(char));
		if (ptr == NULL) {
			return false;
		}
		
		char* s = ptr;
		while (json_next != '\0') {
			if (ch == '\"') {
				*ptr = '\0';
				
				try {
					node->value.cs = memNew(node->value.cs, CString(s));
				} catch(...) {
					memFree(s);
					return false;
				}
				
				memFree(s);
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
					memFree(s);
					return false; // Not UTF8
				}
				while ((cnt > 0) && (json_next != '\0')) {
					if ((ch & 0xC0) != 0x80) {
						memFree(s);
						return false; // Not UTF8
					}
					*ptr = ch; ptr++;
					cnt--;
				}
				if (cnt > 0) {
					if (ch == '\0') {
						json_err(JSON_ERROR_UNEXPECTEDEND);
						memFree(s);
						return false;
					} else {
						json_err(JSON_ERROR_SYNTAX);
						memFree(s);
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
							json_err(JSON_ERROR_SYNTAX);
							memFree(s);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == '\0') {
							json_err(JSON_ERROR_UNEXPECTEDEND);
							memFree(s);
							return false;
						} else {
							json_err(JSON_ERROR_SYNTAX);
							memFree(s);
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
										json_err(JSON_ERROR_SYNTAX);
										memFree(s);
										return false;
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == '\0') {
										json_err(JSON_ERROR_UNEXPECTEDEND);
										memFree(s);
										return false;
									} else {
										json_err(JSON_ERROR_SYNTAX);
										memFree(s);
										return false;
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									json_err(JSON_ERROR_SYNTAX);
									memFree(s);
									return false;
								}
							} else if (ch == '\0') {
								json_err(JSON_ERROR_UNEXPECTEDEND);
								memFree(s);
								return false;
							} else {
								json_err(JSON_ERROR_SYNTAX);
								memFree(s);
								return false;
							}
						} else if (ch == '\0') {
							json_err(JSON_ERROR_UNEXPECTEDEND);
							memFree(s);
							return false;
						} else {
							json_err(JSON_ERROR_SYNTAX);
							memFree(s);
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
						memFree(s);
						return false;
					}
				} else if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					memFree(s);
					return false;
				} else {
					json_err(JSON_ERROR_SYNTAX);
					memFree(s);
					return false;
				}
			} else {
				*ptr = ch; ptr++;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	
	bool jsonw_parse_string(struct JsonContext* ctx, wchar_t** ptrptr, struct JsonNode* node) {
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
					json_err(JSON_ERROR_UNEXPECTEDEND);
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
							jsonw_err(JSON_ERROR_SYNTAX);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == L'\0') {
							jsonw_err(JSON_ERROR_UNEXPECTEDEND);
							return false;
						} else {
							jsonw_err(JSON_ERROR_SYNTAX);
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
										jsonw_err(JSON_ERROR_SYNTAX);
										return false;
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == L'\0') {
										jsonw_err(JSON_ERROR_UNEXPECTEDEND);
										return false;
									} else {
										jsonw_err(JSON_ERROR_SYNTAX);
										return false;
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									jsonw_err(JSON_ERROR_SYNTAX);
									return false;
								}
							} else if (ch == L'\0') {
								jsonw_err(JSON_ERROR_UNEXPECTEDEND);
								return false;
							} else {
								jsonw_err(JSON_ERROR_SYNTAX);
								return false;
							}
						} else if (ch == L'\0') {
							jsonw_err(JSON_ERROR_UNEXPECTEDEND);
							return false;
						} else {
							jsonw_err(JSON_ERROR_SYNTAX);
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
					jsonw_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				} else {
					jsonw_err(JSON_ERROR_SYNTAX);
					return false;
				}
			} else {
				len++;
			}
		}
		jsonw_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
		
	jsonw_parse_string_alloc:
		wchar_t* ptr = memAlloc(wchar_t, ptr, (len + 1) * sizeof(wchar_t));
		if (ptr == NULL) {
			return false;
		}
		
		wchar_t* ws = ptr;
		while (jsonw_next != L'\0') {
			if (ch == L'\"') {
				*ptr = L'\0';
				
				try {
					node->value.cs = memNew(node->value.cs, CString(ws));
				} catch (...) {
					memFree(ws);
					return false;
				}
				
				memFree(ws);
				return true;
#if __WCHAR_MAX__ <= 0x10000
			} else if ((uint16_t)(ch & 0xFC00) == 0xD800) { // found UTF16
				*ptr = ch; ptr++;
				if (jsonw_next == L'\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					memFree(ws);
					return false;
				}
				if ((uint16_t)(ch & 0xFC00) != 0xDC00) {
					memFree(ws);
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
							jsonw_err(JSON_ERROR_SYNTAX);
							memFree(ws);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == L'\0') {
							jsonw_err(JSON_ERROR_UNEXPECTEDEND);
							memFree(ws);
							return false;
						} else {
							jsonw_err(JSON_ERROR_SYNTAX);
							memFree(ws);
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
										jsonw_err(JSON_ERROR_SYNTAX);
										memFree(ws);
										return false;
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == L'\0') {
										jsonw_err(JSON_ERROR_UNEXPECTEDEND);
										memFree(ws);
										return false;
									} else {
										jsonw_err(JSON_ERROR_SYNTAX);
										memFree(ws);
										return false;
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									jsonw_err(JSON_ERROR_SYNTAX);
									memFree(ws);
									return false;
								}
							} else if (ch == L'\0') {
								jsonw_err(JSON_ERROR_UNEXPECTEDEND);
								memFree(ws);
								return false;
							} else {
								jsonw_err(JSON_ERROR_SYNTAX);
								memFree(ws);
								return false;
							}
						} else if (ch == L'\0') {
							jsonw_err(JSON_ERROR_UNEXPECTEDEND);
							memFree(ws);
							return false;
						} else {
							jsonw_err(JSON_ERROR_SYNTAX);
							memFree(ws);
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
						memFree(ws);
						return false; // No UTF32 to UTF16 conversion
					}
#else
					*ptr = (wchar_t)utf16; ptr++;
#endif
				} else if (ch == L'\0') {
					jsonw_err(JSON_ERROR_UNEXPECTEDEND);
					memFree(ws);
					return false;
				} else {
					jsonw_err(JSON_ERROR_SYNTAX);
					memFree(ws);
					return false;
				}
			} else {
				*ptr = ch; ptr++;
			}
		}
		jsonw_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	
	bool json_parse_object(struct JsonContext* ctx, char** ptrptr, struct JsonNode* node, int32_t level);
	bool json_parse_array(struct JsonContext* ctx, char** ptrptr, struct JsonNode* node, int32_t level);
	
	struct JsonNode* json_parse_node(struct JsonContext* ctx, char** ptrptr, struct JsonNode* parent, int32_t level) {
		char ch;
		struct JsonNode* node = NULL;
		while (json_next != '\0') {
			if (ch == '\"') {
				node = JsonCreate(parent, JSON_TYPE_STRING, level, *ptrptr, 0);
				if (node == NULL) {
					json_err(JSON_ERROR_OUTOFMEMORY);
					return NULL;
				}
				if (!json_parse_string(ctx, ptrptr, node)) {
					JsonDestroy(node);
					return NULL;
				}
				return node;
			} else if ((ch == '-') || ((ch >= '0') && (ch <= '9'))) {
				node = JsonCreate(parent, JSON_TYPE_NUMBER, level, *ptrptr, 0);
				if (node == NULL) {
					json_err(JSON_ERROR_OUTOFMEMORY);
					return NULL;
				}
				if (!json_parse_number(ctx, ptrptr, node)) {
					JsonDestroy(node);
					return NULL;
				}
				return node;
			} else if (ch == '{') {
				node = JsonCreate(parent, JSON_TYPE_OBJECT, level, *ptrptr, 0);
				if (node == NULL) {
					json_err(JSON_ERROR_OUTOFMEMORY);
					return NULL;
				}
				if (!json_parse_object(ctx, ptrptr, node, level)) {
					JsonDestroy(node);
					return NULL;
				}
				return node;
			} else if (ch == '[') {
				node = JsonCreate(parent, JSON_TYPE_ARRAY, level, *ptrptr, 0);
				if (node == NULL) {
					json_err(JSON_ERROR_OUTOFMEMORY);
					return NULL;
				}
				if (!json_parse_array(ctx, ptrptr, node, level)) {
					JsonDestroy(node);
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
							node = JsonCreate(parent, JSON_TYPE_BOOLEAN, level, *ptrptr - 3, 4);
							if (node == NULL) {
								json_err(JSON_ERROR_OUTOFMEMORY);
								return NULL;
							}
							node->value.b = true;
							return node;
						}
					}
				}
				if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return NULL;
				}
				json_err(JSON_ERROR_SYNTAX);
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
								node = JsonCreate(parent, JSON_TYPE_BOOLEAN, level, *ptrptr - 4, 5);
								if (node == NULL) {
									json_err(JSON_ERROR_OUTOFMEMORY);
									return NULL;
								}
								node->value.b = false;
								return node;
							}
						}
					}
				}
				if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return NULL;
				}
				json_err(JSON_ERROR_SYNTAX);
				return NULL;
			} else if ((ch == 'n') || (ch == 'N')) {
				json_next;
				if ((ch == 'u') || (ch == 'U')) {
					json_next;
					if ((ch == 'l') || (ch == 'L')) {
						json_next;
						if ((ch == 'l') || (ch == 'L')) {
							node = JsonCreate(parent, JSON_TYPE_NULL, level, *ptrptr - 3, 4);
							if (node == NULL) {
								json_err(JSON_ERROR_OUTOFMEMORY);
								return NULL;
							}
							return node;
						}
					}
				} else if ((ch == 'a') || (ch == 'A')) {
					json_next;
					if ((ch == 'n') || (ch == 'N')) {
						node = JsonCreate(parent, JSON_TYPE_NUMBER, level, *ptrptr - 2, 3);
						if (node == NULL) {
							json_err(JSON_ERROR_OUTOFMEMORY);
							return NULL;
						}
						node->value.n.is_float = true;
						node->value.n.v.f = NAN;
						return node;
					}
				}
				if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return NULL;
				}
				json_err(JSON_ERROR_SYNTAX);
				return NULL;
			} else if ((ch == 'i') || (ch == 'I')) {
				json_next;
				if ((ch == 'n') || (ch == 'N')) {
					json_next;
					if ((ch == 'f') || (ch == 'F')) {
						node = JsonCreate(parent, JSON_TYPE_NUMBER, level, *ptrptr - 2, 3);
						if (node == NULL) {
							json_err(JSON_ERROR_OUTOFMEMORY);
							return NULL;
						}
						node->value.n.is_float = true;
						node->value.n.v.f = INFINITY;
						return node;
					}
				}
				if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return NULL;
				}
				json_err(JSON_ERROR_SYNTAX);
				return NULL;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return NULL;
				}
			} else if (!json_is_space(ch)) {
				json_err(JSON_ERROR_SYNTAX);
				return NULL;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return NULL;
	}
	
	bool jsonw_parse_object(struct JsonContext* ctx, wchar_t** ptrptr, struct JsonNode* node, int32_t level);
	bool jsonw_parse_array(struct JsonContext* ctx, wchar_t** ptrptr, struct JsonNode* node, int32_t level);
	
	struct JsonNode* jsonw_parse_node(struct JsonContext* ctx, wchar_t** ptrptr, struct JsonNode* parent, int32_t level) {
		wchar_t ch;
		struct JsonNode* node = NULL;
		while (json_next != L'\0') {
			if (ch == L'\"') {
				node = JsonCreateW(parent, JSON_TYPE_STRING, level, *ptrptr, 0);
				if (node == NULL) {
					jsonw_err(JSON_ERROR_OUTOFMEMORY);
					return NULL;
				}
				if (!jsonw_parse_string(ctx, ptrptr, node)) {
					JsonDestroyW(node);
					return NULL;
				}
				return node;
			} else if ((ch == L'-') || ((ch >= L'0') && (ch <= L'9'))) {
				node = JsonCreateW(parent, JSON_TYPE_NUMBER, level, *ptrptr, 0);
				if (node == NULL) {
					jsonw_err(JSON_ERROR_OUTOFMEMORY);
					return NULL;
				}
				if (!jsonw_parse_number(ctx, ptrptr, node)) {
					JsonDestroyW(node);
					return NULL;
				}
				return node;
			} else if (ch == L'{') {
				node = JsonCreateW(parent, JSON_TYPE_OBJECT, level, *ptrptr, 0);
				if (node == NULL) {
					jsonw_err(JSON_ERROR_OUTOFMEMORY);
					return NULL;
				}
				if (!jsonw_parse_object(ctx, ptrptr, node, level)) {
					JsonDestroyW(node);
					return NULL;
				}
				return node;
			} else if (ch == L'[') {
				node = JsonCreateW(parent, JSON_TYPE_ARRAY, level, *ptrptr, 0);
				if (node == NULL) {
					jsonw_err(JSON_ERROR_OUTOFMEMORY);
					return NULL;
				}
				if (!jsonw_parse_array(ctx, ptrptr, node, level)) {
					JsonDestroyW(node);
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
							node = JsonCreateW(parent, JSON_TYPE_BOOLEAN, level, *ptrptr - 3, 4 * sizeof(wchar_t));
							if (node == NULL) {
								jsonw_err(JSON_ERROR_OUTOFMEMORY);
								return NULL;
							}
							node->value.b = true;
							return node;
						}
					}
				}
				if (ch == L'\0') {
					jsonw_err(JSON_ERROR_UNEXPECTEDEND);
					return NULL;
				}
				jsonw_err(JSON_ERROR_SYNTAX);
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
								node = JsonCreateW(parent, JSON_TYPE_BOOLEAN, level, *ptrptr - 4, 5 * sizeof(wchar_t));
								if (node == NULL) {
									jsonw_err(JSON_ERROR_OUTOFMEMORY);
									return NULL;
								}
								node->value.b = false;
								return node;
							}
						}
					}
				}
				if (ch == L'\0') {
					jsonw_err(JSON_ERROR_UNEXPECTEDEND);
					return NULL;
				}
				jsonw_err(JSON_ERROR_SYNTAX);
				return NULL;
			} else if ((ch == L'n') || (ch == L'N')) {
				jsonw_next;
				if ((ch == L'u') || (ch == L'U')) {
					jsonw_next;
					if ((ch == L'l') || (ch == L'L')) {
						jsonw_next;
						if ((ch == L'l') || (ch == L'L')) {
							node = JsonCreateW(parent, JSON_TYPE_NULL, level, *ptrptr - 3, 4 * sizeof(wchar_t));
							if (node == NULL) {
								jsonw_err(JSON_ERROR_OUTOFMEMORY);
								return NULL;
							}
							return node;
						}
					}
				} else if ((ch == L'a') || (ch == L'A')) {
					jsonw_next;
					if ((ch == L'n') || (ch == L'N')) {
						node = JsonCreateW(parent, JSON_TYPE_NUMBER, level, *ptrptr - 2, 3 * sizeof(wchar_t));
						if (node == NULL) {
							jsonw_err(JSON_ERROR_OUTOFMEMORY);
							return NULL;
						}
						node->value.n.is_float = true;
						node->value.n.v.f = NAN;
						return node;
					}
				}
				if (ch == L'\0') {
					jsonw_err(JSON_ERROR_UNEXPECTEDEND);
					return NULL;
				}
				jsonw_err(JSON_ERROR_SYNTAX);
				return NULL;
			} else if ((ch == L'i') || (ch == L'I')) {
				jsonw_next;
				if ((ch == L'n') || (ch == L'N')) {
					jsonw_next;
					if ((ch == L'f') || (ch == L'F')) {
						node = JsonCreateW(parent, JSON_TYPE_NUMBER, level, *ptrptr - 2, 3 * sizeof(wchar_t));
						if (node == NULL) {
							jsonw_err(JSON_ERROR_OUTOFMEMORY);
							return NULL;
						}
						node->value.n.is_float = true;
						node->value.n.v.f = INFINITY;
						return node;
					}
				}
				if (ch == L'\0') {
					jsonw_err(JSON_ERROR_UNEXPECTEDEND);
					return NULL;
				}
				jsonw_err(JSON_ERROR_SYNTAX);
				return NULL;
			} else if (ch == '/') {
				if (!jsonw_skip_comment(ctx, ptrptr)) {
					return NULL;
				}
			} else if (!jsonw_is_space(ch)) {
				jsonw_err(JSON_ERROR_SYNTAX);
				return NULL;
			}
		}
		jsonw_err(JSON_ERROR_UNEXPECTEDEND);
		return NULL;
	}
	
	bool json_parse_array(struct JsonContext* ctx, char** ptrptr, struct JsonNode* node, int32_t level) {
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
				struct JsonNode* value = json_parse_node(ctx, ptrptr, node, level);
				if (value == NULL) {
					return false;
				}
				if (!JsonArrayAdd(node, value)) {
					JsonDestroy(value);
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
						json_err(JSON_ERROR_SYNTAX);
						return false;
					}
				}
				json_err(JSON_ERROR_UNEXPECTEDEND);
				return false;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	
	bool jsonw_parse_array(struct JsonContext* ctx, wchar_t** ptrptr, struct JsonNode* node, int32_t level) {
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
				struct JsonNode* value = jsonw_parse_node(ctx, ptrptr, node, level);
				if (value == NULL) {
					return false;
				}
				if (!JsonArrayAddW(node, value)) {
					JsonDestroyW(value);
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
						jsonw_err(JSON_ERROR_SYNTAX);
						return false;
					}
				}
				jsonw_err(JSON_ERROR_UNEXPECTEDEND);
				return false;
			}
		}
		jsonw_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	
	bool json_parse_object(struct JsonContext* ctx, char** ptrptr, struct JsonNode* node, int32_t level) {
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
				struct JsonNode* vkey = JsonCreate(node, JSON_TYPE_STRING, level, *ptrptr, 0);
				if (vkey == NULL) {
					json_err(JSON_ERROR_OUTOFMEMORY);
					return false;
				}
				if (!json_parse_string(ctx, ptrptr, vkey)) {
					JsonDestroy(vkey);
					return false;
				}
				while (json_next != '\0') {
					if (ch == ':') {
						struct JsonNode* value = json_parse_node(ctx, ptrptr, node, level);
						if (value == NULL) {
							JsonDestroy(vkey);
							return false;
						}
						if (!JsonObjectAdd(node, vkey, value)) {
							JsonDestroy(vkey);
							JsonDestroy(value);
							return false;
						}
						while (json_next != '\0') {
							if (ch == ',') {
								goto json_parse_object_repeat;
							} else if (ch == '/') {
								if (!json_skip_comment(ctx, ptrptr)) {
									JsonDestroy(vkey);
									return false;
								}
							} else if (ch == '}') {
								node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->data)) + sizeof(char);
								return true;
							} else if (!json_is_space(ch)) {
								json_err(JSON_ERROR_SYNTAX);
								JsonDestroy(vkey);
								return false;
							}
						}
						json_err(JSON_ERROR_UNEXPECTEDEND);
						JsonDestroy(vkey);
						return false;
					} else if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							JsonDestroy(vkey);
							return false;
						}
					} else if (!json_is_space(ch)) {
						json_err(JSON_ERROR_SYNTAX);
						JsonDestroy(vkey);
						return false;
					}
				}
				json_err(JSON_ERROR_UNEXPECTEDEND);
				JsonDestroy(vkey);
				return false;
			} else if (!json_is_space(ch)) {
				json_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	
	bool jsonw_parse_object(struct JsonContext* ctx, wchar_t** ptrptr, struct JsonNode* node, int32_t level) {
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
				struct JsonNode* vkey = JsonCreateW(node, JSON_TYPE_STRING, level, *ptrptr, 0);
				if (vkey == NULL) {
					jsonw_err(JSON_ERROR_OUTOFMEMORY);
					return false;
				}
				if (!jsonw_parse_string(ctx, ptrptr, vkey)) {
					JsonDestroyW(vkey);
					return false;
				}
				while (jsonw_next != L'\0') {
					if (ch == L':') {
						struct JsonNode* value = jsonw_parse_node(ctx, ptrptr, node, level);
						if (value == NULL) {
							JsonDestroyW(vkey);
							return false;
						}
						if (!JsonObjectAddW(node, vkey, value)) {
							JsonDestroyW(vkey);
							JsonDestroyW(value);
							return false;
						}
						while (jsonw_next != L'\0') {
							if (ch == L',') {
								goto jsonw_parse_object_repeat;
							} else if (ch == L'/') {
								if (!jsonw_skip_comment(ctx, ptrptr)) {
									JsonDestroyW(vkey);
									return false;
								}
							} else if (ch == L'}') {
								node->size = (int)((intptr_t)(*ptrptr) - (intptr_t)(node->wdata)) + sizeof(wchar_t);
								return true;
							} else if (!jsonw_is_space(ch)) {
								jsonw_err(JSON_ERROR_SYNTAX);
								JsonDestroyW(vkey);
								return false;
							}
						}
						jsonw_err(JSON_ERROR_UNEXPECTEDEND);
						JsonDestroyW(vkey);
						return false;
					} else if (ch == '/') {
						if (!jsonw_skip_comment(ctx, ptrptr)) {
							JsonDestroyW(vkey);
							return false;
						}
					} else if (!jsonw_is_space(ch)) {
						jsonw_err(JSON_ERROR_SYNTAX);
						JsonDestroyW(vkey);
						return false;
					}
				}
				jsonw_err(JSON_ERROR_UNEXPECTEDEND);
				JsonDestroy(vkey);
				return false;
			} else if (!jsonw_is_space(ch)) {
				jsonw_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		jsonw_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	
	struct JsonNode* JsonParse(struct JsonContext* ctx, const char* json) {
		ctx->seterror(JSON_ERROR_NONE, NULL);
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
				JsonNode* node = JsonCreate(0, JSON_TYPE_OBJECT, 0, ptr, 0);
				if (node == NULL) {
					json_err(JSON_ERROR_OUTOFMEMORY);
					return NULL;
				}
				if (!json_parse_object(ctx, ptrptr, node, 0)) {
					JsonDestroy(node);
					return NULL;
				}
				while (json_next != '\0') {
					if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							JsonDestroy(node);
							return NULL;
						}
					} else if (!json_is_space(ch)) {
						JsonDestroy(node);
						json_err(JSON_ERROR_SYNTAX);
						return NULL;
					}
				}
				return node;
			} else {
				json_err(JSON_ERROR_SYNTAX);
				return NULL;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return NULL;
	}
	
	struct JsonNode* JsonParseW(struct JsonContext* ctx, const wchar_t* json) {
		ctx->wseterror(JSON_ERROR_NONE, NULL);
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
				JsonNode* node = JsonCreateW(0, JSON_TYPE_OBJECT, 0, ptr, 0);
				if (node == NULL) {
					jsonw_err(JSON_ERROR_OUTOFMEMORY);
					return NULL;
				}
				if (!jsonw_parse_object(ctx, ptrptr, node, 0)) {
					JsonDestroyW(node);
					return NULL;
				}
				while (jsonw_next != L'\0') {
					if (ch == L'/') {
						if (!jsonw_skip_comment(ctx, ptrptr)) {
							JsonDestroyW(node);
							return NULL;
						}
					} else if (!jsonw_is_space(ch)) {
						JsonDestroyW(node);
						jsonw_err(JSON_ERROR_SYNTAX);
						return NULL;
					}
				}
				return node;
			} else {
				jsonw_err(JSON_ERROR_SYNTAX);
				return NULL;
			}
		}
		jsonw_err(JSON_ERROR_UNEXPECTEDEND);
		return NULL;
	}
	
/*
	void json_debug_tab(uint32_t level) {
		for (int i = 0; i < level; i++) {
			printf(" ");
		}
	}
	
	void json_debug(const struct JsonNode* j) {
		if (j != 0) {
			switch (j->type) {
				case JSON_TYPE_NULL: {
					printf("null");
					break;
				}
					
				case JSON_TYPE_OBJECT: {
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
					
				case JSON_TYPE_ARRAY: {
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
					
				case JSON_TYPE_STRING: {
					printf("\"%s\"", j->value.s);
					break;
				}
					
				case JSON_TYPE_NUMBER: {
					if (j->value.n.is_float) {
						printf("%lf", j->value.n.v.f);
					} else {
						printf("%lld", j->value.n.v.i);
					}
					break;
				}
					
				case JSON_TYPE_BOOLEAN: {
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
	
	struct JsonNode* JsonArrayGet(const struct JsonNode* node, uint32_t index) {
		if (node != 0) {
			if (node->type == JSON_TYPE_OBJECT) {
				if (node->value.o.c > index) {
					return node->value.o.v[index];
				}
			} else if (node->type == JSON_TYPE_ARRAY) {
				if (node->value.a.c > index) {
					return node->value.a.v[index];
				}
			}
		}
		return NULL;
	}
	
	struct JsonNode* JsonObjectGet(const struct JsonNode* node, const CString& key) {
		if (node != 0) {
			if (node->type == JSON_TYPE_OBJECT) {
				for (int i = node->value.o.c - 1; i >= 0; i--) {
					if (node->value.o.k[i]->type == JSON_TYPE_STRING) {
						if (node->value.o.k[i]->value.cs->equals(key)) {
							return node->value.o.v[i];
						}
					}
				}
			}
		}
		return 0;
	}
	
	struct JsonNode* JsonObjectGetIC(const struct JsonNode* node, const CString& key) {
		if (node != 0) {
			if (node->type == JSON_TYPE_OBJECT) {
				for (int i = node->value.o.c - 1; i >= 0; i--) {
					if (node->value.o.k[i]->type == JSON_TYPE_STRING) {
						if (node->value.o.k[i]->value.cs->equalsIgnoreCase(key)) {
							return node->value.o.v[i];
						}
					}
				}
			}
		}
		return 0;
	}
	
	const CString* JsonParentKey(const struct JsonNode* value, const CString* def) {
		if ((value != 0) && (value->parent != 0) && (value->parent->type == JSON_TYPE_OBJECT)) {
			for (int i = value->parent->value.o.c - 1; i >= 0; i--) {
				if (value->parent->value.o.v[i] == value) {
					return value->parent->value.o.k[i]->value.cs;
				}
			}
		}
		return def;
	}
	
#ifdef DEBUG
	
	void JsonDebugError(const struct JsonContext& ctx, const char* json) {
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
			case JSON_ERROR_NONE:
				LOG("JSON Parse: No Error!");
				break;
			case JSON_ERROR_OUTOFMEMORY:
				LOG("JSON Parse: Out Of Memory!");
				break;
			case JSON_ERROR_UNEXPECTEDEND:
				LOG("JSON Parse: Unexpected End!");
				break;
			case JSON_ERROR_SYNTAX:
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
	
	void JsonDebugErrorW(const struct JsonContext& ctx, const wchar_t* json) {
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
			case JSON_ERROR_NONE:
				LOG("JSON Parse: No Error!");
				break;
			case JSON_ERROR_OUTOFMEMORY:
				LOG("JSON Parse: Out Of Memory!");
				break;
			case JSON_ERROR_UNEXPECTEDEND:
				LOG("JSON Parse: Unexpected End!");
				break;
			case JSON_ERROR_SYNTAX:
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
	
	bool json_call_number(struct JsonContext* ctx, char** ptrptr, struct JsonNumber* number) {
		// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
		char* nodedata = *ptrptr;
		char ch = *(*ptrptr);
		bool minus = false;
		if (ch == '-') {
			minus = true;
			json_next;
		}
		if (ch == '\0') {
			json_err(JSON_ERROR_UNEXPECTEDEND);
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
				json_err(JSON_ERROR_UNEXPECTEDEND);
				return false;
			}
			json_err(JSON_ERROR_SYNTAX);
			return false;
		} else if (ch == '0') {
			json_next;
			goto json_call_number_point;
		} else if ((ch >= '1') && (ch <= '9')) {
			do {
				if (json_next == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
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
						json_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				json_err(JSON_ERROR_SYNTAX);
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
						json_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else if ((ch < '0') || (ch > '9')) {
						break;
					}
				} while (true);
			} else {
				json_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		
		int nodesize = (int)((intptr_t)(*ptrptr) - (intptr_t)(nodedata));
		if (nodesize > 63 * sizeof(char)) {
			json_err(JSON_ERROR_SYNTAX);
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
	
	bool json_call_string(struct JsonContext* ctx, char** ptrptr, char** string) {
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
						json_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else {
						json_err(JSON_ERROR_SYNTAX);
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
							json_err(JSON_ERROR_SYNTAX);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == '\0') {
							json_err(JSON_ERROR_UNEXPECTEDEND);
							return false;
						} else {
							json_err(JSON_ERROR_SYNTAX);
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
										json_err(JSON_ERROR_SYNTAX);
										return false;
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == '\0') {
										json_err(JSON_ERROR_UNEXPECTEDEND);
										return false;
									} else {
										json_err(JSON_ERROR_SYNTAX);
										return false;
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									json_err(JSON_ERROR_SYNTAX);
									return false;
								}
							} else if (ch == '\0') {
								json_err(JSON_ERROR_UNEXPECTEDEND);
								return false;
							} else {
								json_err(JSON_ERROR_SYNTAX);
								return false;
							}
						} else if (ch == '\0') {
							json_err(JSON_ERROR_UNEXPECTEDEND);
							return false;
						} else {
							json_err(JSON_ERROR_SYNTAX);
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
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				} else {
					json_err(JSON_ERROR_SYNTAX);
					return false;
				}
			} else {
				len++;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
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
						json_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else {
						json_err(JSON_ERROR_SYNTAX);
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
							json_err(JSON_ERROR_SYNTAX);
							return false;
						}
						cnt--;
					}
					if (cnt > 0) {
						if (ch == '\0') {
							json_err(JSON_ERROR_UNEXPECTEDEND);
							return false;
						} else {
							json_err(JSON_ERROR_SYNTAX);
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
										json_err(JSON_ERROR_SYNTAX);
										return false;
									}
									cnt--;
								}
								if (cnt > 0) {
									if (ch == '\0') {
										json_err(JSON_ERROR_UNEXPECTEDEND);
										return false;
									} else {
										json_err(JSON_ERROR_SYNTAX);
										return false;
									}
								}
								
								if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
									utf16 = (utf16 << 10) + utf16_2 - 0x35FDC00;
								} else { // Not UTF16
									json_err(JSON_ERROR_SYNTAX);
									return false;
								}
							} else if (ch == '\0') {
								json_err(JSON_ERROR_UNEXPECTEDEND);
								return false;
							} else {
								json_err(JSON_ERROR_SYNTAX);
								return false;
							}
						} else if (ch == '\0') {
							json_err(JSON_ERROR_UNEXPECTEDEND);
							return false;
						} else {
							json_err(JSON_ERROR_SYNTAX);
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
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				} else {
					json_err(JSON_ERROR_SYNTAX);
					return false;
				}
			} else {
				*ptr = ch; ptr++;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}

	bool json_call_object(struct JsonContext* ctx, char** ptrptr);
	bool json_call_array(struct JsonContext* ctx, char** ptrptr);
	
	bool json_call_array_node(struct JsonContext* ctx, char** ptrptr, int index) {
		char ch;
		struct JsonNode* node = NULL;
		while (json_next != '\0') {
			if (ch == '\"') {
				if (ctx->callbacks->onarray.onstring != NULL) {
					char* value = NULL;
					if (!json_call_string(ctx, ptrptr, &value)) {
						return false;
					}
					try {
						ctx->callbacks->onarray.onstring(ctx, index, value, ctx->callbacks->target);
					} catch (...) {
						json_err(JSON_ERROR_OUTOFMEMORY);
						memFree(value);
						return false;
					}
					memFree(value);
				} else {
					if (!json_check_string(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if ((ch == '-') || ((ch >= '0') && (ch <= '9'))) {
				if (ctx->callbacks->onarray.onnumber != NULL) {
					struct JsonNumber number;
					if (!json_call_number(ctx, ptrptr, &number)) {
						return false;
					}
					try {
						ctx->callbacks->onarray.onnumber(ctx, index, number, ctx->callbacks->target);
					} catch (...) {
						json_err(JSON_ERROR_OUTOFMEMORY);
						return false;
					}
				} else {
					if (!json_check_number(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if (ch == '{') {
				if (ctx->callbacks->onarray.onobjectstart != NULL) {
					struct JsonCallbacks store;
					JsonCopyCallbacks(&store, ctx->callbacks);
					try {
						ctx->callbacks->onarray.onobjectstart(ctx, index, ctx->callbacks->target);
					} catch (...) {
						json_err(JSON_ERROR_OUTOFMEMORY);
						return false;
					}
					bool result = json_call_object(ctx, ptrptr);
					if (ctx->callbacks->onarray.onobjectend != NULL) {
						try {
							ctx->callbacks->onarray.onobjectend(ctx, index, store.target, ctx->callbacks->target, result);
						} catch (...) {
							json_err(JSON_ERROR_OUTOFMEMORY);
							result = false;
						}
					}
					if (!result) {
						JsonCopyCallbacks(ctx->callbacks, &store);
						return false;
					}
					JsonCopyCallbacks(ctx->callbacks, &store);
				} else {
					if (!json_check_object(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if (ch == '[') {
				if (ctx->callbacks->onarray.onarraystart != NULL) {
					struct JsonCallbacks store;
					JsonCopyCallbacks(&store, ctx->callbacks);
					try {
						ctx->callbacks->onarray.onarraystart(ctx, index, ctx->callbacks->target);
					} catch (...) {
						json_err(JSON_ERROR_OUTOFMEMORY);
						return false;
					}
					bool result = json_call_array(ctx, ptrptr);
					if (ctx->callbacks->onarray.onarrayend != NULL) {
						try {
							ctx->callbacks->onarray.onarrayend(ctx, index, store.target, ctx->callbacks->target, result);
						} catch (...) {
							json_err(JSON_ERROR_OUTOFMEMORY);
							result = false;
						}
					}
					if (!result) {
						JsonCopyCallbacks(ctx->callbacks, &store);
						return false;
					}
					JsonCopyCallbacks(ctx->callbacks, &store);
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
								try {
									ctx->callbacks->onarray.onboolean(ctx, index, true, ctx->callbacks->target);
								} catch (...) {
									json_err(JSON_ERROR_OUTOFMEMORY);
									return false;
								}
							}
							return true;
						}
					}
				}
				if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				}
				json_err(JSON_ERROR_SYNTAX);
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
									try {
										ctx->callbacks->onarray.onboolean(ctx, index, false, ctx->callbacks->target);
									} catch (...) {
										json_err(JSON_ERROR_OUTOFMEMORY);
										return false;
									}
								}
								return true;
							}
						}
					}
				}
				if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				}
				json_err(JSON_ERROR_SYNTAX);
				return NULL;
			} else if ((ch == 'n') || (ch == 'N')) {
				json_next;
				if ((ch == 'u') || (ch == 'U')) {
					json_next;
					if ((ch == 'l') || (ch == 'L')) {
						json_next;
						if ((ch == 'l') || (ch == 'L')) {
							if (ctx->callbacks->onarray.onnull != NULL) {
								try {
									ctx->callbacks->onarray.onnull(ctx, index, ctx->callbacks->target);
								} catch (...) {
									json_err(JSON_ERROR_OUTOFMEMORY);
									return false;
								}
							}
							return true;
						}
					}
				} else if ((ch == 'a') || (ch == 'A')) {
					json_next;
					if ((ch == 'n') || (ch == 'N')) {
						if (ctx->callbacks->onarray.onnumber != NULL) {
							try {
								struct JsonNumber number;
								number.is_float = true;
								number.v.f = NAN;
								ctx->callbacks->onarray.onnumber(ctx, index, number, ctx->callbacks->target);
							} catch (...) {
								json_err(JSON_ERROR_OUTOFMEMORY);
								return false;
							}
						}
						return true;
					}
				}
				if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				}
				json_err(JSON_ERROR_SYNTAX);
				return false;
			} else if ((ch == 'i') || (ch == 'I')) {
				json_next;
				if ((ch == 'n') || (ch == 'N')) {
					json_next;
					if ((ch == 'f') || (ch == 'F')) {
						if (ctx->callbacks->onarray.onnumber != NULL) {
							try {
								struct JsonNumber number;
								number.is_float = true;
								number.v.f = INFINITY;
								ctx->callbacks->onarray.onnumber(ctx, index, number, ctx->callbacks->target);
							} catch (...) {
								json_err(JSON_ERROR_OUTOFMEMORY);
								return false;
							}
						}
						return true;
					}
				}
				if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				}
				json_err(JSON_ERROR_SYNTAX);
				return false;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (!json_is_space(ch)) {
				json_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	
	bool json_call_object_node(struct JsonContext* ctx, char** ptrptr, const char* key) {
		char ch;
		struct JsonNode* node = NULL;
		while (json_next != '\0') {
			if (ch == '\"') {
				if (ctx->callbacks->onobject.onstring != NULL) {
					char* value = NULL;
					if (!json_call_string(ctx, ptrptr, &value)) {
						return false;
					}
					try {
						ctx->callbacks->onobject.onstring(ctx, key, value, ctx->callbacks->target);
					} catch (...) {
						json_err(JSON_ERROR_OUTOFMEMORY);
						memFree(value);
						return false;
					}
					memFree(value);
				} else {
					if (!json_check_string(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if ((ch == '-') || ((ch >= '0') && (ch <= '9'))) {
				if (ctx->callbacks->onobject.onnumber != NULL) {
					struct JsonNumber number;
					if (!json_call_number(ctx, ptrptr, &number)) {
						return false;
					}
					try {
						ctx->callbacks->onobject.onnumber(ctx, key, number, ctx->callbacks->target);
					} catch (...) {
						json_err(JSON_ERROR_OUTOFMEMORY);
						return false;
					}
				} else {
					if (!json_check_number(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if (ch == '{') {
				if (ctx->callbacks->onobject.onobjectstart != NULL) {
					struct JsonCallbacks store;
					JsonCopyCallbacks(&store, ctx->callbacks);
					try {
						ctx->callbacks->onobject.onobjectstart(ctx, key, ctx->callbacks->target);
					} catch (...) {
						json_err(JSON_ERROR_OUTOFMEMORY);
						return false;
					}
					bool result = json_call_object(ctx, ptrptr);
					if (ctx->callbacks->onobject.onobjectend != NULL) {
						try {
							ctx->callbacks->onobject.onobjectend(ctx, key, store.target, ctx->callbacks->target, result);
						} catch (...) {
							json_err(JSON_ERROR_OUTOFMEMORY);
							result = false;
						}
					}
					if (!result) {
						JsonCopyCallbacks(ctx->callbacks, &store);
						return false;
					}
					JsonCopyCallbacks(ctx->callbacks, &store);
				} else {
					if (!json_check_object(ctx, ptrptr, 0)) {
						return false;
					}
				}
				return true;
			} else if (ch == '[') {
				if (ctx->callbacks->onobject.onarraystart != NULL) {
					struct JsonCallbacks store;
					JsonCopyCallbacks(&store, ctx->callbacks);
					try {
						ctx->callbacks->onobject.onarraystart(ctx, key, ctx->callbacks->target);
					} catch (...) {
						json_err(JSON_ERROR_OUTOFMEMORY);
						return false;
					}
					bool result = json_call_array(ctx, ptrptr);
					if (ctx->callbacks->onobject.onarrayend != NULL) {
						try {
							ctx->callbacks->onobject.onarrayend(ctx, key, store.target, ctx->callbacks->target, result);
						} catch (...) {
							json_err(JSON_ERROR_OUTOFMEMORY);
							result = false;
						}
					}
					if (!result) {
						JsonCopyCallbacks(ctx->callbacks, &store);
						return false;
					}
					JsonCopyCallbacks(ctx->callbacks, &store);
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
								try {
									ctx->callbacks->onobject.onboolean(ctx, key, true, ctx->callbacks->target);
								} catch (...) {
									json_err(JSON_ERROR_OUTOFMEMORY);
									return false;
								}
							}
							return true;
						}
					}
				}
				if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				}
				json_err(JSON_ERROR_SYNTAX);
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
									try {
										ctx->callbacks->onobject.onboolean(ctx, key, false, ctx->callbacks->target);
									} catch (...) {
										json_err(JSON_ERROR_OUTOFMEMORY);
										return false;
									}
								}
								return true;
							}
						}
					}
				}
				if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				}
				json_err(JSON_ERROR_SYNTAX);
				return NULL;
			} else if ((ch == 'n') || (ch == 'N')) {
				json_next;
				if ((ch == 'u') || (ch == 'U')) {
					json_next;
					if ((ch == 'l') || (ch == 'L')) {
						json_next;
						if ((ch == 'l') || (ch == 'L')) {
							if (ctx->callbacks->onobject.onnull != NULL) {
								try {
									ctx->callbacks->onobject.onnull(ctx, key, ctx->callbacks->target);
								} catch (...) {
									json_err(JSON_ERROR_OUTOFMEMORY);
									return false;
								}
							}
							return true;
						}
					}
				} else if ((ch == 'a') || (ch == 'A')) {
					json_next;
					if ((ch == 'n') || (ch == 'N')) {
						if (ctx->callbacks->onobject.onnumber != NULL) {
							try {
								struct JsonNumber number;
								number.is_float = true;
								number.v.f = NAN;
								ctx->callbacks->onobject.onnumber(ctx, key, number, ctx->callbacks->target);
							} catch (...) {
								json_err(JSON_ERROR_OUTOFMEMORY);
								return false;
							}
						}
						return true;
					}
				}
				if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				}
				json_err(JSON_ERROR_SYNTAX);
				return false;
			} else if ((ch == 'i') || (ch == 'I')) {
				json_next;
				if ((ch == 'n') || (ch == 'N')) {
					json_next;
					if ((ch == 'f') || (ch == 'F')) {
						if (ctx->callbacks->onobject.onnumber != NULL) {
							try {
								struct JsonNumber number;
								number.is_float = true;
								number.v.f = INFINITY;
								ctx->callbacks->onobject.onnumber(ctx, key, number, ctx->callbacks->target);
							} catch (...) {
								json_err(JSON_ERROR_OUTOFMEMORY);
								return false;
							}
						}
						return true;
					}
				}
				if (ch == '\0') {
					json_err(JSON_ERROR_UNEXPECTEDEND);
					return false;
				}
				json_err(JSON_ERROR_SYNTAX);
				return false;
			} else if (ch == '/') {
				if (!json_skip_comment(ctx, ptrptr)) {
					return false;
				}
			} else if (!json_is_space(ch)) {
				json_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	
	bool json_call_array(struct JsonContext* ctx, char** ptrptr) {
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
						json_err(JSON_ERROR_SYNTAX);
						return false;
					}
				}
				json_err(JSON_ERROR_UNEXPECTEDEND);
				return false;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}

	bool json_call_object(struct JsonContext* ctx, char** ptrptr) {
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
								json_err(JSON_ERROR_SYNTAX);
								return false;
							}
						}
						json_err(JSON_ERROR_UNEXPECTEDEND);
						return false;
					} else if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							memFree(vkey);
							return false;
						}
					} else if (!json_is_space(ch)) {
						memFree(vkey);
						json_err(JSON_ERROR_SYNTAX);
						return false;
					}
				}
				memFree(vkey);
				json_err(JSON_ERROR_UNEXPECTEDEND);
				return false;
			} else if (!json_is_space(ch)) {
				json_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}

	bool JsonCall(struct JsonContext* ctx, const char* json) {
		if (ctx->callbacks == NULL)
			return false;
		
		ctx->seterror(JSON_ERROR_NONE, NULL);
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
					if (ctx->callbacks->onrootstart != NULL) {
						try {
							ctx->callbacks->onrootstart(ctx, ctx->callbacks->target);
						} catch (...) {
							json_err(JSON_ERROR_OUTOFMEMORY);
							return false;
						}
					}
				}
				bool result = json_call_object(ctx, ptrptr);
				if (ctx->callbacks != NULL) {
					if (ctx->callbacks->onrootend != NULL) {
						try {
							ctx->callbacks->onrootend(ctx, ctx->callbacks->target, result);
						} catch (...) {
							json_err(JSON_ERROR_OUTOFMEMORY);
							result = false;
						}
					}
				}
				if (!result) {
					return false;
				}
				while (json_next != '\0') {
					if (ch == '/') {
						if (!json_skip_comment(ctx, ptrptr)) {
							return false;
						}
					} else if (!json_is_space(ch)) {
						json_err(JSON_ERROR_SYNTAX);
						return false;
					}
				}
				return true;
			} else {
				json_err(JSON_ERROR_SYNTAX);
				return false;
			}
		}
		json_err(JSON_ERROR_UNEXPECTEDEND);
		return false;
	}
	

#ifdef __cplusplus
}
#endif

JSONObject::JSONObject(const CString& json) {
	if (json.m_length == 0)
		return;
	
	struct JsonContext ctx;
	root = JsonParseW(&ctx, json.m_data);
	if (root == NULL) {
#ifdef DEBUG
		JsonDebugErrorW(ctx, json.m_data);
#endif
		JsonDestroyW(root);
		root = NULL;
	}
}

JSONObject::JSONObject(const wchar_t* json) {
	if (json == NULL)
		return;
	
	struct JsonContext ctx;
	root = JsonParseW(&ctx, json);
	if (root == NULL) {
#ifdef DEBUG
		JsonDebugErrorW(ctx, json);
#endif
		JsonDestroyW(root);
		root = NULL;
	}
}

JSONObject::JSONObject(const char* json) {
	if (json == NULL)
		return;
	
	struct JsonContext ctx;
	root = JsonParse(&ctx, json);
	if (root == NULL) {
#ifdef DEBUG
		JsonDebugError(ctx, json);
#endif
		JsonDestroy(root);
		root = NULL;
	}
}

JSONObject::~JSONObject() {
	if (root != NULL) {
		JsonDestroy(root);
	}
}

int32_t JsonNode::count() const {
	if (type == JSON_TYPE_ARRAY) {
		return value.a.c;
	} else if (type == JSON_TYPE_OBJECT) {
		return value.o.c;
	} else
		return 1;
}

const struct JsonNode** JsonNode::keys() const throw(const char*) {
	if (type == JSON_TYPE_OBJECT) {
		return (const struct JsonNode**)value.o.k;
	} else
		throw eConvert;
}

const struct JsonNode** JsonNode::items() const {
	if (type == JSON_TYPE_ARRAY) {
		return (const struct JsonNode**)value.a.v;
	} else if (type == JSON_TYPE_OBJECT) {
		return (const struct JsonNode**)value.o.v;
	} else
		return (const struct JsonNode**)&it;
}

const struct JsonNode* JsonNode::get(int index) const throw(const char*) {
	if (type == JSON_TYPE_ARRAY) {
		if ((index >= 0) && (index < value.a.c)) {
			return value.a.v[index];
		} else
			throw eOutOfRange;
	} else if (type == JSON_TYPE_OBJECT) {
		if ((index >= 0) && (index < value.o.c)) {
			return value.o.v[index];
		} else
			throw eOutOfRange;
	} else
		throw eConvert;
}

const struct JsonNode* JsonNode::getKey(int index) const throw(const char*) {
	if (type == JSON_TYPE_OBJECT) {
		if ((index >= 0) && (index < value.o.c)) {
			return value.o.k[index];
		} else
			throw eOutOfRange;
	} else
		throw eConvert;
}

const struct JsonNode* JsonNode::get(const CString& key) const throw(const char*) {
	if (type == JSON_TYPE_OBJECT) {
		for (int i = 0; i < value.o.c; i++) {
			if (value.o.k[i]->type == JSON_TYPE_STRING) {
				if (key.equals(*(value.o.k[i]->value.cs))) {
					return value.o.v[i];
				}
			} else if (value.o.k[i]->type == JSON_TYPE_BOOLEAN) {
				if (value.o.k[i]->value.b) {
					if (key.equalsIgnoreCase(L"true")) {
						return value.o.v[i];
					}
				} else {
					if (key.equalsIgnoreCase(L"false")) {
						return value.o.v[i];
					}
				}
			} else if (value.o.k[i]->type == JSON_TYPE_NUMBER) {
				if (value.o.k[i]->value.n.is_float) {
					if (key.equals(value.o.k[i]->value.n.v.f)) {
						return value.o.v[i];
					}
				} else {
					if (key.equals(value.o.k[i]->value.n.v.i)) {
						return value.o.v[i];
					}
				}
			}
		}
		throw eNotFound;
	} else
		throw eConvert;
}

CString JsonNode::toString() const throw(const char*) {
	switch (type) {
		case JSON_TYPE_NULL:
			return L"null";
		case JSON_TYPE_BOOLEAN:
			return value.b;
		case JSON_TYPE_NUMBER:
			if (value.n.is_float) {
				return value.n.v.f;
			} else {
				return value.n.v.i;
			}
		case JSON_TYPE_STRING:
			return *value.cs;
		default:
			throw eConvert;
	}
}

double JsonNode::toDouble() const throw(const char*) {
	switch (type) {
		case JSON_TYPE_NULL:
			return NAN;
		case JSON_TYPE_BOOLEAN:
			return value.b ? 1.0 : 0.0;
		case JSON_TYPE_NUMBER:
			if (value.n.is_float) {
				return value.n.v.f;
			} else {
				return value.n.v.i;
			}
		case JSON_TYPE_STRING:
			return value.cs->operator double();
		default:
			throw eConvert;
	}
}

int64_t JsonNode::toInt() const throw(const char*) {
	switch (type) {
		case JSON_TYPE_NULL:
			return 0;
		case JSON_TYPE_BOOLEAN:
			return value.b ? 1 : 0;
		case JSON_TYPE_NUMBER:
			if (value.n.is_float) {
				return value.n.v.f;
			} else {
				return value.n.v.i;
			}
		case JSON_TYPE_STRING:
			return value.cs->operator int64_t();
		default:
			throw eConvert;
	}
}

bool JsonNode::toBoolean() const throw(const char*) {
	switch (type) {
		case JSON_TYPE_NULL:
			return 0;
		case JSON_TYPE_BOOLEAN:
			return value.b;
		case JSON_TYPE_NUMBER:
			if (value.n.is_float) {
				return value.n.v.f != 0;
			} else {
				return value.n.v.i != 0;
			}
		case JSON_TYPE_STRING:
			return value.cs->operator bool();
		default:
			throw eConvert;
	}
}

CString JsonNode::optString(int index, const CString& fallback) const {
	try {
		return get(index)->toString();
	} catch(...) {
		return fallback;
	}
}

CString JsonNode::optString(const CString& key, const CString& fallback) const {
	try {
		return get(key)->toString();
	} catch(...) {
		return fallback;
	}
}

double JsonNode::optDouble(int index, double fallback) const {
	try {
		return get(index)->toDouble();
	} catch(...) {
		return fallback;
	}
}

double JsonNode::optDouble(const CString& key, double fallback) const {
	try {
		return get(key)->toDouble();
	} catch(...) {
		return fallback;
	}
}

int64_t JsonNode::optInt(int index, int64_t fallback) const {
	try {
		return get(index)->toInt();
	} catch(...) {
		return fallback;
	}
}

int64_t JsonNode::optInt(const CString& key, int64_t fallback) const {
	try {
		return get(key)->toInt();
	} catch(...) {
		return fallback;
	}
}

bool JsonNode::optBoolean(int index, bool fallback) const {
	try {
		return get(index)->toBoolean();
	} catch(...) {
		return fallback;
	}
}

bool JsonNode::optBoolean(const CString& key, bool fallback) const {
	try {
		return get(key)->toBoolean();
	} catch(...) {
		return fallback;
	}
}

CString JSON::encode(const CString& value) throw(const char*) {
	// TODO: encode JSON string
	return value;
}
