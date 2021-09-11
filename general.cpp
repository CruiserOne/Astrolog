/*
** Astrolog (Version 7.30) File: general.cpp
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


/*
******************************************************************************
** General Procedures.
******************************************************************************
*/

// Swap two floating point values.

void SwapR(real *d1, real *d2)
{
  real temp;

  temp = *d1; *d1 = *d2; *d2 = temp;
}


// Return the length of a string (not counting the null terminator).

int CchSz(CONST char *sz)
{
  CONST char *pch = sz;

  while (*pch)
    pch++;
  return (int)(pch - sz);
}


// Compare two strings case sensitively. Return 0 if equal, negative number if
// first less than second, and positive number if first greater than second.

int NCompareSz(CONST char *sz1, CONST char *sz2)
{
  while (*sz1 && *sz1 == *sz2)
    sz1++, sz2++;
  return (int)*sz1 - *sz2;
}


// Compare two strings case insensitively. Return 0 if equal, negative number
// if first less than second, and positive if first greater than second.

int NCompareSzI(CONST char *sz1, CONST char *sz2)
{
  while (*sz1 && ChCap(*sz1) == ChCap(*sz2))
    sz1++, sz2++;
  return (int)ChCap(*sz1) - ChCap(*sz2);
}


// Return whether two ranges of characters are equal. Either string ending
// prematurely with a zero terminator makes the strings not equal.

flag FEqRgch(CONST char *rgch1, CONST char *rgch2, int cch, flag fInsensitive)
{
  int ich;

  if (!fInsensitive) {
    for (ich = 0; ich < cch; ich++) {
      if (rgch1[ich] == chNull || rgch1[ich] != rgch2[ich])
        return fFalse;
    }
  } else {
    for (ich = 0; ich < cch; ich++) {
      if (rgch1[ich] == chNull || ChCap(rgch1[ich]) != ChCap(rgch2[ich]))
        return fFalse;
    }
  }
  return fTrue;
}


// Return whether the first string matches the second, case insensitively.
// The first string may be truncated, but the first three chars must match.

flag FMatchSz(CONST char *sz1, CONST char *sz2)
{
  CONST char *szStart = sz1;

  while (*sz1 && ChCap(*sz1) == ChCap(*sz2))
    sz1++, sz2++;
  return *sz1 == chNull && (*sz2 == chNull || sz1 - szStart >= 3);
}


// Return whether the first string matches any string in the second, case
// sensitively. The second string is subdivided by comma or semicolon
// characters. Return offset into string, and optionally the index into list.

CONST char *SzInList(CONST char *sz1, CONST char *sz2, int *pisz)
{
  CONST char *szStart = sz1;
  int isz = 0;

  loop {
    // Compate string to current string in string list.
    for (sz1 = szStart; *sz1 && *sz1 == *sz2; sz1++, sz2++)
      ;
    if (*sz2 == chNull || (*sz2 == chSep || *sz2 == chSep2)) {
      if (*sz1 == chNull) {
        // Match if reached end of string and of current string in list.
        if (pisz != NULL)
          *pisz = isz;
        return sz2 + (*sz2 == chSep || *sz2 == chSep2);
      }
    } else {
      // Skip ahead to start of next string in string list.
      while (*sz2 && !(*sz2 == chSep || *sz2 == chSep2))
        sz2++;
    }
    if (*sz2 == chSep || *sz2 == chSep2)
      sz2++;
    else
      break;    // If no separator, then end of string list reached.
    isz++;
  }
  if (pisz != NULL)
    *pisz = -1;
  return NULL;
}


// Set a given number of bytes to zero given a starting pointer.

void ClearB(pbyte pb, int cb)
{
  while (cb-- > 0)
    *pb++ = 0;
}


// Copy a given number of bytes from one location to another.

void CopyRgb(byte *pbSrc, byte *pbDst, int cb)
{
  while (cb-- > 0)
    *pbDst++ = *pbSrc++;
}


// Copy a range of characters and zero terminate it. If there are too many
// characters to fit in the destination buffer, the string is truncated.

void CopyRgchToSz(CONST char *pch, int cch, char *sz, int cchMax)
{
  cch = Min(cch, cchMax-1);
  CopyRgb((byte *)pch, (byte *)sz, cch);
  sz[cch] = chNull;
}


// Determine the sign of a number: -1 if value negative, +1 if value positive,
// and 0 if it's zero.

real RSgn(real r)
{
  return r == 0.0 ? 0.0 : RSgn2(r);
}


// Given an x and y coordinate, return the angle formed by a line from the
// origin to this coordinate. This is just converting from rectangular to
// polar coordinates, however this doesn't involve the radius here.

real RAngle(real x, real y)
{
  real a;

  if (x != 0.0) {
    if (y != 0.0)
      a = RAtn(y/x);
    else
      a = x < 0.0 ? rPi : 0.0;
  } else
    a = y < 0.0 ? -rPiHalf : rPiHalf;
  if (a < 0.0)
    a += rPi;
  if (y < 0.0)
    a += rPi;
  return a;
}


// Like RAngle() but return the angle between two 3D vectors instead.

real VAngle(CONST PT3R *v1, CONST PT3R *v2)
{
  real angle, len1, len2;

  len1 = PtLen((*v1));
  len2 = PtLen((*v2));
  if (len1 != 0.0 && len2 != 0.0) {
    angle = PtDot((*v1), (*v2))/len1/len2;
    if (angle == 0.0)
      return rPiHalf;
    else if (angle <= -1.0)
      return rPi;
    angle = RAtn(RSqr(1.0 - Sq(angle)) / angle);
    if (angle >= 0.0)
      return angle;
    else
      return angle + rPi;
  } else
    return rPiHalf;
}


// Modulus function for floating point values, in which we bring the given
// parameter to within the range of 0 to 360.

real Mod(real d)
{
  if (d >= rDegMax)         // In most cases, value is only slightly
    d -= rDegMax;           // out of range, so can test for it and
  else if (d < 0.0)         // avoid the more complicated arithmetic.
    d += rDegMax;
  if (d >= 0 && d < rDegMax)
    return d;
  return (d - RFloor(d/rDegMax)*rDegMax);
}


// Integer division, like the "/" operator but always rounds result down.

long Dvd(long x, long y)
{
  long z;

  if (y == 0)
    return x;
  z = x / y;
  if (((x >= 0) == (y >= 0)) || x-z*y == 0)
    return z;
  return z - 1;
}


// Lookup a string within a table (case insensitively) returning the index
// that goes with the matched string, or -1 if the string is not found.

int SzLookup(CONST StrLook *rgStrLook, CONST char *sz)
{
  CONST char *pch1, *pch2;
  int irg;

  for (irg = 0; rgStrLook[irg].isz >= 0; irg++) {
    for (pch1 = sz, pch2 = rgStrLook[irg].sz;
      *pch1 && ChCap(*pch1) == ChCap(*pch2); pch1++, pch2++)
      ;
    if (*pch1 == chNull && (*pch2 == chNull || pch1 - sz >= 3))
      return rgStrLook[irg].isz;
  }
  return -1;
}


