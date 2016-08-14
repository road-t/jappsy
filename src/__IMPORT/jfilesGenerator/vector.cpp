#include "vector.h"
#include <math.h>

double abs(double v) {
	return fabs(v);
}

vec2 abs(const vec2& v) {
	return vec2(
		fabs(v.x),
		fabs(v.y)
	);
}

vec3 abs(const vec3& v) {
	return vec3(
		fabs(v.x),
		fabs(v.y),
		fabs(v.z)
	);
}

vec4 abs(const vec4& v) {
	return vec4(
		fabs(v.x),
		fabs(v.y),
		fabs(v.z),
		fabs(v.w)
	);
}

vec2 floor(const vec2& v) {
	return vec2(
		floor(v.x),
		floor(v.y)
	);
}

vec3 floor(const vec3& v) {
	return vec3(
		floor(v.x),
		floor(v.y),
		floor(v.z)
	);
}

vec4 floor(const vec4& v) {
	return vec4(
		floor(v.x),
		floor(v.y),
		floor(v.z),
		floor(v.w)
	);
}

double fract(double v) {
	return v - floor(v);
}

vec2 fract(const vec2& v) {
	return vec2(
		fract(v.x),
		fract(v.y)
	);
}

vec3 fract(const vec3& v) {
	return vec3(
		fract(v.x),
		fract(v.y),
		fract(v.z)
	);
}

vec4 fract(const vec4& v) {
	return vec4(
		fract(v.x),
		fract(v.y),
		fract(v.z),
		fract(v.w)
	);
}

double mod(double v, double m) {
	if (m == 0.0) return 0.0;
	return fract(v / m) * m;
}

vec2 mod(const vec2& v, double m) {
	return vec2(
		mod(v.x, m),
		mod(v.y, m)
	);
}

vec3 mod(const vec3& v, double m) {
	return vec3(
		mod(v.x, m),
		mod(v.y, m),
		mod(v.z, m)
	);
}

vec4 mod(const vec4& v, double m) {
	return vec4(
		mod(v.x, m),
		mod(v.y, m),
		mod(v.z, m),
		mod(v.w, m)
	);
}

vec2 mod(const vec2& v, const vec2& m) {
	return vec2(
		mod(v.x, m.x),
		mod(v.y, m.y)
	);
}

vec3 mod(const vec3& v, const vec3& m) {
	return vec3(
		mod(v.x, m.x),
		mod(v.y, m.y),
		mod(v.z, m.z)
	);
}

vec4 mod(const vec4& v, const vec4& m) {
	return vec4(
		mod(v.x, m.x),
		mod(v.y, m.y),
		mod(v.z, m.z),
		mod(v.w, m.w)
	);
}

double clamp(double v, double vmin, double vmax) {
	if (v < vmin) return vmin;
	else if (v > vmax) return vmax;
	return v;
}

vec2 clamp(const vec2& v, double vmin, double vmax) {
	return vec2(
		clamp(v.x, vmin, vmax),
		clamp(v.y, vmin, vmax)
	);
}

vec3 clamp(const vec3& v, double vmin, double vmax) {
	return vec3(
		clamp(v.x, vmin, vmax),
		clamp(v.y, vmin, vmax),
		clamp(v.z, vmin, vmax)
	);
}

vec4 clamp(const vec4& v, double vmin, double vmax) {
	return vec4(
		clamp(v.x, vmin, vmax),
		clamp(v.y, vmin, vmax),
		clamp(v.z, vmin, vmax),
		clamp(v.w, vmin, vmax)
	);
}

double length(const vec2& v) {
	return sqrt(v.x*v.x + v.y*v.y);
}

