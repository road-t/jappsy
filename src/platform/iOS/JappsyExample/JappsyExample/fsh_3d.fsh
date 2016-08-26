precision mediump float;

uniform sampler2D uTexture[3];

varying vec3 vPosition;
varying vec3 vNormal;
varying vec2 vTextureCoord;

varying vec4 vColors[3];
varying vec3 vDiffuseLight;
varying vec3 vSpecularLight;
varying vec2 vAlpha;

void main(void) {
	vec3 cDiffuse = mix(vColors[0].rgb, texture2D(uTexture[0], vTextureCoord).rgb, vColors[0].a);
	vec3 cSpecular = mix(vColors[1].rgb, texture2D(uTexture[1], vTextureCoord).rgb, vColors[1].a);
	vec3 cEmissive = mix(vColors[2].rgb, texture2D(uTexture[2], vTextureCoord).rgb, vColors[2].a);

	// Ambient
	vec3 color = cDiffuse * vDiffuseLight
		+ cSpecular * vSpecularLight
		+ cEmissive
	;
	// Opacity
	float alpha = mix(vAlpha[0], texture2D(uTexture[0], vTextureCoord).a, vAlpha[1]);
	
	gl_FragColor = vec4(color*alpha, alpha);
}
