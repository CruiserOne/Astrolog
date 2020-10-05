/*
** Astrolog (Version 7.10) File: xcharts2.cpp
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


#ifdef GRAPH
/*
******************************************************************************
** Chart Graphics Utility Procedures.
******************************************************************************
*/

/* Return whether the specified object should be displayed in the current  */
/* graphics chart type. For example, don't include the Moon in the solar   */
/* system charts when ephemeris files are off, don't include house cusps   */
/* in astro-graph charts, and so on, in addition to checking restrictions. */

flag FProper(int i)
{
  flag f;

  f = !ignore[i];
  if (gi.nMode == gOrbit)
    f &= FThing(i) && (us.fEphemFiles || !FGeo(i));
  else if (gi.nMode == gSector ||
    fMap || gi.nMode == gGlobe || gi.nMode == gPolar)
    f &= FThing(i);
  else if (gi.nMode == gEphemeris)
    f &= !(gs.fAlt && (i == oMoo || i == oFor));
  else if (gi.nMode == gTraTraGra || gi.nMode == gTraNatGra)
    f &= FProperGraph(i);
  else if (gi.nMode == gTelescope)
    f &= (i != us.objCenter || us.objCenter == oEar);
  return f;
}


/* Adjust an array of zodiac positions so that no two positions are within   */
/* a certain orb of each other. This is used by the wheel drawing chart      */
/* routines in order to make sure that we don't draw any planet glyphs on    */
/* top of each other. We'll later draw the glyphs at the adjusted positions. */

void FillSymbolRing(real *symbol, real factor)
{
  real orb = DEFORB*256.0/(real)gs.yWin*(real)gi.nScale*factor, k1, k2, temp;
  int i, j, k = 1, l;

  // Keep adjusting as long as can still make changes, or until 'n' rounds are
  // done. (With many objects, there just may not be enough room for all.)

  for (l = 0; k && l < us.nDivision*2; l++) {
    k = 0;
    for (i = 0; i <= cObj; i++) if (FProper(i)) {

      // For each object, determine who is closest on either side.

      k1 = rLarge; k2 = -rLarge;
      for (j = 0; j <= cObj; j++)
        if (FProper(j) && i != j) {
          temp = symbol[j]-symbol[i];
          if (RAbs(temp) > rDegHalf)
            temp -= rDegMax*RSgn(temp);
          if (temp < k1 && temp > 0.0)
            k1 = temp;
          else if (temp > k2 && temp <= 0.0)
            k2 = temp;
        }

      // If an object's too close on one side, then move in other direction.

      if (k2 > -orb && k1 > orb) {
        k = 1; symbol[i] = Mod(symbol[i]+orb*0.51+k2*0.49);
      } else if (k1 < orb && k2 < -orb) {
        k = 1; symbol[i] = Mod(symbol[i]-orb*0.51+k1*0.49);

      // If object bracketed by close objects on both sides, then move it to
      // the midpoint, so it's as far away as possible from either one.

      } else if (k2 > -orb && k1 < orb) {
        k = 1; symbol[i] = Mod(symbol[i]+(k1+k2)*0.5);
      }
    }
  }
}


/* Adjust an array of longitude positions so that no two are within a    */
/* certain orb of each other. This is used by the astro-graph routine to */
/* make sure we don't draw any planet glyphs marking the lines on top of */
/* each other. This is almost identical to the FillSymbolRing() routine  */
/* used by the wheel charts; however, there the glyphs are placed in a   */
/* continuous ring, while here we have the left and right screen edges.  */
/* Also, here we are placing two sets of planets at the same time.       */

void FillSymbolLine(real *symbol)
{
  real orb = DEFORB*1.35*(real)gi.nScale, max = rDegMax, k1, k2, temp;
  int i, j, k = 1, l;

  if (gi.nMode != gEphemeris)
    max *= (real)gi.nScale;
  else
    orb *= rDegMax/(real)gs.xWin;

  // Keep adjusting as long as can still make changes.

  for (l = 0; k && l < us.nDivision*2; l++) {
    k = 0;
    for (i = 0; i <= cObj*2+1; i++)
      if (FProper(i >> 1) && symbol[i] >= 0.0) {

        // For each object, determine who is closest to the left and right.

        k1 = max-symbol[i]; k2 = -symbol[i];
        for (j = 0; j <= cObj*2+1; j++) {
          if (FProper(j >> 1) && i != j) {
            temp = symbol[j]-symbol[i];
            if (temp < k1 && temp > 0.0)
              k1 = temp;
            else if (temp > k2 && temp <= 0.0)
              k2 = temp;
          }
        }

        // If an object's too close on one side, then move in other direction.

        if (k2 > -orb && k1 > orb) {
          k = 1; symbol[i] = symbol[i]+orb*0.51+k2*0.49;
        } else if (k1 < orb && k2 < -orb) {
          k = 1; symbol[i] = symbol[i]-orb*0.51+k1*0.49;
        } else if (k2 > -orb && k1 < orb) {
          k = 1; symbol[i] = symbol[i]+(k1+k2)*0.5;
        }
      }
  }
}


/* Given a zodiac position, return the degree on the current wheel chart */
/* circle where that position falls, rotating based on the Ascendant and */
/* adding in the opposite direction for Vedic mode wheels.               */

real PlaceInX(real deg)
{
  if (us.fVedic)
    deg = -chouse[1]*(gi.nMode != gWheel)*2.0-deg-60.0;
  return Mod(rDegHalf-deg+gi.rAsc);
}


/* Given a zodiac degree, adjust it if need be to account for the expanding */
/* and compacting of parts the zodiac that happen when we display a graphic */
/* wheel chart such that all the houses appear the same size.               */

real HousePlaceInX(real deg, real degalt)
{
  int in;
  real rIn;

  if (us.fHouse3D && degalt != 0.0) {
    rIn = RHousePlaceIn3D(deg, degalt) / 30.0;
    in = (int)rIn + 1;
    deg = Mod(chouse[in] +
      (rIn - RFloor(rIn)) * MinDistance(chouse[in], chouse[Mod12(in+1)]));
  }
  if (gi.nMode == gWheel)    // Only adjust for the -w -X combination.
    return deg;
  in = NHousePlaceIn2D(deg);
  return Mod(ZFromS(in)+MinDistance(chouse[in], deg)/
    MinDistance(chouse[in], chouse[Mod12(in+1)])*30.0);
}


/* Draw lines connecting planets between two charts that have aspects. Used */
/* when creating bi-wheels and beyond.                                      */

