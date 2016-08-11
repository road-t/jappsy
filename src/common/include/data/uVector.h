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

#include <opengl/uOpenGL.h>

#ifdef __cplusplus
extern "C" {
#endif
	
	void Vec3Set(GLfloat* d, GLfloat v);
	void Vec4Set(GLfloat* d, GLfloat v);
	void Vec3SetV(GLfloat* d, const GLfloat* v);
	void Vec4SetV(GLfloat* d, const GLfloat* v);
	void Vec3Negative(GLfloat* d, const GLfloat* s);
	void Vec4Negative(GLfloat* d, const GLfloat* s);
	void Vec3Add(GLfloat* d, const GLfloat* a, GLfloat b);
	void Vec4Add(GLfloat* d, const GLfloat* a, GLfloat b);
	void Vec3AddV(GLfloat* d, const GLfloat* a, const GLfloat* b);
	void Vec4AddV(GLfloat* d, const GLfloat* a, const GLfloat* b);
	void Vec3Subtract(GLfloat* d, const GLfloat* a, GLfloat b);
	void Vec4Subtract(GLfloat* d, const GLfloat* a, GLfloat b);
	void Vec3SubtractV(GLfloat* d, const GLfloat* a, const GLfloat* b);
	void Vec4SubtractV(GLfloat* d, const GLfloat* a, const GLfloat* b);
	void Vec3Multiply(GLfloat* d, const GLfloat* a, GLfloat b);
	void Vec4Multiply(GLfloat* d, const GLfloat* a, GLfloat b);
	void Vec3MultiplyV(GLfloat* d, const GLfloat* a, const GLfloat* b);
	void Vec4MultiplyV(GLfloat* d, const GLfloat* a, const GLfloat* b);
	void Vec3Divide(GLfloat* d, const GLfloat* a, GLfloat b);
	void Vec4Divide(GLfloat* d, const GLfloat* a, GLfloat b);
	void Vec3DivideV(GLfloat* d, const GLfloat* a, const GLfloat* b);
	void Vec4DivideV(GLfloat* d, const GLfloat* a, const GLfloat* b);
	GLfloat Vec3Dot(const GLfloat* a, const GLfloat* b);
	GLfloat Vec4Dot(const GLfloat* a, const GLfloat* b);
	void Vec3Cross(GLfloat* d, const GLfloat* a, const GLfloat* b);
	GLfloat Vec3Size(const GLfloat* v);
	void Vec3Normalize(GLfloat* d, const GLfloat* v);
	void Vec4Normalize(GLfloat* d, const GLfloat* v);
	GLfloat Vec3Min(const GLfloat* v);
	GLfloat Vec3Max(const GLfloat* v);
	void Vec3Angles(const GLfloat* v, GLfloat* theta, GLfloat* phi);
	GLfloat Vec3Angle(const GLfloat* a, const GLfloat* b);
	void Vec3Transform(GLfloat* d, const GLfloat* p, const GLfloat* m);
	void Vec3TransformNormal(GLfloat* d, const GLfloat* v, const GLfloat* m);
	void Vec3Random(GLfloat* d, GLfloat x, GLfloat y, GLfloat z);
	
	void Mat4Set(GLfloat* d, GLfloat v);
	void Mat4SetV(GLfloat* d, const GLfloat* m);
	void Mat4Identity(GLfloat* d);
	void Mat4Translate(GLfloat* d, GLfloat x, GLfloat y, GLfloat z);
	void Mat4Scale(GLfloat* d, GLfloat x, GLfloat y, GLfloat z);
	void Mat4RotateX(GLfloat* d, GLfloat a);
	void Mat4RotateY(GLfloat* d, GLfloat a);
	void Mat4RotateZ(GLfloat* d, GLfloat a);
	void Mat4Frustum(GLfloat* d, GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
	void Mat4Perspective(GLfloat* d, GLfloat fov, GLfloat aspect, GLfloat near, GLfloat far);
	void Mat4Ortho(GLfloat* d, GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
	void Mat4Rotate(GLfloat* d, GLfloat x, GLfloat y, GLfloat z, GLfloat a);
	void Mat4LookAt(GLfloat* d, const GLfloat* eye, const GLfloat* center, const GLfloat* up);
	void Mat4Inverse(GLfloat* d, const GLfloat* s);
	void Mat4Transpose(GLfloat* d, const GLfloat* s);
	void Mat4Multiply(GLfloat* d, const GLfloat* a, const GLfloat* b);
	void Mat4DecompositTranslate(GLfloat* d, GLfloat* m);
	void Mat4DecompositScale(GLfloat* d, GLfloat* m);
	void Mat4DecompositRotate(GLfloat* d, GLfloat* m);
	
	struct Vec3;
	struct Vec4;
	struct Mat4;
	
	struct Vec3 {
		union {
			struct {
				GLfloat r;
				GLfloat g;
				GLfloat b;
			};
			
			struct {
				GLfloat x;
				GLfloat y;
				GLfloat z;
			};
			
			GLfloat v[3];
		};
		
		inline GLfloat& operator [](int index) { return v[index]; }
		inline operator GLfloat*() const { return (GLfloat*)v; }
		
		inline GLfloat dot(const Vec3& v) { return Vec3Dot(this->v, v.v); }
		inline GLfloat size() { return Vec3Size(this->v); }
		inline GLfloat min() { return Vec3Min(this->v); }
		inline GLfloat max() { return Vec3Max(this->v); }
		inline GLfloat angle(const Vec3& v) { return Vec3Angle(this->v, v.v); }
		inline void angles(GLfloat* theta, GLfloat* phi) { Vec3Angles(this->v, theta, phi); }
		
		inline Vec3& set(GLfloat v) { Vec3Set(this->v, v); return *this; }
		inline Vec3& set(const Vec3& v) { Vec3SetV(this->v, v.v); return *this; }
		inline Vec3& negative() { Vec3Negative(this->v, this->v); return *this; }
		inline Vec3& negative(const Vec3& v) { Vec3Negative(this->v, v.v); return *this; }
		inline Vec3& add(GLfloat v) { Vec3Add(this->v, this->v, v); return *this; }
		inline Vec3& add(const Vec3& v) { Vec3AddV(this->v, this->v, v.v); return *this; }
		inline Vec3& add(const Vec3& a, GLfloat b) { Vec3Add(this->v, a.v, b); return *this; }
		inline Vec3& add(const Vec3& a, const Vec3& b) { Vec3AddV(this->v, a.v, b.v); return *this; }
		inline Vec3& subtract(GLfloat v) { Vec3Subtract(this->v, this->v, v); return *this; }
		inline Vec3& subtract(const Vec3& v) { Vec3SubtractV(this->v, this->v, v.v); return *this; }
		inline Vec3& subtract(const Vec3& a, GLfloat b) { Vec3Subtract(this->v, a.v, b); return *this; }
		inline Vec3& subtract(const Vec3& a, const Vec3& b) { Vec3SubtractV(this->v, a.v, b.v); return *this; }
		inline Vec3& multiply(GLfloat v) { Vec3Multiply(this->v, this->v, v); return *this; }
		inline Vec3& multiply(const Vec3& v) { Vec3MultiplyV(this->v, this->v, v.v); return *this; }
		inline Vec3& multiply(const Vec3& a, GLfloat b) { Vec3Multiply(this->v, a.v, b); return *this; }
		inline Vec3& multiply(const Vec3& a, const Vec3& b) { Vec3MultiplyV(this->v, a.v, b.v); return *this; }
		inline Vec3& divide(GLfloat v) { Vec3Divide(this->v, this->v, v); return *this; }
		inline Vec3& divide(const Vec3& v) { Vec3DivideV(this->v, this->v, v.v); return *this; }
		inline Vec3& divide(const Vec3& a, GLfloat b) { Vec3Divide(this->v, a.v, b); return *this; }
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
				GLfloat r;
				GLfloat g;
				GLfloat b;
				GLfloat a;
			};
			
			struct {
				GLfloat x;
				GLfloat y;
				GLfloat z;
				GLfloat w;
			};
			
			GLfloat v[4];
		};
		
		inline GLfloat& operator [](int index) { return v[index]; }
		inline operator GLfloat*() const { return (GLfloat*)v; }
		inline operator Vec3() const { return *((Vec3*)this); }
		
		inline GLfloat dot(const Vec4& v) { return Vec4Dot(this->v, v.v); }
		
		inline Vec4& set(GLfloat v) { Vec4Set(this->v, v); return *this; }
		inline Vec4& set(const Vec3& v, GLfloat v3) { Vec3SetV(this->v, v.v); this->v[3] = v3; return *this; }
		inline Vec4& set(const Vec4& v) { Vec4SetV(this->v, v.v); return *this; }
		inline Vec4& negative() { Vec4Negative(this->v, this->v); return *this; }
		inline Vec4& negative(const Vec4& v) { Vec4Negative(this->v, v.v); return *this; }
		inline Vec4& add(GLfloat v) { Vec4Add(this->v, this->v, v); return *this; }
		inline Vec4& add(const Vec4& v) { Vec4AddV(this->v, this->v, v.v); return *this; }
		inline Vec4& add(const Vec4& a, GLfloat b) { Vec4Add(this->v, a.v, b); return *this; }
		inline Vec4& add(const Vec4& a, const Vec4& b) { Vec4AddV(this->v, a.v, b.v); return *this; }
		inline Vec4& subtract(GLfloat v) { Vec4Subtract(this->v, this->v, v); return *this; }
		inline Vec4& subtract(const Vec4& v) { Vec4SubtractV(this->v, this->v, v.v); return *this; }
		inline Vec4& subtract(const Vec4& a, GLfloat b) { Vec4Subtract(this->v, a.v, b); return *this; }
		inline Vec4& subtract(const Vec4& a, const Vec4& b) { Vec4SubtractV(this->v, a.v, b.v); return *this; }
		inline Vec4& multiply(GLfloat v) { Vec4Multiply(this->v, this->v, v); return *this; }
		inline Vec4& multiply(const Vec4& v) { Vec4MultiplyV(this->v, this->v, v.v); return *this; }
		inline Vec4& multiply(const Vec4& a, GLfloat b) { Vec4Multiply(this->v, a.v, b); return *this; }
		inline Vec4& multiply(const Vec4& a, const Vec4& b) { Vec4MultiplyV(this->v, a.v, b.v); return *this; }
		inline Vec4& divide(GLfloat v) { Vec4Divide(this->v, this->v, v); return *this; }
		inline Vec4& divide(const Vec4& v) { Vec4DivideV(this->v, this->v, v.v); return *this; }
		inline Vec4& divide(const Vec4& a, GLfloat b) { Vec4Divide(this->v, a.v, b); return *this; }
		inline Vec4& divide(const Vec4& a, const Vec4& b) { Vec4DivideV(this->v, a.v, b.v); return *this; }
		inline Vec4& normalize() { Vec4Normalize(this->v, this->v); return *this; }
		inline Vec4& normalize(const Vec4& v) { Vec4Normalize(this->v, v.v); return *this; }
	};
	
	struct Mat4 {
		GLfloat v[16];
		
		inline GLfloat& operator [](int index) { return v[index]; }
		inline operator GLfloat*() { return v; }
		
		inline Mat4& set(GLfloat v) { Mat4Set(this->v, v); return *this; }
		inline Mat4& set(const Mat4& m) { Mat4SetV(this->v, m.v); return *this; }
		inline Mat4& identity() { Mat4Identity(this->v); return *this; }
		inline Mat4& translate(const Vec3& v) { Mat4Translate(this->v, v.x, v.y, v.z); return *this; }
		inline Mat4& scale(const Vec3& v) { Mat4Scale(this->v, v.x, v.y, v.z); return *this; }
		inline Mat4& rotateX(GLfloat a) { Mat4RotateX(this->v, a); return *this; }
		inline Mat4& rotateY(GLfloat a) { Mat4RotateY(this->v, a); return *this; }
		inline Mat4& rotateZ(GLfloat a) { Mat4RotateZ(this->v, a); return *this; }
		inline Mat4& frustum(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f) { Mat4Frustum(this->v, l,r,b,t,n,f); return *this; }
		inline Mat4& perspective(GLfloat fov, GLfloat aspect, GLfloat near, GLfloat far) { Mat4Perspective(this->v, fov, aspect, near, far); return *this; }
		inline Mat4& ortho(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f) { Mat4Ortho(this->v, l,r,b,t,n,f); return *this; }
		inline Mat4& rotate(const Vec3& v, GLfloat a) { Mat4Rotate(this->v, v.x, v.y, v.z, a); return *this; }
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