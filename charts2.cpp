/*
** Astrolog (Version 7.30) File: charts2.cpp
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
** Dual Chart Display Routines.
******************************************************************************
*/

// Print a listing of planets for two (or more) charts, as specified by the
// -v -r0 switch combination, along with the maximum delta between planets.

void ChartListingRelation(void)
{
  char sz[cchSzDef], szFormat[cchSzDef];
  int cChart, i, j, k, l, n;
  real r, rT;

  cChart = 2 - (FBetween(us.nRel, rcHexaWheel, rcTriWheel) ? us.nRel+1 : 0);

  // Print header rows.
  AnsiColor(kWhiteA);
  PrintTab(' ', 5);
  n = us.fSeconds ? 23 : 16;
  sprintf(szFormat, " %%-%d.%ds", n, n);
  for (i = 1; i <= cChart; i++) {
    AnsiColor(kMainA[FOdd(i) ? 1 : 3]);
    sprintf(sz, szFormat, rgpci[i]->nam); PrintSz(sz);
  }
  PrintSz("\n      ");
  for (i = 1; i <= cChart; i++) {
    AnsiColor(kMainA[FOdd(i) ? 1 : 3]);
    if (!FNoTimeOrSpace(*rgpci[i])) {
      sprintf(sz, "%s %s", SzDate(rgpci[i]->mon, rgpci[i]->day,
        rgpci[i]->yea, us.fSeconds-1), SzTim(rgpci[i]->tim));
      PrintSz(sz);
      PrintTab(' ', (us.fSeconds ? 24 : 17) - CchSz(sz));
    } else
      PrintSz(us.fSeconds ? "(No time or space)      " : "(No time/space)  ");
  }
  AnsiColor(kDkGrayA);
  PrintSz("\nBody");
  for (i = 1; i <= cChart; i++) {
    AnsiColor(kMainA[FOdd(i) ? 1 : 3]);
    PrintSz("  Location");
    PrintTab(' ', us.fSeconds ? 5 : 1);
    if (us.fSeconds)
      PrintSz(!us.fEquator2 ? " Latitude" : " Declin. ");
    else
      PrintSz(!us.fEquator2 ? "Latit." : "Decl. ");
  }
  AnsiColor(kMainA[FOdd(i) ? 3 : 1]);
  PrintTab(' ', 3 + us.fSeconds);
  PrintSz("Delta\n");

  // Print object positions.
  for (l = 0; l <= is.nObj; l++) {
    i = rgobjList[l];
    if (FIgnore(i))
      continue;
    AnsiColor(kObjA[i]);
    sprintf(sz, "%-4.4s:", szObjDisp[i]); PrintSz(sz);
    for (j = 1; j <= cChart; j++) {
      PrintCh(' ');
      PrintZodiac(rgpcp[j]->obj[i]);
      sprintf(sz, "%c ", rgpcp[j]->dir[i] >= 0.0 ? ' ' : chRet); PrintSz(sz);
      PrintAltitude(rgpcp[j]->alt[i]);
    }

    // Compute maximum offset between any two instances of this planet.
    r = 0.0;
    for (j = 1; j <= cChart; j++)
      for (k = 1; k < j; k++) {
        if (!us.fAspect3D)
          rT = MinDistance(rgpcp[j]->obj[i], rgpcp[k]->obj[i]);
        else
          rT = SphDistance(rgpcp[j]->obj[i], rgpcp[j]->alt[i],
            rgpcp[k]->obj[i], rgpcp[k]->alt[i]);
        r = Max(r, rT);
      }
    AnsiColor(kDkGrayA);
    PrintCh(' ');
    PrintSz(SzDegree(r));
    PrintL();
  }
}


// Print out an aspect (or midpoint if -gm switch in effect) grid of a
// relationship chart. This is similar to the ChartGrid() routine, however
// here both axes are labeled with the planets for the two charts in question,
// instead of just a diagonal down the center for only one chart.

void ChartGridRelation(void)
{
  char sz[cchSzDef];
  int i0, j0, i, j, k, temp;

#ifdef INTERPRET
  if (us.fInterpret && !us.fGridMidpoint) {
    InterpretGridRelation();
    return;
  }
#endif
  PrintSz(" 2>");
  for (i0 = 0; i0 <= is.nObj; i0++) {
    i = rgobjList[i0];
    if (ignore[i])
      continue;
    PrintCh(chV);
    AnsiColor(kObjA[i]);
    sprintf(sz, "%.3s", szObjDisp[i]); PrintSz(sz);
    AnsiColor(kDefault);
  }
  PrintSz("\n1  ");
  for (i0 = 0; i0 <= is.nObj; i0++) {
    i = rgobjList[i0];
    if (ignore[i])
      continue;
    PrintCh(chV);
    AnsiColor(kSignA(SFromZ(cp2.obj[i])));
    sprintf(sz, "%2d%c", (int)cp2.obj[i] % 30, chDeg0); PrintSz(sz);
    AnsiColor(kDefault);
  }
  PrintSz("\nV  ");
  for (i0 = 0; i0 <= is.nObj; i0++) {
    i = rgobjList[i0];
    if (ignore[i])
      continue;
    PrintCh(chV);
    temp = SFromZ(cp2.obj[i]);
    AnsiColor(kSignA(temp));
    sprintf(sz, "%.3s", szSignName[temp]); PrintSz(sz);
    AnsiColor(kDefault);
  }
  if (us.fSeconds) {
    PrintSz("\n   ");
    for (i0 = 0; i0 <= is.nObj; i0++) {
      i = rgobjList[i0];
      if (ignore[i])
        continue;
      PrintCh(chV);
      temp = SFromZ(cp2.obj[i]);
      AnsiColor(kSignA(temp));
      sprintf(sz, "%02d'", (int)(cp2.obj[i]*60.0) % 60); PrintSz(sz);
      AnsiColor(kDefault);
    }
  }
  PrintL();
  for (j0 = 0; j0 <= is.nObj; j0++) {
    j = rgobjList[j0];
    if (ignore[j])
      continue;
    for (k = 1; k <= 4 + us.fSeconds; k++) {
      if (k < 2)
        PrintTab(chH, 3);
      else if (k == 2) {
        AnsiColor(kObjA[j]);
        sprintf(sz, "%.3s", szObjDisp[j]); PrintSz(sz);
      } else {
        temp = SFromZ(cp1.obj[j]);
        AnsiColor(kSignA(temp));
        if (k == 3)
          sprintf(sz, "%2d%c", (int)cp1.obj[j] - (temp-1)*30, chDeg0);
        else if (k == 4)
          sprintf(sz, "%.3s", szSignName[temp]);
        else
          sprintf(sz, "%02d'", (int)(cp1.obj[j]*60.0) % 60);
        PrintSz(sz);
      }
      if (k > 1)
        AnsiColor(kDefault);
      for (i0 = 0; i0 <= is.nObj; i0++) {
        i = rgobjList[i0];
        if (ignore[i])
          continue;
        PrintCh((char)(k < 2 ? chC : chV));
        temp = grid->n[i][j];
        if (k > 1) {
          if (i == j)
            AnsiColor(kReverse);
          AnsiColor(us.fGridMidpoint ? kSignA(temp) : kAspA[temp]);
        }
        if (k < 2)
          PrintTab(chH, 3);
        else if (k == 2) {
          if (us.fGridMidpoint)
            sprintf(sz, "%.3s", szSignName[temp]);
          else
            sprintf(sz, "%s", temp ? SzAspectAbbrev(temp) : "   ");
          PrintSz(sz);
        } else if (k == 3) {
          if (us.fGridMidpoint) {
            sprintf(sz, "%2d%c", grid->v[i][j]/3600, chDeg0); PrintSz(sz);
          } else
            if (grid->n[i][j]) {
              if (NAbs(grid->v[i][j]) < 6000*60)
                sprintf(sz, "%c%2d", us.fAppSep ?
                  (grid->v[i][j] < 0 ? 'a' : 's') :
                  (grid->v[i][j] < 0 ? '-' : '+'), NAbs(grid->v[i][j])/3600);
              else
                sprintf(sz, "%3d", NAbs(temp)/3600);
              PrintSz(sz);
            } else
              PrintSz("   ");
        } else {
          if (grid->n[i][j]) {
            if (k == 4)
              sprintf(sz, "%02d'", NAbs(grid->v[i][j])/60%60);
            else
              sprintf(sz, "%02d\"", NAbs(grid->v[i][j])%60);
            PrintSz(sz);
          } else
            PrintSz("   ");
        }
        AnsiColor(kDefault);
      }
      PrintL();
    }
  }
}


