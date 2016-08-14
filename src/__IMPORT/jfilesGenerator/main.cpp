#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>

#include "gui\cImage.h"
#include "core\cString.h"
#include "core\uMemory.h"
#include "cipher\uCipher.h"
#include "core\uRandom.h"

#include "vector.h"

int initPNG();
void quitPNG();

int initGIF();
void quitGIF();

using namespace std;

const double PI = 3.141592653589793238462643383;
const double PI2 = 6.283185307179586476925286766559;
const double PI05 = 1.5707963267948966192313216916398;

vec4 texture2D(const cImage& texture, const vec2& uv) {
	int w = texture.bmInfo->bmiHeader.biWidth;
	int h = abs(int(texture.bmInfo->bmiHeader.biHeight));
	int x = (int)floor(fract(uv.x) * double(w));
	int y = (int)floor(fract(uv.y) * double(h));
	//if (texture.bmInfo->bmiHeader.biHeight < 0)
	//	y = h - y - 1;
	uint32_t* data = (uint32_t*)(texture.bmBits);
	uint32_t color = *(data + x + y * w);
	double r = double((color >> 16) & 0xFF) / 255.0;
	double g = double((color >> 8) & 0xFF) / 255.0;
	double b = double((color >> 0) & 0xFF) / 255.0;
	double a = double((color >> 24) & 0xFF) / 255.0;
	return vec4(r,g,b,a);
}

vec2 textureSize(const cImage& texture) {
	int w = texture.bmInfo->bmiHeader.biWidth;
	int h = abs(int(texture.bmInfo->bmiHeader.biHeight));
	return vec2(w,h);
}

typedef vec4 tTextureFunc(const vec2& uv);

void textureFunc(const cImage& texture, tTextureFunc* func) {
	int w = texture.bmInfo->bmiHeader.biWidth;
	int h = abs(int(texture.bmInfo->bmiHeader.biHeight));
	uint32_t* data = (uint32_t*)(texture.bmBits);
	vec2 coord;
	for (int iy = 0; iy < h; iy++) {
		uint32_t* ptry = data + iy*w;
		coord.y = double(iy)/double(h);
		for (int ix = 0; ix < w; ix++) {
			uint32_t* ptr = ptry + ix;
			coord.x = double(ix)/double(w);
			*ptr = func(coord);
		}
	}
}

double random() {
    return double(rndRand()) / double(0xFFFFFFFF);
}

double texturize(double v) {
	return floor(v*255.)/255.;
}

vec4 randomColor(const vec2& uv) {
	return vec4(
		random(),
		random(),
		random(),
		random()
	);
}

cImage imgRandom;

double triangularFunc(double f) {
	f = f / 2.0;
	if (f < 0.0)
		return f+1.0;
	else
		return 1.0-f;
	return 0.;
}

double bsplineFunc(double x) {
	double f = fabs(x);
	if (f >= 0.0 && f <= 1.0)
		return 2.0/3.0+0.5*(f*f*f)-(f*f);
	else if (f > 1.0 && f <= 2.0)
		return 1.0/6.0*pow(2.0-f,3.0);
	return 0.0;
}

vec4 smoothRandomFunc(const vec2& uv) {
	vec2 vSize = textureSize(imgRandom);
	vec4 vScale = vec4(4.0, 4.0, 4.0, 4.0);
	vec2 vRange[4];
	vec2 vUVPos[4];
	vec2 vUVOfs[4];
	for (int i = 0; i < 4; i++) {
		vRange[i] = vSize / vScale[i];
		vec2 p = uv * vRange[i];
		vUVPos[i] = floor(p);
		vUVOfs[i] = p - vUVPos[i];
	}
	vec4 sum;
	vec4 denom;
	for (int x = -2; x <= 2; x++) {
		for (int y = -2; y <= 2; y++) {
			vec4 data;
			vec2 ofs = vec2(x,y);
			for (int i = 0; i < 4; i++) {
				vec2 p = mod(vUVPos[i] + ofs, vRange[i]);
				vec4 c = texture2D(imgRandom, p / vSize);
				double coef1 = bsplineFunc(double(x) - vUVOfs[i].x);
				double coef2 = bsplineFunc(vUVOfs[i].y - double(y));
				sum[i] = sum[i] + (c[i] * coef1 * coef2);
				denom[i] = denom[i] + coef1 * coef2;
			}
		}
	}
	return sum / denom;
}

