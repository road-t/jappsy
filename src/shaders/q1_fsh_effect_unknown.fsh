precision highp float;

uniform sampler2D uTexture0;
uniform sampler2D uTexture1;

varying vec2 vTextureCoord;
varying vec3 vTime;

varying vec4 vNoiseConf;
varying vec2 vNoiseCoord[4];

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
// Dark

void main() {
	vec4 tex = texture2D(uTexture1, vTextureCoord);
	float m = 1.-pow3(1.-noiseMap());
	gl_FragColor = vec4(vec3(1.5-m, 1.0-m, 0.5-m)*tex.z + tex.x, tex.y);
}
