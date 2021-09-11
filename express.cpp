/*
** Astrolog (Version 7.30) File: express.cpp
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


#ifdef EXPRESS
/*
******************************************************************************
** Action Tables
******************************************************************************
*/

#define cfunA 342
#ifdef GRAPH
#define cfunX 36
#else
#define cfunX 0
#endif
#ifdef WIN
#define cfunW 9
#else
#define cfunW 0
#endif
#define cfun (cfunA + cfunX + cfunW)

#define P2(n1, n2)             (((n2) << 2) | (n1))
#define P3(n1, n2, n3)         (((n3) << 4) | P2(n1, n2))
#define P4(n1, n2, n3, n4)     (((n4) << 6) | P3(n1, n2, n3))
#define P5(n1, n2, n3, n4, n5) (((n5) << 8) | P4(n1, n2, n3, n4))

#define X_ 0
#define I_ 1
#define R_ 2
#define E_ 3
#define I_I    P2(I_, I_)
#define I_II   P3(I_, I_, I_)
#define I_IX   P3(I_, I_, X_)
#define I_III  P4(I_, I_, I_, I_)
#define I_IIR  P4(I_, I_, I_, R_)
#define I_IXX  P4(I_, I_, X_, X_)
#define I_IIIX P5(I_, I_, I_, I_, X_)
#define I_R    P2(I_, R_)
#define I_E    P2(I_, E_)
#define I_EE   P3(I_, E_, E_)
#define I_EEE  P4(I_, E_, E_, E_)
#define R_I    P2(R_, I_)
#define R_II   P3(R_, I_, I_)
#define R_R    P2(R_, R_)
#define R_RR   P3(R_, R_, R_)
#define R_RRRR P5(R_, R_, R_, R_, R_)
#define E_I    P2(E_, I_)
#define E_IE   P3(E_, I_, E_)
#define E_IX   P3(E_, I_, X_)
#define E_IEE  P4(E_, I_, E_, E_)
#define E_IXX  P4(E_, I_, X_, X_)
#define E_E    P2(E_, E_)
#define E_EE   P3(E_, E_, E_)
#define E_XE   P3(E_, X_, E_)
#define E_XXE  P4(E_, X_, X_, E_)
#define E_XXXE P5(E_, X_, X_, X_, E_)

#define EIR(I, R) if (!fRetReal) {n = (I);} else {r = (R);}
#define IIR(I, R) n = !fOptReal ? (I) : (R)
#define SetReal(I, R) if (FBetween(I, 0, cLetter)) \
  { rgparVar[I].r = (R); rgparVar[I].fReal = fTrue; }
#define FRingObj(ring, obj) (FBetween(ring, 0, cRing) && FValidObj(obj))

typedef struct _function {
  int ifun;     // Index of function
  char *szName; // Name of function
  int nParam;   // Number of parameters function takes
  int nType;    // Types of return value and each parameter
} FUN;

typedef struct _parameter {
  int n;      // Integer value (if !fReal)
  real r;     // Real value (if fReal)
  flag fReal; // Whether parameter is real or integer
} PAR;

PAR rgparVar[cLetter+1];       // List of variables
char *rgszExpMacro[cLetter+1]; // List of macros

extern CONST char *PchGetParameter P((CONST char *, PAR *, int, int, flag));
extern void GetParameter P((CONST char *, PAR *));

// Functions

enum _functionindex {
  // General functions (independent of Astrolog and astrology)
  funFalse = 0,
  funTrue,
  funInt,
  funReal,
  funType,
  funAdd,
  funSub,
  funMul,
  funDiv,
  funMod,
  funPow,
  funNeg,
  funInc,
  funDec,
  funEqu,
  funNeq,
  funLt,
  funGt,
  funLte,
  funGte,
  funNot,
  funAnd,
  funOr,
  funXor,
  funInv,
  funShftL,
  funShftR,
  funOdd,
  funAbs,
  funSgn,
  funSgn2,
  funMin,
  funMax,
  funTween,
  funIfOp,
  funSqu,
  funSqr,
  funDist,
  funLn,
  funLog10,
  funSin,
  funCos,
  funTan,
  funAsin,
  funAcos,
  funAtan,
  funAng,
  funSinD,
  funCosD,
  funTanD,
  funAsinD,
  funAcosD,
  funAtanD,
  funAngD,
  funFloor,
  funFract,
  funRnd,
  funRgb,
  funRgbR,
  funRgbG,
  funRgbB,
  funBlend,
  funHue,

  // Specific functions (related to Astrolog and astrology)
  funSigns,
  funObjs,
  funAsps,
  funMon,  funDay,  funYea,  funTim,  funDst,  funZon,  funLon,  funLat,
  funMon1, funDay1, funYea1, funTim1, funDst1, funZon1, funLon1, funLat1,
  funMon2, funDay2, funYea2, funTim2, funDst2, funZon2, funLon2, funLat2,
  funMon3, funDay3, funYea3, funTim3, funDst3, funZon3, funLon3, funLat3,
  funMon4, funDay4, funYea4, funTim4, funDst4, funZon4, funLon4, funLat4,
  funMon5, funDay5, funYea5, funTim5, funDst5, funZon5, funLon5, funLat5,
  funMon6, funDay6, funYea6, funTim6, funDst6, funZon6, funLon6, funLat6,
  funMonN, funDayN, funYeaN, funTimN, funDstN, funZonN, funLonN, funLatN,
  funMonS, funDayS, funYeaS, funTimS, funDstS, funZonS, funLonS, funLatS,
  funMonT, funDayT, funYeaT, funTimT,
  funMonG, funDayG, funYeaG,
  funObjLon,  funObjLat,  funObjDir,  funObjDirY,  funObjDirL,  funObjHou,
  funObjLon1, funObjLat1, funObjDir1, funObjDirY1, funObjDirL1, funObjHou1,
  funObjLon2, funObjLat2, funObjDir2, funObjDirY2, funObjDirL2, funObjHou2,
  funObjLon3, funObjLat3, funObjDir3, funObjDirY3, funObjDirL3, funObjHou3,
  funObjLon4, funObjLat4, funObjDir4, funObjDirY4, funObjDirL4, funObjHou4,
  funObjLon5, funObjLat5, funObjDir5, funObjDirY5, funObjDirL5, funObjHou5,
  funObjLon6, funObjLat6, funObjDir6, funObjDirY6, funObjDirL6, funObjHou6,
  funObjLonN, funObjLatN, funObjDirN, funObjDirYN, funObjDirLN, funObjHouN,
  funObjX,
  funObjY,
  funObjZ,
  funObjOn,
  funObjOnT,
  funObjOrb,
  funObjAdd,
  funObjInf,
  funObjInfT,
  funObjCol,
  funObjDist,
  funObjYear,
  funObjDiam,
  funObjDay,
  funAspAng,
  funAspOrb,
  funAspInf,
  funAspCol,
  funCusp,  funCusp3D,
  funCusp1, funCusp3D1,
  funCusp2, funCusp3D2,
  funCusp3, funCusp3D3,
  funCusp4, funCusp3D4,
  funCuspInf,
  funSector,
  funLonSign,
  funLonDeg,
  funLonHou,
  funLonHou3,
  funLonDist,
  funLonDiff,
  funLonMid,
  funDayWeek,
  funJulianT,
  funSphDist,
  funOblique,
  funRAMC,
  funDeltaT,
  funSidDiff,
  funNutat,
  funSystem,
  funAspect,
  funAspect2,
  funParall,
  funParall2,
  funGridNam,
  funGridVal,
  funGridDo,
  funGridDo2,
  funContext,
  funVersion,

  // Astrolog command switch settings (general)
  fun_w1,
  fun_aj,
  fun_L1,
  fun_L2,
  fun_d1,
  fun_EY,
  fun_E01,
  fun_E02,
  fun_P1,
  fun_N1,
  fun_I1,
  fun_A3,
  fun_Ap,
  fun_AP,
  fun_b,
  fun_b0,
  fun_c,
  fun_c3,
  fun_c31,
  fun_s,
  fun_s0,
  fun_s1,
  fun_sr,
  fun_sr0,
  fun_h,
  fun_p,
  fun_p0,
  fun_pd,
  fun_pC,
  fun_x,
  fun_1,
  fun_3,
  fun_4,
  fun_f,
  fun_G,
  fun_J,
  fun_9,
  fun_YT,
  fun_YV,
  fun_Yh,
  fun_Ym,
  fun_Ys,
  fun_Yn,
  fun_Yn0,
  fun_Yz0,
  fun_Yu,
  fun_Yu0,
  fun_Yr,
  fun_YC,
  fun_YO,
  fun_Yc,
  fun_Yp,

#ifdef GRAPH
  // Astrolog command switch settings (graphics)
  fun_XI1,
  fun_XI2,
  fun_Xr,
  fun_Xm,
  fun_XT,
  fun_Xi,
  fun_Xu,
  fun_Xx,
  fun_Xl,
  fun_XA,
  fun_XL,
  fun_Xj,
  fun_XF,
  fun_XW0,
  fun_Xe,
  fun_XU,
  fun_XC,
  fun_XQ,
  fun_XN,
  fun_YXe,
  fun_Xwx,
  fun_Xwy,
  fun_Xn,
  fun_Xs,
  fun_XS,
  fun_XU0,
  fun_XE1,
  fun_XE2,
  fun_XE,
  fun_XL0,
  fun_X1,
  fun_Xv,
  fun_XGx,
  fun_XGy,
  fun_XZ,
  fun_YXK,
#endif

#ifdef WIN
  // Functions related to Windows operations
  funMouse,

  // Astrolog command switch settings (Windows)
  fun_WN,
  fun_Wn,
  fun_Wh,
  fun_Wt,
  fun_Wo,
  fun_Wo0,
  fun_Wo3,
  fun_WZ,
#endif

  // Programming functions (related to variables and control flow)
  funVar,
  funDo,
  funDo2,
  funDo3,
  funIf,
  funIfElse,
  funDoCount,
  funWhile,
  funDoWhile,
  funFor,
  funMacro,
  funSwitch,
  funRndSeed,

