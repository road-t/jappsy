precision mediump float;

attribute vec3 aVertexPosition;
attribute vec2 aTextureCoord;

uniform mat4 uLayerProjectionMatrix;

varying vec2 vTextureCoord;

void main() {
    gl_Position = uLayerProjectionMatrix * vec4(aVertexPosition, 1.);

    vTextureCoord = aTextureCoord;
}