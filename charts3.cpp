/*
** Astrolog (Version 7.70) File: charts3.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2024 by
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
** Last code change made 4/22/2024.
*/

#include "astrolog.h"


/*
******************************************************************************
** Multiple Chart Scanning Routines.
******************************************************************************
*/

#ifdef GRAPH
#define RgzCalendar() gi.rgzCalendar
#else
#define RgzCalendar() NULL
#endif

// Display a list of transit events. Called from ChartInDaySearch().

void PrintInDays(InDayInfo *pid, int occurcount, int counttotal, flag fProg)
{
  char sz[cchSzDef];
  int fVoid, nVoid, nSkip = 0, i, j, k, s1, s2, s3;
  CI ciCast = ciSave, ciEvent;
#ifdef EXPRESS
  int nEclipse;
  real rEclipse;
#endif
#ifdef GRAPH
  int nEclipse2;
#endif

  i = (RgzCalendar() == NULL ? -1 : occurcount);
  loop {
    i += (RgzCalendar() == NULL ? 1 : -1);
    if (!FBetween(i, 0, occurcount-1))
      break;

    // Detect whether this aspect indicates the Moon going void of course, and
    // if so how long the Moon is v/c before entering next sign. This requires
    // the sign change event to be in the same list as the Moon aspect.
    nVoid = -1;
    fVoid = FAspectVoid(pid[i].source, pid[i].dest, pid[i].aspect);
#ifdef EXPRESS
    if (RgzCalendar() != NULL &&
      (i >= occurcount || pid[i].day != pid[i+1].day))
      nSkip = 0;
    // Adjust whether this aspect can be going v/c if AstroExpression says so.
    if (!us.fExpOff && FSzSet(us.szExpVoid)) {
      ExpSetN(iLetterV, 0);
      ExpSetN(iLetterW, pid[i].source);
      ExpSetN(iLetterX, pid[i].aspect);
      ExpSetN(iLetterY, pid[i].dest);
      ExpSetN(iLetterZ, fVoid);
      ParseExpression(us.szExpVoid);
      fVoid = NExpGet(iLetterZ);
    }
#endif
    if (fVoid) {
      for (j = i+1; j < counttotal; j++) {
        fVoid = FAspectVoid(pid[j].source, pid[j].dest, pid[j].aspect);
#ifdef EXPRESS
        // Adjust whether aspect can be going v/c if AstroExpression says so.
        if (!us.fExpOff && FSzSet(us.szExpVoid)) {
          ExpSetN(iLetterV, 0);
          ExpSetN(iLetterW, pid[j].source);
          ExpSetN(iLetterX, pid[j].aspect);
          ExpSetN(iLetterY, pid[j].dest);
          ExpSetN(iLetterZ, fVoid);
          ParseExpression(us.szExpVoid);
          fVoid = NExpGet(iLetterZ);
        }
#endif
        if (fVoid)
          break;
        fVoid = (pid[j].aspect == aSig && pid[j].source == oMoo);
#ifdef EXPRESS
        // Adjust whether event can be ending v/c if AstroExpression says so.
        if (!us.fExpOff && FSzSet(us.szExpVoid)) {
          ExpSetN(iLetterV, 1);
          ExpSetN(iLetterW, pid[j].source);
          ExpSetN(iLetterX, pid[j].aspect);
          ExpSetN(iLetterY, pid[j].dest);
          ExpSetN(iLetterZ, fVoid);
          ParseExpression(us.szExpVoid);
          fVoid = NExpGet(iLetterZ);
        }
#endif
        if (fVoid) {
          nVoid = pid[j].day - pid[i].day;
          if (nVoid < 0)
            nVoid += DayInMonth(pid[i].mon, pid[i].yea);
          nVoid *= 24*60*60;
          nVoid += (int)((pid[j].time - pid[i].time) * 60.0);
          break;
        }
      }
    }

    // Display the current transit event.
    s1 = (int)pid[i].time/60;
    s2 = (int)pid[i].time-s1*60;
    s3 = us.fSeconds ? (int)(pid[i].time*60.0)-((s1*60+s2)*60) : -1;
    SetCI(ciCast, pid[i].mon, pid[i].day, pid[i].yea,
      pid[i].time / 60.0, Dst, Zon, Lon, Lat);
    if ((!us.fExpOff && FSzSet(us.szExpDay)) || (us.fEclipse &&
      !us.fParallel && (pid[i].aspect == aCon || pid[i].aspect == aOpp))) {
      ciCore = ciCast;
      CastChart(-1);
    }
#ifdef EXPRESS
    // May want to skip current event if AstroExpression says to do so.
    if (!us.fExpOff && FSzSet(us.szExpDay)) {
      nEclipse = NCheckEclipseAny(pid[i].source, pid[i].aspect, pid[i].dest,
        &rEclipse);
      ExpSetN(iLetterU, pid[i].source);
      ExpSetN(iLetterV, pid[i].aspect);
      ExpSetN(iLetterW, pid[i].dest);
      ExpSetN(iLetterX, nVoid);
      ExpSetN(iLetterY, nEclipse);
      ExpSetR(iLetterZ, rEclipse);
      if (!NParseExpression(us.szExpDay)) {
        nSkip++;
        continue;
      }
    }
#endif
    ciSave = ciCast;
#ifdef GRAPH
    // May want to draw current event within a graphic calendar box.
    if (RgzCalendar() != NULL) {
      for (j = i; j >= 0 && pid[j].day == pid[i].day; j--)
        ;
      for (k = i; k < occurcount && pid[k].day == pid[i].day; k++)
        ;
      nEclipse2 = NCheckEclipseAny(pid[i].source, pid[i].aspect, pid[i].dest,
        NULL);
      DrawCalendarAspect(&pid[i], i-j+nSkip, k-j, nVoid, nEclipse2);
      continue;
    }
#endif
    if (us.fListAuto) {
      // May want to add chart for current event to chart list.
      ciEvent = ciSave;
      *sz = chNull;
      k = pid[i].aspect;
      if (k > 0)
        sprintf(sz, "%s %s %s", szObjDisp[pid[i].source],
          SzAspectAbbrev(pid[i].aspect), szObjDisp[pid[i].dest]);
      else if (k == aSig)
        sprintf(sz, "%s enters %s", szObjDisp[pid[i].source],
          szSignName[pid[i].dest]);
      else if (k == aDir)
        sprintf(sz, "%s S%c", szObjDisp[pid[i].source],
          pid[i].dest ? chRet : 'D');
      else if (k == aDeg)
        sprintf(sz, "%s at %s", szObjDisp[pid[i].source], SzZodiac(
          (real)pid[i].dest * (rDegMax / (real)(cSign * us.nSignDiv))));
      else if (k == aAlt)
        sprintf(sz, "%s LA%c", szObjDisp[pid[i].source],
          pid[i].dest ? '+' : '-');
      else if (k == aLen)
        sprintf(sz, "%s %s", szObjDisp[pid[i].source],
          pid[i].dest ? "Apo" : "Per");
      else if (k == aNod)
        sprintf(sz, "%s LA0 %s", szObjDisp[pid[i].source],
          pid[i].dest ? "South" : "North");
      else if (k == aDis)
        sprintf(sz, "%s distance equal %s", szObjDisp[pid[i].source],
          szObjDisp[pid[i].dest]);
      else
        Assert(fFalse);
      ciEvent.nam = SzClone(sz);
      ciEvent.loc = ciDefa.loc;
      FAppendCIList(&ciEvent);
    }
    k = DayOfWeek(pid[i].mon, pid[i].day, pid[i].yea);
    AnsiColor(kRainbowA[k + 1]);
    sprintf(sz, "%.3s ", szDay[k]); PrintSz(sz);
    AnsiColor(kDefault);
    sprintf(sz, "%s %s ",
      SzDate(pid[i].mon, pid[i].day, pid[i].yea, fFalse),
      SzTime(s1, s2, s3)); PrintSz(sz);
    PrintAspect(pid[i].source, pid[i].pos1, pid[i].ret1, pid[i].aspect,
      pid[i].dest, pid[i].pos2, pid[i].ret2, fProg ? 'e' : 'd');
    PrintInDayEvent(pid[i].source, pid[i].aspect, pid[i].dest, nVoid);
  }
}


// Search through a day or longer period, and print out the times of exact
// aspects among planets during that day, as specified with the -d switch,
// as well as times when planets changes sign or direction. To do this, cast
// charts for the beginning and end of the day (or a part of a day) and do a
// linear equation check to see if anything happens during the interval.

