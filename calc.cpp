/*
** Astrolog (Version 7.10) File: calc.cpp
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
** Last code change made 9/30/2020.
*/

#include "astrolog.h"


/*
******************************************************************************
** Julian Day Calculations.
******************************************************************************
*/

/* Given a month, day, and year, convert it into a single Julian day value, */
/* i.e. the number of days passed since a fixed reference date.             */

long MdyToJulian(int mon, int day, int yea)
{
#ifdef MATRIX
  if (!us.fEphemFiles)
    return MatrixMdyToJulian(mon, day, yea);
#endif
#ifdef EPHEM
  int fGreg = fTrue;
  double jd;

  if (yea < ciGreg.yea || (yea == ciGreg.yea &&
    (mon < ciGreg.mon || (mon == ciGreg.mon && day < ciGreg.day))))
    fGreg = fFalse;
  jd =
#ifdef EPHEM2
    !us.fPlacalcPla ?
#endif
#ifdef SWISS
    SwissJulDay(mon, day, yea, 12.0, fGreg) + rRound
#endif
#ifdef EPHEM2
    :
#endif
#ifdef PLACALC
    julday(mon, day, yea, 12.0, fGreg) + rRound
#endif
    ;
  return (long)RFloor(jd);
#else
  return 0;         // Shouldn't ever be reached.
#endif /* EPHEM */
}


/* Like above but return a fractional Julian time given the extra info. */

real MdytszToJulian(int mon, int day, int yea, real tim, real dst, real zon)
{
  if (dst == dstAuto)
    dst = (real)is.fDst;
  return (real)MdyToJulian(mon, day, yea) + (tim + zon - dst) / 24.0;
}


/* Take a Julian day value, and convert it back into the corresponding */
/* month, day, and year.                                               */

void JulianToMdy(real JD, int *mon, int *day, int *yea)
{
#ifdef MATRIX
  if (!us.fEphemFiles) {
    MatrixJulianToMdy(JD, mon, day, yea);
    return;
  }
#endif
#ifdef EPHEM
  double tim;

#ifdef EPHEM2
  if (!us.fPlacalcPla)
#endif
#ifdef SWISS
    SwissRevJul(JD, JD >= 2299171.0 /* Oct 15, 1582 */, mon, day, yea, &tim);
#endif
#ifdef EPHEM2
  else
#endif
#ifdef PLACALC
    revjul(JD, JD >= 2299171.0 /* Oct 15, 1582 */, mon, day, yea, &tim);
#endif
#endif /* EPHEM */
}


/*
******************************************************************************
** House Cusp Calculations.
******************************************************************************
*/

/* Compute 3D houses for 3D Campanus or the default case where houses are  */
/* 12 equal sized wedges covering the celestial sphere. Basically the same */
/* as doing local horizon, giving coordinates relative to prime vertical.  */

real RHousePlaceIn3DCore(real rLon, real rLat)
{
  real lonM, latM, lon, lat;

  lonM = Tropical(is.MC); latM = 0.0;
  EclToEqu(&lonM, &latM);
  lon = Tropical(rLon); lat = rLat;
  EclToEqu(&lon, &lat);
  lon = Mod(lonM - lon + rDegQuad);
  EquToLocal(&lon, &lat, -Lat);
  lon = rDegMax - lon;
  return Mod(lon + rSmall);
}


/* Compute 3D houses, or the house postion of a 3D location. Given a     */
/* zodiac position and latitude, return the house position as a decimal  */
/* number, which includes how far through the house the coordinates are. */

real RHousePlaceIn3D(real rLon, real rLat)
{
  real deg, rRet;
  int i;

  // Campanus houses are equal sized in 3D, and so a simple case to handle.
  deg = RHousePlaceIn3DCore(rLon, rLat);
  if (us.nHouseSystem == hsCampanus)
    return deg;

  // Determine which 3D house the prime vertical degree falls within.
  i = 0;
  do {
    i++;
  } while (!(i >= cSign ||
    (deg >= chouse3[i] && deg < chouse3[Mod12(i + 1)]) ||
    (chouse3[i] > chouse3[Mod12(i + 1)] &&
    (deg >= chouse3[i] || deg < chouse3[Mod12(i + 1)]))));
  rRet = Mod(ZFromS(i) + MinDistance(chouse3[i], deg) /
    MinDistance(chouse3[i], chouse3[Mod12(i + 1)]) * 30.0);
  return rRet;
}


/* This is a subprocedure of ComputeInHouses(). Given a zodiac position,  */
/* return which of the twelve houses it falls in. Remember that a special */
/* check has to be done for the house that spans 0 degrees Aries.         */

int NHousePlaceIn(real rLon, real rLat)
{
  int i, di;

  // Special processing for 3D houses.
  if (us.fHouse3D && rLat != 0.0)
    return SFromZ(RHousePlaceIn3D(rLon, rLat));

  // This loop also works when house positions decrease through the zodiac.
  rLon = Mod(rLon + rSmall);
  di = MinDifference(chouse[1], chouse[2]) >= 0.0 ? 1 : -1;
  i = 0;
  do {
    i++;
  } while (!(i >= cSign ||
    (rLon >= chouse[i] && rLon < chouse[Mod12(i + di)]) ||
    (chouse[i] > chouse[Mod12(i + di)] &&
    (rLon >= chouse[i] || rLon < chouse[Mod12(i + di)]))));
  if (di < 0)
    i = Mod12(i - 1);
  return i;
}


/* For each object in the chart, determine what house it belongs in. */

void ComputeInHouses(void)
{
  int i;

  // First determine 3D house cusp offsets.
  if (us.nHouseSystem != hsCampanus) {
    for (i = 1; i <= cSign; i++)
      chouse3[i] = RHousePlaceIn3DCore(chouse[i], 0.0);
  } else {
    // 3D Campanus cusps are always equally sized and distributed.
    for (i = 1; i <= cSign; i++)
      chouse3[i] = ZFromS(i);
  }

  // Loop over each object and place it.
  for (i = 0; i <= cObj; i++)
    inhouse[i] = NHousePlaceIn(planet[i], planetalt[i]);

  if (us.fHouse3D) {
    // 3D Campanus cusps should always be in the corresponding house.
    if (us.nHouseSystem == hsCampanus) {
      for (i = cuspLo; i <= cuspHi; i++)
        inhouse[i] = i - cuspLo + 1;
    // 3D angles should always be in the corresponding house.
    } else if (us.fHouseAngle) {
      for (i = cuspLo; i <= cuspHi; i += 3)
        inhouse[i] = i - cuspLo + 1;
    }
  }
}


/* Generic function to compute any of the various Equal house systems, in */
/* which all houses are an equal 30 degrees in size.                      */

void HouseEqualGeneric(real rOffset)
{
  int i;

  for (i = 1; i <= cSign; i++)
    chouse[i] = Mod(ZFromS(i) + rOffset);
}


/* Compute the cusp positions using the Porphyry house system. */

void HousePorphyry(void)
{
  int i;
  real rQuad, rSeg;

  rQuad = MinDistance(is.MC, is.Asc);
  rSeg = rQuad / 3.0;
  for (i = 0; i < 3; i++)
    chouse[sCap + i] = Mod(is.MC + rSeg*(real)i);
  rSeg = (rDegHalf - rQuad) / 3.0;
  for (i = 0; i < 3; i++)
    chouse[sLib + i] = Mod(is.Asc + rSeg*(real)i + rDegHalf);
  for (i = 1; i <= 6; i++)
    chouse[i] = Mod(chouse[6 + i] + rDegHalf);
}


/* The Sripati house system is like the Porphyry system except each house */
/* starts in the middle of the previous house as defined by Porphyry.     */

void HouseSripati(void)
{
  int i;
  real rgr[cSign+1];

  HousePorphyry();
  for (i = 1; i <= cSign; i++)
    rgr[i] = chouse[i];
  for (i = 1; i <= cSign; i++)
    chouse[i] = Midpoint(rgr[i], rgr[Mod12(i-1)]);
}


/* Compute the cusp positions using the Alcabitius house system. */

void HouseAlcabitius(void)
{
  real rDecl, rSda, rSna, r, rLon;
  int i;

  rDecl = RAsin(RSinD(is.OB) * RSinD(is.Asc));
  r = -RTanD(AA) * RTan(rDecl);
  rSda = DFromR(RAcos(r));
  rSna = rDegHalf - rSda;
  chouse[sLib] = DFromR(is.RA) - rSna;
  chouse[sSco] = DFromR(is.RA) - rSna*2.0/3.0;
  chouse[sSag] = DFromR(is.RA) - rSna/3.0;
  chouse[sCap] = DFromR(is.RA);
  chouse[sAqu] = DFromR(is.RA) + rSda/3.0;
  chouse[sPis] = DFromR(is.RA) + rSda*2.0/3.0;
  for (i = sLib; i <= sPis; i++) {
    r = RFromD(Mod(chouse[i]));
    // The transformation below is also done in CuspMidheaven().
    rLon = RAtn(RTan(r)/RCosD(is.OB));
    if (rLon < 0.0)
      rLon += rPi;
    if (r > rPi)
      rLon += rPi;
    chouse[i] = Mod(DFromR(rLon)+is.rSid);
  }
  for (i = sAri; i <= sVir; i++)
    chouse[i] = Mod(chouse[i+6]+rDegHalf);
}


/* This is a newer house system similar in philosophy to Porphyry houses,  */
/* and therefore (at least in the past) has also been called Neo-Porphyry. */
/* Instead of just trisecting the difference in each quadrant, we do a     */
/* smooth sinusoidal distribution of the difference around all the cusps.  */
/* Note that middle houses become 0 sized if a quadrant is <= 30 degrees.  */

void HousePullenSinusoidalDelta(void)
{
  real rQuad, rDelta;
  int iHouse;

  // Solve equations: x+n + x + x+n = q, x+3n + x+4n + x+3n = 180-q.
  rQuad = MinDistance(is.MC, is.Asc);
  rDelta = (rQuad - rDegQuad)/4.0;
  chouse[sLib] = Mod(is.Asc+rDegHalf); chouse[sCap] = is.MC;
  if (rQuad >= 30.0) {
    chouse[sAqu] = Mod(chouse[sCap] + 30.0 + rDelta);
    chouse[sPis] = Mod(chouse[sAqu] + 30.0 + rDelta*2.0);
  } else
    chouse[sAqu] = chouse[sPis] = Midpoint(chouse[sCap], is.Asc);
  if (rQuad <= 150.0) {
    chouse[sSag] = Mod(chouse[sCap] - 30.0 + rDelta);
    chouse[sSco] = Mod(chouse[sSag] - 30.0 + rDelta*2.0);
  } else
    chouse[sSag] = chouse[sSco] = Midpoint(chouse[sCap], chouse[sLib]);
  for (iHouse = sAri; iHouse < sLib; iHouse++)
    chouse[iHouse] = Mod(chouse[iHouse+6] + rDegHalf);
}


/* This is a new house system very similar to Sinusoidal Delta. Instead of */
/* adding a sine wave offset, multiply a sine wave ratio.                  */

