precision mediump float;

attribute vec2 aVertexPosition;
attribute vec2 aTextureCoord;

uniform mat4 uLayerProjectionMatrix;
uniform vec2 uPosition;

varying vec2 vTextureCoord;
varying vec2 vPosition;

void main() {
    gl_Position = uLayerProjectionMatrix * vec4(aVertexPosition+uPosition, 0., 1.);
    vPosition = aVertexPosition+uPosition;

    vTextureCoord = aTextureCoord;
}