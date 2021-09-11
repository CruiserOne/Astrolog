/*
** Astrolog (Version 7.30) File: xcharts1.cpp
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
** Single Chart Graphics Routines.
******************************************************************************
*/

// Draw a wheel chart, in which the 12 signs and houses are delineated, and
// the planets are inserted in their proper places. This is the default
// graphics chart to generate, as is done when the -v or -w (or no) switches
// are included with -X. Draw the aspects in the middle of chart, too.

void XChartWheel()
{
  real xsign[cSign+1], xhouse[cSign+1], xplanet[objMax], symbol[objMax];
  int cx, cy, i, j;
  real unitx, unity;

  // Set up variables and temporarily automatically decrease the horizontal
  // chart size to leave room for the sidebar if that mode is in effect.

  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;
  cx = gs.xWin/2 - 1; cy = gs.yWin/2 - 1;
  unitx = (real)cx; unity = (real)cy;
  gi.rAsc = gs.objLeft ? planet[NAbs(gs.objLeft)-1] +
    rDegQuad*(gs.objLeft < 0) : chouse[1];
  if (us.fVedic)
    gi.rAsc = gs.objLeft ? (gs.objLeft < 0 ? 120.0 : -60.0)-gi.rAsc : 0.0;

  // Fill out arrays with the angular degree on the circle of where to
  // place each object, cusp, and sign glyph based on the chart mode.

  if (gi.nMode == gWheel) {
    for (i = 1; i <= cSign; i++)
      xhouse[i] = PZ(chouse[i]);
  } else {
    gi.rAsc -= chouse[1];
    for (i = 1; i <= cSign; i++)
      xhouse[i] = PZ(ZFromS(i));
  }
  for (i = 1; i <= cSign; i++)
    xsign[i] = PZ(HousePlaceInX(ZFromS(i), 0.0));
  for (i = 0; i <= is.nObj; i++)
    xplanet[i] = PZ(HousePlaceInX(planet[i], planetalt[i]));

  // Go draw the outer sign and house rings.

  DrawWheel(xsign, xhouse, cx, cy, unitx, unity, 0.65, 0.75, 0.80);

  // For each planet, draw a small dot indicating where it is, and then a
  // line from that point to the planet's glyph.

  DrawRing(1, 1, xplanet, symbol, cx, cy, 0.0, 0.0, 0.0,
    0.50, 0.52, 0.56, 0.60, 1.0);
  FProcessCommandLine(szWheelX[0]);

  // Draw lines connecting planets which have aspects between them.

  if (!gs.fEquator) {            // Don't draw aspects in equator mode.
    if (!FCreateGrid(fFalse))
      return;
    for (j = is.nObj; j >= 1; j--)
      for (i = j-1; i >= 0; i--)
        if (grid->n[i][j] && FProper(i) && FProper(j))
          DrawAspectLine(i, j, cx, cy, xplanet[i], xplanet[j], unitx, unity,
            0.48);
  }

  // Go draw sidebar with chart information and positions if need be.

  DrawSidebar();
}


// Draw an astro-graph chart on a map of the world, i.e. the draw the
// Ascendant, Descendant, Midheaven, and Nadir lines corresponding to the
// time in the chart. This chart is done when the -L switch is combined
// with the -X switch.

void XChartAstroGraph()
{
  real planet1[objMax], planet2[objMax],
    end1[cObj*2+2], end2[cObj*2+2],
    symbol1[cObj*2+2], symbol2[cObj*2+2],
    lon = Lon, longm, x, y, z, ad, oa, am, od, dm, lat;
  int unit = gi.nScale, fVector, lat1 = -60, lat2 = 75, y1, y2, xold1, xold2,
    xmid, i, j, k, l;

  // Erase top and bottom parts of map. We don't draw the astro-graph lines
  // above certain latitudes, and this gives us room for glyph labels, too.

  y1 = (90-lat1)*gi.nScale;
  y2 = (90-lat2)*gi.nScale;
  DrawColor(gi.kiOff);
  DrawBlock(0, 1, gs.xWin-1, y2-1);
  DrawBlock(0, y1+1, gs.xWin-1, gs.yWin-2);
  DrawColor(gi.kiLite);
  i = gs.yWin/2;
  if (gs.fEquator)
    DrawDash(0, i, gs.xWin-2, i, 4);    // Draw equator.
  DrawColor(gi.kiOn);
  DrawLine(1, y2, gs.xWin-2, y2);
  DrawLine(1, y1, gs.xWin-2, y1);
  for (i = 0; i <= is.nObj*2+1; i++)
    end1[i] = end2[i] = -rLarge;

  // Draw small hatches every 5 degrees along edges of world map.

  DrawColor(gi.kiLite);
  for (i = lat1+5; i < lat2; i += 5) {
    j = (90-i)*gi.nScale;
    k = (2+(i%10 == 0)+2*(i%30 == 0))*gi.nScaleT;
    DrawLine(1, j, k, j);
    DrawLine(gs.xWin-2, j, gs.xWin-1-k, j);
  }
  for (i = -nDegHalf+5; i < nDegHalf; i += 5) {
    j = (nDegHalf-i)*gi.nScale;
    k = (2+(i%10 == 0)+2*(i%30 == 0)+(i%90 == 0))*gi.nScaleT;
    DrawLine(j, y2+1, j, y2+k);
    DrawLine(j, y1-1, j, y1-k);
  }
  if (us.fLatitudeCross) {
    DrawColor(kPurpleB);
    i = (int)((rDegQuad - Lat)*(real)gi.nScale);
    DrawLine(0, i, gs.xWin-1, i);
  }

  // Calculate zenith locations of each planet.

  for (i = 0; i <= is.nObj; i++) if (!ignore[i]) {
    planet1[i] = Tropical(planet[i]);
    planet2[i] = planetalt[i];
    EclToEqu(&planet1[i], &planet2[i]);
  }

  // Draw the Midheaven lines and zenith location markings.

  if (lon < 0.0)
    lon += rDegMax;
  for (i = 0; i <= is.nObj; i++) if (FProper(i)) {
    x = is.lonMC - planet1[i];
    if (x < 0.0)
      x += rDegMax;
    if (x > rDegHalf)
      x -= rDegMax;
    z = lon + x;
    if (z > rDegHalf)
      z -= rDegMax;
    j = (int)(Mod(rDegHalf-z+gs.rRot)*(real)gi.nScale);
    if (!ignorez[arMC]) {
      DrawColor(kElemB[eEar]);
      DrawLine(j, y1+unit*4, j, y2-unit*1);
    }
    end2[i*2] = (real)j;
    y = planet2[i];
    k = (int)((rDegQuad-y)*(real)gi.nScale);
    if (FBetween((int)y, lat1, lat2) && !ignorez[arMC]) {
      DrawColor(gi.kiLite);
      DrawBlock(j-gi.nScaleT, k-gi.nScaleT, j+gi.nScaleT, k+gi.nScaleT);
      DrawColor(gi.kiOff);
      DrawBlock(j, k, j, k);
    }

    // Draw Nadir lines assuming we aren't in bonus chart mode.

    if (!gs.fAlt && !ignorez[arIC]) {
      j += 180*gi.nScale;
      if (j > gs.xWin-2)
        j -= (gs.xWin-2);
      end1[i*2] = (real)j;
      DrawColor(kElemB[eWat]);
      DrawLine(j, y1+unit*2, j, y2-unit*2);
    }
  }

  // Now, normally (unless are in bonus chart mode) will go on to draw the
  // the Ascendant and Descendant lines here.

  longm = Mod(is.lonMC + lon);
  if (!gs.fAlt && (!ignorez[arAsc] || !ignorez[arDes]))
  for (i = 1; i <= is.nObj; i++) if (FProper(i)) {
    xold1 = xold2 = nNegative;

    // Hack: Normally the Ascendant and Descendant line segments are drawn
    // simultaneously. However, for the PostScript and metafile vector
    // graphics, that would cause the file to get inordinately large due
    // to the constant thrashing between the Asc and Desc colors. Hence
    // for these chart formats only, do two passes for Asc and Desc.
    fVector = (gs.ft == ftPS || gs.ft == ftWmf);
    for (l = 0; l <= fVector; l++)

    for (lat = (real)lat1; lat <= (real)lat2;
      lat += 1.0/(real)(gi.nScale/gi.nScaleT)) {

      // First compute and draw the current segment of Ascendant line.

      j = (int)((rDegQuad-lat)*(real)gi.nScale);
      ad = RTanD(planet2[i])*RTanD(lat);
      if (ad*ad > 1.0)
        ad = rLarge;
      else {
        ad = RAsin(ad);
        oa = planet1[i] - DFromR(ad);
        if (oa < 0.0)
          oa += rDegMax;
        am = oa - rDegQuad;
        if (am < 0.0)
          am += rDegMax;
        z = longm-am;
        if (z < 0.0)
          z += rDegMax;
        if (z > rDegHalf)
          z -= rDegMax;
        k = (int)(Mod(rDegHalf-z+gs.rRot)*(real)gi.nScale);
        if (!fVector || !l) {
          if (!ignorez[arAsc]) {
            DrawColor(kElemB[eFir]);
            DrawWrap(xold1, j+gi.nScaleT, k, j, 1, gs.xWin-2);
          }
          // Line segment pointing to Ascendant.
          if (lat == (real)lat1) {
            if (!ignorez[arAsc])
              DrawLine(k, y1, k, y1+unit*4);
            end2[i*2+1] = (real)k;
          }
        } else if (lat == (real)lat1)
          end2[i*2+1] = (real)k;
        xold1 = k;
      }

      // The curving Ascendant and Descendant lines actually touch at low or
      // high latitudes. Sometimes when loop starts out, a particular planet's
      // lines haven't appeared yet, i.e. are scanning at a latitude at which
      // the planet's lines don't exist. If this is the case, then when they
      // finally do start, draw a thin horizontal line connecting the
      // Ascendant and Descendant lines so they don't just start in space.
      // Note that these connected lines aren't labeled with glyphs.

      if (ad == rLarge) {
        if (xold1 >= 0) {
          if ((!fVector || !l) && !ignorez[arAsc] && !ignorez[arDes]) {
            xmid = (xold1+xold2)/2;
            if (NAbs(xold2-xold1) > (gs.xWin >> 1)) {
              xmid += (gs.xWin >> 1);
              if (xmid >= gs.xWin)
                xmid -= gs.xWin;
            }
            DrawColor(kElemB[eFir]);
            DrawWrap(xold1, j+1, xmid, j+1, 1, gs.xWin-2);
            DrawColor(kElemB[eAir]);
            DrawWrap(xmid, j+1, xold2, j+1, 1, gs.xWin-2);
          }
          lat = rDegQuad;
        }
      } else {

        // Then compute and draw corresponding segment of Descendant line.

        od = planet1[i] + DFromR(ad);
        dm = od + rDegQuad;
        z = longm-dm;
        if (z < 0.0)
          z += rDegMax;
        if (z > rDegHalf)
          z -= rDegMax;
        k = (int)(Mod(rDegHalf-z+gs.rRot)*(real)gi.nScale);
        if (xold2 < 0 && lat > (real)lat1 && (!fVector || l) &&
          !ignorez[arDes]) {
          xmid = (xold1+k)/2;
          if (NAbs(k-xold1) > (gs.xWin >> 1)) {
            xmid += (gs.xWin >> 1);
            if (xmid >= gs.xWin)
              xmid -= gs.xWin;
          }
          DrawColor(kElemB[eFir]);
          DrawWrap(xold1, j, xmid, j, 1, gs.xWin-2);
          DrawColor(kElemB[eAir]);
          DrawWrap(xmid, j, k, j, 1, gs.xWin-2);
        }
        if ((!fVector || l) && !ignorez[arDes]) {
          DrawColor(kElemB[eAir]);
          DrawWrap(xold2, j+gi.nScaleT, k, j, 1, gs.xWin-2);
          // Line segment pointing to Descendant.
          if (lat == (real)lat1)
            DrawLine(k, y1, k, y1+unit*2);
        }
        xold2 = k;
      }
    }

    // Draw segments pointing to top of Ascendant and Descendant lines.

    if (ad != rLarge) {
      if (!ignorez[arAsc]) {
        DrawColor(kElemB[eFir]);
        DrawLine(xold1, y2, xold1, y2-unit*1);
      }
      if (!ignorez[arDes]) {
        DrawColor(kElemB[eAir]);
        DrawLine(k, y2, k, y2-unit*2);
      }
      end1[i*2+1] = (real)k;
    }
  }

  // Plot chart location.

  DrawColor(kMagentaB);
  i = (int)(Mod(rDegHalf - Lon + gs.rRot)*(real)gi.nScale);
  j = (int)((rDegQuad - Lat)*(real)gi.nScale);
  if (us.fLatitudeCross)
    DrawSpot(i, j);
  else
    DrawPoint(i, j);

  // Determine where to draw the planet glyphs. There are four sets of each
  // planet (each planet's glyph appearing in the chart up to four times) one
  // for each type of line. The Midheaven and Ascendant lines are always
  // labeled at the bottom of the chart, while the Nadir and Descendant lines
  // at the top. Therefore need to place two sets of glyphs, twice.

  for (i = 0; i <= is.nObj*2+1; i++) {
    symbol1[i] = end1[i];
    symbol2[i] = end2[i];
  }
  FillSymbolLine(symbol1);
  FillSymbolLine(symbol2);

  // Now actually draw the planet glyphs.

  for (i = 0; i <= is.nObj*2+1; i++) {
    j = i >> 1;
    if (FProper(j)) {
      if ((gi.xTurtle = (int)symbol1[i]) > 0 && gs.fLabel &&
        !ignorez[FOdd(i) ? arDes : arIC]) {
        DrawColor(ret[j] < 0.0 ? gi.kiGray : gi.kiOn);
        DrawDash((int)end1[i], y2-unit*2, (int)symbol1[i], y2-unit*4,
          (ret[i] < 0.0 ? 1 : 0) - gs.fColor);
        DrawObject(j, gi.xTurtle, y2-unit*10);
      }
      if ((gi.xTurtle = (int)symbol2[i]) > 0 && gs.fLabel &&
        !ignorez[FOdd(i) ? arAsc : arMC]) {
        DrawColor(ret[j] < 0.0 ? gi.kiGray : gi.kiOn);
        DrawDash((int)end2[i], y1+unit*4, (int)symbol2[i], y1+unit*8,
          (ret[i] < 0.0 ? 1 : 0) - gs.fColor);
        DrawObject(j, gi.xTurtle, y1+unit*14);
        k = FOdd(i) ? oAsc : oMC;
        l = kObjB[k]; kObjB[k] = kObjB[j];
        DrawObject(k, (int)symbol2[i], y1+unit*24-gi.nScaleT);
        kObjB[k] = l;
      }
    }
  }
}


// Draw an aspect and midpoint grid in the window, with planets labeled down
// the diagonal. This chart is done when the -g switch is combined with the
// -X switch. The chart always has a certain number of cells, hence based on
// how the restrictions are set up, there may be blank columns and rows, or
// else only the first number of unrestricted objects will be included.

void XChartGrid()
{
  char sz[cchSzDef], szT[cchSzDef];
  int nScale, unit, siz, x, y, i, j, k, l, i0, j0, ig, jg;
  KI c;

  nScale = gi.nScale/gi.nScaleT;
  unit = CELLSIZE*gi.nScale; siz = gi.nGridCell*unit;
  *szT = chNull;
  i = us.fSmartCusp; us.fSmartCusp = fFalse;
  if (!FCreateGrid(gs.fAlt))
    return;
  us.fSmartCusp = i;

  // Loop through each cell in each row and column of grid.

  for (y = 1, j0 = -1; y <= gi.nGridCell; y++) {
    do {
      j0++;
      j = rgobjList[j0];
    } while (!FProper(j) && j0 <= is.nObj);
    DrawColor(gi.kiGray);
    DrawDash(0, y*unit, siz, y*unit, !gs.fColor);
    DrawDash(y*unit, 0, y*unit, siz, !gs.fColor);
    if (j0 <= is.nObj) for (x = 1, i0 = -1; x <= gi.nGridCell; x++) {
      do {
        i0++;
        i = rgobjList[i0];
      } while (!FProper(i) && i0 <= is.nObj);
      ig = i; jg = j;
      if ((i > j) != (i0 > j0))
        SwapN(ig, jg);
      if (i0 <= is.nObj) {
        gi.xTurtle = x*unit-unit/2;
        gi.yTurtle = y*unit-unit/2 - (nScale > 2 ? 5*gi.nScaleT : 0);
        k = grid->n[ig][jg];

        // If this is an aspect cell, draw glyph of aspect in effect.
        if (gs.fAlt ? x > y : x < y) {
          if (k) {
            DrawColor(c = kAspB[k]);
            DrawAspect(k, gi.xTurtle, gi.yTurtle);
          }

        // If this is a midpoint cell, draw glyph of sign of midpoint.
        } else if (gs.fAlt ? x < y : x > y) {
          DrawColor(c = kSignB(grid->n[ig][jg]));
          DrawSign(grid->n[ig][jg], gi.xTurtle, gi.yTurtle);

        // For cells on main diagonal, draw glyph of planet.
        } else {
          if (gs.fLabelAsp) {
            DrawColor(kDkBlueB);
            DrawBlock((x-1)*unit+1, (y-1)*unit+1, x*unit-1, y*unit-1);
          }
          DrawColor(gi.kiLite);
          DrawEdge((x-1)*unit, (y-1)*unit, x*unit, y*unit);
          DrawObject(i, gi.xTurtle, gi.yTurtle);
        }

        // When the scale size is 300+, can print text in each cell.
        if (nScale > 2 && gs.fLabel) {
          l = NAbs(grid->v[ig][jg]); k = l / 60; l %= 60;
          if (nScale > 3 && is.fSeconds)
            sprintf(szT, "%s%02d", x == y ? "'" : "", l);

          // For the aspect portion, print the orb in degrees and minutes.
          if (gs.fAlt ? x > y : x < y) {
            if (grid->n[ig][jg]) {
              sprintf(sz, "%c%d%c%02d'%s", grid->v[ig][jg] < 0 ?
                (us.fAppSep ? 'a' : '-') : (us.fAppSep ? 's' : '+'),
                k/60, chDeg2, k%60, szT);
              if (nScale == 3)
                sz[7] = chNull;
            } else
              *sz = chNull;

          // For the midpoint portion, print the degrees and minutes.
          } else if (gs.fAlt ? x < y : x > y)
            sprintf(sz, "%2d%c%02d'%s", k/60, chDeg2, k%60, szT);

          // For the main diagonal, print degree and sign of each planet.
          else {
            c = kSignB(grid->n[ig][jg]);
            sprintf(sz, "%.3s %02d%s", szSignName[grid->n[ig][jg]], k, szT);
          }
          DrawColor(c);
          DrawSz(sz, x*unit-unit/2, y*unit-3*gi.nScaleT, dtBottom);
        }
      }
    }
  }
}


