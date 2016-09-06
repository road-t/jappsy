precision mediump float;

const float PI = 3.141592653589793238462643383;
const float PI2 = 1.570796326794896619231321692;

uniform sampler2D uTexture;

varying vec2 vTextureCoord;
varying vec3 vTime;

////////////////////////////////////////
// Simplex Noise for Sun Flame Generation

vec4 permute( vec4 x ) {
	return mod( ( ( x * 34.0 ) + 1.0 ) * x, 289.0 );
}

vec4 taylorInvSqrt( vec4 r ) {
	return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise( vec3 v ) {
	const vec2 C = vec2( 1.0 / 6.0, 1.0 / 3.0 );
	const vec4 D = vec4( 0.0, 0.5, 1.0, 2.0 );

	// First corner

	vec3 i  = floor( v + dot( v, C.yyy ) );
	vec3 x0 = v - i + dot( i, C.xxx );

	// Other corners

	vec3 g = step( x0.yzx, x0.xyz );
	vec3 l = 1.0 - g;
	vec3 i1 = min( g.xyz, l.zxy );
	vec3 i2 = max( g.xyz, l.zxy );

	//  x0 = x0 - 0. + 0.0 * C
	vec3 x1 = x0 - i1 + 1.0 * C.xxx;
	vec3 x2 = x0 - i2 + 2.0 * C.xxx;
	vec3 x3 = x0 - 1. + 3.0 * C.xxx;

	// Permutations

	i = mod( i, 289.0 );
	vec4 p = permute( permute( permute(
			 i.z + vec4( 0.0, i1.z, i2.z, 1.0 ) )
		   + i.y + vec4( 0.0, i1.y, i2.y, 1.0 ) )
		   + i.x + vec4( 0.0, i1.x, i2.x, 1.0 ) );

	// Gradients
	// ( N*N points uniformly over a square, mapped onto an octahedron.)

	float n_ = 1.0 / 7.0; // N=7

	vec3 ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor( p * ns.z *ns.z );  //  mod(p,N*N)

	vec4 x_ = floor( j * ns.z );
	vec4 y_ = floor( j - 7.0 * x_ );    // mod(j,N)

	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs( x ) - abs( y );

	vec4 b0 = vec4( x.xy, y.xy );
	vec4 b1 = vec4( x.zw, y.zw );

	vec4 s0 = floor( b0 ) * 2.0 + 1.0;
	vec4 s1 = floor( b1 ) * 2.0 + 1.0;
	vec4 sh = -step( h, vec4( 0.0 ) );

	vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
	vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

	vec3 p0 = vec3( a0.xy, h.x );
	vec3 p1 = vec3( a0.zw, h.y );
	vec3 p2 = vec3( a1.xy, h.z );
	vec3 p3 = vec3( a1.zw, h.w );

	// Normalise gradients

	vec4 norm = taylorInvSqrt( vec4( dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3) ) );
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

	// Mix final noise value

	vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3) ), 0.0 );
	m = m * m;
	return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3) ) );
}

float heightMap( vec3 coord ) {
	float n = abs( snoise( coord ) );

	n += 0.25   * abs( snoise( coord * 2.0 ) );
	n += 0.25   * abs( snoise( coord * 4.0 ) );
	n += 0.125  * abs( snoise( coord * 8.0 ) );
	n += 0.0625 * abs( snoise( coord * 16.0 ) );

	return n;
}

vec4 genSun1( vec2 coord, float scale, float time, float rSun, float rFlame ) {
	vec4 color = vec4(0.);

	float r = length(coord.xy - 0.5) * 2.;
	float mFlames = 1.-smoothstep(rSun,rFlame,r);
	if (mFlames > 0.) {
		vec3 p = vec3(coord.xy * scale, time);
		float n = heightMap(p);
		float m = 1.-pow(1.-n,2.);
		color += vec4(vec3(1.5-m, 1.0-m, 0.5-m)*mFlames, (1.-m)*mFlames);

		float mSun = smoothstep(0.,rSun,r);
		float mBody = 1.-min(pow(mSun,5.),1.);
		if (mBody > 0.) {
			n = snoise(p * 0.5);
			m = 1.-pow(1.-n,2.);
			color += vec4(clamp(vec3(1.0-m, 0.5-m, 0.25-m), 0., 1.)*mBody, mBody);
		}
	}

	return color;
}

////////////////////////////////////////
// Sun Flames

vec4 genSun2( vec2 coord, float time, float rSun, float rFlame ) {
	float r = 0.;
	vec2 pos = coord.xy - vec2(0.5);
	float r1 = length(pos) * 2.;
	float r2 = 1.-clamp(((1.-r1)-(1.-rSun))/rSun,0.,1.);
	float m = abs(mod(time * 2., 2.)-1.)-0.5;
	for (int j = 0; j < 3; j++) {
		float n = abs(mod((time+float(j)*0.3) / 5., 1.)-0.5)*0.4+0.3;
		for (int i = 0; i < 3; i++) {
			float a = PI*2.*float(i+j*3)/9.;
			mat2 rot = mat2(cos(a), -sin(a), sin(a), cos(a));
			vec2 p = pos*rot/rFlame;
			
			if (p.y < 0.) r += smoothstep(0.0, 0.05, -abs(p.y)-abs(p.x+cos(p.y * PI * 8.)*m*0.02)*6.+n);
		}
	}
	for (int j = 0; j < 3; j++) {
		float n = abs(mod((time+float(j)*0.7) / 5., 1.)-0.5)*0.4+0.3;
		for (int i = 0; i < 3; i++) {
			float a = PI*2.*float(i+j*3)/9.+PI/9.;
			mat2 rot = mat2(cos(a), -sin(a), sin(a), cos(a));
			vec2 p = pos*rot/rFlame;
			
			float c = 0.;
			if (p.y < 0.) r += smoothstep(0.0, 0.05, -abs(p.y)-abs(p.x+cos(p.y * PI * 8.)*m*0.02)*6.+(0.5-n+0.3));
		}
	}
	r = clamp(r, 0., 1.)*smoothstep(0.95,1.,r2);
	
	float s1 = abs(mod(time / 10., 1.)-0.5)*2.;
	float s2 = smoothstep(0.5, 1., s1);

	return vec4(mix(vec3(1.,1.-s2*0.5,0.5-s2*0.25),vec3(1.-s2*0.3,0.,0.),smoothstep(rSun,rFlame*(1.-s2*0.25),r1))*r, r);
}

////////////////////////////////////////
// Sun

vec4 genSun( vec2 coord, float time, float worldTime ) {
	vec4 color1 = genSun1(coord, 3., worldTime / 2., 0.35, 0.7);
	vec4 color2 = genSun2(coord, worldTime * 1.5, 0.35, 1.0);
	
	float tOpen = smoothstep(0.,1.,time);
	return (vec4(vec3(color1)*(1.-color2.a), color1.a)+color2)*tOpen;
}

void main() {
	gl_FragColor = genSun(vTextureCoord, vTime[0], vTime[1]);
}