void DrawAspectRelation(int n1, int n2, real obj1[objMax], real obj2[objMax],
  int cx, int cy, real rx, real ry, real rz)
{
  CP cpA, cpB;
  int i, j;

  // Put the two sets of chart data to compare in cp1 and cp2.
  if (n1 != 1) {
    cpA = cp1;
    switch (n1) {
    case 2: cp1 = cp2; break;
    case 3: cp1 = cp3; break;
    }
  }
  if (n2 != 2) {
    cpB = cp2;
    switch (n2) {
    case 3: cp2 = cp3; break;
    case 4: cp2 = cp4; break;
    }
  }

  // Compute and draw the aspect lines.
  if (!FCreateGridRelation(fFalse))
    goto LExit;
  for (j = cObj; j >= 0; j--)
    for (i = cObj; i >= 0; i--)
      if (grid->n[i][j] && FProper2(i) && FProper(j) &&
        obj1[j] >= 0.0 && obj2[i] >= 0.0)
        DrawAspectLine(i, j, cx, cy, obj1[j], obj2[i], rx, ry, rz);

LExit:
  if (n1 != 1)
    cp1 = cpA;
  if (n2 != 2)
    cp2 = cpB;
}


/*
******************************************************************************
** Multiple Chart Graphics Routines.
******************************************************************************
*/

/* Draw another wheel chart; however, this time we have two rings of planets */
/* because we are doing a bi-wheel relationship chart between two sets of    */
/* data. This chart is obtained when the -r0 is combined with the -X switch. */

void XChartWheelRelation()
{
  real xsign[cSign+1], xhouse1[cSign+1], xplanet1[objMax], xplanet2[objMax],
    symbol[objMax];
  byte ignoreT[objMax];
  int cx, cy, i;
  real unitx, unity, temp;

  // Set up variables and temporarily automatically decrease the horizontal
  // chart size to leave room for the sidebar if that mode is in effect.

  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;
  cx = gs.xWin/2 - 1; cy = gs.yWin/2 - 1;
  unitx = (real)cx; unity = (real)cy;
  gi.rAsc = gs.objLeft ? cp1.obj[NAbs(gs.objLeft)-1] +
    rDegQuad*(gs.objLeft < 0) : cp1.cusp[1];
  if (us.fVedic)
    gi.rAsc = gs.objLeft ? (gs.objLeft < 0 ? 120.0 : -60.0)-gi.rAsc : 0.0;

  // Fill out arrays with the degree of each object, cusp, and sign glyph.

  if (gi.nMode == gWheel) {
    for (i = 1; i <= cSign; i++)
      xhouse1[i] = PZ(cp1.cusp[i]);
  } else {
    gi.rAsc -= cp1.cusp[1];
    for (i = 1; i <= cSign; i++)
      xhouse1[i] = PZ(ZFromS(i));
  }
  for (i = 1; i <= cSign; i++)
    xsign[i] = PZ(HousePlaceInX(ZFromS(i), 0.0));
  for (i = 0; i <= cObj; i++)
    xplanet1[i] = PZ(HousePlaceInX(cp1.obj[i], cp1.alt[i]));
  for (i = 0; i <= cObj; i++)
    xplanet2[i] = PZ(HousePlaceInX(cp2.obj[i], cp2.alt[i]));

  // Go draw the outer sign and house rings. We are drawing only the houses
  // of one of the two charts in the relationship, however.

  DrawWheel(xsign, xhouse1, cx, cy, unitx, unity,
    0.70, 0.74, 0.78, 0.82, 0.885);

  /* Draw the outer ring of planets (based on the planets in the chart     */
  /* which the houses do not reflect - the houses belong to the inner ring */
  /* below). Draw each glyph, a line from it to its actual position point  */
  /* in the outer ring, and then draw another line from this point to a    */
  /* another dot at the same position in the inner ring as well.           */

  FProcessCommandLine(szWheelX[2]);
  for (i = 0; i <= cObj; i++)
    symbol[i] = xplanet2[i];
  if (us.nRel == rcTransit)
    for (i = 0; i <= cObj; i++) {
      ignoreT[i] = ignore[i];
      ignore[i] = ignore2[i];
    }
  FillSymbolRing(symbol, 1.0);
  if (us.nRel == rcTransit)
    for (i = 0; i <= cObj; i++)
      ignore[i] = ignoreT[i];

  for (i = cObj; i >= 0; i--) if (FProper2(i)) {
    if (gs.fLabel) {
      temp = symbol[i];
      DrawColor(cp2.dir[i] < 0.0 ? gi.kiGray : gi.kiOn);
      DrawDash(cx+POINT1(unitx, 0.58, PX(xplanet2[i])),
        cy+POINT1(unity, 0.58, PY(xplanet2[i])),
        cx+POINT2(unitx, 0.61, PX(temp)),
        cy+POINT2(unity, 0.61, PY(temp)),
        (cp2.dir[i] < 0.0 ? 1 : 0) - gs.fColor);
      DrawObject(i, cx+POINT1(unitx, 0.65, PX(temp)),
        cy+POINT1(unity, 0.65, PY(temp)));
    }
    DrawColor(kObjB[i]);
    DrawPoint(cx+POINT1(unitx, 0.56, PX(xplanet2[i])),
      cy+POINT1(unity, 0.56, PY(xplanet2[i])));
    DrawPoint(cx+POINT1(unitx, 0.43, PX(xplanet2[i])),
      cy+POINT1(unity, 0.43, PY(xplanet2[i])));
    if (!gs.fHouseExtra) {
      DrawColor(cp2.dir[i] < 0.0 ? gi.kiGray : gi.kiOn);
      DrawDash(cx+POINT1(unitx, 0.45, PX(xplanet2[i])),
        cy+POINT1(unity, 0.45, PY(xplanet2[i])),
        cx+POINT2(unitx, 0.54, PX(xplanet2[i])),
        cy+POINT2(unity, 0.54, PY(xplanet2[i])), 2-gs.fColor);
    }
  }
  for (i = 0; i <= cObj; i++)
    if (!FProper2(i))
      xplanet2[i] = -1.0;

  /* Now draw the inner ring of planets. If it weren't for the outer ring,  */
  /* this would be just like the standard non-relationship wheel chart with */
  /* only one set of planets. Again, draw glyph, and a line to true point.  */

  FProcessCommandLine(szWheelX[1]);
  for (i = 0; i <= cObj; i++)
    symbol[i] = xplanet1[i];
  FillSymbolRing(symbol, 1.1);
  DrawSymbolRing(symbol, xplanet1, cp1.dir, cx, cy, unitx, unity,
    0.43, 0.45, 0.48, 0.52);
  for (i = 0; i <= cObj; i++)
    if (!FProper(i))
      xplanet1[i] = -1.0;
  FProcessCommandLine(szWheelX[0]);

  // Draw lines connecting planets between the two charts that have aspects.

  if (!gs.fEquator)
    DrawAspectRelation(1, 2, xplanet1, xplanet2, cx, cy, unitx, unity, 0.41);

  // Draw sidebar with chart information and positions if need be.

  DrawSidebar();
}


/* Draw a tri-wheel chart or quad-wheel chart, where we have three or four */
/* rings, among three or four sets of data we're comparing. This chart is  */
/* obtained when the -r3 or -r4 switch is combined with the -X switch.     */

