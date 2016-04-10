precision mediump float;

uniform sampler2D u_sampler;
uniform int u_negative;

varying vec2 v_texture;

void main() {
    if (u_negative != 0) {
        vec4 u_color = texture2D(u_sampler, v_texture);
        gl_FragColor = vec4(1. - u_color.r, 1. - u_color.g, 1. - u_color.b, u_color.a);
    } else {
        gl_FragColor = texture2D(u_sampler, v_texture);
    }
}