// Эффекты для луны
vec4 effectFunc01(const vec2& uv) {
	vec4 c = smoothRandomFunc(uv);
	{
		double rFlame = 0.8;
		double rMoon = 0.5;
		double r = length(uv - 0.5) * 2.0;
		double r1 = smoothstep(0.0,rFlame,r);
		double r5 = smoothstep(rMoon, rMoon+0.05, r1);
		double a = (1.0 - min(pow(r1,3.0),1.0))*r5;
		c.a = a;
	}
	return c;
}

vec4 effectFunc02(const vec2& uv) {
	vec4 c;
	{
		double rMoon = 0.5;
		double r = 0.0;
		double r1 = length(uv - 0.5) * 2.0;
		double r2 = smoothstep(0.0,rMoon,r1);
		double r3 = 1.0 - smoothstep(rMoon, rMoon+0.02, r1);
		c.x = r3;

		double r5 = smoothstep(rMoon, rMoon+0.05, r1);
		c.y = r5;

		double r6 = 1.0-smoothstep(rMoon + 0.02, rMoon + 0.02+0.05, r1);
		c.z = r6;

		double rShadow = length(uv - 0.5 - 0.15) *2.0;
		c.a = smoothstep(0.0,0.8,rShadow);
	}
	return c;
}

// Эффекты для солнца
vec4 effectFunc03(const vec2& uv) {
	vec4 c = smoothRandomFunc(uv);
	{
		double rSun = 0.35;
		double rFlame = 0.7;
		double r = length(uv - 0.5) * 2.0;
		double mFlames = 1.0-smoothstep(rSun,rFlame,r);
		c.a = mFlames;
	}

	return c;
}

cImage imgSunStar;

vec4 effectFunc04(const vec2& uv) {
	vec4 c = vec4(1.0);
	double r = length(uv - 0.5) * 2.0;
	{
		double rSun = 0.35;
		double mSun = smoothstep(0.0, rSun, r);
		double mBody = 1.0 - min(pow(mSun, 5.0), 1.0);
		c.a = mBody;
	}
	{
		double rSun = 0.35;
		double rFlame = 1.0;
		double mFlame = smoothstep(0.95,1.0, smoothstep(0.0, rSun, r));
		c.z = mFlame;

		double r = 0.0;
		double r2 = 0.0;
		vec2 pos = uv - 0.5;
		double m = 1.0;
		for (int j = 0; j < 3; j++) {
			float n = 0.5;//abs(mod((m+float(j)*0.3) / 5., 1.)-0.5)*0.4+0.3;
			for (int i = 0; i < 3; i++) {
				float a = PI2*float(i+j*3)/9.;
				mat2 rot = mat2(cos(a), -sin(a), sin(a), cos(a));
				vec2 p = pos*rot/rFlame;

				if (p.y < 0.) r += smoothstep(0.0, 0.05, -abs(p.y)-abs(p.x+cos(p.y * PI * 8.)*m*0.02)*6.+n);
				if (p.y < 0.) r2 += smoothstep(0.0, 0.05, -abs(p.y)-abs(p.x)*6.+n);
			}
		}
		c.x = r2;

		r = length(pos);
		double a = atan(pos.y, pos.x);
		double b = atan(cos(r * PI * 8.)*0.02, r) / PI2 * 32. + 0.5;
		c.y = b;
		//c.x = r * cos(a) + 0.5 - pos.x;
		//c.y = r * sin(a) + 0.5 - pos.y;

		//c.z = mod(a + b, 1.);
	}

	return c;
}

// Эффекты для луны в бонусе
vec4 effectFunc05(const vec2& uv) {
	return smoothRandomFunc(uv);
}

cImage imgMoon;

