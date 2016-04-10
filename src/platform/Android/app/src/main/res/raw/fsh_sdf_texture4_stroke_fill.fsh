precision mediump float;

uniform sampler2D u_sampler;
uniform vec4 u_colors[5];

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
    vec4 alpha0;
    alpha0[3] = smoothstep(u_params[3], u_params[3] + u_params[0], dfc[3]);
    float alpha1 = alpha0[3] * u_colors[3].a;
    if (alpha1 < 1.) {
        vec4 color = vec4(u_colors[3].rgb, alpha1);
        
        alpha0[2] = smoothstep(u_params[3], u_params[3] + u_params[0], dfc[2]);
        alpha1 = alpha0[2] * u_colors[2].a;
        if (alpha1 < 1.) {
            color = srcOver(vec4(u_colors[2].rgb, alpha1), color);
            
            alpha0[1] = smoothstep(u_params[3], u_params[3] + u_params[0], dfc[1]);
            alpha1 = alpha0[1] * u_colors[1].a;
            if (alpha1 < 1.) {
                color = srcOver(vec4(u_colors[1].rgb, alpha1), color);
                
                alpha0[0] = smoothstep(u_params[3], u_params[3] + u_params[0], dfc[0]);
                alpha1 = alpha0[0] * u_colors[0].a;
                if (alpha1 < 1.) {
                    color = srcOver(vec4(u_colors[0].rgb, alpha1), color);
                    
                    float alpha2 = 1. - alpha0[3];
                    if (alpha2 > 0.) {
                        alpha2 = alpha2 * (1. - alpha0[2]);
                        if (alpha2 > 0.) {
                            alpha2 = alpha2 * (1. - alpha0[1]);
                            if (alpha2 > 0.) {
                                alpha2 = alpha2 * (1. - alpha0[0]);
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
                    alpha2 = alpha2 * (1. - alpha1);
                    
                    gl_FragColor = srcOver(vec4(u_colors[4].rgb, alpha2 * u_colors[4].a), color);
                } else {
                    gl_FragColor = srcOver(vec4(u_colors[0].rgb, 1.), color);
                }
            } else {
                gl_FragColor = srcOver(vec4(u_colors[1].rgb, 1.), color);
            }
        } else {
            gl_FragColor = srcOver(vec4(u_colors[2].rgb, 1.), color);
        }
    } else {
        gl_FragColor = vec4(u_colors[3].rgb, 1.);
    }
}
