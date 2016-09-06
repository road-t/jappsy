precision mediump float;

varying vec2 vPosition;

uniform vec2 uCorners[2];
uniform float uBorder;
uniform vec4 uColor;

void main() {
	if (
		(vPosition.x < uCorners[0].x || vPosition.x >= uCorners[1].x) ||
		(vPosition.y < uCorners[0].y || vPosition.y >= uCorners[1].y)
	) {
		gl_FragColor = vec4(0.);
	} else {
		if (
			(vPosition.x < (uCorners[0].x + uBorder) || vPosition.x >= (uCorners[1].x - uBorder)) ||
			(vPosition.y < (uCorners[0].y + uBorder) || vPosition.y >= (uCorners[1].y - uBorder))
		) {
			gl_FragColor = vec4(vec3(uColor)*uColor.a, uColor.a);
		} else {
			gl_FragColor = vec4(0.);
		}
	}
}
