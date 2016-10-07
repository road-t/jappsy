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

#include <platform.h>
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
	
	struct Vec2 {
		union {
			struct {
				GLfloat x;
				GLfloat y;
			};
			
			struct {
				GLfloat s;
				GLfloat t;
			};
			
			GLfloat v[2];
		};
		
		inline Vec2() { }
		inline Vec2(const GLfloat x, const GLfloat y) { v[0] = x; v[1] = y; }
		inline Vec2(const GLfloat v[2]) { this->v[0] = v[0]; this->v[1] = v[1]; }
		inline Vec2(const Vec2& v) { this->v[0] = v.v[0]; this->v[1] = v.v[1]; }
		Vec2(const Vec3& v);
		Vec2(const Vec4& v);

		inline GLfloat& operator [](int index) { return v[index]; }
		inline operator GLfloat*() const { return (GLfloat*)v; }
		
		inline Vec2& operator =(const Vec2& v) { this->v[0] = v.v[0]; this->v[1] = v.v[1]; return *this; }
		Vec2& operator =(const Vec3& v);
		Vec2& operator =(const Vec4& v);
	};
	
	struct Vec3i {
		GLshort i[3];
		
		inline Vec3i() { }
		inline Vec3i(const GLshort i1, const GLshort i2, GLshort i3) { i[0] = i1; i[1] = i2; i[2] = i3; }
		inline Vec3i(const GLshort i[3]) { this->i[0] = i[0]; this->i[1] = i[1]; this->i[2] = i[2]; }

		inline GLshort& operator [](int index) { return i[index]; }
		inline operator GLshort*() const { return (GLshort*)i; }

		inline Vec3i& operator =(const Vec3i& v) { this->i[0] = v.i[0]; this->i[1] = v.i[1]; this->i[2] = v.i[2]; return *this; }
	};
	
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
			
			struct {
				GLfloat s;
				GLfloat t;
				GLfloat p;
			};
			
			GLfloat v[3];
		};
		
		inline Vec3() { }
		inline Vec3(const GLfloat x, const GLfloat y, const GLfloat z) { v[0] = x; v[1] = y; v[2] = z; }
		inline Vec3(const GLfloat v[3]) { Vec3SetV(this->v, v); }
		inline Vec3(const Vec3& v) { Vec3SetV(this->v, v.v); }
		Vec3(const Vec4& v);
		
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

		inline Vec3& operator =(const Vec3& v) { Vec3SetV(this->v, v.v); return *this; }
		Vec3& operator =(const Vec4& v);
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
			
			struct {
				GLfloat s;
				GLfloat t;
				GLfloat p;
				GLfloat q;
			};
			
			GLfloat v[4];
		};
		
		inline Vec4() { }
		inline Vec4(const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }
		inline Vec4(const GLfloat v[4]) { Vec4SetV(this->v, v); }
		inline Vec4(const Vec4& v) { Vec4SetV(this->v, v.v); }

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

		inline Vec4& operator =(const Vec4& v) { Vec4SetV(this->v, v.v); return *this; }
	};
	
	struct Mat4 {
		GLfloat v[16];
		
		inline Mat4() { Mat4Identity(this->v); }
		inline Mat4(const GLfloat v[16]) { Mat4SetV(this->v, v); }
		inline Mat4(const Mat4& m) { Mat4SetV(this->v, m.v); }
		
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

		inline Mat4& operator =(const Mat4& m) { Mat4SetV(this->v, m.v); return *this; }
	};
	
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <core/uMemory.h>
#include <data/uObject.h>
#include <data/uString.h>

template <typename Type>
class Vector : public CObject {
private:
	Type* m_items = NULL;
	uint32_t m_count = 0;
	uint32_t m_size = 0;
	uint32_t m_step = 1;

public:
	inline Vector() {}
	inline Vector(uint32_t count) throw(const char*) { resize(count); }
	inline Vector(uint32_t count, uint32_t step) throw(const char*) { if (step != 0) m_step = step; resize(count); }
	inline ~Vector() { resize(0); }
	
	inline void growstep(uint32_t step) { if (step != 0) m_step = step; }
	
	inline void resize(uint32_t count) throw(const char*) {
		if (count == m_count)
			return;
		
		if (count == 0) {
			memFree(m_items);
			m_items = NULL;
			m_count = 0;
			m_size = 0;
			return;
		}
		
		uint32_t newSize = count;
		if (m_step > 1) {
			newSize = count + m_step - 1;
			newSize = newSize - (newSize % m_step);
		}
		
		if (m_size != newSize) {
			Type* newItems = memRealloc(Type, newItems, m_items, newSize * sizeof(Type));
			if (newItems == NULL)
				throw eOutOfMemory;
		
			m_items = newItems;
			m_size = newSize;
		}
		
		m_count = count;
	}
	
