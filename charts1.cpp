/*
** Astrolog (Version 7.00) File: charts1.cpp
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


/*
******************************************************************************
** Single Chart Display Routines.
******************************************************************************
*/

/* Print header info showing time and date of the chart being displayed.   */
/* This is used by ChartListing() and the -l sector chart in PrintChart(). */

void PrintHeader(int nSpace)
{
  char sz[cchSzDef];
  int day, fNam, fLoc;

  if (nSpace < 0) {
    /* If another chart's been printed, just skip a couple lines instead. */
    PrintL2();
    return;
  }

  fNam = *ciMain.nam > chNull; fLoc = *ciMain.loc > chNull;
  AnsiColor(kWhiteA);
  sprintf(sz, "%s %s", szAppName, szVersionCore); PrintSz(sz);
  if (!is.fSeconds || fNam)
    PrintSz(" chart ");
  else
    PrintSz(": ");
  if (FNoTimeOrSpace(ciMain))
    PrintSz("(No time or space)\n");
  else if (us.nRel == rcComposite)
    PrintSz("(Composite)\n");
  else {
    if (!is.fSeconds || fNam)
      PrintSz("for ");
    sprintf(sz, "%s%s", ciMain.nam, fNam ? "\n" : ""); PrintSz(sz);
    day = DayOfWeek(Mon, Day, Yea);
    sprintf(sz, "%.3s %s %s (%cT Zone %s)", szDay[day],
      SzDate(Mon, Day, Yea, 3), SzTim(Tim), ChDst(Dst),
      SzZone(Zon)); PrintSz(sz);
    sprintf(sz, "%c%s%s%s\n", fLoc && !fNam ? '\n' : ' ', ciMain.loc,
      fLoc ? " " : "", SzLocation(Lon, Lat)); PrintSz(sz);
  }
  AnsiColor(kDefault);
  if (nSpace > 0)
    PrintL();
}


/* Print the straight listing of planet and house positions and specified */
/* by the -v switch, along with the element table, etc.                   */

void ChartListing(void)
{
  ET et;
  char sz[cchSzMax];
  int i, j, k, fNam, fLoc;
  real rT;

  CreateElemTable(&et);
  fNam = *ciMain.nam > chNull; fLoc = *ciMain.loc > chNull;

  PrintHeader(0);    /* Show time and date of the chart being displayed. */

#ifdef INTERPRET
  if (us.fInterpret) {          /* Print an interpretation if -I in effect. */
    if (us.nRel == rcSynastry)
      InterpretSynastry();      /* Print synastry interpretaion for -r -I.  */
    else
      InterpretLocation();      /* Do normal interpretation for just -v -I. */
    return;
  }
#endif

  AnsiColor(kDkGrayA);
  if (us.fSeconds) {
    sprintf(sz, "Body  Location   Ret. %s Rul.      House  Rul. Velocity",
      us.fEquator2 ? "Declin. " : "Latitude"); PrintSz(sz);
    if (us.fSeconds && !us.fParallel)
      PrintSz("  Lat-Velocity");
    PrintL();
  } else {
    sprintf(sz,
      "Body  Locat. Ret. %s. Rul.      House  Rul. Veloc.    %s%s Houses.\n",
      us.fEquator2 ? "Decl" : "Lati", us.fHouse3D ? "3D " : "",
      szSystem[is.nHouseSystem]); PrintSz(sz);
  }
  if (!fNam && !fLoc)
    PrintL();

  /* Ok, now print out the location of each object. */

  for (i = j = 0; us.fSeconds ? j <= oNorm : j <= oNorm || i < cSign+8; j++) {
    if (us.fSeconds) {
      if (FIgnore(j))
        continue;
    } else {
      if (j <= oNorm && FIgnore(j))
        continue;
    }
    if (!us.fSeconds && i < cSign+8 && j > oNorm) {
      PrintTab(' ', 51);
      j = oNorm;
    } else {
      AnsiColor(kObjA[j]);
      sprintf(sz, "%-4.4s: ", szObjDisp[j]); PrintSz(sz);
      PrintZodiac(planet[j]);
      sprintf(sz, " %c ", ret[j] >= 0.0 ? ' ' : chRet); PrintSz(sz);
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
      sprintf(sz, us.fSeconds ?
        (rT < 10.0 ? "%9.7f" : (rT < 100.0 ? "%9.6f" : "%9.5f")) :
        (rT < 10.0 ? "%5.3f" : (rT < 100.0 ? "%5.2f" : "%5.1f")), rT);
      PrintSz(sz);
    }
    if (!us.fSeconds) {

      /* For some lines, we have to append the house cusp positions. */

      if (i < cSign) {
        PrintSz("  -  ");
        AnsiColor(kSignA(i+1));
        sprintf(sz, "House cusp %2d: ", i+1); PrintSz(sz);
        PrintZodiac(chouse[i+1]);
      }

      /* For some lines, we have to append the element table information. */

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
      if (!us.fParallel) {
        sprintf(sz, " %c", retalt[j] < 0.0 ? '-' : '+'); PrintSz(sz);
        rT = RAbs(retalt[j]);
        sprintf(sz, "%9.7f", rT); PrintSz(sz);
      } else {
        PrintSz(" Decan: ");
        is.fSeconds = fFalse;
        PrintZodiac(Decan(planet[j]));
        is.fSeconds = us.fSeconds;
      }
    }
    PrintL();
    i++;
  }

  /* Do another loop to print out the stars in their specified order. */

  if (us.nStar) for (i = starLo; i <= starHi; i++) {
    j = oNorm+starname[i-oNorm];
    if (ignore[j])
      continue;
    AnsiColor(kObjA[j]);
    sprintf(sz, "%-4.4s: ", szObjDisp[j]); PrintSz(sz);
    PrintZodiac(planet[j]);
    sprintf(sz, " %c ", ret[j] >= 0.0 ? ' ' : chRet); PrintSz(sz);
    PrintAltitude(planetalt[j]);
    k = inhouse[j];
    AnsiColor(kSignA(k));
    sprintf(sz, "     [%2d%s house]", k, szSuffix[k]); PrintSz(sz);
    AnsiColor(kDefault);
    sprintf(sz, "     %c", ret[j] < 0.0 ? '-' : '+'); PrintSz(sz);
    rT = RAbs(ret[j]);
    sprintf(sz, us.fSeconds ? (rT < 10.0 ? "%9.7f" : "%9.6f") :
      (rT < 10.0 ? "%5.3f" : "%5.2f"), rT); PrintSz(sz);
    AnsiColor(kObjA[j]);
    sprintf(sz, " Star #%2d: %5.2f\n", i-oNorm, rStarBright[j-oNorm]);
    PrintSz(sz);
  }
}


/* Print out the aspect and midpoint grid for a chart, as specified with the */
/* -g switch. (Each grid row takes up 4 lines of text.)                      */

