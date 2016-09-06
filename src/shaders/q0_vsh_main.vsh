precision mediump float;

attribute vec3 aVertexPosition;

uniform mat4 uLayerProjectionMatrix;

varying vec2 vPosition;

void main() {
    gl_Position = uLayerProjectionMatrix * vec4(aVertexPosition, 1.);
    vPosition = aVertexPosition.xy;
}
