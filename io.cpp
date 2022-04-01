/*
** Astrolog (Version 7.40) File: io.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2022 by
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
** Last code change made 3/31/2022.
*/

#include "astrolog.h"
#ifdef PC
#include <io.h>
#else
#include <dirent.h>
#endif


/*
******************************************************************************
** File IO Routines.
******************************************************************************
*/

// Open the file indicated by the given string and return the file's stream
// pointer, or NULL if the file couldn't be found or opened. All parts of the
// program which open files to read call this routine. We look in several
// various locations and directories for the file before giving up.

FILE *FileOpen(CONST char *szFile, int nFileMode, char *szPath)
{
  FILE *file;
  char szFileT[cchSzMax], sz[cchSzMax], szMode[3], *pch;
#ifdef ENVIRON
  char *env;
#endif
  int i, j;

  // Some file types we want to open as binary instead of Ascii.
  sprintf(szMode, "r%s", nFileMode >= 2 ? "b" : "");

  for (i = 0; i <= 1; i++) {
    if (i <= 0)
      sprintf(szFileT, "%s", szFile);
    else {
      if (nFileMode > 1)
        break;
      sprintf(szFileT, "%s.as", szFile);
    }

    // First look for the file in the directory of the Astrolog executable.
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

    // Next look for the file in the current directory.
    sprintf(sz, "%s", szFileT);
    file = fopen(sz, szMode);
    if (file != NULL)
      goto LDone;

    // Next look in the directories indicated by the -Yi switch.
    for (j = 0; j < 10; j++)
      if (us.rgszPath[j] && *us.rgszPath[j]) {
        sprintf(sz, "%s%c%s", us.rgszPath[j], chDirSep, szFileT);
        file = fopen(sz, szMode);
        if (file != NULL)
          goto LDone;
      }

#ifdef ENVIRON
    // Next look for the file in the directory indicated by the version
    // specific system environment variable.
    sprintf(sz, "%s%s", ENVIRONVER, szVerCore);
    env = getenv(sz);
    if (env && *env) {
      sprintf(sz, "%s%c%s", env, chDirSep, szFileT);
      file = fopen(sz, szMode);
      if (file != NULL)
        goto LDone;
    }

    // Next look in the directory in the general environment variable.
    env = getenv(ENVIRONALL);
    if (env && *env) {
      sprintf(sz, "%s%c%s", env, chDirSep, szFileT);
      file = fopen(sz, szMode);
      if (file != NULL)
        goto LDone;
    }

    // Next look in directory in the version prefix environment variable.
    env = getenv(ENVIRONVER);
    if (env && *env) {
      sprintf(sz, "%s%c%s", env, chDirSep, szFileT);
      file = fopen(sz, szMode);
      if (file != NULL)
        goto LDone;
    }
#endif

    // Finally look in one of several compile time specified directories.
    sprintf(sz, "%s%c%s", nFileMode == 0 ? DEFAULT_DIR :
      (nFileMode == 1 ? CHART_DIR : EPHE_DIR), chDirSep, szFileT);
    file = fopen(sz, szMode);
    if (file != NULL)
      goto LDone;
  }

  if (file == NULL && FOdd(nFileMode)) {
    // If file was never found, print an error (unless we were looking for a
    // certain file type, e.g. the optional astrolog.as file).
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


// Read an 8 bit byte from a file.

byte BRead(FILE *file)
{
  return getc(file);
}


// Read a 16 bit word from a file.

word WRead(FILE *file)
{
  byte b1, b2;

  b1 = getbyte(); b2 = getbyte();
  return WFromBB(b1, b2);
}


// Read a 32 bit long from a file.

dword LRead(FILE *file)
{
  byte b1, b2, b3, b4;

  b1 = getbyte(); b2 = getbyte(); b3 = getbyte(); b4 = getbyte();
  return LFromWW(WFromBB(b1, b2), WFromBB(b3, b4));
}


// This is Astrolog's generic file processing routine, which handles chart
// info files, position files, and config files. Given a file name or a file
// handle, run through each line as a series of command switches.

flag FProcessSwitchFile(CONST char *szFile, FILE *file)
{
  char szLine[cchSzLine], *argv[MAXSWITCHES], ch;
  int argc, i;
  flag fHaveFile, fRet = fFalse;

  fHaveFile = (file != NULL);
  if (!fHaveFile) {
    file = FileOpen(szFile, 0, NULL);
    if (file == NULL)
      goto LDone;
  }
  is.fileIn = file;

  // All files have to begin with the -@ switch file type identifier.
  ch = getc(file); ungetc(ch, file);
  if (ch != '@') {
    sprintf(szLine,
      "The command file '%s' is not in any valid format (character %d).",
      szFile, (int)ch);
    PrintWarning(szLine);
    goto LDone;
  }
  loop {
    while (!feof(file) && (ch = getbyte()) < ' ')
      ;
    if (feof(file))
      break;
    for (szLine[0] = ch, i = 1; i < cchSzLine-1 && !feof(file) &&
      (uchar)(ch = getbyte()) >= ' '; i++)
      szLine[i] = ch;
    if (ch != '\n')
      ch = getbyte();
    szLine[i] = chNull;
    argc = NParseCommandLine(szLine, argv);
    if (!FProcessSwitches(argc, argv))
      goto LDone;
  }
  fRet = fTrue;

LDone:
  is.fileIn = NULL;
  if (!fHaveFile && file != NULL)
    fclose(file);
  return fRet;
}


// Take the current chart information, and write it out to the file as
// indicated by the -o switch. This is only executed at the end of program
// execution if the -o switch is in effect.

flag FOutputData(void)
{
  char sz[cchSzMax], *pch;
  FILE *file;
  int i, j, iMax;
  real dst, rT;

  if (us.fNoWrite)
    return fFalse;

  // Write other file formats if -od, -ol, -oa, or -oq is in effect.
  if (us.nWriteFormat == 'd')
    return FOutputSettings();
  else if (us.nWriteFormat == 'l')
    return FOutputChartList();
  else if (us.nWriteFormat == 'a')
    return FOutputAAFFile();
  else if (us.nWriteFormat == 'q')
    return FOutputQuickFile();

  file = fopen(is.szFileOut, "w");  // Create and open the file for output.
  if (file == NULL) {
    sprintf(sz, "File '%s' can not be created.", is.szFileOut);
    PrintError(sz);
    return fFalse;
  }
  if (us.nWriteFormat != '0') {

    // Write the chart information to the file.

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
      // Don't want double quotes within double quoted string parameters.
      fprintf(file, "%czi \"", chSwitch);
      for (pch = ciMain.nam; *pch; pch++)
        putc(*pch != '"' ? *pch : '\'', file);
      fprintf(file, "\" \"");
      for (pch = ciMain.loc; *pch; pch++)
        putc(*pch != '"' ? *pch : '\'', file);
      fprintf(file, "\"\n");
      us.fAnsiChar = i;
    }
  } else {

    // However, if the -o0 switch is in effect, then write the actual
    // positions of the planets and houses to the file instead.

    if (us.fWriteOld) {
      for (i = 1; i <= oNorm; i++) {
        j = (int)planet[i];
        fprintf(file, "%.3s: %2d %2d %10.7f\n", szObjName[i],
          j%30, j/30+1, RFract(planet[i])*60.0);              // Position
        rT = planetalt[i];
        fprintf(file, "[%c]: %3d %12.8f\n",                   // Altitude
          ret[i] >= 0.0 ? 'D' : chRet, (int)(RSgn(rT)*
          RFloor(RAbs(rT))), (rT-(real)(int)rT)*60.0);        // Retrograde?
        if (i == oNod)
          i = oFor-1;
        else if (i == oFor)
          i = oMC -1;
        else if (i == oMC)
          i = oAsc-1;
        else if (i == oAsc)
          i = oVtx-1;
        else if (i == oVtx)    // Skip minor cusps to write Uranians
          i = us.fUranian ? uranLo-1 : cObj;
      }
      for (i = 1; i <= cSign/2; i++) {    // Write first six cusp positions
        j = (int)chouse[i];
        fprintf(file, "H_%c: %2d %2d %10.7f\n",
          'a'+i-1, j%30, j/30+1, RFract(chouse[i])*60.0);
      }

    } else {
      fprintf(file, "@AP%s  ; %s chart positions.\n", szVerCore, szAppName);
      fprintf(file, "%czi \"%s\" \"%s\"\n", chSwitch, ciMain.nam, ciMain.loc);
      iMax = Max(is.nObj, cuspHi);
      for (i = 0; i <= iMax; i++) if (!ignore[i] || FCusp(i)) {
        fprintf(file, "%cYF ", chSwitch);
        fprintf(file, i != oUrT && i != oAlr ? "%-4.4s" : "%-6.6s",
          szObjName[i]);
        rT = FBetween(i, cuspLo-1+4, cuspLo-1+9) ?
          chouse[i-(cuspLo-1)] : planet[i];
        j = (int)rT;
        fprintf(file, "%3d %.3s %12.9f,",
          j%30, szSignName[j/30+1], RFract(rT)*60.0);
        rT = planetalt[i] < 0.0 && planetalt[i] > -1.0 ?
          planetalt[i] : RFract(RAbs(planetalt[i]));
        fprintf(file, "%4d %13.9f,", (int)planetalt[i], rT*60.0);
        rT = i > oNorm ? 999.0 : (i == oMoo && !us.fEphemFiles ? 0.0026 :
          PtLen(space[i]));
        fprintf(file, " %13.9f %13.9f\n", ret[i], rT);
      }
    }
  }

  // Now write any extra strings that were on the command line after the -o
  // specification but before the next switch, to the file as comments.

  for (i = 1; i < is.cszComment; i++) {
    is.rgszComment++;
    fprintf(file, "%s%s\n", us.fWriteOld ? "" : "; ", is.rgszComment[1]);
  }
  fclose(file);
  return fTrue;
}


#define AdvancePast(ch) while (*pch && *pch != (ch)) pch++; \
  if (*pch == (ch)) pch++;

// Load an Astrological Exchange Format (AAF) file into the default set of
// chart information, given a file name or a file handle.

flag FProcessAAFFile(CONST char *szFile, FILE *file)
{
  char szLine[cchSzLine], sz[cchSzMax], *pch, *sz1, *sz2, ch;
  int i, grf;
  flag fHaveFile, fRet = fFalse;

  fHaveFile = (file != NULL);
  if (!fHaveFile) {
    file = FileOpen(szFile, 0, NULL);
    if (file == NULL)
      goto LDone;
  }
  is.fileIn = file;
  do {

  grf = 0;
  while (grf != 3) {
    while (!feof(file) && (ch = getc(file)) < ' ')
      ;
    if (feof(file))
      break;
    for (szLine[0] = ch, i = 1; i < cchSzLine-1 && !feof(file) &&
      (uchar)(szLine[i] = getc(file)) >= ' '; i++)
      ;
    szLine[i] = chNull;
    if (szLine[0] == chNull)
      continue;
    if (szLine[0] != '#') {
      sprintf(szLine,
        "The AAF file '%s' has a line not starting with '#' (character %d).",
        szFile, (int)ch);
      PrintWarning(szLine);
      goto LDone;
    }
    if (szLine[1] == ':')  // Skip over comment lines
      continue;

    // Input row #1.
    if (FEqRgch(szLine, "#A93:", 5, fFalse)) {
      sz1 = pch = szLine + 5;
      AdvancePast(',');
      pch[-1] = chNull;
      sz2 = pch;
      AdvancePast(',');
      pch[-1] = chNull;
      if (*sz1 && !FEqSz(sz1, "*"))
        sprintf(sz, "%s %s", sz2, sz1);
      else
        sprintf(sz, "%s", sz2);
      ciCore.nam = SzPersist(sz);
      AdvancePast(',');
      DD = NFromSz(pch);
      AdvancePast('.');
      MM = NParseSz(pch, pmMon);
      AdvancePast('.');
      YY = NParseSz(pch, pmYea);
      AdvancePast(',');
      TT = RParseSz(pch, pmTim);
      AdvancePast(',');
      sz1 = pch;
      AdvancePast(',');
      pch[-1] = chNull;
      sz2 = pch;
      // Convert something like "Seattle, WA (USA)" to "Seattle, WA, USA".
      AdvancePast('(');
      if (pch[-1] == '(') {
        pch[-2] = ','; pch[-1] = ' ';
        for (; *pch; pch++)
          if (*pch == ')')
            *pch = chNull;
      }
      if (*sz1)
        sprintf(sz, "%s, %s", sz1, sz2);
      else
        sprintf(sz, "%s", sz2);
      ciCore.loc = SzPersist(sz);
      grf |= 1;

    // Input row #2.
    } else if (FEqRgch(szLine, "#B93:", 5, fFalse)) {
      pch = szLine + 5;
      AdvancePast(',');
      AA = RParseSz(pch, pmLon);
      AdvancePast(',');
      OO = RParseSz(pch, pmLat);
      AdvancePast(',');
      ZZ = RParseSz(pch, pmZon);
      AdvancePast(',');
      if (*pch == 'D')         // Double Daylight Saving time
        SS = 2.0;
      else if (*pch == 'L')    // Local Mean Time
        SS = 0.0;
      else
        SS = RParseSz(pch, pmDst);
      grf |= 2;
    } else {
      sprintf(szLine,
        "The AAF file '%s' has a line that can't be parsed.", szFile);
      PrintWarning(szLine);
      goto LDone;
    }
  }
  if (!FValidMon(MM) || !FValidDay(DD, MM, YY) || !FValidYea(YY) ||
    !FValidTim(TT) || !FValidZon(ZZ) || !FValidLon(OO) || !FValidLat(AA)) {
    PrintWarning("Values in AAF file are out of range.");
    goto LDone;
  }
  if (!FAppendCIList(&ciCore))
    goto LDone;

  } while (!feof(file));
  fRet = fTrue;
LDone:
  is.fileIn = NULL;
  if (!fHaveFile)
    fclose(file);
  return fRet;
}


// Write the current chart information to an Astrological Exchange Format
// (AAF) file, as indicated by the -oa switch.

flag FOutputAAFFile(void)
{
  char sz[cchSzMax], *pch;
  FILE *file;
  int nSav;
  flag fSav;

  if (us.fNoWrite)
    return fFalse;
  file = fopen(is.szFileOut, "w");  // Create and open the file for output.
  if (file == NULL) {
    sprintf(sz, "AAF file '%s' can not be created.", is.szFileOut);
    PrintError(sz);
    return fFalse;
  }

  fprintf(file, "#: %s %s\n", szAppName, szVersionCore);

  // Output row #1.
  fprintf(file, "#A93:*,");
  for (pch = ciMain.nam; *pch; pch++)
    putc(*pch == ',' ? ';' : *pch, file);
  fSav = us.fEuroTime; us.fEuroTime = fTrue;
  fprintf(file, ",*,%d.%d.%d,%s,", Day, Mon, Yea, SzTim(Tim));
  us.fEuroTime = fSav;
  for (pch = ciMain.loc; *pch && *pch != ','; pch++)
    putc(*pch, file);
  putc(',', file);
  if (!*pch)
    putc('*', file);
  else {
    // Convert commas to other characters, since they delimit fields.
    for (pch++; *pch == ' '; pch++)
      ;
    for (; *pch && *pch != ','; pch++)
      putc(*pch, file);
    if (*pch == ',') {
      // Convert something like "Seattle, WA, USA" to "Seattle, WA (USA)".
      fprintf(file, " (");
      for (pch++; *pch == ' '; pch++)
        ;
      for (; *pch; pch++)
        putc(*pch == ',' ? ';' : *pch, file);
      putc(')', file);
    }
  }

  // Output row #2.
  nSav = us.fAnsiChar; us.fAnsiChar = 4;
  fprintf(file, "\n#B93:%f,%s,%s,", JulianDayFromTime(is.T),
    SzLocation(Lon, Lat), Zon != zonLMT ? SzZone(Zon) : "*");
  us.fAnsiChar = nSav;
  fprintf(file, "%c\n", Zon == zonLMT ? 'L' : (Dst != 0.0 ? '1' : '0'));

  fprintf(file, "#: AAF end\n");
  fclose(file);
  return fTrue;
}


#define FZonDst(sz) ((sz)[1] == 'D' || (sz)[1] == 'W')

// Load a Quick*Chart format file into the default set of chart information,
// given a file name or a file handle.

flag FProcessQuickFile(CONST char *szFile, FILE *file)
{
  char szLine[cchSzMax], sz[cchSzDef];
  int i;
  flag fHaveFile, fRet = fFalse;

  fHaveFile = (file != NULL);
  if (!fHaveFile) {
    file = FileOpen(szFile, 0, NULL);
    if (file == NULL)
      goto LDone;
  }
  is.fileIn = file;
  do {
    fgets(szLine, cchSzMax, file);
    if (feof(file)) {
      fRet = fTrue;
      goto LDone;
    }

    // Parse line of 100 characters.
    CopyRgb((pbyte)szLine, (pbyte)sz, 23);
    for (i = 23-1; i >= 0 && sz[i] <= ' '; i--)
      ;
    sz[i+1] = chNull;
    ciCore.nam = SzPersist(sz);
    CopyRgchToSz(szLine+23, 3, sz, cchSzDef);
    MM = NParseSz(sz, pmMon);
    DD = NParseSz(szLine+23+3, pmDay);
    CopyRgchToSz(szLine+23+7, 5, sz, cchSzDef);
    YY = NParseSz(sz, pmYea);
    CopyRgchToSz(szLine+23+12, 24-12, sz, cchSzDef);
    TT = RParseSz(sz, pmTim);
    if (FEqRgch(&szLine[23+24], "LMT", 3, fTrue))
      ZZ = zonLMT;
    else {
      CopyRgchToSz(szLine+23+24+3, 9-3, sz, cchSzDef);
      ZZ = RParseSz(sz, pmZon);
    }
    CopyRgchToSz(szLine+23+24+9, 10, sz, cchSzDef);
    OO = RParseSz(sz, pmLon);
    CopyRgchToSz(szLine+23+24+9+10, 9, sz, cchSzDef);
    AA = RParseSz(sz, pmLat);
    CopyRgb((pbyte)(szLine+23+24+9+10+9), (pbyte)sz, 25);
    for (i = 25-1; i >= 0 && sz[i] <= ' '; i--)
      ;
    sz[i+1] = chNull;
    ciCore.loc = SzPersist(sz);
    SS = 0.0;
    if (FZonDst(&szLine[23+24])) {
      ZZ += 1.0; SS += 1.0;
    }
    if (!FValidMon(MM) || !FValidDay(DD, MM, YY) || !FValidYea(YY) ||
      !FValidTim(TT) || !FValidZon(ZZ) || !FValidLon(OO) || !FValidLat(AA)) {
      PrintWarning("Values in Quick*Chart file are out of range.");
      goto LDone;
    }
    if (!FAppendCIList(&ciCore))
      goto LDone;
  } while (!feof(file));

  fRet = fTrue;
LDone:
  is.fileIn = NULL;
  if (!fHaveFile)
    fclose(file);
  return fRet;
}


// Write the current chart information or current chart list to an Quick*Chart
// format file, as indicated by the -oq switch.

flag FOutputQuickFile(void)
{
  char sz[cchSzMax];
  FILE *file = NULL;
  real rOff, rDst;
  int i, iMax, j, fSav1;
  flag fRet = fFalse, fSav2;
  CI *pci;

  if (us.fNoWrite)
    goto LDone;
  file = fopen(is.szFileOut, "w");  // Create and open the file for output.
  if (file == NULL) {
    sprintf(sz, "Quick*Chart file '%s' can not be created.", is.szFileOut);
    PrintError(sz);
    goto LDone;
  }

  iMax = (is.cci > 1 ? is.cci : 1);
  for (i = 0; i < iMax; i++) {
    pci = iMax <= 1 ? &ciMain : &is.rgci[i];
    if (!FBetween(pci->yea, -9999, 99999)) {
      PrintWarning("Can't save this chart to Quick*Chart format because the "
        "Year field is more than 5 characters long.");
      goto LDone;
    }

    // Output line of 101 characters.
    fprintf(file, "%-23.23s", pci->nam);
    fprintf(file, "%c%c%c", szMonth[pci->mon][0],
      ChCap(szMonth[pci->mon][1]), ChCap(szMonth[pci->mon][2]));
    fSav1 = us.fAnsiChar; us.fAnsiChar = 4;
    fSav2 = is.fSeconds; is.fSeconds = fTrue;
    fprintf(file, "%3d,%5d%s", pci->day, pci->yea, SzTim(pci->tim));
    us.fAnsiChar = fSav1; is.fSeconds = fSav2;
    rDst = (pci->dst == dstAuto ? (real)is.fDst : pci->dst);
    rOff = (pci->zon == zonLMT ? pci->lon / 15.0 : pci->zon) - rDst;
    if (pci->zon == zonLMT)
      sprintf(sz, "LMT");
    else {
      sprintf(sz, "   ");
      // Search for the 3 character time zone string that best matches offset.
      for (j = 0; j < cZone; j++)
        if (FSameR(rZon[j], rOff) && CchSz(szZon[j]) == 3 &&
          FZonDst(szZon[j]) == (rDst == 1.0)) {
          sprintf(sz, "%s", szZon[j]);
          break;
        }
    }
    fprintf(file, " %s%c%02d:%02d", sz, rOff < 0.0 ? '-' : '+',
      (int)RAbs(rOff), (int)(RFract(RAbs(rOff))*60.0+rRound/60.0));
    fprintf(file, "%03d%c%02d'%02d", (int)RAbs(pci->lon), pci->lon < 0.0 ?
      'E' : 'W', (int)(RFract(RAbs(pci->lon))*60.0+rRound/60.0), 0);
    fprintf(file, " %02d%c%02d'%02d", (int)RAbs(pci->lat), pci->lat < 0.0 ?
      'S' : 'N', (int)(RFract(RAbs(pci->lat))*60.0+rRound/60.0), 0);
    fprintf(file, " %-25.25s \n", pci->loc);
  }

LDone:
  if (file != NULL)
    fclose(file);
  return fRet;
}


// Load a Astrodatabank XML format file into the default set of chart
// information, given a file name or a file handle.

flag FProcessADBFile(CONST char *szFile, FILE *file)
{
  char szLine[cchSzLine], sz[cchSzDef], szLoc1[cchSzDef], szLoc2[cchSzDef],
    ch, *pch, *pch2;
  int i, grf, pm = 0;
  flag fHaveFile, fDidOne = fFalse, fDidStart, fDidLon, fRet = fFalse;

  fHaveFile = (file != NULL);
  if (!fHaveFile) {
    file = FileOpen(szFile, 0, NULL);
    if (file == NULL)
      goto LDone;
  }
  is.fileIn = file;
  do {

  fDidStart = fFalse;
  grf = 0;
  while (grf != 2047) {
    while (!feof(file) && (ch = getc(file)) < ' ')
      ;
    if (feof(file))
      break;
    for (szLine[0] = ch, i = 1; i < cchSzLine-1 && !feof(file) &&
      (uchar)(szLine[i] = getc(file)) >= ' '; i++)
      ;
    szLine[i] = chNull;

    fDidLon = fFalse;
    for (pch = szLine; *pch; pch++) {
      if (FEqRgch(pch, "<adb_entry", 10, fFalse))
        fDidStart = fTrue;
      else if (!fDidStart)
        continue;
      if ((grf & 1) == 0 && FEqRgch(pch, "imonth=\"", 8, fFalse)) {
        MM = atoi(pch + 8);
        grf |= 1;
      }
      if ((grf & 2) == 0 && FEqRgch(pch, "iday=\"", 6, fFalse)) {
        DD = atoi(pch + 6);
        grf |= 2;
      }
      if ((grf & 4) == 0 && FEqRgch(pch, "iyear=\"", 7, fFalse)) {
        YY = atoi(pch + 7);
        grf |= 4;
      }
      if ((grf & 8) == 0 && FEqRgch(pch, "sbtime_ampm=\"", 13, fFalse)) {
        pch += 13;
        for (pch2 = pch; *pch2 && *pch2 != '"'; pch2++)
          ;
        CopyRgchToSz(pch, pch2 - pch, sz, cchSzDef);
        if (*sz)
          TT = RParseSz(sz, pmTim);
        else
          TT = 12.0;  // Some records are "unknown, 12:00 used"
        grf |= 8;
      }
      if ((grf & 16) == 0 && FEqRgch(pch, "ctimetype=\"", 11, fFalse)) {
        if (pch[11] != 'l') {
          SS = pch[11] == 'd' ? 1.0 : 0.0;
          grf |= 16;
        } else {
          SS = 0.0; ZZ = zonLMT;
          grf |= (16 | 32);
        }
      }
      if ((grf & 32) == 0 && FEqRgch(pch, "stmerid=\"", 9, fFalse)) {
        pch += 9;
        for (pch2 = pch; *pch2 && *pch2 != '"'; pch2++)
          ;
        CopyRgchToSz(pch, pch2 - pch, sz, cchSzDef);
        ZZ = RParseSz(sz, pmZon);
        grf |= 32;
      }
      if ((grf & 64) == 0 && FEqRgch(pch, "slong=\"", 7, fFalse)) {
        OO = RParseSz(pch + 7, pmLon);
        grf |= 64;
        fDidLon = fTrue;
      }
      if ((grf & 128) == 0 && FEqRgch(pch, "slati=\"", 7, fFalse)) {
        AA = RParseSz(pch + 7, pmLat);
        grf |= 128;
      }
      if ((grf & 256) == 0 && FEqRgch(pch, "<sflname>", 9, fFalse)) {
        pch += 9;
        while (*pch == ' ')
          pch++;
        for (pch2 = pch; *pch2 && *pch2 != '<'; pch2++)
          ;
        CopyRgchToSz(pch, pch2 - pch, sz, cchSzDef);
        ConvertSzFromUTF8(sz);
        ciCore.nam = SzPersist(sz);
        grf |= 256;
      }
      if ((grf & 512) == 0 && fDidLon && FEqRgch(pch, "\">", 2, fFalse)) {
        pch += 2;
        for (pch2 = pch; *pch2 && *pch2 != '<'; pch2++)
          ;
        CopyRgchToSz(pch, pch2 - pch, szLoc1, cchSzDef);
        grf |= 512;
      }
      if ((grf & 1024) == 0 && FEqRgch(pch, "<country", 8, fFalse)) {
        for (pch2 = pch + 8; *pch2 && *pch2 != '>'; pch2++)
          ;
        pch = pch2 + (*pch2 == '>');
        for (pch2 = pch; *pch2 && *pch2 != '<'; pch2++)
          ;
        CopyRgchToSz(pch, pch2 - pch, szLoc2, cchSzDef);
        sprintf(sz, "%s, %s", szLoc1, szLoc2);
        ConvertSzFromUTF8(sz);
        ciCore.loc = SzPersist(sz);
        grf |= 1024;
      }
    }
  }

  if (grf != 2047) {
    if (grf == 0 && fDidOne) {
      fRet = fTrue;
      goto LDone;
    }
    PrintWarning("Couldn't detect all fields in Astrodatabank file.");
    goto LDone;
  }
  ZZ += SS;
  if (!FValidMon(MM) || !FValidDay(DD, MM, YY) || !FValidYea(YY) ||
    !FValidTim(TT) || !FValidZon(ZZ) || !FValidLon(OO) || !FValidLat(AA)) {
    PrintWarning("Values in Astrodatabank file are out of range.");
    goto LDone;
  }
  if (!FAppendCIList(&ciCore))
    goto LDone;
  fDidOne = fTrue;

  } while (!feof(file));
  fRet = fTrue;

LDone:
  is.fileIn = NULL;
  if (!fHaveFile)
    fclose(file);
  return fRet;
}


// Load a Solar Fire chart export text file into the default set of chart
// information, given a file name or a file handle.

flag FProcessSFTextFile(CONST char *szFile, FILE *file)
{
  char szLine[cchSzLine], *pch, *pch2;
  int nState = -1, cch;
  flag fHaveFile, fRet = fFalse;

  fHaveFile = (file != NULL);
  if (!fHaveFile) {
    file = FileOpen(szFile, 0, NULL);
    if (file == NULL)
      goto LDone;
  }
  is.fileIn = file;
  loop {

  fgets(szLine, cchSzMax, file);
  if (feof(file))
    break;
  for (pch = szLine; *pch; pch++)
    ;
  while (pch > szLine && pch[-1] <= ' ')
    *(--pch) = chNull;

  if (nState <= 0) {
    if (*szLine == chNull)
      continue;
    if (nState < 0 && FMatchSz("Charts from Solar Fire", szLine))
      continue;
    if (nState < 0 && FMatchSz("Created by Esoteric Technologies", szLine)) {
      nState = 0;
      continue;
    }
    if (nState == 0)
      nState = 1;
    else
      break;
  }
  if (nState == 1) {
    for (pch = szLine; *pch; pch++)
      ;
    cch = pch - szLine;
    if (cch > 14 && FEqRgch(pch - 14, " - Natal Chart", 14, fFalse))
      pch[-14] = chNull;
    else if (cch > 13 && FEqRgch(pch - 13, " - Male Chart", 13, fFalse))
      pch[-13] = chNull;
    else if (cch > 15 && FEqRgch(pch - 15, " - Female Chart", 15, fFalse))
      pch[-15] = chNull;
    ciCore.nam = SzPersist(szLine);
    nState = 2;
  } else if (nState == 2) {
    for (pch = szLine; *pch && *pch <= ' '; pch++)
      ;
    while (*pch > ' ')
      pch++;
    if (*pch == ' ')
      *pch++ = chNull;
    MM = NParseSz(szLine, pmMon);
    while (*pch && *pch <= ' ')
      pch++;
    DD = NParseSz(pch, pmDay);
    while (*pch > ' ')
      pch++;
    while (*pch && *pch <= ' ')
      pch++;
    YY = NParseSz(pch, pmYea);
    while (*pch && *pch != ',')
      pch++;
    while (*pch && (*pch == ',' || *pch == ' '))
      pch++;
    for (pch2 = pch; *pch && *pch != ','; pch++)
      ;
    if (*pch == ',')
      *pch++ = chNull;
    TT = RParseSz(pch2, pmTim);
    while (*pch && *pch == ' ')
      pch++;
    if (FBetween(*pch, 'A', 'Z')) {
      while (*pch > ' ')
        pch++;
      SS = pch[-2] == 'D' ? 1.0 : 0.0;
    } else
      SS = 0.0;
    ZZ = RParseSz(pch, pmZon) + SS;
    nState = 3;
  } else if (nState == 3) {
    for (pch2 = szLine; *pch2 <= ' '; pch2++)
      ;
    for (pch = pch2; *pch; pch++)
      ;
    while (pch > szLine && pch[-1] != ',')
      pch--;
    if (pch > szLine && pch[-1] == ',') {
      pch--;
      while (pch > szLine && pch[-1] != ',')
        pch--;
      if (pch > szLine && pch[-1] == ',')
        pch[-1] = chNull;
    }
    ciCore.loc = SzPersist(pch2);
    AA = RParseSz(pch, pmLat);
    while (*pch && *pch != ',')
      pch++;
    while (*pch && (*pch == ',' || *pch == ' '))
      pch++;
    OO = RParseSz(pch, pmLon);
    nState = 4;
    if (!FValidMon(MM) || !FValidDay(DD, MM, YY) || !FValidYea(YY) ||
      !FValidTim(TT) || !FValidZon(ZZ) || !FValidLon(OO) || !FValidLat(AA)) {
      PrintWarning("Values in text file are out of range.");
      goto LDone;
    }
    if (!FAppendCIList(&ciCore))
      goto LDone;
  } else if (nState == 4) {
    if (*szLine == chNull)
      nState = 0;
  }

  } // loop
  if (nState < 0) {
    PrintWarning("Couldn't detect chart information in text file.");
    goto LDone;
  }

  fRet = fTrue;
LDone:
  is.fileIn = NULL;
  if (!fHaveFile)
    fclose(file);
  return fRet;
}


// Output the chart list in memory to an Astrolog chart list file. (If the
// chart list is empty, output a length 1 list consisting of the main chart.)

flag FOutputChartList()
{
  char sz[cchSzDef], *pch;
  FILE *file;
  CI *pci;
  int cci, i, nSav;

  if (us.fNoWrite)
    return fFalse;
  file = fopen(is.szFileOut, "w");    // Create and open the file for output.
  if (file == NULL) {
    sprintf(sz, "Chart list file %s can not be created.", is.szFileOut);
    PrintError(sz);
    return fFalse;
  }

  sprintf(sz, "@AL%s  ; %s chart list.\n", szVerCore, szAppName); PrintFSz();
  cci = is.cci <= 0 ? 1 : is.cci;
  for (i = 0; i < cci; i++) {
    pci = is.cci <= 0 ? &ciMain : &is.rgci[i];
    nSav = us.fAnsiChar;
    us.fAnsiChar = fFalse;
    fprintf(file, "%cqcl %.3s %2d %d %s %s ",
      chSwitch, szMonth[pci->mon], pci->day, pci->yea, SzTim(pci->tim),
      pci->dst == 0.0 ? "ST" : (pci->dst == 1.0 ? "DT" :
      SzZone(pci->dst == dstAuto ? (real)is.fDst : pci->dst)));
    fprintf(file, "%s %s ", SzZone(pci->zon), SzLocation(pci->lon, pci->lat));
    // Don't put double quotes within double quoted string parameters.
    putc('"', file);
    for (pch = pci->nam; *pch; pch++)
      putc(*pch != '"' ? *pch : '\'', file);
    fprintf(file, "\" \"");
    for (pch = pci->loc; *pch; pch++)
      putc(*pch != '"' ? *pch : '\'', file);
    fprintf(file, "\"\n");
    us.fAnsiChar = nSav;
  }
  fclose(file);
  return fTrue;
}


CONST char *szDegForm = "zdhn";
CONST char *szTypSwiss[] = {"", "b", "O", "m", "j"};
CONST char *szPntSwiss[] = {"", "n", "s", "p", "a"};

// Take many of the user visible settings, and write them out to a new command
// switch file, which may be read in to restore those settings. Most often
// this would be used to create a new astrolog.as default settings file. This
// is called from the -od switch and the File / Save Settings menu command.

flag FOutputSettings()
{
  char sz[cchSzDef];
  FILE *file;
  int i;
  flag f1, f2;
#ifdef SWISS
  int j;
  flag fAny = fFalse;
#endif

  if (us.fNoWrite)
    return fFalse;
  file = fopen(is.szFileOut, "w");    // Create and open the file for output.
  if (file == NULL) {
    sprintf(sz, "Settings file %s can not be created.", is.szFileOut);
    PrintError(sz);
    return fFalse;
  }

  sprintf(sz, "@AD%s  ; %s %s default settings file %s\n\n",
    szVerCore, szAppName, szVersionCore, DEFAULT_INFOFILE); PrintFSz();
  PrintF(
    "; The contents of this file can be automatically generated with the\n"
    "; \"File / Save Settings\" menu command, or with the -od command switch."
    "\n\n");

  f1 = is.fSeconds; is.fSeconds = fTrue;
  sprintf(sz, "-z %s                ", SzZone(us.zonDef)); PrintFSz();
  is.fSeconds = f1;
  PrintF("; Default time zone     [hours W or E of UTC   ]\n");
  if (us.dstDef != dstAuto)
    sprintf(sz, "-z0 %d                   ", (int)us.dstDef);
  else
    sprintf(sz, "-z0 Autodetect          ");
  PrintFSz();
  PrintF("; Default Daylight time [0 standard, 1 daylight]\n");
  f1 = us.fAnsiChar; us.fAnsiChar = 3;
  f2 = is.fSeconds; is.fSeconds = fTrue;
  sprintf(sz, "-zl %s  ", SzLocation(us.lonDef, us.latDef)); PrintFSz();
  us.fAnsiChar = f1; is.fSeconds = f2;
  PrintF("; Default location      [longitude and latitude]\n");
  sprintf(sz, "-zv %s               ", SzElevation(us.elvDef)); PrintFSz();
  PrintF("; Default elevation     [in feet or meters     ]\n");
  sprintf(sz, "-zj \"%s\" \"%s\" ; Default name and location\n\n",
    us.namDef, us.locDef); PrintFSz();

  // The -Yz line needs to be before the -n line in order to take effect.
  sprintf(sz, "-Yz %ld   ", us.lTimeAddition); PrintFSz();
  PrintF(
    "; Time minute addition to be used if \"now\" charts are offset.\n");
  PrintF("-n      "
    "; Comment out this line to not start with chart for \"now\".\n\n");

  sprintf(sz, "%cs      ", ChDashF(us.fSidereal)); PrintFSz();
  PrintF(
    "; Which zodiac to use       [\"_s\" is tropical, \"=s\" is sidereal]\n");
  PrintF(":s "); FormatR(sz, us.rZodiacOffset, 6); PrintFSz();
  PrintF(
    "  ; Sidereal zodiac offset    [Change \"0.0\" to desired ayanamsa  ]\n");
  sprintf(sz, ":s%c     ", szDegForm[us.nDegForm]); PrintFSz();
  PrintF(
    "; Zodiac display format     [\"z\" is sign, \"d\" is 0-360 deg, etc]\n");
  sprintf(sz, "-A %d    ", us.nAsp); PrintFSz();
  PrintF(
    "; Number of aspects         [Change \"5\" to desired number      ]\n");
  i = us.nHouseSystem; sprintf(sz, "-c %.4s ", i == hsEqualMC ?
    rgSystem[1].sz : (i == hsSinewaveDelta ? rgSystem[3].sz : szSystem[i]));
  PrintFSz();
  PrintF(
    "; House system              [Change \"Plac\" to desired system   ]\n");
  sprintf(sz, "%cc3     ", ChDashF(us.fHouse3D)); PrintFSz();
  PrintF(
    "; 3D house boundaries       [\"=c3\" is 3D houses, \"_c3\" is 2D   ]\n");
  sprintf(sz, "%ck      ", ChDashF(us.fAnsiColor)); PrintFSz();
  PrintF(
    "; Ansi color text           [\"=k\" is color, \"_k\" is monochrome ]\n");
  sprintf(sz, ":d %d   ", us.nDivision); PrintFSz();
  PrintF(
    "; Searching divisions       [Change \"48\" to desired divisions  ]\n");
  sprintf(sz, "%cb0     ", ChDashF(us.fSeconds)); PrintFSz();
  PrintF(
    "; Print zodiac seconds      [\"_b0\" to minute, \"=b0\" to second  ]\n");
  sprintf(sz, "%cb      ", ChDashF(us.fEphemFiles)); PrintFSz();
  PrintF(
    "; Use ephemeris files       [\"=b\" uses them, \"_b\" doesn't      ]\n");
  sprintf(sz, ":w %d    ", us.nWheelRows); PrintFSz();
  PrintF(
    "; Wheel chart text rows     [Change \"0\" to desired wheel rows  ]\n");
  sprintf(sz, ":I %d   ", us.nScreenWidth); PrintFSz();
  PrintF(
    "; Text screen columns       [Change \"80\" to desired columns    ]\n");
  sprintf(sz, "-YQ %d   ", us.nScrollRow); PrintFSz();
  PrintF(
    "; Text screen scroll limit  [Change \"24\" or set to \"0\" for none]\n");
  sprintf(sz, "%cYn     ", ChDashF(us.fTrueNode)); PrintFSz();
  PrintF(
    "; Which Nodes and Lilith    [\"_Yn\" shows mean, \"=Yn\" shows true]\n");
  sprintf(sz, "%csr0    ", ChDashF(us.fEquator2)); PrintFSz();
  PrintF(
    "; Latitudes or declinations [\"_sr0\" shows lat., \"=sr0\" declin. ]\n");
  sprintf(sz, "%cYr     ", ChDashF(us.fRound)); PrintFSz();
  PrintF(
    "; Show rounded positions    [\"=Yr\" rounds, \"_Yr\" doesn't       ]\n");
  sprintf(sz, "%cYd     ", ChDashF(us.fEuroDate)); PrintFSz();
  PrintF(
    "; European date format      [\"_Yd\" is M/D/Y, \"=Yd\" is D-M-Y    ]\n");
  sprintf(sz, "%cYt     ", ChDashF(us.fEuroTime)); PrintFSz();
  PrintF(
    "; European time format      [\"_Yt\" is AM/PM, \"=Yt\" is 24 hour  ]\n");
  sprintf(sz, "%cYv     ", ChDashF(us.fEuroDist)); PrintFSz();
  PrintF(
    "; European length units     [\"_Yv\" is imperial, \"=Yv\" is metric]\n");
  sprintf(sz, "%cYC     ", ChDashF(us.fSmartCusp)); PrintFSz();
  PrintF(
    "; Smart cusp displays       [\"=YC\" is smart, \"_YC\" is normal   ]\n");
  sprintf(sz, "%cYO     ", ChDashF(us.fSmartSave)); PrintFSz();
  PrintF(
    "; Smart copy and printing   [\"=YO\" does it smart, \"_YO\" doesn't]\n");
  sprintf(sz, "%cY8     ", ChDashF(us.fClip80)); PrintFSz();
  PrintF(
    "; Clip text to end of line  [\"=Y8\" clips, \"_Y8\" doesn't clip   ]\n");
  sprintf(sz, "-Ya%d    ", us.nCharset); PrintFSz();
  PrintF(
    "; Input character encoding  [0-3 is Default, IBM, Latin-1, UTF8]\n");
  sprintf(sz, "-YP %d   ", us.nArabicNight); PrintFSz();
  PrintF(
    "; Arabic part formula       [\"1\" is fixed, \"0\" checks if night ]\n");
  sprintf(sz, "%cYu%c    ", ChDashF(us.fEclipse), us.fEclipseAny ? '0' : ' ');
  PrintFSz();
  PrintF(
    "; Show eclipse information  [\"=Yu0\" shows, \"_Yu0\" doesn't show ]\n");
  sprintf(sz, "%c0n     ", ChDashF(us.fNoNetwork)); PrintFSz();
  PrintF(
    "; Internet Web queries      [\"=0n\" disables them, \"_0n\" allows ]\n");

  sprintf(sz,
    "\n:pd %9.5f ; Progression degrees per day    [365 is secondary]\n",
    us.rProgDay); PrintFSz();
  PrintF(":pC "); FormatR(sz, us.rProgCusp, 5); PrintFSz();
  PrintF("       ; Progressed cusp movement ratio [1.0 is quotidian]\n");

  PrintF("\n\n; FILE PATHS (-Yi1 through -Yi9):\n; For example, "
    "point -Yi1 to ephemeris dir, -Yi2 to chart files dir, etc.\n\n");
  for (i = 0; i < 10; i++)
    if (us.rgszPath[i] && *us.rgszPath[i]) {
      sprintf(sz, "-Yi%d \"%s\"\n", i, us.rgszPath[i]); PrintFSz();
    }

  PrintF("\n\n; DEFAULT RESTRICTIONS:\n"
    ";  0-10: Ear Sun Moo Mer Ven Mar Jup Sat Ura Nep Plu\n"
    "; 11-21: Chi Cer Pal Jun Ves Nor Sou Lil For Ver EP\n"
    "; 22-33: Asc 2nd 3rd Nad 5th 6th Des 8th 9th MC 11th 12th\n"
    "; 34-42: Vul Cup Had Zeu Kro Apo Adm Vulk Pos\n"
    "; 43-51: Hyg Pho Eri Hau Mak Gon Qua Sed Orc\n"
    "; 52-83: Planetary moons\n"
    "; 84-130: Fixed stars\n"
    "\n-YR 0 10     ");
  for (i = 0; i <= 10; i++) PrintF(SzNumF(ignore[i]));
  PrintF("   ; Planets\n-YR 11 21    ");
  for (i = 11; i <= 21; i++) PrintF(SzNumF(ignore[i]));
  PrintF("   ; Minor planets\n-YR 22 33    ");
  for (i = 22; i <= 33; i++) PrintF(SzNumF(ignore[i]));
  PrintF(" ; House cusps\n-YR 34 42    ");
  for (i = 34; i <= 42; i++) PrintF(SzNumF(ignore[i]));
  PrintF("       ; Uranians\n-YR 43 51    ");
  for (i = 43; i <= 51; i++) PrintF(SzNumF(ignore[i]));
  PrintF("       ; Dwarfs\n\n");

  PrintF("; DEFAULT TRANSIT RESTRICTIONS:\n\n-YRT 0 10    ");
  for (i = 0; i <= 10; i++) PrintF(SzNumF(ignore2[i]));
  PrintF("   ; Planets\n-YRT 11 21   ");
  for (i = 11; i <= 21; i++) PrintF(SzNumF(ignore2[i]));
  PrintF("   ; Minor planets\n-YRT 22 33   ");
  for (i = 22; i <= 33; i++) PrintF(SzNumF(ignore2[i]));
  PrintF(" ; House cusps\n-YRT 34 42   ");
  for (i = 34; i <= 42; i++) PrintF(SzNumF(ignore2[i]));
  PrintF("       ; Uranians\n-YRT 43 51   ");
  for (i = 43; i <= 51; i++) PrintF(SzNumF(ignore2[i]));
  PrintF("       ; Dwarfs\n\n");

  sprintf(sz, "-YR0 %s%s ; Restrict sign changes, direction changes\n",
    SzNumF(us.fIgnoreSign), SzNumF(us.fIgnoreDir)); PrintFSz();
  sprintf(sz, "-YR1 %s%s ; "
    "Restrict latitude direction changes, distance direction changes\n\n",
    SzNumF(us.fIgnoreDiralt), SzNumF(us.fIgnoreDirlen)); PrintFSz();
  PrintF("-YR7 ");
  for (i = 0; i < rrMax; i++) PrintF(SzNumF(ignore7[i]));
  PrintF(" ; Restrict rulerships: std, esoteric, hierarch, exalt, ray\n-YRZ ");
  for (i = 0; i < arMax; i++) PrintF(SzNumF(ignorez[i]));
  PrintF("   ; Restrict angle events: rising, zenith, setting, nadir\n\n\n");

  PrintF("; DEFAULT ASPECT ORBS:\n"
    ";  1- 5: Con Opp Squ Tri Sex\n"
    ";  6-11: Inc SSx SSq Ses Qui BQn\n"
    "; 12-18: SQn Sep Nov BNv BSp TSp QNv\n"
    "; 19-24: TDc Un1 Un2 Un3 Un4 Un5\n\n-YAo 1 5    ");
  for (i = 1; i <= 5; i++) { sprintf(sz, " %.0f", rAspOrb[i]); PrintFSz(); }
  PrintF("      ; Major aspects\n-YAo 6 11   ");
  for (i = 6; i <= 11; i++) { sprintf(sz, " %.0f", rAspOrb[i]); PrintFSz(); }
  PrintF("    ; Minor aspects\n-YAo 12 18  ");
  for (i = 12; i <= 18; i++) { sprintf(sz, " %.0f", rAspOrb[i]); PrintFSz(); }
  PrintF("  ; Obscure aspects\n-YAo 19 24  ");
  for (i = 19; i <= 24; i++) { sprintf(sz, " %.1f", rAspOrb[i]); PrintFSz(); }
  PrintF("  ; Very obscure aspects\n\n");

  PrintF("; DEFAULT MAX PLANET ASPECT ORBS:\n\n-YAm 0 10   ");
  for (i = 0; i <= 10; i++) { sprintf(sz, "%4.0f", rObjOrb[i]); PrintFSz(); }
  PrintF("      ; Planets\n-YAm 11 21  ");
  for (i = 11; i <= 21; i++) { sprintf(sz, "%4.0f", rObjOrb[i]); PrintFSz(); }
  PrintF("      ; Minor planets\n-YAm 22 33  ");
  for (i = 22; i <= 33; i++) { sprintf(sz, "%4.0f", rObjOrb[i]); PrintFSz(); }
  PrintF("  ; Cusp objects\n-YAm 34 42  ");
  for (i = 34; i <= 42; i++) { sprintf(sz, "%4.0f", rObjOrb[i]); PrintFSz(); }
  PrintF("              ; Uranians\n-YAm 43 51  ");
  for (i = 43; i <= 51; i++) { sprintf(sz, "%4.0f", rObjOrb[i]); PrintFSz(); }
  PrintF("              ; Dwarfs\n-YAm 84 84  ");
  sprintf(sz, "%4.0f", rObjOrb[52]); PrintFSz();
  PrintF("                                              ; Fixed stars\n");

  PrintF("\n; DEFAULT PLANET ASPECT ORB ADDITIONS:\n\n-YAd 0 10   ");
  for (i = 0; i <= 10; i++) { sprintf(sz, " %.0f", rObjAdd[i]); PrintFSz(); }
  PrintF("    ; Planets\n-YAd 11 21  ");
  for (i = 11; i <= 21; i++) { sprintf(sz, " %.0f", rObjAdd[i]); PrintFSz(); }
  PrintF("    ; Minor planets\n-YAd 22 33  ");
  for (i = 22; i <= 33; i++) { sprintf(sz, " %.0f", rObjAdd[i]); PrintFSz(); }
  PrintF("  ; Cusp objects\n-YAd 34 42  ");
  for (i = 34; i <= 42; i++) { sprintf(sz, " %.0f", rObjAdd[i]); PrintFSz(); }
  PrintF("        ; Uranians\n-YAd 43 51  ");
  for (i = 43; i <= 51; i++) { sprintf(sz, " %.0f", rObjAdd[i]); PrintFSz(); }
  PrintF("        ; Dwarfs\n-YAd 84 84  ");
  sprintf(sz, " %.0f", rObjAdd[52]); PrintFSz();
  PrintF("                        ; Fixed stars\n\n\n");

  PrintF("; DEFAULT INFLUENCES:\n\n-Yj 0 10   ");
  for (i = 0; i <= 10; i++)
    { sprintf(sz, " %2.0f", rObjInf[i]); PrintFSz(); }
  PrintF("     ; Planets\n-Yj 11 21  ");
  for (i = 11; i <= 21; i++)
    { sprintf(sz, " %2.0f", rObjInf[i]); PrintFSz(); }
  PrintF("     ; Minor planets\n-Yj 22 33  ");
  for (i = 22; i <= 33; i++)
    { sprintf(sz, " %2.0f", rObjInf[i]); PrintFSz(); }
  PrintF("  ; Cusp objects\n-Yj 34 42  ");
  for (i = 34; i <= 42; i++)
    { sprintf(sz, " %2.0f", rObjInf[i]); PrintFSz(); }
  PrintF("           ; Uranians\n-Yj 43 51  ");
  for (i = 43; i <= 51; i++)
    { sprintf(sz, " %2.0f", rObjInf[i]); PrintFSz(); }
  PrintF("           ; Dwarfs\n-Yj 84 84   ");
  sprintf(sz, "%2.0f", rObjInf[84]); PrintFSz();
  PrintF("                                   ; Fixed stars\n\n");

  PrintF("-YjC 1 12  ");
  for (i = 1; i <= cSign; i++)
    { sprintf(sz, " %.0f", rHouseInf[i]); PrintFSz(); }
  PrintF("  ; Houses\n\n-YjA 1 5   ");

  for (i = 1; i <= 5; i++) { sprintf(sz, "%4.1f", rAspInf[i]); PrintFSz(); }
  PrintF("          ; Major aspects\n-YjA 6 11  ");
  for (i = 6; i <= 11; i++) { sprintf(sz, "%4.1f", rAspInf[i]); PrintFSz(); }
  PrintF("      ; Minor aspects\n-YjA 12 18 ");
  for (i = 12; i <= 18; i++) { sprintf(sz, "%4.1f", rAspInf[i]); PrintFSz(); }
  PrintF("  ; Obscure aspects\n\n");

  PrintF("; DEFAULT TRANSIT INFLUENCES:\n\n-YjT 0 10  ");
  for (i = 0; i <= 10; i++)
    { sprintf(sz, " %2.0f", rTransitInf[i]); PrintFSz(); }
  PrintF("  ; Planets\n-YjT 11 21 ");
  for (i = 11; i <= 21; i++)
    { sprintf(sz, " %2.0f", rTransitInf[i]); PrintFSz(); }
  PrintF("  ; Minor planets\n-YjT 34 42 ");
  for (i = 34; i <= 42; i++)
    { sprintf(sz, " %2.0f", rTransitInf[i]); PrintFSz(); }
  PrintF("        ; Uranians\n-YjT 43 51 ");
  for (i = 43; i <= 51; i++)
    { sprintf(sz, " %2.0f", rTransitInf[i]); PrintFSz(); }
  PrintF("        ; Dwarfs\n-YjT 84 84  ");
  sprintf(sz, "%2.0f", rTransitInf[84]); PrintFSz();
  PrintF("                                ; Fixed stars\n\n");

  sprintf(sz, "-Yj0 %.0f %.0f %.0f %.0f ",
    rObjInf[oNorm1 + 1], rObjInf[oNorm1 + 2], rHouseInf[cSign + 1],
    rHouseInf[cSign + 2]); PrintFSz();
  PrintF(" ; In ruling sign, exalted sign, ruling house, exalted house\n");
  sprintf(sz, "-Yj7 %.0f %.0f %.0f %.0f %.0f %.0f ", rObjInf[oNorm1 + 3],
    rObjInf[oNorm1 + 4], rObjInf[oNorm1 + 5], rHouseInf[cSign + 3],
    rHouseInf[cSign + 4], rHouseInf[cSign + 5]); PrintFSz();
  PrintF(" ; In Esoteric, Hierarchical, Ray ruling (signs, houses)\n\n\n");

  PrintF("; DEFAULT RAYS:\n\n-Y7C 1 12  ");
  for (i = 1; i <= cSign; i++)
    { sprintf(sz, " %d", rgSignRay[i]); PrintFSz(); }
  PrintF("  ; Signs\n-Y7O 0 10  ");
  for (i = 0; i <= 10; i++)
    { sprintf(sz, " %d", rgObjRay[i]); PrintFSz(); }
  PrintF("             ; Planets\n-Y7O 34 42 ");
  for (i = 34; i <= 42; i++)
    { sprintf(sz, " %d", rgObjRay[i]); PrintFSz(); }
  PrintF("                 ; Uranians\n-Y7O 43 51 ");
  for (i = 43; i <= 51; i++)
    { sprintf(sz, " %d", rgObjRay[i]); PrintFSz(); }
  PrintF("                 ; Dwarfs\n\n\n");

  PrintF("; DEFAULT COLORS:\n; Black, White, Gray, LtGray, "
    "Red, Orange, Yellow, Green, Cyan, Blue, Purple,\n"
    "; Magenta, Maroon, DkGreen, DkCyan, DkBlue; "
    "Element, Ray, Star, Planet\n"
    "\n-YkO 0 10  ");
  for (i = 0; i <= 10; i++)
    { sprintf(sz, " %.3s", szColor[kObjU[i]]); PrintFSz(); }
  PrintF("      ; Planet colors\n-YkO 11 21 ");
  for (i = 11; i <= 21; i++)
    { sprintf(sz, " %.3s", szColor[kObjU[i]]); PrintFSz(); }
  PrintF("      ; Minor colors\n-YkO 22 33 ");
  for (i = 22; i <= 33; i++)
    { sprintf(sz, " %.3s", szColor[kObjU[i]]); PrintFSz(); }
  PrintF("  ; Cusp colors\n-YkO 34 42 ");
  for (i = 34; i <= 42; i++)
    { sprintf(sz, " %.3s", szColor[kObjU[i]]); PrintFSz(); }
  PrintF("              ; Uranian colors\n-YkO 43 51 ");
  for (i = 43; i <= 51; i++)
    { sprintf(sz, " %.3s", szColor[kObjU[i]]); PrintFSz(); }
  PrintF("              ; Dwarf colors\n-YkO 52 63 ");
  for (i = 52; i <= 63; i++)
    { sprintf(sz, " %.3s", szColor[kObjU[i]]); PrintFSz(); }
  PrintF("  ; Moons\n-YkO 64 75 ");
  for (i = 64; i <= 75; i++)
    { sprintf(sz, " %.3s", szColor[kObjU[i]]); PrintFSz(); }
  PrintF("  ; Moons\n-YkO 76 83 ");
  for (i = 76; i <= 83; i++)
    { sprintf(sz, " %.3s", szColor[kObjU[i]]); PrintFSz(); }
  PrintF("                  ; Moons\n-YkO 84 84 ");
  sprintf(sz, " %.3s", szColor[kObjU[84]]); PrintFSz();
  for (i = 0; i < 46; i++) PrintF(" ");
  PrintF("; Fixed stars\n\n-YkA 1 5   ");

  for (i = 1; i <= 5; i++)
    { sprintf(sz, " %.3s", szColor[kAspA[i]]); PrintFSz(); }
  PrintF("          ; Major aspect colors\n-YkA 6 11  ");
  for (i = 6; i <= 11; i++)
    { sprintf(sz, " %.3s", szColor[kAspA[i]]); PrintFSz(); }
  PrintF("      ; Minor aspect colors\n-YkA 12 18 ");
  for (i = 12; i <= 18; i++)
    { sprintf(sz, " %.3s", szColor[kAspA[i]]); PrintFSz(); }
  PrintF("  ; Obscure aspect colors\n\n-YkC       ");

  for (i = eFir; i <= eWat; i++)
    { sprintf(sz, " %.3s", szColor[kElemA[i]]); PrintFSz(); }
  PrintF("                      ; Element colors\n-Yk7 1 7   ");
  for (i = 1; i <= cRay; i++)
    { sprintf(sz, " %.3s", szColor[kRayA[i]]); PrintFSz(); }
  PrintF("          ; Ray colors\n-Yk0 1 7   ");
  for (i = 1; i <= cRainbow; i++)
    { sprintf(sz, " %.3s", szColor[kRainbowA[i]]); PrintFSz(); }
  PrintF("          ; Rainbow colors\n-Yk  0 8   ");
  for (i = 0; i <= 8; i++)
    { sprintf(sz, " %.3s", szColor[kMainA[i]]); PrintFSz(); }
  PrintF("  ; Main colors\n\n\n");

#ifdef SWISS
  PrintF("; OBJECT CUSTOMIZATION:\n\n");
  for (i = custLo; i <= custHi; i++) {
    j = i - custLo;
    f1 = (rgObjSwiss[j] != rgObjSwissDef[j] || rgTypSwiss[j] !=
      rgTypSwissDef[j] || rgPntSwiss[j] != 0 || rgFlgSwiss[j] != 0);
    f2 = (szObjDisp[i] != szObjName[i]);
    if (!f1 && !f2)
      continue;
    fAny = fTrue;
    if (f1) {
      sprintf(sz, "-Ye%s%s%s%s%s%s%s%s %d %d%s",
        szTypSwiss[rgTypSwiss[j]], szPntSwiss[rgPntSwiss[j]],
        (rgFlgSwiss[j] & 1) ? "H" : "", (rgFlgSwiss[j] & 2) ? "S" : "",
        (rgFlgSwiss[j] & 4) ? "B" : "", (rgFlgSwiss[j] & 8) ? "N" : "",
        (rgFlgSwiss[j] & 16) ? "T" : "", (rgFlgSwiss[j] & 32) ? "V" : "",
        i, rgObjSwiss[j], f2 ? " " : "\n"); PrintFSz();
    }
    if (f2) {
      sprintf(sz, "-YD %d %s\n", i, szObjDisp[i]); PrintFSz();
    }
  }
  if (!fAny)
    PrintF("; [No objects are different from defaults]\n");
  PrintF("\n\n");
#endif

#ifdef GRAPH
  PrintF("; GRAPHICS DEFAULTS:\n\n");
  sprintf(sz, "%cXm              ", ChDashF(gs.fColor)); PrintFSz();
  PrintF("; Color charts       [\"=Xm\" is color, \"_Xm\" is monochrome]\n");
  sprintf(sz, "%cXr              ", ChDashF(gs.fInverse)); PrintFSz();
  PrintF("; Reverse background [\"_Xr\" is black, \"=Xr\" is white     ]\n");
  i = gs.xWin; if (fSidebar) i -= (SIDESIZE * gi.nScaleText) >> 1;
  sprintf(sz, ":Xw %d %d      ", i, gs.yWin); PrintFSz();
  PrintF("; Default X and Y resolution (not including sidebar)\n");
  sprintf(sz, ":Xs %d          ", gs.nScale); PrintFSz();
  PrintF("; Character scale     [100-400]\n");
  sprintf(sz, ":XS %d          ", gs.nScaleText); PrintFSz();
  PrintF("; Graphics text scale [100-400]\n");
  sprintf(sz, "%cXQ              ", ChDashF(gs.fKeepSquare)); PrintFSz();
  PrintF(
    "; Square charts [\"=XQ\" forces square, \"_XQ\" allows rectangle]\n");
  sprintf(sz, "%cXu              ", ChDashF(gs.fBorder)); PrintFSz();
  PrintF(
    "; Chart border  [\"=Xu\" shows border, \"_Xu\" doesn't show     ]\n");
  sprintf(sz, ":Xv %d            ", gs.nDecaFill); PrintFSz();
  PrintF(
    "; Wheel fill    [\"0\" for none, \"1\" for standard, \"2\" rainbow]\n");
  sprintf(sz, "%cXx              ", ChDashF(gs.fThick)); PrintFSz();
  PrintF(
    "; Thicker lines [\"=Xx\" is thicker, \"_Xx\" is thinner         ]\n");
  sprintf(sz, ":Xb%c             ", gi.fBmp ? 'w' : ChUncap(gs.chBmpMode));
  PrintFSz();
  PrintF(
    "; Bitmap file type   [\"Xbw\" is Windows .bmp, \"Xbn\" is X11   ]\n");
  sprintf(sz, ":YXG %05d       ", gs.nGlyphs); PrintFSz();
  PrintF("; Glyph selections [Capricorn, Uranus, Pluto, Lilith, Vertex]\n");
  sprintf(sz, ":YXg %d           ", gs.nGridCell); PrintFSz();
  PrintF("; Aspect grid cells  [\"0\" for autodetect  ]\n");
  sprintf(sz, ":YXS %.1f         ", gs.rspace); PrintFSz();
  PrintF("; Orbit radius in AU [\"0.0\" for autodetect]\n");
  sprintf(sz, ":YXj %d           ", gs.cspace); PrintFSz();
  PrintF("; Orbit trail count\n");
  sprintf(sz, ":YX7 %d         ", gs.nRayWidth); PrintFSz();
  PrintF("; Esoteric ray column influence width\n");
  sprintf(sz, ":YXf %05d       ", gs.nFont); PrintFSz();
  PrintF("; System fonts to use [text, signs, houses, planets, aspects]\n");
  sprintf(sz, ":YXp %d           ", gs.nOrient); PrintFSz();
  PrintF(
    "; PostScript paper orientation [\"-1\" portrait, \"1\" landscape]\n");
  sprintf(sz, ":YXp0 %s ", SzLength(gs.xInch)); PrintFSz();
  sprintf(sz, "%s ", SzLength(gs.yInch)); PrintFSz();
  PrintF("; PostScript paper X and Y sizes\n\n");
  sprintf(sz, "%cX               ", ChDashF(us.fGraphics)); PrintFSz();
  PrintF("; Graphics chart display [\"_X\" is text, \"=X\" is graphics]\n");
#endif

  sprintf(sz, "\n; %s\n", DEFAULT_INFOFILE); PrintFSz();
  fclose(file);
  return fTrue;
}


// Open all the Astrolog chart info files within the specified directory, and
// append their chart information to the chart list.

void OpenDir(CONST char *szDir)
{
#ifdef PC
  char szPath[cchSzMax], szFile[cchSzMax];
  struct _finddata_t fi;
  long hFile;
  flag fAll = us.fWriteOld, fHaveFile;

  // Only look at *.as (or *.dat) extension files.
  sprintf(szPath, "%s\\*.%s", szDir, !fAll ? "as" : "*");
  hFile = _findfirst(szPath, &fi);
  fHaveFile = (hFile != -1);
  while (fHaveFile) {
    if (fi.name[0] == '.')
      goto LNext;
    // Skip over astrolog.as, atlas.as, and timezone.as if present.
    if (!fAll && (FEqSzI(fi.name, DEFAULT_INFOFILE) || FEqSzI(fi.name,
      DEFAULT_ATLASFILE) || FEqSzI(fi.name, DEFAULT_TIMECHANGE)))
      goto LNext;
    sprintf(szFile, "%s\\%s", szDir, fi.name);
    if (!FInputData(szFile))
      break;
    if (!FAppendCIList(&ciCore))
      break;
LNext:
    fHaveFile = _findnext(hFile, &fi) == 0;
  }
  _findclose(hFile);
#else
  char szPath[cchSzMax], szFile[cchSzMax], *pchFile, *pch;
  DIR *dir;
  struct dirent *ent;
  flag fAll = us.fWriteOld;

  dir = opendir(szDir);
  if (dir != NULL) {
    loop {
      ent = readdir(dir);
      if (ent == NULL)
        break;
      pchFile = ent->d_name;
      if (pchFile[0] == '.')
        continue;
      for (pch = pchFile; *pch; pch++)
        ;
      if (!fAll && !(pch - pchFile > 3 &&
        pch[-3] == '.' && pch[-2] == 'a' && pch[-1] == 's'))
        continue;
      // Skip over astrolog.as, atlas.as, and timezone.as if present.
      if (!fAll && (FEqSzI(pchFile, DEFAULT_INFOFILE) || FEqSzI(pchFile,
        DEFAULT_ATLASFILE) || FEqSzI(pchFile, DEFAULT_TIMECHANGE)))
        continue;
      sprintf(szFile, "%s/%s", szDir, pchFile);
      if (!FInputData(szFile))
        break;
      if (!FAppendCIList(&ciCore))
        break;
    }
    closedir(dir);
  } else {
    sprintf(szFile, "Directory '%s' not found.", szDir);
    PrintError(szFile);
  }
#endif
}


/*
******************************************************************************
** User Input Routines.
******************************************************************************
*/

// Convert a string to an integer.

int NFromSz(CONST char *sz)
{
#ifdef EXPRESS
  // If string starts with "~" then parse the rest as an AstroExpression.
  if (sz[0] == '~')
    return NParseExpression(sz+1);
#endif
  return atoi(sz);
}


// Convert a string to a real number.

real RFromSz(CONST char *sz)
{
#ifdef EXPRESS
  // If string starts with "~" then parse the rest as an AstroExpression.
  if (sz[0] == '~')
    return RParseExpression(sz+1);
#endif
  return atof(sz);
}


// Given a string, return an index number corresponding to what the string
// indicates, based on a given parsing mode. In most cases this is mainly
// looking up a string in the appropriate array and returning the index.

int NParseSz(CONST char *szEntry, int pm)
{
  char szLocal[cchSzMax], ch0, ch1, ch2, ch3;
  CONST char *sz;
  int cch, n, i;

  // First strip off any leading or trailing spaces.
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

  if (cch >= 3 - (pm == pmObject)) {
    ch0 = ChCap(sz[0]); ch1 = ChUncap(sz[1]); ch2 = ChUncap(sz[2]);
    switch (pm) {
    // Parse months, e.g. "February" or "Feb" -> 2 for February.
    case pmMon:
      for (i = 1; i <= cSign; i++)
        if (FMatchSz(sz, szMonth[i]))
          return i;
      break;
    // Parse planets, e.g. "Jupiter" or "Jup" -> 6 for Jupiter.
    case pmObject:
      ch3 = ChUncap(sz[3]);
      for (i = 0; i <= cObj; i++) {
        if (FMatchSz(sz, szObjName[i]))
          return i;
        // Old style -o0 position files have "XXX:" format for planets.
        if (ch0 == szObjName[i][0] && ch1 == szObjName[i][1] &&
          ch2 == szObjName[i][2] && ch3 == ':')
          return i;
      }
      // Allow "Node" to match North Node for compatibility with version 5.40
      // and before, for when the object was called just "Node".
      // Allow "Star" to match first star for -YAm and -YAd switches.
      i = SzLookup(rgObjName, szEntry);
      if (i >= 0)
        return i;
      break;
    // Parse aspects, e.g. "Conjunct" or "Con" -> 1 for the Conjunction.
    case pmAspect:
      for (i = 1; i <= cAspect2; i++)
        if (FMatchSz(sz, szAspectAbbrev[i]))
          return i;
      i = SzLookup(rgAspectName, szEntry);
      if (i >= 0)
        return i;
      break;
    // Parse house systems, e.g. "Koch" or "Koc" -> 1 for Koch houses.
    case pmSystem:
      for (i = 0; i < cSystem; i++)
        if (FMatchSz(sz, szSystem[i]))
          return i;
      i = SzLookup(rgSystem, szEntry);
      if (i >= 0)
        return i;
      break;
    // Parse zodiac signs, e.g. "Scorpio" or "Sco" -> 8 for Scorpio.
    case pmSign:
      for (i = 1; i <= cSign; i++)
        if (FMatchSz(sz, szSignName[i]))
          return i;
      break;
    // Parse color indexes, e.g. "White" or "Whi" -> 15 for White.
    case pmColor:
      if (FMatchSz("Ray", sz)) {
        i = atoi(sz + 3);
        if (FBetween(i, 1, cRay))
          return kRayA[i];
      }
      for (i = 0; i < cColor+4; i++)
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
    // Parse RGB color values, e.g. "255,0,0" or "#ff0000" for Red.
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
      i = Rgb(RgbB(i), RgbG(i), RgbR(i));
      return i;
    // Parse day of week, e.g. "Monday" or "Mon" -> 1 for Monday.
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
    // For years, process any "BCE" (or "BC.", "b.c.e", and variations) and
    // convert an example such as "5BC" to -4. For negative years, note the
    // difference of 1, as 1CE/AD was preceeded by 1BCE/BC, with no year 0.
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


// Given a string, return a floating point number corresponding to what the
// string indicates, based on a given parsing mode, like above for integers.

real RParseSz(CONST char *szEntry, int pm)
{
  char szLocal[cchSzMax], *sz, *pch, ch, chT;
  int cch, i, cColon = 0;
  flag fNeg = fFalse, fMeridian = fFalse;
  real r;

  // First strip off any leading or trailing spaces.
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
  // Capitalize all letters.
  for (pch = sz; *pch; pch++)
    *pch = ChCap(*pch);
  ch = sz[0];

  if (pm == pmTim) {
    // For times, process "Noon" and "Midnight" (or just "N" and "M").
    if (ch == 'N')
      return 12.0;
    else if (ch == 'M')
      return 0.0;
  } else if (pm == pmDst) {
    // For the Daylight time flag, "Daylight", "Yes", and "True" (or just
    // their first characters) are all indications to be ahead one hour.
    if (ch == 'D' || ch == 'Y' || ch == 'T')
      return 1.0;
    // "Standard", "No", and "False" mean the normal zero offset.
    else if (ch == 'S' || ch == 'N' || ch == 'F')
      return 0.0;
    // Check for "Autodetect" Daylight time.
    else if (ch == 'A')
      return dstAuto;
  } else if (pm == pmZon) {
    // For time zones, see if the abbrev is in a table, e.g. "EST" -> 5.
    for (i = 0; i < cZone; i++)
      if (FEqSz(sz, szZon[i]))
        return rZon[i];
    // Allow "H" prefix for numeric zones, e.g. "H5W".
    if (ch == 'H' || ch == 'M') {
      fMeridian = (ch == 'M');
      sz++, cch--;
      ch = sz[0];
    }
    // Negate the value for an "E" in the middle somewhere (e.g. "5E30").
    for (i = 0; i < cch; i++) {
      chT = sz[i];
      if (FCapCh(chT)) {
        if (chT == 'H') {  // So zones like "5HW30" work.
          sz[i] = ' ';
          continue;
        }
        if (chT == 'E')
          fNeg = fTrue;
        sz[i] = ':';
        break;
      }
    }
  } else if (pm == pmLon || pm == pmLat) {
    // For locations, negate the value for an "E" or "S" in the middle
    // somewhere (e.g. "105E30" or "27:40S") for eastern/southern values.
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

  // Anything still at this point should be in a numeric format.
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
    // Check for Astrodienst style MMSS with no separator between them.
    if (FNumCh(*pch) && FNumCh(pch[1]) && FNumCh(pch[2]) && FNumCh(pch[3]) &&
      !FNumCh(pch[4]) && pch[4] != ':') {
      i = atoi(pch);
      r += (real)(i / 100) / 60.0 + (real)(i % 100) / 3600.0;
    } else
      r += atof(pch) / 60.0;
    while (*pch && *pch != ':' && *pch != '\'')
      pch++;
    if (*pch == ':' || *pch == '\'')
      r += atof(pch+1) / (60.0*60.0);
  }
  if (fNeg)
    neg(r);
  if (fMeridian)
    r /= 15.0;

  if (pm == pmTim) {
    // Backtrack over any time suffix, e.g. "AM", "p.m." and variations.
    i = Max(cch-1, 0);
    if (i && sz[i] == '.')
      i--;
    if (i && sz[i] == 'M')
      i--;
    if (i && sz[i] == '.')
      i--;
    if (i) {
      chT = sz[i];
      // Adjust value appropriately if AM or PM suffix.
      if (chT == 'A')
        r = r >= 12.0 ? r-12.0 : r;
      else if (chT == 'P')
        r = r >= 12.0 ? r : r+12.0;
    }
  } else if (pm == pmZon) {
    // Check for "E" suffix for East of GMT.
    i = Max(cch-1, 0);
    if (sz[i] == 'E' || (cch >= 9 && sz[cch-4] == ' ' && sz[cch-5] == 'E'))
      neg(r);
  } else if (pm == pmDist) {
    // Check for "mile" or "mi" suffix for miles instead of kilometers.
    i = Max(cch-1, 0);
    if (sz[i] == 'I' || sz[i] == 'E')
      r *= rMiToKm;
  } else if (pm == pmElv) {
    // Check for "feet" or "ft" suffix for feet instead of meters.
    i = Max(cch-1, 0);
    if (sz[i] == 'F' || sz[i] == 'T')
      r *= rFtToM;
  } else if (pm == pmLength) {
    // Check for "cent" or "cm" suffix for centimeters instead of inches.
    i = Max(cch-1, 0);
    if (sz[i] == 'C' || sz[i] == 'M')
      r /= rInToCm;
  }
  return r;
}


#ifndef WIN
// Stop and wait for the user to enter a line of text given a prompt to
// display and a string buffer to fill with it.

void InputString(CONST char *szPrompt, char *sz)
{
  FILE *file;
  int cch;

  file = is.S; is.S = stdout;
  PrintSz(szPrompt);
  AnsiColor(kYellowA);
  PrintSz(" > ");
  AnsiColor(kDefault);
  if (fgets(sz, cchSzMax, stdin) == NULL)    // Pressing Control+d terminates
    Terminate(tcForce);                      // the program on some systems.
  cch = CchSz(sz);
  while (cch > 0 && sz[cch-1] < ' ')
    cch--;
  sz[cch] = chNull;
  is.S = file;
  is.cchCol = 0;
}


// Prompt the user for a floating point value, parsing as appropriate, and
// make sure it conforms to the specified bounds before returning it.

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


// This is identical to above except it takes/returns floating point values.

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
#endif // WIN


// This important procedure gets all the parameters defining the chart that
// will be worked with later. Given a "filename", it gets from it all the
// pertinent chart information. This is more than just reading from a file.
// The procedure also takes care of the cases of prompting the user for the
// information and using the time functions to determine the date now. (The
// program considers these cases "virtual" files.) Finally, when reading from
// a real file, have to check if it was written in the -o0 or other formats.

flag FInputData(CONST char *szFile)
{
  FILE *file;
  char sz[cchSzMax], ch;
  int i, fT;
  real k, l, m;

  // If we are to read from the virtual file "nul" that means to leave the
  // chart information alone with whatever settings it may have already.

  if (FEqSz(szFile, szNulCore)) {
    is.fHaveInfo = fTrue;
    return fTrue;
  }

  // If we are to read from the virtual file "set" then that means use a
  // particular set of chart information generated earlier in the program.

  if (FEqSz(szFile, szSetCore)) {
    is.fHaveInfo = fTrue;
    ciCore = ciSave;
    return fTrue;
  }
  if (FEqSz(szFile, "__t")) {
    is.fHaveInfo = fTrue;
    ciCore = ciTran;
    return fTrue;
  }
  if (FEqSz(szFile, "__g")) {
    is.fHaveInfo = fTrue;
    ciCore = ciGreg;
    return fTrue;
  }

  // If we are to read from the virtual file "__1" through "__6" then that
  // means copy the chart information from the specified chart slot.

  if (szFile[0] == '_' && szFile[1] == '_') {
    ch = szFile[2];
    if (FBetween(ch, '1', '6') && szFile[3] == chNull) {
      is.fHaveInfo = fTrue;
      ciCore = *rgpci[ch - '0'];
      return fTrue;
    }
  }

#ifdef TIME
  // If we are to read from the file "now" then that means use the time
  // functions to calculate the present date and time.

  if (FEqSz(szFile, szNowCore)) {
    is.fHaveInfo = fTrue;
    SS = us.dstDef; ZZ = us.zonDef; OO = us.lonDef; AA = us.latDef;
    GetTimeNow(&MM, &DD, &YY, &TT, SS, ZZ);
    ciCore.nam = us.namDef; ciCore.loc = us.locDef;
    return fTrue;
  }
#endif

#ifndef WIN
  // If we are to read from the file "tty" then that means prompt the user
  // for all the chart information.

  if (FEqSz(szFile, szTtyCore)) {
    file = is.S; is.S = stdout;
    if (!us.fNoSwitches) {
      // Temporarily disable an internal redirection of output to a file
      // because we always want user headers and prompts to be displayed.

      AnsiColor(kWhiteA);
      sprintf(sz, "** %s version %s ", szAppName, szVersionCore); PrintSz(sz);
      sprintf(sz, "(See '%cHc' switch for copyrights and credits.) **\n",
        chSwitch); PrintSz(sz);
      AnsiColor(kDefault);
      sprintf(sz, "   Invoke as '%s %cH' for list of command line options.\n",
        SzProcessProgname(is.szProgName), chSwitch); PrintSz(sz);
    }

    MM = NInputRange("Enter month for chart (e.g. '3' 'Mar')",
      1, 12, pmMon);
    DD = NInputRange("Enter day   for chart (e.g. '1' '31') ",
      1, DayInMonth(MM, 0), pmDay);
    YY = NInputRange("Enter year  for chart (e.g. '2022')   ",
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
#endif // WIN

  // Now that the special cases are taken care of, can assume are to read from
  // a real file.

  file = FileOpen(szFile, 1, NULL);
  if (file == NULL)
    return fFalse;
  is.fHaveInfo = fTrue;
  if (!fgets(sz, cchSzMax, file))
    return fFalse;
  ch = sz[0];
  for (i = CchSz(sz); i > 0 && sz[i-1] < ' '; i--)
    ;
  if (i >= 0)
    sz[i] = chNull;
  fseek(file, 0, SEEK_SET);

  // Read the chart parameters from a standard command switch file.

  if (ch == '@') {
    fT = is.fSzPersist; is.fSzPersist = fFalse;
    if (!FProcessSwitchFile(szFile, file))
      return fFalse;
    is.fSzPersist = fT;

  // Read the chart parameters from an Astrological Exchange (AAF) file.

  } else if (ch == '#') {
    fT = is.fSzPersist; is.fSzPersist = fFalse;
    if (!FProcessAAFFile(szFile, file))
      return fFalse;
    is.fSzPersist = fT;

  // Read the chart parameters from a Quick*Chart file.

  } else if (i == 100 || i == 101) {
    fT = is.fSzPersist; is.fSzPersist = fFalse;
    if (!FProcessQuickFile(szFile, file))
      return fFalse;
    is.fSzPersist = fT;

  // Read the chart parameters from a Astrodatabank file.

  } else if (ch == '<') {
    fT = is.fSzPersist; is.fSzPersist = fFalse;
    if (!FProcessADBFile(szFile, file))
      return fFalse;
    is.fSzPersist = fT;

  // Read the chart parameters from a Solar Fire export text file.

  } else if (ch == '\n') {
    fT = is.fSzPersist; is.fSzPersist = fFalse;
    if (!FProcessSFTextFile(szFile, file))
      return fFalse;
    is.fSzPersist = fT;

  // Read the chart info from an older style -o list of seven numbers.

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

  // Read the actual chart positions from a file produced with -o0 switch.

  } else if (ch == 'S') {
    MM = -1;

    // Hack: A negative month value means the chart parameters are invalid,
    // hence -o0 is in effect and we can assume the chart positions are
    // already in memory so we don't have to calculate them later.

    for (i = 1; i <= oNorm; i++) {
      fscanf(file, "%s%lf%lf%lf", sz, &k, &l, &m);
      planet[i] = Mod((l-1.0)*30.0+k+m/60.0);
      fscanf(file, "%s%lf%lf", sz, &k, &l);
      if ((m = k+l/60.0) > rDegHalf)
        m = rDegMax - m;
      planetalt[i] = m;
      ret[i] = sz[1] == 'D' ? 1.0 : -1.0;

      // -o0 files from versions 3.05 and before don't have the Uranians in
      // them. Be prepared to skip over them in old files for compatibility.

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


#ifdef JPLWEB
/*
******************************************************************************
** Network IO Routines.
******************************************************************************
*/

#include <urlmon.h>  // For URLDownloadToFile()

// Download a Web page from a URL to the specified file.

flag GetURL(const char *szUrl, const char *szFile)
{
  HRESULT hr;
  char sz[cchSzLine];

  hr = URLDownloadToFile(NULL, szUrl, szFile, 0, NULL);
  if (!FAILED(hr))
    return fTrue;
  sprintf(sz, "Failed to download '%s' (error %08x)\n", szUrl, hr);
  PrintWarning(sz);
  return fFalse;
}


// Given a JPL object index, compute its geocentric position and velocity
// vectors. Similar to FSwissPlanet() but does a JPL Horizons Web query
// instead of calling Swiss Ephemeris to compute the position.

flag GetJPLHorizons(int id, real *obj, real *objalt, real *dir, real *space,
  real *diralt, real *dirlen, char *szOut)
{
  char szUrl[cchSzLine], szLine[cchSzLine], szName[cchSzMax],
    szMon[3][4], hr[3], min[3], *pch, *pch2, ch;
  CI ci[3];
  PT3R pt[3];
  FILE *file;
  real sec[3], len[3], rT;
  int phase = -1, i;

  if (us.fNoNetwork) {    // Don't allow if -0n set.
    PrintWarning("Internet features are disabled.");
    return fFalse;
  }

  // Determine time range to get ephemeris for.
  for (i = 0; i < 3; i++) {
    ci[i] = ciCore;
    AddTime(&ci[i], 2, 0);     // Sanitize time if hour out of range
    if (i <= 0)
      AddTime(&ci[i], 2, -5);  // Subtract 5 minutes
    else if (i >= 2)
      AddTime(&ci[i], 2, 6);   // Add at least 5 minutes
    sprintf(szMon[i], "%.3s", szMonth[ci[i].mon]);
    for (pch = szMon[i]; *pch; pch++)
      *pch = ChCap(*pch);
    hr[i] = NFloor(ci[i].tim);
    min[i] = (int)((ci[i].tim - (real)hr[i])*60.0);
    sec[i] = (int)(ci[i].tim * 3600.0 - (real)(hr[i]*3600+min[i]*60));
  }

  // Compose URL to download from internet.
  if (us.fTopoPos) {
    sprintf(szLine, "COORD_TYPE= 'GEODETIC'&"
      "SITE_COORD='%lf,%lf,%lf'&",
      -ciCore.lon, ciCore.lat, us.elvDef / 1000.0);
  } else
    *szLine = chNull;
  sprintf(szUrl, "https://ssd.jpl.nasa.gov/horizons_batch.cgi?batch=1&"
    "COMMAND='%d%s'&"
    "OBJ_DATA='YES'&"
    "MAKE_EPHEM='YES'&"
    "TABLE_TYPE='OBSERVER'&"
    "CENTER='%s'&%s"
    "START_TIME='%d-%s-%02d%%20%d:%02d:%lf'&"
    "STOP_TIME='%d-%s-%02d%%20%d:%02d:%lf'&"
    "STEP_SIZE='5%%20min'&"
    "QUANTITIES= '21,31'&"
    "CSV_FORMAT='YES'",
    id >= nMillion ? id - nMillion : id, id >= nMillion ? ";" : "",
    !us.fTopoPos ? "500" : "coord@399", szLine,
    ci[0].yea, szMon[0], ci[0].day, hr[0], min[0], sec[0],
    ci[2].yea, szMon[2], ci[2].day, hr[2], min[2], sec[2]);
  for (pch = szUrl; *pch; pch++)
    ;
  for (pch--; pch >= szUrl; pch--) {
    ch = *pch;
    if (ch != '\'' && ch != ';')
      continue;
    // Encode ' and ; characters in the URL to %NN format.
    for (pch2 = pch; *pch2; pch2++)
      ;
    for (pch2 += 2; pch2 >= pch; pch2--)
      *pch2 = *(pch2 - 2);
    pch[0] = '%';
    if (ch == '\'') {
      pch[1] = '2'; pch[2] = '7';
    } else {
      pch[1] = '3'; pch[2] = 'B';
    }
  }
  GetURL(szUrl, szFileJPLCore);

  // Process downloaded file.
  file = FileOpen(szFileJPLCore, 1, NULL);
  if (file == NULL) {
    // Error message printed inside FileOpen().
    return fFalse;
  }
  loop {
    while (!feof(file) && (ch = getc(file)) < ' ')
      ;
    if (feof(file))
      break;
    for (szLine[0] = ch, i = 1; i < cchSzLine-1 && !feof(file) &&
      (uchar)(szLine[i] = getc(file)) >= ' '; i++)
      ;
    szLine[i] = chNull;
    if (FBetween(phase, 0, 2)) {
      // Parse ephemeris data for one instant of time.
      for (i = 0, pch = szLine; i < 3 && *pch; pch++)
        if (*pch == ',')
          i++;
      sscanf(pch, "%lf,%lf,%lf", &pt[phase].z, &pt[phase].x, &pt[phase].y);
      phase++;
      if (phase >= 3)
        break;
    } else if (FEqRgch(szLine, "$$SOE", 5, fTrue)) {
      // Search for start of ephemeris data.
      phase = 0;
    } else if (phase < 0 && FEqRgch(szLine, "Target body name: ", 18, fTrue)) {
      // Search for JPL name of body this ephemeris is for.
      i = 0;
      for (pch = szLine+18; *pch &&
        !(pch[0] == ' ' && (pch[1] == ' ' || pch[1] == '(')); pch++)
        szName[i++] = *pch;
      szName[i] = chNull;
    }
  }
  fclose(file);
  _unlink(szFileJPLCore);
  if (phase < 3) {
    PrintWarning("Failed to get positions from " szFileJPLCore);
    return fFalse;
  }

  // Process planet data.
  for (i = 0; i < 3; i++) {
    // Convert speed of light in min to AU.
    len[i] = pt[i].z / (1440.0*rDayInYear) * rLYToAU;
  }
  *obj = pt[1].x;
  *objalt = pt[1].y;
  *dir = (pt[2].x - pt[0].x) * 144.0;
  *space = len[1];
  *diralt = (pt[2].y - pt[0].y) * 144.0;
  *dirlen = (len[2] - len[0]) * 144.0;
  if (us.fTruePos) {
    rT = pt[1].z / 1440.0;
    *obj += *dir * rT;
    *objalt += *diralt * rT;
    *space += *dirlen * rT;
  }
  if (szOut != NULL)
    CopyRgchToSz(szName, CchSz(szName)+1, szOut, cchSzMax);
  return fTrue;
}
#endif // JPLWEB

/* io.cpp */
