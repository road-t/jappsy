precision mediump float;

const float PI = 3.1415926535897932384626433832795;
const float PI2 = 6.283185307179586476925286766559;
const float PI05 = 1.5707963267948966192313216916398;

uniform sampler2D uTexture;

varying vec2 vTextureCoord;
varying vec3 vTime;

////////////////////////////////////////

float genSubStar(float r, float a, float s, float count, float radius, float smooth, float power) {
	float a1 = abs(mod(a*count,1.)-0.5)*2.;
	float a2 = 1.-pow(1.-a1, power);
	float r1 = r*(a2*(1.-radius)+radius)/radius;
	float r2 = 1.-smoothstep(s-smooth,s,r1);
	float r3 = r2*(1.-pow(r,2.));
	return r3;
}

float random(float f) {
	return fract(sin(dot(vec2(f,f),vec2(12.9898,78.233))) * 43758.5453);
}

float genStar2(float r, float a, float s, float time) {
	float t = floor((time+0.5)/2.);
	float rnd1 = floor(random(t)+0.5)*2.-1.;
	float rnd2 = floor(random(t+0.127)*2.+0.5);
	float rnd3 = floor(random(t+1.397)*4.+0.5);
	
	float r1 = genSubStar(r,a+time*rnd1/50.,s,4.,0.25,1.,6.);
	float r2 = genSubStar(r*3./1.5,a+0.05+time/20.,s,3.+rnd2,0.3,1.,6.)*(1.-r);
	float r3 = genSubStar(r*2./1.5,a+0.5+time/25.,s,3.+rnd2,0.2,1.,6.)*(1.-r)*s;
	float r4 = genSubStar(r*4./1.5,a-time/5.,1.,6.+rnd3,0.4,1.,4.)*(1.-r)*s;
	return clamp(r1+r2+r3+r4,0.,1.);
}

vec4 genStar(vec2 coord, float time, float worldTime, float day ) {
	vec4 color = vec4(0.);

	vec2 pCenter = (coord-0.5)*2.;
	float r = length(pCenter);
	float a = atan(pCenter.x,pCenter.y)/PI2+0.5;
	float tOpen = smoothstep(0.,1.,time*2.);

	float s = abs(mod(worldTime*2., 2.)-1.);
	s = 1.-pow(1.-s,2.);
	float r0 = genStar2(r,a,s,worldTime);
	color = vec4(r0*tOpen);

	return color;
}

void main() {
	gl_FragColor = genStar(vTextureCoord, vTime[0], vTime[1], vTime[2]);
}