// Translate zodiac position (or other type of coordinates) into chart pixel
// coordinates representing local horizon position, for the rectangular
// -Z -X switch chart.

void PlotHorizon(real lon, real lat, int x1, int y1, int xs, int ys,
  int *xp, int *yp)
{
  lat = rDegQuad - lat;
  *xp = x1 + (int)((real)xs*lon/rDegMax + rRound);
  *yp = y1 + (int)((real)ys*lat/rDegHalf + rRound);
}

void LocToHorizon(real lon, real lat, int x1, int y1, int xs, int ys,
  int *xp, int *yp)
{
  if (!gs.fEcliptic) {
    lon = Mod(rDegQuad - lon);
    PlotHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
  } else {
    lon = rDegMax - lon;
    CoorXform(&lon, &lat, is.latMC - rDegQuad);
    lon = Mod(is.lonMC - lon + rDegQuad);
    EquToHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
  }
}

void EquToHorizon(real lon, real lat, int x1, int y1, int xs, int ys,
  int *xp, int *yp)
{
  if (!gs.fEcliptic) {
    lon = Mod(is.lonMC - lon + rDegQuad);
    EquToLocal(&lon, &lat, rDegQuad - is.latMC);
    lon = rDegMax - lon;
    LocToHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
  } else {
    EquToEcl(&lon, &lat);
    lon = Mod(Untropical(lon));
    EclToHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
  }
}

void EclToHorizon(real lon, real lat, int x1, int y1, int xs, int ys,
  int *xp, int *yp)
{
  if (!gs.fEcliptic) {
    lon = Tropical(lon);
    EclToEqu(&lon, &lat);
    EquToHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
  } else
    PlotHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
}

void PriToHorizon(real lon, real lat, int x1, int y1, int xs, int ys,
  int *xp, int *yp)
{
  lon = rDegMax - lon;
  CoorXform(&lon, &lat, rDegQuad);
  LocToHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
}

void EarToHorizon(real lon, real lat, int x1, int y1, int xs, int ys,
  int *xp, int *yp)
{
  lon = Mod(lon + rDegHalf);
  CoorXform(&lon, &lat, rDegQuad - Lat);
  LocToHorizon(Mod(lon - rDegHalf), lat, x1, y1, xs, ys, xp, yp);
}


// Draw the local horizon, and draw in the planets where they are at the time
// in question, as done when the -Z is combined with the -X switch.

void XChartHorizon()
{
  int cx, cy, unit, x1, y1, x2, y2, xs, ys, xp, yp, i, j, k;
  real rT;
  ObjDraw rgod[objMax];
  char sz[cchSzDef];
  flag fHouse3D = !us.fHouse3D, fFlip = gs.fEcliptic && us.rHarmonic < 0.0;
#ifdef CONSTEL
  int m1, n1, m2, n2, xpT, ypT;
#endif
#ifdef SWISS
  ES es, *pes1, *pes2;
  int xp2, yp2;
#endif

  unit = Max(12, 6*gi.nScale);
  unit = Max(unit, yFontT);
  x1 = unit; y1 = unit; x2 = gs.xWin-1-unit; y2 = gs.yWin-1-unit;
  xs = x2-x1; ys = y2-y1; cx = (x1+x2)/2; cy = (y1+y2)/2;

  // Calculate the local horizon coordinates of each planet. First convert
  // zodiac position and declination to zenith longitude and latitude.

  is.latMC = Lat;
  ClearB((pbyte)rgod, sizeof(rgod));
  for (i = 0; i <= is.nObj; i++) if (FProper(i)) {
    EclToHorizon(planet[i], planetalt[i], x1, y1, xs, ys,
      &rgod[i].x, &rgod[i].y);
    rgod[i].obj = i;
    rgod[i].kv = ~0;
    rgod[i].f = fTrue;
  }

  // Draw planet disks (which become visible if large enough).
  if (!gs.fAlt)
    for (i = 0; i <= is.nObj; i++) if (FProper(i) && i != us.objCenter) {
      rT = RObjDiam(i);
      if (rT <= 0.0)
        continue;
      rT = RAtnD((rT / 2.0) / (PtLen(space[i]) * rAUToKm));
      j = (int)(rT * (real)xs / rDegMax);
      k = (int)(rT * (real)ys / rDegHalf);
      if (j > 1 || k > 1) {
        DrawColor(kDkGreenB);
        DrawCircle2(rgod[i].x, rgod[i].y, j, k);
      }
    }

  // Draw Earth's equator.
  if (gs.fEquator) {
    DrawColor(kPurpleB);
    for (i = 0; i <= nDegMax; i++) {
      EquToHorizon((real)i, 0.0, x1, y1, xs, ys, &xp, &yp);
      DrawPoint(xp, yp);
    }
  }

#ifdef CONSTEL
  // Draw constellations.
  if (gs.fConstel) {
    EnumConstelLines(NULL, NULL, NULL, NULL, NULL);
    while (EnumConstelLines(&m1, &n1, &m2, &n2, &i)) {
      EquToHorizon((real)(nDegMax-m1), (real)(90-n1), x1, y1, xs, ys,
        &xp, &yp);
      if (i <= 0) {
        DrawColor(kPurpleB);
        EquToHorizon((real)(nDegMax-m2), (real)(90-n2), x1, y1, xs, ys,
          &xpT, &ypT);
        DrawWrap(xp, yp, xpT, ypT, x1, x2);
      } else {
        DrawColor(gi.kiGray);
        DrawSz(szCnstlAbbrev[i], xp, yp, dtCent | dtScale2);
      }
    }
  }
#endif

  // Draw zodiac sign boundary wedges.
  if (us.fVedic) {
    if (!gs.fColorSign)
      DrawColor(kDkBlueB);
    for (i = 0; i < nDegMax; i++) {
      if (gs.fColorSign && i%30 == 0)
        DrawColor(kSignB(i/30 + 1));
      EclToHorizon((real)i, 0.0, x1, y1, xs, ys, &xp, &yp);
      DrawPoint(xp, yp);
    }
    for (i = 0; i < nDegMax; i += 30) {
      if (gs.fColorSign)
        DrawColor(kSignB(i/30 + 1));
      for (j = -90; j <= 90; j++) {
        EclToHorizon((real)i, (real)j, x1, y1, xs, ys, &xp, &yp);
        DrawPoint(xp, yp);
      }
    }
    k = gi.nScale;
    gi.nScale = gi.nScaleTextT;
    for (j = -80; j <= 80; j += 160)
      for (i = 1; i <= cSign; i++) {
        EclToHorizon((real)(i-1)*30.0+15.0, (real)j, x1, y1, xs, ys,
          &xp, &yp);
        if (gs.fColorSign)
          DrawColor(kSignB(i));
        DrawSign(!fFlip ? i : cSign+1 - i, xp, yp);
      }
    gi.nScale = k;
  }

  // Draw house boundary wedges.
  if (gs.fHouseExtra) {
    if (!gs.fColorHouse)
      DrawColor(kDkGreenB);
    if (fHouse3D) {
      for (j = 1; j <= cSign; j++) {
        if (!gs.fEcliptic) {
          if ((j == sCap && FSameR(chouse[j], is.MC)) ||
              (j == sCan && FSameR(chouse[j], Mod(is.MC  + rDegHalf))))
            continue;
          if (us.nHouse3D == hmPrime &&
             ((j == sAri && FSameR(chouse[j], is.Asc)) ||
              (j == sLib && FSameR(chouse[j], Mod(is.Asc + rDegHalf)))))
            continue;
          if (us.nHouse3D == hmHorizon &&
             ((j == (Lat >= 0.0 ? sAri : sLib) &&
                FSameR(chouse[j], Mod(is.Vtx + rDegHalf))) ||
              (j == (Lat >= 0.0 ? sLib : sAri) && FSameR(chouse[j], is.Vtx))))
            continue;
        }
        if (gs.fColorHouse)
          DrawColor(kSignB(j));
        rT = chouse3[j];
        if (us.nHouse3D == hmHorizon)
          rT = (rT + rDegQuad) * (Lat < 0.0 ? -1.0 : 1.0) - rDegQuad;
        for (i = -89; i < 90; i++) {
          if (us.nHouse3D == hmPrime)
            PriToHorizon(rT, i, x1, y1, xs, ys, &xp, &yp);
          else if (us.nHouse3D == hmHorizon)
            LocToHorizon(rT, i, x1, y1, xs, ys, &xp, &yp);
          else
            EarToHorizon(rT, i, x1, y1, xs, ys, &xp, &yp);
          DrawPoint(xp, yp);
        }
      }
      for (i = 1; i <= cSign; i++) {
        rT = Midpoint(chouse3[i], chouse3[Mod12(i+1)]);
        if (us.nHouse3D == hmPrime)
          PriToHorizon(rT, 0.0, x1, y1, xs, ys, &xp, &yp);
        else if (us.nHouse3D == hmHorizon) {
          rT = (rT + rDegQuad) * (Lat < 0.0 ? -1.0 : 1.0) - rDegQuad;
          LocToHorizon(rT, 0.0, x1, y1, xs, ys, &xp, &yp);
        } else
          EarToHorizon(rT, 0.0, x1, y1, xs, ys, &xp, &yp);
        if (gs.fColorHouse)
          DrawColor(kSignB(i));
        DrawHouse(i, xp, yp);
      }
    } else {
      for (i = 1; i <= cSign; i++) {
        if (gs.fColorHouse)
          DrawColor(kSignB(SFromZ(chouse[i])));
        for (j = -90; j <= 90; j++) {
          EclToHorizon(chouse[i], (real)j, x1, y1, xs, ys,
            &xp, &yp);
          DrawPoint(xp, yp);
        }
      }
      for (j = -75; j <= 75; j += 150)
        for (i = 1; i <= cSign; i++) {
          EclToHorizon(Midpoint(chouse[i], chouse[Mod12(i+1)]), (real)j,
            x1, y1, xs, ys, &xp, &yp);
          if (gs.fColorHouse)
            DrawColor(kSignB(i));
          DrawHouse(i, xp, yp);
        }
    }
  }

  // Draw vertical lines dividing our rectangle into four areas. In our
  // local space chart, the middle line represents due south, the left line
  // due east, the right line due west, and the edges due north. A fourth
  // horizontal line divides that which is above and below the horizon.

  if (gs.fHouseExtra && fHouse3D && !gs.fEcliptic) {
    DrawColor(gs.fColorHouse ? kSignB(sCap) : kDkGreenB);
    DrawDash(cx, y1, cx, cy, 1);
    DrawColor(gs.fColorHouse ? kSignB(sCan) : kDkGreenB);
    DrawDash(cx, cy, cx, y2, 1);
  }
  if (!(us.fVedic && gs.fEcliptic)) {
    DrawColor(gi.kiGray);
    if (!(gs.fHouseExtra && fHouse3D && !gs.fEcliptic))
      DrawDash(cx, y1, cx, y2, 1);
    DrawDash((cx+x1)/2, y1, (cx+x1)/2, y2, 1);
    DrawDash((cx+x2)/2, y1, (cx+x2)/2, y2, 1);
  }
  DrawColor(gi.kiOn);
  DrawEdge(x1, y1, x2, y2);
  if (!(us.fVedic && gs.fEcliptic)) {
    if (gs.fHouseExtra && fHouse3D) {
      if (gs.fColorHouse)
        DrawColor(kSignB(sAri));
      DrawDash(x1, cy, cx, cy, 1);
      if (gs.fColorHouse)
        DrawColor(kSignB(sLib));
      DrawDash(cx, cy, x2, cy, 1);
    } else
      DrawDash(x1, cy, x2, cy, 1);
  }

  // Make a slightly smaller rectangle within the window to draw the planets
  // in. Make segments on all four edges marking 5 degree increments.

  DrawColor(gi.kiLite);
  for (i = 5; i < 180; i += 5) {
    j = y1+(int)((real)i*(real)ys/rDegHalf);
    k = (2+(i%10 == 0)+2*(i%30 == 0))*gi.nScaleT;
    DrawLine(x1+1, j, x1+1+k, j);
    DrawLine(x2-1, j, x2-1-k, j);
  }
  for (i = 0; i <= nDegMax; i += 5) {
    j = x1+(int)((real)i*(real)xs/rDegMax);
    if (i > 0 && i < nDegMax) {
      k = (2+(i%10 == 0)+2*(i%30 == 0))*gi.nScaleT;
      DrawLine(j, y1+1, j, y1+1+k);
      DrawLine(j, y2-1, j, y2-1-k);
    }
    if (i % 90 == 0) {
      k = !fFlip ? i : nDegMax-i;
      if (!gs.fEcliptic)
        sprintf(sz, "%c", *szDir[k/90 & 3]);
      else if (us.nDegForm == 0)
        sprintf(sz, "%3.3s", szSignName[Mod12((k / 90)*3 + 1)]);
      else if (us.nDegForm == 1)
        sprintf(sz, "%dh", k/15);
      else
        sprintf(sz, "%d", k);
      DrawSz(sz, j, y1-2*gi.nScaleT, dtBottom | dtScale2);
    }
  }

#ifdef SWISS
  // Draw extra stars.
  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      EclToHorizon(es.lon, es.lat, x1, y1, xs, ys, &xp, &yp);
      DrawStar(xp, yp, &es);
    }

    // Draw constellation lines between stars.
    DrawColor(gi.kiLite);
    EnumStarsLines(fTrue, NULL, NULL);
    while (EnumStarsLines(fFalse, &pes1, &pes2)) {
      EclToHorizon(pes1->lon, pes1->lat, x1, y1, xs, ys, &xp, &yp);
      EclToHorizon(pes2->lon, pes2->lat, x1, y1, xs, ys, &xp2, &yp2);
      DrawWrap(xp, yp, xp2, yp2, x1, x2);
    }
  }

  // Draw extra asteroids.
  if (gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fTrue);
    while (SwissComputeAsteroid(is.T, &es, fTrue)) {
      EclToHorizon(es.lon, es.lat, x1, y1, xs, ys, &xp, &yp);
      DrawStar(xp, yp, &es);
    }
  }
#endif

  // Draw planet glyphs, and spots for actual planet locations.
  DrawObjects(rgod, is.nObj+1, 0);
}


// Translate zodiac position (or other type of coordinates) into chart pixel
// coordinates representing local horizon position, for the circular -Z0 -X
// switch chart.

void PlotHorizonSky(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp)
{
  real s, x, y, rx, ry;

  rx = (real)pcr->xr; ry = (real)pcr->yr;
  s = (rDegQuad-lat)/rDegQuad;
  if (s > 1.0) {
    x = rx * (rSqr2 - 1.0);
    y = ry * (rSqr2 - 1.0);
    if (lon < 45.0 || lon >= rDegMax-45.0 ||
      FBetween(lon, rDegHalf-45.0, rDegHalf+45.0))
      s = 1.0 + (s - 1.0) * (((rx + x)/RAbs(RCosD(lon))-rx) / rx);
    else if (lon < 135.0 || lon >= rDegMax-135.0)
      s = 1.0 + (s - 1.0) * (((ry + y)/RAbs(RCosD(lon-90.0))-ry) / ry);
  }
  *xp = pcr->xc + (int)(rx*s*RCosD(rDegHalf+lon)+rRound);
  *yp = pcr->yc + (int)(ry*s*RSinD(rDegHalf+lon)+rRound);
}

void LocToHorizonSky(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp)
{
  if (!gs.fEcliptic)
    PlotHorizonSky(lon, lat, pcr, xp, yp);
  else {
    lon = rDegMax - lon;
    CoorXform(&lon, &lat, is.latMC - rDegQuad);
    lon = Mod(is.lonMC - lon + rDegQuad);
    EquToHorizonSky(lon, lat, pcr, xp, yp);
  }
}

void EquToHorizonSky(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp)
{
  if (!gs.fEcliptic) {
    lon = Mod(is.lonMC - lon + rDegQuad);
    EquToLocal(&lon, &lat, rDegQuad - is.latMC);
    lon = rDegMax - lon;
    LocToHorizonSky(lon, lat, pcr, xp, yp);
  } else {
    EquToEcl(&lon, &lat);
    lon = Mod(Untropical(lon));
    EclToHorizonSky(lon, lat, pcr, xp, yp);
  }
}

void EclToHorizonSky(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp)
{
  if (!gs.fEcliptic) {
    lon = Tropical(lon);
    EclToEqu(&lon, &lat);
    EquToHorizonSky(lon, lat, pcr, xp, yp);
  } else
    PlotHorizonSky(lon, lat, pcr, xp, yp);
}

void PriToHorizonSky(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp)
{
  lon = rDegMax - lon;
  CoorXform(&lon, &lat, rDegQuad);
  LocToHorizonSky(lon, lat, pcr, xp, yp);
}

void EarToHorizonSky(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp)
{
  lon = Mod(lon + rDegHalf);
  CoorXform(&lon, &lat, rDegQuad - Lat);
  LocToHorizonSky(Mod(lon - rDegHalf), lat, pcr, xp, yp);
}


// Draw the local horizon, and draw in the planets where they are at the time
// time in question. This chart is done when the -Z0 is combined with the
// -X switch. This is an identical function to XChartHorizon(), however that
// routine's chart is centered on the horizon and meridian. Here we center the
// chart around the center of the sky straight up from the local horizon, with
// the horizon itself being an encompassing circle.