vec4 effectFunc06(const vec2& uv) {
	vec4 color = vec4(0.);

	vec2 pCenter = (uv-0.5)*2.0;
	double r = length(pCenter);
	color.x = r;

	// Генерация луны
	double sMoon = 0.5;
	double day = 0.0;
	double aMoonX = (abs(mod(day, 1.0)-0.5)*0.1)*PI2;
	vec4 cMoon = vec4(0.0);
	mat3 rotMoonX = mat3(
		1.0, 0.0, 0.0,
		0.0, cos(aMoonX), sin(aMoonX),
		0.0, -sin(aMoonX), cos(aMoonX)
	);
	double aMoonY = 0.25*PI2;
	mat3 rotMoonY = mat3(
		cos(aMoonY), 0.0, -sin(aMoonY),
		0.0, 1.0, 0.0,
		sin(aMoonY), 0.0, cos(aMoonY)
	);
	double aMoonZ = day*PI2;
	mat3 rotMoonZ = mat3(
		cos(aMoonZ), sin(aMoonZ), 0.0,
		-sin(aMoonZ), cos(aMoonZ), 0.0,
		0.0, 0.0, 1.0
	);
	mat3 rotMoon = rotMoonX * rotMoonZ * rotMoonY;
	vec2 pMoon = pCenter / sMoon;
	double rMoon = length(pMoon);
	if (r <= sMoon) {
		double mMoon = 1.0-smoothstep(sMoon-0.05,sMoon,r);
		vec3 pSphere = vec3(pMoon, -sqrt(1.0-rMoon*rMoon))*rotMoon;
		double a = mod(atan(pSphere.z,pSphere.x) / PI2 + 0.5, 1.0);
		double b = mod(atan(pSphere.y,length(vec2(pSphere.x, pSphere.z))) / PI + 0.5, 1.0);
		vec2 uv = vec2(a, b);
		vec4 c = texture2D(imgMoon, uv);
		color.z = (c.r*0.25+0.75)*mMoon;
		color.a = (c.a*mMoon);
	}

	// Генерация затмения
	double mShadow = 1.0-smoothstep(0.43,0.5,r);
	color.y = mShadow;

	return color;
}

vec4 effectFunc07(const vec2& uv) {
	vec4 color = vec4(0.);

	vec2 pCenter = (uv-0.5)*2.0;
	double r = length(pCenter);
	color.x = r;

	double mShadow = 1.0-smoothstep(0.43,0.5,r);
	color.y = mShadow;

	double sSun = 0.5;
	double rSun = 1.-smoothstep(sSun-0.05,sSun,r);
	color.z = rSun;

	color.a = r*0.2*PI2;

	return color;
}

vec4 effectFunc08(const vec2& uv) {
	vec4 color = vec4(0.);

	vec2 pCenter = (uv-0.5)*2.;
	double r = length(pCenter);

	// Генерация солнца
	double sSun = 0.5;
	double rSun = 1.-smoothstep(sSun-0.05,sSun,r);

	// Генерация затмения
	double rShadow = smoothstep(0.44,0.49,r);
	double mShadow = 1.-smoothstep(0.43,0.5,r);
	double rDark = (1.-rShadow)*mShadow;
	color.y = rDark;
	double rDarkLight = length(pCenter-0.2)*2.*rDark*0.1;
	color.x = rDarkLight;

	// Генерация турбулентностей
	double rLight = (1.-smoothstep(0.47,0.625,r))*rShadow;
	color.z = rLight;

	// Добавляем солнце
	color.x += rSun*rShadow;
	color.y += rSun*rShadow + rLight;
	color.a = 1.;

	return color;
}

// Star
double genSubStar(double r, double a, double s, double count, double radius, double smooth, double power) {
	double a1 = abs(mod(a*count,1.)-0.5)*2.;
	double a2 = 1.-pow(1.-a1, power);
	double r1 = r*(a2*(1.-radius)+radius)/radius;
	double r2 = 1.-smoothstep(s-smooth,s,r1);
	double r3 = r2*(1.-pow(r,2.));
	return r3;
}