void ChartGrid(void)
{
  char sz[cchSzDef];
  int x, y, r, x1, y1, temp;

#ifdef INTERPRET
  if (us.fInterpret) {    /* Print interpretation instead if -I in effect. */
    InterpretGrid();
    return;
  }
#endif

  for (y1 = 0, y = 0; y <= cObj; y++) if (!ignore[y])
    for (r = 1; r <= 4 + us.fSeconds; r++) {
      for (x1 = 0, x = 0; x <= cObj; x++) if (!ignore[x]) {
        if (y1 > 0 && x1 > 0 && y1+r > 1)
          PrintCh((char)(r > 1 ? chV : chC));
        if (r > 1) {
          temp = grid->n[x][y];

          /* Print aspect rows. */

          if (x < y) {
            if (temp)
              AnsiColor(kAspA[temp]);
            if (r == 2)
              PrintSz(temp ? SzAspectAbbrev(temp) : "   ");
            else if (!temp)
              PrintSz("   ");
            else {
              if (r == 3) {
                if (grid->v[x][y] < 6000*60)
                  sprintf(sz, "%c%2d", us.fAppSep ?
                    (grid->v[x][y] < 0 ? 'a' : 's') :
                    (grid->v[x][y] < 0 ? '-' : '+'), NAbs(grid->v[x][y])/3600);
                else
                  sprintf(sz, "%3d", NAbs(grid->v[x][y])/3600);
              } else if (r == 4)
                sprintf(sz, "%02d'", NAbs(grid->v[x][y])/60%60);
              else
                sprintf(sz, "%02d\"", NAbs(grid->v[x][y])%60);
              PrintSz(sz);
            }

          /* Print midpoint rows. */

          } else if (x > y) {
            AnsiColor(kSignA(temp));
            if (r == 2) {
              temp = grid->n[x][y];
              sprintf(sz, "%.3s", szSignName[temp]);
            } else if (r == 3)
              sprintf(sz, "%2d%c", grid->v[x][y]/3600, chDeg0);
            else if (r == 4)
              sprintf(sz, "%02d'", grid->v[x][y]/60%60);
            else
              sprintf(sz, "%02d\"", grid->v[x][y]%60);
            PrintSz(sz);

          /* Print the diagonal of object names. */

          } else {
            AnsiColor(kReverse);
            if (r == 2) {
              AnsiColor(kObjA[y]);
              sprintf(sz, "%.3s", szObjDisp[y]);
            } else {
              temp = SFromZ(planet[y]);
              AnsiColor(kSignA(temp));
              if (r == 3)
                sprintf(sz, "%2d%c", (int)planet[y] - (temp-1)*30, chDeg0);
              else if (r == 4)
                sprintf(sz, "%.3s", szSignName[temp]);
              else
                sprintf(sz, "%2d'", (int)(planet[y]*60.0) % 60);
            }
            PrintSz(sz);
          }
          AnsiColor(kDefault);
        } else
          if (y1 > 0)
            PrintTab(chH, 3);
        x1++;
      }
      if (y1+r > 1)
        PrintL();
      y1++;
    }
  if (y1 == 0)
    PrintSz("Empty aspect grid.\n");
}


/* This is a subprocedure of DisplayGrands(). Here we print out one aspect */
/* configuration found by the parent procedure.                            */

void PrintGrand(int ac, int i1, int i2, int i3, int i4)
{
  char sz[cchSzDef];

  AnsiColor(kAspA[rgAspConfig[ac]]);
  sprintf(sz, "%-11s", szAspectConfig[ac]); PrintSz(sz);
  AnsiColor(kDefault);
  sprintf(sz, " %s ", ac == acS3 || ac == acGT || ac == acGC ||
    ac == acMR || ac == acS4 ? "with" : "from");
  PrintSz(sz);
  AnsiColor(kObjA[i1]);
  sprintf(sz, "%.3s: ", szObjDisp[i1]); PrintSz(sz);
  PrintZodiac(planet[i1]);
  sprintf(sz, " %s ", ac == acS3 || ac == acGT || ac == acS4 ? "and" : "to ");
  PrintSz(sz);
  AnsiColor(kObjA[i2]);
  sprintf(sz, "%.3s: ", szObjDisp[i2]); PrintSz(sz);
  PrintZodiac(planet[i2]);
  sprintf(sz, " %s ", ac == acGC || ac == acC || ac == acMR ? "to " : "and");
  PrintSz(sz);
  AnsiColor(kObjA[i3]);
  sprintf(sz, "%.3s: ", szObjDisp[i3]); PrintSz(sz);
  PrintZodiac(planet[i3]);
  if (ac == acGC || ac == acC || ac == acMR || ac == acS4) {
    PrintSz(ac == acS4 ? " and " : " to  ");
    AnsiColor(kObjA[i4]);
    sprintf(sz, "%.3s: ", szObjDisp[i4]); PrintSz(sz);
    PrintZodiac(planet[i4]);
  }
  PrintL();
}


/* Scan the aspect grid of a chart and print out any major configurations, */
/* as specified with the -g0 switch.                                       */

void DisplayGrands(void)
{
  int cac = 0, i, j, k, l;

  for (i = 0; i <= cObj; i++) if (!FIgnore(i))
    for (j = 0; j <= cObj; j++) if (j != i && !FIgnore(j))
      for (k = 0; k <= cObj; k++) if (k != i && k != j && !FIgnore(k)) {
        if (!(us.objRequire >= 0 &&
          i != us.objRequire && j != us.objRequire && k != us.objRequire)) {

          /* Is there a Stellium among the current three planets? */

          if (i < j && j < k && grid->n[i][j] == aCon &&
              grid->n[i][k] == aCon && grid->n[j][k] == aCon) {
            for (l = 0; l <= cObj; l++)
              if (!FIgnore(l) && l != i && l != j && l != k &&
                grid->n[Min(i, l)][Max(i, l)] == aCon &&
                grid->n[Min(j, l)][Max(j, l)] == aCon &&
                grid->n[Min(k, l)][Max(k, l)] == aCon)
                break;
            if (l > cObj) {
              cac++;
              PrintGrand(acS3, i, j, k, 0);
            }

          /* Is there a Grand Trine? */

          } else if (i < j && j < k && grid->n[i][j] == aTri &&
              grid->n[i][k] == aTri && grid->n[j][k] == aTri) {
            cac++;
            PrintGrand(acGT, i, j, k, 0);

          /* Is there a T-Square? */

          } else if (j < k && grid->n[j][k] == aOpp &&
              grid->n[Min(i, j)][Max(i, j)] == aSqu &&
              grid->n[Min(i, k)][Max(i, k)] == aSqu) {
            cac++;
            PrintGrand(acTS, i, j, k, 0);

          /* Is there a Yod? */

          } else if (j < k && grid->n[j][k] == aSex &&
              grid->n[Min(i, j)][Max(i, j)] == aInc &&
              grid->n[Min(i, k)][Max(i, k)] == aInc) {
            cac++;
            PrintGrand(acY, i, j, k, 0);
          }
        }
        for (l = 0; l <= cObj; l++) if (!FIgnore(l)) {
          if (us.objRequire >= 0 && i != us.objRequire &&
            j != us.objRequire && k != us.objRequire && l != us.objRequire)
            continue;

          /* Is there a Grand Cross among the current four planets? */

          if (i < j && i < k && i < l && j < l && grid->n[i][j] == aSqu &&
              grid->n[Min(j, k)][Max(j, k)] == aSqu &&
              grid->n[Min(k, l)][Max(k, l)] == aSqu &&
              grid->n[i][l] == aSqu &&
              MinDistance(planet[i], planet[k]) > 150.0 &&
              MinDistance(planet[j], planet[l]) > 150.0) {
            cac++;
            PrintGrand(acGC, i, j, k, l);

          /* Is there a Cradle? */

          } else if (i < l && grid->n[Min(i, j)][Max(i, j)] == aSex &&
              grid->n[Min(j, k)][Max(j, k)] == aSex &&
              grid->n[Min(k, l)][Max(k, l)] == aSex &&
              MinDistance(planet[i], planet[l]) > 150.0) {
            cac++;
            PrintGrand(acC, i, j, k, l);

          /* Is there a Mystic Rectangle? */

          } else if (i < j && i < k && i < l && grid->n[i][j] == aTri &&
              grid->n[Min(j, k)][Max(j, k)] == aSex &&
              grid->n[Min(k, l)][Max(k, l)] == aTri &&
              grid->n[i][l] == aSex &&
              MinDistance(planet[i], planet[k]) > 150.0 &&
              MinDistance(planet[j], planet[l]) > 150.0) {
            cac++;
            PrintGrand(acMR, i, j, k, l);

          /* Is there a Stellium among the current four planets? */

          } else if (i < j && j < k && k < l && grid->n[i][j] == aCon &&
              grid->n[i][k] == aCon && grid->n[i][l] == aCon &&
              grid->n[j][k] == aCon && grid->n[j][l] == aCon &&
              grid->n[k][l] == aCon) {
            cac++;
            PrintGrand(acS4, i, j, k, l);
          }
        }
      }
  if (!cac)
    PrintSz("No major configurations in aspect grid.\n");
}


/* This is subprocedure of ChartWheel(). Here we print out the location */
/* of a particular house cusp as well as what house cusp number it is.  */