	virtual inline void push(const Type value) throw(const char*) {
		resize(m_count + 1);
		m_items[m_count - 1] = value;
	}
	
	virtual inline void unshift(const Type value) throw(const char*) {
		resize(m_count + 1);
		memmove(m_items + 1, m_items, (m_count - 1) * sizeof(Type));
		m_items[0] = value;
	}
	
	virtual inline Type pop() throw(const char*) {
		if (m_count > 0) {
			Type result = m_items[m_count - 1];
			resize(m_count - 1);
			return result;
		}
		
		throw eEmpty;
	}
	
	virtual inline Type shift() throw(const char*) {
		if (m_count > 0) {
			Type result = m_items[0];
			memmove(m_items, m_items + 1, (m_count - 1) * sizeof(Type));
			resize(m_count - 1);
			return result;
		}
		
		throw eEmpty;
	}
	
	virtual inline bool contains(const Type value) const {
		if (m_count > 0) {
			for (int i = m_count - 1; i >= 0; i--) {
				if (m_items[i] == value) return true;
			}
		}
		return false;
	}
	
	virtual inline int32_t count() const {
		return m_count;
	}
	
	virtual inline bool empty() const {
		return (m_count == 0);
	}
	
	virtual inline void clear() throw(const char*) {
		resize(0);
	}
	
	virtual inline const Type peek() const throw(const char*) {
		if (m_count > 0) {
			return m_items[m_count - 1];
		}
		throw eEmpty;
	}
	
	virtual inline const Type get(int32_t index) const throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			return m_items[index];
		}
		throw eOutOfRange;
	}
	
	virtual inline int32_t search(const Type value) const {
		if (m_count > 0) {
			for (int i = m_count-1; i >= 0; i--) {
				if (m_items[i] == value) return i;
			}
		}
		return -1;
	}
	
	virtual inline const Type remove(int32_t index) throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			Type result = m_items[index];
			if ((m_count > 0) && (index < (m_count - 1))) {
				memmove(m_items + index, m_items + index + 1, (m_count - index - 1) * sizeof(Type));
			}
			resize(m_count - 1);
			return result;
		}
		throw eOutOfRange;
	}
	
	virtual inline Type* items() const {
		return m_items;
	}
	
	virtual inline Type& operator [](int index) const throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			return m_items[index];
		}
		throw eOutOfRange;
	}
};

template <typename Type>
class Vector<Type*> : public CObject {
private:
	Type** m_items = NULL;
	uint32_t m_count = 0;
	uint32_t m_size = 0;
	uint32_t m_step = 1;
	
public:
	inline Vector() {}
	inline Vector(uint32_t count) throw(const char*) { resize(count); }
	inline Vector(uint32_t count, uint32_t step) throw(const char*) { if (step != 0) m_step = step; resize(count); }
	inline ~Vector() { resize(0); }
	
	inline void growstep(uint32_t step) { if (step != 0) m_step = step; }
	
	inline void resize(uint32_t count) throw(const char*) {
		if (count == m_count)
			return;
		
		if (count == 0) {
			memFree(m_items);
			m_items = NULL;
			m_count = 0;
			m_size = 0;
			return;
		}
		
		uint32_t newSize = count;
		if (m_step > 1) {
			newSize = count + m_step - 1;
			newSize = newSize - (newSize % m_step);
		}
		
		if (m_size != newSize) {
			Type** newItems = memRealloc(Type*, newItems, m_items, newSize * sizeof(Type*));
			if (newItems == NULL)
				throw eOutOfMemory;
			
			m_items = newItems;
			m_size = newSize;
		}
		
		m_count = count;
	}
	
	virtual inline void push(const Type* value) throw(const char*) {
		resize(m_count + 1);
		m_items[m_count - 1] = (Type*)value;
	}
	
	virtual inline void push(const Vector<Type*>& list) throw(const char*) {
		int32_t count = list.count();
		Type** items = list.items();
		int32_t ofs = m_count;
		resize(m_count + count);
		for (int i = 0; i < count; i++) {
			m_items[ofs + i] = items[i];
		}
	}
	
