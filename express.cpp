/*
** Astrolog (Version 7.00) File: express.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2020 by
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
** Last code change made 6/4/2020.
*/

#include "astrolog.h"


#ifdef EXPRESS
/*
******************************************************************************
** Action Tables
******************************************************************************
*/

#define cfun 149

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
#define I_IXX  P4(I_, I_, X_, X_)
#define I_IIIX P5(I_, I_, I_, I_, X_)
#define I_R    P2(I_, R_)
#define I_EE   P3(I_, E_, E_)
#define R_I    P2(R_, I_)
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

typedef struct _function {
  int ifun;     // Index of function
  char *szName; // Name of function
  int nParam;   // Number of parameters function takes
  int nType;    // Types of return value and each parameter
} FUN;

typedef struct _parameter {
  int n;      // Integer value (if !fReal)
  real r;     // Real value (if fReal)
  flag fReal; // Whether parameter is integer or real
} PAR;

PAR rgparVar[cLetter+1];       // List of variables
char *rgszExpMacro[cLetter+1]; // List of macros

extern CONST char *PchGetParameter P((CONST char *, PAR *, int, flag));

// Functions

enum _functionindex {
  funFalse = 0,
  funTrue,
  funInt,
  funReal,
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
  funMin,
  funMax,
  funIfOp,
  funSqu,
  funSqr,
  funDist,
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

  funSigns,
  funObjs,
  funAsps,
  funMon,
  funDay,
  funYea,
  funTim,
  funDst,
  funZon,
  funLon,
  funLat,
  funMon2,
  funDay2,
  funYea2,
  funTim2,
  funDst2,
  funZon2,
  funLon2,
  funLat2,
  funMonT,
  funDayT,
  funYeaT,
  funTimT,
  funDayWeek,
  funObjLon,
  funObjLat,
  funObjDir,
  funObjDirY,
  funObjHou,
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
  funAspAng,
  funAspOrb,
  funAspInf,
  funAspCol,
  funCusp,
  funCusp3,
  funCuspInf,
  funLonSign,
  funLonDeg,
  funLonDist,
  funLonDiff,
  funLonMid,
  funPolDist,
  funSystem,
  funGridNam,
  funGridVal,

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

  funAssign,  funAssign2,
  funAssignA, funAssignB, funAssignC, funAssignD, funAssignE,
  funAssignF, funAssignG, funAssignH, funAssignI, funAssignJ,
  funAssignK, funAssignL, funAssignM, funAssignN, funAssignO,
  funAssignP, funAssignQ, funAssignR, funAssignS, funAssignT,
  funAssignU, funAssignV, funAssignW, funAssignX, funAssignY,
  funAssignZ = cfun-1,
};