void PrintHouse(int i, int left)
{
  char sz[cchSzDef];
  real r;
  int j;

  if (us.fVedic) {
    j = Mod12(12-i);
    r = ZFromS(j);
  } else {
    j = i;
    r = chouse[i];
  }
  if (!left)
    PrintZodiac(r);
  AnsiColor(kSignA(j));
  sprintf(sz, "<%d>", j); PrintSz(sz);
  if (left)
    PrintZodiac(r);
  else
    AnsiColor(kDefault);
}


/* Another subprocedure of ChartWheel(). Print out one of the chart info */
/* rows in the middle of the wheel (which may be blank) given an index.  */

void PrintWheelCenter(int irow)
{
  char sz[cchSzDef], szT[cchSzDef];
  int cch, nT;

  if (is.nWheelRows > 4)                     /* Try to center lines. */
    irow -= (is.nWheelRows - 4);
  if (*ciMain.nam == chNull && *ciMain.loc == chNull && is.nWheelRows >= 4)
    irow--;                                  
  if (*ciMain.nam == chNull && irow >= 1)    /* Don't have blank lines if */
    irow++;                                  /* the name and/or location  */
  if (*ciMain.loc == chNull && irow >= 3)    /* strings are empty.        */
    irow++;

  switch (irow) {
  case 0:
    sprintf(sz, "%s %s chart", szAppName, szVersionCore);
    break;
  case 1:
    sprintf(sz, "%s", ciMain.nam);
    break;
  case 2:
    nT = DayOfWeek(Mon, Day, Yea);
    sprintf(sz, "%.3s %s %s", szDay[nT], SzDate(Mon, Day, Yea, 2),
      SzTim(Tim));
    break;
  case 3:
    sprintf(sz, "%s", ciMain.loc);
    break;
  case 4:
    nT = (int)(RFract(RAbs(Zon))*100.0+rRound);
    sprintf(sz, "%cT Zone %s, %s", ChDst(Dst), SzZone(Zon),
      SzLocation(Lon, Lat));
    break;
  case 5:
    nT = us.fEuroTime; us.fEuroTime = fTrue;
    sprintf(szT, "%s", SzTim(DFromR(is.RA)*(24.0/rDegMax)));
    sprintf(sz, "UT: %s, Sid.T: %s",
      SzTim(Tim+Zon-(us.dstDef != dstAuto ? Dst : is.fDst)), szT);
    us.fEuroTime = nT;
    break;
  case 6:
    sprintf(sz, "%s%s Houses", us.fHouse3D ? "3D " : "",
      szSystem[is.nHouseSystem]);
    break;
  case 7:
    sprintf(sz, "%s, %s", !us.fSidereal ? "Tropical" : "Sidereal",
      us.objCenter == oSun ?
        (!us.fBarycenter ? "Heliocentric" : "Barycentric") :
      (us.objCenter == oEar ? (!us.fTopoPos ? "Geocentric" : "Topocentric") :
      szObjDisp[us.objCenter]));
    break;
  case 8:
    sprintf(szT, "Julian Day: %%%sf", is.fSeconds ? "14.6" : "12.4");
    sprintf(sz, szT, JulianDayFromTime(is.T));
    break;
  default:
    *sz = chNull;
  }

  cch = CchSz(sz);
  nT = WHEELCOLS*2-1 + is.fSeconds*8;
  PrintTab(' ', (nT - cch) / 2);
  PrintSz(sz);
  PrintTab(' ', nT-cch - (nT - cch) / 2);
}


/* Yet another subprocedure of ChartWheel(). Here we print out one line */
/* in a particular house cell (which may be blank).                     */

void PrintWheelSlot(int obj)
{
  char sz[cchSzDef];

  if (obj >= oEar) {
    AnsiColor(kObjA[obj]);
    sprintf(sz, " %.3s ", szObjDisp[obj]); PrintSz(sz);
    PrintZodiac(planet[obj]);
    sprintf(sz, "%c ", ret[obj] < 0.0 ? chRet2 : ' '); PrintSz(sz);
    PrintTab(' ', WHEELCOLS-15);
  } else                            /* This particular line is blank. */
    PrintTab(' ', WHEELCOLS-1 + is.fSeconds*4);
}


/* Display all the objects in a wheel format on the screen, as specified */
/* with the -w switch. The wheel is divided into the 12 houses and the   */
/* planets are placed accordingly.                                       */