void XChartHorizonSky()
{
  int cx, cy, rx, ry, unit, x1, y1, x2, y2, xs, ys, xp, yp, i, j, k;
  real s, rT;
  CIRC cr;
  ObjDraw rgod[objMax];
  flag fHouse3D = !us.fHouse3D;
#ifdef CONSTEL
  int m1, n1, m2, n2, xpT, ypT;
#endif
#ifdef SWISS
  ES es, *pes1, *pes2;
  int xp2, yp2;
#endif

  unit = Max(12, 6*gi.nScale);
  unit = Max(unit, yFontT);
  x1 = unit; y1 = unit; x2 = gs.xWin-1-unit; y2 = gs.yWin-1-unit;
  xs = x2-x1; ys = y2-y1; cx = (x1+x2)/2; cy = (y1+y2)/2;
  rx = (int)((real)xs/2.0/rSqr2); ry = (int)((real)ys/2.0/rSqr2);
  cr.xc = cx; cr.yc = cy; cr.xr = rx; cr.yr = ry;

  // Calculate the local horizon coordinates of each planet. First convert
  // zodiac position and declination to zenith longitude and latitude.

  is.latMC = Lat;
  ClearB((pbyte)rgod, sizeof(rgod));
  for (i = 0; i <= is.nObj; i++) if (FProper(i)) {
    EclToHorizonSky(planet[i], planetalt[i], &cr, &rgod[i].x, &rgod[i].y);
    rgod[i].obj = i;
    rgod[i].kv = ~0;
    rgod[i].f = fTrue;
  }

  // Draw planet disks (which become visible if large enough).
  if (!gs.fAlt)
    for (i = 0; i <= is.nObj; i++) if (FProper(i) && i != us.objCenter) {
      rT = RObjDiam(i);
      if (rT <= 0.0)
        continue;
      rT = RAtnD((rT / 2.0) / (PtLen(space[i]) * rAUToKm));
      j = (int)(rT * (real)rx * 2.0 * rPi / rDegMax);
      k = (int)(rT * (real)ry * 2.0 * rPi / rDegMax);
      if (j > 1 || k > 1) {
        DrawColor(kDkGreenB);
        DrawCircle2(rgod[i].x, rgod[i].y, j, k);
      }
    }

  // Draw Earth's equator.
  if (gs.fEquator) {
    DrawColor(kPurpleB);
    for (i = 0; i <= nDegMax; i++) {
      EquToHorizonSky((real)i, 0.0, &cr, &xp, &yp);
      DrawPoint(xp, yp);
    }
  }

#ifdef CONSTEL
  // Draw constellations.
  if (gs.fConstel) {
    EnumConstelLines(NULL, NULL, NULL, NULL, NULL);
    while (EnumConstelLines(&m1, &n1, &m2, &n2, &i)) {
      EquToHorizonSky((real)(nDegMax-m1), (real)(90-n1), &cr, &xp, &yp);
      if (i <= 0) {
        DrawColor(kPurpleB);
        EquToHorizonSky((real)(nDegMax-m2), (real)(90-n2), &cr, &xpT, &ypT);
        if (NAbs(xpT - xp) + NAbs(ypT - yp) < (xs+ys) >> 4)
          DrawLine(xp, yp, xpT, ypT);
      } else {
        DrawColor(gi.kiGray);
        DrawSz(szCnstlAbbrev[i], xp, yp, dtCent | dtScale2);
      }
    }
  }
#endif

  // Draw zodiac sign boundary wedges.
  if (us.fVedic) {
    if (!gs.fColorSign)
      DrawColor(kDkBlueB);
    for (i = 0; i < nDegMax; i++) {
      if (gs.fColorSign && i%30 == 0)
        DrawColor(kSignB(i/30 + 1));
      EclToHorizonSky((real)i, 0.0, &cr, &xp, &yp);
      DrawPoint(xp, yp);
    }
    for (i = 0; i < nDegMax; i += 30) {
      if (gs.fColorSign)
        DrawColor(kSignB(i/30 + 1));
      for (j = -90; j <= 90; j++) {
        EclToHorizonSky((real)i, (real)j, &cr, &xp, &yp);
        DrawPoint(xp, yp);
      }
    }
    k = gi.nScale;
    gi.nScale = gi.nScaleTextT;
    for (j = -80; j <= 80; j += 160)
      for (i = 1; i <= cSign; i++) {
        EclToHorizonSky((real)(i-1)*30.0+15.0, (real)j, &cr, &xp, &yp);
        if (gs.fColorSign)
          DrawColor(kSignB(i));
        DrawSign(i, xp, yp);
      }
    gi.nScale = k;
  }

  // Draw house boundary wedges.
  if (gs.fHouseExtra) {
    if (!gs.fColorHouse)
      DrawColor(kDkGreenB);
    if (fHouse3D) {
      for (j = 1; j <= cSign; j++) {
        if (!gs.fEcliptic) {
          if ((j == sCap && FSameR(chouse[j], is.MC)) ||
              (j == sCan && FSameR(chouse[j], Mod(is.MC  + rDegHalf))))
            continue;
          if (us.nHouse3D == hmPrime &&
             ((j == sAri && FSameR(chouse[j], is.Asc)) ||
              (j == sLib && FSameR(chouse[j], Mod(is.Asc + rDegHalf)))))
            continue;
          if (us.nHouse3D == hmHorizon &&
             ((j == (Lat >= 0.0 ? sAri : sLib) &&
                FSameR(chouse[j], Mod(is.Vtx + rDegHalf))) ||
              (j == (Lat >= 0.0 ? sLib : sAri) && FSameR(chouse[j], is.Vtx))))
            continue;
        }
        if (gs.fColorHouse)
          DrawColor(kSignB(j));
        rT = chouse3[j];
        if (us.nHouse3D == hmHorizon)
          rT = (rT + rDegQuad) * (Lat < 0.0 ? -1.0 : 1.0) - rDegQuad;
        for (i = -89; i < 90; i++) {
          if (us.nHouse3D == hmPrime)
            PriToHorizonSky(rT, i, &cr, &xp, &yp);
          else if (us.nHouse3D == hmHorizon)
            LocToHorizonSky(rT, i, &cr, &xp, &yp);
          else
            EarToHorizonSky(rT, i, &cr, &xp, &yp);
          DrawPoint(xp, yp);
        }
      }
      for (i = 1; i <= cSign; i++) {
        rT = Midpoint(chouse3[i], chouse3[Mod12(i+1)]);
        if (us.nHouse3D == hmPrime)
          PriToHorizonSky(rT, 0.0, &cr, &xp, &yp);
        else if (us.nHouse3D == hmHorizon) {
          rT = (rT + rDegQuad) * (Lat < 0.0 ? -1.0 : 1.0) - rDegQuad;
          LocToHorizonSky(rT, 0.0, &cr, &xp, &yp);
        } else
          EarToHorizonSky(rT, 0.0, &cr, &xp, &yp);
        if (gs.fColorHouse)
          DrawColor(kSignB(i));
        DrawHouse(i, xp, yp);
      }
    } else {
      for (i = 1; i <= cSign; i++) {
        if (gs.fColorHouse)
          DrawColor(kSignB(SFromZ(chouse[i])));
        for (j = -90; j <= 90; j++) {
          EclToHorizonSky(chouse[i], (real)j, &cr, &xp, &yp);
          DrawPoint(xp, yp);
        }
      }
      for (j = -75; j <= 75; j += 150)
        for (i = 1; i <= cSign; i++) {
          EclToHorizonSky(Midpoint(chouse[i], chouse[Mod12(i+1)]), (real)j,
            &cr, &xp, &yp);
          if (gs.fColorHouse)
            DrawColor(kSignB(i));
          DrawHouse(i, xp, yp);
        }
    }
  }

  // Draw a circle in window to indicate horizon line, lines dividing the
  // window into quadrants to indicate n/s and w/e meridians, and segments
  // on these lines and the edges marking 5 degree increments.

  DrawColor(gi.kiGray);
  DrawDash(cx, y1, cx, y2, 1);
  DrawDash(x1, cy, x2, cy, 1);
  DrawColor(gi.kiLite);
  for (i = -125; i <= 125; i += 5) {
    k = (2+(i/10*10 == i ? 1 : 0)+(i/30*30 == i ? 2 : 0))*gi.nScaleT;
    s = 1.0/(rDegQuad*rSqr2);
    j = cy + (int)(s*ys/2*i);
    DrawLine(cx-k, j, cx+k, j);
    j = cx + (int)(s*xs/2*i);
    DrawLine(j, cy-k, j, cy+k);
  }
  for (i = 5; i < 55; i += 5) {
    k = (2+(i/10*10 == i ? 1 : 0)+(i/30*30 == i ? 2 : 0))*gi.nScaleT;
    s = 1.0/(rDegHalf-rDegQuad*rSqr2);
    j = (int)(s*ys/2*i);
    DrawLine(x1, y1+j, x1+k, y1+j);
    DrawLine(x1, y2-j, x1+k, y2-j);
    DrawLine(x2, y1+j, x2-k, y1+j);
    DrawLine(x2, y2-j, x2-k, y2-j);
    j = (int)(s*xs/2*i);
    DrawLine(x1+j, y1, x1+j, y1+k);
    DrawLine(x2-j, y1, x2-j, y1+k);
    DrawLine(x1+j, y2, x1+j, y2-k);
    DrawLine(x2-j, y2, x2-j, y2-k);
  }
  if (!gs.fEcliptic) {
    DrawSz("N", cx, y1-2*gi.nScaleT, dtBottom | dtScale2);
    DrawSz("E", x1/2, cy+2*gi.nScaleT, dtCent | dtScale2);
    DrawSz("W", (gs.xWin+x2)/2, cy+2*gi.nScaleT, dtCent | dtScale2);
    if (!gs.fText)
      DrawSz("S", cx, gs.yWin-3*gi.nScaleT, dtBottom | dtScale2);
  }
  DrawColor(gi.kiOn);
  DrawEdge(x1, y1, x2, y2);
  DrawCircle(cx, cy, rx, ry);
  for (i = 0; i < nDegMax; i += 5) {
    k = (2+(i/10*10 == i ? 1 : 0)+(i/30*30 == i ? 2 : 0))*gi.nScaleT;
    DrawLine(cx+(int)((rx-k)*RCosD((real)i)), cy+(int)((ry-k)*RSinD((real)i)),
      cx+(int)((rx+k)*RCosD((real)i)), cy+(int)((ry+k)*RSinD((real)i)));
  }

#ifdef SWISS
  // Draw extra stars.
  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      EclToHorizonSky(es.lon, es.lat, &cr, &xp, &yp);
      DrawStar(xp, yp, &es);
    }

    // Draw constellation lines between stars.
    DrawColor(gi.kiLite);
    EnumStarsLines(fTrue, NULL, NULL);
    while (EnumStarsLines(fFalse, &pes1, &pes2)) {
      EclToHorizonSky(pes1->lon, pes1->lat, &cr, &xp, &yp);
      EclToHorizonSky(pes2->lon, pes2->lat, &cr, &xp2, &yp2);
      DrawLine(xp, yp, xp2, yp2);
    }
  }

  // Draw extra asteroids.
  if (gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fTrue);
    while (SwissComputeAsteroid(is.T, &es, fTrue)) {
      EclToHorizonSky(es.lon, es.lat, &cr, &xp, &yp);
      DrawStar(xp, yp, &es);
    }
  }
#endif

  // Draw planet glyphs, and spots for actual planet locations.
  DrawObjects(rgod, is.nObj+1, 0);
}


// Translate zodiac position or local horizon position into chart pixel
// coordinates, given local horizon, equatorial, or ecliptic coordinates,
// for the -XZ switch telescope chart.

void PlotTelescope(real lon, real lat, TELE *pte,
  int *xp, int *yp, real *xr, real *yr)
{
  pte->lon = lon; pte->lat = lat;
  *xr = (real)pte->xCent - (MinDifference(lon, pte->xBase) * pte->xScale);
  *yr = (real)pte->yCent - ((lat - pte->yBase) * pte->yScale);
  *xp = (int)(*xr + rRound);
  *yp = (int)(*yr + rRound);
}

void LocToTelescope(real lon, real lat, TELE *pte,
  int *xp, int *yp, real *xr, real *yr)
{
  if (!gs.fEcliptic) {
    lon = Mod(rDegQuad - lon);
    PlotTelescope(lon, lat, pte, xp, yp, xr, yr);
  } else {
    lon = rDegMax - lon;
    CoorXform(&lon, &lat, is.latMC - rDegQuad);
    lon = Mod(is.lonMC - lon + rDegQuad);
    EquToTelescope(lon, lat, pte, xp, yp, xr, yr);
  }
}

void EquToTelescope(real lon, real lat, TELE *pte,
  int *xp, int *yp, real *xr, real *yr)
{
  if (!gs.fEcliptic) {
    lon = Mod(is.lonMC - lon + rDegQuad);
    EquToLocal(&lon, &lat, rDegQuad - is.latMC);
    lon = rDegMax - lon;
    LocToTelescope(lon, lat, pte, xp, yp, xr, yr);
  } else {
    EquToEcl(&lon, &lat);
    lon = Mod(Untropical(lon));
    EclToTelescope(lon, lat, pte, xp, yp, xr, yr);
  }
}

void EclToTelescope(real lon, real lat, TELE *pte,
  int *xp, int *yp, real *xr, real *yr)
{
  if (!gs.fEcliptic) {
    lon = Tropical(lon);
    EclToEqu(&lon, &lat);
    EquToTelescope(lon, lat, pte, xp, yp, xr, yr);
  } else
    PlotTelescope(lon, lat, pte, xp, yp, xr, yr);
}

void PriToTelescope(real lon, real lat, TELE *pte,
  int *xp, int *yp, real *xr, real *yr)
{
  lon = rDegMax - lon;
  CoorXform(&lon, &lat, rDegQuad);
  LocToTelescope(lon, lat, pte, xp, yp, xr, yr);
}

void EarToTelescope(real lon, real lat, TELE *pte,
  int *xp, int *yp, real *xr, real *yr)
{
  CoorXform(&lon, &lat, rDegQuad - Lat);
  LocToTelescope(lon, -lat, pte, xp, yp, xr, yr);
}


// Draw a chart depicting a telescope view of the sky, showing planetary disks
// (including moon phases), as done with the -XZ switch. This is similar to a
// zoomed in version of the -Z -X switch local horizon chart.