void XChartWheelThreeFour()
{
  real xsign[cSign+1], xhouse1[cSign+1], xplanet1[objMax], xplanet2[objMax],
    xplanet3[objMax], xplanet4[objMax], symbol[objMax];
  int cx, cy, i, fQuad;
  real unitx, unity, base;

  // Set up variables and temporarily automatically decrease the horizontal
  // chart size to leave room for the sidebar if that mode is in effect.

  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;
  cx = gs.xWin/2 - 1; cy = gs.yWin/2 - 1;
  unitx = (real)cx; unity = (real)cy;
  gi.rAsc = gs.objLeft ? cp1.obj[NAbs(gs.objLeft)-1] +
    rDegQuad*(gs.objLeft < 0) : cp1.cusp[1];
  if (us.fVedic)
    gi.rAsc = gs.objLeft ? (gs.objLeft < 0 ? 120.0 : -60.0)-gi.rAsc : 0.0;
  fQuad = (us.nRel == rcQuadWheel);
  base = (fQuad ? 0.23 : 0.36);

  // Fill out arrays with the degrees of the cusps and sign glyphs, and the
  // positions of the planet rings.

  if (gi.nMode == gWheel) {
    for (i = 1; i <= cSign; i++)
      xhouse1[i] = PZ(cp1.cusp[i]);
  } else {
    gi.rAsc -= cp1.cusp[1];
    for (i = 1; i <= cSign; i++)
      xhouse1[i] = PZ(ZFromS(i));
  }
  for (i = 1; i <= cSign; i++)
    xsign[i] = PZ(HousePlaceInX(ZFromS(i), 0.0));
  for (i = 0; i <= cObj; i++) {
    xplanet1[i] = PZ(HousePlaceInX(cp1.obj[i], cp1.alt[i]));
    xplanet2[i] = PZ(HousePlaceInX(cp2.obj[i], cp2.alt[i]));
    xplanet3[i] = PZ(HousePlaceInX(cp3.obj[i], cp3.alt[i]));
    if (fQuad)
      xplanet4[i] = PZ(HousePlaceInX(cp4.obj[i], cp4.alt[i]));
  }

  // Go draw the outer sign and house rings. We are drawing the houses of only
  // the outermost ring of the wheel, however.

  if (gs.fColor) {
    DrawColor(kDkGreenB);
    DrawCircle(cx, cy, (int)(unitx*0.61+rRound), (int)(unity*0.61+rRound));
    DrawCircle(cx, cy, (int)(unitx*0.48+rRound), (int)(unity*0.48+rRound));
    if (fQuad)
      DrawCircle(cx, cy, (int)(unitx*0.35+rRound), (int)(unity*0.35+rRound));
  }
  DrawWheel(xsign, xhouse1, cx, cy, unitx, unity,
    0.745, 0.78, 0.815, 0.84, 0.895);

  /* Draw the outer ring of planets (i.e. the one the house cusps reflect). */
  /* Draw each glyph, a line from it to its actual position point in the    */
  /* outer ring, and then draw another line from this point to a another    */
  /* dot at the same position on the innermost ring as well.                */

  FProcessCommandLine(szWheelX[1]);
  for (i = 0; i <= cObj; i++)
    symbol[i] = xplanet1[i];
  FillSymbolRing(symbol, 0.9);
  DrawSymbolRing(symbol, xplanet1, ret, cx, cy, unitx, unity,
    0.62, 0.63, 0.66, 0.70);
  for (i = cObj; i >= 0; i--) if (FProper(i)) {
    DrawColor(kObjB[i]);
    DrawPoint(cx+POINT1(unitx, base, PX(xplanet1[i])),
      cy+POINT1(unity, base, PY(xplanet1[i])));
    if (!gs.fHouseExtra) {
      DrawColor(ret[i] < 0.0 ? gi.kiGray : gi.kiOn);
      DrawDash(cx+POINT1(unitx, base+0.02, PX(xplanet1[i])),
        cy+POINT1(unity, base+0.02, PY(xplanet1[i])),
        cx+POINT2(unitx, 0.59, PX(xplanet1[i])),
        cy+POINT2(unity, 0.59, PY(xplanet1[i])), 3+fQuad-gs.fColor);
    }
  }
  for (i = 0; i <= cObj; i++)
    if (!FProper(i))
      xplanet1[i] = -1.0;

  // Now draw the second to outermost ring of planets. Again, draw each glyph,
  // a line to its true point, and a line to the innermost ring.

  FProcessCommandLine(szWheelX[2]);
  for (i = 0; i <= cObj; i++)
    symbol[i] = xplanet2[i];
  FillSymbolRing(symbol, 1.1);
  DrawSymbolRing(symbol, xplanet2, cp2.dir, cx, cy, unitx, unity,
    0.49, 0.50, 0.53, 0.57);
  for (i = cObj; i >= 0; i--) if (FProper(i)) {
    DrawColor(kObjB[i]);
    DrawPoint(cx+POINT1(unitx, base, PX(xplanet2[i])),
      cy+POINT1(unity, base, PY(xplanet2[i])));
    if (!gs.fHouseExtra) {
      DrawColor(cp2.dir[i] < 0.0 ? gi.kiGray : gi.kiOn);
      DrawDash(cx+POINT1(unitx, base+0.02, PX(xplanet2[i])),
        cy+POINT1(unity, base+0.02, PY(xplanet2[i])),
        cx+POINT2(unitx, 0.46, PX(xplanet2[i])),
        cy+POINT2(unity, 0.46, PY(xplanet2[i])), 2+fQuad-gs.fColor);
    }
  }
  for (i = 0; i <= cObj; i++)
    if (!FProper(i))
      xplanet2[i] = -1.0;

  // The third ring (either the innermost or second to innermost) is next.
  // Chart was cast earlier, and draw the glyphs and lines to true point.

  FProcessCommandLine(szWheelX[3]);
  for (i = 0; i <= cObj; i++)
    symbol[i] = xplanet3[i];
  FillSymbolRing(symbol, 1.4);
  DrawSymbolRing(symbol, xplanet3, ret, cx, cy, unitx, unity,
    0.36, 0.37, 0.40, 0.44);
  for (i = 0; i <= cObj; i++)
    if (!FProper(i))
      xplanet3[i] = -1.0;

  if (fQuad) {

    // If a fourth ring is being done, first finish the third one by drawing
    // lines from the true positions to the inner ring.

    for (i = cObj; i >= 0; i--) if (FProper(i)) {
      DrawColor(kObjB[i]);
      DrawPoint(cx+POINT1(unitx, base, PX(xplanet3[i])),
        cy+POINT1(unity, base, PY(xplanet3[i])));
      if (!gs.fHouseExtra) {
        DrawColor(ret[i] < 0.0 ? gi.kiGray : gi.kiOn);
        DrawDash(cx+POINT1(unitx, base+0.02, PX(xplanet3[i])),
          cy+POINT1(unity, base+0.02, PY(xplanet3[i])),
          cx+POINT2(unitx, 0.33, PX(xplanet3[i])),
          cy+POINT2(unity, 0.33, PY(xplanet3[i])), 2-gs.fColor);
      }
    }

    // If the fourth (innermost) ring is being done, take the chart that was
    // cast earlier, and draw glyphs and lines to the true positions.

    FProcessCommandLine(szWheelX[4]);
    for (i = 0; i <= cObj; i++)
      xplanet4[i] = PZ(HousePlaceInX(cp4.obj[i], cp4.alt[i]));
    for (i = 0; i <= cObj; i++)
      symbol[i] = xplanet4[i];
    FillSymbolRing(symbol, 1.8);
    DrawSymbolRing(symbol, xplanet4, ret, cx, cy, unitx, unity,
      0.23, 0.24, 0.27, 0.31);
    for (i = 0; i <= cObj; i++)
      if (!FProper(i))
        xplanet4[i] = -1.0;
  }
  FProcessCommandLine(szWheelX[0]);

  // Draw lines connecting planets between the charts that have aspects.

  if (!gs.fEquator) {
    base -= 0.02;
    DrawAspectRelation(1, 2, xplanet1, xplanet2, cx, cy, unitx, unity, base);
    DrawAspectRelation(1, 3, xplanet1, xplanet3, cx, cy, unitx, unity, base);
    DrawAspectRelation(2, 3, xplanet2, xplanet3, cx, cy, unitx, unity, base);
    if (fQuad) {
      DrawAspectRelation(1, 4, xplanet1, xplanet4, cx, cy, unitx, unity, base);
      DrawAspectRelation(2, 4, xplanet2, xplanet4, cx, cy, unitx, unity, base);
      DrawAspectRelation(3, 4, xplanet3, xplanet4, cx, cy, unitx, unity, base);
    }
  }

  // Draw sidebar with chart information and positions if need be.

  ciCore = ciMain;
  DrawSidebar();
}


