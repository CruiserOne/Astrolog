/*
** Astrolog (Version 7.00) File: io.cpp
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
** File IO Routines.
******************************************************************************
*/

/* Open the file indicated by the given string and return the file's stream */
/* pointer, or NULL if the file couldn't be found or opened. All parts of   */
/* the program which open files to read call this routine. We look in       */
/* several various locations and directories for the file before giving up. */

FILE *FileOpen(CONST char *szFile, int nFileMode, char *szPath)
{
  FILE *file;
  char szFileT[cchSzMax], sz[cchSzMax], szMode[3], *pch;
#ifdef ENVIRON
  char *env;
#endif
  int i, j;

  /* Some file types we want to open as binary instead of Ascii. */
  sprintf(szMode, "r%s", nFileMode == 2 ? "b" : "");

  for (i = 0; i <= 1; i++) {
    if (i <= 0)
      sprintf(szFileT, "%s", szFile);
    else {
      if (nFileMode > 1)
        break;
      sprintf(szFileT, "%s.as", szFile);
    }

    /* First look for the file in the directory of the Astrolog executable. */
#ifdef WIN
    GetModuleFileName(wi.hinst, sz, cchSzMax);
#else
    sprintf(sz, "%s", is.szProgName != NULL ? is.szProgName : "");
#endif
    for (pch = sz; *pch; pch++)
      ;
    while (pch > sz && *pch != chDirSep)
      pch--;
    if (*pch == chDirSep)
      pch++;
    sprintf(pch, "%s", szFileT);
    file = fopen(sz, szMode);
    if (file != NULL)
      goto LDone;

    /* Next look for the file in the current directory. */
    sprintf(sz, "%s", szFileT);
    file = fopen(sz, szMode);
    if (file != NULL)
      goto LDone;

    /* Next look in the directories indicated by the -Yi switch. */
    for (j = 0; j < 10; j++)
      if (us.rgszPath[j] && *us.rgszPath[j]) {
        sprintf(sz, "%s%c%s", us.rgszPath[j], chDirSep, szFileT);
        file = fopen(sz, szMode);
        if (file != NULL)
          goto LDone;
      }

#ifdef ENVIRON
    /* Next look for the file in the directory indicated by the version */
    /* specific system environment variable.                            */
    sprintf(sz, "%s%s", ENVIRONVER, szVerCore);
    env = getenv(sz);
    if (env && *env) {
      sprintf(sz, "%s%c%s", env, chDirSep, szFileT);
      file = fopen(sz, szMode);
      if (file != NULL)
        goto LDone;
    }

    /* Next look in the directory in the general environment variable. */
    env = getenv(ENVIRONALL);
    if (env && *env) {
      sprintf(sz, "%s%c%s", env, chDirSep, szFileT);
      file = fopen(sz, szMode);
      if (file != NULL)
        goto LDone;
    }

    /* Next look in directory in the version prefix environment variable. */
    env = getenv(ENVIRONVER);
    if (env && *env) {
      sprintf(sz, "%s%c%s", env, chDirSep, szFileT);
      file = fopen(sz, szMode);
      if (file != NULL)
        goto LDone;
    }
#endif

    /* Finally look in one of several compile time specified directories. */
    sprintf(sz, "%s%c%s", nFileMode == 0 ? DEFAULT_DIR :
      (nFileMode == 1 ? CHART_DIR : EPHE_DIR), chDirSep, szFileT);
    file = fopen(sz, szMode);
    if (file != NULL)
      goto LDone;
  }

  if (file == NULL && FOdd(nFileMode)) {
    /* If file was never found, print an error (unless we were looking */
    /* for a certain file type, e.g. the optional astrolog.as file).   */
    sprintf(sz, "File '%s' not found.", szFile);
    PrintError(sz);
  }

LDone:
  if (file != NULL && szPath != NULL) {
    sprintf(szPath, "%s", sz);
    fclose(file);
  }
  return file;
}


/* This is Astrolog's generic file processing routine, which handles chart */
/* info files, position files, and config files. Given a file name or a    */
/* file handle, run through each line as a series of command switches.     */

