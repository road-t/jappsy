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

#ifndef JAPPSY_UVECTOR_H
#define JAPPSY_UVECTOR_H

#include <core/uMemory.h>

#ifdef __cplusplus
extern "C" {
#endif
	
	void Vec3Set(float* d, float v);
	void Vec4Set(float* d, float v);
	void Vec3SetV(float* d, const float* v);
	void Vec4SetV(float* d, const float* v);
	void Vec3Negative(float* d, const float* s);
	void Vec4Negative(float* d, const float* s);
	void Vec3Add(float* d, const float* a, float b);
	void Vec4Add(float* d, const float* a, float b);
	void Vec3AddV(float* d, const float* a, const float* b);
	void Vec4AddV(float* d, const float* a, const float* b);
	void Vec3Subtract(float* d, const float* a, float b);
	void Vec4Subtract(float* d, const float* a, float b);
	void Vec3SubtractV(float* d, const float* a, const float* b);
	void Vec4SubtractV(float* d, const float* a, const float* b);
	void Vec3Multiply(float* d, const float* a, float b);
	void Vec4Multiply(float* d, const float* a, float b);
	void Vec3MultiplyV(float* d, const float* a, const float* b);
	void Vec4MultiplyV(float* d, const float* a, const float* b);
	void Vec3Divide(float* d, const float* a, float b);
	void Vec4Divide(float* d, const float* a, float b);
	void Vec3DivideV(float* d, const float* a, const float* b);
	void Vec4DivideV(float* d, const float* a, const float* b);
	float Vec3Dot(const float* a, const float* b);
	float Vec4Dot(const float* a, const float* b);
	void Vec3Cross(float* d, const float* a, const float* b);
	float Vec3Size(const float* v);
	void Vec3Normalize(float* d, const float* v);
	void Vec4Normalize(float* d, const float* v);
	float Vec3Min(const float* v);
	float Vec3Max(const float* v);
	void Vec3Angles(const float* v, float* theta, float* phi);
	float Vec3Angle(const float* a, const float* b);
	void Vec3Transform(float* d, const float* p, const float* m);
	void Vec3TransformNormal(float* d, const float* v, const float* m);
	void Vec3Random(float* d, float x, float y, float z);
	
	void Mat4Set(float* d, float v);
	void Mat4SetV(float* d, const float* m);
	void Mat4Identity(float* d);
	void Mat4Translate(float* d, float x, float y, float z);
	void Mat4Scale(float* d, float x, float y, float z);
	void Mat4RotateX(float* d, float a);
	void Mat4RotateY(float* d, float a);
	void Mat4RotateZ(float* d, float a);
	void Mat4Frustum(float* d, float l, float r, float b, float t, float n, float f);
	void Mat4Perspective(float* d, float fov, float aspect, float near, float far);
	void Mat4Ortho(float* d, float l, float r, float b, float t, float n, float f);
	void Mat4Rotate(float* d, float x, float y, float z, float a);
	void Mat4LookAt(float* d, const float* eye, const float* center, const float* up);
	void Mat4Inverse(float* d, const float* s);
	void Mat4Transpose(float* d, const float* s);
	void Mat4Multiply(float* d, const float* a, const float* b);
	void Mat4DecompositTranslate(float* d, float* m);
	void Mat4DecompositScale(float* d, float* m);
	void Mat4DecompositRotate(float* d, float* m);
	
	struct Vec3;
	struct Vec4;
	struct Mat4;
	
	struct Vec3 {
		union {
			struct {
				float r;
				float g;
				float b;
			};
			
			struct {
				float x;
				float y;
				float z;
			};
			
			float v[3];
		};
		
		inline float operator [](int index) { return v[index]; }
		inline operator float*() const { return (float*)v; }
		
		inline float dot(const Vec3& v) { return Vec3Dot(this->v, v.v); }
		inline float size() { return Vec3Size(this->v); }
		inline float min() { return Vec3Min(this->v); }
		inline float max() { return Vec3Max(this->v); }
		inline float angle(const Vec3& v) { return Vec3Angle(this->v, v.v); }
		inline void angles(float* theta, float* phi) { Vec3Angles(this->v, theta, phi); }
		
		inline Vec3& set(float v) { Vec3Set(this->v, v); return *this; }
		inline Vec3& set(const Vec3& v) { Vec3SetV(this->v, v.v); return *this; }
		inline Vec3& negative() { Vec3Negative(this->v, this->v); return *this; }
		inline Vec3& negative(const Vec3& v) { Vec3Negative(this->v, v.v); return *this; }
		inline Vec3& add(float v) { Vec3Add(this->v, this->v, v); return *this; }
		inline Vec3& add(const Vec3& v) { Vec3AddV(this->v, this->v, v.v); return *this; }
		inline Vec3& add(const Vec3& a, float b) { Vec3Add(this->v, a.v, b); return *this; }
		inline Vec3& add(const Vec3& a, const Vec3& b) { Vec3AddV(this->v, a.v, b.v); return *this; }
		inline Vec3& subtract(float v) { Vec3Subtract(this->v, this->v, v); return *this; }
		inline Vec3& subtract(const Vec3& v) { Vec3SubtractV(this->v, this->v, v.v); return *this; }
		inline Vec3& subtract(const Vec3& a, float b) { Vec3Subtract(this->v, a.v, b); return *this; }
		inline Vec3& subtract(const Vec3& a, const Vec3& b) { Vec3SubtractV(this->v, a.v, b.v); return *this; }
		inline Vec3& multiply(float v) { Vec3Multiply(this->v, this->v, v); return *this; }
		inline Vec3& multiply(const Vec3& v) { Vec3MultiplyV(this->v, this->v, v.v); return *this; }
		inline Vec3& multiply(const Vec3& a, float b) { Vec3Multiply(this->v, a.v, b); return *this; }
		inline Vec3& multiply(const Vec3& a, const Vec3& b) { Vec3MultiplyV(this->v, a.v, b.v); return *this; }
		inline Vec3& divide(float v) { Vec3Divide(this->v, this->v, v); return *this; }
		inline Vec3& divide(const Vec3& v) { Vec3DivideV(this->v, this->v, v.v); return *this; }
		inline Vec3& divide(const Vec3& a, float b) { Vec3Divide(this->v, a.v, b); return *this; }
		inline Vec3& divide(const Vec3& a, const Vec3& b) { Vec3DivideV(this->v, a.v, b.v); return *this; }
		inline Vec3& cross(const Vec3& v) { Vec3Cross(this->v, this->v, v.v); return *this; }
		inline Vec3& cross(const Vec3& a, const Vec3& b) { Vec3Cross(this->v, a.v, b.v); return *this; }
		inline Vec3& normalize() { Vec3Normalize(this->v, this->v); return *this; }
		inline Vec3& normalize(const Vec3& v) { Vec3Normalize(this->v, v.v); return *this; }
		Vec3& transform(const Mat4& m);
		Vec3& transform(const Vec3& v, const Mat4& m);
		Vec3& transformNormal(const Mat4& m);
		Vec3& transformNormal(const Vec3& v, const Mat4& m);
		inline Vec3& random(const Vec3& v) { Vec3Random(this->v, v.x, v.y, v.z); return *this; }
	};
	
	struct Vec4 {
		union {
			struct {
				float r;
				float g;
				float b;
				float a;
			};
			
			struct {
				float x;
				float y;
				float z;
				float w;
			};
			
			float v[4];
		};
		
		inline float operator [](int index) { return v[index]; }
		inline operator float*() const { return (float*)v; }
		inline operator Vec3() const { return *((Vec3*)this); }
		
		inline float dot(const Vec4& v) { return Vec4Dot(this->v, v.v); }
		
		inline Vec4& set(float v) { Vec4Set(this->v, v); return *this; }
		inline Vec4& set(const Vec3& v, float v3) { Vec3SetV(this->v, v.v); this->v[3] = v3; return *this; }
		inline Vec4& set(const Vec4& v) { Vec4SetV(this->v, v.v); return *this; }
		inline Vec4& negative() { Vec4Negative(this->v, this->v); return *this; }
		inline Vec4& negative(const Vec4& v) { Vec4Negative(this->v, v.v); return *this; }
		inline Vec4& add(float v) { Vec4Add(this->v, this->v, v); return *this; }
		inline Vec4& add(const Vec4& v) { Vec4AddV(this->v, this->v, v.v); return *this; }
		inline Vec4& add(const Vec4& a, float b) { Vec4Add(this->v, a.v, b); return *this; }
		inline Vec4& add(const Vec4& a, const Vec4& b) { Vec4AddV(this->v, a.v, b.v); return *this; }
		inline Vec4& subtract(float v) { Vec4Subtract(this->v, this->v, v); return *this; }
		inline Vec4& subtract(const Vec4& v) { Vec4SubtractV(this->v, this->v, v.v); return *this; }
		inline Vec4& subtract(const Vec4& a, float b) { Vec4Subtract(this->v, a.v, b); return *this; }
		inline Vec4& subtract(const Vec4& a, const Vec4& b) { Vec4SubtractV(this->v, a.v, b.v); return *this; }
		inline Vec4& multiply(float v) { Vec4Multiply(this->v, this->v, v); return *this; }
		inline Vec4& multiply(const Vec4& v) { Vec4MultiplyV(this->v, this->v, v.v); return *this; }
		inline Vec4& multiply(const Vec4& a, float b) { Vec4Multiply(this->v, a.v, b); return *this; }
		inline Vec4& multiply(const Vec4& a, const Vec4& b) { Vec4MultiplyV(this->v, a.v, b.v); return *this; }
		inline Vec4& divide(float v) { Vec4Divide(this->v, this->v, v); return *this; }
		inline Vec4& divide(const Vec4& v) { Vec4DivideV(this->v, this->v, v.v); return *this; }
		inline Vec4& divide(const Vec4& a, float b) { Vec4Divide(this->v, a.v, b); return *this; }
		inline Vec4& divide(const Vec4& a, const Vec4& b) { Vec4DivideV(this->v, a.v, b.v); return *this; }
		inline Vec4& normalize() { Vec4Normalize(this->v, this->v); return *this; }
		inline Vec4& normalize(const Vec4& v) { Vec4Normalize(this->v, v.v); return *this; }
	};
	
	struct Mat4 {
		float v[16];
		
		inline float operator [](int index) { return v[index]; }
		inline operator float*() { return v; }
		
		inline Mat4& set(float v) { Mat4Set(this->v, v); return *this; }
		inline Mat4& set(const Mat4& m) { Mat4SetV(this->v, m.v); return *this; }
		inline Mat4& identity() { Mat4Identity(this->v); return *this; }
		inline Mat4& translate(const Vec3& v) { Mat4Translate(this->v, v.x, v.y, v.z); return *this; }
		inline Mat4& scale(const Vec3& v) { Mat4Scale(this->v, v.x, v.y, v.z); return *this; }
		inline Mat4& rotateX(float a) { Mat4RotateX(this->v, a); return *this; }
		inline Mat4& rotateY(float a) { Mat4RotateY(this->v, a); return *this; }
		inline Mat4& rotateZ(float a) { Mat4RotateZ(this->v, a); return *this; }
		inline Mat4& frustum(float l, float r, float b, float t, float n, float f) { Mat4Frustum(this->v, l,r,b,t,n,f); return *this; }
		inline Mat4& perspective(float fov, float aspect, float near, float far) { Mat4Perspective(this->v, fov, aspect, near, far); return *this; }
		inline Mat4& ortho(float l, float r, float b, float t, float n, float f) { Mat4Ortho(this->v, l,r,b,t,n,f); return *this; }
		inline Mat4& rotate(const Vec3& v, float a) { Mat4Rotate(this->v, v.x, v.y, v.z, a); return *this; }
		inline Mat4& lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) { Mat4LookAt(this->v, eye.v, center.v, up.v); return *this; }

		inline Mat4& inverse() { Mat4Inverse(this->v, this->v); return *this; }
		inline Mat4& inverse(const Mat4& m) { Mat4Inverse(this->v, m.v); return *this; }
		inline Mat4& transpose() { Mat4Transpose(this->v, this->v); return *this; }
		inline Mat4& transpose(const Mat4& m) { Mat4Transpose(this->v, m.v); return *this; }
		inline Mat4& multiply(const Mat4& m) { Mat4Multiply(this->v, this->v, m.v); return *this; }
		inline Mat4& multiply(const Mat4& a, const Mat4& b) { Mat4Multiply(this->v, a.v, b.v); return *this; }
	};
	
#ifdef __cplusplus
}
#endif

#endif //JAPPSY_UVECTOR_H