// Display all aspects between objects in the relationship comparison chart,
// one per line, in sorted order based on the total "power" of the aspects,
// as specified with the -r0 -a switch combination.

void ChartAspectRelation(void)
{
  int ca[cAspect + 1], co[objMax];
  char sz[cchSzDef];
  int vcut = nLarge, icut, jcut, vhi, ihi, jhi, ahi, phi, v, i0, j0, i, j, k,
    p, count = 0;
  real ip, jp, rPowSum = 0.0;

  ClearB((pbyte)ca, sizeof(ca));
  ClearB((pbyte)co, sizeof(co));
  loop {
    vhi = -nLarge;

    // Search for the next most powerful aspect in the aspect grid.

    for (i0 = 0; i0 <= is.nObj; i0++) {
      i = rgobjList[i0];
      if (FIgnore(i))
        continue;
      for (j0 = 0; j0 <= is.nObj; j0++) {
        j = rgobjList[j0];
        if (FIgnore(j))
          continue;
        if (k = grid->n[i][j]) {
          ip = RObjInf(i);
          jp = RObjInf(j);
          p = (int)(rAspInf[k]*(ip+jp)/2.0*
            (1.0-RAbs((real)(grid->v[i][j]))/3600.0/GetOrb(i, j, k))*1000.0);
#ifdef EXPRESS
          // Adjust power with AstroExpression if one set.
          if (FSzSet(us.szExpAspList)) {
            ExpSetN(iLetterW, i);
            ExpSetN(iLetterX, k);
            ExpSetN(iLetterY, j);
            ExpSetN(iLetterZ, p);
            ParseExpression(us.szExpAspList);
            p = NExpGet(iLetterZ);
          }
#endif
          switch (us.nAspectSort) {
          default:  v = p;                           break;
          case aso: v = -NAbs(grid->v[i][j]);        break;
          case asn: v = -grid->v[i][j];              break;
          case asO: v = -(j0*cObj + i0);             break;
          case asP: v = -(i0*cObj + j0);             break;
          case asA: v = -(k*cObj*cObj + j*cObj + i); break;
          case asC: v = -(int)(cp1.obj[j]*3600.0);   break;
          case asD: v = -(int)(cp2.obj[i]*3600.0);   break;
          case asM: v = -(int)(Midpoint(cp1.obj[j], cp2.obj[i])*3600.0); break;
          }
          if ((v < vcut || (v == vcut && (i > icut ||
            (i == icut && j > jcut)))) && v > vhi) {
            vhi = v; ihi = i; jhi = j; ahi = k; phi = p;
          }
        }
      }
    }
    if (vhi <= -nLarge)    // Exit when no less powerful aspect found.
      break;
    vcut = vhi; icut = ihi; jcut = jhi;
    count++;                              // Display the current aspect.
    rPowSum += (real)phi/1000.0;
    ca[ahi]++;
    co[jhi]++; co[ihi]++;
#ifdef INTERPRET
    if (us.fInterpret) {                  // Interpret it if -I in effect.
      InterpretAspectRelation(jhi, ihi);
      AnsiColor(kDefault);
      continue;
    }
#endif
    sprintf(sz, "%3d: ", count); PrintSz(sz);
    PrintAspect(jhi, cp1.obj[jhi], (int)RSgn(cp1.dir[jhi]), ahi,
      ihi, cp2.obj[ihi], (int)RSgn(cp2.dir[ihi]), 'A');
    k = grid->v[ihi][jhi];
    AnsiColor(k < 0 ? kWhiteA : kLtGrayA);
    sprintf(sz, "- orb: %c%d%c%02d'",
      us.fAppSep ? (k < 0 ? 'a' : 's') : (k < 0 ? '-' : '+'),
      NAbs(k)/3600, chDeg1, NAbs(k)%3600/60); PrintSz(sz);
    if (is.fSeconds) {
      sprintf(sz, "%02d\"", NAbs(k)%60); PrintSz(sz);
    }
    AnsiColor(kDkGreenA);
    sprintf(sz, " - power: %5.2f\n", (real)phi/1000.0); PrintSz(sz);
    AnsiColor(kDefault);
  }

#ifdef EXPRESS
  // Send summary to AstroExpression if one set.
  if (!us.fExpOff && FSzSet(us.szExpAspSumm)) {
    ExpSetN(iLetterY, count);
    ExpSetR(iLetterZ, rPowSum);
    ParseExpression(us.szExpAspSumm);
  }
#endif
  PrintAspectSummary(ca, co, count, rPowSum);
}


