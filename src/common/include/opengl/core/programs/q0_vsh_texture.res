R"JAPPSYRAWSTRING(
precision mediump float;

attribute vec2 aVertexPosition;
attribute vec2 aTextureCoord;

uniform mat4 uLayerProjectionMatrix;
uniform vec2 uSize;
uniform vec2 uPosition;

varying vec2 vTextureCoord;

void main() {
    gl_Position = uLayerProjectionMatrix * vec4((aVertexPosition * uSize) + uPosition, 0., 1.);
    vTextureCoord = aTextureCoord;
}
)JAPPSYRAWSTRING"
