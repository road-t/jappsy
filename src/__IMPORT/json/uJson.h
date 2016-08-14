#ifndef UJSON_H
#define UJSON_H

#include <stdint.h>

bool json_check(const char* json);

enum json_type {
	json_type_null = 0,
	json_type_object = 1,
	json_type_array = 2,
	json_type_string = 3,
	json_type_number = 4,
	json_type_boolean = 5
};

struct json_value {
	json_value*	parent;
	json_type	type;
	int32_t		level;
	uint8_t*	data;
	intptr_t	len;

	union {
		struct {
			json_value** k;
			json_value** v;
			uint32_t 	 c;
		} o;

		struct {
			json_value** v;
			uint32_t	 c;
		} a;

		char*	s;

		struct {
			bool is_float;
			union {
				int64_t	 i;
				double	 f;
			} v;
		} n;

		bool	b;
	} value;
};

struct json_value* json_create(struct json_value* parent, json_type type, uint32_t level, const uint8_t* data);
void json_destroy(struct json_value* j);
bool json_object_add(struct json_value* node, struct json_value* vkey, struct json_value* value);
bool json_array_add(struct json_value* node, struct json_value* value);

struct json_value* json_parse(const char* json);
void json_debug(const struct json_value* j);

struct json_value* json_get(const struct json_value* node, uint32_t index);
struct json_value* json_get(const struct json_value* node, const char* key);
struct json_value* json_geti(const struct json_value* node, const char* key);

const char* json_key(const struct json_value* value, const char* def = 0);
const char* json_string(const struct json_value* value, const char* def = 0);

#endif