// Display locations of all midpoints between objects in the relationship
// comparison chart, one per line, in sorted zodiac order from zero Aries
// onward, as specified with the -r0 -m switch combination.

void ChartMidpointRelation(void)
{
  int cs[cSign + 1];
  char sz[cchSzDef];
  int mcut = -1, icut, jcut, mlo, ilo, jlo, m, i, j, count = 0;
  long lSpanSum = 0;
  real mid, dist, midalt;

  ClearB((pbyte)cs, sizeof(cs));
  loop {
    mlo = 360*60*60;

    // Search for the next closest midpoint farther down in the zodiac.

    for (i = 0; i <= is.nObj; i++) if (!FIgnore(i))
      for (j = 0; j <= is.nObj; j++) if (!FIgnore(j)) {
        m = (grid->n[j][i]-1)*(30*60*60) + grid->v[j][i];
        if ((m > mcut || (m == mcut && (i > icut ||
          (i == icut && j > jcut)))) && m < mlo) {
          ilo = i; jlo = j; mlo = m;
        }
      }
    if (mlo >= 360*60*60)  // Exit when no midpoint farther in zodiac found.
      break;
    mcut = mlo; icut = ilo; jcut = jlo;
    if (us.objRequire >= 0 && ilo != us.objRequire && jlo != us.objRequire)
      continue;
    if (!us.fHouse3D) {
      mid = Midpoint(cp1.obj[ilo], cp2.obj[jlo]);
      midalt = (cp1.alt[ilo] + cp2.alt[jlo]) / 2.0;
      dist = MinDistance(cp1.obj[ilo], cp2.obj[jlo]);
    } else {
      SphRatio(cp1.obj[ilo], cp1.alt[ilo], cp2.obj[jlo], cp2.alt[jlo], 0.5,
        &mid, &midalt);
      dist = SphDistance(cp1.obj[ilo], cp1.alt[ilo],
        cp2.obj[jlo], cp2.alt[jlo]);
    }
#ifdef EXPRESS
    // Skip current midpoint if AstroExpression says to do so.
    if (!us.fExpOff && FSzSet(us.szExpMid)) {
      ExpSetN(iLetterW, ilo);
      ExpSetN(iLetterX, jlo);
      ExpSetR(iLetterY, mid);
      ExpSetR(iLetterZ, dist);
      if (!NParseExpression(us.szExpMid))
        continue;
    }
#endif
    count++;                               // Display the current midpoint.
    cs[mlo/(30*60*60)+1]++;
    m = (int)(dist*3600.0);
    lSpanSum += m;
#ifdef INTERPRET
    if (us.fInterpret) {                   // Interpret it if -I in effect.
      InterpretMidpointRelation(ilo, jlo);
      AnsiColor(kDefault);
      continue;
    }
#endif
    sprintf(sz, "%4d: ", count); PrintSz(sz);
    PrintZodiac(mid);
    PrintCh(' ');
    if (us.fParallel) {
      AnsiColor(kDefault);
      PrintAltitude(midalt);
      PrintCh(' ');
    }
    PrintAspect(ilo, cp1.obj[ilo], (int)RSgn(cp1.dir[ilo]), 0,
      jlo, cp2.obj[jlo], (int)RSgn(cp2.dir[jlo]), 'M');
    AnsiColor(kDefault);
    PrintSz("- ");
    PrintSz(SzDegree(dist));
    if (us.fParallel && !us.fHouse3D) {
      PrintCh(' ');
      PrintSz(SzDegree(RAbs(cp1.alt[ilo] - cp2.alt[jlo])));
    }
    PrintSz(" degree span.\n");
  }

  PrintMidpointSummary(cs, count, lSpanSum);
}


// Calculate any of the various kinds of relationship charts. This involves
// computing and storing the planet and house positions for the "core" and
// "second" charts, and then combining them in the main single chart in the
// proper manner, e.g. for synastry, composite, time space midpoint charts.