/* Draw an aspect (or midpoint) grid in the window, between the planets in  */
/* two different charts, with the planets labeled at the top and side. This */
/* chart is done when the -g switch is combined with the -r0 and -X switch. */
/* Like above, the chart always has a (definable) fixed number of cells.    */

void XChartGridRelation()
{
  char sz[cchSzDef], szT[cchSzDef];
  int nScale, unit, siz, x, y, i, j, i0, j0, k, l;
  KI c;

  nScale = gi.nScale/gi.nScaleT;
  unit = CELLSIZE*gi.nScale; siz = (gi.nGridCell+1)*unit;
  sprintf(szT, "");
  i = us.fSmartCusp; us.fSmartCusp = fFalse;
  if (!FCreateGridRelation(gs.fAlt != us.fGridMidpoint))
    return;
  us.fSmartCusp = i;

  // Loop through each cell in each row and column of grid.

  for (y = 0, j0 = -2; y <= gi.nGridCell; y++) {
    do {
      j0++;
      j = rgobjList[j0];
    } while (j0 >= 0 && ignore[j] && j0 <= cObj);
    DrawColor(gi.kiGray);
    DrawDash(0, (y+1)*unit, siz, (y+1)*unit, !gs.fColor);
    DrawDash((y+1)*unit, 0, (y+1)*unit, siz, !gs.fColor);
    DrawColor(gi.kiLite);
    DrawEdge(0, y*unit, unit, (y+1)*unit);
    DrawEdge(y*unit, 0, (y+1)*unit, unit);
    DrawEdge(y*unit, y*unit, (y+1)*unit, (y+1)*unit);
    if (j0 <= cObj) for (x = 0, i0 = -2; x <= gi.nGridCell; x++) {
      do {
        i0++;
        i = rgobjList[i0];
      } while (i0 >= 0 && ignore[i] && i0 <= cObj);

      // Again, are looping through each cell in each row and column.

      if (i0 <= cObj) {
        gi.xTurtle = x*unit+unit/2;
        gi.yTurtle = y*unit+unit/2 - (nScale > 2 ? 5*gi.nScaleT : 0);
        k = i >= 0 && j >= 0 ? grid->n[i][j] : 0;

        // If current cell is on top row or left hand column, draw glyph of
        // planet owning the particular row or column in question.

        if (y == 0 || x == 0) {
          if (gs.fLabelAsp) {
            DrawColor(kDkBlueB);
            DrawBlock(x*unit+1, y*unit+1, (x+1)*unit-1, (y+1)*unit-1);
          }
          if (x+y > 0)
            DrawObject(y == 0 ? i : j, gi.xTurtle, gi.yTurtle);
        } else {

        // Otherwise, draw glyph of aspect in effect, or glyph of sign of
        // midpoint, between the two planets in question.

          if (gs.fAlt == us.fGridMidpoint) {
            if (k) {
              DrawColor(c = kAspB[k]);
              DrawAspect(k, gi.xTurtle, gi.yTurtle);
            }
          } else {
            DrawColor(c = kSignB(grid->n[i][j]));
            DrawSign(grid->n[i][j], gi.xTurtle, gi.yTurtle);
          }
        }

        // When scale size is 300+, print some text in current cell.

        if (nScale > 2 && gs.fLabel) {

          // For top and left edges, print sign and degree of the planet.

          if (y == 0 || x == 0) {
            if (x+y > 0) {
              k = SFromZ(y == 0 ? cp2.obj[i] : cp1.obj[j]);
              l = (int)((y == 0 ? cp2.obj[i] : cp1.obj[j])-ZFromS(k));
              c = kSignB(k);
              if (nScale > 3 && is.fSeconds)
                sprintf(szT, "%c%02d", chDeg2,
                  (int)((y == 0 ? cp2.obj[i] : cp1.obj[j])*60.0)%60);
              sprintf(sz, "%.3s %02d%s", szSignName[k], l, szT);

              // For extreme upper left corner, print some little arrows
              // pointing out chart1's planets and chart2's planets.

            } else {
              c = gi.kiLite;
              sprintf(sz, "1v 2->");
            }
          } else {
            l = NAbs(grid->v[i][j]); k = l / 60; l %= 60;
            if (nScale > 3 && is.fSeconds)
              sprintf(szT, "%02d", l);

            // For aspect cells, print the orb in degrees and minutes.

            if (gs.fAlt == us.fGridMidpoint) {
              if (grid->n[i][j]) {
                sprintf(sz, "%c%d%c%02d'%s", grid->v[i][j] < 0 ?
                  (us.fAppSep ? 'a' : '-') : (us.fAppSep ? 's' : '+'),
                  k/60, chDeg2, k%60, szT);
                if (nScale == 3)
                  sz[7] = chNull;
              } else
                sprintf(sz, "");

            // For midpoint cells, print degree and minute.

            } else
              sprintf(sz, "%2d%c%02d'%s", k/60, chDeg2, k%60, szT);
          }
          DrawColor(c);
          DrawSz(sz, x*unit+unit/2, (y+1)*unit-3*gi.nScaleT, dtBottom);
        }
      }
    }
  }
}


/* Draw a chart showing a graphical ephemeris for the given month, year,  */
/* or range of years, with the date on the vertical axis and the zodiac   */
/* on the horizontal, as done when the -E is combined with the -X switch. */

