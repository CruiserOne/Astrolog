/*
** Astrolog (Version 7.80) File: charts1.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2025 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** The main ephemeris databases and calculation routines are from the
** library SWISS EPHEMERIS and are programmed and copyright 1997-2008 by
** Astrodienst AG. Use of that source code is subject to license for Swiss
** Ephemeris Free Edition at https://www.astro.com/swisseph/swephinfo_e.htm.
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
** Last code change made 6/19/2025.
*/

#include "astrolog.h"


/*
******************************************************************************
** Single Chart Display Routines.
******************************************************************************
*/

// Print header info showing the date and time of the chart being displayed.
// This is used by ChartListing() and other charts in PrintChart().

void PrintHeader(int nSpace)
{
  char sz[cchSzDef];
  int day, fNam, fLoc, fProg;

  if (nSpace < 0) {
    // If another chart has been printed, just skip a couple lines instead.
    PrintL2();
    return;
  }

  fNam = FSzSet(ciMain.nam); fLoc = FSzSet(ciMain.loc);
  AnsiColor(kWhiteA);
  sprintf(sz, "%s %s", szAppName, szVersionCore); PrintSz(sz);
  if (!us.fSeconds || fNam)
    PrintSz(" chart ");
  else
    PrintSz(": ");
  if (FNoTimeOrSpace(ciMain))
    PrintSz("(No time or space)\n");
  else if (us.nRel == rcComposite)
    PrintSz("(Composite)\n");
  else {
    if (!us.fSeconds || fNam)
      PrintSz("for ");
    sprintf(sz, "%s%s", fNam ? ciMain.nam : "", fNam ? "\n" : "");
    PrintSz(sz);
    day = DayOfWeek(Mon, Day, Yea);
    sprintf(sz, "%.3s %s %s (%s)", szDay[day], SzDate(Mon, Day, Yea, 3),
      SzTim(Tim), SzOffset(Zon, Dst, Lon)); PrintSz(sz);
    sprintf(sz, "%c%s%s%s\n", fLoc && !fNam ? '\n' : ' ',
      fLoc ? ciMain.loc : "", fLoc ? " " : "", SzLocation(Lon, Lat));
    PrintSz(sz);
    if (us.fProgress) {
      sprintf(sz, "Progressed To: %.3s %s %s (%cT Zone %s)\n",
        szDay[DayOfWeek(MonT, DayT, YeaT)], SzDate(MonT, DayT, YeaT, 3),
        SzTim(TimT), ChDst(ciDefa.dst), SzZone(ciDefa.zon)); PrintSz(sz);
    }
  }

  // Print second set of chart information.
  if (us.nRel < rcNone || us.nRel == rcSynastry) {
    AnsiColor(kDkGray);
    fNam = FSzSet(ciTwin.nam); fLoc = FSzSet(ciTwin.loc);
    fProg = us.nRel == rcProgress;
    sprintf(sz, "Chart %s %s%s", !us.fSeconds || fNam ?
      (!fProg ? "#2 comparison for" : "#2 progressed for") :
      (!fProg ? "Number2:" : "Progr.2:"), fNam ? ciTwin.nam : "",
      fNam ? "\n" : ""); PrintSz(sz);
    if (FNoTimeOrSpace(ciTwin))
      PrintSz("(No time or space)\n");
    else {
      day = DayOfWeek(ciTwin.mon, ciTwin.day, ciTwin.yea);
      sprintf(sz, "%.3s %s %s (%cT Zone %s)", szDay[day],
        SzDate(ciTwin.mon, ciTwin.day, ciTwin.yea, 3), SzTim(ciTwin.tim),
        ChDst(ciTwin.dst), SzZone(ciTwin.zon)); PrintSz(sz);
      sprintf(sz, "%c%s%s%s\n", fLoc && !fNam ? '\n' : ' ',
        fLoc ? ciTwin.loc : "", fLoc ? " " : "",
        SzLocation(ciTwin.lon, ciTwin.lat)); PrintSz(sz);
    }
  }

  AnsiColor(kDefault);
  if (nSpace > 0)
    PrintL();
}


// Print the straight listing of planet and house positions and specified by
// the -v switch, along with the element table, etc.

void ChartListing(void)
{
  ET et;
  char sz[cchSzMax];
  int i, j, k, l;
  real rT;
  flag fSav;

  CreateElemTable(&et);

  PrintHeader(0);    // Show time and date of the chart being displayed.

#ifdef INTERPRET
  if (us.fInterpret) {          // Print an interpretation if -I in effect.
    if (us.nRel == rcSynastry)
      InterpretSynastry();      // Print synastry interpretaion for -r -I.
    else
      InterpretLocation();      // Do normal interpretation for just -v -I.
    return;
  }
#endif

  AnsiColor(kDkGrayA);
  if (us.fSeconds) {
    sprintf(sz, "Body  Location%s   Ret. %s%s Rul.      House  Rul. Velocity",
      us.fSecond1K ? "    " : "", us.fEquator2 ? "Declin. " : "Latitude",
      us.fSecond1K ? "    " : "");
    PrintSz(sz);
    if (!us.fListDecan)
      PrintSz("  Lat-Velocity");
    else {
      PrintSz("  ");
      PrintSz(rgszDecan[us.nDecanType]);
    }
    PrintL();
  } else {
    sprintf(sz,
      "Body  Locat. Ret. %s. Rul.      House  Rul. Veloc.    %s%s Houses\n",
      us.fEquator2 ? "Decl" : "Lati", us.fHouse3D ? "3D " : "",
      szSystem[is.nHouseSystem]); PrintSz(sz);
  }
  if (!FSzSet(ciMain.nam) && !FSzSet(ciMain.loc))
    PrintL();

  // Ok, now print out the location of each object.

  for (i = l = 0; us.fSeconds ? l <= oNorm : l <= oNorm || i < cSign+8; l++) {
    j = rgobjList[l];
    if (us.fSeconds) {
      if (FIgnore(j))
        continue;
    } else {
      if (l <= oNorm && FIgnore(j))
        continue;
    }
    if (!us.fSeconds && i < cSign+8 && l > oNorm) {
      PrintTab(' ', 51);
      j = oNorm;
    } else {
      AnsiColor(kObjA[j]);
      sprintf(sz, "%-4.4s: ", szObjDisp[j]); PrintSz(sz);
      PrintZodiac(planet[j]);
      sprintf(sz, " %c ", ChRet(ret[j])); PrintSz(sz);
      PrintAltitude(planetalt[j]);
      sprintf(sz, " (%c)", *Dignify(j, SFromZ(planet[j])));
      PrintSz(FCusp(j) ? "    " : sz);
      k = inhouse[j];
      AnsiColor(kSignA(k));
      sprintf(sz, " [%2d%s house] ", k, szSuffix[k]); PrintSz(sz);
      AnsiColor(kDefault);
      sprintf(sz, "[%c] ", *Dignify(j, k)); PrintSz(FCusp(j) ? "    " : sz);
      PrintCh(ret[j] < 0.0 ? '-' : '+');
      rT = RAbs(ret[j]);
      sprintf(sz, "%.7f", rT); sz[us.fSeconds ? 9 : 5] = chNull;
      PrintSz(sz);
    }
    if (!us.fSeconds) {

      // For some lines, append the house cusp positions.

      if (i < cSign) {
        PrintSz("  -  ");
        AnsiColor(kSignA(i+1));
        sprintf(sz, "House cusp %2d: ", i+1); PrintSz(sz);
        PrintZodiac(chouse[i+1]);
      }

      // For some lines, append the element table information.

      if (i == cSign+1)
        PrintSz("     Car Fix Mut TOT");
      else if (i > cSign+1 && i < cSign+6) {
        k = i-(cSign+1)-1;
        AnsiColor(kElemA[k]);
        sprintf(sz, "  %c%c%c%3d %3d %3d %3d",
          szElem[k][0], szElem[k][1], szElem[k][2],
          et.coElemMode[k][0], et.coElemMode[k][1], et.coElemMode[k][2],
          et.coElem[k]); PrintSz(sz);
        AnsiColor(kDefault);
      } else if (i == cSign+6) {
        AnsiColor(kDefault);
        sprintf(sz, "  TOT %2d %3d %3d %3d",
          et.coMode[0], et.coMode[1], et.coMode[2], et.coSum); PrintSz(sz);
      } else if (i == cSign+7)
        PrintTab(' ', 23);
      sz[0] = chNull;
      k = kDefault;
      switch (i-cSign) {
      case 1: sprintf(sz, "   +:%2d", et.coYang);  break;
      case 2: sprintf(sz, "   -:%2d", et.coYin);   break;
      case 3: sprintf(sz, "   M:%2d", et.coMC);    k = eEar; break;
      case 4: sprintf(sz, "   N:%2d", et.coIC);    k = eWat; break;
      case 5: sprintf(sz, "   A:%2d", et.coAsc);   k = eFir; break;
      case 6: sprintf(sz, "   D:%2d", et.coDes);   k = eAir; break;
      case 7: sprintf(sz,    "<:%2d", et.coLearn); break;
      }
      AnsiColor(k == kDefault ? k : kElemA[k]);
      PrintSz(sz);
    } else {
      if (!us.fListDecan) {
        sprintf(sz, " %c", retalt[j] < 0.0 ? '-' : '+'); PrintSz(sz);
        rT = RAbs(retalt[j]);
        sprintf(sz, "%9.7f", rT); PrintSz(sz);
      } else {
        PrintCh(' ');
        if (us.nDecanType == dd27) {
          k = us.nDegForm; us.nDegForm = dfNak;
          fSav = us.fSeconds; us.fSeconds = fFalse;
          PrintZodiac(planet[j]);
          us.fSeconds = fSav; us.nDegForm = k;
        } else if (us.nDecanType == ddConstel) {
#ifdef CONSTELGRAPH
          int sign, ki = kWhiteA;
          k = LookupConstel(planet[j], planetalt[j]);
          for (sign = 1; sign <= cSign; sign++)
            if (iCnstlZodiac[sign] == k) {
              ki = kSignA(sign);
              break;
            }
          AnsiColor(ki);
          PrintSz(szCnstlAbbrev[k]);
          PrintSz(": ");
          PrintSz(szCnstlName[k]);
#endif
        } else if (us.nDecanType == ddDecanS || us.nDecanType >= ddNavamsa) {
          if (us.nDecanType == ddDecanS)
            rT = Decan(planet[j]);
          else if (us.nDecanType == ddNavamsa)
            rT = Navamsa(planet[j]);
          else if (us.nDecanType == dd12)
            rT = Mod(planet[j] * 12.0);
          else if (us.nDecanType == ddDwad)
            rT = Dwad(planet[j]);
          fSav = us.fSeconds; us.fSeconds = fFalse;
          PrintZodiac(rT);
          us.fSeconds = fSav;
        } else {
          if (us.nDecanType == ddDecanR)
            k = 0;
          else if (us.nDecanType == ddChaldea)
            k = 1;
          else if (us.nDecanType == ddEgypt)
            k = 2;
          else if (us.nDecanType == ddPtolemy)
            k = 3;
          k = ObjTerm(planet[j], k);
          AnsiColor(kObjA[k]);
          sprintf(sz, "%.4s", szObjDisp[k]); PrintSz(sz);
        }
      }
    }
    PrintL();
    i++;
  }

  // Do another loop to print out the stars in their specified order.

  if (us.fStar) for (i = starLo; i <= starHi; i++) {
    j = rgobjList[i];
    if (ignore[j])
      continue;
    AnsiColor(kObjA[j]);
    sprintf(sz, "%-4.4s: ", szObjDisp[j]); PrintSz(sz);
    PrintZodiac(planet[j]);
    sprintf(sz, " %c ", ChRet(ret[j])); PrintSz(sz);
    PrintAltitude(planetalt[j]);
    k = inhouse[j];
    AnsiColor(kSignA(k));
    sprintf(sz, "     [%2d%s house]", k, szSuffix[k]); PrintSz(sz);
    AnsiColor(kDefault);
    sprintf(sz, "     %c", ret[j] < 0.0 ? '-' : '+'); PrintSz(sz);
    rT = RAbs(ret[j]);
    sprintf(sz, "%.7f", rT); sz[us.fSeconds ? 9 : 5] = chNull;
    PrintSz(sz);
    AnsiColor(kObjA[j]);
    sprintf(sz, " Star #%2d: %5.2f\n", i-oNorm, rStarBright[j-oNorm]);
    PrintSz(sz);
  }

#ifdef SWISSGRAPH
  ES es;

  // Print extra stars and extra asteroids.
  for (l = 0; l <= 1; l++) {
    if (!(!l ? gs.fAllStar : gs.nAstLo > 0))
      continue;
    PrintL();
    if (!l)
      SwissComputeStarSort(is.T, NULL);
    else
      SwissComputeAsteroidSort(is.T, NULL);
    for (i = 1; !l ? SwissComputeStarSort(is.T, &es) :
      SwissComputeAsteroidSort(is.T, &es); i++) {
      if (!l) {
        AnsiColor(es.ki != kDefault ? es.ki : KStar2A(es.mag));
        if (es.mag == rStarNot)
          es.mag = 99.99;
        sprintf(sz, "%4d %-8.8s ", i, es.pchDes); PrintSz(sz);
      } else {
        AnsiColor(es.ki != kDefault ? es.ki : kDefault);
        sprintf(sz, "%6d ", i); PrintSz(sz);
      }
      PrintZodiac(es.lon);
      sprintf(sz, " %c ", ChRet(es.dir)); PrintSz(sz);
      PrintAltitude(es.lat);
      k = NHousePlaceIn(es.lon, es.lat);
      AnsiColor(kSignA(k));
      sprintf(sz, " [%2d%s house]", k, szSuffix[k]); PrintSz(sz);
      AnsiColor(kDefault);
      sprintf(sz, " %c", es.dir < 0.0 ? '-' : '+'); PrintSz(sz);
      rT = RAbs(es.dir);
      sprintf(sz, "%.7f", rT); sz[us.fSeconds ? 9 : 5] = chNull;
      PrintSz(sz);
      if (!l)
        sprintf(sz, " %5.2f%s%s\n", es.mag, *es.pchNam ? "  " : "",
          es.pchNam);
      else
        sprintf(sz, "  %s\n", es.sz);
      PrintSz(sz);
    }
  }
#endif
}


// Compose and print a string to display within an aspect grid cell.

