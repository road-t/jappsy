precision highp float;

attribute vec4 aVertexPosition;
attribute vec2 aTextureCoord;

uniform mat4 uLayerProjectionMatrix;
uniform vec2 uTime; // 0. local day, 1. world day

varying vec2 vTextureCoord;
varying vec3 vTime; // 0. local seconds, 1. world seconds, 2. world day

varying vec2 vStarConf[5];
varying vec3 vStarMix[2];

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

const float PI2 = 6.283185307179586476925286766559;

void main() {
    gl_Position = uLayerProjectionMatrix * aVertexPosition;

    float localTime = uTime.x*86400.;
    float delayTime = max(localTime-2.,0.);
    float secTime = uTime.y*86400.;

    vTextureCoord = aTextureCoord;
    vTime = vec3(localTime, secTime, uTime.y);

    // genStar Static Data
    {
		float t = floor((secTime+0.5)/2.);
		float rnd1 = floor(random(t+1.297)*2.+0.5);
		float rnd2 = floor(random(t+2.378)*2.+0.5);

    	vec2 p = (vTextureCoord-0.5);
    	float tOpen = min(1., localTime*2.);
		float s = abs(module(secTime*2., 2.)-1.);
		s = 1.-pow(1.-s,2.);
		
		float a2 = (0.05+secTime/20.)*PI2;
		float a3 = (0.5+secTime/25.)*PI2;
		float a4 = (-secTime/5.)*PI2;
		vStarConf[0] = vec2(p)/s+0.5;
		vStarConf[1] = vec2(p.x*cos(a2)-p.y*sin(a2), p.x*sin(a2)+p.y*cos(a2))*1.5/s+0.5;
		vStarConf[2] = vec2(p.x*cos(a3)-p.y*sin(a3), p.x*sin(a3)+p.y*cos(a3))/s+0.5;
		vStarConf[3] = vec2(p.x*cos(a4)-p.y*sin(a4), p.x*sin(a4)+p.y*cos(a4))/s+0.5;
		
		vStarConf[4].x = s * tOpen;
		
		if (rnd1 < 1.) {
			vStarMix[0] = vec3(1., 0., 0.);
		} else if (rnd1 < 2.) {
			vStarMix[0] = vec3(0., 1., 0.);
		} else {
			vStarMix[0] = vec3(0., 0., 1.);
		}
		
		if (rnd2 < 1.) {
			vStarMix[1] = vec3(1., 0., 0.);
		} else if (rnd2 < 2.) {
			vStarMix[1] = vec3(0., 1., 0.);
		} else {
			vStarMix[1] = vec3(0., 0., 1.);
		}
    }
}