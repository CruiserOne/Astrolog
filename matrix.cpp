/*
** Astrolog (Version 7.30) File: matrix.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2021 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** The main ephemeris databases and calculation routines are from the
** library SWISS EPHEMERIS and are programmed and copyright 1997-2008 by
** Astrodienst AG. The use of that source code is subject to the license for
** Swiss Ephemeris Free Edition, available at http://www.astro.com/swisseph.
** This copyright notice must not be changed or removed by any user of this
** program.
**
** Additional ephemeris databases and formulas are from the calculation
** routines in the program PLACALC and are programmed and Copyright (C)
** 1989,1991,1993 by Astrodienst AG and Alois Treindl (alois@astro.ch). The
** use of that source code is subject to regulations made by Astrodienst
** Zurich, and the code is not in the public domain. This copyright notice
** must not be changed or removed by any user of this program.
**
** The original planetary calculation routines used in this program have
** been copyrighted and the initial core of this program was mostly a
** conversion to C of the routines created by James Neely as listed in
** 'Manual of Computer Programming for Astrologers', by Michael Erlewine,
** available from Matrix Software.
**
** Atlas composed using data from https://www.geonames.org/ licensed under a
** Creative Commons Attribution 4.0 License. Time zone changes composed using
** public domain TZ database: https://data.iana.org/time-zones/tz-link.html
**
** The PostScript code within the core graphics routines are programmed
** and Copyright (C) 1992-1993 by Brian D. Willoughby (brianw@sounds.wa.com).
**
** More formally: This program is free software; you can redistribute it
** and/or modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of the
** License, or (at your option) any later version. This program is
** distributed in the hope that it will be useful and inspiring, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details, a copy of which is in the
** LICENSE.HTM file included with Astrolog, and at http://www.gnu.org
**
** Initial programming 8/28-30/1991.
** X Window graphics initially programmed 10/23-29/1991.
** PostScript graphics initially programmed 11/29-30/1992.
** Last code change made 9/10/2021.
*/

#include "astrolog.h"


#ifdef MATRIX
/*
******************************************************************************
** Object Calculation Tables.
******************************************************************************
*/

CONST int cErrorCount[oPlu-oJup+1] = {11, 5, 4, 4, 4};
CONST int iErrorOffset[oPlu-oJup+1] = {0, 72, 72+51, 72+51+42, 72+51+42+42};

CONST real rErrorData[72+51+42*3] = {
-.001,-.0005,.0045,.0051,581.7,-9.7,-.0005,2510.7,-12.5,-.0026,1313.7,-61.4,
0.0013,2370.79,-24.6,-.0013,3599.3,37.7,-.001,2574.7,31.4,-.00096,6708.2,
-114.5,-.0006,5499.4,-74.97,-.0013,1419,54.2,.0006,6339.3,-109,.0007,4824.5,
-50.9,.0020,-.0134,.0127,-.0023,676.2,.9,.00045,2361.4,174.9,.0015,1427.5,
-188.8,.0006,2110.1,153.6,.0014,3606.8,-57.7,-.0017,2540.2,121.7,-.00099,
6704.8,-22.3,-.0006,5480.2,24.5,.00096,1651.3,-118.3,.0006,6310.8,-4.8,.0007,
4826.6,36.2, // Jupiter error

-.0009,.0037,0,.0134,1238.9,-16.4,-.00426,3040.9,-25.2,.0064,1835.3,36.1,
-.0153,610.8,-44.2,-.0015,2480.5,-69.4,-.0014,.0026,0,.0111,1242.2,78.3,
-.0045,3034.96,62.8,-.0066,1829.2,-51.5,-.0078,640.6,24.2,-.0016,2363.4,
-141.4,.0006,-.0002,0,-.0005,1251.1,43.7,.0005,622.8,13.7,.0003,1824.7,-71.1,
.0001,2997.1,78.2, // Saturn error

-.0021,-.0159,0,.0299,422.3,-17.7,-.0049,3035.1,-31.3,-.0038,945.3,60.1,
-.0023,1227,-4.99,.0134,-.02186,0,.0317,404.3,81.9,-.00495,3037.9,57.3,.004,
993.5,-54.4,-.0018,1249.4,79.2,-.0003,.0005,0,.0005,352.5,-54.99,.0001,3027.5,
54.2,-.0001,1150.3,-88, // Uranus error

0.1832,-.6718,.2726,-.1923,175.7,31.8,.0122,542.1,189.6,.0027,1219.4,178.1,
-.00496,3035.6,-31.3,-.1122,.166,-.0544,-.00496,3035.3,58.7,.0961,177.1,-68.8,
-.0073,630.9,51,-.0025,1236.6,78,.00196,-.0119,.0111,.0001,3049.3,44.2,-.0002,
893.9,48.5,.00007,1416.5,-25.2, // Neptune error

-.0426,.073,-.029,.0371,372,-331.3,-.0049,3049.6,-39.2,-.0108,566.2,318.3,
0.0003,1746.5,-238.3,-.0603,.5002,-.6126,.049,273.97,89.97,-.0049,3030.6,61.3,
0.0027,1075.3,-28.1,-.0007,1402.3,20.3,.0145,-.0928,.1195,.0117,302.6,-77.3,
0.00198,528.1,48.6,-.0002,1000.4,-46.1 // Pluto error
};

