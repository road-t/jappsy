precision mediump float;

uniform vec4 u_color;
uniform int u_negative;

void main() {
    if (u_negative != 0) {
        gl_FragColor = vec4(1. - u_color.x, 1. - u_color.y, 1. - u_color.z, u_color.w);
    } else {
        gl_FragColor = u_color;
    }
}