void CastRelation(void)
{
  byte ignoreT[objMax];
  int i, j;
  real ratio, t1, t2, t, rSav;
  flag fSav;

  // Cast the first chart.

  ciCore = ciMain;
  if (us.nRel == rcProgress) {
    fSav = us.fProgress;
    us.fProgress = fFalse;
  }
#ifdef WIN
  else if (us.nRel == rcMidpoint)
    ciCore = ciMain = ciSave;
#endif
  FProcessCommandLine(szWheel[1]);
  if (FNoTimeOrSpace(ciCore))
    cp0 = cp1;
  t1 = CastChart(1);
  cp1 = cp0;
  rSav = is.MC;

  // Cast the second chart.

  ciCore = ciTwin;
  if (us.nRel == rcTransit) {
    for (i = 0; i <= is.nObj; i++) {
      ignoreT[i] = ignore[i];
      ignore[i] = ignore[i] && ignore2[i];
    }
  } else if (us.nRel == rcProgress) {
    us.fProgress = fTrue;
    is.JDp = MdytszToJulian(MM, DD, YY, TT, SS, ZZ);
    ciCore = ciMain;
  }
  FProcessCommandLine(szWheel[2]);
  if (FNoTimeOrSpace(ciCore))
    cp0 = cp2;
  t2 = CastChart(2);
  if (us.nRel == rcTransit) {
    for (i = 0; i <= is.nObj; i++)
      ignore[i] = ignoreT[i];
  } else if (us.nRel == rcProgress)
    us.fProgress = fSav;
  cp2 = cp0;

  // Cast the third through sixth charts.

  if (FBetween(us.nRel, rcHexaWheel, rcTriWheel)) {
    j = 2-(us.nRel+1);
    for (i = 3; i <= j; i++) {
      ciCore = *rgpci[i];
      FProcessCommandLine(szWheel[i]);
      if (FNoTimeOrSpace(ciCore))
        cp0 = *rgpcp[i];
      CastChart(i);
      *rgpcp[i] = cp0;
    }
  }
  ciCore = ciMain;
  FProcessCommandLine(szWheel[0]);
  is.MC = rSav;

  // Now combine the two charts based on what relation we are doing.
  // For the standard -r synastry chart, use the house cusps of chart1
  // and the planet positions of chart2.

  ratio = (real)us.nRatio1 / ((real)(us.nRatio1 + us.nRatio2));
  if (us.nRel <= rcSynastry) {
    for (i = 1; i <= cSign; i++)
      chouse[i] = cp1.cusp[i];

  // For the -rc composite chart, take the midpoints of the planets/houses.

  } else if (us.nRel == rcComposite) {
    j = Max(is.nObj, cuspHi);
    for (i = 0; i <= j; i++) {
      planet[i] = Ratio(cp1.obj[i], cp2.obj[i], ratio);
      if (RAbs(cp2.obj[i] - cp1.obj[i]) > rDegHalf)
        planet[i] = Mod(planet[i] + rDegMax*ratio);
      planetalt[i] = Ratio(cp1.alt[i], cp2.alt[i], ratio);
      ret[i] = Ratio(cp1.dir[i], cp2.dir[i], ratio);
    }
    for (i = 1; i <= cSign; i++) {
      chouse[i] = Ratio(cp1.cusp[i], cp2.cusp[i], ratio);
      if (RAbs(cp2.cusp[i] - cp1.cusp[i]) > rDegHalf)
        chouse[i] = Mod(chouse[i] + rDegMax*ratio);
    }

    // Make sure don't have any 180 degree errors in house cusp complement
    // pairs, which may happen if the cusps are far apart.

    j = us.fPolarAsc ? sAri : sCap;
    for (i = 1; i <= cSign; i++)
      if (MinDistance(chouse[j], Mod(chouse[i]-ZFromS(i+3))) > rDegQuad)
        chouse[i] = Mod(chouse[i]+rDegHalf);
    for (i = 1; i <= cSign; i++)
      if (RAbs(MinDistance(chouse[i], planet[oAsc - 1 + i])) > rDegQuad)
        planet[oAsc - 1 + i] = Mod(planet[oAsc - 1 + i]+rDegHalf);

  // For the -rm time space midpoint chart, calculate the midpoint time and
  // place between the two charts and then recast for the new chart info.

  } else if (us.nRel == rcMidpoint) {
    is.T = Ratio(t1, t2, ratio);
    t = (is.T*36525.0)+rRound; is.JD = RFloor(t)+2415020.0;
    TT = RFract(t)*24.0;
    ZZ = Ratio(Zon, ciTwin.zon, ratio);
    SS = Ratio(Dst, ciTwin.dst, ratio);
    TT -= ZZ - SS;
    if (TT < 0.0) {
      TT += 24.0; is.JD -= 1.0;
    }
    JulianToMdy(is.JD, &MM, &DD, &YY);
    if (!us.fHouse3D) {
      // Take midpoint of longitude and latitude separately.
      OO = Ratio(Lon, ciTwin.lon, ratio);
      if (RAbs(ciTwin.lon-Lon) > rDegHalf)
        OO = Mod(OO+rDegMax*ratio);
      AA = Ratio(Lat, ciTwin.lat, ratio);
    } else {
      // Take true midpoint along great circle between the two locations.
      SphRatio(Lon, Lat, ciTwin.lon, ciTwin.lat, ratio, &OO, &AA);
    }
    ciMain = ciCore;
    CastChart(0);
#ifndef WIN
    us.nRel = rcNone;  // Turn off so don't move to midpoint again.
#endif

  // There are a couple of non-astrological charts, which only require the
  // number of days that have passed between the two charts to be done.

  } else {
    is.JD = RAbs(t2-t1)*36525.0;
    cp0 = cp1;
  }

  ComputeInHouses();
}


/*
******************************************************************************
** Other Chart Display Routines.
******************************************************************************
*/

// Given two objects and an aspect between them, or an object and a sign that
// it's entering, print if this is a "major" event, such as a season change or
// major lunar phase. This is called from the ChartInDay() searching and
// influence routines. Do an interpretation if need be too.

void PrintInDayEvent(int source, int aspect, int dest, int nVoid)
{
  char sz[cchSzDef];
  int nEclipse;
  real rPct;
  flag fSwap;

  // If the Sun changes sign, then print out if this is a season change.
  if (aspect == aSig) {
    if (source == oSun) {
      AnsiColor(kWhiteA);
      if (dest == sAri || dest == sLib) {
        if ((dest == sAri) == (AA >= 0.0))
          PrintSz(" (Spring Equinox)");
        else
          PrintSz(" (Autumn Equinox)");
      } else if (dest == sCan || dest == sCap) {
        if ((dest == sCan) == (AA >= 0.0))
          PrintSz(" (Summer Solstice)");
        else
          PrintSz(" (Winter Solstice)");
      }
    }

  } else if (aspect > 0 && !us.fParallel) {
    fSwap = (dest == oSun);
    if (fSwap)
      SwapN(source, dest);

    // Print if the present aspect is a New, Full, or Half Moon.
    if (source == oSun && (dest == oMoo || FMoons(dest)) &&
      (us.fMoonMove || ObjOrbit(dest) == us.objCenter)) {
      if (aspect <= aSqu)
        AnsiColor(kWhiteA);
      if (aspect == aCon)
        PrintSz(" (New Moon)");
      else if (aspect == aOpp) {
        PrintSz(" (Full Moon)");
        // Full Moons may be a lunar eclipse.
        if (us.fEclipse) {
          nEclipse = NCheckEclipseLunar(us.objCenter, dest, &rPct);
          if (nEclipse > etNone) {
            AnsiColor(kWhiteA);
            sprintf(sz, " (%s Lunar Eclipse", szEclipse[nEclipse]);
            PrintSz(sz);
            if (us.fSeconds) {
              sprintf(sz, " %.0f%%", rPct); PrintSz(sz);
            }
            PrintSz(")");
          }
        }
      } else if (aspect == aSqu)
        PrintSz(" (Half Moon)");
    }

    // Conjunctions may be a solar eclipse or other occultation.
    if (us.fEclipse && aspect == aCon) {
      nEclipse = NCheckEclipse(source, dest, &rPct);
      if (nEclipse > etNone) {
        AnsiColor(kWhiteA);
        sprintf(sz, " (%s %s%s", szEclipse[nEclipse], source == oSun ?
          "Solar " : "", source == oSun && (dest == oMoo || FMoons(dest)) ?
          "Eclipse" : "Occultation"); PrintSz(sz);
        if (us.fSeconds) {
          sprintf(sz, " %.0f%%", rPct); PrintSz(sz);
        }
        PrintSz(")");
      }
    }
    if (fSwap)
      SwapN(source, dest);
  }

  // Print if the present aspect is the Moon going void of course.
  if (nVoid >= 0) {
    AnsiColor(kDefault);
    sprintf(sz, " (v/c %d:%02d", nVoid / 3600, nVoid / 60 % 60); PrintSz(sz);
    if (us.fSeconds) {
      sprintf(sz, ":%02d", nVoid % 60); PrintSz(sz);
    }
    PrintCh(')');
  }
  PrintL();

#ifdef INTERPRET
  if (us.fInterpret)
    InterpretInDay(source, aspect, dest);
#endif
  AnsiColor(kDefault);
}