vec4 effectFunc09(const vec2& uv) {
	vec4 color = vec4(0.);

	vec2 pCenter = (uv-0.5)*2.;
	double r = length(pCenter);
	double a = atan(pCenter.x,pCenter.y)/PI2+0.5;
	double s = 1.0;

	color.x = genSubStar(r,a,s,6.,0.25,1.,6.);
	color.y = genSubStar(r*2./1.5,a,s,3.,0.2,1.,6.)*(1.-r);
	color.z = genSubStar(r*2./1.5,a,s,4.,0.2,1.,6.)*(1.-r);
	color.a = genSubStar(r*1.5,a,s,0.,1.,1.,1.)*0.5;

	return color;
}

vec4 effectFunc10(const vec2& uv) {
	vec4 color = vec4(0.);

	vec2 pCenter = (uv-0.5)*2.;
	double r = length(pCenter);
	double a = atan(pCenter.x,pCenter.y)/PI2+0.5;
	double s = 1.0;

	color.x = genSubStar(r*4./1.5,a,1.,6.,0.4,1.,4.)*(1.-r)*s;
	color.y = genSubStar(r*4./1.5,a,1.,7.,0.4,1.,4.)*(1.-r)*s;
	color.z = genSubStar(r*4./1.5,a,1.,8.,0.4,1.,4.)*(1.-r)*s;
	color.a = pow(max(1.-r, 0.),2.);

	return color;
}

vec4 effectFunc11(const vec2& uv) {
	vec4 color = vec4(0.);

	vec2 pCenter = (uv-0.5)*2.;
	double r = length(pCenter);
	double a = atan(pCenter.x,pCenter.y)/PI2+0.5;
	double s = 1.0;

	color.x = genSubStar(r*2.,a,s,3.,0.2,1.,6.)*(1.-r)*s;
	color.y = genSubStar(r*2.,a,s,4.,0.2,1.,6.)*(1.-r)*s;
	color.z = genSubStar(r*2.,a,s,5.,0.2,1.,6.)*(1.-r)*s;
	color.a = genSubStar(r,a,s,2.,0.05,0.5,8.);

	return color;
}

vec4 effectFunc12(const vec2& uv) {
	vec4 color = vec4(0.);

	vec2 pCenter = (uv-0.5)*2.;
	double r = length(pCenter);
	double a = atan(pCenter.x,pCenter.y)/PI2+0.5;
	double s = 1.0;

	color.x = genSubStar(r*4.,a,1.,6.,0.4,1.,4.)*(1.-r)*s;
	color.y = genSubStar(r*4.,a,1.,7.,0.4,1.,4.)*(1.-r)*s;
	color.z = genSubStar(r*4.,a,1.,8.,0.4,1.,4.)*(1.-r)*s;
	color.a = genSubStar(r,a,s,4.,0.25,1.,6.);

	return color;
}

#define PI60	0.1047197551196597746154214461

vec4 effectFunc13(const vec2& uv) {
	vec4 color = vec4(0.);

	vec2 pos = uv - 0.5;
	float len = length(pos) * 2.;
	float A = atan(pos.y, pos.x)/PI2 + 0.5;

	float r = 1.-smoothstep(0.95, 1., len);
	float rr = smoothstep(0.2, 0.25, len);
	float c1 = 0.;
	float c2 = 0.;
	float s = 0.005;
	mat2 scale = mat2(1./0.75, 0., 0., 1./1.);
	for (int i = 0; i < 60; i++) {
		float a = PI60*float(i);
		mat2 rot1 = mat2(cos(a), -sin(a), sin(a), cos(a));
		mat2 rot2 = mat2(cos(-a), -sin(-a), sin(-a), cos(-a));
		vec2 p1 = pos+vec2(0.,0.33)*rot1;
		vec2 p2 = pos*rot2;
		float r2 = 1.-abs(smoothstep(0.75-s,0.75+s, length(p1) * 2.)-0.5)*2.;
		if (p2.x < 0.) r2 = 0.;
		c1 += r2*r*rr;

		vec2 p = pos*rot1*scale;
		p1 = (p+vec2(0.,0.35))*rot2;
		p2 = p1*rot1;
		r2 = 1.-abs(smoothstep(0.65-s,0.65+s, length(p1) * 2.)-0.5)*2.;
		if (p2.x < 0.) r2 = 0.;
		c2 += r2*r*rr;
	}
	c1 = min(c1, 1.);
	c2 = min(c2, 1.);

	s = 0.015;
	float r5 = 1.-abs(smoothstep(0.4-s,0.4+s,len)-0.5)*2.;
	float r6 = 1.-abs(smoothstep(0.75-s,0.75+s,len)-0.5)*2.;

	color = vec4(c1, c2, r5+r6, len);
	return color;
}

