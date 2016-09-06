precision highp float;

uniform sampler2D uTexture0;
uniform sampler2D uTexture1;

varying vec2 vTextureCoord;
varying vec3 vTime;

varying vec4 vNoiseConf;
varying vec2 vNoiseCoord[4];

varying vec4 vSunConf;

// BUGFIX: China's Android Bug (mod function replace)
float module(float value, float divider) {
    float v = value / divider;
    return (v - floor(v)) * divider;
}
float pow2(float value) {
	return value*value;
}
float pow3(float value) {
	return value*value*value;
}

////////////////////////////////////////
// Noise

float noise0() {
	vec4 color = texture2D(uTexture0, vNoiseCoord[0]);
	return mix(
		color.r*vNoiseConf.r+
		color.g*vNoiseConf.g+
		color.b*vNoiseConf.b,
		color.r*vNoiseConf.b+
		color.g*vNoiseConf.r+
		color.b*vNoiseConf.g,
		vNoiseConf.a);
}

float noise1() {
	return texture2D(uTexture0, vNoiseCoord[1]).r;
}

float noise2() {
	return texture2D(uTexture0, vNoiseCoord[2]).r;
}

float noise3() {
	return texture2D(uTexture0, vNoiseCoord[3]).r;
}

float noiseMap() {
	float n = abs( (noise0()-0.5)*2. );

	n += 0.5   * abs( (noise1()-0.5)*2. );
	n += 0.25   * abs( (noise2()-0.5)*2. );
	n += 0.125  * abs( (noise3()-0.5)*2. );

	return min(n, 1.0);
}

////////////////////////////////////////
// Sun

vec4 genSun1() {
	float mFlames = texture2D(uTexture0, vTextureCoord).a;
	float mBody = texture2D(uTexture1, vTextureCoord).a;

	float m1 = 1.-pow3(1.-noiseMap());
	float m2 = 1.-pow2(1.-noise1());

	return vec4(
		vec3(1.5-m1, 1.0-m1, 0.5-m1)*mFlames + vec3(1.5-m2, 1.0-m2, 0.5-m2)*mBody,
		(1.-m1)*mFlames + mBody
	);
}

vec4 genSun2() {
	float n1 = vSunConf.y*0.4+0.6;
	float n2 = (1.-vSunConf.y)*0.3+0.6;
	vec2 pos = vTextureCoord - 0.5;

	vec4 c = texture2D(uTexture1, vTextureCoord);
	float mFlame = c.z;
	float b = (c.y - 0.5) * vSunConf.x;
	float cb = cos(b);
	float sb = sin(b);
	float xcb = pos.x * cb;
	float ycb = pos.y * cb;
	float xsb = pos.x * sb;
	float ysb = pos.y * sb;

	vec2 coord1 = vec2(xcb - ysb, xsb + ycb)/n1 + 0.5;
	vec2 coord2 = vec2(ysb - xcb, -xsb - ycb)/n2 + 0.5;
	float rStar1 = texture2D(uTexture1, coord1).x;
	float rStar2 = texture2D(uTexture1, coord2).x;

	float r = min(rStar1 + rStar2, 1.) * mFlame;

	float r1 = length(pos) * 2.;
	float r2 = smoothstep(0.5, (1.-vSunConf.z*0.25), r1);
    return vec4(mix(vec3(1.,1.-vSunConf.z*0.5,0.5-vSunConf.z*0.25),vec3(1.-vSunConf.z*0.3,0.,0.),r2)*r, r);
}

vec4 genSun() {
	vec4 color1 = genSun1();
	vec4 color2 = genSun2();
	return (vec4(vec3(color1)*(1.-color2.a), color1.a)+color2)*vSunConf.w;
}

void main() {
	gl_FragColor = genSun();
}