flag FProcessSwitchFile(CONST char *szFile, FILE *file)
{
  char szLine[cchSzMax], *argv[MAXSWITCHES], ch;
  int argc, i;
  flag fRet = fFalse;

  if (file == NULL) {
    file = FileOpen(szFile, 0, NULL);
    if (file == NULL)
      goto LDone;
  }
  is.fileIn = file;

  /* All files have to begin with the -@ switch file type identifier. */
  ch = getc(file); ungetc(ch, file);
  if (ch != '@') {
    sprintf(szLine,
      "The command file '%s' is not in any valid format (character %d).",
      szFile, (int)ch);
    PrintWarning(szLine);
    goto LDone;
  }

  loop {
    while (!feof(file) && (ch = getc(file)) < ' ')
      ;
    if (feof(file))
      break;
    for (szLine[0] = ch, i = 1; i < cchSzMax && !feof(file) &&
      (uchar)(szLine[i] = getc(file)) >= ' '; i++)
      ;
    szLine[i] = chNull;
    argc = NParseCommandLine(szLine, argv);
    if (!FProcessSwitches(argc, argv))
      goto LDone;
  }
  fRet = fTrue;

LDone:
  is.fileIn = NULL;
  return fRet;
}


/* Take the current chart information, and write it out to the file   */
/* as indicated by the -o switch. This is only executed at the end of */
/* program execution if the -o switch is in effect.                   */

flag FOutputData(void)
{
  char sz[cchSzDef], *pch;
  FILE *file;
  int i, j;
  real dst, rT;

  if (us.fNoWrite)
    return fFalse;
  file = fopen(is.szFileOut, "w");  /* Create and open the file for output. */
  if (file == NULL) {
    sprintf(sz, "File '%s' can not be created.", is.szFileOut);
    PrintError(sz);
    return fFalse;
  }
  if (!us.fWritePos) {

    /* Write the chart information to the file. */

    if (Mon < 1) {
      fclose(file);
      PrintError("Can't output chart with no time/space to file.");
      return fFalse;
    }
    if (us.fWriteOld) {
      dst = Dst;
      if (dst == dstAuto)
        dst = (real)is.fDst;
      fprintf(file, "%d\n%d\n%d\n%.2f\n%.2f\n%.2f\n%.2f\n", Mon, Day, Yea,
        DegToDec(Tim), DegToDec(Zon-dst), DegToDec(Lon), DegToDec(Lat));
    } else {
      fprintf(file, "@AI%s  ; %s chart info.\n", szVerCore, szAppName);
      i = us.fAnsiChar;
      us.fAnsiChar = fFalse;
      fprintf(file, "%cqb %.3s %d %d %s %s ", chSwitch, szMonth[Mon],
        Day, Yea, SzTim(Tim), Dst == 0.0 ? "ST" : (Dst == 1.0 ? "DT" :
        Dst == dstAuto ? "Autodetect" : SzZone(Dst)));
      fprintf(file, "%s %s\n", SzZone(Zon), SzLocation(Lon, Lat));
      /* Don't want double quotes within double quoted string parameters. */
      for (pch = ciMain.nam; *pch; pch++)
        if (*pch == '"')
          *pch = '\'';
      for (pch = ciMain.loc; *pch; pch++)
        if (*pch == '"')
          *pch = '\'';
      fprintf(file, "%czi \"%s\" \"%s\"\n", chSwitch, ciMain.nam, ciMain.loc);
      us.fAnsiChar = i;
    }
  } else {

    /* However, if the -o0 switch is in effect, then write the actual */
    /* positions of the planets and houses to the file instead.       */

    if (us.fWriteOld) {
      for (i = 1; i <= oNorm; i++) {
        j = (int)planet[i];
        fprintf(file, "%.3s: %2d %2d %10.7f\n", szObjName[i],
          j%30, j/30+1, RFract(planet[i])*60.0);              /* Position */
        rT = planetalt[i];
        fprintf(file, "[%c]: %3d %12.8f\n",                   /* Altitude */
          ret[i] >= 0.0 ? 'D' : chRet, (int)(RSgn(rT)*
          RFloor(RAbs(rT))), (rT-(real)(int)rT)*60.0);     /* Retrograde? */
        if (i == oNod)
          i = oFor-1;
        else if (i == oFor)
          i = oMC -1;
        else if (i == oMC)
          i = oAsc-1;
        else if (i == oAsc)
          i = oVtx-1;
        else if (i == oVtx)    /* Skip minor cusps to write Uranians  */
          i = us.fUranian ? uranLo-1 : cObj;
      }
      for (i = 1; i <= cSign/2; i++) {   /* Write first six cusp positions */
        j = (int)chouse[i];
        fprintf(file, "H_%c: %2d %2d %10.7f\n",
          'a'+i-1, j%30, j/30+1, RFract(chouse[i])*60.0);
      }

    } else {
      fprintf(file, "@AP%s  ; %s chart positions.\n", szVerCore, szAppName);
      fprintf(file, "%czi \"%s\" \"%s\"\n", chSwitch, ciMain.nam, ciMain.loc);
      for (i = 0; i <= cObj; i++) if (!ignore[i] || FCusp(i)) {
        fprintf(file, "%cYF ", chSwitch);
        if (i <= oNorm)
          fprintf(file, "%-4.4s", szObjName[i]);
        else
          fprintf(file, "%-4d", i);
        rT = FBetween(i, cuspLo-1+4, cuspLo-1+9) ?
          chouse[i-(cuspLo-1)] : planet[i];
        j = (int)rT;
        fprintf(file, "%3d %.3s %12.9f,",
          j%30, szSignName[j/30+1], RFract(rT)*60.0);
        rT = planetalt[i] < 0.0 && planetalt[i] > -1.0 ?
          planetalt[i] : RFract(RAbs(planetalt[i]));
        fprintf(file, "%4d %13.9f,", (int)planetalt[i], rT*60.0);
        rT = i > oNorm ? 999.0 : (i == oMoo && !us.fEphemFiles ? 0.0026 :
          RLength3(space[i].x, space[i].y, space[i].z));
        fprintf(file, " %13.9f %13.9f\n", ret[i], rT);
      }
    }
  }

  /* Now write any extra strings that were on the command line after the -o */
  /* specification but before the next switch, to the file as comments.     */

  for (i = 1; i < is.cszComment; i++) {
    is.rgszComment++;
    fprintf(file, "%s%s\n", us.fWriteOld ? "" : "; ", is.rgszComment[1]);
  }
  fclose(file);
  return fTrue;
}


