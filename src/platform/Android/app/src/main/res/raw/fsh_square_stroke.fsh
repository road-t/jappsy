precision mediump float;

varying vec4 v_pos;

uniform vec2 u_point1;
uniform vec2 u_point2;
uniform float u_border;
uniform vec4 u_color;
uniform int u_negative;

void main() {
    if ((v_pos.x < u_point1.x || v_pos.x >= u_point2.x) ||
        (v_pos.y < u_point1.y || v_pos.y >= u_point2.y))
    {
        gl_FragColor = vec4(0.);
    } else {
        if ((v_pos.x < (u_point1.x + u_border) || v_pos.x >= (u_point2.x - u_border)) ||
            (v_pos.y < (u_point1.y + u_border) || v_pos.y >= (u_point2.y - u_border)))
        {
            if (u_negative != 0) {
                gl_FragColor = vec4(1. - u_color.x, 1. - u_color.y, 1. - u_color.z, u_color.w);
            } else {
                gl_FragColor = u_color;
            }
        } else {
            gl_FragColor = vec4(0.);
        }
    }
}