void XChartEphemeris()
{
  real symbol[cObj*2+2], objSav[objMax], rT;
  char sz[cchSzDef];
  int cYea, unit = 6*gi.nScale, daytot, d = 1, dd, day, mon, yea, monsiz,
    x1, y1, x2, y2, xs, ys, m, n, u, v = 0, i, j, dx;
  flag fSav;

  cYea = us.nEphemYears;    // Is this -Ey -X or just -E -X?
  if (cYea) {
    daytot = 0;
    for (i = 0; i < cYea; i++)
      daytot += DayInYear(Yea + i);
    day = 1; mon = 1; yea = Yea; monsiz = 31;
  } else
    daytot = DayInMonth(Mon, Yea);
  x1 = (3 + Min(cYea, 2))*xFontT; y1 = unit*2;
  x2 = gs.xWin - x1;
  y2 = gs.yWin - y1 - gs.fText*yFontT;
  xs = x2 - x1; ys = y2 - y1;
  dd = (daytot / ys + 2) * (2 - us.fSeconds);
  dd = Min(dd, 28);

  // Display glyphs of the zodiac along the bottom axis.

  if (!us.fParallel)
    for (i = 1; i <= cSign+1; i++) {
      m = x1 + NMultDiv(xs, i-1, 12);
      j = i > cSign ? 1 : i;
      DrawColor(kSignB(j));
      DrawSign(j, m, y2 + unit);
      if (!gs.fColorSign)
        DrawColor(gi.kiGray);
      DrawDash(m, y1, m, y2, 2);
    }
  else {
    dx = gs.nRayWidth / 10; dx = Min(dx, 90); dx = Max(dx, 1);
    for (i = -90; i <= 90; i += (dx > 30 ? 10 : (dx > 6 ? 5 : 1))) {
      if (i < -dx || i > dx)
        continue;
      m = x1 + NMultDiv(xs, i+dx, dx << 1);
      j = i > cSign ? 1 : i;
      DrawColor(i ? gi.kiLite : gi.kiOn);
      sprintf(sz, "%s%d", i > 0 ? "+" : "", i);
      DrawSz(sz, m, y2+2, dtTop | dtScale2);
      DrawColor(gi.kiGray);
      DrawDash(m, y1, m, y2, 2);
    }
  }

  // Loop and display planet movements for one day segment.

  while (d <= daytot + 1) {
    n = v;
    if (gs.fLabel &&
      (cYea ? (mon == Mon && day == 1 && yea == Yea) : (d == Day))) {
      // Marker line for specific day.
      if (cYea)
        v = y1 + NMultDiv(ys, d-2+Day, daytot);
      else
        v = y1 + NMultDiv(ys, (d-1)*24 + (int)Tim, daytot*24);
      DrawColor(kDkGreenB);
      DrawLine(x1, v, x2, v);
    }
    v = y1 + NMultDiv(ys, d-1, daytot);
    if (!gs.fEquator && (!cYea || day == 1)) {
      // Marker line for day or month.
      DrawColor(gi.kiGray);
      DrawDash(x1, v, x2, v, cYea <= 1 || mon == 1 ? 1 : 3);
    }
    if (d > 1)
      for (i = 0; i <= cObj; i++)
        objSav[i] = planet[i];
    ciCore = ciMain;
    if (cYea) {
      MM = mon; DD = day; YY = yea;
    } else
      DD = d;
    CastChart(-1);
    if (us.fParallel)
      for (i = 0; i <= cObj; i++) {
        rT = (planetalt[i] * rDegHalf / (real)dx) + rDegHalf;
        rT = Min(rT, rDegMax);
        rT = Max(rT, 0.0);
        planet[i] = rT;
      }

    // Draw planet glyphs along top of chart.
    if (d <= 1) {
      for (i = 0; i <= cObj; i++) {
        j = !FProper(i);
        symbol[i*2] = (j || us.nRel > rcDual) ? -rLarge : cp2.obj[i];
        symbol[i*2+1] = (j ? -rLarge : planet[i]);
      }
      FillSymbolLine(symbol);
      fSav = gs.fLabel; gs.fLabel = fTrue;
      for (i = cObj*2+1; i >= 0; i--) {
        j = i >> 1;
        if (symbol[i] >= 0.0)
          DrawObject(j, x1 + (int)((real)xs * symbol[i] / rDegMax), unit);
      }
      gs.fLabel = fSav;
      if (us.nRel <= rcDual) {
        for (i = cObj; i >= 0; i--) {
          if (!FProper(i))
            continue;
          j = x1 + (int)((real)xs * cp2.obj[i] / rDegMax);
          DrawColor(kObjB[i]);
          DrawDash(j, y1, j, y2, 1);
        }
      }

    // Draw a line segment for each object during this time section.
    } else
      for (i = cObj; i >= 0; i--) {
        if (!FProper(i))
          continue;
        m = x1 + (int)((real)xs * objSav[i] / rDegMax);
        u = x1 + (int)((real)xs * planet[i] / rDegMax);
        DrawColor(kObjB[i]);
        DrawWrap(m, n, u, v,
          !us.fParallel && ret[i] > 0.0 && i != oFor ? -x1 : x1, x2);
      }

    // Label months or days in the month along the left and right edges.
    if (d <= daytot && (!cYea || (day == 1 && (cYea <= 1 || mon == 1)))) {
      if (cYea) {
        if (cYea <= 1)
          sprintf(sz, "%.3s", szMonth[mon]);
        else
          sprintf(sz, "%4d", yea);
        i = (cYea <= 1 ? mon == Mon : yea == Yea);
      } else {
        sprintf(sz, "%2d", d);
        i = (d == Day);
      }
      DrawColor(gs.fLabel && i ? gi.kiOn : gi.kiLite);
      DrawSz(sz,      xFontT/2, v + (yFont-2)*gi.nScaleTextT,
        dtLeft | dtBottom | dtScale2);
      DrawSz(sz, x2 + xFontT/2, v + (yFont-2)*gi.nScaleTextT,
        dtLeft | dtBottom | dtScale2);
    }

    // Now increment the day counter. For a month we always go up by one.
    // For a year we go up by four or until the end of the month reached.
    if (cYea) {
      day += dd;
      if (day > monsiz) {
        d += dd - (day-monsiz-1);
        if (d <= daytot + 1) {
          mon++;
          if (mon > cSign) {
            yea++;
            mon = 1;
          }
          monsiz = DayInMonth(mon, yea);
          day = 1;
        }
      } else
        d += dd;
    } else
      d++;
  }
  DrawColor(gi.kiLite);
  DrawEdge(x1, y1, x2, y2);

  ciCore = ciMain;    // Recast original chart.
  CastChart(1);
}


/* Draw a chart showing a graphical ephemeris of Ray influences for the   */
/* given month or year, with the date on the vertical axis and each Ray   */
/* on the horizontal, as done when the -7 is combined with the -X switch. */