  // Variable assignment functions
  funAssign,  funAssign2,
  funAssignA, funAssignB, funAssignC, funAssignD, funAssignE,
  funAssignF, funAssignG, funAssignH, funAssignI, funAssignJ,
  funAssignK, funAssignL, funAssignM, funAssignN, funAssignO,
  funAssignP, funAssignQ, funAssignR, funAssignS, funAssignT,
  funAssignU, funAssignV, funAssignW, funAssignX, funAssignY,
  funAssignZ = cfun-1,
};

CONST FUN rgfun[cfun] = {
// General functions (independent of Astrolog and astrology)
{funFalse, "False", 0, I_},
{funTrue,  "True",  0, I_},
{funInt,   "Int",   1, I_I},
{funReal,  "Real",  1, R_R},
{funType,  "Type",  1, I_E},
{funAdd,   "Add",   2, E_EE},
{funSub,   "Sub",   2, E_EE},
{funMul,   "Mul",   2, E_EE},
{funDiv,   "Div",   2, E_EE},
{funMod,   "Mod",   2, I_II},
{funPow,   "Pow",   2, R_RR},
{funNeg,   "Neg",   1, E_E},
{funInc,   "Inc",   1, E_E},
{funDec,   "Dec",   1, E_E},
{funEqu,   "Equ",   2, I_EE},
{funNeq,   "Neq",   2, I_EE},
{funLt,    "Lt",    2, I_EE},
{funGt,    "Gt",    2, I_EE},
{funLte,   "Lte",   2, I_EE},
{funGte,   "Gte",   2, I_EE},
{funNot,   "Not",   1, I_I},
{funAnd,   "And",   2, I_II},
{funOr,    "Or",    2, I_II},
{funXor,   "Xor",   2, I_II},
{funInv,   "Inv",   1, I_I},
{funShftL, "<<",    2, I_II},
{funShftR, ">>",    2, I_II},
{funOdd,   "Odd",   1, I_I},
{funAbs,   "Abs",   1, E_E},
{funSgn,   "Sgn",   1, E_E},
{funSgn2,  "Sgn2",  1, E_E},
{funMin,   "Min",   2, E_EE},
{funMax,   "Max",   2, E_EE},
{funTween, "Tween", 3, I_EEE},
{funIfOp,  "?:",    3, E_IEE},
{funSqu,   "Squ",   1, E_E},
{funSqr,   "Sqr",   1, R_R},
{funDist,  "Dist",  2, R_RR},
{funLn,    "Ln",    1, R_R},
{funLog10, "Log10", 1, R_R},
{funSin,   "Sin",   1, R_R},
{funCos,   "Cos",   1, R_R},
{funTan,   "Tan",   1, R_R},
{funAsin,  "Asin",  1, R_R},
{funAcos,  "Acos",  1, R_R},
{funAtan,  "Atan",  1, R_R},
{funAng,   "Ang",   2, R_RR},
{funSinD,  "SinD",  1, R_R},
{funCosD,  "CosD",  1, R_R},
{funTanD,  "TanD",  1, R_R},
{funAsinD, "AsinD", 1, R_R},
{funAcosD, "AcosD", 1, R_R},
{funAtanD, "AtanD", 1, R_R},
{funAngD,  "AngD",  2, R_RR},
{funFloor, "Floor", 1, R_R},
{funFract, "Fract", 1, R_R},
{funRnd,   "Rnd",   2, I_II},
{funRgb,   "Rgb",   3, I_III},
{funRgbR,  "RgbR",  1, I_I},
{funRgbG,  "RgbG",  1, I_I},
{funRgbB,  "RgbB",  1, I_I},
{funBlend, "Blend", 3, I_IIR},
{funHue,   "Hue",   1, I_R},

// Specific functions (related to Astrolog and astrology)
{funSigns,   "Signs",    0, I_},
{funObjs,    "Objs",     0, I_},
{funAsps,    "Asps",     0, I_},
{funMon,     "Mon",      0, I_},
{funDay,     "Day",      0, I_},
{funYea,     "Yea",      0, I_},
{funTim,     "Tim",      0, R_},
{funDst,     "Dst",      0, R_},
{funZon,     "Zon",      0, R_},
{funLon,     "Lon",      0, R_},
{funLat,     "Lat",      0, R_},
{funMon1,    "Mon1",     0, I_},
{funDay1,    "Day1",     0, I_},
{funYea1,    "Yea1",     0, I_},
{funTim1,    "Tim1",     0, R_},
{funDst1,    "Dst1",     0, R_},
{funZon1,    "Zon1",     0, R_},
{funLon1,    "Lon1",     0, R_},
{funLat1,    "Lat1",     0, R_},
{funMon2,    "Mon2",     0, I_},
{funDay2,    "Day2",     0, I_},
{funYea2,    "Yea2",     0, I_},
{funTim2,    "Tim2",     0, R_},
{funDst2,    "Dst2",     0, R_},
{funZon2,    "Zon2",     0, R_},
{funLon2,    "Lon2",     0, R_},
{funLat2,    "Lat2",     0, R_},
{funMon3,    "Mon3",     0, I_},
{funDay3,    "Day3",     0, I_},
{funYea3,    "Yea3",     0, I_},
{funTim3,    "Tim3",     0, R_},
{funDst3,    "Dst3",     0, R_},
{funZon3,    "Zon3",     0, R_},
{funLon3,    "Lon3",     0, R_},
{funLat3,    "Lat3",     0, R_},
{funMon4,    "Mon4",     0, I_},
{funDay4,    "Day4",     0, I_},
{funYea4,    "Yea4",     0, I_},
{funTim4,    "Tim4",     0, R_},
{funDst4,    "Dst4",     0, R_},
{funZon4,    "Zon4",     0, R_},
{funLon4,    "Lon4",     0, R_},
{funLat4,    "Lat4",     0, R_},
{funMon5,    "Mon5",     0, I_},
{funDay5,    "Day5",     0, I_},
{funYea5,    "Yea5",     0, I_},
{funTim5,    "Tim5",     0, R_},
{funDst5,    "Dst5",     0, R_},
{funZon5,    "Zon5",     0, R_},
{funLon5,    "Lon5",     0, R_},
{funLat5,    "Lat5",     0, R_},
{funMon6,    "Mon6",     0, I_},
{funDay6,    "Day6",     0, I_},
{funYea6,    "Yea6",     0, I_},
{funTim6,    "Tim6",     0, R_},
{funDst6,    "Dst6",     0, R_},
{funZon6,    "Zon6",     0, R_},
{funLon6,    "Lon6",     0, R_},
{funLat6,    "Lat6",     0, R_},
{funMonN,    "MonN",     1, I_I},
{funDayN,    "DayN",     1, I_I},
{funYeaN,    "YeaN",     1, I_I},
{funTimN,    "TimN",     1, R_I},
{funDstN,    "DstN",     1, R_I},
{funZonN,    "ZonN",     1, R_I},
{funLonN,    "LonN",     1, R_I},
{funLatN,    "LatN",     1, R_I},
{funMonS,    "MonS",     0, I_},
{funDayS,    "DayS",     0, I_},
{funYeaS,    "YeaS",     0, I_},
{funTimS,    "TimS",     0, R_},
{funDstS,    "DstS",     0, R_},
{funZonS,    "ZonS",     0, R_},
{funLonS,    "LonS",     0, R_},
{funLatS,    "LatS",     0, R_},
{funMonT,    "MonT",     0, I_},
{funDayT,    "DayT",     0, I_},
{funYeaT,    "YeaT",     0, I_},
{funTimT,    "TimT",     0, R_},
{funMonG,    "MonG",     0, I_},
{funDayG,    "DayG",     0, I_},
{funYeaG,    "YeaG",     0, I_},
{funObjLon,   "ObjLon",    1, R_I}, 
{funObjLat,   "ObjLat",    1, R_I},
{funObjDir,   "ObjDir",    1, R_I},
{funObjDirY,  "ObjDirY",   1, R_I},
{funObjDirL,  "ObjDirL",   1, R_I},
{funObjHou,   "ObjHouse",  1, I_I},
{funObjLon1,  "ObjLon1",   1, R_I}, 
{funObjLat1,  "ObjLat1",   1, R_I},
{funObjDir1,  "ObjDir1",   1, R_I},
{funObjDirY1, "ObjDirY1",  1, R_I},
{funObjDirL1, "ObjDirL1",  1, R_I},
{funObjHou1,  "ObjHouse1", 1, I_I},
{funObjLon2,  "ObjLon2",   1, R_I}, 
{funObjLat2,  "ObjLat2",   1, R_I},
{funObjDir2,  "ObjDir2",   1, R_I},
{funObjDirY2, "ObjDirY2",  1, R_I},
{funObjDirL2, "ObjDirL2",  1, R_I},
{funObjHou2,  "ObjHouse2", 1, I_I},
{funObjLon3,  "ObjLon3",   1, R_I}, 
{funObjLat3,  "ObjLat3",   1, R_I},
{funObjDir3,  "ObjDir3",   1, R_I},
{funObjDirY3, "ObjDirY3",  1, R_I},
{funObjDirL3, "ObjDirL3",  1, R_I},
{funObjHou3,  "ObjHouse3", 1, I_I},
{funObjLon4,  "ObjLon4",   1, R_I}, 
{funObjLat4,  "ObjLat4",   1, R_I},
{funObjDir4,  "ObjDir4",   1, R_I},
{funObjDirY4, "ObjDirY4",  1, R_I},
{funObjDirL4, "ObjDirL4",  1, R_I},
{funObjHou4,  "ObjHouse4", 1, I_I},
{funObjLon5,  "ObjLon5",   1, R_I}, 
{funObjLat5,  "ObjLat5",   1, R_I},
{funObjDir5,  "ObjDir5",   1, R_I},
{funObjDirY5, "ObjDirY5",  1, R_I},
{funObjDirL5, "ObjDirL5",  1, R_I},
{funObjHou5,  "ObjHouse5", 1, I_I},
{funObjLon6,  "ObjLon6",   1, R_I}, 
{funObjLat6,  "ObjLat6",   1, R_I},
{funObjDir6,  "ObjDir6",   1, R_I},
{funObjDirY6, "ObjDirY6",  1, R_I},
{funObjDirL6, "ObjDirL6",  1, R_I},
{funObjHou6,  "ObjHouse6", 1, I_I},
{funObjLonN,  "ObjLonN",   2, R_II}, 
{funObjLatN,  "ObjLatN",   2, R_II},
{funObjDirN,  "ObjDirN",   2, R_II},
{funObjDirYN, "ObjDirYN",  2, R_II},
{funObjDirLN, "ObjDirLN",  2, R_II},
{funObjHouN,  "ObjHouseN", 2, I_II},
{funObjX,    "ObjX",     1, R_I},
{funObjY,    "ObjY",     1, R_I},
{funObjZ,    "ObjZ",     1, R_I},
{funObjOn,   "ObjOn",    1, I_I},
{funObjOnT,  "ObjOnT",   1, I_I},
{funObjOrb,  "ObjOrb",   1, R_I},
{funObjAdd,  "ObjAdd",   1, R_I},
{funObjInf,  "ObjInf",   1, R_I},
{funObjInfT, "ObjInfT",  1, R_I},
{funObjCol,  "ObjCol",   1, I_I},
{funObjDist, "ObjDist",  1, R_I},
{funObjYear, "ObjYear",  1, R_I},
{funObjDiam, "ObjDiam",  1, R_I},
{funObjDay,  "ObjDay",   1, R_I},
{funAspAng,  "AspAngle", 1, R_I},
{funAspOrb,  "AspOrb",   1, R_I},
{funAspInf,  "AspInf",   1, R_I},
{funAspCol,  "AspCol",   1, I_I},
{funCusp,    "Cusp",     1, R_I},
{funCusp3D,  "Cusp3D",   1, R_I},
{funCusp1,   "Cusp1",    1, R_I},
{funCusp3D1, "Cusp3D1",  1, R_I},
{funCusp2,   "Cusp2",    1, R_I},
{funCusp3D2, "Cusp3D2",  1, R_I},
{funCusp3,   "Cusp3",    1, R_I},
{funCusp3D3, "Cusp3D3",  1, R_I},
{funCusp4,   "Cusp4",    1, R_I},
{funCusp3D4, "Cusp3D4",  1, R_I},
{funCuspInf, "HouseInf", 1, R_I},
{funSector,  "PlusZone", 1, I_I},
{funLonSign, "LonSign",  1, I_R},
{funLonDeg,  "LonDeg",   1, R_R},
{funLonHou,  "LonHouse", 1, I_R},
{funLonHou3, "LonHou3D", 2, R_RR},
{funLonDist, "LonDist",  2, R_RR},
{funLonDiff, "LonDiff",  2, R_RR},
{funLonMid,  "LonMid",   2, R_RR},
{funDayWeek, "DayWeek",  3, I_III},
{funJulianT, "JulianT",  0, R_},
{funSphDist, "PolDist",  4, R_RRRR},
{funOblique, "Oblique",  0, R_},
{funRAMC,    "RAMC",     0, R_},
{funDeltaT,  "DeltaT",   0, R_},
{funSidDiff, "SidDiff",  0, R_},
{funNutat,   "Nutation", 0, R_},
{funSystem,  "HouseSys", 0, I_}, // Same as _c
{funAspect,  "AspLon",   3, I_III},
{funAspect2, "AspLon2",  3, I_III},
{funParall,  "AspLat",   3, I_III},
{funParall2, "AspLat2",  3, I_III},
{funGridNam, "GridNam",  2, I_II},
{funGridVal, "GridVal",  2, I_II},
{funGridDo,  "DoGrid",   0, I_},
{funGridDo2, "DoGrid2",  1, I_I},
{funContext, "Context",  0, I_},
{funVersion, "Version",  0, R_},

// Astrolog command switch settings (general)
{fun_w1,  "_w1",  0, I_},
{fun_aj,  "_aj",  0, I_},
{fun_L1,  "_L1",  0, I_},
{fun_L2,  "_L2",  0, I_},
{fun_d1,  "_d1",  0, I_},
{fun_EY,  "_EY",  0, I_},
{fun_E01, "_E01", 0, I_},
{fun_E02, "_E02", 0, I_},
{fun_P1,  "_P1",  0, I_},
{fun_N1,  "_N1",  0, I_},
{fun_I1,  "_I1",  0, I_},
{fun_A3,  "_A3",  0, I_},
{fun_Ap,  "_Ap",  0, I_},
{fun_AP,  "_APP", 0, I_},
{fun_b,   "_b",   0, I_},
{fun_b0,  "_b0",  0, I_},
{fun_c,   "_c",   0, I_},
{fun_c3,  "_c3",  0, I_},
{fun_c31, "_c31", 0, I_},
{fun_s,   "_s",   0, I_},
{fun_s0,  "_s0",  0, I_},
{fun_s1,  "_s1",  0, R_},
{fun_sr,  "_sr",  0, I_},
{fun_sr0, "_sr0", 0, I_},
{fun_h,   "_h",   0, I_},
{fun_p,   "_p",   0, I_},
{fun_p0,  "_p0",  0, I_},
{fun_pd,  "_pd",  0, R_},
{fun_pC,  "_pC",  0, R_},
{fun_x,   "_x",   0, R_},
{fun_1,   "_1",   0, I_},
{fun_3,   "_3",   0, I_},
{fun_4,   "_4",   0, I_},
{fun_f,   "_f",   0, I_},
{fun_G,   "_G",   0, I_},
{fun_J,   "_J",   0, I_},
{fun_9,   "_9",   0, I_},
{fun_YT,  "_YT",  0, I_},
{fun_YV,  "_YV",  0, I_},
{fun_Yh,  "_Yh",  0, I_},
{fun_Ym,  "_Ym",  0, I_},
{fun_Ys,  "_Ys",  0, I_},
{fun_Yn,  "_Yn",  0, I_},
{fun_Yn0, "_Yn0", 0, I_},
{fun_Yz0, "_Yz0", 0, R_},
{fun_Yu,  "_Yu",  0, I_},
{fun_Yu0, "_Yu0",  0, I_},
{fun_Yr,  "_Yr",  0, I_},
{fun_YC,  "_YC",  0, I_},
{fun_YO,  "_YO",  0, I_},
{fun_Yc,  "_Ycc", 0, I_},
{fun_Yp,  "_Yp",  0, I_},

#ifdef GRAPH
// Astrolog command switch settings (graphics)
{fun_XI1, "_XI1", 0, R_},
{fun_XI2, "_XI2", 0, I_},
{fun_Xr,  "_Xr",  0, I_},
{fun_Xm,  "_Xm",  0, I_},
{fun_XT,  "_XT",  0, I_},
{fun_Xi,  "_Xi",  0, I_},
{fun_Xu,  "_Xuu", 0, I_},
{fun_Xx,  "_Xx",  0, I_},
{fun_Xl,  "_Xll", 0, I_},
{fun_XA,  "_XA",  0, I_},
{fun_XL,  "_XL",  0, I_},
{fun_Xj,  "_Xj",  0, I_},
{fun_XF,  "_XF",  0, I_},
{fun_XW0, "_XW0", 0, I_},
{fun_Xe,  "_Xee", 0, I_},
{fun_XU,  "_XU",  0, I_},
{fun_XC,  "_XC",  0, I_},
{fun_XQ,  "_XQ",  0, I_},
{fun_XN,  "_XN",  0, I_},
{fun_YXe, "_YXe", 0, I_},
{fun_Xwx, "_Xwx", 0, I_},
{fun_Xwy, "_Xwy", 0, I_},
{fun_Xn,  "_Xnn", 0, I_},
{fun_Xs,  "_Xs",  0, I_},
{fun_XS,  "_XSS", 0, I_},
{fun_XU0, "_XU0", 0, I_},
{fun_XE1, "_XE1", 0, I_},
{fun_XE2, "_XE2", 0, I_},
{fun_XE,  "_XE",  0, I_},
{fun_XL0, "_XL0", 0, I_},
{fun_X1,  "_X1",  0, I_},
{fun_Xv,  "_Xv",  0, I_},
{fun_XGx, "_XGx", 0, R_},
{fun_XGy, "_XGy", 0, R_},
{fun_XZ,  "_XZ",  0, I_},
{fun_YXK, "_YXK", 1, I_I},
#endif

#ifdef WIN
// Functions related to Windows operations
{funMouse, "Mouse", 1, I_I},

// Astrolog command switch settings (Windows)
{fun_WN,   "_WN",   1, I_},
{fun_Wn,   "_Wnn",  1, I_},
{fun_Wh,   "_Wh",   1, I_},
{fun_Wt,   "_Wt",   1, I_},
{fun_Wo,   "_Wo",   1, I_},
{fun_Wo0,  "_Wo0",  1, I_},
{fun_Wo3,  "_Wo3",  1, I_},
{fun_WZ,   "_WZ",   1, I_},
#endif

// Programming functions (related to variables and control flow)
{funVar,     "Var",     1, E_I},
{funDo,      "Do",      2, E_XE},
{funDo2,     "Do2",     3, E_XXE},
{funDo3,     "Do3",     4, E_XXXE},
{funIf,      "If",      2, E_IX},
{funIfElse,  "IfElse",  3, E_IXX},
{funDoCount, "DoCount", 2, E_IX},
{funWhile,   "While",   2, E_XE},
{funDoWhile, "DoWhile", 2, E_XE},
{funFor,     "For",     4, I_IIIX},
{funMacro,   "Macro",   1, E_I},
{funSwitch,  "Switch",  1, I_I},
{funRndSeed, "RndSeed", 1, I_I},

// Variable assignment functions
{funAssign,  "Assign", 2, E_IE},
{funAssign2, "=",      2, E_IE},
{funAssignA, "=A",     1, E_E},
{funAssignB, "=B",     1, E_E},
{funAssignC, "=C",     1, E_E},
{funAssignD, "=D",     1, E_E},
{funAssignE, "=E",     1, E_E},
{funAssignF, "=F",     1, E_E},
{funAssignG, "=G",     1, E_E},
{funAssignH, "=H",     1, E_E},
{funAssignI, "=I",     1, E_E},
{funAssignJ, "=J",     1, E_E},
{funAssignK, "=K",     1, E_E},
{funAssignL, "=L",     1, E_E},
{funAssignM, "=M",     1, E_E},
{funAssignN, "=N",     1, E_E},
{funAssignO, "=O",     1, E_E},
{funAssignP, "=P",     1, E_E},
{funAssignQ, "=Q",     1, E_E},
{funAssignR, "=R",     1, E_E},
{funAssignS, "=S",     1, E_E},
{funAssignT, "=T",     1, E_E},
{funAssignU, "=U",     1, E_E},
{funAssignV, "=V",     1, E_E},
{funAssignW, "=W",     1, E_E},
{funAssignX, "=X",     1, E_E},
{funAssignY, "=Y",     1, E_E},
{funAssignZ, "=Z",     1, E_E},
};


