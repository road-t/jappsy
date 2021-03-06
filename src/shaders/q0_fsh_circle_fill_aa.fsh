precision mediump float;

varying vec4 v_pos;

uniform float u_radius;
uniform vec2 u_center;
uniform vec4 u_color;
uniform int u_antialias;
uniform int u_negative;

void main() {
    float dist = length(v_pos.xy - u_center);
    if (dist > u_radius) {
        float antialiasing = 0.;
        if (u_antialias != 0) {
            if (dist < (u_radius + 1.)) {
                antialiasing = (u_radius + 1.) - dist;
            }
        }
        if (u_negative != 0) {
            gl_FragColor = vec4(1. - u_color.x, 1. - u_color.y, 1. - u_color.z, u_color.w * antialiasing);
        } else {
            gl_FragColor = vec4(u_color.xyz, u_color.w * antialiasing);
        }
    } else {
        if (u_negative != 0) {
            gl_FragColor = vec4(1. - u_color.x, 1. - u_color.y, 1. - u_color.z, u_color.w);
        } else {
            gl_FragColor = u_color;
        }
    }
}