void XChartEsoteric()
{
  real rRay[cRay+2], rRaySav[cRay+2], power1[objMax], power2[objMax],
    power[oNorm+1];
  char sz[cchSzDef];
  int daytot, d = 1, day, mon, monsiz,
    x1, y1, x2, y2, xs, ys, m, n, u, v = 0, i, j, k;
  flag fYea;

  EnsureRay();
  fYea = (us.nEphemYears > 0);    // Doing an entire year or just a month?
  if (fYea) {
    daytot = DayInYear(Yea);
    day = 1; mon = 1; monsiz = 31;
  } else
    daytot = DayInMonth(Mon, Yea);
  x1 = (3 + fYea) * xFontT; y1 = 12 * gi.nScaleTextT;
  x2 = gs.xWin - x1; y2 = gs.yWin - y1;
  xs = x2 - x1; ys = y2 - y1;

  // Label Rays along the top axis.

  for (i = 1; i <= cRay+1; i++) {
    m = x1 + NMultDiv(xs, i-1, cRay+1);
    DrawColor(gi.kiGray);
    DrawDash(m, y1, m, y2, 2);
    if (i <= cRay)
      sprintf(sz, "Ray %d", i);
    else
      sprintf(sz, "Average");
    DrawColor(i <= cRay ? kRayB[i] : gi.kiOn);
    DrawSz(sz, x1 + xs*(i-1)/8, y1 - 3*gi.nScaleTextT,
      dtLeft | dtBottom | dtScale2);
  }

  // Loop and display Ray influences for one day segment.

  while (d <= daytot + 1) {
    n = v;
    if (gs.fLabel && (fYea ? (mon == Mon && day == 1) : (d == Day))) {
      if (fYea)
        v = y1 + NMultDiv(ys, d-2+Day, daytot);
      else
        v = y1 + NMultDiv(ys, (d-1)*24 + (int)Tim, daytot*24);
      DrawColor(kDkCyanB);
      DrawLine(x1, v, x2, v);       // Marker line for specific day.
    }
    v = y1 + NMultDiv(ys, d-1, daytot);
    if (!gs.fEquator && (!fYea || day == 1)) {
      DrawColor(gi.kiGray);
      DrawDash(x1, v, x2, v, 1);    // Marker line for day or month.
    }
    if (d > 1)
      for (i = 1; i <= cRay+1; i++)
        rRaySav[i] = rRay[i];
    ciCore = ciMain;
    if (fYea) {
      MM = mon; DD = day;
    } else
      DD = d;
    CastChart(-1);

    // Compute Ray influences for current day.
    for (i = 0; i <= cRay+1; i++)
      rRay[i] = 0.0;
    ComputeInfluence(power1, power2);
    for (i = 0; i <= oNorm; i++) {
      power[i] = power1[i] + power2[i];
      if (FIgnore(i))
        continue;
      k = SFromZ(planet[i]);
      for (j = 1; j <= cRay; j++)
        if (rgSignRay2[k][j]) {
          if (!gs.fAlt)
            rRay[j] += power[i];
          else
            rRay[j] += power[i] / (420 / rgSignRay2[k][j]);
        }
    }
    for (i = 0; i <= cRay; i++)
      rRay[cRay+1] += rRay[i] / 7.0;

    // Draw a line segment for each Ray during this time section.
    if (d > 1)
      for (i = 1; i <= cRay+1; i++) {
        k = x1 + (i-1)*xs/8;
        m = k + (int)((real)xs * rRaySav[i] / 8.0 / (real)gs.nRayWidth);
        u = k + (int)((real)xs * rRay[i]    / 8.0 / (real)gs.nRayWidth);
        DrawColor(i <= cRay ? kRayB[i] : gi.kiOn);
        DrawLine(m, n, u, v);
      }

    // Label months or days in the month along the left and right edges.
    if (d <= daytot && (!fYea || day == 1)) {
      if (fYea) {
        sprintf(sz, "%.3s", szMonth[mon]);
        i = (mon == Mon);
      } else {
        sprintf(sz, "%2d", d);
        i = (d == Day);
      }
      DrawColor(gs.fLabel && i ? gi.kiOn : gi.kiLite);
      i = v + gi.nScaleT;
      DrawSz(sz,      xFontT/2,              i, dtLeft | dtTop | dtScale2);
      DrawSz(sz, x2 + xFontT/2, i, dtLeft | dtTop | dtScale2);
    }

    // Now increment the day counter. For a month we always go up by one.
    // For a year we go up by four or until the end of the month reached.
    if (fYea) {
      i = us.fSeconds ? 1 : 4;
      day += i;
      if (day > monsiz) {
        d += i-(day-monsiz-1);
        if (d <= daytot + 1) {
          mon++;
          monsiz = DayInMonth(mon, Yea);
          day = 1;
        }
      } else
        d += i;
    } else
      d++;
  }
  DrawColor(gi.kiLite);
  DrawEdge(x1, y1, x2, y2);

  ciCore = ciMain;    // Recast original chart.
  CastChart(1);
}


/* Draw a chart graphing transits over time. This covers both transit to  */
/* transit (-B switch) and transit to natal (-V switch), when they're     */
/* combined with the -X switch. Each aspect present during the period has */
/* a row, showing its strength from 0 (outside of orb) to 100% (exact).   */

