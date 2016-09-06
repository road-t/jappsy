precision highp float;

uniform sampler2D uTexture0;
uniform sampler2D uTexture1;

varying vec2 vTextureCoord;
varying vec3 vTime;

varying vec2 vConf[2];

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
// Torsion

void main() {
	vec4 v = texture2D(uTexture0, vConf[0]);
	float c = min(v.x + v.y, 1.);
	
	float s = 0.015;
	float r3 = 1.-abs(smoothstep(vConf[1].x-s, vConf[1].x+s, v.a)-0.5)*2.;
	float r4 = 1.-abs(smoothstep(vConf[1].y-s, vConf[1].y+s, v.a)-0.5)*2.;
	//float m = abs(abs(fract(v.a - vTime[1]/4.) - 0.5)-0.25)*4.;
	
	c = (c + c*min(r3+r4+v.z, 1.))*0.5;
	
	//float n = 1.-c;
	//vec4 c1 = vec4( clamp(vec3(0.25-n, 0.5-n, 1.0-n), 0., 1.), c*0.5 );

	gl_FragColor = vec4(c);
}
