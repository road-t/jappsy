#include "uColor.h"

double min3(double v1, double v2, double v3) {
	double m; if (v1<v2) m = v1; else m = v2; if (v3<m) m = v3;
	return m;
}
double max3(double v1, double v2, double v3) {
	double m; if (v1>v2) m = v1; else m = v2; if (v3>m) m = v3;
	return m;
}
double trunc(double v) { return floor(v); }
double centre(double v1, double v2, double v3) {
	if ((v1<v2) && (v1<v3)) { if (v2<v3) return v2; else return v3; }
	if ((v2<v1) && (v2<v3)) { if (v1<v3) return v1; else return v3; }
	if (v1<v2) return v1;
	return v2;
}
bool verify(double* v) {
	if (*v < -eps) return false;
	if (*v > (1+eps)) return false;
	if (*v <= eps) *v = 0;
	else if (*v >= (1-eps)) *v = 1;
	return true;
}

cColorTypes cColor::Type() { return ctNone; }
cColorTypes cRGB::Type() { return ctRGB; }
cColorTypes cCMY::Type() { return ctCMY; }
cColorTypes cCMYK::Type() { return ctCMYK; }
cColorTypes cHLS::Type() { return ctHLS; }
cColorTypes cHSI::Type() { return ctHSI; }
cColorTypes cYCbCr::Type() { return ctYCbCr; }
cColorTypes cXYZ::Type() { return ctXYZ; }
cColorTypes cLuv::Type() { return ctLuv; }
cColorTypes cLab::Type() { return ctLab; }

double cColor::distance() {
	double res = 0;
	switch (this->Type()) {
		case ctNone:  res = 0; break;
		case ctRGB:   res = sqrt(pow(((cRGB*)this)->R,2)+pow(((cRGB*)this)->G,2)+pow(((cRGB*)this)->B,2)); break;
		case ctCMY:   res = sqrt(pow(((cCMY*)this)->C,2)+pow(((cCMY*)this)->M,2)+pow(((cCMY*)this)->Y,2)); break;
		case ctCMYK:  res = sqrt(pow(((cCMYK*)this)->C,2)+pow(((cCMYK*)this)->M,2)+pow(((cCMYK*)this)->Y,2)+pow(((cCMYK*)this)->K,2)); break;
		case ctHLS:   res = sqrt(pow(((cHLS*)this)->H,2)+pow(((cHLS*)this)->L,2)+pow(((cHLS*)this)->S,2)); break;
		case ctHSI:   res = sqrt(pow(((cHSI*)this)->H,2)+pow(((cHSI*)this)->S,2)+pow(((cHSI*)this)->I,2)); break;
		case ctYCbCr: res = sqrt(pow(((cYCbCr*)this)->Y,2)+pow(((cYCbCr*)this)->Cb,2)+pow(((cYCbCr*)this)->Cr,2)); break;
		case ctXYZ:   res = sqrt(pow(((cXYZ*)this)->X,2)+pow(((cXYZ*)this)->Y,2)+pow(((cXYZ*)this)->Z,2)); break;
		case ctLuv:   res = sqrt(pow(((cLuv*)this)->L,2)+pow(((cLuv*)this)->u,2)+pow(((cLuv*)this)->v,2)); break;
		case ctLab:   res = sqrt(pow(((cLab*)this)->L,2)+pow(((cLab*)this)->a,2)+pow(((cLab*)this)->b,2)); break;
	}
	return res;
}

double cColor::distance(cColor* Color) {
	double res = 0;
	switch (this->Type()) {
		case ctNone:  res = 0; break;
		case ctRGB:   res = sqrt(pow(((cRGB*)this)->R-((cRGB*)Color)->R,2)+pow(((cRGB*)this)->G-((cRGB*)Color)->G,2)+pow(((cRGB*)this)->B-((cRGB*)Color)->B,2)); break;
		case ctCMY:   res = sqrt(pow(((cCMY*)this)->C-((cCMY*)Color)->C,2)+pow(((cCMY*)this)->M-((cCMY*)Color)->M,2)+pow(((cCMY*)this)->Y-((cCMY*)Color)->Y,2)); break;
		case ctCMYK:  res = sqrt(pow(((cCMYK*)this)->C-((cCMYK*)Color)->C,2)+pow(((cCMYK*)this)->M-((cCMYK*)Color)->M,2)+pow(((cCMYK*)this)->Y-((cCMYK*)Color)->Y,2)+pow(((cCMYK*)this)->K-((cCMYK*)Color)->K,2)); break;
		case ctHLS:   res = sqrt(pow(((cHLS*)this)->H-((cHLS*)Color)->H,2)+pow(((cHLS*)this)->L-((cHLS*)Color)->L,2)+pow(((cHLS*)this)->S-((cHLS*)Color)->S,2)); break;
		case ctHSI:   res = sqrt(pow(((cHSI*)this)->H-((cHSI*)Color)->H,2)+pow(((cHSI*)this)->S-((cHSI*)Color)->S,2)+pow(((cHSI*)this)->I-((cHSI*)Color)->I,2)); break;
		case ctYCbCr: res = sqrt(pow(((cYCbCr*)this)->Y-((cYCbCr*)Color)->Y,2)+pow(((cYCbCr*)this)->Cb-((cYCbCr*)Color)->Cb,2)+pow(((cYCbCr*)this)->Cr-((cYCbCr*)Color)->Cr,2)); break;
		case ctXYZ:   res = sqrt(pow(((cXYZ*)this)->X-((cXYZ*)Color)->X,2)+pow(((cXYZ*)this)->Y-((cXYZ*)Color)->Y,2)+pow(((cXYZ*)this)->Z-((cXYZ*)Color)->Z,2)); break;
		case ctLuv:   res = sqrt(pow(((cLuv*)this)->L-((cLuv*)Color)->L,2)+pow(((cLuv*)this)->u-((cLuv*)Color)->u,2)+pow(((cLuv*)this)->v-((cLuv*)Color)->v,2)); break;
		case ctLab:   res = sqrt(pow(((cLab*)this)->L-((cLab*)Color)->L,2)+pow(((cLab*)this)->a-((cLab*)Color)->a,2)+pow(((cLab*)this)->b-((cLab*)Color)->b,2)); break;
	}
	return res;
}

void cColor::toRGB(cRGB* resColor) {
	switch (this->Type()) {
		case ctRGB:   *resColor = (cRGB*)this; break;
		case ctCMY:   *resColor = CMY2RGB(*(cCMY*)this); break;
		case ctCMYK:
			{
				cCMY cmy = CMYK2CMY(*(cCMYK*)this);
				*resColor = CMY2RGB(cmy);
			}
			break;
		case ctHLS:   *resColor = HLS2RGB(*(cHLS*)this); break;
		case ctHSI:   *resColor = HSI2RGB(*(cHSI*)this); break;
		case ctYCbCr: *resColor = YCbCr2RGB(*(cYCbCr*)this); break;
		case ctXYZ:   *resColor = XYZEBU2RGB(*(cXYZ*)this); break;
		case ctLuv:
			{
				cXYZ xyz = Luv2XYZ(*(cLuv*)this);
				*resColor = XYZEBU2RGB(xyz);
			}
			break;
		case ctLab:
			{
				cXYZ xyz = Lab2XYZ(*(cLab*)this);
				*resColor = XYZEBU2RGB(xyz);
			}
			break;
		default:;
	}
}

void cColor::fromRGB(cRGB* srcColor) {
	switch (this->Type()) {
		case ctRGB:   *(cRGB*)this = srcColor; break;
		case ctCMY:   *(cCMY*)this = RGB2CMY(*srcColor); break;
		case ctCMYK:
			{
				cCMY cmy = RGB2CMY(*srcColor);
				*(cCMYK*)this = CMY2CMYK(cmy);
			}
			break;
		case ctHLS:   *(cHLS*)this = RGB2HLS(*srcColor); break;
		case ctHSI:   *(cHSI*)this = RGB2HSI(*srcColor); break;
		case ctYCbCr: *(cYCbCr*)this = RGB2YCbCr(*srcColor); break;
		case ctXYZ:   *(cXYZ*)this = RGB2XYZEBU(*srcColor); break;
		case ctLuv:
			{
				cXYZ xyz = RGB2XYZEBU(*srcColor);
				*(cLuv*)this = XYZ2Luv(xyz);
			}
			break;
		case ctLab:
			{
				cXYZ xyz = RGB2XYZEBU(*srcColor);
				*(cLab*)this = XYZ2Lab(xyz);
			}
			break;
		default:;
	}
}

cRGB::cRGB() { }
cCMY::cCMY() { }
cCMYK::cCMYK() { }
cHLS::cHLS() { }
cHSI::cHSI() { }
cYCbCr::cYCbCr() { }
cXYZ::cXYZ() { }
cLuv::cLuv() { }
cLab::cLab() { }

void cRGB::operator =(cRGB* srcColor) { R = srcColor->R; G = srcColor->G; B = srcColor->B; }
void cRGB::operator =(cRGB& srcColor) { R = srcColor.R; G = srcColor.G; B = srcColor.B; }
void cRGB::operator =(cRGB srcColor) { R = srcColor.R; G = srcColor.G; B = srcColor.B; }

cRGB::cRGB(double nR, double nG, double nB) { R = nR; G =nG; B = nB; }
DWORD cRGB::getColor() {
  return ((DWORD)(R*255)<<16)+((DWORD)(G*255)<<8)+((DWORD)(B*255));
}
void cRGB::setColor(DWORD dwColor) {
  R = double((dwColor>>16)&0xFF)/255;
  G = double((dwColor>>8)&0xFF)/255;
  B = double(dwColor&0xFF)/255;
}

double cRGB::getW() { return max3(R,G,B); }
double cRGB::getK() { return min3(R,G,B); }
cRGB cRGB::getWhitness() { double w = getW(); return cRGB(w,w,w); }
cRGB cRGB::getChromatic() { double k = getK(); return cRGB(k,k,k); }

cCMY::cCMY(double nC, double nM, double nY) { C = nC; M =nM; Y = nY; }

cCMYK::cCMYK(double nC, double nM, double nY, double nK) { C = nC; M =nM; Y = nY; K = nK; }

cHLS::cHLS(double nH, double nL, double nS) { H = nH; L = nL; S = nS; }

cHSI::cHSI(double nH, double nS, double nI) { H = nH; S = nS; I = nI; }