/*
******************************************************************************
** User Input Routines.
******************************************************************************
*/

/* Given a string, return an index number corresponding to what the string */
/* indicates, based on a given parsing mode. In most cases this is mainly  */
/* looking up a string in the appropriate array and returning the index.   */

int NParseSz(CONST char *szEntry, int pm)
{
  char szLocal[cchSzMax], ch0, ch1, ch2, ch3;
  CONST char *sz;
  int cch, n, i;

  /* First strip off any leading or trailing spaces. */
  for (cch = 0; (szLocal[cch] = szEntry[cch]); cch++)
    ;
  while (cch && szLocal[cch-1] <= ' ')
    szLocal[--cch] = chNull;
  for (sz = szLocal; *sz && *sz <= ' '; sz++, cch--)
    ;
#ifdef EXPRESS
  if (sz[0] == '~')
    return NParseExpression(sz+1);
#endif

  if (cch >= 3) {
    ch0 = ChCap(sz[0]); ch1 = ChUncap(sz[1]); ch2 = ChUncap(sz[2]);
    switch (pm) {
    /* Parse months, e.g. "February" or "Feb" -> 2 for February. */
    case pmMon:
      for (i = 1; i <= cSign; i++)
        if (FMatchSz(sz, szMonth[i]))
          return i;
      break;
    /* Parse planets, e.g. "Jupiter" or "Jup" -> 6 for Jupiter. */
    case pmObject:
      ch3 = ChUncap(sz[3]);
      for (i = 0; i <= cObj; i++) {
        if (FMatchSz(sz, szObjName[i]))
          return i;
        /* Old style -o0 position files have "XXX:" format for planets. */
        if (ch0 == szObjName[i][0] && ch1 == szObjName[i][1] &&
          ch2 == szObjName[i][2] && ch3 == ':')
          return i;
      }
      /* Allow "Node" to match North Node for compatibility with version */
      /* 5.40 and before, for when the object was called just "Node".    */
      /* Allow "Star" to match first star for -YAm and -YAd switches.    */
      i = SzLookup(rgObjName, szEntry);
      if (i >= 0)
        return i;
      break;
    /* Parse aspects, e.g. "Conjunct" or "Con" -> 1 for the Conjunction. */
    case pmAspect:
      for (i = 1; i <= cAspect2; i++)
        if (FMatchSz(sz, szAspectAbbrev[i]))
          return i;
      i = SzLookup(rgAspectName, szEntry);
      if (i >= 0)
        return i;
      break;
    /* Parse house systems, e.g. "Koch" or "Koc" -> 1 for Koch houses. */
    case pmSystem:
      for (i = 0; i < cSystem; i++)
        if (FMatchSz(sz, szSystem[i]))
          return i;
      i = SzLookup(rgSystem, szEntry);
      if (i >= 0)
        return i;
      break;
    /* Parse zodiac signs, e.g. "Scorpio" or "Sco" -> 8 for Scorpio. */
    case pmSign:
      for (i = 1; i <= cSign; i++)
        if (FMatchSz(sz, szSignName[i]))
          return i;
      break;
    /* Parse color indexes, e.g. "White" or "Whi" -> 15 for White. */
    case pmColor:
      if (FMatchSz(sz, "Ray")) {
        i = atoi(sz + 3);
        if (FBetween(i, 1, cRay))
          return kRayA[i];
      }
      for (i = 0; i < cColor+2; i++)
        if (FMatchSz(sz, szColor[i]))
          return i;
      for (i = 0; i < cElem; i++)
        if (FMatchSz(sz, szElem[i]))
          return kElemA[i];
      for (i = 0; i <= cObj; i++)
        if (FMatchSz(sz, szObjName[i]))
          return kObjA[i];
      for (i = 1; i <= cAspect; i++)
        if (FMatchSz(sz, szAspectAbbrev[i]))
          return kAspA[i];
      if (!FNumCh(ch0))
        return -1;
      break;
    /* Parse RGB color values, e.g. "255,0,0" or "#ff0000" for Red. */
    case pmRGB:
      i = 0;
      if (ch0 == '#' && CchSz(sz) == 7) {
        for (n = 1; sz[n]; n += 2)
          i = (i << 8) | (NHex(ChUncap(sz[n])) << 4) | NHex(ChUncap(sz[n+1]));
      } else {
        for (n = 0;; n++) {
          i = (i << 8) | atoi(&sz[n]);
          while (sz[n] && sz[n] != ',')
            n++;
          if (sz[n] == chNull)
            break;
        }
      }
      i = Rgb(RGBB(i), RGBG(i), RGBR(i));
      return i;
    /* Parse day of week, e.g. "Monday" or "Mon" -> 1 for Monday. */
    case pmWeek:
      for (i = 0; i < cWeek; i++)
        if (FMatchSz(sz, szDay[i]))
          return i;
      return -1;
    }
  }
  if (pm == pmObject && !FNumCh(sz[0]))
    return -1;
  n = atoi(sz);

  if (pm == pmYea) {
    /* For years, process any "BCE" (or "BC.", "b.c.e", and variations) and */
    /* convert an example such as "5BC" to -4. For negative years, note the */
    /* difference of 1, as 1CE/AD was preceeded by 1BCE/BC, with no year 0. */
    i = Max(cch-1, 0);
    if (i && sz[i] == '.')
      i--;
    if (i && ChCap(sz[i]) == 'E')
      i--;
    if (i && sz[i] == '.')
      i--;
    if (i && ChCap(sz[i]) == 'C')
      i--;
    if (i && sz[i] == '.')
      i--;
    if (i && ChCap(sz[i]) == 'B')
      n = 1 - n;
  }
  return n;
}