void XChartTelescope()
{
  char sz[cchSzDef], szT[cchSzDef], *pch;
  int rgobj[objMax+1], xunit, yunit, x1, y1, x2, y2, xs, ys, xc, yc, xp, yp,
    xT, yT, xd, yd, i, j, k, i0, i1, i2, iEar, iMoo, dx, dy = 0, nEclipse,
    zLimit, nSav;
  real rglen[objMax+1], lonH, latH, xBase, yBase, xScale, yScale,
    xBase2, yBase2, xScale2, yScale2, xi, yi, diam, radi, rRatio, len, ang,
    xo, yo, xr, yr, zr, xrSun, yrSun, xrT, yrT,
    radiS, radiE, radiM, radiU, radiP, lenS, lenM, theta, rPct;
  flag fFlip = gs.fEcliptic && us.rHarmonic < 0.0,
    fShowMinute, fShowLabel, fShowUmbra, fSav;
  TELE te;
#ifdef CONSTEL
  int m1, n1, m2, n2;
#endif
#ifdef SWISS
  ES es, *pes1, *pes2;
#endif
  // Variables for Saturn's rings
  real radi2, len2, ang2, theta2, dRing, xr2, yr2, radi3;
  int iSat, iUra, xT2, yT2, xd2, yd2, xp2, yp2, xT3, yT3, xd3, yd3;
  PT3R ptSat, ptCen, vS2C, vCross, vBest, vUp, vLeft;

  // Initialize variables.
  is.latMC = Lat;
  iSat = !ignore[oSaC] ? oSaC :
#ifdef SWISS
    (!ignore[oVul] && rgTypSwiss[oVul - custLo] == 3 &&
    rgObjSwiss[oVul - custLo] == 699 ? oVul : oSat);
#else
    oSat;
#endif
  iUra = !ignore[oUrC] ? oUrC :
#ifdef SWISS
    (!ignore[oVul] && rgTypSwiss[oVul - custLo] == 3 &&
    rgObjSwiss[oVul - custLo] == 799 ? oVul : oUra);
#else
    oUra;
#endif
  iEar = oEar; iMoo = oMoo; fShowUmbra = fFalse;
  if (us.objCenter == oEar) {
    fShowUmbra = fTrue;
  } else if (ObjOrbit(gs.objTrack) == us.objCenter) {
    iEar = us.objCenter; iMoo = gs.objTrack; fShowUmbra = fTrue;
  }

  for (fShowMinute = 0; fShowMinute <= 1; fShowMinute++) {
    xunit = xFontT*(!fShowMinute ? 4 : 7);
    yunit = 12*gi.nScaleTextT;
    x1 = xunit; y1 = yunit; x2 = gs.xWin-1-xunit; y2 = gs.yWin-1-yunit;
    xs = x2-x1; ys = y2-y1; xc = (x1+x2)/2; yc = (y1+y2)/2;

    // Determine point to center chart around.
    if (gs.objTrack >= 0) {
      xBase = planet[gs.objTrack], yBase = planetalt[gs.objTrack];
      if (!gs.fEcliptic) {
        xBase = Tropical(xBase);
        EclToEqu(&xBase, &yBase);
        xBase = Mod(is.lonMC - xBase + rDegQuad);
        EquToLocal(&xBase, &yBase, rDegQuad - is.latMC);
        xBase = rDegMax - xBase;
        xBase = Mod(rDegQuad - xBase);
      }
    } else {
      xBase = gs.rRot; yBase = gs.rTilt;
    }

    // Determine degrees the chart area covers.
    len = (real)Min(xs, ys);
    yScale = gs.rspace > 0.0 ? len / Min(gs.rspace, 90.0) :
      len / (real)(1 << (4-gi.nScale/gi.nScaleT));
    len = (real)ys/yScale/2.0;
    if (yBase - len < -89.0)
      yBase = len - 89.0;
    else if (yBase + len > 89.0)
      yBase = 89.0 - len;
    if (len > 89.0 || !FBetween(yBase, -89.9, 89.0))
      yBase = 0.0;
    xScale = yScale * RSinD(rDegQuad - yBase);
    xi = (real)xs/xScale; yi = (real)ys/yScale;
    gi.zViewRatio = (xi + yi) / 128.0;
    if (gi.zViewRatio > 1.0)
      gi.zViewRatio = 1.0;
    if (xi >= 2.0 || yi >= 2.0)
      break;
  }

  dx = (int)xScale*10;
  gs.rRot = xBase; gs.rTilt = yBase;
  te.xCent = (real)xc; te.yCent = (real)yc;
  te.xBase = xBase; te.yBase = yBase;
  te.xScale = xScale; te.yScale = yScale;

  // Sort planets in order of distance.
  for (i = 0; i <= cObj+1; i++) {
    rgobj[i] = i;
    xr = space[i].x; yr = space[i].y; zr = space[i].z;
    rglen[i] = RLength3(xr, yr, zr);
  }
  rglen[iEar] = rSmall;
  rglen[cObj+1] = 1/rSmall;
  for (i = 1; i <= cObj+1; i++) {
    j = i-1;
    while (j >= 0 && rglen[rgobj[j]] < rglen[rgobj[j+1]]) {
      SwapN(rgobj[j], rgobj[j+1]);
      j--;
    }
  }
  if (!gs.fLabelAsp) {
    EclToTelescope(planet[oSun], planetalt[oSun], &te, &xp, &yp, &xr, &yr);
    xrSun = te.lon; yrSun = te.lat;
  }

  // Draw grid of degree points.
  xBase2 = xBase; yBase2 = yBase; xScale2 = xScale; yScale2 = yScale;
  if (fShowMinute) {
    xi      *= 60.0; yi      *= 60.0;
    xBase2  *= 60.0; yBase2  *= 60.0;
    xScale2 /= 60.0; yScale2 /= 60.0;
  }
  if (gs.fLabelCity && (fShowMinute || (yScale >= 1.0 && xScale >= 1.0))) {
    DrawColor(gi.kiGray);
    k = !fShowMinute ? 90 : 90*60;
    for (j = (int)(yBase2 - yi); j <= (int)(yBase2 + yi); j++) {
      if (!FBetween(j, -k, k))
        continue;
      yp = yc - (int)(((real)j - yBase2) * yScale2);
      if (!FBetween(yp, y1, y2))
        continue;
      for (i = (int)(xBase2 - xi); i <= (int)(xBase2 + xi); i++) {
        xp = xc + (int)(((real)i - xBase2) * xScale2);
        if (!FBetween(xp, x1, x2))
          continue;
        DrawPoint(xp, yp);
      }
    }
  }

#ifdef CONSTEL
  // Draw constellations.
  if (gs.fConstel) {
    EnumConstelLines(NULL, NULL, NULL, NULL, NULL);
    while (EnumConstelLines(&m1, &n1, &m2, &n2, &i)) {
      EquToTelescope((real)(nDegMax-m1), (real)(90-n1), &te, &xp, &yp,
        &xr, &yr);
      if (i <= 0) {
        DrawColor(kPurpleB);
        EquToTelescope((real)(nDegMax-m2), (real)(90-n2), &te, &xT, &yT,
          &xr, &yr);
        if ((real)NAbs(xT - xp) < xScale*rDegQuad)
          DrawClip(xp, yp, xT, yT, x1, y1, x2, y2, 0);
      } else {
        DrawColor(gi.kiGray);
        DrawSz(szCnstlAbbrev[i], xp, yp, dtCent | dtScale2);
      }
    }
  }
#endif

  // Draw ecliptic and zodiac boundary wedges.
  if (us.fVedic) {
    if (!gs.fColorSign)
      DrawColor(kMaroonB);
    for (i = 0; i <= nDegMax; i++) {
      EclToTelescope((real)i, 0.0, &te, &xp, &yp, &xr, &yr);
      if (i > 0 && NAbs(xp - xT) < dx)
        DrawClip(xT, yT, xp, yp, x1, y1, x2, y2, 0);
      xT = xp; yT = yp;
    }
    for (k = 1; k <= cSign; k++) {
      if (gs.fColorSign)
        DrawColor(kSignB(k));
      len = (real)((k-1)*30);
      for (i = -90; i <= 90; i++) {
        EclToTelescope(len, (real)i, &te, &xp, &yp, &xr, &yr);
        if (i > -90 && NAbs(xp - xT) < dx)
          if (FDrawClip(xT, yT, xp, yp, x1, y1, x2, y2, 0, &xd, &yd))
            DrawSign(k, xd + ((xd == x1)-(xd == x2))*gi.nScale*6,
              yd + ((yd == y1)-(yd == y2))*gi.nScale*8);
        xT = xp; yT = yp;
      }
    }
  }

  // Draw house boundary wedges.
  if (gs.fHouseExtra) {
    if (!gs.fColorHouse)
      DrawColor(kDkGreenB);
    if (!us.fHouse3D) {
      // 3D house boundaries defined by prime vertical or other wedges.
      k = (us.nHouse3D == hmPrime && gs.fEquator && FSameR(chouse[1], is.Asc));
      for (j = 1 + k; j <= cSign; j += (k && j%6 == 0 ? 2 : 1)) {
        if (gs.fColorHouse)
          DrawColor(kSignB(j));
        lonH = chouse3[j];
        if (us.nHouse3D == hmHorizon)
          lonH = (lonH + rDegQuad) * (Lat < 0.0 ? -1.0 : 1.0) - rDegQuad;
        for (i = 0; i <= nDegHalf; i++) {
          latH = (real)i - rDegQuad;
          if (us.nHouse3D == hmPrime)
            PriToTelescope(lonH, latH, &te, &xp, &yp, &xr, &yr);
          else if (us.nHouse3D == hmHorizon)
            LocToTelescope(lonH, latH, &te, &xp, &yp, &xr, &yr);
          else
            EarToTelescope(lonH, latH, &te, &xp, &yp, &xr, &yr);
          if (i > 0 && NAbs(xp - xT) < dx)
            if (FDrawClip(xT, yT, xp, yp, x1, y1, x2, y2, 0, &xd, &yd))
              DrawHouse(j, xd + ((xd == x1)-(xd == x2))*gi.nScale*6,
                yd + ((yd == y1)-(yd == y2))*gi.nScale*6);
          xT = xp; yT = yp;
        }
      }
    } else {
      // Standard 2D house boundaries defined by ecliptic wedges only.
      for (k = 1; k <= cSign; k++) {
        if (gs.fColorHouse)
          DrawColor(kSignB(SFromZ(chouse[k])));
        for (i = -90; i <= 90; i++) {
          EclToTelescope(chouse[k], (real)i, &te, &xp, &yp, &xr, &yr);
          if (i > -90 && NAbs(xp - xT) < dx)
            if (FDrawClip(xT, yT, xp, yp, x1, y1, x2, y2, 0, &xd, &yd))
              DrawHouse(k, xd + ((xd == x1)-(xd == x2))*gi.nScale*6,
                yd + ((yd == y1)-(yd == y2))*gi.nScale*6);
          xT = xp; yT = yp;
        }
      }
    }
  }

  // Draw horizon line.
  if (gs.fEquator) {
    DrawColor(gi.kiLite);
    for (i = 0; i <= nDegMax; i++) {
      LocToTelescope((real)i, 0.0, &te, &xp, &yp, &xr, &yr);
      if (i > 0 && NAbs(xp - xT) < dx)
        DrawClip(xT, yT, xp, yp, x1, y1, x2, y2, 0);
      xT = xp; yT = yp;
    }
  }

  // Draw planet disks.
  for (k = 0; k <= 1; k++) {
    for (i0 = 0; i0 <= cObj+1; i0++) {
      i1 = rgobj[i0];
      i = (i1 <= cObj ? i1 : iEar);
      // Only draw Earth's umbra disk when doing a geocentric chart.
      // Also allow other planet's umbra when focused upon its moon.
      if (!FProper(i) || (!fShowUmbra && (i == us.objCenter || i > cObj)))
        continue;
      xo = planet[i]; yo = planetalt[i];
      radi = diam = RObjDiam(i);
      if (radi > 0.0) {
        if (!(fShowUmbra && i == iEar)) {
          // Calculate object's radius and angular size in sky.
          radi /= 2.0;
          len = PtLen(space[i]) * rAUToKm;
          ang = RAtnD(radi / len);
        } else {
          // Earth object in geocentric chart is Earth's shadow.
          xo = Mod(planet[oSun] + rDegHalf); yo = -planetalt[oSun];

          radiS = rObjDiam[oSun] / 2.0;
          radiE = rObjDiam[iEar] / 2.0;
          radiM = rObjDiam[iMoo] / 2.0;
          lenS = PtLen(space[oSun]);
          lenM = PtLen(space[iMoo]);
          lenS *= rAUToKm; lenM *= rAUToKm;

          theta = RAsinD((radiS - radiE) / lenS);
          radiU = radiE - lenM * RTanD(theta);
          theta = RAsinD((radiE + radiS) / lenS);
          radiP = (lenS + lenM) * RTanD(theta) - radiS;

          // Calculate angular size in sky of Earth's umbra or penumbra.
          ang = RAtnD((i1 == iEar ? radiU : radiP) / lenM);
        }
        radi = ang * yScale;
        j = (int)(radi + rRound);
      } else
        j = 0;
      EclToTelescope(xo, yo, &te, &xp, &yp, &xr, &yr);
      if (!FBetween(xp, x1-j, x2+j) || !FBetween(yp, y1-j, y2+j))
        continue;
      DrawColor(i1 <= cObj ? kObjB[i] : kDkGreenB);
      if (j <= (int)!gs.fAlt) {
        if (k == !us.fLatitudeCross) {
          if (gs.fAlt)
            DrawPoint(xp, yp);
          else
            DrawSpot(xp, yp);
        }
      } else {
        yd = (int)(radi * 2.0); yT = (int)(yr - radi + rRound);
        rRatio = RSinD(rDegQuad - ((((real)yc - yr) / yScale) + yBase)) /
          (xScale / yScale);
        radi /= rRatio;
        xd = (int)(radi * 2.0); xT = (int)(xr - radi + rRound);
        if (!k) {
          if (i == oSun || FIgnore(oSun) || gs.fLabelAsp ||
            (fShowUmbra && i == iEar)) {
            // Sun will never be in shadow, so always draw it filled in.
            // Earth's umbra disk isn't a physical object, so fill it in too.
            DrawEllipse2(xT, yT, xT + xd, yT + yd);
          } else {
            // Determine tilt of crescent Moon (point it toward the Sun).
            SphRatio(te.lon, te.lat, xrSun, yrSun, 0.01, &xrT, &yrT);
            theta = RAngleD(MinDifference(te.lon, xrT), yrT - te.lat);
            // Determine phase of Moon, or how much it's covered in shadow.
            if (!us.fAspect3D) {
              ang = RAngleD(space[oSun].x - space[i].x,
                space[oSun].y - space[i].y);
              len = RAngleD(space[i].x, space[i].y);
              ang = rDegHalf - MinDistance(ang, len);
            } else {
              PT3R v1, v2;
              PtVec(v1, space[oSun], space[i]);
              PtVec(v2, space[iEar], space[i]);
              ang = VAngleD(&v1, &v2);
            }
            len = 1.0 - (RCosD(ang) / 2.0 + 0.5);
            // Display how full the Moon (or other planet) is.
            if (gs.fText && (i == gs.objTrack || ObjOrbit(i) <= oSun)) {
              sprintf(szT, "%%s: %%.%df%%%% Full (area), "
                "%%.%df%%%% Full (time)", us.fSeconds*2, us.fSeconds*2);
              sprintf(sz, szT, szObjDisp[i], 100.0 - (len*100.0),
                100.0 - (ang / rDegHalf * 100.0));
              DrawSz(sz, xc, y2 - gi.nScaleT*10 - dy*yFontT,
                dtCent | dtBottom | dtScale2);
              dy++;
            }

            // Draw surrounding ellipse to indicate extent of Saturn's rings.
            if (i == iSat || i == iUra) {
              if (i == iSat) {
                // Saturn's "A" ring has outer radius of 136K km.
                radi2 = radi * (rSatRingA * 2.0) / diam;
                xd2 = (int)(radi2 * 2.0); xT2 = (int)(xr - radi2 + rRound);
                radi2 *= rRatio;
                // Saturn's "B" ring has inner radius of 92K km.
                radi3 = radi * (rSatRingB * 2.0) / diam;
                xd3 = (int)(radi3 * 2.0); xT3 = (int)(xr - radi3 + rRound);
                radi3 *= rRatio;
                ptSat = space[iSat];
              } else {
                // Uranus' "E" ring has outer radius of 136K km.
                radi2 = radi * (rUraRing * 2.0) / diam;
                xd2 = (int)(radi2 * 2.0); xT2 = (int)(xr - radi2 + rRound);
                radi2 *= rRatio;
                ptSat = space[iUra];
              }
#if FALSE
              // Calculate plane of Uranus' rings based on its moons.
              static PT3R vSum = {0.0, 0.0, 0.0};
              PT3R v1, v2;
              PtVec(v1, ptSat, space[moonsLo+15]);
              len2 = nDegHalf;
              for (i2 = moonsLo+16; i2 <= moonsLo+18; i2++) {
                PtVec(v2, ptSat, space[i2 < moonsLo+18 ? i2 : moonsLo+14]);
                ang = VAngleD(&v1, &v2);
                ang = NAbs(ang - rDegQuad);
                if (ang < len2) {
                  len2 = ang;
                  vBest = v2;
                  if (ang < 85.0)
                    break;
                }
              }
              v2 = vBest;
              PtCross(vCross, v1, v2);
              vCross.x = RAbs(vCross.x);
              vCross.y = RAbs(vCross.y);
              vCross.z = RAbs(vCross.z);
              len2 = PtLen(vCross);
              PtDiv(vCross, len2);
              PtAdd2(vSum, vCross);
              len2 = PtLen(vSum);
              vCross = vSum;
              PtDiv(vCross, len2);
#endif
              if (i == iSat) {
                PtSet(vCross, -0.0833346, -0.4629964, -0.8824339);
              } else {
                PtSet(vCross, 0.2059129, 0.9691102, -0.1357401);
              }
              // Calculate tilt of Saturn's rings up or down.
              ptCen = space[us.objCenter];
              PtVec(vS2C, ptSat, ptCen);
              ang2 = VAngleD(&vCross, &vS2C);
              dRing = (ang2 >= 0.0 ? rDegHalf : 0.0);
              radi2 *= RCosD(ang2);
              if (i == iSat)
                radi3 *= RCosD(ang2);
              // Calculate rotation of Saturn's rings left or right.
              PtSet(vUp, 0.0, 0.0, 1.0);
              PtCross(vLeft, vS2C, vUp);
              PtCross(vBest, vS2C, vCross);
              ang = VAngleD(&vBest, &vUp);
              len2 = VAngleD(&vBest, &vLeft);
              if ((ang >= rDegQuad) == (len2 >= rDegQuad) == fFlip)
                ang = RAbs(ang - rDegQuad);
              else 
                ang = -RAbs(ang - rDegQuad);
              if (!gs.fEcliptic) {
                EclToTelescope(xo - 0.01, yo, &te, &xp2, &yp2, &xr2, &yr2);
                theta2 = RAngleD(xr2 - xr, yr2 - yr);
                ang = theta2 - ang;
              }
              // Draw Saturn's rings.
              yd2 = (int)(radi2 * 2.0); yT2 = (int)(yr - radi2 + rRound);
              DrawColor(kDkGreenB);
              DrawArc(xT2, yT2, xT2 + xd2, yT2 + yd2,
                ang, rDegHalf - dRing, rDegMax - dRing);
              if (i == iSat) {
                yd3 = (int)(radi3 * 2.0); yT3 = (int)(yr - radi3 + rRound);
                DrawArc(xT3, yT3, xT3 + xd3, yT3 + yd3,
                  ang, rDegHalf - dRing, rDegMax - dRing);
              }
              DrawColor(kObjB[i]);
            }
            // Draw planet's disk here.
            DrawCrescent(xT, yT, xT + xd, yT + yd, len, theta + rDegHalf,
              kDkBlueB, gi.kiCur);
            // Draw other half of Saturn's rings on top of planet's disk.
            if (i == iSat || i == iUra) {
              DrawColor(kDkGreenB);
              DrawArc(xT2, yT2, xT2 + xd2, yT2 + yd2,
                ang, dRing, dRing + rDegHalf);
              if (i == iSat)
                DrawArc(xT3, yT3, xT3 + xd3, yT3 + yd3,
                  ang, dRing, dRing + rDegHalf);
              if (gs.fText) {
                sprintf(szT, "%%s's rings: %%.%df%%%c Tilt, "
                  "%%.%df%%%c Rotation", us.fSeconds*2, chDeg2,
                  us.fSeconds*2, chDeg2);
                sprintf(sz, szT, szObjDisp[i], ang2-rDegQuad,
                  ang);
                DrawSz(sz, xc, y2 - gi.nScaleT*10 - dy*yFontT,
                  dtCent | dtBottom | dtScale2);
                dy++;
              }
            }
          }
        } else if (!us.fLatitudeCross)
          DrawEllipse(xT, yT, xT + xd, yT + yd);
      }
      if (k && i1 <= cObj) {
        yp += j + (i < starLo ? 7*gi.nScale : 9*gi.nScaleT);
        if (FBetween(xp, x1, x2) && FBetween(yp, y1, y2))
          DrawObject(i, xp, yp);
        if (gs.fText && us.fEclipse) {
          // Display eclipse information if any visible planets overlapping.
          for (i2 = -1; i2 < i; i2++) {
            nEclipse = etUndefined;
            if (i2 >= 0) {
              if (!FProper(i2) || i == us.objCenter || i2 == us.objCenter ||
                (i == iMoo && i2 == iEar))
                continue;
              fSav = us.fEclipseAny; us.fEclipseAny = fFalse;
              nEclipse = NCheckEclipse(i2, i, &rPct);
              us.fEclipseAny = fSav;
            } else {
              if (!(i == iMoo && us.objCenter == iEar && !FIgnore(iEar)))
                continue;
              nEclipse = NCheckEclipseLunar(iEar, iMoo, &rPct);
            }
            if (nEclipse > etNone) {
              if (i2 >= 0) {
                DrawColor(nEclipse >= etAnnular ? gi.kiOn : gi.kiLite);
                sprintf(szT, "%%s & %%s: %%s %%s%%s %%.%df%%%%",
                  us.fSeconds*2);
                sprintf(sz, szT, szObjDisp[i2], szObjDisp[i],
                  szEclipse[nEclipse], i2 == oSun ? "Solar " : "",
                  i2 == oSun && i == iMoo ? "Eclipse" : "Occultation", rPct);
              } else {
                DrawColor(nEclipse >= etTotal ? gi.kiOn :
                  (nEclipse <= etPenumbra ? gi.kiGray : gi.kiLite));
                sprintf(szT, "%%s & %%s: %%s Lunar Eclipse %%.%df%%%%",
                  us.fSeconds*2);
                sprintf(sz, szT, szObjDisp[i2 < 0 ? iEar : i2], szObjDisp[i],
                  szEclipse[nEclipse], rPct);
              }
              DrawSz(sz, xc, y2 - gi.nScaleT*10 - dy*yFontT,
                dtCent | dtBottom | dtScale2);
              dy++;
            }
          } // i2
        } // if
      } // if
    } // i0
  } // k

#ifdef SWISS
  // Draw extra stars.
  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      EclToTelescope(es.lon, es.lat, &te, &xp, &yp, &xr, &yr);
      if (!FBetween(xp, x1, x2) || !FBetween(yp, y1, y2))
        continue;
      DrawStar(xp, yp, &es);
    }

    // Draw constellation lines between stars.
    DrawColor(gi.kiLite);
    EnumStarsLines(fTrue, NULL, NULL);
    while (EnumStarsLines(fFalse, &pes1, &pes2)) {
      EclToTelescope(pes1->lon, pes1->lat, &te, &xp, &yp, &xr, &yr);
      EclToTelescope(pes2->lon, pes2->lat, &te, &xT, &yT, &xr, &yr);
      if ((!FBetween(xp, x1, x2) && !FBetween(xT, x1, x2)) ||
        (!FBetween(yp, y1, y2) && !FBetween(yT, y1, y2)))
        continue;
      DrawClip(xp, yp, xT, yT, x1, y1, x2, y2, 0);
    }
  }

  // Draw extra asteroids.
  if (gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fTrue);
    while (SwissComputeAsteroid(is.T, &es, fTrue)) {
      EclToTelescope(es.lon, es.lat, &te, &xp, &yp, &xr, &yr);
      if (!FBetween(xp, x1, x2) || !FBetween(yp, y1, y2))
        continue;
      DrawStar(xp, yp, &es);
    }
  }