void ChartInDaySearch(flag fProg)
{
  InDayInfo id[MAXINDAY], idT, *pid = id;
  int yea0, yea1, yea2, mon0, mon1, mon2, day0, day1, day2, counttotal = 0,
    occurcount, maxinday = MAXINDAY, division, div, divSign,
    i, j, k, l, s1, s2;
  real divsiz, d1, d2, e1, e2, f1, f2, g;
  flag fYear, fVoid, fPrint = fTrue;
  CP cpA, cpB;

  // If parameter 'fProg' is set, look for changes in a progressed chart.

#ifdef GRAPH
  fPrint &= (RgzCalendar() == NULL);
#endif
  fYear = us.fInDayMonth && us.fInDayYear;
  fVoid = !FIgnore(oMoo) && !us.fIgnoreSign && us.fInDayMonth;
  division = (fYear || fProg) ? (us.nDivision + 9) / 10 : us.nDivision;
  divsiz = 24.0 / (real)division*60.0;
  divSign = cSign * us.nSignDiv;
  if (us.fListAuto)
    is.cci = 0;

  // If -dY in effect, then search through a range of years.

  yea1 = yea2 = !fProg ? Yea : YeaT;
  if (fYear && us.nEphemYears != 0) {
    if (us.nEphemYears < 0)
      yea1 += (us.nEphemYears + 1);
    else
      yea2 += (us.nEphemYears - 1);
  }
  for (yea0 = yea1; yea0 <= yea2; yea0++) {

  // If -dy in effect, then search through the whole year, month by month.

  if (fYear) {
    mon1 = 1; mon2 = 12;
  } else
    mon1 = mon2 = !fProg ? Mon : MonT;

  // Start searching the month or months in question for exciting events.

  for (mon0 = mon1; mon0 <= mon2; mon0++) {
    if (us.fInDayMonth) {
      day1 = 1;
      day2 = DayInMonth(mon0, yea0);
    } else
      day1 = day2 = !fProg ? Day : DayT;

  // Start searching the day or days in question for exciting events.

  for (day0 = day1; day0 <= day2; day0 = AddDay(mon0, day0, yea0, 1)) {
    occurcount = 0;
    maxinday = MAXINDAY - (int)(pid - id);

    // Cast chart for beginning of day and store it for future use.

    SetCI(ciCore, mon0, day0, yea0, 0.0, Dst, Zon, Lon, Lat);
    us.fProgress = fProg;
    if (fProg) {
      is.JDp = MdytszToJulian(mon0, day0, yea0, 0.0, Dst, Zon);
      ciCore = ciMain;
    }
    CastChart(-1);
    cpB = cp0;

    // Now divide the day into segments and search each segment in turn.
    // More segments is slower, but has slightly better time accuracy.

    for (div = 1; div <= division; div++) {

      // Cast the chart for the ending time of the present segment. The
      // beginning time chart is copied from the previous end time chart.

      SetCI(ciCore, mon0, day0, yea0,
        24.0*(real)div/(real)division, Dst, Zon, Lon, Lat);
      if (fProg) {
        is.JDp = MdytszToJulian(mon0, day0, yea0, TT, Dst, Zon);
        ciCore = ciMain;
      }
      CastChart(-1);
      cpA = cpB; cpB = cp0;

      // Now search through the present segment for anything exciting.

      for (i = 0; i <= is.nObj; i++)
        if (!FIgnore(i) && (fProg || us.fGraphAll || FThing(i))) {
        s1 = SFromZ(cpA.obj[i])-1;
        s2 = SFromZ(cpB.obj[i])-1;

        // Does the current planet change into the next or previous sign?

        if (!us.fIgnoreSign && FAllow(i) && occurcount < maxinday) {
          l = NAbs(s1-s2);
          if (s1 != s2 && (l == 1 || l == cSign-1)) {
            pid[occurcount].source = i;
            pid[occurcount].aspect = aSig;
            pid[occurcount].dest = s2+1;
            pid[occurcount].time = MinDistance(cpA.obj[i],
              (real)(cpA.dir[i] >= 0.0 ? s2 : s1) * 30.0) / MinDistance(
              cpA.obj[i], cpB.obj[i])*divsiz + (real)(div-1)*divsiz;
            pid[occurcount].pos1 = pid[occurcount].pos2 = ZFromS(s1+1);
            pid[occurcount].ret1 = cpA.dir[i];
            pid[occurcount].ret2 = cpB.dir[i];
            pid[occurcount].mon = mon0;
            pid[occurcount].day = day0;
            pid[occurcount].yea = yea0;
            occurcount++;

          // Does the current planet change into next or previous degree?

          } else if (us.nSignDiv > 1) {
            j = (int)(cpA.obj[i] / (rDegMax / (real)divSign));
            k = (int)(cpB.obj[i] / (rDegMax / (real)divSign));
            l = NAbs(j-k);
            if (j != k && (l == 1 || l == divSign-1)) {
              l = k;
              if (j == k+1 || j == k-(divSign-1))
                l = j;
              pid[occurcount].source = i;
              pid[occurcount].aspect = aDeg;
              pid[occurcount].dest = l;
              pid[occurcount].time = MinDistance(cpA.obj[i],
                (real)l * (rDegMax / (real)divSign)) / MinDistance(cpA.obj[i],
                cpB.obj[i])*divsiz + (real)(div-1)*divsiz;
              pid[occurcount].pos1 = pid[occurcount].pos2 = cpA.obj[i];
              pid[occurcount].ret1 = pid[occurcount].ret2 =
                (l == k) ? 1.0 : -1.0;
              pid[occurcount].mon = mon0;
              pid[occurcount].day = day0;
              pid[occurcount].yea = yea0;
              occurcount++;
            }
          }
        }

        // Does the current planet go retrograde or direct?

        if (!us.fIgnoreDir && (cpA.dir[i] < 0.0) != (cpB.dir[i] < 0.0) &&
          FAllow(i) && occurcount < maxinday) {
          pid[occurcount].source = i;
          pid[occurcount].aspect = aDir;
          pid[occurcount].dest = cpB.dir[i] < 0.0;
          pid[occurcount].time = RAbs(cpA.dir[i])/(RAbs(cpA.dir[i])+
            RAbs(cpB.dir[i]))*divsiz + (real)(div-1)*divsiz;
          pid[occurcount].pos1 = pid[occurcount].pos2 =
            RAbs(cpA.dir[i])/(RAbs(cpA.dir[i])+RAbs(cpB.dir[i])) *
            (cpB.obj[i]-cpA.obj[i]) + cpA.obj[i];
          pid[occurcount].ret1 = pid[occurcount].ret2 = 0.0;
          pid[occurcount].mon = mon0;
          pid[occurcount].day = day0;
          pid[occurcount].yea = yea0;
          occurcount++;
        }

        // Does the current planet reach maximum or minimum latitude?

        if (!us.fIgnoreDiralt && (cpA.diralt[i] < 0.0) != (cpB.diralt[i] < 0.0)
          && FAllow(i) && occurcount < maxinday) {
          pid[occurcount].source = i;
          pid[occurcount].aspect = aAlt;
          pid[occurcount].dest = cpB.diralt[i] < 0.0;
          pid[occurcount].time = RAbs(cpA.diralt[i])/(RAbs(cpA.diralt[i])+
            RAbs(cpB.diralt[i]))*divsiz + (real)(div-1)*divsiz;
          pid[occurcount].pos1 = pid[occurcount].pos2 =
            RAbs(cpA.diralt[i])/(RAbs(cpA.diralt[i])+RAbs(cpB.diralt[i])) *
            (cpB.alt[i]-cpA.alt[i]) + cpA.alt[i];
          pid[occurcount].ret1 = cpA.dir[i]; pid[occurcount].ret2 = cpB.dir[i];
          pid[occurcount].mon = mon0;
          pid[occurcount].day = day0;
          pid[occurcount].yea = yea0;
          occurcount++;
        }

        // Does the current planet reach maximum or minimum distance?

        if (!us.fIgnoreDirlen && (cpA.dirlen[i] < 0.0) != (cpB.dirlen[i] < 0.0)
          && FAllow(i) && occurcount < maxinday) {
          pid[occurcount].source = i;
          pid[occurcount].aspect = aLen;
          pid[occurcount].dest = (cpB.dirlen[i] < 0.0);
          pid[occurcount].time = RAbs(cpA.dirlen[i])/(RAbs(cpA.dirlen[i])+
            RAbs(cpB.dirlen[i]))*divsiz + (real)(div-1)*divsiz;
          pid[occurcount].pos1 = pid[occurcount].pos2 =
            RAbs(cpA.dirlen[i])/(RAbs(cpA.dirlen[i])+RAbs(cpB.dirlen[i])) *
            (cpB.obj[i]-cpA.obj[i]) + cpA.obj[i];
          pid[occurcount].ret1 = cpA.dir[i]; pid[occurcount].ret2 = cpB.dir[i];
          pid[occurcount].mon = mon0;
          pid[occurcount].day = day0;
          pid[occurcount].yea = yea0;
          occurcount++;
        }

        // Does the current planet cross zero latitude?

        if (!us.fIgnoreAlt0 && ((cpA.alt[i] < 0.0 && cpB.alt[i] >= 0.0) ||
          (cpA.alt[i] >= 0.0 && cpB.alt[i] < 0.0)) &&
          FAllow(i) && occurcount < maxinday) {
          pid[occurcount].source = i;
          pid[occurcount].aspect = aNod;
          pid[occurcount].dest = (cpA.alt[i] >= 0.0);
          pid[occurcount].time = cpA.alt[i]/(cpA.alt[i]-cpB.alt[i])*divsiz +
            (real)(div-1)*divsiz;
          pid[occurcount].pos1 = pid[occurcount].pos2 =
            Mod(cpA.obj[i] + cpA.alt[i]/(cpA.alt[i]-cpB.alt[i]) *
            MinDifference(cpA.obj[i], cpB.obj[i]));
          pid[occurcount].ret1 = cpA.dir[i]; pid[occurcount].ret2 = cpB.dir[i];
          pid[occurcount].mon = mon0;
          pid[occurcount].day = day0;
          pid[occurcount].yea = yea0;
          occurcount++;
        }

        // Now search for anything making an aspect to the current planet.

        for (j = i+1; j <= is.nObj; j++)
          if (!FIgnore(j) && (fProg || us.fGraphAll || FThing(j))) {
          if (!us.fParallel) {

          for (k = 1; k <= us.nAsp; k++) if (FAcceptAspect(i, -k, j)) {
            d1 = cpA.obj[i]; d2 = cpB.obj[i];
            e1 = cpA.obj[j]; e2 = cpB.obj[j];
            if (MinDistance(d1, d2) < MinDistance(e1, e2)) {
              SwapR(&d1, &e1);
              SwapR(&d2, &e2);
            }

            // Search each potential aspect in turn. First subtract the size
            // of the aspect from the angular difference, so can then treat it
            // like a conjunction.

            if (MinDistance(e1, Mod(d1-rAspAngle[k])) <
                MinDistance(e2, Mod(d2+rAspAngle[k]))) {
              e1 = Mod(e1+rAspAngle[k]);
              e2 = Mod(e2+rAspAngle[k]);
            } else {
              e1 = Mod(e1-rAspAngle[k]);
              e2 = Mod(e2-rAspAngle[k]);
            }

            // Check to see if the aspect actually occurs during this segment,
            // making sure to take into account if one or both planets are
            // retrograde or if they cross the Aries point.

            f1 = e1-d1;
            if (RAbs(f1) > rDegHalf)
              f1 -= RSgn(f1)*rDegMax;
            f2 = e2-d2;
            if (RAbs(f2) > rDegHalf)
              f2 -= RSgn(f2)*rDegMax;
            if (MinDistance(Midpoint(d1, d2), Midpoint(e1, e2)) < rDegQuad &&
              RSgn(f1) != RSgn(f2) && occurcount < maxinday) {
              pid[occurcount].source = i;
              pid[occurcount].aspect = k;
              pid[occurcount].dest = j;
              pid[occurcount].mon = mon0;
              pid[occurcount].day = day0;
              pid[occurcount].yea = yea0;

              // Horray! The aspect occurs sometime during the interval. Now
              // just have to solve an equation in two variables to find out
              // where their "lines" of motion cross, i.e. the aspect's time.

              f1 = d2-d1;
              if (RAbs(f1) > rDegHalf)
                f1 -= RSgn(f1)*rDegMax;
              f2 = e2-e1;
              if (RAbs(f2) > rDegHalf)
                f2 -= RSgn(f2)*rDegMax;
              g = (RAbs(d1-e1) > rDegHalf ?
                (d1-e1)-RSgn(d1-e1)*rDegMax : d1-e1)/(f2-f1);
              pid[occurcount].time = g*divsiz + (real)(div-1)*divsiz;
              pid[occurcount].pos1 = Mod(cpA.obj[i] +
                RSgn(cpB.obj[i]-cpA.obj[i])*
                (RAbs(cpB.obj[i]-cpA.obj[i]) > rDegHalf ? -1 : 1)*
                RAbs(g)*MinDistance(cpA.obj[i], cpB.obj[i]));
              pid[occurcount].pos2 = Mod(cpA.obj[j] +
                RSgn(cpB.obj[j]-cpA.obj[j])*
                (RAbs(cpB.obj[j]-cpA.obj[j]) > rDegHalf ? -1 : 1)*
                RAbs(g)*MinDistance(cpA.obj[j], cpB.obj[j]));
              pid[occurcount].ret1 = (cpA.dir[i] + cpB.dir[i]) / 2.0;
              pid[occurcount].ret2 = (cpA.dir[j] + cpB.dir[j]) / 2.0;
              occurcount++;
            }
          }

          } else {

          for (k = 1; k <= Min(us.nAsp, aOpp); k++)
            if (FAcceptAspect(i, -k, j)) {

            d1 = cpA.alt[i]; d2 = cpB.alt[i];
            e1 = cpA.alt[j]; e2 = cpB.alt[j];
            if (!us.fEquator2 && !us.fParallel2) {
              // If have ecliptic latitude and want declination, convert.
              g = cpA.obj[i]; EclToEqu(&g, &d1);
              g = cpB.obj[i]; EclToEqu(&g, &d2);
              g = cpA.obj[j]; EclToEqu(&g, &e1);
              g = cpB.obj[j]; EclToEqu(&g, &e2);
            } else if (us.fEquator2 && us.fParallel2) {
              // If have equatorial declination and want latitude, convert.
              g = cpA.obj[i]; EquToEcl(&g, &d1);
              g = cpB.obj[i]; EquToEcl(&g, &d2);
              g = cpA.obj[j]; EquToEcl(&g, &e1);
              g = cpB.obj[j]; EquToEcl(&g, &e2);
            }

            // Search each potential aspect in turn. Negate the sign of the
            // aspect if needed, so can then treat it like a parallel.

            if (k == aOpp) {
              neg(e1);
              neg(e2);
            }

            // Check if the aspect actually occurs during this segment, making
            // sure to take into account if one or both planets are retrograde.

            f1 = e1-d1;
            f2 = e2-d2;
            if (RSgn(f1) != RSgn(f2) && occurcount < maxinday) {
              pid[occurcount].source = i;
              pid[occurcount].aspect = k;
              pid[occurcount].dest = j;
              pid[occurcount].mon = mon0;
              pid[occurcount].day = day0;
              pid[occurcount].yea = yea0;

              // Horray! The aspect occurs sometime during the interval. Now
              // just have to solve an equation in two variables to find out
              // where their "lines" of motion cross, i.e. the aspect's time.

              f1 = d2-d1;
              f2 = e2-e1;
              g = (d1-e1)/(f2-f1);
              if (k == aOpp) {
                neg(e1);
                neg(e2);
              }
              pid[occurcount].time = g*divsiz + (real)(div-1)*divsiz;
              pid[occurcount].pos1 = d1 + (d2 - d1)*g;
              pid[occurcount].pos2 = e1 + (e2 - e1)*g;
              pid[occurcount].ret1 = (cpA.diralt[i] + cpB.diralt[i]) / 2.0;
              pid[occurcount].ret2 = (cpA.diralt[j] + cpB.diralt[j]) / 2.0;
              occurcount++;
            }
          }
          } // us.fParallel

          // Check for planet pairs equidistant from each other.

          if (!us.fIgnoreDisequ) {
            d1 = cpA.dist[i]; d2 = cpB.dist[i];
            e1 = cpA.dist[j]; e2 = cpB.dist[j];
            f1 = e1-d1; f2 = e2-d2;
            if (RSgn(f1) != RSgn(f2) && occurcount < maxinday) {
              pid[occurcount].source = i;
              pid[occurcount].aspect = aDis;
              pid[occurcount].dest = j;
              pid[occurcount].mon = mon0;
              pid[occurcount].day = day0;
              pid[occurcount].yea = yea0;
              f1 = d2-d1; f2 = e2-e1;
              g = (d1-e1)/(f2-f1);
              pid[occurcount].time = g*divsiz + (real)(div-1)*divsiz;
              pid[occurcount].pos1 = Mod(cpA.obj[i] +
                RSgn(cpB.obj[i]-cpA.obj[i])*
                (RAbs(cpB.obj[i]-cpA.obj[i]) > rDegHalf ? -1 : 1)*
                RAbs(g)*MinDistance(cpA.obj[i], cpB.obj[i]));
              pid[occurcount].pos2 = Mod(cpA.obj[j] +
                RSgn(cpB.obj[j]-cpA.obj[j])*
                (RAbs(cpB.obj[j]-cpA.obj[j]) > rDegHalf ? -1 : 1)*
                RAbs(g)*MinDistance(cpA.obj[j], cpB.obj[j]));
              pid[occurcount].ret1 = (cpA.dir[i] + cpB.dir[i]) / 2.0;
              pid[occurcount].ret2 = (cpA.dir[j] + cpB.dir[j]) / 2.0;
              occurcount++;
            }
          }
        }
      } // i
    } // div

    // After all the aspects and evemts in the day have been located, sort
    // them by time at which they occur, so can print them in order.

    for (i = 1; i < occurcount; i++) {
      j = i-1;
      while (j >= 0 && pid[j].time > pid[j+1].time) {
        idT = pid[j]; pid[j] = pid[j+1]; pid[j+1] = idT;
        j--;
      }
    }

    // Finally, loop through and display each aspect and when it occurs.

    if (!fVoid || (day0 >= day2 && mon0 >= mon2 && yea0 >= yea2)) {
      // If no v/c aspects, or reached end of period, output all at once.
      if (fVoid) {
        occurcount += (int)(pid - id);
        pid = id;
      }
      PrintInDays(id, occurcount, occurcount, fProg);
    } else {
      // Output a chunk of events, knowing there's more to come.
      pid += occurcount;
      j = MAXINDAY >> 2;
      if (pid - id > j << 1) {
#ifdef GRAPH
        if (RgzCalendar() != NULL && id[j].day != id[0].day) {
          // Don't split day when drawing within calendar boxes.
          while (j > 0 && id[j].day == id[j-1].day)
            j--;
        }
#endif
        PrintInDays(id, j, pid - id, fProg);
        for (i = 0; i < pid - id - j; i++)
          id[i] = id[i + j];
        pid -= j;
      }
    }
    if (occurcount >= maxinday && fPrint)
      PrintSz("Too many transit events found.\n");
    counttotal += occurcount;
  } // day0
  } // mon0
  } // yea0
  if (counttotal == 0 && fPrint)
    PrintSz("No transit events found.\n");

  // Recompute original chart placements as have overwritten them.

  ciCore = ciMain;
  CastChart(1);
}


