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
	vec3 p = vec3(coord.xy * scale, time);
	float n = heightMap(p);
	float m = 1.-pow(1.-n,2.);
	vec3 c = vec3(1.5-m, 1.0-m, 0.5-m);

	float r = length(coord.xy - vec2(0.5)) * 2.;
	float r1 = 1.-clamp(((1.-r)-(1.-rFlame))/rFlame,0.,1.);
	float a = 1.-min(pow(r1,3.),1.);
	vec4 flames = vec4(c*a, (1.-m)*a);

	n = snoise(p * 0.5);
	m = 1.-pow(1.-n,2.);
	c = clamp(vec3(1.0-m, 0.5-m, 0.25-m), 0., 1.);

	float r2 = 1.-clamp(((1.-r)-(1.-rSun))/rSun,0.,1.);
	a = 1.-min(pow(r2,5.),1.);
	vec4 body = vec4(c*a, a);

	return flames + body;
}

////////////////////////////////////////
// Moon

vec4 genMoon1( vec2 coord, float scale, float time, float rFlame) {
	vec3 p = vec3(coord.xy * scale, time);
	float n = heightMap(p);
	float m = 1.-pow(1.-n,2.);
	vec3 c = vec3(0.5-m, 1.0-m, 1.5-m);

	float r = length(coord.xy - vec2(0.5)) * 2.;
	float r1 = 1.-clamp(((1.-r)-(1.-rFlame))/rFlame,0.,1.);
	float a = 1.-min(pow(r1,3.),1.);
	return vec4(c*a, (1.-m)*a);
}

vec4 genMoon2( vec2 coord, float time, float worldTime, float rMoon ) {
	float r = 0.;
	vec2 pos = coord.xy - vec2(0.5);
	float r1 = length(pos) * 2.;
	float r2 = 1.-clamp(((1.-r1)-(1.-rMoon))/rMoon,0.,1.);
	float r3 = 1.-smoothstep(rMoon, rMoon+0.02, r1); // Черное пятно
	
	//Сдвинутый круг
	// 1. Влияние временного смещения
	float m = (pow(mod(time / 2., 2.)-1., 3.)+1.)*0.5;
	// 2. Смещение координаты Х
	float px = mod(pos.x + m - 0.25, 1.) - 0.5;
	// 3. Расстояние от центра смещенного круга
	float r4 = length(vec2(px, pos.y)) * 2.;
	// 4. Маска смещенной окружности
	float r5 = smoothstep(rMoon, rMoon+0.05, r4);
	
	// Обратная маска центрального круга
	float r6 = 1.-smoothstep(0.52, 0.52+0.05, r1);
	// Маска видимого полумесяца
	float r7 = r5*r6;
	// Маска контура вокруг полумесяца
	float r8 = 1.-abs(r7-0.5)*2.;
	// Маска контура вокруг полумесяца с уточнением границ
	float r9 = 1.-pow(1.-r8,2.);
	// Расстояние от смещенного центра тени
	float rShadow = length(pos-0.15) *2.;
	
	float n = heightMap(vec3(coord*4., worldTime / 4.));
	n = 1.-pow(1.-n,2.);
	vec3 c =
		// Свечение края месяца
		vec3(0.5-n, 1.0-n, 1.5-n)*r9
		// Месяц
		+r7*0.95
		// Тень на месяце
		*(smoothstep(0.,0.8,rShadow)*0.25+0.75);
	n = snoise(vec3(coord * 5., worldTime / 2.));
	float r10 = 1.-(abs(mod(m + 0.2, 1.)-0.5))*2.;
	r10 = smoothstep(0.,0.5,r10)*0.5+0.5;
	c +=
		// Туман на темном круге
		(vec3(max(0.5-n,0.))*0.1+0.05)*(1.-r5)*r6*r10
		// Тень на темном круге
		+(0.1-smoothstep(0.,0.8,rShadow)*0.1);

	return vec4(c, min(r5*r6+r3, 1.));
}

vec4 genMoon( vec2 coord, float time, float worldTime ) {
	vec4 color1 = genMoon1(coord, 3., worldTime / 2., 0.8);
	vec4 color2 = genMoon2(coord, max(time-2.,0.), worldTime, 0.5);
	
	float tOpen = smoothstep(0.,1.,time);
	return (vec4(vec3(color1)*(1.-color2.a), color1.a)+color2)*tOpen;
}

void main() {
	gl_FragColor = genMoon(vTextureCoord, vTime[0], vTime[1]);
}