CONST FUN rgfun[cfun] = {
{funFalse, "False", 0, I_},
{funTrue,  "True",  0, I_},
{funInt,   "Int",   1, I_I},
{funReal,  "Real",  1, R_R},
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
{funMin,   "Min",   2, E_EE},
{funMax,   "Max",   2, E_EE},
{funIfOp,  "?:",    3, E_IEE},
{funSqu,   "Squ",   1, E_E},
{funSqr,   "Sqr",   1, R_R},
{funDist,  "Dist",  2, R_RR},
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
{funMon2,    "Mon2",     0, I_},
{funDay2,    "Day2",     0, I_},
{funYea2,    "Yea2",     0, I_},
{funTim2,    "Tim2",     0, R_},
{funDst2,    "Dst2",     0, R_},
{funZon2,    "Zon2",     0, R_},
{funLon2,    "Lon2",     0, R_},
{funLat2,    "Lat2",     0, R_},
{funMonT,    "MonT",     0, I_},
{funDayT,    "DayT",     0, I_},
{funYeaT,    "YeaT",     0, I_},
{funTimT,    "TimT",     0, R_},
{funDayWeek, "DayWeek",  3, I_III},
{funObjLon,  "ObjLon",   1, R_I}, 
{funObjLat,  "ObjLat",   1, R_I},
{funObjDir,  "ObjDir",   1, R_I},
{funObjDirY, "ObjDirY",  1, R_I},
{funObjHou,  "ObjHouse", 1, I_I},
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
{funAspAng,  "AspAngle", 1, R_I},
{funAspOrb,  "AspOrb",   1, R_I},
{funAspInf,  "AspInf",   1, R_I},
{funAspCol,  "AspCol",   1, I_I},
{funCusp,    "Cusp",     1, R_I},
{funCusp3,   "Cusp3",    1, R_I},
{funCuspInf, "HouseInf", 1, R_I},
{funLonSign, "LonSign",  1, I_R},
{funLonDeg,  "LonDeg",   1, R_R},
{funLonDist, "LonDist",  2, R_RR},
{funLonDiff, "LonDiff",  2, R_RR},
{funLonMid,  "LonMid",   2, R_RR},
{funPolDist, "PolDist",  4, R_RRRR},
{funSystem,  "HouseSys", 0, I_},
{funGridNam, "GridNam",  2, I_II},
{funGridVal, "GridVal",  2, I_II},

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

{funAssign,  "Assign",  2, E_IE},
{funAssign2, "=",       2, E_IE},
{funAssignA, "=A",      1, E_E},
{funAssignB, "=B",      1, E_E},
{funAssignC, "=C",      1, E_E},
{funAssignD, "=D",      1, E_E},
{funAssignE, "=E",      1, E_E},
{funAssignF, "=F",      1, E_E},
{funAssignG, "=G",      1, E_E},
{funAssignH, "=H",      1, E_E},
{funAssignI, "=I",      1, E_E},
{funAssignJ, "=J",      1, E_E},
{funAssignK, "=K",      1, E_E},
{funAssignL, "=L",      1, E_E},
{funAssignM, "=M",      1, E_E},
{funAssignN, "=N",      1, E_E},
{funAssignO, "=O",      1, E_E},
{funAssignP, "=P",      1, E_E},
{funAssignQ, "=Q",      1, E_E},
{funAssignR, "=R",      1, E_E},
{funAssignS, "=S",      1, E_E},
{funAssignT, "=T",      1, E_E},
{funAssignU, "=U",      1, E_E},
{funAssignV, "=V",      1, E_E},
{funAssignW, "=W",      1, E_E},
{funAssignX, "=X",      1, E_E},
{funAssignY, "=Y",      1, E_E},
{funAssignZ, "=Z",      1, E_E},
};


/*
******************************************************************************
** Utilities
******************************************************************************
*/

// Copy a range of characters and zero terminate it. If there are too many
// characters to fit in the destination buffer, the string is truncated.