#endif

  // Label degree points on vertical axis.
  DrawColor(gi.kiLite);
  zLimit = !fShowMinute ? 90 : 90*60;
  k = xFontT*(!fShowMinute ? 4 : 7) >> 1;
  i = (yFont-2)*gi.nScaleTextT / Max((int)yScale2, 1) + 1;
  for (j = (int)(yBase2 - yi); j <= (int)(yBase2 + yi); j++) {
    if (!FBetween(j, -zLimit, zLimit))
      continue;
    yp = yc - (int)(((real)j - yBase2) * yScale2);
    if (!FBetween(yp, y1, y2))
      continue;
    fShowLabel = ((j % i) == 0);
    DrawLineX(x1, x1 + (3+fShowLabel*2)*gi.nScaleT, yp);
    DrawLineX(x2 - (3+fShowLabel*2)*gi.nScaleT, x2, yp);
    if (!fShowLabel)
      continue;
    if (!fShowMinute)
      sprintf(sz, "%s%d", j > 0 ? "+" : "", j);
    else
      sprintf(sz, "%c%d%c%02d", j > 0 ? '+' : '-', NAbs(j)/60, chDeg2,
        NAbs(j)%60);
    DrawSz(sz, k, yp + 2*gi.nScaleTextT, dtCent | dtScale2);
    DrawSz(sz, gs.xWin-gi.nScaleT - k, yp + 2*gi.nScaleTextT,
      dtCent | dtScale2);
  }

  // Label degree points on horizontal axis.
  j = us.nDegForm == 3 ? 8 + fShowMinute*2 :
    ((!fShowMinute ? (us.nDegForm == 0 ? 5 : (us.nDegForm == 1 ? 7 : 4)) :
    (us.nDegForm == 0 ? 7 : (us.nDegForm == 1 ? 11 : 6))));
  j = j*xFontT / Max((int)xScale2, 1) + 1;
  for (i = (int)(xBase2 - xi); i <= (int)(xBase2 + xi); i++) {
    xp = xc + (int)(((real)i - xBase2) * xScale2);
    if (!FBetween(xp, x1, x2))
      continue;
    fShowLabel = ((i % j) == 0);
    DrawLineY(xp, y1, y1 + (3+fShowLabel*2)*gi.nScaleT);
    DrawLineY(xp, y2 - (3+fShowLabel*2)*gi.nScaleT, y2);
    if (!fShowLabel)
      continue;
    fSav = is.fSeconds; nSav = us.nDegForm;
    is.fSeconds = ((us.nDegForm == 1 || us.nDegForm == 3) && fShowMinute);
    us.nDegForm = (gs.fEcliptic ? us.nDegForm : 2);
    ang = (gs.fEcliptic ? (!fShowMinute ? (real)i : (real)i/60.0) :
      (!fShowMinute ? (real)(90-i) : rDegQuad-(real)i/60.0));
    if (fFlip)
      ang = rDegMax - ang;
    ang = Mod(ang + rSmall);
    if (us.nDegForm != 2)
      sprintf(sz, "%s", SzZodiac(ang));
    else {
      us.nDegForm = 0;
      sprintf(sz, "%s", SzDegree(ang));
      us.nDegForm = 2;
    }
    if (us.nDegForm == 0)
      sz[!fShowMinute ? 5 : 7] = chNull;
    else if (us.nDegForm == 2) {
      sz[3] = chDeg2;
      sz[!fShowMinute ? 4 : 6] = chNull;
    } else if (us.nDegForm == 3)
      sz[!fShowMinute ? 7 : 9] = chNull;
    us.nDegForm = nSav; is.fSeconds = fSav;
    pch = sz + (sz[0] == ' ');
    DrawSz(pch, xp, 2*gi.nScaleTextT, dtCent | dtTop | dtScale2);
    if (!gs.fText)
      DrawSz(pch, xp, gs.yWin - gi.nScaleT - 2*gi.nScaleTextT,
        dtCent | dtBottom | dtScale2);
  }

  // Draw boundary of chart area.
  DrawEdge(x1, y1, x2, y2);
}


// Draw a chart depicting a map of the local space area, showing cities and
// compass lines from the chart location to the zenith locations of each
// planet, as done when the -Nl is combined with the -X switch. This is
// similar to a zoomed in version of the rectangular world map chart.

void XChartLocal()
{
  char sz[cchSzDef];
  int xunit, yunit, x1, y1, x2, y2, xs, ys, xc, yc, xp, yp, xp2, yp2,
    m1, n1, m2, n2, i, j, k, zLimit;
  real xBase, yBase, xScale, yScale, xBase2, yBase2, xScale2, yScale2,
    xr, yr, xi, yi, lon, lat, len;
  flag fShowMinute, fShowLabel, fDidBitmap;
  TELE te;
  static real lonPrev = rLarge, latPrev = rLarge;
  static int objPrev = nLarge;

  // Initialize variables.
  for (fShowMinute = 0; fShowMinute <= 1; fShowMinute++) {
    xunit = xFontT*(!fShowMinute ? 4 : 6);
    yunit = 12*gi.nScaleTextT;
    x1 = xunit; y1 = yunit; x2 = gs.xWin-1-xunit; y2 = gs.yWin-1-yunit;
    xs = x2-x1; ys = y2-y1; xc = (x1+x2)/2; yc = (y1+y2)/2;

    // Determine point to center chart around.
    if (objPrev == nLarge)
      objPrev = gs.objTrack;
    if (gs.objTrack >= 0 && gs.objTrack != objPrev) {
      objPrev = -1;
      xBase = planet[gs.objTrack], yBase = planetalt[gs.objTrack];
      EclToEqu(&xBase, &yBase);
      xBase = Mod(xBase - is.lonMC - Lon + rDegHalf);
    } else {
      if (Lon != lonPrev || Lat != latPrev) {
        lonPrev = Lon; latPrev = Lat;
        gs.rRot = Mod(rDegHalf - lonPrev); gs.rTilt = latPrev;
      }
      xBase = gs.rRot; yBase = gs.rTilt;
    }

    // Determine degrees the chart area covers.
    len = (real)Min(xs, ys);
    yScale = gs.rspace > 0.0 ? len / Min(gs.rspace, 90.0) :
      len / (real)(1 << (4-gi.nScale/gi.nScaleT));
    len = (real)ys/yScale/2.0;
    if (yBase - len < -89.0)
      yBase = len - 89.0;
    else if (yBase + len > 89.0)
      yBase = 89.0 - len;
    if (len > 89.0 || !FBetween(yBase, -89.9, 89.0))
      yBase = 0.0;
    xScale = yScale * RSinD(rDegQuad - yBase);
    xi = xs/xScale; yi = ys/yScale;
    gi.zViewRatio = (xi + yi) / 128.0;
    if (gi.zViewRatio > 1.0)
      gi.zViewRatio = 1.0;
    if (xi >= 2.0 || yi >= 2.0)
      break;
  }

  gs.rRot = xBase; gs.rTilt = yBase;
  te.xCent = (real)xc; te.yCent = (real)yc;
  te.xBase = xBase; te.yBase = yBase;
  te.xScale = xScale; te.yScale = yScale;

  fDidBitmap = FBmpDrawMap2(x1, y1, x2, y2,
    xBase - xi/2.0, rDegQuad - yBase - yi/2.0,
    xBase + xi/2.0, rDegQuad - yBase + yi/2.0);

  // Draw grid of degree points.
  xBase2 = xBase; yBase2 = yBase; xScale2 = xScale; yScale2 = yScale;
  if (fShowMinute) {
    xi      *= 60.0; yi      *= 60.0;
    xBase2  *= 60.0; yBase2  *= 60.0;
    xScale2 /= 60.0; yScale2 /= 60.0;
  }
  if (gs.fHouseExtra && (fShowMinute || (yScale2 >= 1.0 && xScale2 >= 1.0))) {
    DrawColor(gi.kiGray);
    k = !fShowMinute ? 90 : 90*60;
    for (j = (int)(yBase2 - yi); j <= (int)(yBase2 + yi); j++) {
      if (!FBetween(j, -k, k))
        continue;
      yp = yc - (int)(((real)j - yBase2) * yScale2 + rRound);
      if (!FBetween(yp, y1, y2))
        continue;
      for (i = (int)(xBase2 - xi); i <= (int)(xBase2 + xi); i++) {
        xp = xc + (int)(((real)i - xBase2) * xScale2 + rRound);
        if (!FBetween(xp, x1, x2))
          continue;
        DrawPoint(xp, yp);
      }
    }
  }

  // Draw equator.
  if (gs.fEquator) {
    yp = yc - (int)(-yBase * yScale + rRound);
    if (FBetween(yp, y1, y2)) {
      DrawColor(gi.kiGray);
      DrawLine(x1, yp, x2, yp);
    }
  }

  // Draw world map.
  if (!fDidBitmap) {
    EnumWorldLines(NULL, NULL, NULL, NULL, NULL);
    while (EnumWorldLines(&m1, &n1, &m2, &n2, &k)) {
      if (k >= 0)
        DrawColor(k <= 0 ? kDkBlueB :
          (gs.fColorSign ? kRainbowB[k] : kLtGrayB));
      xr = MinDifference(xBase, (real)m1);
      if (RAbs(xr) > rDegHalf-2.0)
        continue;
      yr = (real)(90 - n1) - yBase;
      xp = xc + (int)(xr * xScale + rRound);
      yp = yc - (int)(yr * yScale + rRound);
      xr = MinDifference(xBase, (real)m2);
      yr = (real)(90 - n2) - yBase;
      xp2 = xc + (int)(xr * xScale + rRound);
      yp2 = yc - (int)(yr * yScale + rRound);
      DrawClip(xp, yp, xp2, yp2, x1, y1, x2, y2, 0);
    }
  }

#ifdef ATLAS
  // Draw locations of cities from atlas.
  if (FEnsureAtlas()) {
    if (!gs.fLabelAsp)
      DrawColor(kOrangeB);
    KiCity(-1);
    for (i = 0; i < is.cae; i++) {
      lon = Mod(rDegHalf - is.rgae[i].lon);
      lat = is.rgae[i].lat;
      xr = MinDifference(xBase, lon); yr = lat - yBase;
      xp = xc + (int)(xr * xScale + rRound);
      yp = yc - (int)(yr * yScale + rRound);
      if (!FBetween(xp, x1, x2) || !FBetween(yp, y1, y2))
        continue;
      if (gs.fLabelAsp)
        DrawColor(KiCity(i));
      if (gs.fAlt)
        DrawPoint(xp, yp);
      else
        DrawSpot(xp, yp);
      if (!gs.fLabelCity)
        DrawSz(is.rgae[i].szNam, xp, yp + 9*gi.nScaleT, dtCent | dtScale2);
    }
  }
#endif

  // Draw zenith locations of planets.
  for (i = 0; i <= is.nObj; i++) if (!FIgnore(i)) {
    lon = Tropical(planet[i]);
    lat = planetalt[i];
    EclToEqu(&lon, &lat);
    lon = Mod(lon - is.lonMC - Lon + rDegHalf);
    xr = MinDifference(xBase, lon); yr = lat - yBase;
    xp = xc + (int)(xr * xScale + rRound);
    yp = yc - (int)(yr * yScale + rRound);
    DrawColor(kObjB[i]);
    if (FBetween(xp, x1, x2) && FBetween(yp, y1, y2)) {
      if (gs.fAlt)
        DrawPoint(xp, yp);
      else
        DrawSpot(xp, yp);
      DrawObject(i, xp, yp + 9*gi.nScale);
    }

    // Draw line from chart location to planet's zenith location.
    if (us.fVedic)
      continue;
    k = (int)SphDistance(Lon, Lat, lon, lat) + 1;
    for (j = 0; j <= k; j++) {
      SphRatio(rDegHalf - Lon, Lat, lon, lat, (real)j / (real)k, &xr, &yr);
      xr = MinDifference(xBase, xr); yr -= yBase;
      xp = xc + (int)(xr * xScale + rRound);
      yp = yc - (int)(yr * yScale + rRound);
      if (j > 0 && RAbs(xr) < rDegHalf-20.0)
        if (FDrawClip(xp2, yp2, xp, yp, x1, y1, x2, y2, 0, &m1, &n1))
          DrawObject(i, m1 + ((m1 == x1)-(m1 == x2))*gi.nScale*6,
            n1 + ((n1 == y1)-(n1 == y2))*gi.nScale*6);
      xp2 = xp; yp2 = yp;
    }
  }

  // Mark chart location.
  DrawColor(kMagentaB);
  lon = Mod(rDegHalf - Lon); lat = Lat;
  xr = MinDifference(xBase, lon); yr = lat - yBase;
  xp = xc + (int)(xr * xScale + rRound);
  yp = yc - (int)(yr * yScale + rRound);
  if (FBetween(xp, x1, x2) && FBetween(yp, y1, y2))
    DrawSpot(xp, yp);

  // Label degree points on vertical axis.
  DrawColor(gi.kiLite);
  zLimit = !fShowMinute ? 90 : 90*60;
  k = xFontT*(!fShowMinute ? 2 : 3);
  i = (yFont-2)*gi.nScaleTextT / Max((int)yScale2, 1) + 1;
  for (j = (int)(yBase2 - yi); j <= (int)(yBase2 + yi); j++) {
    if (!FBetween(j, -zLimit, zLimit))
      continue;
    yp = yc - (int)(((real)j - yBase2) * yScale2 + rRound);
    if (!FBetween(yp, y1, y2))
      continue;
    fShowLabel = ((j % i) == 0);
    DrawLineX(x1, x1 + (3+fShowLabel*2)*gi.nScaleT, yp);
    DrawLineX(x2 - (3+fShowLabel*2)*gi.nScaleT, x2, yp);
    if (!fShowLabel)
      continue;
    if (!fShowMinute)
      sprintf(sz, "%d%c", NAbs(j), j >= 0 ? 'N' : 'S');
    else
      sprintf(sz, "%d%c%02d", NAbs(j)/60, j >= 0 ? 'W' : 'E', NAbs(j)%60);
    DrawSz(sz, k, yp + 2*gi.nScaleTextT, dtCent | dtScale2);
    DrawSz(sz, gs.xWin-1 - k, yp + 2*gi.nScaleTextT, dtCent | dtScale2);
  }

  // Label degree points on horizontal axis.
  zLimit = !fShowMinute ? nDegHalf : nDegHalf*60;
  j = (!fShowMinute ? 5 : 7)*xFontT / Max((int)xScale2, 1) + 1;
  for (i = (int)(xBase2 - xi); i <= (int)(xBase2 + xi); i++) {
    xp = xc + (int)(((real)i - xBase2) * xScale2 + rRound);
    if (!FBetween(xp, x1, x2))
      continue;
    fShowLabel = ((i % j) == 0);
    DrawLineY(xp, y1, y1 + (3+fShowLabel*2)*gi.nScaleT);
    DrawLineY(xp, y2 - (3+fShowLabel*2)*gi.nScaleT, y2);
    if (!fShowLabel)
      continue;
    k = zLimit - i;
    while (k > zLimit)
      k -= (zLimit << 1);
    while (k <= -zLimit)
      k += (zLimit << 1);
    if (!fShowMinute)
      sprintf(sz, "%d%c", NAbs(k), k >= 0 ? 'W' : 'E');
    else
      sprintf(sz, "%d%c%02d", NAbs(k)/60, k >= 0 ? 'W' : 'E', NAbs(k)%60);
    DrawSz(sz, xp, 2*gi.nScaleTextT, dtCent | dtTop | dtScale2);
    if (!gs.fText)
      DrawSz(sz, xp, gs.yWin-1 - 2*gi.nScaleTextT,
        dtCent | dtBottom | dtScale2);
  }

  // Draw boundary of chart area.
  DrawEdge(x1, y1, x2, y2);
}


// This is a subprocedure of XChartOrbit(). Adjust the coordinates of a planet
// so its distance from the central body is on a logarithmic scale.