// Given two objects and an aspect (or one object, and an event such as a sign
// or direction change) display the configuration in question. This is called
// by the many charts which list aspects among items, such as the -a aspect
// lists, -m midpoint lists, -d aspect in day search and -D influence charts,
// and -t transit search and -T influence charts.

void PrintAspect(int obj1, real pos1, int ret1, int asp,
  int obj2, real pos2, int ret2, char chart)
{
  char sz[cchSzDef];
  KI ki;
  flag fPar = (us.fParallel && asp >= aCon) || asp == aAlt, fSav = is.fSeconds;

  is.fSeconds = fFalse;
  AnsiColor(kObjA[obj1]);
  if (chart == 't' || chart == 'T')
    PrintSz("trans ");
  else if (chart == 'e' || chart == 'u' || chart == 'U')
    PrintSz("progr ");
  sprintf(sz, "%7.7s", szObjDisp[obj1]); PrintSz(sz);
  ki = !fPar ? kSignA(SFromZ(pos1)) : kDefault;
  AnsiColor(ki);
  sprintf(sz, " %c", ret1 > 0 ? '(' : (ret1 < 0 ? '[' : '<')); PrintSz(sz);
  if (!us.fSeconds) {
    if (!fPar)
      sprintf(sz, "%.3s", szSignName[SFromZ(pos1)]);
    else
      sprintf(sz, "%c%2d", pos1 < 0 ? '-' : '+', (int)RAbs(pos1));
    PrintSz(sz);
  } else {
    if (asp == aSig && ret1 > 0)
      pos1 += 29.999;
    else if (asp == aDeg)
      pos1 = (real)obj2 * (rDegMax / (real)(cSign * us.nSignDiv));
    if (!fPar)
      PrintZodiac(pos1);
    else
      PrintAltitude(pos1);
    AnsiColor(ki);
  }
  sprintf(sz, "%c", ret1 > 0 ? ')' : (ret1 < 0 ? ']' : '>')); PrintSz(sz);
  AnsiColor(asp > 0 ? kAspA[asp] : kWhiteA);
  PrintCh(' ');

  if (asp == aSig || asp == aHou)
    sprintf(sz, "-->");                        // Print a sign change.
  else if (asp == aDir)
    sprintf(sz, "S/%c", obj2 ? chRet : 'D');   // Print a direction change.
  else if (asp == aAlt)
    sprintf(sz, "LA%c", obj2 ? '+' : '-');     // Print a latitude extreme.
  else if (asp == aLen)
    sprintf(sz, "%s", obj2 ? "Apo" : "Per");   // Print a distance extreme.
  else if (asp == aDeg)
    sprintf(sz, "At:");                        // Print a degree change.
  else if (asp == 0)
    sprintf(sz, chart == 'm' ? "&" : "with");
  else
    sprintf(sz, "%s", SzAspectAbbrev(asp));    // Print an aspect.
  PrintSz(sz);
  if (asp != aDir && asp != aAlt)
    PrintCh(' ');

  if (chart == 'A')
    PrintSz("with ");
  if (asp == aSig) {
    AnsiColor(kSignA(obj2));
    sprintf(sz, "%s", szSignName[obj2]); PrintSz(sz);
  } else if (asp == aDeg) {
    is.fSeconds = fSav;
    PrintZodiac((real)obj2 * (rDegMax / (real)(cSign * us.nSignDiv)));
  } else if (asp == aHou) {
    AnsiColor(kSignA(obj2));
    if (chart == 't' || chart == 'u' || chart == 'T' || chart == 'U')
      PrintSz("natal ");
    sprintf(sz, "%d%s 3D House", obj2, szSuffix[obj2]); PrintSz(sz);
  } else if (asp >= 0) {
    ki = !fPar ? kSignA(SFromZ(pos2)) : kDefault;
    AnsiColor(ki);
    if (chart == 't' || chart == 'u' || chart == 'T' || chart == 'U')
      PrintSz("natal ");
    sprintf(sz, "%c", ret2 > 0 ? '(' : (ret2 < 0 ? '[' : '<')); PrintSz(sz);
    if (!us.fSeconds) {
      if (!fPar)
        sprintf(sz, "%.3s", szSignName[SFromZ(pos2)]);
      else
        sprintf(sz, "%c%2d", pos2 < 0 ? '-' : '+', (int)RAbs(pos2));
      PrintSz(sz);
    } else {
      if (!fPar)
        PrintZodiac(pos2);
      else
        PrintAltitude(pos2);
      AnsiColor(ki);
    }
    sprintf(sz, "%c ", ret2 > 0 ? ')' : (ret2 < 0 ? ']' : '>')); PrintSz(sz);
    AnsiColor(kObjA[obj2]);
    sprintf(sz, "%.10s", szObjDisp[obj2]); PrintSz(sz);
  }
  if (chart == 'D' || chart == 'T' || chart == 'U' ||
    chart == 'a' || chart == 'A' || chart == 'm' || chart == 'M')
    PrintTab(' ', 10-CchSz(szObjDisp[obj2]));
  is.fSeconds = fSav;
}


// Based on the given chart information, display all the aspects taking place
// in the chart, as specified with the -D switch. The aspects are printed in
// order of influence determined by treating them as happening outside among
// transiting planets, such that rare outer planet aspects are given more
// power than common ones among inner planets. (This is almost identical to
// the -a list, except the influences are different.)

