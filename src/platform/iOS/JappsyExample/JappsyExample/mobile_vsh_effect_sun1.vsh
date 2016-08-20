precision highp float;

attribute vec4 aVertexPosition;
attribute vec2 aTextureCoord;

uniform mat4 uLayerProjectionMatrix;
uniform vec2 uTime; // 0. local day, 1. world day

varying vec2 vTextureCoord;
varying vec3 vTime; // 0. local seconds, 1. world seconds, 2. world day

varying vec4 vNoiseConf;
varying vec2 vNoiseCoord[4];

varying vec4 vSunConf;

// BUGFIX: China's Android Bug (mod functions replace)
float module(float value, float divider) {
    float v = value / divider;
    return (v - floor(v)) * divider;
}
float pow3(float value) {
    return value * value * value;
}

const float PI2 = 6.283185307179586476925286766559;

void main() {
    gl_Position = uLayerProjectionMatrix * aVertexPosition;

    float localTime = uTime.x*86400.;
    float delayTime = max(localTime-2.,0.);
    float secTime = uTime.y*86400.;

    vTextureCoord = aTextureCoord;
    vTime = vec3(localTime, secTime, uTime.y);

    // Noise Static Data
    {
        float modTime = floor(module(secTime, 3.));
        vec3 vModTime = vec3(0.);
        if (modTime < 1.) vModTime[0] = 1.;
        else if (modTime < 2.) vModTime[1] = 1.;
        else vModTime[2] = 1.;
        vNoiseConf = vec4(vModTime, fract(secTime));

        float coordTime = module(secTime, 80.);
        vNoiseCoord[0] = aTextureCoord/8. + coordTime*0.0125;
        vNoiseCoord[1] = aTextureCoord/4. - coordTime*0.025;
        vNoiseCoord[2] = aTextureCoord/2. + coordTime*0.05;
        vNoiseCoord[3] = aTextureCoord - coordTime*0.1;
    }

    // genSun2 Static Data
    {
       	float m = abs(module(secTime * 2., 2.)-1.)-0.5;
       	float n = abs(module(secTime / 5., 1.)-0.5)*2.;
        vSunConf = vec4(m * PI2 / 32., n, smoothstep(0.5, 1., n), smoothstep(0.,1.,localTime));
    }
}