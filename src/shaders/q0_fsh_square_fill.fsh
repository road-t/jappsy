precision mediump float;

uniform vec4 uColor;

void main() {
	gl_FragColor = vec4(vec3(uColor)*uColor.a, uColor.a);
}