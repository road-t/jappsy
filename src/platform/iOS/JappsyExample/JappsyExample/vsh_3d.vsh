precision mediump float;

const float PI2 = 1.570796326794896619231321692;

attribute vec3 aVertexPosition;
attribute vec3 aVertexNormal;
attribute vec2 aTextureCoord;

uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uNormalMatrix;

uniform vec4 uColors[5];

uniform vec3 uAmbientLightColor;

uniform mat4 uLights[6];
uniform int uLightsCount;

varying vec3 vPosition;
varying vec3 vNormal;
varying vec2 vTextureCoord;

varying vec4 vColors[3];
varying vec3 vDiffuseLight;
varying vec3 vSpecularLight;
varying vec2 vAlpha;

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
	gl_Position = uModelViewProjectionMatrix * vec4(aVertexPosition, 1.);

	vPosition = vec3(uModelViewMatrix * vec4(aVertexPosition, 1.));
	vNormal = normalize(mat3(uNormalMatrix) * aVertexNormal);
	vTextureCoord = aTextureCoord;
	
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

	vColors[0] = uColors[1];
	vColors[1] = uColors[2];
	vColors[2] = uColors[3];
	vDiffuseLight = uAmbientLightColor * uColors[0].rgb * (1. - diffuse) + diffuseLight;
	vSpecularLight = specularLight;
	vAlpha = vec2( uColors[0].a, uColors[4].a );
}