void ChartInDayInfluence(void)
{
  int source[MAXINDAY], aspect[MAXINDAY], dest[MAXINDAY];
  real power[MAXINDAY], rPowSum = 0.0;
  int ca[cAspect + 1], co[objMax];
  char sz[cchSzDef];
  int occurcount = 0, i, j, k, l, m;
  flag f;

  ClearB((pbyte)ca, sizeof(ca));
  ClearB((pbyte)co, sizeof(co));

  // Go compute the aspects in the chart.

  i = us.fAppSep;
  us.fAppSep = fTrue;     // Always want applying vs. separating orbs.
  FCreateGrid(fFalse);
  us.fAppSep = i;

  // Search through the grid and build up the list of aspects.

  for (j = 1; j <= is.nObj; j++) {
    if (FIgnore(j))
      continue;
    for (i = 0; i < j; i++) {
      if (FIgnore(i) || (k = grid->n[i][j]) == 0 || occurcount >= MAXINDAY)
        continue;
      source[occurcount] = i; aspect[occurcount] = k; dest[occurcount] = j;
      l = grid->v[i][j];
      power[occurcount] = (RTransitInf(i)/4.0) * (RTransitInf(j)/4.0) *
        rAspInf[k]*(1.0-(real)NAbs(l)/3600.0/GetOrb(i, j, k));
      rPowSum += power[occurcount];
      ca[k]++;
      co[i]++; co[j]++;
      occurcount++;
    }
  }

  // Sort aspects by order of influence.

  for (i = 1; i < occurcount; i++) {
    j = i-1;
    while (j >= 0) {
      k = j+1;
      switch (us.nAspectSort) {
      default:  f = power[j] > power[k]; break;
      case aso: f = NAbs(grid->v[source[j]][dest[j]]) <
                    NAbs(grid->v[source[k]][dest[k]]); break;
      case asn: f = grid->v[source[j]][dest[j]] <
                    grid->v[source[k]][dest[k]]; break;
      case asO: f = source[j]*cObj + dest[j] <
                    source[k]*cObj + dest[k]; break;
      case asP: f = dest[j]*cObj + source[j] <
                    dest[k]*cObj + source[k]; break;
      case asA: f = aspect[j]*cObj*cObj + source[j]*cObj + dest[j] <
                    aspect[k]*cObj*cObj + source[k]*cObj + dest[k]; break;
      case asC: f = planet[source[j]] < planet[source[k]]; break;
      case asD: f = planet[dest[j]] < planet[dest[k]]; break;
      case asM: f = Midpoint(planet[dest[j]], planet[source[j]]) <
                    Midpoint(planet[dest[k]], planet[source[k]]); break;
      }
      if (f)
        break;
      SwapN(source[j], source[j+1]);
      SwapN(aspect[j], aspect[j+1]);
      SwapN(dest[j], dest[j+1]);
      SwapR(&power[j], &power[j+1]);
      j--;
    }
  }

  // Now display each aspect line.

  for (i = 0; i < occurcount; i++) {
    sprintf(sz, "%3d: ", i+1); PrintSz(sz);
    j = source[i]; k = aspect[i]; l = dest[i];
    PrintAspect(j, planet[j], (int)RSgn(ret[j]), k,
      l, planet[l], (int)RSgn(ret[l]), 'D');
    m = grid->v[j][l];
    AnsiColor(m < 0 ? kWhiteA : kLtGrayA);
    sprintf(sz, " - %s%2d%c%02d'", m < 0 ? "app" : "sep",
      NAbs(m)/3600, chDeg1, NAbs(m)%3600/60); PrintSz(sz);
    if (is.fSeconds) {
      sprintf(sz, "%02d\"", NAbs(m)%60); PrintSz(sz);
    }
    AnsiColor(kDkGreenA);
    sprintf(sz, " - power:%6.2f", power[i]); PrintSz(sz);
    PrintInDayEvent(j, k, l, -1);
  }
  if (occurcount == 0)
    PrintSz("Empty transit aspect list.\n");
  PrintAspectSummary(ca, co, occurcount, rPowSum);
}


// Given an arbitrary day, determine what aspects are made between this
// transiting chart and the given natal chart, as specified with the -T
// switch, and display the transits in order sorted by influence.

