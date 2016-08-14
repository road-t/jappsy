#include "uJson.h"
#include "core/uMemoryManager.h"

#define is_space(ch)	(((ch) == ' ') || ((ch) == '\t') || ((ch) == '\r') || ((ch) == '\n') || ((ch) == '\b') || ((ch) == '\f'))
#define is_hex(ch)		((((ch) >= '0') && ((ch) <= '9')) || (((ch) >= 'a') && ((ch) <= 'f')) || (((ch) >= 'A') && ((ch) <= 'F')))

static __thread uint8_t* _json_ptr_ = 0;
#define jnext			(ch = *(++_json_ptr_))

bool check_json_number(int32_t level) {
	// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
	uint8_t ch = *_json_ptr_;
	if (ch == '-') {
		jnext;
	}
	if (ch == 0) {
		return false; // Неожиданный конец
	} else if ((ch == 'i') || (ch == 'I')) {
		jnext;
		if ((ch == 'n') || (ch == 'N')) {
			jnext;
			if ((ch == 'f') || (ch == 'F')) {
				return true;
			}
		}
		if (ch == 0) {
			return false; // Неожиданный конец
		}
		return false; // Ошибка синтаксиса
	} else if (ch == '0') {
		jnext;
		goto check_json_number_point;
	} else if ((ch >= '1') && (ch <= '9')) {
		do {
			if (jnext == 0) {
				return false; // Неожиданный конец
			} else if ((ch < '0') || (ch > '9')) {
				break;
			}
		} while (true);
	}
check_json_number_point:
	if (ch == '.') {
		jnext;
		if ((ch >= '0') && (ch <= '9')) {
			do {
				if (jnext == 0) {
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
		jnext;
		if ((ch == '-') || (ch == '+')) {
			jnext;
		}
		if ((ch >= '0') && (ch <= '9')) {
			do {
				if (jnext == 0) {
					return false; // Неожиданный конец
				} else if ((ch < '0') || (ch > '9')) {
					break;
				}
			} while (true);
		} else {
			return false; // Ошибка синтаксиса
		}
	}
	_json_ptr_--;
	return true;
}

bool check_json_string(int32_t level) {
	uint8_t ch;
	while (jnext != 0) {
		if (ch == '\"') {
			return true;
		} else if (ch & 0x80) { // found UTF8
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
			while ((cnt > 0) && (jnext != 0)) {
				if ((ch & 0xC0) != 0x80) {
					return false; // Not UTF8
				}
				cnt--;
			}
			if (cnt > 0) {
				if (ch == 0) {
					return false; // Неожиданный конец
				} else {
					return false; // Ошибка синтаксиса
				}
			}
		} else if (ch == '\\') {
			jnext;
			if ((ch == '\"') || (ch == '\\') || (ch == '\/') || (ch == 'b') || (ch == 'f') || (ch == 'n') || (ch == 'r') || (ch == 't')) {
			} else if (ch == 'u') {
				int cnt = 4;
				while ((cnt > 0) && (jnext != 0)) {
					if (!is_hex(ch)) {
						return false; // Ошибка синтаксиса
					}
					cnt--;
				}
				if (cnt > 0) {
					if (ch == 0) {
						return false; // Неожиданный конец
					} else {
						return false; // Ошибка синтаксиса
					}
				}
			} else if (ch == 0) {
				return false; // Неожиданный конец
			} else {
				return false; // Ошибка синтаксиса
			}
		}
	}
}

bool check_json_object(int32_t level);
bool check_json_array(int32_t level);

bool check_json_value(int32_t level) {
	uint8_t ch;
	while (jnext != 0) {
		if (ch == '\"') {
			return check_json_string(level);
		} else if ((ch == '-') || ((ch >= '0') && (ch <= '9'))) {
			return check_json_number(level);
		} else if (ch == '{') {
			return check_json_object(level);
		} else if (ch == '[') {
			return check_json_array(level);
		} else if ((ch == 't') || (ch == 'T')) {
			jnext;
			if ((ch == 'r') || (ch == 'R')) {
				jnext;
				if ((ch == 'u') || (ch == 'U')) {
					jnext;
					if ((ch == 'e') || (ch == 'E')) {
						return true;
					}
				}
			}
			return false; // Ошибка синтаксиса
		} else if ((ch == 'f') || (ch == 'F')) {
			jnext;
			if ((ch == 'a') || (ch == 'A')) {
				jnext;
				if ((ch == 'l') || (ch == 'L')) {
					jnext;
					if ((ch == 's') || (ch == 'S')) {
						jnext;
						if ((ch == 'e') || (ch == 'E')) {
							return true;
						}
					}
				}
			}
			return false; // Ошибка синтаксиса
		} else if ((ch == 'n') || (ch == 'N')) {
			jnext;
			if ((ch == 'u') || (ch == 'U')) {
				jnext;
				if ((ch == 'l') || (ch == 'L')) {
					jnext;
					if ((ch == 'l') || (ch == 'L')) {
						return true;
					}
				}
			} else if ((ch == 'a') || (ch == 'A')) {
				jnext;
				if ((ch == 'n') || (ch == 'N')) {
					return true;
				}
			}
			return false; // Ошибка синтаксиса
		} else if ((ch == 'i') || (ch == 'I')) {
			jnext;
			if ((ch == 'n') || (ch == 'N')) {
				jnext;
				if ((ch == 'f') || (ch == 'F')) {
					return true;
				}
			}
			if (ch == 0) {
				return false; // Неожиданный конец
			}
			return false; // Ошибка синтаксиса
		} else if (!is_space(ch)) {
			return false; // Ошибка синтаксиса
		}
	}
	return false; // Неожиданный конец
}

bool check_json_array(int32_t level) {
	uint8_t ch;
	level++;
check_json_array_repeat:
	while (jnext != 0) {
		if (ch == ']') {
			return true;
		} else if (!is_space(ch)) {
			_json_ptr_--;
			if (!check_json_value(level)) {
				return false;
			}
			while (jnext != 0) {
				if (ch == ',') {
					goto check_json_array_repeat;
				} else if (ch == ']') {
					return true;
				} else if (!is_space(ch)) {
					return false; // Ошибка синтаксиса
				}
			}
			return false; // Неожиданный конец
		}
	}
	return false; // Неожиданный конец
}

bool check_json_object(int32_t level) {
	uint8_t ch;
	level++;
check_json_object_repeat:
	while (jnext != 0) {
		if (ch == '}') {
			return true;
		} else if (ch == '\"') {
			if (!check_json_string(level)) {
				return false;
			}
			while (jnext != 0) {
				if (ch == ':') {
					if (!check_json_value(level)) {
						return false;
					}
					while (jnext != 0) {
						if (ch == ',') {
							goto check_json_object_repeat;
						} else if (ch == '}') {
							return true;
						} else if (!is_space(ch)) {
							return false; // Ошибка синтаксиса
						}
					}
					return false; // Неожиданный конец
				} else if (!is_space(ch)) {
					return false; // Ошибка синтаксиса
				}
			}
			return false; // Неожиданный конец
		} else if (!is_space(ch)) {
			return false; // Ошибка синтаксиса
		}
	}
	return false; // Неожиданный конец
}

bool json_check(const char* json) {
	_json_ptr_ = (uint8_t*)json;
	uint8_t ch;
	while ((ch = *_json_ptr_) != 0) {
		if (is_space(ch)) {
			_json_ptr_++;
		} else if (ch == '{') {
			if (!check_json_object(0)) {
				return false;
			}
			while (jnext != 0) {
				if (!is_space(ch)) {
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

struct json_value* json_create(struct json_value* parent, json_type type, uint32_t level, const uint8_t* data) {
	json_value* j = (struct json_value*)mmalloc(sizeof(struct json_value));
	if (j == 0) {
		return 0;
	}
	memset(j, 0, sizeof(struct json_value));

	j->parent = parent;
	j->type = type;
	j->level = level;
	j->data = (uint8_t*)data;

	return j;
}

void json_destroy(struct json_value* j) {
	if (j != 0) {
		switch (j->type) {
			case json_type_object: {
				for (int i = j->value.o.c - 1; i >= 0; i--) {
					json_destroy(j->value.o.k[i]);
					json_destroy(j->value.o.v[i]);
				}
				mmfree(j->value.o.k);
				mmfree(j->value.o.v);
				break;
			}

			case json_type_array: {
				for (int i = j->value.a.c - 1; i >= 0; i--) {
					json_destroy(j->value.a.v[i]);
				}
				mmfree(j->value.a.v);
				break;
			}

			case json_type_string: {
				if (j->value.s != 0) {
					mmfree(j->value.s);
				}
				break;
			}
		}

		mmfree(j);
	}
}

bool json_object_add(struct json_value* node, struct json_value* vkey, struct json_value* value) {
	if ((node != 0) && (node->type == json_type_object)) {
		struct json_value** jk = (struct json_value**)mmrealloc(node->value.o.k, sizeof(struct json_value*) * (node->value.o.c + 1));
		struct json_value** jv = (struct json_value**)mmrealloc(node->value.o.v, sizeof(struct json_value*) * (node->value.o.c + 1));
		if ((jk != 0) && (jv != 0)) {
			node->value.o.k = jk;
			node->value.o.v = jv;
			jk[node->value.o.c] = vkey;
			jv[node->value.o.c] = value;
			node->value.o.c++;

			return true;
		} else {
			if (jk != 0) {
				for (int i = node->value.o.c - 1; i >= 0; i--) {
					json_destroy(jk[i]);
				}
				mmfree(jk);
			} else {
				for (int i = node->value.o.c - 1; i >= 0; i--) {
					json_destroy(node->value.o.k[i]);
				}
				mmfree(node->value.o.k);
			}
			node->value.o.k = 0;

			if (jv != 0) {
				for (int i = node->value.o.c - 1; i >= 0; i--) {
					json_destroy(jv[i]);
				}
				mmfree(jv);
			} else {
				for (int i = node->value.o.c - 1; i >= 0; i--) {
					json_destroy(node->value.o.v[i]);
				}
				mmfree(node->value.o.v);
			}
			node->value.o.v = 0;

			node->value.o.c = 0;
		}
	}

	return false;
}

bool json_array_add(struct json_value* node, struct json_value* value) {
	if ((node != 0) && (node->type == json_type_array)) {
		struct json_value** jv = (struct json_value**)mmrealloc(node->value.a.v, sizeof(struct json_value*) * (node->value.a.c + 1));
		if (jv != 0) {
			node->value.a.v = jv;
			jv[node->value.a.c] = value;
			node->value.a.c++;

			return true;
		} else {
			for (int i = node->value.a.c - 1; i >= 0; i--) {
				json_destroy(node->value.a.v[i]);
			}
			mmfree(node->value.a.v);
			node->value.a.c = 0;
		}
	}

	return false;
}

#include <math.h>

bool parse_json_number(struct json_value* node) {
	// (-)?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][-+]?[0-9]+)?
	uint8_t ch = *_json_ptr_;
	bool minus = false;
	if (ch == '-') {
		minus = true;
		jnext;
	}
	if (ch == 0) {
		return false; // Неожиданный конец
	} else if ((ch == 'i') || (ch == 'I')) {
		jnext;
		if ((ch == 'n') || (ch == 'N')) {
			jnext;
			if ((ch == 'f') || (ch == 'F')) {
				node->value.n.is_float = true;
				node->value.n.v.f = (minus ? -INFINITY : INFINITY);
				node->len = (intptr_t)(_json_ptr_ - node->data) + 1;
				return true;
			}
		}
		if (ch == 0) {
			return false; // Неожиданный конец
		}
		return false; // Ошибка синтаксиса
	} else if (ch == '0') {
		jnext;
		goto parse_json_number_point;
	} else if ((ch >= '1') && (ch <= '9')) {
		do {
			if (jnext == 0) {
				return false; // Неожиданный конец
			} else if ((ch < '0') || (ch > '9')) {
				break;
			}
		} while (true);
	}
parse_json_number_point:
	bool is_float = false;
	if (ch == '.') {
		is_float = true;
		jnext;
		if ((ch >= '0') && (ch <= '9')) {
			do {
				if (jnext == 0) {
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
		jnext;
		if ((ch == '-') || (ch == '+')) {
			jnext;
		}
		if ((ch >= '0') && (ch <= '9')) {
			do {
				if (jnext == 0) {
					return false; // Неожиданный конец
				} else if ((ch < '0') || (ch > '9')) {
					break;
				}
			} while (true);
		} else {
			return false; // Ошибка синтаксиса
		}
	}

	node->len = (intptr_t)(_json_ptr_ - node->data);
	*_json_ptr_ = 0;
	if (is_float) {
		node->value.n.is_float = true;
		node->value.n.v.f = atof((char*)(node->data));
	} else {
		node->value.n.is_float = false;
		node->value.n.v.i = atoll((char*)(node->data));
	}
	*_json_ptr_ = ch;

	_json_ptr_--;

	return true;
}

bool parse_json_string(struct json_value* node) {
	uint8_t ch;
	uint32_t len = 0;
	uint8_t* save = _json_ptr_;
	while (jnext != 0) {
		if (ch == '\"') {
			node->len = (intptr_t)(_json_ptr_ - node->data) + 1;
			_json_ptr_ = save;
			goto parse_json_string_alloc;
		} else if (ch & 0x80) { // found UTF8
			len++;
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
			while ((cnt > 0) && (jnext != 0)) {
				len++;
				if ((ch & 0xC0) != 0x80) {
					return false; // Not UTF8
				}
				cnt--;
			}
			if (cnt > 0) {
				if (ch == 0) {
					return false; // Неожиданный конец
				} else {
					return false; // Ошибка синтаксиса
				}
			}
		} else if (ch == '\\') {
			jnext;
			if ((ch == '\"') || (ch == '\\') || (ch == '\/') || (ch == 'b') || (ch == 'f') || (ch == 'n') || (ch == 'r') || (ch == 't')) {
				len++;
			} else if (ch == 'u') {
				uint32_t utf16 = 0;
				int cnt = 4;
				while ((cnt > 0) && (jnext != 0)) {
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
					if (ch == 0) {
						return false; // Неожиданный конец
					} else {
						return false; // Ошибка синтаксиса
					}
				}

				if ((utf16 & 0xFC00) == 0xD800) { // found UTF16
					jnext;
					if (ch == '\\') {
						jnext;
						if (ch == 'u') {
							uint32_t utf16_2 = 0;
							int cnt = 4;
							while ((cnt > 0) && (jnext != 0)) {
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
								if (ch == 0) {
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
						} else if (ch == 0) {
							return false; // Неожиданный конец
						} else {
							return false; // Ошибка синтаксиса
						}
					} else if (ch == 0) {
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
			} else if (ch == 0) {
				return false; // Неожиданный конец
			} else {
				return false; // Ошибка синтаксиса
			}
		} else {
			len++;
		}
	}
	return false; // Неожиданный конец

parse_json_string_alloc:
	uint8_t* ptr = (uint8_t*)mmalloc(len + 1);
	if (ptr == 0) {
		return false;
	}
	node->value.s = (char*)ptr;

	while (jnext != 0) {
		if (ch == '\"') {
			*ptr = 0;
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
			while ((cnt > 0) && (jnext != 0)) {
				if ((ch & 0xC0) != 0x80) {
					return false; // Not UTF8
				}
				*ptr = ch; ptr++;
				cnt--;
			}
			if (cnt > 0) {
				if (ch == 0) {
					return false; // Неожиданный конец
				} else {
					return false; // Ошибка синтаксиса
				}
			}
		} else if (ch == '\\') {
			jnext;
			if ((ch == '\"') || (ch == '\\') || (ch == '\/') || (ch == 'b') || (ch == 'f') || (ch == 'n') || (ch == 'r') || (ch == 't')) {
				*ptr = ch; ptr++;
			} else if (ch == 'u') {
				uint32_t utf16 = 0;
				int cnt = 4;
				while ((cnt > 0) && (jnext != 0)) {
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
					if (ch == 0) {
						return false; // Неожиданный конец
					} else {
						return false; // Ошибка синтаксиса
					}
				}

				if ((utf16 & 0xFC00) == 0xD800) { // found UTF16
					jnext;
					if (ch == '\\') {
						jnext;
						if (ch == 'u') {
							uint32_t utf16_2 = 0;
							int cnt = 4;
							while ((cnt > 0) && (jnext != 0)) {
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
								if (ch == 0) {
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
						} else if (ch == 0) {
							return false; // Неожиданный конец
						} else {
							return false; // Ошибка синтаксиса
						}
					} else if (ch == 0) {
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
			} else if (ch == 0) {
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

bool parse_json_object(struct json_value* node, int32_t level);
bool parse_json_array(struct json_value* node, int32_t level);

struct json_value* parse_json_value(struct json_value* parent, int32_t level) {
	uint8_t ch;
	struct json_value* node = 0;
	while (jnext != 0) {
		if (ch == '\"') {
			node = json_create(parent, json_type_string, level, _json_ptr_);
			if (node == 0) {
				return 0;
			}
			if (!parse_json_string(node)) {
				json_destroy(node);
				return 0;
			}
			return node;
		} else if ((ch == '-') || ((ch >= '0') && (ch <= '9'))) {
			node = json_create(parent, json_type_number, level, _json_ptr_);
			if (node == 0) {
				return 0;
			}
			if (!parse_json_number(node)) {
				json_destroy(node);
				return 0;
			}
			return node;
		} else if (ch == '{') {
			node = json_create(parent, json_type_object, level, _json_ptr_);
			if (node == 0) {
				return 0;
			}
			if (!parse_json_object(node, level)) {
				json_destroy(node);
				return 0;
			}
			return node;
		} else if (ch == '[') {
			node = json_create(parent, json_type_array, level, _json_ptr_);
			if (node == 0) {
				return 0;
			}
			if (!parse_json_array(node, level)) {
				json_destroy(node);
				return 0;
			}
			return node;
		} else if ((ch == 't') || (ch == 'T')) {
			jnext;
			if ((ch == 'r') || (ch == 'R')) {
				jnext;
				if ((ch == 'u') || (ch == 'U')) {
					jnext;
					if ((ch == 'e') || (ch == 'E')) {
						node = json_create(parent, json_type_boolean, level, _json_ptr_ - 3);
						if (node == 0) {
							return 0;
						}
						node->len = 4;
						node->value.b = true;
						return node;
					}
				}
			}
			if (ch == 0) {
				return 0; // Неожиданный конец
			}
			return 0; // Ошибка синтаксиса
		} else if ((ch == 'f') || (ch == 'F')) {
			jnext;
			if ((ch == 'a') || (ch == 'A')) {
				jnext;
				if ((ch == 'l') || (ch == 'L')) {
					jnext;
					if ((ch == 's') || (ch == 'S')) {
						jnext;
						if ((ch == 'e') || (ch == 'E')) {
							node = json_create(parent, json_type_boolean, level, _json_ptr_ - 4);
							if (node == 0) {
								return 0;
							}
							node->len = 5;
							node->value.b = false;
							return node;
						}
					}
				}
			}
			if (ch == 0) {
				return 0; // Неожиданный конец
			}
			return 0; // Ошибка синтаксиса
		} else if ((ch == 'n') || (ch == 'N')) {
			jnext;
			if ((ch == 'u') || (ch == 'U')) {
				jnext;
				if ((ch == 'l') || (ch == 'L')) {
					jnext;
					if ((ch == 'l') || (ch == 'L')) {
						node = json_create(parent, json_type_null, level, _json_ptr_ - 3);
						if (node == 0) {
							return 0;
						}
						node->len = 4;
						return node;
					}
				}
			} else if ((ch == 'a') || (ch == 'A')) {
				jnext;
				if ((ch == 'n') || (ch == 'N')) {
					node = json_create(parent, json_type_number, level, _json_ptr_ - 2);
					if (node == 0) {
						return 0;
					}
					node->len = 3;
					node->value.n.is_float = true;
					node->value.n.v.f = NAN;
					return node;
				}
			}
			if (ch == 0) {
				return 0; // Неожиданный конец
			}
			return 0; // Ошибка синтаксиса
		} else if ((ch == 'i') || (ch == 'I')) {
			jnext;
			if ((ch == 'n') || (ch == 'N')) {
				jnext;
				if ((ch == 'f') || (ch == 'F')) {
					node = json_create(parent, json_type_number, level, _json_ptr_ - 2);
					if (node == 0) {
						return 0;
					}
					node->len = 3;
					node->value.n.is_float = true;
					node->value.n.v.f = INFINITY;
					return node;
				}
			}
			if (ch == 0) {
				return 0; // Неожиданный конец
			}
			return 0; // Ошибка синтаксиса
		} else if (!is_space(ch)) {
			return 0; // Ошибка синтаксиса
		}
	}
	return 0; // Неожиданный конец
}

bool parse_json_array(struct json_value* node, int32_t level) {
	uint8_t ch;
	level++;
parse_json_array_repeat:
	while (jnext != 0) {
		if (ch == ']') {
			node->len = (intptr_t)(_json_ptr_ - node->data) + 1;
			return true;
		} else if (!is_space(ch)) {
			_json_ptr_--;
			struct json_value* value = parse_json_value(node, level);
			if (value == 0) {
				return false;
			}
			if (!json_array_add(node, value)) {
				json_destroy(value);
				return false;
			}
			while (jnext != 0) {
				if (ch == ',') {
					goto parse_json_array_repeat;
				} else if (ch == ']') {
					node->len = (intptr_t)(_json_ptr_ - node->data) + 1;
					return true;
				} else if (!is_space(ch)) {
					return false; // Ошибка синтаксиса
				}
			}
			return false; // Неожиданный конец
		}
	}
	return false; // Неожиданный конец
}

bool parse_json_object(struct json_value* node, int32_t level) {
	uint8_t ch;
	level++;
parse_json_object_repeat:
	while (jnext != 0) {
		if (ch == '}') {
			node->len = (intptr_t)(_json_ptr_ - node->data) + 1;
			return true;
		} else if (ch == '\"') {
			struct json_value* vkey = json_create(node, json_type_string, level, _json_ptr_);
			if (vkey == 0) {
				return false;
			}
			if (!parse_json_string(vkey)) {
				json_destroy(vkey);
				return false;
			}
			while (jnext != 0) {
				if (ch == ':') {
					struct json_value* value = parse_json_value(node, level);
					if (value == 0) {
						json_destroy(vkey);
						return false;
					}
					if (!json_object_add(node, vkey, value)) {
						json_destroy(vkey);
						json_destroy(value);
						return false;
					}
					while (jnext != 0) {
						if (ch == ',') {
							goto parse_json_object_repeat;
						} else if (ch == '}') {
							node->len = (intptr_t)(_json_ptr_ - node->data) + 1;
							return true;
						} else if (!is_space(ch)) {
							return false; // Ошибка синтаксиса
						}
					}
					return false; // Неожиданный конец
				} else if (!is_space(ch)) {
					return false; // Ошибка синтаксиса
				}
			}
			return false; // Неожиданный конец
		} else if (!is_space(ch)) {
			return false; // Ошибка синтаксиса
		}
	}
	return false; // Неожиданный конец
}

struct json_value* json_parse(const char* json) {
	_json_ptr_ = (uint8_t*)json;
	uint8_t ch;
	while ((ch = *_json_ptr_) != 0) {
		if (is_space(ch)) {
			_json_ptr_++;
		} else if (ch == '{') {
			json_value* node = json_create(0, json_type_object, 0, _json_ptr_);
			if (node == 0) {
				return 0;
			}
			if (!parse_json_object(node, 0)) {
				json_destroy(node);
				return 0;
			}
			while (jnext != 0) {
				if (!is_space(ch)) {
					json_destroy(node);
					return 0; // Ошибка синтаксиса
				}
			}
			return node;
		} else {
			return 0; // Ошибка синтаксиса
		}
	}
	return 0; // Неожиданный конец
}

#include <stdio.h>

void json_debug_tab(uint32_t level) {
	for (int i = 0; i < level; i++) {
		printf(" ");
	}
}

void json_debug(const struct json_value* j) {
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

struct json_value* json_get(const struct json_value* node, uint32_t index) {
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
	return 0;
}

struct json_value* json_get(const struct json_value* node, const char* key) {
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

struct json_value* json_geti(const struct json_value* node, const char* key) {
	if (node != 0) {
		if (node->type == json_type_object) {
			for (int i = node->value.o.c - 1; i >= 0; i--) {
				if (node->value.o.k[i]->type == json_type_string) {
					if (strcmpi(node->value.o.k[i]->value.s, key) == 0) {
						return node->value.o.v[i];
					}
				}
			}
		}
	}
	return 0;
}

const char* json_key(const struct json_value* value, const char* def) {
	if ((value != 0) && (value->parent != 0) && (value->parent->type == json_type_object)) {
		for (int i = value->parent->value.o.c - 1; i >= 0; i--) {
			if (value->parent->value.o.v[i] == value) {
				return value->parent->value.o.k[i]->value.s;
			}
		}
	}
	return def;
}

const char* json_string(const struct json_value* value, const char* def) {
	if (value != 0) {
		if (value->type == json_type_string) {
			return value->value.s;
		}
	}
	return def;
}