cYCbCr::cYCbCr(double nY, double nCb, double nCr) { Y = nY; Cb = nCb; Cr = nCr; }

cXYZ::cXYZ(double nX, double nY, double nZ) { X = nX; Y = nY; Z = nZ; }

cLab::cLab(double nL, double na, double nb) { L = nL; a = na; b = nb; }

cLuv::cLuv(double nL, double nu, double nv) { L = nL; u = nu; v = nv; }

cRGB  CMY2RGB(cCMY& vCMY) { return cRGB(1-vCMY.C,1-vCMY.M,1-vCMY.Y); }
cCMY  RGB2CMY(cRGB& vRGB) { return cCMY(1-vRGB.R,1-vRGB.G,1-vRGB.B); }
cCMYK CMY2CMYK(cCMY& vCMY) { double k = min3(vCMY.C,vCMY.M,vCMY.Y); return cCMYK(vCMY.C-k,vCMY.M-k,vCMY.Y-k,k); }
cCMY  CMYK2CMY(cCMYK& vCMYK) { return cCMY(vCMYK.C+vCMYK.K,vCMYK.M+vCMYK.K,vCMYK.Y+vCMYK.K); }

// Linear transformation (1)
cRGB lHue2RGB(double minRGB, double maxRGB, double Hue) {
  while (Hue < 0) Hue+=360;
  int n = (int)trunc(Hue/60);
  double F = (Hue - n*60)/60;
  n = (int)fmod(n,6);
  double mu,md;
  if (F > eps) {
    mu = minRGB + (maxRGB-minRGB)*F;
    md = maxRGB - (maxRGB-minRGB)*F;
  } else {
    mu = minRGB; md = maxRGB;
  }
  switch (n) {
    case 0: return cRGB(maxRGB,mu,minRGB);
    case 1: return cRGB(md,maxRGB,minRGB);
    case 2: return cRGB(minRGB,maxRGB,mu);
    case 3: return cRGB(minRGB,md,maxRGB);
    case 4: return cRGB(mu,minRGB,maxRGB);
  }
  return cRGB(maxRGB,minRGB,md);
}
double lRGB2Hue(cRGB& vRGB) {
  double m0 = vRGB.getK();
  double m2 = vRGB.getW();
  int n;
  if (fabs(m2-m0)<=eps) return 0;
  double m1 = centre(vRGB.R,vRGB.G,vRGB.B);
  if (fabs(vRGB.R-m2)<=eps) {
    if (fabs(vRGB.B-m0)<=eps) n = 0;
    else n = 5;
  } else if (fabs(vRGB.G-m2)<=eps) {
    if (fabs(vRGB.B-m0)<=eps) n = 1;
    else n = 2;
  } else if (fabs(vRGB.R-m0)<=eps) n = 3;
  else n = 4;
  double F;
  if ((n&1) == 0) F = m1-m0;
  else F = m2-m1;
  F = F/(m2-m0);
  return 60*(n+F);
}
// Non-linear transformation (3)
cRGB nlHue2RGB(double minRGB, double maxRGB, double Hue) {
  while (Hue < 0) Hue+=360;
  while (Hue >= 360) Hue-=360;
  int n = (int)trunc(Hue/60);
  double F;
  if (Hue < 60) F = Hue;
  else if (Hue < 180) F = Hue-120;
  else if (Hue < 300) F = Hue-240;
  else F = Hue-360;
  double m = minRGB + (maxRGB-minRGB)*sqrt(fabs(F/60));
  switch (n) {
    case 0: return cRGB(maxRGB,m,minRGB);
    case 1: return cRGB(m,maxRGB,minRGB);
    case 2: return cRGB(minRGB,maxRGB,m);
    case 3: return cRGB(minRGB,m,maxRGB);
    case 4: return cRGB(m,minRGB,maxRGB);
  }
  return cRGB(maxRGB,minRGB,m);
}
double nlRGB2Hue(cRGB& vRGB) {
  double m0 = vRGB.getK();
  double m2 = vRGB.getW();
  double m1 = centre(vRGB.R,vRGB.G,vRGB.B);
  double H;
//  int n;
  if (fabs(m2-m1)<=eps) {
    if (fabs(vRGB.R-vRGB.G)<=eps) H = 60;
    else if (fabs(vRGB.G-vRGB.B)<=eps) H=180;
    else H = 300;
  } else {
    double F = 60*(m1-m0)/(m2-m0);
    if (fabs(vRGB.R-m2)<=eps) H = 0+F*(vRGB.G-vRGB.B);
    else if (fabs(vRGB.G-m2)<=eps) H = 120+F*(vRGB.B-vRGB.R);
    else H = 240+F*(vRGB.R-vRGB.G);
  }
  if (H<0) H = H + 360;
  return H;
}

// R || G || B < 0 then out of range color
cRGB HLS2RGB(cHLS& HLS) {
  if ((!verify(&(HLS.L))) || (!verify(&(HLS.S)))) return cRGB(-1,-1,-1);
  if ((1-eps2) <= HLS.L) return cRGB(1,1,1);
  if (HLS.L <= eps2) return cRGB(0,0,0);
  double d;
  if (HLS.L <= 0.5) d = HLS.S*HLS.L;
  else d = HLS.S*(1-HLS.L);
  if (d <= eps) return cRGB(HLS.L,HLS.L,HLS.L);
  return lHue2RGB(HLS.L-d,HLS.L+d,HLS.H);
}

// S < 0 then out of range color
cHLS RGB2HLS(cRGB& vRGB) {
  if ((!verify(&(vRGB.R))) || (!verify(&(vRGB.G))) || (!verify(&(vRGB.B)))) return cHLS(0,-1,-1);
  double m2 = vRGB.getW();
  double m0 = vRGB.getK();
  double L = (m2+m0)/2;
  if ((1-L) <= eps2) return cHLS(0,1,0);
  if (L <= eps2) return cHLS(0,0,0);
  double d = (m2-m0)/2; if (d <= eps) d = 0;
  double S;
  if (L <= 0.5) S = d/L; else S = d/(1-L);
  if (S <= eps2) return cHLS(0,L,0);
  return cHLS(lRGB2Hue(vRGB),L,S);
}

// R || G || B < 0 then out of range color
cRGB HSI2RGB(cHSI& HSI) {
  if ((!verify(&(HSI.I))) || (!verify(&(HSI.S)))) return cRGB(-1,-1,-1);
  double m0 = HSI.I*(1-HSI.S); double m2 = HSI.I;
  if (m2 <= eps2) return cRGB(0,0,0);
  if ((HSI.S <= eps2) || ((m2-m0) <= eps2)) return cRGB(HSI.I,HSI.I,HSI.I);
  return lHue2RGB(m0,m2,HSI.H);
}

// S < 0 then out of range color
cHSI RGB2HSI(cRGB& vRGB) {
  if ((!verify(&(vRGB.R))) || (!verify(&(vRGB.G))) || (!verify(&(vRGB.B)))) return cHSI(0,-1,-1);
  double m2 = vRGB.getW();
  double m0 = vRGB.getK();
  double I = m2; double S = (m2-m0);
  if (I <= eps2) return cHSI(0,0,0);
  if (S <= eps2) S = 0; else S = S/m2;
  if (S < eps2) S = 0;
  return cHSI(lRGB2Hue(vRGB),S,I);
}

cYCbCr RGB2YCbCr(cRGB& vRGB) {
  double r = vRGB.R;
  double g = vRGB.G;
  double b = vRGB.B;
  double Y = 0.299*r + 0.587*g + 0.114*b;
  double Cb = -0.148*r - 0.291*g + 0.439*b + 0.5;
  double Cr = 0.439*r - 0.368*g - 0.071*b + 0.5;
  return cYCbCr(Y, Cb, Cr);
}

double clip(double v) {
  if (v <= eps) return 0;
  if (v >= 1-eps) return 1;
  return v;
}

cRGB YCbCr2RGB(cYCbCr& YCbCr) {
  double R = clip(YCbCr.Y + 1.371*(YCbCr.Cr-0.5));
  double G = clip(YCbCr.Y - 0.698*(YCbCr.Cr-0.5) - 0.336*(YCbCr.Cb-0.5));
  double B = clip(YCbCr.Y + 1.732*(YCbCr.Cb-0.5));
  return cRGB(R,G,B);
}

const double normX = 1/(0.412291+0.357664+0.180209);
const double normY = 1/(0.212588+0.715329+0.072084);
const double normZ = 1/(0.019326+0.119221+0.949102);

cXYZ RGB2XYZEBU(cRGB& vRGB) {
  double r = vRGB.R;
  double g = vRGB.G;
  double b = vRGB.B;
  if ( r > 0.04045 ) r = pow(( ( r + 0.055 ) / 1.055 ),2.4); else r = r / 12.92;
  if ( g > 0.04045 ) g = pow(( ( g + 0.055 ) / 1.055 ),2.4); else g = g / 12.92;
  if ( b > 0.04045 ) b = pow(( ( b + 0.055 ) / 1.055 ),2.4); else b = b / 12.92;
  double X = 0.412291*r+0.357664*g+0.180209*b;
  double Y = 0.212588*r+0.715329*g+0.072084*b;
  double Z = 0.019326*r+0.119221*g+0.949102*b;
  return cXYZ(X*normX,Y*normY,Z*normZ);
}
cRGB XYZEBU2RGB(cXYZ& XYZ) {
	double X = XYZ.X/normX;
	double Y = XYZ.Y/normY;
	double Z = XYZ.Z/normZ;
  double R = clip(3.241755*X-1.537756*Y-0.498732*Z);
  double G = clip(-0.969027*X+1.875548*Y+0.041546*Z);
  double B = clip(0.055714*X-0.204284*Y+1.058564*Z);
  if ( R > 0.0031308 ) R = 1.055 * ( pow(R,( 1 / 2.4 )) ) - 0.055;
  else R = 12.92 * R;
  if ( G > 0.0031308 ) G = 1.055 * ( pow(G,( 1 / 2.4 )) ) - 0.055;
  else G = 12.92 * G;
  if ( B > 0.0031308 ) B = 1.055 * ( pow(B,( 1 / 2.4 )) ) - 0.055;
  else B = 12.92 * B;
  return cRGB(R,G,B);
}