/*
******************************************************************************
** Utilities
******************************************************************************
*/

// Convert a range of characters to upper case.

void UpperRgch(char *rgch, int cch)
{
  while (cch > 0) {
    *rgch = ChCap(*rgch);
    rgch++, cch--;
  }
}


// Parse and return a number contained in a range of characters.

long LFromRgch(CONST char *rgch, int cch)
{
  char sz[cchSzDef], *pch;
  long l;
  flag fBinary;

  CopyRgchToSz(rgch, cch, sz, cchSzDef);
  if (*sz != '#')
    return atol(sz);
  fBinary = (*(sz+1) == '#');

  // Strings starting with "#" are considered hexadecimal numbers.
  // Strings starting with "##" are considered binary numbers.
  l = 0;
  for (pch = sz+1+fBinary; *pch; pch++)
    l = fBinary ? ((l << 1) | (*pch == '1')) :
      ((l << 4) | NHex2(ChCap(*pch)));
  return l;
}


/*
******************************************************************************
** Action Processing
******************************************************************************
*/

// Evaluate a function, generating the number it evaluates to, given a list of
// parameters to the function.

flag FEvalFunction(int ifun, PAR *rgpar, char *rgpchEval[2])
{
  int ipar, nType, n = 0, n1, n2, n3, n4;
  real r = 0.0, r1, r2, r3, r4;
  char sz[cchSzMax];
  flag fRetOpt = fFalse, fRetReal = fFalse, fOpt = fFalse, fOptReal = fFalse;
#ifdef WIN
  POINT pt;
#endif

  // Analyze the input and output parameter types.
  for (ipar = 0; ipar <= rgfun[ifun].nParam; ipar++) {
    nType = (rgfun[ifun].nType >> (ipar << 1)) & 3;
    if (ipar == 0) {
      if (nType == R_)
        fRetReal = fTrue;
      else if (nType == E_)
        fRetOpt = fTrue;
      continue;
    }
    if (nType == I_) {
      if (rgpar[ipar].fReal)
        rgpar[ipar].n = (int)rgpar[ipar].r;
    } else if (nType == R_) {
      if (!rgpar[ipar].fReal)
        rgpar[ipar].r = (real)rgpar[ipar].n;
    } else if (nType == E_) {
      fOpt = fTrue;
      if (rgpar[ipar].fReal)
        fOptReal = fTrue;
    }
  }

  // Convert optional type parameters to real if any are real.
  if (fOpt && fOptReal)
    for (ipar = 1; ipar <= rgfun[ifun].nParam; ipar++) {
      nType = (rgfun[ifun].nType >> (ipar << 1)) & 3;
      if (nType == E_ && !rgpar[ipar].fReal) {
        rgpar[ipar].r = (real)rgpar[ipar].n;
        rgpar[ipar].fReal = fTrue;
      }
    }
  if (fRetOpt && fOptReal)
    fRetReal = fTrue;

  n1 = rgpar[1].n; n2 = rgpar[2].n; n3 = rgpar[3].n; n4 = rgpar[4].n;
  r1 = rgpar[1].r; r2 = rgpar[2].r; r3 = rgpar[3].r; r4 = rgpar[4].r;

  switch (ifun) {
  // General functions (independent of Astrolog and astrology)
  case funFalse: n = fFalse; break;
  case funTrue:  n = fTrue;  break;
  case funInt:   n = n1;     break;
  case funReal:  r = r1;     break;
  case funType:  n = fOptReal; break;
  case funAdd:   EIR(n1 + n2, r1 + r2); break;
  case funSub:   EIR(n1 - n2, r1 - r2); break;
  case funMul:   EIR(n1 * n2, r1 * r2); break;
  case funDiv:   EIR(n2 != 0 ? n1 / n2 : n1, r2 != 0.0 ? r1 / r2 : r1); break;
  case funMod:   n = n2 != 0 ? n1 % n2 : n1; break;
  case funPow:   r = pow(r1, r2); break;
  case funNeg:   EIR(-n1, -r1); break;
  case funInc:   EIR(n1 + 1, r1 + 1.0); break;
  case funDec:   EIR(n1 - 1, r1 - 1.0); break;
  case funEqu:   IIR(n1 == n2, r1 == r2); break;
  case funNeq:   IIR(n1 != n2, r1 != r2); break;
  case funLt:    IIR(n1 < n2,  r1 < r2);  break;
  case funGt:    IIR(n1 > n2,  r1 > r2);  break;
  case funLte:   IIR(n1 <= n2, r1 <= r2); break;
  case funGte:   IIR(n1 >= n2, r1 >= r2); break;
  case funNot:   n = !n1;      break;
  case funAnd:   n = n1 & n2;  break;
  case funOr:    n = n1 | n2;  break;
  case funXor:   n = n1 ^ n2;  break;
  case funInv:   n = ~n1;      break;
  case funShftL: n = n1 << n2; break;
  case funShftR: n = n1 >> n2; break;
  case funOdd:   n = FOdd(n1); break;
  case funAbs:   EIR(NAbs(n1), RAbs(r1)); break;
  case funSgn:   EIR(NSgn(n1), RSgn(r1)); break;
  case funSgn2:  EIR(NSgn2(n1), RSgn2(r1)); break;
  case funMin:   EIR(Min(n1, n2), Min(r1, r2)); break;
  case funMax:   EIR(Max(n1, n2), Max(r1, r2)); break;
  case funTween: IIR(FBetween(n1, n2, n3), FBetween(r1, r2, r3)); break;
  case funIfOp:  EIR(n1 ? n2 : n3, n1 ? r2 : r3); break;
  case funSqu:   EIR(Sq(n1), Sq(r1)); break;
  case funSqr:   r = RSqr(r1); break;
  case funDist:  r = RSqr(Sq(r1)+Sq(r2)); break;
  case funLn:    r = RLog(r1); break;
  case funLog10: r = RLog10(r1); break;
  case funSin:   r = RSin(r1); break;
  case funCos:   r = RCos(r1); break;
  case funTan:   r = RTan(r1); break;
  case funAsin:  r = RAsin(r1); break;
  case funAcos:  r = RAcos(r1); break;
  case funAtan:  r = RAtn(r1);  break;
  case funAng:   r = RAngle(r1, r2); break;
  case funSinD:  r = RSinD(r1); break;
  case funCosD:  r = RCosD(r1); break;
  case funTanD:  r = RTanD(r1); break;
  case funAsinD: r = RAsinD(r1); break;
  case funAcosD: r = RAcosD(r1); break;
  case funAtanD: r = RAtnD(r1);  break;
  case funAngD:  r = RAngleD(r1, r2); break;
  case funFloor: r = RFloor(r1); break;
  case funFract: r = r1 - RFloor(r1); break;
  case funRnd:   n = (rand() & 16383) * (n2 - n1 + 1) / 16384 + n1; break;
  case funRgb:   n = Rgb(n1, n2, n3); break;
  case funRgbR:  n = RgbR(n1); break;
  case funRgbG:  n = RgbG(n1); break;
  case funRgbB:  n = RgbB(n1); break;
  case funBlend: n = KvBlend(n1, n2, r3); break;
  case funHue:   n = KvHue(r1); break;

  // Specific functions (related to Astrolog and astrology)
  case funSigns: n = cSign;   break;
  case funObjs:  n = is.nObj; break;
  case funAsps:  n = us.nAsp; break;
  case funMon:  n = MM; break;
  case funDay:  n = DD; break;
  case funYea:  n = YY; break;
  case funTim:  r = TT; break;
  case funDst:  r = SS; break;
  case funZon:  r = ZZ; break;
  case funLon:  r = OO; break;
  case funLat:  r = AA; break;
  case funMon1: n = ciMain.mon; break;
  case funDay1: n = ciMain.day; break;
  case funYea1: n = ciMain.yea; break;
  case funTim1: r = ciMain.tim; break;
  case funDst1: r = ciMain.dst; break;
  case funZon1: r = ciMain.zon; break;
  case funLon1: r = ciMain.lon; break;
  case funLat1: r = ciMain.lat; break;
  case funMon2: n = ciTwin.mon; break;
  case funDay2: n = ciTwin.day; break;
  case funYea2: n = ciTwin.yea; break;
  case funTim2: r = ciTwin.tim; break;
  case funDst2: r = ciTwin.dst; break;
  case funZon2: r = ciTwin.zon; break;
  case funLon2: r = ciTwin.lon; break;
  case funLat2: r = ciTwin.lat; break;
  case funMon3: n = ciThre.mon; break;
  case funDay3: n = ciThre.day; break;
  case funYea3: n = ciThre.yea; break;
  case funTim3: r = ciThre.tim; break;
  case funDst3: r = ciThre.dst; break;
  case funZon3: r = ciThre.zon; break;
  case funLon3: r = ciThre.lon; break;
  case funLat3: r = ciThre.lat; break;
  case funMon4: n = ciFour.mon; break;
  case funDay4: n = ciFour.day; break;
  case funYea4: n = ciFour.yea; break;
  case funTim4: r = ciFour.tim; break;
  case funDst4: r = ciFour.dst; break;
  case funZon4: r = ciFour.zon; break;
  case funLon4: r = ciFour.lon; break;
  case funLat4: r = ciFour.lat; break;
  case funMon5: n = ciFive.mon; break;
  case funDay5: n = ciFive.day; break;
  case funYea5: n = ciFive.yea; break;
  case funTim5: r = ciFive.tim; break;
  case funDst5: r = ciFive.dst; break;
  case funZon5: r = ciFive.zon; break;
  case funLon5: r = ciFive.lon; break;
  case funLat5: r = ciFive.lat; break;
  case funMon6: n = ciHexa.mon; break;
  case funDay6: n = ciHexa.day; break;
  case funYea6: n = ciHexa.yea; break;
  case funTim6: r = ciHexa.tim; break;
  case funDst6: r = ciHexa.dst; break;
  case funZon6: r = ciHexa.zon; break;
  case funLon6: r = ciHexa.lon; break;
  case funLat6: r = ciHexa.lat; break;
  case funMonN: n = FBetween(n1, 0, cRing) ? rgpci[n1]->mon : 0;   break;
  case funDayN: n = FBetween(n1, 0, cRing) ? rgpci[n1]->day : 0;   break;
  case funYeaN: n = FBetween(n1, 0, cRing) ? rgpci[n1]->yea : 0;   break;
  case funTimN: r = FBetween(n1, 0, cRing) ? rgpci[n1]->tim : 0.0; break;
  case funDstN: r = FBetween(n1, 0, cRing) ? rgpci[n1]->dst : 0.0; break;
  case funZonN: r = FBetween(n1, 0, cRing) ? rgpci[n1]->zon : 0.0; break;
  case funLonN: r = FBetween(n1, 0, cRing) ? rgpci[n1]->lon : 0.0; break;
  case funLatN: r = FBetween(n1, 0, cRing) ? rgpci[n1]->lat : 0.0; break;
  case funMonS: n = ciSave.mon; break;
  case funDayS: n = ciSave.day; break;
  case funYeaS: n = ciSave.yea; break;
  case funTimS: r = ciSave.tim; break;
  case funDstS: r = ciSave.dst; break;
  case funZonS: r = ciSave.zon; break;
  case funLonS: r = ciSave.lon; break;
  case funLatS: r = ciSave.lat; break;
  case funMonT: n = ciTran.mon; break;
  case funDayT: n = ciTran.day; break;
  case funYeaT: n = ciTran.yea; break;
  case funTimT: r = ciTran.tim; break;
  case funMonG: n = ciGreg.mon; break;
  case funDayG: n = ciGreg.day; break;
  case funYeaG: n = ciGreg.yea; break;
  case funObjLon:   r = FValidObj(n1) ? planet[n1]     : 0.0; break;
  case funObjLat:   r = FValidObj(n1) ? planetalt[n1]  : 0.0; break;
  case funObjDir:   r = FValidObj(n1) ? ret[n1]        : 0.0; break;
  case funObjDirY:  r = FValidObj(n1) ? retalt[n1]     : 0.0; break;
  case funObjDirL:  r = FValidObj(n1) ? retlen[n1]     : 0.0; break;
  case funObjHou:   n = FValidObj(n1) ? inhouse[n1]    : 0;   break;
  case funObjLon1:  r = FValidObj(n1) ? cp1.obj[n1]    : 0.0; break;
  case funObjLat1:  r = FValidObj(n1) ? cp1.alt[n1]    : 0.0; break;
  case funObjDir1:  r = FValidObj(n1) ? cp1.dir[n1]    : 0.0; break;
  case funObjDirY1: r = FValidObj(n1) ? cp1.diralt[n1] : 0.0; break;
  case funObjDirL1: r = FValidObj(n1) ? cp1.dirlen[n1] : 0.0; break;
  case funObjHou1:  n = FValidObj(n1) ? cp1.house[n1]  : 0;   break;
  case funObjLon2:  r = FValidObj(n1) ? cp2.obj[n1]    : 0.0; break;
  case funObjLat2:  r = FValidObj(n1) ? cp2.alt[n1]    : 0.0; break;
  case funObjDir2:  r = FValidObj(n1) ? cp2.dir[n1]    : 0.0; break;
  case funObjDirY2: r = FValidObj(n1) ? cp2.diralt[n1] : 0.0; break;
  case funObjDirL2: r = FValidObj(n1) ? cp2.dirlen[n1] : 0.0; break;
  case funObjHou2:  n = FValidObj(n1) ? cp2.house[n1]  : 0;   break;
  case funObjLon3:  r = FValidObj(n1) ? cp3.obj[n1]    : 0.0; break;
  case funObjLat3:  r = FValidObj(n1) ? cp3.alt[n1]    : 0.0; break;
  case funObjDir3:  r = FValidObj(n1) ? cp3.dir[n1]    : 0.0; break;
  case funObjDirY3: r = FValidObj(n1) ? cp3.diralt[n1] : 0.0; break;
  case funObjDirL3: r = FValidObj(n1) ? cp3.dirlen[n1] : 0.0; break;
  case funObjHou3:  n = FValidObj(n1) ? cp3.house[n1]  : 0;   break;
  case funObjLon4:  r = FValidObj(n1) ? cp4.obj[n1]    : 0.0; break;
  case funObjLat4:  r = FValidObj(n1) ? cp4.alt[n1]    : 0.0; break;
  case funObjDir4:  r = FValidObj(n1) ? cp4.dir[n1]    : 0.0; break;
  case funObjDirY4: r = FValidObj(n1) ? cp4.diralt[n1] : 0.0; break;
  case funObjDirL4: r = FValidObj(n1) ? cp4.dirlen[n1] : 0.0; break;
  case funObjHou4:  n = FValidObj(n1) ? cp4.house[n1]  : 0;   break;
  case funObjLon5:  r = FValidObj(n1) ? cp5.obj[n1]    : 0.0; break;
  case funObjLat5:  r = FValidObj(n1) ? cp5.alt[n1]    : 0.0; break;
  case funObjDir5:  r = FValidObj(n1) ? cp5.dir[n1]    : 0.0; break;
  case funObjDirY5: r = FValidObj(n1) ? cp5.diralt[n1] : 0.0; break;
  case funObjDirL5: r = FValidObj(n1) ? cp5.dirlen[n1] : 0.0; break;
  case funObjHou5:  n = FValidObj(n1) ? cp5.house[n1]  : 0;   break;
  case funObjLon6:  r = FValidObj(n1) ? cp6.obj[n1]    : 0.0; break;
  case funObjLat6:  r = FValidObj(n1) ? cp6.alt[n1]    : 0.0; break;
  case funObjDir6:  r = FValidObj(n1) ? cp6.dir[n1]    : 0.0; break;
  case funObjDirY6: r = FValidObj(n1) ? cp6.diralt[n1] : 0.0; break;
  case funObjDirL6: r = FValidObj(n1) ? cp6.dirlen[n1] : 0.0; break;
  case funObjHou6:  n = FValidObj(n1) ? cp6.house[n1]  : 0;   break;
  case funObjLonN:  r = FRingObj(n1, n2) ? rgpcp[n1]->obj[n2]    : 0.0; break;
  case funObjLatN:  r = FRingObj(n1, n2) ? rgpcp[n1]->alt[n2]    : 0.0; break;
  case funObjDirN:  r = FRingObj(n1, n2) ? rgpcp[n1]->dir[n2]    : 0.0; break;
  case funObjDirYN: r = FRingObj(n1, n2) ? rgpcp[n1]->diralt[n2] : 0.0; break;
  case funObjDirLN: r = FRingObj(n1, n2) ? rgpcp[n1]->dirlen[n2] : 0.0; break;
  case funObjHouN:  n = FRingObj(n1, n2) ? rgpcp[n1]->house[n2]  : 0;   break;
  case funObjX:    r = FValidObj(n1)    ? space[n1].x   : 0.0; break;
  case funObjY:    r = FValidObj(n1)    ? space[n1].y   : 0.0; break;
  case funObjZ:    r = FValidObj(n1)    ? space[n1].z   : 0.0; break;
  case funObjOn:   n = FValidObj(n1)    ? !ignore[n1]   : 0;   break;
  case funObjOnT:  n = FValidObj(n1)    ? !ignore2[n1]  : 0;   break;
  case funObjOrb:  r = FValidObj(n1)    ? rObjOrb[n1]   : 0.0; break;
  case funObjAdd:  r = FValidObj(n1)    ? rObjAdd[n1]   : 0.0; break;
  case funObjInf:  r = FBetween(n1, 0, oNorm1+5) ? rObjInf[n1]   : 0.0; break;
  case funObjInfT: r = FBetween(n1, 0, oNorm1) ? rTransitInf[n1] : 0.0; break;
  case funObjCol:  n = FValidObj(n1)    ? kObjA[n1]     : 0;   break;
  case funObjDist: r = FNorm(n1)        ? rObjDist[n1]  : 0.0; break;
  case funObjYear: r = FNorm(n1)        ? rObjYear[n1]  : 0.0; break;
  case funObjDiam: r = RObjDiam(n1); break;
  case funObjDay:  r = FNorm(n1)        ? rObjDay[n1]   : 0.0; break;
  case funAspAng:  r = FValidAspect(n1) ? rAspAngle[n1] : 0.0; break;
  case funAspOrb:  r = FValidAspect(n1) ? rAspOrb[n1]   : 0.0; break;
  case funAspInf:  r = FValidAspect(n1) ? rAspInf[n1]   : 0.0; break;
  case funAspCol:  n = FValidAspect(n1) ? kAspA[n1]     : 0;   break;
  case funCusp:    r = FValidSign(n1)   ? chouse[n1]    : 0.0; break;
  case funCusp3D:  r = FValidSign(n1)   ? chouse3[n1]   : 0.0; break;
  case funCusp1:   r = FValidSign(n1)   ? cp1.cusp[n1]  : 0.0; break;
  case funCusp3D1: r = FValidSign(n1)   ? cp1.cusp3[n1] : 0.0; break;
  case funCusp2:   r = FValidSign(n1)   ? cp2.cusp[n1]  : 0.0; break;
  case funCusp3D2: r = FValidSign(n1)   ? cp2.cusp3[n1] : 0.0; break;
  case funCusp3:   r = FValidSign(n1)   ? cp3.cusp[n1]  : 0.0; break;
  case funCusp3D3: r = FValidSign(n1)   ? cp3.cusp3[n1] : 0.0; break;
  case funCusp4:   r = FValidSign(n1)   ? cp4.cusp[n1]  : 0.0; break;
  case funCusp3D4: r = FValidSign(n1)   ? cp4.cusp3[n1] : 0.0; break;
  case funCuspInf: r = FBetween(n1, 1, cSign+5) ? rHouseInf[n1] : 0.0; break;
  case funSector:  n = FSector(n1) ? pluszone[n1] : 0; break;
  case funLonSign: n = SFromZ(r1); break;
  case funLonDeg:  r = r1 - (real)((SFromZ(r1)-1)*30); break;
  case funLonHou:  n = NHousePlaceIn2D(r1); break;
  case funLonHou3: r = RHousePlaceIn3D(r1, r2); break;
  case funLonDist: r = MinDistance(r1, r2); break;
  case funLonDiff: r = MinDifference(r1, r2); break;
  case funLonMid:  r = Midpoint(r1, r2); break;
  case funDayWeek: n = DayOfWeek(n1, n2, n3); break;
  case funJulianT: r = JulianDayFromTime(is.T); break;
  case funSphDist: r = SphDistance(r1, r2, r3, r4); break;
  case funOblique: r = is.OB; break;
  case funRAMC:    r = is.RA; break;
  case funDeltaT:  r = is.rDeltaT * 86400.0; break;
  case funSidDiff: r = is.rOff; break;
  case funNutat:   r = is.rNut; break;
  case funSystem:  n = us.nHouseSystem; break;
  case funAspect:  n = GetAspect(planet, planet, planetalt, planetalt, ret,
    ret, n1, n2, &r); SetReal(n3, r); break;
  case funAspect2: n = GetAspect(cp1.obj, cp2.obj, cp1.alt, cp2.alt, cp1.dir,
    cp2.dir, n1, n2, &r); SetReal(n3, r); break;
  case funParall:  n = GetParallel(planet, planet, planetalt, planetalt,
    retalt, retalt, n1, n2, &r); SetReal(n3, r); break;
  case funParall2: n = GetParallel(cp1.obj, cp2.obj, cp1.alt, cp2.alt,
    cp1.diralt, cp2.diralt, n1, n2, &r); SetReal(n3, r); break;
  case funGridNam: n = grid != NULL && FValidObj(n1) && FValidObj(n2) ?
    grid->n[n1][n2] : 0; break;
  case funGridVal: n = grid != NULL && FValidObj(n1) && FValidObj(n2) ?
    grid->v[n1][n2] : 0; break;
  case funGridDo:  n = FCreateGrid(fFalse); break;
  case funGridDo2: n = FCreateGridRelation(n1 != 0); break;
  case funContext: n = is.nContext; break;
  case funVersion: r = atof(szVersionCore); break;

  // Astrolog command switch settings (general)
  case fun_w1:  n = us.nWheelRows;      break;
  case fun_aj:  n = us.nAspectSort;     break;
  case fun_L1:  n = us.nAstroGraphStep; break;
  case fun_L2:  n = us.nAstroGraphDist; break;
  case fun_d1:  n = us.nDivision;       break;
  case fun_EY:  n = us.nEphemYears;     break;
  case fun_E01: n = us.nEphemRate;      break;
  case fun_E02: n = us.nEphemFactor;    break;
  case fun_P1:  n = us.nArabicParts;    break;
  case fun_N1:  n = us.nAtlasList;      break;
  case fun_I1:  n = us.nScreenWidth;    break;
  case fun_A3:  n = us.fAspect3D;     break;
  case fun_Ap:  n = us.fAspectLat;    break;
  case fun_AP:  n = us.fParallel2;    break;
  case fun_b:   n = us.fEphemeris;    break;
  case fun_b0:  n = us.fSeconds;      break;
  case fun_c:   n = us.nHouseSystem;  break;
  case fun_c3:  n = us.fHouse3D;      break;
  case fun_c31: n = us.nHouse3D;      break;
  case fun_s:   n = us.fSidereal;     break;
  case fun_s0:  n = us.nDegForm;      break;
  case fun_s1:  r = us.rZodiacOffset; break;
  case fun_sr:  n = us.fEquator;      break;
  case fun_sr0: n = us.fEquator2;     break;
  case fun_h:   n = us.objCenter;     break;
  case fun_p:   n = us.fProgress;     break;
  case fun_p0:  n = us.nProgress;     break;
  case fun_pd:  r = us.rProgDay;      break;
  case fun_pC:  r = us.rProgCusp;     break;
  case fun_x:   r = us.rHarmonic;     break;
  case fun_1:   n = us.objOnAsc;      break;
  case fun_3:   n = us.fDecan;        break;
  case fun_4:   n = us.nDwad;         break;
  case fun_f:   n = us.fFlip;         break;
  case fun_G:   n = us.fGeodetic;     break;
  case fun_J:   n = us.fVedic;        break;
  case fun_9:   n = us.fNavamsa;      break;
  case fun_YT:  n = us.fTruePos;    break;
  case fun_YV:  n = us.fTopoPos;    break;
  case fun_Yh:  n = us.fBarycenter; break;
  case fun_Ym:  n = us.fMoonMove;   break;
  case fun_Ys:  n = us.fSidereal2;  break;
  case fun_Yn:  n = us.fTrueNode;   break;
  case fun_Yn0: n = us.fNoNutation; break;
  case fun_Yz0: r = us.rDeltaT;     break;
  case fun_Yu:  n = us.fEclipse;    break;
  case fun_Yu0: n = us.fEclipseAny; break;
  case fun_Yr:  n = us.fRound;      break;
  case fun_YC:  n = us.fSmartCusp;  break;
  case fun_YO:  n = us.fSmartSave;  break;
  case fun_Yc:  n = us.fHouseAngle; break;
  case fun_Yp:  n = us.fPolarAsc;   break;

#ifdef GRAPH
  // Astrolog command switch settings (graphics)
  case fun_XI1: r = gs.rBackPct;    break;
  case fun_XI2: n = gs.nBackOrient; break;
  case fun_Xr:  n = gs.fInverse;    break;
  case fun_Xm:  n = !gs.fColor;     break;
  case fun_XT:  n = gs.fText;       break;
  case fun_Xi:  n = gs.fAlt;        break;
  case fun_Xu:  n = gs.fBorder;     break;
  case fun_Xx:  n = gs.fThick;      break;
  case fun_Xl:  n = gs.fLabel;      break;
  case fun_XA:  n = gs.fLabelAsp;   break;
  case fun_XL:  n = gs.fLabelCity;  break;
  case fun_Xj:  n = gs.fJetTrail;   break;
  case fun_XF:  n = gs.fConstel;    break;
  case fun_XW0: n = gs.fMollewide;  break;
  case fun_Xe:  n = gs.fEquator;    break;
  case fun_XU:  n = gs.fAllStar;    break;
  case fun_XC:  n = gs.fHouseExtra; break;
  case fun_XQ:  n = gs.fKeepSquare; break;
  case fun_XN:  n = gs.fAnimMap;    break;
  case fun_YXe: n = gs.fEcliptic;   break;
  case fun_Xwx: n = gs.xWin;        break;
  case fun_Xwy: n = gs.yWin;        break;
  case fun_Xn:  n = gs.nAnim;       break;
  case fun_Xs:  n = gs.nScale;      break;
  case fun_XS:  n = gs.nScaleText;  break;
  case fun_XU0: n = gs.nAllStar;    break;
  case fun_XE1: n = gs.nAstLo;      break;
  case fun_XE2: n = gs.nAstHi;      break;
  case fun_XE:  n = gs.nAstLabel;   break;
  case fun_XL0: n = gs.nLabelCity;  break;
  case fun_X1:  n = gs.objLeft;     break;
  case fun_Xv:  n = gs.nDecaFill;   break;
  case fun_XGx: r = gs.rRot;        break;
  case fun_XGy: r = gs.rTilt;       break;
  case fun_XZ:  n = gs.objTrack;    break;
  case fun_YXK: n = FValidColor(n1) ? rgbbmp[n1] : 0; break;
#endif

#ifdef WIN
  // Functions related to Windows operations
  case funMouse:
    GetCursorPos(&pt);
    ScreenToClient(wi.hwnd, &pt);
    if (FBetween(n1, 0, cLetter-1)) {
      rgparVar[n1].n   = pt.x; rgparVar[n1].fReal   = fFalse;
      rgparVar[n1+1].n = pt.y; rgparVar[n1+1].fReal = fFalse;
    }
    n = LFromWW(pt.x, pt.y);
    break;

  // Astrolog command switch settings (Windows)
  case fun_WN:  n = wi.nTimerDelay;   break;
  case fun_Wn:  n = wi.fNoUpdate;     break;
  case fun_Wh:  n = wi.fHourglass;    break;
  case fun_Wt:  n = wi.fNoPopup;      break;
  case fun_Wo:  n = wi.fAutoSave;     break;
  case fun_Wo0: n = wi.fAutoSaveNum;  break;
  case fun_Wo3: n = wi.fAutoSaveWire; break;
  case fun_WZ:  n = wi.fSaverRun;     break;
#endif

  // Programming functions (related to variables and control flow)
  case funVar:
    if (FBetween(n1, 0, cLetter)) {
      fRetReal = rgparVar[n1].fReal;
      EIR(rgparVar[n1].n, rgparVar[n1].r);
    } else
      n = 0;
    break;
  case funDo:
    EIR(n2, r2);
    break;
  case funDo2:
    EIR(n3, r3);
    break;
  case funDo3:
    EIR(n4, r4);
    break;
  case funIf:
    fRetReal = fFalse;
    n = (n1 != 0);
    if (n) {
      PchGetParameter(rgpchEval[0], &rgpar[0], ifun, 1, fTrue);
      goto LParseRet;
    }
    break;
  case funIfElse:
    n = (n1 != 0);
    PchGetParameter(rgpchEval[!n], &rgpar[0], ifun, 1, fTrue);
    goto LParseRet;
    break;
  case funDoCount:
    fRetReal = fFalse;
    for (n = 0; n < n1; n++)
      PchGetParameter(rgpchEval[0], &rgpar[0], ifun, 1, fTrue);
    if (n > 0)
      goto LParseRet;
    break;
  case funWhile:
    fRetReal = fFalse;
    loop {
      PchGetParameter(rgpchEval[0], &rgpar[0], ifun, 1, fTrue);
      if (!(rgpar[0].fReal ? rgpar[0].r != 0.0 : rgpar[0].n != 0))
        break;
      PchGetParameter(rgpchEval[1], &rgpar[0], ifun, 1, fTrue);
      fRetReal = rgpar[0].fReal;
      EIR(rgpar[0].n, rgpar[0].r);
    }
    break;
  case funDoWhile:
    do {
      PchGetParameter(rgpchEval[1], &rgpar[0], ifun, 1, fTrue);
      fRetReal = rgpar[0].fReal;
      EIR(rgpar[0].n, rgpar[0].r);
      PchGetParameter(rgpchEval[0], &rgpar[0], ifun, 1, fTrue);
    } while (rgpar[0].fReal ? rgpar[0].r != 0.0 : rgpar[0].n != 0);
    break;
  case funFor:
    if (FBetween(n1, 0, cLetter)) {
      rgparVar[n1] = rgpar[2];
      for (rgparVar[n1].n = n2; rgparVar[n1].n <= n3; rgparVar[n1].n++)
        PchGetParameter(rgpchEval[0], &rgpar[0], ifun, 1, fTrue);
      n = rgparVar[n1].n;
    } else
      n = 0;
    break;
  case funMacro:
    if (FBetween(n1, 0, cLetter) && FSzSet(rgszExpMacro[n1])) {
      GetParameter(rgszExpMacro[n1], &rgpar[0]);
      goto LParseRet;
    } else
      n = 0;
    break;
  case funSwitch:
    n = FValidMacro(n1) && FProcessCommandLine(szMacro[n1-1]);
    break;
  case funRndSeed:
    srand(n1);
    n = n1;
    break;

  // Variable assignment functions
  case funAssign:
  case funAssign2:
    if (FBetween(n1, 0, cLetter)) {
      rgparVar[n1] = rgpar[2];
      fRetReal = rgpar[2].fReal;
      EIR(n2, r2);
    } else
      n = 0;
    break;
  default:
    if (FBetween(ifun, funAssignA, funAssignZ)) {
      rgparVar[ifun - funAssignA + 1] = rgpar[1];
      fRetReal = rgpar[1].fReal;
      EIR(n1, r1);
      break;
    }
    sprintf(sz, "Function %d is undefined.", ifun);
    PrintError(sz);
    return fFalse;
  }

  if (!fRetReal) {
    rgpar[0].n = n;
    rgpar[0].fReal = fFalse;
  } else {
    rgpar[0].r = r;
    rgpar[0].fReal = fTrue;
  }
  return fTrue;

LParseRet:
  fRetReal = rgpar[0].fReal;
  EIR(rgpar[0].n, rgpar[0].r);
  return fTrue;
}


