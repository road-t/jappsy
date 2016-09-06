precision mediump float;

uniform sampler2D uTexture;
uniform vec3 uColor;

varying vec2 vTextureCoord;
varying float vAlpha;
varying float vPulse;

void main(void) {
	float c = clamp(1. - length(vTextureCoord.xy - vec2(0.5)) * 2., 0., 1.);
	if (vAlpha == 0.) {
		gl_FragColor = vec4(0.);
	} else {
		vec4 color1 = texture2D(uTexture, vTextureCoord) * vec4(uColor, 1.) * (vPulse*0.25+0.75);
		vec4 color2 = texture2D(uTexture, vTextureCoord+vec2(0,0.5)) * vec4(uColor, 1.) * vPulse;
		vec4 color = vec4(
			color1.r + color2.r*(1. - color1.a),
			color1.g + color2.g*(1. - color1.a),
			color1.b + color2.b*(1. - color1.a),
			color1.a + color2.a*(1. - color1.a)
		);
		//vec4 color = min(color1+color2*vPulse, 1.);
		gl_FragColor = color * vAlpha;
			//vec4(vec3(uColor)*c, c*vAlpha);
			//vec4(color1.rgb*vec3(uColor), color1.a*vAlpha*vPulse);
	}
}
