precision highp float;

uniform sampler2D uTexture0;
uniform sampler2D uTexture1;

varying vec2 vTextureCoord;
varying vec3 vTime;

varying vec2 vStarConf[4];
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
	return texture2D(uTexture0, vStarConf[0]).x;
}

float genSubStar1() {
	vec4 tex = texture2D(uTexture0, vStarConf[1]);
	return tex.y * vStarMix[0].x + tex.z * vStarMix[0].y;
}

float genSubStar2() {
	vec4 tex = texture2D(uTexture0, vStarConf[2]);
	return tex.y * vStarMix[0].x + tex.z * vStarMix[0].y;
}

float genSubStar3() {
	vec4 tex = texture2D(uTexture1, vStarConf[3]);
	return tex.x* vStarMix[1].x + tex.y * vStarMix[1].y + tex.z * vStarMix[1].z;
}

float genSubStar4() {
	return texture2D(uTexture0, vTextureCoord).a;
}

vec4 genStar() {
	float r1 = genSubStar0();
	float r2 = genSubStar1();
	float r3 = genSubStar2();
	float r4 = genSubStar3();
	float r5 = genSubStar4();
	float r0 = clamp(r1+r2+r3+r4+r5,0.,1.);
	
	float s = vStarMix[0].z;
	float p = texture2D(uTexture1, vTextureCoord).a;
	return vec4(mix(vec3(1.5,1.,0.5),vec3(1.),r0*p)*r0*s,r0*0.75*s);
}

void main() {
	gl_FragColor = genStar();
}