// Read a parameter to an action from a command line, given the current
// position into the command line string. Return the evaluation of the
// parameter in either a string or numeric return variable, or null on error.
// Also update the command line position to point after the parameter.

CONST char *PchGetParameter(CONST char *pchCur, PAR *rgpar, int ifun,
  int iParam, flag fEval)
{
  char sz[cchSzMax*2], szT[cchSzMax], ch1, ch, *pchEdit;
  CONST char *rgpchEval[2], *pchParam, *pchT;
  int ifunT, iParamT, cch, n;
  PAR rgpar2[4];
  flag fSkipEval;

  // Skip whitespace.
  while (*pchCur == ' ')
    pchCur++;
  if (*pchCur == chNull) {
    if (ifun >= 0)
      sprintf(szT, " of function %s", rgfun[ifun].szName);
    else
      *szT = chNull;
    sprintf(sz, "Couldn't get parameter %d%s due to end of line.\n", iParam,
      szT);
    PrintWarning(sz);
    goto LError;
  }

  // Get parameter string.
  for (pchParam = pchCur; *pchCur && *pchCur != ' '; pchCur++)
    ;
  cch = (int)(pchCur - pchParam);
  ch1 = *pchParam;

  // Evaluate the parameter. First check for integer or real number.
  if (FNumCh(ch1) || ((ch1 == '-' || ch1 == '#') && cch > 1)) {
    for (pchT = pchParam; pchT < pchCur; pchT++)
      if (*pchT == '.') {
        rgpar[0].r = atof(pchParam);
        rgpar[0].fReal = fTrue;
        goto LDone;
      }
    rgpar[0].n = LFromRgch(pchParam, cch);
    rgpar[0].fReal = fFalse;
    goto LDone;
  }

  // Check for variable name.
  if (ch1 == '%') {
    ch = ChCap(pchParam[1]);
    if (FCapCh(ch)) {
      rgpar[0].n = ch - '@';
      rgpar[0].fReal = fFalse;
      goto LDone;
    }
    if (FNumCh(ch)) {
      rgpar[0].n = ch - '0';
      rgpar[0].fReal = fFalse;
      goto LDone;
    }
  }

  // Check for variable value.
  if (ch1 == '@') {
    ch = ChCap(pchParam[1]);
    if (FCapCh(ch)) {
      rgpar[0] = rgparVar[ch - '@'];
      goto LDone;
    }
    if (FNumCh(ch)) {
      rgpar[0] = rgparVar[ch - '0'];
      goto LDone;
    }
  }

  // Check for named constants.
  if (ch1 != chNull && pchParam[1] == '_') {
    for (pchT = pchParam+2, n = 0; *pchT && *pchT > ' '; pchT++, n++)
      szT[n] = *pchT;
    szT[n] = chNull;
    switch (ChCap(ch1)) {
    case 'M': n = NParseSz(szT, pmMon);    break;
    case 'O': n = NParseSz(szT, pmObject); break;
    case 'A': n = NParseSz(szT, pmAspect); break;
    case 'H': n = NParseSz(szT, pmSystem); break;
    case 'S': n = NParseSz(szT, pmSign);   break;
    case 'K': n = NParseSz(szT, pmColor);  break;
    case 'W': n = NParseSz(szT, pmWeek);   break;
    default: n = -1;
    }
    if (n >= 0) {
      rgpar[0].n = n;
      rgpar[0].fReal = fFalse;
      goto LDone;
    }
  }

  // Check for function.
  ifunT = ILookupTrie(is.rgsTrieFun, pchParam, cch, fTrue);
  if (ifunT >= 0) {
    rgpchEval[0] = rgpchEval[1] = NULL;

    // Recursively get the parameters to the function.
    for (iParamT = 1; iParamT <= rgfun[ifunT].nParam; iParamT++) {
      fSkipEval = fFalse;
      // Some parameters shouldn't be evaluated yet, but just skipped over.
      if (((ifunT == funIf || ifunT == funIfElse || ifunT == funDoCount) &&
          iParamT == 2) ||
        ((ifunT == funWhile || ifunT == funDoWhile) && iParamT == 1) ||
        (ifunT == funFor && iParamT == 4)) {
        fSkipEval = fTrue;
        rgpchEval[0] = pchCur;
      } else if ((ifunT == funIfElse && iParamT == 3) ||
        ((ifunT == funWhile || ifunT == funDoWhile) && iParamT == 2)) {
        fSkipEval = fTrue;
        rgpchEval[1] = pchCur;
      }
      pchCur = PchGetParameter(pchCur, &rgpar2[iParamT], ifunT, iParamT,
        fEval && !fSkipEval);
      if (pchCur == NULL)
        return NULL;
    }
    if (fEval) {
      if (!FEvalFunction(ifunT, rgpar2, (char **)rgpchEval))
        return NULL;
      rgpar[0] = rgpar2[0];
    }
    goto LDone;
  }

  // Error if can't parse contents.
  CopyRgchToSz(pchParam, cch, szT, cchSzMax);
  sprintf(sz, "Unknown parameter: '%s'\nContext: '", szT);
  for (pchEdit = sz; *pchEdit; pchEdit++)
    ;
  pchT = pchCur;
  while (*pchT && pchEdit - sz < cchSzMax)
    *pchEdit++ = *pchT++;
  if (pchEdit - sz < cchSzMax)
    *pchEdit++ = '\'';
  *pchEdit = chNull;
  PrintWarning(sz);
LError:
  us.fExpOff = fTrue;
  return NULL;

LDone:
  return pchCur;
}