	virtual inline void unshift(const Type* value) throw(const char*) {
		resize(m_count + 1);
		memmove(m_items + 1, m_items, (m_count - 1) * sizeof(Type*));
		m_items[0] = (Type*)value;
	}
	
	virtual inline Type* pop() throw(const char*) {
		if (m_count > 0) {
			Type* result = m_items[m_count - 1];
			resize(m_count - 1);
			return result;
		}
		
		throw eEmpty;
	}
	
	virtual inline Type* shift() throw(const char*) {
		if (m_count > 0) {
			Type* result = m_items[0];
			memmove(m_items, m_items + 1, (m_count - 1) * sizeof(Type*));
			resize(m_count - 1);
			return result;
		}
		
		throw eEmpty;
	}
	
	virtual inline bool contains(const Type* value) const {
		if (m_count > 0) {
			for (int i = m_count - 1; i >= 0; i--) {
				if (m_items[i] == value) return true;
			}
		}
		return false;
	}
	
	virtual inline int32_t count() const {
		return m_count;
	}
	
	virtual inline bool empty() const {
		return (m_count == 0);
	}
	
	virtual inline void clear() throw(const char*) {
		resize(0);
	}
	
	virtual inline Type* peek() const throw(const char*) {
		if (m_count > 0) {
			return m_items[m_count - 1];
		}
		throw eEmpty;
	}
	
	virtual inline Type* get(int32_t index) const throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			return m_items[index];
		}
		throw eOutOfRange;
	}
	
	virtual inline int32_t search(const Type* value) const {
		if (m_count > 0) {
			for (int i = m_count-1; i >= 0; i--) {
				if (m_items[i] == value) return i;
			}
		}
		return -1;
	}
	
	virtual inline Type* remove(int32_t index) throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			Type* result = m_items[index];
			if ((m_count > 0) && (index < (m_count - 1))) {
				memmove(m_items + index, m_items + index + 1, (m_count - index - 1) * sizeof(Type*));
			}
			resize(m_count - 1);
			return result;
		}
		return NULL;
	}
	
	virtual inline void removefree(int32_t index) throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			mmfree(m_items[index]);
			if ((m_count > 0) && (index < (m_count - 1))) {
				memmove(m_items + index, m_items + index + 1, (m_count - index - 1) * sizeof(Type*));
			}
			resize(m_count - 1);
		}
	}
	
	virtual inline void removedelete(int32_t index) throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			delete m_items[index];
			if ((m_count > 0) && (index < (m_count - 1))) {
				memmove(m_items + index, m_items + index + 1, (m_count - index - 1) * sizeof(Type*));
			}
			resize(m_count - 1);
		}
	}
	
	virtual inline Type** items() const {
		return m_items;
	}
	
	virtual inline Type*& operator [](int index) const throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			return m_items[index];
		}
		throw eOutOfRange;
	}
};

template <class Type>
class Vector<Type&> : public CObject {
private:
	Type** m_items = NULL;
	uint32_t m_count = 0;
	uint32_t m_size = 0;
	uint32_t m_step = 1;
	
public:
	inline Vector() {}
	inline Vector(uint32_t count) throw(const char*) { resize(count); }
	inline Vector(uint32_t count, uint32_t step) throw(const char*) { if (step != 0) m_step = step; resize(count); }
	inline ~Vector() { resize(0); }
	
	inline void growstep(uint32_t step) { if (step != 0) m_step = step; }
	
	inline void resize(uint32_t count) throw(const char*) {
		if (count == m_count)
			return;
		
		if (count == 0) {
			for (int i = 0; i < m_count; i++) {
				if (m_items[i] != NULL)
					delete m_items[i];
			}
			memFree(m_items);
			m_items = NULL;
			m_count = 0;
			m_size = 0;
			return;
		}
		
		uint32_t newSize = count;
		if (m_step > 1) {
			newSize = count + m_step - 1;
			newSize = newSize - (newSize % m_step);
		}
		
		if (m_size != newSize) {
			CString** newItems = memRealloc(CString*, newItems, m_items, newSize * sizeof(CString*));
			if (newItems == NULL)
				throw eOutOfMemory;
			
			m_items = newItems;
			m_size = newSize;
		}
		
		m_count = count;
	}
	
	virtual inline Type& push(const Type& value) throw(const char*) {
		resize(m_count + 1);
		return *(m_items[m_count - 1] = new Type(value));
	}
	