// Search through a month, year, or years, and print out the times of exact
// transits where planets in the time frame make aspect to the planets in
// some other chart, as specified with the -t switch. To do this, cast charts
// for the start and end of each month, or within a month, and do an equation
// check for aspects to the other base chart during the interval.

void ChartTransitSearch(flag fProg)
{
  TransInfo ti[MAXINDAY], tiT, *pti;
  char sz[cchSzDef];
  int M1, M2, Y1, Y2, counttotal = 0, occurcount, division, div, nAsp, fNoCusp,
    nSkip = 0, i, j, k, s1, s2, s3, s4, s1prev = 0;
  real divsiz, daysiz, d, e1, e2, f1, f2, mc = is.MC, ob = is.OB;
  flag fPrint = fTrue;
  CP cpA, cpB, cpN = cp0;
  CI ciSav, ciCast = ciSave, ciEvent;

  // Save away natal chart and initialize things.

#ifdef GRAPH
  InDayInfo idT;
  fPrint &= (RgzCalendar() == NULL);
#endif
  ciSav = ciTran;
  if (fProg)
    fNoCusp = fFalse;
  else {
    fNoCusp = fTrue;
    for (i = cuspLo; i <= cuspHi; i++)
      fNoCusp &= ignore2[i];
  }
  division = us.nDivision;
  if (!fProg && !fNoCusp)
    division = Max(division, 96);
  nAsp = is.fReturn ? aCon : us.nAsp;
  if (us.fParallel)
    nAsp = Min(nAsp, aOpp);
  if (us.fListAuto)
    is.cci = 0;

  Y1 = Y2 = YeaT;
  M1 = M2 = MonT;
  if (us.fInDayYear) {
    M1 = 1; M2 = 12;
    if (us.nEphemYears != 0) {
      if (us.nEphemYears < 0)
        Y1 += (us.nEphemYears + 1);
      else
        Y2 += (us.nEphemYears - 1);
    }
  }

  // Start searching the year or years in question for any transits.

  for (YeaT = Y1; YeaT <= Y2; YeaT++)

  // Start searching the month or months in question for any transits.

  for (MonT = M1; MonT <= M2; MonT++) {
    occurcount = 0; pti = ti;
    daysiz = (real)(us.fInDayMonth ? DayInMonth(MonT, YeaT) : 1)*24.0*60.0;
    divsiz = daysiz / (real)division;

    // Cast chart for beginning of month and store it for future use.

    SetCI(ciCore, MonT, us.fInDayMonth ? 1 : DayT, YeaT, 0.0, DstT, ZonT,
      LonT, LatT);
    if (us.fProgress = fProg) {
      is.JDp = MdytszToJulian(MM, DD, YY, TT, SS, ZZ);
      ciCore = ciMain;
    }
    for (i = 0; i <= is.nObj; i++)
      SwapN(ignore[i], ignore2[i]);
    CastChart(-1);
    for (i = 0; i <= is.nObj; i++)
      SwapN(ignore[i], ignore2[i]);
    cpB = cp0;

    // Divide month into segments and then search each segment in turn.

    for (div = 1; div <= division; div++) {
      if (fPrint) {
        occurcount = 0; pti = ti;
      }

      // Cast the chart for the ending time of the present segment, and copy
      // the start time chart from the previous end time chart.

      d = (us.fInDayMonth ? 1.0 : (real)DayT) +
        (daysiz/24.0/60.0)*(real)div/(real)division;
      SetCI(ciCore, MonT, (int)d, YeaT, RFract(d)*24.0,
        DstT, ZonT, LonT, LatT);
      if (fProg) {
        is.JDp = MdytszToJulian(MM, DD, YY, TT, SS, ZZ);
        ciCore = ciMain;
      }
      for (i = 0; i <= oNorm; i++)
        SwapN(ignore[i], ignore2[i]);
      CastChart(-1);
      for (i = 0; i <= oNorm; i++)
        SwapN(ignore[i], ignore2[i]);
      cpA = cpB; cpB = cp0;

      // Now search through the present segment for any transits. Note that
      // stars can be transited, but they can't make transits themselves.

      for (i = 0; i <= is.nObj; i++) {

        // Check if 3D house change occurs during time segment.

        if (us.fHouse3D && !us.fIgnoreSign && !FIgnore2(i)) {
          is.MC = mc; is.OB = ob;
          e1 = cpA.obj[i]; f1 = RHousePlaceIn3D(e1, cpA.alt[i]);
          e2 = cpB.obj[i]; f2 = RHousePlaceIn3D(e2, cpB.alt[i]);
          s1 = SFromZ(f1)-1; s2 = SFromZ(f2)-1;
          k = NAbs(s1-s2);
          if (s1 != s2 && (k == 1 || k == cSign-1) && !FIgnore(cuspLo+s2) &&
            occurcount < MAXINDAY) {
            pti->source = i;
            pti->aspect = aHou;
            pti->dest = s2+1;
            pti->time = MinDistance(f1,
              (real)(cpA.dir[i] >= 0.0 ? s2 : s1) * 30.0) /
              MinDistance(f1, f2)*divsiz + (real)(div-1)*divsiz;
            pti->posT = cpA.obj[i];
            pti->posN = cpN.obj[i];
            pti->retT = (cpA.dir[i] + cpB.dir[i]) / 2.0;
            occurcount++, pti++;
          }
        }

        if (FIgnore(i))
          continue;
        for (j = 0; j <= oNorm; j++) {
          if ((is.fReturn ? i != j : FIgnore2(j)) || (fNoCusp && !FThing(j)))
            continue;

          // Between each pair of planets, check if they make any aspects.

          if (!us.fParallel) {

          for (k = 1; k <= nAsp; k++) if (FAcceptAspect(i, k, j)) {
            d = cpN.obj[i]; e1 = cpA.obj[j]; e2 = cpB.obj[j];
            if (MinDistance(e1, Mod(d-rAspAngle[k])) <
                MinDistance(e2, Mod(d+rAspAngle[k]))) {
              e1 = Mod(e1+rAspAngle[k]);
              e2 = Mod(e2+rAspAngle[k]);
            } else {
              e1 = Mod(e1-rAspAngle[k]);
              e2 = Mod(e2-rAspAngle[k]);
            }

            // Check to see if the present aspect actually occurs during the
            // segment, making sure we check any Aries point crossings.

            f1 = e1-d;
            if (RAbs(f1) > rDegHalf)
              f1 -= RSgn(f1)*rDegMax;
            f2 = e2-d;
            if (RAbs(f2) > rDegHalf)
              f2 -= RSgn(f2)*rDegMax;
            if (MinDistance(d, Midpoint(e1, e2)) < rDegQuad &&
              RSgn(f1) != RSgn(f2) && occurcount < MAXINDAY) {

              // Ok, have found a transit! Now determine the time and save
              // this transit in our list to be printed.

              pti->source = j;
              pti->aspect = k;
              pti->dest = i;
              pti->time = RAbs(f1)/(RAbs(f1)+RAbs(f2))*divsiz +
                (real)(div-1)*divsiz;
              pti->posT = Mod(MinDistance(cpA.obj[j], Mod(d-rAspAngle[k])) <
                              MinDistance(cpB.obj[j], Mod(d+rAspAngle[k])) ?
                d-rAspAngle[k] : d+rAspAngle[k]);
              pti->posN = cpN.obj[i];
              pti->retT = (cpA.dir[j] + cpB.dir[j]) / 2.0;
              occurcount++, pti++;
            }
          }

          } else {

          for (k = 1; k <= nAsp; k++) if (FAcceptAspect(i, k, j)) {
            d = cpN.alt[i]; e1 = cpA.alt[j]; e2 = cpB.alt[j];
            if (!us.fEquator2 && !us.fParallel2) {
              // If have ecliptic latitude and want declination, convert.
              f1 = cpN.obj[i]; EclToEqu(&f1, &d);
              f1 = cpA.obj[j]; EclToEqu(&f1, &e1);
              f2 = cpB.obj[j]; EclToEqu(&f2, &e2);
            } else if (us.fEquator2 && us.fParallel2) {
              // If have equatorial declination and want latitude, convert.
              f1 = cpN.obj[i]; EquToEcl(&f1, &d);
              f1 = cpA.obj[j]; EquToEcl(&f1, &e1);
              f2 = cpB.obj[j]; EquToEcl(&f2, &e2);
            }

            if (k == aOpp) {
              neg(e1);
              neg(e2);
            }

            // Check if parallel aspect occurs during time segment.

            f1 = e1-d;
            f2 = e2-d;
            if (RSgn(f1) != RSgn(f2) && occurcount < MAXINDAY) {

              // Ok, found a parallel transit. Now determine the time and save
              // this transit in the list to be printed.

              if (k == aOpp) {
                neg(e1);
                neg(e2);
              }
              pti->source = j;
              pti->aspect = k;
              pti->dest = i;
              pti->time = RAbs(f1)/(RAbs(f1)+RAbs(f2))*divsiz +
                (real)(div-1)*divsiz;
              pti->posT = e1 + (e2 - e1)*RAbs(f1)/(RAbs(f1)+RAbs(f2));
              pti->posN = d;
              pti->retT = (cpA.diralt[j] + cpB.diralt[j]) / 2.0;
              occurcount++, pti++;
            }
          }
          } // us.fParallel

          // Check for planet pairs equidistant from each other.

          if (!us.fIgnoreDisequ) {
            d = cpN.dist[i]; e1 = cpA.dist[j]; e2 = cpB.dist[j];
            if (((d > e1 && d < e2) || (d > e2 && d < e1)) &&
              occurcount < MAXINDAY) {
              f1 = d-e1; f2 = e2-d;
              pti->source = j;
              pti->aspect = aDis;
              pti->dest = i;
              pti->time = RAbs(f1)/(RAbs(f1)+RAbs(f2))*divsiz +
                (real)(div-1)*divsiz;
              pti->posT = Mod(cpA.obj[j] + RAbs(f1)/(RAbs(f1)+RAbs(f2)) *
                MinDifference(cpA.obj[j], cpB.obj[j]));
              pti->posN = cpN.obj[i];
              pti->retT = (cpA.dir[j] + cpB.dir[j]) / 2.0;
              occurcount++, pti++;
            }
          }
        } // j
      } // i

#ifdef GRAPH
      // May want to draw current transit event within a graphic calendar box.
      if (RgzCalendar() != NULL && div < division)
        continue;
#endif

      // After all transits located, sort them by time at which they occur.

      for (i = 1; i < occurcount; i++) {
        j = i-1;
        while (j >= 0 && ti[j].time > ti[j+1].time) {
          tiT = ti[j]; ti[j] = ti[j+1]; ti[j+1] = tiT;
          j--;
        }
      }

      // Now loop through list and display all the transits.

      i = (RgzCalendar() == NULL ? -1 : occurcount);
      loop {
        i += (RgzCalendar() == NULL ? 1 : -1);
        if (!FBetween(i, 0, occurcount-1))
          break;

        pti = &ti[i];
        j = (int)(pti->time * 60.0);
        if (us.fInDayMonth) {
          s1 = j / (24*60*60);
          j = j - s1 * (24*60*60);
        } else
          s1 = DayT-1;
        s2 = j / (60*60);
        k = j - s2 * (60*60);
        s3 = k / 60;
        s4 = us.fSeconds ? k - s3*60 : -1;
        SetCI(ciCast, MonT, s1+1, YeaT, (real)j / (60.0*60.0),
          DstT, ZonT, LonT, LatT);
#ifdef EXPRESS
        if (RgzCalendar() != NULL && (i >= occurcount || s1 != s1prev))
          nSkip = 0;
        s1prev = s1;
        // May want to skip this transit if AstroExpression says to do so.
        if (!us.fExpOff && FSzSet(us.szExpTra)) {
          ciCore = ciCast;
          CastChart(-1);
          ExpSetN(iLetterX, pti->source);
          ExpSetN(iLetterY, pti->aspect);
          ExpSetN(iLetterZ, pti->dest);
          if (!NParseExpression(us.szExpTra)) {
            nSkip++;
            continue;
          }
        }
#endif
        ciSave = ciCast;
#ifdef GRAPH
        // May want to draw current transit within a graphic calendar box.
        if (RgzCalendar() != NULL) {
          for (j = i; j >= 0 && (int)ti[j].time/(24*60)+1 == ciSave.day; j--)
            ;
          for (k = i; k < occurcount &&
            (int)ti[k].time/(24*60)+1 == ciSave.day; k++)
            ;
          idT.source = pti->source; idT.aspect = pti->aspect;
          idT.dest = pti->dest; idT.ret1 = pti->retT;
          idT.mon = -1; idT.day = ciSave.day; idT.time = pti->time;
          idT.pos1 = pti->posT; idT.pos2 = pti->posN;
          DrawCalendarAspect(&idT, i-j+nSkip, k-j, -1, etUndefined);
          continue;
        }
#endif
        if (us.fListAuto) {
          // May want to add chart for current transit to chart list.
          ciEvent = ciSave;
          *sz = chNull;
          if (pti->aspect > 0)
            sprintf(sz, "%c.%s %s N.%s", fProg ? 'P' : 'T', szObjDisp[
              pti->source], SzAspectAbbrev(pti->aspect), szObjDisp[pti->dest]);
          else if (pti->aspect == aHou)
            sprintf(sz, "%c.%s enters N.%d%s 3D House", fProg ? 'P' : 'T',
              szObjDisp[pti->source], pti->dest, szSuffix[pti->dest]);
          else if (pti->aspect == aDis)
            sprintf(sz, "%c.%s distance equal N.%s", fProg ? 'P' : 'T',
              szObjDisp[pti->source], szObjDisp[pti->dest]);
          else
            Assert(fFalse);
          ciEvent.nam = SzClone(sz);
          ciEvent.loc = ciDefa.loc;
          FAppendCIList(&ciEvent);
        }
        if (us.fSeconds) {
          k = DayOfWeek(MonT, s1+1, YeaT);
          AnsiColor(kRainbowA[k + 1]);
          sprintf(sz, "%.3s ", szDay[k]); PrintSz(sz);
          AnsiColor(kDefault);
        }
        sprintf(sz, "%s %s ",
          SzDate(MonT, s1+1, YeaT, fFalse), SzTime(s2, s3, s4)); PrintSz(sz);
        PrintAspect(pti->source, pti->posT, pti->retT, pti->aspect,
          pti->dest, pti->posN, cpN.dir[pti->dest], fProg ? 'u' : 't');

        // Check for a Solar, Lunar, or any other return.

        if (pti->aspect == aCon && pti->source == pti->dest) {
          AnsiColor(kWhiteA);
          sprintf(sz, " (%s Return)", pti->source == oSun ? "Solar" :
            (pti->source == oMoo ? "Lunar" : szObjDisp[pti->source]));
          PrintSz(sz);
        }
        PrintL();
#ifdef INTERPRET
        if (us.fInterpret)
          InterpretTransit(pti->source, pti->aspect, pti->dest);
#endif
        AnsiColor(kDefault);
      } // i
      if (occurcount >= MAXINDAY && fPrint)
        PrintSz("Too many transits found.\n");
      counttotal += occurcount;
    } // div
  } // MonT
  if (counttotal == 0 && fPrint)
    PrintSz("No transits found.\n");

  // Recompute original chart placements as have overwritten them.

  ciCore = ciMain; ciTran = ciSav;
  cp0 = cpN;
  us.fProgress = fFalse;
  CastChart(1);
}


