/*
** Astrolog (Version 7.30) File: xcharts0.cpp
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
** Graphics Chart Utilities.
******************************************************************************
*/

// Given a string, draw it on the screen using the given color. The position
// of the text is based the saved positions of where we drew the text the last
// time the routine was called, being either directly below in the same column
// or in the same row just to the right. This is used by the sidebar drawing
// routine to print a list of text on the chart.

int DrawPrint(CONST char *sz, int m, int n)
{
  static int xStart, x, y;

  if (sz == NULL) {           // Null string means just initialize position.
    if (n >= 0) {
      xStart = x = m; y = n;
    } else
      x += m;
    return y;
  }
  if (y >= gs.yWin-1)     // Don't draw if have scrolled off the chart bottom.
    return y;
  DrawColor(m);
  if (CchSz(sz) > 25)     // Adjust slightly to fit in 26 character strings.
    x -= xFont2*gi.nScaleText/2;
  DrawSz(sz, x, y, dtLeft | dtBottom | dtScale2);

  // If the second parameter is TRUE, then stay on the same line, otherwise
  // when FALSE go to the next line at the original column setting.

  if (n)
    x += CchSz(sz) * xFontT;
  else {
    x = xStart;
    n = y;
    y += yFontT;
  }
  return y;
}


// Draw a zodiac position on the screen, rounding and formatting as needed.

int DrawZodiac(real deg, int n)
{
  if (us.fRound) {
    if (us.nDegForm == 0)
      deg = Mod(deg + (is.fSeconds ? rRound/60.0/60.0 : rRound/60.0));
    else if (us.nDegForm == 1)
      deg = Mod(deg + (is.fSeconds ? rRound/4.0/60.0 : rRound/4.0));
  }
  return DrawPrint(SzZodiac(deg), kSignB(SFromZ(deg)), n);
}


// Print chart info in the sidebar for a single chart. Called from
// DrawSidebar() 1-6 times depending on the relationship chart mode.

void DrawInfo(CI *pci, CONST char *szHeader, flag fAll)
{
  char sz[cchSzDef], szT[cchSzDef], *pch, *pch2;

  if (szHeader != NULL)
    DrawPrint(szHeader, gi.kiOn, fFalse);
  if (*pci->nam)
    DrawPrint(pci->nam, gi.kiLite, fFalse);

  if (FNoTimeOrSpace(*pci))
    DrawPrint("No time or space", gi.kiLite, fFalse);
  else if (us.nRel == rcComposite)
    DrawPrint("Composite chart", gi.kiLite, fFalse);
  else {

    // Standard case: Print date, time, location, and maybe more.
    sprintf(sz, "%.3s %s", szDay[DayOfWeek(pci->mon, pci->day, pci->yea)],
      SzDate(pci->mon, pci->day, pci->yea, fTrue));
    DrawPrint(sz, gi.kiLite, fFalse);
    DrawPrint(SzTim(pci->tim), gi.kiLite, fTrue);
    sprintf(sz, " %s%cT Zone %s%s", is.fSeconds ? "" : "(", ChDst(pci->dst),
      SzZone(pci->zon), is.fSeconds ? "" : ")");
    DrawPrint(sz, gi.kiLite, fFalse);
    if (*pci->loc)
      DrawPrint(pci->loc, gi.kiLite, fFalse);
    DrawPrint(SzLocation(pci->lon, pci->lat), gi.kiLite, fFalse);
    if (fAll) {
      sprintf(sz, "%s%s houses", us.fHouse3D == (gi.nMode != gSphere) ?
        "3D " : (us.fHouse3D && gi.nMode == gSphere ? "2D " : ""),
        szSystem[is.nHouseSystem]);
      DrawPrint(sz, gi.kiLite, fFalse);
      sprintf(sz, "%s, %s", !us.fSidereal ? "Tropical" :
        (!us.fSidereal2 ? "Sidereal" : "Sidereal Inv"),
        us.objCenter == oSun ?
          (!us.fBarycenter ? "Heliocentric" : "Barycentric") :
        (us.objCenter == oEar ?
          (!us.fTopoPos ? "Geocentric" : "Topocentric") :
        szObjDisp[us.objCenter]));
      DrawPrint(sz, gi.kiLite, fFalse);
      sprintf(sz, "Julian Day: %13.5f", JulianDayFromTime(is.T));
      DrawPrint(sz, gi.kiLite, fFalse);
      if (us.fProgress) {
        sprintf(sz, "Progressed To: %s", SzDate(MonT, DayT, YeaT, 0));
        DrawPrint(sz, gi.kiLite, fFalse);
      }
      if (us.objRot1 != us.objRot2 || us.fObjRotWhole) {
        sprintf(sz, "Rotate: %.3s to %.3s%s", szObjDisp[us.objRot1],
          szObjDisp[us.objRot2], us.fObjRotWhole ? "'s sign" : "");
        DrawPrint(sz, gi.kiLite, fFalse);
      }
      if (us.objOnAsc) {
        sprintf(sz, "Solar: %.4s%s on %.3s", szObjDisp[NAbs(us.objOnAsc)-1],
          us.fSolarWhole ? "'s sign" : "",
          szObjDisp[us.objOnAsc > 0 ? oAsc : oMC]);
        DrawPrint(sz, gi.kiLite, fFalse);
      }
      if (us.fEquator)
        DrawPrint("Special: Equatorial lon.", gi.kiLite, fFalse);
      if (us.fGeodetic)
        DrawPrint("Special: Geodetic houses", gi.kiLite, fFalse);
      if (us.fFlip)
        DrawPrint("Special: Domal mode", gi.kiLite, fFalse);
      if (us.fDecan)
        DrawPrint("Special: Decan mode", gi.kiLite, fFalse);
      if (us.rHarmonic != 1.0) {
        FormatR(szT, us.rHarmonic, -5);
        sprintf(sz, "Special: Harmonic %.7s", szT);
        DrawPrint(sz, gi.kiLite, fFalse);
      }
      if (us.nDwad == 1)
        DrawPrint("Special: Dwad mode", gi.kiLite, fFalse);
      else if (us.nDwad > 1) {
        sprintf(sz, "Special: Dwad level %d", us.nDwad);
        DrawPrint(sz, gi.kiLite, fFalse);
      }
      if (us.fNavamsa)
        DrawPrint("Special: Navamsa mode", gi.kiLite, fFalse);
      if (us.fMoonMove) {
        sprintf(sz, "Special: Overlay %ss",
          FStar(us.objCenter) ? "planet" : "moon");
        DrawPrint(sz, gi.kiLite, fFalse);
      }
      if (us.fNoNutation && !us.fSidereal)
        DrawPrint("Special: No Nutation", gi.kiLite, fFalse);
      if (us.rDeltaT != rInvalid) {
        FormatR(szT, us.rDeltaT, -4);
        sprintf(sz, "Special: Delta-T = %s", szT);
        DrawPrint(sz, gi.kiLite, fFalse);
      }
      if (us.rObjAddition != 0.0) {
        sprintf(sz, "%s: %s (%s)", us.rObjAddition != us.rCuspAddition ?
          "Objs" : "Time", SzHMS((int)(us.rObjAddition*3600.0 +
          rSmall*RSgn2(us.rObjAddition))*60), SzTim(Tim + us.rObjAddition));
        DrawPrint(sz, gi.kiLite, fFalse);
      }
      if (us.rCuspAddition != 0.0 && us.rCuspAddition != us.rObjAddition) {
        sprintf(sz, "Cusp: %s (%s)", SzHMS((int)(us.rCuspAddition*3600.0 +
          rSmall*RSgn2(us.rCuspAddition))*60), SzTim(Tim + us.rCuspAddition));
        DrawPrint(sz, gi.kiLite, fFalse);
      }
      if (FSzSet(gs.szSidebar))
        for (pch2 = gs.szSidebar; *pch2;) {
          for (pch = sz; *pch2 && *pch2 != '\n' &&
            !(*pch2 == '\\' && pch2[1] == 'n'); pch++, pch2++) {
#ifdef EXPRESS
            if (*pch2 == '\\' && FCapCh(pch2[1])) {
              pch = PchFormatExpression(pch, pch2[1] - '@') + 1;
              pch2++;
              continue;
            }
#endif
            *pch = *pch2;
            if (*pch2 == '\\' && pch2[1] == '\\')
              pch2++;
          }
          *pch = sz[26] = chNull;
          DrawPrint(sz, gi.kiLite, fFalse);
          if (*pch2 == '\n')
            pch2++;
          else if (*pch2 == '\\' && pch2[1] == 'n')
            pch2 += 2;
        }
    }
  }
}


#define szC1 "Chart #1"
#define szC2 "Chart #2"
#define szC3 "Chart #3"
#define szC4 "Chart #4"
#define szC5 "Chart #5"
#define szC6 "Chart #6"
#define DrawInfoSphere(ci, sz1, sz2) \
  DrawInfo(&ci, gi.nMode != gSphere ? sz1 : sz2, fFalse)

// Print text showing the chart information and house and planet positions
// of a chart in a "sidebar" to the right of the chart in question. This is
// always done for the -v and -w graphic wheel charts unless the -v0 switch
// flag is also set, in which case none of the things here are done.