cXYZ RGB2XYZSMPTE(cRGB& vRGB) {
  double X = 0.393420*vRGB.R+0.365354*vRGB.G+0.191391*vRGB.B;
  double Y = 0.212322*vRGB.R+0.701243*vRGB.G+0.086434*vRGB.B;
  double Z = 0.018734*vRGB.R+0.111963*vRGB.G+0.956953*vRGB.B;
  return cXYZ(X,Y,Z);
}
cRGB XYZSMPTE2RGB(cXYZ& XYZ) {
  double R = clip(3.506899*XYZ.X-1.740235*XYZ.Y-0.544197*XYZ.Z);
  double G = clip(-1.068768*XYZ.X+1.977261*XYZ.Y+0.035162*XYZ.Z);
  double B = clip(0.056391*XYZ.X-0.197270*XYZ.Y+1.051524*XYZ.Z);
  return cRGB(R,G,B);
}

const double Wu = 0.2009;
const double Wv = 0.4610;
const double Wy = 0.329184;
const double Wx = 0.312779;
const double Wz = 0.358037;

const double normU = 0.684491;

cLuv XYZ2Luv(cXYZ& XYZ) {
  double sum = (XYZ.X+15*XYZ.Y+3*XYZ.Z);
  if (sum <= eps) return cLuv(0,0,0);
  double _u = 4*XYZ.X/sum;
  double _v = 9*XYZ.Y/sum;
  double YWy = XYZ.Y;//Wy;
  double L;
  if (YWy > 0.008856) L = 116*pow(YWy,(double)1/3)-16;
  else L = 903.3*YWy;
  double U = 13*L*(_u-Wu)*normU;
  double V = 13*L*(_v-Wv);
  return cLuv(L,U,V);
}
cXYZ Luv2XYZ(cLuv& Luv) {
  double _u,_v;
  if (Luv.L > 0) { _u = Luv.u/(13*Luv.L*normU)+Wu; _v = Luv.v/(13*Luv.L)+Wv; }
  else { return cXYZ(0,0,0); }
  double Y = 0;
  if (Luv.L <= 8) Y = Luv.L / 903.3;// * pow(3.0/29.0,3);// * Wy;
  else Y = pow(( Luv.L + 16 ) / 116,3);// * Wy;
  double X = Y * (9 * _u)/(4 * _v);
  double Z = (9*Y - 15*_v*Y - _v*X)/(3*_v);
//  double Z = Y * ((12 - 3 * _u - 20 * _v)/(4 * _v));
/*  double Y = clip(pow((Luv.L+16)/116,3));
  double X = clip(-9*Y*_u/((_u-4)*_v-_u*_v));
  double Z = clip((9*Y-15*_v*Y-_v*X)/(3*_v));
*/
  return cXYZ(X,Y,Z);
}

double FLab(double p) {
  if (p > 0.008856) return pow(p,(double)1/3);
  return 7.787*p+16.0/116.0;
}

const double normLab = 2.0/3.333333;

cLab XYZ2Lab(cXYZ& XYZ) {
	double X = FLab(XYZ.X / Wx);
	double Y = FLab(XYZ.Y / Wy);
	double Z = FLab(XYZ.Z / Wz);
	double L = ( 116 * Y ) - 16;
	double a = 500 * ( X - Y );
	double b = 200 * ( Y - Z );
	return cLab(L*normLab,a*normLab,b*normLab);
}
cXYZ Lab2XYZ(cLab& Lab) {
  double P = (Lab.L/normLab+16)/116;
  double X = Wx*pow(P+Lab.a/(500*normLab),3);
  double Y = Wy*pow(P,3);
  double Z = Wz*pow(P-Lab.b/(200*normLab),3);
  return cXYZ(X,Y,Z);
}

void Between(cRGB* resColor, cRGB* minColor, cRGB* maxColor, double Percent) {
  resColor->R = minColor->R + (maxColor->R - minColor->R)*Percent;
  resColor->G = minColor->G + (maxColor->G - minColor->G)*Percent;
  resColor->B = minColor->B + (maxColor->B - minColor->B)*Percent;
}

void Between(cCMYK* resColor, cCMYK* minColor, cCMYK* maxColor, double Percent) {
  resColor->C = minColor->C + (maxColor->C - minColor->C)*Percent;
  resColor->M = minColor->M + (maxColor->M - minColor->M)*Percent;
  resColor->Y = minColor->Y + (maxColor->Y - minColor->Y)*Percent;
  resColor->K = minColor->K + (maxColor->K - minColor->K)*Percent;
}

void Between(cHLS* resColor, cHLS* minColor, cHLS* maxColor, double Percent) {
  resColor->H = minColor->H + (maxColor->H - minColor->H)*Percent;
  resColor->L = minColor->L + (maxColor->L - minColor->L)*Percent;
  resColor->S = minColor->S + (maxColor->S - minColor->S)*Percent;
}

void Between(cHSI* resColor, cHSI* minColor, cHSI* maxColor, double Percent) {
  resColor->H = minColor->H + (maxColor->H - minColor->H)*Percent;
  resColor->S = minColor->S + (maxColor->S - minColor->S)*Percent;
  resColor->I = minColor->I + (maxColor->I - minColor->I)*Percent;
}

void Between(cYCbCr* resColor, cYCbCr* minColor, cYCbCr* maxColor, double Percent) {
  resColor->Y = minColor->Y + (maxColor->Y - minColor->Y)*Percent;
  resColor->Cb = minColor->Cb + (maxColor->Cb - minColor->Cb)*Percent;
  resColor->Cr = minColor->Cr + (maxColor->Cr - minColor->Cr)*Percent;
}

void Between(cXYZ* resColor, cXYZ* minColor, cXYZ* maxColor, double Percent) {
  resColor->X = minColor->X + (maxColor->X - minColor->X)*Percent;
  resColor->Y = minColor->Y + (maxColor->Y - minColor->Y)*Percent;
  resColor->Z = minColor->Z + (maxColor->Z - minColor->Z)*Percent;
}

void Between(cLuv* resColor, cLuv* minColor, cLuv* maxColor, double Percent) {
  resColor->L = minColor->L + (maxColor->L - minColor->L)*Percent;
  resColor->u = minColor->u + (maxColor->u - minColor->u)*Percent;
  resColor->v = minColor->v + (maxColor->v - minColor->v)*Percent;
}

void Between(cLab* resColor, cLab* minColor, cLab* maxColor, double Percent) {
  resColor->L = minColor->L + (maxColor->L - minColor->L)*Percent;
  resColor->a = minColor->a + (maxColor->a - minColor->a)*Percent;
  resColor->b = minColor->b + (maxColor->b - minColor->b)*Percent;
}

void Between(cColor* resColor, cColor* minColor, cColor* maxColor, double Percent) {
  switch (resColor->Type()) {
    case ctRGB:   Between((cRGB*)resColor,(cRGB*)minColor,(cRGB*)maxColor, Percent); break;
    case ctCMY:   Between((cRGB*)resColor,(cRGB*)minColor,(cRGB*)maxColor, Percent); break;
    case ctCMYK:  Between((cCMYK*)resColor,(cCMYK*)minColor,(cCMYK*)maxColor, Percent); break;
    case ctHLS:   Between((cHLS*)resColor,(cHLS*)minColor,(cHLS*)maxColor, Percent); break;
    case ctHSI:   Between((cHSI*)resColor,(cHSI*)minColor,(cHSI*)maxColor, Percent); break;
    case ctYCbCr: Between((cYCbCr*)resColor,(cYCbCr*)minColor,(cYCbCr*)maxColor, Percent); break;
    case ctXYZ:   Between((cXYZ*)resColor,(cXYZ*)minColor,(cXYZ*)maxColor, Percent); break;
    case ctLuv:   Between((cLuv*)resColor,(cLuv*)minColor,(cLuv*)maxColor, Percent); break;
    case ctLab:   Between((cLab*)resColor,(cLab*)minColor,(cLab*)maxColor, Percent); break;
    default:;
  }
}

DWORD flipColor(DWORD color) {
/* AT&T Assembler */
	__asm__ (
		"roll $0x08, %0\n\t"
		"bswap %0\n\t"
	: "=r" (color) : "r" (color)
	);
/* Intel Assembler */
/*
  __asm {
	mov eax, color
    rol eax, 8
    bswap eax
    mov color, eax
  }
*/
  return color;
}

BYTE getLight(DWORD Color) {
	BYTE res;
/* AT&T Assembler */
	__asm__ __volatile__ (
		// Sort Max..Min
		"cmpb %%al, %%ah\n\t"
		"jae 1f\n\t"
		"xchgb %%al, %%ah\n\t"
	"1:"
		"rorl $0x08, %%eax\n\t"
		"cmpb %%al, %%ah\n\t"
		"jae 2f\n\t"
		"xchgb %%al, %%ah\n\t"
	"2:"
		"xorl %%edx, %%edx\n\t"
		"movb %%ah, %%dl\n\t"
		"movl %%edx, %%edi\n\t"
		"rorl $0x08, %%eax\n\t"
		"cmpb %%al, %%ah\n\t"
		"jae 3f\n\t"
		"xchgb %%al, %%ah\n\t"
	"3:"
		"andl $0xFF, %%eax\n\t"
		"addl %%edi, %%eax\n\t"
		"shrl $0x01, %%eax\n\t"
	: "=a" (res)
	: "a" (Color)
	: "%edx" , "%edi" , "cc"
	);
/* Intel Assembler */
/*
	__asm {
		// Sort Max..Min
		mov eax, Color
		cmp ah, al
		jae getl_gt1
		xchg ah, al
getl_gt1:
		ror eax, 8
		cmp ah, al
		jae getl_gt2
		xchg ah, al
getl_gt2:
		xor edx, edx
		mov dl, ah
		mov edi, edx
		rol eax, 8
		cmp ah, al
		jae getl_gt3
		xchg ah, al
getl_gt3:
		and eax, 0xFF
		add eax, edi
		shr eax, 1
		mov res, al
	}
*/
	return res;
}