void HousePullenSinusoidalRatio(void)
{
  real qSmall, rRatio, rRatio3, rRatio4, xHouse, rLo, rHi;
  int iHouse, dir;

  // Start by determining the quadrant sizes.
  qSmall = MinDistance(is.MC, is.Asc);
  dir = qSmall <= rDegQuad ? 1 : -1;
  if (dir < 0)
    qSmall = rDegHalf - qSmall;

#if TRUE
  // Solve equations: rx + x + rx = q, xr^3 + xr^4 + xr^3 = 180-q. Solve
  // quartic for r, then compute x given 1st equation: x = q / (2r + 1).
  if (qSmall > 0.0) {
    rLo = (2.0*pow(qSmall*qSmall - 270.0*qSmall + 16200.0, 1.0/3.0)) /
      pow(qSmall, 2.0/3.0);
    rHi = RSqr(rLo + 1.0);
    rRatio = 0.5*rHi +
      0.5*RSqr(-6.0*(qSmall-120.0)/(qSmall*rHi) - rLo + 2.0) - 0.5;
  } else
    rRatio = 0.0;
  rRatio3 = rRatio * rRatio * rRatio; rRatio4 = rRatio3 * rRatio;
  xHouse = qSmall / (2.0 * rRatio + 1.0);

#else
  // Can also solve equations empirically. Given candidate for r, compute x
  // given 1st equation: x = q / (2r + 1), then compare both against 2nd:
  // 2xr^3 + xr^4 = 180-q, to see whether current r is too large or small.
  // Before binary searching, first keep doubling rHi until too large.

  real qLarge = rDegHalf - qSmall;
  flag fBinarySearch = fFalse;

  rLo = rRatio = 1.0;
  loop {
    rRatio = fBinarySearch ? (rLo + rHi) / 2.0 : rRatio * 2.0;
    rRatio3 = rRatio * rRatio * rRatio; rRatio4 = rRatio3 * rRatio;
    xHouse = qSmall / (2.0 * rRatio + 1.0);
    if ((fBinarySearch && (rRatio <= rLo || rRatio >= rHi)) || xHouse <= 0.0)
      break;
    if (2.0 * xHouse * rRatio3 + xHouse * rRatio4 >= qLarge) {
      rHi = rRatio;
      fBinarySearch = fTrue;
    } else if (fBinarySearch)
      rLo = rRatio;
  }
#endif

  // xHouse and rRatio have been calculated. Fill in the house cusps.
  if (dir < 0)
    neg(xHouse);
  chouse[sAri] = is.Asc; chouse[sCap] = is.MC;
  chouse[sLib] = Mod(is.Asc+rDegHalf); 
  chouse[sCap + dir]   = Mod(chouse[sCap]                + xHouse * rRatio);
  chouse[sCap + dir*2] = Mod(chouse[Mod12(sCap + dir*3)] - xHouse * rRatio);
  chouse[sCap - dir]   = Mod(chouse[sCap]                - xHouse * rRatio3);
  chouse[sCap - dir*2] = Mod(chouse[Mod12(sCap - dir*3)] + xHouse * rRatio3);
  for (iHouse = sTau; iHouse < sLib; iHouse++)
    chouse[iHouse] = Mod(chouse[iHouse+6] + rDegHalf);
}


/* Compute the cusp positions using the Equal (Ascendant) house system. */
#define HouseEqual() HouseEqualGeneric(is.Asc)

/* This house system is just like the Equal system except that we start */
/* our 12 equal segments from the Midheaven instead of the Ascendant.   */
#define HouseEqualMC() HouseEqualGeneric(is.MC + rDegQuad)

/* The "Whole" house system is like the Equal system with 30 degree houses, */
/* where the 1st house starts at zero degrees of the sign of the Ascendant. */
#define HouseWhole() HouseEqualGeneric((real)((SFromZ(is.Asc)-1)*30));

/* Like "Whole" houses but the 10th house starts at the sign of the MC. */
#define HouseWholeMC() \
  HouseEqualGeneric((real)((SFromZ(is.MC)-1)*30) + rDegQuad);

/* The "Vedic" house system is like the Equal system except each house      */
/* starts 15 degrees earlier. The Asc falls in the middle of the 1st house. */
#define HouseVedic() HouseEqualGeneric(is.Asc - 15.0);

/* Like "Vedic" houses bit the MC falls in the middle of the 10th house. */
#define HouseVedicMC() HouseEqualGeneric(is.MC + rDegQuad - 15.0);

/* Balanced Equal house systems split the difference between Asc and MC. */
#define HouseEqualBalanced() HouseEqualGeneric(Midpoint(is.Asc, is.MC) + 45.0)
#define HouseWholeBalanced() HouseEqualGeneric((real)\
  ((SFromZ(Midpoint(is.Asc, is.MC) + 15.0)-1)*30 + 30.0))
#define HouseVedicBalanced() HouseEqualGeneric(Midpoint(is.Asc, is.MC) + 30.0)

/* In "null" houses, the cusps are fixed to start at their corresponding */
/* sign, i.e. the 1st house is always at 0 degrees Aries, etc.           */
#define HouseNull() HouseEqualGeneric(0.0);


/* Calculate the house cusp positions, using the specified system. Note  */
/* this is only called when Swiss Ephemeris is NOT computing the houses. */

void ComputeHouses(int housesystem)
{
  char sz[cchSzDef];

  // Don't allow polar latitudes if system not defined in polar zones.
  if ((housesystem == hsPlacidus || housesystem == hsKoch) &&
    RAbs(AA) >= rDegQuad - is.OB) {
    sprintf(sz,
      "The %s system of houses is not defined at extreme latitudes.",
      szSystem[housesystem]);
    PrintWarning(sz);
    housesystem = hsPorphyry;
  }

  // Flip the Ascendant or MC if it falls in the wrong half of the zodiac.
  if (MinDifference(is.MC, is.Asc) < 0.0) {
    if (us.fPolarAsc)
      is.MC = Mod(is.MC + rDegHalf);
    else
      is.Asc = Mod(is.Asc + rDegHalf);
  }

  switch (housesystem) {
#ifdef MATRIX
  case hsPlacidus:      HousePlacidus();              break;
  case hsKoch:          HouseKoch();                  break;
  case hsCampanus:      HouseCampanus();              break;
  case hsMeridian:      HouseMeridian();              break;
  case hsRegiomontanus: HouseRegiomontanus();         break;
  case hsMorinus:       HouseMorinus();               break;
  case hsTopocentric:   HouseTopocentric();           break;
#endif
  case hsEqual:         HouseEqual();                 break;
  case hsPorphyry:      HousePorphyry();              break;
  case hsAlcabitius:    HouseAlcabitius();            break;
  case hsEqualMC:       HouseEqualMC();               break;
  case hsSinewaveRatio: HousePullenSinusoidalRatio(); break;
  case hsSinewaveDelta: HousePullenSinusoidalDelta(); break;
  case hsWhole:         HouseWhole();                 break;
  case hsVedic:         HouseVedic();                 break;
  case hsSripati:       HouseSripati();               break;
  // New experimental house systems follow:
  case hsWholeMC:       HouseWholeMC();               break;
  case hsVedicMC:       HouseVedicMC();               break;
  case hsEqualBalanced: HouseEqualBalanced();         break;
  case hsWholeBalanced: HouseWholeBalanced();         break;
  case hsVedicBalanced: HouseVedicBalanced();         break;
  default:              HouseNull();
    housesystem = hsNull;
  }
  is.nHouseSystem = housesystem;
}


/*
******************************************************************************
** Star Position Calculations.
******************************************************************************
*/

/* This is used by the chart calculation routine to calculate the positions */
/* of the fixed stars. Since stars don't move much in the sky over time,    */
/* getting their positions is mostly just reading info from an array and    */
/* converting it to the correct reference frame. However, we have to add    */
/* in the correct precession for the tropical zodiac.                       */

void ComputeStars(real t, real Off)
{
#ifdef MATRIX
  int i;
  real x, y, z;
#endif

  // Read in star positions.

#ifdef SWISS
  if (FCmSwissStar())
    SwissComputeStars(t, fFalse);
  else
#endif
  {
#ifdef MATRIX
    for (i = 1; i <= cStar; i++) {
      x = rStarData[i*6-6]; y = rStarData[i*6-5]; z = rStarData[i*6-4];
      planet[oNorm+i] = x*rDegMax/24.0 + y*15.0/60.0 + z*0.25/60.0;
      x = rStarData[i*6-3]; y = rStarData[i*6-2]; z = rStarData[i*6-1];
      if (x < 0.0) {
        neg(y); neg(z);
      }
      planetalt[oNorm+i] = x + y/60.0 + z/60.0/60.0;
      // Convert to ecliptic zodiac coordinates.
      EquToEcl(&planet[oNorm+i], &planetalt[oNorm+i]);
      planet[oNorm+i] = Mod(planet[oNorm+i] + rEpoch2000 + Off);
      if (!us.fSidereal)
        ret[oNorm+i] = !us.fVelocity ? rDegMax/25765.0/rDayInYear : 1.0;
      SphToRec(rStarDist[i], planet[oNorm+i], planetalt[oNorm+i],
        &space[oNorm+i].x, &space[oNorm+i].y, &space[oNorm+i].z);
    }
#endif
  }
}


/* Given the list of computed planet positions, sort and compose the final   */
/* index list based on what order the planets are supposed to be printed in. */

void SortPlanets()
{
  int i;
#ifdef EXPRESS
  real rgrSort[oNorm1];
  int j;
#endif

  // By default, objects are displayed in object index order.
  for (i = 0; i <= cObj; i++)
    rgobjList[i] = i;

#ifdef EXPRESS
  // Adjust indexes used for display with AstroExpressions.
  if (!FSzSet(us.szExpSort))
    return;
  for (i = 0; i <= oNorm; i++) {
    ExpSetN(iLetterZ, i);
    ParseExpression(us.szExpSort);
    rgrSort[i] = RExpGet(iLetterZ);
  }

  // Sort adjusted list to determine final display ordering.
  for (i = 1; i <= oNorm; i++) {
    j = i-1;
    while (j >= 0 && rgrSort[rgobjList[j]] > rgrSort[rgobjList[j+1]]) {
      SwapN(rgobjList[j], rgobjList[j+1]);
      j--;
    }
  }
#endif
}


/* Given the list of computed star positions, sort and compose the final   */
/* index list based on what order the stars are supposed to be printed in. */

void SortStars()
{
  int i, j;

  // Sort the index list if one of the special -U subswitches is in effect.

  if (us.nStar <= 1)
    return;
  for (i = starLo+1; i <= starHi; i++) {
    j = i-1;

    // Compare star names for -Un switch.
    if (us.nStar == 'n') while (j >= starLo && NCompareSz(
      szObjDisp[rgobjList[j]], szObjDisp[rgobjList[j+1]]) > 0) {
      SwapN(rgobjList[j], rgobjList[j+1]);
      j--;

    // Compare star brightnesses for -Ub switch.
    } else if (us.nStar == 'b') while (j >= starLo &&
      rStarBright[rgobjList[j]-oNorm] > rStarBright[rgobjList[j+1]-oNorm]) {
      SwapN(rgobjList[j], rgobjList[j+1]);
      j--;

    // Compare star zodiac locations for -Uz switch.
    } else if (us.nStar == 'z') while (j >= starLo &&
      planet[rgobjList[j]] > planet[rgobjList[j+1]]) {
      SwapN(rgobjList[j], rgobjList[j+1]);
      j--;

    // Compare star latitudes for -Ul switch.
    } else if (us.nStar == 'l') while (j >= starLo &&
      planetalt[rgobjList[j]] < planetalt[rgobjList[j+1]]) {
      SwapN(rgobjList[j], rgobjList[j+1]);
      j--;

    // Compare star distances for -Ud switch.
    } else if (us.nStar == 'd') while (j >= starLo &&
      rStarDist[rgobjList[j]-oNorm] > rStarDist[rgobjList[j+1]-oNorm]) {
      SwapN(rgobjList[j], rgobjList[j+1]);
      j--;

    // Compare star velocities for -Uv switch.
    } else if (us.nStar == 'v') while (j >= starLo &&
      ret[rgobjList[j]] < ret[rgobjList[j+1]]) {
      SwapN(rgobjList[j], rgobjList[j+1]);
      j--;
    }
  }
}


/*
******************************************************************************
** Chart Calculation.
******************************************************************************
*/

/* Given a zodiac degree, transform it into its Decan sign, where each    */
/* sign is trisected into the three signs of its element. For example,    */
/* 1 Aries -> 3 Aries, 10 Leo -> 0 Sagittarius, 25 Sagittarius -> 15 Leo. */