void DrawSidebar()
{
  char sz[cchSzDef];
  ET et;
  int i, j, k, l, y, a, s;
  real r;

  // Decorate the chart a little.

  if (gs.nDecaType == 1) {

    // If decoration value 1, draw spider web lines in each corner.
    DrawColor(gi.kiGray);
    j = gs.nDecaLine + 1;
    k = gs.yWin * gs.nDecaSize / 100;
    l = gs.xWin * gs.nDecaSize / 100;
    for (y = 0; y <= 1; y++)
      for (i = 0; i <= 1; i++)
        for (a = 1; a < j; a++)
          DrawLine(i*(gs.xWin-1), y ? (gs.yWin-1-a*k/j) : a*k/j,
            i ? gs.xWin-1-l+a*l/j : l-a*l/j, y*(gs.yWin-1));
  } else if (gs.nDecaType == 2) {

    // If decoration value 2, draw a moire pattern in each corner.
    k = gs.yWin * gs.nDecaSize / 200;
    l = gs.xWin * gs.nDecaSize / 200;
    for (y = 0; y <= 1; y++)
      for (i = 0; i <= 1; i++)
        for (s = 0; s <= 1; s++)
          for (a = 1; a < (s ? l : k)*2; a++) {
            DrawColor(FOdd(a) ? gi.kiGray : gi.kiOff);
            DrawLine(i ? gs.xWin-1-l : l, y ? gs.yWin-1-k : k,
              s ? (i ? gs.xWin-1-a : a) : i*(gs.xWin-1),
              s ? y*(gs.yWin-1) : (y ? gs.yWin-1-a : a));
          }
  }

  if (!gs.fText || us.fVelocity)    // Don't draw sidebar if -v0 flag is set.
    return;
  a = us.fAnsiChar;
  us.fAnsiChar = (gs.nFont == 0 || (gs.ft != ftPS && gs.ft != ftWmf)) << 1;
  DrawColor(gi.kiLite);
  i = gs.xWin-1;
  gs.xWin += xSideT;
  if (gs.fBorder)
    DrawLineY(i, 1, gs.yWin-2);
  DrawPrint(NULL, gs.xWin-xSideT+xFontT-gi.nScaleT, yFontT*7/5);
#ifdef EXPRESS
  // Notify AstroExpression the sidebar is about to be drawn.
  if (!us.fExpOff && FSzSet(us.szExpSidebar))
    ParseExpression(us.szExpSidebar);
#endif

  // Print chart header and setting information.

  sprintf(sz, "%s %s", szAppName, szVersionCore);
  DrawPrint(sz, gi.kiOn, fFalse);
  if (us.nRel == rcComposite) {
    DrawInfo(&ciMain, szC1, fFalse);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfo(&ciTwin, szC2, fFalse);
  } else if (us.nRel == rcSynastry && !FEqCI(ciMain, ciTwin)) {
    DrawInfo(&ciMain, szC1 ": Houses", fFalse);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfo(&ciTwin, szC2 ": Planets", fFalse);
  } else if (us.nRel == rcDual && !FEqCI(ciMain, ciTwin)) {
    DrawInfoSphere(ciMain, "#1: Inner Ring + Houses", szC1 " + Houses");
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciTwin, "#2: Outer Ring + Planets", szC2);
  } else if (us.nRel == rcTriWheel &&
    !(FEqCI(ciMain, ciTwin) && FEqCI(ciTwin, ciThre))) {
    DrawInfoSphere(ciMain, "#1: Outer Ring + Houses", szC1 " + Houses");
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciTwin, "#2: Middle Ring", szC2);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciThre, "#3: Inner Ring + Planets", szC3);
  } else if (us.nRel == rcQuadWheel && !(FEqCI(ciMain, ciTwin) &&
    FEqCI(ciTwin, ciThre) && FEqCI(ciThre, ciFour))) {
    DrawInfoSphere(ciMain, "#1: Outer Ring + Houses", szC1 " + Houses");
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciTwin, "#2: Middle Outer Ring", szC2);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciThre, "#3: Middle Inner Ring", szC3);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciFour, "#4: Inner Ring + Planets", szC4);
  } else if (us.nRel == rcQuinWheel) {
    DrawInfoSphere(ciMain, "#1: Outer Ring + Houses", szC1 " + Houses");
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciTwin, "#2: Middle Outer Ring", szC2);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciThre, "#3: Middle Ring", szC3);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciFour, "#4: Middle Inner Ring", szC4);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciFive, "#5: Inner Ring + Planets", szC5);
  } else if (us.nRel == rcHexaWheel) {
    DrawInfoSphere(ciMain, "#1: Outer Ring + Houses", szC1 " + Houses");
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciTwin, "#2: Second Outer Ring", szC2);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciThre, "#3: Middle Outer Ring", szC3);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciFour, "#4: Middle Inner Ring", szC4);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciFive, "#5: Second Inner Ring", szC5);
    DrawPrint("", gi.kiLite, fFalse);
    DrawInfoSphere(ciHexa, "#6: Inner Ring + Planets", szC6);
  } else if (us.nRel == rcTransit || us.nRel == rcProgress) {
    DrawInfoSphere(ciMain, "#1: Inner Ring (Natal)", szC1 ": Natal");
    DrawPrint("", gi.kiLite, fFalse);
    if (us.nRel == rcTransit)
      DrawInfoSphere(ciTwin, "#2: Outer Ring (Transit)", szC2 ": Transit");
    else
      DrawInfoSphere(ciTwin, "#2: Outer Ring (Progress)", szC2 ": Progress");
  } else
    DrawInfo(&ciMain, NULL, fTrue);

  // Print house cusp positions.

  DrawPrint("", gi.kiLite, fFalse);
  for (i = 1; i <= cSign; i++) {
    sprintf(sz, "%2d%s house: ", i, szSuffix[i]);
    y = DrawPrint(sz, kSignB(i), fTrue);
    if (!is.fSeconds && (gs.nScale == 100 ||
      gs.nFont == 0 || !gi.fFile || gs.ft == ftBmp) && y < gs.yWin-1) {
      s = gi.nScale;
      gi.nScale = gi.nScaleTextT;
      DrawSign(SFromZ(chouse[i]),
        gs.xWin-12*gi.nScale, y-(yFont/2-1)*gi.nScale);
      gi.nScale = s;
    }
    DrawZodiac(chouse[i], fFalse);
  }

  // Print planet positions.

  DrawPrint("", gi.kiLite, fFalse);
  for (j = 0; j <= oNorm; j++) {
    i = rgobjList[j];
    if (!(FProper2(i) &&
      (!FCusp(i) || RAbs(planetalt[i]) > rSmall ||
      MinDistance(planet[i], chouse[i-cuspLo+1]) > rSmall)))
      continue;
    sprintf(sz, is.fSeconds ? "%-3.3s: " : "%-4.4s: ", szObjDisp[i]);
    DrawPrint(sz, kObjB[i], fTrue);
    y = DrawZodiac(planet[i], fTrue);
    if (!is.fSeconds && i < starLo && gi.nMode != gSector && (gs.nScale ==
      100 || gs.nFont == 0 || !gi.fFile || gs.ft == ftBmp) && y < gs.yWin-1) {
      // Don't draw planet glyph in PS or Metafile, since can't be resized.
      s = gi.nScale;
      gi.nScale = gi.nScaleTextT;
      DrawObject(-i-1, gs.xWin-12*gi.nScale, y-(yFont/2-1)*gi.nScale);
      gi.nScale = s;
    }
    sprintf(sz, "%c ", ret[i] < 0.0 ? chRet : ' ');
    DrawPrint(sz, gi.kiOn, fTrue);
    if (gi.nMode != gSector || !is.fSeconds) {
      is.fSeconds = fFalse;
      DrawPrint(SzAltitude(planetalt[i]), gi.kiLite, fTrue);
      is.fSeconds = us.fSeconds;
    }
    if (gi.nMode == gSector) {
      r = GFromO(cp1.obj[i]); s = (int)r + 1;
      if (!is.fSeconds)
        sprintf(sz, " %2d", s);
      else
        sprintf(sz, "%6.3f%c", r + 1.0, pluszone[s] ? '+' : '-');
      DrawPrint(sz, pluszone[s] ? kRedB : kDkGreenB, fFalse);
    } else
      DrawPrint("", gi.kiOn, fFalse);
  }

  // Print star positions.

  for (i = starLo; i <= starHi; i++) {
    s = rgobjList[i];
    if (!FProper(s))
      continue;
    sprintf(sz, is.fSeconds ? "%3.3s: " : "%4.4s: ", szObjDisp[s]);
    DrawPrint(sz, kObjB[s], fTrue);
    DrawZodiac(planet[s], fTrue);
    DrawPrint("  ", gi.kiOn, fTrue);
    if (gi.nMode != gSector || !is.fSeconds) {
      is.fSeconds = fFalse;
      DrawPrint(SzAltitude(planetalt[s]), gi.kiLite, fTrue);
      is.fSeconds = us.fSeconds;
    }
    if (gi.nMode == gSector) {
      r = GFromO(cp1.obj[s]); s = (int)r + 1;
      if (!is.fSeconds)
        sprintf(sz, " %2d", s);
      else
        sprintf(sz, "%6.3f%c", r + 1.0, pluszone[s] ? '+' : '-');
      DrawPrint(sz, pluszone[s] ? kRedB : kDkGreenB, fFalse);
    } else
      DrawPrint("", gi.kiOn, fFalse);
  }

  // Print element table information.

  DrawPrint("", gi.kiLite, fFalse);
  CreateElemTable(&et);
  sprintf(sz, "Fire: %d,",   et.coElem[eFir]);
  DrawPrint(sz, kElemB[eFir], fTrue);
  sprintf(sz, " Earth: %d,", et.coElem[eEar]);
  DrawPrint(sz, kElemB[eEar], fFalse);
  sprintf(sz, "Air : %d,",   et.coElem[eAir]);
  DrawPrint(sz, kElemB[eAir], fTrue);
  sprintf(sz, " Water: %d",  et.coElem[eWat]);
  DrawPrint(sz, kElemB[eWat], fFalse);
  sprintf(sz, "Car: %d,",  et.coMode[0]); DrawPrint(sz, kModeB(0), fTrue);
  sprintf(sz, " Fix: %d,", et.coMode[1]); DrawPrint(sz, kModeB(1), fTrue);
  sprintf(sz, " Mut: %d",  et.coMode[2]); DrawPrint(sz, kModeB(2), fFalse);
  sprintf(sz, "Yang: %d, Yin: %d", et.coYang, et.coYin);
  DrawPrint(sz, gi.kiLite, fFalse);
  if (et.coMC > 9 && et.coIC > 9 && et.coAsc > 9 && et.coDes > 9)
    DrawPrint(NULL, -xFont2*gi.nScaleText/2, -1);
  sprintf(sz, "M: %d,",  et.coMC);  DrawPrint(sz, kElemB[eEar], fTrue);
  sprintf(sz, " N: %d,", et.coIC);  DrawPrint(sz, kElemB[eWat], fTrue);
  sprintf(sz, " A: %d,", et.coAsc); DrawPrint(sz, kElemB[eFir], fTrue);
  sprintf(sz, " D: %d",  et.coDes); DrawPrint(sz, kElemB[eAir], fFalse);
  sprintf(sz, "Ang: %d,",  et.coModeH[0]); DrawPrint(sz, kModeB(0), fTrue);
  sprintf(sz, " Suc: %d,", et.coModeH[1]); DrawPrint(sz, kModeB(1), fTrue);
  sprintf(sz, " Cad: %d",  et.coModeH[2]); DrawPrint(sz, kModeB(2), fFalse);
  sprintf(sz, "Learn: %d, Share: %d", et.coLearn, et.coShare);
  DrawPrint(sz, gi.kiLite, fFalse);
  us.fAnsiChar = a;
}


