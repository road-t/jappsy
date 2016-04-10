precision mediump float;

uniform sampler2D u_sampler;
uniform int u_index;
uniform vec4 u_colors[2];

// 0 - edgeDistance
// 1 - edgeCenter
// 2 - outerOffset
// 3 - innerOffset
uniform vec4 u_params;

varying vec2 v_texture;

vec4 srcOver(vec4 color_back, vec4 color_front) {
    if (color_back[3] > 0.) {
		if (color_front[3] > 0.) {
			float alpha_back = color_back[3] * (1. - color_front[3]);
			vec4 color;
			color[3] = color_front[3] + alpha_back;
			color[0] = (color_front[0] * color_front[3] + color_back[0] * alpha_back) / color[3];
			color[1] = (color_front[1] * color_front[3] + color_back[1] * alpha_back) / color[3];
			color[2] = (color_front[2] * color_front[3] + color_back[2] * alpha_back) / color[3];
			
			return color;
		} else {
			return color_back;
		}
    }
    
    return color_front;
}

void main() {
    vec4 dfc = texture2D(u_sampler, v_texture);
    float alpha0 = smoothstep(u_params[3], u_params[3] + u_params[0], dfc[u_index]);
    float alpha1 = alpha0 * u_colors[0].a;
    if (alpha1 < 1.) {
		float alpha2 = smoothstep(u_params[2], u_params[2] + u_params[0], dfc[u_index]) * (1. - alpha0) * u_colors[1].a;
		if (alpha1 > 0.) {
			gl_FragColor = srcOver(vec4(u_colors[1].rgb, alpha2), vec4(u_colors[0].rgb, alpha1));
		} else {
			gl_FragColor = vec4(u_colors[1].rgb, alpha2);
		}
    } else {
		gl_FragColor = vec4(u_colors[0].rgb, 1.);
	}
}