BYTE getSaturation(DWORD Color) {
	BYTE res;
/* AT&T Assembler */
    uint32_t c0;
	__asm__ __volatile__ (
		// Sort Max..Min
		"cmpb %%cl, %%ch\n\t"
		"jae 1f\n\t"
		"xchgb %%cl, %%ch\n\t"
	"1:"
		"rorl $0x08, %%ecx\n\t"
		"cmpb %%cl, %%ch\n\t"
		"jae 2f\n\t"
		"xchgb %%cl, %%ch\n\t"
	"2:"
		"xorl %%edx, %%edx\n\t"
		"movb %%ch, %%dl\n\t"
		"movl %%edx, %%edi\n\t"
		"roll $0x08, %%ecx\n\t"
		"cmpb %%cl, %%ch\n\t"
		"jae 3f\n\t"
		"xchgb %%cl, %%ch\n\t"
	"3:"
		"movb %%ch, %%dl\n\t"
		"movl %%edx, %%esi\n\t"
		"andl $0xFF, %%ecx\n\t"
		// edi = max, esi = mid, ecx = min
		"movl %%edi, %%ebx\n\t"
		"addl %%ecx, %%ebx\n\t"
		"shrl $0x01, %%ebx\n\t"
		// ebx = Lightness
		"orb %%bl, %%bl\n\t"
		"jns 4f\n\t"
		"negb %%bl\n\t"
	"4:"
		// ebx = (128-abs(L-128))
		"subl %%ecx, %%edi\n\t"
		"xorl %%eax, %%eax\n\t"
		"or %%ebx, %%ebx\n\t"
		"jz 5f\n\t"
		"movl %%edi, %%eax\n\t"
		"shll $0x06, %%eax\n\t"
		"xorl %%edx, %%edx\n\t"
		"divl %%ebx\n\t"
	"5:"
		// eax = Saturation
	: "=&c" (c0), "=a" (res)
	: "0" (Color)
	: "%edx" , "%edi" , "%esi" , "%ebx" , "cc"
	);
/* Intel Assembler */
/*
	__asm {
		// Sort Max..Min
		mov ecx, Color
		cmp ch, cl
		jae clsm_gt1
		xchg ch, cl
clsm_gt1:
		ror ecx, 8
		cmp ch, cl
		jae clsm_gt2
		xchg ch, cl
clsm_gt2:
		xor edx, edx
		mov dl, ch
		mov edi, edx
		rol ecx, 8
		cmp ch, cl
		jae clsm_gt3
		xchg ch, cl
clsm_gt3:
		mov dl, ch
		mov esi, edx
		and ecx, 0xFF
		// edi = max, esi = mid, ecx = min
		mov ebx, edi
		add ebx, ecx
		shr ebx, 1
		// ebx = Lightness
		or bl, bl
		jns clsm_ns1
		neg bl
clsm_ns1:
		// ebx = (128-abs(L-128))
		sub edi, ecx
		xor eax, eax
		or ebx, ebx
		jz clsm_satzer
		mov eax, edi
		shl eax, 6
		xor edx, edx
		div ebx
clsm_satzer:
		// eax = Saturation
		mov res, al
	}
*/
	return res;
}


DWORD colorLSmove(DWORD Color, int L, int S) { // -255..+255
	DWORD res;
/* AT&T Assembler */
    uint32_t c0;
	__asm__ __volatile__ (
		// Sort Max..Min
		"movl $0x030201, %%ebx\n\t"
		"cmpb %%cl, %%ch\n\t"
		"jae 1f\n\t"
		"xchgb %%cl, %%ch\n\t"
		"xchgb %%bl, %%bh\n\t"
	"1:"
		"rorl $0x08, %%ecx\n\t"
		"rorl $0x08, %%ebx\n\t"
		"cmpb %%cl, %%ch\n\t"
		"jae 2f\n\t"
		"xchgb %%cl, %%ch\n\t"
		"xchgb %%bl, %%bh\n\t"
	"2:"
		"xorl %%edx, %%edx\n\t"
		"movb %%ch, %%dl\n\t"
		"movl %%edx, %%edi\n\t"
		"roll $0x08, %%ecx\n\t"
		"roll $0x08, %%ebx\n\t"
		"cmpb %%cl, %%ch\n\t"
		"jae 3f\n\t"
		"xchgb %%cl, %%ch\n\t"
		"xchgb %%bl, %%bh\n\t"
	"3:"
		"movb %%ch, %%dl\n\t"
		"movl %%edx, %%esi\n\t"
		"andl $0xFF, %%ecx\n\t"
		// edi = max, esi = mid, ecx = min
		// ebx = indexes
		"push %%ebx\n\t"
		"movl %%edi, %%ebx\n\t"
		"addl %%ecx, %%ebx\n\t"
		"shrl $0x01, %%ebx\n\t"
		// ebx = Lightness
		"push %%ebx\n\t"
		"orb %%bl, %%bl\n\t"
		"jns 4f\n\t"
		"negb %%bl\n\t"
	"4:"
		// ebx = (128-abs(L-128))
		"subl %%ecx, %%edi\n\t"
		"xorl %%eax, %%eax\n\t"
		"orl %%ebx, %%ebx\n\t"
		"jz 5f\n\t"
		"movl %%edi, %%eax\n\t"
		"shll $0x06, %%eax\n\t"
		"xorl %%edx, %%edx\n\t"
		"divl %%ebx\n\t"
	"5:"
		"pop %%ebx\n\t"
		// eax = Saturation
		// ebx = Lightness

		// Change L,S
		"addl %3, %%eax\n\t"
		"jns 6f\n\t"
		"xorl %%eax, %%eax\n\t"
		"jmp 7f\n\t"
	"6:"
		"testl $0xFFFFFF00, %%eax\n\t"
		"jz 7f\n\t"
		"movl $0xFF, %%eax\n\t"
	"7:"
		"addl %4, %%ebx\n\t"
		"jns 8f\n\t"
		"xorl %%ebx, %%ebx\n\t"
		"jmp 9f\n\t"
	"8:"
		"testl $0xFFFFFF00, %%ebx\n\t"
		"jz 9f\n\t"
		"movl $0xFF, %%ebx\n\t"
	"9:"

		"push %%ebx\n\t"
		"orb %%bl, %%bl\n\t"
		"jns 10f\n\t"
		"negb %%bl\n\t"
	"10:"
		// ebx = (128-abs(L-128))
		"mull %%ebx\n\t"
		"shrl $0x07, %%eax\n\t"
		"pop %%ebx\n\t"

		"orb %%al, %%al\n\t"
		"jnz 11f\n\t"
		"movl %%ebx, %%ecx\n\t"
		"movl %%ebx, %%esi\n\t"
		"movl %%ebx, %%edi\n\t"
		"jmp 12f\n\t"
	"11:"
		"push %%eax\n\t"
		// nm1 = nm0 + 2*d*(mid-min)/(max-min);
		"shll $0x01, %%eax\n\t"
		"subl %%ecx, %%esi\n\t"
		"mull %%esi\n\t"
		"divl %%edi\n\t"
		"movl %%eax, %%esi\n\t"
		"pop %%eax\n\t"
		"movl %%ebx, %%ecx\n\t"
		"subl %%eax, %%ecx\n\t"
		"addl %%ecx, %%esi\n\t"
		"movl %%ebx, %%edi\n\t"
		"addl %%eax, %%edi\n\t"
	"12:"
		"pop %%ebx\n\t"
		// edi = max, esi = mid, ecx = min
		// ebx = indexes
		// Check Min
		"orl %%ecx, %%ecx\n\t"
		"jns 13f\n\t"
		"xorl %%ecx, %%ecx\n\t"
		"jmp 14f\n\t"
	"13:"
		"testl $0xFFFFFF00, %%ecx\n\t"
		"jz 14f\n\t"
		"movl $0xFF, %%ecx\n\t"
	"14:"
		// Check Mid
		"orl %%esi, %%esi\n\t"
		"jns 15f\n\t"
		"xorl %%esi, %%esi\n\t"
		"jmp 16f\n\t"
	"15:"
		"testl $0xFFFFFF00, %%esi\n\t"
		"jz 16f\n\t"
		"movl $0xFF, %%esi\n\t"
	"16:"
		// Check Max
		"orl %%edi, %%edi\n\t"
		"jns 17f\n\t"
		"xorl %%edi, %%edi\n\t"
		"jmp 18f\n\t"
	"17:"
		"testl $0xFFFFFF00, %%edi\n\t"
		"jz 18f\n\t"
		"movl $0xFF, %%edi\n\t"
	"18:"
		// Sort Indexes
		"movl %%edi, %%edx\n\t"
		"movl %%esi, %%eax\n\t"
		"cmpb %%bh, %%bl\n\t"
		"jna 19f\n\t"
		"xchgb %%bh, %%bl\n\t"
		"xchgl %%ecx, %%eax\n\t"
	"19:"
		"rorl $0x08, %%ebx\n\t"
		"cmpb %%bh, %%bl\n\t"
		"jna 20f\n\t"
		"xchgb %%bh, %%bl\n\t"
		"xchgl %%eax, %%edx\n\t"
	"20:"
		"roll $0x08, %%ebx\n\t"
		"cmpb %%bh, %%bl\n\t"
		"jna 21f\n\t"
		"xchgl %%ecx, %%eax\n\t"
	"21:"

		// Set Values
		"xchgb %%al, %%ah\n\t"
		"xchgb %%dl, %%dh\n\t"
		"bswap %%edx\n\t"
		"orl %%edx, %%eax\n\t"
		"orl %%ecx, %%eax\n\t"
		// eax = newColor
	: "=&c" (c0), "=a" (res)
	: "0" (Color), "m" (S) , "m" (L)
	: "%ebx" , "%edi" , "%edx" , "%esi" , "cc"
	);
/* Intel Assembler */
/*
	__asm {
		// Sort Max..Min
		mov ecx, Color
		mov ebx, 0x030201
		cmp ch, cl
		jae clsm_gt1
		xchg ch, cl
		xchg bh, bl
clsm_gt1:
		ror ecx, 8
		ror ebx, 8
		cmp ch, cl
		jae clsm_gt2
		xchg ch, cl
		xchg bh, bl
clsm_gt2:
		xor edx, edx
		mov dl, ch
		mov edi, edx
		rol ecx, 8
		rol ebx, 8
		cmp ch, cl
		jae clsm_gt3
		xchg ch, cl
		xchg bh, bl
clsm_gt3:
		mov dl, ch
		mov esi, edx
		and ecx, 0xFF
		// edi = max, esi = mid, ecx = min
		// ebx = indexes
		push ebx
		mov ebx, edi
		add ebx, ecx
		shr ebx, 1
		// ebx = Lightness
		push ebx
		or bl, bl
		jns clsm_ns1
		neg bl
clsm_ns1:
		// ebx = (128-abs(L-128))
		sub edi, ecx
		xor eax, eax
		or ebx, ebx
		jz clsm_satzer
		mov eax, edi
		shl eax, 6
		xor edx, edx
		div ebx
clsm_satzer:
		pop ebx
		// eax = Saturation
		// ebx = Lightness

		// Change L,S
		add eax, S
		jns clsm_satok1
		xor eax, eax
		jmp clsm_satok2
clsm_satok1:
		test eax,0xFFFFFF00
		jz clsm_satok2
		mov eax, 0xFF
clsm_satok2:
		add ebx, L
		jns clsm_ligok1
		xor ebx, ebx
		jmp clsm_ligok2
clsm_ligok1:
		test ebx,0xFFFFFF00
		jz clsm_ligok2
		mov ebx, 0xFF
clsm_ligok2:

		push ebx
		or bl, bl
		jns clsm_ns2
		neg bl
clsm_ns2:
		// ebx = (128-abs(L-128))
		mul ebx
		shr eax, 7
		pop ebx

		or al, al
		jnz clsm_nz
		mov ecx, ebx
		mov esi, ebx
		mov edi, ebx
		jmp clsm_end
clsm_nz:
		push eax
		// nm1 = nm0 + 2*d*(mid-min)/(max-min);
		shl eax, 1
		sub esi, ecx
		mul esi
		div edi
		mov esi, eax
		pop eax
		mov ecx, ebx
		sub ecx, eax
		add esi, ecx
		mov edi, ebx
		add edi, eax
clsm_end:
		pop ebx
		// edi = max, esi = mid, ecx = min
		// ebx = indexes
		// Check Min
		or ecx, ecx
		jns clsm_nsmin
		xor ecx, ecx
		jmp clsm_zmin
clsm_nsmin:
		test ecx, 0xFFFFFF00
		jz clsm_zmin
		mov ecx, 0xFF
clsm_zmin:
		// Check Mid
		or esi, esi
		jns clsm_nsmid
		xor esi, esi
		jmp clsm_zmid
clsm_nsmid:
		test esi, 0xFFFFFF00
		jz clsm_zmid
		mov esi, 0xFF
clsm_zmid:
		// Check Max
		or edi, edi
		jns clsm_nsmax
		xor edi, edi
		jmp clsm_zmax
clsm_nsmax:
		test edi, 0xFFFFFF00
		jz clsm_zmax
		mov edi, 0xFF
clsm_zmax:
		// Sort Indexes
		mov edx, edi
		mov eax, esi
		cmp bl, bh
		jna clsm_le1
		xchg bl, bh
		xchg eax, ecx
clsm_le1:
		ror ebx, 8
		cmp bl, bh
		jna clsm_le2
		xchg bl, bh
		xchg edx, eax
clsm_le2:
		rol ebx, 8
		cmp bl, bh
		jna clsm_le3
		xchg eax, ecx
clsm_le3:

		// Set Values
		xchg ah, al
		xchg dh, dl
		bswap edx
		or eax, edx
		or eax, ecx
		// eax = newColor
		mov res, eax
	}
*/
	return res;
}