// Fill in the specified sector of a wheel chart at the given coordinates. The
// sector type may be sign, house, or Gauquelin sector.

void DrawFillWheel(int x, int y, int i, int typ)
{
  KV kvC, kvB, kvF;
  int nTrans, nRC, nGC, nBC, nRB, nGB, nBB, nRF, nGF, nBF;

  if (gs.nDecaFill <= 0)
    return;

  // Figure out what RGB colors to use to fill.
  if (!gi.fFile || gi.fBmp) {
    nTrans = (int)(gs.rBackPct * 256.0 / 100.0);
    if (nTrans <= 0)
      return;
    if (gs.nDecaFill == 1)
      kvC = rgbbmp[gi.kiCur];
    else
      kvC = KvHue((real)(i-1) * rDegMax / (real)(typ < 2 ? cSign : cSector));
    kvB = rgbbmp[gi.kiOff];
    nRC = RgbR(kvC); nGC = RgbG(kvC); nBC = RgbB(kvC);
    nRB = RgbR(kvB); nGB = RgbG(kvB); nBB = RgbB(kvB);
    nRF = nRB + ((nRC - nRB) * nTrans >> 8);
    nGF = nGB + ((nGC - nGB) * nTrans >> 8);
    nBF = nBB + ((nBC - nBB) * nTrans >> 8);
    kvF = Rgb(nRF, nGF, nBF);
#ifdef EXPRESS
    // Modify RGB color through AstroExpression if one defined.
    if (!us.fExpOff && FSzSet(us.szExpColFill)) {
      ExpSetN(iLetterX, typ);
      ExpSetN(iLetterY, i);
      ExpSetN(iLetterZ, kvF);
      ParseExpression(us.szExpColFill);
      kvF = NExpGet(iLetterZ);
    }
#endif
  } else
    kvF = -1;

  // Actually go fill in the area.
  DrawFill(x, y, kvF);
}


// This is a subprocedure of XChartWheel() and XChartWheelRelation(). Draw
// the outer sign and house rings for a wheel chart at the specified zodiac
// locations and at the given radius values.

void DrawWheel(real *xsign, real *xhouse, int cx, int cy, real unitx,
  real unity, real rh1, real rh2, real rs1)
{
  int nTrans = (int)(gs.rBackPct * 256.0 / 100.0), *rgRules, i, j, k, x, y,
    nSav;
  real rh, rs, rs2 = 0.95, r9 = 0.99, ra, rb, px, py, rDeg;
  flag fVector = (gs.ft == ftPS || gs.ft == ftWmf), fSav;

  rh = (rh1 + rh2) / 2.0; rs = (rs1 + rs2) / 2.0;

  // Draw small five or one degree increments around the zodiac sign ring.

  fSav = us.fHouse3D; us.fHouse3D = fFalse;
  for (i = 0; i < nDegMax; i++) {
    rDeg = PZ(HousePlaceInX((real)i, 0.0));
    px = PX(rDeg); py = PY(rDeg);
    DrawColor(i%10 ? gi.kiGray : (gs.fColorSign ? kSignB(i/30 + 1) : gi.kiOn));
    k = (!gs.fColor && !fVector && i%5 > 0);
    if (i%5 > 0) {
      ra = rh2 + (rs1 - rh2) * 0.30;
      rb = rh2 + (rs1 - rh2) * 0.70;
    } else {
      ra = rh2; rb = rs1;
      if (us.fListDecan && i%10 == 0 && i%30 != 0)
        rb += (rs2 - rs1) * 0.30;
    }
    DrawDash(cx+POINT1(unitx, ra, px), cy+POINT1(unity, ra, py),
      cx+POINT2(unitx, rb, px), cy+POINT2(unity, rb, py), k);
  }
  us.fHouse3D = fSav;

  // Draw circles for the zodiac sign rings.

  DrawColor(gi.kiOn);
  DrawCircle(cx, cy, (int)(unitx*rs2+rRound), (int)(unity*rs2+rRound));
  DrawCircle(cx, cy, (int)(unitx*rs1+rRound), (int)(unity*rs1+rRound));

  // Draw the glyphs for the signs themselves.

  for (i = 1; i <= cSign; i++) {
    rDeg = xsign[i];
    // Draw lines separating each sign from each other.
    DrawColor(gs.fColorSign ? kSignB(i) : gi.kiOn);
    px = PX(rDeg); py = PY(rDeg);
    DrawLine(cx+POINT2(unitx, rs2, px), cy+POINT2(unity, rs2, py),
      cx+POINT1(unitx, rs1, px), cy+POINT1(unity, rs1, py));
  }
  if (us.fListDecan)
    rs = rs1 + (rs2 - rs1) * 0.67;
  for (i = 1; i <= cSign; i++) {
    rDeg = Midpoint(xsign[i], xsign[Mod12(i+1)]);
    DrawColor(kSignB(i));
    x = cx+POINT0(unitx, rs, PX(rDeg));
    y = cy+POINT0(unity, rs, PY(rDeg));
    DrawFillWheel(x, y, i, 0);
    if (nTrans >= 128)
      DrawColor(gi.kiOn);
    DrawSign(i, x, y);
    // Draw decan rulers if specified.
    if (us.fListDecan) {
      if (ignore7[rrStd] && ignore7[rrEso] && !ignore7[rrHie])
        rgRules = rgSignHie1;
      else if (ignore7[rrStd] && !ignore7[rrEso])
        rgRules = rgSignEso1;
      else
        rgRules = rules;
      for (j = 0; j < 3; j++) {
        rDeg = ZFromS(i) + (real)j*10.0 + 5.0;
        k = rgRules[SFromZ(Decan(rDeg))];
        rDeg = PZ(HousePlaceInX(rDeg, 0.0));
        ra = rs1 + (rs2 - rs1) * 0.20;
        nSav = gi.nScale;
        gi.nScale = (gi.nScale + 1) >> 1;
        x = cx+POINT0(unitx, ra, PX(rDeg));
        y = cy+POINT0(unity, ra, PY(rDeg));
#ifdef EXPRESS
        // Notify AstroExpression a decan symbol is about to be drawn.
        if (!us.fExpOff && FSzSet(us.szExpDecan)) {
          ExpSetN(iLetterX, i);
          ExpSetN(iLetterY, j);
          ExpSetN(iLetterZ, k);
          ParseExpression(us.szExpDecan);
          k = NExpGet(iLetterZ);
        }
#endif
        if (k >= 0 && FItem(k))
          DrawObject(k, x, y);
        else if (FValidSign(-k))
          DrawSign(-k, x, y);
        gi.nScale = nSav;
      }
    }
  }

  // Draw Ascendant/Descendant and Midheaven/Nadir lines across whole chart.

  DrawColor(gs.fColorHouse ? kSignB(sAri) : gi.kiLite);
  DrawDash(cx+POINT1(unitx, r9, PX(xhouse[sAri])),
           cy+POINT1(unity, r9, PY(xhouse[sAri])), cx, cy, !gs.fColor);
  if (gs.fColorHouse) DrawColor(kSignB(sLib));
  DrawDash(cx+POINT1(unitx, r9, PX(xhouse[sLib])),
           cy+POINT1(unity, r9, PY(xhouse[sLib])), cx, cy, !gs.fColor);
  if (gs.fColorHouse) DrawColor(kSignB(sCap));
  DrawDash(cx+POINT1(unitx, r9, PX(xhouse[sCap])),
           cy+POINT1(unity, r9, PY(xhouse[sCap])), cx, cy, !gs.fColor);
  if (gs.fColorHouse) DrawColor(kSignB(sCan));
  DrawDash(cx+POINT1(unitx, r9, PX(xhouse[sCan])),
           cy+POINT1(unity, r9, PY(xhouse[sCan])), cx, cy, !gs.fColor);

  // Draw circles for the house rings.

  DrawColor(gi.kiOn);
  DrawCircle(cx, cy, (int)(unitx*rh2+rRound), (int)(unity*rh2+rRound));
  DrawCircle(cx, cy, (int)(unitx*rh1+rRound), (int)(unity*rh1+rRound));
  if (nTrans > 0) {
    // Draw sign rings again to cover up angle lines drawn over them above.
    DrawCircle(cx, cy, (int)(unitx*rs2+rRound), (int)(unity*rs2+rRound));
    DrawCircle(cx, cy, (int)(unitx*rs1+rRound), (int)(unity*rs1+rRound));
  }

  // Draw hatches within houses, if -YRd switch set.

  k = us.nSignDiv;
  if (k > 1) {
    ra = rh1 + (rh - rh1) * 0.25;
    rb = rh1 - (rh - rh1) * 0.25;
    for (i = 1; i <= cSign; i++)
      for (j = 0; j < k; j++) {
        rDeg = xhouse[i] +
          MinDifference(xhouse[i], xhouse[Mod12(i+1)]) * (real)j / (real)k;
        px = PX(rDeg); py = PY(rDeg);
        DrawColor(gs.fColorHouse ? kSignB(i) : gi.kiOn);
        DrawLine(cx+POINT2(unitx, j ? ra : rh1, px),
          cy+POINT2(unity, j ? ra : rh1, py),
          cx+POINT1(unitx, rb, px),
          cy+POINT1(unity, rb, py));
      }
  }

  // Draw the glyphs for the houses themselves.

  for (i = 1; i <= cSign; i++) {
    DrawColor(gs.fColorHouse ? kSignB(i) : gi.kiOn);
    px = PX(xhouse[i]); py = PY(xhouse[i]);
    DrawLine(cx+POINT2(unitx, rh2, px), cy+POINT2(unity, rh2, py),
      cx+POINT1(unitx, rh1, px), cy+POINT1(unity, rh1, py));
    // Draw minor lines from non-angular houses to center of wheel.
    if (i%3 != 1) {
      DrawColor(gs.fColorHouse ? kSignB(i) : gi.kiGray);
      DrawDash(cx, cy, cx+POINT2(unitx, rh1, px),
        cy+POINT2(unity, rh1, py), 1 + !gs.fColor*3);
    }
  }
  for (i = 1; i <= cSign; i++) {
    rDeg = Midpoint(xhouse[i], xhouse[Mod12(i+1)]);
    DrawColor(kSignB(i));
    x = cx+POINT0(unitx, rh, PX(rDeg));
    y = cy+POINT0(unity, rh, PY(rDeg));
    DrawFillWheel(x, y, i, 1);
    if (nTrans >= 128)
      DrawColor(gi.kiOn);
    DrawHouse(i, x, y);
  }
}