void OrbitPlot(real *pxp, real *pyp, real *pzp, real sz, int obj, PT3R *rgspc)
{
  real xp, yp, zp, xd, yd, zd, rDist;
  int objCenter;
  PT3R *spc;

  // Copy input parameters. Only care about Z-axis in 3D wireframe charts.
  xp = *pxp; yp = *pyp;
  zp = pzp != NULL ? *pzp : 0.0;

  // Tempoarily treat things orbiting planets as that planet's position.
  objCenter = ObjOrbit(obj);
  if (objCenter >= 0 && objCenter != oSun) {
    spc = rgspc + objCenter;
    xd = (xp - spc->x);
    yd = (yp - spc->y);
    zd = (zp - spc->z);
    xp = spc->x;
    yp = spc->y;
    zp = spc->z;
  }

  // Compute distance, determine ratio, and recompute coordinates.
  rDist = RLength3(xp, yp, zp);
  if (rDist < rSmall)
    return;
  rDist = (RLog(rDist / sz * 100.0 + 1.0) / rLog101) / (rDist / sz);
  xp *= rDist; yp *= rDist; zp *= rDist;

  // Scale things orbiting planets to be more distant from them.
  if (objCenter >= 0 && objCenter != oSun) {
    rDist = 1000.0 * (objCenter == oEar ? 1.0 : (objCenter == oMar ? 15.0 :
      (objCenter == oJup ? 0.9 : (objCenter == oSat ? 0.7 :
      (objCenter == oUra ? 4.0 : (objCenter == oNep ? 0.4 : 25.0))))));
#if FALSE
    // Modify radius with AstroExpression if one set.
    if (!us.fExpOff && FSzSet(us.szExpOrbit)) {
      ExpSetN(iLetterY, obj);
      ExpSetR(iLetterZ, rDist);
      ParseExpression(us.szExpOrbit);
      rDist = RExpGet(iLetterZ);
    }
#endif
    xp += xd * rDist;
    yp += yd * rDist;
    zp += zd * rDist;
  }

  // Copy back to input parameters.
  *pxp = xp; *pyp = yp;
  if (pzp != NULL)
    *pzp = zp;
}


// This is a subprocedure of XChartOrbit(). Append the current set of planet
// coordinates to an internal list, so the trails feature can be drawn.

void OrbitRecord()
{
  int i, j;

  // Do nothing if coordinate trails aren't being saved.
  if (gs.cspace <= 0)
    return;

  // Don't append coordinates to list if they haven't changed.
  for (i = 0; i <= oNorm; i++) {
    j = ((gi.ispace - 1 + gs.cspace) % gs.cspace)*oNorm1 + i;
    if (gi.rgspace[j].x != space[i].x || gi.rgspace[j].y != space[i].y ||
      gi.rgspace[j].z != space[i].z)
      break;
  }
  if (i > oNorm)
    return;

  // Append latest set of coordinates to list.
  j = gi.ispace*oNorm1;
  for (i = 0; i <= oNorm; i++)
    gi.rgspace[j++] = space[i];
  gi.ispace = (gi.ispace + 1) % gs.cspace;
  if (gi.cspace < gs.cspace)
    gi.cspace++;
}


// Draw a chart depicting an aerial view of the solar system in space, with
// all the planets drawn around the Sun, and the specified central planet in
// the middle, as done when the -S is combined with the -X switch.

void XChartOrbit()
{
  int cx = gs.xWin / 2, cy = gs.yWin / 2, unit, x1, y1, x2, y2,
    i, j, k, l, nSav;
  real sx, sy, sz, xp, yp, xp2, yp2, xpEar = 0.0, ypEar = 0.0, rT;
  ObjDraw rgod[objMax];
#ifdef SWISS
  ES es, *pes1, *pes2;
  int j2, k2;
#endif

  unit = Max(6*gi.nScale, gs.fText*Max(12, yFont*gi.nScaleText/2)*gi.nScaleT);
  x1 = unit; y1 = unit; x2 = gs.xWin-1-unit; y2 = gs.yWin-1-unit;

  // Determine the scale of the chart. For a scale size of 400, make the
  // graphic 1 AU in radius (just out to Earth's orbit). For 300, make the
  // chart 6 AU in radius (enough for inner planets out to asteroid belt).
  // For a scale of 200, make window 30 AU in radius (enough for planets out
  // to Neptune). For scale of 100, make it 90 AU in radius (enough for all
  // planets including the orbits of the Uranians). For larger text scale,
  // make it .006 AU (a little more than Moon's orbit around Earth). Finally,
  // if -YXS switch is set, use that custom value over all the above defaults.

  i = gi.nScale/gi.nScaleT;
  sz = gs.rspace > 0.0 ? gs.rspace : (i <= 1 ? 90.0 : (i == 2 ? 30.0 :
    (i == 3 ? 6.0 : (gi.nScaleText/2 <= 1 ? 1.0 : 0.006))));
  sx = (real)(cx-x1)/sz; sy = (real)(cy-y1)/sz;
  ClearB((pbyte)rgod, sizeof(rgod));
  for (i = 0; i <= is.nObj; i++) if (FProper(i)) {
    xp = space[i].x; yp = space[i].y;
    if (us.nStar > 0 || gs.fAllStar) {
      xp /= rLYToAU; yp /= rLYToAU;
    }
    if (us.fHouse3D)
      OrbitPlot(&xp, &yp, NULL, sz, i, space);
    rgod[i].x = cx-(int)(xp*sx); rgod[i].y = cy+(int)(yp*sy);
    rgod[i].obj = i;
    rgod[i].kv = ~0;
    rgod[i].f = fTrue;
  }

  // Draw planet disks (which become visible if zoomed in enough).
  if (!gs.fAlt)
    for (i = 0; i <= is.nObj; i++) if (FProper(i)) {
      rT = RObjDiam(i) / 2.0 / rAUToKm;
      j = (int)(rT * sx);
      k = (int)(rT * sy);
      if ((j > 1 || k > 1) &&
        FInRect(rgod[i].x, rgod[i].y, x1-j, y1-j, x2+j, y2+j)) {
        DrawColor(kDkGreenB);
        DrawCircle2(rgod[i].x, rgod[i].y, j, k);
        if (i == oSat || i == oSaC) {
          j = (int)(rSatRingA / rAUToKm * sx);
          k = (int)(rSatRingA / rAUToKm * sy);
          DrawCircle(rgod[i].x, rgod[i].y, j, k);
          j = (int)(rSatRingB / rAUToKm * sx);
          k = (int)(rSatRingB / rAUToKm * sy);
          DrawCircle(rgod[i].x, rgod[i].y, j, k);
        }
      }
    }

  // Draw the 12 sign boundaries from the center body to edges of screen.
  nSav = gi.nScale;
  gi.nScale = gi.nScaleTextT;
  if (!gs.fHouseExtra) {
    i = us.objCenter != oSun ? oSun : oEar;
    if (!gs.fColorSign)
      DrawColor(gi.kiGray);
    for (i = 0; i < cSign; i++) {
      j = i+1;
      if (gs.fColorSign)
        DrawColor(kSignB(j));
      k = cx - 2*(int)((real)cx*RCosD((real)i*30.0));
      l = cy + 2*(int)((real)cy*RSinD((real)i*30.0));
      DrawClip(cx, cy, k, l, x1, y1, x2, y2, 1);

      // Draw sign glyphs near edge of screen.
      k = (j == sGem || j == sCap) ? cx - (cx - x1) * 27 / 100 :
        ((j == sCan || j == sSag) ? cx + (cx - x1) * 27 / 100 :
        (FBetween(j, sLeo, sSco) ? x2 - gi.nScale*8 : x1 + gi.nScale*8));
      l = (j == sPis || j == sLib) ? cy - (cy - y1) * 27 / 100 :
        ((j == sAri || j == sVir) ? cy + (cy - y1) * 27 / 100 :
        (FBetween(j, sTau, sLeo) ? y2 - gi.nScale*8 : y1 + gi.nScale*8));
      DrawSign(j, k, l);
    }
  }
  gi.nScale = nSav;

  // Draw internal boundary.
  DrawColor(gi.kiLite);
  DrawEdge(x1, y1, x2, y2);

  // Draw orbital trails.
  if (gs.cspace > 0) {
    if (gi.rgspace == NULL) {
      gi.rgspace = RgAllocate(oNorm1*gs.cspace, PT3R, "orbits");
      if (gi.rgspace == NULL)
        return;
    }
    OrbitRecord();
    for (i = 0; i < gi.cspace; i++) {
      l = (gi.ispace - gi.cspace + i + gs.cspace) % gs.cspace;
      for (j = 0; j <= oNorm; j++) if (FProper(j)) {
        k = l*oNorm1 + j;
        xp = gi.rgspace[k].x; yp = gi.rgspace[k].y;
        if (us.fHouse3D)
          OrbitPlot(&xp, &yp, NULL, sz, j, &gi.rgspace[k - j]);
        DrawColor(kObjB[j]);
        if (!gs.fLabelAsp)
          DrawPoint(cx-(int)(xp*sx), cy+(int)(yp*sy));
        else if (i > 0) {
          // -XA setting on means orbit trails are lines instead of just dots.
          k = ((gi.ispace - gi.cspace + i-1 + gs.cspace) % gs.cspace) *
            oNorm1 + j;
          xp2 = gi.rgspace[k].x; yp2 = gi.rgspace[k].y;
          if (us.fHouse3D)
            OrbitPlot(&xp2, &yp2, NULL, sz, j, &gi.rgspace[k - j]);
          DrawLine(cx-(int)(xp2*sx), cy+(int)(yp2*sy),
            cx-(int)(xp*sx), cy+(int)(yp*sy));
        }
      }
    }
  } else if (gs.cspace < 0) {
    // Negative -YXj setting means draw orbits (assume they're circular).
    for (i = 0; i <= oNorm; i++) if (FProper(i)) {
      l = !FGeo(i) ? oSun : oEar;
      if (i == l || FIgnore(l) || FIgnore2(i))
        continue;
      k = (int)RLength2((real)(rgod[i].x - rgod[l].x),
        (real)(rgod[i].y - rgod[l].y));
      if (us.objCenter == l && k > cx + cy)
        continue;
      DrawColor(kObjB[i]);
      DrawCircle(rgod[l].x, rgod[l].y, k, k);
    }
  }

  // Draw lines connecting planets which have aspects between them.
  if (!gs.fEquator && us.nAsp > 0) {
    if (!FCreateGrid(fFalse))
      return;
    nSav = gi.nScale;
    gi.nScale = gi.nScaleTextT;
    for (j = oNorm; j >= 1; j--)
      for (i = j-1; i >= 0; i--)
        if (grid->n[i][j] && FProper(i) && FProper(j)) {
          DrawColor(kAspB[grid->n[i][j]]);
          DrawClip(rgod[i].x, rgod[i].y, rgod[j].x, rgod[j].y, x1, y1, x2, y2,
            NAbs(grid->v[i][j]/(60*60*2)));
          if (gs.fLabelAsp) {
            k = (rgod[i].x + rgod[j].x) >> 1;
            l = (rgod[i].y + rgod[j].y) >> 1;
            if (FBetween(k, x1, x2) && FBetween(l, y1, y2))
              DrawAspect(grid->n[i][j], k, l);
          }
        }
    gi.nScale = nSav;
  }

#ifdef SWISS
  // Draw extra stars.
  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      xp = es.space.x / rLYToAU; yp = es.space.y / rLYToAU;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, NULL, sz, -1, NULL);
      j = cx-(int)(xp*sx); k = cy+(int)(yp*sy);
      if (FInRect(j, k, x1, y1, x2, y2))
        DrawStar(j, k, &es);
    }

    // Draw constellation lines between stars.
    DrawColor(gi.kiLite);
    EnumStarsLines(fTrue, NULL, NULL);
    while (EnumStarsLines(fFalse, &pes1, &pes2)) {
      xp = pes1->space.x / rLYToAU; yp = pes1->space.y / rLYToAU;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, NULL, sz, -1, NULL);
      j = cx-(int)(xp*sx); k = cy+(int)(yp*sy);
      xp = pes2->space.x / rLYToAU; yp = pes2->space.y / rLYToAU;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, NULL, sz, -1, NULL);
      j2 = cx-(int)(xp*sx); k2 = cy+(int)(yp*sy);
      if (FInRect(j, k, x1, y1, x2, y2) && FInRect(j2, k2, x1, y1, x2, y2))
        DrawLine(j, k, j2, k2);
    }
  }

  // Draw extra asteroids.
  if (!gs.fAllStar && gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fTrue);
    while (SwissComputeAsteroid(is.T, &es, fTrue)) {
      xp = es.space.x; yp = es.space.y;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, NULL, sz, -1, NULL);
      j = cx-(int)(xp*sx); k = cy+(int)(yp*sy);
      if (FInRect(j, k, x1, y1, x2, y2))
        DrawStar(j, k, &es);
    }
  }
#endif

  // Draw planet glyphs, and spots for actual planet locations.
  DrawObjects(rgod, is.nObj+1, unit);
}


// Draw a chart showing the 36 Gauquelin sectors, with all the planets
// positioned in their appropriate sector (and at the correct fracton
// across the sector) as done when the -l is combined with the -X switch.

void XChartSector()
{
  real xplanet[objMax], symbol[objMax];
  byte ignoreSav[objMax], ignoreT[objMax];
  char sz[3];
  int nTrans = (int)(gs.rBackPct * 256.0 / 100.0), cx, cy, yi, i, j, k;
  real unitx, unity, px, py, temp;

  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;
  cx = gs.xWin/2 - 1; cy = gs.yWin/2 - 1;
  unitx = (real)cx; unity = (real)cy;
  yi = !us.fVedic ? 1 : -1;

  // Draw lines across the whole chart at the four angles.
  DrawColor(gi.kiLite);
  DrawDash(cx+POINT1(unitx, 0.99, PX(0.0)),
           cy+POINT1(unity, 0.99, PY(0.0)),
           cx+POINT1(unitx, 0.99, PX(180.0)),
           cy+POINT1(unity, 0.99, PY(180.0)), !gs.fColor);
  DrawDash(cx+POINT1(unitx, 0.99, PX(90.0)),
           cy+POINT1(unity, 0.99, PY(90.0)),
           cx+POINT1(unitx, 0.99, PX(270.0)),
           cy+POINT1(unity, 0.99, PY(270.0)), !gs.fColor);

  // Draw circles and radial lines delineating the 36 sectors.
  DrawColor(gi.kiOn);
  for (i = 0; i < nDegMax; i += 10) {
    px = PX((real)i); py = PY((real)i);
    DrawLine(cx+POINT1(unitx, 0.81, px), cy+POINT1(unity, 0.81, py)*yi,
      cx+POINT2(unitx, 0.95, px), cy+POINT2(unity, 0.95, py)*yi);
  }
  DrawCircle(cx, cy, (int)(unitx*0.95+rRound), (int)(unity*0.95+rRound));
  DrawCircle(cx, cy, (int)(unitx*0.81+rRound), (int)(unity*0.81+rRound));

  // Label the 36 sectors, with plus zones in red and normal in dark green.
  k = pluszone[cSector];
  for (i = 1; i <= cSector; i++) {
    j = pluszone[i];
    DrawColor(j ? kRedB : kDkGreenB);
    DrawFillWheel(cx+POINT1(unitx, 0.88, PX((real)(i*10+175))),
      cy+POINT1(unity, 0.88, PY((real)(i*10+175)))*yi+gi.nScale, i, 2);
    if (nTrans >= 128)
      DrawColor(gi.kiOn);
    sprintf(sz, "%d", i);
    DrawSz(sz, cx+POINT1(unitx, 0.88, PX((real)(i*10+175)))+
      (FBetween(i, 12, 19) ? -gi.nScale : 0),
      cy+POINT1(unity, 0.88, PY((real)(i*10+175)))*yi+gi.nScale,
      dtCent | dtScale);
    sprintf(sz, "%c", j ? '+' : '-');
    DrawSz(sz, cx+POINT1(unitx, 0.97, PX((real)(i*10+175))),
      cy+POINT1(unity, 0.97, PY((real)(i*10+175)))*yi+gi.nScaleTextT*2,
      dtCent | dtScale2);
    if (j != k) {
      DrawColor(gi.kiGray);
      DrawDash(cx, cy, cx+POINT2(unitx, 0.81, PX((real)(i*10+170))),
        cy+POINT2(unity, 0.81, PY((real)(i*10+170)))*yi, 1);
    }
    k = j;
  }

  // Compute the planets' sector positions.
  CopyRgb(ignore, ignoreSav, sizeof(ignore));
  CastSectors();

  // Figure out where to put planet glyphs.
  for (i = 0; i <= is.nObj; i++) {
    if (!us.fVedic)
      temp = Mod(rDegHalf - planet[i]);
    else
      temp = Mod(rDegHalf + planet[i]);
    symbol[i] = xplanet[i] = temp;
  }
  FillSymbolRing(symbol, 1.0);

  // For each planet, draw a small dot indicating where it is, and then a line
  // from that point to the planet's glyph.
  for (i = is.nObj; i >= 0; i--) if (FProper(i)) {
    if (gs.fLabel) {
      temp = symbol[i];
      DrawColor(ret[i] < 0.0 ? gi.kiGray : gi.kiOn);
      DrawDash(cx+POINT1(unitx, 0.67, PX(xplanet[i])),
        cy+POINT1(unity, 0.67, PY(xplanet[i])),
        cx+POINT1(unitx, 0.71, PX(temp)),
        cy+POINT1(unity, 0.71, PY(temp)),
        (ret[i] < 0.0 ? 1 : 0) - gs.fColor);
      DrawObject(i, cx+POINT1(unitx, 0.75, PX(temp)),
        cy+POINT1(unity, 0.75, PY(temp)));
    } else
      DrawColor(kObjB[i]);
    if (!gs.fHouseExtra)
      DrawSpot(cx+POINT1(unitx, 0.65, PX(xplanet[i])),
        cy+POINT1(unity, 0.65, PY(xplanet[i])));
    else
      DrawPoint(cx+POINT1(unitx, 0.65, PX(xplanet[i])),
        cy+POINT1(unity, 0.65, PY(xplanet[i])));
  }

  // Draw lines connecting planets which have aspects between them.
  if (!gs.fEquator) {            // Don't draw aspects in equator mode.
    if (!FCreateGrid(fFalse))
      return;
    for (j = is.nObj; j >= 1; j--)
      for (i = j-1; i >= 0; i--)
        if (grid->n[i][j] && FProper(i) && FProper(j))
          DrawAspectLine(i, j, cx, cy, xplanet[i], xplanet[j], unitx, unity,
            0.63);
  }

  // Draw sidebar based on normal chart positions.
  CopyRgb(ignore, ignoreT, sizeof(ignore));
  cp1 = cp0;
  CopyRgb(ignoreSav, ignore, sizeof(ignore));
  CastChart(1);
  CopyRgb(ignoreT, ignore, sizeof(ignore));
  DrawSidebar();
  CopyRgb(ignoreSav, ignore, sizeof(ignore));
}