// Return whether a zero terminated string is a substring of another string,
// case insensitively.

flag FCompareSzSubI(CONST char *sz1, CONST char *sz2)
{
  while (*sz1 && ChCap(*sz1) == ChCap(*sz2))
    sz1++, sz2++;
  return *sz1 == chNull;
}


// Set a string to a floating point value, with at most 'n' significant
// fractional digits, and dropping trailing '0' characters.

void FormatR(char *sz, real r, int n)
{
  char szT[cchSzDef], *pch;

  sprintf(szT, "%%.%df", NAbs(n));
  sprintf(sz, szT, r);
  for (pch = sz; *pch; pch++)
    ;
  while (pch > sz && *(--pch) == '0')    // Drop off any trailing 0 digits.
    ;
  // Positive n means ensure at least one fractional digit.
  pch[n > 0 ? 1 + (*pch == '.') : (*pch != '.')] = chNull;
}


// Blend two RGB colors along the specified proportion between them. Returned
// color ranges from the first color (ratio = 0) to the second (ratio = 1).

KV KvBlend(KV kv1, KV kv2, real rRatio)
{
  return Rgb((int)((real)(RgbR(kv2) - RgbR(kv1)) * rRatio) + RgbR(kv1),
    (int)((real)(RgbG(kv2) - RgbG(kv1)) * rRatio) + RgbG(kv1),
    (int)((real)(RgbB(kv2) - RgbB(kv1)) * rRatio) + RgbB(kv1));
}


#define rHueMax  rDegMax
#define rHueHalf rDegHalf
#define rHue13   120.0
#define rHue23   240.0
#define rHue16   60.0

// Return a RGB color of the rainbow given a number 0-360, in which 0 is red,
// 120 is green, and 240 is blue.

KV KvHue(real deg)
{
  int nR, nG, nB;
  real rDiff;

  while (deg >= rHueMax)
    deg -= rHueMax;
  while (deg < 0.0)
    deg += rHueMax;
  rDiff = RAbs(deg - rHueHalf);
  if (rDiff > rHue13)
    nR = 255;
  else if (rDiff < rHue16)
    nR = 0;
  else
    nR = NMultDiv((int)(rDiff - rHue16), 255, rHue16);
  rDiff = RAbs(deg - rHue13);
  if (rDiff < rHue16)
    nG = 255;
  else if (rDiff > rHue13)
    nG = 0;
  else
    nG = NMultDiv((int)(rHue13 - rDiff), 255, rHue16);
  rDiff = RAbs(deg - rHue23);
  if (rDiff < rHue16)
    nB = 255;
  else if (rDiff > rHue13)
    nB = 0;
  else
    nB = NMultDiv((int)(rHue13 - rDiff), 255, rHue16);
  return Rgb(nR, nG, nB);
}


/*
******************************************************************************
** General Astrology Procedures.
******************************************************************************
*/

// A similar modulus function: Convert an integer to value from 1-12.

int Mod12(int i)
{
  while (i > cSign)
    i -= cSign;
  while (i < 1)
    i += cSign;
  return i;
}


// Convert an inputed fractional degrees/minutes value to a true decimal
// degree quantity. For example, the user enters the decimal value "10.30"
// to mean 10 degrees and 30 minutes; this will return 10.5, i.e. 10 degrees
// and 30 minutes expressed as a floating point degree value.

real DecToDeg(real d)
{
  return RSgn(d)*(RFloor(RAbs(d))+RFract(RAbs(d))*100.0/60.0);
}


// This is the inverse of the above function. Given a true decimal value for
// a zodiac degree, adjust it so the degrees are in the integer part and the
// minute expressed as hundredths, e.g. 10.5 degrees -> 10.30

real DegToDec(real d)
{
  return RSgn(d)*(RFloor(RAbs(d))+RFract(RAbs(d))*60.0/100.0);
}


// Return the shortest distance between two degrees in the zodiac. This is
// normally their difference, but we have to check if near the Aries point.

real MinDistance(real deg1, real deg2)
{
  real r;

  r = RAbs(deg1-deg2);
  return r <= rDegHalf ? r : rDegMax - r;
}


// This is just like the above routine, except the min distance value returned
// will either be positive or negative based on whether the second value is
// ahead or behind the first one in a circular zodiac.

real MinDifference(real deg1, real deg2)
{
  real r;

  r = deg2 - deg1;
  if (RAbs(r) < rDegHalf)
    return r;
  return r >= 0 ? r - rDegMax : r + rDegMax;
}


// Return the degree of the midpoint between two zodiac positions, making sure
// we return the true midpoint closest to the positions in question.

real Midpoint(real deg1, real deg2)
{
  real mid;

  mid = (deg1+deg2)/2.0;
  return MinDistance(deg1, mid) < rDegQuad ? mid : Mod(mid+rDegHalf);
}


// Return the minimum great circle distance between two sets of spherical
// coordinates. This is like MinDistance() but takes latitude into account.

real SphDistance(real lon1, real lat1, real lon2, real lat2)
{
  real dLon, r;

  dLon = RAbs(lon1 - lon2);
  r = RAcosD(RSinD(lat1)*RSinD(lat2) + RCosD(lat1)*RCosD(lat2)*RCosD(dLon));
  return r;
}


// Given two pairs of coordinates on a sphere, return coordinates at some
// proportion (0.0-1.0) along the great circle path between them.

void SphRatio(real lon1, real lat1, real lon2, real lat2, real rRatio,
  real *lon, real *lat)
{
  real x1, y1, z1, x2, y2, z2, x, y, z, len, ang, adj, ang2;

  SphToRec(1.0, lon1, lat1, &x1, &y1, &z1);
  SphToRec(1.0, lon2, lat2, &x2, &y2, &z2);
  if (rRatio != 0.5) {
    // Bisecting an arc is easy, however other proportions require extra math.
    len = RLength3(x2 - x1, y2 - y1, z2 - z1) / 2.0;
    ang = RAsinD(len);
    adj = 1.0 / RTanD(ang);
    rRatio = (rRatio - 0.5) / 0.5;
    ang2 = rRatio * ang;
    rRatio = adj * RTanD(ang2);
    rRatio = (rRatio / 2.0) + 0.5;
  }
  x = x1 + (x2 - x1) * rRatio;
  y = y1 + (y2 - y1) * rRatio;
  z = z1 + (z2 - z1) * rRatio;
  RecToSph3(x, y, z, lon, lat);
}


// Given a planet and sign, determine whether: The planet rules the sign or
// is in detriment in the sign, the planet exalts in sign or is in fall /
// debilitated in sign, the planet esoterically and hierarchically and ray
// rules or is in detriment in the sign, and return an appropriate string.