// Another subprocedure of XChartWheel() and XChartWheelRelation(). Draw a set
// of planets in a wheel chart, drawing each glyph and a line from it to a dot
// indicating the planet's actual location.

void DrawSymbolRing(real *symbol, real *xplanet, real *dir, int cx, int cy,
  real unitx, real unity, real rp, real rl1, real rl2, real rg)
{
  int i;
  real temp;

  for (i = is.nObj; i >= 0; i--) if (FProper(i)) {
    if (gs.fLabel) {
      temp = symbol[i];
      DrawColor(dir[i] < 0.0 ? gi.kiGray : gi.kiOn);
      DrawDash(cx+POINT1(unitx, rl1, PX(xplanet[i])),
        cy+POINT1(unity, rl1, PY(xplanet[i])),
        cx+POINT1(unitx, rl2, PX(temp)),
        cy+POINT1(unity, rl2, PY(temp)),
        (dir[i] < 0.0 ? 1 : 0) - gs.fColor);
      DrawObject(i, cx+POINT1(unitx, rg, PX(temp)),
        cy+POINT1(unity, rg, PY(temp)));
    } else
      DrawColor(kObjB[i]);
    if (!gs.fHouseExtra && us.nRel >= rcNone)
      DrawSpot(cx+POINT1(unitx, rp, PX(xplanet[i])),
        cy+POINT1(unity, rp, PY(xplanet[i])));
    else
      DrawPoint(cx+POINT1(unitx, rp, PX(xplanet[i])),
        cy+POINT1(unity, rp, PY(xplanet[i])));
  }
}


// Another subprocedure of XChartWheel() and XChartWheelRelation(). Draw one
// planet ring of a wheel chart at the specified locations and radius values.

void DrawRing(int iRing, int iRingMax,
  real xplanet[objMax], real symbol[objMax], int cx, int cy, real base,
  real ri1, real ri2, real rp, real rl1, real rl2, real rg, real rGlyph)
{
  CP *pcp = rgpcp[iRing];
  real unitx = (real)cx, unity = (real)cy;
  int i;

  FProcessCommandLine(szWheelX[iRing]);

  // Draw the planet glyphs and lines to where the planets actually are.
  for (i = 0; i <= is.nObj; i++)
    symbol[i] = xplanet[i];
  FillSymbolRing(symbol, rGlyph);
  DrawSymbolRing(symbol, xplanet, pcp->dir, cx, cy, unitx, unity,
    rp, rl1, rl2, rg);
  for (i = 0; i <= is.nObj; i++)
    if (!FProper(i))
      xplanet[i] = -1.0;
  if (ri1 >= ri2)
    return;

  // Draw dotted lines from this ring to the innermost ring of a multiwheel.
  for (i = is.nObj; i >= 0; i--) if (FProper(i)) {
    DrawColor(kObjB[i]);
    DrawPoint(cx+POINT1(unitx, base, PX(xplanet[i])),
      cy+POINT1(unity, base, PY(xplanet[i])));
    if (!gs.fHouseExtra) {
      DrawColor(pcp->dir[i] < 0.0 ? gi.kiGray : gi.kiOn);
      DrawDash(cx+POINT1(unitx, ri1, PX(xplanet[i])),
        cy+POINT1(unity, ri1, PY(xplanet[i])),
        cx+POINT2(unitx, ri2, PX(xplanet[i])),
        cy+POINT2(unity, ri2, PY(xplanet[i])),
        1 + Min(iRingMax-iRing, 3) - gs.fColor);
    }
  }
}


// Another drawing routine similar to above, plot a set of planets within a 2D
// expanse of pixels, so that there's a minimum of overlapping glyphs.

void DrawObjects(ObjDraw *rgod, int cod, int zEdge)
{
  int zGlyph, zGlyph2, zGlyphS, zGlyphS2, i, j, k, k2, obj;
  KI kSav;

  // Define or adjust some initial values.
  zGlyph = 7*gi.nScale; zGlyphS = 9*gi.nScaleTextT;
  zGlyph2 = zGlyph << 1; zGlyphS2 = zGlyphS << 1;

  // Assume glyph is positioned below actual point, unless say otherwise.
  for (i = 0; i < cod; i++) if (rgod[i].f) {
    obj = rgod[i].obj;
    rgod[i].yg = rgod[i].y + (obj < starLo ? zGlyph : zGlyphS);
  }

  // Determine where to draw the glyphs in relation to the actual points,
  // so that the glyphs aren't drawn on top of each other if possible.
  for (i = 0; i < cod; i++) if (rgod[i].f) {
    obj = rgod[i].obj;
    k = k2 = gs.xWin + gs.yWin;

    // For each planet, will draw glyph either right over or right under the
    // actual planet location point. Find out the closest distance of any
    // other planet assuming glyph is placed at both possibilities.
    for (j = 0; j < i; j++) if (rgod[j].f) {
      k  = Min(k,  NAbs(rgod[i].x-rgod[j].x) + NAbs(rgod[i].yg-rgod[j].yg));
      k2 = Min(k2, NAbs(rgod[i].x-rgod[j].x) + NAbs(rgod[i].yg-rgod[j].yg -
        zGlyph2));
    }

    // Normally, put the glyph right below the actual point. If however
    // another planet is close enough to have their glyphs overlap, and the
    // above location is better, then will draw the glyph above instead.
    if ((k < zGlyph2 || k2 < zGlyph2) && k < k2)
      rgod[i].yg -= (obj < starLo ? zGlyph2 : zGlyphS2);
  }

  // Draw planet glyphs.
  for (i = cod-1; i >= 0; i--) if (rgod[i].f) {
    if (zEdge > 0 && !FInRect(rgod[i].x, rgod[i].yg,
      zEdge, zEdge, gs.xWin-zEdge, gs.yWin-zEdge))
      continue;
    obj = rgod[i].obj;
    if (rgod[i].kv != ~0) {
      kSav = kObjB[obj]; kObjB[obj] = rgod[i].kv;
    }
    DrawObject(rgod[i].obj, rgod[i].x, rgod[i].yg);
    if (rgod[i].kv != ~0)
      kObjB[obj] = kSav;
  }

  // Draw dots for actual object location.
  for (i = cod-1; i >= 0; i--) if (rgod[i].f) {
    if (zEdge > 0 && !FInRect(rgod[i].x, rgod[i].y,
      zEdge, zEdge, gs.xWin-zEdge, gs.yWin-zEdge))
      continue;
    obj = rgod[i].obj;
    DrawColor(rgod[i].kv != ~0 ? rgod[i].kv : kObjB[obj]);
    // Draw small or large dot near glyph indicating exact object location.
    if ((gi.nMode == gHorizon && (gs.fAlt || obj > oNorm)) ||
      (gi.nMode == gOrbit && gs.fAlt))
      DrawPoint(rgod[i].x, rgod[i].y);
    else
      DrawSpot(rgod[i].x, rgod[i].y);
  }
}


// Draw an aspect line between two points in a wheel chart.

void DrawAspectLine(int obj1, int obj2, int cx, int cy,
  real deg1, real deg2, real rx, real ry, real rz)
{
  int asp = grid->n[obj1][obj2], orb = grid->v[obj1][obj2],
    x1, y1, x2, y2, nDash, nSav, col;

  // Get color of aspect. Modify it through AstroExpression if one defined.
  col = kAspB[asp];
#ifdef EXPRESS
  if (!us.fExpOff && FSzSet(us.szExpColAsp)) {
    ExpSetN(iLetterW, obj1);
    ExpSetN(iLetterX, asp);
    ExpSetN(iLetterY, obj2);
    ExpSetN(iLetterZ, col);
    ParseExpression(us.szExpColAsp);
    col = NExpGet(iLetterZ);
    if (!FValidColor(col))
      col = 0;
  }
#endif
  DrawColor(col);

  // Compute coordinates of line, and draw it.
  x1 = cx + POINT1(rx, rz, PX(deg1));
  y1 = cy + POINT1(ry, rz, PY(deg1));
  x2 = cx + POINT1(rx, rz, PX(deg2));
  y2 = cy + POINT1(ry, rz, PY(deg2));
  if ((gs.nDashMax >= 0) != gs.fAlt)
    nDash = NAbs(orb / (60*60*2));
  else
    nDash = NAbs(orb) * NAbs(gs.nDashMax) /
      (int)(GetOrb(obj1, obj2, asp)*3600.0);
  DrawDash(x1, y1, x2, y2, nDash);

  // Draw aspect glyph over middle of line.
  if (gs.fLabelAsp) {
    nSav = gi.nScale;
    gi.nScale = gi.nScaleTextT;
    DrawAspect(asp, (x1 + x2) >> 1, (y1 + y2) >> 1);
    gi.nScale = nSav;
  }
}


/*
******************************************************************************
** Map Chart Routines.
******************************************************************************
*/

// Another stream reader, this one is used by the globe drawing routine. For
// the next body of land/water, return its name (and color), its longitude and
// latitude, and a vector description of its outline.

flag FReadWorldData(CONST char **nam, CONST char **loc, CONST char **lin)
{
  static CONST char **psz = szWorldData;
  int i;

  *loc = *psz++;
  *lin = *psz++;
  *nam = *psz++;
  if (*loc[0]) {
    if (gs.fPrintMap && gi.fFile) {
      i = **nam - '0';
      AnsiColor(i ? kRainbowA[i] : kDkBlueA);
      PrintSz(*nam+1); PrintL();
    }
    return fTrue;
  }
  psz = szWorldData;  // Reset stream when no data left.
  return fFalse;
}


// Enumerator to return line segments (and their color) composing Astrolog's
// map of the world. Used by astro-graph, map/globe, and nearest city charts.