void CopyRgchToSz(CONST char *pch, int cch, char *sz, int cchMax)
{
  cch = Min(cch, cchMax-1);
  CopyRgb((byte *)pch, (byte *)sz, cch);
  sz[cch] = chNull;
}


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
  case funFalse: n = fFalse; break;
  case funTrue:  n = fTrue;  break;
  case funInt:   n = n1;     break;
  case funReal:  r = r1;     break;
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
  case funMin:   EIR(Min(n1, n2), Min(r1, r2)); break;
  case funMax:   EIR(Max(n1, n2), Max(r1, r2)); break;
  case funIfOp:  EIR(n1 ? n2 : n3, n1 ? r2 : r3); break;
  case funSqu:   EIR(Sq(n1), Sq(r1)); break;
  case funSqr:   r = RSqr(r1); break;
  case funDist:  r = RSqr(Sq(r1)+Sq(r2)); break;
  case funSin:   r = RSin(r1); break;
  case funCos:   r = RCos(r1); break;
  case funTan:   r = RTan(r1); break;
  case funAsin:  r = RAsin(r1); break;
  case funAcos:  r = RAcos(r1); break;
  case funAtan:  r = RAtn(r1);  break;
  case funAng:   r = Angle(r1, r2); break;
  case funSinD:  r = RSinD(r1); break;
  case funCosD:  r = RCosD(r1); break;
  case funTanD:  r = RTanD(r1); break;
  case funAsinD: r = RAsinD(r1); break;
  case funAcosD: r = RAcosD(r1); break;
  case funAtanD: r = RAtnD(r1);  break;
  case funAngD:  r = DFromR(Angle(r1, r2)); break;
  case funFloor: r = RFloor(r1); break;
  case funFract: r = r1 - RFloor(r1); break;
  case funRnd:   n = (rand() & 16383) * (n2 - n1 + 1) / 16384 + n1; break;

  case funSigns:   n = cSign;   break;
  case funObjs:    n = cObj;    break;
  case funAsps:    n = us.nAsp; break;
  case funMon:     n = MM; break;
  case funDay:     n = DD; break;
  case funYea:     n = YY; break;
  case funTim:     r = TT; break;
  case funDst:     r = SS; break;
  case funZon:     r = ZZ; break;
  case funLon:     r = OO; break;
  case funLat:     r = AA; break;
  case funMon2:    n = ciTwin.mon; break;
  case funDay2:    n = ciTwin.day; break;
  case funYea2:    n = ciTwin.yea; break;
  case funTim2:    r = ciTwin.tim; break;
  case funDst2:    r = ciTwin.dst; break;
  case funZon2:    r = ciTwin.zon; break;
  case funLon2:    r = ciTwin.lon; break;
  case funLat2:    r = ciTwin.lat; break;
  case funMonT:    n = ciTran.mon; break;
  case funDayT:    n = ciTran.day; break;
  case funYeaT:    n = ciTran.yea; break;
  case funTimT:    r = ciTran.tim; break;
  case funDayWeek: n = DayOfWeek(n1, n2, n3); break;
  case funObjLon:  r = FValidObj(n1)    ? planet[n1]    : 0.0; break;
  case funObjLat:  r = FValidObj(n1)    ? planetalt[n1] : 0.0; break;
  case funObjDir:  r = FValidObj(n1)    ? ret[n1]       : 0.0; break;
  case funObjDirY: r = FValidObj(n1)    ? retalt[n1]    : 0.0; break;
  case funObjHou:  n = FValidObj(n1)    ? inhouse[n1]   : 0;   break;
  case funObjX:    r = FValidObj(n1)    ? space[n1].x   : 0.0; break;
  case funObjY:    r = FValidObj(n1)    ? space[n1].y   : 0.0; break;
  case funObjZ:    r = FValidObj(n1)    ? space[n1].z   : 0.0; break;
  case funObjOn:   n = FValidObj(n1)    ? !ignore[n1]   : 0;   break;
  case funObjOnT:  n = FValidObj(n1)    ? !ignore2[n1]  : 0;   break;
  case funObjOrb:  r = FValidObj(n1)    ? rObjOrb[n1]   : 0.0; break;
  case funObjAdd:  r = FValidObj(n1)    ? rObjAdd[n1]   : 0.0; break;
  case funObjInf:  r = FBetween(n1, 0, oNorm1+5) ? rObjInf[n1]   : 0.0; break;
  case funObjInfT: r = FBetween(n1, 0, oNorm1) ? rTransitInf[n1] : 0.0; break;
  case funObjCol:  n = FValidObj(n1)    ? kObjB[n1]     : 0;   break;
  case funObjDist: r = FNorm(n1)        ? rObjDist[n1]  : 0.0; break;
  case funObjYear: r = FNorm(n1)        ? rObjYear[n1]  : 0.0; break;
  case funObjDiam: r = FBetween(n1, 0, oVes) ? rObjDiam[n1] : 0.0; break;
  case funAspAng:  r = FValidAspect(n1) ? rAspAngle[n1] : 0.0; break;
  case funAspOrb:  r = FValidAspect(n1) ? rAspOrb[n1]   : 0.0; break;
  case funAspInf:  r = FValidAspect(n1) ? rAspInf[n1]   : 0.0; break;
  case funAspCol:  n = FValidAspect(n1) ? kAspB[n1]     : 0;   break;
  case funCusp:    r = FValidSign(n1)   ? chouse[n1]    : 0.0; break;
  case funCusp3:   r = FValidSign(n1)   ? chouse3[n1]   : 0.0; break;
  case funCuspInf: r = FBetween(n1, 1, cSign+5) ? rHouseInf[n1] : 0.0; break;
  case funLonSign: n = SFromZ(r1); break;
  case funLonDeg:  r = r1 - (real)((SFromZ(r1)-1)*30); break;
  case funLonDist: r = MinDistance(r1, r2); break;
  case funLonDiff: r = MinDifference(r1, r2); break;
  case funLonMid:  r = Midpoint(r1, r2); break;
  case funPolDist: r = PolarDistance(r1, r2, r3, r4); break;
  case funSystem:  n = us.nHouseSystem; break;
  case funGridNam: n = grid != NULL && FValidObj(n1) && FValidObj(n2) ?
    grid->n[n1][n2] : 0; break;
  case funGridVal: n = grid != NULL && FValidObj(n1) && FValidObj(n2) ?
    grid->v[n1][n2] : 0; break;

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
      PchGetParameter(rgpchEval[0], &rgpar[0], 1, fTrue);
      goto LParseRet;
    }
    break;
  case funIfElse:
    n = (n1 != 0);
    PchGetParameter(rgpchEval[!n], &rgpar[0], 1, fTrue);
    goto LParseRet;
    break;
  case funDoCount:
    fRetReal = fFalse;
    for (n = 0; n < n1; n++)
      PchGetParameter(rgpchEval[0], &rgpar[0], 1, fTrue);
    if (n > 0)
      goto LParseRet;
    break;
  case funWhile:
    fRetReal = fFalse;
    loop {
      PchGetParameter(rgpchEval[0], &rgpar[0], 1, fTrue);
      if (!(rgpar[0].fReal ? rgpar[0].r != 0.0 : rgpar[0].n != 0))
        break;
      PchGetParameter(rgpchEval[1], &rgpar[0], 1, fTrue);
      fRetReal = rgpar[0].fReal;
      EIR(rgpar[0].n, rgpar[0].r);
    }
    break;
  case funDoWhile:
    do {
      PchGetParameter(rgpchEval[1], &rgpar[0], 1, fTrue);
      fRetReal = rgpar[0].fReal;
      EIR(rgpar[0].n, rgpar[0].r);
      PchGetParameter(rgpchEval[0], &rgpar[0], 1, fTrue);
    } while (rgpar[0].fReal ? rgpar[0].r != 0.0 : rgpar[0].n != 0);
    break;
  case funFor:
    if (FBetween(n1, 0, cLetter)) {
      rgparVar[n1] = rgpar[2];
      for (rgparVar[n1].n = n2; rgparVar[n1].n <= n3; rgparVar[n1].n++)
        PchGetParameter(rgpchEval[0], &rgpar[0], 1, fTrue);
      n = rgparVar[n1].n;
    } else
      n = 0;
    break;
  case funMacro:
    if (FBetween(n1, 0, cLetter) && FSzSet(rgszExpMacro[n1])) {
      PchGetParameter(rgszExpMacro[n1], &rgpar[0], 1, fTrue);
      goto LParseRet;
    } else
      n = 0;
    break;

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

