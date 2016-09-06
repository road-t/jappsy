precision mediump float;

attribute vec4 aVertexPosition;
attribute vec2 aTextureCoord;
attribute vec4 aVelocity;
attribute vec4 aAcceleration;
attribute vec4 aTime;

uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uPixelX;
uniform vec3 uPixelY;
uniform vec3 uTime;

varying vec2 vTextureCoord;
varying float vAlpha;
varying float vPulse;

void main(void) {
	float scale = aVelocity.w;

	float localTime = mod(uTime.x, uTime.y) - aTime.x;
	if (localTime < 0.) vAlpha = 0.;
	else if (localTime > aTime.y) vAlpha = 0.;
	else vAlpha = 1. - localTime / aTime.y;
	
	vPulse = 1. - abs((mod(localTime, 20.) - 10.) / 10.);

	vec3 pZ = normalize(cross(uPixelX, uPixelY));
	float c = cos(localTime);
	float s = sin(localTime);
	float t = 1. - c;
	float xt = pZ.x*t;
	float yt = pZ.y*t;
	float zt = pZ.z*t;
	float xs = pZ.x*s;
	float ys = pZ.y*s;
	float zs = pZ.z*s;
	mat3 rotate = mat3(
		pZ.x*xt+c, pZ.y*xt+zs, pZ.z*xt-ys,
		pZ.x*yt-zs, pZ.y*yt+c, pZ.z*yt+xs,
		pZ.x*zt+ys, pZ.y*zt-xs, pZ.z*zt+c
	);
	
	vec3 pX = rotate * uPixelX * scale;
	vec3 pY = rotate * uPixelY * scale;

	vec3 pos = vec3(aVertexPosition);
	if (aVertexPosition.w == 0.) {
		pos = pos - pX - pY;
	} else if (aVertexPosition.w == 1.) {
		pos = pos - pX + pY;
	} else if (aVertexPosition.w == 2.) {
		pos = pos + pX + pY;
	} else {
		pos = pos + pX - pY;
	}
	
	pos = pos + aVelocity.xyz * localTime + aAcceleration.xyz * (localTime * (localTime + 1.) / 2.);
	
	gl_Position = uModelViewProjectionMatrix * vec4(vec3(pos), 1.);

	vTextureCoord = vec2(aTextureCoord.x, aTextureCoord.y / 2.);
}