// Like PchGetParameter() but parse multiple expressions in sequence in a
// string, placing the value of the last expressions within parameter par.

void GetParameter(CONST char *sz, PAR *ppar)
{
  CONST char *pch = sz;

  do {
    pch = PchGetParameter(pch, ppar, -1, 1, fTrue);
  } while (pch != NULL && *pch != chNull);
}


/*
******************************************************************************
** Trie Tree Routines
******************************************************************************
*/

// Create a trie tree within the buffer rgsOut of size csTrieMax. Returns the
// size of the trie in shorts, or -1 on failure. The input list of strings is
// used to create the trie. It does not have to be in sorted order. Trie
// lookups on strings will return the index they are in the original list. No
// two strings should be the same. Individual strings shouldn't be longer than
// 255 characters, or rather trailing unique substrings can't be longer than
// 255. With signed 16 bit indexes, these tries can't be larger than 32K
// shorts, or 64K bytes. That can store about 2500 strings.
//
// The trie format is a sequence of nodes. "Pointers" to other nodes are
// indexes into the array of shorts. There are two types of nodes: Standard
// nodes which handle all the branches from a particular leading substring,
// and leaf nodes for unique trailing substrings. Standard node format:
//
// Short 0: 0 = No string ends at this point. Non-zero = Payload + 1 for
//   substring leading up to this node.
// Short 1: High byte = Highest character covered by this node. Low byte =
//   Lowest character covered. If high character is 0, this is a leaf node.
// Short 2+: Pointers to nodes for the range of low through high characters.
//   0 = null pointer. Less than 0 means a string ends with this character,
//   where value contains -(Payload + 1).
//
// Leaf node format:
// Short 0: Same as standard node.
// Short 1: Length of trailing substring.
// Short 2: Payload for the string ending after this point.
// Short 3+: Range of characters storing final substring to compare against.

