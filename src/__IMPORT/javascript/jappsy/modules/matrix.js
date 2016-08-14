Mat4 = (function(){
	function Mat4(){
		var obj = Object.create(Float32Array.prototype);
		if (arguments.length == 0)
			obj = Float32Array.apply(obj, [16]);
		else
			obj = (Float32Array.apply(obj, arguments) || Float32Array.apply(obj, [16]));
		Mat4.injectClassMethods(obj);
		return obj;
	}
	
	Mat4.injectClassMethods = function(obj){
		for (var method in Mat4.prototype)
			if (Mat4.prototype.hasOwnProperty(method))
				obj[method] = Mat4.prototype[method];
		return obj;
	};
	
	Mat4.isArray = function(val) {
		return (val instanceof Float32Array) || (Array.isArray(val));
	};
	
	Mat4.set = function(d, m) {
		d[0]=m[0];d[1]=m[1];d[2]=m[2];d[3]=m[3];
		d[4]=m[4];d[5]=m[5];d[6]=m[6];d[7]=m[7];
		d[8]=m[8];d[9]=m[9];d[10]=m[10];d[11]=m[11];
		d[12]=m[12];d[13]=m[13];d[14]=m[14];d[15]=m[15];
		return d;
	};
	
	Mat4.identity = function(d) {
		if (d === undefined) d = new Mat4();
		d[0]=d[5]=d[10]=d[15]=1.0;
		d[1]=d[2]=d[3]=d[4]=d[6]=d[7]=d[8]=d[9]=d[11]=d[12]=d[13]=d[14]=0.0;
		return d;
	};
	
	Mat4.translate = function(d, x,y,z) {
		d[0]=d[5]=d[10]=d[15]=1.0;
		d[1]=d[2]=d[3]=d[4]=d[6]=d[7]=d[8]=d[9]=d[11]=0.0;
		d[12]=x;d[13]=y;d[14]=z;
		return d;
	};

	Mat4.scale = function(d, x,y,z) {
		d[0]=z;d[5]=y;d[10]=x;d[15]=1.0;
		d[1]=d[2]=d[3]=d[4]=d[6]=d[7]=d[8]=d[9]=d[11]=d[12]=d[13]=d[14]=0.0;
		return d;
	};
	
	Mat4.__TORADIANS = Math.PI / 180.0;

	Mat4.rotateX = function (d, a) {
		a = a * Mat4.__TORADIANS;
		d[0]=d[15]=1.0;
		d[1]=d[2]=d[3]=d[4]=d[7]=d[8]=d[11]=d[12]=d[13]=d[14]=0.0;
		d[5]=d[10]=Math.cos(a);
		d[6]=Math.sin(a);
		d[9]=-d[6];
		return d;
	};

	Mat4.rotateY = function (d, a) {
		a = a * Mat4.__TORADIANS;
		d[5]=d[15]=1.0;
		d[1]=d[3]=d[4]=d[6]=d[7]=d[9]=d[11]=d[12]=d[13]=d[14]=0.0;
		d[0]=d[10]=Math.cos(a);
		d[8]=Math.sin(a);
		d[2]=-d[8];
		return d;
	};

	Mat4.rotateZ = function (d, a) {
		a = a * Mat4.__TORADIANS;
		d[10]=d[15]=1.0;
		d[2]=d[3]=d[6]=d[7]=d[8]=d[9]=d[11]=d[12]=d[13]=d[14]=0.0;
		d[0]=d[5]=Math.cos(a);
		d[1]=Math.sin(a);
		d[4]=-d[1];
		return d;
	};

	Mat4.frustum = function (d, l,r,b,t,n,f) {
		var rl = (r-l);
		d[0]=2.0*n/rl;
		d[8]=(r+l)/rl;
		var tb = (t-b);
		d[5]=2.0*n/tb;
		d[9]=(t+b)/tb;
		var fn = (f-n);
		d[10]=-(f+n)/fn;
		d[14]=-2.0*f*n/fn;
		d[11]=-1.0;
		d[1]=d[2]=d[3]=d[4]=d[6]=d[7]=d[12]=d[13]=d[15]=0.0;
		return d;
	};
	
	Mat4.perspective = function (d, fov,aspect,near,far) {
		/*
		var y = Math.tan(fov * Math.PI / 360.0) * near;
		var x = y * aspect;
		return Mat4.frustum(d, -x,x,-y,y,near,far);
		*/
		var leftHanded = true;
		var frustumDepth = far - near;
		var oneOverDepth = 1.0 / frustumDepth;
		
		/* fovY
		d[5] = 1.0 / Math.tan(fov * Math.PI / 360.0);
		d[0] = (leftHanded ? 1.0 : -1.0) * d[5] / aspect;
		*/
		d[0] = (leftHanded ? 1.0 : -1.0) / Math.tan(fov * Math.PI / 360.0);
		d[5] = (leftHanded ? 1.0 : -1.0) * d[0] * aspect;
		d[10] = -far * oneOverDepth;
		d[11] = -1.0;
		d[14] = -2.0 * far * near * oneOverDepth;
		d[1]=d[2]=d[3]=d[4]=d[6]=d[7]=d[8]=d[9]=d[12]=d[13]=d[15]=0.0;
		return d;
	};

	Mat4.ortho = function (d, l,r,b,t,n,f) {
		var rl = (r-l);
		d[0]=2.0/rl;
		d[12]=-(r+l)/rl;
		var tb = (t-b);
		d[5]=2.0/tb;
		d[13]=-(t+b)/tb;
		var fn = (f-n);
		d[10]=-2.0/fn;
		d[14]=-(f+n)/fn;
		d[15]=1.0;
		d[1]=d[2]=d[3]=d[4]=d[6]=d[7]=d[8]=d[9]=d[11]=0.0;
		return d;
	};

	Mat4.rotate = function (d, x,y,z,a) {
		a *= Mat4.__TORADIANS;
		var c = Math.cos(a), s = Math.sin(a), t = 1.0-c;
		var xt=x*t, yt=y*t, zt=z*t, xs=x*s, ys=y*s, zs=z*s;
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
		return d;
	};

	// Put camera at the eye point looking toward the center point with an up direction
	Mat4.lookAt = function (d, eye,center,up) {
		var z = new Vec3(eye).subtract(center).normalize();
		var x = new Vec3().cross(up,z).normalize();
		var y = new Vec3().cross(z,x).normalize();
	
		d[0]=x[0];d[1]=y[0];d[2]=z[0];
		d[4]=x[1];d[5]=y[1];d[6]=z[1];
		d[8]=x[2];d[9]=y[2];d[10]=z[2];
		d[12]=-x.dot(eye);
		d[13]=-y.dot(eye);
		d[14]=-z.dot(eye);
		d[15]=1.0;
		d[3]=d[7]=d[11]=0.0;
		return d;
	};

	Mat4.inverse = function (d, s) {
		if (d === s)
			return Mat4.inverse(d, new Mat4(s));
		
		var a0 = s[0]*s[5]-s[4]*s[1];
		var a1 = s[0]*s[6]-s[4]*s[2];
		var a2 = s[0]*s[7]-s[4]*s[3];
		var a3 = s[1]*s[6]-s[5]*s[2];
		var a4 = s[1]*s[7]-s[5]*s[3];
		var a5 = s[2]*s[7]-s[6]*s[3];
		
		var b0 = s[8]*s[13]-s[12]*s[9];
		var b1 = s[8]*s[14]-s[12]*s[10];
		var b2 = s[8]*s[15]-s[12]*s[11];
		var b3 = s[9]*s[14]-s[13]*s[10];
		var b4 = s[9]*s[15]-s[13]*s[11];
		var b5 = s[10]*s[15]-s[14]*s[11];
		
		var invdet = 1.0/(a0*b5-a1*b4+a2*b3+a3*b2-a4*b1+a5*b0);
		
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
		
		return d;
	};

	Mat4.transpose = function (d, s) {
		if (d !== s) {
			d[0]=s[0]; d[1]=s[4]; d[2]=s[8]; d[3]=s[12];
			d[4]=s[1]; d[5]=s[5]; d[6]=s[9]; d[7]=s[13];
			d[8]=s[2]; d[9]=s[6]; d[10]=s[10]; d[11]=s[14];
			d[12]=s[3]; d[13]=s[7]; d[14]=s[11]; d[15]=s[15];
		} else {
			var t;
			t=d[1];d[1]=d[4];d[4]=t;
			t=d[2];d[2]=d[8];d[8]=t;
			t=d[3];d[3]=d[12];d[12]=t;
			t=d[6];d[6]=d[9];d[9]=t;
			t=d[7];d[7]=d[13];d[13]=t;
			t=d[11];d[11]=d[14];d[14]=t;
		}
		
		return d;
	};

	Mat4.multiply = function (d, a, b) {
		if ((d === a) || (d === b)) {
			return Mat4.multiply(d, (d === a) ? new Mat4(a) : a, (d === b) ? new Mat4(b) : b);
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
		
		return d;
	};
	
	Mat4.decompositTranslate = function(d, m) {
		d[0] = m[12];
		d[1] = m[13];
		d[2] = m[14];
		m[12] = m[13] = m[14] = 0;
		return d;
	};
	
	Mat4.decompositScale = function(d, m) {
		d[0] = Vec3.size([m[0], m[1], m[2]]);
		d[1] = Vec3.size([m[4], m[5], m[6]]);
		d[2] = Vec3.size([m[8], m[9], m[10]]);
		m[0] /= d[0]; m[1] /= d[0]; m[2] /= d[0];
		m[4] /= d[1]; m[5] /= d[1]; m[6] /= d[1];
		m[8] /= d[2]; m[9] /= d[2]; m[10] /= d[2];
		return d;
	};
	
	Mat4.decompositRotate = function(d, m) {
		var m14 = m[1]+m[4], m28 = m[2]+m[8], m69 = m[6]+m[9];
		var c1 = m[0] - m14*m28/(2*m69);
		var c2 = m[5] - m14*m69/(2*m28);
		var c3 = m[10] - m28*m69/(2*m14);
		var c = (c1+c2+c3)/3;
		
		var s = Math.sqrt(1 - c*c);
		
		d[0] = (m[6]-m[9])/(2*s);
		d[1] = (m[8]-m[2])/(2*s);
		d[2] = (m[1]-m[4])/(2*s);
		
		d[3] = Math.acos(c) / Mat4.__TORADIANS;
		
		var trans = new Mat4().rotate(d, d[3]).inverse();
		Mat4.multiply(m, m, trans);
		
		return d;
	};

	Mat4.prototype = {
		dup: function() {
			return new Mat4(this);
		},
		
		// Initializers
		set:			function(m) { return Mat4.set(this, m); },
		identity:		function() { return Mat4.identity(this); },
		translate:		function(vec3) { return Mat4.translate(this, vec3[0], vec3[1], vec3[2]); },
		scale:			function(vec3) { return Mat4.scale(this, vec3[0], vec3[1], vec3[2]); },
		rotateX:		function(a) { return Mat4.rotateX(this, a); },
		rotateY:		function(a) { return Mat4.rotateX(this, a); },
		rotateZ:		function(a) { return Mat4.rotateX(this, a); },
		frustum:		function(l,r,b,t,n,f) { return Mat4.frustum(this, l,r,b,t,n,f); },
		perspective:	function(fov,aspect,near,far) { return Mat4.perspective(this, fov,aspect,near,far); },
		ortho:			function(l,r,b,t,n,f) { return Mat4.ortho(this, l,r,b,t,n,f); },
		rotate:			function(v,a) { return Mat4.rotate(this, v[0],v[1],v[2],a); },
		lookAt:			function(eye,center,up) { return Mat4.lookAt(this, eye,center,up); },
		
		// Operations
		inverse:		function(m) { if (m !== undefined) return Mat4.inverse(this, m); return Mat4.inverse(this, this); },
		transpose:		function(m) { if (m !== undefined) return Mat4.transpose(this, m); return Mat4.transpose(this, this); },
		multiply:		function(a,b) { if (b !== undefined) return Mat4.multiply(this, a,b); return Mat4.multiply(this, this,a); },
	};
	
	return Mat4;
}).call({});

Vec3 = (function(){
	function Vec3(){
		var obj = Object.create(Float32Array.prototype);
		if (arguments.length == 0)
			obj = Float32Array.apply(obj, [3]);
		else {
			var args = Array.prototype.slice.call(arguments);
			var v = [];
			args.forEach(function(arg){
				if ((arg instanceof Float32Array) || (Array.isArray(arg))) {
					Array.prototype.push.apply(v, arg);
				} else {
					v.push(arg);
				}
			});
			while (v.length < 3) v.push(0);
			if (v.length > 3)
				obj = (Float32Array.apply(obj, [v.slice(0, 3)]) || Float32Array.apply(obj, [3]));
			else
				obj = (Float32Array.apply(obj, [v]) || Float32Array.apply(obj, [3]));
		}
		Vec3.injectClassMethods(obj);
		return obj;
	}
	
	Vec3.injectClassMethods = function(obj){
		for (var method in Vec3.prototype)
			if (Vec3.prototype.hasOwnProperty(method))
				obj[method] = Vec3.prototype[method];
		return obj;
	};
	
	Vec3.isArray = function(val) {
		return (val instanceof Float32Array) || (Array.isArray(val));
	};
	
	Vec3.set = function (d, v) {
		d[0]=v[0];d[1]=v[1];d[2]=v[2];
		return d;
	};
	
	Vec3.negative = function (d, s) {
		d[0]=-s[0];d[1]=-s[1];d[2]=-s[2];
		return d;
	};

	Vec3.add = function (d, a,b) {
		if (Vec3.isArray(b)) {
			d[0]=a[0]+b[0];d[1]=a[1]+b[1];d[2]=a[2]+b[2];
		} else {
			d[0]=a[0]+b;d[1]=a[1]+b;d[2]=a[2]+b;
		}
		return d;
	};

	Vec3.subtract = function (d, a,b) {
		if (Vec3.isArray(b)) {
			d[0]=a[0]-b[0];d[1]=a[1]-b[1];d[2]=a[2]-b[2];
		} else {
			d[0]=a[0]-b;d[1]=a[1]-b;d[2]=a[2]-b;
		}
		return d;
	};

	Vec3.multiply = function (d, a,b) {
		if (Vec3.isArray(b)) {
			d[0]=a[0]*b[0];d[1]=a[1]*b[1];d[2]=a[2]*b[2];
		} else {
			d[0]=a[0]*b;d[1]=a[1]*b;d[2]=a[2]*b;
		}
		return d;
	};

	Vec3.divide = function (d, a,b) {
		if (Vec3.isArray(b)) {
			d[0]=a[0]/b[0];d[1]=a[1]/b[1];d[2]=a[2]/b[2];
		} else {
			d[0]=a[0]/b;d[1]=a[1]/b;d[2]=a[2]/b;
		}
		return d;
	};

	Vec3.dot = function (a,b) {
		return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
	};

	Vec3.cross = function (d, a,b) {
		if ((d !== a) && (d !== b)) {
			d[0]=a[1]*b[2]-a[2]*b[1];
			d[1]=a[2]*b[0]-a[0]*b[2];
			d[2]=a[0]*b[1]-a[1]*b[0];
		} else {
			var a0 = a[0], a1 = a[1], b0 = b[0], b1 = b[1];
			d[0]=a[1]*b[2]-a[2]*b[1];
			d[1]=a[2]*b0-a0*b[2];
			d[2]=a0*b1-a1*b0;
		}
		return d;
	};

	Vec3.size = function (v) {
		return Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	};
	
	Vec3.normalize = function (d, v) {
		var l=Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
		d[0]=v[0]/l;d[1]=v[1]/l;d[2]=v[2]/l;
		return d;
	};

	Vec3.min = function (v) {
		return Math.min(Math.min(v[0], v[1]), v[2]);
	};

	Vec3.max = function (v) {
		return Math.max(Math.max(v[0], v[1]), v[2]);
	};

	Vec3.angles = function (v) {
		return { "theta": Math.atan2(v[2], v[0]), "phi": Math.asin(v[1]/Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2])) };
	};

	Vec3.angle = function (a,b) {
		return Math.acos((a[0]*b[0]+a[1]*b[1]+a[2]*b[2])/(Math.sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2])*Math.sqrt(b[0]*b[0]+b[1]*b[1]+b[2]*b[2])));
	};

	Vec3.transform = function (d, p, m) {
		var s = (m[3]*p[0]+m[7]*p[1]+m[11]*p[2]+m[15]);
		if (d !== p) {
			d[0]=(m[0]*p[0]+m[4]*p[1]+m[8]*p[2]+m[12])/s;
			d[1]=(m[1]*p[0]+m[5]*p[1]+m[9]*p[2]+m[13])/s;
			d[2]=(m[2]*p[0]+m[6]*p[1]+m[10]*p[2]+m[14])/s;
		} else {
			var p0 = p[0], p1 = p[1], p2 = p[2];
			d[0]=(m[0]*p0+m[4]*p1+m[8]*p2+m[12])/s;
			d[1]=(m[1]*p0+m[5]*p1+m[9]*p2+m[13])/s;
			d[2]=(m[2]*p0+m[6]*p1+m[10]*p2+m[14])/s;
		}
		return d;
	};

	Vec3.transformNormal = function (d, v, m) {
		if (d !== v) {
			d[0]=m[0]*v[0]+m[4]*v[1]+m[8]*v[2];
			d[1]=m[1]*v[0]+m[5]*v[1]+m[9]*v[2];
			d[2]=m[2]*v[0]+m[6]*v[1]+m[10]*v[2];
		} else {
			var v0 = v[0], v1 = v[1], v2 = v[2];
			d[0]=m[0]*v0+m[4]*v1+m[8]*v2;
			d[1]=m[1]*v0+m[5]*v1+m[9]*v2;
			d[2]=m[2]*v0+m[6]*v1+m[10]*v2;
		}
		return d;
	};
	
	Vec3.random = function (d, x,y,z) {
		var l;
		do {
			d[0] = Math.random()-x;
			d[1] = Math.random()-y;
			d[2] = Math.random()-z;
			l = Math.sqrt(d[0]*d[0]+d[1]*d[1]+d[2]*d[2]);
		} while (l < 0.01);
		d[0]/=l;d[1]/=l;d[2]/=l;
		return d;
	};

	Vec3.prototype = {
		dup: function() {
			return new Vec3(this);
		},

		// Returns Float
		dot:			function(v) { return Vec3.dot(this, v); },
		size:			function() { return Vec3.size(this); },
		min:			function() { return Vec3.min(this); },
		max:			function() { return Vec3.max(this); },
		angle:			function(v) { return Vec3.angle(this, v); },
		angles:			function() { return Vec3.angles(this); },

		// Returns Vector
		set:			function(v) { return Vec3.set(this, v); },
		negative:		function(v) { if (v !== undefined) return Vec3.negative(this, v); return Vec3.negative(this, this); },
		add:			function(a,b) { if (b !== undefined) return Vec3.add(this, a,b); return Vec3.add(this, this,a); },
		subtract:		function(a,b) { if (b !== undefined) return Vec3.subtract(this, a,b); return Vec3.subtract(this, this,a); },
		multiply:		function(a,b) { if (b !== undefined) return Vec3.multiply(this, a,b); return Vec3.multiply(this, this,a); },
		divide:			function(a,b) { if (b !== undefined) return Vec3.divide(this, a,b); return Vec3.divide(this, this,a); },
		cross:			function(a,b) { if (b !== undefined) return Vec3.cross(this, a,b); return Vec3.cross(this, this,a); },
		normalize:		function(v) { if (v !== undefined) return Vec3.normalize(this, v); return Vec3.normalize(this, this); },
		transform:		function(v,m) { if (m !== undefined) return Vec3.transform(this, v,m); return Vec3.transform(this, this,v); },
		transformNormal:function(v,m) { if (m !== undefined) return Vec3.transformNormal(this, v,m); return Vec3.transform(this, this,v); },
		
		random:			function(v) { return Vec3.random(this, v[0],v[1],v[2]); },
	};
	
	return Vec3;
}).call({});

