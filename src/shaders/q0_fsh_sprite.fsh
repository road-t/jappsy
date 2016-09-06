precision mediump float;

uniform sampler2D uTexture;
uniform vec4 uLight;
uniform float uTime;

varying vec2 vTextureCoord;
varying vec2 vPosition;

void main() {
	vec4 color = texture2D(uTexture, vTextureCoord);
	float timeX = (uTime * 1920. * 3.) - 1920.;
	float dist = abs(timeX - vPosition.y * 0.5 - vPosition.x);
	float light = clamp(1. - dist * 20. / 1920., 0., 1.) * 0.25;
	
	gl_FragColor = vec4(color.rgb * uLight.rgb * color.a + light * color.a, color.a * uLight.a);
}