void ChartTransitInfluence(flag fProg)
{
  int source[MAXINDAY], aspect[MAXINDAY], dest[MAXINDAY];
  real power[MAXINDAY], rPowSum = 0.0;
  int ca[cAspect + 1], co[objMax];
  byte ignore3[objMax];
  char sz[cchSzDef];
  int occurcount = 0, fProgress = us.fProgress, i, j, k, l, m;
  flag fSav, f;

  ClearB((pbyte)ca, sizeof(ca));
  ClearB((pbyte)co, sizeof(co));

  // Cast the natal and transiting charts as with a relationship chart.

  cp1 = cp0;
  for (i = 0; i <= is.nObj; i++) {
    ignore3[i] = ignore[i]; ignore[i] = ignore2[i];
  }
  SetCI(ciCore, ciTran.mon, ciTran.day, ciTran.yea, ciTran.tim,
    Dst, Zon, Lon, Lat);
  if (us.fProgress = fProg) {
    is.JDp = MdytszToJulian(MM, DD, YY, TT, SS, ZZ);
    ciCore = ciMain;
  }
  CastChart(0);
  cp2 = cp0;
  for (i = 0; i <= is.nObj; i++) {
    ignore[i] = ignore3[i];
  }

  // Do a relationship aspect grid to get the transits. Have to make and
  // restore three changes to get it right for this chart: (1) Make the natal
  // planets have zero velocity so applying vs. separating is only a function
  // of the transiter. (2) Force applying vs. separating orbs regardless if
  // -ga or -ma is in effect or not. (3) Finally tweak the main restrictions
  // to allow for transiting objects not restricted.

  for (i = 0; i <= is.nObj; i++) {
    ret[i] = cp1.dir[i];
    cp1.dir[i] = 0.0;
    ignore3[i] = ignore[i];
    ignore[i] = ignore[i] && ignore2[i];
  }
  fSav = us.fAppSep; us.fAppSep = fTrue;
  f = FCreateGridRelation(fFalse);
  us.fAppSep = fSav;
  for (i = 0; i <= is.nObj; i++) {
    cp1.dir[i] = ret[i];
    ignore[i] = ignore3[i];
  }
  if (!f)
    return;

  // Loop through the grid, and build up a list of the valid transits.

  for (i = 0; i <= is.nObj; i++) {
    if (FIgnore2(i))
      continue;
    for (j = 0; j <= is.nObj; j++) {
      if (FIgnore(j) || (is.fReturn && i != j) || (k = grid->n[i][j]) == 0 ||
        occurcount >= MAXINDAY)
        continue;
      source[occurcount] = i; aspect[occurcount] = k; dest[occurcount] = j;
      l = grid->v[i][j];
      power[occurcount] = RTransitInf(i) * (RObjInf(j)/4.0) * rAspInf[k] *
        (1.0-(real)NAbs(l)/3600.0/GetOrb(i, j, k));
      rPowSum += power[occurcount];
      ca[k]++;
      co[i]++; co[j]++;
      occurcount++;
    }
  }

  // After all transits located, sort them by their total power.

  for (i = 1; i < occurcount; i++) {
    j = i-1;
    while (j >= 0) {
      k = j+1;
      switch (us.nAspectSort) {
      default:  f = power[j] > power[k]; break;
      case aso: f = NAbs(grid->v[source[j]][dest[j]]) <
                    NAbs(grid->v[source[k]][dest[k]]); break;
      case asn: f = grid->v[source[j]][dest[j]] <
                    grid->v[source[k]][dest[k]]; break;
      case asO: f = source[j]*cObj + dest[j] <
                    source[k]*cObj + dest[k]; break;
      case asP: f = dest[j]*cObj + source[j] <
                    dest[k]*cObj + source[k]; break;
      case asA: f = aspect[j]*cObj*cObj + source[j]*cObj + dest[j] <
                    aspect[k]*cObj*cObj + source[k]*cObj + dest[k]; break;
      case asC: f = cp2.obj[source[j]] < cp2.obj[source[k]]; break;
      case asD: f = cp1.obj[dest[j]] < cp1.obj[dest[k]]; break;
      case asM: f = Midpoint(cp1.obj[dest[j]], cp2.obj[source[j]]) <
                    Midpoint(cp1.obj[dest[k]], cp2.obj[source[k]]); break;
      }
      if (f)
        break;
      SwapN(source[j], source[k]);
      SwapN(aspect[j], aspect[k]);
      SwapN(dest[j], dest[k]);
      SwapR(&power[j], &power[k]);
      j--;
    }
  }

  // Now loop through list and display each transit in effect at the time.

  for (i = 0; i < occurcount; i++) {
    k = aspect[i];
    l = source[i];
    sprintf(sz, "%3d: ", i+1); PrintSz(sz);
    PrintAspect(l, cp2.obj[l], (int)RSgn(cp2.dir[l]), k,
      dest[i], cp1.obj[dest[i]], (int)RSgn(cp1.dir[dest[i]]),
      (char)(fProg ? 'U' : 'T'));
    m = grid->v[l][dest[i]];
    AnsiColor(m < 0 ? kWhiteA : kLtGrayA);
    sprintf(sz, "- %s%2d%c%02d'", m < 0 ? "app" : "sep",
      NAbs(m)/3600, chDeg1, NAbs(m)%3600/60); PrintSz(sz);
    if (is.fSeconds) {
      sprintf(sz, "%02d\"", NAbs(m)%60); PrintSz(sz);
    }
    AnsiColor(kDkGreenA);
    sprintf(sz, " - power: %5.2f", power[i]); PrintSz(sz);
    if (k == aCon && l == dest[i]) {    // Print a "R" to mark returns.
      AnsiColor(kWhiteA);
      PrintSz(" R");
      if (is.fSeconds)
        PrintSz("eturn");
    }
    PrintL();
#ifdef INTERPRET
    if (us.fInterpret)
      InterpretTransit(l, k, dest[i]);
#endif
    AnsiColor(kDefault);
  }

  if (occurcount == 0)
    PrintSz("Empty transit list.\n");
  PrintAspectSummary(ca, co, occurcount, rPowSum);
  us.fProgress = fProgress;
  ciCore = ciMain;
  CastChart(1);
}


// Given the zodiac location of a planet in the sky and its declination, and
// a location on the Earth, compute the azimuth and altitude of where on the
// local horizon sky the planet would appear to one at the given location. A
// reference MC position at Greenwich is also needed for this.

void EclToHoriz(real *azi, real *alt, real obj, real objalt, real mc,
  real lat)
{
  real lonz, latz;

  lonz = obj; latz = objalt;
  EclToEqu(&lonz, &latz);
  lonz = Mod(mc - lonz + rDegQuad);
  EquToLocal(&lonz, &latz, rDegQuad - lat);
  *azi = rDegMax - lonz; *alt = latz;
}


// Display a calendar for the given month in the chart, as specified with the
// -K switch. When color is on, the title is white, weekends are highlighted
// in red, and the specific day in the chart is colored green.

void ChartCalendarMonth(void)
{
  char sz[cchSzDef];
  int i, j, k;

  AnsiColor(kWhiteA);
  PrintTab(' ', (16-CchSz(szMonth[Mon])) >> 1);
  sprintf(sz, "%s%5d\n", szMonth[Mon], Yea); PrintSz(sz);
  for (i = 0; i < cWeek; i++) {
    sprintf(sz, "%c%c%c", szDay[i][0], szDay[i][1], i < cWeek-1 ? ' ' : '\n');
    PrintSz(sz);
  }
  j = DayOfWeek(Mon, 1, Yea);
  AnsiColor(kDefault);
  for (i = 0; i < j; i++) {
    if (i == 0)
      AnsiColor(kRedA);
    PrintSz("-- ");
    if (i == 0)
      AnsiColor(kDefault);
  }
  k = DayInMonth(Mon, Yea);
  for (i = 1; i <= k; i = AddDay(Mon, i, Yea, 1)) {
    if (i == (int)Day)
      AnsiColor(kGreenA);
    else if (j == 0 || j == cWeek-1)
      AnsiColor(kRedA);
    sprintf(sz, "%2d", i); PrintSz(sz);
    if (j == 0 || j == cWeek-1 || i == Day)
      AnsiColor(kDefault);
    if (j < cWeek-1) {
      j++;
      PrintCh(' ');
    } else {
      j = 0;
      PrintL();
    }
  }
  while (j > 0 && j < cWeek) {
    if (j == cWeek-1)
      AnsiColor(kRedA);
    j++;
    sprintf(sz, "--%c", j < cWeek ? ' ' : '\n'); PrintSz(sz);
  }
  AnsiColor(kDefault);
}


// Display a calendar for the entire year given in the chart, as specified
// with the -Ky switch. This is just like twelve of the individual month
// calendars above displayed together, with same color highlights and all.