	virtual inline Type& unshift(const Type& value) throw(const char*) {
		resize(m_count + 1);
		memmove(m_items + 1, m_items, (m_count - 1) * sizeof(Type*));
		return *(m_items[0] = new Type(value));
	}
	
	virtual inline Type pop() throw(const char*) {
		if (m_count > 0) {
			Type result = *(m_items[m_count - 1]);
			delete m_items[m_count - 1];
			resize(m_count - 1);
			return result;
		}
		
		throw eEmpty;
	}
	
	virtual inline Type shift() throw(const char*) {
		if (m_count > 0) {
			Type result = *(m_items[0]);
			delete m_items[0];
			memmove(m_items, m_items + 1, (m_count - 1) * sizeof(Type*));
			resize(m_count - 1);
			return result;
		}
		
		throw eEmpty;
	}
	
	virtual inline bool contains(const Type& value) const {
		if (m_count > 0) {
			for (int i = m_count - 1; i >= 0; i--) {
				if (*(m_items[i]) == value) return true;
			}
		}
		return false;
	}
	
	virtual inline int32_t count() const {
		return m_count;
	}
	
	virtual inline bool empty() const {
		return (m_count == 0);
	}
	
	virtual inline void clear() throw(const char*) {
		resize(0);
	}
	
	virtual inline const Type& peek() const throw(const char*) {
		if (m_count > 0) {
			return *(m_items[m_count - 1]);
		}
		throw eEmpty;
	}
	
	virtual inline const Type& get(int32_t index) const throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			return *(m_items[index]);
		}
		throw eOutOfRange;
	}
	
	virtual inline int32_t search(const Type& value) const {
		if (m_count > 0) {
			for (int i = m_count-1; i >= 0; i--) {
				if (*(m_items[i]) == value) return i;
			}
		}
		return -1;
	}
	
	virtual inline const Type remove(int32_t index) throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			Type result = *(m_items[index]);
			delete m_items[index];
			if ((m_count > 0) && (index < (m_count - 1))) {
				memmove(m_items + index, m_items + index + 1, (m_count - index - 1) * sizeof(Type*));
			} else if (m_count == 1) {
				m_items[index] = NULL;
			}
			resize(m_count - 1);
			return result;
		}
		throw eOutOfRange;
	}
	
	virtual inline Type** items() const {
		return m_items;
	}
	
	virtual inline Type& operator [](int index) const throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			return *(m_items[index]);
		}
		throw eOutOfRange;
	}
};

template <typename K, typename V>
class VectorMap : public CObject {
public:
	Vector<K>* m_keys;
	Vector<V>* m_values;
	
	inline VectorMap() {
		m_keys = new Vector<K>();
		m_values = new Vector<V>();
	}
	
	inline VectorMap(uint32_t count) {
		m_keys = new Vector<K>(count);
		m_values = new Vector<V>(count);
	}
	
	inline VectorMap(uint32_t count, uint32_t step) {
		m_keys = new Vector<K>(count, step);
		m_values = new Vector<V>(count, step);
	}
	
	inline ~VectorMap() {
		delete m_keys;
		delete m_values;
	}
	
	virtual inline void clear() {
		m_keys->clear();
		m_values->clear();
	}
	
	virtual inline bool containsKey(K key) const {
		return m_keys->contains(key);
	}
	
	virtual inline bool containsValue(V value) const {
		return m_values->contains(value);
	}
	
	virtual inline V get(K key) const {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			return m_values->get(index);
		}
		throw eNotFound;
	}
	
	virtual inline bool empty() const {
		return m_keys->empty();
	}
	
	virtual inline void put(K key, V value) {
		int32_t index = m_keys->search(key);
		if (index < 0) {
			m_keys->push(key);
			m_values->push(value);
		} else {
			m_values->operator[](index) = value;
		}
	}
	
	virtual inline void remove(K key) {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			m_keys->remove(index);
			return m_values->remove(index);
		}
	}
	
	virtual inline int32_t count() const {
		return m_keys->count();
	}
	
	virtual inline K* keys() const {
		return m_keys->items();
	}
	
	virtual inline V* values() const {
		return m_values->items();
	}
};

template <typename K, typename V>
class VectorMap<K&,V> : public CObject {
public:
	Vector<K&>* m_keys;
	Vector<V>* m_values;
	
	inline VectorMap() {
		m_keys = new Vector<K&>();
		m_values = new Vector<V>();
	}
	
	inline VectorMap(uint32_t count) {
		m_keys = new Vector<K&>(count);
		m_values = new Vector<V>(count);
	}
	