char *Dignify(int obj, int sign)
{
  static char szDignify[7];
  int sign2 = Mod12(sign+6), ray, ich;

  sprintf(szDignify, "-_____");
  if (obj > oNorm)
    goto LExit;

  // Check standard rulerships.
  if (!ignore7[rrStd]) {
    if (ruler1[obj] == sign || ruler2[obj] == sign)
      szDignify[rrStd+1] = 'R';
    else if (ruler1[obj] == sign2 || ruler2[obj] == sign2)
      szDignify[rrStd+1] = 'd';
  }
  if (!ignore7[rrExa]) {
    if (exalt[obj] == sign)
      szDignify[rrExa+1] = 'X';
    else if (exalt[obj] == sign2)
      szDignify[rrExa+1] = 'f';
  }

  // Check esoteric rulerships.
  if (!ignore7[rrEso]) {
    if (rgObjEso1[obj] == sign || rgObjEso2[obj] == sign)
      szDignify[rrEso+1] = 'S';
    else if (rgObjEso1[obj] == sign2 || rgObjEso2[obj] == sign2)
      szDignify[rrEso+1] = 's';
  }
  if (!ignore7[rrHie]) {
    if (rgObjHie1[obj] == sign || rgObjHie2[obj] == sign)
      szDignify[rrHie+1] = 'H';
    else if (rgObjHie1[obj] == sign2 || rgObjHie2[obj] == sign2)
      szDignify[rrHie+1] = 'h';
  }
  if (!ignore7[rrRay]) {
    ray = rgObjRay[obj];
    if (ray > 0) {
      if (rgSignRay2[sign][ray] > 0)
        szDignify[rrRay+1] = 'Y';
      else if (rgSignRay2[sign2][ray] > 0)
        szDignify[rrRay+1] = 'z';
    }
  }

LExit:
  // Put "most significant" rulership state present in the first character.
  for (ich = 1; ich <= 5; ich += ich == 1 ? 3 :
    (ich == 4 ? -2 : (ich == 3 ? 2 : 1))) {
    if (szDignify[ich] != '_') {
      szDignify[0] = szDignify[ich];
      break;
    }
  }
  return szDignify;
}


// Process the list of each sign's rays, creating a grid based on it
// indicating whether each ray applies to a sign, and its proportion.

void EnsureRay()
{
  int i, j, c, n;

  for (i = 1; i <= cSign; i++) {
    for (j = 1; j <= cRay; j++)
      rgSignRay2[i][j] = 0;
    c = 0;
    n = rgSignRay[i];
    while (n) {
      j = n % 10;
      n /= 10;
      if (!FBetween(j, 1, cRay))
        continue;
      rgSignRay2[i][j] = 1;
      c++;
    }
    for (j = 1; j <= cRay; j++)
      rgSignRay2[i][j] *= 420 / c;
  }
}


// Initialize table of star brightnesses. Usually only called once before
// first star accessed, but may be redone if computation method changes.

void EnsureStarBright()
{
  int i;
  real rMode;

  rMode = FCmSwissStar() ? 1.0 : 0.0;
  if (rStarBrightDef[0] != rMode) {
    rStarBrightDef[0] = rMode;

    // Matrix formulas have star brightnesses in a simple table.
    for (i = 1; i <= cStar; i++) {
#ifdef MATRIX
      rStarBrightDef[i] = rStarBrightMatrix[i];
#else
      rStarBrightDef[i] = 1.0;
#endif
      rStarBright[i] = rStarBrightDef[i];
      // Assume each star is 100 LY away.
      rStarDistDef[i] = rStarDist[i] = 100.0 * rLYToAU;
    }

#ifdef SWISS
    // Swiss Ephemeris reads star brightnesses from an external file.
    if (FCmSwissStar())
      SwissComputeStars(0.0, fTrue);
#endif
  }
}


// Determine the number of days in a particular month. The year is needed too,
// because have to check for leap years in the case of February.

int DayInMonth(int month, int year)
{
  int d;

  if (month == mSep || month == mApr || month == mJun || month == mNov)
    d = 30;
  else if (month != mFeb)
    d = 31;
  else {
    d = 28;
    if (year % 4 == 0 &&
      (year % 100 != 0 || year % 400 == 0 || year <= yeaJ2G))
      d++;
  }
  return d;
}


// Return the actual number of days in a particular month. Normally, this is
// the same as the above routine which determines the index of the last day of
// the month, but the values can differ when changing between calendar systems
// (Julian to Gregorian) in which a month can skip over days.

int DaysInMonth(int month, int year)
{
  int d;

  d = DayInMonth(month, year);
  if (ciGreg.yea == yeaJ2G && ciGreg.mon == monJ2G && ciGreg.day == dayJ2G2 &&
    year == yeaJ2G && month == monJ2G)
    d -= (dayJ2G2 - dayJ2G1 - 1);
  return d;
}


// Return the day of the week (Sunday is 0) of the specified given date.

int DayOfWeek(int month, int day, int year)
{
  int d;

  d = (MdyToJulian(month, day, year) + 1) % 7;
  return d < 0 ? d+7 : d;
}


// Given a day, and the month and year it falls in, add a number of days to
// it and return the new day index. As month changes are not checked for here,
// this is mostly just adding the offset to the day, however need to check for
// calendar changes for when days in a month may be skipped.

int AddDay(int month, int day, int year, int delta)
{
  int d;

  d = day + delta;
  if (ciGreg.yea == yeaJ2G && ciGreg.mon == monJ2G && ciGreg.day == dayJ2G2 &&
    year == yeaJ2G && month == monJ2G) {       // Check for Julian to
    if (d > dayJ2G1 && d < dayJ2G2)            // Gregorian crossover.
      d += NSgn(delta)*(dayJ2G2-dayJ2G1-1);
  }
  return d;
}


// Given an aspect and two objects making that aspect with each other, return
// the maximum orb allowed for such an aspect. Normally this only depends on
// the aspect itself, but some objects require narrow orbs, and some allow
// wider orbs, so check for these cases.

real GetOrb(int obj1, int obj2, int asp)
{
  real orb, r;

  orb = rAspOrb[asp];
  r = rObjOrb[Min(obj1, oNorm1)];
  orb = Min(orb, r);
  r = rObjOrb[Min(obj2, oNorm1)];
  orb = Min(orb, r);
  orb += rObjAdd[Min(obj1, oNorm1)];
  orb += rObjAdd[Min(obj2, oNorm1)];
  return orb;
}


// Return an aspect's name, checking whether parallel aspects are on.

CONST char *SzAspect(int asp)
{
  if (us.fParallel && asp <= aOpp)
    asp += cAspect;
  return szAspectDisp[asp];
}


// Return the three letter abbreviation for an aspect.

CONST char *SzAspectAbbrev(int asp)
{
  if (us.fParallel && asp <= aOpp)
    asp += cAspect;
  return szAspectAbbrevDisp[asp];
}


// Set the central planet (e.g. geocentric or heliocentric).

void SetCentric(int obj)
{
  if (!us.fIgnoreAuto && ignore[us.objCenter] && !ignore[obj]) {
    // If -YRh switch in effect, might auto(un)restrict central object.
    inv(ignore[us.objCenter]);
    inv(ignore[obj]);
  }
  us.objCenter = obj;
}