/*
float randomTextureBSpline(vec2 uv, float repeat, float time) {
	vec2 uvPos = floor(uv);
	vec2 uvOfs = uv-uvPos;
	float sum = 0.;
	float denom = 0.;
	for (int x = -2; x <= 2; x++) {
		for (int y = -2; y <= 2; y++) {
			float data = random(uvPos+vec2(float(x), float(y)), repeat, time);
			float coef1 = bsplineFunc(float(x)-uvOfs.x);
			float coef2 = bsplineFunc(uvOfs.y-float(y));
			sum += data*coef1*coef2;
			denom += coef1*coef2;
		}
	}
	return sum/denom;
}
*/

int main(int argc, char *argv[]) {
    init_cString();
    initPNG();
    initGIF();

	printf("Jappsy Texture Generator v1.0\n\n");

	unsigned long rndLong = (unsigned long)time(0) ^ (unsigned int)clock();
	unsigned short int rndInit = ((rndLong>>16) ^ (rndLong & 0xFFFF)) % RAND_MAX;
//	srand(rndInit);
    rndSeed(rndInit);

	int s = 256;
	imgRandom.createImage(s, s, true);
	textureFunc(imgRandom, randomColor);
	imgRandom.savePNG(L"random_r.png", true, NULL);

	cImage img;
	img.createImage(s, s, false);
	//textureFunc(img, smoothRandomFunc);
	//img.savePNG(L"random_32.png", true, NULL);

	/*
	textureFunc(img, effectFunc01);
	img.savePNG(L"mobile_tex_effect_moon1_01_rs.png", true, NULL);

	textureFunc(img, effectFunc02);
	img.savePNG(L"mobile_tex_effect_moon1_02_s.png", true, NULL);

	textureFunc(img, effectFunc03);
	img.savePNG(L"mobile_tex_effect_sun1_01_rs.png", true, NULL);

	imgSunStar.createImage(s, s, true);
	textureFunc(imgSunStar, effectFunc04);
	imgSunStar.savePNG(L"mobile_tex_effect_sun1_02_s.png", true, NULL);

	textureFunc(img, effectFunc05);
	img.savePNG(L"mobile_tex_random_rs.png", true, NULL);

	imgMoon.createImage(L"moon_s.png", true);
	textureFunc(img, effectFunc06);
	img.savePNG(L"mobile_tex_effect_moon2_s.png", true, NULL);

	textureFunc(img, effectFunc07);
	img.savePNG(L"mobile_tex_effect_sun2_s.png", true, NULL);

	textureFunc(img, effectFunc08);
	img.savePNG(L"mobile_tex_effect_unknown_s.png", true, NULL);

	textureFunc(img, effectFunc09);
	img.savePNG(L"mobile_tex_effect_unknown_star_01_s.png", true, NULL);

	textureFunc(img, effectFunc10);
	img.savePNG(L"mobile_tex_effect_unknown_star_02_s.png", true, NULL);

	textureFunc(img, effectFunc11);
	img.savePNG(L"mobile_tex_effect_star_01_s.png", true, NULL);

	textureFunc(img, effectFunc12);
	img.savePNG(L"mobile_tex_effect_star_02_s.png", true, NULL);
	*/

	img.createImage(1024, 1024, false);
	textureFunc(img, effectFunc13);
	img.savePNG(L"mobile_tex_effect_torsion_s.png", true, NULL);

	printf("\n\nPress Any Key...\n");
	getch();

	quitGIF();
    quitPNG();
    uninit_cString();

    return 0;
}
