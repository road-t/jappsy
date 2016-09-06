precision mediump float;

const float PI = 3.1415926535897932384626433832795;
const float PI2 = 6.283185307179586476925286766559;
const float PI05 = 1.5707963267948966192313216916398;

uniform sampler2D uTexture;

varying vec2 vTextureCoord;
varying vec3 vTime;

////////////////////////////////////////

float random(vec2 pos, float time) {
	return fract(sin(dot(pos.xy+time,vec2(12.9898,78.233))) * 43758.5453 + time*10.);
}

vec4 genStars(vec2 coord, float time, float worldTime, float day ) {
	vec4 color = vec4(0.);


	return color;
}

void main() {
	//gl_FragColor = genSun(vTextureCoord, vTime[0], vTime[1], vTime[2]);
	
	// Example Moon
	//gl_FragColor = genMoon(vTextureCoord, vTime[0], vTime[1], vTime[2]);
	
	gl_FragColor = vec4(vec3(random(vTextureCoord, 0.)), 1.);
}
