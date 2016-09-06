precision highp float;

uniform sampler2D uTexture0;
uniform sampler2D uTexture1;

varying vec2 vTextureCoord;
varying vec3 vTime;

varying vec4 vNoiseConf;
varying vec2 vNoiseCoord[4];

varying vec4 vMoonConf;

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
// Moon

vec4 genMoon() {
	// Генерация луны
	vec4 tex = texture2D(uTexture1, vTextureCoord);
	float r = tex.x;
	vec4 cMoon = vec4(vec3(tex.z), tex.a);

	// Генерация затмения
	float rShadow = smoothstep(0.43,0.5,length(vMoonConf.xy));
	float mShadow = tex.y;
	//float mShadow = 1.-smoothstep(0.43,0.5,r);
	float rDark = (1.-rShadow)*mShadow;
	float rDarkLight = length(vMoonConf.xy-0.2)*2.*rDark*0.1;

	// Генерация турбулентностей
	float rLight = (1.-smoothstep(0.45-vMoonConf.a,0.65-vMoonConf.a,r))*rShadow*(1.-vMoonConf.z*mShadow);
    float m = 1.-pow3(1.-noiseMap());
    vec4 cLight = vec4(1.5-m, 1.0-m+vMoonConf.z*0.5, 0.5-m+vMoonConf.z, 1.)*rLight;

	// Добавляем затмение
	vec4 color = vec4(cLight.rgb + rDarkLight + ((cMoon.rgb-0.9)*(1.-rShadow)*vMoonConf.z), cLight.a + rDark);

	// Добавляем луну
	color += cMoon*rShadow;

	return color;
}

void main() {
	gl_FragColor = genMoon();
}