Vec4 = (function(){
	function Vec4(){
		var obj = Object.create(Float32Array.prototype);
		if (arguments.length == 0)
			obj = Float32Array.apply(obj, [4]);
		else {
			var args = Array.prototype.slice.call(arguments);
			var v = [];
			args.forEach(function(arg){
				if ((arg instanceof Float32Array) || (Array.isArray(arg))) {
					Array.prototype.push.apply(v, arg);
				} else {
					v.push(arg);
				}
			});
			while (v.length < 4) v.push(0);
			if (v.length > 4)
				obj = (Float32Array.apply(obj, [v.slice(0, 4)]) || Float32Array.apply(obj, [4]));
			else
				obj = (Float32Array.apply(obj, [v]) || Float32Array.apply(obj, [4]));
		}
		Vec4.injectClassMethods(obj);
		return obj;
	}
	
	Vec4.injectClassMethods = function(obj){
		for (var method in Vec3.prototype)
			if (Vec3.prototype.hasOwnProperty(method))
				obj[method] = Vec3.prototype[method];
		for (var method in Vec4.prototype)
			if (Vec4.prototype.hasOwnProperty(method))
				obj[method] = Vec4.prototype[method];
		return obj;
	};
	
	Vec4.isArray = function(val) {
		return (val instanceof Float32Array) || (Array.isArray(val));
	};
	
	Vec4.prototype = {
	};
	
	return Vec4;
}).call({});