	inline VectorMap(uint32_t count, uint32_t step) {
		m_keys = new Vector<K&>(count, step);
		m_values = new Vector<V>(count, step);
	}
	
	inline ~VectorMap() {
		delete m_keys;
		delete m_values;
	}
	
	virtual inline void clear() {
		m_keys->clear();
		m_values->clear();
	}
	
	virtual inline bool containsKey(const K& key) const {
		return m_keys->contains(key);
	}
	
	virtual inline bool containsValue(V value) const {
		return m_values->contains(value);
	}
	
	virtual inline V get(const K& key) const {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			return m_values->get(index);
		}
		throw eNotFound;
	}
	
	virtual inline bool empty() const {
		return m_keys->empty();
	}
	
	virtual inline void put(const K& key, V value) {
		int32_t index = m_keys->search(key);
		if (index < 0) {
			m_keys->push(key);
			m_values->push(value);
		} else {
			m_values->operator[](index) = value;
		}
	}
	
	virtual inline void remove(const K& key) {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			m_keys->remove(index);
			return m_values->remove(index);
		}
	}
	
	virtual inline int32_t count() const {
		return m_keys->count();
	}
	
	virtual inline K** keys() const {
		return m_keys->items();
	}
	
	virtual inline V* items() const {
		return m_values->items();
	}
};

template <typename K, typename V>
class VectorMap<K&,V&> : public CObject {
public:
	Vector<K&>* m_keys;
	Vector<V&>* m_values;
	
	inline VectorMap() {
		m_keys = new Vector<K&>();
		m_values = new Vector<V&>();
	}
	
	inline VectorMap(uint32_t count) {
		m_keys = new Vector<K&>(count);
		m_values = new Vector<V&>(count);
	}
	
	inline VectorMap(uint32_t count, uint32_t step) {
		m_keys = new Vector<K&>(count, step);
		m_values = new Vector<V&>(count, step);
	}
	
	inline ~VectorMap() {
		delete m_keys;
		delete m_values;
	}
	
	virtual inline void clear() {
		m_keys->clear();
		m_values->clear();
	}
	
	virtual inline bool containsKey(const K& key) const {
		return m_keys->contains(key);
	}
	
	virtual inline bool containsValue(const V& value) const {
		return m_values->contains(value);
	}
	
	virtual inline V& get(const K& key) const {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			return m_values->get(index);
		}
		throw eNotFound;
	}
	
	virtual inline bool empty() const {
		return m_keys->empty();
	}
	
	virtual inline void put(const K& key, const V& value) {
		int32_t index = m_keys->search(key);
		if (index < 0) {
			m_keys->push(key);
			m_values->push(value);
		} else {
			m_values->operator[](index) = value;
		}
	}
	
	virtual inline void remove(const K& key) {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			m_keys->remove(index);
			m_values->remove(index);
		}
	}
	
	virtual inline int32_t count() const {
		return m_keys->count();
	}
	
	virtual inline K** keys() const {
		return m_keys->items();
	}
	
	virtual inline V** items() const {
		return m_values->items();
	}
};

template <typename K, typename V>
class VectorMap<K,V&> : public CObject {
public:
	Vector<K>* m_keys;
	Vector<V&>* m_values;
	
	inline VectorMap() {
		m_keys = new Vector<K>();
		m_values = new Vector<V&>();
	}
	
	inline VectorMap(uint32_t count) {
		m_keys = new Vector<K>(count);
		m_values = new Vector<V&>(count);
	}
	
	inline VectorMap(uint32_t count, uint32_t step) {
		m_keys = new Vector<K>(count, step);
		m_values = new Vector<V&>(count, step);
	}
	
	inline ~VectorMap() {
		delete m_keys;
		delete m_values;
	}
	
	virtual inline void clear() {
		m_keys->clear();
		m_values->clear();
	}
	
	virtual inline bool containsKey(K key) const {
		return m_keys->contains(key);
	}
	
	virtual inline bool containsValue(const V& value) const {
		return m_values->contains(value);
	}
	
	virtual inline V& get(K key) const {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			return m_values->get(index);
		}
		throw eNotFound;
	}
	
	virtual inline bool empty() const {
		return m_keys->empty();
	}
	
	virtual inline void put(K key, const V& value) {
		int32_t index = m_keys->search(key);
		if (index < 0) {
			m_keys->push(key);
			m_values->push(value);
		} else {
			m_values->operator[](index) = value;
		}
	}
	
	virtual inline void remove(K key) {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			m_keys->remove(index);
			m_values->remove(index);
		}
	}
	
	virtual inline int32_t count() const {
		return m_keys->count();
	}
	
	virtual inline K** keys() const {
		return m_keys->items();
	}
	
	virtual inline V** items() const {
		return m_values->items();
	}
};