OE rgoe[oVes+cUran-2] = {
{358.4758,35999.0498,-.0002,.01675,-.4E-4,0,1,101.2208,1.7192,.00045,0,0,0,0,
0,0}, // Earth/Sun
{102.2794,149472.515,0,.205614,.2E-4,0,.3871,28.7538,.3703,.0001,47.1459,
1.1852,0.0002,7.009,.00186,0}, // Mercury
{212.6032,58517.8039,.0013,.00682,-.5E-4,0,.7233,54.3842,.5082,-.14E-2,
75.7796,0.8999,.4E-3,3.3936,.1E-2,0}, // Venus
{319.5294,19139.8585,.2E-3,.09331,.9E-4,0,1.5237,285.4318,1.0698,.1E-3,
48.7864,0.77099,0,1.8503,-.7E-3,0}, // Mars

{225.4928,3033.6879,0,.04838,-.2E-4,0,5.2029,273.393,1.3383,0,99.4198,1.0583,
0,1.3097,-.52E-2,0}, // Jupiter
{174.2153,1223.50796,0,.05423,-.2E-3,0,9.5525,338.9117,-.3167,0,112.8261,
.8259,0,2.4908,-.0047,0}, // Saturn
{74.1757,427.2742,0,.04682,.00042,0,19.2215,95.6863,2.0508,0,73.5222,.5242,0,
0.7726,.1E-3,0}, // Uranus
{30.13294,240.45516,0,.00913,-.00127,0,30.11375,284.1683,-21.6329,0,130.68415,
1.1005,0,1.7794,-.0098,0}, // Neptune
{229.781,145.1781,0,.24797,.002898,0,39.539,113.5366,.2086,0,108.944,1.3739,0,
17.1514,-.0161,0}, // Pluto

{34.6127752,713.5756219,0,.382270369,-.004694073,0,13.66975144,337.407213,
2.163306646,0,208.1482658,1.247724355,0,6.911179715,.011236955,0}, // Chiron
{108.2925,7820.36556,0,.0794314,0,0,2.7672273,71.0794444,0,0,80.23555556,
1.3960111,0,10.59694444,0,0}, // Ceres
{106.6641667,7806.531667,0,.2347096,0,0,2.7704955,310.166111,0,0,172.497222,
1.39601111,0,34.81416667,0,0}, // Pallas Athena
{267.685,8256.081111,0,.2562318,0,0,2.6689897,245.3752778,0,0,170.137777,
1.396011111,.0003083333,13.01694444,0,0}, // Juno
{138.7733333,9924.931111,0,.0902807,0,0,2.360723,149.6386111,0,0,103.2197222,
1.396011111,.000308333,7.139444444,0,0}, // Vesta

{104.5959,138.5369,0,0,0,0,40.99837, 0,0,0,0,0,0,0,0,0}, // Cupido
{337.4517,101.2176,0,0,0,0,50.667443,0,0,0,0,0,0,0,0,0}, // Hades
{104.0904,80.4057, 0,0,0,0,59.214362,0,0,0,0,0,0,0,0,0}, // Zeus
{17.7346, 70.3863, 0,0,0,0,64.816896,0,0,0,0,0,0,0,0,0}, // Kronos
{138.0354,62.5,    0,0,0,0,70.361652,0,0,0,0,0,0,0,0,0}, // Apollon
{-8.678,  58.3468, 0,0,0,0,73.736476,0,0,0,0,0,0,0,0,0}, // Admetos
{55.9826, 54.2986, 0,0,0,0,77.445895,0,0,0,0,0,0,0,0,0}, // Vulkanus
{165.3595,48.6486, 0,0,0,0,83.493733,0,0,0,0,0,0,0,0,0}  // Poseidon
};