// Draw an arrow from one point to another, a line with an arrowhead at the
// ending point. The size of the arrowhead is based on current scale size, and
// the line segment is actually shorter and doesn't touch either endpoint by
// the same amount. This is used by XChartDispositor() below.

void DrawArrow(int x1, int y1, int x2, int y2)
{
  real r, s, a;

  // Shrink line by the scale amount.
  r = RAngleD((real)(x2-x1), (real)(y2-y1));
  s = (real)(gi.nScale*8);
  x1 += (int)(s*RCosD(r)); y1 += (int)(s*RSinD(r));
  x2 -= (int)(s*RCosD(r)); y2 -= (int)(s*RSinD(r));
  s = (real)(gi.nScale)*4.5;
  // Draw main arrow shaft segment.
  DrawLine(x1, y1, x2, y2);
  // Draw the two arrowhead line pieces.
  for (a = -1.0; a <= 1.0; a += 2.0)
    DrawLine(x2, y2, x2 + (int)(s*RCosD(r + a*135.0)),
      y2 + (int)(s*RSinD(r + a*135.0)));
}


// Draw dispositor graphs (for the 10 main planets usually) as done when the
// -j is combined with the -X switch. Four graphs are drawn, one in each
// screen quadrant. A dispositor graph may be based on the sign or house
// position, and the planets may be arranged in a hierarchy or a wheel format.

void XChartDispositor()
{
  int oDis[oNorm1], dLev[oNorm1], cLev[oNorm1], xo[oNorm1], yo[oNorm1],
    obj[oNorm1];
  real xCirc[oNorm1], yCirc[oNorm1];
  char sz[cchSzDef];
  int *rgRules, oNum, xLev, yLev, xSub, ySub, cx0, cy0, cx, cy, i, j, k;

  // Determine rulership and object sets to use.

  if (ignore7[rrStd] && ignore7[rrEso] && !ignore7[rrHie])
    rgRules = rgSignHie1;
  else if (ignore7[rrStd] && !ignore7[rrEso])
    rgRules = rgSignEso1;
  else
    rgRules = rules;
  oNum = 0;
  for (i = 0; i <= oNorm; i++)
    if (FThing(i) && (!FIgnore(i) || (rgRules == rules ?
      FBetween(i, oSun, oMain) : (FBetween(i, oEar, oMain) || i == oVul))))
      obj[++oNum] = i;

  // Set up screen positions of the 10 planets for the wheel graphs.

  cx0 = gs.xWin / 2; cy0 = gs.yWin / 2;
  for (i = 1; i <= oNum; i++) {
    j = (gs.fHouseExtra ? 270 : 180) - (i-1)*(us.fVedic ? -360 : 360)/oNum;
    xCirc[i] = (real)cx0*0.4*RCosD((real)j);
    yCirc[i] = (real)cy0*0.4*RSinD((real)j);
  }

  // Loop over the two basic dispositor types: sign based and house based.

  for (xSub = 0; xSub <= 1; xSub++) {
    cx = xSub * cx0 + cx0 / 2;

    // For each planet, get its dispositor planet for current graph type.
    for (i = 1; i <= oNum; i++) {
      k = obj[i];
      j = rgRules[xSub ? inhouse[k] : SFromZ(planet[k])];
      for (k = 1; k <= oNum; k++)
        if (obj[k] == j)
          break;
      if (k > oNum)
        k = 1;
      oDis[i] = k;
      dLev[i] = 1;
    }

    // Determine the final dispositors (including mutual reception loops).
    do {
      j = fFalse;
      for (i = 1; i <= oNum; i++)
        cLev[i] = fFalse;
      for (i = 1; i <= oNum; i++)
        if (dLev[i])
          cLev[oDis[i]] = fTrue;
      for (i = 1; i <= oNum; i++)     // A planet isn't a final dispositor if
        if (dLev[i] && !cLev[i]) {    // nobody is pointing to it.
          dLev[i] = 0;
          j = fTrue;
        }
    } while (j);

    // Determine the level of each planet, i.e. how many times you have to
    // jump to your dispositor before reaching a final, with finals == 1.
    do {
      j = fFalse;
      for (i = 1; i <= oNum; i++)
        if (!dLev[i]) {
          if (!dLev[oDis[i]])
            j = fTrue;
          else                              // If my dispositor already has a
            dLev[i] = dLev[oDis[i]] + 1;    // level, mine is one more.
        }
    } while (j);

    // Count the number of planets at each dispositor level.
    for (i = 1; i <= oNum; i++)
      cLev[i] = 0;
    for (i = 1; i <= oNum; i++)
      cLev[dLev[i]]++;

    // Count the number of levels total, and max planets on any one level.
    xLev = yLev = 0;
    for (i = 1; i <= oNum; i++)
      if (cLev[i]) {
        yLev = i;
        if (cLev[i] > xLev)
          xLev = cLev[i];
      }

    // Loop over our two dispositor display formats: hierarchy and wheel.
    for (ySub = 0; ySub <= 1; ySub++) {
      cy = ySub * cy0 + cy0 / 2;
      sprintf(sz, "%s dispositor %s", xSub ? "House" : "Sign",
        ySub ? "wheel" : "hierarchy");
      DrawColor(gi.kiLite);
      DrawSz(sz, cx, ySub * cy0 + 3*gi.nScaleT, dtTop);

      if (ySub) {

        // Draw a graph in wheel format.
        for (i = 1; i <= oNum; i++) {
          k = obj[i];
          DrawObject(k, cx + (int)xCirc[i], cy + (int)yCirc[i]);
          j = oDis[i];
          if (j != i) {
            if (dLev[i] < 2)
              DrawColor(gi.kiOn);
            else
              DrawColor(kObjB[k]);
            DrawArrow(cx + (int)xCirc[i], cy + (int)yCirc[i],
              cx + (int)xCirc[j], cy + (int)yCirc[j]);
          }
          if (!gs.fAlt && (j == i || dLev[i] < 2)) {
            DrawColor(j == i ? gi.kiOn : gi.kiGray);
            DrawCircle(cx + (int)xCirc[i], cy + (int)yCirc[i],
              7*gi.nScale, 7*gi.nScale);
          }
        }
      } else {

        // For level hierarchies, first figure out the screen coordinates for
        // each planet, based on its level, total levels, and max width.

        for (i = 1; i <= oNum; i++) {
          yo[i] = cy0*(dLev[i]*2-1)/(yLev*2);
          k = 0;
          for (j = 1; j < i; j++)
            if (dLev[i] == dLev[j])
              k = j;
          if (k)
            xo[i] = xo[k] + cx0/xLev;    // One right of last one on level.
          else
            xo[i] = cx - ((cx0/xLev)*(cLev[dLev[i]]-1)/2);
        }

        // Draw graph in level hierarchy format.
        for (i = 1; i <= oNum; i++) {
          k = obj[i];
          DrawObject(k, xo[i], yo[i]);
          j = oDis[i];
          if (j != i) {
            if (dLev[i] < 2) {
              if (NAbs(xo[i] - xo[j]) < cx0/xLev*3/2) {
                DrawColor(gi.kiOn);
                DrawArrow(xo[i], yo[i], xo[j], yo[j]);
              }
              DrawColor(gi.kiGray);
            } else {
              DrawColor(kObjB[k]);
              DrawArrow(xo[i], yo[i], xo[j], yo[j]);
            }
          } else
            DrawColor(gi.kiOn);
          if (!gs.fAlt && dLev[i] < 2)
            DrawCircle(xo[i], yo[i], 7*gi.nScale, 7*gi.nScale);
        }
      }
    }
  }

  // Draw boundary lines between the four separate dispositor graphs.

  if (gs.fBorder) {
    DrawColor(gi.kiLite);
    DrawLineY(cx0, 0, gs.yWin-1);
    DrawLineX(0, gs.xWin-1, cy0);
  }
}


// Draw one aspect event within a box in a calendar. Called from
// ChartInDaySearch() which computes aspect events, which is in turn
// called from XChartCalendar() which draws the graphic calendar chart.

flag DrawCalendarAspect(InDayInfo *pid, int i, int iMax, int nVoid)
{
  int x1, y1, x2, y2, asp, x, y, z, s1, s2, s3, nT, k,
    nScale = gi.nScaleTextT2;
  char sz[4], *szTime;

  // Get pixel coordinates of the calendar and pixel size of each box.
  x1 = gi.rgzCalendar[pid->day*2];
  y1 = gi.rgzCalendar[pid->day*2 + 1];
  x2 = x1 + gi.rgzCalendar[0];
  y2 = y1 + gi.rgzCalendar[1];
  z = nScale * 5;
  y = y2 - z*(iMax-i) + nScale*3/2;
  if (y - nScale*5/2 <= y1)
    return fFalse;
  x = x2 + nScale*3/2;
  if (x - z*3 - nScale*5/2 <= x1)
    return fFalse;

  // Get aspect event and time that it takes place.
  asp = pid->aspect;
  s1 = (int)pid->time/60;
  s2 = (int)pid->time-s1*60;
  s3 = us.fSeconds ? (int)(pid->time*60.0)-((s1*60+s2)*60) : -1;

  // Draw the aspect event itself.
  DrawColor(kObjB[pid->source]);
  DrawObject(pid->source, x - z*3, y);
  if (asp >= aCon) {
    DrawColor(kAspB[asp]);
    DrawAspect(pid->aspect, x - z*2, y);
  } else if (asp == aSig || asp == aHou) {
    DrawColor(gi.kiOn);
    DrawTurtle("NL4R4NG4H4", x - z*2, y);
  } else if (asp == aDir) {
    DrawColor(gi.kiOn);
    DrawTurtle("NR3L3HU2ER6FBD4NHD2GL6H", x - z*2, y);
    DrawTurtle(pid->dest ? "F4BL8U8R7FD2GL7" : "BG4U8R6F2D4G2L6", x - z, y);
  } else if (asp == aDeg) {
    DrawColor(gi.kiOn);
    DrawTurtle("BRUL2D2REFREU2H2L4G2D4F2R4", x - z*2, y);
  } else if (asp == aAlt) {
    DrawColor(gi.kiOn);
    DrawTurtle("BH4D8R8", x - z*2, y);
    DrawTurtle(pid->dest ? "NU4ND4NL4R4" : "NL4R4", x - z, y);
  } else if (asp == aLen) {
    DrawColor(gi.kiOn);
    DrawTurtle(pid->dest ? "NL4R4ND4U3HL6GD7" : "NL4R3EU2HL7D8", x - z*2, y);
    DrawTurtle(pid->dest ? "NL4R3EU2HL7D8" : "NR3L4D4R8BU8L8D4", x - z, y);
  }
  if (asp >= aCon) {
    DrawColor(kObjB[pid->source]);
    DrawObject(pid->dest, x - z, y);
  } else if (asp == aSig || asp == aDeg) {
    nT = asp == aSig ? pid->dest : pid->dest / us.nSignDiv + 1;
    DrawColor(kSignB(nT));
    DrawSign(nT, x - z, y);
  }

  // Draw the time that the aspect event takes place (if room).
  szTime = SzTime(s1, s2, s3);
  nT = x - z*4 - CchSz(szTime)*xFont2*nScale;
  if (nT <= x1)
    return fTrue;
  DrawColor(gi.kiGray);
  DrawSz(szTime, nT, y+nScale, dtLeft | dtScale2);

  // Draw extra information about the aspect event (if room).
  nT -= 4*xFont2*nScale;
  if (nT <= x1)
    return fTrue;
  k = kSignB(SFromZ(planet[oMoo]));
  if (pid->source == oSun && pid->dest == oMoo &&
    (asp == aCon || asp == aOpp)) {
    DrawColor(k);
    DrawSz(asp == aCon ? (nVoid < 0 ? "NEW" : "N+v") :
      (nVoid < 0 ? "FUL" : "F+v"), nT, y+nScale, dtLeft | dtScale2);
  } else if (nVoid >= 0) {
    DrawColor(k);
    DrawSz("v/c", nT, y+nScale, dtLeft | dtScale2);
  } else if (pid->source == oMoo && asp == aSig) {
    DrawColor(kSignB(pid->dest));
    sprintf(sz, "%.3s", szSignName[pid->dest]);
    DrawSz(sz, nT, y+nScale, dtLeft | dtScale2);
  }
  return fTrue;
}


// Draw a graphical calendar for a given month, with numbers in boxes,
// scaled to fit within the given bounds. This is used for single month
// -K switch images and is called 12 times for a full year -Ky image.

void DrawCalendar(int mon, int x1, int y1, int x2, int y2)
{
  char sz[cchSzDef];
  int rgz[(31+1)*2], day, cday, dayHi, cweek, xunit, yunit, xs, ys, x0, y0,
    x, y, s, nSav;
  flag fSav;

  xs = x2 - x1; ys = y2 - y1;
  day = DayOfWeek(mon, 1, Yea);    // Day of week of 1st of month.
  cday = DaysInMonth(mon, Yea);    // Count of days in the month.
  dayHi = DayInMonth(mon, Yea);    // Number of last day in the month.
  cweek = us.fCalendarYear ? 6 : (day + cday + 6) / 7;   // Week rows.
  xunit = xs/8;                    // Hor. pixel size of each day box.
  yunit = ys/(cweek+2);            // Ver. pixel size of each day box.
  x0 = x1 + (xs - xunit*7) / 2;    // Blank space to left of calendar.
  y0 = y1 + yunit*3/2;             // Blank space to top of calendar.

  // Print the month and year in big letters at top of chart.
  DrawColor(gi.kiOn);
  sprintf(sz, "%s, %d", szMonth[mon], Yea);
  s = gi.nScale;
  gi.nScale = Min((yunit*3/2-yFont*s) / yFont, xs/15/*CchSz(sz)*/ / xFont);
  gi.nScale = (gi.nScale/s)*s;        // So scale is an even multiple of "s".
  gi.nScale = Max(gi.nScale-1, 1);
  DrawSz(sz, x1 + xs/2, y1 + (yunit*3/2-yFont*s)/2, dtCent | dtScale);

  // Draw the grid of boxes for the days.
  for (gi.nScale = s; gi.nScale > 0 && xunit / (xFont*gi.nScale) < 3;
    gi.nScale--)
    ;
  for (x = 0; x <= cWeek; x++) {

    // Print days of week at top of each column (abbreviated if need be).
    if (x < cWeek) {
      if (xunit / (xFont*gi.nScale) < 9)
        sprintf(sz, "%.3s", szDay[x]);
      else
        sprintf(sz, "%s", szDay[x]);
      DrawColor(kYellowB);
      DrawSz(sz, x0 + x*xunit + xunit/2, y0 - s*3, dtBottom | dtScale);
      DrawColor(kCyanB);
    }
    DrawLine(x0 + x*xunit, y0, x0 + x*xunit, y0 + cweek*yunit);
  }
  for (y = 0; y <= cweek; y++)
    DrawLine(x0, y0 + y*yunit, x0 + 7*xunit, y0 + y*yunit);

  // Actually draw the day numbers in their appropriate boxes.
  x = day; y = 0;
  for (day = 1; day <= dayHi; day = AddDay(mon, day, Yea, 1)) {
    rgz[day*2] = x0 + x*xunit;
    rgz[day*2+1] = y0 + y*yunit;
    sprintf(sz, gs.fText ? "%2d" : "%d", day);
    DrawColor(day == Day && mon == Mon && gs.fLabel ? kGreenB :
      (x <= 0 || x >= cWeek-1 ? kRedB : gi.kiLite));
    if (!gs.fAlt || gs.fLabelAsp)
      DrawSz(sz, x0 + x*xunit + s*2, y0 + y*yunit + s*4,
        dtLeft | dtTop | dtScale);
    else
      DrawSz(sz, x0 + x*xunit + xunit/2,
        y0 + y*yunit + yunit/2 + gi.nScale, dtCent | dtScale);
    if (++x >= cWeek) {
      x = 0;
      y++;
    }
  }

  // Draw aspect events taking place within each day within the boxes.
  if (gs.fLabelAsp) {
    gi.nScale = gi.nScaleTextT;
    rgz[0] = xunit; rgz[1] = yunit;
    gi.rgzCalendar = rgz;
    us.fInDayMonth = fTrue; us.fInDayYear = fFalse;
    nSav = Mon; Mon = mon;
    fSav = gs.fLabel; gs.fLabel = fTrue;
    ChartInDaySearch(is.fProgress);
    Mon = nSav; gs.fLabel = fSav;
    gi.rgzCalendar = NULL;
  }
  gi.nScale = s;
}


// Draw a graphical calendar on the screen for the chart month or entire year,
// as done when the -K or -Ky is combined with the -X switch.

void XChartCalendar()
{
  int xs, ys, xunit, yunit, x1, y1, x, y;

  if (!us.fCalendarYear) {
    DrawCalendar(Mon, 0, 0, gs.xWin, gs.yWin);
    return;
  }

  // Determine the best sized rectangle of months to draw the year in based on
  // the chart dimensions: Either do 6x2 months, or 4x3, 3x4, or 2x6.

  if (gs.xWin > gs.yWin) {
    if (gs.xWin > gs.yWin * 3) {
      xs = 6; ys = 2;
    } else {
      xs = 4; ys = 3;
    }
  } else {
    if (gs.yWin > gs.xWin * 2) {
      xs = 2; ys = 6;
    } else {
      xs = 3; ys = 4;
    }
  }
  xunit = gs.xWin / xs; yunit = gs.yWin / ys;
  x1 = (gs.xWin - xunit*xs) / 2;
  y1 = (gs.yWin - yunit*ys) / 2;
  for (y = 0; y < ys; y++)
    for (x = 0; x < xs; x++) {
      DrawCalendar(y * xs + x + 1, x1 + x*xunit, y1 + y*yunit,
        x1 + (x+1)*xunit, y1 + (y+1)*yunit);
    }
}


