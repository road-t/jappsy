precision mediump float;

const float PI = 3.141592653589793238462643383;
const float PI2 = 1.570796326794896619231321692;
const float PI60 = 0.1047197551196597746154214461;

uniform sampler2D uTexture;

varying vec2 vTextureCoord;
varying vec3 vTime;

void main() {
	vec2 pos = vTextureCoord - 0.5;
	float len = length(pos) * 2.;
	float A = atan(pos.y, pos.x)/PI2 + 0.5;

	float r = 1.-smoothstep(0.95, 1., len);
	float rr = smoothstep(0.2, 0.25, len);
	float c = 0.;
	float s = 0.005;
	mat2 scale = mat2(1./0.75, 0., 0., 1./1.);
	float a = PI60*vTime[1]*4.;
	mat2 rot1 = mat2(cos(a), -sin(a), sin(a), cos(a));
	mat2 rot2 = mat2(cos(-a), -sin(-a), sin(-a), cos(-a));
	vec2 pos1 = pos*rot2;
	vec2 pos2 = pos*rot2;
	for (int i = 0; i < 60; i++) {
		float a = PI60*float(i);
		rot1 = mat2(cos(a), -sin(a), sin(a), cos(a));
		rot2 = mat2(cos(-a), -sin(-a), sin(-a), cos(-a));
		vec2 p1 = pos1+vec2(0.,0.33)*rot1;
		vec2 p2 = pos1*rot2;
		float r2 = 1.-abs(smoothstep(0.75-s,0.75+s, length(p1) * 2.)-0.5)*2.;
		if (p2.x < 0.) r2 = 0.;
		c += r2*r*rr;

		vec2 p = pos2*rot1*scale;
		p1 = (p+vec2(0.,0.35))*rot2;
		p2 = p1*rot1;
		r2 = 1.-abs(smoothstep(0.65-s,0.65+s, length(p1) * 2.)-0.5)*2.;
		if (p2.x < 0.) r2 = 0.;
		c += r2*r*rr;
	}
	c = min(c, 1.);
	
	s = 0.015;
	float t1 = mod(vTime[1]*0.5, 1.);
	float t2 = mod(1.-t1+0.7, 1.);
	float r3 = 1.-abs(smoothstep(t1-s,t1+s,len)-0.5)*2.;
	float r4 = 1.-abs(smoothstep(t2-s,t2+s,len)-0.5)*2.;
	float r5 = 1.-abs(smoothstep(0.4-s,0.4+s,len)-0.5)*2.;
	float r6 = 1.-abs(smoothstep(0.75-s,0.75+s,len)-0.5)*2.;
	c = (c + c*min(r3+r4+r5+r6, 1.))*0.5;
	
	//pow(c/2.,2.);
	//gl_FragColor = vec4(vec3(c), 1.);

	gl_FragColor = vec4(vec3(c), c);
}