flag EnumWorldLines(int *x1, int *y1, int *x2, int *y2, int *kRainbow)
{
  static CONST char **psz;
  static CONST char *lin;
  static int xold, yold;
  CONST char *nam, *loc;
  char chCmd;
  int lon, lat, x, y, i, k = -1;

  // Call with Null pointer to initialize new enumeration.
  if (x1 == NULL) {
    psz = szWorldData;
    lin = "";
    return fTrue;
  }

  // Get the next chunk of data to process.
  if (!*lin) {

    // Get data for the next coastline piece.
    loc = *psz++;
    lin = *psz++;
    nam = *psz++;
    if (loc[0]) {
      i = nam[0]-'0';
      if (gs.fPrintMap && gi.fFile) {
        AnsiColor(i ? kRainbowA[i] : kDkBlueA);
        PrintSz(nam+1); PrintL();
      }
    } else
      return fFalse;
    k = i;
    lon = (loc[0] == '+' ? 1 : -1)*
      ((loc[1]-'0')*100 + (loc[2]-'0')*10 + (loc[3]-'0'));
    lat = (loc[4] == '+' ? 1 : -1)*((loc[5]-'0')*10 + (loc[6]-'0'));
    xold = nDegHalf - lon;
    yold = 90 - lat;
  }
  x = xold; y = yold;

  // Get the next unit from the world map string.
  chCmd = *lin++;

  // Each unit is exactly one character in the coastline string.
  if (chCmd == 'L' || chCmd == 'H' || chCmd == 'G')
    x--;
  else if (chCmd == 'R' || chCmd == 'E' || chCmd == 'F')
    x++;
  if (chCmd == 'U' || chCmd == 'H' || chCmd == 'E')
    y--;
  else if (chCmd == 'D' || chCmd == 'G' || chCmd == 'F')
    y++;

  // Take care of coordinate wrap around.
  while (x >= nDegMax)
    x -= nDegMax;
  while (x < 0)
    x += nDegMax;

  *x1 = xold; *y1 = yold; *x2 = x; *y2 = y;
  *kRainbow = k;
  xold = x; yold = y;
  return fTrue;
}


#ifdef CONSTEL
// Enumerator to return line segments (and their constellation) composing
// Astrolog's map of constellation boundaries. Used by local horizon,
// map/globe, chart sphere, and telescope charts.

flag EnumConstelLines(int *x1, int *y1, int *x2, int *y2, int *iConst)
{
  static int isz, nC, xold = 0, yold = 0, xDelta, yDelta, xLo, xHi, yLo, yHi;
  static CONST char *pch;
  int lon, lat, x, y;
  char chCmd;
  flag fBlank;

  // Call with Null pointer to initialize new enumeration.
  if (x1 == NULL) {
    isz = nC = 0;
    pch = "";
    return fTrue;
  }

  // Get the next chunk of data to process.
LNext:
  x = xold; y = yold;
  if (!*pch && nC <= 0) {

    if (isz > 0) {
      // If reached the end of current constellation, compute the center
      // location in it based on lower and upper bounds that have been
      // maintained, and print the name of the constellation there.

      x = xLo + (xHi - xLo)*(szDrawConstel[isz][0]-'1')/8;
      y = yLo + (yHi - yLo)*(szDrawConstel[isz][1]-'1')/8;
      if (x < 0)
        x += nDegMax;
      else if (x > nDegMax)
        x -= nDegMax;
      *x1 = *x2 = x;
      *y1 = *y2 = y;
      *iConst = isz;
      neg(isz);
      return fTrue;
    } else if (isz < 0)
      neg(isz);

    // Get data for the next constellation shape.
    isz++;
    if (isz > cCnstl)
      return fFalse;
    pch = szDrawConstel[isz];
    lon = nDegMax -
      (((pch[2]-'0')*10 + (pch[3]-'0'))*15 + (pch[4]-'0')*10 + (pch[5]-'0'));
    lat = 90-((pch[6] == '-' ? -1 : 1) * ((pch[7]-'0')*10 + (pch[8]-'0')));
    if (lon >= nDegMax)
      lon -= nDegMax;
    pch += 9;
    xLo = xHi = xold = x = lon;
    yLo = yHi = yold = y = lat;
  }

  // Get the next unit from the string to draw on the screen as a line. For
  // constellations have a cache of how long should keep going in the previous
  // direction, for example "u5" for up five moves pointer up five times
  // without advancing string pointer.

  fBlank = fFalse;
  loop {
    if (nC <= 0) {
      chCmd = *pch++;

      // Get the next direction and distance from constellation string.
      if (fBlank = (chCmd == 'b'))
        chCmd = *pch++;
      xDelta = yDelta = 0;
      switch (chCmd) {
      case 'u': yDelta = -1; break;    // Up
      case 'd': yDelta =  1; break;    // Down
      case 'l': xDelta = -1; break;    // Left
      case 'r': xDelta =  1; break;    // Right
      case 'U': yDelta = -1; nC = (y-1)%10+1;    break;  // Up until
      case 'D': yDelta =  1; nC = 10-y%10;       break;  // Down until
      case 'L': xDelta = -1; nC = (x+599)%15+1;  break;  // Left until
      case 'R': xDelta =  1; nC = 15-(x+600)%15; break;  // Right until
      default: PrintError("Bad constel enum.");          // Shouldn't happen.
      }
      if (chCmd >= 'a')
        nC = NFromPch(&pch);  // Figure out how far to draw.
    }
    nC--;
    x += xDelta; y += yDelta;
    if (!fBlank)
      break;
    // Occasionally want to move the pointer without drawing anything.
    xold = x; yold = y;     
    continue;
  }

  if (x < xLo)              // Maintain bounding rectangle for this
    xLo = x;                // constellation if have crossed over it.
  else if (x > xHi)
    xHi = x;
  if (y < yLo)
    yLo = y;
  else if (y > yHi)
    yHi = y;

  // Take care of coordinate wrap around.
  *x1 = xold; *y1 = yold; 
  xold = x; yold = y;
  if (!(x > *x1 || y > *y1))  // draw each line segment once instead of twice.
    goto LNext;
  while (x >= nDegMax)
    x -= nDegMax;
  while (x < 0)
    x += nDegMax;
  *x2 = x; *y2 = y;
  *iConst = 0;
  return fTrue;
}
#endif


// Given longitude and latitude values on a globe, return the window
// coordinates corresponding to them. In other words, project the globe
// onto the view plane, and return where our coordinates got projected to,
// as well as whether our location is hidden on the back side of the globe.

flag FGlobeCalc(real x1, real y1, int *u, int *v, CONST CIRC *pcr, real deg)
{
  real rT, siny1;

  // Chart sphere coordinates are relative to the local horizon.

  if (gi.nMode == gSphere) {
    y1 = rDegQuad - y1;
    if (!gs.fEcliptic) {
      x1 = Mod(x1 + is.lonMC + rDegQuad);
      EquToLocal(&x1, &y1, rDegQuad - is.latMC);
    } else {
      EclToEqu(&x1, &y1);
      x1 = Mod(Tropical(x1) - rDegQuad);
    }
    y1 = rDegQuad - y1;
  }

  // Globe coordinates may be relative to the ecliptic.

  if ((gi.nMode == gGlobe || gi.nMode == gPolar) && gs.fEcliptic) {
    y1 = rDegQuad - y1;
    if (!gs.fConstel) {
      x1 = Mod(x1 + is.lonMC - rDegHalf + Lon);
      EquToEcl(&x1, &y1);
      x1 = Untropical(x1);
    } else
      EclToEqu(&x1, &y1);
    y1 = rDegQuad - y1;
  }

  // Compute coordinates for a general globe invoked with -XG switch.

  if (gi.nMode != gPolar) {
    // Shift by current globe rotation value.
    x1 = Mod(x1+deg);
    // Do another coordinate shift if the globe's equator is tilted any.
    if (gs.rTilt != 0.0) {
      y1 = rDegQuad - y1;
      CoorXform(&x1, &y1, gs.rTilt);
      x1 = Mod(x1); y1 = rDegQuad - y1;
    }
    *v = pcr->yc + (int)((real)pcr->yr*-RCosD(y1) - rRound);
    *u = pcr->xc + (int)((real)pcr->xr*-RCosD(x1)*RSinD(y1) - rRound);
    return x1 > rDegHalf;
  }

  // Compute coordinates for a polar globe invoked with -XP switch.

  siny1 = RSinD((gs.fSouth ? nDegHalf-y1 : y1) / 2.0);
  rT = gs.fSouth ? rDegQuad+x1+deg : 270.0-x1-deg;
  *v = pcr->yc + (int)(siny1*(real)pcr->yr*RSinD(rT) - rRound);
  *u = pcr->xc + (int)(siny1*(real)pcr->xr*RCosD(rT) - rRound);
  return fFalse;
}


// Given longitude and latitude values, return the pixel coordinates
// corresponding to them. Covers maps, globes, and chart spheres.

flag FMapCalc(real x1, real y1, int *xp, int *yp, flag fGlobe, flag fSky,
  real rT, int nScl, CONST CIRC *pcr, real deg)
{
  int j, k, u, v;

  if (fSky >= 0) {
    if (!fSky)
      x1 = is.lonMC - x1;
    x1 = Mod(rDegHalf - rT - x1);
    y1 = rDegQuad - y1;
  }
  if (fGlobe) {
    j = FGlobeCalc(x1, y1, &u, &v, pcr, deg) ? nNegative : u;
    k = v;
  } else {
    j = (int)(x1 * (real)nScl);
    k = (int)(y1 * (real)nScl);
    if (gs.fMollewide)
      j = 180*nScl + (int)((x1-180.0) * RMollewide(y1-90.0) / 180.0 + rRound);
  }
  *xp = j; *yp = k;
  return (j == nNegative);
}


// Draw one "Ley line" on the world map, based coordinates given in terms of
// longitude and vertical fractional distance from the center of the earth.

void DrawLeyLine(real l1, real f1, real l2, real f2)
{
  l1 = Mod(l1); l2 = Mod(l2);

  // Convert vertical fractional distance to a corresponding coordinate.
  f1 = rDegQuad - RAsin(f1)/rPiHalf*rDegQuad;
  f2 = rDegQuad - RAsin(f2)/rPiHalf*rDegQuad;
  DrawWrap((int)(l1*(real)gi.nScale+rRound),
           (int)(f1*(real)gi.nScale+rRound),
           (int)(l2*(real)gi.nScale+rRound),
           (int)(f2*(real)gi.nScale+rRound), 0, gs.xWin-1);
}


// Draw the main set of planetary Ley lines on the map of the world. This
// consists of drawing an icosahedron and then a dodecahedron lattice.