/* Given a string, return a floating point number corresponding to what the  */
/* string indicates, based on a given parsing mode, like above for integers. */

real RParseSz(CONST char *szEntry, int pm)
{
  char szLocal[cchSzMax], *sz, *pch, ch, chT;
  int cch, i, cColon = 0;
  flag fNeg = fFalse;
  real r;

  /* First strip off any leading or trailing spaces. */
  for (cch = 0; (szLocal[cch] = szEntry[cch]); cch++)
    ;
  while (cch && szLocal[cch-1] <= ' ')
    szLocal[--cch] = chNull;
  for (sz = szLocal; *sz && *sz <= ' '; sz++, cch--)
    ;
#ifdef EXPRESS
  if (sz[0] == '~')
    return RParseExpression(sz+1);
#endif
  /* Capitalize all letters. */
  for (pch = sz; *pch; pch++)
    *pch = ChCap(*pch);
  ch = sz[0];

  if (pm == pmTim) {
    /* For times, process "Noon" and "Midnight" (or just "N" and "M"). */
    if (ch == 'N')
      return 12.0;
    else if (ch == 'M')
      return 0.0;
  } else if (pm == pmDst) {
    /* For the Daylight time flag, "Daylight", "Yes", and "True" (or just */
    /* their first characters) are all indications to be ahead one hour.  */
    if (ch == 'D' || ch == 'Y' || ch == 'T')
      return 1.0;
    /* "Standard", "No", and "False" mean the normal zero offset. */
    else if (ch == 'S' || ch == 'N' || ch == 'F')
      return 0.0;
    /* Check for "Autodetect" Daylight time. */
    else if (ch == 'A')
      return dstAuto;
  } else if (pm == pmZon) {
    /* For time zones, see if the abbrev is in a table, e.g. "EST" -> 5. */
    for (i = 0; i < cZone; i++)
      if (NCompareSz(sz, szZon[i]) == 0)
        return rZon[i];
    /* Allow "H" prefix for numeric zones, e.g. "H5W". */
    if (ch == 'H') {
      sz++, cch--;
      ch = sz[0];
    }
    /* Negate the value for an "E" in the middle somewhere (e.g. "5E30"). */
    for (i = 0; i < cch; i++) {
      chT = sz[i];
      if (FCapCh(chT)) {
        if (chT == 'E')
          fNeg = fTrue;
        sz[i] = ':';
        break;
      }
    }
  } else if (pm == pmLon || pm == pmLat) {
    /* For locations, negate the value for an "E" or "S" in the middle    */
    /* somewhere (e.g. "105E30" or "27:40S") for eastern/southern values. */
    for (i = 0; i < cch; i++) {
      chT = sz[i];
      if (FCapCh(chT)) {
        if (chT == 'E' || chT == 'S')
          fNeg = fTrue;
        sz[i] = ':';
        break;
      }
    }
  }

  /* Anything still at this point should be in a numeric format. */
  if (!FNumCh(ch) && ch != '+' && ch != '-' && ch != '.')
    return rLarge;
  r = atof(sz);
  if (r < 0.0) {
    fNeg = fTrue;
    neg(r);
  }
  for (pch = sz; *pch && *pch != ':'; pch++)
    ;
  if (*pch == ':') {
    pch++;
    /* Check for Astrodienst style MMSS with no separator between them. */
    if (FNumCh(*pch) && FNumCh(pch[1]) && FNumCh(pch[2]) && FNumCh(pch[3]) &&
      !FNumCh(pch[4]) && pch[4] != ':') {
      i = atoi(pch);
      r += (real)(i / 100) / 60.0 + (real)(i % 100) / 3600.0;
    } else
      r += atof(pch) / 60.0;
    while (*pch && *pch != ':')
      pch++;
    if (*pch == ':')
      r += atof(pch+1) / (60.0*60.0);
  }
  if (fNeg)
    neg(r);

  if (pm == pmTim) {
    /* Backtrack over any time suffix, e.g. "AM", "p.m." and variations. */
    i = Max(cch-1, 0);
    if (i && sz[i] == '.')
      i--;
    if (i && sz[i] == 'M')
      i--;
    if (i && sz[i] == '.')
      i--;
    if (i) {
      chT = sz[i];
      if (chT == 'A')                  /* Adjust value appropriately */
        r = r >= 12.0 ? r-12.0 : r;    /* if AM or PM suffix.        */
      else if (chT == 'P')
        r = r >= 12.0 ? r : r+12.0;
    }
  } else if (pm == pmZon) {
    /* Check for "E" suffix for East of GMT. */
    i = Max(cch-1, 0);
    if (sz[i] == 'E' || (cch >= 9 && sz[cch-4] == ' ' && sz[cch-5] == 'E'))
      neg(r);
  } else if (pm == pmElv) {
    /* Check for "feet" or "ft" suffix for feet instead of meters. */
    i = Max(cch-1, 0);
    if (sz[i] == 'F' || sz[i] == 'T')
      r *= rFtToM;
  } else if (pm == pmLength) {
    /* Check for "cent" or "cm" suffix for centimeters instead of inches. */
    i = Max(cch-1, 0);
    if (sz[i] == 'C' || sz[i] == 'M')
      r /= rInToCm;
  }
  return r;
}


