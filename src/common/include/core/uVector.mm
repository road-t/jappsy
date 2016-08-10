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

#include "uVector.h"

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif
	
	void Vec3Set(float* d, float v) {
		d[0]=d[1]=d[2]=v;
	}
	
	void Vec4Set(float* d, float v) {
		d[0]=d[1]=d[2]=d[3] = v;
	}

	void Vec3SetV(float* d, const float* v) {
		d[0]=v[0];d[1]=v[1];d[2]=v[2];
	}
	
	void Vec4SetV(float* d, const float* v) {
		d[0]=v[0];d[1]=v[1];d[2]=v[2];d[3]=v[3];
	}
	
	void Vec3Negative(float* d, const float* s) {
		d[0]=-s[0];d[1]=-s[1];d[2]=-s[2];
	}
	
	void Vec4Negative(float* d, const float* s) {
		d[0]=-s[0];d[1]=-s[1];d[2]=-s[2];d[3]=-s[3];
	}
	
	void Vec3Add(float* d, const float* a, float b) {
		d[0]=a[0]+b;d[1]=a[1]+b;d[2]=a[2]+b;
	}
	
	void Vec4Add(float* d, const float* a, float b) {
		d[0]=a[0]+b;d[1]=a[1]+b;d[2]=a[2]+b;d[3]=a[3]+b;
	}
	
	void Vec3AddV(float* d, const float* a, const float* b) {
		d[0]=a[0]+b[0];d[1]=a[1]+b[1];d[2]=a[2]+b[2];
	}
	
	void Vec4AddV(float* d, const float* a, const float* b) {
		d[0]=a[0]+b[0];d[1]=a[1]+b[1];d[2]=a[2]+b[2];d[3]=a[3]+b[3];
	}
	
	void Vec3Subtract(float* d, const float* a, float b) {
		d[0]=a[0]-b;d[1]=a[1]-b;d[2]=a[2]-b;
	}
	
	void Vec4Subtract(float* d, const float* a, float b) {
		d[0]=a[0]-b;d[1]=a[1]-b;d[2]=a[2]-b;d[3]=a[3]-b;
	}
	
	void Vec3SubtractV(float* d, const float* a, const float* b) {
		d[0]=a[0]-b[0];d[1]=a[1]-b[1];d[2]=a[2]-b[2];
	}
	
	void Vec4SubtractV(float* d, const float* a, const float* b) {
		d[0]=a[0]-b[0];d[1]=a[1]-b[1];d[2]=a[2]-b[2];d[3]=a[3]-b[3];
	}
	
	void Vec3Multiply(float* d, const float* a, float b) {
		d[0]=a[0]*b;d[1]=a[1]*b;d[2]=a[2]*b;
	}
	
	void Vec4Multiply(float* d, const float* a, float b) {
		d[0]=a[0]*b;d[1]=a[1]*b;d[2]=a[2]*b;d[3]=a[3]*b;
	}
	
	void Vec3MultiplyV(float* d, const float* a, const float* b) {
		d[0]=a[0]*b[0];d[1]=a[1]*b[1];d[2]=a[2]*b[2];
	}
	
	void Vec4MultiplyV(float* d, const float* a, const float* b) {
		d[0]=a[0]*b[0];d[1]=a[1]*b[1];d[2]=a[2]*b[2];d[3]=a[3]*b[3];
	}
	
	void Vec3Divide(float* d, const float* a, float b) {
		if (b != 0) {
			d[0]=a[0]/b;d[1]=a[1]/b;d[2]=a[2]/b;
		} else {
			for (int i = 0; i < 3; i++)
				d[i] = (d[i] < 0) ? -INFINITY : INFINITY;
		}
	}
	
	void Vec4Divide(float* d, const float* a, float b) {
		if (b != 0) {
			d[0]=a[0]/b;d[1]=a[1]/b;d[2]=a[2]/b;d[3]=a[3]/b;
		} else {
			for (int i = 0; i < 4; i++)
				d[i] = (d[i] < 0) ? -INFINITY : INFINITY;
		}
	}
	
	void Vec3DivideV(float* d, const float* a, const float* b) {
		for (int i = 0; i < 3; i++) {
			if (b[i] != 0)
				d[i] = a[i]/b[i];
			else
				d[i] = (d[i] < 0) ? -INFINITY : INFINITY;
		}
	}
	
	void Vec4DivideV(float* d, const float* a, const float* b) {
		for (int i = 0; i < 4; i++) {
			if (b[i] != 0)
				d[i] = a[i]/b[i];
			else
				d[i] = (d[i] < 0) ? -INFINITY : INFINITY;
		}
	}
	
	float Vec3Dot(const float* a, const float* b) {
		return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
	}
	
	float Vec4Dot(const float* a, const float* b) {
		return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3];
	}
	
	void Vec3Cross(float* d, const float* a, const float* b) {
		if ((d != a) && (d != b)) {
			d[0]=a[1]*b[2]-a[2]*b[1];
			d[1]=a[2]*b[0]-a[0]*b[2];
			d[2]=a[0]*b[1]-a[1]*b[0];
		} else {
			float a0 = a[0], a1 = a[1], b0 = b[0], b1 = b[1];
			d[0]=a[1]*b[2]-a[2]*b[1];
			d[1]=a[2]*b0-a0*b[2];
			d[2]=a0*b1-a1*b0;
		}
	}
	
	float Vec3Size(const float* v) {
		return sqrtf(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	}
	
	void Vec3Normalize(float* d, const float* v) {
		float l = sqrtf(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
		if (l > 0) {
			d[0]=v[0]/l;d[1]=v[1]/l;d[2]=v[2]/l;
		} else {
			d[0]=d[1]=d[2] = 0;
		}
	}
	
	void Vec4Normalize(float* d, const float* v) {
		float l = sqrtf(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]+v[3]*v[3]);
		if (l > 0) {
			d[0]=v[0]/l;d[1]=v[1]/l;d[2]=v[2]/l;d[3]=v[3]/l;
		} else {
			d[0]=d[1]=d[2]=d[3] = 0;
		}
	}
	
	float Vec3Min(const float* v) {
		return (v[0] < v[1]) ? (v[0] < v[2] ? v[0] : v[2]) : (v[1] < v[2] ? v[1] : v[2]);
	}
	
	float Vec3Max(const float* v) {
		return (v[0] > v[1]) ? (v[0] > v[2] ? v[0] : v[2]) : (v[1] > v[2] ? v[1] : v[2]);
	}
	
	void Vec3Angles(const float* v, float* theta, float* phi) {
		*theta = atan2f(v[2], v[0]);
		*phi = asinf(v[1] / sqrtf(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]));
	}
	
	float Vec3Angle(const float* a, const float* b) {
		return acosf((a[0]*b[0]+a[1]*b[1]+a[2]*b[2]) / (sqrtf(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]) * sqrtf(b[0]*b[0]+b[1]*b[1]+b[2]*b[2])));
	}
	
	void Vec3Transform(float* d, const float* p, const float* m) {
		float s = (m[3]*p[0]+m[7]*p[1]+m[11]*p[2]+m[15]);
		if (d != p) {
			d[0]=(m[0]*p[0]+m[4]*p[1]+m[8]*p[2]+m[12])/s;
			d[1]=(m[1]*p[0]+m[5]*p[1]+m[9]*p[2]+m[13])/s;
			d[2]=(m[2]*p[0]+m[6]*p[1]+m[10]*p[2]+m[14])/s;
		} else {
			float p0 = p[0], p1 = p[1], p2 = p[2];
			d[0]=(m[0]*p0+m[4]*p1+m[8]*p2+m[12])/s;
			d[1]=(m[1]*p0+m[5]*p1+m[9]*p2+m[13])/s;
			d[2]=(m[2]*p0+m[6]*p1+m[10]*p2+m[14])/s;
		}
	}
	
	void Vec3TransformNormal(float* d, const float* v, const float* m) {
		if (d != v) {
			d[0]=m[0]*v[0]+m[4]*v[1]+m[8]*v[2];
			d[1]=m[1]*v[0]+m[5]*v[1]+m[9]*v[2];
			d[2]=m[2]*v[0]+m[6]*v[1]+m[10]*v[2];
		} else {
			float v0 = v[0], v1 = v[1], v2 = v[2];
			d[0]=m[0]*v0+m[4]*v1+m[8]*v2;
			d[1]=m[1]*v0+m[5]*v1+m[9]*v2;
			d[2]=m[2]*v0+m[6]*v1+m[10]*v2;
		}
	}
	
	void Vec3Random(float* d, float x, float y, float z) {
		float l;
		do {
			d[0] = (float)rand() / RAND_MAX - x;
			d[1] = (float)rand() / RAND_MAX - y;
			d[2] = (float)rand() / RAND_MAX - z;
			l = sqrtf(d[0]*d[0]+d[1]*d[1]+d[2]*d[2]);
		} while (l < 0.01);
		d[0]/=l;d[1]/=l;d[2]/=l;
	}
	
	Vec3& Vec3::transform(const Mat4& m) {
		Vec3Transform(this->v, this->v, m.v);
		return *this;
	}
	
	Vec3& Vec3::transform(const Vec3& v, const Mat4& m) {
		Vec3Transform(this->v, v.v, m.v);
		return *this;
	}
	
	Vec3& Vec3::transformNormal(const Mat4& m) {
		Vec3TransformNormal(this->v, this->v, m.v);
		return *this;
	}
	
	Vec3& Vec3::transformNormal(const Vec3& v, const Mat4& m) {
		Vec3TransformNormal(this->v, v.v, m.v);
		return *this;
	}
	
	void Mat4Set(float* d, float v) {
		for (int i = 0; i < 16; i++)
			d[i] = v;
	}
	
	void Mat4SetV(float* d, const float* m) {
		for (int i = 0; i < 16; i++)
			d[i] = m[i];
	}
	
	void Mat4Identity(float* d) {
		d[0]=d[5]=d[10]=d[15]=1.0;
		d[1]=d[2]=d[3]=d[4]=d[6]=d[7]=d[8]=d[9]=d[11]=d[12]=d[13]=d[14]=0.0;
	}
	
	void Mat4Translate(float* d, float x, float y, float z) {
		d[0]=d[5]=d[10]=d[15]=1.0;
		d[1]=d[2]=d[3]=d[4]=d[6]=d[7]=d[8]=d[9]=d[11]=0.0;
		d[12]=x;d[13]=y;d[14]=z;
	}
	
	void Mat4Scale(float* d, float x, float y, float z) {
		d[0]=z;d[5]=y;d[10]=x;d[15]=1.0;
		d[1]=d[2]=d[3]=d[4]=d[6]=d[7]=d[8]=d[9]=d[11]=d[12]=d[13]=d[14]=0.0;
	}
	