real Decan(real deg)
{
  int sign;
  real unit;

  sign = SFromZ(deg);
  unit = deg - ZFromS(sign);
  sign = Mod12(sign + 4*((int)RFloor(unit/10.0)));
  unit = (unit - RFloor(unit/10.0)*10.0)*3.0;
  return ZFromS(sign)+unit;
}


/* Given a zodiac degree, transform it into its Dwad sign, in which each   */
/* sign is divided into twelfths, starting with its own sign. For example, */
/* 15 Aries -> 0 Libra, 10 Leo -> 0 Sagittarius, 20 Sagittarius -> 0 Leo.  */

real Dwad(real deg)
{
  int sign;
  real unit;

  sign = SFromZ(deg);
  unit = deg - ZFromS(sign);
  sign = Mod12(sign + ((int)RFloor(unit/2.5)));
  unit = (unit - RFloor(unit/2.5)*2.5)*12.0;
  return ZFromS(sign)+unit;
}


/* Given a zodiac degree, transform it into its Navamsa position, where   */
/* each sign is divided into ninths, which determines the number of signs */
/* after a base element sign to use. Degrees within signs are unaffected. */

real Navamsa(real deg)
{
  int sign, sign2;
  real unit;

  sign = SFromZ(deg);
  unit = deg - ZFromS(sign);
  sign2 = Mod12(((sign-1 & 3)^(2*FOdd(sign-1)))*3 + (int)(unit*0.3) + 1);
  return ZFromS(sign2) + unit;
}


/* Transform rectangular coordinates in x, y to polar coordinates. */

void RecToPol(real x, real y, real *a, real *r)
{
  *r = RLength2(x, y);
  *a = RAngle(x, y);
}


/* Transform spherical to rectangular coordinates in x, y, z. */

void SphToRec(real r, real azi, real alt, real *rx, real *ry, real *rz)
{
  real rT;

  *rz = r *RSinD(alt);
  rT  = r *RCosD(alt);
  *rx = rT*RCosD(azi);
  *ry = rT*RSinD(azi);
}


/* Convert 3D rectangular to spherical coordinates. */

void RecToSph3(real rx, real ry, real rz, real *azi, real *alt)
{
  real ang, rad;

  RecToPol(rx, ry, &ang, &rad);
  *azi = DFromR(ang);
  ang = RAngleD(rad, rz);
  // Ensure latitude is from -90 to +90 degrees.
  while (ang > rDegQuad)
    ang -= rDegHalf;
  while (ang < -rDegQuad)
    ang += rDegHalf;
  *alt = ang;
}


/* Do a coordinate transformation: Given a longitude and latitude value,    */
/* return the new longitude and latitude values that the same location      */
/* would have, were the equator tilted by a specified number of degrees.    */
/* In other words, do a pole shift! This is used to convert among ecliptic, */
/* equatorial, and local coordinates, each of which have zero declination   */
/* in different planes. In other words, take into account the Earth's axis. */

void CoorXform(real *azi, real *alt, real tilt)
{
  real x, y, a1, l1;
  real sinalt, cosalt, sinazi, sintilt, costilt;

  *azi = RFromD(*azi); *alt = RFromD(*alt); tilt = RFromD(tilt);
  sinalt = RSin(*alt); cosalt = RCos(*alt); sinazi = RSin(*azi);
  sintilt = RSin(tilt); costilt = RCos(tilt);

  x = cosalt * sinazi * costilt - sinalt * sintilt;
  y = cosalt * RCos(*azi);
  l1 = RAngle(y, x);
  a1 = cosalt * sinazi * sintilt + sinalt * costilt;
  a1 = RAsin(a1);
  *azi = DFromR(l1); *alt = DFromR(a1);
}


/* Another subprocedure of the ComputeEphem() routine. Convert the final */
/* rectangular coordinates of a planet to zodiac position and latitude.  */

void ProcessPlanet(int ind, real aber)
{
  real ang, rad;

  RecToPol(space[ind].x, space[ind].y, &ang, &rad);
  planet[ind] = Mod(DFromR(ang) - aber + is.rSid);
  RecToPol(rad, space[ind].z, &ang, &rad);
  if (us.objCenter == oSun && ind == oSun)
    ang = 0.0;
  ang = DFromR(ang);
  // Ensure latitude is from -90 to +90 degrees.
  while (ang > rDegQuad)
    ang -= rDegHalf;
  while (ang < -rDegQuad)
    ang += rDegHalf;
  planetalt[ind] = ang;
}


#ifdef EPHEM
/* Compute the positions of the planets at a certain time using the Swiss  */
/* Ephemeris accurate formulas. This will supersede the Matrix routine     */
/* values and is only called when the -b switch is in effect. Not all      */
/* objects or modes are available using this, but some additional values   */
/* such as Moon and Node velocities not available without -b are. (This is */
/* the main place in Astrolog which calls the Swiss Ephemeris functions.)  */

void ComputeEphem(real t)
{
  int i, j;
  real r1, r2, r3, r4, r5, dist1, dist2;

  // Can compute the positions of Sun through Pluto, Chiron, the four
  // asteroids, Lilith, and the North Node using ephemeris files.

  for (i = oSun; i <= uranHi; i++) {
    if ((ignore[i] &&
      i != us.objCenter && i > oMoo && (i != oNod || ignore[oSou])) ||
      FBetween(i, oFor, cuspHi) ||
      (us.fPlacalcPla && i >= oFor) ||
      (us.fPlacalcAst && FBetween(i, oCer, oVes)))
      continue;
    if (
#ifdef EPHEM2
      !us.fPlacalcPla ?
#endif
#ifdef SWISS
      FSwissPlanet(i, JulianDayFromTime(t), us.objCenter != oEar,
        &r1, &r2, &r3, &r4, &r5)
#endif
#ifdef EPHEM2
      :
#endif
#ifdef PLACALC
      FPlacalcPlanet(i, JulianDayFromTime(t), us.objCenter != oEar,
        &r1, &r2, &r3, &r4, &r5)
#endif
      ) {
      planet[i]    = Mod(r1 + is.rSid);
      planetalt[i] = r2;
      ret[i]       = r3;
      retalt[i]    = r5;
      if (us.fVelocity && i <= oLil) {
        ret[i] /= (rDegMax / (rObjYear[i == oSun ||
          (i == oMoo && us.objCenter != oEar) ? oEar : i] * rDayInYear));
        if (i == oNod || i == oSou)
          neg(ret[i]);
      }
    } else
      r4 = 0.0;

    // Compute x,y,z coordinates from azimuth, altitude, and distance.

    SphToRec(r4, planet[i], planetalt[i],
      &space[i].x, &space[i].y, &space[i].z);
  }

  // If heliocentric, move Earth position to object slot zero.

  if (!ignore[oSou]) {
    space[oSou].x = -space[oNod].x;
    space[oSou].y = -space[oNod].y;
    space[oSou].z = -space[oNod].z;
    planet[oSou] = Mod(planet[oNod] + rDegHalf);
    ret[oSou] = ret[oNod];
  }
  if (us.objCenter == oEar) {
    if (us.fBarycenter) {
      space[oSun].x = -space[oSun].x;
      space[oSun].y = -space[oSun].y;
      space[oSun].z = -space[oSun].z;
      planet[oSun] = Mod(planet[oSun] + rDegHalf);
      planetalt[oSun] = -planetalt[oSun];
    }
    goto LDone;
  }
  planet[oEar] = planet[oSun];
  planetalt[oEar] = planetalt[oSun];
  ret[oEar] = ret[oSun];
  space[oEar] = space[oSun];
  planet[oSun] = planetalt[oSun] =
    space[oSun].x = space[oSun].y = space[oSun].z = 0.0;
  for (i = oNod; i <= oLil; i++) if (!ignore[i]) {
    space[i].x += space[oEar].x;
    space[i].y += space[oEar].y;
    space[i].z += space[oEar].z;
    if (us.objCenter == oSun)
      ProcessPlanet(i, 0.0);
    ret[i] = ret[oEar];
  }
  if (us.objCenter == oSun)
    goto LDone;

  // If other planet centered, shift all positions by central planet.

  for (i = 0; i <= cObj; i++)
    if ((!ignore[i] || i == oSun) && i != us.objCenter) {
      if (us.fStarMagDist && FStar(i))
        dist1 = RLength3(space[i].x, space[i].y, space[i].z);
      space[i].x -= space[us.objCenter].x;
      space[i].y -= space[us.objCenter].y;
      space[i].z -= space[us.objCenter].z;
      if (us.fStarMagDist && FStar(i)) {
        dist2 = us.fStarMagAbs ? 10.0 * rPCToAU :
          RLength3(space[i].x, space[i].y, space[i].z);
        rStarBright[i-oNorm] =
          RStarBright(rStarBrightDef[i-oNorm], dist1, dist2);
        kObjA[i] = KStarA(rStarBright[i-oNorm]);
      }
      ProcessPlanet(i, us.objCenter > oSun && us.fSidereal ? is.rSid : 0.0);
    }
  planet[us.objCenter] = planetalt[us.objCenter] = space[us.objCenter].x =
    space[us.objCenter].y = space[us.objCenter].z = 0.0;

LDone:
  // Potentially relocate objects so they orbit the central planet.
  if (!us.fMoonMove)
    return;
  for (i = oMoo; i <= oNorm; i++)
    if (!ignore[i]) {
      j = ObjOrbit(i);
      if (j < 0 || j == us.objCenter || (j == oSun && us.objCenter < starLo))
        continue;
      space[i].x -= space[j].x;
      space[i].y -= space[j].y;
      space[i].z -= space[j].z;
      ProcessPlanet(i, us.objCenter > oSun && us.fSidereal ? is.rSid : 0.0);
    };
}
#endif


/* This is probably the main routine in all of Astrolog. It generates a   */
/* chart, calculating the positions of all the celestial bodies and house */
/* cusps, based on the current chart information, and saves them for use  */
/* by any of the display routines.                                        */

