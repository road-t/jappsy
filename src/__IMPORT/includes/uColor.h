#ifndef uColorH
#define uColorH

#include <windows.h>
//#include <fastmath.h>
#include <math.h>
#include <inttypes.h>

const double eps  = 0.00000001;
const double eps2 = 2*eps;

enum cColorTypes { ctNone, ctRGB, ctCMY, ctCMYK, ctHLS, ctHSI, ctYCbCr, ctXYZ, ctLuv, ctLab};

class cRGB;

class cColor {
public:
	virtual cColorTypes Type();
	virtual double distance();
	virtual double distance(cColor* Color);
	virtual void toRGB(cRGB* destColor);
	virtual void fromRGB(cRGB* srcColor);
};

class cRGB : cColor {
public:
	cRGB();
	cRGB(double nR, double nG, double nB);
public:

	// Whitness
	double getW();
	// Non-Chromatic
	double getK();
	cRGB getWhitness();
	cRGB getChromatic();
	DWORD getColor();
	void setColor(DWORD dwColor);

	cColorTypes Type();

	double R; // Red    0..1
	double G; // Green  0..1
	double B; // Blue   0..1
	void operator =(cRGB* srcColor);
	void operator =(cRGB& srcColor);
	void operator =(cRGB srcColor);
};

class cCMY : cColor  {
public:
	cCMY();
	cCMY(double nC, double nM, double nY);
public:
	double C; // Cyan     0..1
	double M; // Magenta  0..1
	double Y; // Yellow   0..1
	cColorTypes Type();
};

class cCMYK : cColor  {
public:
	cCMYK();
	cCMYK(double nC, double nM, double nY, double nK);
public:
	double C; // Cyan      0..1
	double M; // Magenta   0..1
	double Y; // Yellow    0..1
	double K; // Blackness 0..1
	cColorTypes Type();
};

class cHLS : cColor  {
public:
	cHLS();
	cHLS(double nH, double nL, double nS);
public:
	double H; // Hue          0..360
	double L; // Lightness    0..1
	double S; // Saturation   0..1
	cColorTypes Type();
};

class cHSI : cColor  { // or HSV or HSB (another names)
public:
	cHSI();
	cHSI(double nH, double nS, double nI);
public:
	double H; // Hue          0..360
	double S; // Saturation   0..1
	double I; // Intensity or Value or Brightness 0..2
	cColorTypes Type();
};

class cYCbCr : cColor  { // NTSC Standart
public:
	cYCbCr();
	cYCbCr(double nY, double nCb, double nCr);
public:
	double Y; // Luminance   0..1
	double Cb; //            0..1
	double Cr; //            0..1
	cColorTypes Type();
};

class cXYZ : cColor  { // CIE XYZ
public:
	cXYZ();
	cXYZ(double nX, double nY, double nZ);
public:
	double X;  // 0..1
	double Y;  // 0..1
	double Z;  // 0..1
	cColorTypes Type();
};

class cLab : cColor  { // CIE Lab
public:
	cLab();
	cLab(double nL, double na, double nb);
public:
	double L; // Lightness  0..100
	double a; //            -127..+128
	double b; //            -127..+128
	cColorTypes Type();
};

class cLuv : cColor  { // CIE Luv
public:
	cLuv();
	cLuv(double nL, double nu, double nv);
public:
	double L; // Lightness  0..100
	double u; //            -127..+128
	double v; //            -127..+128
	cColorTypes Type();
};

cRGB  CMY2RGB(cCMY& vCMY);
cCMY  RGB2CMY(cRGB& vRGB);
cCMYK CMY2CMYK(cCMY& vCMY);
cCMY  CMYK2CMY(cCMYK& vCMYK);
cRGB  HLS2RGB(cHLS& HLS); // R || G || B < 0 then out of range color
cHLS  RGB2HLS(cRGB& vRGB); // S < 0 then out of range color
cRGB  HSI2RGB(cHSI& HSI); // R || G || B < 0 then out of range color
cHSI  RGB2HSI(cRGB& vRGB); // S < 0 then out of range color
cYCbCr RGB2YCbCr(cRGB& vRGB);
cRGB  YCbCr2RGB(cYCbCr& YCbCr);
cXYZ  RGB2XYZEBU(cRGB& vRGB);
cRGB  XYZEBU2RGB(cXYZ& XYZ);
cXYZ  RGB2XYZSMPTE(cRGB& vRGB);
cRGB  XYZSMPTE2RGB(cXYZ& XYZ);
cLuv  XYZ2Luv(cXYZ& XYZ);
cXYZ  Luv2XYZ(cLuv& Luv);
cLab  XYZ2Lab(cXYZ& XYZ);
cXYZ  Lab2XYZ(cLab& Lab);

void Between(cRGB* resColor, cRGB* minColor, cRGB* maxColor, double Percent);
void Between(cCMYK* resColor, cCMYK* minColor, cCMYK* maxColor, double Percent);
void Between(cHLS* resColor, cHLS* minColor, cHLS* maxColor, double Percent);
void Between(cHSI* resColor, cHSI* minColor, cHSI* maxColor, double Percent);
void Between(cYCbCr* resColor, cYCbCr* minColor, cYCbCr* maxColor, double Percent);
void Between(cXYZ* resColor, cXYZ* minColor, cXYZ* maxColor, double Percent);
void Between(cLuv* resColor, cLuv* minColor, cLuv* maxColor, double Percent);
void Between(cLab* resColor, cLab* minColor, cLab* maxColor, double Percent);

void Between(cColor* resColor, cColor* minColor, cColor* maxColor, double Percent);

DWORD flipColor(DWORD color);

BYTE getLight(DWORD Color);
BYTE getSaturation(DWORD Color);
DWORD colorLSmove(DWORD Color, int L, int S); // -255..+255
DWORD colorLSmul(DWORD Color, int L, int S); // 0..256 = 0..100%
DWORD colorLSset(DWORD Color, BYTE L, BYTE S); // 0..255
DWORD inverseColor(DWORD Color);

// whitness & chromatic
DWORD colorWCset(DWORD Color, int W, int C); // 0..255
BYTE getWhitness(DWORD Color); // 0..255
BYTE getChromatic(DWORD Color); // 0..255
BYTE getLuminance(DWORD Color); // 0..255

unsigned long clRGB2CMY(unsigned long color); // ARGB > ACMY
unsigned long clCMY2RGB(unsigned long color); // ACMY > ARGB
unsigned long clRGB2HLS(unsigned long color); // ARGB > ALHS
unsigned long clHLS2RGB(unsigned long color); // ALHS > ARGB
unsigned long clRGB2HSI(unsigned long color); // ARGB > ASHI
unsigned long clHSI2RGB(unsigned long color); // ASHI > ARGB
unsigned long clRGB2YCbCr(unsigned long color); // ARGB > ACbYCr
unsigned long clYCbCr2RGB(unsigned long color); // ACbYCr > ARGB
unsigned long clRGB2XYZ(unsigned long color); // ARGB > AYXZ
unsigned long clXYZ2RGB(unsigned long color); // AYXZ > ARGB
unsigned long clRGB2Luv(unsigned long color); // ARGB > AuLv
unsigned long clLuv2RGB(unsigned long color); // AuLv > ARGB
unsigned long clRGB2Lab(unsigned long color); // ARGB > AaLb
unsigned long clLab2RGB(unsigned long color); // AaLb > ARGB

unsigned long clDistance(unsigned long color1, unsigned long color2);

inline uint32_t clSwapRGB(uint32_t val) {
	__asm__("bswap %0\n\tror $8, %0" : "+r" (val));
	return val;
}

#endif