// Return the planet that an object orbits, if any.

int ObjOrbit(int obj)
{
  if (FGeo(obj))
    return oEar;
  if (FCust(obj)) {
#ifdef SWISS
    // Check if this object has been redefined to be related to Earth's Moon.
    if (rgTypSwiss[obj - custLo] == 2 && FGeo(rgObjSwiss[obj - custLo]))
      return oEar;
    // Check if this object has been redefined to be a planetary moon.
    if (rgTypSwiss[obj - custLo] == 3 && rgObjSwiss[obj - custLo] % 100 < 99)
      return rgObjSwiss[obj - custLo] / 100 + 1;
#endif
    return oSun;
  }
  if (FBetween(obj, oMer, cPlanet) || obj == oEar)
    return oSun;
  return -1;
}


// Map a planetary moon ephemeris number to an Astrolog object index.

int ObjMoons(int i)
{
  int pla, moo, obj;

  pla = i/100; moo = i - pla*100;
  if (FBetween(pla, 4, 9) && FBetween(moo, 1, 8))
    obj = moonsLo + nMooMap[pla-4][moo-1];
  else if (FBetween(pla, 5, 9) && moo == 99)
    obj = cobLo + (pla-5);
  else
    obj = -1;
  return obj;
}


// Return the diameter of an object, in km.

real RObjDiam(int obj)
{
#ifdef SWISS
  int i;

  // Check if this object has been redefined to be a planetary moon.
  if (FCust(obj) && rgTypSwiss[obj - custLo] == 3) {
    i = rgObjSwiss[obj - custLo];
    obj = ObjMoons(i);
  }
#endif
  if (!FNorm(obj))
    return 0.0;
  // If planet Center of Body (COB) is present, barycenter size should be 0.
  if (FBetween(obj, oJup, oPlu) && (!ignore[obj - oJup + cobLo]
#ifdef SWISS
    || (!ignore[oVul] && rgTypSwiss[oVul - custLo] == 3 &&
    rgObjSwiss[oVul - custLo] == (obj - oJup + 5)*100 + 99)
#endif
    ))
    return 0.0;
  return rObjDiam[obj];
}


/*
******************************************************************************
** String Procedures.
******************************************************************************
*/

// Exit the program, and do any cleanup necessary. Note that if there was
// a non-fatal error, and were in the -Q loop mode, then won't actually
// terminate the program, but rather drop back to the command line loop.

void Terminate(int tc)
{
  char sz[cchSzDef];

  if (us.fNoQuit)
    return;
  if (tc == tcForce) {
    is.S = stdout;
    AnsiColor(kWhiteA);
    sprintf(sz, "\n%s %s exited.\n", szAppName, szVersionCore);
    PrintSz(sz);
  }
  if (tc == tcError && us.fLoop)
    return;
  if (us.fAnsiColor) {
    sprintf(sz, "%c[0m", chEscape);    // Get out of any Ansi color mode.
    PrintSz(sz);
  }
  FinalizeProgram(tc != tcOK);
  exit(NAbs(tc));
}


// Print a string on the screen. A seemingly simple operation, however
// keep track of what column are printing at after each newline so can
// automatically clip at the appropriate point, and keep track of the row
// printing at too, so can prompt before screen scrolling.

void PrintSz(CONST char *sz)
{
  char szInput[cchSzDef], *pch;
#ifndef WIN
  int fT;
#endif

  for (pch = (char *)sz; *pch; pch++) {
    if (*pch != '\n') {
      if (is.nHTML != 2) {
        is.cchCol++;
        if (us.fClip80 && is.cchCol >= us.nScreenWidth)  // Clip if needed.
          continue;
      }
    } else {
      if (is.cchCol > is.cchColMax)
        is.cchColMax = is.cchCol;
      is.cchRow++;
      is.cchCol = 0;
    }
#ifdef WIN
    if (is.S == stdout) {
      if ((byte)*pch >= ' ') {
        szInput[0] = *pch; szInput[1] = chNull;
        TextOut(wi.hdc, (is.cchCol - 1 - wi.xScroll * 10) * wi.xChar + 4,
          (is.cchRow - wi.yScroll * 10) * wi.yChar, szInput, 1);
      }
    } else
#endif
    if (is.nHTML == 1) {
      if (*pch == '<')
        fprintf(is.S, "&lt;");
      else if (*pch == '>')
        fprintf(is.S, "&gt;");
      else if (*pch == '&')
        fprintf(is.S, "&amp;");
      else if (*pch == '\"')
        fprintf(is.S, "&quot;");
      else if (*pch == ' ' && (pch <= sz || *(pch+1) <= ' '))
        fprintf(is.S, "&nbsp;");
      else if (*pch == '\n')
        fprintf(is.S, "<br>\n");
      else
        putc(*pch, is.S);
    } else
      putc(*pch, is.S);
#ifndef WIN
    if (*pch == '\n' && is.S == stdout &&
      us.nScrollRow > 0 && is.cchRow >= us.nScrollRow) {

      // If have printed 'n' rows, stop and wait for a line to be entered.

      fT = us.fAnsiColor;
      us.fAnsiColor = fFalse;
      InputString("Press return to continue scrolling", szInput);
      us.fAnsiColor = fT;
      is.cchRow = 0;

      // One can actually give a few simple commands before hitting return.

      if (szInput[0] == 'q' || szInput[0] == '.')
        Terminate(tcForce);
      else if (szInput[0] == '8')
        inv(us.fClip80);
      else if (szInput[0] == 'Q')
        us.nScrollRow = 0;
      else if (szInput[0] == 'k') {
        if (us.fAnsiColor)
          AnsiColor(kDefault);
        inv(us.fAnsiColor); inv(us.fAnsiChar);
      }
    }
#else
    if (*pch == '\n' && is.S == stdout && wi.hdcPrint != hdcNil &&
      is.cchRow >= us.nScrollRow) {

      // If writing to the printer, start a new page when appropriate.

      is.cchRow = 0;
      EndPage(wi.hdcPrint);
      StartPage(wi.hdcPrint);
      // StartPage clobbers all the DC settings.
      SetMapMode(wi.hdcPrint, MM_ANISOTROPIC);      // For SetViewportExt
      SetViewportOrg(wi.hdcPrint, 0, 0);
      SetViewportExt(wi.hdcPrint, GetDeviceCaps(wi.hdcPrint, HORZRES),
        GetDeviceCaps(wi.hdcPrint, VERTRES));
      SetWindowOrg(wi.hdcPrint, 0, 0);
      SetWindowExt(wi.hdcPrint, wi.xClient, wi.yClient);
      SetBkMode(wi.hdcPrint, TRANSPARENT);
      SelectObject(wi.hdcPrint, wi.hfont);
    }
#endif
  }
}


// Print a single character on the screen.