DWORD colorLSmul(DWORD Color, int L, int S) { // 0..256 = 0..100%
	DWORD res;
/* AT&T Assembler */
    uint32_t c0;
	__asm__ __volatile__ (
		// Sort Max..Min
		"movl $0x030201, %%ebx\n\t"
		"cmpb %%cl, %%ch\n\t"
		"jae 1f\n\t"
		"xchgb %%cl, %%ch\n\t"
		"xchgb %%bl, %%bh\n\t"
	"1:"
		"rorl $0x08, %%ecx\n\t"
		"rorl $0x08, %%ebx\n\t"
		"cmpb %%cl, %%ch\n\t"
		"jae 2f\n\t"
		"xchgb %%cl, %%ch\n\t"
		"xchgb %%bl, %%bh\n\t"
	"2:"
		"xorl %%edx, %%edx\n\t"
		"movb %%ch, %%dl\n\t"
		"movl %%edx, %%edi\n\t"
		"roll $0x08, %%ecx\n\t"
		"roll $0x08, %%ebx\n\t"
		"cmpb %%cl, %%ch\n\t"
		"jae 3f\n\t"
		"xchgb %%cl, %%ch\n\t"
		"xchgb %%bl, %%bh\n\t"
	"3:"
		"movb %%ch, %%dl\n\t"
		"movl %%edx, %%esi\n\t"
		"andl $0xFF, %%ecx\n\t"
		// edi = max, esi = mid, ecx = min
		// ebx = indexes
		"push %%ebx\n\t"
		"movl %%edi, %%ebx\n\t"
		"addl %%ecx, %%ebx\n\t"
		"shrl $0x01, %%ebx\n\t"
		// ebx = Lightness
		"push %%ebx\n\t"
		"orb %%bl, %%bl\n\t"
		"jns 4f\n\t"
		"negb %%bl\n\t"
	"4:"
		// ebx = (128-abs(L-128))
		"subl %%ecx, %%edi\n\t"
		"xorl %%eax, %%eax\n\t"
		"orl %%ebx, %%ebx\n\t"
		"jz 5f\n\t"
		"movl %%edi, %%eax\n\t"
		"shll $0x06, %%eax\n\t"
		"xorl %%edx, %%edx\n\t"
		"divl %%ebx\n\t"
	"5:"
		"pop %%ebx\n\t"
		// eax = Saturation
		// ebx = Lightness

		// Change L,S
		"mull %3\n\t"
		"shrl $0x08, %%eax\n\t"
		"testl $0xFFFFFF00, %%eax\n\t"
		"jz 6f\n\t"
		"movl $0xFF, %%eax\n\t"
	"6:"
		"xchgl %%ebx, %%eax\n\t"
		"mull %4\n\t"
		"shrl $0x08, %%eax\n\t"
		"testl $0xFFFFFF00, %%eax\n\t"
		"jz 7f\n\t"
		"movl $0xFF, %%eax\n\t"
	"7:"
		"xchgl %%ebx, %%eax\n\t"

		"push %%ebx\n\t"
		"orb %%bl, %%bl\n\t"
		"jns 8f\n\t"
		"negb %%bl\n\t"
	"8:"
		// ebx = (128-abs(L-128))
		"mull %%ebx\n\t"
		"shrl $0x07, %%eax\n\t"
		"pop %%ebx\n\t"

		"orb %%al, %%al\n\t"
		"jnz 9f\n\t"
		"mov %%ebx, %%ecx\n\t"
		"mov %%ebx, %%esi\n\t"
		"mov %%ebx, %%edi\n\t"
		"jmp 10f\n\t"
	"9:"
		"push %%eax\n\t"
		// nm1 = nm0 + 2*d*(mid-min)/(max-min);
		"shll $0x01, %%eax\n\t"
		"subl %%ecx, %%esi\n\t"
		"mull %%esi\n\t"
		"divl %%edi\n\t"
		"movl %%eax, %%esi\n\t"
		"pop %%eax\n\t"
		"movl %%ebx, %%ecx\n\t"
		"subl %%eax, %%ecx\n\t"
		"addl %%ecx, %%esi\n\t"
		"movl %%ebx, %%edi\n\t"
		"addl %%eax, %%edi\n\t"
	"10:"
		"pop %%ebx\n\t"
		// edi = max, esi = mid, ecx = min
		// ebx = indexes
		// Check Min
		"orl %%ecx, %%ecx\n\t"
		"jns 11f\n\t"
		"xorl %%ecx, %%ecx\n\t"
		"jmp 12f\n\t"
	"11:"
		"testl $0xFFFFFF00, %%ecx\n\t"
		"jz 12f\n\t"
		"movl $0xFF, %%ecx\n\t"
	"12:"
		// Check Mid
		"orl %%esi, %%esi\n\t"
		"jns 13f\n\t"
		"xorl %%esi, %%esi\n\t"
		"jmp 14f\n\t"
	"13:"
		"testl $0xFFFFFF00, %%esi\n\t"
		"jz 14f\n\t"
		"movl $0xFF, %%esi\n\t"
	"14:"
		// Check Max
		"orl %%edi, %%edi\n\t"
		"jns 15f\n\t"
		"xorl %%edi, %%edi\n\t"
		"jmp 16f\n\t"
	"15:"
		"testl $0xFFFFFF00, %%edi\n\t"
		"jz 16f\n\t"
		"movl $0xFF, %%edi\n\t"
	"16:"
		// Sort Indexes
		"movl %%edi, %%edx\n\t"
		"movl %%esi, %%eax\n\t"
		"cmpb %%bh, %%bl\n\t"
		"jna 17f\n\t"
		"xchgb %%bh, %%bl\n\t"
		"xchgb %%ecx, %%eax\n\t"
	"17:"
		"rorl $0x08, %%ebx\n\t"
		"cmpb %%bh, %%bl\n\t"
		"jna 18f\n\t"
		"xchgb %%bh, %%bl\n\t"
		"xchgl %%eax, %%edx\n\t"
	"18:"
		"roll $0x08, %%ebx\n\t"
		"cmpb %%bh, %%bl\n\t"
		"jna 19f\n\t"
		"xchgl %%ecx, %%eax\n\t"
	"19:"

		// Set Values
		"xchgb %%al, %%ah\n\t"
		"xchgb %%dl, %%dh\n\t"
		"bswap %%edx\n\t"
		"orl %%edx, %%eax\n\t"
		"orl %%ecx, %%eax\n\t"
		// eax = newColor
	: "=&c" (c0), "=a" (res)
	: "0" (Color), "m" (S) , "m" (L)
	: "%ebx" , "%edi" , "%edx" , "%esi" , "cc"
	);
/* Intel Assembler */
/*
	__asm {
		// Sort Max..Min
		mov ecx, Color
		mov ebx, 0x030201
		cmp ch, cl
		jae clsm_gt1
		xchg ch, cl
		xchg bh, bl
clsm_gt1:
		ror ecx, 8
		ror ebx, 8
		cmp ch, cl
		jae clsm_gt2
		xchg ch, cl
		xchg bh, bl
clsm_gt2:
		xor edx, edx
		mov dl, ch
		mov edi, edx
		rol ecx, 8
		rol ebx, 8
		cmp ch, cl
		jae clsm_gt3
		xchg ch, cl
		xchg bh, bl
clsm_gt3:
		mov dl, ch
		mov esi, edx
		and ecx, 0xFF
		// edi = max, esi = mid, ecx = min
		// ebx = indexes
		push ebx
		mov ebx, edi
		add ebx, ecx
		shr ebx, 1
		// ebx = Lightness
		push ebx
		or bl, bl
		jns clsm_ns1
		neg bl
clsm_ns1:
		// ebx = (128-abs(L-128))
		sub edi, ecx
		xor eax, eax
		or ebx, ebx
		jz clsm_satzer
		mov eax, edi
		shl eax, 6
		xor edx, edx
		div ebx
clsm_satzer:
		pop ebx
		// eax = Saturation
		// ebx = Lightness

		// Change L,S
		mul S
		shr eax, 8
		test eax,0xFFFFFF00
		jz clsm_satok1
		mov eax, 0xFF
clsm_satok1:
		xchg eax, ebx
		mul L
		shr eax, 8
		test eax,0xFFFFFF00
		jz clsm_ligok1
		mov eax, 0xFF
clsm_ligok1:
		xchg eax, ebx

		push ebx
		or bl, bl
		jns clsm_ns2
		neg bl
clsm_ns2:
		// ebx = (128-abs(L-128))
		mul ebx
		shr eax, 7
		pop ebx

		or al, al
		jnz clsm_nz
		mov ecx, ebx
		mov esi, ebx
		mov edi, ebx
		jmp clsm_end
clsm_nz:
		push eax
		// nm1 = nm0 + 2*d*(mid-min)/(max-min);
		shl eax, 1
		sub esi, ecx
		mul esi
		div edi
		mov esi, eax
		pop eax
		mov ecx, ebx
		sub ecx, eax
		add esi, ecx
		mov edi, ebx
		add edi, eax
clsm_end:
		pop ebx
		// edi = max, esi = mid, ecx = min
		// ebx = indexes
		// Check Min
		or ecx, ecx
		jns clsm_nsmin
		xor ecx, ecx
		jmp clsm_zmin
clsm_nsmin:
		test ecx, 0xFFFFFF00
		jz clsm_zmin
		mov ecx, 0xFF
clsm_zmin:
		// Check Mid
		or esi, esi
		jns clsm_nsmid
		xor esi, esi
		jmp clsm_zmid
clsm_nsmid:
		test esi, 0xFFFFFF00
		jz clsm_zmid
		mov esi, 0xFF
clsm_zmid:
		// Check Max
		or edi, edi
		jns clsm_nsmax
		xor edi, edi
		jmp clsm_zmax
clsm_nsmax:
		test edi, 0xFFFFFF00
		jz clsm_zmax
		mov edi, 0xFF
clsm_zmax:
		// Sort Indexes
		mov edx, edi
		mov eax, esi
		cmp bl, bh
		jna clsm_le1
		xchg bl, bh
		xchg eax, ecx
clsm_le1:
		ror ebx, 8
		cmp bl, bh
		jna clsm_le2
		xchg bl, bh
		xchg edx, eax
clsm_le2:
		rol ebx, 8
		cmp bl, bh
		jna clsm_le3
		xchg eax, ecx
clsm_le3:

		// Set Values
		xchg ah, al
		xchg dh, dl
		bswap edx
		or eax, edx
		or eax, ecx
		// eax = newColor
		mov res, eax
	}
*/
	return res;
}

