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

vec4 genMoon1() {
	float a = texture2D(uTexture0, vTextureCoord).a;
	float m = 1.-pow3(1.-noiseMap());
	vec3 c = vec3(0.5-m, 1.0-m, 1.5-m);
	return vec4(c*a, (1.-m)*a);
}

vec4 genMoon2() {
	vec4 tex = texture2D(uTexture1, vTextureCoord);

	// Черное пятно
	float mBlack = tex.x;
	// Маска смещенной окружности
	float mClip = texture2D(uTexture1, vMoonConf.xy).y;
	// Обратная маска центрального круга
	float rCenter = tex.z;
	// Обратная маска тени в центре
	float rShadow = tex.a;

	// Маска видимого полумесяца
	float mMoon = mClip*rCenter;
	// Маска контура вокруг полумесяца
	float mBorder = 1.-abs(mMoon-0.5)*2.;
	// Маска контура вокруг полумесяца с уточнением границ
	float mEdge = 1.-pow2(1.-mBorder);

	float n = 1.-pow3(1.-abs( (noise1()-0.5)*2. ));
	vec3 c =
		// Свечение края месяца
		vec3(0.5-n, 1.0-n, 1.5-n)*mEdge
		// Месяц
		+mMoon*0.95
		// Тень на месяце
		*(rShadow*0.25+0.75);

	n = noise1();
	c +=
		// Туман на темном круге
		(vec3(max(0.8-n,0.))*0.1+0.05)*(1.-mClip)*rCenter*vMoonConf.z
		// Тень на темном круге
		+(0.1-rShadow*0.1);

	return vec4(c, min(mMoon+mBlack, 1.));
}

vec4 genMoon() {
	vec4 color1 = genMoon1();
	vec4 color2 = genMoon2();
	return (vec4(vec3(color1)*(1.-color2.a), color1.a)+color2)*vMoonConf.w;
}

void main() {
	gl_FragColor = genMoon();
}