void PrintCh(char ch)
{
  char sz[2];

  sz[0] = ch; sz[1] = chNull;    // Treat char as a string of length one.
  PrintSz(sz);                   // Then call above to print the string.
}


// Print a string on the screen. Unlike the normal PrintSz(), here still
// go to the standard output even if text is being sent to a file with -os.

void PrintSzScreen(CONST char *sz)
{
  FILE *fileT;

  fileT = is.S;
  is.S = stdout;
  PrintSz(sz);
  is.S = fileT;
}


// Print a partial progress message given a string. This is meant to be used
// in the middle of long operations such as creating and saving files.

void PrintProgress(CONST char *sz)
{
#ifndef WIN
  // Progress messages are ignored in the Windows version.
  AnsiColor(kYellowA);
  fprintf(stderr, "%s\n", sz);
  AnsiColor(kDefault);
#endif
}


// Print a general user message given a string. This is just like the warning
// displayer below, except print in a different color.

void PrintNotice(CONST char *sz)
{
#ifndef WIN
  AnsiColor(kYellowA);
  fprintf(stderr, "%s\n", sz);
  AnsiColor(kDefault);
#else
  char szT[cchSzDef];

  if (wi.fNoPopup)
    return;
  sprintf(szT, "%s Notice", szAppName);
  MessageBox(wi.hwndMain, sz, szT, MB_ICONINFORMATION);
#endif
}


// Print a warning message given a string. This is called in non-fatal cases
// in which normal execution is resumed after printing the string.

void PrintWarning(CONST char *sz)
{
#ifndef WIN
  AnsiColor(kRedA);
  fprintf(stderr, "%s\n", sz);
  AnsiColor(kDefault);
#else
  char szT[cchSzDef];

  if (wi.fNoPopup)
    return;
  sprintf(szT, "%s Warning", szAppName);
  MessageBox(wi.hwndMain, sz, szT, MB_ICONSTOP);
#endif
}


// Print an error message. This is called in more serious cases which halt
// running of the current chart sequence, which may terminate the program,
// but isn't a fatal error in that can still fall back to the -Q loop.

void PrintError(CONST char *sz)
{
#ifndef WIN
  AnsiColor(kRedA);
  fprintf(stderr, "%s: %s\n", szAppName, sz);
  AnsiColor(kDefault);
  Terminate(tcError);
#else
  char szT[cchSzDef];

  if (wi.fNoPopup)
    return;
  sprintf(szT, "%s Error", szAppName);
  MessageBox(wi.hwndMain, sz, szT, MB_ICONEXCLAMATION);
#endif
}


// Print error message for missing parameters to a command switch.

flag FErrorArgc(CONST char *szOpt, int carg, int cargMax)
{
  char sz[cchSzDef];

  carg--;
  if (carg >= cargMax)
    return fFalse;
  sprintf(sz, "Too few options to switch %c%s (%d given, %d required)",
    chSwitch, szOpt, carg, cargMax);
  PrintError(sz);
  return fTrue;
}


// Print error message for out of range integer parameter to a command switch.

flag FErrorValN(CONST char *szOpt, flag f, int nVal, int nPar)
{
  char sz[cchSzMax], szPar[cchSzDef];

  if (!f)
    return fFalse;
  if (nPar <= 0)
    szPar[0] = chNull;
  else
    sprintf(szPar, "parameter #%d of ", nPar);
  sprintf(sz, "Value %d passed to %sswitch %c%s out of range.\n",
    nVal, szPar, chSwitch, szOpt);
  PrintError(sz);
  return fTrue;
}


// Print error message for out of range real parameter to a command switch.

flag FErrorValR(CONST char *szOpt, flag f, real rVal, int nPar)
{
  char sz[cchSzMax], szPar[cchSzDef], szVal[cchSzDef];

  if (!f)
    return fFalse;
  if (nPar <= 0)
    szPar[0] = chNull;
  else
    sprintf(szPar, "parameter #%d of ", nPar);
  if (rVal != rLarge) {
    FormatR(szVal, rVal, -6);
    sprintf(sz, "Value %s passed to %sswitch %c%s out of range.\n",
      szVal, szPar, chSwitch, szOpt);
  } else
    sprintf(sz, "Bad value passed to %sswitch %c%s\n",
      szPar, chSwitch, szOpt);
  PrintError(sz);
  return fTrue;
}


// Print error message for a disallowed command switch.

void ErrorArgv(CONST char *szOpt)
{
  char sz[cchSzDef];

  sprintf(sz, "The switch %c%s is not allowed now.\n", chSwitch, szOpt);
  PrintError(sz);
}


// Print error message for a completely unknown command switch.

void ErrorSwitch(CONST char *szOpt)
{
  char sz[cchSzDef];

  sprintf(sz, "Unknown switch '%s'", szOpt);
  PrintError(sz);
}


#ifdef PLACALC
// Print error messages dealing with ephemeris file access.

void ErrorEphem(CONST char *sz, long l)
{
  char szT[cchSzDef];

  if (is.fNoEphFile)
    return;
  if (l < 0)
    sprintf(szT, "Ephemeris file %s not found.\n", sz);
  else
    sprintf(szT, "Seek error in file %s at position %ld.\n", sz, l);
  is.fNoEphFile = fTrue;
  PrintWarning(szT);
}
#endif


// A simple procedure used throughout Astrolog: Print a particular
// character on the screen 'n' times.

void PrintTab(char ch, int cch)
{
  while (cch-- > 0)
    PrintCh(ch);
}


// Set an Ansi or MS Windows text color.

void AnsiColor(int k)
{
  char sz[cchSzDef];
  int cchSav;
#ifdef GRAPH
  KV kv;
#endif

#ifdef WIN
  if (is.S == stdout) {
    if (k < 0)
      k = kLtGrayA;
    SetTextColor(wi.hdc, (COLORREF)rgbbmp[us.fAnsiColor ? k : kLtGrayA]);
    return;
  }
#endif

  // Special case: If passed the "color" Reverse, and Ansi color is not only
  // on but set to a value > 1, then enter reverse video mode.

  if (!us.fAnsiColor || (k == kReverse && us.fAnsiColor < 2))
    return;
  cchSav = is.cchCol;
  is.cchCol = 0;
  if (is.nHTML <= 0) {
    sprintf(sz, "%c[", chEscape);
    PrintSz(sz);
    if (k == kDefault)
      PrintCh('0');
    else if (k == kReverse) {
      PrintCh('7');
    } else {
      sprintf(sz, "%c;%d", k > 7 ? '1' : '0', 30 + (k & 7));
      PrintSz(sz);
    }
    PrintCh('m');
  } else {
    if (is.nHTML != 3) {
      is.nHTML = 2;
      PrintSz("</font>");
    } else
      is.nHTML = 2;
    if (k < 0)
      k = kLtGrayA;
    PrintSz("<font color=\"");
#ifdef GRAPH
    if (rgbbmp[k] == rgbbmpDef[k])
      PrintSz(szColorHTML[k]);
    else {
      kv = rgbbmp[k];
      sprintf(sz, "#%06x", Rgb(RgbB(kv), RgbG(kv), RgbR(kv)));
      PrintSz(sz);
    }
#else
    PrintSz(szColorHTML[k]);
#endif
    PrintSz("\">");
    is.nHTML = 1;
  }
  is.cchCol = cchSav;
}


