precision mediump float;

attribute vec4 a_position;

uniform mat4 u_projection;

varying vec4 v_pos;

void main() {
    gl_Position = a_position * u_projection;
    v_pos = a_position;
}