DWORD colorLSset(DWORD Color, BYTE L, BYTE S) { // 0..255
	DWORD res;
	if (L == 0) return 0;
	if (L == 255) return 0x00FFFFFF;
/* AT&T Assembler */
    uint32_t c0;
	__asm__ __volatile__ (
		// Sort Max..Min
		"movl $0x030201, %%ebx\n\t"
		"cmpb %%cl, %%ch\n\t"
		"jae 1f\n\t"
		"xchgb %%cl, %%ch\n\t"
		"xchgb %%bl, %%bh\n\t"
	"1:"
		"rorl $0x08, %%ecx\n\t"
		"rorl $0x08, %%ebx\n\t"
		"cmpb %%cl, %%ch\n\t"
		"jae 2f\n\t"
		"xchgb %%cl, %%ch\n\t"
		"xchgb %%bl, %%bh\n\t"
	"2:"
		"xorl %%edx, %%edx\n\t"
		"movb %%ch, %%dl\n\t"
		"movl %%edx, %%edi\n\t"
		"roll $0x08, %%ecx\n\t"
		"roll $0x08, %%ebx\n\t"
		"cmpb %%cl, %%ch\n\t"
		"jae 3f\n\t"
		"xchgb %%cl, %%ch\n\t"
		"xchgb %%bl, %%bh\n\t"
	"3:"
		"movb %%ch, %%dl\n\t"
		"movl %%edx, %%esi\n\t"
		"andl $0xFF, %%ecx\n\t"
		// edi = max, esi = mid, ecx = min
		// ebx = indexes
		"push %%ebx\n\t"
		"movl %%edi, %%ebx\n\t"
		"addl %%ecx, %%ebx\n\t"
		"shrl $0x01, %%ebx\n\t"
		// ebx = Lightness
		"push %%ebx\n\t"
		"orb %%bl, %%bl\n\t"
		"jns 4f\n\t"
		"negb %%bl\n\t"
	"4:"
		// ebx = (128-abs(L-128))
		"subl %%ecx, %%edi\n\t"
		"xorl %%eax, %%eax\n\t"
		"orl %%ebx, %%ebx\n\t"
		"jz 5f\n\t"
		"subl %%ecx, %%edi\n\t"
		"xorl %%eax, %%eax\n\t"
		"orl %%ebx, %%ebx\n\t"
		"jz 5f\n\t"
		"movl %%edi, %%eax\n\t"
		"shll $0x06, %%eax\n\t"
		"xorl %%edx, %%edx\n\t"
		"divl %%ebx\n\t"
	"5:"
		"pop %%ebx\n\t"
		// eax = Saturation
		// ebx = Lightness

		// Change L,S
		"xorl %%eax, %%eax\n\t"
		"movb %3, %%al\n\t"
		"xorl %%ebx, %%ebx\n\t"
		"movb %4, %%bl\n\t"

		"push %%ebx\n\t"
		"orb %%bl, %%bl\n\t"
		"jns 6f\n\t"
		"negb %%bl\n\t"
	"6:"
		// ebx = (128-abs(L-128))
		"mull %%ebx\n\t"
		"shrl $0x07, %%eax\n\t"
		"pop %%ebx\n\t"

		"orb %%al, %%al\n\t"
		"jnz 7f\n\t"
		"movl %%ebx, %%ecx\n\t"
		"movl %%ebx, %%esi\n\t"
		"movl %%ebx, %%edi\n\t"
		"jmp 10f\n\t"
	"7:"
		"cmpl $0x00, %%edi\n\t"
		"jz 8f\n\t"
		"push %%eax\n\t"
		// nm1 = nm0 + 2*d*(mid-min)/(max-min);
		"shll $0x01, %%eax\n\t"
		"subl %%ecx, %%esi\n\t"
		"mull %%esi\n\t"
		"divl %%edi\n\t"
		"movl %%eax, %%esi\n\t"
		"pop %%eax\n\t"
		"jmp 9f\n\t"
	"8:"
		"movl %%ebx, %%ecx\n\t"
		"movl %%ebx, %%esi\n\t"
		"movl %%ebx, %%edi\n\t"
		"jmp 10f\n\t"
	"9:"
		"movl %%ebx, %%ecx\n\t"
		"subl %%eax, %%ecx\n\t"
		"addl %%ecx, %%esi\n\t"
		"movl %%ebx, %%edi\n\t"
		"addl %%eax, %%edi\n\t"
	"10:"
		"pop %%ebx\n\t"
		// edi = max, esi = mid, ecx = min
		// ebx = indexes
		// Check Min
		"orl %%ecx, %%ecx\n\t"
		"jns 11f\n\t"
		"xorl %%ecx, %%ecx\n\t"
		"jmp 12f\n\t"
	"11:"
		"testl $0xFFFFFF00, %%ecx\n\t"
		"jz 12f\n\t"
		"movl $0xFF, %%ecx\n\t"
	"12:"
		// Check Mid
		"orl %%esi, %%esi\n\t"
		"jns 13f\n\t"
		"xorl %%esi, %%esi\n\t"
		"jmp 14f\n\t"
	"13:"
		"testl $0xFFFFFF00, %%esi\n\t"
		"jz 14f\n\t"
		"mov $0xFF, %%esi\n\t"
	"14:"
		// Check Max
		"orl %%edi, %%edi\n\t"
		"jns 15f\n\t"
		"xorl %%edi, %%edi\n\t"
		"jmp 16f\n\t"
	"15:"
		"testl $0xFFFFFF00, %%edi\n\t"
		"jz 16f\n\t"
		"movl $0xFF, %%edi\n\t"
	"16:"
		// Sort Indexes
		"movl %%edi, %%edx\n\t"
		"movl %%esi, %%eax\n\t"
		"cmpb %%bh, %%bl\n\t"
		"jna 17f\n\t"
		"xchgb %%bh, %%bl\n\t"
		"xchgl %%ecx, %%eax\n\t"
	"17:"
		"rorl $0x08, %%ebx\n\t"
		"cmpb %%bh, %%bl\n\t"
		"jna 18f\n\t"
		"xchgb %%bh, %%bl\n\t"
		"xchgl %%eax, %%edx\n\t"
	"18:"
		"roll $0x08, %%ebx\n\t"
		"cmpb %%bh, %%bl\n\t"
		"jna 19f\n\t"
		"xchgl %%ecx, %%eax\n\t"
	"19:"

		// Set Values
		"xchgb %%al, %%ah\n\t"
		"xchgb %%dl, %%dh\n\t"
		"bswap %%edx\n\t"
		"orl %%edx, %%eax\n\t"
		"orl %%ecx, %%eax\n\t"
		// eax = newColor
	: "=&c" (c0), "=a" (res)
	: "0" (Color), "m" (S) , "m" (L)
	: "%ebx" , "%edi" , "%edx" , "%esi" , "cc"
	);
/* Intel Assembler */
/*
	__asm {
		// Sort Max..Min
		mov ecx, Color
		mov ebx, 0x030201
		cmp ch, cl
		jae clss_gt1
		xchg ch, cl
		xchg bh, bl
clss_gt1:
		ror ecx, 8
		ror ebx, 8
		cmp ch, cl
		jae clss_gt2
		xchg ch, cl
		xchg bh, bl
clss_gt2:
		xor edx, edx
		mov dl, ch
		mov edi, edx
		rol ecx, 8
		rol ebx, 8
		cmp ch, cl
		jae clss_gt3
		xchg ch, cl
		xchg bh, bl
clss_gt3:
		mov dl, ch
		mov esi, edx
		and ecx, 0xFF
		// edi = max, esi = mid, ecx = min
		// ebx = indexes
		push ebx
		mov ebx, edi
		add ebx, ecx
		shr ebx, 1
		// ebx = Lightness
		push ebx
		or bl, bl
		jns clss_ns1
		neg bl
clss_ns1:
		// ebx = (128-abs(L-128))
		sub edi, ecx
		xor eax, eax
		or ebx, ebx
		jz clss_satzer
		mov eax, edi
		shl eax, 6
		xor edx, edx
		div ebx
clss_satzer:
		pop ebx
		// eax = Saturation
		// ebx = Lightness

		// Change L,S
		xor eax, eax
		mov al, S
		xor ebx, ebx
		mov bl, L

		push ebx
		or bl, bl
		jns clss_ns2
		neg bl
clss_ns2:
		// ebx = (128-abs(L-128))
		mul ebx
		shr eax, 7
		pop ebx

		or al, al
		jnz clss_nz
		mov ecx, ebx
		mov esi, ebx
		mov edi, ebx
		jmp clss_end
clss_nz:
		cmp edi, 0
		jz clss_zero
		push eax
		// nm1 = nm0 + 2*d*(mid-min)/(max-min);
		shl eax, 1
		sub esi, ecx
		mul esi
		div edi
		mov esi, eax
		pop eax
		jmp clss_notzero
clss_zero:
		mov ecx, ebx
		mov esi, ebx
		mov edi, ebx
		jmp clss_end
clss_notzero:
		mov ecx, ebx
		sub ecx, eax
		add esi, ecx
		mov edi, ebx
		add edi, eax
clss_end:
		pop ebx
		// edi = max, esi = mid, ecx = min
		// ebx = indexes
		// Check Min
		or ecx, ecx
		jns clss_nsmin
		xor ecx, ecx
		jmp clss_zmin
clss_nsmin:
		test ecx, 0xFFFFFF00
		jz clss_zmin
		mov ecx, 0xFF
clss_zmin:
		// Check Mid
		or esi, esi
		jns clss_nsmid
		xor esi, esi
		jmp clss_zmid
clss_nsmid:
		test esi, 0xFFFFFF00
		jz clss_zmid
		mov esi, 0xFF
clss_zmid:
		// Check Max
		or edi, edi
		jns clss_nsmax
		xor edi, edi
		jmp clss_zmax
clss_nsmax:
		test edi, 0xFFFFFF00
		jz clss_zmax
		mov edi, 0xFF
clss_zmax:
		// Sort Indexes
		mov edx, edi
		mov eax, esi
		cmp bl, bh
		jna clss_le1
		xchg bl, bh
		xchg eax, ecx
clss_le1:
		ror ebx, 8
		cmp bl, bh
		jna clss_le2
		xchg bl, bh
		xchg edx, eax
clss_le2:
		rol ebx, 8
		cmp bl, bh
		jna clss_le3
		xchg eax, ecx
clss_le3:

		// Set Values
		xchg ah, al
		xchg dh, dl
		bswap edx
		or eax, edx
		or eax, ecx
		// eax = newColor
		mov res, eax
	}
*/
	return res;
}

