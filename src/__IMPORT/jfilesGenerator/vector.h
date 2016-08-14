#ifndef vector_H
#define vector_H

#include <stdint.h>

typedef struct vec2;
typedef struct vec3;
typedef struct vec4;
typedef struct mat2;
typedef struct mat3;
typedef struct mat4;

double abs(double v);
vec2 abs(const vec2& v);
vec3 abs(const vec3& v);
vec4 abs(const vec4& v);

vec2 floor(const vec2& v);
vec3 floor(const vec3& v);
vec4 floor(const vec4& v);

double fract(double v);
vec2 fract(const vec2& v);
vec3 fract(const vec3& v);
vec4 fract(const vec4& v);

double mod(double v, double m);
vec2 mod(const vec2& v, double m);
vec3 mod(const vec3& v, double m);
vec4 mod(const vec4& v, double m);
vec2 mod(const vec2& v, const vec2& m);
vec3 mod(const vec3& v, const vec3& m);
vec4 mod(const vec4& v, const vec4& m);

double clamp(double v, double vmin, double vmax);
vec2 clamp(const vec2& v, double vmin, double vmax);
vec3 clamp(const vec3& v, double vmin, double vmax);
vec4 clamp(const vec4& v, double vmin, double vmax);

double length(const vec2& v);
double length(const vec3& v);
double length(const vec4& v);

double smoothstep(double vmin, double vmax, double v);
vec2 smoothstep(double vmin, double vmax, const vec2& v);
vec3 smoothstep(double vmin, double vmax, const vec3& v);
vec4 smoothstep(double vmin, double vmax, const vec4& v);

double atan(double v1, double v2);

typedef struct vec2 {
	union {
		double x;
		double u;
		double s;
		double r;
	};

	union {
		double y;
		double v;
		double t;
		double g;
	};

	vec2();
	vec2(double v);
	vec2(double x, double y);
	vec2(const vec2& v);
	vec2(const vec3& v);
	vec2(const vec4& v);
	vec2& operator =(double v);
	vec2& operator =(const vec2& v);
	double& operator [](int index);

	vec2 operator -(double v) const;
	vec2 operator +(double v) const;
	vec2 operator *(double v) const;
	vec2 operator /(double v) const;
	vec2 operator -(const vec2& v) const;
	vec2 operator +(const vec2& v) const;
	vec2 operator *(const vec2& v) const;
	vec2 operator /(const vec2& v) const;

	vec2 operator *(const mat2& m) const;
} vec2;

typedef struct vec3 {
	union {
		double x;
		double s;
		double r;
	};

	union {
		double y;
		double t;
		double g;
	};

	union {
		double z;
		double p;
		double b;
	};

	vec3();
	vec3(double v);
	vec3(double x, double y, double z);
	vec3(const vec2& v, double z);
	vec3(double x, const vec2& v);
	vec3(const vec3& v);
	vec3(const vec4& v);
	vec3& operator =(double v);
	vec3& operator =(const vec3& v);
	double& operator [](int index);

	vec3 operator -(double v) const;
	vec3 operator +(double v) const;
	vec3 operator *(double v) const;
	vec3 operator /(double v) const;
	vec3 operator -(const vec3& v) const;
	vec3 operator +(const vec3& v) const;
	vec3 operator *(const vec3& v) const;
	vec3 operator /(const vec3& v) const;

	vec3 operator *(const mat3& m) const;
	vec3 operator *(const mat4& m) const;
} vec3;

typedef struct vec4 {
	union {
		double x;
		double s;
		double r;
	};

	union {
		double y;
		double t;
		double g;
	};

	union {
		double z;
		double p;
		double b;
	};

	union {
		double w;
		double q;
		double a;
	};

	vec4();
	vec4(double v);
	vec4(double x, double y, double z, double w);
	vec4(const vec2& v, double z, double w);
	vec4(double x, const vec2& v, double w);
	vec4(double x, double y, const vec2& v);
	vec4(const vec2& v1, const vec2& v2);
	vec4(double x, const vec3& v);
	vec4(const vec3& v, double w);
	vec4(const vec4& v);
	vec4& operator =(double v);
	vec4& operator =(const vec4& v);
	double& operator [](int index);

	vec4 operator -(double v) const;
	vec4 operator +(double v) const;
	vec4 operator *(double v) const;
	vec4 operator /(double v) const;
	vec4 operator -(const vec4& v) const;
	vec4 operator +(const vec4& v) const;
	vec4 operator *(const vec4& v) const;
	vec4 operator /(const vec4& v) const;

	operator uint32_t() const;
} vec4;

typedef struct mat2 {
	double m[4];

	mat2();
	mat2(double v);
	mat2(double v0, double v1, double v2, double v3);
	mat2(const vec2& v0, const vec2& v1);
	mat2(const mat2& m);
	mat2(const mat3& m);
	mat2(const mat4& m);
	mat2& operator =(double v);
	mat2& operator =(const mat2& m);
	mat2& operator =(const mat3& m);
	mat2& operator =(const mat4& m);

	double& operator [](int index);

	mat2 operator *(const mat2& v);
} mat2;

typedef struct mat3 {
	double m[9];

	mat3();
	mat3(double v);
	mat3(double v0, double v1, double v2, double v3, double v4, double v5, double v6, double v7, double v8);
	mat3(const vec3& v0, const vec3& v1, const vec3& v2);
	mat3(const mat3& m);
	mat3(const mat4& m);
	mat3& operator =(double v);
	mat3& operator =(const mat3& m);
	mat3& operator =(const mat4& m);

	double& operator [](int index);

	mat3 operator *(const mat3& v);
} mat3;

typedef struct mat4 {
	double m[16];

	mat4();
	mat4(double v);
	mat4(double v0, double v1, double v2, double v3, double v4, double v5, double v6, double v7, double v8, double v9, double v10, double v11, double v12, double v13, double v14, double v15);
	mat4(const vec4& v0, const vec4& v1, const vec4& v2, const vec4& v3);
	mat4(const mat4& m);
	mat4& operator =(double v);
	mat4& operator =(const mat4& m);

	double& operator [](int index);

	mat4 operator *(const mat4& v);
} mat4;

#endif