#ifndef WIN
/* Stop and wait for the user to enter a line of text given a prompt to */
/* display and a string buffer to fill with it.                         */

void InputString(CONST char *szPrompt, char *sz)
{
  FILE *file;
  int cch;

  file = is.S; is.S = stdout;
  PrintSz(szPrompt);
  AnsiColor(kYellowA);
  PrintSz(" > ");
  AnsiColor(kDefault);
  if (fgets(sz, cchSzMax, stdin) == NULL)  /* Pressing control-D terminates */
    Terminate(tcForce);                    /* the program on some machines. */
  cch = CchSz(sz);
  while (cch > 0 && sz[cch-1] < ' ')
    cch--;
  sz[cch] = chNull;
  is.S = file;
  is.cchCol = 0;
}


/* Prompt the user for a floating point value, parsing as appropriate, and */
/* make sure it conforms to the specified bounds before returning it.      */

int NInputRange(CONST char *szPrompt, int low, int high, int pm)
{
  char szLine[cchSzDef];
  int n;

  loop {
    InputString(szPrompt, szLine);
    n = NParseSz(szLine, pm);
    if (FBetween(n, low, high))
      return n;
    sprintf(szLine, "Value %d out of range from %d to %d.", n, low, high);
    PrintWarning(szLine);
  }
}