DWORD inverseColor(DWORD Color) {
	BYTE satur = getSaturation(Color);
	if (satur < 64) {
		BYTE light = getLight(Color);
		if (light < 64) {
			return colorLSmove(Color,256,0);
		}
		return colorLSmove(Color,-256,0);
	}
	return Color ^ 0xFFFFFF;
}

DWORD colorWCset(DWORD colorNormalized, int W, int C) { // 0..255
	DWORD color = colorNormalized;
	// set Whitness
/* AT&T Assembler */
	__asm__ __volatile__ (
		"xorl %%ebx, %%ebx\n\t"
		"xorl %%eax, %%eax\n\t"
		"movb $0xFF, %%al\n\t"
		"subb %%cl, %%al\n\t"
		"mull %%esi\n\t"
		"addb %%cl, %%ah\n\t"
		"jnc 1f\n\t"
		"movb $0xFF, %%ah\n\t"
	"1:"
		"movb %%ah, %%bl\n\t"

		"xorl %%eax, %%eax\n\t"
		"movb $0xFF, %%al\n\t"
		"subb %%ch, %%al\n\t"
		"mull %%esi\n\t"
		"addb %%ch, %%ah\n\t"
		"jnc 2f\n\t"
		"movb $0xFF, %%ah\n\t"
	"2:"
		"movb %%ah, %%bh\n\t"
		"bswap %%ebx\n\t"
		"bswap %%ecx\n\t"

		"xorl %%eax, %%eax\n\t"
		"movb $0xFF, %%al\n\t"
		"subb %%ch, %%al\n\t"
		"mull %%esi\n\t"
		"addb %%ch, %%ah\n\t"
		"jnc 3f\n\t"
		"movb $0xFF, %%ah\n\t"
	"3:"
		"movb %%ah, %%bh\n\t"
		"bswap %%ebx\n\t"
	: "=b" (color)
	: "S" (W) , "c" (color)
	: "%eax" , "%edx" , "cc"
	);
/* Intel Assembler */
/*
	__asm {
		mov esi, W
		xor ebx, ebx
		mov ecx, color
		xor eax, eax
		mov al, 0xFF
		sub al, cl
		mul esi
		add ah, cl
		jnc clmul01_done
		mov ah, 0xFF
	clmul01_done:
		mov bl, ah

		xor eax, eax
		mov al, 0xFF
		sub al, ch
		mul esi
		add ah, ch
		jnc clmul02_done
		mov ah, 0xFF
	clmul02_done:
		mov bh, ah
		bswap ebx
		bswap ecx

		xor eax, eax
		mov al, 0xFF
		sub al, ch
		mul esi
		add ah, ch
		jnc clmul03_done
		mov ah, 0xFF
	clmul03_done:
		mov bh, ah
		bswap ebx

		mov color, ebx
	}
*/

	// set Chromatic
/* AT&T Assembler */
	__asm__ __volatile__ (
		"xorl %%ebx, %%ebx\n\t"
		"xorl %%eax, %%eax\n\t"
		"movb %%cl, %%al\n\t"
		"mull %%esi\n\t"
		"movb %%ah, %%bl\n\t"

		"xorl %%eax, %%eax\n\t"
		"movb %%ch, %%al\n\t"
		"mull %%esi\n\t"
		"movb %%ah, %%bh\n\t"
		"bswap %%ebx\n\t"
		"bswap %%ecx\n\t"

		"xorl %%eax, %%eax\n\t"
		"movb %%ch, %%al\n\t"
		"mul %%esi\n\t"
		"movb %%ah, %%bh\n\t"
		"bswap %%ebx\n\t"
	: "=b" (color)
	: "S" (C) , "c" (color)
	: "%eax" , "%edx" , "cc"
	);
/* Intel Assembler */
/*
	__asm {
		mov esi, C
		xor ebx, ebx
		mov ecx, color
		xor eax, eax
		mov al, cl
		mul esi
		mov bl, ah

		xor eax, eax
		mov al, ch
		mul esi
		mov bh, ah
		bswap ebx
		bswap ecx

		xor eax, eax
		mov al, ch
		mul esi
		mov bh, ah
		bswap ebx

		mov color, ebx
	}
*/

	return color;
}

BYTE getWhitness(DWORD Color) { // 0..255
	BYTE W;
/* AT&T Assembler */
    uint32_t c0;
	__asm__ __volatile__ (
		// Sort Max..Min
		"cmpb %%cl, %%ch\n\t"
		"jae 1f\n\t"
		"xchgb %%cl, %%ch\n\t"
	"1:"
		"rorl $0x08, %%ecx\n\t"
		"cmpb %%cl, %%ch\n\t"
		"jae 2f\n\t"
		"xchgb %%cl, %%ch\n\t"
	"2:"
		"xorl %%edx, %%edx\n\t"
		"movb %%ch, %%dl\n\t"
		"movl %%edx, %%edi\n\t"
		"roll $0x08, %%ecx\n\t"
		"cmpb %%cl, %%ch\n\t"
		"jae 3f\n\t"
		"xchgb %%cl, %%ch\n\t"
	"3:"
		"movb %%ch, %%dl\n\t"
		"movl %%edx, %%esi\n\t"
		"andl $0xFF, %%ecx\n\t"
		// edi = max, esi = mid, ecx = min

		"xorl %%eax, %%eax\n\t"
		"orl %%edi, %%edi\n\t"
		"jz 4f\n\t"
		"movl $0xFF, %%ebx\n\t"
		"movl %%ecx, %%eax\n\t"
		"mull %%ebx\n\t"
		"divl %%edi\n\t"
		"cmpl $0xFF, %%eax\n\t"
		"jna 4f\n\t"
		"movl $0xFF, %%eax\n\t"
	"4:"
	: "=&c" (c0), "=a" (W)
	: "0" (Color)
	: "%edx" , "%edi" , "%esi" , "%ebx" , "cc"
	);
/* Intel Assembler */
/*
	__asm {
		// Sort Max..Min
		mov ecx, Color
		cmp ch, cl
		jae clsm_gt1
		xchg ch, cl
clsm_gt1:
		ror ecx, 8
		cmp ch, cl
		jae clsm_gt2
		xchg ch, cl
clsm_gt2:
		xor edx, edx
		mov dl, ch
		mov edi, edx
		rol ecx, 8
		cmp ch, cl
		jae clsm_gt3
		xchg ch, cl
clsm_gt3:
		mov dl, ch
		mov esi, edx
		and ecx, 0xFF
		// edi = max, esi = mid, ecx = min

		xor eax, eax
		or edi, edi
		jz cl_zero
		mov ebx, 0xFF
		mov eax, ecx
		mul ebx
		div edi
		cmp eax, 0xFF
		jna cl_zero
		mov eax, 0xFF
	cl_zero:
		mov W, al
	}
*/
	return W;
}