void XChartTransit(flag fTrans, flag fProg)
{
  TransGraInfo *rgEph;
  word **ppw, *pw, *pw2;
  char sz[cchSzDef];
  int cAsp, cSect, cTot, ymin, x, y, asp, iw, iwFocus = -1, nMax, n, obj,
    iy, yRow, cRow = 0, xWid, xo, yo, iSect, iFrac, xp, yp, yp2, dyp, et;
  flag fMonth = us.fInDayMonth, fYear = us.fInDayYear, fEclipse =
    us.fEclipse && !fTrans && us.objCenter == oEar && !us.fParallel;
  CI ciT;
  real rT, rPct;

  // Initialize variables.
  rgEph = (TransGraInfo *)PAllocate(sizeof(TransGraInfo),
    "transit graph grid");
  if (rgEph == NULL)
    goto LDone;
  ClearB((pbyte)(*rgEph), sizeof(TransGraInfo));

  if (!fTrans && !fProg)
    ciT = ciMain;
  else
    ciT = ciTran;
  cAsp = fTrans && is.fReturn ? aCon : us.nAsp;
  yRow = gi.nScale * 12;
  xo = yRow*3 + fTrans*gi.nScaleT*20;
  ymin = 1-fTrans;

  // Determine pixel width of chart based on time period being graphed.
  if (!fMonth)
    cSect = 24;
  else if (!fYear)
    cSect = DayInMonth(ciT.mon, ciT.yea);
  else if (us.nEphemYears <= 1)
    cSect = 12;
  else
    cSect = 5*12;
  xWid = (gs.xWin - xo - 2) / cSect; xWid = Max(xWid, 1);
  cTot = cSect * xWid + 1;

  // Calculate and fill out aspect strength arrays for each aspect present.
  if (fTrans || fProg) {
    ciCore = ciMain;
    us.fProgress = fFalse;
    CastChart(0);
    cp1 = cp0;
  }
  if (!gs.fAlt) {
    if (!fMonth)
      iwFocus = (int)(ciT.tim * (real)xWid);
    else if (!fYear)
      iwFocus = (int)(((real)(ciT.day-1) + ciT.tim/24.0) * (real)xWid);
    else if (us.nEphemYears <= 1)
      iwFocus = (int)(((real)(ciT.mon-1) + ((real)(ciT.day-1) +
        ciT.tim/24.0) / (real)DayInMonth(ciT.mon, ciT.yea)) * (real)xWid);
    else
      iwFocus = (int)((24.0 + ((real)(ciT.mon-1) + ((real)(ciT.day-1) +
        ciT.tim/24.0) / (real)DayInMonth(ciT.mon, ciT.yea))) * (real)xWid);
    if (iwFocus == 0 && ciT.tim <= 0.0)
      iwFocus = -1;
  }
  for (iw = 0; iw < cTot; iw++) {
    iSect = iw / xWid; iFrac = iw % xWid;

    // Cast chart for current time slice.
    ciCore = ciT;
    rT = (real)iFrac / (real)xWid;
    if (!fMonth) {
      TT = (real)iSect + rT;
    } else if (!fYear) {
      DD = iSect + 1;
      TT = rT * 24.0;
    } else if (us.nEphemYears <= 1) {
      MM = iSect + 1;
      rT *= (real)DayInMonth(MM, YY);
      DD = (int)rT;
      TT = RFract(rT) * 24.0;
    } else {
      YY = YY - 2 + iSect/12;
      MM = (iSect % 12) + 1;
      rT *= (real)DayInMonth(MM, YY);
      DD = (int)rT;
      TT = RFract(rT) * 24.0;
    }

    if (fTrans)
      for (obj = 0; obj <= oNorm; obj++)
        SwapN(ignore[obj], ignore2[obj]);
    if (us.fProgress = fProg) {
      is.JDp = MdytszToJulian(MM, DD, YY, TT, SS, ZZ);
      ciCore = ciMain;
    }
    CastChart(-1);
    if (fTrans)
      for (obj = 0; obj <= oNorm; obj++)
        SwapN(ignore[obj], ignore2[obj]);

    // Compute aspects present for current time slice.
    if (!fTrans) {
      if (!FCreateGrid(fFalse))
        goto LDone;
    } else {
      cp2 = cp0;
      if (!FCreateGridRelation(fFalse))
        goto LDone;
    }

    // For each aspect present in slice, add its strength to array.
    for (y = ymin; y <= cObj; y++) {
      if (FIgnore(y) || (!fTrans && !FProperGraph(y)))
        continue;
      for (x = 0; x < (fTrans ? cObj+1 : y); x++) {
        if (!fTrans ? !FProper(x) :
          FIgnore2(x) || !FProperGraph(x) || (is.fReturn && x != y))
          continue;
        asp = grid->n[x][y];
        if (!FBetween(asp, aCon, cAsp))
          continue;
        ppw = &(*rgEph)[x][y][asp];
        if (*ppw == NULL) {
          *ppw = (word *)PAllocate(cTot * sizeof(word),
            "transit ephemeris entry");
          if (*ppw == NULL)
            goto LDone;
          pw = *ppw;
          ClearB((pbyte)pw, cTot * sizeof(word));
          cRow++;
        } else
          pw = *ppw;
        n = grid->v[x][y];
        rT = (real)NAbs(n) / 3600.0;
        rT /= GetOrb(x, y, asp);
        pw[iw] = 65535 - (int)(rT * (65536.0 - rSmall));

        // Check for and add eclipse information to array too.
        if (fEclipse) {
          et = etNone;
          if (asp == aCon)
            et = NCheckEclipse(x, y, &rPct);
          else if (asp == aOpp && x == oSun && y == oMoo)
            et = NCheckEclipseLunar(&rPct);
          if (et > etNone) {
            ppw = &(*rgEph)[y][x][asp];
            if (*ppw == NULL) {
              *ppw = (word *)PAllocate(cTot * sizeof(word),
                "transit eclipse entry");
              if (*ppw == NULL)
                goto LDone;
              pw2 = *ppw;
              ClearB((pbyte)pw2, cTot * sizeof(word));
            } else
              pw2 = *ppw;
            rT = (et < etPartial ? 50.0 : (et > etPartial ? 450.0 : 250.0)) +
              rPct;
            pw2[iw] = (int)(rT * 65535.0 / 600.0);
          }
        }
      }
    }
  }

  // Print chart header row.
  yo = gi.nScaleTextT*12;
  yp = gi.nScaleTextT*2;

  DrawColor(gi.kiOn);
  if (!fMonth)
    sprintf(sz, SzDate(ciT.mon, ciT.day, ciT.yea, fFalse));
  else if (!fYear)
    sprintf(sz, "%3.3s%5d", szMonth[ciT.mon], ciT.yea);
  else if (us.nEphemYears <= 1)
    sprintf(sz, "%4d", ciT.yea);
  else
    sz[0] = chNull;
  if (CchSz(sz) * xFontT < xo)
    DrawSz(sz, yp, yp, dtLeft | dtTop | dtScale2);

  DrawColor(gi.kiLite);
  if (!fMonth) {
    for (x = 0; x < 24; x++) {
      if (!us.fEuroTime)
        sprintf(sz, "%d%c", ((x+11) % 12)+1, x < 12 ? 'a' : 'p');
      else
        sprintf(sz, "%d", x);
      DrawSz(sz, xo + x*xWid, yp, dtLeft | dtTop | dtScale2);
    }
  } else if (!fYear) {
    for (x = 0; x < cSect; x++) {
      if (gs.fColorSign) {
        n = DayOfWeek(MM, x+1, YY);
        DrawColor(kRainbowB[n+1]);
      }
      sprintf(sz, "%d", x+1);
      DrawSz(sz, xo + x*xWid, yp, dtLeft | dtTop | dtScale2);
    }
  } else if (us.nEphemYears <= 1) {
    for (x = 0; x < cSign; x++) {
      if (gs.fColorSign)
        DrawColor(kElemB[x & 3]);
      sprintf(sz, "%3.3s", szMonth[x+1]);
      DrawSz(sz, xo + x*xWid, yp, dtLeft | dtTop | dtScale2);
    }
  } else {
    for (x = 0; x < 5; x++) {
      sprintf(sz, "%d", (fTrans || fProg ? YeaT : Yea) - 2 + x);
      DrawSz(sz, xo + x*12*xWid, yp, dtLeft | dtTop | dtScale2);
    }
  }

  // Determine scrolling position if too many aspects to fit on bitmap.
#ifdef WIN
  cRow = (cRow - (gs.yWin / (yRow + 1))) * wi.yScroll / nScrollDiv;
#else
  cRow = 0;
#endif

  // Draw the individual aspects present in order.
  iy = 0;
  for (y = ymin; y <= cObj; y++)
    for (x = 0; x < (fTrans ? cObj+1 : y); x++)
      for (asp = 1; asp <= cAsp; asp++) {
        pw = (*rgEph)[x][y][asp];
        if (pw == NULL)
          continue;
        if (cRow > 0) {
          cRow--;
          continue;
        }
        iy++;
        yp2 = yo + iy*yRow - yRow/2;
        yp = yp2 + yRow/2;
        if (yp >= gs.yWin)
          goto LDone;
        xp = 0;

        // Draw the glyphs for the aspect in question.
        if (fTrans) {
          DrawColor(gi.kiGray);
          DrawSz(!fProg ? "T." : "P.", xp + gi.nScaleT*5 + gi.nScale,
            yp2 + gi.nScaleT*2, dtCent | dtScale2);
          xp += gi.nScaleT*10;
        }
        xp += yRow/2;
        DrawObject(x, xp, yp2);
        xp += yRow;
        DrawColor(kAspB[asp]);
        DrawAspect(asp, xp, yp2);
        xp += yRow;
        if (fTrans) {
          DrawColor(kSignB(SFromZ(cp1.obj[y])));
          DrawSz("N.", xp + gi.nScaleT*5 - gi.nScale*5, yp2 + gi.nScaleT*2,
            dtCent | dtScale2);
          xp += gi.nScaleT*10;
        }
        DrawObject(y, xp, yp2);
        DrawColor(gi.kiGray);
        DrawBlock(0, yp, xo + iw - 1, yp);
        if (iy <= 1)
          DrawBlock(0, yp - yRow, xo + iw - 1, yp - yRow);

        // Draw the graph itself for the aspect in question.
        nMax = -1;
        for (iw = 0; iw < cTot; iw++) {
          n = pw[iw];
          if (n > nMax)
            nMax = n;
        }
        for (iw = 0; iw < cTot; iw++) {
          if (iw == iwFocus) {
            DrawColor(kDkGreenB);
            DrawBlock(xo + iw, yp - yRow + 1, xo + iw, yp-1);
          } else if (iw % xWid == 0) {
            DrawColor(gi.kiGray);
            n = 1;
            if (fMonth && fYear && us.nEphemYears > 1 && iw % (xWid * 12) > 0)
              n = 3;
            DrawDash(xo + iw, yp - yRow + 1 + (n > 1), xo + iw, yp, n);
          }
          n = pw[iw];
          if (n > 0) {
            dyp = (n-1) * (yRow-1) / 65535;
            DrawColor(n >= nMax || ((iw <= 0 || n > pw[iw-1]) &&
              (iw >= cTot-1 || n > pw[iw+1])) ? gi.kiOn : kAspB[asp]);
            DrawBlock(xo + iw, yp-1 - dyp, xo + iw, yp-1);

            // Draw eclipse strength overlaying aspect strength, if present.
            if (fEclipse) {
              pw2 = (*rgEph)[y][x][asp];
              if (pw2 != NULL && pw2[iw] > 0) {
                n = pw2[iw];
                dyp = (n-1) * (yRow-1) / 65535;
                DrawColor(kDkBlueB);
                DrawBlock(xo + iw, yp-1 - dyp, xo + iw, yp-1);
              }
            }
          }
        }
      }

LDone:
  if (gs.fBorder) {
    DrawColor(gi.kiOn);
    DrawEdge(0, 0, xo + cTot, gs.yWin-1);
  }

  // Free temporarily allocated data, and restore original chart.
  for (y = ymin; y <= cObj; y++)
    for (x = 0; x < (fTrans ? cObj+1 : y); x++)
      for (asp = 1; asp <= cAsp; asp++) {
        pw = (*rgEph)[x][y][asp];
        if (pw != NULL)
          DeallocateP(pw);
        if (fEclipse) {
          pw2 = (*rgEph)[y][x][asp];
          if (pw2 != NULL)
            DeallocateP(pw2);
        }
      }
  if (rgEph != NULL)
    DeallocateP(rgEph);
  ciCore = ciMain;
  us.fProgress = fFalse;
  CastChart(1);
}


