precision highp float;

attribute vec4 aVertexPosition;
attribute vec2 aTextureCoord;

uniform mat4 uLayerProjectionMatrix;
uniform vec2 uTime; // 0. local day, 1. world day

varying vec2 vTextureCoord;
varying vec3 vTime; // 0. local seconds, 1. world seconds, 2. world day

varying vec2 vConf[2];

// BUGFIX: China's Android Bug (mod functions replace)
float module(float value, float divider) {
    float v = value / divider;
    return (v - floor(v)) * divider;
}
float pow3(float value) {
    return value * value * value;
}

float random(float f) {
	return fract(sin(dot(vec2(f,f),vec2(12.9898,78.233))) * 43758.5453);
}

const float PI60 = 0.1047197551196597746154214461;
const float PI2 = 6.283185307179586476925286766559;

void main() {
    gl_Position = uLayerProjectionMatrix * aVertexPosition;

    float localTime = uTime.x*86400.;
    float delayTime = max(localTime-2.,0.);
    float secTime = uTime.y*86400.;

    vTextureCoord = aTextureCoord;
    vTime = vec3(localTime, secTime, uTime.y);

    // genTorsion Static Data
    {
		vec2 p = vTextureCoord-0.5;
		
		float a = -secTime*PI60*4.;
		vConf[0] = vec2(p.x*cos(a)-p.y*sin(a), p.x*sin(a)+p.y*cos(a))+0.5;
		
		float t1 = fract(secTime*0.5);
		float t2 = fract(1.-t1+0.7);
		
		vConf[1] = vec2(t1, t2);
    }
}