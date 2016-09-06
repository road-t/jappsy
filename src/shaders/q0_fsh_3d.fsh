precision mediump float;

//const float PI = 3.141592653589793238462643383;
const float PI2 = 1.570796326794896619231321692;

uniform sampler2D uTexture[3];

// 0.rgb - Ambient
// 0.a - Opacity
// 1.rgb - Diffuse
// 1.a - Diffuse Blend
// 2.rgb - Specular
// 2.a - Specular Blend
// 3.rgb - Emissive
// 3.a - Emissive Blend
// 4.r - Shininess (Glossiness)
// 4.g - Shininess Percent
// 4.a - Opacity Blend
uniform vec4 uColors[5];

uniform vec3 uAmbientLightColor;

// 0.xyz - Position
// 0.w - Hotspot Distance / Angle ( 0 - removes hotspot )
// 1.xyz - Target (Omni Light = 0)
// 1.w - Hotspot Falloff Distance / Angle
// 2.rgb - Color
// 2.w - Type (0 - Omni Light, 1 - Spot Light, 2 - Direct Light)
uniform mat4 uLights[6];
uniform int uLightsCount;

varying vec3 vPosition;
varying vec3 vNormal;
varying vec2 vTextureCoord;

mat4 getLight(int index) {
	if (index == 0)
		return uLights[0];
	else if (index == 1)
		return uLights[1];
	else if (index == 2)
		return uLights[2];
	else if (index == 3)
		return uLights[3];
	else if (index == 4)
		return uLights[4];
	else if (index == 5)
		return uLights[5];
		
	return uLights[0];
}

mat3 calculateLight(mat4 mLight) {
	mat3 result = mat3(0.);

	vec3 normal = vNormal;

	vec3 vLight = vPosition - mLight[0].xyz;
	if (mLight[2].w == 0.) {
		vec3 nLight = normalize(vLight);
		
		float angle = dot(normal, -nLight);
		// Double-Side ???
		//if (angle < 0.) {
		//	angle = -angle;
		//	normal = -normal;
		//}
		
		if (angle > 0.) {
			float falloff = 1.;
			if (mLight[0].w > 0.) {
				falloff = clamp(1. - (length(vLight) - mLight[0].w) / mLight[1].w, 0., 1.);
			}
			if (falloff > 0.) {
				result[0][0] = angle * falloff;
				result[1] = result[0][0] * mLight[2].rgb;
				result[2] = pow(max(dot(reflect(nLight, normal), vec3(0.,0.,1.)), 0.) * falloff, uColors[4].r) * uColors[4].g * mLight[2].rgb;
			}
		}
	} else if (mLight[2].w == 1.) {
		vec3 nLight = normalize(vLight);

		float angle = dot(normal, -nLight);
		
		if (angle > 0.) {
			float falloff = 1.;
			if (mLight[0].w > 0.) {
				float angleFalloff = acos(dot(nLight, normalize(mLight[1].xyz - mLight[0].xyz)));
				falloff = 1. - clamp((angleFalloff - mLight[0].w) / mLight[1].w / PI2, 0., 1.);
			}
			if (falloff > 0.) {
				result[0][0] = angle * falloff;
				result[1] = result[0][0] * mLight[2].rgb;
				result[2] = pow(max(dot(reflect(nLight, normal), vec3(0.,0.,1.)), 0.) * falloff, uColors[4].r) * uColors[4].g * mLight[2].rgb;
			}
		}
	} else if (mLight[2].w == 2.) {
		vec3 nLight = normalize(mLight[1].xyz - mLight[0].xyz);

		float angle = dot(normal, -nLight);
		if (angle > 0.) {
			float falloff = 1.;
			if (mLight[0].w > 0.) {
				float cosDist = dot(normalize(vLight), nLight);
				float sinDist = sqrt(1. - cosDist * cosDist);
				float dist = length(vLight) * sinDist;
				falloff = clamp(1. - (dist - mLight[0].w) / mLight[1].w, 0., 1.);
			}
			if (falloff > 0.) {
				result[0][0] = angle * falloff;
				result[1] = result[0][0] * mLight[2].rgb;
				result[2] = pow(max(dot(reflect(nLight, normal), vec3(0.,0.,1.)), 0.) * falloff, uColors[4].r) * uColors[4].g * mLight[2].rgb;
			}
		}
	}
	
	return result;
}

void main(void) {
	vec3 cDiffuse = mix(uColors[1].rgb, texture2D(uTexture[0], vTextureCoord).rgb, uColors[1].a);
	vec3 cSpecular = mix(uColors[2].rgb, texture2D(uTexture[1], vTextureCoord).rgb, uColors[2].a);
	vec3 cEmissive = mix(uColors[3].rgb, texture2D(uTexture[2], vTextureCoord).rgb, uColors[3].a);

	float diffuse = 0.;
	vec3 diffuseLight = vec3(0.);
	vec3 specularLight = vec3(0.);
	
	// Calculate Lights
	for (int i = 0; i < 6; i++) {
		if (i < uLightsCount) {
			mat3 rLight = calculateLight(getLight(i));
			diffuse = diffuse + rLight[0][0];
			diffuseLight = diffuseLight + rLight[1];
			specularLight = specularLight + rLight[2];
		}
	}

	// Ambient
	vec3 color = cDiffuse * (uAmbientLightColor * uColors[0].rgb * (1. - diffuse) + diffuseLight)
		+ cSpecular * specularLight
		+ cEmissive;
	// Opacity
	float alpha = mix(uColors[0].a, texture2D(uTexture[0], vTextureCoord).a, uColors[4].a);
	
	gl_FragColor = vec4(color*alpha, alpha);//vec4(normal, 1.);//vec4(diffuseLight, 1.);//
}