CONST char *PchGetParameter(CONST char *pchCur, PAR *rgpar, int iParam,
  flag fEval)
{
  char sz[cchSzMax*2], szT[cchSzMax], ch1, ch, *pchEdit;
  CONST char *rgpchEval[2], *pchParam, *pchT;
  int ifun, iParamT, cch, n;
  PAR rgpar2[4];
  flag fSkipEval;

  // Skip whitespace.
  while (*pchCur == ' ')
    pchCur++;
  if (*pchCur == chNull) {
    sprintf(sz, "Couldn't get parameter %d due to end of line.\n", iParam);
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
  ifun = ILookupTrie(is.rgsTrieFun, pchParam, cch, fTrue);
  if (ifun >= 0) {
    rgpchEval[0] = rgpchEval[1] = NULL;

    // Recursively get the parameters to the function.
    for (iParamT = 1; iParamT <= rgfun[ifun].nParam; iParamT++) {
      fSkipEval = fFalse;
      // Some parameters shouldn't be evaluated yet, but just skipped over.
      if (((ifun == funIf || ifun == funIfElse || ifun == funDoCount) &&
          iParamT == 2) ||
        ((ifun == funWhile || ifun == funDoWhile) && iParamT == 1) ||
        (ifun == funFor && iParamT == 4)) {
        fSkipEval = fTrue;
        rgpchEval[0] = pchCur;
      } else if ((ifun == funIfElse && iParamT == 3) ||
        ((ifun == funWhile || ifun == funDoWhile) && iParamT == 2)) {
        fSkipEval = fTrue;
        rgpchEval[1] = pchCur;
      }
      pchCur = PchGetParameter(pchCur, &rgpar2[iParamT], iParamT,
        fEval && !fSkipEval);
      if (pchCur == NULL)
        return NULL;
    }
    if (fEval) {
      if (!FEvalFunction(ifun, rgpar2, (char **)rgpchEval))
        return NULL;
      rgpar[0] = rgpar2[0];
    }
    goto LDone;
  }

  // Error if can't parse contents.
  CopyRgchToSz(pchParam, cch, szT, cchSzMax);
  sprintf(sz, "Unknown parameter: '%s'\nContext: ", szT);
  for (pchEdit = sz; *pchEdit; pchEdit++)
    ;
  pchT = pchCur;
  while (*pchT && pchEdit - sz < cchSzMax)
    *pchEdit++ = *pchT++;
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
    pch = PchGetParameter(pch, ppar, 1, fTrue);
  } while (pch != NULL && *pch != chNull);
}