#ifdef BIORHYTHM
/* Draw a graphic biorhythm chart on the screen, as is done when the -rb    */
/* switch is combined with -X. This is technically a relationship chart in  */
/* that biorhythm status is determined by a natal chart time at another     */
/* later time. For the day in question, and for two weeks before and after, */
/* the Physical, Emotional, and Mental percentages are plotted.             */

void XChartBiorhythm()
{
  char sz[cchSzDef];
  CONST char *c;
  real jd, r, a;
  int x1, x2, xs, cx, y1, y2, ys, cy, i, j, k, x, y, x0, y0;

  k = xFontT*6;
  x1 = k; x2 = gs.xWin-k; xs = x2-x1; cx = (x1+x2)/2;
  k = CELLSIZE*gi.nScaleTextT;
  y1 = k; y2 = gs.yWin-k; ys = y2-y1; cy = (y1+y2)/2;

  // Create a dotted day/percentage grid to graph on.
  DrawColor(gi.kiGray);
  DrawDash(x1, cy, x2, cy, 1);
  DrawDash(cx, y1, cx, y2, 1);
  for (j = -us.nBioday+1; j <= us.nBioday-1; j++) {
    x = x1 + NMultDiv(xs, j+us.nBioday, us.nBioday*2);
    for (k = -90; k <= 90; k += 10) {
      y = y1 + NMultDiv(ys, 100+k, 200);
      DrawPoint(x, y);
    }
  }

  // Now actually draw the three biorhythm curves.
  for (i = 1; i <= 3; i++) {
    jd = RFloor(is.JD + rRound);
    switch (i) {
    case 1: r = brPhy; c = "PHYS"; j = eFir; break;
    case 2: r = brEmo; c = "EMOT"; j = eWat; break;
    case 3: r = brInt; c = "INTE"; j = eEar; break;
    }
    DrawColor(kElemB[j]);
    for (jd -= (real)us.nBioday, j = -us.nBioday; j <= us.nBioday;
      j++, jd += 1.0) {
      a = RBiorhythm(jd, r);
      x = x1 + NMultDiv(xs, j+us.nBioday, us.nBioday*2);
      y = y1 + (int)((real)ys * (100.0-a) / 200.0);
      if (j > -us.nBioday)
        DrawLine(x0, y0, x, y);
      else
        DrawSz(c, x1/2, y+2*gi.nScaleT, dtCent | dtScale2);
      x0 = x; y0 = y;
    }
  }

  DrawColor(gi.kiLite);
  // Label biorhythm percentages along right vertical axis.
  for (k = -100; k <= 100; k += 10) {
    sprintf(sz, "%c%3d%%", k < 0 ? '-' : '+', NAbs(k));
    y = y1 + NMultDiv(ys, 100-k, 200);
    DrawSz(sz, (x2+gs.xWin)/2, y+2*gi.nScaleTextT, dtCent | dtScale2);
  }
  // Label days on top horizontal axis.
  k = Max(us.nBioday/7, 1);
  for (j = -(us.nBioday/k)*k; j < us.nBioday; j += k) {
    x = x1 + NMultDiv(xs, j+us.nBioday, us.nBioday*2);
    sprintf(sz, "%c%d", j < 0 ? '-' : '+', NAbs(j));
    DrawSz(sz, x, y1-2*gi.nScaleTextT, dtBottom | dtScale2);
  }
  DrawEdge(x1, y1, x2, y2);
}
#endif /* BIORHYTHM */
#endif /* GRAPH */

/* xcharts2.cpp */
