precision mediump float;

const float PI = 3.1415926535897932384626433832795;
//const float PI2 = 6.283185307179586476925286766559;
//const float PI05 = 1.5707963267948966192313216916398;

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

////////////////////////////////////////
// Unknown

vec4 genSun( vec2 coord, float time, float worldTime, float day ) {
	vec4 color = vec4(0.);
	
	vec2 pCenter = (coord-0.5)*2.;
	float r = length(pCenter);
	float tOpen = smoothstep(0.,1.,time/2.);

	// Генерация солнца
	float sSun = 0.5;
	float rSun = 1.-smoothstep(sSun-0.05,sSun,r);

	// Генерация затмения
	vec2 pDark = vec2(pCenter.x+tOpen,pCenter.y);
	float rShadow = smoothstep(0.43,0.5,length(pDark));
	float mShadow = 1.-smoothstep(0.43,0.5,r);
	float rDark = (1.-rShadow)*mShadow;
	float rDarkLight = length(pDark-0.2)*2.*rDark*0.1;

	// Генерация вихрей на солнце
	float rFlames = 1.-smoothstep(0.3,0.3+0.5*tOpen,r);
	if (rFlames > 0.) {
		float a = atan(pCenter.y,pCenter.x) / (2.0*PI);
		float aSpiral = mod(a+r*0.2*tOpen-worldTime/16.,1.)*2.0*PI;
		vec2 pSpiral = vec2(cos(aSpiral)*r, sin(aSpiral)*r)+0.5;
		vec3 p = vec3(pSpiral * (5. - tOpen*2.), worldTime/4.);
		float n = snoise(p);
		float m = 1.-pow(1.-n,2.);
		color += vec4(vec3(1.5-m, 1.0-m, 0.5-m)*rFlames*(1.-rDark), rFlames*0.25)*(1.-rSun);
	}
	
	// Генерация турбулентностей
	float rLight = (1.-smoothstep(0.45-tOpen*0.05,0.65-tOpen*0.05,r))*rShadow;
	if (rLight > 0.) {
		vec3 p = vec3(coord * 3., worldTime/2.);
		float n = heightMap(p);
		float m = 1.-pow(1.-n,3.);
		vec3 c = vec3(1.5-m, 1.0-m, 0.5-m);
		
		color += vec4(c*rLight, rLight);
	}

	// Добавляем затмение
	color = vec4(color.rgb + rDarkLight, color.a + rDark);

	// Добавляем солнце
	color += vec4(vec3(rSun)*rShadow, rSun*rShadow);

	return color;
}

void main() {
	gl_FragColor = genSun(vTextureCoord, vTime[0], vTime[1], vTime[2]);
}