real CastChart(int nContext)
{
  CI ciSav;
  real housetemp[cSign+1], vtx = 0.0, ep = 0.0, r, r2;
  int i, k, k2;

  is.nContext = nContext;
#ifdef EXPRESS
  // Notify AstroExpression a chart is about to be cast.
  if (!us.fExpOff && FSzSet(us.szExpCast1))
    ParseExpression(us.szExpCast1);
#endif

  // Hack: If month is negative, then know chart was read in through a -o0
  // position file, so planet positions are already in the arrays.

  if (FNoTimeOrSpace(ciCore)) {
    is.MC = planet[oMC]; is.Asc = planet[oAsc];
    ComputeInHouses();
    return 0.0;
  }

  // Hack: Time zone 24 means to have the time of day be in Local Mean Time
  // (LMT). This is done by making the time zone value reflect the logical
  // offset from UTC as indicated by the chart's longitude value.

  ciSav = ciCore;
  if (ZZ == zonLMT)
    ZZ = OO / 15.0;
  if (SS == dstAuto)
    SS = (real)is.fDst;
  TT = RSgn(TT)*RFloor(RAbs(TT))+RFract(RAbs(TT)) + (ZZ - SS);
  AA = Min(AA, rDegQuad-rSmall);     // Make sure chart isn't being cast on
  AA = Max(AA, -(rDegQuad-rSmall));  // precise North or South Pole.

  ClearB((pbyte)&cp0, sizeof(CP));     // On ecliptic unless say otherwise.
  ClearB((pbyte)space, sizeof(space));

  is.JD = (real)MdyToJulian(MM, DD, YY);
  is.T = (is.JD + TT/24.0);
  if (us.fProgress && us.nProgress != ptSolarArc) {

    // For ptCast, is.Tp is time that progressed chart cusps cast for.
    // For ptMixed, is.Tp is base chart time to solar arc cusps from.
    is.Tp = is.T;
    if (us.nProgress == ptCast)
      is.Tp += ((is.JDp - is.Tp) / (us.rProgDay * us.rProgCusp));
    is.Tp = (is.Tp - 2415020.5) / 36525.0;

    // Determine actual time that a progressed chart is to be cast for.
    is.T += ((is.JDp - is.T) / us.rProgDay);

#ifdef EXPRESS
    // Adjust progression times with AstroExpressions.
    if (!us.fExpOff && FSzSet(us.szExpProg)) {
      ExpSetR(iLetterX, is.JDp);
      ExpSetR(iLetterY, is.Tp);
      ExpSetR(iLetterZ, is.T);
      ParseExpression(us.szExpProg);
      is.Tp = RExpGet(iLetterY);
      is.T  = RExpGet(iLetterZ);
    }
#endif
  }
  is.T = (is.T - 2415020.5) / 36525.0;

#ifdef SWISS
  if (FCmSwissAny()) {
    SwissHouse(us.fProgress && us.nProgress != ptSolarArc ? is.Tp : is.T,
      OO, AA, us.nHouseSystem,
      &is.Asc, &is.MC, &is.RA, &vtx, &ep, &is.OB, &is.rOff);
  } else
#endif
  {
#ifdef MATRIX
    is.rOff = ProcessInput();
    ComputeVariables(&vtx);
    if (us.fGeodetic)                // Check for -G geodetic chart.
      is.RA = RFromD(Mod(-OO));
    is.MC  = CuspMidheaven();        // Calculate Ascendant & Midheaven.
    is.Asc = CuspAscendant();
    ep = CuspEastPoint();
    ComputeHouses(us.nHouseSystem);  // Go calculate house cusps.
#endif
  }

#ifdef MATRIX
  // Go calculate planet, Moon, and North Node positions.

  if (FCmMatrix() || (FCmPlacalc() && us.fUranian)) {
    ComputePlanets();
    if (!ignore[oMoo] || !ignore[oNod] || !ignore[oSou] || !ignore[oFor]) {
      ComputeLunar(&planet[oMoo], &planetalt[oMoo],
        &planet[oNod], &planetalt[oNod]);
      ret[oNod] = -1.0;
    }
  }
#endif

  // Go calculate star positions if -U switch in effect.

  if (us.nStar || FStar(us.objCenter))
    ComputeStars(is.T, (us.fSidereal ? 0.0 : -is.rOff) + us.rZodiacOffset);

#ifdef EPHEM
  // Compute more accurate ephemeris positions for certain objects.

  if (us.fEphemFiles)
    ComputeEphem(is.T);
#endif

  // Certain objects are positioned directly opposite to other objects.

  i = us.objCenter == oEar ? oSun : oEar;
  planet[us.objCenter] = Mod(planet[i] + rDegHalf);
  planetalt[us.objCenter] = -planetalt[i];
  ret[us.objCenter] = ret[i];
  if (!us.fEphemFiles) {
    planet[oSou] = Mod(planet[oNod] + rDegHalf);
    if (!us.fVelocity) {
      ret[oNod] = ret[oSou] = -0.053;
      ret[oMoo] = 12.2;
    } else
      ret[oNod] = ret[oSou] = ret[oMoo] = 1.0;
  }

  // Calculate position of Part of Fortune.

  if (!ignore[oFor]) {
    r = MinDifference(planet[oSun], planet[oMoo]);
    r2 = ret[oMoo] - ret[oSun];
    planetalt[oFor] = us.fHouse3D ? planetalt[oMoo] - planetalt[oSun] : 0.0;
    retalt[oFor] = us.fHouse3D ? retalt[oMoo] - retalt[oSun] : 0.0;
    // Invert formula for night charts. Note since planet positions are still
    // being computed, house placements haven't been determined yet.
    i = us.nHouseSystem; us.nHouseSystem = hsCampanus;
    if (us.nArabicNight < 0 || (us.nArabicNight == 0 &&
      NHousePlaceIn(planet[oSun], planetalt[oSun]) < sLib)) {
      neg(r); neg(r2);
      neg(planetalt[oFor]); neg(retalt[oFor]);
    }
    us.nHouseSystem = i;
    planet[oFor] = Mod(r + is.Asc);
    ret[oFor] += r2;                 // Already contains ret[oAsc].
  }

  // Fill in "planet" positions corresponding to house cusps.

  planet[oVtx] = vtx; planet[oEP] = ep;
  for (i = 1; i <= cSign; i++)
    planet[cuspLo + i - 1] = chouse[i];
  if (!us.fHouseAngle) {
    planet[oAsc] = is.Asc; planet[oMC] = is.MC;
    planet[oDes] = Mod(is.Asc + rDegHalf);
    planet[oNad] = Mod(is.MC + rDegHalf);
  }
  for (i = oFor; i <= cuspHi; i++) {
    r = FCmSwissAny() ? ret[i] : (rDegMax + 1.0);
    if (us.fVelocity)
      r /= (rDegMax + 1.0);
    ret[i] = r;
  }

  // Transform ecliptic to equatorial coordinates if -sr in effect.

  if (us.fEquator || us.fEquator2)
    for (i = 0; i <= cObj; i++) if (!ignore[i]) {
      r = Tropical(planet[i]); r2 = planetalt[i];
      EclToEqu(&r, &r2);
      if (us.fEquator)
        planet[i] = r;
      if (us.fEquator2)
        planetalt[i] = r2;
    }

  // Now, may have to modify the base positions calculated above based on what
  // type of chart is being generated. To begin with: Solar arc progressions
  // apply an offset to planets and/or houses.

  if (us.fProgress && us.nProgress != ptCast) {
    r2 = JulianDayFromTime(us.nProgress == ptSolarArc ? is.T : is.Tp);
    r = (is.JDp - r2 - 0.5) / us.rProgDay;
#ifdef EXPRESS
    // Adjust progression arc with AstroExpression.
    if (!us.fExpOff && FSzSet(us.szExpProg0)) {
      ExpSetR(iLetterX, is.JDp);
      ExpSetR(iLetterY, r2);
      ExpSetR(iLetterZ, r);
      ParseExpression(us.szExpProg0);
      r = RExpGet(iLetterZ);
    }
#endif
    // Full solar arc progressions apply offset to all planets.
    if (us.nProgress == ptSolarArc) {
      for (i = 0; i <= cObj; i++) {
        if (i == oFor)
          i = cuspHi+1;
        planet[i] = Mod(planet[i] + r);
      }
    }
    // Mixed solar arc progressions only apply offset to house cusps.
    r /= us.rProgCusp;
    for (i = oFor; i <= cuspHi; i++)
      planet[i] = Mod(planet[i] + r);
    for (i = 1; i <= cSign; i++)
      chouse[i] = Mod(chouse[i] + r);
  }

  // If -x harmonic chart in effect, then multiply all planet positions.

  if (us.rHarmonic != 1.0)
    for (i = 0; i <= cObj; i++)
      planet[i] = Mod(planet[i] * us.rHarmonic);

  // If -Y1 chart rotation in effect, then rotate the planets accordingly.

  if (us.objRot1 != us.objRot2 || us.fObjRotWhole) {
    r = planet[us.objRot2];
    if (us.fObjRotWhole)
      r = (real)((SFromZ(r)-1)*30);
    r -= planet[us.objRot1];
    for (i = 0; i <= cObj; i++)
      planet[i] = Mod(planet[i] + r);
  }

  // If -1 or -2 solar chart in effect, then rotate the houses accordingly.

  if (us.objOnAsc) {
    r = planet[NAbs(us.objOnAsc)-1];
    if (us.fSolarWhole)
      r = (real)((SFromZ(r)-1)*30);
    r -= (us.objOnAsc > 0 ? is.Asc : is.MC);
    for (i = 1; i <= cSign; i++)
      chouse[i] = Mod(chouse[i] + r + rSmall);
  }

  // Check to see if are -F forcing any objects to be particular values.

  for (i = 0; i <= cObj; i++)
    if (force[i] != 0.0) {
      if (force[i] > 0.0) {
        // Force to a specific zodiac position.
        planet[i] = force[i]-rDegMax;
        planetalt[i] = ret[i] = 0.0;
      } else {
        // Force to a midpoint of two other positions.
        k = (-(int)force[i])-1;
        k2 = k % cObj; k /= cObj;
        planet[i] = Midpoint(planet[k], planet[k2]);
        planetalt[i] = (planetalt[k] + planetalt[k2]) / 2.0;
        ret[i] = (ret[k] + ret[k2]) / 2.0;
      }
    }

  // If -f domal chart switch in effect, switch planet and house positions.

  if (us.fFlip) {
    ComputeInHouses();
    for (i = 0; i <= cObj; i++) {
      k = inhouse[i];
      inhouse[i] = SFromZ(planet[i]);
      planet[i] = ZFromS(k)+MinDistance(chouse[k], planet[i]) /
        MinDistance(chouse[k], chouse[Mod12(k+1)])*30.0;
    }
    for (i = 1; i <= cSign; i++) {
      k = NHousePlaceIn2D(ZFromS(i));
      housetemp[i] = ZFromS(k)+MinDistance(chouse[k], ZFromS(i)) /
        MinDistance(chouse[k], chouse[Mod12(k+1)])*30.0;
    }
    for (i = 1; i <= cSign; i++)
      chouse[i] = housetemp[i];
  }

  // If -3 decan chart switch in effect, edit planet positions accordingly.

  if (us.fDecan)
    for (i = 0; i <= cObj; i++)
      planet[i] = Decan(planet[i]);

  // If -4 dwad chart switch in effect, edit planet positions accordingly.

  if (us.nDwad > 0)
    for (k = 0; k < us.nDwad; k++)
      for (i = 0; i <= cObj; i++)
        planet[i] = Dwad(planet[i]);

  // If -9 navamsa chart switch in effect, edit planet positions accordingly.

  if (us.fNavamsa)
    for (i = 0; i <= cObj; i++)
      planet[i] = Navamsa(planet[i]);

  // Sort star positions now that all positions are finalized.

  SortPlanets();
  if (us.nStar || FStar(us.objCenter))
    SortStars();

#ifdef EXPRESS
  // Adjust final planet and house positions with AstroExpressions.

  if (!us.fExpOff) {
    if (FSzSet(us.szExpObj))
      for (i = 0; i <= cObj; i++) if (!ignore[i] || FCusp(i)) {
        ExpSetN(iLetterV, i);
        ExpSetR(iLetterW, planet[i]);
        ExpSetR(iLetterX, planetalt[i]);
        ExpSetR(iLetterY, ret[i]);
        ExpSetR(iLetterZ, retalt[i]);
        ParseExpression(us.szExpObj);
        planet[i]    = Mod(RExpGet(iLetterW));
        planetalt[i] = RExpGet(iLetterX);
        ret[i]       = RExpGet(iLetterY);
        retalt[i]    = RExpGet(iLetterZ);
      }
    if (FSzSet(us.szExpHou))
      for (i = 1; i <= cSign; i++) {
        ExpSetN(iLetterX, i);
        ExpSetR(iLetterY, chouse[i]);
        ExpSetR(iLetterZ, chouse3[i]);
        ParseExpression(us.szExpHou);
        chouse[i]  = Mod(RExpGet(iLetterY));
        chouse3[i] = Mod(RExpGet(iLetterZ));
      }
  }
#endif

  ComputeInHouses();    // Figure out what house everything falls in.
#ifdef EXPRESS
  // Notify AstroExpression a chart has just been cast.
  if (!us.fExpOff && FSzSet(us.szExpCast2))
    ParseExpression(us.szExpCast2);
#endif
  ciCore = ciSav;
  return is.T;
}


/* Calculate the position of each planet with respect to the Gauquelin      */
/* sectors. This is used by the sector charts. Fill out the planet position */
/* array where one degree means 1/10 the way across one of the 36 sectors.  */