CONST real rStarBrightMatrix[cStar+1] = {0,
  0.46, 2.02, 5.24, 5.09, 0.85, 0.08, 0.12, 1.64, 1.65, 1.70,
  0.50, 1.90, 1.98,-0.72, 1.93,-1.46, 1.50, 1.84, 1.59, 0.38,
  1.14, 1.78, 1.86, 1.68, 1.98, 1.35, 1.79, 1.58, 1.63, 1.25,
  1.77, 0.98, 1.86, 0.61,-0.04,-0.01, 0.96, 1.63, 1.87, 1.85,
  0.03, 0.77, 1.94, 1.25, 1.74, 1.16, 4.61};

CONST real rStarData[cStar*6] = {
 1,37,42.9,-57,14,12,   2,31,50.5, 89,15,51,   3,17,46.1,-62,34,32,
 3,49,11.1, 24, 8,12,   4,35,55.2, 16,30,33,   5,16,41.3, 45,59,53,
 5,14,32.2, -8,12, 6,   5,25, 7.8,  6,20,59,   5,26,17.5, 28,36,27,
 5,36,12.7, -1,12, 7,   5,55,10.3,  7,24,25,   5,59,31.7, 44,56,51,
 6,22,41.9,-17,57,22,   6,23,57.2,-52,41,44,   6,37,42.7, 16,23,57,
 6,45, 8.9,-16,42,58,   6,58,37.5,-28,58,20,   7, 8,23.4,-26,23,35,
 7,34,35.9, 31,53,18,   7,39,18.1,  5,13,30,   7,45,18.9, 28, 1,34,
 8, 9,31.9,-47,20,12,   8,22,30.8,-59,30,34,   9,13,12.1,-69,43, 2,
 9,27,35.2, -8,39,31,  10, 8,22.3, 11,58, 2,  11, 3,43.6, 61,45, 3,
12,26,35.9,-63, 5,56,  12,31, 9.9,-57, 6,47,  12,47,43.3,-59,41,19,
12,54, 1.7, 55,57,35,  13,25,11.5,-11, 9,41,  13,47,32.3, 49,18,48,
14, 3,49.4,-60,22,22,  14,15,39.6, 19,10,57,  14,39,36.2,-60,50, 7,
16,29,24.4,-26,25,55,  17,33,36.4,-37, 6,13,  17,37,19.0,-42,59,52,
18,24,10.3,-34,23, 5,  18,36,56.2, 38,47, 1,  19,50,46.9,  8,52, 6,
20,25,38.8,-56,44, 7,  20,41,25.8, 45,16,49,  22, 8,13.9,-46,57,40,
22,57,39.0,-29,37,20,   0,42, 7.0, 41,16, 0};


/*
******************************************************************************
** Assorted Calculations.
******************************************************************************
*/

// Given a month, day, and year, convert it into a single Julian day value,
// i.e. the number of days passed since a fixed reference date.

long MatrixMdyToJulian(int mon, int day, int yea)
{
  long im, j;

  im = 12*(yea + 4800) + mon - 3;
  j = (2*(im%12) + 7 + 365*im)/12;
  j += day + im/48 - 32083;
  if (j > 2299171)                // Take care of dates in Gregorian calendar.
    j += im/4800 - im/1200 + 38;
  return j;
}


// Take a Julian day value, and convert it back into the corresponding month,
// day, and year.