template <typename K, typename V>
class VectorMap<K,V*> : public CObject {
public:
	Vector<K>* m_keys;
	Vector<V*>* m_values;
	
	inline VectorMap() {
		m_keys = new Vector<K>();
		m_values = new Vector<V*>();
	}
	
	inline VectorMap(uint32_t count) {
		m_keys = new Vector<K>(count);
		m_values = new Vector<V*>(count);
	}
	
	inline VectorMap(uint32_t count, uint32_t step) {
		m_keys = new Vector<K>(count, step);
		m_values = new Vector<V*>(count, step);
	}
	
	inline ~VectorMap() {
		delete m_keys;
		delete m_values;
	}
	
	virtual inline void clear() {
		m_keys->clear();
		m_values->clear();
	}
	
	virtual inline bool containsKey(K key) const {
		return m_keys->contains(key);
	}
	
	virtual inline bool containsValue(const V* value) const {
		return m_values->contains(value);
	}
	
	virtual inline V* get(K key) const {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			return m_values->get(index);
		}
		throw eNotFound;
	}
	
	virtual inline bool empty() const {
		return m_keys->empty();
	}
	
	virtual inline void put(K key, const V* value) {
		int32_t index = m_keys->search(key);
		if (index < 0) {
			m_keys->push(key);
			m_values->push(value);
		} else {
			m_values->operator[](index) = value;
		}
	}
	
	virtual inline V* remove(K key) {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			m_keys->remove(index);
			return m_values->remove(index);
		}
	}
	
	virtual inline void removefree(K key) {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			m_keys->remove(index);
			m_values->removefree(index);
		}
	}
	
	virtual inline void removedelete(K key) {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			m_keys->remove(index);
			m_values->removedelete(index);
		}
	}
	
	virtual inline int32_t count() const {
		return m_keys->count();
	}
	
	virtual inline K* keys() const {
		return m_keys->items();
	}
	
	virtual inline V* values() const {
		return m_values->items();
	}
};

template <typename K, typename V>
class VectorMap<K&,V*> : public CObject {
public:
	Vector<K&>* m_keys;
	Vector<V*>* m_values;
	
	inline VectorMap() {
		m_keys = new Vector<K&>();
		m_values = new Vector<V*>();
	}
	
	inline VectorMap(uint32_t count) {
		m_keys = new Vector<K&>(count);
		m_values = new Vector<V*>(count);
	}
	
	inline VectorMap(uint32_t count, uint32_t step) {
		m_keys = new Vector<K&>(count, step);
		m_values = new Vector<V*>(count, step);
	}
	
	inline ~VectorMap() {
		delete m_keys;
		delete m_values;
	}
	
	virtual inline void clear() {
		m_keys->clear();
		m_values->clear();
	}
	
	virtual inline bool containsKey(const K& key) const {
		return m_keys->contains(key);
	}
	
	virtual inline bool containsValue(const V* value) const {
		return m_values->contains(value);
	}
	
	virtual inline V* get(const K& key) const {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			return m_values->get(index);
		}
		throw eNotFound;
	}
	
	virtual inline bool empty() const {
		return m_keys->empty();
	}
	
	virtual inline void put(const K& key, const V* value) {
		int32_t index = m_keys->search(key);
		if (index < 0) {
			m_keys->push(key);
			m_values->push(value);
		} else {
			m_values->operator[](index) = (V*)value;
		}
	}
	
	virtual inline V* remove(const K& key) {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			m_keys->remove(index);
			return m_values->remove(index);
		}
		return NULL;
	}
	
	virtual inline void removefree(const K& key) {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			m_keys->remove(index);
			m_values->removefree(index);
		}
	}
	
	virtual inline void removedelete(const K& key) {
		int32_t index = m_keys->search(key);
		if (index >= 0) {
			m_keys->remove(index);
			m_values->removedelete(index);
		}
	}
	
	virtual inline int32_t count() const {
		return m_keys->count();
	}
	
	virtual inline K** keys() const {
		return m_keys->items();
	}
	
	virtual inline V** items() const {
		return m_values->items();
	}
};

#endif

#endif //JAPPSY_UVECTOR_H