/*
******************************************************************************
** Trie Tree Routines
******************************************************************************
*/

// Return whether two ranges of characters are equal. Either string ending
// prematurely with a zero terminator makes the strings not equal.

flag FEqRgch(CONST char *rgch1, CONST char *rgch2, int cch, flag fInsensitive)
{
  int ich;

  if (!fInsensitive) {
    for (ich = 0; ich < cch; ich++) {
      if (rgch1[ich] == '\0' || rgch1[ich] != rgch2[ich])
        return fFalse;
    }
  } else {
    for (ich = 0; ich < cch; ich++) {
      if (rgch1[ich] == '\0' || ChCap(rgch1[ich]) != ChCap(rgch2[ich]))
        return fFalse;
    }
  }
  return fTrue;
}


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
      if (rgszIn[isz][ich] == '\0') {
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
        Assert(rgszIn[isz][ich + 1] == '\0');
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
          Assert(*pch1 != '\0');
          pch1++, pch2++, cchT--;
        }
      } else {
        while (cchT > 0 && *pch1 == ChCap(*pch2)) {
          Assert(*pch1 != '\0');
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
      sprintf(sz, "Function string %d maps to trie value %d.", isz, cs);
      PrintError(sz);
      break;
    }
    if (rgfun[isz].ifun != isz) {
      sprintf(sz, "Function %d defined in slot %d.", rgfun[isz].ifun, isz);
      PrintError(sz);
      break;
    }
  }
#endif

  ClearB((pbyte)rgparVar, sizeof(rgparVar));
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


// Parse an arbitrary integer or real expression.

flag FParseExpression(CONST char *sz)
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
  if (!us.fExpOff) {
    sprintf(szMsg, "Expression returned: %s\n", szNum);
    PrintNotice(szMsg);
  }
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
#endif /* EXPRESS */

/* express.cpp */