void MatrixJulianToMdy(real JD, int *mon, int *day, int *yea)
{
  long l, n, it, jt, k, ik;

  l = (long)RFloor(JD + rRound) + 68569;
  n = Dvd(4*l, 146097);
  l -= Dvd(146097*n + 3, 4);
  it = Dvd(4000*(l+1), 1461001);
  l -= Dvd(1461*it, 4) - 31;
  jt = Dvd(80*l, 2447);
  k = l - Dvd(2447*jt, 80);
  l = Dvd(jt, 11);
  jt += 2 - 12*l;
  ik = 100*(n-49) + it + l;
  *mon = jt; *day = k; *yea = ik;
}


// This is a subprocedure of CastChart(). Once we have the chart parameters
// calculate a few important things related to the date, i.e. the Greenwich
// (UTC) time, the Julian day and fractional part of the day, the offset to
// the sidereal, and a couple of other things.

real ProcessInput(void)
{
  real Ln, Off;

  // Compute angle that the ecliptic is inclined to the Celestial Equator.
  is.OB = 23.452294 - 0.0130125*is.T;

  Ln = Mod((933060-6962911*is.T+7.5*is.T*is.T)/3600.0);    // Mean lunar node
  Off = (259205536.0*is.T+2013816.0)/3600.0;               // Mean Sun
  Off = 17.23*RSin(RFromD(Ln)) + 1.27*RSin(RFromD(Off)) -
    (5025.64+1.11*is.T)*is.T;
  Off = (Off-84038.27)/3600.0;
  is.rSid = (us.fSidereal ? Off : 0.0) + us.rZodiacOffset;
  return Off;
}


// Another modulus function, this time for the range of 0 to 2 Pi.

real ModRad(real r)
{
  while (r >= rPi2)    // Assume value is only slightly out of range, so
    r -= rPi2;         // test and never do any complicated math.
  while (r < 0.0)
    r += rPi2;
  return r;
}


// Convert polar to rectangular coordinates.

void PolToRec(real a, real r, real *x, real *y)
{
  if (a == 0.0)
    a = rSmall;
  *x = r*RCos(a);
  *y = r*RSin(a);
}


// Convert rectangular to spherical coordinates.

real RecToSph(real B, real L, real O)
{
  real R, Q, G, X, Y, A;

  A = B; R = 1.0;
  PolToRec(A, R, &X, &Y);
  Q = Y; R = X; A = L;
  PolToRec(A, R, &X, &Y);
  G = X; X = Y; Y = Q;
  RecToPol(X, Y, &A, &R);
  A += RFromD(O);
  PolToRec(A, R, &X, &Y);
  Q = RAsin(Y);
  Y = X; X = G;
  RecToPol(X, Y, &A, &R);
  if (A < 0.0)
    A += 2*rPi;
  G = A;
  return G;  // We only ever care about and return one of the coordinates.
}


// This is another subprocedure of CastChart(). Calculate a few variables
// corresponding to the chart parameters that are used later on. The
// astrological vertex is also calculated here.

void ComputeVariables(real *vtx)
{
  real B, L, G, tim = TT;

  if (us.fProgress && us.nProgress != ptSolarArc) {
    tim = JulianDayFromTime(is.T) + 0.5;
    tim = RFract(tim)*24.0;
  }
  is.RA = Mod((6.6460656 + 2400.0513*is.T + 2.58E-5*is.T*is.T +
    tim)*15.0 - OO);
  L = RFromD(is.RA) + rPi; B = rPiHalf - RAbs(RFromD(AA));
  if (AA < 0.0)
    B = -B;
  G = RecToSph(B, L, -is.OB);
  *vtx = Mod(is.rSid + DFromR(G+rPiHalf));    // Vertex
}


/*
******************************************************************************
** House Cusp Calculations.
******************************************************************************
*/

// The following three functions calculate the Midheaven, Ascendant, and East
// Point of the chart in question, based on time and location. The first two
// are also used in some of the house cusp calculation routines as a quick way
// to get the 10th and 1st cusps. The East Point object is technically defined
// as the Ascendant's position at zero latitude.

real CuspMidheaven(void)
{
  real MC;

  MC = RAtn(RTanD(is.RA)/RCosD(is.OB));
  if (MC < 0.0)
    MC += rPi;
  if (RFromD(is.RA) > rPi)
    MC += rPi;
  return Mod(DFromR(MC)+is.rSid);
}