void PrintGridCell(int x, int y, int type, int row)
{
  char sz[cchSzDef], szT[cchSzDef];
  int n, d, m, s;
  real v;

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
      AnsiColor(kAspA[n]);
      if (row <= 2) {
        if (us.fParallel && n <= aOpp)
          n += cAspect;
        sprintf(sz, "%.3s", szAspectAbbrev[n]);
      } else if (us.fDistance && !us.fParallel) {
        sprintf(szT, "%c%f", rgchAppSep[us.nAppSep*2 + (v >= 0.0)],
          RAbs(v));
        sprintf(&szT[5 + 3*us.fSeconds], "%s", "%");
        sprintf(sz, "%.3s", szT + (row <= 3 ? 0 : (row == 4 ? 3 : 6)));
      } else if (us.nDegForm != df360) {
        if (row <= 3) {
          if (d < 100)
            sprintf(sz, "%c%2d", rgchAppSep[us.nAppSep*2 + (v >= 0.0)], d);
          else
            sprintf(sz, "%3d", d);
        } else
          sprintf(sz, "%02d%c", row <= 4 ? m : s, row <= 4 ? '\'' : '"');
      } else {
        sprintf(szT, "%c%f", rgchAppSep[us.nAppSep*2 + (v >= 0.0)], RAbs(v));
        sprintf(sz, "%.3s", szT + (row <= 3 ? 0 : (row == 4 ? 3 : 6)));
      }
    } else
      sprintf(sz, "   ");
  }

  // For midpoint cells, print degrees and minutes.
  else if (type == 2 || (type == 0 && us.nDegForm == dfHM)) {
    AnsiColor(kSignA(n));
    if (row <= 2)
      sprintf(sz, "%.3s", szSignName[n]);
    else if (us.nDegForm == dfHM) {
      sprintf(szT, "%s", SzZodiac((real)((n-1)*30) + v));
      sprintf(sz, "%.3s", szT + (row <= 3 ? 0 : (row == 4 ? 4 : 8)));
    } else if (us.nDegForm != df360) {
      sprintf(szT, "%%%s2d%%c", row <= 3 ? "" : "0");
      sprintf(sz, szT, row <= 3 ? d : (row == 4 ? m : s),
        row <= 3 ? chDegS : (row == 4 ? '\'' : '\"'));
    } else {
      sprintf(szT, "%9.6f", RAbs(v));
      sprintf(sz, "%.3s", szT + (row <= 3 ? 0 : (row == 4 ? 3 : 6)));
    }
  }

  // For main diagonal cells, print sign and degree of the planet.
  else {
    if (row <= 2) {
      AnsiColor(kObjA[y]);
      sprintf(sz, "%.3s", szObjDisp[y]);
    } else {
      AnsiColor(kSignA(n));
      if (us.nDegForm != df360) {
        if (row == 4)
          sprintf(sz, "%.3s", szSignName[n]);
        else
          sprintf(sz, "%2d%c", row <= 3 ? d : m, row <= 3 ? chDegS : '\'');
      } else {
        sprintf(szT, "%9.5f", RAbs((real)((n-1)*30) + v));
        sprintf(sz, "%.3s", szT + (row <= 3 ? 0 : (row == 4 ? 3 : 6)));
      }
    }
  }
  PrintSz(sz);
}


// Print out the aspect and midpoint grid for a chart, as specified with the
// -g switch. Each grid row takes up 4 lines of text by default.

void ChartGrid(void)
{
  int x0, y0, x, y, r, xc, yc, xg, yg, temp;

#ifdef INTERPRET
  if (us.fInterpret) {    // Print interpretation instead if -I in effect.
    InterpretGrid();
    return;
  }
#endif

  for (yc = 0, y0 = 0; y0 <= is.nObj; y0++) {
    y = rgobjList[y0];
    if (ignore[y])
      continue;
    for (r = 1; r <= 4 + us.fSeconds; r++) {
      for (xc = 0, x0 = 0; x0 <= is.nObj; x0++) {
        x = rgobjList[x0];
        if (ignore[x])
          continue;
        if (yc > 0 && xc > 0 && yc+r > 1)
          PrintCh2(r > 1 ? chV : chC);
        if (r > 1) {
          xg = x; yg = y;
          if ((x > y) != (x0 > y0))
            SwapN(xg, yg);
          temp = grid->n[xg][yg];

          // Print aspect rows.
          if (xg < yg) {
            PrintGridCell(xg, yg, 1, r);

          // Print midpoint rows.
          } else if (xg > yg) {
            PrintGridCell(xg, yg, 2, r);

          // Print the diagonal of object names.
          } else {
            AnsiColor(kReverse);
            PrintGridCell(-1, y, 0, r);
          }
          AnsiColor(kDefault);
        } else
          if (yc > 0)
            PrintTab2(chH, 3);
        xc++;
      }
      if (yc+r > 1)
        PrintL();
      yc++;
    }
  }
  if (yc <= 0)
    PrintSz("Empty aspect grid.\n");
}


// This is a subprocedure of DisplayAspectConfigs(). Here we print out one
// aspect configuration found by the parent procedure.

flag FPrintAspectConfig(int ac, int i1, int i2, int i3, int i4)
{
  char sz[cchSzDef];

#ifdef EXPRESS
  // Skip this aspect configuration if AstroExpression says to do so.
  if (!us.fExpOff && FSzSet(us.szExpConfig)) {
    ExpSetN(iLetterV, ac);
    ExpSetN(iLetterW, i1);
    ExpSetN(iLetterX, i2);
    ExpSetN(iLetterY, i3);
    ExpSetN(iLetterZ, i4);
    if (!NParseExpression(us.szExpConfig))
      return fFalse;
  }
#endif
  AnsiColor(kAspA[rgAspConfig[ac]]);
  sprintf(sz, "%-11s", szAspectConfig[ac]); PrintSz(sz);
  AnsiColor(kDefault);
  sprintf(sz, " %s ", ac == acS3 || ac == acGT || ac == acGC ||
    ac == acMR || ac == acS4 ? "with" : "from");
  PrintSz(sz);
  AnsiColor(kObjA[i1]);
  sprintf(sz, "%.3s: ", szObjDisp[i1]); PrintSz(sz);
  if (!us.fParallel)
    PrintZodiac(planet[i1]);
  else
    PrintAltitude(planetalt[i1]);
  sprintf(sz, " %s ", ac == acS3 || ac == acGT || ac == acS4 ? "and" : "to ");
  PrintSz(sz);
  AnsiColor(kObjA[i2]);
  sprintf(sz, "%.3s: ", szObjDisp[i2]); PrintSz(sz);
  if (!us.fParallel)
    PrintZodiac(planet[i2]);
  else
    PrintAltitude(planetalt[i2]);
  sprintf(sz, " %s ", ac == acGC || ac == acC || ac == acMR ? "to " : "and");
  PrintSz(sz);
  AnsiColor(kObjA[i3]);
  sprintf(sz, "%.3s: ", szObjDisp[i3]); PrintSz(sz);
  if (!us.fParallel)
    PrintZodiac(planet[i3]);
  else
    PrintAltitude(planetalt[i3]);
  if (ac == acGC || ac == acC || ac == acMR || ac == acS4) {
    PrintSz(ac == acS4 ? " and " : " to  ");
    AnsiColor(kObjA[i4]);
    sprintf(sz, "%.3s: ", szObjDisp[i4]); PrintSz(sz);
    if (!us.fParallel)
      PrintZodiac(planet[i4]);
    else
      PrintAltitude(planetalt[i4]);
  }
  PrintL();
  return fTrue;
}


// Scan the aspect grid of a chart and print out any major configurations,
// as specified with the -g0 switch.

void DisplayAspectConfigs(void)
{
  int cac = 0, i, j, k, l;

  for (i = 0; i <= is.nObj; i++) if (!FIgnore(i))
    for (j = 0; j <= is.nObj; j++) if (j != i && !FIgnore(j))
      for (k = 0; k <= is.nObj; k++) if (k != i && k != j && !FIgnore(k)) {
        if (!(us.objRequire >= 0 &&
          i != us.objRequire && j != us.objRequire && k != us.objRequire)) {

          // Is there a Stellium among the current three planets?

          if (i < j && j < k && grid->n[i][j] == aCon &&
              grid->n[i][k] == aCon && grid->n[j][k] == aCon) {
            for (l = 0; l <= is.nObj; l++)
              if (!FIgnore(l) && l != i && l != j && l != k &&
                grid->n[Min(i, l)][Max(i, l)] == aCon &&
                grid->n[Min(j, l)][Max(j, l)] == aCon &&
                grid->n[Min(k, l)][Max(k, l)] == aCon)
                break;
            if (l > is.nObj)
              cac += FPrintAspectConfig(acS3, i, j, k, -1);

          // Is there a Grand Trine?

          } else if (i < j && j < k && grid->n[i][j] == aTri &&
              grid->n[i][k] == aTri && grid->n[j][k] == aTri) {
            cac += FPrintAspectConfig(acGT, i, j, k, -1);

          // Is there a T-Square?

          } else if (j < k && grid->n[j][k] == aOpp &&
              grid->n[Min(i, j)][Max(i, j)] == aSqu &&
              grid->n[Min(i, k)][Max(i, k)] == aSqu) {
            cac += FPrintAspectConfig(acTS, i, j, k, -1);

          // Is there a Yod?

          } else if (j < k && grid->n[j][k] == aSex &&
              grid->n[Min(i, j)][Max(i, j)] == aInc &&
              grid->n[Min(i, k)][Max(i, k)] == aInc) {
            cac += FPrintAspectConfig(acY, i, j, k, -1);
          }
        }
        for (l = 0; l <= is.nObj; l++) if (!FIgnore(l)) {
          if (us.objRequire >= 0 && i != us.objRequire &&
            j != us.objRequire && k != us.objRequire && l != us.objRequire)
            continue;

          // Is there a Grand Cross among the current four planets?

          if (i < j && i < k && i < l && j < l && grid->n[i][j] == aSqu &&
              grid->n[Min(j, k)][Max(j, k)] == aSqu &&
              grid->n[Min(k, l)][Max(k, l)] == aSqu &&
              grid->n[i][l] == aSqu &&
              MinDistance(planet[i], planet[k]) > 150.0 &&
              MinDistance(planet[j], planet[l]) > 150.0) {
            cac += FPrintAspectConfig(acGC, i, j, k, l);

          // Is there a Cradle?

          } else if (i < l && grid->n[Min(i, j)][Max(i, j)] == aSex &&
              grid->n[Min(j, k)][Max(j, k)] == aSex &&
              grid->n[Min(k, l)][Max(k, l)] == aSex &&
              MinDistance(planet[i], planet[l]) > 150.0) {
            cac += FPrintAspectConfig(acC, i, j, k, l);

          // Is there a Mystic Rectangle?

          } else if (i < j && i < k && i < l && grid->n[i][j] == aTri &&
              grid->n[Min(j, k)][Max(j, k)] == aSex &&
              grid->n[Min(k, l)][Max(k, l)] == aTri &&
              grid->n[i][l] == aSex &&
              MinDistance(planet[i], planet[k]) > 150.0 &&
              MinDistance(planet[j], planet[l]) > 150.0) {
            cac += FPrintAspectConfig(acMR, i, j, k, l);

          // Is there a Stellium among the current four planets?

          } else if (i < j && j < k && k < l && grid->n[i][j] == aCon &&
              grid->n[i][k] == aCon && grid->n[i][l] == aCon &&
              grid->n[j][k] == aCon && grid->n[j][l] == aCon &&
              grid->n[k][l] == aCon) {
            cac += FPrintAspectConfig(acS4, i, j, k, l);
          }
        }
      }
  if (cac <= 0)
    PrintSz("No major configurations in aspect grid.\n");
}


// This is subprocedure of ChartWheel(). Here print out the location of a
// particular house cusp as well as what house cusp number it is.

void PrintHouse(int i, flag fLeft)
{
  char sz[cchSzDef];
  real deg;
  int j;

  if (us.fIndian) {
    j = Mod12(12-i);
    deg = ZFromS(j);
  } else {
    j = i;
    deg = chouse[i];
  }
  if (!fLeft)
    PrintZodiac(deg);
  AnsiColor(kSignA(j));
  sprintf(sz, "<%d>", j); PrintSz(sz);
  if (fLeft)
    PrintZodiac(deg);
  else
    AnsiColor(kDefault);
}


// Another subprocedure of ChartWheel(). Print out one of the chart info
// rows in the middle of the wheel (which may be blank) given an index.

void PrintWheelCenter(int irow)
{
  char sz[cchSzDef], szT[cchSzDef];
  int cch, nT;
  flag fNam = FSzSet(ciMain.nam), fLoc = FSzSet(ciMain.loc);

  if (is.nWheelRows > 4)                     // Try to center lines.
    irow -= (is.nWheelRows - 4);
  if (!fNam && !fLoc && is.nWheelRows >= 4)
    irow--;
  if (!fNam && irow >= 1)    // Don't have blank lines if the name
    irow++;                  // and/or location strings are empty.
  if (!fLoc && irow >= 3)
    irow++;

  switch (irow) {
  case 0:
    sprintf(sz, "%s %s chart", szAppName, szVersionCore);
    break;
  case 1:
    sprintf(sz, "%s", ciMain.nam);
    break;
  case 2:
    if (FNoTimeOrSpace(ciMain))
      sprintf(sz, "(No time or space)");
    else {
      nT = DayOfWeek(Mon, Day, Yea);
      sprintf(sz, "%.3s %s %s", szDay[nT], SzDate(Mon, Day, Yea, 2),
        SzTim(Tim));
    }
    break;
  case 3:
    sprintf(sz, "%s", ciMain.loc);
    break;
  case 4:
    sprintf(sz, "%s, %s", SzOffset(Zon, Dst, Lon), SzLocation(Lon, Lat));
    break;
  case 5:
    if (!us.fProgress) {
      nT = us.fEuroTime; us.fEuroTime = fTrue;
      sprintf(szT, "%s", SzTim(cp0.lonMC * (24.0/rDegMax)));
      sprintf(sz, "UT: %s, Sid.T: %s", SzTim(Tim + GetOffsetCI(&ciMain)), szT);
      us.fEuroTime = nT;
    } else
      sprintf(sz, "Prog: %s %s", SzDate(MonT, DayT, YeaT, 2), SzTim(TimT));
    break;
  case 6:
    sprintf(sz, "%s%s houses", us.fHouse3D ? "3D " : "",
      szSystem[is.nHouseSystem]);
    break;
  case 7:
    sprintf(sz, "%s, %s", !us.fSidereal ? "Tropical" :
      (!us.fSidereal2 ? "Sidereal" : "Sidereal Inv"),
      us.objCenter == oSun ?
        (!us.fBarycenter ? "Heliocentric" : "Barycentric") :
      (us.objCenter == oEar ? (!us.fTopoPos ? "Geocentric" : "Topocentric") :
      szObjDisp[us.objCenter]));
    break;
  case 8:
    sprintf(szT, "Julian Day: %%%sf",
      !us.fSeconds ? "13.5" : (!f1K ? "15.7" : "16.8"));
    sprintf(sz, szT, JulianDayFromTime(is.T));
    break;
  default:
    *sz = chNull;
  }

  cch = CwchSz(sz);
  nT = WHEELCOLS*2-1 + us.fSeconds*8 + f1K*8;
  PrintTab(' ', (nT - cch) / 2);
  PrintSz(sz);
  PrintTab(' ', nT-cch - (nT - cch) / 2);
}


