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

float noise(vec2 coord) {
	return texture2D(uTexture0, coord).r;
}

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

vec4 genSun() {
	vec4 color;

	vec4 tex = texture2D(uTexture1, vTextureCoord);
	vec2 pCenter = (vTextureCoord-0.5)*2.;

	// Генерация затмения
	float rShadow = smoothstep(0.43,0.5,length(vSunConf.xy));
	float mShadow = tex.y;
	//float mShadow = 1.-smoothstep(0.43,0.5,r);
	float rDark = (1.-rShadow)*mShadow;
	float rDarkLight = length(vSunConf.xy-0.2)*2.*rDark*0.1;

	// Генерация вихрей на солнце
	float rFlames = 1.-smoothstep(0.3,0.3+0.5*vSunConf.z,tex.x);
	{
		float b = tex.a*vSunConf.z - vSunConf.a;
		float cb = cos(b) / 8.;
		float sb = sin(b) / 8.;
		float xcb = pCenter.x * cb;
		float ycb = pCenter.y * cb;
		float xsb = pCenter.x * sb;
		float ysb = pCenter.y * sb;
		vec2 coord = vec2(xcb - ysb, xsb + ycb);

		float n = noise(coord + vTime[1]*0.0125)*1.7;
		float m = 1.-pow3(n);

		color = vec4(vec3(1.5-m, 1.0-m, 0.5-m)*rFlames*(1.-rDark), rFlames*0.25)*(1.-tex.z);
	}

	// Генерация турбулентностей
	{
		float rLight = (1.-smoothstep(0.45-vSunConf.z*0.05,0.65-vSunConf.z*0.05,tex.x))*rShadow;
		float m = 1.-pow3(1.-noiseMap());
		color += vec4(1.5-m, 1.0-m, 0.5-m, 1.)*rLight;
	}

	// Добавляем затмение
	color = vec4(color.rgb + rDarkLight, color.a + rDark);

	// Добавляем солнце
	color += tex.z*rShadow;

	return color;
}

void main() {
	gl_FragColor = genSun();
}