real CuspAscendant(void)
{
  real Asc;

  Asc = RAngle(-RSinD(is.RA)*RCosD(is.OB) - RTanD(AA)*RSinD(is.OB),
    RCosD(is.RA));
  return Mod(DFromR(Asc)+is.rSid);
}

real CuspEastPoint(void)
{
  real EP;

  EP = RAngle(-RSinD(is.RA)*RCosD(is.OB), RCosD(is.RA));
  return Mod(DFromR(EP)+is.rSid);
}


// These are various different algorithms for calculating the house cusps.

real CuspPlacidus(real deg, real FF, flag fNeg)
{
  real LO, R1, XS, X;
  int i;

  R1 = RFromD(is.RA + deg);
  X = fNeg ? 1.0 : -1.0;
  // Looping 10 times is arbitrary, but it's what other programs do.
  for (i = 1; i <= 10; i++) {

    // This formula works except at 0 latitude (AA == 0.0).
    XS = X*RSin(R1)*RTanD(is.OB)*RTanD(AA == 0.0 ? 0.0001 : AA);
    XS = RAcos(XS);
    if (XS < 0.0)
      XS += rPi;
    R1 = RFromD(is.RA) + (fNeg ? rPi-(XS/FF) : (XS/FF));
  }
  LO = RAtn(RTan(R1)/RCosD(is.OB));
  if (LO < 0.0)
    LO += rPi;
  if (RSin(R1) < 0.0)
    LO += rPi;
  return DFromR(LO);
}

void HousePlacidus(void)
{
  int i;

  chouse[1] = Mod(is.Asc-is.rSid);
  chouse[4] = Mod(is.MC+rDegHalf-is.rSid);
  chouse[5] = CuspPlacidus(30.0, 3.0, fFalse) + rDegHalf;
  chouse[6] = CuspPlacidus(60.0, 1.5, fFalse) + rDegHalf;
  chouse[2] = CuspPlacidus(120.0, 1.5, fTrue);
  chouse[3] = CuspPlacidus(150.0, 3.0, fTrue);
  for (i = 1; i <= cSign; i++) {
    if (i <= 6)
      chouse[i] = Mod(chouse[i]+is.rSid);
    else
      chouse[i] = Mod(chouse[i-6]+rDegHalf);
  }
}

void HouseKoch(void)
{
  real A1, A2, A3, KN, D, X;
  int i;

  A1 = RSinD(is.RA)*RTanD(AA)*RTanD(is.OB);
  A1 = RAsin(A1);
  for (i = 1; i <= cSign; i++) {
    D = Mod(60.0+30.0*(real)i);
    A2 = D/rDegQuad-1.0; KN = 1.0;
    if (D >= rDegHalf) {
      KN = -1.0;
      A2 = D/rDegQuad-3.0;
    }
    A3 = RFromD(Mod(is.RA+D+A2*DFromR(A1)));
    X = RAngle(RCos(A3)*RCosD(is.OB)-KN*RTanD(AA)*RSinD(is.OB), RSin(A3));
    chouse[i] = Mod(DFromR(X)+is.rSid);
  }
}

void HouseCampanus(void)
{
  real KO, DN, X;
  int i;

  for (i = 1; i <= cSign; i++) {
    KO = RFromD(60.000001+30.0*(real)i);
    DN = RAtn(RTan(KO)*RCosD(AA));
    if (DN < 0.0)
      DN += rPi;
    if (RSin(KO) < 0.0)
      DN += rPi;
    X = RAngle(RCos(RFromD(is.RA)+DN)*RCosD(is.OB) -
      RSin(DN)*RTanD(AA)*RSinD(is.OB), RSin(RFromD(is.RA)+DN));
    chouse[i] = Mod(DFromR(X)+is.rSid);
  }
}

void HouseMeridian(void)
{
  real D, X;
  int i;

  for (i = 1; i <= cSign; i++) {
    D = 60.0 + 30.0*(real)i;
    X = RAngleD(RCosD(is.RA + D)*RCosD(is.OB), RSinD(is.RA + D));
    chouse[i] = Mod(X + is.rSid);
  }
}

