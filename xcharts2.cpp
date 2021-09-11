/*
** Astrolog (Version 7.30) File: xcharts2.cpp
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


#ifdef GRAPH
/*
******************************************************************************
** Chart Graphics Utility Procedures.
******************************************************************************
*/

// Return whether the specified object should be displayed in the current
// graphics chart type. For example, don't include the Moon in the solar
// system charts when ephemeris files are off, don't include house cusps
// in astro-graph charts, and so on, in addition to checking restrictions.

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
  return f;
}


// Adjust an array of zodiac positions so that no two positions are within a
// certain orb of each other. This is used by the wheel drawing chart routines
// in order to make sure that planet glyphs aren't drawn on top of each other.
// Later draw the glyphs at the adjusted positions.

void FillSymbolRing(real *symbol, real factor)
{
  real orb = DEFORB*256.0/(real)gs.yWin*(real)gi.nScale*factor, k1, k2, temp;
  int i, j, k = 1, l;

  // Keep adjusting as long as can still make changes, or until 'n' rounds are
  // done. (With many objects, there just may not be enough room for all.)

  for (l = 0; k && l < us.nDivision*2; l++) {
    k = 0;
    for (i = 0; i <= is.nObj; i++) if (FProper(i)) {

      // For each object, determine who is closest on either side.

      k1 = rLarge; k2 = -rLarge;
      for (j = 0; j <= is.nObj; j++)
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


// Adjust an array of longitude positions so that no two are within a certain
// orb of each other. This is used by the astro-graph routine to make sure no
// planet glyphs marking the lines are drawn on top of each other. This is
// almost identical to the FillSymbolRing() routine used by the wheel charts,
// however there the glyphs are placed in a continuous ring, while here the
// left and right screen edges are present. Also, here are placing two sets of
// planets at the same time.

void FillSymbolLine(real *symbol)
{
  real orb = DEFORB*1.35*(real)gi.nScale, max = rDegMax, k1, k2, temp;
  int i, j, k = 1, l, tot = is.nObj*2+1;

  if (gi.nMode != gEphemeris)
    max *= (real)gi.nScale;
  else
    orb *= rDegMax/(real)gs.xWin;

  // Keep adjusting as long as can still make changes.

  for (l = 0; k && l < us.nDivision*2; l++) {
    k = 0;
    for (i = 0; i <= tot; i++)
      if (FProper(i >> 1) && symbol[i] >= 0.0) {

        // For each object, determine who is closest to the left and right.

        k1 = max-symbol[i]; k2 = -symbol[i];
        for (j = 0; j <= tot; j++) {
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


// Given a zodiac position, return the degree on the current wheel chart
// circle where that position falls, rotating based on the Ascendant and
// adding in the opposite direction for Vedic mode wheels.

real PlaceInX(real deg)
{
  if (us.fVedic)
    deg = -chouse[1]*(gi.nMode != gWheel)*2.0-deg-60.0;
  return Mod(rDegHalf-deg+gi.rAsc);
}


// Given a zodiac degree, adjust it if need be to account for the expanding
// and compacting of parts the zodiac that happen when displaying a graphic
// wheel chart such that all the houses appear the same size.

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


// Draw lines connecting planets between two charts that have aspects. Used
// when creating bi-wheels and beyond.

void DrawAspectRelation(int n1, int n2, real obj1[objMax], real obj2[objMax],
  int cx, int cy, real rz)
{
  CP cpA, cpB;
  real rx = (real)cx, ry = (real)cy;
  int i, j;

  // Put the two sets of chart data to compare in cp1 and cp2.
  if (n1 != 1) {
    cpA = cp1;
    cp1 = *rgpcp[n1];
  }
  if (n2 != 2) {
    cpB = cp2;
    cp2 = *rgpcp[n2];
  }

  // Compute and draw the aspect lines.
  if (!FCreateGridRelation(fFalse))
    goto LExit;
  for (j = is.nObj; j >= 0; j--)
    for (i = is.nObj; i >= 0; i--)
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

// Draw another wheel chart, however this time there are two rings of planets
// because this is a bi-wheel relationship chart between two sets of data.
// This chart is obtained when the -r0 is combined with the -X switch.

void XChartWheelRelation()
{
  real xsign[cSign+1], xhouse1[cSign+1], xplanet1[objMax], xplanet2[objMax],
    symbol[objMax];
  byte ignoreT[objMax];
  int cx, cy, i;
  real unitx, unity;

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
  for (i = 0; i <= is.nObj; i++)
    xplanet1[i] = PZ(HousePlaceInX(cp1.obj[i], cp1.alt[i]));
  for (i = 0; i <= is.nObj; i++)
    xplanet2[i] = PZ(HousePlaceInX(cp2.obj[i], cp2.alt[i]));

  // Go draw the outer sign and house rings. We are drawing only the houses
  // of one of the two charts in the relationship, however.

  if (gs.fColor) {
    DrawColor(kDkGreenB);
    DrawCircle(cx, cy, (int)(unitx*0.55+rRound), (int)(unity*0.55+rRound));
  }
  DrawWheel(xsign, xhouse1, cx, cy, unitx, unity, 0.70, 0.78, 0.82);

  // Draw the outer ring of planets (based on the planets in the chart which
  // the houses do not reflect - the houses belong to the inner ring below).
  // Draw each glyph, a line from it to its actual position point in the outer
  // ring, and then draw another line from this point to a another dot at the
  // same position in the inner ring as well.

  if (us.nRel == rcTransit)
    for (i = 0; i <= is.nObj; i++) {
      ignoreT[i] = ignore[i];
      ignore[i] = ignore2[i];
    }
  DrawRing(2, 2 /* so lines are dotted */ + 1, xplanet2, symbol, cx, cy,
    0.41, 0.43, 0.54, 0.56, 0.58, 0.61, 0.65, 1.0);
  if (us.nRel == rcTransit)
    for (i = 0; i <= is.nObj; i++)
      ignore[i] = ignoreT[i];

  // Now draw the inner ring of planets. If it weren't for the outer ring,
  // this would be just like the standard non-relationship wheel chart with
  // only one set of planets. Again, draw glyph, and a line to the true point.

  DrawRing(1, 2, xplanet1, symbol, cx, cy,
    0.0, 0.0, 0.0, 0.41, 0.43, 0.46, 0.50, 1.1);
  FProcessCommandLine(szWheelX[0]);

  // Draw lines connecting planets between the two charts that have aspects.

  if (!gs.fEquator)
    DrawAspectRelation(1, 2, xplanet1, xplanet2, cx, cy, 0.40);

  // Draw sidebar with chart information and positions if need be.

  DrawSidebar();
}


// Draw a tri-wheel chart or quad-wheel chart, in which there are three or
// four rings, among three or four sets of chart data being compared. This
// chart is obtained when the -r3 or -r4 switch is combined with -X switch.

void XChartWheelMulti()
{
  real xsign[cSign+1], xhouse1[cSign+1], xplanet1[objMax], xplanet2[objMax],
    xplanet3[objMax], xplanet4[objMax], xplanet5[objMax], xplanet6[objMax],
    symbol[objMax], ri2, rp, rl1, rl2, rg, rT;
  int cx, cy, i, fQuad, fQuin, fHexa, nRing;
  real unitx, unity, base, base2, off;

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
  fHexa = (us.nRel == rcHexaWheel);
  fQuin = fHexa || (us.nRel == rcQuinWheel);
  fQuad = fQuin || (us.nRel == rcQuadWheel);
  nRing = 3 + fQuad + fQuin + fHexa;
  base = (fHexa ? 0.11 : (fQuin ? 0.22 : (fQuad ? 0.23 : 0.36)));
  base2 = base + (fQuin ? 0.01 : 0.02);
  off = fQuin ? 0.11 : 0.13;

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
  for (i = 0; i <= is.nObj; i++) {
    xplanet1[i] = PZ(HousePlaceInX(cp1.obj[i], cp1.alt[i]));
    xplanet2[i] = PZ(HousePlaceInX(cp2.obj[i], cp2.alt[i]));
    xplanet3[i] = PZ(HousePlaceInX(cp3.obj[i], cp3.alt[i]));
    if (fQuad) {
      xplanet4[i] = PZ(HousePlaceInX(cp4.obj[i], cp4.alt[i]));
      if (fQuin) {
        xplanet5[i] = PZ(HousePlaceInX(cp5.obj[i], cp5.alt[i]));
        if (fHexa)
          xplanet6[i] = PZ(HousePlaceInX(cp6.obj[i], cp6.alt[i]));
      }
    }
  }

  // Go draw the outer sign and house rings. We are drawing the houses of only
  // the outermost ring of the wheel, however.

  if (gs.fColor) {
    DrawColor(kDkGreenB);
    rT = fQuin ? 0.64 : 0.61;
    DrawCircle(cx, cy, (int)(unitx*rT+rRound), (int)(unity*rT+rRound));
    rT -= off;
    DrawCircle(cx, cy, (int)(unitx*rT+rRound), (int)(unity*rT+rRound));
    if (fQuad) {
      rT -= off;
      DrawCircle(cx, cy, (int)(unitx*rT+rRound), (int)(unity*rT+rRound));
      if (fQuin) {
        rT -= off;
        DrawCircle(cx, cy, (int)(unitx*rT+rRound), (int)(unity*rT+rRound));
        if (fHexa)
          rT -= off;
          DrawCircle(cx, cy, (int)(unitx*rT+rRound), (int)(unity*rT+rRound));
      }
    }
  }
  if (fQuin)
    DrawWheel(xsign, xhouse1, cx, cy, unitx, unity, 0.76, 0.82, 0.86);
  else
    DrawWheel(xsign, xhouse1, cx, cy, unitx, unity, 0.745, 0.815, 0.84);

  // Draw the outer ring of planets (i.e. the one the house cusps reflect).
  // Draw each glyph, a line from it to its actual position point in the outer
  // ring, and then draw another line from this point to a another dot at the
  // same position on the innermost ring as well.

  if (fQuin) {
    ri2 = 0.62; rp = 0.65; rl1 = 0.66; rl2 = 0.68; rg = 0.72;
  } else {
    ri2 = 0.59; rp = 0.62; rl1 = 0.63; rl2 = 0.66; rg = 0.70;
  }
  DrawRing(1, nRing, xplanet1, symbol, cx, cy,
    base, base2, ri2, rp, rl1, rl2, rg, 0.9);

  // Now draw the second to outermost ring of planets. Again, draw each glyph,
  // a line to its true point, and a line to the innermost ring.

  ri2 -= off; rp -= off; rl1 -= off; rl2 -= off; rg -= off;
  DrawRing(2, nRing, xplanet2, symbol, cx, cy,
    base, base2, ri2, rp, rl1, rl2, rg, 1.1);

  // The third ring is next. Chart was cast earlier, and draw the glyphs and
  // lines to true point. If a fourth ring is being done, first finish the
  // third one by drawing lines from the true positions to the inner ring.

  ri2 -= off; rp -= off; rl1 -= off; rl2 -= off; rg -= off;
  DrawRing(3, nRing, xplanet3, symbol, cx, cy,
    base, base2, ri2, rp, rl1, rl2, rg, 1.4);

  if (fQuad) {
    // If the fourth ring is being done, take the chart that was cast earlier,
    // and draw glyphs and lines to the true positions. If a fifth ring is
    // being done, first finish the fourth one by drawing lines from the true
    // positions to the inner ring.

    ri2 -= off; rp -= off; rl1 -= off; rl2 -= off; rg -= off;
    DrawRing(4, nRing, xplanet4, symbol, cx, cy,
      base, base2, ri2, rp, rl1, rl2, rg, 1.8);

    if (fQuin) {
      // If the fifth ring is being done, take the chart that was cast
      // earlier, and draw glyphs and lines to the true positions. If a sixth
      // ring is being done, first finish the fifth one by drawing lines from
      // the true positions to the inner ring.

      ri2 -= off; rp -= off; rl1 -= off; rl2 -= off; rg -= off;
      DrawRing(5, nRing, xplanet5, symbol, cx, cy,
        base, base2, ri2, rp, rl1, rl2, rg, 2.3);

      if (fHexa) {
        // If the sixth (innermost) ring is being done, take the chart that was
        // cast earlier, and draw glyphs and lines to the true positions.

        ri2 -= off; rp -= off; rl1 -= off; rl2 -= off; rg -= off;
        DrawRing(6, nRing, xplanet6, symbol, cx, cy,
          base, base2, ri2, rp, rl1, rl2, rg, 3.8);
      }
    }
  }
  FProcessCommandLine(szWheelX[0]);

  // Draw lines connecting planets between the charts that have aspects.

  if (!gs.fEquator) {
    base -= 0.02;
    DrawAspectRelation(1, 2, xplanet1, xplanet2, cx, cy, base);
    DrawAspectRelation(1, 3, xplanet1, xplanet3, cx, cy, base);
    DrawAspectRelation(2, 3, xplanet2, xplanet3, cx, cy, base);
    if (fQuad) {
      DrawAspectRelation(1, 4, xplanet1, xplanet4, cx, cy, base);
      DrawAspectRelation(2, 4, xplanet2, xplanet4, cx, cy, base);
      DrawAspectRelation(3, 4, xplanet3, xplanet4, cx, cy, base);
      if (fQuin) {
        DrawAspectRelation(1, 5, xplanet1, xplanet5, cx, cy, base);
        DrawAspectRelation(2, 5, xplanet2, xplanet5, cx, cy, base);
        DrawAspectRelation(3, 5, xplanet3, xplanet5, cx, cy, base);
        DrawAspectRelation(4, 5, xplanet4, xplanet5, cx, cy, base);
        if (fHexa) {
          DrawAspectRelation(1, 6, xplanet1, xplanet6, cx, cy, base);
          DrawAspectRelation(2, 6, xplanet2, xplanet6, cx, cy, base);
          DrawAspectRelation(3, 6, xplanet3, xplanet6, cx, cy, base);
          DrawAspectRelation(4, 6, xplanet4, xplanet6, cx, cy, base);
          DrawAspectRelation(5, 6, xplanet5, xplanet6, cx, cy, base);
        }
      }
    }
  }

  // Draw sidebar with chart information and positions if need be.

  ciCore = ciMain;
  DrawSidebar();
}


// Draw an aspect (or midpoint) grid in the window, between the planets in two
// different charts, with the planets labeled at the top and side. This chart
// is done when the -g switch is combined with the -r0 and -X switches. Like
// the text version, the chart has a (definable) fixed number of cells.

void XChartGridRelation()
{
  char sz[cchSzDef], szT[cchSzDef];
  int nScale, unit, siz, x, y, i, j, i0, j0, k, l;
  KI c;

  nScale = gi.nScale/gi.nScaleT;
  unit = CELLSIZE*gi.nScale; siz = (gi.nGridCell+1)*unit;
  *szT = chNull;
  i = us.fSmartCusp; us.fSmartCusp = fFalse;
  if (!FCreateGridRelation(gs.fAlt != us.fGridMidpoint))
    return;
  us.fSmartCusp = i;

  // Loop through each cell in each row and column of grid.

  for (y = 0, j0 = -2; y <= gi.nGridCell; y++) {
    do {
      j0++;
      j = rgobjList[j0];
    } while (j0 >= 0 && ignore[j] && j0 <= is.nObj);
    DrawColor(gi.kiGray);
    DrawDash(0, (y+1)*unit, siz, (y+1)*unit, !gs.fColor);
    DrawDash((y+1)*unit, 0, (y+1)*unit, siz, !gs.fColor);
    DrawColor(gi.kiLite);
    DrawEdge(0, y*unit, unit, (y+1)*unit);
    DrawEdge(y*unit, 0, (y+1)*unit, unit);
    DrawEdge(y*unit, y*unit, (y+1)*unit, (y+1)*unit);
    if (j0 <= is.nObj) for (x = 0, i0 = -2; x <= gi.nGridCell; x++) {
      do {
        i0++;
        i = rgobjList[i0];
      } while (i0 >= 0 && ignore[i] && i0 <= is.nObj);

      // Again, are looping through each cell in each row and column.

      if (i0 <= is.nObj) {
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
                *sz = chNull;

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


// Draw a chart showing a graphical ephemeris for the given month, year, or
// range of years, with the date on the vertical axis and the zodiac on the
// horizontal, as done when the -E is combined with the -X switch.

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
      for (i = 0; i <= is.nObj; i++)
        objSav[i] = planet[i];
    ciCore = ciMain;
    if (cYea) {
      MM = mon; DD = day; YY = yea;
    } else
      DD = d;
    CastChart(-1);
    if (us.fParallel)
      for (i = 0; i <= is.nObj; i++) {
        rT = (planetalt[i] * rDegHalf / (real)dx) + rDegHalf;
        rT = Min(rT, rDegMax);
        rT = Max(rT, 0.0);
        planet[i] = rT;
      }

    // Draw planet glyphs along top of chart.
    if (d <= 1) {
      for (i = 0; i <= is.nObj; i++) {
        j = !FProper(i);
        symbol[i*2] = (j || us.nRel > rcDual) ? -rLarge : cp2.obj[i];
        symbol[i*2+1] = (j ? -rLarge : planet[i]);
      }
      FillSymbolLine(symbol);
      fSav = gs.fLabel; gs.fLabel = fTrue;
      for (i = is.nObj*2+1; i >= 0; i--) {
        j = i >> 1;
        if (symbol[i] >= 0.0)
          DrawObject(j, x1 + (int)((real)xs * symbol[i] / rDegMax), unit);
      }
      gs.fLabel = fSav;
      if (us.nRel <= rcDual) {
        for (i = is.nObj; i >= 0; i--) {
          if (!FProper(i))
            continue;
          j = x1 + (int)((real)xs * cp2.obj[i] / rDegMax);
          DrawColor(kObjB[i]);
          DrawDash(j, y1, j, y2, 1);
        }
      }

    // Draw a line segment for each object during this time section.
    } else
      for (i = is.nObj; i >= 0; i--) {
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


// Draw a chart showing a graphical ephemeris of Ray influences for the given
// month or year, with the date on the vertical axis and each Ray on the
// horizontal, as done when the -7 is combined with the -X switch.

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


// Draw a chart graphing transits over time. This covers both transit to
// transit (-B switch) and transit to natal (-V switch), when they're combined
// with the -X switch. Each aspect present during the period has a row,
// showing its strength from 0 (outside of orb) to 100% (exact).

void XChartTransit(flag fTrans, flag fProg)
{
  TransGraInfo *rgEph;
  word **ppw, *pw, *pw2;
  char sz[cchSzDef];
  int cYea, dYea, cAsp, cSect, cTot, ymin, x0, y0, x, y, asp, iw, iwFocus = -1,
    nMax, n, obj, iy, yRow, cRow = 0, xWid, xo, yo, iSect, iFrac, xp, yp, yp2,
    dyp, et;
  flag fMonth = us.fInDayMonth, fYear = us.fInDayYear, fEclipse =
    us.fEclipse && !fTrans && !us.fParallel;
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
  else {
    cYea = Max(us.nEphemYears, 2); cYea = Min(cYea, 21);
    dYea = (cYea - 1) >> 1;
    cSect = cYea*12;
  }
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
      iwFocus = (int)(((real)(dYea*12) + ((real)(ciT.mon-1) +
        ((real)(ciT.day-1) + ciT.tim/24.0) /
        (real)DayInMonth(ciT.mon, ciT.yea))) * (real)xWid);
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
      YY = YY - dYea + iSect/12;
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
    for (y = ymin; y <= is.nObj; y++) {
      if (FIgnore(y) || (!fTrans && !FProperGraph(y)))
        continue;
      for (x = 0; x < (fTrans ? is.nObj+1 : y); x++) {
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
          else if (asp == aOpp && x == oSun && ObjOrbit(y) == us.objCenter)
            et = NCheckEclipseLunar(us.objCenter, y, &rPct);
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
    sprintf(sz, "%s", SzDate(ciT.mon, ciT.day, ciT.yea, fFalse));
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
    for (x = 0; x < cYea; x++) {
      sprintf(sz, "%d", (fTrans || fProg ? YeaT : Yea) - dYea + x);
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
  for (y0 = ymin; y0 <= is.nObj; y0++) {
    y = rgobjList[y0];
    for (x0 = 0; x0 < (fTrans ? is.nObj+1 : y); x0++) {
      x = rgobjList[x0];
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
        DrawLineX(0, xo + iw - 1, yp);
        if (iy <= 1)
          DrawLineX(0, xo + iw - 1, yp - yRow);

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
            DrawLineY(xo + iw, yp - yRow + 1, yp-1);
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
            DrawColor(n >= nMax ||
              ((iw <= 0 || n > pw[iw-1] ||
                (iw > 1 && n == pw[iw-1] && n > pw[iw-2])) &&
              (iw >= cTot-1 || n > pw[iw+1] ||
                (iw < cTot-2 && n == pw[iw+1] && n > pw[iw+2]))) ?
              (dyp >= yRow-2 ? gi.kiOn : gi.kiLite) : kAspB[asp]);
            DrawLineY(xo + iw, yp-1 - dyp, yp-1);

            // Draw eclipse strength overlaying aspect strength, if present.
            if (fEclipse) {
              pw2 = (*rgEph)[y][x][asp];
              if (pw2 != NULL && pw2[iw] > 0) {
                n = pw2[iw];
                n = (n-1) * (yRow-1) / 65535;
                dyp = Min(dyp, n);
                DrawColor(kDkBlueB);
                DrawLineY(xo + iw, yp-1 - dyp, yp-1);
              }
            }
          } // if
        } // iw
      } // asp
    } // x0
  } // y0

LDone:
  if (gs.fBorder) {
    DrawColor(gi.kiOn);
    DrawEdge(0, 0, xo + cTot, gs.yWin-1);
  }

  // Free temporarily allocated data, and restore original chart.
  for (y = ymin; y <= is.nObj; y++)
    for (x = 0; x < (fTrans ? is.nObj+1 : y); x++)
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


// Draw a black or white pixel on the screen, using a simple 1x1, 2x2, or 3x3
// pixel dither pattern. Called from XChartRising() to simulate colors in
// monochrome mode.

void DrawPointDither(Bitmap *b, int x, int y, int k, int kmax)
{
  int m, n;
  flag f;

  if (kmax <= 1) {
    f = (k > 0);
  } else if (kmax == 2) {
    m = x&1; n = y&1;
    f = (k > n*2 + m);
  } else {
    m = x%3; n = y%3;
    f = (k > n*3 + m);
  }
  if (b != NULL)
    BmpSetXY(b, x, y, rgbbmp[f ? gi.kiOn : gi.kiOff]);
  else if (f)
    DrawPoint(x, y);
}


// Draw a chart showing visibility above the horizon for 1-3 planets. This
// is a 2D graph with date on the vertical axis, and time of day along the
// horizontal, with colors showing which planet(s) are above the horizon.

flag XChartRising()
{
  byte ignoreSav[objMax];
  char sz[cchSzDef];
  int obj[3], x1, y1, x2, y2, xs, ys, imax = 0, i, j, x, y, ymax, n,
    z = us.fSeconds ? 1 : 7, dy, dx, xp, yp;
  real rgalt[241][3][2], mc, rT, azi, alt, altL, altR, altT, altB;
  KI ki[8];

  // Determine which objects and colors to use in chart.
  for (i = 0; i <= cObj; i++)
    if (!ignore[i]) {
      obj[imax] = i;
      imax++;
      if (imax >= 3)
        break;
    }
  if (imax <= 0)
    obj[imax++] = oSun;
  ki[0] = kBlackB; ki[1] = kRedB; ki[2] = kGreenB; ki[3] = kYellowB;
  ki[4] = kBlueB; ki[5] = kMagentaB; ki[6] = kCyanB; ki[7] = kWhiteB;
  if (gs.fInverse)
    SwapN(ki[0], ki[7]);

  // Set temporary restriction list for those objects used in this chart.
  ClearB((pbyte)rgalt, sizeof(rgalt));
  CopyRgb(ignore, ignoreSav, sizeof(ignore));
  for (i = 0; i <= cObj; i++)
    ignore[i] = fTrue;
  for (i = 0; i < imax; i++)
    ignore[obj[i]] = fFalse;
  AdjustRestrictions();

  // Determine pixel dimensions of chart.
  ymax = DayInYear(YY);
  x1 = 4 * xFontT; y1 = 6 * gi.nScaleTextT2;
  x2 = gs.xWin - x1;
  y2 = gs.yWin - y1 - gs.fLabel*gi.nScale*(gs.fColor ? 20 : 40);
  xs = x2 - x1 - 2; ys = y2 - y1 - 2;
  xs = (xs / 24) * 24; xs = Max(xs, 24);
  dy = ys / ymax; dy = Max(dy, 1); ys = ymax * dy;
  x2 = x1 + xs + 1; y2 = y1 + ys + 1;
  dx = us.nDivision; dx = Min(dx, xs); dx = Min(dx, 240); dx = Max(dx, 4);
#ifdef WINANY
  if (!gi.fFile && !FAllocateBmp(&gi.bmpRising, xs, ys))
    return fFalse;
#endif

  // Draw object combinations and their colors.
  y = y2 + yFontT + gi.nScale*10;
  x = ((x1 + x2) >> 1) - gi.nScale*5*(imax < 2 ? 0 : (imax == 2 ? 5 : 17));
  for (i = 1; i <= 7; i += (FBetween(i, 2, 3) ? 2 : (i == 4 ? -1 : 1))) {
    if (i >= (1 << imax))
      continue;
    for (n = 0; n < imax; n++) {
      if (i & (1 << n)) {
        j = kObjB[obj[n]]; kObjB[obj[n]] = ki[i];
        DrawObject(obj[n], x, y);
        kObjB[obj[n]] = j;
        if (!gs.fColor) {
          j = i + (imax == 2 && i >= 2) + (imax >= 3 && i >= 4)*2;
          for (yp = y + gi.nScale*10; yp <= y + gi.nScale*20; yp++)
            for (xp = x - gi.nScale*5; xp <= x + gi.nScale*5; xp++)
              DrawPointDither(NULL, xp, yp, j, imax);
        }
        x += gi.nScale*10;
      }
    }
    x += gi.nScale*10;
  }
  if (gs.fInverse)
    SwapN(ki[0], ki[7]);

  // Draw the main graph itself.
  MM = DD = 1;
  for (y = 0; y < ymax; y++) {
    if (z > 1 && y % z == 0)
      for (x = 0; x <= dx; x++)
        for (i = 0; i < imax; i++)
          rgalt[x][i][1] = rgalt[x][i][0];
    if (z <= 1 || y <= 0 || y % z == 1) {
      n = (z <= 1 || y <= 0);
      for (x = 0; x <= dx; x++) {
        TT = (real)x * 24.0 / (real)dx;
        CastChart(-1);
        mc = planet[oMC]; rT = planetalt[oMC];
        EclToEqu(&mc, &rT);
        for (i = 0; i < imax; i++) {
          j = obj[i];
          EclToHoriz(&azi, &alt, planet[j], planetalt[j], mc, AA);
          rgalt[x][i][n] = alt;
        }
      }
      TT = 0.0;
      AddTime(&ciCore, 4, z);
    }
    for (x = 0; x < xs; x++) {
      n = 0;
      for (i = imax-1; i >= 0; i--) {
        rT = (real)x * dx / (real)xs;
        j = (int)rT;
        rT = RFract(rT);
        altL = rgalt[j][i][1]; altR = rgalt[j+1][i][1];
        altT = altL + (altR - altL) * rT;
        if (z <= 1)
          alt = altT;
        else {
          altL = rgalt[j][i][0]; altR = rgalt[j+1][i][0];
          altB = altL + (altR - altL) * rT;
          rT = (real)(y % z) / (real)z;
          rT = RFract(rT);
          alt = altT + (altB - altT) * rT;
        }
        if (!gi.fBmp || !gs.fColor || (gi.fFile && gs.ft > ftBmp)
#ifndef WINANY
          || !gi.fFile
#endif
          )
          n = (n << 1) | (alt >= 0.0);
        else
          n = (n << 8) | (alt >= 0.0 ? 192+(int)(alt/rDegQuad*63.99) :
            64+(int)(alt/rDegQuad*64.0));
      }
      xp = x; yp = y*dy;

      // Draw the current pixels within the graph.
      // This chart doesn't really work as a PS, Metafile, or Wireframe.
      if (gi.fFile && gs.ft > ftBmp)
        continue;
      if (gs.fColor) {
        for (i = 0; i < dy; i++) {
          if (!gi.fFile) {
#ifdef WINANY
            BmpSetXY(&gi.bmpRising, xp, yp + i, !gi.fBmp ? rgbbmp[ki[n]] : n);
#else
            DrawColor(ki[n]);
            DrawPoint(x1+1 + xp, y1+1 + yp + i);
#endif
          } else if (gs.ft == ftBmp)
            SetXY(x1+1 + xp, y1+1 + yp + i, !gi.fBmp ? ki[n] : n);
          else {
            DrawColor(ki[n]);
            DrawPoint(x1+1 + xp, y1+1 + yp + i);
          }
        }
      } else {
        n += (imax == 2 && n >= 2) + (imax >= 3 && n >= 4)*2;
        for (i = 0; i < dy; i++) {
#ifdef WINANY
          if (!gi.fFile)
            DrawPointDither(&gi.bmpRising, xp, yp + i, n, imax);
          else
#endif
          DrawPointDither(NULL, x1+1 + xp, y1+1 + yp + i, n, imax);
        }
      }
    }
  }
#ifdef WINANY
  if (!gi.fFile)
    BmpCopyWin(&gi.bmpRising, wi.hdc, x1+1, y1+1);
#endif

  // Label vertical (month) axis.
  y = y1;
  for (i = 1; i <= cSign; i++) {
    sprintf(sz, "%.3s", szMonth[i]);
    DrawColor(gi.kiOn);
    DrawSz(sz,      xFontT/2, y, dtLeft | dtTop | dtScale2);
    DrawSz(sz, x2 + xFontT/2, y, dtLeft | dtTop | dtScale2);
    if (gs.fLabelCity && i > 1) {
      DrawColor(gi.kiGray);
      DrawLineX(x1, x2, y);
    }
    if (gs.fAlt && i == Mon) {
      x = x1 + (int)((real)xs * Tim / 24.0);
      n = y + Day*dy;
      DrawColor(gi.kiGray);
      DrawLineY(x, y1, y2);
      DrawLineX(x1, x2, n);
    }
    y += DayInMonth(i, Yea)*dy;
  }

  // Label horizontal (time) axis.
  n = 1 + xFontT * (4-us.fEuroTime) / (xs / 24);
  for (i = 0; i < 24; i++) {
    x = x1 + NMultDiv(xs, i, 24);
    if (i % n == 0) {
      if (!us.fEuroTime)
        sprintf(sz, "%d%c", ((i+11) % 12)+1, i < 12 ? 'a' : 'p');
      else
        sprintf(sz, "%d", i);
      DrawColor(gi.kiOn);
      DrawSz(sz, x, y1 - 3*gi.nScaleTextT,
        dtLeft | dtBottom | dtScale2);
      DrawSz(sz, x, y2 + 3*gi.nScaleTextT,
        dtLeft | dtTop | dtScale2);
    }
    if (gs.fLabelCity && i > 0) {
      DrawColor(gi.kiGray);
      DrawLineY(x, y1, y2);
    }
  }
  DrawColor(gi.kiOn);
  DrawEdge(x1, y1, x2, y2);

  // Restore original chart.
  CopyRgb(ignoreSav, ignore, sizeof(ignore));
  AdjustRestrictions();
  ciCore = ciMain;
  CastChart(1);
  return fTrue;
}


#ifdef BIORHYTHM
// Draw a graphic biorhythm chart on the screen, as is done when the -rb
// switch is combined with -X. This is technically a relationship chart in
// that biorhythm status is determined by a natal chart time at another later
// time. For the day in question, and for two weeks before and after, the
// Physical, Emotional, and Mental percentages are plotted.

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
  for (i = 1; i <= 3 + gs.fAlt; i++) {
    jd = RFloor(is.JD + rRound);
    switch (i) {
    case 1: r = brPhy; c = "PHYS"; j = eFir; break;
    case 2: r = brEmo; c = "EMOT"; j = eWat; break;
    case 3: r = brInt; c = "INTE"; j = eEar; break;
    case 4:            c = "AVER"; j = eAir; break;
    }
    DrawColor(kElemB[j]);
    for (jd -= (real)us.nBioday, j = -us.nBioday; j <= us.nBioday;
      j++, jd += 1.0) {
      if (i <= 3)
        a = RBiorhythm(jd, r);
      else
        a = (RBiorhythm(jd, brPhy) + RBiorhythm(jd, brEmo) +
          RBiorhythm(jd, brInt)) / 3.0;
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
#endif // BIORHYTHM
#endif // GRAPH

/* xcharts2.cpp */