void DrawLeyLines(real deg)
{
  real h, h1, h2, r, i;

  h = 1.0/(rPhi*2.0-1.0);                      // Icosahedron constant.
  DrawColor(kDkCyanB);
  for (i = deg; i < rDegMax+deg; i += 72.0) {  // Draw icosahedron edges.
    DrawLeyLine(i, h, i+72.0, h);
    DrawLeyLine(i-36.0, -h, i+36.0, -h);
    DrawLeyLine(i, h, i, 1.0);
    DrawLeyLine(i+36.0, -h, i+36.0, -1.0);
    DrawLeyLine(i, h, i+36.0, -h);
    DrawLeyLine(i, h, i-36.0, -h);
  }
  r = 1.0/rSqr3/rPhi/RCosD(54.0);              // Dodecahedron constants.
  h2 = RSqr(1.0-r*r); h1 = h2/(rPhi*2.0+1.0);
  DrawColor(kMaroonB);
  for (i = deg; i < rDegMax+deg; i += 72.0) {  // Draw docecahedron edges.
    DrawLeyLine(i-36.0, h2, i+36.0, h2);
    DrawLeyLine(i, -h2, i+72.0, -h2);
    DrawLeyLine(i+36.0, h2, i+36.0, h1);
    DrawLeyLine(i, -h2, i, -h1);
    DrawLeyLine(i+36.0, h1, i+72.0, -h1);
    DrawLeyLine(i+36.0, h1, i, -h1);
  }
}


// Draw a line between two coordinates on the globe, along the great circle
// between them.

void DrawMapLine(real lon1, real lat1, real lon2, real lat2,
  flag fGlobe, int nScl, CIRC *pcr, real deg)
{
  int x1, y1, x2, y2;
  real lon3, lat3;

  // Recursive case: Draw each half of the line separately.
  if (SphDistance(lon1, lat1, lon2, lat2) > 10.0) {
    SphRatio(lon1, lat1, lon2, lat2, 0.5, &lon3, &lat3);
    DrawMapLine(lon1, lat1, lon3, lat3, fGlobe, nScl, pcr, deg);
    DrawMapLine(lon3, lat3, lon2, lat2, fGlobe, nScl, pcr, deg);
    return;
  }

  // Base case: Draw a single line segment.
  lat1 = rDegQuad - lat1;
  lat2 = rDegQuad - lat2;
  if (!FMapCalc(lon1, lat1, &x1, &y1, fGlobe, -1, 0.0, nScl, pcr, deg) &&
    !FMapCalc(lon2, lat2, &x2, &y2, fGlobe, -1, 0.0, nScl, pcr, deg)) {
    if (gi.nMode != gWorldMap || gs.fMollewide) {
      if (NAbs(x2-x1) < (pcr->xr >> 2))
        DrawLine(x1, y1, x2, y2);
    } else {
      if (lat1 <= 0.0 || lat1 >= rDegHalf)
        x1 = x2;
      else if (lat2 <= 0.0 || lat2 >= rDegHalf)
        x2 = x1;
      DrawWrap(x1, Min(y1, gs.yWin-1), x2, Min(y2, gs.yWin-1), 0, gs.xWin-1);
    }
  }
}


// Draw a triangle between three coordinates on the globe, in which other
// triangles should be nested a specified number of levels deep.

void DrawMapTriangle(real lon1, real lat1, real lon2, real lat2, real lon3,
  real lat3, flag fGlobe, int nScl, CIRC *pcr, real deg, int nLevel, int grf)
{
  real x1, y1, x2, y2, x3, y3;

  // Recursive case: Draw four smaller triangles composing this triangle.
  if (nLevel > 0) {
    SphRatio(lon1, lat1, lon2, lat2, 0.5, &x1, &y1);
    SphRatio(lon2, lat2, lon3, lat3, 0.5, &x2, &y2);
    SphRatio(lon3, lat3, lon1, lat1, 0.5, &x3, &y3);
    DrawMapTriangle(x1,   y1,   x2, y2, x3, y3, fGlobe, nScl, pcr, deg,
      nLevel-1, 7);
    DrawMapTriangle(lon1, lat1, x1, y1, x3, y3, fGlobe, nScl, pcr, deg,
      nLevel-1, grf & 5);
    DrawMapTriangle(lon2, lat2, x1, y1, x2, y2, fGlobe, nScl, pcr, deg,
      nLevel-1, grf & 5);
    DrawMapTriangle(lon3, lat3, x2, y2, x3, y3, fGlobe, nScl, pcr, deg,
      nLevel-1, grf & 5);
    return;
  }

  // Base case: Draw the three sides of the triangle.
  if (!fGlobe) {
    lon1 = Mod(lon1 + deg);
    lon2 = Mod(lon2 + deg);
    lon3 = Mod(lon3 + deg);
  }
  if (grf & 1)
    DrawMapLine(lon1, lat1, lon2, lat2, fGlobe, nScl, pcr, deg);
  if (grf & 2)
    DrawMapLine(lon2, lat2, lon3, lat3, fGlobe, nScl, pcr, deg);
  if (grf & 4)
    DrawMapLine(lon3, lat3, lon1, lat1, fGlobe, nScl, pcr, deg);
}


// Draw a square between four coordinates on the globe, in which other
// squares should be nested a specified number of levels deep.

void DrawMapSquare(real lon1, real lat1, real lon2, real lat2,
  real lon3, real lat3, real lon4, real lat4,
  flag fGlobe, int nScl, CIRC *pcr, real deg, int nLevel, int grf)
{
  real x0, y0, x1, y1, x2, y2, x3, y3, x4, y4;

  // Recursive case: Draw four smaller squares composing this square.
  if (nLevel > 0) {
    // Square's center is the midpoint of the midpoints of opposite corners.
    SphRatio(lon1, lat1, lon3, lat3, 0.5, &x1, &y1);
    SphRatio(lon2, lat2, lon4, lat4, 0.5, &x2, &y2);
    SphRatio(x1, y1, x2, y2, 0.5, &x0, &y0);

    SphRatio(lon1, lat1, lon2, lat2, 0.5, &x1, &y1);
    SphRatio(lon2, lat2, lon3, lat3, 0.5, &x2, &y2);
    SphRatio(lon3, lat3, lon4, lat4, 0.5, &x3, &y3);
    SphRatio(lon4, lat4, lon1, lat1, 0.5, &x4, &y4);
    DrawMapSquare(lon1, lat1, x1, y1, x0, y0, x4, y4, fGlobe, nScl, pcr, deg,
      nLevel-1, (grf | 2) & 11);
    DrawMapSquare(x1, y1, lon2, lat2, x2, y2, x0, y0, fGlobe, nScl, pcr, deg,
      nLevel-1, (grf | 4) &  7);
    DrawMapSquare(x0, y0, x2, y2, lon3, lat3, x3, y3, fGlobe, nScl, pcr, deg,
      nLevel-1, (grf | 8) & 14);
    DrawMapSquare(x4, y4, x0, y0, x3, y3, lon4, lat4, fGlobe, nScl, pcr, deg,
      nLevel-1, (grf | 1) & 13);
    return;
  }

  // Base case: Draw the four sides of the square.
  if (!fGlobe) {
    lon1 = Mod(lon1 + deg);
    lon2 = Mod(lon2 + deg);
    lon3 = Mod(lon3 + deg);
    lon4 = Mod(lon4 + deg);
  }
  if (grf & 1)
    DrawMapLine(lon1, lat1, lon2, lat2, fGlobe, nScl, pcr, deg);
  if (grf & 2)
    DrawMapLine(lon2, lat2, lon3, lat3, fGlobe, nScl, pcr, deg);
  if (grf & 4)
    DrawMapLine(lon3, lat3, lon4, lat4, fGlobe, nScl, pcr, deg);
  if (grf & 8)
    DrawMapLine(lon4, lat4, lon1, lat1, fGlobe, nScl, pcr, deg);
}


// This major routine draws all of Astrolog's map charts. This means either
// the world map or the constellations, in either rectangular or globe
// hemisphere form. The rectangular chart may also be done in a Mollewide
// projection, for six total combinations. We shift the chart by specified
// rotational and tilt values, and may draw on the chart each planet at its
// zenith position on Earth or location among the constellations.