// Translate to chart pixel coordinates, that indicate where to draw on a
// chart sphere, for the -XX switch chart. Inputs may be local horizon
// altitude and azimuth coordinates, local horizon prime vertical, local
// horizon meridian, zodiac position and latitude, or Earth coordinates.

flag FSphereLocal(real azi, real alt, CONST CIRC *pcr, int *xp, int *yp)
{
  if (gs.fEcliptic) {
    azi = Mod(azi - rDegQuad); neg(alt);
    CoorXform(&azi, &alt, is.latMC - rDegQuad);
    azi = Mod(is.lonMC - azi + rDegQuad);
    EquToEcl(&azi, &alt);
    azi = rDegMax - Untropical(azi); neg(alt);
  }
  azi = Mod(rDegQuad*3 - (azi + gs.rRot));
  if (gs.rTilt != 0.0)
    CoorXform(&azi, &alt, gs.rTilt);
  *xp = pcr->xc + (int)((real)pcr->xr * RCosD(azi) * RCosD(alt) - rRound);
  *yp = pcr->yc + (int)((real)pcr->yr * RSinD(alt) - rRound);
  return azi >= rDegHalf;
}

flag FSpherePrime(real azi, real alt, CONST CIRC *pcr, int *xp, int *yp)
{
  CoorXform(&azi, &alt, rDegQuad);
  return FSphereLocal(azi + rDegQuad, alt, pcr, xp, yp);
}

flag FSphereMeridian(real azi, real alt, CONST CIRC *pcr, int *xp, int *yp)
{
  azi = Mod(azi + rDegQuad);
  CoorXform(&azi, &alt, rDegQuad);
  return FSphereLocal(azi, alt, pcr, xp, yp);
}

flag FSphereZodiac(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp)
{
  real lonT, latT;

  lonT = Tropical(lon); latT = lat;
  EclToEqu(&lonT, &latT);
  lonT = Mod(is.lonMC - lonT + rDegQuad);
  EquToLocal(&lonT, &latT, rDegQuad - is.latMC);
  return FSphereLocal(lonT + rDegQuad, -latT, pcr, xp, yp);
}

flag FSphereEarth(real azi, real alt, CONST CIRC *pcr, int *xp, int *yp)
{
  azi = Mod(-azi);
  CoorXform(&azi, &alt, rDegQuad - Lat);
  return FSphereLocal(azi + rDegQuad, -alt, pcr, xp, yp);
}


// Draw a chart sphere (like a chart wheel but in 3D) as done with the -XX
// switch. This is similar to Astrolog's local horizon charts.

void XChartSphere()
{
  char sz[cchSzDef];
  int zGlyph, zGlyph2, zGlyphS, cChart, iChart, xo = 0, yo = 0, xp, yp,
    i, j, k, k2, nSav;
  flag fHouse3D = !us.fHouse3D, fDir = !gs.fSouth, fAny = !gs.fAlt,
    fNoHorizon, f;
  real rT;
  CIRC cr, cr2;
  CONST CP *pcp;
  CP cpSav;
  ObjDraw rgod[objMax];
#ifdef SWISS
  ES es, *pes1, *pes2;
#endif

  // Initialize variables.
  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;

  fNoHorizon = ignorez[0] && ignorez[1] && ignorez[2] && ignorez[3];
  zGlyph = Max(7*gi.nScale,
    (gs.fText && us.fVelocity)*Max(12, yFont*gi.nScaleText/2)*gi.nScaleT);
  zGlyph2 = 14*gi.nScale; zGlyphS = 9*gi.nScaleT;
  cr.xc = gs.xWin >> 1; cr.yc = gs.yWin >> 1;
  cr.xr = cr.xc - zGlyph; cr.yr = cr.yc - zGlyph;
  cr2 = cr;
  cr2.xr += zGlyph >> 1; cr2.yr += zGlyph >> 1;
  cChart = 1 +
    (us.nRel <= rcDual) + (us.nRel <= rcTriWheel) + (us.nRel <= rcQuadWheel);

  if (us.nRel < rcNone)
    CastChart(1);
  is.latMC = Lat;

  // Avoid default alignments of sphere that don't look as good.
  if (us.fSmartSave && !gi.fDidSphere && gs.rRot == 0.0 && gs.rTilt == 0.0) {
    gs.rRot = 7.0;
    gs.rTilt = -7.0;
  }
  gi.fDidSphere = fTrue;

  // Draw constellations.
  if (gs.fConstel) {
    neg(gs.rTilt);
    DrawMap(fTrue, fTrue, gs.rRot);
    neg(gs.rTilt);
  }

  // Draw horizon.
  if (!fNoHorizon || (!gs.fHouseExtra && fHouse3D)) {
    if (!gs.fColorHouse)
      DrawColor(gi.kiOn);
    for (i = 0; i <= nDegMax; i++) {
      if (gs.fColorHouse && (i == 0 || i == nDegHalf))
        DrawColor(kSignB(i ? sLib : sAri));
      f = FSphereLocal((real)i, 0.0, &cr, &xp, &yp) ^ fDir;
      if (f && i > 0) {
        DrawLine(xo, yo, xp, yp);
        k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
        for (j = -k; j <= k; j += (k << 1)) {
          FSphereLocal((real)i, (real)j / 2.0, &cr, &xo, &yo);
          DrawLine(xo, yo, xp, yp);
        }
      } else if (fAny)
        DrawPoint(xp, yp);
      xo = xp; yo = yp;
    }
  }

  // Draw Earth's equator.
  if (gs.fEquator) {
    DrawColor(kPurpleB);
    for (i = 0; i <= nDegMax; i++) {
      f = FSphereEarth((real)i, 0.0, &cr, &xp, &yp) ^ fDir;
      if (f && i > 0)
        DrawLine(xo, yo, xp, yp);
      else if (fAny && !FOdd(i))
        DrawPoint(xp, yp);
      xo = xp; yo = yp;
    }
  }

  // Draw prime vertical.
  if (!fNoHorizon) {
    if (!gs.fColorHouse)
      DrawColor(gi.kiGray);
    for (i = 0; i <= nDegMax; i++) {
      if (gs.fColorHouse)
        DrawColor(kSignB((i-1)/30 + 1));
      f = FSpherePrime((real)i, 0.0, &cr, &xp, &yp) ^ fDir;
      if (f && i > 0) {
        DrawLine(xo, yo, xp, yp);
        k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
        for (j = -k; j <= k; j += (k << 1)) {
          FSpherePrime((real)i, (real)j / 2.0, &cr, &xo, &yo);
          DrawLine(xo, yo, xp, yp);
        }
      } else if (fAny && !FOdd(i))
        DrawPoint(xp, yp);
      xo = xp; yo = yp;
    }
  }

  // Draw 3D house wedges and meridian.
  if (!gs.fColorHouse)
    DrawColor(kDkGreenB);
  for (j = -1; j <= cSign; j++) {
    if (!(!gs.fHouseExtra && fHouse3D) && !(j <= 0 && !fNoHorizon))
      continue;
    if (fHouse3D &&
      ((j == sCap && FSameR(chouse[j], is.MC)) ||
       (j == sCan && FSameR(chouse[j], Mod(is.MC  + rDegHalf)))))
      continue;
    if (fHouse3D && us.nHouse3D == hmPrime &&
      ((j == sAri && FSameR(chouse[j], is.Asc)) ||
       (j == sLib && FSameR(chouse[j], Mod(is.Asc + rDegHalf)))))
      continue;
    if (fHouse3D && us.nHouse3D == hmHorizon &&
      ((j == (Lat >= 0.0 ? sAri : sLib) &&
        FSameR(chouse[j], Mod(is.Vtx + rDegHalf))) ||
       (j == (Lat >= 0.0 ? sLib : sAri) && FSameR(chouse[j], is.Vtx))))
      continue;
    if (gs.fColorHouse) {
      k = j > 0 ? j : (j < 0 ? sCan : sCap);
      DrawColor(kSignB(k));
    }
    rT = j > 0 ? chouse3[j] : (j < 0 ? rDegQuad : 270.0);
    if (j > 0 && us.nHouse3D == hmHorizon)
      rT = (rDegQuad - rT) * (Lat < 0.0 ? -1.0 : 1.0);
    for (i = -90; i <= 90; i++) {
      if (j <= 0 || us.nHouse3D == hmPrime)
        f = FSpherePrime(rT, (real)i, &cr, &xp, &yp) ^ fDir;
      else if (us.nHouse3D == hmHorizon)
        f = FSphereLocal(rT, (real)i, &cr, &xp, &yp) ^ fDir;
      else
        f = FSphereEarth(rT, (real)i, &cr, &xp, &yp) ^ fDir;
      if (f && i > -90) {
        DrawLine(xo, yo, xp, yp);
        if (j <= 0) {
          k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
          for (k2 = -k; k2 <= k; k2 += (k << 1)) {
            FSphereMeridian((real)(j == 0 ? i+180 : 360-i), (real)k2 / 2.0,
              &cr, &xo, &yo);
            DrawLine(xo, yo, xp, yp);
          }
        }
      } else if (fAny && !FOdd(i))
        DrawPoint(xp, yp);
      xo = xp; yo = yp;
    }
  }

  // Draw 2D house wedges.
  if (!gs.fHouseExtra && !fHouse3D)
    for (i = 1; i <= cSign; i++) {
      if (gs.fColorHouse)
        DrawColor(kSignB(i));
      for (j = -90; j <= 90; j++) {
        f = FSphereZodiac(chouse[i], (real)j, &cr, &xp, &yp) ^ fDir;
        if (f && j > -90)
          DrawLine(xo, yo, xp, yp);
        else if (fAny && !FOdd(i))
          DrawPoint(xp, yp);
        xo = xp; yo = yp;
      }
    }

  // Draw sign wedges.
  if (!us.fVedic) {
    if (!gs.fColorSign)
      DrawColor(kDkBlueB);
    // Draw ecliptic circle.
    for (i = 0; i <= nDegMax; i++) {
      if (gs.fColorSign)
        DrawColor(kSignB((i-1)/30 + 1));
      f = FSphereZodiac((real)i, 0.0, &cr, &xp, &yp) ^ fDir;
      if (f && i > 0) {
        DrawLine(xo, yo, xp, yp);
        if (i % 30 != 0) {
          k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
          for (j = -k; j <= k; j += (k << 1)) {
            FSphereZodiac((real)i, (real)j / 2.0, &cr, &xo, &yo);
            DrawLine(xo, yo, xp, yp);
          }
        }
      } else if (fAny)
        DrawPoint(xp, yp);
      xo = xp; yo = yp;
    }
    // Draw sign boundaries.
    for (i = 0; i < nDegMax; i += 30) {
      if (gs.fColorSign)
        DrawColor(kSignB(i/30 + 1));
      for (j = -90; j <= 90; j++) {
        f = FSphereZodiac((real)i, (real)j, &cr, &xp, &yp) ^ fDir;
        if (f && j > -90)
          DrawLine(xo, yo, xp, yp);
        else if (fAny && !FOdd(i))
          DrawPoint(xp, yp);
        xo = xp; yo = yp;
      }
    }
  }

  // Draw outer boundary.
  DrawColor(gi.kiOn);
  DrawCircle(cr.xc, cr.yc, cr.xr, cr.yr);

  // Label signs.
  if (!us.fVedic) {
    nSav = gi.nScale;
    gi.nScale = gi.nScaleTextT;
    for (j = 78; j >= -78; j -= 156)
      for (i = 1; i <= cSign; i++) {
        f = FSphereZodiac((real)(i*30-15), (real)j, &cr, &xp, &yp) ^ fDir;
        if (f || fAny) {
          DrawColor(f ? (gs.fColorSign ? kSignB(i) : kDkBlueB) : gi.kiGray);
          DrawSign(i, xp, yp);
        }
      }
    gi.nScale = nSav;
  }

  // Label houses.
  if (!gs.fHouseExtra) {
    nSav = gi.nScale;
    gi.nScale = gi.nScaleTextT;
    for (j = 82; j >= -82; j -= 164)
      for (i = 1; i <= cSign; i++) {
        if (!fHouse3D)
          f = FSphereZodiac(Midpoint(chouse[i], chouse[Mod12(i+1)]), (real)j,
            &cr, &xp, &yp) ^ fDir;
        else {
          rT = Midpoint(chouse3[i], chouse3[Mod12(i+1)]);
          if (us.nHouse3D == hmPrime)
            f = FSpherePrime(rT, (real)j, &cr, &xp, &yp) ^ fDir;
          else if (us.nHouse3D == hmHorizon) {
            rT = (rDegQuad - rT) * (Lat < 0.0 ? -1.0 : 1.0);
            f = FSphereLocal(rT, (real)j, &cr, &xp, &yp) ^ fDir;
          } else
            f = FSphereEarth(rT, (real)j, &cr, &xp, &yp) ^ fDir;
        }
        if (f || fAny) {
          DrawColor(f ? (gs.fColorHouse ? kSignB(i) : kDkGreenB) : gi.kiGray);
          DrawHouse(i, xp, yp);
        }
      }
    gi.nScale = nSav;
  }

  // Label directions.
  if (!fNoHorizon) {
    k = zGlyph >> 1;
    for (i = 0; i < nDegMax; i += 90) {
      f = FSphereLocal((real)i, 0.0, &cr, &xp, &yp) ^ fDir;
      if (f || fAny) {
        j = i / 90;
        DrawColor(kObjB[oAsc + ((j + 3) & 3)*3]);
        if (!ignorez[(1 - j) & 3])
          DrawDash(cr.xc, cr.yc, xp, yp, f ? 0 : 2);
        if (gs.fColorHouse)
          DrawColor(f ? gi.kiOn : gi.kiGray);
        FSphereLocal((real)i, 0.0, &cr2, &xp, &yp);
        sprintf(sz, "%c", szDir[j][0]);
        DrawSz(sz, xp, yp + gi.nScale, dtCent | dtScale2);
      }
    }
    // Label zenith and nadir points.
    for (j = -90; j <= 90; j += nDegHalf) {
      f = FSphereLocal(0.0, (real)j, &cr2, &xp, &yp) ^ fDir;
      if (f || fAny) {
        DrawColor(gs.fColorHouse ? (f ? gi.kiOn : gi.kiGray) :
          kObjB[j <= 0 ? oMC : oNad]);
        sprintf(sz, "%c", j <= 0 ? 'Z' : 'N');
        DrawSz(sz, xp, yp + gi.nScale, dtCent | dtScale2);
      }
    }
  }

  // Draw center point.
  if (!fNoHorizon) {
    DrawColor(gi.kiOn);
    DrawSpot(cr.xc, cr.yc);
  }

#ifdef SWISS
  // Draw extra stars.
  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      f = FSphereZodiac(es.lon, es.lat, &cr, &xp, &yp) ^ fDir;
      if (f)
        DrawStar(xp, yp, &es);
    }

    // Draw constellation lines between stars.
    DrawColor(gi.kiLite);
    EnumStarsLines(fTrue, NULL, NULL);
    while (EnumStarsLines(fFalse, &pes1, &pes2)) {
      f  = FSphereZodiac(pes1->lon, pes1->lat, &cr, &xo, &yo) ^ fDir;
      f &= FSphereZodiac(pes2->lon, pes2->lat, &cr, &xp, &yp) ^ fDir;
      if (f)
        DrawLine(xo, yo, xp, yp);
    }
  }

  // Draw extra asteroids.
  if (gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fTrue);
    while (SwissComputeAsteroid(is.T, &es, fTrue)) {
      f = FSphereZodiac(es.lon, es.lat, &cr, &xp, &yp) ^ fDir;
      if (f)
        DrawStar(xp, yp, &es);
    }
  }
#endif

  // Determine set of planet data to use.
  for (iChart = cChart; iChart >= 1; iChart--) {
    FProcessCommandLine(szWheelX[iChart]);
    if (iChart <= 1)
      pcp = rgpcp[us.nRel <= rcDual];
    else
      pcp = rgpcp[iChart];
    cpSav = cp0;
    cp0 = *pcp;

  // Calculate planet coordinates.
  for (i = 0; i <= is.nObj; i++) {
    f = FProper(i);
    if (f) {
      f = FSphereZodiac(planet[i], planetalt[i], &cr, &xp, &yp) ^ fDir;
      rgod[i].obj = i;
      rgod[i].x = xp; rgod[i].y = yp;
      rgod[i].kv = f ? ~0 : gi.kiGray;
      rgod[i].f = fAny || f;
    } else
      rgod[i].f = fFalse;
  }

  // Draw lines connecting planets which have aspects between them.
  if (!FCreateGrid(fFalse))
    return;
  nSav = gi.nScale;
  gi.nScale = gi.nScaleTextT;
  for (j = is.nObj; j >= 1; j--)
    for (i = j-1; i >= 0; i--)
      if (grid->n[i][j] && FProper(i) && FProper(j) &&
        (fAny || (rgod[i].f && rgod[j].f))) {
        DrawColor(rgod[i].kv == ~0 && rgod[j].kv == ~0 ?
          kAspB[grid->n[i][j]] : gi.kiGray);
        DrawDash(rgod[i].x, rgod[i].y, rgod[j].x, rgod[j].y,
          NAbs(grid->v[i][j] / (60*60*2)) +
          ((rgod[i].kv != ~0) + (rgod[j].kv != ~0))*2);
        if (gs.fLabelAsp)
          DrawAspect(grid->n[i][j],
            (rgod[i].x + rgod[j].x) >> 1, (rgod[i].y + rgod[j].y) >> 1);
      }
  gi.nScale = nSav;

  // Draw planet glyphs, and spots for actual planet locations.
  DrawObjects(rgod, is.nObj+1, 0);

    cp0 = cpSav;
  } // iChart
  FProcessCommandLine(szWheelX[0]);

  DrawSidebar();
}
#endif // GRAPH

/* xcharts1.cpp */