int CsCreateTrie(CONST uchar *rgszIn[], int cszIn, TRIE rgsOut, int csTrieMax,
  flag fInsensitive)
{
  uchar rgchStack[cchSzMax];
  int rgisStack[cchSzMax];
  long rgchUsed[256], iUsed = 0;
  int iStack, csOut = 0, isz, ich, chLo, chHi, chT, is, cch, isT,
    isRemember = 0, csz, iszSav;

  for (ich = 0; ich < 256; ich++)
    rgchUsed[ich] = 0;

  for (iStack = 0;; iStack++) {
    if (iStack >= cchSzMax) {
      Assert(fFalse);
      return -1;
    }
    rgisStack[iStack] = csOut;
    chLo = 255, chHi = 0, iUsed++, csz = 0;

    // Count how many strings match the current leading substring. Also
    // get the low and high character for these strings.
    for (isz = 0; isz < cszIn; isz++) {
      if (FEqRgch((CONST char *)rgszIn[isz], (CONST char *)rgchStack, iStack,
        fInsensitive)) {
        chT = rgszIn[isz][iStack];
        if (chT != 0) {
          if (fInsensitive)
            chT = ChCap(chT);
          csz++;
          iszSav = isz;
          rgchUsed[chT] = iUsed;
          if (chT < chLo)
            chLo = chT;
          if (chT > chHi)
            chHi = chT;
        }
      }
    }

    // If no strings match, back up to an earlier node.
    if (csz <= 0) {
LPop:
      loop {
        // Pop the stack to the parent node.
        iStack--;
        if (iStack < 0)
          goto LDone;
        is = rgisStack[iStack];
        chLo = (word)rgsOut[is + 1] & 255;
        chHi = (word)rgsOut[is + 1] >> 8;

        // Scan for a pointer that hasn't been filled out yet.
        for (ich = chLo + 1; ich <= chHi; ich++) {
          if (rgsOut[is + 2 + (ich - chLo)] == 1) {
            chT = ich;
            goto LPush;
          }
        }
      }
    }

    // Since there's at least one string, there will be a new node. Set the
    // pointer in the parent node to here.
    rgsOut[isRemember] = csOut;

    if (csOut >= csTrieMax - 1) {
      Assert(fFalse);
      return -1;
    }
    rgsOut[csOut++] = 0;  // Short 0

    // If exactly one string matches, create a leaf node.
    if (csz == 1) {
      for (ich = iStack; rgszIn[iszSav][ich] != 0; ich++)
        ;
      cch = ich - iStack;
      if (cch > 255) {
        Assert(fFalse);
        return -1;
      }
      if (csOut >= csTrieMax - 2 - ((cch + 1) >> 1)) {
        Assert(fFalse);
        return -1;
      }
      rgsOut[csOut++] = cch;  // Short 1
      rgsOut[csOut++] = 0;    // Short 2
      CopyRgb((byte *)&rgszIn[iszSav][iStack], (byte *)&rgsOut[csOut],
        (cch + 1) & ~1);
      if (fInsensitive)
        UpperRgch((char *)&rgsOut[csOut], cch);
      csOut += (cch + 1) >> 1;
      goto LPop;
    }

    // Create a standard node.
    if (csOut >= csTrieMax - 1 - (chHi - chLo + 1)) {
      Assert(fFalse);
      return -1;
    }
    rgsOut[csOut++] = (chHi << 8) | chLo;  // Short 1
    // Set those characters in use to the temporary pointer value 1, which
    // will be filled out later.
    for (ich = chLo; ich <= chHi; ich++)
      rgsOut[csOut++] = (rgchUsed[ich] == iUsed);
    chT = chLo;

LPush:
    rgchStack[iStack] = chT;
    isRemember = rgisStack[iStack] + 2 + (chT - chLo);
    rgsOut[isRemember] = -1;
  }

LDone:
  // Fill out payloads. For each string in the input list, walk the trie and
  // set the appropriate point in it to the string's index in the input list.
  Assert(csOut != 0);
  for (isz = 0; isz < cszIn; isz++) {
    is = 0;
    for (ich = 0;; ich++) {
      if (rgszIn[isz][ich] == chNull) {
        // Handle the substring case (short 0).
        Assert(rgsOut[is] == 0 || !fInsensitive);
        rgsOut[is] = isz+1;
        break;
      }
      chLo = (word)rgsOut[is + 1] & 255;
      chHi = (word)rgsOut[is + 1] >> 8;
      if (chHi == 0) {
        // Handle the leaf node case (short 2).
        Assert(rgsOut[is + 2] == 0 && FEqRgch((char *)&rgsOut[is+3],
          (char *)&rgszIn[isz][ich], chLo, fInsensitive));
        rgsOut[is + 2] = isz;
        break;
      }
      chT = rgszIn[isz][ich];
      if (fInsensitive)
        chT = ChCap(chT);
      Assert(chT >= chLo && chT <= chHi);
      isT = rgsOut[is + 2 + (chT - chLo)];
      Assert(isT != 0);
      if (isT <= 0) {
        // Handle the payload pointer within standard node case.
        Assert(rgszIn[isz][ich + 1] == chNull);
        rgsOut[is + 2 + (chT - chLo)] = -(isz+1);
        break;
      }
      is = isT;
    }
  }

  return csOut;
}


