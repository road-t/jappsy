precision mediump float;

varying vec4 v_pos;

uniform vec2 u_point1;
uniform vec2 u_point2;
uniform vec4 u_color1;
uniform vec4 u_color2;
uniform int u_negative;

void main() {
    if (u_negative != 0) {
        vec4 u_color = mix(u_color1, u_color2, (v_pos.y - u_point1.y) / (u_point2.y - u_point1.y));
        gl_FragColor = vec4(1. - u_color.x, 1. - u_color.y, 1. - u_color.z, u_color.w);
    } else {
        gl_FragColor = mix(u_color1, u_color2, (v_pos.y - u_point1.y) / (u_point2.y - u_point1.y));
    }
}