void DrawMap(flag fSky, flag fGlobe, real deg)
{
  int cx = gs.xWin/2, cy = gs.yWin/2, rx, ry, unit = 12*gi.nScale,
    nScl = gi.nScale, x, y, xold, yold, m, n, u, v, i, j, k, l;
  flag fNext = fTrue, fSimple, fDir = (gi.nMode == gSphere && gs.fSouth),
    fDidBitmap;
  real planet1[objMax], planet2[objMax], x1, y1, rT;
  ObjDraw rgod[objMax * arMax];
  CIRC cr;
#ifdef CONSTEL
  int xT, yT;
#endif
#ifdef SWISS
  ES es, *pes1, *pes2;
  int xp, yp, xp2, yp2;
#endif

  // Set up some variables.
  fDidBitmap = !fSky && FBmpDrawMap();
  rx = cx-1; ry = cy-1;
  if (gi.nMode == gSphere) {
    rx = cx - 7*gi.nScale; ry = cy - 7*gi.nScale;
  }
  cr.xc = cx; cr.yc = cy; cr.xr = rx; cr.yr = ry;
  if (fGlobe)
    fSimple = (gs.rTilt == 0.0 && gi.nMode == gGlobe && !gs.fEcliptic);

#ifdef CONSTEL
  // Draw a dot grid for large rectangular constellation charts.
  if (fSky && !fGlobe && !gs.fMollewide && us.fHouse3D &&
    gi.nScale/gi.nScaleT > 2)
    for (yT = 5; yT < nDegHalf; yT += 5)
      for (xT = 5; xT <= nDegMax; xT += 5) {
        DrawColor(xT % 15 == 0 && yT % 10 == 0 ? gi.kiOn : gi.kiGray);
        rT = (real)xT+deg;
        if (rT >= rDegMax)
          rT -= rDegMax;
        DrawPoint((int)(rT*(real)nScl), yT*nScl);
      }
#endif

  // Draw the map (either a constellation map, or a world map).

  if (fDidBitmap)
    goto LAfter;
#ifdef CONSTEL
  if (fSky)
    EnumConstelLines(NULL, NULL, NULL, NULL, NULL);
  else
#endif
    EnumWorldLines(NULL, NULL, NULL, NULL, NULL);
  while (
#ifdef CONSTEL
    fSky ? EnumConstelLines(&xold, &yold, &x, &y, &i) :
#endif
    EnumWorldLines(&xold, &yold, &x, &y, &i)) {
    if (fSky) {
      if (i > 0)
        DrawColor(gi.nMode == gSphere || (!gs.fAlt && gi.nMode != gPolar &&
          gi.nMode != gWorldMap) ? gi.kiGray : kDkGreenB);
      else
        DrawColor(gi.nMode == gSphere ? kPurpleB :
          (gs.fAlt ? kBlueB : kDkBlueB));
    } else {
      if (i >= 0)
        DrawColor((!fGlobe && gi.nMode == gAstroGraph) ? gi.kiOn :
          (!gs.fAlt && !gs.fColorHouse ? gi.kiGray :
          (i ? kRainbowB[i] : kDkBlueB)));
    }
    if (fGlobe) {
      // For globes, have to do a complicated transformation, and not draw
      // when hidden on the back side of the sphere. Be smart and only do slow
      // calculations when know will be visible.

      j = fTrue;
      if (fSimple && !gs.fSouth) {
        rT = (real)xold+deg;
        if (rT >= rDegMax)
          rT -= rDegMax;
        j &= rT <= rDegHalf;
        rT = (real)x+deg;
        if (rT >= rDegMax)
          rT -= rDegMax;
        j &= rT <= rDegHalf;
      }
      if (j) {
        k = FGlobeCalc((real)xold, (real)yold, &m, &n, &cr, deg) ^
          fDir;
#ifdef CONSTEL
        if (fSky && i > 0) {
          if (!k && gs.fText)
            DrawSz(szCnstlAbbrev[i], m, n, dtCent | dtScale2);
          continue;
        }
#endif
        l = FGlobeCalc((real)x, (real)y, &u, &v, &cr, deg) ^ fDir;
        if (!k && !l)
          DrawLine(m, n, u, v);
        else if (gs.fSouth && gi.nMode != gSphere) {
          if (k) DrawPoint(m, n);
          if (l) DrawPoint(u, v);
        }
      }
    } else {
      // Rectangular maps are much simpler, with screen coordinates
      // proportional to internal coordinates. For the Mollewide projection
      // have to apply a factor to the horizontal positioning though.

      m = (int)(Mod((real)xold + deg)*(real)nScl);
      u = (int)(Mod((real)x + deg)*(real)nScl);
      if (NAbs(u-m) <= nDegHalf) {
        n = yold*nScl;
        v = y*nScl;
        if (gs.fMollewide && gi.nMode != gAstroGraph) {
          j = nDegHalf*nScl;
          m = j + NMultDiv(m-j, NMollewide(yold-90), j);
          u = j + NMultDiv(u-j, NMollewide(y   -90), j);
        }
#ifdef CONSTEL
        if (fSky && i > 0) {
          if (gs.fText)
            DrawSz(szCnstlAbbrev[i], m, n, dtCent | dtScale2);
          continue;
        }
#endif
        DrawLine(m, n, u, v);
      }
    }
  }
LAfter:

  // Draw the outline of the map, either a circle around globes or a
  // Mollewide type ellipse for that type of rectangular chart.

  if (gs.fBorder && gi.nMode != gSphere) {
    DrawColor(gi.kiOn);
    if (!fGlobe) {
      if (gs.fMollewide && gi.nMode != gAstroGraph)
        for (j = -1; j <= 1; j += 2)
          for (xold = 0, y = 89; y >= 0; y--, xold = x)
            for (x = NMollewide(y), i = -1; i <= 1; i += 2)
              DrawLine(180*nScl + i*xold - (i==1), (90+j*(y+1))*nScl - (j==1),
                180*nScl + i*x - (i==1), (90+j*y)*nScl - (j==1));
    } else
      DrawEllipse(0, 0, gs.xWin-1, gs.yWin-1);
  }

  // Now, if in an appropriate bonus chart mode, then draw each planet at its
  // zenith or visible location on the globe or map (assuming it not hidden).

  if (gs.fAlt || gi.nMode == gAstroGraph || gi.nMode == gSphere)
    return;
  rT = gs.fConstel ? rDegHalf - (fGlobe ? 0.0 : deg) :
    (fGlobe ? Lon : Lon - deg);
  if (rT < 0.0)
    rT += rDegMax;
  for (i = 0; i <= is.nObj; i++) if (FProper(i)) {
    planet1[i] = Tropical(planet[i]);
    planet2[i] = planetalt[i];
    EclToEqu(&planet1[i], &planet2[i]);    // Calculate zenith long. & lat.
  }

  // Compute screen coordinates of each object, if it's even visible.

  ClearB((pbyte)rgod, sizeof(rgod));
  for (j = 0; j < arMax; j++) {
    k = (!j ? arDes : (k == 1 ? arIC : (k == 2 ? arAsc : arMC)));
    if (k != arMC && (fSky || ignorez[k]))
      continue;
    for (i = 0; i <= is.nObj; i++) if (FProper(i)) {
      x1 = planet1[i]; y1 = planet2[i];
      if (k == arIC) {
        x1 = Mod(x1 + rDegHalf);
        neg(y1);
      } else if (k == arAsc) {
        x1 = Mod(x1 - rDegQuad);
        y1 = 0.0;
      } else if (k == arDes) {
        x1 = Mod(x1 + rDegQuad);
        y1 = 0.0;
      }
      l = j*objMax + i;
      rgod[l].f = !FMapCalc(x1, y1, &rgod[l].x, &rgod[l].y, fGlobe, fSky,
        rT, nScl, &cr, deg);
      rgod[l].obj = i;
      rgod[l].kv = k <= arMC ? ~0 : gi.kiLite;
    }
  }

  // Draw grid of triangles or squares over the planet.

  if (gs.nTriangles > 0) {
    DrawColor(kCyanB);
    y1 = RAsinD(1.0/(rPhi*2.0-1.0));    // Icosahedron constant.
    i = gs.nTriangles - 1;
    for (x1 = 0.0; x1 < rDegMax; x1 += 72.0) {
      DrawMapTriangle(x1,       y1, x1+72.0,   y1, x1+36.0,  90.0,
        fGlobe, nScl, &cr, deg, i, 5);
      DrawMapTriangle(x1,       y1, x1+72.0,   y1, x1+36.0, -y1,
        fGlobe, nScl, &cr, deg, i, 4);
      DrawMapTriangle(x1+36.0, -y1, x1+108.0, -y1, x1+72,   -90.0,
        fGlobe, nScl, &cr, deg, i, 5);
      DrawMapTriangle(x1+36.0, -y1, x1+108.0, -y1, x1+72,    y1,
        fGlobe, nScl, &cr, deg, i, 4);
    }
  } else if (gs.nTriangles < 0) {
    DrawColor(kRedB);
    y1 = RAsinD(1.0/rSqr3);             // Cube constant.
    i = -gs.nTriangles - 1;
    for (x1 = 0.0; x1 < rDegMax; x1 += rDegQuad)
      DrawMapSquare(x1, y1, x1, -y1, x1+90.0, -y1, x1+90.0, y1,
        fGlobe, nScl, &cr, deg, i, 11);
    for (x1 = -y1; x1 <= y1; x1 += y1*2.0)
      DrawMapSquare(0.0, x1, 90.0, x1, 180.0, x1, 270.0, x1,
        fGlobe, nScl, &cr, deg, i, 0);
  }

#ifdef ATLAS
  // Draw locations of cities from atlas.

  if (gs.fLabelCity && !fSky && FEnsureAtlas()) {
    if (!gs.fLabelAsp)
      DrawColor(kOrangeB);
    KiCity(-1);
    for (i = 0; i < is.cae; i++) {
      x1 = nDegHalf - is.rgae[i].lon + (!fGlobe ? deg : 0.0);
      y1 = rDegQuad - is.rgae[i].lat;
      if (x1 < 0.0)
        x1 += rDegMax;
      else if (x1 >= rDegMax)
        x1 -= rDegMax;
      if (!FMapCalc(x1, y1, &j, &k, fGlobe, -1, 0.0, nScl, &cr, deg)) {
        if (gs.fLabelAsp)
          DrawColor(KiCity(i));
        DrawPoint(j, k);
      }
    }
  }
#endif

  // Draw ecliptic equator and zodiac sign wedges.

  if (gs.fHouseExtra) {
    if (!gs.fColorSign)
      DrawColor(kDkGreenB);
    for (l = -2; l < cSign; l++) {
      if (gs.fColorSign && l >= 0)
        DrawColor(kSignB(l+1));
      for (i = -90; i <= 90; i++) {
        if (gs.fColorSign && l < 0 && i % 30 == 0)
          DrawColor(kSignB((i+90)/30 + (l < -1)*6 + 1));
        if (l >= 0) {
          // Coordinates for zodiac sign wedge longitude lines.
          j = l*30; k = i;
        } else {
          // Coordinates for ecliptic equator latitude line.
          j = i+90 + (l < -1)*180; k = 0;
        }
        x1 = Tropical((real)j);
        y1 = (real)k;
        EclToEqu(&x1, &y1);
        if (!FMapCalc(x1, y1, &j, &k, fGlobe, fSky, rT, nScl, &cr, deg))
          DrawPoint(j, k);
      }
    }
  }

  // Draw Earth's equator.

  if (gs.fEquator) {
    DrawColor(kPurpleB);
    for (i = 0; i < nDegMax; i++) {
      x1 = (real)i; y1 = 90.0;
      if (!FMapCalc(x1, y1, &j, &k, fGlobe, -1, 0.0, nScl, &cr, deg))
        DrawPoint(j, k);
    }
  }

  // Draw chart latitude.

  if (us.fLatitudeCross && !fSky) {
    DrawColor(kMagentaB);
    for (i = 0; i < nDegMax; i++) {
      x1 = (real)i; y1 = 90.0 - Lat;
      if (!FMapCalc(x1, y1, &j, &k, fGlobe, -1, 0.0, nScl, &cr, deg))
        DrawPoint(j, k);
    }
  }

#ifdef SWISS
  // Draw extra stars.

  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      x1 = es.lon; y1 = es.lat;
      x1 = Tropical(x1);
      EclToEqu(&x1, &y1);
      if (!FMapCalc(x1, y1, &j, &k, fGlobe, fSky, rT, nScl, &cr, deg))
        DrawStar(j, k, &es);
    }
    DrawColor(gi.kiLite);
    EnumStarsLines(fTrue, NULL, NULL);
    while (EnumStarsLines(fFalse, &pes1, &pes2)) {
      x1 = pes1->lon; y1 = pes1->lat;
      x1 = Tropical(x1);
      EclToEqu(&x1, &y1);
      if (!FMapCalc(x1, y1, &xp, &yp, fGlobe, fSky, rT, nScl, &cr, deg)) {
        x1 = pes2->lon; y1 = pes2->lat;
        x1 = Tropical(x1);
        EclToEqu(&x1, &y1);
        if (!FMapCalc(x1, y1, &xp2, &yp2, fGlobe, fSky, rT, nScl, &cr, deg))
          DrawLine(xp, yp, xp2, yp2);
      }
    }
  }

  // Draw extra asteroids.

  if (gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fTrue);
    while (SwissComputeAsteroid(is.T, &es, fTrue)) {
      x1 = es.lon; y1 = es.lat;
      x1 = Tropical(x1);
      EclToEqu(&x1, &y1);
      if (!FMapCalc(x1, y1, &j, &k, fGlobe, fSky, rT, nScl, &cr, deg))
        DrawStar(j, k, &es);
    }
  }