// Lookup a string in a trie created with CsCreateTrie. Return -1 if string
// not found, otherwise return the index of the string in the original list
// used to create the trie. This lookup is very fast, and not much slower than
// a single string compare. For strings not in the list, usually don't even
// have to look at all of its characters before knowing it's not in the trie.

int ILookupTrie(CONST TRIE rgsIn, CONST char *rgch, int cch,
  flag fInsensitive)
{
  int is = 0, chLo, chHi, ch, cchT;
  CONST char *pchEnd = rgch + cch, *pch, *pch1, *pch2;

  // Walk the input string, while going from node to node in the trie.
  for (pch = rgch;; pch++) {
    if (pch >= pchEnd) {

      // At end of input string. Check whether current node has a payload.
      if (rgsIn[is] != 0)
        return rgsIn[is]-1;
      else
        return -1;
    }
    chLo = (word)rgsIn[is + 1];
    chHi = chLo >> 8;
    if (chHi == 0) {

      // Leaf node. Compare rest of input string with substring in node.
      pch1 = (char *)&rgsIn[is + 3], pch2 = pch, cchT = chLo;
      if (!fInsensitive) {
        while (cchT > 0 && *pch1 == *pch2) {
          Assert(*pch1 != chNull);
          pch1++, pch2++, cchT--;
        }
      } else {
        while (cchT > 0 && *pch1 == ChCap(*pch2)) {
          Assert(*pch1 != chNull);
          pch1++, pch2++, cchT--;
        }
      }
      if (cchT > 0 || pch + chLo < pchEnd)
        return -1;
      else
        return rgsIn[is + 2];
    }

    // Standard node. Get pointer to appropriate child node.
    chLo &= 255;
    ch = *pch;
    if (fInsensitive)
      ch = ChCap(ch);
    if (ch < chLo || ch > chHi)
      return -1;
    is = rgsIn[is + 2 + (ch - chLo)];
    if (is < 0) {
      if (pch + 1 == pchEnd)
        return -is-1;
      else
        return -1;
    } else if (is == 0)
      return -1;
  }
}