// Print a chart graphing transits over time. This covers both transit
// to transit (-B switch) and transit to natal (-V switch). Each aspect 
// present during the period has its own row, showing its strength from 0-9
// (blank=aspect out of orb, "0"=0-9% of max strength, "9"=90-100% exact).

void ChartTransitGraph(flag fTrans, flag fProg)
{
  TransGraInfo *rgEph;
  word **ppw, *pw, *pw2;
  char sz[cchSzDef];
  int cAsp, cSlice, cYea, dYea, occurcount = 0, ymin, x0, y0, x, y, asp,
    iw, iwFocus, nMax, n, ch, obj, et;
  flag fMonth = us.fInDayMonth, fYear = us.fInDayYear, fMark, fEclipse =
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
  ymin = 1-fTrans;

  // Determine character width of chart based on time period being graphed.
  if (!fMonth) {
    cSlice = 49;
    iwFocus = (int)(ciT.tim / 0.5);
  } else if (!fYear) {
    cSlice = (DayInMonth(ciT.mon, ciT.yea) << 1) + 1;
    iwFocus = ((ciT.day-1) << 1) + (ciT.tim >= 12.0);
  } else if (us.nEphemYears <= 1) {
    cSlice = 12*5;
    iwFocus = (ciT.mon-1)*5 + (Min(ciT.day, 30)-1)/6;
  } else {
    cYea = Max(us.nEphemYears, 2); cYea = Min(cYea, 21);
    dYea = (cYea - 1) >> 1;
    cSlice = cYea*12;
    iwFocus = dYea*12 + (ciT.mon-1);
  }
  if (iwFocus == 0 && ciT.tim <= 0.0)
    iwFocus = -1;

  // Calculate and fill out aspect strength arrays for each aspect present.
  if (fTrans || fProg) {
    ciCore = ciMain;
    us.fProgress = fFalse;
    CastChart(0);
    cp1 = cp0;
  }
  for (iw = 0; iw < cSlice; iw++) {

    // Cast chart for current time slice.
    ciCore = ciT;
    if (!fMonth) {
      TT = (real)iw * 0.5;
    } else if (!fYear) {
      DD = (iw >> 1) + 1;
      TT = FOdd(iw) ? 12.0 : 0.0;
    } else if (us.nEphemYears <= 1) {
      MM = (iw / 5) + 1;
      DD = (iw % 5) * 5 + 1;
      TT = 0.0;
    } else {
      MM = (iw % 12) + 1;
      DD = 1;
      YY = YY - dYea + (iw / 12);
      TT = 0.0;
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
        if (!fTrans ? FIgnore(x) || !FProperGraph(x) :
          FIgnore2(x) || !FProperGraph(x) || (is.fReturn && x != y))
          continue;
        asp = grid->n[x][y];
        if (!FBetween(asp, aCon, cAsp))
          continue;
        ppw = &(*rgEph)[x][y][asp];
        if (*ppw == NULL) {
          *ppw = (word *)PAllocate(cSlice * sizeof(word),
            "transit ephemeris entry");
          if (*ppw == NULL)
            goto LDone;
          pw = *ppw;
          ClearB((pbyte)pw, cSlice * sizeof(word));
        } else
          pw = *ppw;
        rT = RAbs(grid->v[x][y]);
        rT /= GetOrb(x, y, asp);
        pw[iw] = 65535 - (int)(rT * (65536.0 - rSmall));

        // Check for and add eclipse information to array too.
        if (fEclipse) {
          et = NCheckEclipseAny(x, asp, y, &rPct);
          if (et > etNone) {
            ppw = &(*rgEph)[y][x][asp];
            if (*ppw == NULL) {
              *ppw = (word *)PAllocate(cSlice * sizeof(word),
                "transit eclipse entry");
              if (*ppw == NULL)
                goto LDone;
              pw2 = *ppw;
              ClearB((pbyte)pw2, cSlice * sizeof(word));
            } else
              pw2 = *ppw;
            pw2[iw] = et;
          }
        }
      }
    }
  }

  // Print chart header row(s).
  AnsiColor(kWhiteA);
  if (!fMonth)
    sprintf(sz, SzDate(ciT.mon, ciT.day, ciT.yea, fFalse));
  else if (!fYear)
    sprintf(sz, "%3.3s%5d", szMonth[ciT.mon], ciT.yea);
  else if (us.nEphemYears <= 1)
    sprintf(sz, "%4d", ciT.yea);
  else
    sz[0] = chNull;
  PrintSz(sz);
  PrintTab(' ', 12 + fTrans*4 - CchSz(sz));
  AnsiColor(kDefault);

  if (!fMonth) {
    if (!us.fEuroTime)
      sprintf(sz, "121a2a3a4a5a6a7a8a9a1011121p2p3p4p5p6p7p8p9p1011");
    else
      sprintf(sz, "000102030405060708091011121314151617181920212223");
    for (iw = 0; sz[iw]; iw++) {
      if (!FOdd(iw))
        AnsiColor((iw & 2) == 0 ? kLtGrayA : kDkGrayA);
      PrintCh(sz[iw]);
    }
  } else if (!fYear) {
    for (iw = 1; iw <= (cSlice >> 1); iw++) {
      n = DayOfWeek(MM, iw, YY);
      AnsiColor(kRainbowA[n+1]);
      sprintf(sz, "%.2s", szDay[n]); PrintSz(sz);
    }
    PrintL();
    PrintTab(' ', 12 + fTrans*4);
    for (iw = 1; iw <= (cSlice >> 1); iw++) {
      AnsiColor(FOdd(iw) ? kLtGrayA : kDkGrayA);
      sprintf(sz, "%02d", iw); PrintSz(sz);
    }
  } else if (us.nEphemYears <= 1) {
    for (iw = 1; iw <= 12; iw++) {
      AnsiColor(FOdd(iw) ? kLtGrayA : kDkGrayA);
      sprintf(sz, "%3.3s", szMonth[iw]); PrintSz(sz);
      if (iw < 12)
        PrintSz("  ");
    }
  } else {
    for (iw = 0; iw < cYea; iw++) {
      AnsiColor(!FOdd(iw) ? kLtGrayA : kDkGrayA);
      sprintf(sz, "%-12d", (fTrans || fProg ? YeaT : Yea) - dYea + iw);
      PrintSz(sz);
    }
    PrintL();
    PrintTab(' ', 12 + fTrans*4);
    for (iw = 0; iw < cSlice; iw++) {
      if ((iw % 12) == 0)
        AnsiColor(iw % 24 == 0 ? kLtGrayA : kDkGrayA);
      PrintCh(szMonth[iw % 12 + 1][0]);
    }
  }
  PrintL();

  // Print the individual aspects present in order.
  for (y0 = ymin; y0 <= is.nObj; y0++) {
    y = rgobjList[y0];
    for (x0 = 0; x0 < (fTrans ? is.nObj+1 : y); x0++) {
      x = rgobjList[x0];
      for (asp = 1; asp <= cAsp; asp++) {
        pw = (*rgEph)[x][y][asp];
        if (pw == NULL)
          continue;
        occurcount++;

        // Print the name of the aspect in question.
        if (fTrans) {
          AnsiColor(kDkGrayA);
          PrintSz(!fProg ? "T." : "P.");
        }
        AnsiColor(kObjA[x]);
        sprintf(sz, "%3.3s ", szObjDisp[x]); PrintSz(sz);
        AnsiColor(kAspA[asp]);
        sprintf(sz, "%s ", SzAspectAbbrev(asp)); PrintSz(sz);
        if (fTrans) {
          AnsiColor(kSignA(SFromZ(cp1.obj[y])));
          PrintSz("N.");
        }
        AnsiColor(kObjA[y]);
        sprintf(sz, "%3.3s ", szObjDisp[y]); PrintSz(sz);
        AnsiColor(kAspA[asp]);
        nMax = -1;
        for (iw = 0; iw < cSlice; iw++) {
          n = pw[iw];
          if (n > nMax)
            nMax = n;
        }

        // Print the graph itself for the aspect in question.
        fMark = fFalse;
        for (iw = 0; iw < cSlice; iw++) {
          n = pw[iw];
          if (n >= nMax || ((iw <= 0 || n > pw[iw-1]) &&
            (iw >= cSlice-1 || n >= pw[iw+1]))) {
            AnsiColor(kWhiteA);
            fMark = fTrue;
          }
          if (n == 0) {
            if (iw != iwFocus) {
              if (fMonth && fYear &&
                iw % (us.nEphemYears <= 1 ? 5 : 12) == 0) {
                ch = '.';
                AnsiColor(kDkGrayA);
                fMark = fTrue;
              } else
                ch = ' ';
            } else {
              ch = '|';
              AnsiColor(kDkGreenA);
              fMark = fTrue;
            }
          } else {
            pw2 = (*rgEph)[y][x][asp];
            if (!fEclipse || pw2 == NULL || pw2[iw] == 0)
              ch = '0' + ((n - 1) * 10 / 65535);
            else
              ch = rgchEclipse[pw2[iw]];
          }
          PrintCh(ch);
          if (fMark) {
            AnsiColor(kAspA[asp]);
            fMark = fFalse;
          }
        }
        PrintL();
      }
    }
  }
  if (occurcount == 0)
    PrintSz("No transits found.\n");

  // Free temporarily allocated data, and restore original chart.