void CastSectors()
{
  int source[MAXINDAY], type[MAXINDAY], occurcount, division, div,
    i, j, s1, s2, ihouse, fT;
  real time[MAXINDAY], rgalt1[objMax], rgalt2[objMax],
    azi1, azi2, alt1, alt2, mc1, mc2, d, k;

  // If the -l0 approximate sectors flag is set, we can quickly get rough
  // positions by having each position be the location of the planet as mapped
  // into Placidus houses. The -f flip houses flag does this for us.

  if (us.fSectorApprox) {
    ihouse = us.nHouseSystem; us.nHouseSystem = hsPlacidus;
    inv(us.fFlip);
    CastChart(0);
    inv(us.fFlip);
    us.nHouseSystem = ihouse;
    return;
  }

  // If not approximating sectors, then they need to be computed the formal
  // way: based on a planet's nearest rising and setting times. The code below
  // is similar to ChartInDayHorizon() accessed by the -Zd switch.

  fT = us.fSidereal; us.fSidereal = fFalse;
  division = us.nDivision * 4;
  occurcount = 0;

  // Start scanning from 18 hours before to 18 hours after the time of the
  // chart in question, to find the closest rising and setting times.

  ciCore = ciMain; ciCore.tim -= 18.0;
  if (ciCore.tim < 0.0) {
    ciCore.tim += 24.0;
    ciCore.day--;
  }
  CastChart(0);
  mc2 = planet[oMC]; k = planetalt[oMC];
  EclToEqu(&mc2, &k);
  cp2 = cp0;
  for (i = 0; i <= cObj; i++)
    rgalt2[i] = planetalt[i];

  // Loop through 36 hours, dividing it into a certain number of segments.
  // For each segment we get the planet positions at its endpoints.

  for (div = 1; div <= division; div++) {
    ciCore = ciMain;
    ciCore.tim = ciCore.tim - 18.0 + 36.0*(real)div/(real)division;
    if (ciCore.tim < 0.0) {
      ciCore.tim += 24.0;
      ciCore.day--;
    } else if (ciCore.tim >= 24.0) {
      ciCore.tim -= 24.0;
      ciCore.day++;
    }
    CastChart(-1);
    mc1 = mc2;
    mc2 = planet[oMC]; k = planetalt[oMC];
    EclToEqu(&mc2, &k);
    cp1 = cp2; cp2 = cp0;
    for (i = 1; i <= cObj; i++) {
      rgalt1[i] = rgalt2[i]; rgalt2[i] = planetalt[i];
    }

    // During our segment, check to see if each planet rises or sets.

    for (i = 0; i <= cObj; i++) if (!FIgnore(i) && FThing(i)) {
      EclToHorizon(&azi1, &alt1, cp1.obj[i], rgalt1[i], mc1, Lat);
      EclToHorizon(&azi2, &alt2, cp2.obj[i], rgalt2[i], mc2, Lat);
      j = 0;
      if ((alt1 > 0.0) != (alt2 > 0.0)) {
        d = RAbs(alt1)/(RAbs(alt1)+RAbs(alt2));
        k = Mod(azi1 + d*MinDifference(azi1, azi2));
        j = 1 + (MinDistance(k, rDegHalf) < rDegQuad);
      }
      if (j && occurcount < MAXINDAY) {
        source[occurcount] = i;
        type[occurcount] = j;
        time[occurcount] = 36.0*((real)(div-1)+d)/(real)division*60.0;
        occurcount++;
      }
    }
  }

  // Sort each event in order of time when it happens during the day.

  for (i = 1; i < occurcount; i++) {
    j = i-1;
    while (j >= 0 && time[j] > time[j+1]) {
      SwapN(source[j], source[j+1]);
      SwapN(type[j], type[j+1]);
      SwapR(&time[j], &time[j+1]);
      j--;
    }
  }

  // Now fill out the planet array with the appropriate sector location.

  for (i = 0; i <= cObj; i++) if (!ignore[i] && FThing(i)) {
    planet[i] = 0.0;
    // Search for the first rising or setting event of our planet.
    for (s2 = 0; s2 < occurcount && source[s2] != i; s2++)
      ;
    if (s2 == occurcount) {
LFail:
      // If we failed to find a rising/setting bracket around our time,
      // automatically restrict that planet so it doesn't show up.
      ignore[i] = fTrue;
      continue;
    }
LRetry:
    // One rising or setting event was found. Now search for the next one.
    s1 = s2;
    for (s2 = s1 + 1; s2 < occurcount && source[s2] != i; s2++)
      ;
    if (s2 == occurcount)
      goto LFail;
    // Reject the two events if either (1) they're both the same, i.e. both
    // rising or both setting, or (2) they don't bracket the chart's time.
    if (type[s2] == type[s1] || time[s1] > 18.0*60.0 || time[s2] < 18.0*60.0)
      goto LRetry;
    // Cool, found the rising/setting bracket. The sector position is the
    // proportion the chart time is between the two event times.
    planet[i] = (18.0*60.0 - time[s1])/(time[s2] - time[s1])*rDegHalf;
    if (type[s1] == 2)
      planet[i] += rDegHalf;
    planet[i] = Mod(rDegMax - planet[i]);
  }

  // Restore original chart info as have overwritten it.

  ciCore = ciMain;
  us.fSidereal = fT;
}


/*
******************************************************************************
** Aspect Calculations.
******************************************************************************
*/

/* Set up the aspect/midpoint grid. Allocate memory for this array, if not */
/* already done. Allocation is only done once, first time this is called.  */

flag FEnsureGrid(void)
{
  if (grid != NULL)
    return fTrue;
  grid = (GridInfo *)PAllocate(sizeof(GridInfo), "grid");
  return grid != NULL;
}


/* Indicate whether some aspect between two objects should be shown. */

flag FAcceptAspect(int obj1, int asp, int obj2)
{
  int oCen;
  flag fSupp, fTrans;

  // Negative aspect means context is a transit to transit consideration.
  fTrans = (asp < 0);
  if (fTrans)
    neg(asp);

  // If the aspect restricted, reject immediately.
  if (FIgnoreA(asp))
    return fFalse;
  if (us.objRequire >= 0 && obj1 != us.objRequire && obj2 != us.objRequire)
    return fFalse;

  // Transits always need to prevent aspects involving continually opposite
  // objects, to prevent exact aspect events numerous times per day.
  oCen = us.objCenter == oSun ? oEar : us.objCenter;
  if (!us.fSmartCusp) {
    if (!fTrans)
      return fTrue;
    if (us.objCenter == oEar &&
      (obj1 == oNod || obj2 == oNod) && (obj1 == oSou || obj2 == oSou))
      return fFalse;
    if ((obj1 == oSun || obj2 == oSun) && (obj1 == oCen || obj2 == oCen))
      return fFalse;
    return fTrue;
  }

  // Allow only conjunctions to the minor house cusps.
  if ((FMinor(obj1) || FMinor(obj2)) && asp > aCon)
    return fFalse;

  // Is this a weak aspect supplemental to a stronger one present?
  fSupp = (asp == aOpp && !FIgnoreA(aCon)) ||
    (asp == aSex && !FIgnoreA(aTri)) || (asp == aSSx && !FIgnoreA(aInc)) ||
    (asp == aSes && !FIgnoreA(aSSq));

  // Prevent any simultaneous aspects to opposing angle cusps, e.g. if
  // conjunct one, don't be opposite the other; if trine one, don't sextile
  // the other; don't square both at once, etc.
  if (!ignore[oMC] && !ignore[oNad] &&
    (((obj1 == oMC || obj2 == oMC) && fSupp) ||
    ((obj1 == oNad || obj2 == oNad) && (fSupp || asp == aSqu))))
    return fFalse;
  if (!ignore[oAsc] && !ignore[oDes] &&
    (((obj1 == oAsc || obj2 == oAsc) && fSupp) ||
    ((obj1 == oDes || obj2 == oDes) && (fSupp || asp == aSqu))))
    return fFalse;

  // Prevent any simultaneous aspects to the North and South Node.
  if (us.objCenter == oEar && !ignore[oNod] && !ignore[oSou] &&
    (((obj1 == oNod || obj2 == oNod) && fSupp) ||
    ((obj1 == oSou || obj2 == oSou) && (fSupp || asp == aSqu))))
    return fFalse;

  // Prevent any simultaneous aspects to the Sun and central planet.
  if (!ignore[oCen] && !ignore[oSun] &&
    (((obj1 == oSun || obj2 == oSun) && fSupp) ||
    ((obj1 == oCen || obj2 == oCen) && (fSupp || asp == aSqu))))
    return fFalse;

  return fTrue;
}


/* This is a subprocedure of FCreateGrid() and FCreateGridRelation().   */
/* Given two planets, determine what aspect, if any, is present between */
/* them, and save the aspect name and orb in the specified grid cell.   */

void GetAspect(real *planet1, real *planet2,
  real *planetalt1, real *planetalt2, real *ret1, real *ret2, int i, int j)
{
  int asp;
  real rAngle, rAngle3D, rDiff, rOrb, ret1a;

  // Compute the angle between the two planets.
  grid->v[i][j] = grid->n[i][j] = 0;
  rAngle = MinDistance(planet2[i], planet1[j]);
  if (us.fAspect3D || us.fAspectLat)
    rAngle3D = SphDistance(planet2[i], planetalt2[i],
      planet1[j], planetalt1[j]);

  // Check each aspect angle to see if it applies.
  for (asp = 1; asp <= us.nAsp; asp++) {
    if (!FAcceptAspect(i, asp, j))
      continue;
    rDiff = (!us.fAspect3D ? rAngle : rAngle3D) - rAspAngle[asp];
    rOrb = GetOrb(i, j, asp);

    // If -ga switch in effect, then change the sign of the orb to correspond
    // to whether the aspect is applying or separating. To do this, check the
    // velocity vectors to see if the planets are moving toward, away, or are
    // overtaking each other.

    if (us.fAppSep) {
      ret1a = us.nRel > rcTransit ? ret1[j] : 0.0;
      rDiff *= RSgn2(ret1a-ret2[i]) * RSgn2(MinDifference(planet2[i],
        planet1[j]));
    }

#ifdef EXPRESS
    // Adjust orb with AstroExpression, if one defined.
    if (!us.fExpOff && FSzSet(us.szExpAsp)) {
      ExpSetN(iLetterV, i);
      ExpSetN(iLetterW, asp);
      ExpSetN(iLetterX, j);
      ExpSetR(iLetterY, rDiff);
      ExpSetR(iLetterZ, rOrb);
      ParseExpression(us.szExpAsp);
      rDiff = RExpGet(iLetterY);
      rOrb = RExpGet(iLetterZ);
    }
#endif

    // If aspect within orb, add it to aspect grid.
    if (RAbs(rDiff) < rOrb) {
      if (us.fAspectLat &&
        !(RAbs((!us.fAspect3D ? rAngle3D : rAngle) - rAspAngle[asp]) < rOrb))
        continue;
      grid->n[i][j] = asp;
      grid->v[i][j] = (int)(rDiff * 3600.0);
      break;
    }
  }
}


/* Very similar to GetAspect(), this determines if there is a parallel or */
/* contraparallel aspect between the given two planets, and stores the    */
/* result as above. The settings and orbs for conjunction are used for    */
/* parallel and those for opposition are used for contraparallel.         */

void GetParallel(real *planet1, real *planet2, real *planetalt1,
  real *planetalt2, real *retalt1, real *retalt2, int i, int j)
{
  int asp;
  real rDiff, azi, alt1, alt2, retalt1a;

  // Compute the declination of the two planets.
  alt1 = planetalt1[j];
  alt2 = planetalt2[i];
  if (!us.fEquator2 && !us.fParallel2) {
    azi = planet1[j];
    EclToEqu(&azi, &alt1);
    azi = planet2[i];
    EclToEqu(&azi, &alt2);
  } else if (us.fEquator2 && us.fParallel2) {
    azi = planet1[j];
    EquToEcl(&azi, &alt1);
    azi = planet2[i];
    EquToEcl(&azi, &alt2);
  }
  grid->v[i][j] = grid->n[i][j] = 0;

  // Check each aspect type to see if it applies.
  for (asp = 1; asp <= Min(us.nAsp, aOpp); asp++) {
    if (!FAcceptAspect(i, asp, j))
      continue;
    rDiff = (asp == aCon ? alt1 - alt2 : alt1 + alt2);
    if (RAbs(rDiff) < GetOrb(i, j, asp)) {
      if (us.fAppSep) {
        if (FCmSwissAny()) {
          retalt1a = us.nRel > rcTransit ? retalt1[j] : 0.0;
          rDiff *= RSgn2(retalt1a - retalt2[i]);
        } else {
          // If no declination velocity, make aspect separating.
          rDiff = RAbs(rDiff);
        }
      }
      grid->n[i][j] = asp;
      grid->v[i][j] = (int)(rDiff*3600.0);
      break;
    }
  }
}