BYTE getChromatic(DWORD Color) { // 0..255
	BYTE C;
/* AT&T Assembler */
	__asm__ __volatile__ (
		"xorl %%eax, %%eax\n\t"
		"movb %%cl, %%al\n\t"
		"cmpb %%ch, %%al\n\t"
		"ja 1f\n\t"
		"movb %%ch, %%al\n\t"
	"1:"
		"bswap %%ecx\n\t"
		"cmpb %%ch, %%al\n\t"
		"ja 2f\n\t"
		"movb %%ch, %%al\n\t"
	"2:"
	: "=a" (C)
	: "c" (Color)
	: "cc"
	);
/* Intel Assembler */
/*
	__asm {
		xor eax, eax
		mov ecx, Color
		mov al, cl
		cmp al, ch
		ja cl_gt01
		mov al, ch
	cl_gt01:
		bswap ecx
		cmp al, chunsigned long clLab2RGB(unsigned long color) {

		ja cl_gt02
		mov al, ch
	cl_gt02:
		mov C, al
	}
*/
	return C;
}

BYTE getLuminance(DWORD Color) { // 0..255     0x00RRGGBB
	BYTE C;
	//	0.299*r + 0.587*g + 0.114*b
/* AT&T Assembler */
	__asm__ __volatile__ (
		"xorl %%esi, %%esi\n\t"

		// B
		"xorl %%eax, %%eax\n\t"
		"movb %%cl, %%al\n\t"
		"movl $0x00001D2F, %%ebx\n\t"
		"mull %%ebx\n\t"
		"addl %%eax, %%esi\n\t"

		// G
		"xorl %%eax, %%eax\n\t"
		"movb %%ch, %%al\n\t"
		"movl $0x00009645, %%ebx\n\t"
		"mull %%ebx\n\t"
		"addl %%eax, %%esi\n\t"

		"bswap %%ecx\n\t"
		// R
		"xorl %%eax, %%eax\n\t"
		"movb %%ch, %%al\n\t"
		"movl $0x00004C8B, %%ebx\n\t"
		"mull %%ebx\n\t"
		"addl %%eax, %%esi\n\t"

		"movl $0x00007FFF, %%eax\n\t"
		"addl %%esi, %%eax\n\t"
		"bswap %%eax\n\t"
		"xchgb %%ah, %%al\n\t"
	: "=a" (C)
	: "c" (Color)
	: "%ebx" , "%esi" , "cc"
	);
/* Intel Assembler */
/*
	__asm {
		xor esi, esi
		mov ecx, Color

		// B
		xor eax, eax
		mov al, cl
		mov ebx, 0x00001D2F
		mul ebx
		add esi, eax

		// G
		xor eax, eax
		mov al, ch
		mov ebx, 0x00009645
		mul ebx
		add esi, eax

		bswap ecx
		// R
		xor eax, eax
		mov al, ch
		mov ebx, 0x00004C8B
		mul ebx
		add esi, eax

		mov eax, 0x00007FFF
		add eax, esi
		bswap eax

		mov C, ah
	}
*/
	return C;
}

unsigned long clRGB2CMY(unsigned long color) { // ARGB
	return color ^ 0x00FFFFFF;
}

unsigned long clCMY2RGB(unsigned long color) {
	return color ^ 0x00FFFFFF;
}

unsigned long clRGB2HLS(unsigned long color) { // ALHS
	unsigned long a = color & 0xFF000000;
	int r = (color >> 16) & 0xFF;
	int g = (color >> 8) & 0xFF;
	int b = color & 0xFF;
	int m0 = r; int m1 = g; int m2 = b;
	if (m0 > m1) { int temp = m0; m0 = m1; m1 = temp; }
	if (m0 > m2) { int temp = m0; m0 = m2; m2 = temp; }
	if (m1 > m2) { int temp = m1; m1 = m2; m2 = temp; }
	int L = (m2+m0)>>1;
	if (L == 0) return a;
	if (L == 255) return a | 0x00FF0000;
	int d = (m2-m0)>>1;
	int S;
	if (L <= 128) S = d * 0xFF / L; else S = d * 0xFF / (L ^ 0xFF);
	if (S == 0) return a | ( L << 16 );
	int n;
	if (m2 == m0) return a | ( L << 16 ) | ( S );
	if (r == m2) {
		n = ( b == m0 ) ? 0 : 5;
	} else if (g == m2) {
		n = ( b == m0 ) ? 1 : 2;
	} else {
		n = ( r == m0 ) ? 3 : 4;
	}
	int F;
	if ((n&1) == 0) F = m1 - m0;
	else F = m2 - m1;
	F = (F * 85) / (m2 - m0);
	return a | ( L << 16) | (((n*85+F)>>1)<<8) | ( S );
}

unsigned long clHLS2RGB(unsigned long color) {
	unsigned long a = color & 0xFF000000;
	int L = (color >> 16) & 0xFF;
	int H = (color >> 8) & 0xFF;
	int S = color & 0xFF;
	if (L == 255) return a | 0x00FFFFFF;
	if (L == 0) return a;
	int d;
	if (L < 128) d = S * L / 255;
	else d = S * ( L ^ 0xFF ) / 255;
	if (d == 0) return a | ( L << 16 ) | ( L << 8 ) | ( L );
	int m0 = L - d;
	int m2 = L + d;
	int n = (H << 3) / 341;
	int F = ((H << 3) - n*341);
	int mu,md;
	if ( F > 0 ) {
		mu = m0 + ( ( ( m2 - m0 ) * F ) / 341 );
		md = m2 - ( ( ( m2 - m0 ) * F ) / 341 );
	} else {
		mu = m0; md = m2;
	}
	switch (n) {
		case 0: return a | ( m2 << 16 ) | ( mu << 8 ) | ( m0 );
		case 1: return a | ( md << 16 ) | ( m2 << 8 ) | ( m0 );
		case 2: return a | ( m0 << 16 ) | ( m2 << 8 ) | ( mu );
		case 3: return a | ( m0 << 16 ) | ( md << 8 ) | ( m2 );
		case 4: return a | ( mu << 16 ) | ( m0 << 8 ) | ( m2 );
	}
	return a | ( m2 << 16 ) | ( m0 << 8 ) | ( md );
}

unsigned long clRGB2HSI(unsigned long color) {
	cRGB rgb; rgb.setColor(color);
	cHSI hsi = RGB2HSI(rgb);
	return (color & 0xFF000000) | (int(hsi.S*255) << 16) | (int(hsi.H*255/360) << 8) | (int(hsi.I*255));
}

unsigned long clHSI2RGB(unsigned long color) {
	cHSI hsi;
	hsi.S = double((color >> 16) & 0xFF)/255;
	hsi.H = double((color >> 8) & 0xFF)*360/255;
	hsi.I = double(color & 0xFF)/255;
	cRGB rgb = HSI2RGB(hsi);
	return (color & 0xFF000000) | rgb.getColor();
}

unsigned long clRGB2YCbCr(unsigned long color) {
	cRGB rgb; rgb.setColor(color);
	cYCbCr ycc = RGB2YCbCr(rgb);
	return (color & 0xFF000000) | (int(ycc.Cb*255) << 16) | (int(ycc.Y*255) << 8) | (int(ycc.Cr*255));
}

unsigned long clYCbCr2RGB(unsigned long color) {
	cYCbCr ycc;
	ycc.Cb = double((color >> 16) & 0xFF)/255;
	ycc.Y = double((color >> 8) & 0xFF)/255;
	ycc.Cr = double(color & 0xFF)/255;
	cRGB rgb = YCbCr2RGB(ycc);
	return (color & 0xFF000000) | rgb.getColor();
}

unsigned long clRGB2XYZ(unsigned long color) {
	cRGB rgb; rgb.setColor(color);
	cXYZ xyz = RGB2XYZEBU(rgb);
	return (color & 0xFF000000) | (int(xyz.Y*255) << 16) | (int(xyz.X*255) << 8) | (int(xyz.Z*255));
}

unsigned long clXYZ2RGB(unsigned long color) {
	cXYZ xyz;
	xyz.Y = double((color >> 16) & 0xFF)/255;
	xyz.X = double((color >> 8) & 0xFF)/255;
	xyz.Z = double(color & 0xFF)/255;
	cRGB rgb = XYZEBU2RGB(xyz);
	return (color & 0xFF000000) | rgb.getColor();
}

unsigned long clRGB2Luv(unsigned long color) {
	cRGB rgb; rgb.setColor(color);
	cXYZ xyz = RGB2XYZEBU(rgb);
	cLuv luv = XYZ2Luv(xyz);
	return (color & 0xFF000000) | (int(luv.u+127) << 16) | (int(luv.L*255/100) << 8) | (int(luv.v+127));
}

unsigned long clLuv2RGB(unsigned long color) {
	cLuv luv;
	luv.u = double((color >> 16) & 0xFF) - 127;
	luv.L = double((color >> 8) & 0xFF)*100/255;
	luv.v = double(color & 0xFF) - 127;
	cXYZ xyz = Luv2XYZ(luv);
	cRGB rgb = XYZEBU2RGB(xyz);
	return (color & 0xFF000000) | rgb.getColor();
}

unsigned long clRGB2Lab(unsigned long color) {
	cRGB rgb; rgb.setColor(color);
	cXYZ xyz = RGB2XYZEBU(rgb);
	cLab lab = XYZ2Lab(xyz);
	return (color & 0xFF000000) | (int(lab.a+127) << 16) | (int(lab.L*255/100) << 8) | (int(lab.b+127));
}

unsigned long clLab2RGB(unsigned long color) {
	cLab lab;
	lab.a = double((color >> 16) & 0xFF) - 127;
	lab.L = double((color >> 8) & 0xFF)*100/255;
	lab.b = double(color & 0xFF) - 127;
	cXYZ xyz = Lab2XYZ(lab);
	cRGB rgb = XYZEBU2RGB(xyz);
	return (color & 0xFF000000) | rgb.getColor();
}

unsigned long clDistance(unsigned long color1, unsigned long color2) {
    int b = (color2 & 0xFF)-(color1 & 0xFF);
    int g = ((color2 >> 8) & 0xFF)-((color1 >> 8) & 0xFF);
    int r = ((color2 >> 16) & 0xFF)-((color1 >> 16) & 0xFF);
    return sqrt(b*b+g*g+r*r);
}