void HouseRegiomontanus(void)
{
  real D, X;
  int i;

  for (i = 1; i <= cSign; i++) {
    D = 60.0 + 30.0*(real)i;
    X = RAngleD(RCosD(is.RA + D)*RCosD(is.OB) -
      RSinD(D)*RTanD(AA)*RSinD(is.OB), RSinD(is.RA + D));
    chouse[i] = Mod(X + is.rSid);
  }
}

void HouseMorinus(void)
{
  real D, X;
  int i;

  for (i = 1; i <= cSign; i++) {
    D = 60.0 + 30.0*(real)i;
    X = RAngleD(RCosD(is.RA + D), RSinD(is.RA + D)*RCosD(is.OB));
    chouse[i] = Mod(X + is.rSid);
  }
}

real CuspTopocentric(real deg)
{
  real OA, X, LO;

  OA = ModRad(RFromD(is.RA + deg));
  X = RAtn(RTan(AA)/RCos(OA));
  LO = RAtn(RCos(X)*RTan(OA)/RCos(X+RFromD(is.OB)));
  if (LO < 0.0)
    LO += rPi;
  if (RSin(OA) < 0.0)
    LO += rPi;
  return LO;
}

void HouseTopocentric(void)
{
  real TL, P1, P2, LT;
  int i;

  chouse[4] = ModRad(RFromD(is.MC+rDegHalf-is.rSid));
  TL = RTanD(AA); P1 = RAtn(TL/3.0); P2 = RAtn(TL/1.5); LT = AA;
  AA = P1;         chouse[5] = CuspTopocentric(30.0) + rPi;
  AA = P2;         chouse[6] = CuspTopocentric(60.0) + rPi;
  AA = RFromD(LT); chouse[1] = CuspTopocentric(90.0);
  AA = P2;         chouse[2] = CuspTopocentric(120.0);
  AA = P1;         chouse[3] = CuspTopocentric(150.0);
  AA = LT;
  for (i = 1; i <= 6; i++) {
    chouse[i] = Mod(DFromR(chouse[i])+is.rSid);
    chouse[i+6] = Mod(chouse[i]+rDegHalf);
  }
}


/*
******************************************************************************
** Planetary Position Calculations.
******************************************************************************
*/

// Given three values, return them combined as the coefficients of a
// quadratic equation as a function of the chart time.

real ReadThree(real r0, real r1, real r2)
{
  return RFromD(r0 + r1*is.T + r2*is.T*is.T);
}


// Another coordinate transformation. This is used by the ComputePlanets()
// procedure to rotate rectangular coordinates by a certain amount.

void RecToSph2(real AP, real AN, real _IN, real *X, real *Y, real *G)
{
  real R, D, A;

  RecToPol(*X, *Y, &A, &R); A += AP; PolToRec(A, R, X, Y);
  D = *X; *X = *Y; *Y = 0.0; RecToPol(*X, *Y, &A, &R);
  A += _IN; PolToRec(A, R, X, Y);
  *G = *Y; *Y = *X; *X = D; RecToPol(*X, *Y, &A, &R); A += AN;
  if (A < 0.0)
    A += 2.0*rPi;
  PolToRec(A, R, X, Y);
}


// Calculate some harmonic delta error correction factors to add onto the
// coordinates of Jupiter through Pluto, for better accuracy.

void ErrorCorrect(int ind, real *x, real *y, real *z)
{
  real U, V, W, A, S0, T0[4], *pr;
  int IK, IJ, irError;

  irError = cErrorCount[ind-oJup];
  pr = (real *)&rErrorData[iErrorOffset[ind-oJup]];
  for (IK = 1; IK <= 3; IK++) {
    if (ind == oJup && IK == 3) {
      T0[3] = 0.0;
      break;
    }
    if (IK == 3)
      irError--;
    S0 = ReadThree(pr[0], pr[1], pr[2]); pr += 3;
    A = 0.0;
    for (IJ = 1; IJ <= irError; IJ++) {
      U = *pr++; V = *pr++; W = *pr++;
      A += RFromD(U)*RCos((V*is.T+W)*rPi/rDegHalf);
    }
    T0[IK] = DFromR(S0+A);
  }
  *x += T0[2]; *y += T0[1]; *z += T0[3];
}