void ChartWheel(void)
{
  int wheel[cSign][WHEELROWS], wheelcols, count = 0, i, j, k, l;

  /* Autodetect wheel house size, based on house with most planets in it. */

  if (us.nWheelRows <= 0) {
    k = 0;
    for (i = 0; i < cSign; i++)
      wheel[i][0] = 0;
    for (i = 0; i <= cObj; i++) {
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

  /* If the seconds (-b0) flag is set, we'll print all planet and house    */
  /* locations to the nearest zodiac second instead of just to the minute. */

  wheelcols = WHEELCOLS + is.fSeconds*4;

  for (i = 0; i < cSign; i++)
    for (j = 0; j < is.nWheelRows; j++)
      wheel[i][j] = -1;                    /* Clear out array. */

  /* This section of code places each object in the wheel house array. */

  for (i = 0; i <= cObj && count < is.nWheelRows*12; i++) {
    if (FIgnore(i) || (FCusp(i) &&
      MinDistance(planet[i], chouse[i-oAsc+1]) < rRound/60.0))
      continue;

    /* Try to put object in its proper house. If no room, */
    /* then overflow over to the next succeeding house.   */

    for (j = (us.fVedic ? Mod12(11-SFromZ(planet[i])) : inhouse[i])-1;
      j < cSign; j = j < cSign ? (j+1)%cSign : j) {

      /* Now try to find the proper place in the house to put the object. */
      /* This is in sorted order, although a check is made for 0 Aries.   */

      if (wheel[j][is.nWheelRows-1] >= 0)
        continue;
      l = chouse[j+1] > chouse[Mod12(j+2)];
      for (k = 0; wheel[j][k] >= 0 && (planet[i] >= planet[wheel[j][k]] ||
         (l && planet[i] < rDegHalf && planet[wheel[j][k]] > rDegHalf)) &&
        !(l && planet[i] > rDegHalf && planet[wheel[j][k]] < rDegHalf); k++)
        ;

      /* Actually insert object in proper place. */

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

  /* Now, if this is really the -w switch and not -w0, then reverse the  */
  /* order of objects in western houses for more intuitive reading. Also */
  /* reverse the order of everything in the reverse order Vedic wheels.  */

  if (us.fVedic)
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

  /* Here we actually print the wheel and the objects in it. */

  PrintCh(chNW); PrintTab(chH, WHEELCOLS-8); PrintHouse(11, fTrue);
  PrintTab(chH, WHEELCOLS-11+us.fVedic); PrintHouse(10, fTrue);
  PrintTab(chH, WHEELCOLS-10+us.fVedic); PrintHouse(9, fTrue);
  PrintTab(chH, wheelcols-4); PrintCh(chNE); PrintL();
  for (i = 0; i < is.nWheelRows; i++) {
    for (j = 10; j >= 7; j--) {
      PrintCh(chV); PrintWheelSlot(wheel[j][i]);
    }
    PrintCh(chV); PrintL();
  }
  PrintHouse(12, fTrue); PrintTab(chH, WHEELCOLS-11);
  PrintCh(chC); PrintTab(chH, wheelcols-1); PrintCh(chJN);
  PrintTab(chH, wheelcols-1); PrintCh(chC); PrintTab(chH, WHEELCOLS-10);
  PrintHouse(8, fFalse); PrintL();
  for (i = 0; i < is.nWheelRows; i++) {
    PrintCh(chV); PrintWheelSlot(wheel[11][i]); PrintCh(chV);
    PrintWheelCenter(i);
    PrintCh(chV); PrintWheelSlot(wheel[6][i]);
    PrintCh(chV); PrintL();
  }
  PrintHouse(1, fTrue); PrintTab(chH, WHEELCOLS-10-us.fVedic);
  PrintCh(chJW); PrintWheelCenter(is.nWheelRows); PrintCh(chJE);
  PrintTab(chH, WHEELCOLS-10); PrintHouse(7, fFalse); PrintL();
  for (i = 0; i < is.nWheelRows; i++) {
    PrintCh(chV); PrintWheelSlot(wheel[0][i]); PrintCh(chV);
    PrintWheelCenter(is.nWheelRows+1 + i);
    PrintCh(chV); PrintWheelSlot(wheel[5][i]);
    PrintCh(chV); PrintL();
  }
  PrintHouse(2, fTrue); PrintTab(chH, WHEELCOLS-10-us.fVedic);
  PrintCh(chC); PrintTab(chH, wheelcols-1); PrintCh(chJS);
  PrintTab(chH, wheelcols-1); PrintCh(chC);
  PrintTab(chH, WHEELCOLS-10); PrintHouse(6, fFalse); PrintL();
  for (i = 0; i < is.nWheelRows; i++) {
    for (j = 1; j <= 4; j++) {
      PrintCh(chV); PrintWheelSlot(wheel[j][i]);
    }
    PrintCh(chV); PrintL();
  }
  PrintCh(chSW); PrintTab(chH, wheelcols-4); PrintHouse(3, fFalse);
  PrintTab(chH, WHEELCOLS-10); PrintHouse(4, fFalse);
  PrintTab(chH, WHEELCOLS-10); PrintHouse(5, fFalse);
  PrintTab(chH, WHEELCOLS-7); PrintCh(chSE); PrintL();
}


/* This is a subprocedure of ChartAspect() and ChartAspectRelation().       */
/* Display summary information about the aspect list, i.e. the total number */
/* of aspects of each type, and the number of aspects to each object, as    */
/* done when the -a0 aspect summary setting is set.                         */

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
  for (j = 0, i = 0; i <= cObj; i++) if (!FIgnore(i)) {
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


/* Display all aspects between objects in the chart, one per line, in       */
/* sorted order based on the total "power" of the aspect, as specified with */
/* the -a switch. The same influences used for -j charts are used here.     */
/* This is almost the same as the -D list, except influences are different. */

void ChartAspect(void)
{
  int ca[cAspect + 1], co[objMax];
  char sz[cchSzDef];
  int vcut = nLarge, icut, jcut, vhi, ihi, jhi, ahi, phi, v, i, j, k, p,
    count = 0;
  real ip, jp, rPowSum = 0.0;

  ClearB((pbyte)ca, sizeof(ca));
  ClearB((pbyte)co, sizeof(co));
  loop {
    vhi = -nLarge;

    /* Search for the next most powerful aspect in the aspect grid. */

    for (i = 1; i <= cObj; i++) if (!FIgnore(i))
      for (j = 0; j < i; j++) if (!FIgnore(j))
        if (k = grid->n[j][i]) {
          ip = RObjInf(i);
          jp = RObjInf(j);
          p = (int)(rAspInf[k]*(ip+jp)/2.0*
            (1.0-RAbs((real)(grid->v[j][i]))/3600.0/GetOrb(i, j, k))*1000.0);
#ifdef EXPRESS
          // Adjust power with AstroExpression if one set.
          if (!us.fExpOff && FSzSet(us.szExpAsplist)) {
            ExpSetN(iLetterW, j);
            ExpSetN(iLetterX, k);
            ExpSetN(iLetterY, i);
            ExpSetN(iLetterZ, p);
            p = NParseExpression(us.szExpAsplist);
          }
#endif
          switch (us.nAspectSort) {
          default:  v = p;                           break;
          case aso: v = -NAbs(grid->v[j][i]);        break;
          case asn: v = -grid->v[j][i];              break;
          case asO: v = -(j*cObj + i);               break;
          case asP: v = -(i*cObj + j);               break;
          case asA: v = -(k*cObj*cObj + j*cObj + i); break;
          case asC: v = -(int)(planet[j]*3600.0);    break;
          case asD: v = -(int)(planet[i]*3600.0);    break;
          case asM: v = -(int)(Midpoint(planet[j], planet[i])*3600.0); break;
          }
          if ((v < vcut || (v == vcut && (i > icut ||
            (i == icut && j > jcut)))) && v > vhi) {
            vhi = v; ihi = i; jhi = j; ahi = k; phi = p;
          }
        }
    if (vhi <= -nLarge)    /* Exit when no less powerful aspect found. */
      break;
    vcut = vhi; icut = ihi; jcut = jhi;
    count++;                               /* Display the current aspect.   */
    rPowSum += (real)phi/1000.0;
    ca[ahi]++;
    co[jhi]++; co[ihi]++;
#ifdef INTERPRET
    if (us.fInterpret) {                   /* Interpret it if -I in effect. */
      InterpretAspect(jhi, ihi);
      AnsiColor(kDefault);
      continue;
    }
#endif
    sprintf(sz, "%3d: ", count); PrintSz(sz);
    PrintAspect(jhi, planet[jhi], (int)RSgn(ret[jhi]), ahi,
      ihi, planet[ihi], (int)RSgn(ret[ihi]), 'a');
    k = grid->v[jhi][ihi];
    AnsiColor(k < 0 ? kWhiteA : kLtGrayA);
    sprintf(sz, " - orb: %c%d%c%02d'",
      us.fAppSep ? (k < 0 ? 'a' : 's') : (k < 0 ? '-' : '+'),
      NAbs(k)/3600, chDeg1, NAbs(k)%3600/60); PrintSz(sz);
    if (is.fSeconds) {
      sprintf(sz, "%02d\"", NAbs(k)%60); PrintSz(sz);
    }
    AnsiColor(kDkGreenA);
    sprintf(sz, " - power:%6.2f", (real)phi/1000.0); PrintSz(sz);
    PrintInDayEvent(jhi, ahi, ihi, -1);
  }

  PrintAspectSummary(ca, co, count, rPowSum);
}


/* Display all aspects in effect from a given degree point to all planets   */
/* in the main chart. Called from ChartMidpoint() and DisplayArabic() to do */
/* aspects to computed midpoints or aspects to Arabic part positions.       */

void PrintAspectsToPoint(real deg, int obj, real rRet, char *szWhat)
{
  char sz[cchSzDef];
  int i, j, asp;
  real rAng, rOrb;

  for (i = 0; i < cObj; i++) if (!FIgnore(i)) {
    rAng = MinDistance(planet[i], deg);
    for (asp = us.nAsp; asp >= 1; asp--) {
      if (!FAcceptAspect(i, asp, obj >= 0 ? obj : i))
        continue;
      rOrb = rAng - rAspAngle[asp];
      if (RAbs(rOrb) < GetOrb(i, obj >= 0 ? obj : i, asp)) {
        if (us.fAppSep)
          rOrb *= RSgn2(rRet - ret[i]) * RSgn2(MinDifference(planet[i], deg));
        sprintf(sz, "      %s ", szWhat); PrintSz(sz);
        PrintZodiac(deg);
        PrintSz(" makes ");
        AnsiColor(kAspA[asp]); PrintSz(szAspectAbbrevDisp[asp]);
        AnsiColor(kDefault); PrintSz(" to ");
        AnsiColor(kObjA[i]); sprintf(sz, "%.10s ", szObjDisp[i]); PrintSz(sz);
        PrintTab(' ', 10-CchSz(szObjDisp[i]));
        j = (int)(rOrb * 3600.0);
        AnsiColor(j < 0.0 ? kWhiteA : kLtGrayA);
        sprintf(sz, "- orb: %c%d%c%02d'",
          us.fAppSep ? (j < 0 ? 'a' : 's') : (j < 0 ? '-' : '+'),
          NAbs(j)/3600, chDeg1, NAbs(j)%3600/60); PrintSz(sz);
        if (is.fSeconds) {
          sprintf(sz, "%02d\"", NAbs(j)%60); PrintSz(sz);
        }
        PrintL();
        AnsiColor(kDefault);
      }
    }
  }
}


/* This is a subprocedure of ChartMidpoint() and ChartMidpointRelation().  */
/* Display summary information about the midpoint list, i.e. the total     */
/* number of midpoints in each sign, and their average span in degrees, as */
/* done when the -m0 midpoint summary setting is set.                      */

void PrintMidpointSummary(int *cs, int count, long lSpanSum)
{
  char sz[cchSzDef];
  int m, i;

  if (count == 0) {
    PrintSz("No midpoints in list.\n");
    return;
  }
  if (!us.fMidSummary)
    return;
  PrintL();
  m = (int)(lSpanSum/count);
  sprintf(sz, "Average span:%4d%c%02d'", m/3600, chDeg1, m%3600/60);
  PrintSz(sz);
  if (is.fSeconds) {
    sprintf(sz, "%02d\"", m%60); PrintSz(sz);
  }
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


/* Display locations of all midpoints between objects in the chart, */
/* one per line, in sorted zodiac order from zero Aries onward, as  */
/* specified with the -m switch.                                    */

void ChartMidpoint(void)
{
  int cs[cSign + 1];
  char sz[cchSzDef];
  int mcut = -1, icut, jcut, mlo, ilo, jlo, m, i, j, count = 0;
  long lSpanSum = 0;
  real mid;

  ClearB((pbyte)cs, sizeof(cs));
  loop {
    mlo = 360*60*60;

    /* Search for the next closest midpoint farther down in the zodiac. */

    for (i = 0; i < cObj; i++) if (!FIgnore(i))
      for (j = i+1; j <= cObj; j++) if (!FIgnore(j)) {
        m = (grid->n[j][i]-1)*(30*3600) + grid->v[j][i];
        if ((m > mcut || (m == mcut && (i > icut ||
          (i == icut && j > jcut)))) && m < mlo) {
          ilo = i; jlo = j; mlo = m;
        }
      }
    if (mlo >= 360*60*60) /* Exit when no midpoint farther in zodiac found. */
      break;
    mcut = mlo; icut = ilo; jcut = jlo;
    if (us.objRequire >= 0 && ilo != us.objRequire && jlo != us.objRequire)
      continue;
    count++;                               /* Display the current midpoint. */
    cs[mlo/(3600*30)+1]++;
    m = (int)(MinDistance(planet[ilo], planet[jlo])*3600.0);
    lSpanSum += m;
#ifdef INTERPRET
    if (us.fInterpret) {                   /* Interpret it if -I in effect. */
      InterpretMidpoint(ilo, jlo);
      AnsiColor(kDefault);
      continue;
    }
#endif
    sprintf(sz, "%4d: ", count); PrintSz(sz);
    mid = (real)mlo/3600.0; PrintZodiac(mid);
    PrintCh(' ');
    PrintAspect(ilo, planet[ilo], (int)RSgn(ret[ilo]), 0,
      jlo, planet[jlo], (int)RSgn(ret[jlo]), 'm');
    AnsiColor(kDefault);
    sprintf(sz, " -%4d%c%02d'", m/3600, chDeg1, m%3600/60); PrintSz(sz);
    if (is.fSeconds) {
      sprintf(sz, "%02d\"", m%60); PrintSz(sz);
    }
    PrintSz(" degree span.\n");

    /* If the -ma switch is set, determine and display each aspect from one */
    /* of the planets to the current midpoint, and the aspect's orb.        */

    if (us.fMidAspect)
      PrintAspectsToPoint(mid, ilo, (ret[ilo]+ret[jlo])/2.0, "Midpoint");
  }

  PrintMidpointSummary(cs, count, lSpanSum);
}


/* Display locations of the objects on the screen with respect to the local */
/* horizon, as specified with the -Z switch.                                */

void ChartHorizon(void)
{
  char sz[cchSzDef], szFormat[cchSzDef];
  real lon, lat, sx, sy, vx, vy,
    lonz[objMax], latz[objMax], azi[objMax], alt[objMax];
  int fPrime, i, j, k, tot;

  /* Set up some initial variables. */

  fPrime = us.fPrimeVert || us.fHouse3D;
  lon = Lon; lat = Lat;
  tot = us.nStar ? cObj : oNorm;

  /* First find zenith location on Earth of each object. */

  for (i = 0; i <= tot; i++) if (!ignore[i] || i == oMC) {
    lonz[i] = Tropical(planet[i]); latz[i] = planetalt[i];
    EclToEqu(&lonz[i], &latz[i]);
  }

  /* Then, convert this to local horizon altitude and azimuth. */

  vx = lonz[oMC];
  for (i = 0; i <= tot; i++) if (!ignore[i]) {
    lonz[i] = Mod(vx - lonz[i] + rDegQuad);
    EquToLocal(&lonz[i], &latz[i], rDegQuad - lat);
    azi[i] = rDegMax - lonz[i]; alt[i] = latz[i];
  }

  /* If the -Z0 switch flag is in effect, convert from altitude/azimuth  */
  /* coordinates to prime vertical coordinates that we'll print instead. */

  if (fPrime) {
    for (i = 0; i <= tot; i++) if (!ignore[i]) {
      CoorXform(&azi[i], &alt[i], rDegQuad);

      /* 3D Campanus cusps should always be start of corresponding house. */
      if (us.fHouse3D && us.nHouseSystem == hsCampanus && FCusp(i))
        azi[i] = (real)((i - oAsc) * 30);
    }
  }

  /* Now, actually print the location of each object. */

  sprintf(szFormat, is.fSeconds ? " " : "");
  sprintf(sz, "Body %s%sAltitude%s %s%sAzimuth%s%s%s  Azi. Vector%s    ",
    szFormat, szFormat, szFormat, szFormat, szFormat,
    us.fHouse3D ? "  " : "", szFormat, szFormat, szFormat);
  PrintSz(sz);
  sprintf(sz, "%s Vector%s%s    Moon Vector\n\n",
    us.objCenter != oSun ? "Sun" : "Earth", szFormat, szFormat); PrintSz(sz);
  for (k = 0; k <= tot; k++) {
    i = k <= oNorm ? k : oNorm+starname[k-oNorm];
    if (ignore[i] || (!us.fHouse3D && !FThing(i)))
      continue;
    AnsiColor(kObjA[i]);
    sprintf(sz, "%-4.4s: ", szObjDisp[i]); PrintSz(sz);
    PrintAltitude(alt[i]);

    /* Determine directional vector based on azimuth. */

    if (!us.fHouse3D)
      sprintf(sz, " %s", SzDegree(azi[i]));
    else {
      sprintf(sz, " %2d%s", SFromZ(azi[i]),
        SzDegree(azi[i] - (real)((SFromZ(azi[i])-1)*30)));
    }
    PrintSz(sz);
    sx = RCos(RFromD(azi[i])); sy = RSin(RFromD(azi[i]));
    if (RAbs(sx) < RAbs(sy)) {
      vx = RAbs(sx / sy); vy = 1.0;
    } else {
      vy = RAbs(sy / sx); vx = 1.0;
    }
    sprintf(sz, is.fSeconds ? " (%.3f%c" : " (%.2f%c", vy,
      sy < 0.0 ? (fPrime ? 'u' : 's') : (fPrime ? 'd' : 'n')); PrintSz(sz);
    sprintf(sz, is.fSeconds ? " %.3f%c)" : " %.2f%c)", vx,
      sx > 0.0 ? 'e' : 'w'); PrintSz(sz);

    /* Determine distance vector of current object from Sun and Moon. */

    vx = (ignore[oSun] ? 0.0 : azi[oSun]) - azi[i];
    vy = (ignore[oMoo] ? 0.0 : azi[oMoo]) - azi[i];
    j = 1 + is.fSeconds;
    sprintf(szFormat, " [%%%d.%df%%%d.%df] [%%%d.%df%%%d.%df]",
      j+5, j, j+5, j, j+5, j, j+5, j);
    sprintf(sz, szFormat,
      RAbs(vx) < rDegHalf ? vx : RSgn(vx) * (rDegMax - RAbs(vx)),
      (ignore[oSun] ? 0.0 : alt[oSun]) - alt[i],
      RAbs(vy) < rDegHalf ? vy : RSgn(vy) * (rDegMax - RAbs(vy)),
      (ignore[oMoo] ? 0.0 : alt[oMoo]) - alt[i]);
    PrintSz(sz);
    if (!is.fSeconds && i >= uranLo) {
      if (i <= uranHi)
        sprintf(sz, "  Uranian #%d", i-uranLo+1);
      else
        sprintf(sz, "  Star #%2d", i-starLo+1);
      PrintSz(sz);
    }
    PrintL();
  }
  AnsiColor(kDefault);
}


/* Display x,y,z locations of each body (in AU) with respect to the Sun */
/* (or whatever the specified center planet is), as in the -S switch.   */
/* These values were already determined when calculating the planet     */
/* positions themselves, so this procedure is basically just a loop.    */

void ChartOrbit(void)
{
  char sz[cchSzMax], sz1[cchSzDef];
  real x, y, z;
  int i, j;

  sprintf(sz1, is.fSeconds ? " " : "");
  sprintf(sz, "Body   %s%sAzimuth%s%s   ", sz1, sz1, sz1, sz1);
  PrintSz(sz);
  sprintf(sz,
    "%s%sX axis%s%s   %s%sY axis%s%s   %s%sZ axis%s%s   %s%sLength\n",
    sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1, sz1);
  PrintSz(sz);
  for (i = 0; i <= oNorm; i++) {
    if (ignore[i] || (!FThing(i) ||
      ((i == oMoo || i == oNod || i == oSou) && !us.fEphemFiles)))
      continue;
    AnsiColor(kObjA[i]);
    sprintf(sz, "%-4.4s:  ", szObjDisp[i]); PrintSz(sz);
    x = space[i].x; y = space[i].y; z = space[i].z;
    PrintZodiac(planet[i]);
    AnsiColor(kObjA[i]);
    sprintf(sz, is.fSeconds ? " %12.7f %12.7f %12.7f %12.7f" :
      " %8.3f %8.3f %8.3f %8.3f", x, y, z, RLength3(x, y, z)); PrintSz(sz);
    if (!is.fSeconds && i >= uranLo) {
      sprintf(sz, "  Uranian #%d", i-uranLo); PrintSz(sz);
    }
    PrintL();
  }

  /* Do another loop to print out the stars in their specified order. */

  if (us.nStar) for (i = starLo; i <= starHi; i++) {
    j = oNorm+starname[i-oNorm];
    if (ignore[j])
      continue;
    AnsiColor(kObjA[j]);
    sprintf(sz, "%-4.4s:  ", szObjDisp[j]); PrintSz(sz);
    x = space[j].x / rLYToAU;
    y = space[j].y / rLYToAU;
    z = space[j].z / rLYToAU;
    PrintZodiac(planet[j]);
    AnsiColor(kObjA[j]);
    sprintf(sz, is.fSeconds ? " %12.7f %12.7f %12.7f %12.7f" :
      " %8.3f %8.3f %8.3f %8.3f", x, y, z, RLength3(x, y, z)); PrintSz(sz);
    if (!is.fSeconds) {
      sprintf(sz, "  Star #%2d", i-oNorm); PrintSz(sz);
    }
    sprintf(sz, " %5.2f", rStarBright[j-oNorm]); PrintSz(sz);
    PrintL();
  }
  AnsiColor(kDefault);
}


/* Display a chart showing esoteric astrology information, as in the -7 */
/* switch. This includes Rays, esoteric rulerships, and Ray influences. */

void ChartEsoteric(void)
{
  char sz[cchSzDef], *pch;
  int nRay1[cRay+1], nRay2[cRay+1], rank1[cRay+1], rank2[cRay+1],
    rank[objMax], i, j, n, c, n1 = 0, n2 = 0;
  real power1[objMax], power2[objMax], rRay1[cRay+1], rRay2[cRay+1],
    power[objMax], r = 0.0, r1 = 0.0, r2 = 0.0;
  flag fIgnore7Sav[5], fRayRuler;

  /* Calculate data. */
  EnsureRay();
  for (i = 0; i <= cRay; i++) {
    nRay1[i] = nRay2[i] = i ? 0   : -1;
    rRay1[i] = rRay2[i] = i ? 0.0 : -1.0;
  }
  ComputeInfluence(power1, power2);
  for (i = 0; i <= cObj; i++) {
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
  SortRank(power, rank, cObj);
  SortRank(rRay1, rank1, cRay); SortRank(rRay2, rank2, cRay);

  /* Print planet table. */
  for (i = 0; i < 5; i++) {
    fIgnore7Sav[i] = ignore7[i];
    ignore7[i] = fFalse;
  }
  PrintSz("Body    Location Rulers House Rulers Power Rank Percent\n");
  for (i = 0; i <= cObj; i++) {
    if (FIgnore(i))
      continue;
    AnsiColor(kObjA[i]);
    sprintf(sz, "%-4.4s", szObjDisp[i]); PrintSz(sz);
    n = i <= oNorm ? rgObjRay[i] : 0;
    if (n) {
      AnsiColor(kRayA[n]);
      sprintf(sz, "%2d", n); PrintSz(sz);
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
    fRayRuler = (pch[4] == 'Y');
    sprintf(sz, " %s", pch); PrintSz(sz);
    n = inhouse[i];
    pch = Dignify(i, n)+1;
    fRayRuler &= (pch[4] == 'Y');
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
  for (i = 0; i < 5; i++)
    ignore7[i] = fIgnore7Sav[i];

  /* Print Ray table. */
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
}


/* Display locations of the planets on the screen with respect to the 36    */
/* Gauquelin sectors and their plus zones, as specified with the -l switch. */

void ChartSector(void)
{
  char sz[cchSzDef];
  CP cp;
  byte ignoreSav[objMax];
  int c[cSector + 1], i, sec, pls, kPls, cpls = 0, co = 0, cq = 0;
  real r, rT;

  for (i = 1; i <= cSector; i++) {
    c[i] = 0;
    cpls += pluszone[i];
  }
  cp = cp0;
  CopyRgb(ignore, ignoreSav, sizeof(ignore));
  CastSectors();

  AnsiColor(kDkGrayA);
  PrintSz("Body  Sector ");
  if (is.fSeconds)
    PrintSz(
      "  Plus      House   Sign Loc. Ret. Latitude Velocity Sec.18 Sec.12\n");
  else
    PrintSz("Plus    House   Sign Ret. Latit. Veloc. 18 12\n");
  for (i = 0; i <= cObj; i++) {
    if (FIgnore(i))
      continue;
    co++;
    AnsiColor(kObjA[i]);
    sprintf(sz, "%-4.4s: ", szObjDisp[i]); PrintSz(sz);
    r = GFromO(planet[i]);
    sec = (int)r + 1; c[sec]++;
    pls = pluszone[sec];
    cq += pls;
    kPls = (pls ? kRedA : kDkGreenA);
    AnsiColor(kDkGrayA);
    PrintSz("Sec");
    AnsiColor(kPls);
    sprintf(sz, " %2d", sec); PrintSz(sz);
    if (is.fSeconds) {
      AnsiColor(kDkGrayA);
      sprintf(sz, "%.3f", RFract(r)); PrintSz(&sz[1]);
      AnsiColor(kPls);
    }
    sprintf(sz, " %c", (char)(pls ? '+' : '-')); PrintSz(sz);
    AnsiColor(kSignA(cp.house[i]));
    sprintf(sz, " [%2d%s house] ", cp.house[i], szSuffix[cp.house[i]]);
    PrintSz(sz);
    PrintZodiac(cp.obj[i]);
    sprintf(sz, " %c ", cp.dir[i] < 0.0 ? chRet : ' '); PrintSz(sz);
    PrintAltitude(cp.alt[i]);
    PrintCh(' ');
    if (FThing(i)) {
      PrintCh((char)(ret[i] < 0.0 ? '-' : '+'));
      rT = RAbs(ret[i]);
      sprintf(sz, is.fSeconds ? (rT < 10.0 ? "%7.5f" : "%7.4f") :
        (rT < 10.0 ? "%5.3f" : "%5.2f"), rT); PrintSz(sz);
    } else
      PrintTab('_', us.fSeconds ? 8 : 6);
    AnsiColor(kPls);
    sprintf(sz, " %2d", (sec-1)/2 + 1); PrintSz(sz);
    if (is.fSeconds) {
      AnsiColor(kDkGrayA);
      sprintf(sz, "%.3f", RFract(r/2.0)); PrintSz(&sz[1]);
    }
    AnsiColor(kPls);
    sprintf(sz, " %2d", (sec-1)/3 + 1); PrintSz(sz);
    if (is.fSeconds) {
      AnsiColor(kDkGrayA);
      sprintf(sz, "%.3f", RFract(r/3.0)); PrintSz(&sz[1]);
    }
    PrintL();
  }

  /* Display summary information, i.e. the planet in plus zone ratio. */

  AnsiColor(kDefault);
  sprintf(sz, "\nPlus zones: %d/%d = %.2f%% - ", cpls, cSector,
    (real)cpls/(real)36*100.0); PrintSz(sz);
  sprintf(sz, "Planets in plus zones: %d/%d = %.2f%%\n", cq, co,
    co ? (real)cq/(real)co*100.0 : 0.0); PrintSz(sz);

  /* Display more summary information, i.e. the number of planets in each */
  /* sector, as well as whether each sector is a plus zone or not.        */

  PrintSz("\nZone:");
  for (i = 1; i <= cSector/2; i++) {
    pls = pluszone[i];
    AnsiColor(pls ? kRedA : kDkGreenA);
    sprintf(sz, " %2d%c", i, pls ? '+' : '-'); PrintSz(sz);
  }
  AnsiColor(kDefault);
  PrintSz("\nNum :");
  for (i = 1; i <= cSector/2; i++) {
    if (c[i]) {
      sprintf(sz, " %2d ", c[i]); PrintSz(sz);
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
    if (c[i]) {
      sprintf(sz, " %2d ", c[i]); PrintSz(sz);
    } else
      PrintSz("  . ");
  }
  PrintL();
  CopyRgb(ignoreSav, ignore, sizeof(ignore));
  CastChart(fTrue);
}


/* Print the locations of the astro-graph lines on the Earth as specified */
/* with the -L switch. This includes Midheaven and Nadir lines, zenith    */
/* positions, and locations of Ascendant and Descendant lines.            */

void ChartAstroGraph(void)
{
  CrossInfo *c;
  char sz[cchSzDef];
  real planet1[objMax], planet2[objMax], mc[objMax], ic[objMax],
    asc[objMax], des[objMax], asc1[objMax], des1[objMax],
    lo = Lon, longm, w, x, y, z, ad, oa, am, od, dm;
  int cCross = 0, tot = cObj, i, j, k, l, m, n;

  if (us.fLatitudeCross) {
    if ((c = (CrossInfo *)PAllocate(sizeof(CrossInfo),
      "crossing table")) == NULL)
      return;
  }

  for (i = 0; i <= cObj; i++) if (!ignore[i] || i == oMC) {
    planet1[i] = Tropical(i == oMC ? is.MC : planet[i]);
    planet2[i] = planetalt[i];             /* Calculate zenith location on */
    EclToEqu(&planet1[i], &planet2[i]);    /* Earth of each object.        */
  }

  /* Print header. */

  PrintSz("Object :");
  for (i = 0; i <= cObj; i++)
    if (!ignore[i] && FThing(i)) {
      AnsiColor(kObjA[i]);
      if (is.fSeconds) {
        sprintf(sz, " %-10.10s", szObjDisp[i]);
      } else
        sprintf(sz, " %.3s", szObjDisp[i]);
      PrintSz(sz);
    }
  AnsiColor(kDefault);
  PrintSz("\n------ :");
  for (i = 0; i <= tot; i++)
    if (!ignore[i] && FThing(i)) {
      PrintSz(" ###");
      if (is.fSeconds)
        PrintSz("#######");
    }

  /* Print the longitude locations of the Midheaven lines. */

  PrintSz("\nMidheav: ");
  if (lo < 0.0)
    lo += rDegMax;
  for (i = 0; i <= tot; i++)
    if (!ignore[i] && FThing(i)) {
      AnsiColor(kObjA[i]);
      x = planet1[oMC]-planet1[i];
      if (x < 0.0)
        x += rDegMax;
      if (x > rDegHalf)
        x -= rDegMax;
      z = lo + x;
      if (z > rDegHalf)
        z -= rDegMax;
      mc[i] = z;
      if (is.fSeconds) {
        sprintf(sz, "%s ", SzLocation(z, 0.0)); sz[11] = chNull;
      } else {
        sprintf(sz, "%3.0f%c", RAbs(z), z < 0.0 ? 'e' : 'w');
      }
      PrintSz(sz);
    }
  AnsiColor(kDefault);

  /* The Nadir lines are just always 180 degrees away from the Midheaven. */

  PrintSz("\nNadir  : ");
  for (i = 0; i <= tot; i++)
    if (!ignore[i] && FThing(i)) {
      AnsiColor(kObjA[i]);
      z = mc[i] + rDegHalf;
      if (z > rDegHalf)
        z -= rDegMax;
      ic[i] = z;
      if (is.fSeconds) {
        sprintf(sz, "%s ", SzLocation(z, 0.0)); sz[11] = chNull;
      } else {
        sprintf(sz, "%3.0f%c", RAbs(z), z < 0.0 ? 'e' : 'w');
      }
      PrintSz(sz);
    }
  AnsiColor(kDefault);

  /* Print the Zenith latitude locations. */

  PrintSz("\nZenith : ");
  for (i = 0; i <= tot; i++)
    if (!ignore[i] && FThing(i)) {
      AnsiColor(kObjA[i]);
      y = planet2[i];
      if (is.fSeconds) {
        sprintf(sz, " %s ", SzLocation(0.0, y));
        for (j = 1; sz[j] = sz[j+11]; j++)
          ;
      } else {
        sprintf(sz, "%3.0f%c", RAbs(y), y < 0.0 ? 's' : 'n');
      }
      PrintSz(sz);
      asc[i] = des[i] = asc1[i] = des1[i] = rLarge;
    }
  PrintL2();

  /* Now print the locations of Ascendant and Descendant lines. Since these */
  /* are curvy, we loop through the latitudes, and for each object at each  */
  /* latitude, print the longitude location of the line in question.        */

  longm = Mod(planet1[oMC] + lo);
  for (j = 89-(89 % us.nAstroGraphStep); j > -90; j -= us.nAstroGraphStep) {
    AnsiColor(kDefault);
    sprintf(sz, "Asc@%2d%c: ", j >= 0 ? j : -j, j < 0 ? 's' : 'n');
    PrintSz(sz);
    for (i = 0; i <= tot; i++)
      if (!ignore[i] && FThing(i)) {
      AnsiColor(kObjA[i]);
      ad = RTanD(planet2[i])*RTanD(j);
      if (ad*ad > 1.0) {
        PrintSz(" --");
        if (is.fSeconds)
          PrintSz("------ ");
        PrintCh(' ');
        asc1[i] = des1[i] = cp2.dir[i] = rLarge;
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
        asc1[i] = asc[i];
        asc[i] = z;
        cp2.dir[i] = ad;
        if (is.fSeconds) {
          sprintf(sz, "%s ", SzLocation(z, 0.0)); sz[11] = chNull;
        } else
          sprintf(sz, "%3.0f%c", RAbs(z), z < 0.0 ? 'e' : 'w');
        PrintSz(sz);
      }
    }

    /* Again, the Descendant position is related to the Ascendant's,  */
    /* being a mirror image, so it can be calculated somewhat easier. */

    AnsiColor(kDefault);
    sprintf(sz, "\nDsc@%2d%c: ", j >= 0 ? j : -j, j < 0 ? 's' : 'n');
    PrintSz(sz);
    for (i = 0; i <= tot; i++)
      if (!ignore[i] && FThing(i)) {
      AnsiColor(kObjA[i]);
      ad = cp2.dir[i];
      if (ad == rLarge) {
        PrintSz(" --");
        if (is.fSeconds)
          PrintSz("------ ");
        PrintCh(' ');
      } else {
        od = planet1[i] + DFromR(ad);
        dm = od + rDegQuad;
        z = longm-dm;
        if (z < 0.0)
          z += rDegMax;
        if (z > rDegHalf)
          z -= rDegMax;
        des1[i] = des[i];
        des[i] = z;
        if (is.fSeconds) {
          sprintf(sz, "%s ", SzLocation(z, 0.0)); sz[11] = chNull;
        } else
          sprintf(sz, "%3.0f%c", RAbs(z), z < 0.0 ? 'e' : 'w');
        PrintSz(sz);
      }
    }
    PrintL();

    /* Now, if the -L0 switch is in effect, then take these line positions, */
    /* which we saved in an array above as we were printing them, and       */
    /* calculate and print the latitude crossings.                          */

    if (us.fLatitudeCross)
      for (l = 0; l <= cObj; l++) {
        if (ignore[l] || !FThing(l))
          continue;
        for (k = 0; k <= cObj; k++) {
          if (ignore[k] || !FThing(k))
            continue;
          for (n = 0; n <= 1; n++) {
            x = n ? des1[l] : asc1[l];
            y = n ? des[l] : asc[l];
            for (m = 0; m <= 1; m++) {

            /* Check if Ascendant/Descendant cross Midheaven/Nadir. */

            z = m ? ic[k] : mc[k];
            if (cCross < MAXCROSS &&
              RAbs(x-y) < rDegHalf && RSgn(z-x) != RSgn(z-y)) {
              c->obj1[cCross] = (n ? -(l+1) : l+1);
              c->obj2[cCross] = (m ? -(k+1) : k+1);
              c->lat[cCross] = (real)j+5.0*RAbs(z-y)/RAbs(x-y);
              c->lon[cCross] = z;
              cCross++;
            }

            /* Check if Ascendant/Descendant cross another Asc/Des. */

            w = m ? des1[k] : asc1[k];
            z = m ? des[k] : asc[k];
            if (cCross < MAXCROSS && k > l &&
              RAbs(x-y)+RAbs(w-z) < rDegHalf && RSgn(w-x) != RSgn(z-y) &&
              !(k == oSou && l == oNod)) {
              c->obj1[cCross] = (n ? -(l+1) : l+1);
              c->obj2[cCross] = (m ? -(k+1) : k+1) + 200;
              c->lat[cCross] = (real)j+5.0*
                RAbs(y-z)/(RAbs(x-w)+RAbs(y-z));
              c->lon[cCross] = Min(x, y)+RAbs(x-y)*
                RAbs(y-z)/(RAbs(x-w)+RAbs(y-z));
              cCross++;
            }
          }
        }
      }
    }
  }
  if (!us.fLatitudeCross)
    return;
  PrintL();

  /* Now, print out all the latitude crossings found.  */
  /* First, sort them in order of decreasing latitude. */

  for (i = 1; i < cCross; i++) {
    j = i-1;
    while (j >= 0 && c->lat[j] < c->lat[j+1]) {
      SwapN(c->obj1[j], c->obj1[j+1]); SwapN(c->obj2[j], c->obj2[j+1]);
      SwapR(&c->lat[j], &c->lat[j+1]); SwapR(&c->lon[j], &c->lon[j+1]);
      j--;
    }
  }
  for (i = 0; i < cCross; i++) {
    j = NAbs(c->obj1[i]) - 1;
    AnsiColor(kObjA[j]);
    sprintf(sz, "%.3s ", szObjDisp[j]); PrintSz(sz);
    AnsiColor(kElemA[c->obj1[i] > 0 ? eFir : eAir]);
    PrintSz(c->obj1[i] > 0 ? "Ascendant " : "Descendant");
    AnsiColor(kWhiteA);
    PrintSz(" crosses ");
    j = NAbs(c->obj2[i] - (c->obj2[i] < 100 ? 0 : 200)) - 1;
    AnsiColor(kObjA[j]);
    sprintf(sz, "%.3s ", szObjDisp[j]); PrintSz(sz);
    AnsiColor(kElemA[c->obj2[i] < 100 ?
      (c->obj2[i] > 0 ? eEar : eWat) : (c->obj2[i] > 200 ? eFir : eAir)]);
    sprintf(sz, "%s ", c->obj2[i] < 100 ? (c->obj2[i] > 0 ? "Midheaven " :
      "Nadir     ") : (c->obj2[i] > 200 ? "Ascendant " : "Descendant"));
    PrintSz(sz);
    AnsiColor(kDefault);
    sprintf(sz, "at %s%c,", SzDegree(c->lon[i]),
      c->lon[i] < 0.0 ? 'E' : 'W'); PrintSz(sz);
    j = (int)(RFract(RAbs(c->lat[i]))*60.0);
    sprintf(sz, "%s%c\n", SzDegree(c->lat[i]),
      c->lat[i] < 0.0 ? 'S' : 'N'); PrintSz(sz);
  }
  DeallocateP(c);
  if (cCross <= 0) {
    AnsiColor(kDefault);
    PrintSz("No latitude crossings.\n");
  }
}


/* Another important procedure: Display any of the types of (text) charts    */
/* that the user specified they wanted, by calling the appropriate routines. */

void PrintChart(flag fProg)
{
  int fCall = fFalse, nSav;

  if (us.fListing) {
    if (is.fMult)
      PrintL2();
    if (us.nRel <= rcDual)
      ChartListingRelation();
    else if (us.nRel < rcDifference)
      ChartListing();
    else

      /* If the -rb or -rd relationship charts are in effect, then instead  */
      /* of doing the standard -v chart, print either of these chart types. */

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
      if (us.fGridConfig) {    /* If -g0 switch in effect, then  */
        PrintL();              /* display aspect configurations. */
        if (!fCall) {
          nSav = us.objRequire; us.objRequire = -1;
          FCreateGrid(fFalse);
          us.objRequire = nSav;
        }
        DisplayGrands();
      }
    } else {

      /* Do a relationship aspect grid between two charts if -r0 in effect. */

      fCall = us.fSmartCusp; us.fSmartCusp = fFalse;
      if (!FCreateGridRelation(us.fGridMidpoint))
        return;
      us.fSmartCusp = fCall;
      ChartGridRelation();
    }
    is.fMult = fTrue;
  }
  if (us.fAspList) {
    PrintHeader(is.fMult ? -1 : 1);
    if (us.nRel > rcDual) {
      if (!fCall) {
        fCall = fTrue;
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
      if (!fCall) {
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
  if (us.fInfluence) {
    PrintHeader(is.fMult ? -1 : 1);
    ChartInfluence();
    is.fMult = fTrue;
  }
  if (us.fEsoteric) {
    PrintHeader(is.fMult ? -1 : 1);
    ChartEsoteric();
    is.fMult = fTrue;
  }
  if (us.fAstroGraph) {
    PrintHeader(is.fMult ? -1 : 1);
    ChartAstroGraph();
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
  if (us.fHorizonSearch) {
    if (is.fMult)
      PrintL2();
    ChartInDayHorizon();
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
    ChartTransitGraph(fFalse, fProg);
    is.fMult = fTrue;
  }
  if (us.fEphemeris) {
    if (is.fMult)
      PrintL2();
    ChartEphemeris();
    is.fMult = fTrue;
  }
  if (us.fTransit) {
    if (is.fMult)
      PrintL2();
    ChartTransitSearch(fProg);
    is.fMult = fTrue;
  }
  if (us.fTransitInf) {
    PrintHeader(is.fMult ? -1 : 1);
    ChartTransitInfluence(fProg);
    is.fMult = fTrue;
  }
  if (us.fTransitGra) {
    if (is.fMult)
      PrintL2();
    ChartTransitGraph(fTrue, fProg);
    is.fMult = fTrue;
  }
#ifdef ARABIC
  if (us.nArabic) {
    PrintHeader(is.fMult ? -1 : 1);
    DisplayArabic();
    is.fMult = fTrue;
  }
#endif
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
    DisplayAtlasNearby(ciCore.lon, ciCore.lat, 0, NULL);
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

  if (!is.fMult) {          /* Assume the -v chart if user */
    us.fListing = fTrue;    /* didn't indicate anything.   */
    PrintChart(fProg);
    is.fMult = fTrue;
  }
}

/* charts1.cpp */
