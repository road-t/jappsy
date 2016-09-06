precision highp float;

uniform sampler2D uTexture0;
uniform sampler2D uTexture1;

varying vec2 vTextureCoord;
varying vec3 vTime;

varying vec2 vStarConf[5];
varying vec3 vStarMix[2];

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
// Star

float genSubStar0() {
	return texture2D(uTexture1, vStarConf[0]).a;
}

float genSubStar1() {
	vec4 tex = texture2D(uTexture0, vStarConf[1]);
	return tex.x * vStarMix[0].x + tex.y * vStarMix[0].y + tex.z * vStarMix[0].z;
}

float genSubStar2() {
	vec4 tex = texture2D(uTexture0, vStarConf[2]);
	return tex.x * vStarMix[0].x + tex.y * vStarMix[0].y + tex.z * vStarMix[0].z;
}

float genSubStar3() {
	vec4 tex = texture2D(uTexture1, vStarConf[3]);
	return tex.x* vStarMix[1].x + tex.y * vStarMix[1].y + tex.z * vStarMix[1].z;
}

vec4 genStar() {
	float r1 = genSubStar0();
	float r2 = genSubStar1();
	float r3 = genSubStar2();
	float r4 = genSubStar3();
	float r0 = clamp(r1+r2+r3+r4,0.,1.);
	
	float s = vStarConf[4].x;
	return vec4(r0*s);
}

void main() {
	gl_FragColor = genStar();
}
