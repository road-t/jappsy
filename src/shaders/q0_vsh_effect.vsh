precision mediump float;

attribute vec4 aVertexPosition;
attribute vec2 aTextureCoord;

uniform mat4 uLayerProjectionMatrix;
uniform float uTime;
uniform float uWorldTime;

varying vec2 vTextureCoord;
varying vec3 vTime; // 0. local seconds, 1. world seconds, 2. world day

void main() {
    gl_Position = uLayerProjectionMatrix * aVertexPosition;

    vTextureCoord = aTextureCoord;
    vTime = vec3(uTime*86400., uWorldTime*86400., uWorldTime);
}