#define __TORADIANS (M_PI / 180.0f)
	
	void Mat4RotateX(float* d, float a) {
		a = a * __TORADIANS;
		d[0]=d[15]=1.0;
		d[1]=d[2]=d[3]=d[4]=d[7]=d[8]=d[11]=d[12]=d[13]=d[14]=0.0;
		d[5]=d[10]=cosf(a);
		d[6]=sinf(a);
		d[9]=-d[6];
	}
	
	void Mat4RotateY(float* d, float a) {
		a = a * __TORADIANS;
		d[5]=d[15]=1.0;
		d[1]=d[3]=d[4]=d[6]=d[7]=d[9]=d[11]=d[12]=d[13]=d[14]=0.0;
		d[0]=d[10]=cosf(a);
		d[8]=sinf(a);
		d[2]=-d[8];
	}
	
	void Mat4RotateZ(float* d, float a) {
		a = a * __TORADIANS;
		d[10]=d[15]=1.0;
		d[2]=d[3]=d[6]=d[7]=d[8]=d[9]=d[11]=d[12]=d[13]=d[14]=0.0;
		d[0]=d[5]=cosf(a);
		d[1]=sinf(a);
		d[4]=-d[1];
	}

	void Mat4Frustum(float* d, float l, float r, float b, float t, float n, float f) {
		float rl = (r-l);
		if (rl != 0) {
			d[0]=2.0*n/rl;
			d[8]=(r+l)/rl;
		} else {
			d[0]=d[8]=0;
		}
		float tb = (t-b);
		if (tb != 0) {
			d[5]=2.0*n/tb;
			d[9]=(t+b)/tb;
		} else {
			d[5]=d[9]=0;
		}
		float fn = (f-n);
		if (fn != 0) {
			d[10]=-(f+n)/fn;
			d[14]=-2.0*f*n/fn;
		} else {
			d[10]=d[14]=0;
		}
		d[11]=-1.0;
		d[1]=d[2]=d[3]=d[4]=d[6]=d[7]=d[12]=d[13]=d[15]=0.0;
	}
	
	void Mat4Perspective(float* d, float fov, float aspect, float near, float far) {
		/*
		 float y = tanf(fov * M_PI / 360.0) * near;
		 float x = y * aspect;
		 Mat4Frustum(d, -x,x,-y,y,near,far);
		 return;
		 */
		bool leftHanded = true;
		float frustumDepth = far - near;
		float oneOverDepth = 1.0 / frustumDepth;
		
		/* fovY
		 d[5] = 1.0 / tanf(fov * M_PI / 360.0);
		 d[0] = (leftHanded ? 1.0 : -1.0) * d[5] / aspect;
		 */
		float t = tanf(fov * M_PI / 360.0);
		if (t != 0) {
			d[0] = (leftHanded ? 1.0 : -1.0) / t;
		} else {
			d[0]=0;
		}
		d[5] = (leftHanded ? 1.0 : -1.0) * d[0] * aspect;
		d[10] = -far * oneOverDepth;
		d[11] = -1.0;
		d[14] = -2.0 * far * near * oneOverDepth;
		d[1]=d[2]=d[3]=d[4]=d[6]=d[7]=d[8]=d[9]=d[12]=d[13]=d[15]=0.0;
	}
	
	void Mat4Ortho(float* d, float l, float r, float b, float t, float n, float f) {
		float rl = (r-l);
		if (rl != 0) {
			d[0]=2.0/rl;
			d[12]=-(r+l)/rl;
		} else {
			d[0]=d[12]=0;
		}
		float tb = (t-b);
		if (tb != 0) {
			d[5]=2.0/tb;
			d[13]=-(t+b)/tb;
		} else {
			d[5]=d[13]=0;
		}
		float fn = (f-n);
		if (fn != 0) {
			d[10]=-2.0/fn;
			d[14]=-(f+n)/fn;
		} else {
			d[10]=d[14]=0;
		}
		d[15]=1.0;
		d[1]=d[2]=d[3]=d[4]=d[6]=d[7]=d[8]=d[9]=d[11]=0.0;
	}

	void Mat4Rotate(float* d, float x, float y, float z, float a) {
		a *= __TORADIANS;
		float c = cosf(a), s = sinf(a), t = 1.0-c;
		float xt=x*t, yt=y*t, zt=z*t, xs=x*s, ys=y*s, zs=z*s;
		d[0]=x*xt+c;
		d[1]=y*xt+zs;
		d[2]=z*xt-ys;
		d[4]=x*yt-zs;
		d[5]=y*yt+c;
		d[6]=z*yt+xs;
		d[8]=x*zt+ys;
		d[9]=y*zt-xs;
		d[10]=z*zt+c;
		d[15]=1.0;
		d[3]=d[7]=d[11]=d[12]=d[13]=d[14]=0.0;
	}
	
	void Mat4LookAt(float* d, const float* eye, const float* center, const float* up) {
		float z[3];
		Vec3SetV(z, eye);
		Vec3SubtractV(z, z, center);
		Vec3Normalize(z, z);
		
		float x[3];
		Vec3Cross(x, up, z);
		Vec3Normalize(x, x);
		
		float y[3];
		Vec3Cross(y, z, x);
		Vec3Normalize(y, y);
		
		d[0]=x[0];d[1]=y[0];d[2]=z[0];
		d[4]=x[1];d[5]=y[1];d[6]=z[1];
		d[8]=x[2];d[9]=y[2];d[10]=z[2];
		d[12]=-Vec3Dot(x, eye);
		d[13]=-Vec3Dot(y, eye);
		d[14]=-Vec3Dot(z, eye);
		d[15]=1.0;
		d[3]=d[7]=d[11]=0.0;
	}
	
	void Mat4Inverse(float* d, const float* s) {
		float tmp[16];
		if (d == s) {
			Mat4SetV(tmp, s);
			s = tmp;
		}
		
		float a0 = s[0]*s[5]-s[4]*s[1];
		float a1 = s[0]*s[6]-s[4]*s[2];
		float a2 = s[0]*s[7]-s[4]*s[3];
		float a3 = s[1]*s[6]-s[5]*s[2];
		float a4 = s[1]*s[7]-s[5]*s[3];
		float a5 = s[2]*s[7]-s[6]*s[3];
		
		float b0 = s[8]*s[13]-s[12]*s[9];
		float b1 = s[8]*s[14]-s[12]*s[10];
		float b2 = s[8]*s[15]-s[12]*s[11];
		float b3 = s[9]*s[14]-s[13]*s[10];
		float b4 = s[9]*s[15]-s[13]*s[11];
		float b5 = s[10]*s[15]-s[14]*s[11];
		
		float det = a0*b5-a1*b4+a2*b3+a3*b2-a4*b1+a5*b0;
		float invdet = (det == 0) ? 0 : (1.0 / det);
		
		d[0] = (s[5]*b5-s[6]*b4+s[7]*b3) * invdet;
		d[1] = (-s[1]*b5+s[2]*b4-s[3]*b3) * invdet;
		d[2] = (s[13]*a5-s[14]*a4+s[15]*a3) * invdet;
		d[3] = (-s[9]*a5+s[10]*a4-s[11]*a3) * invdet;
		
		d[4] = (-s[4]*b5+s[6]*b2-s[7]*b1) * invdet;
		d[5] = (s[0]*b5-s[2]*b2+s[3]*b1) * invdet;
		d[6] = (-s[12]*a5+s[14]*a2-s[15]*a1) * invdet;
		d[7] = (s[8]*a5-s[10]*a2+s[11]*a1) * invdet;
		
		d[8] = (s[4]*b4-s[5]*b2+s[7]*b0) * invdet;
		d[9] = (-s[0]*b4+s[1]*b2-s[3]*b0) * invdet;
		d[10] = (s[12]*a4-s[13]*a2+s[15]*a0) * invdet;
		d[11] = (-s[8]*a4+s[9]*a2-s[11]*a0) * invdet;
		
		d[12] = (-s[4]*b3+s[5]*b1-s[6]*b0) * invdet;
		d[13] = (s[0]*b3-s[1]*b1+s[2]*b0) * invdet;
		d[14] = (-s[12]*a3+s[13]*a1-s[14]*a0) * invdet;
		d[15] = (s[8]*a3-s[9]*a1+s[10]*a0) * invdet;
	}
	
	void Mat4Transpose(float* d, const float* s) {
		if (d != s) {
			d[0]=s[0]; d[1]=s[4]; d[2]=s[8]; d[3]=s[12];
			d[4]=s[1]; d[5]=s[5]; d[6]=s[9]; d[7]=s[13];
			d[8]=s[2]; d[9]=s[6]; d[10]=s[10]; d[11]=s[14];
			d[12]=s[3]; d[13]=s[7]; d[14]=s[11]; d[15]=s[15];
		} else {
			float t;
			t=d[1];d[1]=d[4];d[4]=t;
			t=d[2];d[2]=d[8];d[8]=t;
			t=d[3];d[3]=d[12];d[12]=t;
			t=d[6];d[6]=d[9];d[9]=t;
			t=d[7];d[7]=d[13];d[13]=t;
			t=d[11];d[11]=d[14];d[14]=t;
		}
	}
	
	void Mat4Multiply(float* d, const float* a, const float* b) {
		float tmp[16];
		
		if ((d == a) || (d == b)) {
			Mat4SetV(tmp, d);
			if (d == a)
				a = tmp;
			if (d == b)
				b = tmp;
		}
		
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
	}
	
	void Mat4DecompositTranslate(float* d, float* m) {
		d[0] = m[12];
		d[1] = m[13];
		d[2] = m[14];
		m[12] = m[13] = m[14] = 0;
	}
	
	void Mat4DecompositScale(float* d, float* m) {
		d[0] = Vec3Size(&(m[0]));
		d[1] = Vec3Size(&(m[4]));
		d[2] = Vec3Size(&(m[8]));
		if (d[0] != 0) {
			m[0] /= d[0]; m[1] /= d[0]; m[2] /= d[0];
		} else {
			m[0]=m[1]=m[2]=0;
		}
		if (d[1] != 0) {
			m[4] /= d[1]; m[5] /= d[1]; m[6] /= d[1];
		} else {
			m[4]=m[5]=m[6]=0;
		}
		if (d[2] != 0) {
			m[8] /= d[2]; m[9] /= d[2]; m[10] /= d[2];
		} else {
			m[8]=m[9]=m[10]=0;
		}
	}

	void Mat4DecompositRotate(float* d, float* m) {
		float m14 = m[1]+m[4], m28 = m[2]+m[8], m69 = m[6]+m[9];
		float c1 = m[0] - ((m69 != 0) ? m14*m28/(2*m69) : 0);
		float c2 = m[5] - ((m28 != 0) ? m14*m69/(2*m28) : 0);
		float c3 = m[10] - ((m14 != 0) ? m28*m69/(2*m14) : 0);
		float c = (c1+c2+c3)/3;
		
		float s = sqrtf(1 - c*c);
		if (s != 0) {
			d[0] = (m[6]-m[9])/(2*s);
			d[1] = (m[8]-m[2])/(2*s);
			d[2] = (m[1]-m[4])/(2*s);
		} else {
			d[0]=d[1]=d[2]=0;
		}
		
		d[3] = acosf(c) / __TORADIANS;
		
		float trans[16];
		Mat4Rotate(trans, d[0], d[1], d[2], d[3]);
		Mat4Inverse(trans, trans);
		Mat4Multiply(m, m, trans);
	}

#ifdef __cplusplus
}
#endif