double length(const vec3& v) {
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

double length(const vec4& v) {
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

vec4::operator uint32_t() const {
	uint32_t r = (uint32_t)round(clamp(this->r, 0.0, 1.0) * 255.0);
	uint32_t g = (uint32_t)round(clamp(this->g, 0.0, 1.0) * 255.0);
	uint32_t b = (uint32_t)round(clamp(this->b, 0.0, 1.0) * 255.0);
	uint32_t a = (uint32_t)round(clamp(this->a, 0.0, 1.0) * 255.0);
	return (a<<24) | (r<<16) | (g<<8) | b;
}

double smoothstep(double vmin, double vmax, double v) {
	if (v <= vmin) return 0.0;
	else if (v >= vmax) return 1.0;
	return (v - vmin) / (vmax - vmin);
}

vec2 smoothstep(double vmin, double vmax, const vec2& v) {
	return vec2(
		smoothstep(vmin, vmax, v.x),
		smoothstep(vmin, vmax, v.y)
	);
}

vec3 smoothstep(double vmin, double vmax, const vec3& v) {
	return vec3(
		smoothstep(vmin, vmax, v.x),
		smoothstep(vmin, vmax, v.y),
		smoothstep(vmin, vmax, v.z)
	);
}

vec4 smoothstep(double vmin, double vmax, const vec4& v) {
	return vec4(
		smoothstep(vmin, vmax, v.x),
		smoothstep(vmin, vmax, v.y),
		smoothstep(vmin, vmax, v.z),
		smoothstep(vmin, vmax, v.w)
	);
}

double atan(double v1, double v2) {
	return atan2(v1, v2);
}

///////////////

vec2::vec2() {
	this->x = this->y = 0.0;
}

vec2::vec2(double v) {
	this->x = this->y = v;
}

vec2::vec2(double x, double y) {
	this->x = x;
	this->y = y;
}

vec2::vec2(const vec2& v) {
	this->x = v.x;
	this->y = v.y;
}

vec2::vec2(const vec3& v) {
	this->x = v.x;
	this->y = v.y;
}

vec2::vec2(const vec4& v) {
	this->x = v.x;
	this->y = v.y;
}

vec2& vec2::operator =(double v) {
	this->x = this->y = v;
	return *this;
}

vec2& vec2::operator =(const vec2& v) {
	this->x = v.x;
	this->y = v.y;
	return *this;
}

double& vec2::operator [](int index) {
	if ((index < 0) || (index >= 2)) return this->x;
	return ((double*)this)[index];
}

vec2 vec2::operator -(double v) const {
	return vec2(this->x - v, this->y - v);
}

vec2 vec2::operator +(double v) const {
	return vec2(this->x + v, this->y + v);
}

vec2 vec2::operator *(double v) const {
	return vec2(this->x * v, this->y * v);
}

vec2 vec2::operator /(double v) const {
	if (v == 0.0)
		return vec2();
	return vec2(this->x / v, this->y / v);
}

vec2 vec2::operator -(const vec2& v) const {
	return vec2(this->x - v.x, this->y - v.y);
}

vec2 vec2::operator +(const vec2& v) const {
	return vec2(this->x + v.x, this->y + v.y);
}

vec2 vec2::operator *(const vec2& v) const {
	return vec2(this->x * v.x, this->y * v.y);
}

vec2 vec2::operator /(const vec2& v) const {
	return vec2(
		(v.x == 0.0 ? 0.0 : this->x / v.x),
		(v.y == 0.0 ? 0.0 : this->y / v.y)
	);
}

vec3::vec3() {
	this->x = this->y = this->z = 0.0;
}

vec3::vec3(double v) {
	this->x = this->y = this->z = v;
}

vec3::vec3(double x, double y, double z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

vec3::vec3(const vec2& v, double z) {
	this->x = v.x;
	this->y = v.y;
	this->z = z;
}

vec3::vec3(double x, const vec2& v) {
	this->x = x;
	this->y = v.x;
	this->z = v.y;
}

vec3::vec3(const vec3& v) {
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
}

vec3::vec3(const vec4& v) {
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
}

vec3& vec3::operator =(double v) {
	this->x = this->y = this->z = v;
	return *this;
}

vec3& vec3::operator =(const vec3& v) {
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
	return *this;
}

double& vec3::operator [](int index) {
	if ((index < 0) || (index >= 3)) return this->x;
	return ((double*)this)[index];
}

vec3 vec3::operator -(double v) const {
	return vec3(this->x - v, this->y - v, this->z - v);
}

vec3 vec3::operator +(double v) const {
	return vec3(this->x + v, this->y + v, this->z + v);
}

vec3 vec3::operator *(double v) const {
	return vec3(this->x * v, this->y * v, this->z * v);
}

vec3 vec3::operator /(double v) const {
	if (v == 0.0)
		return vec3();
	return vec3(this->x / v, this->y / v, this->z / v);
}

vec3 vec3::operator -(const vec3& v) const {
	return vec3(this->x - v.x, this->y - v.y, this->z - v.z);
}

vec3 vec3::operator +(const vec3& v) const {
	return vec3(this->x + v.x, this->y + v.y, this->z + v.z);
}

vec3 vec3::operator *(const vec3& v) const {
	return vec3(this->x * v.x, this->y * v.y, this->z * v.z);
}

vec3 vec3::operator /(const vec3& v) const {
	return vec3(
		(v.x == 0.0 ? 0.0 : this->x / v.x),
		(v.y == 0.0 ? 0.0 : this->y / v.y),
		(v.z == 0.0 ? 0.0 : this->z / v.z)
	);
}

vec4::vec4() {
	this->x = this->y = this->z = this->w = 0.0;
}

vec4::vec4(double v) {
	this->x = this->y = this->z = this->w = v;
}

vec4::vec4(double x, double y, double z, double w) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

vec4::vec4(const vec2& v, double z, double w) {
	this->x = v.x;
	this->y = v.y;
	this->z = z;
	this->w = w;
}

vec4::vec4(double x, const vec2& v, double w) {
	this->x = x;
	this->y = v.x;
	this->z = v.y;
	this->w = w;
}

vec4::vec4(double x, double y, const vec2& v) {
	this->x = x;
	this->y = y;
	this->z = v.x;
	this->w = v.y;
}

vec4::vec4(const vec2& v1, const vec2& v2) {
	this->x = v1.x;
	this->y = v1.y;
	this->z = v2.x;
	this->w = v2.y;
}

vec4::vec4(double x, const vec3& v) {
	this->x = x;
	this->y = v.x;
	this->z = v.y;
	this->w = v.z;
}

vec4::vec4(const vec3& v, double w) {
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
	this->w = w;
}

vec4::vec4(const vec4& v) {
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
	this->w = v.w;
}

vec4& vec4::operator =(double v) {
	this->x = this->y = this->z = this->w = v;
	return *this;
}

vec4& vec4::operator =(const vec4& v) {
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
	this->w = v.w;
	return *this;
}

double& vec4::operator [](int index) {
	if ((index < 0) || (index >= 4)) return this->x;
	return ((double*)this)[index];
}

vec4 vec4::operator -(double v) const {
	return vec4(
		this->x - v,
		this->y - v,
		this->z - v,
		this->w - v
	);
}

vec4 vec4::operator +(double v) const {
	return vec4(
		this->x + v,
		this->y + v,
		this->z + v,
		this->w + v
	);
}

vec4 vec4::operator *(double v) const {
	return vec4(
		this->x * v,
		this->y * v,
		this->z * v,
		this->w * v
	);
}

vec4 vec4::operator /(double v) const {
	if (v == 0.0)
		return vec4();
	return vec4(this->x / v, this->y / v, this->z / v, this->w / v);
}

vec4 vec4::operator -(const vec4& v) const {
	return vec4(
		this->x - v.x,
		this->y - v.y,
		this->z - v.z,
		this->w - v.w
	);
}

vec4 vec4::operator +(const vec4& v) const {
	return vec4(
		this->x + v.x,
		this->y + v.y,
		this->z + v.z,
		this->w + v.w
	);
}

vec4 vec4::operator *(const vec4& v) const {
	return vec4(
		this->x * v.x,
		this->y * v.y,
		this->z * v.z,
		this->w * v.w
	);
}

vec4 vec4::operator /(const vec4& v) const {
	return vec4(
		(v.x == 0.0 ? 0.0 : this->x / v.x),
		(v.y == 0.0 ? 0.0 : this->y / v.y),
		(v.z == 0.0 ? 0.0 : this->z / v.z),
		(v.w == 0.0 ? 0.0 : this->w / v.w)
	);
}

mat2::mat2() {
	this->m[0] = this->m[3] = 1.0;
	this->m[1] = this->m[2] = 0.0;
}

mat2::mat2(double v) {
	this->m[0] = this->m[1] =
	this->m[2] = this->m[3] = v;
}

mat2::mat2(double v0, double v1, double v2, double v3) {
	this->m[0] = v0; this->m[1] = v1;
	this->m[2] = v2; this->m[3] = v3;
}

mat2::mat2(const vec2& v0, const vec2& v1) {
	this->m[0] = v0.x; this->m[1] = v0.y;
	this->m[2] = v1.x; this->m[3] = v1.y;
}

mat2::mat2(const mat2& m) {
	this->m[0] = m.m[0]; this->m[1] = m.m[1];
	this->m[2] = m.m[2]; this->m[3] = m.m[3];
}

mat2::mat2(const mat3& m) {
	this->m[0] = m.m[0]; this->m[1] = m.m[1];
	this->m[2] = m.m[3]; this->m[3] = m.m[4];
}

mat2::mat2(const mat4& m) {
	this->m[0] = m.m[0]; this->m[1] = m.m[1];
	this->m[2] = m.m[4]; this->m[3] = m.m[5];
}

mat2& mat2::operator =(double v) {
	this->m[0] = this->m[1] =
	this->m[2] = this->m[3] = v;
	return *this;
}

mat2& mat2::operator =(const mat2& m) {
	this->m[0] = m.m[0]; this->m[1] = m.m[1];
	this->m[2] = m.m[2]; this->m[3] = m.m[3];
	return *this;
}

mat2& mat2::operator =(const mat3& m) {
	this->m[0] = m.m[0]; this->m[1] = m.m[1];
	this->m[2] = m.m[3]; this->m[3] = m.m[4];
	return *this;
}

mat2& mat2::operator =(const mat4& m) {
	this->m[0] = m.m[0]; this->m[1] = m.m[1];
	this->m[2] = m.m[4]; this->m[3] = m.m[5];
	return *this;
}

double& mat2::operator [](int index) {
	if ((index < 0) || (index >= 4)) return this->m[0];
	return this->m[index];
}

mat3::mat3() {
	this->m[0] = this->m[4] = this->m[8] = 1.0;
	this->m[1] = this->m[2] = this->m[3] =
	this->m[5] = this->m[6] = this->m[7] = 0.0;
}

mat3::mat3(double v) {
	this->m[0] = this->m[1] = this->m[2] =
	this->m[3] = this->m[4] = this->m[5] =
	this->m[6] = this->m[7] = this->m[8] = v;
}

mat3::mat3(double v0, double v1, double v2, double v3, double v4, double v5, double v6, double v7, double v8) {
	this->m[0] = v0; this->m[1] = v1; this->m[2] = v2;
	this->m[3] = v3; this->m[4] = v4; this->m[5] = v5;
	this->m[6] = v6; this->m[7] = v7; this->m[8] = v8;
}

mat3::mat3(const vec3& v0, const vec3& v1, const vec3& v2) {
	this->m[0] = v0.x; this->m[1] = v0.y; this->m[2] = v0.z;
	this->m[3] = v1.x; this->m[4] = v1.y; this->m[5] = v1.z;
	this->m[6] = v2.x; this->m[7] = v2.y; this->m[8] = v2.z;
}

mat3::mat3(const mat3& m) {
	this->m[0] = m.m[0]; this->m[1] = m.m[1]; this->m[2] = m.m[2];
	this->m[3] = m.m[3]; this->m[4] = m.m[4]; this->m[5] = m.m[5];
	this->m[6] = m.m[6]; this->m[7] = m.m[7]; this->m[8] = m.m[8];
}

mat3::mat3(const mat4& m) {
	this->m[0] = m.m[0]; this->m[1] = m.m[1]; this->m[2] = m.m[2];
	this->m[3] = m.m[4]; this->m[4] = m.m[5]; this->m[5] = m.m[6];
	this->m[6] = m.m[8]; this->m[7] = m.m[9]; this->m[8] = m.m[10];
}

double& mat3::operator [](int index) {
	if ((index < 0) || (index >= 9)) return this->m[0];
	return this->m[index];
}

mat3& mat3::operator =(double v) {
	this->m[0] = this->m[1] = this->m[2] =
	this->m[3] = this->m[4] = this->m[5] =
	this->m[6] = this->m[7] = this->m[8] = v;
	return *this;
}

mat3& mat3::operator =(const mat3& m) {
	this->m[0] = m.m[0]; this->m[1] = m.m[1]; this->m[2] = m.m[2];
	this->m[3] = m.m[3]; this->m[4] = m.m[4]; this->m[5] = m.m[5];
	this->m[6] = m.m[6]; this->m[7] = m.m[7]; this->m[8] = m.m[8];
	return *this;
}

mat3& mat3::operator =(const mat4& m) {
	this->m[0] = m.m[0]; this->m[1] = m.m[1]; this->m[2] = m.m[2];
	this->m[3] = m.m[4]; this->m[4] = m.m[5]; this->m[5] = m.m[6];
	this->m[6] = m.m[8]; this->m[7] = m.m[9]; this->m[8] = m.m[10];
	return *this;
}

mat4::mat4() {
	this->m[0] = this->m[5] = this->m[10] = this->m[15] = 1.0;
	this->m[1] = this->m[2] = this->m[3] = this->m[4] =
	this->m[6] = this->m[7] = this->m[8] = this->m[9] =
	this->m[11] = this->m[12] = this->m[13] = this->m[14] = 0.0;
}

mat4::mat4(double v) {
	this->m[0] = this->m[1] = this->m[2] = this->m[3] =
	this->m[4] = this->m[5] = this->m[6] = this->m[7] =
	this->m[8] = this->m[9] = this->m[10] = this->m[11] =
	this->m[12] = this->m[13] = this->m[14] = this->m[15] = v;
}

mat4::mat4(double v0, double v1, double v2, double v3, double v4, double v5, double v6, double v7, double v8, double v9, double v10, double v11, double v12, double v13, double v14, double v15) {
	this->m[0] = v0; this->m[1] = v1; this->m[2] = v2; this->m[3] = v3;
	this->m[4] = v4; this->m[5] = v5; this->m[6] = v6; this->m[7] = v7;
	this->m[8] = v8; this->m[9] = v9; this->m[10] = v10; this->m[11] = v11;
	this->m[12] = v12; this->m[13] = v13; this->m[14] = v14; this->m[15] = v15;
}

mat4::mat4(const vec4& v0, const vec4& v1, const vec4& v2, const vec4& v3) {
	this->m[0] = v0.x; this->m[1] = v0.y; this->m[2] = v0.z; this->m[3] = v0.w;
	this->m[4] = v1.x; this->m[5] = v1.y; this->m[6] = v1.z; this->m[7] = v1.w;
	this->m[8] = v2.x; this->m[9] = v2.y; this->m[10] = v2.z; this->m[11] = v2.w;
	this->m[12] = v3.x; this->m[13] = v3.y; this->m[14] = v3.z; this->m[15] = v3.w;
}

mat4::mat4(const mat4& m) {
	this->m[0] = m.m[0]; this->m[1] = m.m[1]; this->m[2] = m.m[2]; this->m[3] = m.m[3];
	this->m[4] = m.m[4]; this->m[5] = m.m[5]; this->m[6] = m.m[6]; this->m[7] = m.m[7];
	this->m[8] = m.m[8]; this->m[9] = m.m[9]; this->m[10] = m.m[10]; this->m[11] = m.m[11];
	this->m[12] = m.m[12]; this->m[13] = m.m[13]; this->m[14] = m.m[14]; this->m[15] = m.m[15];
}

mat4& mat4::operator =(double v) {
	this->m[0] = this->m[1] = this->m[2] = this->m[3] =
	this->m[4] = this->m[5] = this->m[6] = this->m[7] =
	this->m[8] = this->m[9] = this->m[10] = this->m[11] =
	this->m[12] = this->m[13] = this->m[14] = this->m[15] = v;
	return *this;
}

mat4& mat4::operator =(const mat4& m) {
	this->m[0] = m.m[0]; this->m[1] = m.m[1]; this->m[2] = m.m[2]; this->m[3] = m.m[3];
	this->m[4] = m.m[4]; this->m[5] = m.m[5]; this->m[6] = m.m[6]; this->m[7] = m.m[7];
	this->m[8] = m.m[8]; this->m[9] = m.m[9]; this->m[10] = m.m[10]; this->m[11] = m.m[11];
	this->m[12] = m.m[12]; this->m[13] = m.m[13]; this->m[14] = m.m[14]; this->m[15] = m.m[15];
	return *this;
}

double& mat4::operator [](int index) {
	if ((index < 0) || (index >= 16)) return this->m[0];
	return this->m[index];
}

mat2 mat2::operator *(const mat2& v) {
	double* a = (double*)(this);
	double* b = (double*)(&v);
	mat2 d;

	d[0]=a[0]*b[0]+a[2]*b[1];
	d[1]=a[1]*b[0]+a[3]*b[1];

	d[2]=a[0]*b[2]+a[2]*b[3];
	d[3]=a[1]*b[2]+a[3]*b[3];

	return d;
}

mat3 mat3::operator *(const mat3& v) {
	double* a = (double*)(this);
	double* b = (double*)(&v);
	mat3 d;

	d[0]=a[0]*b[0]+a[3]*b[1]+a[6]*b[2];
	d[1]=a[1]*b[0]+a[4]*b[1]+a[7]*b[2];
	d[2]=a[2]*b[0]+a[5]*b[1]+a[8]*b[2];

	d[3]=a[0]*b[3]+a[3]*b[4]+a[6]*b[5];
	d[4]=a[1]*b[3]+a[4]*b[4]+a[7]*b[5];
	d[5]=a[2]*b[3]+a[5]*b[4]+a[8]*b[5];

	d[6]=a[0]*b[6]+a[3]*b[7]+a[6]*b[8];
	d[7]=a[1]*b[6]+a[4]*b[7]+a[7]*b[8];
	d[8]=a[2]*b[6]+a[5]*b[7]+a[8]*b[8];

	return d;
}

mat4 mat4::operator *(const mat4& v) {
	double* a = (double*)(this);
	double* b = (double*)(&v);
	mat4 d;

	d[0]=a[0]*b[0]+a[4]*b[1]+a[8]*b[2]+a[12]*b[3];
	d[1]=a[1]*b[0]+a[5]*b[1]+a[9]*b[2]+a[13]*b[3];
	d[2]=a[2]*b[0]+a[6]*b[1]+a[10]*b[2]+a[14]*b[3];
	d[3]=a[3]*b[0]+a[7]*b[1]+a[11]*b[2]+a[15]*b[3];

	d[4]=a[0]*b[4]+a[4]*b[5]+a[8]*b[6]+a[12]*b[7];
	d[5]=a[1]*b[4]+a[5]*b[5]+a[9]*b[6]+a[13]*b[7];
	d[6]=a[2]*b[4]+a[6]*b[5]+a[10]*b[6]+a[14]*b[7];
	d[7]=a[3]*b[4]+a[7]*b[5]+a[11]*b[6]+a[15]*b[7];

	d[8]=a[0]*b[8]+a[4]*b[9]+a[8]*b[10]+a[12]*b[11];
	d[9]=a[1]*b[8]+a[5]*b[9]+a[9]*b[10]+a[13]*b[11];
	d[10]=a[2]*b[8]+a[6]*b[9]+a[10]*b[10]+a[14]*b[11];
	d[11]=a[3]*b[8]+a[7]*b[9]+a[11]*b[10]+a[15]*b[11];

	d[12]=a[0]*b[12]+a[4]*b[13]+a[8]*b[14]+a[12]*b[15];
	d[13]=a[1]*b[12]+a[5]*b[13]+a[9]*b[14]+a[13]*b[15];
	d[14]=a[2]*b[12]+a[6]*b[13]+a[10]*b[14]+a[14]*b[15];
	d[15]=a[3]*b[12]+a[7]*b[13]+a[11]*b[14]+a[15]*b[15];

	return d;
}

vec2 vec2::operator *(const mat2& m) const {
	double* v = (double*)(this);
	vec2 d;

	d[0]=m.m[0]*v[0]+m.m[1]*v[1];
	d[1]=m.m[2]*v[0]+m.m[3]*v[1];

	return d;
}

vec3 vec3::operator *(const mat3& m) const {
	double* v = (double*)(this);
	vec3 d;

	d[0]=m.m[0]*v[0]+m.m[1]*v[1]+m.m[2]*v[2];
	d[1]=m.m[3]*v[0]+m.m[4]*v[1]+m.m[5]*v[2];
	d[2]=m.m[6]*v[0]+m.m[7]*v[1]+m.m[8]*v[2];

	return d;
}

vec3 vec3::operator *(const mat4& m) const {
	double* p = (double*)(this);
	vec3 d;

	double s = m.m[12]*p[0]+m.m[13]*p[1]+m.m[14]*p[2]+m.m[15];
	if (s != 0) {
		d[0]=(m.m[0]*p[0]+m.m[1]*p[1]+m.m[2]*p[2]+m.m[3])/s;
		d[1]=(m.m[4]*p[0]+m.m[5]*p[1]+m.m[6]*p[2]+m.m[7])/s;
		d[2]=(m.m[8]*p[0]+m.m[9]*p[1]+m.m[10]*p[2]+m.m[11])/s;
	}

	return d;
}
