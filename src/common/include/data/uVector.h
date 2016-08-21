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

		inline GLfloat& operator [](int index) { return v[index]; }
		inline operator GLfloat*() const { return (GLfloat*)v; }
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
		
		inline GLfloat& operator [](int index) { return v[index]; }
		inline operator GLfloat*() const { return (GLfloat*)v; }
		
		inline GLfloat dot(const Vec3& v) { return Vec3Dot(THIS.v, v.v); }
		inline GLfloat size() { return Vec3Size(THIS.v); }
		inline GLfloat min() { return Vec3Min(THIS.v); }
		inline GLfloat max() { return Vec3Max(THIS.v); }
		inline GLfloat angle(const Vec3& v) { return Vec3Angle(THIS.v, v.v); }
		inline void angles(GLfloat* theta, GLfloat* phi) { Vec3Angles(THIS.v, theta, phi); }
		
		inline Vec3& set(GLfloat v) { Vec3Set(THIS.v, v); return *this; }
		inline Vec3& set(const Vec3& v) { Vec3SetV(THIS.v, v.v); return *this; }
		inline Vec3& negative() { Vec3Negative(THIS.v, THIS.v); return *this; }
		inline Vec3& negative(const Vec3& v) { Vec3Negative(THIS.v, v.v); return *this; }
		inline Vec3& add(GLfloat v) { Vec3Add(THIS.v, THIS.v, v); return *this; }
		inline Vec3& add(const Vec3& v) { Vec3AddV(THIS.v, THIS.v, v.v); return *this; }
		inline Vec3& add(const Vec3& a, GLfloat b) { Vec3Add(THIS.v, a.v, b); return *this; }
		inline Vec3& add(const Vec3& a, const Vec3& b) { Vec3AddV(THIS.v, a.v, b.v); return *this; }
		inline Vec3& subtract(GLfloat v) { Vec3Subtract(THIS.v, THIS.v, v); return *this; }
		inline Vec3& subtract(const Vec3& v) { Vec3SubtractV(THIS.v, THIS.v, v.v); return *this; }
		inline Vec3& subtract(const Vec3& a, GLfloat b) { Vec3Subtract(THIS.v, a.v, b); return *this; }
		inline Vec3& subtract(const Vec3& a, const Vec3& b) { Vec3SubtractV(THIS.v, a.v, b.v); return *this; }
		inline Vec3& multiply(GLfloat v) { Vec3Multiply(THIS.v, THIS.v, v); return *this; }
		inline Vec3& multiply(const Vec3& v) { Vec3MultiplyV(THIS.v, THIS.v, v.v); return *this; }
		inline Vec3& multiply(const Vec3& a, GLfloat b) { Vec3Multiply(THIS.v, a.v, b); return *this; }
		inline Vec3& multiply(const Vec3& a, const Vec3& b) { Vec3MultiplyV(THIS.v, a.v, b.v); return *this; }
		inline Vec3& divide(GLfloat v) { Vec3Divide(THIS.v, THIS.v, v); return *this; }
		inline Vec3& divide(const Vec3& v) { Vec3DivideV(THIS.v, THIS.v, v.v); return *this; }
		inline Vec3& divide(const Vec3& a, GLfloat b) { Vec3Divide(THIS.v, a.v, b); return *this; }
		inline Vec3& divide(const Vec3& a, const Vec3& b) { Vec3DivideV(THIS.v, a.v, b.v); return *this; }
		inline Vec3& cross(const Vec3& v) { Vec3Cross(THIS.v, THIS.v, v.v); return *this; }
		inline Vec3& cross(const Vec3& a, const Vec3& b) { Vec3Cross(THIS.v, a.v, b.v); return *this; }
		inline Vec3& normalize() { Vec3Normalize(THIS.v, THIS.v); return *this; }
		inline Vec3& normalize(const Vec3& v) { Vec3Normalize(THIS.v, v.v); return *this; }
		Vec3& transform(const Mat4& m);
		Vec3& transform(const Vec3& v, const Mat4& m);
		Vec3& transformNormal(const Mat4& m);
		Vec3& transformNormal(const Vec3& v, const Mat4& m);
		inline Vec3& random(const Vec3& v) { Vec3Random(THIS.v, v.x, v.y, v.z); return *this; }
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

		inline GLfloat& operator [](int index) { return v[index]; }
		inline operator GLfloat*() const { return (GLfloat*)v; }
		inline operator Vec3() const { return *((Vec3*)this); }
		
		inline GLfloat dot(const Vec4& v) { return Vec4Dot(THIS.v, v.v); }
		
		inline Vec4& set(GLfloat v) { Vec4Set(THIS.v, v); return *this; }
		inline Vec4& set(const Vec3& v, GLfloat v3) { Vec3SetV(THIS.v, v.v); THIS.v[3] = v3; return *this; }
		inline Vec4& set(const Vec4& v) { Vec4SetV(THIS.v, v.v); return *this; }
		inline Vec4& negative() { Vec4Negative(THIS.v, THIS.v); return *this; }
		inline Vec4& negative(const Vec4& v) { Vec4Negative(THIS.v, v.v); return *this; }
		inline Vec4& add(GLfloat v) { Vec4Add(THIS.v, THIS.v, v); return *this; }
		inline Vec4& add(const Vec4& v) { Vec4AddV(THIS.v, THIS.v, v.v); return *this; }
		inline Vec4& add(const Vec4& a, GLfloat b) { Vec4Add(THIS.v, a.v, b); return *this; }
		inline Vec4& add(const Vec4& a, const Vec4& b) { Vec4AddV(THIS.v, a.v, b.v); return *this; }
		inline Vec4& subtract(GLfloat v) { Vec4Subtract(THIS.v, THIS.v, v); return *this; }
		inline Vec4& subtract(const Vec4& v) { Vec4SubtractV(THIS.v, THIS.v, v.v); return *this; }
		inline Vec4& subtract(const Vec4& a, GLfloat b) { Vec4Subtract(THIS.v, a.v, b); return *this; }
		inline Vec4& subtract(const Vec4& a, const Vec4& b) { Vec4SubtractV(THIS.v, a.v, b.v); return *this; }
		inline Vec4& multiply(GLfloat v) { Vec4Multiply(THIS.v, THIS.v, v); return *this; }
		inline Vec4& multiply(const Vec4& v) { Vec4MultiplyV(THIS.v, THIS.v, v.v); return *this; }
		inline Vec4& multiply(const Vec4& a, GLfloat b) { Vec4Multiply(THIS.v, a.v, b); return *this; }
		inline Vec4& multiply(const Vec4& a, const Vec4& b) { Vec4MultiplyV(THIS.v, a.v, b.v); return *this; }
		inline Vec4& divide(GLfloat v) { Vec4Divide(THIS.v, THIS.v, v); return *this; }
		inline Vec4& divide(const Vec4& v) { Vec4DivideV(THIS.v, THIS.v, v.v); return *this; }
		inline Vec4& divide(const Vec4& a, GLfloat b) { Vec4Divide(THIS.v, a.v, b); return *this; }
		inline Vec4& divide(const Vec4& a, const Vec4& b) { Vec4DivideV(THIS.v, a.v, b.v); return *this; }
		inline Vec4& normalize() { Vec4Normalize(THIS.v, THIS.v); return *this; }
		inline Vec4& normalize(const Vec4& v) { Vec4Normalize(THIS.v, v.v); return *this; }
	};
	
	struct Mat4 {
		GLfloat v[16];
		
		inline GLfloat& operator [](int index) { return v[index]; }
		inline operator GLfloat*() { return v; }
		
		inline Mat4& set(GLfloat v) { Mat4Set(THIS.v, v); return *this; }
		inline Mat4& set(const Mat4& m) { Mat4SetV(THIS.v, m.v); return *this; }
		inline Mat4& identity() { Mat4Identity(THIS.v); return *this; }
		inline Mat4& translate(const Vec3& v) { Mat4Translate(THIS.v, v.x, v.y, v.z); return *this; }
		inline Mat4& scale(const Vec3& v) { Mat4Scale(THIS.v, v.x, v.y, v.z); return *this; }
		inline Mat4& rotateX(GLfloat a) { Mat4RotateX(THIS.v, a); return *this; }
		inline Mat4& rotateY(GLfloat a) { Mat4RotateY(THIS.v, a); return *this; }
		inline Mat4& rotateZ(GLfloat a) { Mat4RotateZ(THIS.v, a); return *this; }
		inline Mat4& frustum(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f) { Mat4Frustum(THIS.v, l,r,b,t,n,f); return *this; }
		inline Mat4& perspective(GLfloat fov, GLfloat aspect, GLfloat near, GLfloat far) { Mat4Perspective(THIS.v, fov, aspect, near, far); return *this; }
		inline Mat4& ortho(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f) { Mat4Ortho(THIS.v, l,r,b,t,n,f); return *this; }
		inline Mat4& rotate(const Vec3& v, GLfloat a) { Mat4Rotate(THIS.v, v.x, v.y, v.z, a); return *this; }
		inline Mat4& lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) { Mat4LookAt(THIS.v, eye.v, center.v, up.v); return *this; }

		inline Mat4& inverse() { Mat4Inverse(THIS.v, THIS.v); return *this; }
		inline Mat4& inverse(const Mat4& m) { Mat4Inverse(THIS.v, m.v); return *this; }
		inline Mat4& transpose() { Mat4Transpose(THIS.v, THIS.v); return *this; }
		inline Mat4& transpose(const Mat4& m) { Mat4Transpose(THIS.v, m.v); return *this; }
		inline Mat4& multiply(const Mat4& m) { Mat4Multiply(THIS.v, THIS.v, m.v); return *this; }
		inline Mat4& multiply(const Mat4& a, const Mat4& b) { Mat4Multiply(THIS.v, a.v, b.v); return *this; }
	};
	
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <core/uMemory.h>