void ChartCalendarYear(void)
{
  char sz[cchSzDef];
  int r, w, c, m, d, dy, p[3], l[3], n[3];

  dy = DayOfWeek(1, 1, Yea);
  for (r = 0; r < 4; r++) {     // Loop over one set of three months.
    AnsiColor(kWhiteA);
    for (c = 0; c < 3; c++) {
      m = r*3+c+1;
      PrintTab(' ', (16-CchSz(szMonth[m])) >> 1);
      sprintf(sz, "%s%5d", szMonth[m], Yea); PrintSz(sz);
      if (c < 2)
        PrintTab(' ', 20 + MONTHSPACE -
          ((16-CchSz(szMonth[m])) >> 1) - CchSz(szMonth[m]) - 5);
    }
    PrintL();
    for (c = 0; c < 3; c++) {
      for (d = 0; d < cWeek; d++) {
        sprintf(sz, "%c%c%c", szDay[d][0], szDay[d][1],
          d < cWeek-1 || c < 2 ? ' ' : '\n'); PrintSz(sz);
      }
      if (c < 2)
        PrintTab(' ', MONTHSPACE-1);
      m = r*3+c+1;
      p[c] = dy % cWeek;
      l[c] = DayInMonth(m, Yea);
      n[c] = 0;
      dy += DaysInMonth(m, Yea);
    }
    for (w = 0; w < cWeek-1; w++) {    // Loop over one set of week rows.
      for (c = 0; c < 3; c++) {        // Loop over one week in a month.
        m = r*3+c+1;
        d = 0;
        if (w == 0)
          while (d < p[c]) {
            if (d == 0)
              AnsiColor(kRedA);
            PrintSz("-- ");
            if (d == 0)
              AnsiColor(kDefault);
            d++;
          }
        AnsiColor(kDefault);
        while (d < cWeek && n[c] < l[c]) {
          n[c] = AddDay(m, n[c], Yea, 1);
          if (n[c] == Day && m == Mon)
            AnsiColor(kGreenA);
          else if (d == 0 || d == cWeek-1)
            AnsiColor(kRedA);
          sprintf(sz, "%2d%c", n[c], d < cWeek-1 || c < 2 ? ' ' : '\n');
          PrintSz(sz);
          if (d == 0 || d == cWeek-1 || (n[c] == Day && m == Mon))
            AnsiColor(kDefault);
          d++;
        }
        while (d < cWeek) {
          if (d == 0 || d == cWeek-1)
            AnsiColor(kRedA);
          sprintf(sz, "--%c", d < cWeek-1 || c < 2 ? ' ' : '\n'); PrintSz(sz);
          if (d == 0)
            AnsiColor(kDefault);
          d++;
        }
        if (c < 2)
          PrintTab(' ', MONTHSPACE-1);
      }
    }
    if (r < 3)
      PrintL();
  }
  AnsiColor(kDefault);
}


// Display either a biorhythm chart or the time difference in various units
// between two charts, i.e. two types of relationship "charts" that aren't
// related in any way to planetary positions, as specified by either the
// -rb or -rd switches, respectively.

void DisplayRelation(void)
{
  char sz[cchSzDef];
  int i;
  real k, l;
#ifdef BIORHYTHM
  int j;
#endif

  // If calculating the difference between two dates, then display the value
  // and return, as with the -rd switch.

  if (us.nRel == rcDifference) {
    PrintSz("Differences between the dates in the two charts:\n");
    k = RAbs(ciMain.lon - ciTwin.lon);
    l = RAbs(ciMain.lat - ciTwin.lat);
    for (i = 1; i <= 10; i++) {
      if (i <= cRainbow)
        AnsiColor(kRainbowA[i]);
      else
        AnsiColor(kDefault);
      switch (i) {
      case 1: sprintf(sz, "Years  : %.2f", is.JD/rDayInYear);    break;
      case 2: sprintf(sz, "Months : %.2f", is.JD/(rDayInYear/12.0)); break;
      case 3: sprintf(sz, "Weeks  : %.2f", is.JD/7.0);           break;
      case 4: sprintf(sz, "Days   : %.2f", is.JD);               break;
      case 5: sprintf(sz, "Hours  : %.2f", is.JD*24.0);          break;
      case 6: sprintf(sz, "Minutes: %.2f", is.JD*(24.0*60.0));   break;
      case 7: sprintf(sz, "Seconds: %.2f", is.JD*(24.0*3600.0)); break;
      case 8: sprintf(sz, "Longitude: %.2f", k);                 break;
      case 9: sprintf(sz, "Latitude : %.2f", l);                 break;
      case 10:
        l = SphDistance(ciMain.lon, ciMain.lat, ciTwin.lon, ciTwin.lat);
        sprintf(sz, "Distance : %.2f (%.2f %s)", l, l / 360.0 *
          (us.fEuroDist ? 40075.0 : 24901.0), us.fEuroDist ? "km" : "miles");
        break;
      }
      PrintSz(sz);
      PrintL();
    }
    AnsiColor(kDefault);
    return;
  }

#ifdef BIORHYTHM
  // If doing a biorhythm (-rb switch), then calculate it for someone born on
  // the older date, at the time of the younger date. Loop through the days
  // preceeding and following the date in question.

  is.JD = RFloor(is.JD + rRound);
  for (is.JD -= (real)(us.nBioday/2), i = -us.nBioday/2; i <= us.nBioday/2;
    i++, is.JD += 1.0) {
    if (i == 0)
      AnsiColor(kWhiteA);
    else if (i == 1)
      AnsiColor(kDefault);
    j = NAbs(i);
    sprintf(sz, "T%c%d%sDay%c:", i < 0 ? '-' : '+', j,
      j < 10 ? " " : "", j != 1 ? 's' : ' '); PrintSz(sz);
    for (j = 1; j <= 3; j++) {
      PrintCh(' ');
      AnsiColor(j <= 1 ? kRedA : (j == 2 ? kBlueA : kYellowA));
      switch (j) {
      case 1: k = brPhy; PrintSz("Physical");     break;
      case 2: k = brEmo; PrintSz("Emotional");    break;
      case 3: k = brInt; PrintSz("Intellectual"); break;
      }
      AnsiColor(i ? kDefault : kWhiteA);

      // The biorhythm calculation is below.

      l = RBiorhythm(is.JD, k);
      sprintf(sz, " at %c%3.0f%%", l < 0.0 ? '-' : '+', RAbs(l)); PrintSz(sz);

      // Print smiley face, medium face, or sad face based on current cycle.

      AnsiColor(kDkGreenA);
      sprintf(sz, " :%c", l > 50.0 ? ')' : (l < -50.0 ? '(' : '|'));
      PrintSz(sz);
      AnsiColor(i ? kDefault : kWhiteA);
      if (j < 3)
        PrintCh(',');
    }
    PrintL();
  }
#endif /* BIORHYTHM */
}

/* charts2.cpp */