// Yet another subprocedure of ChartWheel(). Here print out one line in a
// particular house cell (which may be blank).

void PrintWheelSlot(int obj)
{
  char sz[cchSzDef];

  if (obj >= oEar) {
    AnsiColor(kObjA[obj]);
    sprintf(sz, " %.3s ", szObjDisp[obj]); PrintSz(sz);
    PrintZodiac(planet[obj]);
    sprintf(sz, "%c ", ChRet(ret[obj])); PrintSz(sz);
  } else                            // This particular line is blank.
    PrintTab(' ', WHEELCOLS-1 + us.fSeconds*4 + f1K*4);
}


// Display all the objects in a wheel format on the screen, as specified with
// the -w switch. The wheel is divided into the 12 houses and the planets are
// placed accordingly.

void ChartWheel(void)
{
  int wheel[cSign][WHEELROWS], wheelcols, count = 0, i, j, k, l;
#ifdef WIN
  int nSav;

  // Turn off IBM line characters if displaying in font other than Terminal.
  if (gs.nFontTxt > 0) {
    nSav = us.fAnsiChar; us.fAnsiChar = 0;
  }
#endif

  // Autodetect wheel house size, based on house with most planets in it.

  if (us.nWheelRows <= 0) {
    k = 0;
    for (i = 0; i < cSign; i++)
      wheel[i][0] = 0;
    for (i = 0; i <= is.nObj; i++) {
      if (FIgnore(i) || (FCusp(i) &&
        MinDistance(planet[i], chouse[i-oAsc+1]) < rRound/60.0))
        continue;
      j = inhouse[i]-1;
      l = wheel[j][0] + 1;
      wheel[j][0] = l;
      if (l > k)
        k = l;
    }
    k = Max(k, 4); k = Min(k, WHEELROWS);
    is.nWheelRows = k;
  } else
    is.nWheelRows = us.nWheelRows;  

  // If the seconds (-b0) flag is set, then print all planet and house
  // locations to the nearest zodiac second instead of just to the minute.

  wheelcols = WHEELCOLS + us.fSeconds*4 + f1K*4;

  for (i = 0; i < cSign; i++)
    for (j = 0; j < is.nWheelRows; j++)
      wheel[i][j] = -1;                    // Clear out array.

  // This section of code places each object in the wheel house array.

  for (i = 0; i <= is.nObj && count < is.nWheelRows*12; i++) {
    if (FIgnore(i) || (FCusp(i) &&
      MinDistance(planet[i], chouse[i-oAsc+1]) < rRound/60.0))
      continue;

    // Try to put object in its proper house. If no room, then overflow over
    // to the next succeeding house.

    for (j = (us.fIndian ? Mod12(11-SFromZ(planet[i])) : inhouse[i])-1;
      j < cSign; j = j < cSign ? (j+1)%cSign : j) {

      // Now try to find the proper place in the house to put the object.
      // This is in sorted order, although a check is made for 0 Aries.

      if (wheel[j][is.nWheelRows-1] >= 0)
        continue;
      l = chouse[j+1] > chouse[Mod12(j+2)];
      for (k = 0; wheel[j][k] >= 0 && (planet[i] >= planet[wheel[j][k]] ||
         (l && planet[i] < rDegHalf && planet[wheel[j][k]] > rDegHalf)) &&
        !(l && planet[i] > rDegHalf && planet[wheel[j][k]] < rDegHalf); k++)
        ;

      // Actually insert object in proper place.

      if (wheel[j][k] < 0)
        wheel[j][k] = i;
      else {
        for (l = is.nWheelRows-1; l > k; l--)
          wheel[j][l] = wheel[j][l-1];
        wheel[j][k] = i;
      }
      count++;
      j = cSign;
    }
  }

  // Now, if this is really the -w switch and not -w0, then reverse the order
  // of objects in western houses for more intuitive reading. Also reverse the
  // order of everything in the reverse order Indian wheels.

  if (us.fIndian)
    for (i = 0; i < cSign; i++)
      for (j = 0; j < is.nWheelRows/2; j++) {
        k = is.nWheelRows-1-j;
        l = wheel[i][j]; wheel[i][j] = wheel[i][k]; wheel[i][k] = l;
      }
  if (!us.fWheelReverse)
    for (i = 3; i < 9; i++)
      for (j = 0; j < is.nWheelRows/2; j++) {
        k = is.nWheelRows-1-j;
        l = wheel[i][j]; wheel[i][j] = wheel[i][k]; wheel[i][k] = l;
      }

  // Here we actually print the wheel and the objects in it.

  PrintCh2(chNW); PrintTab2(chH, WHEELCOLS-8); PrintHouse(11, fTrue);
  PrintTab2(chH, WHEELCOLS-11+us.fIndian); PrintHouse(10, fTrue);
  PrintTab2(chH, WHEELCOLS-10+us.fIndian); PrintHouse(9, fTrue);
  PrintTab2(chH, wheelcols-4); PrintCh2(chNE); PrintL();
  for (i = 0; i < is.nWheelRows; i++) {
    for (j = 10; j >= 7; j--) {
      PrintCh2(chV); PrintWheelSlot(wheel[j][i]);
    }
    PrintCh2(chV); PrintL();
  }
  PrintHouse(12, fTrue); PrintTab2(chH, WHEELCOLS-11);
  PrintCh2(chC); PrintTab2(chH, wheelcols-1); PrintCh2(chJN);
  PrintTab2(chH, wheelcols-1); PrintCh2(chC); PrintTab2(chH, WHEELCOLS-10);
  PrintHouse(8, fFalse); PrintL();
  for (i = 0; i < is.nWheelRows; i++) {
    PrintCh2(chV); PrintWheelSlot(wheel[11][i]); PrintCh2(chV);
    PrintWheelCenter(i);
    PrintCh2(chV); PrintWheelSlot(wheel[6][i]);
    PrintCh2(chV); PrintL();
  }
  PrintHouse(1, fTrue); PrintTab2(chH, WHEELCOLS-10-us.fIndian);
  PrintCh2(chJW); PrintWheelCenter(is.nWheelRows); PrintCh2(chJE);
  PrintTab2(chH, WHEELCOLS-10); PrintHouse(7, fFalse); PrintL();
  for (i = 0; i < is.nWheelRows; i++) {
    PrintCh2(chV); PrintWheelSlot(wheel[0][i]); PrintCh2(chV);
    PrintWheelCenter(is.nWheelRows+1 + i);
    PrintCh2(chV); PrintWheelSlot(wheel[5][i]);
    PrintCh2(chV); PrintL();
  }
  PrintHouse(2, fTrue); PrintTab2(chH, WHEELCOLS-10-us.fIndian);
  PrintCh2(chC); PrintTab2(chH, wheelcols-1); PrintCh2(chJS);
  PrintTab2(chH, wheelcols-1); PrintCh2(chC);
  PrintTab2(chH, WHEELCOLS-10); PrintHouse(6, fFalse); PrintL();
  for (i = 0; i < is.nWheelRows; i++) {
    for (j = 1; j <= 4; j++) {
      PrintCh2(chV); PrintWheelSlot(wheel[j][i]);
    }
    PrintCh2(chV); PrintL();
  }
  PrintCh2(chSW); PrintTab2(chH, wheelcols-4); PrintHouse(3, fFalse);
  PrintTab2(chH, WHEELCOLS-10); PrintHouse(4, fFalse);
  PrintTab2(chH, WHEELCOLS-10); PrintHouse(5, fFalse);
  PrintTab2(chH, WHEELCOLS-7); PrintCh2(chSE); PrintL();

#ifdef WIN
  if (gs.nFontTxt > 0)
    us.fAnsiChar = nSav;
#endif
}


// This is a subprocedure of ChartAspect() and ChartAspectRelation(). Display
// summary information about the aspect list, i.e. the total number of aspects
// of each type, and the number of aspects to each object, as done when the
// -a0 aspect summary setting is set.

void PrintAspectSummary(int *ca, int *co, int count, real rPowSum)
{
  char sz[cchSzDef];
  int i, j, k;

  if (count == 0) {
    PrintSz("No aspects in list.\n");
    return;
  }
  if (!us.fAspSummary)
    return;
  PrintL();
  sprintf(sz, "Sum power: %.2f - Average power: %.2f\n",
    rPowSum, rPowSum/(real)count); PrintSz(sz);
  k = us.fParallel ? Min(us.nAsp, aOpp) : us.nAsp;
  for (j = 0, i = 1; i <= k; i++) if (!FIgnoreA(i)) {
    if (!(j & 7)) {
      if (j)
        PrintL();
    } else
      PrintSz("   ");
    AnsiColor(kAspA[i]);
    sprintf(sz, "%s:%3d", SzAspectAbbrev(i), ca[i]); PrintSz(sz);
    j++;
  }
  PrintL();
  for (j = 0, i = 0; i <= is.nObj; i++) if (!FIgnore(i)) {
    if (!(j & 7)) {
      if (j)
        PrintL();
    } else
      PrintSz("   ");
    AnsiColor(kObjA[i]);
    sprintf(sz, "%.3s:%3d", szObjDisp[i], co[i]); PrintSz(sz);
    j++;
  }
  PrintL();
  AnsiColor(kDefault);
}


// Display all aspects between objects in the chart, one per line, in sorted
// order based on the total "power" of the aspect, as specified with the -a
// switch. The same influences used for -j charts are used here. This is
// almost the same as the -D list, except influences are different.