// Print a zodiac position on the screen. This basically just prints the
// string returned from SzZodiac() below, except also take care of color.

void PrintZodiac(real deg)
{
  if (us.fRound) {
    if (us.nDegForm == 0)
      deg = Mod(deg + (is.fSeconds ? rRound/60.0/60.0 : rRound/60.0));
    else if (us.nDegForm == 1)
      deg = Mod(deg + (is.fSeconds ? rRound/4.0/60.0 : rRound/4.0));
  }
  AnsiColor(kSignA((int)(deg / 30.0) + 1));
  PrintSz(SzZodiac(deg));
  AnsiColor(kDefault);
}


CONST char *szNakshatra[27+1] = {"",
  "Ashv", "Bhar", "Krit", "Rohi", "Mrig", "Ardr", "Puna", "Push", "Ashl",
  "Magh", "PPha", "UPha", "Hast", "Chit", "Swat", "Vish", "Anur", "Jyes",
  "Mula", "PAsh", "UAsh", "Srav", "Dhan", "Shat", "PBha", "UBha", "Reva"};

// Given a zodiac position, return a string containing it as it's formatted
// for display to the user.

char *SzZodiac(real deg)
{
  static char szZod[12];
  int sign, d, m;
  real s;

  switch (us.nDegForm) {
  case 0:
    // Normally, format the position in degrees/sign/minutes format.

    sign = (int)deg / 30;
    d = (int)deg - sign*30;
    m = (int)(RFract(deg)*60.0);
    sprintf(szZod, "%2d%.3s%02d", d, szSignName[sign + 1], m);
    if (is.fSeconds) {
      s = RFract(deg)*60.0; s = RFract(s)*60.0;
      sprintf(&szZod[7], "'%02d\"", (int)s);
    }
    break;

  case 1:
    // However, if -sh switch in effect, format position as hours/minutes.

    d = (int)deg / 15;
    m = (int)((deg - (real)d*15.0)*4.0);
    sprintf(szZod, "%2dh,%02dm", d, m);
    if (is.fSeconds) {
      s = RFract(deg)*4.0; s = RFract(s)*60.0;
      sprintf(&szZod[7], ",%02ds", (int)s);
    }
    break;

  case 2:
    // Or, if -sd in effect, format position as a simple degree.

    sprintf(szZod, is.fSeconds ? "%11.7f" : "%7.3f", deg);
    break;

  default:
    // Otherwise, if -sn in effect, format position as a Nakshatra.

    deg = Mod(deg + rSmall);
    sign = (int)(deg / (rDegMax/27.0));
    d = (int)((deg - (real)sign*(rDegMax/27.0)) * 40.0 / (rDegMax/27.0));
    sprintf(szZod, "%2d%s%d", sign+1, szNakshatra[sign + 1], d/10 + 1);
    if (is.fSeconds)
      sprintf(&szZod[7], ".%d%s", d%10,
        szSignAbbrev[Mod12((int)(deg/(rDegMax/27.0/4.0))+1)]);
    break;
  }
  return szZod;
}


// This is similar to formatting a zodiac degree, but here return a string of
// a signed latitude value in degrees and minutes.

char *SzAltitude(real deg)
{
  static char szAlt[11];
  int d, m, f;
  real s;
  char ch;

  f = deg < 0.0;
  deg = RAbs(deg);
  if (us.nDegForm != 2) {
    if (us.fRound)
      deg += (is.fSeconds ? rRound/60.0/60.0 : rRound/60.0);
    d = (int)deg;
    m = (int)(RFract(deg)*60.0);
    ch = us.fAnsiChar > 1 ? 176 : chDeg1;
    sprintf(szAlt, "%c%2d%c%02d'", f ? '-' : '+', d, ch, m);
    if (is.fSeconds) {
      s = RFract(deg)*60.0; s = RFract(s)*60.0;
      sprintf(&szAlt[7], "%02d\"", (int)s);
    }
  } else {
    s = RAbs(deg);
    if (!is.fSeconds)
      sprintf(szAlt, s < 10.0 ? "%c%1.4f" : "%c%2.3f", f ? '-' : '+', s);
    else
      sprintf(szAlt, s < 10.0 ? "%c%1.7f" : "%c%2.6f", f ? '-' : '+', s);
  }
  return szAlt;
}


// Here return a string simply expressing the given value as degrees and
// minutes (and sometimes seconds) in the 0 to 360 degree circle.

char *SzDegree(real deg)
{
  static char szPos[11];
  int d, m;
  real s;

  deg = RAbs(deg);
  if (us.nDegForm != 2) {
    if (us.fRound)
      deg += (is.fSeconds ? rRound/60.0/60.0 : rRound/60.0);
    d = (int)deg;
    m = (int)(RFract(deg)*60.0);
    sprintf(szPos, "%3d%c%02d'", d, chDeg1, m);
    if (is.fSeconds) {
      s = RFract(deg)*60.0; s = RFract(s)*60.0;
      sprintf(&szPos[7], "%02d\"", (int)s);
    }
  } else {
    if (!is.fSeconds)
      sprintf(szPos, "%7.3f", deg);
    else
      sprintf(szPos, "%10.6f", deg);
  }
  return szPos;
}


// Compose an Hours:Minutes:Seconds (HMS) time value, given a total number of
// seconds. For example, 45015 (12*60*60+30*60+15) maps to "+12:30:15"

char *SzHMS(int sec)
{
  static char szHMS[10];
  int hr, min;
  char ch;

  ch = sec >= 0 ? '+' : '-';
  sec = NAbs(sec);
  hr = sec / 3600;
  min = sec / 60 % 60;
  sec %= 60;
  // Don't display seconds or minutes:seconds if they're zero.
  if (!us.fSeconds && min == 0 && sec == 0)
    sprintf(szHMS, "%c%d", ch, hr);
  else if (!us.fSeconds || sec == 0)
    sprintf(szHMS, "%c%d:%02d", ch, hr, min);
  else
    sprintf(szHMS, "%c%d:%02d:%02d", ch, hr, min, sec);
  return szHMS;
}


// Another string formatter, here return a date string given a month, day, and
// year. Format with the day or month first based on whether the "European"
// date variable is set or not. The routine also takes a parameter to indicate
// how much the string should be abbreviated, if any.