/* Fill in the aspect grid based on the aspects taking place among the */
/* planets in the present chart. Also fill in the midpoint grid.       */

flag FCreateGrid(flag fFlip)
{
  int i, j, k;
  real l;

  if (!FEnsureGrid())
    return fFalse;
  ClearB((pbyte)grid, sizeof(GridInfo));

  for (j = 0; j <= cObj; j++) if (!FIgnore(j))
    for (i = 0; i <= cObj; i++) if (!FIgnore(i))

      // The parameter 'flip' determines what half of the grid is filled in
      // with the aspects and what half is filled in with the midpoints.

      if (fFlip ? i > j : i < j) {
        if (!us.fParallel)
          GetAspect(planet, planet, planetalt, planetalt, ret, ret, i, j);
        else
          GetParallel(planet, planet, planetalt, planetalt, retalt, retalt,
            i, j);
      } else if (fFlip ? i < j : i > j) {
        l = Mod(Midpoint(planet[i], planet[j])); k = (int)l;  // Calculate
        grid->n[i][j] = k/30+1;                               // midpoint.
        grid->v[i][j] = (int)(l*3600.0) % (30*3600);
      } else {
        grid->n[i][j] = SFromZ(planet[j]);
        grid->v[i][j] = (int)(planet[j]*60.0) % (30*60);
      }
  return fTrue;
}


/* This is similar to the previous function; however, this time fill in the */
/* grid based on the aspects (or midpoints if 'fMidpoint' set) taking place */
/* among the planets in two different charts, as in the -g -r0 combination. */

flag FCreateGridRelation(flag fMidpoint)
{
  int i, j, k;
  real l;

  if (!FEnsureGrid())
    return fFalse;
  ClearB((pbyte)grid, sizeof(GridInfo));

  for (j = 0; j <= cObj; j++) if (!FIgnore(j) || !FIgnore2(j))
    for (i = 0; i <= cObj; i++) if (!FIgnore(i) || !FIgnore2(i))
      if (!fMidpoint) {
        if (!us.fParallel)
          GetAspect(cp1.obj, cp2.obj, cp1.alt, cp2.alt, cp1.dir, cp2.dir,
            i, j);
        else
          GetParallel(cp1.obj, cp2.obj, cp1.alt, cp2.alt, cp1.dir, cp2.dir,
            i, j);
      } else {
        l = Mod(Midpoint(cp2.obj[i], cp1.obj[j])); k = (int)l;  // Calculate
        grid->n[i][j] = k/30+1;                                 // midpoint.
        grid->v[i][j] = (int)(l*3600.0) % (30*3600);
      }
  return fTrue;
}


/* Check whether one planet's disk is overlapping another, and if so by */
/* what percentage of distance overlap. Detects partial, annular, total */
/* solar eclipses, along with transits and occulations of other bodies. */

int NCheckEclipse(int obj1, int obj2, real *prPct)
{
  real radi1, radi2, len1, len2, angDiff, ang1, ang2;

  // Objects that aren't actual things in space can't eclipse or be eclipsed.
  if (!FThing(obj1) || !FThing(obj2))
    return etNone;

  // Calculate radius of the two objects in km.
  radi1 = obj1 <= oVes ? rObjDiam[obj1] / 2.0 : 0.0;
  radi2 = obj2 <= oVes ? rObjDiam[obj2] / 2.0 : 0.0;
  if (radi1 <= 0.0 && radi2 <= 0.0)
    return etNone;

  // Calculate angular distance between center points of the two objects.
  angDiff = SphDistance(planet[obj1], planetalt[obj1],
    planet[obj2], planetalt[obj2]);
  if (us.objCenter == oEar && angDiff > 0.75)
    return etNone;

  // Calculate angular size in the sky spanned by the two objects.
  len1 = RLength3(space[obj1].x, space[obj1].y, space[obj1].z) * rAUToKm;
  len2 = RLength3(space[obj2].x, space[obj2].y, space[obj2].z) * rAUToKm;
  ang1 = RAtnD(radi1 / len1);
  ang2 = RAtnD(radi2 / len2);
  if (ang1 + ang2 <= angDiff)
    return etNone;

  // Compare angular sizes to distance, to see how much overlap there is.
  if (prPct != NULL)
    *prPct = 100.0 - angDiff / RAbs(ang2 - ang1) * 100.0;
  if (ang1 >= ang2 + angDiff)
    return len1 - radi1 >= len2 + radi2 ? etAnnular : etTotal;
  else if (ang2 >= ang1 + angDiff)
    return len2 - radi2 >= len1 + radi1 ? etAnnular : etTotal;
  if (prPct != NULL)
    *prPct = 100.0 -
      (angDiff - RAbs(ang2 - ang1)) / (Min(ang1, ang2) * 2.0) * 100.0;
  return etPartial;
}


/* Check whether a lunar eclipse is taking place as seen from the Earth, */
/* and if so by what percentage of distance overlap. Detects penumbral,  */
/* total penumbral, partial, and total lunar eclipses.                   */

int NCheckEclipseLunar(real *prPct)
{
  real radiS, radiE, radiM, radiU, radiP, lenS, lenM,
    angDiff, angM, angU, angP, theta;

  // Calculate angular distance between the Moon and point opposite the Sun.
  angDiff = SphDistance(Mod(planet[oSun] + rDegHalf), -planetalt[oSun],
    planet[oMoo], planetalt[oMoo]);
  if (angDiff > 2.0)
    return etNone;

  // Calculate radius of the Sun, Earth, and Moon in km.
  radiS = rObjDiam[oSun] / 2.0;
  radiE = rObjDiam[oEar] / 2.0;
  radiM = rObjDiam[oMoo] / 2.0;
  lenS = RLength3(space[oSun].x, space[oSun].y, space[oSun].z) * rAUToKm;
  lenM = RLength3(space[oMoo].x, space[oMoo].y, space[oMoo].z) * rAUToKm;

  //radiU = (radiE - radiS) / lenS * (lenS + lenM) + radiS;
  //radiP = (radiS + radiE) / lenS * (lenS + lenM) - radiS;
  theta = RAsinD((radiS - radiE) / lenS);
  radiU = radiE - lenM * RTanD(theta);
  theta = RAsinD((radiE + radiS) / lenS);
  radiP = (lenS + lenM) * RTanD(theta) - radiS;

  // Calculate angular size in sky of Moon, and Earth's umbra and penumbra.
  angM = RAtnD(radiM / lenM);
  angU = RAtnD(radiU / lenM);
  angP = RAtnD(radiP / lenM);

  // Compare angular sizes to distance, to see how much overlap there is.
  if (angDiff - angM >= angP)
    return etNone;
  else if (angDiff + angM <= angU) {
    if (prPct != NULL)
      *prPct = 100.0 - angDiff / (angU - angM) * 100.0;
    return etTotal;
  } else if (angDiff - angM < angU) {
    if (prPct != NULL)
      *prPct = 100.0 - (angDiff - (angU - angM)) / (angM * 2.0) * 100.0;
    return etPartial;
  }
  if (prPct != NULL) {
    *prPct = (angDiff - (angP - angM)) / (angM * 2.0) * 100.0;
    *prPct = 100.0 - Max(*prPct, 0.0);
  }
  return angDiff + angM <= angP ? etPenumbra2 : etPenumbra;
}


/*
******************************************************************************
** Other Calculations.
******************************************************************************
*/

/* Fill out tables based on the number of unrestricted planets in signs by  */
/* element, signs by mode, as well as other values such as the number of    */
/* objects in yang vs. yin signs, in various house hemispheres (north/south */
/* and east/west), and the number in first six signs vs. second six signs.  */
/* This is used by the -v chart listing and the sidebar in graphics charts. */

void CreateElemTable(ET *pet)
{
  int i, s;

  ClearB((pbyte)pet, sizeof(ET));
  for (i = 0; i <= cObj; i++) if (!FIgnore(i)) {
    pet->coSum++;
    s = SFromZ(planet[i]);
    pet->coSign[s-1]++;
    pet->coElemMode[(s-1)&3][(s-1)%3]++;
    pet->coElem[(s-1)&3]++; pet->coMode[(s-1)%3]++;
    pet->coYang += FOdd(s);
    pet->coLearn += (s < sLib);
    if (!FCusp(i)) {
      pet->coHemi++;
      s = inhouse[i];
      pet->coHouse[s-1]++;
      pet->coModeH[(s-1)%3]++;
      pet->coMC += (s >= sLib);
      pet->coAsc += (s < sCan || s >= sCap);
    }
  }
  pet->coYin   = pet->coSum  - pet->coYang;
  pet->coShare = pet->coSum  - pet->coLearn;
  pet->coDes   = pet->coHemi - pet->coAsc;
  pet->coIC    = pet->coHemi - pet->coMC;
}


/*
******************************************************************************
** Swiss Ephemeris Calculations.
******************************************************************************
*/

#ifdef SWISS
#include "swephexp.h"
#include "swephlib.h"

// Set up path for Swiss Ephemeris to search in for ephemeris files.

void SwissEnsurePath()
{
  char szPath[AS_MAXCH];
#ifdef ENVIRON
  char szT[cchSzDef], *env, *pch;
#endif
  int i;

  if (!is.fSwissPathSet) {
    // First look for the file in the current directory.
    sprintf(szPath, ".");
    // Next look in the directories indicated by the -Yi switch.
    for (i = 0; i < 10; i++)
      if (us.rgszPath[i] && *us.rgszPath[i])
        sprintf(szPath + CchSz(szPath), "%s%s", PATH_SEPARATOR,
          us.rgszPath[i]);
#ifdef ENVIRON
    // Next look for the file in the directory indicated by the version
    // specific system environment variable.
    sprintf(szT, "%s%s", ENVIRONVER, szVersionCore);
    for (pch = szT; *pch && *pch != '.'; pch++)
      ;
    while (*pch && (*pch = pch[1]) != chNull)
      pch++;
    env = getenv(szT);
    if (env && *env)
      sprintf(szPath + CchSz(szPath), "%s%s", PATH_SEPARATOR, env);
    // Next look in the directory in the general environment variable.
    env = getenv(ENVIRONALL);
    if (env && *env)
      sprintf(szPath + CchSz(szPath), "%s%s", PATH_SEPARATOR, env);
    // Next look in the directory in the version prefix environment variable.
    env = getenv(ENVIRONVER);
    if (env && *env)
      sprintf(szPath + CchSz(szPath), "%s%s", PATH_SEPARATOR, env);
#endif
    // Finally look in a directory specified at compile time.
    sprintf(szPath + CchSz(szPath), "%s%s", PATH_SEPARATOR, EPHE_DIR);
    swe_set_ephe_path(szPath);
    is.fSwissPathSet = fTrue;
  }
}


int rgObjSwiss[cUran] = {SE_VULCAN - SE_FICT_OFFSET_1,
  SE_CUPIDO   - SE_FICT_OFFSET_1, SE_HADES    - SE_FICT_OFFSET_1,
  SE_ZEUS     - SE_FICT_OFFSET_1, SE_KRONOS   - SE_FICT_OFFSET_1,
  SE_APOLLON  - SE_FICT_OFFSET_1, SE_ADMETOS  - SE_FICT_OFFSET_1,
  SE_VULKANUS - SE_FICT_OFFSET_1, SE_POSEIDON - SE_FICT_OFFSET_1};
int rgTypSwiss[cUran];
int rgPntSwiss[cUran];
int rgFlgSwiss[cUran];