/* This is identical to above except it takes/returns floating point values. */

real RInputRange(CONST char *szPrompt, real low, real high, int pm)
{
  char szLine[cchSzDef];
  real r;

  loop {
    InputString(szPrompt, szLine);
    r = RParseSz(szLine, pm);
    if (FBetween(r, low, high))
      return r;
    sprintf(szLine, "Value %.0f out of range from %.0f to %.0f.",
      r, low, high);
    PrintWarning(szLine);
  }
}
#endif /* WIN */


/* This important procedure gets all the parameters defining the chart that  */
/* will be worked with later. Given a "filename", it gets from it all the    */
/* pertinent chart information. This is more than just reading from a file - */
/* the procedure also takes care of the cases of prompting the user for the  */
/* information and using the time functions to determine the date now - the  */
/* program considers these cases "virtual" files. Furthermore, when reading  */
/* from a real file, we have to check if it was written in the -o0 format.   */

flag FInputData(CONST char *szFile)
{
  FILE *file;
  char sz[cchSzDef], ch;
  int i, fT;
  real k, l, m;

  /* If we are to read from the virtual file "nul" that means to leave the */
  /* chart information alone with whatever settings it may have already.   */

  if (NCompareSz(szFile, szNulCore) == 0) {
    is.fHaveInfo = fTrue;
    return fTrue;
  }

  /* If we are to read from the virtual file "set" then that means use a   */
  /* particular set of chart information generated earlier in the program. */

  if (NCompareSz(szFile, szSetCore) == 0) {
    is.fHaveInfo = fTrue;
    ciCore = ciSave;
    return fTrue;
  }
  if (NCompareSz(szFile, "__t") == 0) {
    is.fHaveInfo = fTrue;
    ciCore = ciTran;
    return fTrue;
  }
  if (NCompareSz(szFile, "__g") == 0) {
    is.fHaveInfo = fTrue;
    ciCore = ciGreg;
    return fTrue;
  }

  /* If we are to read from the virtual file "__1" through "__4" then that */
  /* means copy the chart information from the specified chart slot.       */

  if (NCompareSz(szFile, "__1") == 0) {
    is.fHaveInfo = fTrue;
    ciCore = ciMain;
    return fTrue;
  }
  if (NCompareSz(szFile, "__2") == 0) {
    is.fHaveInfo = fTrue;
    ciCore = ciTwin;
    return fTrue;
  }
  if (NCompareSz(szFile, "__3") == 0) {
    is.fHaveInfo = fTrue;
    ciCore = ciThre;
    return fTrue;
  }
  if (NCompareSz(szFile, "__4") == 0) {
    is.fHaveInfo = fTrue;
    ciCore = ciFour;
    return fTrue;
  }

#ifdef TIME
  /* If we are to read from the file "now" then that means use the time */
  /* functions to calculate the present date and time.                  */

  if (NCompareSz(szFile, szNowCore) == 0) {
    is.fHaveInfo = fTrue;
    SS = us.dstDef; ZZ = us.zonDef; OO = us.lonDef; AA = us.latDef;
    GetTimeNow(&MM, &DD, &YY, &TT, SS, ZZ);
    ciCore.nam = us.namDef; ciCore.loc = us.locDef;
    return fTrue;
  }
#endif

#ifndef WIN
  /* If we are to read from the file "tty" then that means prompt the user */
  /* for all the chart information.                                        */

  if (NCompareSz(szFile, szTtyCore) == 0) {
    file = is.S; is.S = stdout;
    if (!us.fNoSwitches) {
      /* Temporarily disable an internal redirection of output to a file  */
      /* because we always want user headers and prompts to be displayed. */

      AnsiColor(kWhiteA);
      sprintf(sz, "** %s version %s ", szAppName, szVersionCore); PrintSz(sz);
      sprintf(sz, "(See '%cHc' switch for copyrights and credits.) **\n",
        chSwitch); PrintSz(sz);
      AnsiColor(kDefault);
      sprintf(sz, "   Invoke as '%s %cH' for list of command line options.\n",
        SzProcessProgname(is.szProgName), chSwitch); PrintSz(sz);
    }

    MM = NInputRange("Enter month for chart (e.g. '9' 'Sep')",
      1, 12, pmMon);
    DD = NInputRange("Enter day   for chart (e.g. '1' '31') ",
      1, DayInMonth(MM, 0), pmDay);
    YY = NInputRange("Enter year  for chart (e.g. '2020')   ",
      -32000, 32000, pmYea);
    if (FBetween(YY, 0, 99)) {
      sprintf(sz,
        "Assuming first century C.E. is really meant instead of %d.",
        1900 + YY);
      PrintWarning(sz);
    }
    TT = RInputRange("Enter time  for chart (e.g. '18:30' '6:30pm')  ",
      -2.0, 24.0, pmTim);
#ifdef ATLAS
    InputString("Enter name of city or location", sz);
    ciCore.loc = SzPersist(sz);
    if (DisplayAtlasLookup(sz, 0, &i)) {
      ciCore.loc = SzPersist(sz);  // DisplayAtlasLookup changes ciCore.loc.
      if (DisplayTimezoneChanges(is.rgae[i].izn, 0, &ciCore)) {
        sprintf(sz, "Atlas data for %s: (%cT Zone %s) %s\n", SzCity(i),
          ChDst(SS), SzZone(ZZ),
          SzLocation(is.rgae[i].lon, is.rgae[i].lat));
        PrintSz(sz);
      } else
        PrintWarning("Couldn't get time zone data!");
    } else
      PrintWarning("Couldn't find anything in atlas matching location.");
#endif
    SS = RInputRange("Enter if Daylight time in effect (e.g. 'y' '1')",
      -24.0, 24.0, pmDst);
    ZZ = RInputRange("Enter time zone (e.g. '5W' 'ET' for Eastern)   ",
      -24.0, 24.0, pmZon);
    OO = RInputRange("Enter Longitude of place (e.g. '122W20')",
      -rDegHalf, rDegHalf, pmLon);
    AA = RInputRange("Enter Latitude  of place (e.g. '47N36') ",
      -rDegQuad, rDegQuad, pmLat);
    InputString("Enter name or title for chart ", sz);
    ciCore.nam = SzPersist(sz);
#ifndef ATLAS
    InputString("Enter name of city or location", sz);
    ciCore.loc = SzPersist(sz);
#endif
    PrintL();
    is.cchRow = 0;
    is.S = file;
    return fTrue;
  }
#endif /* WIN */

  /* Now that the special cases are taken care of, we can assume we are */
  /* to read from a real file.                                          */

  file = FileOpen(szFile, 1, NULL);
  if (file == NULL)
    return fFalse;
  is.fHaveInfo = fTrue;
  ch = getc(file); ungetc(ch, file);

  /* Read the chart parameters from a standard command switch file. */

  if (ch == '@') {
    fT = is.fSzPersist; is.fSzPersist = fFalse;
    if (!FProcessSwitchFile(szFile, file))
      return fFalse;
    is.fSzPersist = fT;

  /* Read the chart info from an older style -o list of seven numbers. */

  } else if (FNumCh(ch)) {
    SS = 0.0;
    fscanf(file, "%d%d%d", &MM, &DD, &YY);
    fscanf(file, "%lf%lf%lf%lf", &TT, &ZZ, &OO, &AA);
    TT = DecToDeg(TT); ZZ = DecToDeg(ZZ);
    OO = DecToDeg(OO); AA = DecToDeg(AA);
    if (!FValidMon(MM) || !FValidDay(DD, MM, YY) || !FValidYea(YY) ||
      !FValidTim(TT) || !FValidZon(ZZ) || !FValidLon(OO) || !FValidLat(AA)) {
      PrintWarning("Values in old style chart info file are out of range.");
      return fFalse;
    }
    ciCore.nam = ciCore.loc = "";

  /* Read the actual chart positions from a file produced with -o0 switch. */

  } else if (ch == 'S') {
    MM = -1;

    /* Hack: A negative month value means the chart parameters are invalid, */
    /* hence -o0 is in effect and we can assume the chart positions are     */
    /* already in memory so we don't have to calculate them later.          */

    for (i = 1; i <= oNorm; i++) {
      fscanf(file, "%s%lf%lf%lf", sz, &k, &l, &m);
      planet[i] = Mod((l-1.0)*30.0+k+m/60.0);
      fscanf(file, "%s%lf%lf", sz, &k, &l);
      if ((m = k+l/60.0) > rDegHalf)
        m = rDegMax - m;
      planetalt[i] = m;
      ret[i] = sz[1] == 'D' ? 1.0 : -1.0;

      /* -o0 files from versions 3.05 and before don't have the Uranians in  */
      /* them. Be prepared to skip over them in old files for compatibility. */

      if (i == oVtx) {
        while (getc(file) >= ' ')
          ;
        if ((ch = getc(file)) != 'H')
          i = cuspHi;
        else
          i = cObj;
      }
      if (i == oNod)
        i = oFor-1;
      else if (i == oFor)
        i = oLil-1;
      else if (i == oLil)
        i = oEP -1;
      else if (i == oEP)
        i = oVtx-1;
    }
    for (i = 1; i <= cSign/2; i++) {
      fscanf(file, "%s%lf%lf%lf", sz, &k, &l, &m);
      chouse[i+6] = Mod((chouse[i] = Mod((l-1.0)*30.0+k+m/60.0))+rDegHalf);
    }
    for (i = 1; i <= cSign; i++)
      planet[cuspLo-1+i] = chouse[i];
    planet[oMC] = planet[oLil]; planet[oNad] = Mod(planet[oMC]  + rDegHalf);
    planet[oAsc] = planet[oEP]; planet[oDes] = Mod(planet[oAsc] + rDegHalf);
    planet[oSou] = Mod(planet[oNod] + rDegHalf); ret[oSou] = ret[oNod];

  } else {
    sprintf(sz,
      "The chart info file is not in any valid format (character %d).",
      (int)ch);
    PrintWarning(sz);
    return fFalse;
  }
  fclose(file);
  return fTrue;
}

/* io.cpp */