void ChartAspect(void)
{
  int ca[cAspect + 1], co[objMax];
  char sz[cchSzDef], *pch;
  int icut, jcut, ihi, jhi, ahi, i0, j0, i, j, k, count = 0, nSav;
  real ip, jp, vcut = (real)nLarge, vhi, phi, v, p, rPowSum = 0.0, rT;
  flag fDistance = us.fDistance && !us.fParallel;

  ClearB((pbyte)ca, sizeof(ca));
  ClearB((pbyte)co, sizeof(co));

  loop {
    vhi = -(real)nLarge;

    // Search for the next most powerful aspect in the aspect grid.

    for (i0 = 0; i0 <= is.nObj; i0++) {
      i = rgobjList[i0];
      if (FIgnore(i))
        continue;
      for (j0 = 0; j0 <= is.nObj; j0++) {
        j = rgobjList[j0];
        if (j >= i || FIgnore(j))
          continue;
        k = grid->n[j][i];
        if (k > 0) {
          ip = RObjInf(i);
          jp = RObjInf(j);
          p = rAspInf[k] * (ip+jp)/2.0 *
            (1.0-RAbs(grid->v[j][i])/GetOrb(i, j, k));
#ifdef EXPRESS
          // Adjust power with AstroExpression if one set.
          if (!us.fExpOff && FSzSet(us.szExpAspList)) {
            ExpSetN(iLetterW, j);
            ExpSetN(iLetterX, k);
            ExpSetN(iLetterY, i);
            ExpSetR(iLetterZ, p);
            ParseExpression(us.szExpAspList);
            p = RExpGet(iLetterZ);
          }
#endif
          switch (us.nAspectSort) {
          default:  v = p;                    break;
          case aso: v = -RAbs(grid->v[j][i]); break;
          case asn: v = -grid->v[j][i];       break;
          case asO: v = -(real)(Min(j0,i0)*cObj + Max(i0,j0)); break;
          case asP: v = -(real)(Max(i0,j0)*cObj + Min(j0,i0)); break;
          case asA: v = -(real)(k*cObj*cObj + j*cObj + i);     break;
          case asC: v = -planet[j];           break;
          case asD: v = -planet[i];           break;
          case asM: v = -Midpoint(planet[j], planet[i]);       break;
          }
          if ((v < vcut || (v == vcut && (i0 > icut ||
            (i0 == icut && j0 > jcut)))) && v > vhi) {
            vhi = v; ihi = i0; jhi = j0; ahi = k; phi = p;
          }
        }
      }
    }
    if (vhi <= -(real)nLarge)    // Exit when no less powerful aspect found.
      break;
    vcut = vhi; icut = ihi; jcut = jhi;
    i = rgobjList[ihi]; j = rgobjList[jhi];
    count++;                                 // Display the current aspect.
    rPowSum += phi;
    ca[ahi]++;
    co[j]++; co[i]++;
#ifdef INTERPRET
    if (us.fInterpret) {                     // Interpret it if -I in effect.
      InterpretAspect(j, i);
      AnsiColor(kDefault);
      continue;
    }
#endif
    sprintf(sz, "%3d: ", count); PrintSz(sz);
    PrintAspect(i, planetval(i), planetdir(i), ahi,
      j, planetval(j), planetdir(j), 'a');
    rT = grid->v[j][i];
    AnsiColor(rT < 0.0 ? kWhiteA : kLtGrayA);
    if (fDistance) {
      nSav = us.nDegForm; us.nDegForm = df360;
    }
    sprintf(sz, " - orb: %c%s", rgchAppSep[us.nAppSep*2 + (rT >= 0.0)],
      SzDegree2(RAbs(rT)));
    if (fDistance) {
      us.nDegForm = nSav;
      for (pch = sz; *pch; pch++)
        ;
      pch[-1] = '%';
    }
    PrintSz(sz);
    AnsiColor(kDkGreenA);
    PrintSz(" - power: ");
    sprintf(sz, us.fSeconds ? "%7.4f" : "%5.2f", phi);
    PrintSz(sz);
    PrintInDayEvent(j, ahi, i, -1);
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


// Display all aspects in effect from a given degree point to all planets in
// the main chart. Called from ChartMidpoint() and DisplayArabic() to do
// aspects to computed midpoints or aspects to Arabic part positions.

void PrintAspectsToPoint(real deg, int obj, real rRet, char *szWhat)
{
  char sz[cchSzDef];
  int i, j, k, asp;
  real rAng, rOrb;

  for (k = 0; k <= is.nObj; k++) {
    i = rgobjList[k];
    if (FIgnore(i))
      continue;
    rAng = MinDistance(planet[i], deg);
    for (asp = us.nAsp; asp >= 1; asp--) {
      if (!FAcceptAspect(i, asp, obj >= 0 ? obj : i))
        continue;
      rOrb = rAng - rAspAngle[asp];
      if (RAbs(rOrb) < GetOrb(i, obj >= 0 ? obj : i, asp)) {
        if (us.nAppSep == 1)
          rOrb *= RSgn2(rRet - ret[i]) * RSgn2(MinDifference(planet[i], deg));
        else if (us.nAppSep == 2)
          rOrb = RAbs(rOrb) *
            RSgn2(ret[i] - rRet) * RSgn2(MinDifference(planet[i], deg));
#ifdef EXPRESS
        // May want to skip current aspect if AstroExpression says to do so.
        if (!us.fExpOff && FSzSet(us.szExpMidAsp)) {
          ExpSetR(iLetterV, deg);
          ExpSetR(iLetterW, rRet);
          ExpSetN(iLetterX, asp);
          ExpSetN(iLetterY, k);
          ExpSetR(iLetterZ, rOrb);
          if (!NParseExpression(us.szExpMidAsp))
            continue;
        }
#endif
        sprintf(sz, "      %s ", szWhat); PrintSz(sz);
        PrintZodiac(deg);
        PrintSz(" makes ");
        AnsiColor(kAspA[asp]); PrintSz(szAspectAbbrevDisp[asp]);
        AnsiColor(kDefault); PrintSz(" to ");
        AnsiColor(kObjA[i]); sprintf(sz, "%.10s ", szObjDisp[i]); PrintSz(sz);
        PrintTab(' ', 10-CchSz(szObjDisp[i]));
        j = (int)(rOrb * 3600.0);
        AnsiColor(j < 0.0 ? kWhiteA : kLtGrayA);
        sprintf(sz, "- orb: %c%d%c%02d'", rgchAppSep[us.nAppSep*2 + (j >= 0)],
          NAbs(j)/3600, chDegC, NAbs(j)%3600/60); PrintSz(sz);
        if (us.fSeconds) {
          sprintf(sz, "%02d\"", NAbs(j)%60); PrintSz(sz);
        }
        PrintL();
        AnsiColor(kDefault);
      }
    }
  }
}


// This is a subprocedure of ChartMidpoint() and ChartMidpointRelation().
// Display summary information about the midpoint list, i.e. the total number
// of midpoints in each sign, and their average span in degrees, as done when
// the -m0 midpoint summary setting is set.

void PrintMidpointSummary(int *cs, int count, real rSpanSum)
{
  char sz[cchSzDef];
  int i;

  if (count == 0) {
    PrintSz("No midpoints in list.\n");
    return;
  }
  if (!us.fMidSummary)
    return;
  PrintL();
  PrintSz("Average span: ");
  PrintSz(SzDegree(rSpanSum / (real)count));
  PrintL();
  for (i = 1; i <= cSign; i++) {
    if (i == sLib)
      PrintL();
    else if (i != sAri)
      PrintSz("   ");
    AnsiColor(kSignA(i));
    sprintf(sz, "%.3s:%3d", szSignName[i], cs[i]); PrintSz(sz);
  }
  PrintL();
  AnsiColor(kDefault);
}


// Display locations of all midpoints between objects in the chart, one per
// line, in sorted zodiac order from zero Aries onward, as specified with the
// -m switch.

void ChartMidpoint(void)
{
  int cs[cSign + 1];
  char sz[cchSzDef];
  int icut, jcut, ilo, jlo, i, j, count = 0;
  real rSpanSum = 0.0, mcut = -1.0, mlo, m, mid, dist, midalt;

  ClearB((pbyte)cs, sizeof(cs));
  loop {
    mlo = rDegMax;

    // Search for the next closest midpoint farther down in the zodiac.

    for (i = 0; i < is.nObj; i++) if (!FIgnore(i))
      for (j = i+1; j <= is.nObj; j++) if (!FIgnore(j)) {
        m = ZFromS(grid->n[j][i]) + grid->v[j][i];
        if ((m > mcut || (m == mcut && (i > icut ||
          (i == icut && j > jcut)))) && m < mlo) {
          ilo = i; jlo = j; mlo = m;
        }
      }
    if (mlo >= rDegMax)  // Exit when no midpoint farther in zodiac found.
      break;
    mcut = mlo; icut = ilo; jcut = jlo;
    if (us.objRequire >= 0 && ilo != us.objRequire && jlo != us.objRequire)
      continue;
    if (!us.fHouse3D) {
      mid = Midpoint(planet[ilo], planet[jlo]);
      midalt = (planetalt[ilo] + planetalt[jlo]) / 2.0;
      dist = MinDistance(planet[ilo], planet[jlo]);
    } else {
      SphRatio(planet[ilo], planetalt[ilo], planet[jlo], planetalt[jlo], 0.5,
        &mid, &midalt);
      dist = SphDistance(planet[ilo], planetalt[ilo],
        planet[jlo], planetalt[jlo]);
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
    cs[SFromZ(mlo)]++;
    rSpanSum += dist;
#ifdef INTERPRET
    if (us.fInterpret) {                   // Interpret it if -I in effect.
      InterpretMidpoint(ilo, jlo);
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
    PrintAspect(ilo, planet[ilo], ret[ilo], 0, jlo, planet[jlo], ret[jlo],
      'm');
    AnsiColor(kDefault);
    PrintSz(" - ");
    PrintSz(SzDegree(dist));
    if (us.fParallel && !us.fHouse3D) {
      PrintCh(' ');
      PrintSz(SzDegree(RAbs(planetalt[ilo] - planetalt[jlo])));
    }
    PrintSz(" degree span.\n");

    // If the -ma switch is set, determine and display each aspect from one of
    // the planets to the current midpoint, and the aspect's orb.

    if (us.fMidAspect)
      PrintAspectsToPoint(mid, ilo, (ret[ilo]+ret[jlo])/2.0, "Midpoint");
  }

  PrintMidpointSummary(cs, count, rSpanSum);
}


// This is subprocedure of ChartHorizon(). Print out one line of data given a
// pre-computed azimuth and altitude.

void PrintHorizonLine(real azi, real alt,
  real aziSun, real altSun, real aziMoo, real altMoo, int hm)
{
  char sz[cchSzDef], szFormat[cchSzDef];
  real rAzi, sx, sy, vx, vy;
  int j;

  PrintAltitude(alt);

  // Determine directional vector based on azimuth.

  if (!us.fHouse3D) {
    rAzi = Mod(azi * (us.nHorizon >= 4 ? -1.0 : 1.0) +
      (real)(((us.nHorizon & 3) + 4) * 90));
    sprintf(sz, " %s", SzDegree(rAzi));
  } else {
    sprintf(sz, " %2d%s", SFromZ(azi),
      SzDegree(azi - (real)((SFromZ(azi)-1)*30)));
  }
  PrintSz(sz);
  sx = RCosD(azi); sy = RSinD(azi);
  if (RAbs(sx) < RAbs(sy)) {
    vx = RAbs(sx / sy); vy = 1.0;
  } else {
    vy = RAbs(sy / sx); vx = 1.0;
  }
  sprintf(sz, us.fSeconds ? " (%.3f%c" : " (%.2f%c", vy, sy < 0.0 ?
    (hm != hmHorizon ? 'u' : 's') : (hm != hmHorizon ? 'd' : 'n'));
  PrintSz(sz);
  sprintf(sz, us.fSeconds ? " %.3f%c)" : " %.2f%c)", vx,
    sx > 0.0 ? 'e' : 'w');
  PrintSz(sz);

  if (us.fListDecan) {
    // Determine distance vector of current object from Sun and Moon.

    vx = aziSun - azi;
    vy = aziMoo - azi;
    j = 1 + us.fSeconds;
    sprintf(szFormat, " [%%%d.%df%%%d.%df] [%%%d.%df%%%d.%df]",
      j+5, j, j+5, j, j+5, j, j+5, j);
    sprintf(sz, szFormat,
      RAbs(vx) < rDegHalf ? vx : RSgn(vx) * (rDegMax - RAbs(vx)),
      altSun - alt,
      RAbs(vy) < rDegHalf ? vy : RSgn(vy) * (rDegMax - RAbs(vy)),
      altMoo - alt);
    PrintSz(sz);
  }
}


// Display locations of the objects on the screen with respect to the local
// horizon, as specified with the -Z switch.

void ChartHorizon(void)
{
  char sz[cchSzDef], szFormat[cchSzDef];
  real azi[objMax], alt[objMax], aziSun, altSun, aziMoo, altMoo,
    rPhase, rDiam, rMag, radi, len;
  int hm = us.fPrimeVert || us.fHouse3D ? us.nHouse3D : hmHorizon, i, k;
  flag fSav;

  for (i = 0; i <= is.nObj; i++) if (!ignore[i]) {
    // First find zenith location on Earth of each object.
    // Then, convert this to local horizon altitude and azimuth.

    azi[i] = Tropical(planet[i]); alt[i] = planetalt[i];
    EclToEqu(&azi[i], &alt[i]);
    azi[i] = Mod(cp0.lonMC - azi[i] + rDegQuad);
    if (hm == hmHorizon) {
      EquToLocal(&azi[i], &alt[i], rDegQuad - Lat);
      if (us.fRefract)
        alt[i] = SwissRefract(alt[i]);
    } else if (hm == hmPrime)
      EquToLocal(&azi[i], &alt[i], -Lat);
    azi[i] = rDegMax - azi[i];

    // 3D Campanus cusps and such should always be at particular degrees.
    if (FCusp(i) &&
      ((hm == hmPrime && us.nHouseSystem == hsCampanus) ||
      (hm == hmHorizon && us.nHouseSystem == hsHorizon &&
        (us.fHouseAngle || (i != oAsc && i != oDes))) ||
      (hm == hmEquator && us.nHouseSystem == hsMeridian &&
        (us.fHouseAngle || (i != oAsc && i != oDes)))))
      azi[i] = (real)((i - oAsc) * 30);
    else if (i == oVtx && hm == hmHorizon && FNearR(chouse[7], planet[oVtx]))
      azi[i] = (real)((sLib - 1) * 30);
    else if (i == oEP && hm == hmEquator && FNearR(chouse[1], planet[oEP]))
      azi[i] = (real)((sAri - 1) * 30);
  }
  aziSun = (!ignore[oSun] ? azi[oSun] : 0.0);
  altSun = (!ignore[oSun] ? alt[oSun] : 0.0);
  aziMoo = (!ignore[oMoo] ? azi[oMoo] : 0.0);
  altMoo = (!ignore[oMoo] ? alt[oMoo] : 0.0);

  // Now, actually print the location of each object.

  sprintf(szFormat, us.fSeconds ? " " : "");
  sprintf(sz, "\nBody %s%sAltitude", szFormat, szFormat); PrintSz(sz);
  PrintTab(' ', VSeconds(1, 4, 8));
  PrintSz("Azimuth");
  PrintTab(' ', VSeconds(2, 4, 8) + us.fHouse3D*2);
  sprintf(sz, "Azi. Vector%s", szFormat); PrintSz(sz);
  if (us.fListDecan) {
    sprintf(sz, "    %s Vector%s%s    Moon Vector",
      us.objCenter != oSun ? "Sun" : "Earth", szFormat, szFormat);
    PrintSz(sz);
  }
  PrintTab(' ', !us.fListDecan ? 3 : (us.fSeconds ? 6 : 4));
  sprintf(sz, "Magn. Full(Area,Time)  Diameter\n"); PrintSz(sz);
  for (k = 0; k <= is.nObj; k++) {
    i = rgobjList[k];
    if (ignore[i])
      continue;
    AnsiColor(kObjA[i]);
    sprintf(sz, "%-4.4s: ", szObjDisp[i]); PrintSz(sz);
    PrintHorizonLine(azi[i], alt[i], aziSun, altSun, aziMoo, altMoo, hm);

    // Determine magnitude, phase, and angular diameter.

    rMag = rPhase = rDiam = rInvalid;
    if (FStar(i)) {
      rMag = rStarBright[i-oNorm];
      rPhase = 1.0;
#ifdef SWISS
    } else if (FCmSwissAny() && us.objCenter == oEar &&
      FSwissPlanetData(is.T, i, &rPhase, &rDiam, &rMag)) {
      // Swiss Ephemeris can calculate all three of these at once.
      if (i == oEar)
        rMag = rInvalid;
      else if (us.fStarMagDist)
        rMag = RStarBright(rMag, cp0.dist[i], 1.0);
      if (rDiam == 0.0)
        rDiam = rInvalid;
#endif
    } else {
      // Determine phase of body, or how much it's covered in shadow.
      if (FThing(i)) {
        radi = RAngleD(space[oSun].x-space[i].x, space[oSun].y-space[i].y);
        len = RAngleD(space[i].x, space[i].y);
        radi = rDegHalf - MinDistance(radi, len);
        rPhase = RCosD(radi) / 2.0 + 0.5;
      }
      // Determine angular diameter covered by body in sky.
      radi = RObjDiam(i) / 2.0;
      if (radi > 0.0) {
        len = PtLen(space[i]) * rAUToKm;
        rDiam = RAtnD(radi / len) * 2.0;
      }
    }
    if (rDiam >= 100.0)
      rDiam = rInvalid;
    if (i == oSun)
      rPhase = 1.0;
    else if (i == us.objCenter)
      rPhase = (i == oEar ? (rDegQuad + alt[oSun]) / rDegHalf : rInvalid);
    if (rMag != rInvalid) {
      sprintf(sz, " %6.2f", rMag); PrintSz(sz);
    } else if (rPhase != rInvalid || rDiam != rInvalid)
      PrintSz(" ______");
    if (rPhase != rInvalid) {
      sprintf(sz, " %6.2f%% %6.2f%%", rPhase * 100.0,
        100.0 - RAcosD((rPhase - 0.5) * 2.0) / rDegHalf * 100.0);
      PrintSz(sz);
    } else if (rDiam != rInvalid)
      PrintSz(" _______ _______");
    if (rDiam != rInvalid) {
      if (!us.fSeconds) {
        fSav = us.fSeconds; us.fSeconds = fTrue;
        sprintf(sz, "%s", SzDegree(rDiam));
        us.fSeconds = fSav;
      } else {
        fSav = us.fSecond1K; us.fSecond1K = fTrue;
        sprintf(sz, "%s", SzDegree(rDiam));
        us.fSecond1K = fSav;
      }
      PrintSz(sz);
    }
    if (i >= starLo) {
      sprintf(sz, "  Star #%2d", i-starLo+1); PrintSz(sz);
    }
    PrintL();
  }

#ifdef SWISSGRAPH
  ES es;
  real aziT, altT;

  // Print extra stars and extra asteroids.
  for (k = 0; k <= 1; k++) {
    if (!(!k ? gs.fAllStar : gs.nAstLo > 0))
      continue;
    PrintL();
    if (!k)
      SwissComputeStarSort(is.T, NULL);
    else
      SwissComputeAsteroidSort(is.T, NULL);
    for (i = 1; !k ? SwissComputeStarSort(is.T, &es) :
      SwissComputeAsteroidSort(is.T, &es); i++) {

      // First find zenith location on Earth of each object.
      // Then, convert this to local horizon altitude and azimuth.
      aziT = Tropical(es.lon); altT = es.lat;
      EclToEqu(&aziT, &altT);
      aziT = Mod(cp0.lonMC - aziT + rDegQuad);
      if (hm == hmHorizon) {
        EquToLocal(&aziT, &altT, rDegQuad - Lat);
        if (us.fRefract)
          altT = SwissRefract(altT);
      } else if (hm == hmPrime)
        EquToLocal(&aziT, &altT, -Lat);
      aziT = rDegMax - aziT;

      // Now, actually print the location of each object.
      if (!k) {
        AnsiColor(es.ki != kDefault ? es.ki : KStar2A(es.mag));
        if (es.mag == rStarNot)
          es.mag = 99.99;
        sprintf(sz, "%4d %-8.8s ", i, es.pchDes); PrintSz(sz);
      } else {
        AnsiColor(es.ki != kDefault ? es.ki : kDefault);
        sprintf(sz, "%6d ", i); PrintSz(sz);
      }
      PrintHorizonLine(aziT, altT, aziSun, altSun, aziMoo, altMoo, hm);
      if (!k) {
        sprintf(sz, " %5.2f%s%s\n", es.mag, *es.pchNam ? "  " : "",
          es.pchNam); PrintSz(sz);
      } else {
        sprintf(sz, "  %s\n", es.sz); PrintSz(sz);
      }
    }
  }
#endif
  AnsiColor(kDefault);
}


// Display x,y,z locations of each body (in AU) with respect to the Sun (or
// whatever the specified center planet is), as in the -S switch. These values
// were already determined when calculating the planet positions themselves,
// so this procedure is basically just a loop.

void ChartOrbit(void)
{
  char sz[cchSzMax], sz1[cchSzDef];
  real x, y, z, rT;
  int i, j;

  sprintf(sz1, us.fSeconds ? " " : "");
  sprintf(sz, "Body   %s%sAzimuth", sz1, sz1); PrintSz(sz);
  PrintTab(' ', VSeconds(3, 5, 9));
  sprintf(sz,
    "%s%sX axis%s%s   %s%sY axis%s%s   %s%sZ axis%s%s   %s%sLength\n",
    sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1);
  PrintSz(sz);
  for (j = 0; j <= is.nObj; j++) {
    i = rgobjList[j];
    if (ignore[i] || (!FThing(i) ||
      ((i == oMoo || i == oNod || i == oSou) && !us.fEphemFiles)))
      continue;
    AnsiColor(kObjA[i]);
    sprintf(sz, "%-4.4s:  ", szObjDisp[i]); PrintSz(sz);
    x = space[i].x; y = space[i].y; z = space[i].z;
    rT = cp0.dist[i];
    if (FStar(i)) {
      // Star distances are in light years, not AU.
      x /= rLYToAU; y /= rLYToAU; z /= rLYToAU;
      rT /= rLYToAU;
    } else if (us.fEuroDist)
      rT *= rAUToKm;    
    PrintZodiac(planet[i]);
    AnsiColor(kObjA[i]);
    sprintf(sz, us.fSeconds ? " %12.7f %12.7f %12.7f %12.7f" :
      " %8.3f %8.3f %8.3f %8.3f", x, y, z, rT);
    PrintSz(sz);
    if (!us.fSeconds) {
      if (FBetween(i, uranLo+1, uranHi)) {
        sprintf(sz, "  Uranian #%d", i-uranLo); PrintSz(sz);
      } else if (FBetween(i, dwarfLo+2, dwarfHi)) {
        sprintf(sz, "  Dwarf #%d", i-dwarfLo-1); PrintSz(sz);
      } else if (FMoons(i)) {
        sprintf(sz, "  Moon #%d", i-moonsLo+1); PrintSz(sz);
      } else if (FCob(i)) {
        sprintf(sz, "  COB #%d", i-cobLo+1); PrintSz(sz);
      } else if (FStar(i)) {
        sprintf(sz, "  Star #%2d", i-oNorm); PrintSz(sz);
      }
    }
    if (FStar(i)) {
      sprintf(sz, " %5.2f", rStarBright[j-oNorm]); PrintSz(sz);
    }
    PrintL();
  }
  AnsiColor(kDefault);

#ifdef SWISSGRAPH
  ES es;

  // Print extra stars.
  if (gs.fAllStar) {
    PrintL();
    SwissComputeStarSort(is.T, NULL);
    for (i = 1; SwissComputeStarSort(is.T, &es); i++) {
      AnsiColor(es.ki != kDefault ? es.ki : KStar2A(es.mag));
      if (es.mag == rStarNot)
        es.mag = 99.99;
      sprintf(sz, "%4d %-8.8s ", i, es.pchDes); PrintSz(sz);
      PrintZodiac(es.lon);
      AnsiColor(es.ki != kDefault ? es.ki : KStar2A(es.mag));
      x = es.pt.x / rLYToAU;  // Star distances are in light years, not AU.
      y = es.pt.y / rLYToAU;
      z = es.pt.z / rLYToAU;
      sprintf(sz, us.fSeconds ? " %12.7f %12.7f %12.7f %12.7f" :
        " %8.3f %8.3f %8.3f %8.3f", x, y, z, RLength3(x, y, z)); PrintSz(sz);
      sprintf(sz, " %5.2f%s%s\n", es.mag, *es.pchNam ? "  " : "", es.pchNam);
      PrintSz(sz);
    }
  }

  // Print extra asteroids.
  if (gs.nAstLo > 0) {
    PrintL();
    SwissComputeAsteroidSort(is.T, NULL);
    for (i = gs.nAstLo; SwissComputeAsteroidSort(is.T, &es); i++) {
      AnsiColor(es.ki != kDefault ? es.ki : kDefault);
      sprintf(sz, "%6d ", i); PrintSz(sz);
      PrintZodiac(es.lon);
      x = es.pt.x;
      y = es.pt.y;
      z = es.pt.z;
      sprintf(sz, us.fSeconds ? " %12.7f %12.7f %12.7f %12.7f" :
        " %8.3f %8.3f %8.3f %8.3f", x, y, z, RLength3(x, y, z)); PrintSz(sz);
      sprintf(sz, "  %s\n", es.sz); PrintSz(sz);
    }
  }
  AnsiColor(kDefault);
#endif
}


// Display a chart showing esoteric astrology information, as in the -7
// switch. This includes Rays, esoteric rulerships, and Ray influences.

int ChartEsoteric(flag fGetRay)
{
  char sz[cchSzDef], *pch;
  int nRay1[cRay+1], nRay2[cRay+1], rank1[cRay+1], rank2[cRay+1],
    rank[objMax], i, j, k, n, c, n1 = 0, n2 = 0;
  real power1[objMax], power2[objMax], rRay1[cRay+1], rRay2[cRay+1],
    power[objMax], r = 0.0, r1 = 0.0, r2 = 0.0;
  flag fIgnore7Sav[rrMax], fRayRuler;

  EnsureRay();
#ifdef INTERPRET
  if (us.fInterpret && !fGetRay) {    // Print interpretation if -I in effect.
    InterpretEsoteric(fFalse);
    return 0;
  }
#endif

  // Calculate data.
  for (i = 0; i <= cRay; i++) {
    nRay1[i] = nRay2[i] = i ? 0   : -1;
    rRay1[i] = rRay2[i] = i ? 0.0 : -1.0;
  }
  ComputeInfluence(power1, power2);
  for (i = 0; i <= is.nObj; i++) {
    if (FIgnore(i))
      continue;
    power[i] = power1[i] + power2[i];
    r += power[i];
    n = SFromZ(planet[i]);
    for (j = 1; j <= cRay; j++) {
      c = rgSignRay2[n][j];
      if (c) {
        nRay1[j]++;
        nRay2[j] += c;
        rRay1[j] += power[i];
        rRay2[j] += power[i] / (real)(420 / rgSignRay2[n][j]);
      }
    }
  }
  SortRank(power, rank, is.nObj, fTrue);
  SortRank(rRay1, rank1, cRay, fFalse); SortRank(rRay2, rank2, cRay, fFalse);
  if (fGetRay) {
    // If only returning the top Ray(s), do so now.
    n = 0;
    for (i = 1; i <= cRay; i++) {
      if (rank1[i] == 1)
        n += i*10;
      else if (rank1[i] == 2)
        n += i;
    }
    return n;
  }

  // Print planet table.
  for (i = 0; i < rrMax; i++) {
    fIgnore7Sav[i] = ignore7[i];
    ignore7[i] = fFalse;
  }
  PrintSz("Body    Location Rulers House Rulers Power Rank Percent\n");
  for (k = 0; k <= is.nObj; k++) {
    i = rgobjList[k];
    if (FIgnore(i))
      continue;
    AnsiColor(kObjA[i]);
    sprintf(sz, "%-4.4s", szObjDisp[i]); PrintSz(sz);
    n = (i <= oNorm ? rgObjRay[i] : 0);
    if (n) {
      AnsiColor(kRayA[n]);
      sprintf(sz, " %d", n); PrintSz(sz);
    } else
      PrintSz("  ");
    PrintSz("  ");
    n = SFromZ(planet[i]);
    AnsiColor(kSignA(n));
    sprintf(sz, "%.3s ", szSignName[n]); PrintSz(sz);
    for (j = 1; j <= cRay; j++)
      if (rgSignRay2[n][j]) {
        AnsiColor(kRayA[j]);
        sprintf(sz, "%d", j);
        PrintSz(sz);
      }
    AnsiColor(kObjA[i]);
    c = 0;
    for (j = 1; j <= cRay; j++)
      c += !rgSignRay2[n][j];
    PrintTab(' ', c - 3);
    pch = Dignify(i, n)+1;
    fRayRuler = (pch[rrRay] == 'Y');
    sprintf(sz, " %s", pch); PrintSz(sz);
    n = inhouse[i];
    pch = Dignify(i, n)+1;
    fRayRuler &= (pch[rrRay] == 'Y');
    PrintSz(fRayRuler ? "2" : " ");
    AnsiColor(kSignA(n));
    sprintf(sz, "%4d%s %s", n, szSuffix[n], pch);
    PrintSz(sz);
    PrintSz(fRayRuler ? "2" : " ");
    AnsiColor(kObjA[i]);
    sprintf(sz, "%6.1f (%2d) /%5.1f%%\n", power[i], rank[i],
      power[i]/r*100.0); PrintSz(sz);
  }
  AnsiColor(kDefault);
  sprintf(sz, "Total            RSHXY        RSHXY%7.1f       100.0%%\n", r);
  PrintSz(sz);
  for (i = 0; i < rrMax; i++)
    ignore7[i] = fIgnore7Sav[i];

  // Print Ray table.
  for (i = 1; i <= cRay; i++) {
    n1 += nRay1[i]; n2 += nRay2[i];
    r1 += rRay1[i]; r2 += rRay2[i];
  }
  if (r1 == 0.0)
    r1 = 1.0;
  if (r2 == 0.0)
    r2 = 1.0;
  PrintL();
  sprintf(sz, "Ray Count  Power Rank  Perc. %c Slice  Power Rank  Perc.\n",
    chV);
  PrintSz(sz);
  for (i = 1; i <= cRay; i++) {
    AnsiColor(kRayA[i]);
    sprintf(sz, "%d:%7d%7.1f (%d) /%5.1f%% %c%6.2f%7.1f (%d) /%5.1f%%", i,
      nRay1[i], rRay1[i], rank1[i], rRay1[i]/r1*100.0, chV,
      (real)nRay2[i] / 420.0, rRay2[i], rank2[i], rRay2[i]/r2*100.0);
    PrintSz(sz);
    PrintL();
  }
  AnsiColor(kDefault);
  sprintf(sz, "Tot:%5d%7.1f      100.0%% %c%6.2f%7.1f      100.0%%\n",
    n1, r1, chV, (real)n2 / 420.0, r2);
  PrintSz(sz);
  return 0;
}


// Display locations of the planets on the screen with respect to the 36
// Gauquelin sectors and their plus zones, as specified with the -l switch.

void ChartSector(void)
{
  char sz[cchSzDef];
  CP cp;
  byte ignoreSav[objMax];
  int rgc[cSector + 1], i, j, sec, pls, kPls, cpls = 0, co = 0, cq = 0;
  real r, rT;

  for (i = 1; i <= cSector; i++) {
    rgc[i] = 0;
    cpls += pluszone[i];
  }
  cp = cp0;
  CopyRgb(ignore, ignoreSav, sizeof(ignore));
  CastSectors();

  AnsiColor(kDkGrayA);
  PrintSz("Body  Sector ");
  if (!us.fSeconds)
    PrintSz("Plus    House   Sign Ret. Latit. Veloc. 18 12\n");
  else {
    PrintSz("  Plus      House   Sign Loc.");
    PrintTab(' ', !us.fSecond1K ? 1 : 5);
    PrintSz("Ret. Latitude");
    PrintTab(' ', !us.fSecond1K ? 1 : 5);
    PrintSz("Velocity Sec.18 Sec.12\n");
  }
  for (j = 0; j <= is.nObj; j++) {
    i = rgobjList[j];
    if (FIgnore(i))
      continue;
    co++;
    AnsiColor(kObjA[i]);
    sprintf(sz, "%-4.4s: ", szObjDisp[i]); PrintSz(sz);
    r = GFromO(planet[i]);
    sec = (int)r + 1; rgc[sec]++;
    pls = pluszone[sec];
    cq += pls;
    kPls = (pls ? kRedA : kDkGreenA);
    AnsiColor(kDkGrayA);
    PrintSz("Sec");
    AnsiColor(kPls);
    sprintf(sz, " %2d", sec); PrintSz(sz);
    if (us.fSeconds) {
      AnsiColor(kDkGrayA);
      sprintf(sz, "%.3f", RFract(r)); PrintSz(&sz[1]);
      AnsiColor(kPls);
    }
    sprintf(sz, " %c", (char)(pls ? '+' : '-')); PrintSz(sz);
    AnsiColor(kSignA(cp.house[i]));
    sprintf(sz, " [%2d%s house] ", cp.house[i], szSuffix[cp.house[i]]);
    PrintSz(sz);
    PrintZodiac(cp.obj[i]);
    sprintf(sz, " %c ", ChRet(cp.dir[i])); PrintSz(sz);
    PrintAltitude(cp.alt[i]);
    PrintCh(' ');
    PrintCh(cp.dir[i] < 0.0 ? '-' : '+');
    rT = RAbs(cp.dir[i]);
    sprintf(sz, "%.5f", rT); sz[us.fSeconds ? 7 : 5] = chNull;
    PrintSz(sz);
    AnsiColor(kPls);
    sprintf(sz, " %2d", (sec-1)/2 + 1); PrintSz(sz);
    if (us.fSeconds) {
      AnsiColor(kDkGrayA);
      sprintf(sz, "%.3f", RFract(r/2.0)); PrintSz(&sz[1]);
    }
    AnsiColor(kPls);
    sprintf(sz, " %2d", (sec-1)/3 + 1); PrintSz(sz);
    if (us.fSeconds) {
      AnsiColor(kDkGrayA);
      sprintf(sz, "%.3f", RFract(r/3.0)); PrintSz(&sz[1]);
    }
    PrintL();
  }

  // Display summary information, i.e. the planet in plus zone ratio.

  AnsiColor(kDefault);
  sprintf(sz, "\nPlus zones: %d/%d = %.2f%% - ", cpls, cSector,
    (real)cpls/(real)36*100.0); PrintSz(sz);
  sprintf(sz, "Planets in plus zones: %d/%d = %.2f%%\n", cq, co,
    co ? (real)cq/(real)co*100.0 : 0.0); PrintSz(sz);

  // Display more summary information, i.e. the number of planets in each
  // sector, as well as whether each sector is a plus zone or not.

  PrintSz("\nZone:");
  for (i = 1; i <= cSector/2; i++) {
    pls = pluszone[i];
    AnsiColor(pls ? kRedA : kDkGreenA);
    sprintf(sz, " %2d%c", i, pls ? '+' : '-'); PrintSz(sz);
  }
  AnsiColor(kDefault);
  PrintSz("\nNum :");
  for (i = 1; i <= cSector/2; i++) {
    if (rgc[i]) {
      sprintf(sz, " %2d ", rgc[i]); PrintSz(sz);
    } else
      PrintSz("  . ");
  }
  PrintSz("\nZone:");
  for (i = cSector; i > cSector/2; i--) {
    pls = pluszone[i];
    AnsiColor(pls ? kRedA : kDkGreenA);
    sprintf(sz, " %2d%c", i, pls ? '+' : '-'); PrintSz(sz);
  }
  AnsiColor(kDefault);
  PrintSz("\nNum :");
  for (i = cSector; i > cSector/2; i--) {
    if (rgc[i]) {
      sprintf(sz, " %2d ", rgc[i]); PrintSz(sz);
    } else
      PrintSz("  . ");
  }
  PrintL();
  CopyRgb(ignoreSav, ignore, sizeof(ignore));
  CastChart(1);
}


// Print the locations of the astro-graph lines on the Earth as specified
// with the -L switch. This includes Midheaven and Nadir lines, zenith
// positions, and locations of Ascendant and Descendant lines.

flag ChartAstroGraph(void)
{
  CrossInfo *rgcr, *pcr, crT;
  char sz[cchSzDef];
  real planet1[objMax], planet2[objMax], mc[objMax], ic[objMax],
    asc[objMax], des[objMax], asc1[objMax], des1[objMax], rgad[objMax],
    lo = Lon, longm, w, x, y, z, ww, xx, yy, zz, ad, oa, am, od;
  int cCross = 0, i, j, k, l, m, n, o;
  flag fEdge;

  if (us.fLatitudeCross) {
    rgcr = RgAllocate(MAXCROSS, CrossInfo, "crossing table");
    if (rgcr == NULL)
      return fFalse;
    pcr = rgcr;
  }

  // Calculate zenith location on Earth of each object.

  for (i = 0; i <= is.nObj; i++) if (!ignore[i]) {
    planet1[i] = Tropical(planet[i]);
    planet2[i] = !us.fHouse3D ? planetalt[i] : 0.0;
    EclToEqu(&planet1[i], &planet2[i]);
  }

  // Print header.

  PrintSz("Object :");
  for (j = 0; j <= is.nObj; j++) {
    i = rgobjList[j];
    if (!ignore[i] && FThing2(i)) {
      AnsiColor(kObjA[i]);
      sprintf(sz, VSeconds(" %.3s", " %-10.10s", " %-14.14s"), szObjDisp[i]);
      PrintSz(sz);
    }
  }
  AnsiColor(kDefault);
  PrintSz("\n------ :");
  for (i = 0; i <= is.nObj; i++)
    if (!ignore[i] && FThing2(i)) {
      PrintCh(' ');
      PrintTab('#', VSeconds(3, 10, 14));
    }

  // Print the longitude locations of the Midheaven lines.

  PrintSz("\nMidheav: ");
  if (lo < 0.0)
    lo += rDegMax;
  for (j = 0; j <= is.nObj; j++) {
    i = rgobjList[j];
    if (!ignore[i] && FThing2(i)) {
      AnsiColor(kObjA[i]);
      x = cp0.lonMC - planet1[i];
      if (x < 0.0)
        x += rDegMax;
      if (x > rDegHalf)
        x -= rDegMax;
      z = lo + x;
      if (z > rDegHalf)
        z -= rDegMax;
      mc[i] = z;
      if (us.fSeconds) {
        sprintf(sz, "%s ", SzLocation(z, 0.0));
        sz[11 + us.fSecond1K*4] = chNull;
      } else {
        sprintf(sz, "%3.0f%c", RAbs(z), z < 0.0 ? 'e' : 'w');
      }
      PrintSz(sz);
    }
  }
  AnsiColor(kDefault);

  // The Nadir lines are just always 180 degrees away from the Midheaven.

  PrintSz("\nNadir  : ");
  for (j = 0; j <= is.nObj; j++) {
    i = rgobjList[j];
    if (!ignore[i] && FThing2(i)) {
      AnsiColor(kObjA[i]);
      z = mc[i] + rDegHalf;
      if (z > rDegHalf)
        z -= rDegMax;
      ic[i] = z;
      if (us.fSeconds) {
        sprintf(sz, "%s ", SzLocation(z, 0.0));
        sz[11 + us.fSecond1K*4] = chNull;
      } else {
        sprintf(sz, "%3.0f%c", RAbs(z), z < 0.0 ? 'e' : 'w');
      }
      PrintSz(sz);
    }
  }
  AnsiColor(kDefault);

  // Print the Zenith latitude locations.

  PrintSz("\nZenith : ");
  for (k = 0; k <= is.nObj; k++) {
    i = rgobjList[k];
    if (!ignore[i] && FThing2(i)) {
      AnsiColor(kObjA[i]);
      y = planet2[i];
      if (us.fSeconds) {
        sprintf(sz, " %s ", SzLocation(0.0, y));
        for (j = 1; sz[j] = sz[j+11 + us.fSecond1K*4]; j++)
          ;
      } else {
        sprintf(sz, "%3.0f%c", RAbs(y), y < 0.0 ? 's' : 'n');
      }
      PrintSz(sz);
    }
  }
  PrintL2();

  // Now print the locations of Ascendant and Descendant lines. Since these
  // are curvy, loop through the latitudes, and for each object at each
  // latitude, print the longitude location of the line in question.

  longm = Mod(cp0.lonMC + lo);
  for (k = 0; k <= is.nObj; k++)
    asc[k] = des[k] = rgad[k] = rLarge;
  for (j = 90-(90 % us.nAstroGraphStep); j >= -90; j -= us.nAstroGraphStep) {
    fEdge = (j >= 90 || j <= -90);
    if (!fEdge) {
      AnsiColor(kDefault);
      sprintf(sz, "Asc@%2d%c: ", NAbs(j), j < 0 ? 's' : 'n');
      PrintSz(sz);
    }
    for (k = 0; k <= is.nObj; k++) {
      i = rgobjList[k];
      if (!ignore[i] && FThing2(i)) {
        AnsiColor(kObjA[i]);
        asc1[i] = asc[i];
        if (fEdge)
          ad = rLarge;
        else
          ad = RTanD(planet2[i])*RTanD((real)j);
        if (ad*ad > 1.0) {
          asc[i] = rgad[i] = rLarge;
          if (fEdge)
            continue;
          PrintCh(' ');
          PrintTab('-', VSeconds(2, 8, 12));
          PrintTab(' ', 1 + us.fSeconds);
        } else {
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
          asc[i] = z;
          rgad[i] = ad;
          if (fEdge)
            continue;
          if (us.fSeconds) {
            sprintf(sz, "%s ", SzLocation(z, 0.0));
            sz[11 + us.fSecond1K*4] = chNull;
          } else
            sprintf(sz, "%3.0f%c", RAbs(z), z < 0.0 ? 'e' : 'w');
          PrintSz(sz);
        }
      }
    }

    // Again, the Descendant position is related to the Ascendant's, being a
    // mirror image, so it can be calculated somewhat easier.

    if (!fEdge) {
      AnsiColor(kDefault);
      sprintf(sz, "\nDsc@%2d%c: ", NAbs(j), j < 0 ? 's' : 'n');
      PrintSz(sz);
    }
    for (k = 0; k <= is.nObj; k++) {
      i = rgobjList[k];
      if (!ignore[i] && FThing2(i)) {
        AnsiColor(kObjA[i]);
        des1[i] = des[i];
        ad = rgad[i];
        if (ad == rLarge) {
          des[i] = rLarge;
          if (fEdge)
            continue;
          PrintCh(' ');
          PrintTab('-', VSeconds(2, 8, 12));
          PrintTab(' ', 1 + us.fSeconds);
        } else {
          od = planet1[i] + DFromR(ad);
          z = longm - (od + rDegQuad);
          if (z < 0.0)
            z += rDegMax;
          if (z > rDegHalf)
            z -= rDegMax;
          des[i] = z;
          if (fEdge)
            continue;
          if (us.fSeconds) {
            sprintf(sz, "%s ", SzLocation(z, 0.0));
            sz[11 + us.fSecond1K*4] = chNull;
          } else
            sprintf(sz, "%3.0f%c", RAbs(z), z < 0.0 ? 'e' : 'w');
          PrintSz(sz);
        }
      }
    }
    if (!fEdge)
      PrintL();

    // Now, if the -L0 switch is in effect, then take these line positions,
    // which were saved in an array above as they were printed, and calculate
    // and print the latitude crossings.

    if (!us.fLatitudeCross)
      continue;
    for (l = 0; l <= is.nObj; l++) {
      if (ignore[l] || !FThing2(l))
        continue;
      for (k = 0; k <= is.nObj; k++) {
        if (ignore[k] || !FThing2(k))
          continue;
        for (n = 0; n <= 1; n++) {
          if (ignorez[n ? arDes : arAsc])
            continue;
          x = n ? des1[l] : asc1[l];
          y = n ? des[l] : asc[l];
          // Make sure Asc/Des crossings in top/bottom lat bands are seen.
          if ((x == rLarge) != (y == rLarge)) {
            zz = (x == rLarge ? y : x);
            z = MinDistance(Mod(zz), Mod(mc[l])) <
              MinDistance(Mod(zz), Mod(ic[l])) ? mc[l] : ic[l];
            if (x == rLarge)
              x = z;
            else
              y = z;
          }
          // Second set of variables used to detect crossings near lon 180.
          xx = (x >= 0.0 ? x : x + rDegMax);
          yy = (y >= 0.0 ? y : y + rDegMax);
          for (m = 0; m <= 1; m++) {

            // Check if Ascendant/Descendant cross Midheaven/Nadir.

            z = m ? ic[k] : mc[k];
            zz = (z >= 0 ? z : z + rDegMax);
            if (cCross < MAXCROSS && k != l && !ignorez[m ? arIC : arMC] &&
              (FCrossAscMC(x, y, z) || FCrossAscMC(xx, yy, zz))) {
              pcr->obj1 = l;
              pcr->ang1 = n ? oDes : oAsc;
              pcr->obj2 = k;
              pcr->ang2 = m ? oNad : oMC;
              pcr->lon  = z;
              pcr->lat  = (real)j+5.0*(FCrossAscMC(x, y, z) ?
                RAbs(z-y)/RAbs(x-y) : RAbs(zz-yy)/RAbs(xx-yy));
              cCross++, pcr++;
#ifdef EXPRESS
              // Skip current crossing if AstroExpression says to do so.
              if (!us.fExpOff && FSzSet(us.szExpCross)) {
                ExpSetR(iLetterU, (pcr-1)->lon);
                ExpSetR(iLetterV, (pcr-1)->lat);
                ExpSetN(iLetterW, l);
                ExpSetN(iLetterX, (pcr-1)->ang1);
                ExpSetN(iLetterY, k);
                ExpSetN(iLetterZ, (pcr-1)->ang2);
                if (!NParseExpression(us.szExpCross))
                  cCross--, pcr--;
              }
#endif
            }

            // Check if Ascendant/Descendant cross another Asc/Des.

            w = m ? des1[k] : asc1[k];
            z = m ? des[k] : asc[k];
            // Make sure Asc/Des crossings in top/bottom lat bands are seen.
            if ((w == rLarge) != (z == rLarge)) {
              zz = (w == rLarge ? z : w);
              ww = MinDistance(Mod(zz), Mod(mc[k])) <
                MinDistance(Mod(zz), Mod(ic[k])) ? mc[k] : ic[k];
              if (w == rLarge)
                w = ww;
              else
                z = ww;
            }
            // Second set of variables used to detect crossings near lon 180.
            ww = (w >= 0.0 ? w : w + rDegMax);
            zz = (z >= 0.0 ? z : z + rDegMax);
            if (cCross < MAXCROSS && k > l && !(k == oSou && l == oNod) &&
              !ignorez[m ? arDes : arAsc] &&
              (FCrossAscAsc(w, x, y, z) || FCrossAscAsc(ww, xx, yy, zz))) {
              pcr->obj1 = l;
              pcr->ang1 = n ? oDes : oAsc;
              pcr->obj2 = k;
              pcr->ang2 = m ? oDes : oAsc;
              pcr->lon = y + (x-y)*RAbs(y-z)/(RAbs(x-w)+RAbs(y-z));
              pcr->lat = (real)j + 5.0*RAbs(y-z)/(RAbs(x-w)+RAbs(y-z));
              // Ensure no duplicate event due to high lat band step rate
              for (o = 0; o < cCross; o++)
                if (rgcr[o].obj1 == pcr->obj1 && rgcr[o].obj2 == pcr->obj2 &&
                  rgcr[o].ang1 == pcr->ang1 &&
                  (rgcr[o].ang2 == oAsc || rgcr[o].ang2 == oDes)) {
                  if (RAbs(rgcr[o].lat) > RAbs(pcr->lat)) {
                    crT = rgcr[o]; rgcr[o] = *pcr; *pcr = crT;
                  }
                  break;
                }
              if (o < cCross)
                continue;
              cCross++, pcr++;
#ifdef EXPRESS
              // Skip current crossing if AstroExpression says to do so.
              if (!us.fExpOff && FSzSet(us.szExpCross)) {
                ExpSetR(iLetterU, (pcr-1)->lon);
                ExpSetR(iLetterV, (pcr-1)->lat);
                ExpSetN(iLetterW, l);
                ExpSetN(iLetterX, (pcr-1)->ang1);
                ExpSetN(iLetterY, k);
                ExpSetN(iLetterZ, (pcr-1)->ang2);
                if (!NParseExpression(us.szExpCross))
                  cCross--, pcr--;
              }
#endif
            }
          } // m
        } // n
      } // k
    } // l
  } // j
  if (!us.fLatitudeCross)
    return fTrue;
  PrintL();

  // Now, print out all the latitude crossings found.
  // First, sort them in order of decreasing latitude.

  for (i = 1; i < cCross; i++) {
    j = i-1;
    while (j >= 0 && rgcr[j].lat < rgcr[j+1].lat) {
      crT = rgcr[j]; rgcr[j] = rgcr[j+1]; rgcr[j+1] = crT;
      j--;
    }
  }
  for (i = 0; i < cCross; i++) {
    pcr = rgcr + i;
    j = pcr->obj1;
    AnsiColor(kObjA[j]);
    sprintf(sz, "%.3s ", szObjDisp[j]); PrintSz(sz);
    k = pcr->ang1;
    AnsiColor(kObjA[k]);
    sprintf(sz, "%-10s", szObjName[k]);
    PrintSz(sz);
    AnsiColor(kDefault);
    PrintSz(" crosses ");
    l = pcr->obj2;
    AnsiColor(kObjA[l]);
    sprintf(sz, "%.3s ", szObjDisp[l]); PrintSz(sz);
    m = pcr->ang2;
    AnsiColor(kObjA[m]);
    sprintf(sz, "%-10s ", szObjName[m]);
    PrintSz(sz);
    AnsiColor(kDefault);
    PrintSz("at ");
    PrintSz(SzLocation(pcr->lon, pcr->lat));
    PrintL();
#ifdef INTERPRET
    if (us.fInterpret)                    // Interpret it if -I in effect.
      InterpretAstroGraph(j, k, l, m);
#endif
#ifdef ATLAS
    DisplayAtlasNearby(pcr->lon, pcr->lat, 0, NULL, fTrue);
#endif
  }
  DeallocateP(rgcr);
  if (cCross <= 0) {
    AnsiColor(kDefault);
    PrintSz("No latitude crossings.\n");
  }
  return fTrue;
}


#ifdef INTERPRET
CONST char *szMoonsAngle1[4] = {"Leading", "Distant", "Following", "Close"};
CONST char *szMoonsAngle2[4] = {"emerging, increasing, or being developed",
  "subtle, weak, or internal", "leaving, ignored, or being left behind",
  "strong, obvious, or prominent"};
#endif

// Print a chart showing the locations and aspects involving planetary moons,
// as specified with the -8 switch. This includes both standard geocentric as
// well as planet centered positions and aspects.

flag ChartMoons()
{
  char sz[cchSzMax];
  byte ignoreSav[objMax];
  int objCenterSav = us.objCenter, m, i0, i, j, et, asp, pass, nAsp,
    count;
  real radi1, radi2, len1, len2, ang1, ang2, ang, rOrb, rPct, rx, ry, rT;
  flag fMoonSav = us.fMoonMove, fInterpretSav = us.fInterpret;
  CP rgcp[cHasMoons+1], *cp;
  PT3R pt;

  // If no moons unrestricted, temporarily unrestrict them all.
  Assert(sizeof(ignore) == sizeof(ignoreSav));
  CopyRgb(ignore, ignoreSav, sizeof(ignore));
  count = 0;
  for (m = custLo; m <= custHi; m++) {
    i = ObjOrbit(m);
    count += (!ignore[m] && FHasMoon(i));
  }
  if (count == 0)
    for (m = moonsLo; m <= moonsHi; m++)
      ignore[m] = fFalse;
  AdjustRestrictions();

  // Cast initial charts.
  us.fMoonMove = fFalse;
  if (!us.fMoonChartSep) {
    // Planetcentric charts are relative to geocentric reference.
    CastChart(1);
    rgcp[0] = cp0;
    for (i0 = 1; i0 <= cHasMoons; i0++) {
      i = rgobjHasMoons[i0-1];
      if (ignore[i])
        continue;
      j = ObjCOB(i);
      j = (ignore[j] ? i : j);
      pt = space[j];
      for (i = 0; i <= is.nObj; i++) {
        PtSub2(space[i], pt);
        ProcessPlanet(i, 0.0);
      }
      rgcp[i0] = cp0;
      cp0 = rgcp[0];
    }
  } else {
    // Planetcentric charts are truly relative to each planet separately.
    for (i0 = cHasMoons; i0 >= 0; i0--) {
      if (i0 <= 0)
        us.objCenter = objCenterSav;
      else {
        i = rgobjHasMoons[i0-1];
        if (ignore[i])
          continue;
        j = ObjCOB(i);
        us.objCenter = (ignore[j] ? i : j);
      }
      CastChart(-1);
      rgcp[i0] = cp0;
    }
  }

  // Geocentric and planet-centric moon positions.
  for (i0 = 1; i0 <= cHasMoons; i0++) {
    i = rgobjHasMoons[i0-1];
    if (ignore[i])
      continue;
    if (FCust(i)) {
      j = ObjOrbit(i);
      if (FHasMoon(j))
        continue;
    }
    cp = &rgcp[i0];
    count = 0;
    for (m = custLo; m <= custHi; m++) {
      if (ignore[m] || ObjOrbit(m) != i)
        continue;
      if (count <= 0) {
        AnsiColor(kObjA[i]);
        sprintf(sz, "%s'%s moons:\n", szObjDisp[i],
          szObjDisp[i][CchSz(szObjDisp[i])-1] == 's' ? "" : "s"); PrintSz(sz);
        AnsiColor(kDefault);
        PrintSz("     Name: ");

        if (objCenterSav == oEar)
          PrintSz(!us.fSeconds ? "Geocentric pos." : "Geocentric position   ");
        else if (objCenterSav == oSun)
          PrintSz(!us.fSeconds ? "Heliocentr pos." : "Heliocentric position ");
        else {
          sprintf(sz, "%scentric pos.          ", szObjDisp[objCenterSav]);
          sz[!us.fSeconds ? 15 : 22] = chNull;
          PrintSz(sz);
        }
        AnsiColor(kObjA[oAsc]);
        if (us.fSeconds)
          PrintSz("   ");
        PrintSz("   Lead");
        AnsiColor(kObjA[oMC]);
        if (us.fSeconds)
          PrintSz("  ");
        PrintSz(" Close");

        AnsiColor(kObjA[m]);
        sprintf(sz, " %c ", chV); PrintSz(sz);
        AnsiColor(kDefault);
        sprintf(sz, "Planetcent%s pos%s", !us.fSeconds ? "" : "ric",
          !us.fSeconds ? "." : "ition"); PrintSz(sz);
        AnsiColor(kObjA[oAsc]);
        if (us.fSeconds)
          PrintSz("   ");
        PrintSz("   Lead");
        AnsiColor(kObjA[oMC]);
        if (us.fSeconds)
          PrintSz("  ");
        PrintSz(" Close\n");
      }
      AnsiColor(kObjA[m]);
      sprintf(sz, "%9.9s: ", szObjDisp[m]); PrintSz(sz);

      // Print geocentric positions
      PrintZodiac(rgcp[0].obj[m]);
      PrintCh(ChRet(rgcp[0].dir[m]));
      PrintCh(' ');
      PrintAltitude(rgcp[0].alt[m]);
      rx = space[m].x - space[i].x; ry = space[m].y - space[i].y;
      rx = Mod(planet[i] - RAngleD(rx, ry));
      rT = (MinDistance(90.0, rx) / rDegQuad - 1.0)*100.0;
      j = (rT >= 75.0 ? oAsc : (rT <= -75.0 ? oDes : 0));
      AnsiColor(j != 0 ? kObjA[j] : kDefault);
      PrintCh(' ');
      sprintf(sz, !us.fSeconds ? "%4.0f" : "%7.2f", rT); PrintSz(sz);
      PrintSz("% ");
      rT = (MinDistance(0.0, rx) / rDegQuad - 1.0)*100.0;
      if (j == 0)
        j = (rT >= 75.0 ? oMC : (rT <= -75.0 ? oNad : 0));
      AnsiColor(rT >= 75.0 ? kObjA[oMC] :
        (rT <= -75.0 ? kObjA[oNad] : kDefault));
      sprintf(sz, !us.fSeconds ? "%4.0f" : "%6.2f", rT); PrintSz(sz);
      PrintSz("% ");

      // Print planet-centric positions
      AnsiColor(kObjA[m]);
      PrintCh2(chV); PrintCh(' ');
      PrintZodiac(cp->obj[m]);
      PrintCh(ChRet(cp->dir[m]));
      PrintCh(' ');
      PrintAltitude(cp->alt[m]);
      rT = MinDistance(cp->obj[m], Mod(cp->obj[oSun] + rDegQuad));
      rT = (rT / rDegQuad - 1.0)*100.0;
      if (j == 0)
        j = (rT >= 75.0 ? oAsc : (rT <= -75.0 ? oDes : 0));
      AnsiColor(rT >= 75.0 ? kObjA[oAsc] :
        (rT <= -75.0 ? kObjA[oDes] : kDefault));
      PrintCh(' ');
      sprintf(sz, !us.fSeconds ? "%4.0f" : "%7.2f", rT); PrintSz(sz);
      rT = MinDistance(cp->obj[m], Mod(cp->obj[oSun] + rDegHalf));
      rT = (rT / rDegQuad - 1.0)*100.0;
      PrintSz("% ");
      if (j == 0)
        j = (rT >= 75.0 ? oMC : (rT <= -75.0 ? oNad : 0));
      AnsiColor(rT >= 75.0 ? kObjA[oMC] :
        (rT <= -75.0 ? kObjA[oNad] : kDefault));
      sprintf(sz, !us.fSeconds ? "%4.0f" : "%6.2f", rT); PrintSz(sz);
      PrintSz("%\n");
      count++;
#ifdef INTERPRET
      if (us.fInterpret && j > 0 && FInterpretObj(i) && FInterpretObj(m)) {
        AnsiColor(kObjA[j]);
        j = (j - oAsc) / 3;
        sprintf(sz, "%s %s:", szObjDisp[m], szMoonsAngle1[j]); FieldWord(sz);
        sprintf(sz, "%s's %s is %s.", szPerson, szMindPart[m],
          szMoonsAngle2[j]); FieldWord(sz);
        j = SFromZ(cp->obj[m]);
        sprintf(sz, "It is %s and %s.", szDesc[j], szDesire[j]); FieldWord(sz);
        FieldWord(NULL);
      }
#endif
    }
    if (count > 0)
      PrintL();
  }

  // Geocentric moons precise Conjunct bodies.
  for (pass = 1; pass <= 2; pass++) {
    rOrb = (pass <= 1 ? 1.0 : rAspOrb[aCon] + 1.0);
    AnsiColor(kDefault);
    if (pass <= 1)
      sprintf(sz, "Precise Conjunctions between planet disks and their moons, "
        "seen from %s:\n", szObjDisp[us.objCenter]);
    else
      sprintf(sz, "\nPrecise Conjunctions between moons of the same planet, "
        "seen from %s:\n", szObjDisp[us.objCenter]);
    PrintSz(sz);
    count = 0;
    for (i = custLo; i <= custHi; i++) {
      if (pass >= 2 && (ignore[i] || i == us.objCenter))
        continue;
      for (m = custLo; m <= custHi; m++) {
        if (pass <= 1) {
          if (ignore[m])
            continue;
          i = ObjOrbit(m);
          if (ignore[i] || !FHasMoon(i) || i == m)
            continue;
          j = ObjCOB(i);
          j = ignore[j] ? i : j;
        } else {
          if (ignore[m] || m <= i || m == us.objCenter)
            continue;
          j = ObjOrbit(m);
          if (!FHasMoon(j))
            continue;
          j = i;
        }

        radi1 = RObjDiam(m) / 2.0;
        radi2 = RObjDiam(j) / 2.0;
        len1 = PtLen(space[m]) * rAUToKm;
        len2 = PtLen(space[j]) * rAUToKm;
        ang1 = len1 == 0.0 ? 0.0 : RAtnD(radi1 / len1);
        ang2 = len2 == 0.0 ? 0.0 : RAtnD(radi2 / len2);

        rT = !us.fAspect3D ? (!us.fParallel ? MinDistance(planet[m],
          planet[j]) : RAbs(planetalt[m] - planetalt[j])) :
          SphDistance(planet[m], planetalt[m], planet[j], planetalt[j]);
        ang = ang1 + ang2;
        if (rT > ang*rOrb)
          continue;
        count++;
        PrintAspect(i, planetval(j), planetdir(j), aCon,
          m, planetval(m), planetdir(m), '8');
        if (us.fSeconds) {
          AnsiColor(rT <= ang ? kDefault : kDkGray);
          sprintf(sz, " - alignment: %.0f%%",
            ang == 0.0 ? 100.0 : (1.0 - rT/ang)*100.0);
          PrintSz(sz);
        }
        et = us.fEclipse ? NCheckEclipse(m, j, &rPct) : etNone;
        if (et > etNone) {
          AnsiColor(kWhiteA);
          sprintf(sz, " (%s Occultation", szEclipse[et]); PrintSz(sz);
          if (us.fSeconds) {
            sprintf(sz, " %.0f%%", rPct); PrintSz(sz);
          }
          PrintSz(")");
        }
        PrintL();
#ifdef INTERPRET
        if (us.fInterpret)
          InterpretAspectCore(i, aCon, m,
            et > etNone ? 0 : (rT <= ang ? 1 : 2));
#endif
      }
      if (pass <= 1)
        i = custHi;
    }
    AnsiColor(kDefault);
    if (count <= 0)
      PrintSz("  None.\n");
  }

  // Planet-centric moon aspect planet-centric planet.
  nAsp = (us.fGraphAll ? us.nAsp : Min(aSqu, us.nAsp));
  sprintf(sz, "\n%ss between moons and other planets, seen from planet "
    "the moon orbits:\n", nAsp == 1 ? "Conjunction" : "Aspect");
  PrintSz(sz);
  count = 0;
  for (i0 = 1; i0 <= cHasMoons; i0++) {
    i = rgobjHasMoons[i0-1];
    if (ignore[i])
      continue;
    cp0 = rgcp[i0];
    j = ObjCOB(i);
    us.objCenter = (ignore[j] ? i : j);
    if (!FCreateGrid(fFalse))
      return fFalse;
    for (m = custLo; m <= custHi; m++) {
      if (ignore[m] || ObjOrbit(m) != i)
        continue;
      for (j = 0; j <= is.nObj; j++) {
        asp = ObjOrbit(j);
        if (ignore[j] || FHasMoon(asp) || j == i || !FThing(j))
          continue;
        asp = grid->n[Min(j, m)][Max(m, j)];
        if (asp <= 0 || asp > nAsp ||
          (!us.fGraphAll && j != oSun && asp > aCon))
          continue;
        count++;
        PrintAspect(m, planetval(m), planetdir(m), asp,
          j, planetval(j), planetdir(j), '8');
        rOrb = grid->v[Min(j, m)][Max(m, j)];
        AnsiColor(rOrb < 0.0 ? kWhiteA : kLtGrayA);
        sprintf(sz, " - orb: %c%d%c%02d'",
          rgchAppSep[us.nAppSep*2 + (rOrb >= 0.0)], NAbs((int)rOrb), chDegC,
          NAbs((int)(rOrb*60.0))%60); PrintSz(sz);
        if (us.fSeconds) {
          sprintf(sz, "%02d\"", NAbs((int)(rOrb*3600.0))%60); PrintSz(sz);
        }
        us.fInterpret = fFalse;
        PrintInDayEvent(j, asp, m, -1);
        us.fInterpret = fInterpretSav;
#ifdef INTERPRET
        if (us.fInterpret)
          InterpretAspectCore(j, asp, m, (int)(rOrb*3600.0)/(150*60));
#endif
      }
    }
  }
  us.objCenter = objCenterSav;
  if (count <= 0)
    PrintSz("  None.\n");

  // Planet-centric moon aspect other bodies.
  us.fMoonMove = fTrue;
  CastChart(0);
  if (!us.fMoonChartSep) {
    // Want planetcentric positions as seen from geocentric reference here.
    for (i0 = 0; i0 < cHasMoons; i0++) {
      i = rgobjHasMoons[i0];
      if (ignore[i])
        continue;
      for (m = custLo; m <= custHi; m++) {
        if (ignore[m])
          continue;
        j = ObjOrbit(m);
        if (j != i)
          continue;
        planet[m]    = rgcp[i0+1].obj[m];
        planetalt[m] = rgcp[i0+1].alt[m];
        ret[m]       = rgcp[i0+1].dir[m];
      }
    }
  }
  if (!FCreateGrid(fFalse))
    return fFalse;
  for (pass = 1; pass <= 3; pass++) {
    nAsp = (pass == 1 || us.fGraphAll ? us.nAsp : aCon);
    AnsiColor(kDefault);
    PrintL();
    if (pass <= 1)
      sprintf(sz, "%ss between moons of the same planet, "
        "seen from that planet:\n", nAsp == 1 ? "Conjunction" : "Aspect");
    else if (pass == 2)
      sprintf(sz, "%ss between moons of different planets, "
        "seen from those planets:\n", nAsp == 1 ? "Conjunction" : "Aspect");
    else {
      i = us.objCenter;
      sprintf(sz,
        "%ss between %scentric planets and planet-centric moons, overlayed:\n",
        nAsp == 1 ? "Conjunction" : "Aspect",
        i == oEar ? "geo" : (i == oSun ? "helio" : szObjDisp[i]));
    }
    PrintSz(sz);
    count = 0;
    for (i = 0; i <= is.nObj; i++) {
      j = ObjOrbit(i);
      if (ignore[i] || (pass < 3) != (FCust(i) && FHasMoon(j)))
        continue;
      for (m = custLo; m <= custHi; m++) {
        j = ObjOrbit(m);
        if (ignore[m] || !FHasMoon(j) || m <= i ||
          (pass < 3 && (pass <= 1) == (ObjOrbit(i) != j)))
          continue;
        asp = grid->n[Min(i, m)][Max(m, i)];
        if (!FBetween(asp, aCon, nAsp))
          continue;
        count++;
        PrintAspect(i, planetval(i), planetdir(i), asp,
          m, planetval(m), planetdir(m), '8');
        rOrb = grid->v[Min(i, m)][Max(m, i)];
        AnsiColor(rOrb < 0.0 ? kWhiteA : kLtGrayA);
        sprintf(sz, " - orb: %c%d%c%02d'",
          rgchAppSep[us.nAppSep*2 + (rOrb >= 0.0)], NAbs((int)rOrb), chDegC,
          NAbs((int)(rOrb*60.0)%60)); PrintSz(sz);
        if (us.fSeconds) {
          sprintf(sz, "%02d\"", NAbs((int)(rOrb*3600.0))%60); PrintSz(sz);
        }
        us.fInterpret = fFalse;
        PrintInDayEvent(i, asp, m, -1);
        us.fInterpret = fInterpretSav;
#ifdef INTERPRET
        if (us.fInterpret)
          InterpretAspectCore(i, asp, m, (int)(rOrb*3600.0)/(150*60));
#endif
      }
    }
    AnsiColor(kDefault);
    if (count <= 0)
      PrintSz("  None.\n");
  }

  // Restore settings and recast original chart.
  CopyRgb(ignoreSav, ignore, sizeof(ignoreSav));
  AdjustRestrictions();
  us.fMoonMove = fMoonSav;
  CastChart(1);
  return fTrue;
}


// Another important procedure: Display any of the types of (text) charts that
// the user specified they wanted, by calling the appropriate routines.

void PrintChart(flag fProg)
{
  int fCall = fFalse, nSav;
  flag fHaveGrid = fFalse;

  if (us.fListing) {
    if (is.fMult)
      PrintL2();
    if (us.nRel <= rcDual)
      ChartListingRelation();
    else if (us.nRel < rcDifference)
      ChartListing();
    else

      // If the -rb or -rd relationship charts are in effect, then instead of
      // doing the standard -v chart, print either of these chart types.

      DisplayRelation();
    is.fMult = fTrue;
  }
  if (us.fWheel) {
    if (is.fMult)
      PrintL2();
    ChartWheel();
    is.fMult = fTrue;
  }
  if (us.fGrid) {
    PrintHeader(is.fMult ? -1 : 1);
    if (us.nRel > rcDual) {
      fCall = us.fSmartCusp; us.fSmartCusp = fFalse;
      nSav = us.objRequire; us.objRequire = -1;
      if (!FCreateGrid(fFalse))
        return;
      us.fSmartCusp = fCall; us.objRequire = nSav;
      inv(fCall);
      ChartGrid();
      if (us.fGridConfig) {    // If -g0 switch in effect, then
        PrintL();              // display aspect configurations.
        if (!fCall) {
          nSav = us.objRequire; us.objRequire = -1;
          FCreateGrid(fFalse);
          us.objRequire = nSav;
        }
        DisplayAspectConfigs();
      }
    } else {

      // Do a relationship aspect grid between two charts if -r0 in effect.

      fCall = us.fSmartCusp; us.fSmartCusp = fFalse;
      nSav = us.objRequire; us.objRequire = -1;
      if (!FCreateGridRelation(us.fGridMidpoint))
        return;
      us.fSmartCusp = fCall; us.objRequire = nSav;
      ChartGridRelation();
    }
    is.fMult = fTrue;
  }
  if (us.fAspList) {
    PrintHeader(is.fMult ? -1 : 1);
    if (us.nRel > rcDual) {
      if (!fHaveGrid) {
        fHaveGrid = fTrue;
        if (!FCreateGrid(fFalse))
          return;
      }
      ChartAspect();
    } else {
      if (!FCreateGridRelation(fFalse))
        return;
      ChartAspectRelation();
    }
    is.fMult = fTrue;
  }
  if (us.fMidpoint) {
    PrintHeader(is.fMult ? -1 : 1);
    if (us.nRel > rcDual) {
      if (!fHaveGrid) {
        if (!FCreateGrid(fFalse))
          return;
      }
      ChartMidpoint();
    } else {
      if (!FCreateGridRelation(fTrue))
        return;
      ChartMidpointRelation();
    }
    is.fMult = fTrue;
  }
  if (us.fHorizon) {
    PrintHeader(-is.fMult);
    ChartHorizon();
    is.fMult = fTrue;
  }
  if (us.fOrbit) {
    PrintHeader(is.fMult ? -1 : 1);
    ChartOrbit();
    is.fMult = fTrue;
  }
  if (us.fSector) {
    PrintHeader(-is.fMult);
    ChartSector();
    is.fMult = fTrue;
  }
  if (us.fCalendar) {
    if (is.fMult)
      PrintL2();
    if (us.fCalendarYear)
      ChartCalendarYear();
    else
      ChartCalendarMonth();
    is.fMult = fTrue;
  }
  if (us.fInfluence) {
    PrintHeader(is.fMult ? -1 : 1);
    ChartInfluence();
    is.fMult = fTrue;
  }
  if (us.fEsoteric) {
    PrintHeader(is.fMult ? -1 : 1);
    ChartEsoteric(fFalse);
    is.fMult = fTrue;
  }
  if (us.fAstroGraph) {
    PrintHeader(is.fMult ? -1 : 1);
    if (us.nRel > rcDual)
      ChartAstroGraph();
    else
      ChartAstroGraphRelation();
    is.fMult = fTrue;
  }
  if (us.fEphemeris) {
    if (is.fMult)
      PrintL2();
    ChartEphemeris();
    is.fMult = fTrue;
  }
#ifdef ARABIC
  if (us.fArabic) {
    PrintHeader(is.fMult ? -1 : 1);
    DisplayArabic();
    is.fMult = fTrue;
  }
#endif
  if (us.fHorizonSearch) {
    if (is.fMult)
      PrintL2();
    ChartHorizonRising();
    is.fMult = fTrue;
  }
  if (us.fInDay) {
    if (is.fMult)
      PrintL2();
    ChartInDaySearch(fProg);
    is.fMult = fTrue;
  }
  if (us.fInDayInf) {
    PrintHeader(is.fMult ? -1 : 1);
    ChartInDayInfluence();
    is.fMult = fTrue;
  }
  if (us.fInDayGra) {
    if (is.fMult)
      PrintL2();
    if (!is.fMult && fProg)
      PrintHeader(1);
    ChartTransitGraph(fFalse, fProg);
    is.fMult = fTrue;
  }
  if (us.fTransit) {
    PrintHeader(is.fMult ? -1 : 1);
    ChartTransitSearch(fProg);
    is.fMult = fTrue;
  }
  if (us.fTransitInf) {
    PrintHeader(is.fMult ? -1 : 1);
    ChartTransitInfluence(fProg);
    is.fMult = fTrue;
  }
  if (us.fTransitGra) {
    PrintHeader(is.fMult ? -1 : 1);
    ChartTransitGraph(fTrue, fProg);
    is.fMult = fTrue;
  }
#ifdef ATLAS
  if (us.fAtlasLook) {
    if (is.fMult)
      PrintL2();
    DisplayAtlasLookup(ciCore.loc, 0, NULL);
    is.fMult = fTrue;
  }
  if (us.fAtlasNear) {
    if (is.fMult)
      PrintL2();
    DisplayAtlasNearby(ciCore.lon, ciCore.lat, 0, NULL, fFalse);
    is.fMult = fTrue;
  }
  if (us.fZoneChange) {
    if (is.fMult)
      PrintL2();
    if (!DisplayAtlasLookup(ciCore.loc, 0, &nSav))
      nSav = -1;
    DisplayTimezoneChanges(nSav >= 0 ? is.rgae[nSav].izn : -1, 0, NULL);
    is.fMult = fTrue;
  }
#endif
  if (us.fMoonChart) {
    PrintHeader(is.fMult ? -1 : 1);
    ChartMoons();
    is.fMult = fTrue;
  }
  if (us.fExoTransit) {
    PrintHeader(is.fMult ? -1 : 1);
    ChartExoplanet(fFalse);
    is.fMult = fTrue;
  }

  if (!is.fMult) {          // Assume the -v chart if user
    us.fListing = fTrue;    // didn't indicate anything.
    PrintChart(fProg);
    is.fMult = fTrue;
  }
}

/* charts1.cpp */
