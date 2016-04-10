precision mediump float;

uniform sampler2D u_sampler;
uniform vec4 u_color;
uniform int u_negative;

// 0 - edgeDistance
// 1 - edgeCenter
// 2 - outerOffset
// 3 - innerOffset
uniform vec4 u_params;

varying vec2 v_texture;

void main() {
    vec4 dfc = texture2D(u_sampler, v_texture);
    float alpha0;
    float alpha1;
    float alpha2;
    
    alpha0 = 1. - smoothstep(u_params[3], u_params[3] + u_params[0], dfc[3]);
    if (alpha0 > 0.) {
		alpha0 = alpha0 * (1. - smoothstep(u_params[3], u_params[3] + u_params[0], dfc[2]));
		if (alpha0 > 0.) {
			alpha0 = alpha0 * (1. - smoothstep(u_params[3], u_params[3] + u_params[0], dfc[1]));
			if (alpha0 > 0.) {
				alpha0 = alpha0 * (1. - smoothstep(u_params[3], u_params[3] + u_params[0], dfc[0]));
			}
		}
	}
    alpha1 = 1. - smoothstep(u_params[2], u_params[2] + u_params[0], dfc[3]);
    if (alpha1 > 0.) {
		alpha1 = alpha1 * (1. - smoothstep(u_params[2], u_params[2] + u_params[0], dfc[2]));
		if (alpha1 > 0.) {
			alpha1 = alpha1 * (1. - smoothstep(u_params[2], u_params[2] + u_params[0], dfc[1]));
			if (alpha1 > 0.) {
				alpha1 = alpha1 * (1. - smoothstep(u_params[2], u_params[2] + u_params[0], dfc[0]));
			}
		}
	}
	alpha2 = alpha0 * (1. - alpha1);

    gl_FragColor = vec4(u_color.rgb, alpha2 * u_color.a);
}