// This is the (classic) heart of the whole program of Astrolog. Calculate
// the position of each body that orbits the Sun. A heliocentric chart is
// most natural. Extra calculation is needed to have other central bodies.

void ComputePlanets(void)
{
  real helioret[uranHi+1],
    heliox[uranHi+1], helioy[uranHi+1], helioz[uranHi+1];
  real aber = 0.0, AU, E, EA, E1, M, XW, YW, AP, AN, _IN, X, Y, G, XS, YS, ZS;
  int ind = oSun, i;
  OE *poe;

  for (ind = oSun; ind <= (us.fUranian ? uranHi : cPlanet);
    ind += (ind == oSun ? 2 : (ind == cPlanet ? uranLo+1-cPlanet : 1))) {
    if (ignore[ind] && ind > oSun && ind != us.objCenter)
      continue;
    poe = &rgoe[IoeFromObj(ind)];

    EA = M = ModRad(ReadThree(poe->ma0, poe->ma1, poe->ma2));
    E = DFromR(ReadThree(poe->ec0, poe->ec1, poe->ec2));
    for (i = 1; i <= 5; i++)
      EA = M+E*RSin(EA);            // Solve Kepler's equation
    AU = poe->sma;                  // Semi-major axis
    E1 = 0.01720209/(pow(AU, 1.5)*
      (1.0-E*RCos(EA)));                     // Begin velocity coordinates
    XW = -AU*E1*RSin(EA);                    // Perifocal coordinates
    YW = AU*E1*pow(1.0-E*E,0.5)*RCos(EA);
    AP = ReadThree(poe->ap0, poe->ap1, poe->ap2);
    AN = ReadThree(poe->an0, poe->an1, poe->an2);
    _IN = ReadThree(poe->in0, poe->in1, poe->in2); // Calculate inclination
    X = XW; Y = YW;
    RecToSph2(AP, AN, _IN, &X, &Y, &G);  // Rotate velocity coords
    heliox[ind] = X; helioy[ind] = Y;
    helioz[ind] = G;                     // Helio ecliptic rectangular
    X = AU*(RCos(EA)-E);                 // Perifocal coordinates for
    Y = AU*RSin(EA)*pow(1.0-E*E,0.5);    // rectangular position coordinates
    RecToSph2(AP, AN, _IN, &X, &Y, &G);  // Rotate for rectangular
    XS = X; YS = Y; ZS = G;              // position coordinates
    if (FBetween(ind, oJup, oPlu))
      ErrorCorrect(ind, &XS, &YS, &ZS);
    ret[ind] = DFromR((XS*helioy[ind]-YS*heliox[ind]) /
      (XS*XS+YS*YS));  // Helio daily motion
    space[ind].x = XS; space[ind].y = YS; space[ind].z = ZS;
    ProcessPlanet(ind, 0.0);
  }

  space[oEar] = space[oSun];
  planet[oEar] = planet[oSun]; planetalt[oEar] = planetalt[oSun];
  ret[oEar] = ret[oSun];
  heliox[oEar] = heliox[oSun]; helioy[oEar] = helioy[oSun];
  helioret[oEar] = helioret[oSun] = RFromD(1.0);
  space[oSun].x = space[oSun].y = space[oSun].z =
    planet[oSun] = planetalt[oSun] = heliox[oSun] = helioy[oSun] = 0.0;
  if (us.objCenter == oSun) {
    // Use relative velocity if -v0 is in effect.
    if (us.fVelocity)
      for (i = 0; i <= uranHi; i++)
        ret[i] = 1.0;
    return;
  }

  // A second loop is needed for geocentric charts or central bodies other
  // than the Sun. For example, we can't find the position of Mercury in
  // relation to Pluto until we know the position of Pluto in relation to
  // the Sun, and since Mercury is calculated first, another pass needed.

  ind = us.objCenter != oMoo ? us.objCenter : oEar;
  for (i = 0; i <= uranHi; i++) {
    helioret[i] = ret[i];
    if (i != oMoo && i != ind) {
      space[i].x -= space[ind].x;
      space[i].y -= space[ind].y;
      space[i].z -= space[ind].z;
    }
  }
  for (i = oEar; i <= (us.fUranian ? uranHi : cPlanet);
    i += (i == oSun ? 2 : (i == cPlanet ? uranLo+1-cPlanet : 1))) {
    if ((ignore[i] && i > oSun) || i == ind)
      continue;
    XS = space[i].x; YS = space[i].y; ZS = space[i].z;
    ret[i] = DFromR((XS*(helioy[i]-helioy[ind])-YS*(heliox[i]-heliox[ind])) /
      (XS*XS + YS*YS));
    if (ind == oEar && !us.fTruePos)
      aber = 0.0057756 * RLength3(XS, YS, ZS) * ret[i];  // Aberration
    ProcessPlanet(i, aber);
    // Use relative velocity if -v0 is in effect.
    if (us.fVelocity)
      ret[i] = ret[i]/helioret[i];
  }
  space[ind].x = space[ind].y = space[ind].z = 0.0;
}


