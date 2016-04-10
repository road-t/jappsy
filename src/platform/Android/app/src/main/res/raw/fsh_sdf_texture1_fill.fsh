precision lowp float;

uniform sampler2D u_sampler;
uniform int u_index;
uniform vec4 u_color;

// 0 - edgeDistance
// 1 - edgeCenter
// 2 - outerOffset
// 3 - innerOffset
uniform vec4 u_params;

varying vec2 v_texture;

void main() {
    vec4 dfc = texture2D(u_sampler, v_texture);
    float alpha0 = smoothstep(u_params[3], u_params[3] + u_params[0], dfc[u_index]);
    gl_FragColor = vec4(u_color.rgb, alpha0 * u_color.a);
}