LDone:
  AnsiColor(kDefault);
  for (y = ymin; y <= is.nObj; y++)
    for (x = 0; x < (fTrans ? is.nObj+1 : y); x++)
      for (asp = 1; asp <= cAsp; asp++) {
        pw = (*rgEph)[x][y][asp];
        DeallocatePIf(pw);
        if (fEclipse) {
          pw2 = (*rgEph)[y][x][asp];
          DeallocatePIf(pw2);
        }
      }
  DeallocatePIf(rgEph);
  ciCore = ciMain;
  us.fProgress = fFalse;
  CastChart(1);
}


CONST char *rgszHorizon[4] = {"rises", "zeniths", "sets", "nadirs"};

// Display a list of planetary rising times relative to the local horizon for
// the day indicated in the chart information, as specified with the -Zd
// switch. For the day, the time each planet rises (transits horizon in East
// half of sky), sets (transits horizon in West), reaches its zenith point
// (transits meridian in South half of sky), and nadirs (transits meridian in
// North), is displayed.

void ChartHorizonRising(void)
{
  char sz[cchSzDef];
  int source[MAXINDAY], type[MAXINDAY], fRet[MAXINDAY],
    occurcount, division, div, s1, s2, s3, i, j, fT;
  real time[MAXINDAY], rgalt1[objMax], rgalt2[objMax], azialt[MAXINDAY],
    pos[MAXINDAY], azi1, azi2, alt1, alt2, mc1, mc2, xA, yA, xV, yV, d, k;
  int yea0, yea1, yea2, mon0, mon1, mon2, day0, day1, day2, counttotal = 0;
  flag fSav = is.fSeconds, fYear;
  CI ciSav, ciEvent;

  fT = us.fSidereal; us.fSidereal = fFalse;
  division = us.nDivision;
  fYear = us.fInDayMonth && us.fInDayYear;
  if (us.fListAuto)
    is.cci = 0;

  // If -dY in effect, then search through a range of years.

  yea1 = yea2 = Yea;
  if (fYear && us.nEphemYears != 0) {
    if (us.nEphemYears < 0)
      yea1 += (us.nEphemYears + 1);
    else
      yea2 += (us.nEphemYears - 1);
  }
  for (yea0 = yea1; yea0 <= yea2; yea0++) {

  // If -dy in effect, then search through the whole year, month by month.

  if (fYear) {
    mon1 = 1; mon2 = 12;
  } else
    mon1 = mon2 = Mon;
  for (mon0 = mon1; mon0 <= mon2; mon0++) {

  // If -dm in effect, then search through the whole month, day by day.

  if (us.fInDayMonth) {
    day1 = 1;
    day2 = DayInMonth(mon0, yea0);
  } else
    day1 = day2 = Day;
  for (day0 = day1; day0 <= day2; day0 = AddDay(mon0, day0, yea0, 1)) {

  occurcount = 0;
  ciSav = ciTwin;
  SetCI(ciCore, mon0, day0, yea0, 0.0, Dst, Zon, Lon, Lat);
  CastChart(-1);
  mc2 = planet[oMC]; k = planetalt[oMC];
  EclToEqu(&mc2, &k);
  cp2 = cp0;
  for (i = 0; i <= is.nObj; i++)
    rgalt2[i] = planetalt[i];

  // Loop through the day, dividing it into a certain number of segments.
  // For each segment we get the planet positions at its endpoints.

  for (div = 1; div <= division; div++) {
    SetCI(ciCore, mon0, day0, yea0, 24.0*(real)div/(real)division,
      Dst, Zon, Lon, Lat);
    CastChart(-1);
    mc1 = mc2;
    mc2 = planet[oMC]; k = planetalt[oMC];
    EclToEqu(&mc2, &k);
    cp1 = cp2; cp2 = cp0;
    for (i = 0; i <= is.nObj; i++) {
      rgalt1[i] = rgalt2[i]; rgalt2[i] = planetalt[i];
    }

    // For our segment, check to see if each planet during it rises, sets,
    // reaches its zenith, or reaches its nadir.

    for (i = 0; i <= is.nObj; i++) if (!ignore[i] && FThing(i)) {
      EclToHoriz(&azi1, &alt1, cp1.obj[i], rgalt1[i], mc1, Lat);
      EclToHoriz(&azi2, &alt2, cp2.obj[i], rgalt2[i], mc2, Lat);
      j = 0;

      // Check for transits to the horizon.
      if ((alt1 > 0.0) != (alt2 > 0.0)) {
        d = RAbs(alt1)/(RAbs(alt1)+RAbs(alt2));
        k = Mod(azi1 + d*MinDifference(azi1, azi2));
        j = 1 + 2*(MinDistance(k, rDegHalf) < rDegQuad);

      // Check for transits to the meridian.
      } else if (RSgn(MinDifference(azi1, rDegQuad)) !=
        RSgn(MinDifference(azi2, rDegQuad))) {
        j = 2 + 2*(alt1+alt2 < 0.0);
        d = RAbs(azi1 - (MinDistance(azi1, rDegQuad) < rDegQuad ? rDegQuad :
          270.0))/MinDistance(azi1, azi2);
        k = alt1 + d*(alt2-alt1);
      }
      if (j && !ignorez[j-1] && occurcount < MAXINDAY) {
        source[occurcount] = i;
        type[occurcount] = j;
        time[occurcount] = 24.0*((real)(div-1)+d)/(real)division*60.0;
        pos[occurcount] = Mod(cp1.obj[i] +
          d*MinDifference(cp1.obj[i], cp2.obj[i]));
        fRet[occurcount] = (int)RSgn(cp1.dir[i]) + (int)RSgn(cp2.dir[i]);
        azialt[occurcount] = k;
        ciSave = ciMain;
        ciSave.tim = time[occurcount] / 60.0;
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
      SwapR(&pos[j], &pos[j+1]);
      SwapN(fRet[j], fRet[j+1]);
      SwapR(&azialt[j], &azialt[j+1]);
      j--;
    }
  }

  // Finally display the list showing each event and when it occurs.

  for (i = 0; i < occurcount; i++) {
    ciSave = ciMain;
    ciSave.mon = mon0; ciSave.day = day0; ciSave.yea = yea0;
    ciSave.tim = time[i] / 60.0;
#ifdef EXPRESS
    // May want to skip this event if AstroExpression says to do so.
    if (!us.fExpOff && FSzSet(us.szExpRis)) {
      ciCore = ciSave;
      CastChart(-1);
      ExpSetN(iLetterX, source[i]);
      ExpSetN(iLetterY, type[i] - 1);
      ExpSetR(iLetterZ, azialt[i]);
      if (!NParseExpression(us.szExpRis))
        continue;
    }
#endif
    if (us.fListAuto) {
      // May want to add chart for current event to chart list.
      ciEvent = ciSave;
      sprintf(sz, "%s %s", szObjDisp[source[i]], rgszHorizon[type[i]-1]);
      ciEvent.nam = SzClone(sz);
      ciEvent.loc = ciDefa.loc;
      FAppendCIList(&ciEvent);
    }
    j = DayOfWeek(mon0, day0, yea0);
    AnsiColor(kRainbowA[j + 1]);
    sprintf(sz, "%.3s ", szDay[j]); PrintSz(sz);
    AnsiColor(kDefault);
    s1 = (int)time[i]/60;
    s2 = (int)time[i]-s1*60;
    s3 = is.fSeconds ? (int)(time[i]*60.0)-((s1*60+s2)*60) : -1;
    sprintf(sz, "%s %s ", SzDate(mon0, day0, yea0, fFalse),
      SzTime(s1, s2, s3));
    PrintSz(sz);
    AnsiColor(kObjA[source[i]]);
    sprintf(sz, "%7.7s ", szObjDisp[source[i]]); PrintSz(sz);

    j = kSignA(SFromZ(pos[i]));
    AnsiColor(j);
    sprintf(sz, "%c", fRet[i] > 0 ? '(' : (fRet[i] < 0 ? '[' : '<'));
    PrintSz(sz);
    if (!us.fSeconds) {
      sprintf(sz, "%.3s", szSignName[SFromZ(pos[i])]); PrintSz(sz);
    } else {
      is.fSeconds = fFalse;
      PrintZodiac(pos[i]);
      is.fSeconds = fSav;
      AnsiColor(j);
    }
    sprintf(sz, "%c ", fRet[i] > 0 ? ')' : (fRet[i] < 0 ? ']' : '>'));
    PrintSz(sz);

    AnsiColor(kElemA[type[i]-1]);
    sprintf(sz, "%-7s", rgszHorizon[type[i]-1]); PrintSz(sz);
    AnsiColor(kDefault);
    PrintSz(" at ");
    if (FOdd(type[i])) {
      j = (int)(azialt[i]*60.0)%60;
      sprintf(sz, "%3d%c%02d'", (int)azialt[i], chDegC, j); PrintSz(sz);
      if (is.fSeconds) {
        sprintf(sz, "%02d\"", (int)(azialt[i]*3600.0)%60); PrintSz(sz);
      }

      // For rising and setting events, also display a direction vector to
      // make the 360 degree azimuth value thought of easier.

      xA = RCosD(azialt[i]); yA = RSinD(azialt[i]);
      if (RAbs(xA) < RAbs(yA)) {
        xV = RAbs(xA / yA); yV = 1.0;
      } else {
        yV = RAbs(yA / xA); xV = 1.0;
      }
      sprintf(sz, " (%.2f%c %.2f%c)",
        yV, yA < 0.0 ? 's' : 'n', xV, xA > 0.0 ? 'e' : 'w'); PrintSz(sz);
    } else
      PrintAltitude(azialt[i]);
    PrintL();
  }
  counttotal += occurcount;
  } // day0
  } // mon0
  } // yea0
  if (counttotal == 0)
    PrintSz("No horizon events found.\n");

  // Recompute original chart placements as have overwritten them.

  ciCore = ciMain; ciTwin = ciSav;
  us.fSidereal = fT;
  CastChart(1);
}


// Print out an ephemeris, i.e. the positions of the planets (at the time in
// the current chart) each day during a specified month, as done with the -E
// switch. Display the ephemeris for the whole year if -Ey is in effect.

void ChartEphemeris(void)
{
  char sz[cchSzDef];
  int yea, yea1, yea2, mon, mon1, mon2, daysiz, timsiz, t, i, j, k, s, d, m;
  real tim, rT;
  flag fDidBlank = fFalse, fWantHeader = fTrue;

  // If -Ey is in effect, then loop through all months in the whole year.

  yea1 = !us.fProgress ? Yea : YeaT;
  if (us.nEphemYears) {
    yea2 = yea1 + us.nEphemYears - 1;
    mon1 = 1; mon2 = us.nEphemRate > 1 ? 1 : 12;
  } else {
    yea2 = yea1;
    mon1 = mon2 = !us.fProgress ? Mon : MonT;
  }
  timsiz = us.nEphemRate < 0 ? (24-1)/us.nEphemFactor : 0;

  // Loop through the year or years in question.

  for (yea = yea1; yea <= yea2;
    yea += (us.nEphemRate > 1 ? us.nEphemFactor : 1))

  // Loop through the month or months in question, printing each ephemeris.

  for (mon = mon1; mon <= mon2;
    mon += (us.nEphemRate == 1 ? us.nEphemFactor : 1)) {
    daysiz = us.nEphemRate < 1 ? DayInMonth(mon, yea) : 1;
    if ((us.nEphemRate < 1 || (us.nEphemRate == 1 && mon == mon1) ||
      (us.nEphemRate > 1 && yea == yea1)) && fWantHeader) {

      // Print header row before certain months.

      fWantHeader = fFalse;
      if (!fDidBlank)
        fDidBlank = fTrue;
      else
        PrintL();
      if (is.fSeconds)
        PrintSz(us.fEuroDate ? "Dy/Mo/Year" : "Mo/Dy/Year");
      else
        PrintSz(us.fEuroDate ? "Dy/Mo/Yr" : "Mo/Dy/Yr");
      if (us.nEphemRate < 0) {
        PrintSz("  Time");
        PrintTab(' ', CchSz(SzTim(0.0)) - 5);
      }
      for (k = 0; k <= is.nObj; k++) {
        j = rgobjList[k];
        if (FIgnore(j))
          continue;
        if (is.fSeconds)
          sprintf(sz, "  %-10.10s", szObjDisp[j]);
        else
          sprintf(sz, "  %-4.4s", szObjDisp[j]);
        PrintSz(sz);
        PrintTab(' ', us.fParallel ? 2*!is.fSeconds : 1);
      }
      PrintL();
    }
    for (i = 1; i <= daysiz;
      i = AddDay(mon, i, yea, us.nEphemRate == 0 ? us.nEphemFactor : 1)) {
      for (t = 0; t <= timsiz; t++) {

      // Loop through each day in the month, casting a chart for that day.

      tim = us.nEphemRate < 0 ? (real)(t*us.nEphemFactor): Tim;
      if (!us.fProgress) {
        SetCI(ciCore, mon, i, yea, tim, Dst, Zon, Lon, Lat);
      } else {
        is.JDp = MdytszToJulian(mon, i, yea, tim, Dst, Zon);
        ciCore = ciMain;
      }
      CastChart(-1);
#ifdef EXPRESS
      if (FSzSet(us.szExpEph) && !NParseExpression(us.szExpEph))
        continue;
#endif
      fWantHeader = fTrue;
      PrintSz(SzDate(mon, i, yea, is.fSeconds-1));
      PrintCh(' ');
      if (us.nEphemRate < 0) {
        PrintSz(SzTim(tim));
        PrintCh(' ');
      }
      for (k = 0; k <= is.nObj; k++) {
        j = rgobjList[k];
        if (FIgnore(j))
          continue;
        if (!us.fParallel) {
          if (is.fSeconds)
            PrintZodiac(planet[j]);
          else {
            AnsiColor(kObjA[j]);
            switch (us.nDegForm) {
            case dfZod:
              // -sz: Format position in degrees/sign/minutes format.
              s = SFromZ(planet[j]);
              d = (int)planet[j] - (s-1)*30;
              m = (int)(RFract(planet[j])*60.0);
              sprintf(sz, "%2d%s%02d", d, szSignAbbrev[s], m);
              break;
            case dfHM:
              // -sh: Format position in hours/minutes.
              s = (int)Mod(planet[j] + rRound/4.0);
              d = (int)planet[j] / 15;
              m = (int)((planet[j] - (real)d*15.0)*4.0);
              sprintf(sz, "%2dh%02dm", d, m);
              break;
            default:
              // -sd: Format position as a simple degree.
              sprintf(sz, "%6.2f", planet[j]);
              break;
            }
            PrintSz(sz);
          }
        } else {
          AnsiColor(kObjA[j]);
          PrintAltitude(planetalt[j]);
        }
        rT = (!us.fParallel ? ret[j] : retalt[j]);
        if (rT < 0.0) {
          AnsiColor(kDefault);
          PrintCh(is.fSeconds ? chRet : chRet2);
        }
        if (k < is.nObj)
          PrintTab(' ', 1 - (rT < 0.0) + is.fSeconds);
      }
      PrintL();
      AnsiColor(kDefault);
    } // t
    } // i
  } // mon

  ciCore = ciMain;    // Recast original chart.
  CastChart(1);
}


// Print a list of exoplanet transits, i.e. when planets pass in front of
// their star, as displayed with the -Ux switch. Can also produce a colored
// list of exoplanet transit locations for displaying in graphics charts.

flag ChartExoplanet(flag fColor)
{
  char szLine[cchSzLine], sz[cchSzDef], szT[cchSzDef], *pch;
  ExoData *pexod;
  real jd1, jd2, jd, off, jdMin, jda, jdb, jdChart, lon, lat, rT;
  int cexod, count = 0, dTrans, mon, day, yea, mon2, day2, yea2, hr, min, sec,
    i, j, n;
  CI ci = ciMain, ci2 = ciMain, ci3 = ciMain, ci4 = ciMain, ci5 = ciMain;
  flag fInstant = !us.fParallel || fColor, fNoOverlap = !us.fHouse3D,
    fMonth = us.fInDayMonth, fYear = us.fInDayYear, fOverlap;
  static flag fTOI = fFalse;
  FILE *file;

  // Read in exoplanet data file
  if (is.rgexod == NULL) {
    file = FileOpen(szFileExoCore, 0, NULL);
    if (file == NULL)
      return fFalse;
    for (i = 0; !feof(file); i++)
      fgets(szLine, cchSzLine, file);
    fseek(file, 0, SEEK_SET);
    fgets(szLine, cchSzLine, file);
    fTOI = CchSz(szLine) > 100;
    cexod = i-2;
    is.rgexod = RgAllocate(cexod, ExoData, "exoplanet list");
    if (is.rgexod == NULL)
      return fFalse;
    is.cexod = cexod;
    ClearB((pbyte)is.rgexod, sizeof(ExoData) * cexod);
    for (i = 0; i < cexod; i++) {
      // Read in data for next exoplanet
      fgets(szLine, cchSzLine, file);
      pexod = &is.rgexod[i];
      for (pch = szLine; *pch && *pch != ','; pch++)
        ;
      if (pch > szLine)
        pch[-1] = chNull;
      if (!FCloneSz(szLine + (szLine[0] != chNull), &pexod->sz))
        return fFalse;
      pch++;
      hr = atoi(pch); min = atoi(pch+3); lon = atof(pch+6);
      pexod->ra = (real)NAbs(hr) + (real)min/60.0 + lon/3600.0;
      if (hr < 0)
        neg(pexod->ra);
      AdvancePast(',');
      hr = atoi(pch); min = atoi(pch+4); lat = atof(pch+7);
      pexod->dec = (real)NAbs(hr) + (real)min/60.0 + lat/3600.0;
      if (hr < 0)
        neg(pexod->dec);
      AdvancePast(',');
      if (fTOI) {
        AdvancePast(',');
      }
      pexod->mag = atof(pch);
      AdvancePast(',');
      pexod->epoch = atof(pch);
      AdvancePast(',');
      pexod->epochU = atof(pch);
      AdvancePast(',');
      pexod->period = atof(pch);
      AdvancePast(',');
      pexod->periodU = atof(pch);
      AdvancePast(',');
      pexod->dur = atof(pch);
    }
    fclose(file);
  } else
    cexod = is.cexod;

  // Determine range of dates to scan over
  day = day2 = mon = mon2 = 1;
  yea = yea2 = Yea;
  if (!fMonth || fInstant) {
    day = Day; day2 = Day+1;
    mon = mon2 = Mon;
    sprintf(szT, "1 day");
  } else if (!fYear) {
    mon = Mon; mon2 = Mon+1;
    sprintf(szT, "1 month");
  } else if (us.nEphemYears <= 1) {
    yea2 = Yea+1;
    sprintf(szT, "1 year");
  } else {
    yea2 = Yea + us.nEphemYears;
    sprintf(szT, "%d year", us.nEphemYears);
  }
  jd1 = MdytszToJulian(mon, day, yea, 0.0, Dst, Zon) - 0.5;
  jd2 = MdytszToJulian(mon2, day2, yea2, 0.0, Dst, Zon) - 0.5;
  jdChart = MdytszToJulian(Mon, Day, Yea, Tim, Dst, Zon);
  if (!fColor) {
    PrintSz("Exoplanet transits ");
    if (fInstant)
      sprintf(sz, "taking place at chart time");
    else
      sprintf(sz, "over a %s period", szT);
    PrintSz(sz);
    PrintSz(":\nName");
    PrintTab(' ', !fTOI ? 11 : 14);
    PrintSz("Location");
    PrintTab(' ', !us.fSeconds ? 7 : 14);
    PrintSz("Mag.                 Start");
    PrintTab(' ', (!us.fSeconds ? 11 : 17) - us.fEuroTime*4);
    PrintSz("Middle");
    PrintTab(' ', (!us.fSeconds ? 10 : 16) - us.fEuroTime*4);
    PrintSz("End");
    PrintTab(' ', 5 - us.fEuroTime*2);
    if (!us.fSeconds)
      PrintSz("Dur.  Unc");
    else
      PrintSz("   Durat.   Uncert");
    PrintSz(".\n\n");
  }

  if (fInstant) {
    jd1 -= 1.0;
    jd2 += 1.0;
  }
  for (i = 0; i < cexod; i++) {
    pexod = &is.rgexod[i];
    dTrans = (int)((pexod->epoch - jd1) / pexod->period);
    is.rgexod[i].JDLoop = pexod->epoch - (real)dTrans * pexod->period;
    is.rgexod[i].kiLoop = kRed;
  }

  loop {
    // Find the next most recent exoplanet transit
    jdMin = 1.0E+20; j = -1;
    for (i = 0; i < cexod; i++) {
      jd = is.rgexod[i].JDLoop;
      if (jd < jdMin && jd < jd2) {
        jdMin = jd;
        j = i;
      }
    }
    if (j < 0)
      break;

    // Display current exoplanet transit
    pexod = &is.rgexod[j];
    is.rgexod[j].JDLoop += pexod->period;
    jd = jdMin;
    if (jd < jd1)
      continue;
    // For example: -YUx "TOI-2180 b;Kepler-1580 b"
    if (FSzSet(us.szExoList) &&
      SzInList(pexod->sz, us.szExoList, NULL) == NULL)
      continue;
    dTrans = (int)(RAbs(pexod->epoch - jd) / pexod->period + rRound);
    off = (pexod->epochU + pexod->periodU * (real)dTrans) * 24.0;
    fOverlap = (off > pexod->dur/2.0);
    if (fNoOverlap && fOverlap && !fColor)
      continue;
#ifdef EXPRESS
    // May want to skip this exoplanet if AstroExpression says to do so.
    if (!us.fExpOff && FSzSet(us.szExpExo)) {
      ExpSetR(iLetterW, pexod->mag);
      ExpSetR(iLetterX, pexod->period);
      ExpSetR(iLetterY, pexod->dur);
      ExpSetR(iLetterZ, jd);
      if (!NParseExpression(us.szExpExo))
        continue;
    }
#endif

    JulianToMdy(jd, &ci.mon, &ci.day, &ci.yea);

    ci.tim = RFract(jd - 0.5) * 24.0 -
      (Zon - (Dst == dstAuto ? (real)is.fDst : Dst)) - (pexod->dur/2.0 + off);
    AddTime(&ci, 2, 0);     // Sanitize time if hour out of range
    ci2 = ci; ci2.tim += (off*2.0);
    AddTime(&ci2, 2, 0);    // Sanitize time if hour out of range
    ci3 = ci2; ci3.tim += (pexod->dur/2.0 - off);
    AddTime(&ci3, 2, 0);    // Sanitize time if hour out of range
    ci4 = ci3; ci4.tim += (pexod->dur/2.0 - off);
    AddTime(&ci4, 2, 0);    // Sanitize time if hour out of range
    ci5 = ci4; ci5.tim += (off*2.0);
    AddTime(&ci5, 2, 0);    // Sanitize time if hour out of range

    // Check for whether current exoplanet transit overlaps chart time
    if (fInstant) {
      if (fColor) {
        jda = MdytszToJulian(ci.mon,  ci.day,  ci.yea,  ci.tim,  Dst, Zon);
        jdb = MdytszToJulian(ci5.mon, ci5.day, ci5.yea, ci5.tim, Dst, Zon);
        if (FBetween(jdChart, jda, jdb))
          is.rgexod[j].kiLoop = kYellow;
      }
      jda = MdytszToJulian(ci2.mon, ci2.day, ci2.yea, ci2.tim, Dst, Zon);
      jdb = MdytszToJulian(ci4.mon, ci4.day, ci4.yea, ci4.tim, Dst, Zon);
      if (!FBetween(jdChart, jda, jdb))
        continue;
      is.rgexod[j].kiLoop = kGreen;
    }

    count++;
    if (fColor)
      continue;

    // Print current exoplanet transit event
    sprintf(sz, fTOI ? "%-16.16s" : "%-13.13s", pexod->sz); PrintSz(sz);
    PrintCh(' ');
    lon = pexod->ra * rDegMax / 24.0; lat = pexod->dec;
    if (!us.fEquator || !us.fEquator2) {
      if (us.fEquator && !us.fEquator2) {
        rT = lon; EquToEcl(&rT, &lat);
      } else if (!us.fEquator && us.fEquator2) {
        rT = lat; EquToEcl(&lon, &rT);
      } else
        EquToEcl(&lon, &lat);
    }
    lon = Mod(Untropical(lon));
    PrintZodiac(lon);
    PrintCh(' ');
    PrintAltitude(lat);
    sprintf(sz, " %4.1f", pexod->mag); PrintSz(sz);
    n = DayOfWeek(ci.mon, ci.day, ci.yea);
    AnsiColor(kRainbowA[n + 1]);
    sprintf(sz, " %.3s", szDay[n]); PrintSz(sz);
    AnsiColor(kDefault);
    sprintf(sz, " %s", SzDate(ci.mon, ci.day, ci.yea, fFalse)); PrintSz(sz);
    AnsiColor(kRedA);
    sprintf(sz, " %s", SzTim(ci.tim)); PrintSz(sz);

    AnsiColor(fOverlap ? kBlackA :
      (ci2.day != ci.day || ci2.mon != ci.mon ? kWhiteA : kYellowA));
    sprintf(sz, " %s", SzTim(ci2.tim));
    if (fOverlap)
      for (pch = sz; *pch; pch++)
        *pch = ' ';
    PrintSz(sz);

    AnsiColor(fOverlap ?
      (ci3.day != ci.day || ci3.mon != ci.mon ? kWhiteA : kYellowA) :
      (ci3.day != ci2.day || ci3.mon != ci2.mon ? kWhiteA : kGreenA));
    sprintf(sz, " %s", SzTim(ci3.tim)); PrintSz(sz);

    AnsiColor(fOverlap ? kBlackA :
      (ci4.day != ci3.day || ci4.mon != ci3.mon ? kWhiteA : kYellowA));
    sprintf(sz, " %s", SzTim(ci4.tim));
    if (fOverlap)
      for (pch = sz; *pch; pch++)
        *pch = ' ';
    PrintSz(sz);

    AnsiColor(fOverlap ?
      (ci5.day != ci3.day || ci5.mon != ci3.mon ? kWhiteA : kRedA) :
      (ci5.day != ci4.day || ci5.mon != ci4.mon ? kWhiteA : kRedA));
    sprintf(sz, " %s", SzTim(ci5.tim)); PrintSz(sz);

    // Print duration and overall uncertainty of this transit
    AnsiColor(kDefault);
    for (n = 0; n <= 1; n++) {
      sec = (int)((n ? off : pexod->dur) * 3600.0);
      hr = sec / 3600;
      min = sec / 60 % 60;
      sec %= 60;
      sprintf(sz, " %2d:%02d", hr, min); PrintSz(sz);
      if (us.fSeconds) {
        sprintf(sz, ":%02d", sec); PrintSz(sz);
      }
    }
    PrintL();
    if (us.fMidAspect)
      PrintAspectsToPoint(lon, -1, 0.0, "Exoplanet");
  }
  if (count == 0 && !fColor)
    PrintSz("No exoplanet transits found.\n");
  return fTrue;
}

/* charts3.cpp */