#endif

  // Draw MC, IC, Asc, and Des lines for each object, as great circles around
  // the globe. The result is a (3D for globes) astro-graph chart.

  if (fSky)
    goto LDone;
  for (i = is.nObj; i >= 0; i--) if (FProper(i)) {
    for (k = 0; k < 4; k++) {
      if (ignorez[!k ? arDes : (k == 1 ? arIC : (k == 2 ? arAsc : arMC))])
        continue;
      DrawColor(kObjB[!k ? oDes : (k == 1 ? oNad : (k == 2 ? oAsc : oMC))]);
      xold = nNegative;
      for (j = 0; j <= nDegHalf; j++) {
        if (FOdd(k)) {
          x1 = planet1[i] + (k == 3 ? 0.0 : rDegHalf);
          y1 = (real)(j - 90);
        } else {
          l = j + (k == 0)*nDegHalf + 90;
          if (l >= nDegMax)
            l -= nDegMax;
          x1 = (real)l; y1 = 0.0;
          CoorXform(&x1, &y1, rDegQuad - planet2[i]);
          x1 += planet1[i] + rDegQuad;
        }
        if (!FMapCalc(x1, y1, &x, &y, fGlobe, fSky, rT, nScl, &cr, deg)) {
          if (xold > nNegative) {
            if (gi.nMode != gWorldMap || gs.fMollewide) {
              if (NAbs(xold-x) < (rx >> 2))
                DrawLine(xold, yold, x, y);
            } else
              DrawWrap(xold, Min(yold, gs.yWin-1), x, y, 0, gs.xWin-1);
          }
          xold = x; yold = y;
        } else
          xold = nNegative;
      }
    }
    // Draw astro-graph lines for minor house cusps, if unrestricted.
    for (k = 1; k <= cSign; k++) {
      if (FIgnore(cuspLo-1 + k) ||
        (k == sAri && chouse[k] == is.Asc) ||
        (k == sCap && chouse[k] == is.MC)  ||
        (k == sLib && chouse[k] == Mod(is.Asc + rDegHalf)) ||
        (k == sCan && chouse[k] == Mod(is.MC  + rDegHalf)))
        continue;
      DrawColor(kObjB[cuspLo-1 + k]);
      xold = nNegative;
      for (j = 0; j <= nDegHalf; j++) {
        x1 = 0.0; y1 = (real)j;
        CoorXform(&x1, &y1, rDegQuad - chouse3[k]);
        x1 = Mod(x1 + rDegQuad);
        CoorXform(&x1, &y1, rDegQuad + planet2[i]);
        x1 = Mod(x1 - rDegQuad + planet1[i]);
        if (!FMapCalc(x1, y1, &x, &y, fGlobe, fSky, rT, nScl, &cr, deg)) {
          if (xold > nNegative) {
            if (gi.nMode != gWorldMap || gs.fMollewide) {
              if (NAbs(xold-x) < (rx >> 2))
                DrawLine(xold, yold, x, y);
            } else
              DrawWrap(xold, Min(yold, gs.yWin-1), x, y, 0, gs.xWin-1);
          }
          if (j == 90) {
            // Draw glyph to label minor house cusp line.
            DrawSpot(x, y);
            DrawObject(cuspLo-1 + k, x, y + 7*gi.nScale);
          }
          xold = x; yold = y;
        } else
          xold = nNegative;
      }
    }
  }

  // Plot chart location.

  if (us.fLatitudeCross &&
    !FMapCalc(Mod(rDegHalf - Lon + (!fGlobe ? deg : 0.0)), rDegQuad - Lat,
    &j, &k, fGlobe, -1, 0.0, nScl, &cr, deg)) {
    DrawColor(kMagentaB);
    DrawSpot(j, k);
  }

LDone:
  // Draw planet glyphs, and points for actual zenith locations.
  DrawObjects(rgod, objMax * arMax, 0);
}


#ifdef SWISS
// Enumerate the list of extra stars to draw lines between. Each call, return
// the next pair of previously computed extra stars.

flag EnumStarsLines(flag fInit, ES **ppes1, ES **ppes2)
{
  static char *pchCur = NULL;
  static int iBase, iMax, iChain;
  int i1, i2;

  // Check for initialization and empty list cases.
  if (fInit) {
    pchCur = gs.szStarsLnk;
    iBase = iMax = 0;
    iChain = -1;
    return fTrue;
  }
  if (!FSzSet(pchCur))
    return fFalse;

  // Parse first star index.
  if (iChain >= 0)
    i1 = iChain;
  else {
    i1 = iBase + atoi(pchCur);
    iMax = Max(iMax, i1);
    while (FNumCh(*pchCur))
      pchCur++;
    while (*pchCur && !FNumCh(*pchCur))
      pchCur++;
  }

  // Parse second star index.
  i2 = iBase + atoi(pchCur);
  iMax = Max(iMax, i2);
  while (FNumCh(*pchCur))
    pchCur++;
  iChain = -1;
  if (*pchCur == chSep2)
    iBase = iMax + 1;
  else if (*pchCur == '_')
    iChain = i2;
  while (*pchCur && !FNumCh(*pchCur))
    pchCur++;

  // If both star indexes are valid, then return those stars.
  if (gi.rges == NULL ||
    !FBetween(i1, 0, gi.cStarsLin-1) || !FBetween(i2, 0, gi.cStarsLin-1))
    return fFalse;
  *ppes1 = &gi.rges[i1]; *ppes2 = &gi.rges[i2];
  return fTrue;
}
#endif


/*
******************************************************************************
** Driver Routines.
******************************************************************************
*/

// Create a chart in the window based on the current graphics chart mode.
// This is the main dispatch routine for all of the program's graphics.

void DrawChartX()
{
  char sz[cchSzMax];
  int i;
  flag fAltWire = fFalse, fSky, fSav;

  gi.nScale = gs.nScale/100;

  if (gs.ft == ftBmp || gs.ft == ftWmf || gs.ft == ftWire)
    PrintProgress("Creating graphics chart in memory.");
  DrawClearScreen();
  FBmpDrawBack(NULL);
  fAltWire = gs.ft == ftWire &&
    (gi.nMode == gOrbit || gi.nMode == gSphere || gi.nMode == gGlobe);
#ifdef CONSTEL
  fSky = gs.fConstel;
#else
  fSky = fFalse;
#endif

  switch (gi.nMode) {
  case gWheel:
  case gHouse:
    if (us.nRel > rcDual)
      XChartWheel();
    else if (FBetween(us.nRel, rcHexaWheel, rcTriWheel))
      XChartWheelMulti();
    else
      XChartWheelRelation();
    break;
  case gGrid:
    if (us.nRel > rcDual)
      XChartGrid();
    else
      XChartGridRelation();
    break;
  case gHorizon:
    if (us.fPrimeVert)
      XChartHorizonSky();
    else
      XChartHorizon();
    break;
  case gOrbit:
#ifdef WIRE
    if (gs.ft == ftWire)
      WireChartOrbit();
    else
#endif
      XChartOrbit();
    break;
  case gSector:
    XChartSector();
    break;
  case gDisposit:
    XChartDispositor();
    break;
  case gEsoteric:
    XChartEsoteric();
    break;
  case gAstroGraph:
    DrawMap(fFalse, fFalse, gs.rRot);    // First draw map of world.
    XChartAstroGraph();                  // Then draw astro-graph lines on it.
    break;
  case gCalendar:
    XChartCalendar();
    break;
  case gEphemeris:
    XChartEphemeris();
    break;
  case gRising:
    XChartRising();
    break;
  case gLocal:
    XChartLocal();
    break;
#ifdef WIN
  case gTraTraTim:
  case gTraTraInf:
#endif
  case gTraTraGra:
    XChartTransit(fFalse, is.fProgress);
    break;
#ifdef WIN
  case gTraNatTim:
  case gTraNatInf:
#endif
  case gTraNatGra:
    XChartTransit(fTrue, is.fProgress);
    break;
  case gSphere:
#ifdef WIRE
    if (gs.ft == ftWire)
      WireChartSphere();
    else
#endif
      XChartSphere();
    break;
  case gWorldMap:
    // First draw map of world, then maybe Ley lines.
    DrawMap(fSky, fFalse, gs.rRot);
    if (!fSky && !gs.fMollewide && gs.fAlt && gs.nTriangles != 0)
      DrawLeyLines(gs.rRot);
    break;
  case gGlobe:
#ifdef WIRE
    if (gs.ft == ftWire) {
      WireDrawGlobe(fSky, gs.rRot);
      break;
    }
#endif
    // Fall through
  case gPolar:
    DrawMap(fSky, fTrue, gs.rRot);
    break;
  case gTelescope:
    XChartTelescope();
    break;
#ifdef BIORHYTHM
  case gBiorhythm:
    XChartBiorhythm();
    break;
#endif
  }

  // Print text showing chart information at bottom of window.

  if (fAltWire)
    return;
  DrawColor(gi.kiLite);
  if (fDrawText) {
    if (FNoTimeOrSpace(ciMain))
      sprintf(sz, "(No time or space)");
    else if (us.nRel == rcComposite)
      sprintf(sz, "(Composite)");
    else {
      fSav = us.fAnsiChar;
      us.fAnsiChar = (gs.nFont == 0 || (gs.ft != ftPS && gs.ft != ftWmf)) << 1;
      i = DayOfWeek(Mon, Day, Yea);
      sprintf(sz, "%s%s%.3s %s %s (%cT Zone %s) %s%s%s",
        FSzSet(ciCore.nam) ? ciCore.nam : "", FSzSet(ciCore.nam) ? ", " : "",
        szDay[i], SzDate(Mon, Day, Yea, 2), SzTim(Tim), ChDst(Dst),
        SzZone(Zon), FSzSet(ciCore.loc) ? ciCore.loc : "",
        FSzSet(ciCore.loc) ? " " : "", SzLocation(Lon, Lat));
      us.fAnsiChar = fSav;
    }
    DrawSz(sz, gs.xWin/2, gs.yWin - gi.nScaleT - gi.nScaleTextT2,
      dtBottom | dtErase | dtScale2);
  }

  // Draw a border around the chart if the mode is set and appropriate.

  if (fDrawBorder)
    DrawEdgeAll();
}
#endif // GRAPH

/* xcharts0.cpp */