/*
******************************************************************************
** Lunar Position Calculations
******************************************************************************
*/

// Calculate the position and declination of the Moon, and the Moon's North
// Node. This has to be done separately from the other planets, because they
// all orbit the Sun, while the Moon orbits the Earth.

void ComputeLunar(real *moonlo, real *moonla, real *nodelo, real *nodela)
{
  real LL, G, N, G1, D, L, ML, L1, MB, T1, Y, M = 3600.0, T2;

  T2 = is.T*is.T;
  LL = 973563.0+1732564379.0*is.T-4.0*T2;  // Compute mean lunar longitude
  G = 1012395.0+6189.0*is.T;               // Sun's mean longitude of perigee
  N = 933060.0-6962911.0*is.T+7.5*T2;      // Compute mean lunar node
  G1 = 1203586.0+14648523.0*is.T-37.0*T2;  // Mean longitude of lunar perigee
  D = 1262655.0+1602961611.0*is.T-5.0*T2;  // Mean elongation of Moo from Sun
  L = (LL-G1)/M; L1 = ((LL-D)-G)/M;        // Some auxiliary angles
  T1 = (LL-N)/M; D = D/M; Y = 2.0*D;

  // Compute Moon's perturbations.

  ML = 22639.6*RSinD(L) - 4586.4*RSinD(L-Y) + 2369.9*RSinD(Y) +
    769.0*RSinD(2.0*L) - 669.0*RSinD(L1) - 411.6*RSinD(2.0*T1) -
    212.0*RSinD(2.0*L-Y) - 206.0*RSinD(L+L1-Y);
  ML += 192.0*RSinD(L+Y) - 165.0*RSinD(L1-Y) + 148.0*RSinD(L-L1) -
    125.0*RSinD(D) - 110.0*RSinD(L+L1) - 55.0*RSinD(2.0*T1-Y) -
    45.0*RSinD(L+2.0*T1) + 40.0*RSinD(L-2.0*T1);

  *moonlo = G = Mod((LL+ML)/M+is.rSid);    // Lunar longitude

  // Compute lunar latitude.

  MB = 18461.5*RSinD(T1) + 1010.0*RSinD(L+T1) - 999.0*RSinD(T1-L) -
    624.0*RSinD(T1-Y) + 199.0*RSinD(T1+Y-L) - 167.0*RSinD(L+T1-Y);
  MB += 117.0*RSinD(T1+Y) + 62.0*RSinD(2.0*L+T1) -
    33.0*RSinD(T1-Y-L) - 32.0*RSinD(T1-2.0*L) - 30.0*RSinD(L1+T1-Y);
  *moonla = MB =
    RSgn(MB)*((RAbs(MB)/M)/rDegMax-RFloor((RAbs(MB)/M)/rDegMax))*rDegMax;

  // Compute position of the North Lunar Node, either True or Mean.

  if (us.fTrueNode)
    N = N+5392.0*RSinD(2.0*T1-Y)-541.0*RSinD(L1)-442.0*RSinD(Y)+
      423.0*RSinD(2.0*T1)-291.0*RSinD(2.0*L-2.0*T1);
  *nodelo = Mod(N/M+is.rSid);
  *nodela = 0.0;
}
#endif /* MATRIX */

/* matrix.cpp */