/* Given an object index and a Julian Day time, get ecliptic longitude and */
/* latitude of the object and its velocity and distance from the Earth or  */
/* Sun. This basically just calls the Swiss Ephemeris calculation function */
/* to actually do it. Because this is the one of the only routines called  */
/* from Astrolog, this routine has knowledge of and uses both Astrolog and */
/* Swiss Ephemeris definitions, and does things such as translation to     */
/* indices and formats of Swiss Ephemeris.                                 */

flag FSwissPlanet(int ind, real jd, flag fHelio,
  real *obj, real *objalt, real *dir, real *space, real *diralt)
{
  int iobj, iflag, nRet, nTyp, nPnt = 0, nFlg = 0, ix;
  double jde, xx[6], xnasc[6], xndsc[6], xperi[6], xaphe[6], *px;
  char serr[AS_MAXCH];
  static flag fSwissMosh = fFalse;

  // Reset Swiss Ephemeris if changing computation method.
  if (us.fSwissMosh != fSwissMosh)
    swe_close();
  fSwissMosh = us.fSwissMosh;
  SwissEnsurePath();

  // Convert Astrolog object index to Swiss Ephemeris index.
  if (ind == oSun && (fHelio || us.fBarycenter))
    iobj = SE_EARTH;
  else if (ind <= oPlu)
    iobj = ind-1;
  else if (ind == oChi)
    iobj = SE_CHIRON;
  else if (FBetween(ind, oCer, oVes))
    iobj = ind - oCer + SE_CERES;
  else if (ind == oNod)
    iobj = us.fTrueNode ? SE_TRUE_NODE : SE_MEAN_NODE;
  else if (ind == oSou)
    return fFalse;
  else if (ind == oLil)
    iobj = us.fTrueNode ? SE_OSCU_APOG : SE_MEAN_APOG;
  else if (FBetween(ind, uranLo, uranHi)) {
    iobj = rgObjSwiss[ind - uranLo];
    nTyp = rgTypSwiss[ind - uranLo];
    nPnt = rgPntSwiss[ind - uranLo];
    nFlg = rgFlgSwiss[ind - uranLo];
    if (nTyp != 2)
      iobj += (nTyp <= 0 ? SE_FICT_OFFSET_1 : (nTyp == 1 ? SE_AST_OFFSET :
        SE_PLMOON_OFFSET));
    else {
      if (iobj <= oEar)
        iobj = SE_EARTH;
      else if (iobj <= oPlu)
        iobj--;
      else if (iobj == oChi)
        iobj = SE_CHIRON;
      else if (FBetween(iobj, oCer, oVes))
        iobj = iobj - oCer + SE_CERES;
      else if (iobj == oVul)
        iobj = SE_VULCAN;
      else if (FBetween(iobj, uranLo, uranHi))
        iobj = iobj - uranLo + SE_FICT_OFFSET_1;
      else
        return fFalse;
    }
    if (nFlg > 0) {
      if (nFlg & 1)
        inv(fHelio);
      if (nFlg & 2)
        inv(us.fSidereal);
      if (nFlg & 4)
        inv(us.fBarycenter);
      if (nFlg & 8)
        inv(us.fTrueNode);
      if (nFlg & 16)
        inv(us.fTruePos);
      if (nFlg & 32)
        inv(us.fTopoPos);
    }
  } else
    iobj = ind;

  // Convert Astrolog calculation settings to Swiss Ephemeris flags.
  iflag = SEFLG_SPEED;
  iflag |= (us.fSwissMosh ? SEFLG_MOSEPH : SEFLG_SWIEPH);
  if (us.fSidereal) {
    swe_set_sid_mode(!us.fSidereal2 ? SE_SIDM_FAGAN_BRADLEY :
      SE_SIDBIT_SSY_PLANE, 0.0, 0.0);
    iflag |= SEFLG_SIDEREAL;
  }
  if (fHelio && ind != oNod && ind != oLil)
    iflag |= (us.fBarycenter ? SEFLG_BARYCTR : SEFLG_HELCTR);
  if (!fHelio && ind == oSun && us.fBarycenter)
    iflag |= SEFLG_BARYCTR;
  if (us.fNoNutation)
    iflag |= SEFLG_NONUT;
  if (us.fTruePos)
    iflag |= SEFLG_TRUEPOS;
  if (us.fTopoPos && !fHelio) {
    swe_set_topo(-OO, AA, us.elvDef);
    iflag |= SEFLG_TOPOCTR;
  }

  // Compute position of planet or node/helion.
  jde = jd + swe_deltat(jd);
  if (nPnt == 0)
    nRet = swe_calc(jde, iobj, iflag, xx, serr);
  else {
    nRet = swe_nod_aps(jde, iobj, iflag, us.fTrueNode ? SE_NODBIT_OSCU :
      SE_NODBIT_MEAN, xnasc, xndsc, xperi, xaphe, serr);
    switch (nPnt) {
    case 1:  px = xnasc; break;
    case 2:  px = xndsc; break;
    case 3:  px = xperi; break;
    default: px = xaphe; break;
    }
    for (ix = 0; ix < 6; ix++)
      xx[ix] = px[ix];
  }

  // Clean up and return position.
  if (nFlg > 0) {
    if (nFlg & 2)
      inv(us.fSidereal);
    if (nFlg & 4)
      inv(us.fBarycenter);
    if (nFlg & 8)
      inv(us.fTrueNode);
    if (nFlg & 16)
      inv(us.fTruePos);
    if (nFlg & 32)
      inv(us.fTopoPos);
  }
  if (nRet < 0) {
    if (!is.fNoEphFile) {
      is.fNoEphFile = fTrue;
      PrintWarning(serr);
    }
    return fFalse;
  }
  *obj    = xx[0] - is.rSid + us.rZodiacOffset;
  *objalt = xx[1];
  *space  = xx[2];
  *dir    = xx[3];
  *diralt = xx[4];
  if (us.fParallel) {
    swe_cotrans_sp(xx, xnasc, -RAbs(is.OB));
    *diralt = xnasc[4];
  }
  return fTrue;
}


/* Compute house cusps and related variables like the Ascendant. Given a  */
/* Julian Day time, location, and house system, call Swiss Ephemeris to   */
/* compute them. This is similar to FSwissPlanet() in that it knows about */
/* and translates between Astrolog and Swiss Ephemeris defintions.        */

void SwissHouse(real jd, real lon, real lat, int housesystem,
  real *asc, real *mc, real *ra, real *vtx, real *ep, real *ob, real *off)
{
  double cusp[cSign+1], ascmc[11], cuspr[cSign+1], ascmcr[11];
  int i;
  char serr[AS_MAXCH], ch;

  // Translate Astrolog house index to Swiss Ephemeris house character.
  // Don't do hsWhole houses ('W') yet, until after is.rSid computed.
  switch (housesystem) {
  case hsPlacidus:      ch = 'P'; break;
  case hsKoch:          ch = 'K'; break;
  case hsEqual:         ch = 'E'; break;
  case hsCampanus:      ch = 'C'; break;
  case hsMeridian:      ch = 'X'; break;
  case hsRegiomontanus: ch = 'R'; break;
  case hsPorphyry:      ch = 'O'; break;
  case hsMorinus:       ch = 'M'; break;
  case hsTopocentric:   ch = 'T'; break;
  case hsAlcabitius:    ch = 'B'; break;
  case hsKrusinski:     ch = 'U'; break;
  case hsSinewaveRatio: ch = 'Q'; break;
  case hsSinewaveDelta: ch = 'L'; break;
  case hsVedic:         ch = 'V'; break;
  case hsSripati:       ch = 'S'; break;
  case hsHorizon:       ch = 'H'; break;
  case hsAPC:           ch = 'Y'; break;
  case hsCarter:        ch = 'F'; break;
  case hsSunshine:      ch = 'I'; break;
  default:              ch = 'A'; break;
  }
  jd = JulianDayFromTime(jd);
  lon = -lon;

  // The following is largely copied from swe_houses().
  double armc, eps, nutlo[2];
  double tjde = jd + swe_deltat(jd);
  eps = swi_epsiln(tjde, 0) * RADTODEG;
  swi_nutation(tjde, 0, nutlo);
  for (i = 0; i < 2; i++)
    nutlo[i] *= RADTODEG;
  if (!us.fGeodetic)
    armc = swe_degnorm(swe_sidtime0(jd, eps + nutlo[1], nutlo[0]) * 15 + lon);
  else
    armc = lon;
  if (ch == 'I') {  // Sun declination for Sunshine houses.
    int flags = SEFLG_SPEED | SEFLG_EQUATORIAL;
    double xp[6];
    int result = swe_calc_ut(jd, SE_SUN, flags, xp, NULL);
    ascmc[9] = xp[1];
  }
  if (swe_houses_armc_ex2(armc, lat, eps + nutlo[1], ch, cusp, ascmc,
    cuspr, ascmcr, serr))
    housesystem = hsPorphyry;

  // Fill out return parameters for cusp array, Ascendant, etc.
  *off = -swe_get_ayanamsa(tjde);
  is.rSid = us.rZodiacOffset + (us.fSidereal ? *off : 0.0);

  *asc = Mod(ascmc[SE_ASC] + is.rSid);
  *mc  = Mod(ascmc[SE_MC]  + is.rSid);
  *ra  = RFromD(Mod(ascmc[SE_ARMC] + is.rSid));
  *vtx = Mod(ascmc[SE_VERTEX] + is.rSid);
  *ep  = Mod(ascmc[SE_EQUASC] + is.rSid);
  *ob  = eps;
  for (i = 1; i <= cSign; i++) {
    chouse[i] = Mod(cusp[i] + is.rSid);
    ret[cuspLo-1+i] = cuspr[i];
  }
  ret[oFor] = ascmcr[SE_ASC];
  ret[oVtx] = ascmcr[SE_VERTEX];
  ret[oEP]  = ascmcr[SE_EQUASC];
  if (!us.fHouseAngle) {
    ret[oAsc] = ret[oDes] = ascmcr[SE_ASC];
    ret[oMC]  = ret[oNad] = ascmcr[SE_MC];
  }

  // Want generic MC. Undo if house system flipped it 180 degrees.
  if ((housesystem == hsCampanus || housesystem == hsRegiomontanus ||
    housesystem == hsTopocentric || housesystem == hsAPC ||
    housesystem == hsSunshine) && MinDifference(*mc, *asc) < 0.0)
    *mc = Mod(*mc + rDegHalf);

  // Have Astrolog compute the houses if Swiss Ephemeris didn't do so.
  if (ch == 'A')
    ComputeHouses(housesystem);
  else
    is.nHouseSystem = housesystem;
}


/* Compute fixed star locations. Given a time, call Swiss Ephemeris to    */
/* compute them. This is similar to FSwissPlanet() in that it knows about */
/* and translates between Astrolog and Swiss Ephemeris defintions.        */

