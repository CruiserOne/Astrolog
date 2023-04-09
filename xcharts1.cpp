/*
** Astrolog (Version 7.60) File: xcharts1.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2023 by
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
** Last code change made 4/8/2023.
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
  // chart size to leave room for the sidebar, if that mode is in effect.

  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;
  cx = gs.xWin/2 - 1; cy = gs.yWin/2 - 1;
  unitx = (real)cx; unity = (real)cy;
  gi.rAsc = gs.objLeft ? planet[NAbs(gs.objLeft)-1] +
    rDegQuad*(gs.objLeft < 0) : chouse[1];
  if (us.fIndian)
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
    x = cp0.lonMC - planet1[i];
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
      if (j >= gs.xWin)
        j -= gs.xWin;
      end1[i*2] = (real)j;
      DrawColor(kElemB[eWat]);
      DrawLine(j, y1+unit*2, j, y2-unit*2);
    }
  }

  // Now, normally (unless are in bonus chart mode) will go on to draw the
  // the Ascendant and Descendant lines here.

  longm = Mod(cp0.lonMC + lon);
  if (!gs.fAlt && (!ignorez[arAsc] || !ignorez[arDes]))
  for (i = 1; i <= is.nObj; i++) if (FProper(i)) {
    xold1 = xold2 = nNegative;

    // Normally the Ascendant and Descendant line segments are drawn
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


// Compose a string to display within a graphic aspect grid cell.

KI FormatGridCell(char *sz, int x, int y, int type, flag fWide)
{
  char szT[cchSzDef];
  int n, d, m, s;
  real v;
  KI ki = -1;

  if (x < 0) {
    v = rgpcp[-x-1]->obj[y];
    n = SFromZ(v); v = v - ZFromS(n);
  } else {
    n = grid->n[x][y]; v = grid->v[x][y];
  }
  *szT = chNull;
  s = NAbs((int)(v*3600.0)); m = s/60; d = m/60; m %= 60; s %= 60;

  // For aspect cells, print orb in degrees and minutes.
  if (type == 1) {
    if (n > 0) {
      if (us.fDistance && !us.fParallel) {
        sprintf(sz, "%c%f", rgchAppSep[us.nAppSep*2 + (v >= 0.0)],
          RAbs(v));
        sprintf(&sz[fWide ? 8 : 5], "%s", "%");
      } else if (us.nDegForm != df360) {
        if (fWide)
          sprintf(szT, "%02d", s);
        sprintf(sz, "%c%d%c%02d'%s", rgchAppSep[us.nAppSep*2 + (v >= 0.0)],
          d, chDegL, m, szT);
        sz[fWide ? (d >= 100 ? 8 : 9) : (d >= 100 ? 5 : 6)] = chNull;
      } else {
        sprintf(sz, "%c%f", rgchAppSep[us.nAppSep*2 + (v >= 0.0)], RAbs(v));
        sz[fWide ? 8 : 5] = chNull;
      }
    } else
      *sz = chNull;
  }

  // For midpoint cells, print degrees and minutes.
  else if (type == 2 || (type == 0 && us.nDegForm == dfHM)) {
    if (us.nDegForm == dfHM) {
      sprintf(sz, "%s", SzZodiac((real)((n-1)*30) + v));
      sz[3] = sz[4]; sz[4] = sz[5]; sz[5] = 'm';
      if (fWide)
        sprintf(sz+6, "%s", sz+8);
      sz[fWide ? 8 : 6] = chNull;
    } else if (us.nDegForm != df360) {
      if (fWide)
        sprintf(szT, "%02d", s);
      sprintf(sz, "%2d%c%02d'%s", d, chDegL, m, szT);
    } else
      sprintf(sz, fWide ? "%8.5f" : "%5.2f", RAbs(v));
  }

  // For main diagonal cells, print sign and degree of the planet.
  else {
    ki = kSignB(n);
    if (us.nDegForm != df360) {
      if (fWide)
        sprintf(szT, "%c%02d", chDegL, m);
      sprintf(sz, "%.3s %02d%s", szSignName[n], d, szT);
    } else
      sprintf(sz, fWide ? "%8.4f" : "%5.1f", RAbs((real)((n-1)*30) + v));
  }
  return ki;
}


// Draw an aspect and midpoint grid in the window, with planets labeled down
// the diagonal. This chart is done when the -g switch is combined with the
// -X switch. The chart always has a certain number of cells, hence based on
// how the restrictions are set up, there may be blank columns and rows, or
// else only the first number of unrestricted objects will be included.

void XChartGrid(int x0, int y0)
{
  char sz[cchSzDef];
  int nScale, unit, siz, x, y, i, j, k, i0, j0, ig, jg;
  KI c;

  nScale = gi.nScale/gi.nScaleT;
  unit = CELLSIZE*gi.nScale; siz = gi.nGridCell*unit;
  i = us.fSmartCusp; us.fSmartCusp = fFalse;
  j = us.objRequire; us.objRequire = -1;
  if (!FCreateGrid(gs.fAlt))
    return;
  us.fSmartCusp = i; us.objRequire = j;

  // Loop through each cell in each row and column of grid.

  for (y = 1, j0 = -1; y <= gi.nGridCell; y++) {
    do {
      j0++;
      j = rgobjList[j0];
    } while (!FProper(j) && j0 <= is.nObj);
    DrawColor(gi.kiGray);
    DrawDash(x0, y0 + y*unit, x0 + siz, y0 + y*unit, !gs.fColor);
    DrawDash(x0 + y*unit, y0, x0 + y*unit, y0 + siz, !gs.fColor);
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
            DrawAspect(k + (NCheckEclipseAny(ig, k, jg, NULL) >
              etNone)*cAspect2, x0 + gi.xTurtle, y0 + gi.yTurtle);
          }

        // If this is a midpoint cell, draw glyph of sign of midpoint.
        } else if (gs.fAlt ? x < y : x > y) {
          DrawColor(c = kSignB(grid->n[ig][jg]));
          DrawSign(grid->n[ig][jg], x0 + gi.xTurtle, y0 + gi.yTurtle);

        // For cells on main diagonal, draw glyph of planet.
        } else {
          if (gs.fLabelAsp) {
            DrawColor(kDkBlueB);
            DrawBlock(x0 + (x-1)*unit+1, y0 + (y-1)*unit+1,
              x0 + x*unit-1, y0 + y*unit-1);
          }
          DrawColor(gi.kiLite);
          DrawEdge(x0 + (x-1)*unit, y0 + (y-1)*unit, x0 + x*unit, y0 + y*unit);
          DrawObject(i, x0 + gi.xTurtle, y0 + gi.yTurtle);
        }

        // When the scale size is 300+, can print text in each cell.
        if (nScale > 2 && gs.fLabel) {

          // For the aspect portion, print the orb in degrees and minutes.
          if (x != y)
            c = FormatGridCell(sz, ig, jg, 1 + (gs.fAlt ? x < y : x > y),
              nScale > 3 && us.fSeconds);

          // For the main diagonal, print degree and sign of each planet.
          else
            c = FormatGridCell(sz, ig, jg, 0, nScale > 3 && us.fSeconds);
          if (c >= 0)
            DrawColor(c);
          DrawSz(sz, x0 + x*unit-unit/2, y0 + y*unit-3*gi.nScaleT, dtBottom);
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
    if (us.fRefract)
      lat = SwissRefract(lat);
    PlotHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
  } else {
    lon = rDegMax - lon;
    CoorXform(&lon, &lat, Lat - rDegQuad);
    lon = Mod(cp0.lonMC - lon + rDegQuad);
    EquToHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
  }
}

void EquToHorizon(real lon, real lat, int x1, int y1, int xs, int ys,
  int *xp, int *yp)
{
  if (!gs.fEcliptic) {
    lon = Mod(cp0.lonMC - lon + rDegQuad);
    EquToLocal(&lon, &lat, rDegQuad - Lat);
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

void EquToHorizon2(real lon, real lat, int x1, int y1, int xs, int ys,
  int *xp, int *yp, flag fFlip)
{
  if (!fFlip)
    EquToHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
  else {
    EquToEcl(&lon, &lat);
    EclToHorizon(rDegMax - lon, lat, x1, y1, xs, ys, xp, yp);
  }
}


#define NDashAspect(i, j, asp, orb) (gs.nDashMax >= 0 ? \
  NAbs((int)(orb*3600.0)) / (60*60*2) : NAbs((int)(orb*3600.0)) * \
  NAbs(gs.nDashMax) / (int)(GetOrb(i, j, asp)*3600.0))

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
  x1 = y1 = unit; x2 = gs.xWin-1-unit; y2 = gs.yWin-1-unit;
  xs = x2-x1; ys = y2-y1; cx = (x1+x2)/2; cy = (y1+y2)/2;

  // Calculate the local horizon coordinates of each planet. First convert
  // zodiac position and declination to zenith longitude and latitude.

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
      EquToHorizon2((real)(nDegMax-m1), (real)(90-n1), x1, y1, xs, ys,
        &xp, &yp, fFlip);
      if (i <= 0) {
        DrawColor(kPurpleB);
        EquToHorizon2((real)(nDegMax-m2), (real)(90-n2), x1, y1, xs, ys,
          &xpT, &ypT, fFlip);
        DrawWrap(xp, yp, xpT, ypT, x1, x2);
      } else {
        DrawColor(gi.kiGray);
        DrawSz(szCnstlAbbrev[i], xp, yp, dtCent | dtScale2);
      }
    }
  }
#endif

  // Draw zodiac sign boundary wedges.
  if (us.fIndian) {
    if (!gs.fColorSign)
      DrawColor(kDkBlueB);
    for (i = 0; i < nDegMax; i++) {
      if (gs.fColorSign && i%30 == 0) {
        k = i/30 + 1;
        DrawColor(kSignB(!fFlip ? k : cSign+1 - k));
      }
      EclToHorizon((real)i, 0.0, x1, y1, xs, ys, &xp, &yp);
      DrawPoint(xp, yp);
    }
    for (i = 0; i < nDegMax; i += 30) {
      if (gs.fColorSign) {
        k = i/30 + 1;
        DrawColor(kSignB(!fFlip ? k : Mod12(cSign+2 - k)));
      }
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
          DrawColor(kSignB(!fFlip ? i : cSign+1 - i));
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
  if (!(us.fIndian && gs.fEcliptic)) {
    DrawColor(gi.kiGray);
    if (!(gs.fHouseExtra && fHouse3D && !gs.fEcliptic))
      DrawDash(cx, y1, cx, y2, 1);
    DrawDash((cx+x1)/2, y1, (cx+x1)/2, y2, 1);
    DrawDash((cx+x2)/2, y1, (cx+x2)/2, y2, 1);
  }
  DrawColor(gi.kiOn);
  DrawEdge(x1, y1, x2, y2);
  if (!(us.fIndian && gs.fEcliptic)) {
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
    if (i%90 == 0) {
      k = !fFlip ? i : nDegMax-i;
      if (!gs.fEcliptic)
        sprintf(sz, "%c", *rgszDir[k/90 & 3]);
      else if (us.nDegForm == dfZod)
        sprintf(sz, "%3.3s", szSignName[Mod12((k / 90)*3 + 1)]);
      else if (us.nDegForm == dfHM)
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

  // Draw lines connecting planets which have aspects between them.
  if (gs.fLabelCity) {
    if (!FCreateGrid(fFalse))
      return;
    k = gi.nScale;
    gi.nScale = gi.nScaleTextT;
    for (j = is.nObj; j >= 1; j--)
      for (i = j-1; i >= 0; i--)
        if (grid->n[i][j] && FProper(i) && FProper(j)) {
          DrawColor(kAspB[grid->n[i][j]]);
          DrawDash(rgod[i].x, rgod[i].y, rgod[j].x, rgod[j].y,
            NDashAspect(i, j, grid->n[i][j], grid->v[i][j]));
          if (gs.fLabelAsp)
            DrawAspect(grid->n[i][j],
              (rgod[i].x + rgod[j].x) >> 1, (rgod[i].y + rgod[j].y) >> 1);
        }
    gi.nScale = k;
  }

  // Draw planet glyphs, and spots for actual planet locations.
  DrawObjects(rgod, is.nObj+1, unit);
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
  if (!gs.fEcliptic) {
    if (us.fRefract)
      lat = SwissRefract(lat);
    PlotHorizonSky(lon, lat, pcr, xp, yp);
  } else {
    lon = rDegMax - lon;
    CoorXform(&lon, &lat, Lat - rDegQuad);
    lon = Mod(cp0.lonMC - lon + rDegQuad);
    EquToHorizonSky(lon, lat, pcr, xp, yp);
  }
}

void EquToHorizonSky(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp)
{
  if (!gs.fEcliptic) {
    lon = Mod(cp0.lonMC - lon + rDegQuad);
    EquToLocal(&lon, &lat, rDegQuad - Lat);
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

void EquToHorizonSky2(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp,
  flag fFlip)
{
  if (!fFlip)
    EquToHorizonSky(lon, lat, pcr, xp, yp);
  else {
    EquToEcl(&lon, &lat);
    EclToHorizonSky(rDegMax - lon, lat, pcr, xp, yp);
  }
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
  x1 = y1 = unit; x2 = gs.xWin-1-unit; y2 = gs.yWin-1-unit;
  xs = x2-x1; ys = y2-y1; cx = (x1+x2)/2; cy = (y1+y2)/2;
  rx = (int)((real)xs/2.0/rSqr2); ry = (int)((real)ys/2.0/rSqr2);
  cr.xc = cx; cr.yc = cy; cr.xr = rx; cr.yr = ry;

  // Calculate the local horizon coordinates of each planet. First convert
  // zodiac position and declination to zenith longitude and latitude.

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
      EquToHorizonSky2((real)(nDegMax-m1), (real)(90-n1), &cr, &xp, &yp,
        fFlip);
      if (i <= 0) {
        DrawColor(kPurpleB);
        EquToHorizonSky2((real)(nDegMax-m2), (real)(90-n2), &cr, &xpT, &ypT,
          fFlip);
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
  if (us.fIndian) {
    if (!gs.fColorSign)
      DrawColor(kDkBlueB);
    for (i = 0; i < nDegMax; i++) {
      if (gs.fColorSign && i%30 == 0) {
        k = i/30 + 1;
        DrawColor(kSignB(!fFlip ? k : cSign+1 - k));
      }
      EclToHorizonSky((real)i, 0.0, &cr, &xp, &yp);
      DrawPoint(xp, yp);
    }
    for (i = 0; i < nDegMax; i += 30) {
      if (gs.fColorSign) {
        k = i/30 + 1;
        DrawColor(kSignB(!fFlip ? k : Mod12(cSign+2 - k)));
      }
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
          DrawColor(kSignB(!fFlip ? i : cSign+1 - i));
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
  i = gi.nScaleT;
  DrawSz(!gs.fEcliptic ? "N" : (!fFlip ? "Can" : "Cap"), cx, y1-2*i,
    dtBottom | dtScale2);
  DrawSz(!gs.fEcliptic ? "E" : "r", x1/2, cy+2*i, dtCent | dtScale2);
  DrawSz(!gs.fEcliptic ? "W" : "i", (gs.xWin+x2)/2, cy+2*i, dtCent | dtScale2);
  if (gs.fEcliptic) {
    DrawSz("A", x1/2, cy+2*i-yFontT, dtCent | dtScale2);
    DrawSz("i", x1/2, cy+2*i+yFontT, dtCent | dtScale2);
    DrawSz("L", (gs.xWin+x2)/2, cy+2*i-yFontT, dtCent | dtScale2);
    DrawSz("b", (gs.xWin+x2)/2, cy+2*i+yFontT, dtCent | dtScale2);
  }
  if (!gs.fText)
    DrawSz(!gs.fEcliptic ? "S" : (!fFlip ? "Cap" : "Can"), cx, gs.yWin-3*i,
      dtBottom | dtScale2);
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

  // Draw lines connecting planets which have aspects between them.
  if (gs.fLabelCity) {
    if (!FCreateGrid(fFalse))
      return;
    k = gi.nScale;
    gi.nScale = gi.nScaleTextT;
    for (j = is.nObj; j >= 1; j--)
      for (i = j-1; i >= 0; i--)
        if (grid->n[i][j] && FProper(i) && FProper(j)) {
          DrawColor(kAspB[grid->n[i][j]]);
          DrawDash(rgod[i].x, rgod[i].y, rgod[j].x, rgod[j].y,
            NDashAspect(i, j, grid->n[i][j], grid->v[i][j]));
          if (gs.fLabelAsp)
            DrawAspect(grid->n[i][j],
              (rgod[i].x + rgod[j].x) >> 1, (rgod[i].y + rgod[j].y) >> 1);
        }
    gi.nScale = k;
  }

  // Draw planet glyphs, and spots for actual planet locations.
  DrawObjects(rgod, is.nObj+1, unit);
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
    if (us.fRefract)
      lat = SwissRefract(lat);
    PlotTelescope(lon, lat, pte, xp, yp, xr, yr);
  } else {
    lon = rDegMax - lon;
    CoorXform(&lon, &lat, Lat - rDegQuad);
    lon = Mod(cp0.lonMC - lon + rDegQuad);
    EquToTelescope(lon, lat, pte, xp, yp, xr, yr);
  }
}

void EquToTelescope(real lon, real lat, TELE *pte,
  int *xp, int *yp, real *xr, real *yr)
{
  if (!gs.fEcliptic) {
    lon = Mod(cp0.lonMC - lon + rDegQuad);
    EquToLocal(&lon, &lat, rDegQuad - Lat);
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

void EquToTelescope2(real lon, real lat, TELE *pte,
  int *xp, int *yp, real *xr, real *yr, flag fFlip)
{
  if (!fFlip)
    EquToTelescope(lon, lat, pte, xp, yp, xr, yr);
  else {
    EquToEcl(&lon, &lat);
    EclToTelescope(rDegMax - lon, lat, pte, xp, yp, xr, yr);
  }
}


#define N012(n, n0, n1, n2) ((n) <= 0 ? (n0) : ((n) == 1 ? (n1) : (n2)))

// Draw a chart depicting a telescope view of the sky, showing planetary disks
// (including moon phases and rings), as done with the -XZ switch. This is
// similar to a zoomed in version of the -Z -X switch local horizon chart.

void XChartTelescope()
{
  char sz[cchSzDef], szT[cchSzDef], *pch;
  int rgobj[objMax+1], xunit, yunit, x1, y1, x2, y2, xs, ys, xc, yc, xp, yp,
    xT, yT, xd, yd, i, j, k, i0, i1, i2, iEar, iMoo, dx, dy = 0, nEclipse,
    zLimit, nShowMinute, nSav;
  real rglen[objMax+1], lonH, latH, xBase, yBase, xScale, yScale,
    xBase2, yBase2, xScale2, yScale2, xi, yi, diam, radi, rRatio, len, ang,
    xo, yo, xr, yr, zr, xrSun, yrSun, xrT, yrT,
    radiS, radiE, radiM, radiU, radiP, lenS, lenM, theta, rPct;
  flag fFlip = gs.fEcliptic && us.rHarmonic < 0.0, fShowLabel, fShowUmbra,
    fSav;
  TELE te;
#ifdef CONSTEL
  int m1, n1, m2, n2;
#endif
#ifdef SWISS
  ES es, *pes1, *pes2;
#endif
  // Variables for Saturn's or other planet's rings
  real radi2, len2, ang2, theta2, dRing, xr2, yr2, radi3;
  int iRng, iJup, iSat, iUra, iNep, xT2, yT2, xd2, yd2, xp2, yp2, xT3, yT3,
    xd3, yd3;
  PT3R ptSat, ptCen, vS2C, vCross, vBest, vUp, vLeft;

  // Initialize variables.
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
  iNep = !ignore[oNeC] ? oNeC :
#ifdef SWISS
    (!ignore[oVul] && rgTypSwiss[oVul - custLo] == 3 &&
    rgObjSwiss[oVul - custLo] == 899 ? oVul : oNep);
#else
    oNep;
#endif
  iJup = !ignore[oJuC] ? oJuC :
#ifdef SWISS
    (!ignore[oVul] && rgTypSwiss[oVul - custLo] == 3 &&
    rgObjSwiss[oVul - custLo] == 599 ? oVul : oJup);
#else
    oJup;
#endif
  iEar = oEar; iMoo = oMoo; fShowUmbra = fFalse;
  if (us.objCenter == oEar) {
    fShowUmbra = fTrue;
  } else if (ObjOrbit(gs.objTrack) == us.objCenter) {
    iEar = us.objCenter; iMoo = gs.objTrack; fShowUmbra = fTrue;
  }
  fSav = us.fRefract; us.fRefract = fFalse;

  for (nShowMinute = 0; nShowMinute <= 2; nShowMinute++) {
    xunit = xFontT * N012(nShowMinute, 4, 7, 10);
    yunit = 6*gi.nScaleTextT2;
    x1 = xunit; y1 = yunit; x2 = gs.xWin-1-xunit; y2 = gs.yWin-1-yunit;
    xs = x2-x1; ys = y2-y1; xc = (x1+x2)/2; yc = (y1+y2)/2;

    // Determine point to center chart around.
    if (gs.objTrack >= 0) {
      xBase = planet[gs.objTrack], yBase = planetalt[gs.objTrack];
      if (!gs.fEcliptic) {
        xBase = Tropical(xBase);
        EclToEqu(&xBase, &yBase);
        xBase = Mod(cp0.lonMC - xBase + rDegQuad);
        EquToLocal(&xBase, &yBase, rDegQuad - Lat);
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
    if (N012(nShowMinute,
      xi >= 2.0 || yi >= 2.0, xi >= 2.0/60.0 || yi >= 2.0/60.0, fTrue))
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
  for (i = 0; i < nShowMinute; i++) {
    xi      *= 60.0; yi      *= 60.0;
    xBase2  *= 60.0; yBase2  *= 60.0;
    xScale2 /= 60.0; yScale2 /= 60.0;
  }
  zLimit = N012(nShowMinute, 90, 90*60, 90*60*60);
  if (gs.fLabelCity && (nShowMinute || (yScale >= 1.0 && xScale >= 1.0))) {
    DrawColor(gi.kiGray);
    for (j = (int)(yBase2 - yi); j <= (int)(yBase2 + yi); j++) {
      if (!FBetween(j, -zLimit, zLimit))
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
      EquToTelescope2((real)(nDegMax-m1), (real)(90-n1), &te, &xp, &yp,
        &xr, &yr, fFlip);
      if (i <= 0) {
        DrawColor(kPurpleB);
        EquToTelescope2((real)(nDegMax-m2), (real)(90-n2), &te, &xT, &yT,
          &xr, &yr, fFlip);
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
  if (us.fIndian) {
    if (!gs.fColorSign)
      DrawColor(kMaroonB);
    for (i = 0; i <= nDegMax; i++) {
      if (gs.fColorSign && i%30 == 1) {
        j = i/30 + 1;
        DrawColor(kSignB(!fFlip ? j : Mod12(cSign+1 - j)));
      }
      EclToTelescope((real)i, 0.0, &te, &xp, &yp, &xr, &yr);
      if (i > 0 && NAbs(xp - xT) < dx)
        DrawClip(xT, yT, xp, yp, x1, y1, x2, y2, 0);
      xT = xp; yT = yp;
    }
    for (k = 1; k <= cSign; k++) {
      j = (!fFlip ? k : Mod12(cSign+2 - k));
      if (gs.fColorSign)
        DrawColor(kSignB(j));
      len = (real)((k-1)*30);
      for (i = -90; i <= 90; i++) {
        EclToTelescope(len, (real)i, &te, &xp, &yp, &xr, &yr);
        if (i > -90 && NAbs(xp - xT) < dx)
          if (FDrawClip(xT, yT, xp, yp, x1, y1, x2, y2, 0, &xd, &yd))
            DrawSign(j, xd + ((xd == x1)-(xd == x2))*gi.nScale*6,
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
  us.fRefract = fSav;

  // Draw planet disks.
  for (k = 0; k <= 1; k++) {
    for (i0 = 0; i0 <= cObj+1; i0++) {
      i1 = rgobj[i0];
      i = (i1 <= cObj ? i1 : iEar);
      // Only draw Earth's umbra disk when doing a geocentric chart.
      // Also allow other planet's umbra when focused upon its moon.
      if (!FProper(i) || (!fShowUmbra && (i == us.objCenter || i1 > cObj)))
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
            iRng = (i == iSat ? oSat : (i == iUra ? oUra : (i == iNep ? oNep :
              (i == iJup ? oJup : (i == oHau ? i : -1)))));
            if (iRng >= 0) {
              radi2 = radi * (rgrObjRing[IObjRing(iRng)][0] * 2.0) / diam;
              xd2 = (int)(radi2 * 2.0); xT2 = (int)(xr - radi2 + rRound);
              radi2 *= rRatio;
              if (rgrObjRing[IObjRing(iRng)][1] > 0.0) {
                radi3 = radi * (rgrObjRing[IObjRing(iRng)][1] * 2.0) / diam;
                xd3 = (int)(radi3 * 2.0); xT3 = (int)(xr - radi3 + rRound);
                radi3 *= rRatio;
              } else
                xd3 = 0;
              ptSat = space[i];
#if FALSE
              // Calculate plane of Haumea's rings based on its moons.
              static PT3R vSum = {0.0, 0.0, 0.0};
              static PT3R vSav[6];
              static int cv = 0;
              PT3R v1, v2;
              PtVec(v1, ptSat, space[custLo]);
              if (cv < 6)
                vSav[cv++] = v1;
              else {
                for (i2 = 0; i2 < 5; i2++) {
                  v1 = vSav[i2]; v2 = vSav[i2+1];
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
                }
              }
#endif
              vCross = rgvObjRing[IObjRing(iRng)];
              // Adjust ring vector appropriately if in sidereal zodiac.
              if (is.rSid != 0.0) {
                dRing = RLength2(vCross.x, vCross.y);
                ang2 = RAngleD(vCross.x, vCross.y) + is.rSid;
                vCross.x = RCosD(ang2) * dRing;
                vCross.y = RSinD(ang2) * dRing;
              }
              // Calculate tilt of Saturn's rings up or down.
              ptCen = space[us.objCenter];
              PtVec(vS2C, ptSat, ptCen);
              ang2 = VAngleD(&vCross, &vS2C);
              dRing = (ang2 >= 0.0 ? rDegHalf : 0.0);
              radi2 *= RCosD(ang2);
              if (xd3 > 0)
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
                if (ang > rDegHalf)
                  ang -= rDegMax;
              }
              // Draw Saturn's rings.
              yd2 = (int)(radi2 * 2.0); yT2 = (int)(yr - radi2 + rRound);
              DrawColor(kDkGreenB);
              DrawArc(xT2, yT2, xT2 + xd2, yT2 + yd2,
                ang, rDegHalf - dRing, rDegMax - dRing);
              if (xd3 > 0) {
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
            if (iRng >= 0) {
              DrawColor(kDkGreenB);
              DrawArc(xT2, yT2, xT2 + xd2, yT2 + yd2,
                ang, dRing, dRing + rDegHalf);
              if (xd3 > 0)
                DrawArc(xT3, yT3, xT3 + xd3, yT3 + yd3,
                  ang, dRing, dRing + rDegHalf);
              if (gs.fText) {
                sprintf(szT, "%%s's rings: %%.%df%%%c Tilt, "
                  "%%.%df%%%c Rotation", us.fSeconds*2, chDegL,
                  us.fSeconds*2, chDegL);
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
              if (!FProper(i2) || i == us.objCenter || i2 == us.objCenter)
                continue;
              fSav = us.fEclipseAny; us.fEclipseAny = fFalse;
              nEclipse = NCheckEclipse(i2, i, &rPct);
              us.fEclipseAny = fSav;
            } else {
              if (!(i == iMoo && us.objCenter == iEar && !FIgnore(iEar)))
                continue;
              nEclipse = NCheckEclipseLunar(iEar, iMoo, oSun, &rPct);
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
  k = (xFontT * N012(nShowMinute, 4, 7, 10)) >> 1;
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
    if (nShowMinute <= 0)
      sprintf(sz, "%s%d", j > 0 ? "+" : "", j);
    else if (nShowMinute == 1)
      sprintf(sz, "%c%d%c%02d", j > 0 ? '+' : '-', NAbs(j)/60, chDegL,
        NAbs(j)%60);
    else
      sprintf(sz, "%c%d%c%02d'%02d", j > 0 ? '+' : '-', NAbs(j)/3600, chDegL,
        NAbs(j)/60%60, NAbs(j)%60);
    DrawSz(sz, k, yp + 2*gi.nScaleTextT, dtCent | dtScale2);
    DrawSz(sz, gs.xWin-gi.nScaleT - k, yp + 2*gi.nScaleTextT,
      dtCent | dtScale2);
  }

  // Label degree points on horizontal axis.
  j = us.nDegForm == dfNak ? 8 + (nShowMinute > 0)*2 : N012(nShowMinute,
    (us.nDegForm == dfZod ? 5 : (us.nDegForm == dfHM ? 7 : 4)),
    (us.nDegForm == dfZod ? 7 : (us.nDegForm == dfHM ? 11 : 6)),
    (us.nDegForm == dfZod ? 10 : (us.nDegForm == dfHM ? 11 : 9)));
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
    is.fSeconds =
      ((us.nDegForm == dfHM  || us.nDegForm == dfNak) && nShowMinute > 0) ||
      ((us.nDegForm == dfZod || us.nDegForm == df360) && nShowMinute > 1);
    us.nDegForm = (gs.fEcliptic ? us.nDegForm : df360);
    ang = (gs.fEcliptic ? (real)i / N012(nShowMinute, 1.0, 60.0, 3600.0) :
      rDegQuad - (real)i / N012(nShowMinute, 1.0, 60.0, 3600.0));
    if (fFlip)
      ang = rDegMax - ang;
    ang = Mod(ang + rSmall);
    if (us.nDegForm != df360) {
      if (gs.fColorSign)
        DrawColor(kSignB(SFromZ(ang)));
      sprintf(sz, "%s", SzZodiac(ang));
    } else {
      us.nDegForm = dfZod;
      sprintf(sz, "%s", SzDegree(ang));
      us.nDegForm = df360;
    }
    if (us.nDegForm == dfZod)
      sz[N012(nShowMinute, 5, 7, 10)] = chNull;
    else if (us.nDegForm == df360)
      sz[N012(nShowMinute, 4, 6, 9)] = chNull;
    else if (us.nDegForm == dfNak)
      sz[nShowMinute <= 0 ? 7 : 9] = chNull;
    us.nDegForm = nSav; is.fSeconds = fSav;
    pch = sz + (sz[0] == ' ');
    DrawSz(pch, xp, 2*gi.nScaleTextT, dtCent | dtTop | dtScale2);
    if (!gs.fText)
      DrawSz(pch, xp, gs.yWin - gi.nScaleT - 2*gi.nScaleTextT,
        dtCent | dtBottom | dtScale2);
    if (gs.fColorSign)
      DrawColor(gi.kiLite);
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
    m1, n1, m2, n2, i, j, k, zLimit, nShowMinute;
  real xBase, yBase, xScale, yScale, xBase2, yBase2, xScale2, yScale2,
    xr, yr, xi, yi, lon, lat, len;
  flag fShowLabel, fDidBitmap;
  TELE te;
  static real lonPrev = rLarge, latPrev = rLarge;
  static int objPrev = nLarge;

  // Initialize variables.
  for (nShowMinute = 0; nShowMinute <= 2; nShowMinute++) {
    xunit = xFontT * N012(nShowMinute, 4, 6, 9);
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
      xBase = Mod(xBase - cp0.lonMC - Lon + rDegHalf);
    } else {
      if (Lon != lonPrev || Lat != latPrev) {
        lonPrev = Lon; latPrev = Lat;
        gs.rRot = Mod(rDegHalf - lonPrev); gs.rTilt = latPrev;
      }
      xBase = gs.rRot; yBase = gs.rTilt;
    }

    // Determine degrees the chart area covers.
    len = (real)Min(xs, ys);
    yScale = gs.rspace > 0.0 ? len / Max(Min(gs.rspace, 90.0), 1.0/60.0) :
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
    if (N012(nShowMinute,
      xi >= 2.0 || yi >= 2.0, xi >= 2.0/60.0 || yi >= 2.0/60.0, fTrue))
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
  for (i = 0; i < nShowMinute; i++) {
    xi      *= 60.0; yi      *= 60.0;
    xBase2  *= 60.0; yBase2  *= 60.0;
    xScale2 /= 60.0; yScale2 /= 60.0;
  }
  zLimit = N012(nShowMinute, 90, 90*60, 90*60*60);
  if (gs.fHouseExtra && (nShowMinute || (yScale2 >= 1.0 && xScale2 >= 1.0))) {
    DrawColor(gi.kiGray);
    for (j = (int)(yBase2 - yi); j <= (int)(yBase2 + yi); j++) {
      if (!FBetween(j, -zLimit, zLimit))
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
      if (gs.fLabelAsp) {
        j = KiCity(i);
        if (j < 0)
          continue;
        DrawColor(j);
      }
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
    lon = Mod(lon - cp0.lonMC - Lon + rDegHalf);
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
    if (us.fIndian)
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
  k = (xFontT * N012(nShowMinute, 4, 6, 9)) >> 1;
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
    if (nShowMinute <= 0)
      sprintf(sz, "%d%c", NAbs(j), j >= 0 ? 'N' : 'S');
    else if (nShowMinute == 1)
      sprintf(sz, "%d%c%02d", NAbs(j)/60, j >= 0 ? 'N' : 'S', NAbs(j)%60);
    else
      sprintf(sz, "%d%c%02d'%02d", NAbs(j)/3600, j >= 0 ? 'N' : 'S',
        NAbs(j)/60%60, NAbs(j)%60);
    DrawSz(sz, k, yp + 2*gi.nScaleTextT, dtCent | dtScale2);
    DrawSz(sz, gs.xWin-1 - k, yp + 2*gi.nScaleTextT, dtCent | dtScale2);
  }

  // Label degree points on horizontal axis.
  zLimit = nDegHalf * N012(nShowMinute, 1, 60, 60*60);
  j = N012(nShowMinute, 5, 7, 9) * xFontT / Max((int)xScale2, 1) + 1;
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
    if (nShowMinute <= 0)
      sprintf(sz, "%d%c", NAbs(k), k >= 0 ? 'W' : 'E');
    else if (nShowMinute == 1)
      sprintf(sz, "%d%c%02d", NAbs(k)/60, k >= 0 ? 'W' : 'E', NAbs(k)%60);
    else
      sprintf(sz, "%d%c%02d'%02d", NAbs(k)/3600, k >= 0 ? 'W' : 'E',
        NAbs(k)/60%60, NAbs(k)%60);
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
  x1 = y1 = unit; x2 = gs.xWin-1-unit; y2 = gs.yWin-1-unit;

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
    if (us.fStar || gs.fAllStar) {
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

        // Draw rings around Saturn or other planet.
        l = FBetween(i, oJuC, oNeC) ? i - oJuC + oJup :
          (FBetween(i, oJup, oNep) && ignore[i + oJuC - oJup] ? i :
          (i == oHau ? i : -1));
        if (l >= 0) {
          l = IObjRing(l);
          j = (int)(rgrObjRing[l][0] / rAUToKm * sx);
          k = (int)(rgrObjRing[l][0] / rAUToKm * sy);
          DrawCircle(rgod[i].x, rgod[i].y, j, k);
          if (rgrObjRing[l][1] > 0.0) {
            j = (int)(rgrObjRing[l][1] / rAUToKm * sx);
            k = (int)(rgrObjRing[l][1] / rAUToKm * sy);
            DrawCircle(rgod[i].x, rgod[i].y, j, k);
          }
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
  if (gs.fEquator && us.nAsp > 0) {
    if (!FCreateGrid(fFalse))
      return;
    nSav = gi.nScale;
    gi.nScale = gi.nScaleTextT;
    for (j = oNorm; j >= 1; j--)
      for (i = j-1; i >= 0; i--)
        if (grid->n[i][j] && FProper(i) && FProper(j)) {
          DrawColor(kAspB[grid->n[i][j]]);
          DrawClip(rgod[i].x, rgod[i].y, rgod[j].x, rgod[j].y, x1, y1, x2, y2,
            NDashAspect(i, j, grid->n[i][j], grid->v[i][j]));
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
      xp = es.pt.x / rLYToAU; yp = es.pt.y / rLYToAU;
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
      xp = pes1->pt.x / rLYToAU; yp = pes1->pt.y / rLYToAU;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, NULL, sz, -1, NULL);
      j = cx-(int)(xp*sx); k = cy+(int)(yp*sy);
      xp = pes2->pt.x / rLYToAU; yp = pes2->pt.y / rLYToAU;
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
      xp = es.pt.x; yp = es.pt.y;
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
  flag fOff;

  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;
  cx = gs.xWin/2 - 1; cy = gs.yWin/2 - 1;
  unitx = (real)cx; unity = (real)cy;
  yi = !us.fIndian ? 1 : -1;

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
    fOff = DrawFillWheel(cx+POINT1(unitx, 0.88, PX((real)(i*10+175))),
      cy+POINT1(unity, 0.88, PY((real)(i*10+175)))*yi+gi.nScale, i, 2);
    if (nTrans >= 128)
      DrawColor(fOff ? gi.kiOff : gi.kiOn);
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
    if (!us.fIndian)
      temp = Mod(rDegHalf - planet[i]);
    else
      temp = Mod(rDegHalf + planet[i]);
    xplanet[i] = temp;
  }

  // For each planet, draw a small dot indicating where it is, and then a line
  // from that point to the planet's glyph.
  DrawRing(1, 1, xplanet, symbol, cx, cy, 0.0, 0.0, 0.0,
    0.65, 0.67, 0.71, 0.75, 1.0);
  FProcessCommandLine(szWheelX[0]);

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


// Draw a wheel chart but labeled with degrees instead of zodiac signs, as
// done when the -m is combined with the -X switch. Inside the wheel, may draw
// an arrow with midpoint lines across it, as done with N-degree dial charts.

void XChartMidpoint()
{
  real xplanet[objMax], symbol[objMax], rOrb[MAXINDAY], rDiff[MAXINDAY];
  char sz[cchSzDef];
  int obj1[MAXINDAY], obj2[MAXINDAY], count = 0, cx, cy, xi, x, y, i, j,
    dtMid = (gs.nScaleText <= gs.nScale ? dtScale2 : dtScale), nDash;
  real rHarmonic, unitx, unity, px, py, rxi, rBase, temp;
  flag fLabelOrb = !gs.fLabelCity, f;

  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;
  cx = gs.xWin/2 - 1; cy = gs.yWin/2 - 1;
  unitx = (real)cx; unity = (real)cy;
  xi = !us.fIndian ? 1 : -1;
  rxi = !us.fIndian ? 1.0 : -1.0;
  rHarmonic = us.rHarmonic != 0 ? RAbs(us.rHarmonic) : rSmall;

  // Draw circles and radial lines delineating the degrees.
  DrawColor(gi.kiOn);
  DrawCircle(cx, cy, (int)(unitx*0.95+rRound), (int)(unity*0.95+rRound));
  DrawCircle(cx, cy, (int)(unitx*0.81+rRound), (int)(unity*0.81+rRound));
  if (gs.nDecaFill > 0) {
    DrawColor(kGreenB);
    DrawFill(cx, cy-POINT0(unity, 0.88, 1.0),
      KvBlend(rgbbmp[gi.kiOff], rgbbmp[kGreenB], gs.rBackPct/100.0));
    DrawColor(gi.kiOn);
  }
  j = rHarmonic >= 1.0 ? 1 : (int)(1.0/rHarmonic);
  for (i = (int)(rDegMax / rHarmonic / (real)j - rSmall)*j;
    i >= 0; i -= j) {
    temp = 270.0-(real)(i*xi)*rHarmonic;
    px = PX(temp); py = PY(temp);
    temp = 0.81 + (i%5 == 0 ? 0.04 : 0.02);
    DrawLine(cx+POINT1(unitx, 0.81, px), cy+POINT1(unity, 0.81, py),
      cx+POINT2(unitx, temp, px), cy+POINT2(unity, temp, py));
  }
  j = rHarmonic >= 30 ? 1 : (rHarmonic >= 5.0 ? 5 :
    (rHarmonic >= 1.0 ? 10 : (int)(10.0/rHarmonic)));
  for (i = (int)(rDegMax / rHarmonic / (real)j - rSmall)*j;
    i >= 0; i -= j) {
    if (i > 0 || us.rHarmonic == 0)
      sprintf(sz, "%d", i);
    else
      FormatR(sz, rDegMax / rHarmonic, -1);
    temp = 270.0-(real)(i*xi)*rHarmonic;
    DrawSz(sz, cx+POINT1(unitx, 0.90, PX(temp)),
      cy+POINT1(unity, 0.90, PY(temp))+gi.nScale, dtCent | dtScale);
  }

  // Figure out where to put planet glyphs.
  for (i = 0; i <= is.nObj; i++) {
    temp = Mod(270.0 - planet[i]*rxi);
    xplanet[i] = temp;
  }

  // For each planet, draw a small dot indicating where it is, and then a line
  // from that point to the planet's glyph.
  DrawRing(1, 1, xplanet, symbol, cx, cy, 0.0, 0.0, 0.0,
    0.65, 0.67, 0.71, 0.75, 1.0);
  FProcessCommandLine(szWheelX[0]);

  // Draw lines connecting planets.
  if (!gs.fEquator) {
    // Draw dial lines connecting planets which have focus as their midpoint.
    rBase = gs.objTrack >= 0 ? planet[gs.objTrack] : gs.rRot;
    temp = 270.0 - rBase*rxi;
    px = PX(temp); py = PY(temp);
    x = cx+POINT1(unitx, 0.63, px); y = cy+POINT1(unity, 0.63, py);
    DrawColor(kRedB);
    DrawLine(x, y, cx+POINT1(unitx, 0.63, -px), cy+POINT1(unity, 0.63, -py));
    for (i = -1; i <= 1; i += 2)
      DrawLine(x, y, cx+POINT1(unitx, 0.55, PX(temp-2.0*(real)i)),
        cy+POINT1(unity, 0.55, PY(temp-2.0*(real)i)));
    DrawColor(kMaroonB);
    for (j = is.nObj; j >= 1; j--) if (FProper(j))
      for (i = j-1; i >= 0; i--) if (FProper(i)) {
        px = MinDifference(rBase, planet[i]);
        py = MinDifference(rBase, planet[j]);
        if (RSgn(px) == RSgn(py))
          continue;
        if (us.fAspect3D) {
          px =  SphDistance(rBase, 0.0, planet[i], planetalt[i]);
          py = -SphDistance(rBase, 0.0, planet[j], planetalt[j]);
        }
        temp = RAbs(px + py) / 2.0;
        if (temp < rAspOrb[aCon]) {
          if ((gs.nDashMax >= 0) != gs.fAlt)
            nDash = (int)temp;
          else
            nDash = (int)(temp * 2.0 / rAspOrb[aCon] * NAbs(gs.nDashMax));
          DrawDash(cx+POINT1(unitx, 0.63, PX(270 - planet[i]*rxi)),
            cy+POINT1(unity, 0.63, PY(270.0 - planet[i]*rxi)),
            cx+POINT1(unitx, 0.63, PX(270.0 - planet[j]*rxi)),
            cy+POINT1(unity, 0.63, PY(270.0 - planet[j]*rxi)), nDash);
          if (count < MAXINDAY) {
            obj1[count] = i; obj2[count] = j;
            rOrb[count] = (px + py) / 2.0 / rHarmonic;
            rDiff[count] = (RAbs(px) + RAbs(py)) / 2.0 / rHarmonic;
            count++;
          }
        }
      }
  } else {
    // Draw lines connecting planets which have aspects between them.
    if (!FCreateGrid(fFalse))
      return;
    for (j = is.nObj; j >= 1; j--)
      for (i = j-1; i >= 0; i--)
        if (grid->n[i][j] && FProper(i) && FProper(j))
          DrawAspectLine(i, j, cx, cy, xplanet[i], xplanet[j], unitx, unity,
            0.63);
  }

  // List midpoints in middle of wheel.
  if (!gs.fEquator && !gs.fLabelAsp) {
    for (i = 1; i < count; i++) {
      j = i-1;
      loop {
        switch (us.nAspectSort) {
        case asO: f = obj1[j]*objMax+obj2[j]>obj1[j+1]*objMax+obj2[j+1]; break;
        case asP: f = obj2[j]*objMax+obj1[j]>obj2[j+1]*objMax+obj1[j+1]; break;
        case aso: f = RAbs(rOrb[j]) > RAbs(rOrb[j+1]); break;
        case asn: f = rOrb[j] > rOrb[j+1];             break;
        case asA: default: f = rDiff[j] > rDiff[j+1];  break;
        }
        if (!(j >= 0 && f))
          break;
        SwapN(obj1[j], obj1[j+1]); SwapN(obj2[j], obj2[j+1]);
        SwapR(&rOrb[j], &rOrb[j+1]); SwapR(&rDiff[j], &rDiff[j+1]);
        j--;
      }
    }
    i = (int)(unity * 2.0 * 0.65) / (gi.nScale*10);
    count = Min(count+1, i-1);
    for (i = -1; i < count-1; i++) {
      y = cy + (i*2+3 - count) * gi.nScale * 5;
      if (i < 0) {
        if (gs.objTrack >= 0)
          DrawObject(gs.objTrack, cx, y);
        else {
          DrawColor(kSignB(SFromZ(gs.rRot)));
          f = is.fSeconds; is.fSeconds &= (!fLabelOrb || gs.nScale >= 300);
          DrawSz(SzZodiac(gs.rRot), cx, y, dtCent);
          is.fSeconds = f;
        }
        if (fLabelOrb) {
          DrawColor(gi.kiOn);
          DrawSz("Orb",   cx + 12*gi.nScale +   xFontT, y, dtLeft | dtMid);
          DrawSz("Angle", cx - 12*gi.nScale - 6*xFontT, y, dtLeft | dtMid);
        }
        continue;
      }
      DrawObject(obj1[i], cx - 6*gi.nScale, y);
      DrawObject(obj2[i], cx + 6*gi.nScale, y);
      if (fLabelOrb) {
        DrawColor(gi.kiLite);
        DrawSz(rOrb[i] < 0.0 ? "-" : "+", cx + 12*gi.nScale + xFontT, y,
          dtLeft | dtMid);
        DrawSz(SzDegree2(RAbs(rOrb[i])),  cx + 12*gi.nScale + 2*xFontT, y,
          dtLeft | dtMid);
        DrawSz(SzDegree(rDiff[i]),
          cx - 12*gi.nScale - (8+is.fSeconds*3)*xFontT, y, dtLeft | dtMid);
      }
    }
  }

  DrawSidebar();
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
    if ((FThing(i) || gs.fAlt) && (!FIgnore(i) || (rgRules == rules ?
      FBetween(i, oSun, oMain) : (FBetween(i, oEar, oMain) || i == oVul))))
      obj[++oNum] = i;

  // Set up screen positions of the 10 planets for the wheel graphs.

  cx0 = gs.xWin >> 1; cy0 = gs.yWin >> 1;
  for (i = 1; i <= oNum; i++) {
    j = (gs.fHouseExtra ? 270 : 180) - (i-1)*(us.fIndian ? -360 : 360)/oNum;
    xCirc[i] = (real)cx0*0.4*RCosD((real)j);
    yCirc[i] = (real)cy0*0.4*RSinD((real)j);
  }

  // Loop over the two basic dispositor types: sign based and house based.

  for (xSub = 0; xSub <= 1; xSub++) {
    cx = xSub * cx0 + (cx0 >> 1);

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
      cy = ySub * cy0 + (cy0 >> 1);
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
          if (!gs.fLabelAsp && (j == i || dLev[i] < 2)) {
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
          if (!gs.fLabelAsp && dLev[i] < 2)
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
// ChartInDaySearch() and ChartTransitSearch() which computes aspect events,
// which is in turn called from XChartCalendar() which draws the graphic
// calendar chart.

flag DrawCalendarAspect(CONST InDayInfo *pid, int i, int iMax, int nVoid,
  int nEclipse)
{
  int x1, y1, x2, y2, asp, x, y, z, s1, s2, s3, nT, k,
    nScale = gi.nScaleTextT2;
  char sz[4], *szTime;
  real rDiff;
  flag fTrans = (pid->mon < 0), fFlip, fDoThin;

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
  if (x - z*(3+fTrans) - nScale*5/2 <= x1)
    return fFalse;

  // Get aspect event and time that it takes place.
  asp = pid->aspect;
  s1 = (int)pid->time/60;
  s2 = (int)pid->time-s1*60;
  s3 = us.fSeconds ? (int)(pid->time*60.0)-((s1*60+s2)*60) : -1;

  // Draw the aspect event itself.
  fFlip = !fTrans && asp >= aCon &&
    rgobjList2[pid->source] > rgobjList2[pid->dest];
  DrawObject(!fFlip ? pid->source : pid->dest, x - z*3, y);
  fDoThin = gs.fThick;
  if (fDoThin && asp < aCon)
    DrawThick(fFalse);
  if (asp >= aCon) {
    DrawColor(kAspB[asp]);
    DrawAspect(asp + (nEclipse > etNone)*cAspect2, x - z*2, y);
  } else if (asp == aSig || asp == aHou) {
    DrawColor(gi.kiOn);
    DrawTurtle(pid->ret1 >= 0.0 ? "NL4R4NG4H4" : "NR4L4NF4E4", x - z*2, y);
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
  } else if (asp == aNod) {
    DrawColor(gi.kiOn);
    DrawTurtle(pid->dest ? "NR3L3HU2ER6FBD4NHD2GL6H" : "BG4U8F8U8",
      x - z*2, y);
    DrawTurtle("BG4U8F8U8", x - z, y);
  } else if (asp == aDis) {
    DrawColor(gi.kiOn);
    DrawTurtle("BL4BU2R8BD4L8", x - z*2, y);
  }
  // Transit to natal events are prefixed with a "T" or "P".
  if (fTrans) {
    DrawColor(gi.kiLite);
    DrawTurtle(us.fProgress ? "NL4R3EU2HL7D8" : "ND4U4NL4R4", x - z*4, y);
  }
  if (fDoThin)
    DrawThick(fTrue);
  if (asp >= aCon || asp == aDis)
    DrawObject(!fFlip ? pid->dest : pid->source, x - z, y);
  else if (asp == aSig || asp == aDeg) {
    nT = (asp == aSig ? pid->dest : pid->dest / us.nSignDiv + 1);
    DrawColor(kSignB(nT));
    DrawSign(nT, x - z, y);
  } else if (asp == aHou) {
    DrawColor(kSignB(pid->dest));
    nT = us.fHouseAngle; us.fHouseAngle = fTrue;
    DrawObject(cuspLo-1 + pid->dest, x - z, y);
    us.fHouseAngle = nT;
  }

  // Draw the time that the aspect event takes place (if room).
  szTime = SzTime(s1, s2, s3);
  nT = x - z*(4+fTrans) - CchSz(szTime)*xFont2*nScale;
  if (nT <= x1)
    return fTrue;
  DrawColor(gi.kiGray);
  DrawSz(szTime, nT, y+nScale, dtLeft | dtScale2);

  // Draw extra information about the aspect event (if room).
  nT -= 4*xFont2*nScale;
  if (nT <= x1)
    return fTrue;
  if (us.nAppSep == 2 && asp != aCon && asp != aOpp) {
    rDiff = MinDifference(pid->pos2, pid->pos1);
    if (fTrans)
      k = (rDiff >= 0.0);
    else
      k = (RSgn2(pid->ret1 - pid->ret2) * rDiff >= 0.0);
    DrawColor(k ? gi.kiOn : gi.kiGray);
    DrawSz(k ? "wax" : "wan", nT, y+nScale, dtLeft | dtScale2);
    return fTrue;
  }
  if (fTrans) {
    if (pid->source == pid->dest && asp == aCon) {
      DrawColor(gi.kiOn);
      DrawSz("RET", nT, y+nScale, dtLeft | dtScale2);
    }
    return fTrue;
  }
  k = kSignB(SFromZ(planet[oMoo]));
  if (pid->source == oSun && pid->dest == oMoo &&
    (asp == aCon || asp == aOpp) && !us.fParallel) {
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

void DrawCalendar(int mon, int yea, int x1, int y1, int x2, int y2)
{
  char sz[cchSzDef];
  int rgz[(31+1)*2], day, cday, dayHi, cweek, xunit, yunit, xs, ys, x0, y0,
    x, y, s, nSav;
  CP cpSav;
  flag fSav;

  if (mon < mJan)
    mon = mJan;
  xs = x2 - x1; ys = y2 - y1;
  day = DayOfWeek(mon, 1, yea);    // Day of week of 1st of month.
  cday = DaysInMonth(mon, yea);    // Count of days in the month.
  dayHi = DayInMonth(mon, yea);    // Number of last day in the month.
  cweek = us.fCalendarYear ? 6 : (day + cday + 6) / 7;   // Week rows.
  xunit = xs/8;                    // Hor. pixel size of each day box.
  yunit = ys/(cweek+2);            // Ver. pixel size of each day box.
  x0 = x1 + (xs - xunit*7) / 2;    // Blank space to left of calendar.
  y0 = y1 + yunit*3/2;             // Blank space to top of calendar.

  // Print the month and year in big letters at top of chart.
  DrawColor(gi.kiOn);
  sprintf(sz, "%s, %d", szMonth[mon], yea);
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
  for (day = 1; day <= dayHi; day = AddDay(mon, day, yea, 1)) {
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
    fSav = gs.fLabel; gs.fLabel = fTrue;
    if (us.nRel >= rcNone) {
      nSav = Mon; Mon = mon;
      ChartInDaySearch(us.fProgress);
      Mon = MM = nSav;
    } else {
      nSav = MonT; MonT = mon; cpSav = cp1; cp0 = cp1;
      ChartTransitSearch(us.nRel == rcProgress);
      MonT = nSav; MM = Mon; cp1 = cpSav;
    }
    gs.fLabel = fSav;
    gi.rgzCalendar = NULL;
  }
  gi.nScale = s;
}


// Draw a graphical calendar on the screen for the chart month or entire year,
// as done when the -K or -Ky is combined with the -X switch.

void XChartCalendar()
{
  int xs, ys, xunit, yunit, x1, y1, x, y;
  flag fTransit = gs.fLabelAsp && us.nRel < rcNone;

  if (!us.fCalendarYear) {
    DrawCalendar(!fTransit ? Mon : MonT, !fTransit ? Yea : YeaT,
      0, 0, gs.xWin, gs.yWin);
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
      DrawCalendar(y * xs + x + 1, !fTransit ? Yea : YeaT, x1 + x*xunit,
        y1 + y*yunit, x1 + (x+1)*xunit, y1 + (y+1)*yunit);
    }
}


// Draw a chart showing the locations of planetary moons, showing geocentric
// and heliocentric views of moons from the front and from above, as done when
// the -8 is combined with the -X switch.

void XChartMoons()
{
  CP rgcp[2], *cp;
  ObjDraw rgod[cCust];
  char sz[cchSzDef], szCh[2];
  int objCenterSav = us.objCenter, cx0, cy0, xSub, ySub, cx, cy, xs, ys,
    m, i0, i, j, k, x, y, count, countOld;
  real rgrLenP[2][custHi+1], rgrLenM[2][cCust], rgrAngM[2][cCust],
    rgrLenZ[cCust], rgrAngZ[cCust], rRatio = 1.0/3.0, rRadi, rLenMax,
    radi1, radi2, len1, len2, ang1, ang2, ang, rx, ry, rT;
  byte ignoreSav[objMax];
  flag fMoonSav = us.fMoonMove;
  PT3R pt;
  KI kiP = -1;

  // If no moons unrestricted, temporarily unrestrict them all.
  Assert(sizeof(ignore) == sizeof(ignoreSav));
  CopyRgb(ignore, ignoreSav, sizeof(ignore));
  count = 0;
  for (m = custLo; m <= custHi; m++) {
    if (ignore[m])
      continue;
    i = ObjOrbit(m);
    if (!FHasMoon(i))
      continue;
    count++;
    k = kObjB[i];
    if (kiP < 0)
      kiP = k;
    else if (kiP != k)
      kiP = gi.kiLite;
  }
  if (count == 0)
    for (m = moonsLo; m <= moonsHi; m++)
      ignore[m] = fFalse;
  AdjustRestrictions();

  // Cast charts.
  us.fMoonMove = fFalse;
  for (i = 0; i <= 1; i++) {
    // Planetcentric charts are relative to geocentric reference.
    us.objCenter = (i <= 0 ? objCenterSav : oSun);
    CastChart(-1);
    rgcp[i] = cp0;
    for (j = 0; j < cHasMoons; j++) {
      m = rgobjHasMoons[j];
      if (m < custLo)
        rgrLenP[i][m] = PtLen(space[m]);
    }
    for (m = custLo; m <= custHi; m++) {
      if (ignore[m])
        continue;
      rgrLenM[i][m - custLo] = rgrAngM[i][m - custLo] = 0.0;
      rgrLenP[i][m] = PtLen(space[m]);
      j = ObjOrbit(m);
      if (j < 0)
        continue;
      rx = space[m].x - space[j].x; ry = space[m].y - space[j].y;
      rgrLenM[i][m - custLo] = RLength2(rx, ry);
      rgrAngM[i][m - custLo] = RAngleD(rx, ry) - planet[j] - rDegQuad;
    }
  }
  if (us.fMoonChartSep) {
    // Planetcentric charts are truly relative to each planet separately.
    for (i = 0; i < cHasMoons; i++) {
      j = rgobjHasMoons[i];
      if (ignore[j])
        continue;
      m = ObjCOB(j);
      us.objCenter = (ignore[m] ? j : m);
      CastChart(-1);
      for (m = custLo; m <= custHi; m++) {
        if (ignore[m] || ObjOrbit(m) != j)
          continue;
        for (k = 0; k <= 1; k++) {
          pt = space[m];
          PtSub2(pt, space[k <= 0 ? objCenterSav : oSun]);
          rgrLenP[k][m] = PtLen(pt);
          pt = space[m];
          rgrLenM[k][m - custLo] = RLength2(pt.x, pt.y);
          rgrAngM[k][m - custLo] = Mod(planet[m] - planet[k <= 0 ?
            objCenterSav : oSun]) + rDegQuad;
        }
      }
    }
  }
  us.objCenter = objCenterSav;
  us.fMoonMove = fTrue;
  CastChart(0);

  cx0 = gs.xWin >> 1; cy0 = gs.yWin >> 1;
  for (xSub = 0; xSub <= 1; xSub++) {
    cx = xSub * cx0 + (cx0 >> 1);
  for (ySub = 0; ySub <= 1; ySub++) {
    cy = ySub * cy0 + (cy0 >> 1);

    // Draw section markers
    i = yFontT*2 + gi.nScaleT*3;
    xs = (cx0 >> 1) - i; ys = (cy0 >> 1) - i;
    if (!gs.fHouseExtra)
      for (i = 0; i < 16; i++) {
        if ((i & 3) == 3)
          continue;
        rT = (real)(i-1) * 22.5;
        x = cx + (int)((real)xs * RCosD(rT));
        y = cy + (int)((real)ys * RSinD(rT));
        k = (i >> 2);
        if (FOdd(i))
          k = gi.kiGray;
        else if (ySub <= 0 && FOdd(k))
          k = kRainbowB[k > 1 ? 7 : 2];
        else
          k = kElemB[k];
        DrawColor(k);
        DrawDash(cx, cy, x, y, 1 + 2*FOdd(i));
      }
    if (!gs.fEquator) {
      DrawColor(gi.kiOn);
      DrawCircle(cx, cy, xs, ys);
    }
    DrawColor(kiP < 0 ? gi.kiLite : kiP);
    DrawCircle(cx, cy, (int)((real)xs * rRatio), (int)((real)ys * rRatio));

    // Label quadrants
    sprintf(sz, "%scentric %s view", xSub ? "Helio" : (objCenterSav == oEar ?
      "Geo" : szObjDisp[objCenterSav]), ySub ? "top" : "front");
    DrawColor(gi.kiLite);
    DrawSz(sz, cx, ySub * cy0 + 3*gi.nScaleT, dtTop);
    if (ySub <= 0) {
      DrawColor(kRainbowB[7]);
      DrawSz("Above", cx, cy - ys - 3*gi.nScaleT, dtBottom);
      DrawColor(kRainbowB[2]);
      DrawSz("Below", cx, cy + ys + 3*gi.nScaleT, dtTop);
    } else {
      DrawColor(kObjB[oNad]);
      if (xSub <= 0)
        sprintf(sz, "Distant (Opp %s)", szObjDisp[objCenterSav]);
      else
        sprintf(sz, "Distant (Full Moon)");
      DrawSz(sz, cx, cy - ys - 3*gi.nScaleT, dtBottom);
      DrawColor(kObjB[oMC]);
      if (xSub <= 0)
        sprintf(sz, "Close (Con %s)", szObjDisp[objCenterSav]);
      else
        sprintf(sz, "Close (New Moon)");
      DrawSz(sz, cx, cy + ys + 3*gi.nScaleT, dtTop);
    }
    x = xs+(xFont2+4)*gi.nScaleT; y = cy+6*gi.nScaleT;
    for (i = -1; i <= 1; i += 2) {
      sprintf(sz, "%s", i < 0 ? "Lead" : "Follow");
      DrawColor(kObjB[i < 0 ? oAsc : oDes]);
      k = CchSz(sz);
      for (j = 0; j < k; j++) {
        sprintf(szCh, "%c", sz[j]);
        DrawSz(szCh, cx-i*x, y+(j*yFont-k*yFont2)*gi.nScaleT, dtCent);
      }
    }

    // Place moons
    count = 0;
    for (i0 = 0; i0 < cHasMoons; i0++) {
      i = rgobjHasMoons[i0];
      if (ignore[i])
        continue;
      if (FCust(i)) {
        j = ObjOrbit(i);
        if (FHasMoon(j))
          continue;
      }
      countOld = count;
      rLenMax = 0.0;
      for (m = custLo; m <= custHi; m++) {
        if (ignore[m] || ObjOrbit(m) != i)
          continue;
        j = ObjCOB(i);
        j = ignore[j] ? i : j;
        if (ignore[j])
          continue;
        cp = &rgcp[xSub];
        if (ySub <= 0) {
          rx = MinDifference(cp->obj[j], cp->obj[m]);
          ry = cp->alt[j] - cp->alt[m];
          rgrLenZ[m - custLo] = RLength2(rx, ry);
          rgrAngZ[m - custLo] = RAngleD(rx, ry);
        } else {
          rgrLenZ[m - custLo] = rgrLenM[xSub][m - custLo];
          rgrAngZ[m - custLo] = rgrAngM[xSub][m - custLo];
        }
        if (rLenMax < rgrLenZ[m - custLo])
          rLenMax = rgrLenZ[m - custLo];
        rgod[count].obj = m;
        rgod[count].kv = ~0;
        rgod[count].f = fTrue;
        count++;
      }
      if (count <= countOld)
        continue;
      rRadi = RObjDiam(i) / 2.0;
      if (ySub <= 0) {
        // Determine angular diameter covered by moon in sky.
        if (rRadi > 0.0) {
          rT = rgrLenP[xSub][i] * rAUToKm;
          rRadi = RAtnD(rRadi / rT);
        }
      } else
        rRadi /= rAUToKm;
      for (m = custLo; m <= custHi; m++) {
        if (ignore[m] || ObjOrbit(m) != i)
          continue;
        if (rgrLenZ[m - custLo] <= rRadi)
          rgrLenZ[m - custLo] = rRatio * rgrLenZ[m - custLo] / rRadi;
        else
          rgrLenZ[m - custLo] = rRatio + (1.0 - rRatio) * RSinD((rgrLenZ[m -
            custLo] - rRadi) / (rLenMax - rRadi) * rDegQuad);
      }
    }
    for (i = 0; i < count; i++) {
      j = rgod[i].obj - custLo;
      rgod[i].x = cx + (int)((real)xs * rgrLenZ[j] * RCosD(rgrAngZ[j]));
      rgod[i].y = cy + (int)((real)ys * rgrLenZ[j] * RSinD(rgrAngZ[j]));
    }

    // Draw lines connecting planets which have aspects between them.
    if (gs.fLabelCity) {
      if (ySub <= 0) {
        cp = &rgcp[xSub];
        DrawColor(kAspB[aCon]);
        for (j = count-1; j >= 1; j--) {
          y = rgod[j].obj;
          for (i = j-1; i >= 0; i--) {
            x = rgod[i].obj;
            if (ObjOrbit(x) != ObjOrbit(y))
              continue;
            // Determine angular diameters covered by moons in sky.
            radi1 = RObjDiam(x) / 2.0;
            radi2 = RObjDiam(y) / 2.0;
            len1 = rgrLenP[xSub][x] * rAUToKm;
            len2 = rgrLenP[xSub][y] * rAUToKm;
            ang1 = RAtnD(radi1 / len1);
            ang2 = RAtnD(radi2 / len2);
            rT = !us.fAspect3D ? MinDistance(cp->obj[x], cp->obj[y]) :
              SphDistance(cp->obj[x], cp->alt[x], cp->obj[y], cp->alt[y]);
            ang = ang1 + ang2;
            if (rT > ang*(rAspOrb[aCon] + 1.0))
              continue;
            DrawDash(rgod[i].x, rgod[i].y, rgod[j].x, rgod[j].y,
              (int)(rT / (ang*(rAspOrb[aCon] + 1.0)) * NAbs(gs.nDashMax)));
          }
        }
      } else {
        if (!FCreateGrid(fFalse))
          return;
        k = gi.nScale;
        gi.nScale = gi.nScaleTextT;
        for (j = count-1; j >= 1; j--) {
          y = rgod[j].obj;
          for (i = j-1; i >= 0; i--) {
            x = rgod[i].obj;
            if (grid->n[x][y] && ObjOrbit(x) == ObjOrbit(y)) {
              DrawColor(kAspB[grid->n[x][y]]);
              DrawDash(rgod[i].x, rgod[i].y, rgod[j].x, rgod[j].y,
                NDashAspect(x, y, grid->n[x][y], grid->v[x][y]));
              if (gs.fLabelAsp)
                DrawAspect(grid->n[x][y],
                  (rgod[i].x + rgod[j].x) >> 1, (rgod[i].y + rgod[j].y) >> 1);
            }
          }
        }
        gi.nScale = k;
      }
    }
    DrawObjects(rgod, count, 0);
  } // ySub
  } // xSub

  // Draw boundary lines between the four separate moons graphs.
  if (gs.fBorder) {
    DrawColor(gi.kiGray);
    DrawLineY(cx0, 0, gs.yWin-1);
    DrawLineX(0, gs.xWin-1, cy0);
  }

  // Restore settings and recast original chart.
  CopyRgb(ignoreSav, ignore, sizeof(ignore));
  AdjustRestrictions();
  us.fMoonMove = fMoonSav;
  CastChart(1);
}


#define XiN(i) ( FBetween(i, 3, 5)  ? -1 : (FBetween(i, 9, 11) ? 1 : 0))
#define YiN(i) (!FBetween(i, 3, 11) ? -1 : (FBetween(i, 6, 8)  ? 1 : 0))
#define ZiN(i, x, y) ((i) % 3 == 1 ? (x) : (y))
#define XiE(i) \
  (FBetween(i, 2, 6) ? -1-FOdd(i) : (FBetween(i, 1, 7) ? 0 : 1+FOdd(i)))
#define YiE(i) \
  (FBetween(i, 5, 9) ? 2-FOdd(i) : (FBetween(i, 4, 10) ? 0 : FOdd(i)-2))

// Draw a South Indian, North Indian, or East Indian style wheel chart, in
// which the 12 signs and houses are square or triangular areas. This is done
// when the -v -X standard wheel (South Indian) or -w -X house wheel (North
// Indian) is displayed, and the -XJ Indian style wheel setting is also on.
// East Indian is displayed in place of South Indian when -XC setting is on.

void XChartIndian()
{
  char sz[cchSzDef];
  int rgcbox[cSign+1], mpobox[objMax], rgibox[objMax],
    rgx[cSign+1], rgy[cSign+1], cx, cy, unit, x1, y1, x2, y2, x11, y11,
    x22, y22, xs, ys, xb, yb, xi, yi, xp, yp, z, i, j, k, n, sig, nGrid;
  flag fSouthIndian = (gi.nMode == gWheel && !gs.fHouseExtra),
    fNorthIndian = gi.nMode == gHouse,
    fEastIndian = (gi.nMode == gWheel && gs.fHouseExtra),
    fTextHouse = (gs.nFontHou == 0),
    fHouseSign = (us.nHouseSystem == hsWhole || us.nHouseSystem == hsNull);

  // Initialize box size and other variables
  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;
  unit = Max(12, 6*gi.nScale);
  unit = Max(unit, yFontT);
  x1 = y1 = unit; x2 = gs.xWin-1-unit; y2 = gs.yWin-1-unit;
  cx = (x1+x2)/2; cy = (y1+y2)/2;
  if (!fEastIndian) {
    xb = (x2-x1)/4; yb = (y2-y1)/4;
    x1 = cx - xb*2; y1 = cy - yb*2; x2 = cx + xb*2; y2 = cy + yb*2;
  } else {
    xb = (x2-x1)/3; yb = (y2-y1)/3;
    x1 = cx - xb*3/2; y1 = cy - yb*3/2; x2 = cx + xb*3/2; y2 = cy + yb*3/2;
  }
  xs = x2-x1; ys = y2-y1;
  x11 = x1 + xb; y11 = y1 + yb; x22 = x2 - xb; y22 = y2 - yb;
  ClearB((pbyte)rgcbox, sizeof(rgcbox));
  ClearB((pbyte)mpobox, sizeof(mpobox));
  ClearB((pbyte)rgibox, sizeof(rgibox));

  // Draw box outlines
  DrawColor(gi.kiOn);
  DrawEdge(x1, y1, x2, y2);
  if (fSouthIndian) {
    DrawLineX(x1,  x2,  y11); DrawLineX(x1,  x2,  y22);
    DrawLineY(x11, y1,  y2);  DrawLineY(x22, y1,  y2);
    DrawLineX(x1,  x11, cy);  DrawLineX(x22, x2,  cy);
    DrawLineY(cx,  y1,  y11); DrawLineY(cx,  y22, y2);
    rgx[1]  = x11; rgy[1]  = y1;
    rgx[2]  = cx;  rgy[2]  = y1;
    rgx[3]  = x22; rgy[3]  = y1;
    rgx[4]  = x22; rgy[4]  = y11;
    rgx[5]  = x22; rgy[5]  = cy;
    rgx[6]  = x22; rgy[6]  = y22;
    rgx[7]  = cx;  rgy[7]  = y22;
    rgx[8]  = x11; rgy[8]  = y22;
    rgx[9]  = x1;  rgy[9]  = y22;
    rgx[10] = x1;  rgy[10] = cy;
    rgx[11] = x1;  rgy[11] = y11;
    rgx[12] = x1;  rgy[12] = y1;
    z = 6*gi.nScale;
  } else if (fNorthIndian) {
    DrawLine(x1, y1, x2, y2); DrawLine(x2, y1, x1, y2);
    DrawLine(cx, y1, x1, cy); DrawLine(x1, cy, cx, y2);
    DrawLine(cx, y2, x2, cy); DrawLine(x2, cy, cx, y1);
    rgx[1]  = cx;  rgy[1]  = cy;
    rgx[2]  = x11; rgy[2]  = y11;
    rgx[3]  = x11; rgy[3]  = y11;
    rgx[4]  = cx;  rgy[4]  = cy;
    rgx[5]  = x11; rgy[5]  = y22;
    rgx[6]  = x11; rgy[6]  = y22;
    rgx[7]  = cx;  rgy[7]  = cy;
    rgx[8]  = x22; rgy[8]  = y22;
    rgx[9]  = x22; rgy[9]  = y22;
    rgx[10] = cx;  rgy[10] = cy;
    rgx[11] = x22; rgy[11] = y11;
    rgx[12] = x22; rgy[12] = y11;
    z = 12*gi.nScale;
  } else {
    DrawLineX(x1,  x2, y11); DrawLineX(x1,  x2, y22);
    DrawLineY(x11, y1, y2);  DrawLineY(x22, y1, y2);
    DrawLine(x1, y1, x11, y11); DrawLine(x2, y1, x22, y11);
    DrawLine(x1, y2, x11, y22); DrawLine(x2, y2, x22, y22);
    rgx[1]  = cx;  rgy[1]  = y11;
    rgx[2]  = x11; rgy[2]  = y11;
    rgx[3]  = x11; rgy[3]  = y11;
    rgx[4]  = x11; rgy[4]  = cy;
    rgx[5]  = x11; rgy[5]  = y22;
    rgx[6]  = x11; rgy[6]  = y22;
    rgx[7]  = cx;  rgy[7]  = y22;
    rgx[8]  = x22; rgy[8]  = y22;
    rgx[9]  = x22; rgy[9]  = y22;
    rgx[10] = x22; rgy[10] = cy;
    rgx[11] = x22; rgy[11] = y11;
    rgx[12] = x22; rgy[12] = y11;
    z = 7*gi.nScale;
  }
  sig = SFromZ(chouse[1]) - 1;

  // Label boxes
  for (i = 1; i <= cSign; i++) {
    j = !us.fIndian ? i :
      Mod12(fSouthIndian ? sig-1-i : (fNorthIndian ? i+3 : i-sig+3));
    xi = fSouthIndian ? 1 : (fNorthIndian ? XiN(j) : XiE(j));
    yi = fSouthIndian ? 1 : (fNorthIndian ? YiN(j) : YiE(j));
    DrawColor(kSignB(i));
    DrawFillWheel(rgx[j] + xb*xi/3, rgy[j] + yb*yi/3, i, fNorthIndian);
    if (fSouthIndian) {
      DrawSign(i, rgx[j] + z, rgy[j] + z);
      if (fTextHouse)
        gi.nScale <<= 1;
      n = Mod12(i - sig);
      if (fHouseSign) {
        DrawColor(kSignB(n));
        DrawHouse(n, rgx[j] + xb/2, rgy[j] + z);
      }
      if (fTextHouse)
        gi.nScale >>= 1;
      if (n == 1) {
        DrawColor(gi.kiOn);
        DrawLine(rgx[j] + xb - xb/4, rgy[j], rgx[j] + xb, rgy[j] + yb/4);
        DrawLine(rgx[j] + xb - xb/5, rgy[j], rgx[j] + xb, rgy[j] + yb/5);
      }
    } else if (fNorthIndian) {
      if (fTextHouse)
        gi.nScale <<= 1;
      DrawHouse(i, rgx[j] + z*xi, rgy[j] + z*yi);
      if (fTextHouse)
        gi.nScale >>= 1;
      if (fHouseSign) {
        n = Mod12(i + sig);
        DrawColor(kSignB(n));
        DrawSign(n, rgx[j] + z*2*xi, rgy[j] + z*2*yi);
      }
    } else {
      DrawSign(i, rgx[j] + z*(xi*3/2), rgy[j] + z*(yi*3/2));
      if (fTextHouse)
        gi.nScale <<= 1;
      n = Mod12(i - sig);
      if (fHouseSign) {
        DrawColor(kSignB(n));
        DrawHouse(n, rgx[j] + z*(xi*3/2 + XiN(j)*2),
          rgy[j] + z*(yi*3/2 + YiN(j)*2));
      }
      if (fTextHouse)
        gi.nScale >>= 1;
    }
  }

  // Determine placement and sorting order within boxes
  for (i = 0; i <= is.nObj; i++) if (!ignore[i]) {
    if (fHouseSign && FCusp(i) && (!FAngle(i) || us.fHouseAngle))
      continue;
    if (!fNorthIndian) {
      n = Mod12(SFromZ(planet[i]));
    } else
      n = inhouse[i];
    mpobox[i] = n;
    rgibox[i] = rgcbox[n];
    rgcbox[n]++;
  }
  do {
    n = 0;
    for (i = is.nObj; i > 0; i--) if (mpobox[i])
      for (j = 0; j < i; j++) if (mpobox[j])
        if (mpobox[i] == mpobox[j] &&
          (rgibox[i] < rgibox[j]) != (planet[i] < planet[j])) {
          SwapN(rgibox[i], rgibox[j]);
          n++;
        }
  } while (n > 0);

  // Draw planets in boxes
  for (i = 0; i <= is.nObj; i++) if (mpobox[i]) {
    n = mpobox[i];
    j = !us.fIndian ? n :
      Mod12(fSouthIndian ? sig-1-n : (fNorthIndian ? n+3 : n-sig+3));
    if (fSouthIndian) {
      k = (10*gi.nScale *
        ((!us.fWheelReverse || !FBetween(j, sTau, sLib) == us.fIndian ?
        rgibox[i] : rgcbox[n]-1-rgibox[i])*2+fHouseSign - rgcbox[n])) >> 1;
      xp = rgx[j] + xb/2;
      yp = rgy[j] + yb/2 + z + k;
    } else if (fNorthIndian) {
      k = (10*gi.nScale * ((!us.fWheelReverse || FBetween(j, 3, 8) ?
        rgibox[i] : rgcbox[n]-1-rgibox[i])*2+1 - rgcbox[n])) >> 1;
      if (FBetween(j, 3, 5) || FBetween(j, 9, 11))
        n = 0;
      else
        n = k*(3-gs.fLabel)/2, k = 0;
      xp = rgx[j] + xb*ZiN(j, 4, 3)/4*XiN(j) + n;
      yp = rgy[j] + yb*ZiN(j, 4, 3)/4*YiN(j) + k;
    } else {
      k = (10*gi.nScale * ((!us.fWheelReverse || FBetween(j, 3, 8) ?
        rgibox[i] : rgcbox[n]-1-rgibox[i])*2+1 - rgcbox[n])) >> 1;
      if (FBetween(j, 3, 5) || FBetween(j, 9, 11))
        n = 0;
      else
        n = k*(3-gs.fLabel)/2, k = 0;
      xp = rgx[j] + xb*ZiN(j, 5, 7)/8*XiN(j) + XiE(j)*NAbs(YiN(j))*xb*7/16 + n;
      yp = rgy[j] + yb*ZiN(j, 5, 7)/8*YiN(j) + YiE(j)*NAbs(XiN(j))*yb*7/16 + k;
    }
    if (gs.fLabel)
      DrawObject(i, xp, yp);
    else {
      DrawColor(kObjB[i]);
      sprintf(sz, "%.2s", szObjDisp[i]);
      DrawSz(sz, xp, yp, dtCent | dtScale);
    }
  }

  // Draw aspect grid in center
  if (!fNorthIndian && !gs.fEquator) {
    for (n = i = 0; i <= is.nObj; i++)
      if (!ignore[i])
        n++;
    k = CELLSIZE*gi.nScale;
    nGrid = Min(xb, yb)*(2-fEastIndian) / k - 1; nGrid = Min(n, nGrid);
    j = nGrid*k;
    xp = cx - (j >> 1); yp = cy - (j >> 1);
    k = gi.nGridCell; gi.nGridCell = nGrid;
    if (gs.fLabel)
      XChartGrid(xp, yp);
    else {
      gs.fLabel = fTrue; XChartGrid(xp, yp); gs.fLabel = fFalse;
    }
    gi.nGridCell = k;
    DrawColor(gi.kiLite);
    if (j > 0)
      DrawEdge(xp, yp, xp + j, yp + j);
  }

  // Go draw sidebar with chart information and positions if need be.
  DrawSidebar();
}


// Translate to chart pixel coordinates, that indicate where to draw on a
// chart sphere, for the -XX switch chart. Inputs may be local horizon
// altitude and azimuth coordinates, local horizon prime vertical, local
// horizon meridian, zodiac position and latitude, or Earth coordinates.

flag FSphereLocal(real azi, real alt, CONST CIRC *pcr, int *xp, int *yp)
{
  if (gs.fEcliptic) {
    azi = Mod(azi - rDegQuad); neg(alt);
    CoorXform(&azi, &alt, Lat - rDegQuad);
    azi = Mod(cp0.lonMC - azi + rDegQuad);
    EquToEcl(&azi, &alt);
    azi = rDegMax - Untropical(azi); neg(alt);
  } else {
    if (us.fRefract)
      alt = -SwissRefract(-alt);
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
  lonT = Mod(cp0.lonMC - lonT + rDegQuad);
  EquToLocal(&lonT, &latT, rDegQuad - Lat);
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
    fNoHorizon, fSav, f;
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
  cChart = 1 - (FBetween(us.nRel, rcHexaWheel, rcDual) ? us.nRel : 0);
  fSav = us.fRefract; us.fRefract = fFalse;

  if (us.nRel < rcNone)
    CastChart(1);

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
        k = i%10 == 0 ? 3 : (i%5 == 0 ? 2 : 1);
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
        k = i%10 == 0 ? 3 : (i%5 == 0 ? 2 : 1);
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
          k = i%10 == 0 ? 3 : (i%5 == 0 ? 2 : 1);
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
  if (!us.fIndian) {
    if (!gs.fColorSign)
      DrawColor(kDkBlueB);
    // Draw ecliptic circle.
    for (i = 0; i <= nDegMax; i++) {
      if (gs.fColorSign)
        DrawColor(kSignB((i-1)/30 + 1));
      f = FSphereZodiac((real)i, 0.0, &cr, &xp, &yp) ^ fDir;
      if (f && i > 0) {
        DrawLine(xo, yo, xp, yp);
        if (i%30 != 0) {
          k = i%10 == 0 ? 3 : (i%5 == 0 ? 2 : 1);
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
  if (!us.fIndian) {
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
        sprintf(sz, "%c", rgszDir[j][0]);
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

  // Calculate planet coordinates.
  us.fRefract = fSav;
  for (i = 0; i <= is.nObj; i++) {
    f = FProper(i);
    if (f) {
      f = FSphereZodiac(pcp->obj[i], pcp->alt[i], &cr, &xp, &yp) ^ fDir;
      rgod[i].obj = i;
      rgod[i].x = xp; rgod[i].y = yp;
      rgod[i].kv = f ? ~0 : gi.kiGray;
      rgod[i].f = fAny || f;
    } else
      rgod[i].f = fFalse;
  }

  // Draw lines connecting planets which have aspects between them.
  cpSav = cp0;
  cp0 = *pcp;
  if (!FCreateGrid(fFalse))
    return;
  cp0 = cpSav;
  nSav = gi.nScale;
  gi.nScale = gi.nScaleTextT;
  for (j = is.nObj; j >= 1; j--)
    for (i = j-1; i >= 0; i--)
      if (grid->n[i][j] && FProper(i) && FProper(j) &&
        (fAny || (rgod[i].f && rgod[j].f))) {
        DrawColor(rgod[i].kv == ~0 && rgod[j].kv == ~0 ?
          kAspB[grid->n[i][j]] : gi.kiGray);
        DrawDash(rgod[i].x, rgod[i].y, rgod[j].x, rgod[j].y,
          NDashAspect(i, j, grid->n[i][j], grid->v[i][j]) +
          ((rgod[i].kv != ~0) + (rgod[j].kv != ~0))*2);
        if (gs.fLabelAsp)
          DrawAspect(grid->n[i][j],
            (rgod[i].x + rgod[j].x) >> 1, (rgod[i].y + rgod[j].y) >> 1);
      }
  gi.nScale = nSav;

  // Draw planet glyphs, and spots for actual planet locations.
  DrawObjects(rgod, is.nObj+1, 0);

  } // iChart
  FProcessCommandLine(szWheelX[0]);

  DrawSidebar();
}
#endif // GRAPH

/* xcharts1.cpp */