// Create trie tree lookup tables for the various action lists. This work is
// only done once, right before the first lookup is needed.

flag FCreateTries()
{
#ifdef DEBUG
  char sz[cchSzDef];
#endif
  char *rgsz[cfun];
  int csMax = 16000, isz, cs;

  is.rgsTrieFun = RgAllocate(csMax, short, "trie");
  if (is.rgsTrieFun == NULL)
    return fFalse;

  for (isz = 0; isz < cfun; isz++)
    rgsz[isz] = rgfun[isz].szName;
  cs = CsCreateTrie((CONST uchar **)rgsz, cfun, is.rgsTrieFun, csMax, fTrue);
  csMax -= cs;

#ifdef DEBUG
  // Sanity check to ensure the tries are valid. For each string, look it up
  // in its trie and see if it returns the expected index.

  for (isz = 0; isz < cfun; isz++) {
    cs = ILookupTrie(is.rgsTrieFun,
      rgfun[isz].szName, CchSz(rgfun[isz].szName), fTrue);
    if (cs != isz) {
      sprintf(sz, "Function string %d (%s) maps to trie value %d.",
        isz, rgfun[isz].szName, cs);
      PrintError(sz);
      break;
    }
    if (rgfun[isz].ifun != isz) {
      sprintf(sz, "Function %d (%s) defined in slot %d.",
        rgfun[isz].ifun, rgfun[isz].szName, isz);
      PrintError(sz);
      break;
    }
  }
#endif
  return fTrue;
}


/*
******************************************************************************
** Command Line Processing
******************************************************************************
*/

// Parse an arbitrary integer expression, and return its result.

long NParseExpression(CONST char *sz)
{
  PAR par = {0, 0.0, fFalse};  // If parsing fails, assume 0.

  // Ensure the lookup tables have been created.
  if (us.fNoExp || us.fExpOff || (is.rgsTrieFun == NULL && !FCreateTries()))
    return par.n;

  GetParameter(sz, &par);
  return !par.fReal ? par.n : (long)par.r;
}


// Parse an arbitrary real expression, and return its result.

real RParseExpression(CONST char *sz)
{
  PAR par = {0, 0.0, fFalse};  // If parsing fails, assume 0.

  // Ensure the lookup tables have been created.
  if (us.fNoExp || us.fExpOff || (is.rgsTrieFun == NULL && !FCreateTries()))
    return par.r;

  GetParameter(sz, &par);
  return par.fReal ? par.r : (real)par.n;
}


// Parse an arbitrary integer or real expression, and display its result.

flag ShowParseExpression(CONST char *sz)
{
  PAR par = {0, 0.0, fFalse};  // If parsing fails, assume 0.
  char szMsg[cchSzMax], szNum[cchSzDef];

  // Ensure the lookup tables have been created.
  if (us.fNoExp || (is.rgsTrieFun == NULL && !FCreateTries()))
    return fFalse;

  GetParameter(sz, &par);
  if (!par.fReal)
    sprintf(szNum, "%d", par.n);
  else
    FormatR(szNum, par.r, 6);
  sprintf(szMsg, "Expression returned: %s\n", szNum);
  PrintNotice(szMsg);
  return fTrue;
}


// Get a parameter in integer format.

int NExpGet(int i)
{
  return !rgparVar[i].fReal ? rgparVar[i].n : (int)rgparVar[i].r;
}


// Get a parameter in real format.

real RExpGet(int i)
{
  return rgparVar[i].fReal ? rgparVar[i].r : (real)rgparVar[i].n;
}


// Set a parameter to an integer value.

void ExpSetN(int i, int n)
{
  rgparVar[i].fReal = fFalse;
  rgparVar[i].n = n;
}


// Set a parameter to a real value.

void ExpSetR(int i, real r)
{
  rgparVar[i].fReal = fTrue;
  rgparVar[i].r = r;
}


// Set a string to an AstroExpression custom variable.

char *PchFormatExpression(char *sz, int i)
{
  char *pch;

  if (rgparVar[i].fReal)
    sprintf(sz, "%lf", rgparVar[i].r);
  else
    sprintf(sz, "%d", rgparVar[i].n);
  for (pch = sz; *pch; pch++)
    ;
  return pch;
}


// Initialize all custom variables to integer zero.

void ExpInit(void)
{
  ClearB((pbyte)rgparVar, sizeof(rgparVar));
}
#endif // EXPRESS

/* express.cpp */