void SwissComputeStars(real jd, flag fInitBright)
{
  char sz[cchSzDef], serr[AS_MAXCH];
  int i, iflag;
  double xx[6], mag;

  SwissEnsurePath();
  if (!fInitBright) {
    jd = JulianDayFromTime(jd);
    iflag = SEFLG_SPEED;
    iflag |= (us.fSwissMosh ? SEFLG_MOSEPH : SEFLG_SWIEPH);
    if (us.fSidereal) {
      swe_set_sid_mode(!us.fSidereal2 ? SE_SIDM_FAGAN_BRADLEY :
        SE_SIDBIT_SSY_PLANE, 0.0, 0.0);
      iflag |= SEFLG_SIDEREAL;
    }
    if (us.objCenter != oEar)
      iflag |= (us.fBarycenter ? SEFLG_BARYCTR : SEFLG_HELCTR);
    if (us.fTruePos)
      iflag |= SEFLG_TRUEPOS;
    if (us.fNoNutation)
      iflag |= SEFLG_NONUT;
  } else {
    jd = rJD2000;
    iflag = SEFLG_SPEED | SEFLG_SWIEPH | SEFLG_HELCTR;
  }
  for (i = 1; i <= cStar; i++) {

    // In most cases Astrolog's star name is the same as Swiss Ephemeris,
    // however for a few stars we need to translate to a different string.
    if (szStarCustom[i] == NULL || szStarCustom[i][0] == chNull) {
      if      (i == 3)  sprintf(sz, ",ze-1Ret");         // Zeta Retic.
      else if (i == 4)  sprintf(sz, "Pleione");          // Pleiades
      else if (i == 10) sprintf(sz, "Alnilam");          // Orion
      else if (i == 30) sprintf(sz, ",beCru");           // Becrux
      else if (i == 36) sprintf(sz, "Rigil Kentaurus");  // Rigil Kent.
      else if (i == 40) sprintf(sz, "Kaus Australis");   // Kaus Austr.
      else if (i == 47) sprintf(sz, ",M31");             // Andromeda
      else
        sprintf(sz, "%s", szObjName[oNorm + i]);
    } else
      sprintf(sz, "%s", szStarCustom[i]);

    // Compute the star location or get the star's brightness.
    swe_fixstar2(sz, jd, iflag, xx, serr);
    if (!fInitBright) {
      planet[oNorm+i] = Mod(xx[0] + us.rZodiacOffset);
      planetalt[oNorm+i] = xx[1];
      ret[oNorm+i] = xx[3];
      if (!us.fSidereal && us.fVelocity)
        ret[oNorm+i] /= rDegMax/25765.0/rDayInYear;
      SphToRec(xx[2], planet[oNorm+i], planetalt[oNorm+i],
        &space[oNorm+i].x, &space[oNorm+i].y, &space[oNorm+i].z);
      if (us.fStarMagDist)
        rStarBright[i] = RStarBright(rStarBrightDef[i], rStarDistDef[i],
          us.fStarMagAbs ? 10.0 * rPCToAU : xx[2]);
      else
        rStarBright[i] = rStarBrightDef[i];
      rStarDist[i] = xx[2];
      kObjA[oNorm+i] = KStarA(rStarBright[i]);
    } else {
      rStarDistDef[i] = rStarDist[i] = xx[2];
      swe_fixstar2_mag(sz, &mag, serr);
      rStarBrightDef[i] = rStarBright[i] = mag;
    }
  }
}


/* Compute one fixed star location. Given a star index and time, call Swiss */
/* Ephemeris to compute it. This is similar to SwissComputeStars().         */

flag SwissComputeStar(real jd, ES *pes)
{
  char serr[AS_MAXCH], *pch;
  int iflag, isz = 0, i;
  double xx[6], dist1, dist2;
  static real lonPrev = 0.0, latPrev = 0.0;
  static int istar = 1;

  // Calling with empty parameters means initialize to first star.
  if (pes == NULL) {
    istar = 1;
#ifdef GRAPH
    if (gi.rges != NULL)
      ClearB((pbyte)gi.rges, sizeof(ES) * gi.cStarsLin);
#endif
    return fTrue;
  }

  // Determine Swiss Ephemeris flags.
  jd = JulianDayFromTime(jd);
  iflag = SEFLG_SPEED;
  iflag |= (us.fSwissMosh ? SEFLG_MOSEPH : SEFLG_SWIEPH);
  if (us.fSidereal) {
    swe_set_sid_mode(!us.fSidereal2 ? SE_SIDM_FAGAN_BRADLEY :
      SE_SIDBIT_SSY_PLANE, 0.0, 0.0);
    iflag |= SEFLG_SIDEREAL;
  }
  if (us.objCenter != oEar)
    iflag |= (us.fBarycenter ? SEFLG_BARYCTR : SEFLG_HELCTR);
  if (us.fTruePos)
    iflag |= SEFLG_TRUEPOS;
  if (us.fNoNutation)
    iflag |= SEFLG_NONUT;
LNext:
  sprintf(pes->sz, "%d", istar);

  // Compute the star coordinates and get the star's brightness.
  if (us.fStarMagDist) {
    if (swe_fixstar2(pes->sz, rJD2000,
      SEFLG_SPEED | SEFLG_SWIEPH | SEFLG_HELCTR, xx, serr) < 0)
      return fFalse;
    dist1 = xx[2];
  }
  if (swe_fixstar2(pes->sz, jd, iflag, xx, serr) < 0)
    return fFalse;
  pes->lon = Mod(xx[0] + us.rZodiacOffset);
  pes->lat = xx[1];
  if (us.fStarMagDist)
    dist2 = xx[2];
  pes->dir = xx[3];

  // Store star data.
  SphToRec(xx[2], pes->lon, pes->lat,
    &pes->space.x, &pes->space.y, &pes->space.z);
  if (us.objCenter > oSun) {
    pes->space.x += space[oSun].x;
    pes->space.y += space[oSun].y;
    pes->space.z += space[oSun].z;
    RecToSph3(pes->space.x, pes->space.y, pes->space.z, &pes->lon, &pes->lat);
    if (us.fStarMagDist)
      dist2 = us.fStarMagAbs ? 10.0 * rPCToAU :
        RLength3(pes->space.x, pes->space.y, pes->space.z);
  }
  if (swe_fixstar2_mag(pes->sz, &pes->mag, serr) < 0)
    return fFalse;
  if (pes->mag == 0.0)
    pes->mag = rStarNot;
  else if (us.fStarMagDist && pes->mag != rStarNot)
    pes->mag = RStarBright(pes->mag, dist1, dist2);

  // Adjust star coordinates if needed.
  if (us.rHarmonic != 1.0)
    pes->lon = Mod(pes->lon * us.rHarmonic);
  if (us.fDecan)
    pes->lon = Decan(pes->lon);
  if (us.nDwad > 0)
    for (i = 0; i < us.nDwad; i++)
      pes->lon = Dwad(pes->lon);
  if (us.fNavamsa)
    pes->lon = Navamsa(pes->lon);

  // Skip over effectively duplicate stars, or non-stars.
  istar++;
  if ((pes->lon == lonPrev && pes->lat == latPrev) ||
    (pes->mag == rStarNot && !us.fGraphAll))
    goto LNext;
  lonPrev = pes->lon; latPrev = pes->lat;

  // Parse star name.
  pes->pchNam = pes->sz;
  for (pch = pes->sz; *pch && *pch != ','; pch++)
    ;
  if (*pch == ',') {
    *pch++ = chNull;
    pes->pchDes = pch;
    pes->pchBest = *pes->pchNam ? pes->pchNam : pes->pchDes;
  } else {
    pes->pchDes = "";
    pes->pchBest = pes->pchNam;
  }

  // Check for if should do anything special with this star?
  if (*us.szStarsList &&
    ((*pes->pchDes && SzInList(pes->pchDes, us.szStarsList, NULL) != NULL) ||
    (*pes->pchNam && SzInList(pes->pchNam, us.szStarsList, NULL) != NULL)) !=
    us.fStarsList)
    goto LNext;
  pes->ki = kDefault;
  if (*us.szStarsColor) {
    pch = (char *)SzInList(pes->pchBest, us.szStarsColor, NULL);
    if (FSzSet(pch))
      pes->ki = NParseSz(pch, pmColor);
  }
#ifdef GRAPH
  if (*gs.szStarsLin) {
    pch = (char *)SzInList(pes->pchBest, gs.szStarsLin, &isz);
    if (isz >= 0)
      gi.rges[isz] = *pes;
  }
#endif
  return fTrue;
}


/* Compute one asteroid location. Given an asteroid number and time, call */
/* Swiss Ephemeris to compute it. This is similar to SwissComputeStars(). */

flag SwissComputeAsteroid(real jd, ES *pes, flag fBack)
{
  int iflag, isz = 0, i;
  real r1, r2, r3, r4, r5, rDiff;
  char sz[cchSzDef], *pch;
  static int iast = 1;

  // Calling with empty parameters means initialize to first asteroid.
  if (pes == NULL) {
    iast = fBack ? gs.nAstHi : gs.nAstLo;
    return fTrue;
  } else if (iast < Max(gs.nAstLo, 1) || iast > gs.nAstHi)
    return fFalse;

  // Determine Swiss Ephemeris flags.
  jd = JulianDayFromTime(jd);
  iflag = SEFLG_SPEED;
  iflag |= (us.fSwissMosh ? SEFLG_MOSEPH : SEFLG_SWIEPH);
  if (us.fSidereal) {
    swe_set_sid_mode(!us.fSidereal2 ? SE_SIDM_FAGAN_BRADLEY :
      SE_SIDBIT_SSY_PLANE, 0.0, 0.0);
    iflag |= SEFLG_SIDEREAL;
  }
  if (us.objCenter != oEar)
    iflag |= (us.fBarycenter ? SEFLG_BARYCTR : SEFLG_HELCTR);
  if (us.fTruePos)
    iflag |= SEFLG_TRUEPOS;
  if (us.fNoNutation)
    iflag |= SEFLG_NONUT;

  // Compute the asteroid coordinates.
  if (!FSwissPlanet(iast + SE_AST_OFFSET, jd, us.objCenter != oEar,
    &r1, &r2, &r3, &r4, &r5))
    return fFalse;
  pes->lon = Mod(r1 + is.rSid);
  pes->lat = r2;
  pes->dir = r3;
  SphToRec(r4, pes->lon, pes->lat,
    &pes->space.x, &pes->space.y, &pes->space.z);
  if (us.objCenter > oSun) {
    pes->space.x += space[oSun].x;
    pes->space.y += space[oSun].y;
    pes->space.z += space[oSun].z;
    RecToSph3(pes->space.x, pes->space.y, pes->space.z, &pes->lon, &pes->lat);
  }

  // Adjust asteroid coordinates if needed.
  if (us.rHarmonic != 1.0)
    pes->lon = Mod(pes->lon * us.rHarmonic);
  if (us.fDecan)
    pes->lon = Decan(pes->lon);
  if (us.nDwad > 0)
    for (i = 0; i < us.nDwad; i++)
      pes->lon = Dwad(pes->lon);
  if (us.fNavamsa)
    pes->lon = Navamsa(pes->lon);

  // Determine asteroid display name.
  pch = pes->sz;
  *pch = chNull;
  if (FOdd(gs.nAstLabel))
    sprintf(pch, "%d", iast);
  if (gs.nAstLabel >= 3) {
    while (*pch)
      pch++;
    *pch++ = ' ';
    *pch = chNull;
  }
  if (gs.nAstLabel & 2) {
    swe_get_planet_name(iast + SE_AST_OFFSET, pch);
    // This check only needed for old style ephemeris files.
    if (*pch == '?' && pch[1] == ' ')
      while (*pch = pch[2])
        pch++;
  }
  pes->pchBest = pes->sz;

  // Determine asteroid coloring.
  rDiff = gs.nAstHi <= gs.nAstLo ? 1.0 : (real)(gs.nAstHi - gs.nAstLo);
  pes->mag = ((real)(iast - gs.nAstLo) / rDiff * rStarSpan) + rStarLite;
  pes->ki = kDefault;
  if (*us.szAstColor) {
    pch = (char *)SzInList(pes->pchBest, us.szAstColor, NULL);
    if (!FSzSet(pch)) {
      sprintf(sz, "%d", iast);
      pch = (char *)SzInList(sz, us.szAstColor, NULL);
    }
    if (FSzSet(pch))
      pes->ki = NParseSz(pch, pmColor);
  }

  iast += (fBack ? -1 : 1);
  return fTrue;
}


/* Wrappers around Swiss ephemeris Julian Day conversion routines. */

double SwissJulDay(int month, int day, int year, real hour, int gregflag)
{
  return swe_julday(year, month, day, hour, gregflag);
}

void SwissRevJul(real jd, int gregflag,
  int *jmon, int *jday, int *jyear, double *jut)
{
  swe_revjul(jd, gregflag, jyear, jmon, jday, jut);
}
#endif /* SWISS */

/* calc.cpp */