char *SzDate(int mon, int day, int yea, int nFormat)
{
  static char szDat[20];

  if (us.fEuroDate) {
    switch (nFormat) {
    case  2: sprintf(szDat, "%2d %.3s %4d", day, szMonth[mon], yea); break;
    case  1: sprintf(szDat, "%d %s %d", day, szMonth[mon], yea);     break;
    case -1: sprintf(szDat, "%2d-%2d-%2d", day, mon, NAbs(yea)%100); break;
    default: sprintf(szDat, "%2d-%2d-%4d", day, mon, yea);           break;
    }
  } else {
    switch (nFormat) {
    case  3: sprintf(szDat, "%.3s %2d, %d", szMonth[mon], day, yea); break;
    case  2: sprintf(szDat, "%.3s %2d %4d", szMonth[mon], day, yea); break;
    case  1: sprintf(szDat, "%s %d, %d", szMonth[mon], day, yea);    break;
    case -1: sprintf(szDat, "%2d/%2d/%2d", mon, day, NAbs(yea)%100); break;
    default: sprintf(szDat, "%2d/%2d/%4d", mon, day, yea);           break;
    }
  }
  return szDat;
}


// Return a string containing the given time expressed as an hour and minute
// (and second) quantity. This is formatted in 24 hour or am/pm time based on
// whether the "European" time format flag is set or not.

char *SzTime(int hr, int min, int sec)
{
  static char szTim[11];

  while (min >= 60) {
    min -= 60;
    hr++;
  }
  while (hr < 0)
    hr += 24;
  while (hr >= 24)
    hr -= 24;
  if (us.fEuroTime) {
    if (sec < 0)
      sprintf(szTim, "%2d:%02d", hr, min);
    else
      sprintf(szTim, "%2d:%02d:%02d", hr, min, sec);
  } else {
    if (sec < 0)
      sprintf(szTim, "%2d:%02d%cm", Mod12(hr), min, hr < 12 ? 'a' : 'p');
    else
      sprintf(szTim, "%2d:%02d:%02d%cm",
        Mod12(hr), min, sec, hr < 12 ? 'a' : 'p');
  }
  return szTim;
}


// This just determines the correct hour and minute and calls the above.

char *SzTim(real tim)
{
  tim += rSmall;
  return SzTime(NFloor(tim), (int)(RFract(RAbs(tim))*60.0),
    is.fSeconds ? (int)(RFract(RAbs(tim))*3600.0) % 60 : -1);
}


// Return a string containing the given time zone, given as a real value
// having the hours before UTC in the integer part and minutes fractionally.

char *SzZone(real zon)
{
  static char szZon[7];

  if (zon == zonLMT)
    sprintf(szZon, "LMT");
  else if (!is.fSeconds && RFract(RAbs(zon)) < rSmall)
    sprintf(szZon, "%d%c", (int)RAbs(zon), zon < 0.0 ? 'E' : 'W');
  else
    sprintf(szZon, "%d:%02d%c", (int)RAbs(zon), (int)(RFract(RAbs(zon))*60.0+
      rRound/60.0), zon < 0.0 ? 'E' : 'W');
  return szZon;
}


// Nicely format the given longitude and latitude locations and return them
// in a string. Various parts of the program display a chart header, and this
// allows the similar computations to be coded only once.

char *SzLocation(real lon, real lat)
{
  static char szLoc[21];
  int i, j, i2, j2;
  char chDeg, chLon, chLat;

  if (us.fRound) {
    lon = RSgn(lon) *
      (RAbs(lon) + (is.fSeconds ? rRound/60.0/60.0 : rRound/60.0));
    lat = RSgn(lat) *
      (RAbs(lat) + (is.fSeconds ? rRound/60.0/60.0 : rRound/60.0));
  } else {
    lon = RSgn(lon) * (RAbs(lon) + rSmall);
    lat = RSgn(lat) * (RAbs(lat) + rSmall);
  }
  i = (int)(RFract(RAbs(lon))*60.0);
  j = (int)(RFract(RAbs(lat))*60.0);
  if (is.fSeconds) {
    i2 = (int)(RFract(RAbs(lon))*3600.0) % 60;
    j2 = (int)(RFract(RAbs(lat))*3600.0) % 60;
  }
  chLon = (lon < 0.0 ? 'E' : 'W');
  chLat = (lat < 0.0 ? 'S' : 'N');
  if (us.fAnsiChar == 4) {
    // Format like "47N36,122W19", as seen in AAF files.
    if (!is.fSeconds)
      sprintf(szLoc, "%.0f%c%02d,%.0f%c%02d",
        RFloor(RAbs(lat)), chLat, j, RFloor(RAbs(lon)), chLon, i);
    else
      sprintf(szLoc, "%.0f%c%02d:%02d,%.0f%c%02d:%02d",
        RFloor(RAbs(lat)), chLat, j, j2, RFloor(RAbs(lon)), chLon, i, i2);
    return szLoc;
  }
  if (us.fAnsiChar != 3) {
    chDeg = us.fAnsiChar > 1 ? 176 : chDeg1;
    if (us.nDegForm != 2) {
      if (!is.fSeconds)
        sprintf(szLoc, "%3.0f%c%02d%c%3.0f%c%02d%c",
          RFloor(RAbs(lon)), chDeg, i, chLon,
          RFloor(RAbs(lat)), chDeg, j, chLat);
      else
        sprintf(szLoc, "%3.0f%c%02d:%02d%c%3.0f%c%02d:%02d%c",
          RFloor(RAbs(lon)), chDeg, i, i2, chLon,
          RFloor(RAbs(lat)), chDeg, j, j2, chLat);
    } else {
      if (!is.fSeconds)
        sprintf(szLoc, "%6.2f%c%6.2f%c",
          RAbs(lon), chLon, RAbs(lat), chLat);
      else
        sprintf(szLoc, "%9.5f%c%9.5f%c",
          RAbs(lon), chLon, RAbs(lat), chLat);
    }
  } else {
    if (us.nDegForm != 2) {
      if (!is.fSeconds)
        sprintf(szLoc, "%3.0f%c%02d%3.0f%c%02d",
          RFloor(RAbs(lon)), chLon, i,
          RFloor(RAbs(lat)), chLat, j);
      else
        sprintf(szLoc, "%3.0f%c%02d:%02d%3.0f%c%02d:%02d",
          RFloor(RAbs(lon)), chLon, i, i2,
          RFloor(RAbs(lat)), chLat, j, j2);
    } else {
      if (!is.fSeconds)
        sprintf(szLoc, "%5.1f%c%5.1f%c",
          RAbs(lon), chLon, RAbs(lat), chLat);
      else
        sprintf(szLoc, "%8.4f%c%8.4f%c",
          RAbs(lon), chLon, RAbs(lat), chLat);
    }
  }
  return szLoc;
}


// Format and return a string containing an elevation above sea level,
// displayed in either meters or feet, as used with topocentric charts.

char *SzElevation(real elv)
{
  static char szElev[21];
  char *pch;

  FormatR(szElev, us.fEuroDist ? elv : elv / rFtToM, -2);
  for (pch = szElev; *pch; pch++)
    ;
  sprintf(pch, "%s", us.fEuroDist ? "m" : "ft");
  return szElev;
}


// Format and return a string containing a relatively short length, displayed
// in either inches or centimeters, as used with paper sizes.

