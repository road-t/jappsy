precision mediump float;

attribute vec3 aVertexPosition;
attribute vec3 aVertexNormal;
attribute vec2 aTextureCoord;

uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uNormalMatrix;

varying vec3 vPosition;
varying vec3 vNormal;
varying vec2 vTextureCoord;

void main(void) {
	gl_Position = uModelViewProjectionMatrix * vec4(aVertexPosition, 1.);

	vPosition = vec3(uModelViewMatrix * vec4(aVertexPosition, 1.));
	vNormal = normalize(mat3(uNormalMatrix) * aVertexNormal);
	vTextureCoord = aTextureCoord;
}