template <typename Type>
class Vector {
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
	
	virtual inline Type& push(const Type& value) throw(const char*) {
		resize(m_count + 1);
		return (m_items[m_count - 1] = value);
	}
	
	virtual inline Type& unshift(const Type& value) throw(const char*) {
		resize(m_count + 1);
		memmove(m_items + 1, m_items, (m_count - 1) * sizeof(Type));
		return (m_items[0] = value);
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
	
	virtual inline bool contains(const Type& value) const {
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
	
	// synonym to count()
	virtual inline int32_t size() const {
		return m_count;
	}
	
	virtual inline bool empty() const {
		return (m_count == 0);
	}
	
	// synonym to empty()
	virtual inline bool isEmpty() const {
		return (m_count == 0);
	}
	
	virtual inline void clear() throw(const char*) {
		resize(0);
	}
	
	virtual inline const Type& peek() const throw(const char*) {
		if (m_count > 0) {
			return m_items[m_count - 1];
		}
		throw eEmpty;
	}
	
	virtual inline const Type& peek(int32_t index) const throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			return m_items[index];
		}
		throw eOutOfRange;
	}
	
	// synonym to peek(index)
	virtual inline const Type& get(int32_t index) const throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			return m_items[index];
		}
		throw eOutOfRange;
	}
	
	virtual inline int32_t search(const Type& value) const {
		if (m_count > 0) {
			for (int i = m_count-1; i >= 0; i--) {
				if (m_items[i] == value) return i;
			}
		}
		return -1;
	}
	
	// synonym to search(value)
	virtual inline int32_t indexOf(const Type& value) const {
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

#endif

#endif //JAPPSY_UVECTOR_H