char *SzLength(real len)
{
  static char szLen[21];
  char *pch;

  FormatR(szLen, !us.fEuroDist ? len : len * rInToCm, -2);
  for (pch = szLen; *pch; pch++)
    ;
  sprintf(pch, "%s", us.fEuroDist ? "cm" : "in");
  return szLen;
}


#ifdef TIME
// Compute the date and time it is right now as the program is running using
// the computer's internal clock. Do this by getting the number of seconds
// which have passed since January 1, 1970 and going from there. The time
// return value filled is expressed in the given zone parameter.

void GetTimeNow(int *mon, int *day, int *yea, real *tim, real dst, real zon)
{
#ifdef PC
  SYSTEMTIME st, lt;
  real jd;
  int dh;

  GetSystemTime(&st);
  if (dst == dstAuto) {
    // Daylight field of 24 means autodetect whether Daylight Saving Time.

    GetLocalTime(&lt);
    dh = NAbs(st.wHour - lt.wHour);
    if (dh > 12)
      dh = 24-dh;
    is.fDst = (dh == us.zonDef-1);
    dst = (real)is.fDst;
  }
  if (zon == zonLMT)
    zon = us.lonDef / 15.0;
  jd = MdytszToJulian(st.wMonth, st.wDay, st.wYear,
    (real)(((st.wHour * 60 + st.wMinute + us.lTimeAddition) * 60 +
    st.wSecond) * 1000 + st.wMilliseconds) / (60.0 * 60.0 * 1000.0),
    0.0, -(zon-dst));
  *tim = (jd - RFloor(jd)) * 24.0;
  JulianToMdy(jd - 0.5, mon, day, yea);
#else
  time_t curtimer;
  int min, sec, i;
  real hr;
  CI ci;

  time(&curtimer);
  sec = (int)(curtimer % 60);
  curtimer = curtimer / 60 + us.lTimeAddition;
  min = (int)(curtimer % 60);
  curtimer /= 60;
#ifdef MACOLD
  curtimer += 8;
#endif
  if (zon == zonLMT)
    zon = us.lonDef / 15.0;
  hr = (real)(curtimer % 24) - (zon - (dst == dstAuto ? 0.0 : dst));
  curtimer /= 24;
  while (hr < 0.0) {
    curtimer--;
    hr += 24.0;
  }
  while (hr >= 24.0) {
    curtimer++;
    hr -= 24.0;
  }
  curtimer += ldTime;  // Number of days between 1/1/1970 and 1/1/4713 BC.
  JulianToMdy((real)curtimer, mon, day, yea);
  *tim = HMS(hr, min, sec);
  if (dst == dstAuto) {
    // Daylight field of 24 means autodetect whether Daylight Saving Time.

    SetCI(ci, *mon, *day, *yea, *tim, 0.0, zon, us.lonDef, us.latDef);
    if (DisplayAtlasLookup(us.locDef, 0, &i) &&
      DisplayTimezoneChanges(is.rgae[i].izn, 0, &ci)) {
      hr += ci.dst;
      while (hr < 0.0) {
        curtimer--;
        hr += 24.0;
      }
      while (hr >= 24.0) {
        curtimer++;
        hr -= 24.0;
      }
      JulianToMdy((real)curtimer, mon, day, yea);
      *tim = HMS(hr, min, sec);
    }
  }
  is.fDst = (dst > 0.0);
#endif // PC
}
#endif // TIME


// Given a string representing the complete pathname to a file, strip off all
// the path information leaving just the filename itself. This is called by
// the main program to determine the name of the Astrolog executable.

char *SzProcessProgname(char *szPath)
{
  char *pchStart, *pch, *pchEnd;

  pchStart = pch = szPath;
  while (*pch) {
#ifdef PC
    *pch = ChUncap(*pch);    // Because PC filenames are case insensitive.
#endif
    pch++;
  }
  pchEnd = pch;
  while (pch > pchStart && *pch != '.')
    pch--;
  if (pch > pchStart)
    *pch = 0;
  else
    pch = pchEnd;
  while (pch > pchStart && *pch != chDirSep)
    pch--;
  if (pch > pchStart)
    szPath = pch+1;
  return szPath;
}


// Given a string, return a pointer to a persistent version of it, in which
// "persistent" means its contents won't be invalidated when the stack frame
// changes. Strings such as macros and such need to be in their own space and
// can't just be local variables in a function reading them in.

char *SzPersist(char *szSrc)
{
  char *szNew;
  int cb;

  // Some strings such as outer level command line parameter arguments
  // already persist, so we can just return the same string passed in.
  if (is.fSzPersist)
    return szSrc;

  // Otherwise make a copy of the string and use it.
  cb = CchSz(szSrc)+1;
  szNew = (char *)PAllocate(cb, "string");
  is.cAlloc--;
  if (szNew != NULL)
    CopyRgb((byte *)szSrc, (byte *)szNew, cb);
  return szNew;
}


// This is Astrolog's memory allocation routine, returning a pointer given
// a size, and a string to use when printing error if the allocation fails.

pbyte PAllocate(long cb, CONST char *szType)
{
  char szT[cchSzDef];
  pbyte pb;

#ifdef DEBUG
  pb = (pbyte)PAllocateCore(cb + sizeof(dword)*3);
#else
  pb = (pbyte)PAllocateCore(cb);
#endif

  // Handle success or failure of the allocation.
  if (pb == NULL && szType) {
    sprintf(szT, "%s: Not enough memory for %s (%ld bytes).",
      szAppName, szType, cb);
    PrintWarning(szT);
  } else {
    is.cAlloc++;
    is.cAllocTotal++;
    is.cbAllocSize += cb;
  }

#ifdef DEBUG
  // Put sentinels at ends of allocation to check for buffer overruns.
  *(dword *)pb = dwCanary;
  *(dword *)(pb + sizeof(dword)) = cb;
  *(dword *)(pb + sizeof(dword)*2 + cb) = dwCanary;
  return pb + sizeof(dword)*2;
#else
  return pb;
#endif
}


// Free a memory buffer allocated with PAllocate().

void DeallocateP(void *pv)
{
  Assert(pv != NULL);
#ifdef DEBUG
  // Ensure buffer wasn't overrun during its existence.
  pbyte pbSys;
  dword lcb, dw;

  pbSys = (pbyte)pv - sizeof(dword)*2;
  Assert(pbSys != NULL);
  dw = *(dword *)pbSys;
  Assert(dw == dwCanary);
  lcb = *(dword *)(pbSys + sizeof(dword));
  dw = *(dword *)((pbyte)pv + lcb);
  Assert(dw == dwCanary);
  DeallocatePCore(pbSys);
#else
  DeallocatePCore(pv);
#endif
  is.cAlloc--;
}


#ifdef DEBUG
// Assert a condition. If not, display an error message.

void Assert(flag f)
{
  if (!f)
    PrintError("Debug Assert failed!\n");
}
#endif

/* general.cpp */
