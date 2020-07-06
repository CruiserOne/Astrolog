/*
** Astrolog (Version 7.00) File: astrolog.cpp
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
** Program Dispatch Procedures.
******************************************************************************
*/

/* Initialize the Ansi color arrays with the color to print each object in. */

void InitColors(void)
{
  int *rgObjRuler = ruler1, i, k;

  /* Figure out which rulership set to use for "Element" color. */
  if (ignore7[rrStd]) {
    if      (!ignore7[rrExa]) rgObjRuler = exalt;
    else if (!ignore7[rrEso]) rgObjRuler = rgObjEso1;
    else if (!ignore7[rrHie]) rgObjRuler = rgObjHie1;
    else if (!ignore7[rrRay]) rgObjRuler = NULL;
  }

  /* Determine and assign the color of each object. */
  for (i = 0; i <= oNorm; i++) {
    k = kObjU[i];
    if (k == kRay || rgObjRuler == NULL)
      k = kRayA[rgObjRay[i]];
    else if (k == kElement)
      k = kElemA[(rgObjRuler[i]-1) & 3];
    kObjA[i] = k;
  }
  EnsureStarBright();
  for (i = starLo; i <= starHi; i++)
    kObjA[i] = KStarA(rStarBright[i-starLo+1]);
}


/* This is the dispatch procedure for the entire program. After all the   */
/* command switches have been processed, this routine is called to        */
/* actually call the various routines to generate and display the charts. */

void Action(void)
{
  char sz[cchSzDef];
  int iChart = 0, i;
  flag fHTML, fHTMLClip;

  /* If the -os switch is in effect, open a file and set a global to */
  /* internally 'redirect' all screen output to.                     */

  if (is.szFileScreen) {
    is.S = fopen(is.szFileScreen, "w");
    if (is.S == NULL) {
      sprintf(sz, "File %s can not be created.", is.szFileScreen);
      PrintError(sz);
      is.S = stdout;
    }
  } else
    is.S = stdout;
  is.cchRow = 0;

  /* If the -kh switch is in effect, start outputting a new HTML file. */

  fHTML = us.fTextHTML && !us.fGraphics && is.S != stdout;
  if (fHTML) {
    fHTMLClip = is.nHTML < 0;
    is.nHTML = 2;
    if (fHTMLClip)
      PrintSz("Version:0.9\n"
        "StartHTML:00000094\n"
        "EndHTML:00010000\n"
        "StartFragment:00000129\n"
        "EndFragment:00010000\n");
    PrintSz("<html><body>");
    if (fHTMLClip)
      PrintSz("\n<!--StartFragment -->\n");
    PrintSz("<font face=\"Courier\">");
    is.nHTML = 3;
  } else
    is.nHTML = 0;

LNext:
  if (iChart < us.cSequenceLine && is.rgszLine[iChart] != NULL)
    FProcessCommandLine(is.rgszLine[iChart]);
#ifdef WIN
  is.fMult = fFalse;    /* Cleared here because no InitVariables routine. */
#endif
  is.fSzPersist = is.fNoEphFile = fFalse;
  InitColors();
  AnsiColor(kDefault);

  /* First let's adjust the restriction status of the cusps, Uranians, and */
  /* fixed stars based on whether -C, -u, and -U switches are in effect.   */

  if (!us.fCusp)
    for (i = cuspLo; i <= cuspHi; i++)
      ignore[i] = ignore2[i] = fTrue;
  if (!us.fUranian)
    for (i = uranLo; i <= uranHi; i++)
      ignore[i] = ignore2[i] = fTrue;
  if (!us.nStar)
    for (i = starLo; i <= starHi; i++)
      ignore[i] = ignore2[i] = fTrue;

  if (FPrintTables())    /* Print out any generic tables specified.        */
    goto LDone;          /* If nothing else to do, we can exit right away. */
  if (is.fMult) {
    PrintL2();
    is.fMult = fFalse;
  }

  /* If -+ or -- switches in effect, then add the specified delta value to */
  /* the date and use that as a new date before proceeding to make chart.  */

  if (us.dayDelta != 0) {
    is.JD = (real)MdyToJulian(MM, DD+us.dayDelta, YY);
    JulianToMdy(is.JD, &MM, &DD, &YY);
    us.dayDelta = 0;
  }

  /* Here we either do a normal chart or some kind of relationship chart. */

  if (!us.nRel) {
#ifndef WIN
    /* If chart info not in memory yet, then prompt the user for it. */
    if (!is.fHaveInfo && !FInputData(szTtyCore))
      return;
    ciMain = ciCore;
    CastChart(fTrue);
#else
    ciMain = ciCore;
    if (wi.fCast || us.cSequenceLine > 0) {
      wi.fCast = fFalse;
      CastChart(fTrue);
    }
#endif
  } else {
    ciMain = ciCore;
    CastRelation();
  }
#ifndef WIN
  ciSave = ciMain;
#endif

#ifdef GRAPH
  if (us.fGraphics)         /* If any of the X window switches in effect, */
    FActionX();             /* then go make a graphics chart...           */
  else
#endif
  {
#ifdef GRAPH
    if (gs.fInverse) {
      SwapN(kBlackA, kWhiteA);
      SwapN(kLtGrayA, kDkGrayA);
      AnsiColor(kDefault);
    }
#endif
    PrintChart(is.fProgress);    /* Otherwise print chart on text screen. */
#ifdef GRAPH
    if (gs.fInverse) {
      SwapN(kBlackA, kWhiteA);
      SwapN(kLtGrayA, kDkGrayA);
    }
#endif
  }

LDone:
  iChart++;
  if (iChart < us.cSequenceLine && !us.fGraphics) {
    PrintL2();
    goto LNext;
  }

  if (fHTML) {        /* If -kh switch in effect, end the HTML file. */
    is.nHTML = 2;
    PrintSz("</font>\n</font>");
    if (fHTMLClip)
      PrintSz("\n<!--EndFragment-->\n");
    PrintSz("</body></html>\n");
    is.nHTML = 0;
  }

  if (us.fWriteFile)        /* If -o switch in effect, then write */
    FOutputData();          /* the chart information to a file.   */

  if (is.S != stdout) /* If we were internally directing chart display to a */
    fclose(is.S);     /* file as with the -os switch, close it here.        */
}


#ifndef WIN
/* Reset a few variables to their default values they have upon startup of */
/* the program. We don't reset all variables, just the most volatile ones. */
/* This is called when in the -Q loop to reset things like which charts to */
/* display, but leave setups such as object restrictions and orbs alone.   */

void InitVariables(void)
{
  us.fInterpret = us.fProgress = is.fHaveInfo = is.fMult = fFalse;
  us.nRel = rcNone;
  is.szFileScreen = NULL;
  ClearB((pbyte)&us.fListing, (int)((pbyte)&us.fLoop - (pbyte)&us.fListing));
}
#endif


/*
******************************************************************************
** Command Switch Procedures.
******************************************************************************
*/

/* Given a string representing a command line (e.g. a macro string), go    */
/* parse it into its various switches and parameters, then go process them */
/* and change program settings. Basically a wrapper for other functions.   */

flag FProcessCommandLine(char *szLine)
{
  char szCommandLine[cchSzMax], *rgsz[MAXSWITCHES];
  int argc, cb;
  flag fT = fFalse, fSav;
  FILE *fileT;

  if (szLine == NULL || *szLine == chNull)
    return fTrue;
  cb = CchSz(szLine)+1;

  /* Check for filename on command line */
  if (!FChSwitch(szLine[0])) {
    fileT = fopen(szLine, "r");
    if (fileT != NULL) {
      fclose(fileT);
      sprintf(szCommandLine, "-i \"%s\"", szLine);
      fT = fTrue;
    }
  }

  if (!fT)
    CopyRgb((byte *)szLine, (byte *)szCommandLine, cb);
  argc = NParseCommandLine(szCommandLine, rgsz);
  fSav = is.fSzPersist; is.fSzPersist = fFalse;
  fT = FProcessSwitches(argc, rgsz);
  is.fSzPersist = fSav;
  return fT;
}


/* Given string representing a command line, convert it to an "argv" format */
/* of an array of strings, one for each switch or parameter, i.e. exactly   */
/* like the format of the command line as given when the program starts.    */

int NParseCommandLine(char *szLine, char **argv)
{
  int argc = 1, fSpace = fTrue;
  char *pch = szLine, chQuote = chNull;

  /* Split the entered line up into its individual switch strings. */
  while ((uchar)*pch >= ' ' || *pch == chTab) {
    if (*pch == ' ' || *pch == chTab) {
      if (fSpace)
        /* Skip over the current run of spaces between strings. */
        ;
      else {
        /* First space after a string, end parameter here. */
        if (chQuote == chNull) {
          *pch = chNull;
          fSpace = fTrue;
        }
      }
    } else {
      if (fSpace) {
        /* First character after run of spaces, begin parameter here. */
        if (argc >= MAXSWITCHES-1) {
          PrintWarning("Too many parameters! Rest of line ignored.");
          break;
        }
        chQuote = (*pch == '"' || *pch == '\'') ? *pch : chNull;
        argv[argc++] = pch + (chQuote != chNull);
        fSpace = fFalse;
      } else {
        /* Skip over the current string. */
        if (chQuote != chNull && *pch == chQuote) {
          *pch = chNull;
          fSpace = fTrue;
        }
      }
    }
    pch++;
  }
  argv[0] = (char *)szAppNameCore;
  argv[argc] = NULL;         /* Set last string in switch array to Null. */
  return argc;
}


#ifndef WIN
/* This routine is called by the main program to interactively prompt the  */
/* user for command switches and parameters, entered in the same format as */
/* they would be on a command line. This needs to be called with certain   */
/* systems which don't allow passing of a command line to the program,     */
/* or when -Q is in effect. The result of this routine is returned to the  */
/* main program which then processes it as done with a real command line.  */

int NPromptSwitches(char *line, char *argv[MAXSWITCHES])
{
  FILE *data;
  char sz[cchSzDef];

  data = is.S; is.S = stdout;
  is.cchRow = 0;
  AnsiColor(kWhiteA);
  sprintf(sz, "** %s version %s ", szAppName, szVersionCore); PrintSz(sz);
  sprintf(sz, "(See '%cHc' switch for copyrights and credits.) **\n",
    chSwitch); PrintSz(sz);
  AnsiColor(kDefault);
  PrintSz("Enter all parameter options below. ");
  sprintf(sz, "(Enter '%cH' for help. Enter '.' to exit.)\n", chSwitch);
  PrintSz(sz);
  is.S = data;
  InputString("Input command line", line);
  PrintL();
  return NParseCommandLine(line, argv);
}
#endif


/* This subprocedure is like FProcessSwitches() below, except that we only */
/* process one switch, which we know to be one of the obscure -Y types.    */

int NProcessSwitchesRare(int argc, char **argv, int pos,
  flag fOr, flag fAnd, flag fNot)
{
  int darg = 0, i, j, k, l;
  real r;
  char ch1, ch2 = chNull;
  pbyte pb;
  int *lpn;
  real *lpr;
#ifdef MATRIX
  OE oe;
#endif
#ifdef INTERPRET
  char *sz;
#endif

  ch1 = argv[0][pos+1];
  if (ch1 != chNull)
    ch2 = argv[0][pos+2];

  switch (argv[0][pos]) {
  case chNull:
    SwitchF(us.fSwitchRare);
    break;

  case 'T':
    SwitchF(us.fTruePos);
    break;

  case 'V':
    SwitchF(us.fTopoPos);
    break;

  case 'h':
    SwitchF(us.fBarycenter);
    break;

  case 's':
    SwitchF(us.fSidereal2);
    break;

  case 'n':
    SwitchF(us.fTrueNode);
    break;

  case 'd':
    SwitchF(us.fEuroDate);
    break;

  case 't':
    SwitchF(us.fEuroTime);
    break;

  case 'v':
    SwitchF(us.fEuroDist);
    break;

  case 'r':
    SwitchF(us.fRound);
    break;

  case 'C':
    SwitchF(us.fSmartCusp);
    break;

  case 'O':
    SwitchF(us.fSmartSave);
    break;

  case '8':
    SwitchF(us.fClip80);
    break;

  case 'Q':
    if (argc <= 1) {
      ErrorArgc("YQ");
      return tcError;
    }
    i = atoi(argv[1]);
    if (i < 0) {
      ErrorValN("YQ", i);
      return tcError;
    }
    us.nScrollRow = i;
    darg++;
    break;

  case 'q':
    i = (ch1 - '0');
    if (!FBetween(i, 0, 9))
      i = 0;
    if (argc <= i) {
      ErrorArgc("Yq");
      return tcError;
    }
    us.cSequenceLine = i;
    for (i = 0; i < us.cSequenceLine; i++)
      is.rgszLine[i] = SzPersist(argv[i+1]);
    darg += i;
    break;

  case 'i':
    if (argc <= 1) {
      ErrorArgc("Yi");
      return tcError;
    }
    i = (ch1 - '0');
    if (!FBetween(i, 0, 9))
      i = 0;
    us.rgszPath[i] = SzPersist(argv[1]);
    is.fSwissPathSet = fFalse;
    darg++;
    break;

  case 'o':
    SwitchF(us.fWriteOld);
    break;

  case 'c':
    SwitchF(us.fHouseAngle);
    break;

  case 'p':
    SwitchF(us.fPolarAsc);
    break;

  case 'u':
    SwitchF(us.fEclipse);
    break;

  case 'z':
    if (argc <= 1) {
      ErrorArgc("Yz");
      return tcError;
    }
    us.lTimeAddition = atol(argv[1]);
    darg++;
    break;

  case '1':
    if (argc <= 2) {
      ErrorArgc("Y1");
      return tcError;
    }
    i = NParseSz(argv[1], pmObject);
    if (!FItem(i)) {
      ErrorValN("Y1", i);
      return tcError;
    }
    j = NParseSz(argv[2], pmObject);
    if (!FItem(j)) {
      ErrorValN("Y1", j);
      return tcError;
    }
    us.fObjRotWhole = (ch1 == '0' && !fAnd);
    us.objRot1 = i;
    us.objRot2 = j;
    darg += 2;
    break;

  case 'l':
    if (argc <= 1) {
      ErrorArgc("Yl");
      return tcError;
    }
    i = atoi(argv[1]);
    if (!FSector(i)) {
      ErrorValN("Yl", i);
      return tcError;
    }
    SwitchF(pluszone[i]);
    darg++;
    break;

#ifdef ARABIC
  case 'P':
    if (argc <= 1) {
      ErrorArgc("YP");
      return tcError;
    }
    i = atoi(argv[1]);
    if (!FBetween(i, -1, 1)) {
      ErrorValN("YP", i);
      return tcError;
    }
    us.nArabicNight = i;
    darg++;
    break;
#endif

  case 'b':
    if (argc <= 1) {
      ErrorArgc("Yb");
      return tcError;
    }
    i = atoi(argv[1]);
    if (!FValidBioday(i)) {
      ErrorValN("Yb", i);
      return tcError;
    }
    us.nBioday = i;
    darg++;
    break;

#ifdef SWISS
  case 'e':
    if (argc <= 2) {
      ErrorArgc("Ye");
      return tcError;
    }
    i = NParseSz(argv[1], pmObject);
    if (!FUranian(i)) {
      ErrorValN("Ye", i);
      return tcError;
    }
    i -= uranLo;
    j = (ch1 == 'b') + (ch1 == 'O')*2;
    if (j > 0)
      ch1 = ch2;
    rgObjSwiss[i] = j == 2 ? NParseSz(argv[2], pmObject) : atoi(argv[2]);
    rgTypSwiss[i] = j;
    rgPntSwiss[i] =
      (ch1 == 'n') + (ch1 == 's')*2 + (ch1 == 'p')*3 + (ch1 == 'a')*4;
    rgFlgSwiss[i] = 0;
    k = pos + 1;
    do {
      ch2 = argv[0][k++];
      rgFlgSwiss[i] |= (ch2 == 'H') + (ch2 == 'S')*2 + (ch2 == 'B')*4 +
        (ch2 == 'N')*8 + (ch2 == 'T')*16 + (ch2 == 'V')*32;
    } while (ch2);
    darg += 2;
    break;
#endif

#ifdef MATRIX
  case 'E':
    if (argc <= 17) {
      ErrorArgc("YE");
      return tcError;
    }
    i = NParseSz(argv[1], pmObject);
    if (!FHelio(i)) {
      ErrorValN("YE", i);
      return tcError;
    }
    oe.sma = atof(argv[2]);
    oe.ec0 = atof(argv[3]);  oe.ec1 = atof(argv[4]);  oe.ec2 = atof(argv[5]);
    oe.in0 = atof(argv[6]);  oe.in1 = atof(argv[7]);  oe.in2 = atof(argv[8]);
    oe.ap0 = atof(argv[9]);  oe.ap1 = atof(argv[10]); oe.ap2 = atof(argv[11]);
    oe.an0 = atof(argv[12]); oe.an1 = atof(argv[13]); oe.an2 = atof(argv[14]);
    oe.ma0 = atof(argv[15]); oe.ma1 = atof(argv[16]); oe.ma2 = atof(argv[17]);
    rgoe[IoeFromObj(i)] = oe;
    darg += 17;
    break;
#endif

  case 'U':
    if (ch1 == 'b') {
      SwitchF(us.fStarMagDist);
      if (ch2 == '0')
        SwitchF(us.fStarMagAbs);
      break;
    }
    if (argc <= 2) {
      ErrorArgc("YU");
      return tcError;
    }
    i = NParseSz(argv[1], pmObject);
    if (!FStar(i)) {
      ErrorValN("YU", i);
      return tcError;
    }
    szStarCustom[i-oNorm] = SzPersist(argv[2]);
    darg += 2;
    break;

  case 'R':
    if (ch1 == 'd') {
      if (argc <= 1) {
        ErrorArgc("YRd");
        return tcError;
      }
      us.nSignDiv = atoi(argv[1]);
      darg++;
      break;
    } else if (ch1 == 'h') {
      SwitchF(us.fIgnoreAuto);
      break;
    } else if (ch1 == 'U') {
      if (argc <= 1) {
        ErrorArgc("YRU");
        return tcError;
      }
      us.fStarsList = (ch2 == '0');
      us.szStarsList = SzPersist(argv[1]);
      darg++;
      break;
    }
    if (argc <= 2 + (ch1 == 'Z')*2 + (ch1 == '7')*3) {
      ErrorArgc("YR");
      return tcError;
    }
    i = NParseSz(argv[1], pmObject); j = NParseSz(argv[2], pmObject);
    if (ch1 == '0') {
      us.fIgnoreSign = i != 0;
      us.fIgnoreDir  = j != 0;
      darg += 2;
      break;
    } else if (ch1 == 'Z') {
      ignorez[0] = i != 0;
      ignorez[1] = j != 0;
      ignorez[2] = atoi(argv[3]) != 0;
      ignorez[3] = atoi(argv[4]) != 0;
      darg += 4;
      break;
    } else if (ch1 == '7') {
      ignore7[0] = i != 0;
      ignore7[1] = j != 0;
      ignore7[2] = atoi(argv[3]) != 0;
      ignore7[3] = atoi(argv[4]) != 0;
      ignore7[4] = atoi(argv[5]) != 0;
      darg += 5;
      break;
    }
    if (!FItem(i)) {
      ErrorValN("YR", i);
      return tcError;
    }
    if (!FItem(j) || j < i) {
      ErrorValN("YR", j);
      return tcError;
    }
    if (argc <= 3+j-i) {
      ErrorArgc("YR");
      return tcError;
    }
    pb = ch1 == 'T' ? ignore2 : ignore;
    for (k = i; k <= j; k++)
      pb[k] = atoi(argv[3+k-i]) != 0;
    darg += 3+j-i;
    for (j = fFalse, i = cuspLo; i <= cuspHi; i++)
      if (!ignore[i] || !ignore2[i]) {
        j = fTrue;
        break;
      }
    us.fCusp = j;
    for (j = fFalse, i = uranLo; i <= uranHi; i++)
      if (!ignore[i] || !ignore2[i]) {
        j = fTrue;
        break;
      }
    us.fUranian = j;
    for (j = fFalse, i = starLo; i <= starHi; i++)
      if (!ignore[i] || !ignore2[i]) {
        j = fTrue;
        break;
      }
    if (!(us.nStar && j))
      us.nStar = j;
    break;

  case 'A':
    if (ch1 == 'D') {
      if (argc <= 4) {
        ErrorArgc("YAD");
        return tcError;
      }
      i = NParseSz(argv[1], pmAspect);
      if (!FAspect2(i)) {
        ErrorValN("YAD", i);
        return tcError;
      }
      if (CchSz(argv[2]) >= 3)
        szAspectDisp[i] = SzPersist(argv[2]);
      else
        szAspectDisp[i] = szAspectName[i];
      if (CchSz(argv[3]) != 3)
        szAspectAbbrevDisp[i] = SzPersist(argv[3]);
      else
        szAspectAbbrevDisp[i] = szAspectAbbrev[i];
      if (CchSz(argv[4]) > 0)
        szAspectGlyphDisp[i] = SzPersist(argv[4]);
      else
        szAspectGlyphDisp[i] = szAspectGlyph[i];
      darg += 4;
      break;
    }
    if (argc <= 2) {
      ErrorArgc("YA");
      return tcError;
    }
    k = ch1 == 'm' || ch1 == 'd' ? pmObject : pmAspect;
    i = NParseSz(argv[1], k); j = NParseSz(argv[2], k);
    k = ch1 == 'm' || ch1 == 'd' ? oNorm+1 : cAspect;
    if (!FBetween(i, (int)(ch1 == 'o' || ch1 == 'a'), k)) {
      ErrorValN("YA", i);
      return tcError;
    }
    if (!FBetween(j, 0, k) || j < i) {
      ErrorValN("YA", j);
      return tcError;
    }
    if (argc <= 3+j-i) {
      ErrorArgc("YA");
      return tcError;
    }
    lpr = ch1 == 'o' ? rAspOrb : (ch1 == 'm' ? rObjOrb :
      (ch1 == 'd' ? rObjAdd : rAspAngle));
    for (k = i; k <= j; k++)
      lpr[k] = atof(argv[3+k-i]);
    darg += 3+j-i;
    break;

  case 'j':
    if (argc <= 2 + 2*(ch1 == '0') + 4*(ch1 == '7')) {
      ErrorArgc("Yj");
      return tcError;
    }
    if (ch1 == '0') {
      rObjInf[oNorm1+1]  = atof(argv[1]);
      rObjInf[oNorm1+2]  = atof(argv[2]);
      rHouseInf[cSign+1] = atof(argv[3]);
      rHouseInf[cSign+2] = atof(argv[4]);
      darg += 4;
      break;
    } else if (ch1 == '7') {
      rObjInf[oNorm1+3]  = atof(argv[1]);
      rObjInf[oNorm1+4]  = atof(argv[2]);
      rObjInf[oNorm1+5]  = atof(argv[3]);
      rHouseInf[cSign+3] = atof(argv[4]);
      rHouseInf[cSign+4] = atof(argv[5]);
      rHouseInf[cSign+5] = atof(argv[6]);
      darg += 6;
      break;
    }
    k = ch1 == 'C' ? pmSign : (ch1 == 'A' ? pmAspect : pmObject);
    i = NParseSz(argv[1], k); j = NParseSz(argv[2], k);
    k = ch1 == 'C' ? cSign : (ch1 == 'A' ? cAspect : oNorm1);
    if (!FBetween(i, 0, k)) {
      ErrorValN("Yj", i);
      return tcError;
    }
    if (!FBetween(j, 0, k) || j < i) {
      ErrorValN("Yj", j);
      return tcError;
    }
    if (argc <= 3+j-i) {
      ErrorArgc("Yj");
      return tcError;
    }
    lpr = ch1 == 'C' ? rHouseInf : (ch1 == 'A' ? rAspInf :
      (ch1 == 'T' ? rTransitInf : rObjInf));
    for (k = i; k <= j; k++)
      lpr[k] = atof(argv[3+k-i]);
    darg += 3+j-i;
    break;

  case 'J':
    if (argc <= 3 - (ch1 == '0')) {
      ErrorArgc("YJ");
      return tcError;
    }
    i = NParseSz(argv[1], pmObject);
    if (!FNorm(i)) {
      ErrorValN("YJ", i);
      return tcError;
    }
    j = NParseSz(argv[2], pmSign);
    if (!FBetween(j, 0, cSign)) {
      ErrorValN("YJ", j);
      return tcError;
    }
    if (ch1 != '0') {
      k = NParseSz(argv[3], pmSign);
      if (!FBetween(k, 0, cSign)) {
        ErrorValN("YJ", k);
        return tcError;
      }
    }
    if (ch1 == chNull) {
      ruler1[i] = j;
      ruler2[i] = k;
      if (FBetween(i, 0, oMain)) {
        if (j != 0)
          rules[j] = i;
        if (k != 0)
          rules[k] = i;
      }
    } else if (ch1 == '0') {
      exalt[i] = j;
    } else if (ch2 != '0') {
      rgObjEso1[i] = j;
      rgObjEso2[i] = k;
      if (FBetween(i, 0, oMain)) {
        if (j != 0)
          rgSignEso1[j] = i;
        if (k != 0)
          rgSignEso1[k] = i;
      }
    } else {
      rgObjHie1[i] = j;
      rgObjHie2[i] = k;
      if (FBetween(i, 0, oMain)) {
        if (j != 0)
          rgSignHie1[j] = i;
        if (k != 0)
          rgSignHie1[k] = i;
      }
    }
    darg += 3 - (ch1 == '0');
    break;

  case '7':
    if (argc <= 2) {
      ErrorArgc("Y7");
      return tcError;
    }
    k = ch1 == 'O' ? pmObject : (ch1 == 'C' ? pmSign : 0);
    i = NParseSz(argv[1], k); j = NParseSz(argv[2], k);
    k = ch1 == 'O' ? oNorm : (ch1 == 'C' ? cSign : 0);
    if (!FBetween(i, (int)(ch1 == 'C'), k)) {
      ErrorValN("Y7", i);
      return tcError;
    }
    if (!FBetween(j, (int)(ch1 == 'C'), k) || j < i) {
      ErrorValN("Y7", j);
      return tcError;
    }
    if (argc <= 3+j-i) {
      ErrorArgc("Y7");
      return tcError;
    }
    lpn = ch1 == 'O' ? rgObjRay : (ch1 == 'C' ? rgSignRay : NULL);
    for (k = i; k <= j; k++) {
      l = atoi(argv[3+k-i]);
      if (!FBetween(l, (int)(ch1 == 'C'), ch1 != 'C' ? 7 : 1234567)) {
        ErrorValN("Y7", l);
        return tcError;
      }
      lpn[k] = l;
    }
    darg += 3+j-i;
    break;

#ifdef INTERPRET
  case 'I':
    if (argc <= 2) {
      ErrorArgc("YI");
      return tcError;
    }
    i = NParseSz(argv[1],
      ch1 == 'A' ? pmAspect : (ch1 == chNull ? pmObject : pmSign));
    j = ch1 == 'A' ? cAspect : (ch1 == chNull ? oNorm : cSign);
    if (!FBetween(i, (int)(ch1 != chNull), j)) {
      ErrorValN("YI", i);
      return tcError;
    }
    if (ch1 == 'A' && ch2 == '0')
      ch1 = '0';
    sz = SzPersist(argv[2]);
    switch (ch1) {
    case 'A':    szInteract[i]  = sz; break;
    case '0':    szTherefore[i] = sz; break;
    case chNull: szMindPart[i]  = sz; break;
    case 'C':    szLifeArea[i]  = sz; break;
    case 'v':    szDesire[i]    = sz; break;
    default:     szDesc[i]      = sz;
    }
    darg += 2;
    break;
#endif

  case 'k':
    if (ch1 == 'U') {
      if (argc <= 1) {
        ErrorArgc("YkU");
        return tcError;
      }
      us.szStarsColor = SzPersist(argv[1]);
      darg++;
      break;
    }
    if (ch1 == 'E') {
      if (argc <= 1) {
        ErrorArgc("YkE");
        return tcError;
      }
      us.szAstColor = SzPersist(argv[1]);
      darg++;
      break;
    }
    if (argc <= 2 + 2*(ch1 == 'C')) {
      ErrorArgc("Yk");
      return tcError;
    }
    if (ch1 == 'C') {
      kElemA[eFir] = NParseSz(argv[1], pmColor) & (cColor-1);
      kElemA[eEar] = NParseSz(argv[2], pmColor) & (cColor-1);
      kElemA[eAir] = NParseSz(argv[3], pmColor) & (cColor-1);
      kElemA[eWat] = NParseSz(argv[4], pmColor) & (cColor-1);
      darg += 4;
      break;
    }
    k = ch1 == 'O' ? pmObject : (ch1 == 'A' ? pmAspect : 0);
    i = NParseSz(argv[1], k); j = NParseSz(argv[2], k);
    k = ch1 == 'O' ? oNorm : (ch1 == 'A' ? cAspect :
      (ch1 == '0' || ch1 == '7' ? cRainbow : 8));
    if (!FBetween(i, (int)(ch1 != chNull && ch1 != 'O'), k)) {
      ErrorValN("Yk", i);
      return tcError;
    }
    if (!FBetween(j, (int)(ch1 != chNull && ch1 != 'O'), k) || j < i) {
      ErrorValN("Yk", j);
      return tcError;
    }
    if (argc <= 3+j-i) {
      ErrorArgc("Yk");
      return tcError;
    }
    lpn = ch1 == 'O' ? kObjU : (ch1 == 'A' ? kAspA : (ch1 == '7' ? kRayA :
      (ch1 == '0' ? kRainbowA : kMainA)));
    for (k = i; k <= j; k++)
      lpn[k] = NParseSz(argv[3+k-i], pmColor) & (ch1 != 'O' ? cColor-1 : ~0);
    darg += 3+j-i;
    break;

  case 'D':
    if (argc <= 2) {
      ErrorArgc("YD");
      return tcError;
    }
    i = NParseSz(argv[1], pmObject);
    if (!FItem(i)) {
      ErrorValN("YD", i);
      return tcError;
    }
    if (CchSz(argv[2]) >= 2)
      szObjDisp[i] = SzPersist(argv[2]);
    else
      szObjDisp[i] = szObjName[i];
    darg += 2;
    break;

  case 'F':
    if (argc <= 8) {
      ErrorArgc("YF");
      return tcError;
    }
    i = NParseSz(argv[1], pmObject);
    if (!FItem(i)) {
      ErrorValN("YF", i);
      return tcError;
    }
    r = Mod((real)(atoi(argv[2]) +
      (NParseSz(argv[3], pmSign)-1)*30) + atof(argv[4])/60.0);
    planet[i] = r;
    if (FCusp(i) && i != oAsc && i != oMC) {
      chouse[i-(cuspLo-1)] = r;
      if (i == oDes)
        chouse[sAri] = Mod(chouse[sLib] + rDegHalf);
      else if (i == oNad)
        chouse[sCap] = Mod(chouse[sCan] + rDegHalf);
    }
    j = atoi(argv[5]);
    r = (j < 0 ? -1.0 : 1.0)*((real)NAbs(j) + atof(argv[6])/60.0);
    planetalt[i] = Mod((r + rDegQuad) * 2.0) / 2.0 - rDegQuad;
    ret[i] = atof(argv[7]);
    if (i <= oNorm)
      SphToRec(atof(argv[8]), planet[i], planetalt[i],
        &space[i].x, &space[i].y, &space[i].z);
    MM = -1;
    darg += 8;
    break;

#ifdef GRAPH
  case 'X':
    return NProcessSwitchesRareX(argc, argv, pos+1, fOr, fAnd, fNot);
#endif

  case 'B':
#ifndef WIN
    putchar(chBell);
#else
    MessageBeep((UINT)-1);
#endif
    break;

#ifdef ATLAS
  case 'Y':
    i = ch1 - '0';
    if (argc <= 1 + (i == 1 || i == 2)) {
      ErrorArgc("YY");
      return tcError;
    }
    if (is.fileIn == NULL) {
      PrintError("Switch only allowed in file context.");
      return tcError;
    }
    ch2 = getc(is.fileIn);
    if (ch2 >= ' ')
      ungetc(ch2, is.fileIn);
    if (i <= 0) {
      if (!FLoadAtlas(is.fileIn, atoi(argv[1])))
        return tcError;
    } else if (i == 1) {
      if (!FLoadZoneRules(is.fileIn, atoi(argv[1]), atoi(argv[2])))
        return tcError;
    } else if (i == 2) {
      if (!FLoadZoneChanges(is.fileIn, atoi(argv[1]), atoi(argv[2])))
        return tcError;
    } else if (i >= 3) {
      if (!FLoadZoneLinks(is.fileIn, atoi(argv[1])))
        return tcError;
    }
    darg += 1 + (i == 1 || i == 2);
    break;
#endif

  default:
    ErrorSwitch(argv[0]);
    return tcError;
  }
  return darg;    /* Return the value to be added to argc. */
}


/* Process a command switch line passed to the program. Read each entry in */
/* the argument list and set all the program modes and charts to display.  */

flag FProcessSwitches(int argc, char **argv)
{
  int ich, i, j, k;
  flag fNot, fOr, fAnd;
  real rT;
  char ch1, ch2, *pch;
  CI ci;

  argc--; argv++;
  while (argc) {
    ch1 = argv[0][0];
    fNot = fOr = fAnd = fFalse;
    switch (ch1) {
    case '=': fOr  = fTrue; break;
    case '_': fAnd = fTrue; break;
    case ':':               break;
    default:  fNot = fTrue; break;
    }
    ich = 1 + FChSwitch(ch1);    /* Leading dash? */
    ch1 = argv[0][ich];
    ch2 = ch1 == chNull ? chNull : argv[0][ich+1];
    switch (argv[0][ich-1]) {

    case 'H':
      if (ch1 == 'c')
        SwitchF(us.fCredit);
      else if (ch1 == 'Y')
        SwitchF(us.fSwitchRare);
#ifdef ISG
      else if (ch1 == 'X')
        SwitchF(us.fKeyGraph);
#endif
      else if (ch1 == 'C')
        SwitchF(us.fSign);
      else if (ch1 == 'O')
        SwitchF(us.fObject);
      else if (ch1 == 'A')
        SwitchF(us.fAspect);
      else if (ch1 == 'F')
        SwitchF(us.fConstel);
      else if (ch1 == 'S')
        SwitchF(us.fOrbitData);
      else if (ch1 == '7')
        SwitchF(us.fRay);
      else if (ch1 == 'I')
        SwitchF(us.fMeaning);
      else if (ch1 == 'e') {
        SwitchF(us.fCredit); SwitchF(us.fSwitch); SwitchF(us.fSwitchRare);
        SwitchF(us.fKeyGraph); SwitchF(us.fSign); SwitchF(us.fObject);
        SwitchF(us.fAspect); SwitchF(us.fConstel); SwitchF(us.fOrbitData);
        SwitchF(us.fRay); SwitchF(us.fMeaning);
      } else
        SwitchF(us.fSwitch);
      break;

    case 'Q':
      if (ch1 == '0')
        SwitchF(us.fLoopInit);
      SwitchF(us.fLoop);
      break;

    case 'M':
      if (FBetween(ch1, '1', '4')) {
        i = (ch1 - '0') + (ch2 == '0');
        if (argc <= i) {
          ErrorArgc("M");
          return fFalse;
        }
        for (j = 1; j <= i; j++)
          szWheel[(ch2 == '0' && j >= i) ? 0 : j] = SzPersist(argv[j]);
        argc -= i; argv += i;
        break;
      }
      i = (ch1 == '0');
      if (argc <= 1+i) {
        ErrorArgc("M");
        return fFalse;
      }
      j = atoi(argv[1]);
      if (!FValidMacro(j)) {
        ErrorValN("M", j);
        return fFalse;
      }
      j--;
      if (i)
        szMacro[j] = SzPersist(argv[2]);
      else
        FProcessCommandLine(szMacro[j]);
      argc -= 1+i; argv += 1+i;
      break;

    case 'Y':
      i = NProcessSwitchesRare(argc, argv, ich, fOr, fAnd, fNot);
      if (i < 0)
        return fFalse;
      argc -= i; argv += i;
      break;

    /* Switches which determine the type of chart to display: */

    case 'v':
      if (ch1 == '0')
        SwitchF(us.fVelocity);
      SwitchF(us.fListing);
      break;

    case 'w':
      if (ch1 == '0')
        SwitchF(us.fWheelReverse);
      if (argc > 1 && (i = atoi(argv[1]))) {
        argc--; argv++;
        if (!FValidWheel(i)) {
          ErrorValN("w", i);
          return fFalse;
        }
        us.nWheelRows = i;
      }
      SwitchF(us.fWheel);
      break;

    case 'g':
      if (ch1 == '0' || ch2 == '0')
        SwitchF(us.fGridConfig);
      else if (ch1 == 'm' || ch2 == 'm')
        SwitchF(us.fGridMidpoint);
      if (ch1 == 'a')
        SwitchF(us.fAppSep);
      else if (ch1 == 'p')
        SwitchF(us.fParallel);
#ifdef X11
      else if (ch1 == 'e') {
        if (argc <= 1) {
          ErrorArgc("geometry");
          return fFalse;
        }
        gs.xWin = atoi(argv[1]);
        if (argc > 2 && (gs.yWin = atoi(argv[2]))) {
          argc--; argv++;
        } else
          gs.yWin = gs.xWin;
        if (!FValidGraphx(gs.xWin)) {
          ErrorValN("geometry", gs.xWin);
          return fFalse;
        }
        if (!FValidGraphy(gs.yWin)) {
          ErrorValN("geometry", gs.yWin);
          return fFalse;
        }
        argc--; argv++;
        break;
      }
#endif
      SwitchF(us.fGrid);
      break;

    case 'a':
      SwitchF(us.fAspList);
      if (ch1 == '0') {
        SwitchF(us.fAspSummary);
        ch1 = ch2;
      }
      if (ch1 == 'a') {
        SwitchF(us.fAppSep);
        ch1 = ch2;
      } else if (ch1 == 'p') {
        SwitchF(us.fParallel);
        ch1 = ch2;
      }
      switch (ch1) {
      case 'j': us.nAspectSort = asj; break;
      case 'o': us.nAspectSort = aso; break;
      case 'n': us.nAspectSort = asn; break;
      case 'O': us.nAspectSort = asO; break;
      case 'P': us.nAspectSort = asP; break;
      case 'A': us.nAspectSort = asA; break;
      case 'C': us.nAspectSort = asC; break;
      case 'D': us.nAspectSort = asD; break;
      case 'm': us.nAspectSort = asM; break;
      }
      break;

    case 'm':
      if (ch1 == '0' || ch2 == '0')
        SwitchF(us.fMidSummary);
      if (ch1 == 'a')
        SwitchF(us.fMidAspect);
      SwitchF(us.fMidpoint);
      break;

    case 'Z':
      if (ch1 == 'd') {
        if (ch2 == 'm' || ch2 == 'y' || ch2 == 'Y') {
          if (ch2 == 'y')
            us.nEphemYears = 1;
          else if (ch2 == 'Y') {
            if (argc <= 1) {
              ErrorArgc("ZdY");
              return fFalse;
            }
            i = atoi(argv[1]);
            if (i < 1) {
              ErrorValN("ZdY", i);
              return fFalse;
            }
            us.nEphemYears = i;
            argc--; argv++;
          }
          SwitchF(us.fInDayMonth);
          us.fInDayYear = us.fInDayMonth && (ch2 != 'm');
        }
        SwitchF(us.fHorizonSearch);
        break;
      }
      if (ch1 == '0')
        SwitchF(us.fPrimeVert);
      SwitchF(us.fHorizon);
      break;

    case 'S':
      SwitchF(us.fOrbit);
      break;

    case 'l':
      if (ch1 == '0')
        SwitchF(us.fSectorApprox);
      SwitchF(us.fSector);
      break;

    case 'j':
      if (ch1 == '0')
        SwitchF(us.fInfluenceSign);
      SwitchF(us.fInfluence);
      break;

    case '7':
      SwitchF(us.fEsoteric);
      break;

    case 'L':
      if (ch1 == '0')
        SwitchF(us.fLatitudeCross);
      if (argc > 1 && (i = atoi(argv[1]))) {
        argc--; argv++;
        if (!FValidAstrograph(i)) {
          ErrorValN("L", i);
          return fFalse;
        }
        us.nAstroGraphStep = i;
      }
      SwitchF(us.fAstroGraph);
      break;

    case 'K':
      if (ch1 == 'y')
        SwitchF(us.fCalendarYear);
      SwitchF(us.fCalendar);
      break;

    case 'd':
      if (ch1 == 'p') {
        us.nProgress = (ch2 == '0') + 2*(ch2 == '1');
        if (us.nProgress)
          ch2 = argv[0][ich++ + 1];
        i = (ch2 == 'y') + 2*(ch2 == 'Y');
#ifdef TIME
        j = i < 2 && (argv[0][ich+i+1] == 'n');
#else
        j = fFalse;
#endif
        if (!j && argc <= 2-(i&1)) {
          ErrorArgc("dp");
          return fFalse;
        }
        is.fProgress = us.fInDayMonth = fTrue;
        DstT = us.dstDef; ZonT = us.zonDef;
        LonT = us.lonDef; LatT = us.latDef;
#ifdef TIME
        if (j)
          GetTimeNow(&MonT, &DayT, &YeaT, &TimT, DstT, ZonT);
#endif
        if (i) {
          us.fInDayYear = fTrue;
          if (!j)
            YeaT = NParseSz(argv[1], pmYea);
          us.nEphemYears = i == 2 ? atoi(argv[2]) : 1;
        } else {
          if (!j) {
            MonT = NParseSz(argv[1], pmMon);
            YeaT = NParseSz(argv[2], pmYea);
            if (!FValidMon(MonT)) {
              ErrorValN("dp", MonT);
              return fFalse;
            }
          }
        }
        if (!FValidYea(YeaT)) {
          ErrorValN("dp", YeaT);
          return fFalse;
        }
        if (!j) {
          i = 2-(i&1);
          argc -= i; argv += i;
        }
      } else if (ch1 == 'm' || ch1 == 'y' || ch1 == 'Y') {
        if (ch1 == 'y')
          us.nEphemYears = 1;
        else if (ch1 == 'Y') {
          if (argc <= 1) {
            ErrorArgc("dY");
            return fFalse;
          }
          i = atoi(argv[1]);
          if (i < 1) {
            ErrorValN("dY", i);
            return fFalse;
          }
          us.nEphemYears = i;
          argc--; argv++;
        }
        SwitchF(us.fInDayMonth);
        us.fInDayYear = us.fInDayMonth && (ch1 != 'm');
      }
#ifdef X11
      else if (ch1 == 'i') {    /* -display switch for X */
        if (argc <= 1) {
          ErrorArgc("display");
          return fFalse;
        }
        gs.szDisplay = SzPersist(argv[1]);
        argc--; argv++;
        break;
      }
#endif
      else if (argc > 1 && (i = atoi(argv[1]))) {
        if (!FValidDivision(i)) {
          ErrorValN("d", i);
          return fFalse;
        }
        us.nDivision = i;
        argc--; argv++;
      }
      SwitchF(us.fInDay);
      break;

    case 'D':
      SwitchF(us.fInDayInf);
      break;

    case 'E':
      j = ch1 == '0' || ch2 == '0';
      if (ch1 == 'Y' && argc <= 1 + j) {
        ErrorArgc("E");
        return fFalse;
      }
      SwitchF(us.fEphemeris);
      if (ch1 == 'y')
        us.nEphemYears = us.fEphemeris ? 1 : 0;
      else if (ch1 == 'Y') {
        i = atoi(argv[1]);
        if (i < 1) {
          ErrorValN("EY", i);
          return fFalse;
        }
        us.nEphemYears = i;
        argc--; argv++;
      }
      if (j) {
        ch1 = argv[1][0];
        if (ch1) {
          us.nEphemRate = ch1 == 'm' ? 1 : (ch1 == 'y' ? 2 : 0);
          i = atoi(&argv[1][1]);
          us.nEphemFactor = Max(i, 1);
        }
        argc--; argv++;
      }
      break;

    case 'e':
      SwitchF(us.fListing); SwitchF(us.fWheel);
      SwitchF(us.fGrid); SwitchF(us.fAspList); SwitchF(us.fMidpoint);
      SwitchF(us.fHorizon); SwitchF(us.fOrbit); SwitchF(us.fSector);
      SwitchF(us.fInfluence); SwitchF(us.fEsoteric); SwitchF(us.fAstroGraph);
      SwitchF(us.fCalendar); SwitchF(us.fHorizonSearch);
      SwitchF(us.fInDay); SwitchF(us.fInDayInf); SwitchF(us.fInDayGra);
      SwitchF(us.fEphemeris);
      break;

    case 't':
      SwitchF(us.fTransit);
      ZonT = us.zonDef; DstT = us.dstDef; LonT = us.lonDef; LatT = us.latDef;
      if (ch1 == 'p') {
        us.nProgress = (ch2 == '0') + 2*(ch2 == '1');
        if (us.nProgress)
          ich++;
        is.fProgress = fTrue;
        ch1 = argv[0][++ich];
      }
      if (ch1 == 'r') {
        is.fReturn = fTrue;
        ch1 = argv[0][++ich];
      }
      i = (ch1 == 'y') + 2*(ch1 == 'Y') - (ch1 == 'd');
      if (i != 0)
        ch1 = argv[0][++ich];
      us.fInDayMonth = (i >= 0);
      us.fInDayYear = (i >= 1);
#ifdef TIME
      if (ch1 == 'n') {
        GetTimeNow(&MonT, &DayT, &YeaT, &TimT, DstT, ZonT);
        if (i >= 2) {
          if (argc <= 1) {
            ErrorArgc("tYn");
            return fFalse;
          }
          us.nEphemYears = atoi(argv[1]);
          argc--; argv++;
        }
        break;
      }
#endif
      if (argc <= 2 - (i == 1) + (i < 0)) {
        ErrorArgc("t");
        return fFalse;
      }
      YeaT = NParseSz(argv[2 - (i > 0) + (i < 0)], pmYea);
      if (!FValidYea(YeaT)) {
        ErrorValN("t", YeaT);
        return fFalse;
      }
      if (i <= 0) {
        MonT = NParseSz(argv[1], pmMon);
        if (!FValidMon(MonT)) {
          ErrorValN("t", MonT);
          return fFalse;
        }
      }
      if (i < 0) {
        DayT = NParseSz(argv[2], pmDay);
        if (!FValidDay(DayT, MonT, YeaT)) {
          ErrorValN("td", DayT);
          return fFalse;
        }
      }
      if (i > 1)
        us.nEphemYears = atoi(argv[2]);
      i = 2 - (i == 1) + (i < 0);
      argc -= i; argv += i;
      break;

    case 'T':
      SwitchF(us.fTransitInf);
      ZonT = us.zonDef; DstT = us.dstDef; LonT = us.lonDef; LatT = us.latDef;
      i = (ch1 == 't');
      if (i > 0)
        ch1 = argv[0][++ich];
      if (ch1 == 'p') {
        is.fProgress = fTrue;
        ch1 = argv[0][++ich];
      }
      if (ch1 == 'r') {
        is.fReturn = fTrue;
        ch1 = argv[0][++ich];
      }
#ifdef TIME
      if (ch1 == 'n') {
        GetTimeNow(&MonT, &DayT, &YeaT, &TimT, DstT, ZonT);
        break;
      }
#endif
      if (argc <= 3 + i) {
        ErrorArgc("T");
        return fFalse;
      }
      MonT = NParseSz(argv[1], pmMon);
      DayT = NParseSz(argv[2], pmDay);
      YeaT = NParseSz(argv[3], pmYea);
      TimT = i > 0 ? RParseSz(argv[4], pmTim) : Tim;
      if (!FValidMon(MonT)) {
        ErrorValN("T", MonT);
        return fFalse;
      } else if (!FValidYea(YeaT)) {
        ErrorValN("T", YeaT);
        return fFalse;
      } else if (!FValidDay(DayT, MonT, YeaT)) {
        ErrorValN("T", DayT);
        return fFalse;
      } else if (i > 0 && !FValidTim(TimT)) {
        ErrorValR("T0", TimT);
        return fFalse;
      }
      argc -= 3+i; argv += 3+i;
      break;

    case 'B':
      SwitchF(us.fInDayGra);
      if (ch1 == 'p') {
        is.fProgress = fTrue;
        ch1 = argv[0][++ich];
      }
      if (ch1 == 'm' || ch1 == 'y' || ch1 == 'Y') {
        if (ch1 == 'y')
          us.nEphemYears = 1;
        else if (ch1 == 'Y')
          us.nEphemYears = 5;
        SwitchF(us.fInDayMonth);
        us.fInDayYear = us.fInDayMonth && (ch1 != 'm');
      }
      if (ch1 == '0' || ch2 == '0')
        SwitchF(us.fGraphAll);
      break;

    case 'V':
      SwitchF(us.fTransitGra);
      ZonT = us.zonDef; DstT = us.dstDef; LonT = us.lonDef; LatT = us.latDef;
      if (ch1 == 'p') {
        is.fProgress = fTrue;
        ch1 = argv[0][++ich];
      }
      if (ch1 == 'r') {
        is.fReturn = fTrue;
        ch1 = argv[0][++ich];
      }
      if (i = (ch1 == 'd') + 2*(ch1 == 'm') + 3*(ch1 == 'y') + 4*(ch1 == 'Y'))
        ch1 = argv[0][++ich];
      if (i < 1)
        i = 2;
      if (ch1 == '0') {
        SwitchF(us.fGraphAll);
        ch1 = argv[0][++ich];
      }
      if (i >= 2) {
        if (i == 3)
          us.nEphemYears = 1;
        else if (i == 4)
          us.nEphemYears = 5;
        SwitchF(us.fInDayMonth);
        if (i >= 3)
          us.fInDayYear = us.fInDayMonth;
      }
#ifdef TIME
      if (ch1 == 'n') {
        GetTimeNow(&MonT, &DayT, &YeaT, &TimT, DstT, ZonT);
        if (i >= 3)
          us.fInDayYear = us.fInDayMonth;
        break;
      }
#endif
      j = i < 2 ? 3 : (i == 2 ? 2 : 1);
      if (argc <= j) {
        ErrorArgc("V");
        return fFalse;
      }
      if (i == 1) {
        MonT = NParseSz(argv[1], pmMon);
        DayT = NParseSz(argv[2], pmDay);
        YeaT = NParseSz(argv[3], pmYea);
      } else if (i == 2) {
        MonT = NParseSz(argv[1], pmMon);
        YeaT = NParseSz(argv[2], pmYea);
        DayT = 1;
      } else {
        YeaT = NParseSz(argv[1], pmYea);
        MonT = DayT = 1;
      }
      if (!FValidMon(MonT)) {
        ErrorValN("V", MonT);
        return fFalse;
      }
      if (!FValidYea(YeaT)) {
        ErrorValN("V", YeaT);
        return fFalse;
      }
      if (!FValidDay(DayT, MonT, YeaT)) {
        ErrorValN("V", DayT);
        return fFalse;
      }
      argc -= j; argv += j;
      break;

#ifdef ARABIC
    case 'P':
      if (argc > 1 && (i = atoi(argv[1]))) {
        argc--; argv++;
        if (!FValidPart(i)) {
          ErrorValN("P", i);
          return fFalse;
        }
        us.nArabicParts = i;
      }
      if (ch1 == 'z' || ch1 == 'n' || ch1 == 'f') {
        us.nArabic = ch1;
        ch1 = ch2;
      } else
        SwitchF(us.nArabic);
      if (ch1 == '0')
        SwitchF(us.fArabicFlip);
      break;
#endif

    case 'N':
      if (argc > 1 && (i = atoi(argv[1]))) {
        argc--; argv++;
        us.nAtlasList = i;
      }
      if (ch1 == 'z')
        SwitchF(us.fZoneChange);
      else if (ch1 == 'l')
        SwitchF(us.fAtlasNear);
      else
        SwitchF(us.fAtlasLook);
      break;

    case 'I':
      if (argc > 1 && (i = atoi(argv[1]))) {
        argc--; argv++;
        if (!FValidScreen(i)) {
          ErrorValN("I", i);
          return fFalse;
        }
        us.nScreenWidth = i;
      }
      SwitchF(us.fInterpret);
      break;

    /* Switches which affect how the chart parameters are obtained: */

#ifdef TIME
    case 'n':
      FInputData(szNowCore);
      if (ch1 == 'd')
        TT = 0.0;
      else if (ch1 == 'm') {
        DD = 1; TT = 0.0;
      } else if (ch1 == 'y') {
        MM = DD = 1; TT = 0.0;
      }
      break;
#endif

    case 'z':
      if (ch1 == '0') {
        if (argc <= 1 || RParseSz(argv[1], pmDst) == rLarge) {
          i = us.dstDef != 0.0;
          SwitchF(i);
          SS = us.dstDef = i ? 1.0 : 0.0;
        } else {
          SS = us.dstDef = RParseSz(argv[1], pmDst);
          if (!FValidDst(us.dstDef)) {
            ErrorValR("z0", us.dstDef);
            return fFalse;
          }
          argc--; argv++;
        }
        break;
      } else if (ch1 == 'l') {
        if (argc <= 2) {
          ErrorArgc("zl");
          return fFalse;
        }
        OO = us.lonDef = RParseSz(argv[1], pmLon);
        AA = us.latDef = RParseSz(argv[2], pmLat);
        if (!FValidLon(us.lonDef)) {
          ErrorValR("zl", us.lonDef);
          return fFalse;
        } else if (!FValidLat(us.latDef)) {
          ErrorValR("zl", us.latDef);
          return fFalse;
        }
        argc -= 2; argv += 2;
        break;
      } else if (ch1 == 'v') {
        if (argc <= 1) {
          ErrorArgc("zv");
          return fFalse;
        }
        us.elvDef = RParseSz(argv[1], pmElv);
        argc--; argv++;
        break;
      } else if (ch1 == 'j') {
        if (argc <= 2) {
          ErrorArgc("zj");
          return fFalse;
        }
        us.namDef = SzPersist(argv[1]);
        us.locDef = SzPersist(argv[2]);
        argc -= 2; argv += 2;
        break;
      } else if (ch1 == 't') {
        if (argc <= 1) {
          ErrorArgc("zt");
          return fFalse;
        }
        rT = RParseSz(argv[1], pmTim);
        if (!FValidTim(rT)) {
          ErrorValR("zt", rT);
          return fFalse;
        }
        TT = rT;
        argc--; argv++;
        break;
      } else if (ch1 == 'd') {
        if (argc <= 1) {
          ErrorArgc("zd");
          return fFalse;
        }
        i = NParseSz(argv[1], pmDay);
        if (!FValidDay(i, MM, YY)) {
          ErrorValN("zd", i);
          return fFalse;
        }
        DD = i;
        argc--; argv++;
        break;
      } else if (ch1 == 'm') {
        if (argc <= 1) {
          ErrorArgc("zm");
          return fFalse;
        }
        i = NParseSz(argv[1], pmMon);
        if (!FValidMon(i)) {
          ErrorValN("zm", i);
          return fFalse;
        }
        MM = i;
        argc--; argv++;
        break;
      } else if (ch1 == 'y') {
        if (argc <= 1) {
          ErrorArgc("zy");
          return fFalse;
        }
        i = NParseSz(argv[1], pmYea);
        if (!FValidYea(i)) {
          ErrorValN("zy", i);
          return fFalse;
        }
        YY = i;
        argc--; argv++;
        break;
      } else if (ch1 == 'i') {
        if (argc <= 2) {
          ErrorArgc("zi");
          return fFalse;
        }
        ciCore.nam = SzPersist(argv[1]);
        ciCore.loc = SzPersist(argv[2]);
        argc -= 2; argv += 2;
        break;
      }
#ifdef ATLAS
      else if (ch1 == 'N') {
        if (argc <= 1) {
          ErrorArgc("zc");
          return fFalse;
        }
        if (!DisplayAtlasLookup(argv[1], 0, &i))
          PrintWarning("City doesn't match anything in atlas.");
        else if (!DisplayTimezoneChanges(is.rgae[i].izn, 0, &ciCore))
          PrintWarning("Couldn't get time zone data!");
        argc--; argv++;
        break;
      }
#endif
      if (argc <= 1 || RParseSz(argv[1], pmZon) == rLarge)
        ZZ -= 1.0;
      else {
        ZZ = us.zonDef = RParseSz(argv[1], pmZon);
        if (!FValidZon(us.zonDef)) {
          ErrorValR("z", us.zonDef);
          return fFalse;
        }
        argc--; argv++;
      }
      break;

    case 'q':
      i = (ch1 == 'y' || ch1 == 'j') + 2*(ch1 == 'm') + 3*(ch1 == 'd') +
        7*(ch1 == 'a') + 8*(ch1 == 'b');
      if (i <= 0)
        i = 4;
      if (argc <= i) {
        ErrorArgc("q");
        return fFalse;
      }
      is.fHaveInfo = fTrue;
      if (ch1 == 'j') {
        is.JD = atof(argv[1]) + rRound;
        TT = RFract(is.JD);
        JulianToMdy(is.JD - TT, &MM, &DD, &YY);
        TT *= 24.0;
        SS = ZZ = 0.0; OO = us.lonDef; AA = us.latDef;
      } else {
        MM = i > 1 ? NParseSz(argv[1], pmMon) : 1;
        DD = i > 2 ? NParseSz(argv[2], pmDay) : 1;
        YY = NParseSz(argv[3-(i<3)-(i<2)], pmYea);
        TT = i > 3 ? RParseSz(argv[4], pmTim) : (i < 3 ? 0.0 : 12.0);
        SS = i > 7 ? RParseSz(argv[5], pmDst) : (i > 6 ? 0.0 : us.dstDef);
        ZZ = i > 6 ? RParseSz(argv[5 + (i > 7)], pmZon) : us.zonDef;
        OO = i > 6 ? RParseSz(argv[6 + (i > 7)], pmLon) : us.lonDef;
        AA = i > 6 ? RParseSz(argv[7 + (i > 7)], pmLat) : us.latDef;
        if (!FValidMon(MM)) {
          ErrorValN("q", MM);
          return fFalse;
        } else if (!FValidDay(DD, MM, YY)) {
          ErrorValN("q", DD);
          return fFalse;
        } else if (!FValidYea(YY)) {
          ErrorValN("q", YY);
          return fFalse;
        } else if (!FValidTim(TT)) {
          ErrorValR("q", TT);
          return fFalse;
        } else if (!FValidDst(SS)) {
          ErrorValR("q", SS);
          return fFalse;
        } else if (!FValidZon(ZZ)) {
          ErrorValR("q", ZZ);
          return fFalse;
        } else if (!FValidLon(OO)) {
          ErrorValR("q", OO);
          return fFalse;
        } else if (!FValidLat(AA)) {
          ErrorValR("q", AA);
          return fFalse;
        }
      }
      argc -= i; argv += i;
      break;

    case 'i':
      if (us.fNoRead) {
        ErrorArgv("i");
        return tcError;
      }
      if (argc <= 1) {
        ErrorArgc("i");
        return fFalse;
      }
      ci = ciCore;
      if (!FInputData(argv[1]))
        return fFalse;
      if (ch1 == '2') {
        ciTwin = ciCore;
        ciCore = ci;
      } else if (ch1 == '3') {
        ciThre = ciCore;
        ciCore = ci;
      } else if (ch1 == '4') {
        ciFour = ciCore;
        ciCore = ci;
      } else if (ch1 == 's') {
        ciSave = ciCore;
        ciCore = ci;
      } else if (ch1 == 't') {
        ciTran = ciCore;
        ciCore = ci;
      } else if (ch1 == 'g') {
        ciGreg = ciCore;
        ciCore = ci;
      }
      argc--; argv++;
      break;

    case '>':
      ch1 = 's';
      /* Fall through */

    case 'o':
      if (us.fNoWrite) {
        ErrorArgv("o");
        return tcError;
      }
      if (argc <= 1) {
        ErrorArgc("o");
        return fFalse;
      }
      if (ch1 == 's') {
        is.szFileScreen = SzPersist(argv[1]);
        argc--; argv++;
        break;
      } else if (ch1 == '0')
        SwitchF(us.fWritePos);
      SwitchF(us.fWriteFile);
      is.szFileOut = SzPersist(argv[1]);
      if (is.fSzPersist) {
        is.rgszComment = argv;
        do {
          argc--; argv++;
          is.cszComment++;
        } while (argc > 1 && !FChSwitch(argv[1][0]));
      }
      break;

    /* Switches which affect what information is used in a chart: */

    case 'R':
      if (ch1 == 'A') {
        if (ch2 == '0')
          for (i = 1; i <= cAspect; i++)
            ignorea[i] = fTrue;
        else if (ch2 == '1')
          for (i = 1; i <= cAspect; i++)
            ignorea[i] = fFalse;
        while (argc > 1 && (i = NParseSz(argv[1], pmAspect)))
          if (!FAspect(i)) {
            ErrorValN("RA", i);
            return fFalse;
          } else {
            SwitchF(ignorea[i]);
            argc--; argv++;
          }
        for (us.nAsp = cAspect; us.nAsp > 0 && ignorea[us.nAsp]; us.nAsp--)
          ;
        break;
      }
      if (ch1 == 'O') {
        if (argc <= 1) {
          ErrorArgc("RO");
          return fFalse;
        }
        i = NParseSz(argv[1], pmObject);
        if (!FBetween(i, -1, cObj)) {
          ErrorValN("RO", i);
          return fFalse;
        }
        us.objRequire = i;
        argc--; argv++;
        break;
      }
      if (ch1 == 'T') {
        pch = (char *)ignore2;
        ch1 = argv[0][++ich];
      } else
        pch = (char *)ignore;
      if (ch1 == '0')
        for (i = 0; i <= cObj; i++)
          pch[i] = fTrue;
      else if (ch1 == '1')
        for (i = 0; i <= cObj; i++)
          pch[i] = fFalse;
      else if (ch1 == 'C')
        for (i = cuspLo; i <= cuspHi; i++)
          SwitchF(pch[i]);
      else if (ch1 == 'u')
        for (i = uranLo; i <= uranHi; i++)
          SwitchF(pch[i]);
      else if (ch1 == 'U')
        for (i = starLo; i <= starHi; i++)
          SwitchF(pch[i]);
      else if (argc <= 1 || NParseSz(argv[1], pmObject) < 0)
        for (i = oChi; i <= oEP; i++)
          if (i != oNod)
            SwitchF(pch[i]);
      while (argc > 1 && (i = NParseSz(argv[1], pmObject)) >= 0)
        if (!FItem(i)) {
          ErrorValN("R", i);
          return fFalse;
        } else {
          if (ch1 != 'C' && ch1 != 'u' && ch1 != 'U')
            SwitchF(pch[i]);
          else
            inv(pch[i]);
          argc--; argv++;
        }
      for (j = fFalse, i = cuspLo; i <= cuspHi; i++)
        if (!ignore[i] || !ignore2[i]) {
          j = fTrue;
          break;
        }
      us.fCusp = j;
      for (j = fFalse, i = uranLo; i <= uranHi; i++)
        if (!ignore[i] || !ignore2[i]) {
          j = fTrue;
          break;
        }
      us.fUranian = j;
      for (j = fFalse, i = starLo; i <= starHi; i++)
        if (!ignore[i] || !ignore2[i]) {
          j = fTrue;
          break;
        }
      if (!(us.nStar && j))
        us.nStar = j;
      break;

    case 'C':
      j = us.fCusp;
      SwitchF(us.fCusp);
      if (j != us.fCusp)
        for (i = cuspLo; i <= cuspHi; i++)
          ignore[i] = j;
      break;

    case 'u':
      j = us.fUranian;
      SwitchF(us.fUranian);
      if (j != us.fUranian)
        for (i = uranLo; i <= uranHi; i++)
          ignore[i] = j;
      break;

    case 'U':
      j = us.nStar;
      if (ch1 == 'z' || ch1 == 'l' || ch1 == 'n' || ch1 == 'b' ||
        ch1 == 'd' || ch1 == 'v')
        us.nStar = ch1;
      else
        SwitchF(us.nStar);
      if ((j != 0) != (us.nStar != 0))
        for (i = starLo; i <= starHi; i++)
          ignore[i] = (j != 0);
      break;

    case 'A':
      if (ch1 == '3')
        SwitchF(us.fAspect3D);
      else if (ch1 == 'p')
        SwitchF(us.fAspectLat);
      else if (ch1 != 'o' && ch1 != 'm' && ch1 != 'd' && ch1 != 'a') {
        if (argc <= 1) {
          ErrorArgc("A");
          return fFalse;
        }
        i = NParseSz(argv[1], pmAspect);
        if (!FValidAspect(i)) {
          ErrorValN("A", i);
          return fFalse;
        }
        for (j = us.nAsp + 1; j <= i; j++)
          ignorea[j] = fFalse;
        for (j = i + 1; j <= cAspect; j++)
          ignorea[j] = fTrue;
        us.nAsp = i;
        argc--; argv++;
      } else {
        if (argc <= 2) {
          ErrorArgc("A");
          return fFalse;
        }
        i = NParseSz(argv[1], ch1 == 'o' || ch1 == 'a' ? pmAspect : pmObject);
        if (i < (int)(ch1 == 'o' || ch1 == 'a') ||
          i > (ch1 == 'o' || ch1 == 'a' ? cAspect : oNorm+1)) {
          ErrorValN("A", i);
          return fFalse;
        }
        rT = RParseSz(argv[2], 0);
        if (rT < -rDegMax || rT > rDegMax) {
          ErrorValR("A", rT);
          return fFalse;
        }
        if (ch1 == 'o')
          rAspOrb[i] = rT;
        else if (ch1 == 'm')
          rObjOrb[i] = rT;
        else if (ch1 == 'd')
          rObjAdd[i] = rT;
        else
          rAspAngle[i] = rT;
        argc -= 2; argv += 2;
      }
      break;

    /* Switches which affect how a chart is computed: */

    case 'b':
      if (ch1 == '0') {
        SwitchF(us.fSeconds);
        is.fSeconds = us.fSeconds;
        break;
      } else if (ch1 == 's')
        SwitchF(us.fSwissMosh);
      else if (ch1 == 'p')
        SwitchF(us.fPlacalcPla);
      else if (ch1 == 'm')
        SwitchF(us.fMatrixPla);
      else if (ch1 == 'a')
        SwitchF(us.fPlacalcAst);
      else if (ch1 == 'U')
        SwitchF(us.fMatrixStar);
      SwitchF(us.fEphemFiles);
      break;

    case 'c':
      if (ch1 == '3') {
        SwitchF(us.fHouse3D);
        break;
      }
#ifdef WIN
      if (argc <= 1) {
        if (!wi.fSaverCfg)
          ErrorArgc("c");
        return fFalse;
      }
#endif
      i = NParseSz(argv[1], pmSystem);
      if (!FValidSystem(i)) {
        ErrorValN("c", i);
        return fFalse;
      }
      us.nHouseSystem = i;
      argc--; argv++;
      break;

    case 's':
      if (argc > 1 && ((rT = atof(argv[1])) != 0.0 || FNumCh(argv[1][0]))) {
        if (!FValidOffset(rT)) {
          ErrorValR("s", rT);
          return fFalse;
        }
        argc--; argv++;
        us.rZodiacOffset = rT;
      }
      if (ch1 == 'r') {
        if (ch2 != '0')
          SwitchF(us.fEquator);
        SwitchF(us.fEquator2);
      } else if (ch1 == 'h')
        us.nDegForm = 1;
      else if (ch1 == 'd')
        us.nDegForm = 2;
      else if (ch1 == 'z')
        us.nDegForm = 0;
#ifdef WIN
      else if (ch1 == 'e')    /* -setup switch for Windows */
        i = FCreateProgramGroup(fFalse) && FCreateDesktopIcon() &&
          FRegisterExtensions();
#endif
      else
        SwitchF(us.fSidereal);
      break;

    case 'h':
      if (argc > 1 && (i = NParseSz(argv[1], pmObject)) >= 0) {
        argc--; argv++;
      } else
        i = FSwitchF(us.objCenter != 0);
      if (!FValidCenter(i)) {
        ErrorValN("h", i);
        return fFalse;
      }
      SetCentric(i);
      break;

    case 'p':
      if (fAnd) {
        us.fProgress = fFalse;
        break;
      }
      SwitchF(us.fProgress);
      us.nProgress = (ch1 == '0') + 2*(ch1 == '1');
      if (us.nProgress)
        ch1 = argv[0][++ich];
#ifdef TIME
      if (ch1 == 'n') {
        GetTimeNow(&MonT, &DayT, &YeaT, &TimT, us.dstDef, us.zonDef);
        is.JDp = MdytszToJulian(MonT, DayT, YeaT, TimT, us.dstDef, us.zonDef);
        break;
      }
#endif
      if (ch1 == 'd') {
        if (argc <= 1) {
          ErrorArgc("pd");
          return fFalse;
        }
        us.rProgDay = atof(argv[1]);
        if (us.rProgDay == 0.0) {
          ErrorValR("pd", us.rProgDay);
          return fFalse;
        }
        argc--; argv++;
        break;
      } else if (ch1 == 'C') {
        if (argc <= 1) {
          ErrorArgc("pC");
          return fFalse;
        }
        us.rProgCusp = atof(argv[1]);
        if (us.rProgCusp == 0.0) {
          ErrorValR("pC", us.rProgCusp);
          return fFalse;
        }
        argc--; argv++;
        break;
      }
      i = 3 + (ch1 == 't');
      if (argc <= i) {
        ErrorArgc("p");
        return fFalse;
      }
      MonT = NParseSz(argv[1], pmMon);
      DayT = NParseSz(argv[2], pmDay);
      YeaT = NParseSz(argv[3], pmYea);
      TimT = ch1 == 't' ? RParseSz(argv[4], pmTim) : 0.0;
      if (!FValidMon(MonT)) {
        ErrorValN("p", MonT);
        return fFalse;
      } else if (!FValidDay(DayT, MonT, YeaT)) {
        ErrorValN("p", DayT);
        return fFalse;
      } else if (!FValidYea(YeaT)) {
        ErrorValN("p", YeaT);
        return fFalse;
      } else if (ch1 == 't' && !FValidTim(TimT)) {
        ErrorValR("p", TimT);
        return fFalse;
      }
      is.JDp = MdytszToJulian(MonT, DayT, YeaT, TimT, us.dstDef, us.zonDef);
      argc -= i; argv += i;
      break;

    case 'x':
      if (argc <= 1) {
        ErrorArgc("x");
        return fFalse;
      }
      rT = atof(argv[1]);
      if (!FValidHarmonic(rT)) {
        ErrorValR("x", rT);
        return fFalse;
      }
      us.rHarmonic = rT;
      argc--; argv++;
      break;

    case '1':
      if (argc > 1 && (i = NParseSz(argv[1], pmObject)) >= 0) {
        argc--; argv++;
      } else
        i = oSun;
      if (!FItem(i)) {
        ErrorValN("1", i);
        return fFalse;
      }
      us.fSolarWhole = (ch1 == '0' && !fAnd);
      us.objOnAsc = fAnd ? 0 : i+1;
      break;

    case '2':
      if (argc > 1 && (i = NParseSz(argv[1], pmObject)) >= 0) {
        argc--; argv++;
      } else
        i = oSun;
      if (!FItem(i)) {
        ErrorValN("2", i);
        return fFalse;
      }
      us.fSolarWhole = (ch1 == '0' && !fAnd);
      us.objOnAsc = fAnd ? 0 : -(i+1);
      break;

    case '3':
      SwitchF(us.fDecan);
      break;

    case '4':
      if (argc > 1 && (i = atoi(argv[1])) >= 0) {
        argc--; argv++;
      } else
        i = 1;
      if (!FValidDwad(i)) {
        ErrorValN("4", i);
        return fFalse;
      }
      us.nDwad = fAnd ? 0 : i;
      break;

    case 'f':
      SwitchF(us.fFlip);
      break;

    case 'G':
      SwitchF(us.fGeodetic);
      break;

    case 'J':
      SwitchF(us.fVedic);
      break;

    case '9':
      SwitchF(us.fNavamsa);
      break;

    case 'F':
      if (argc <= 3) {
        ErrorArgc("F");
        return fFalse;
      }
      i = NParseSz(argv[1], pmObject);
      if (!FItem(i)) {
        ErrorValN("F", i);
        return fFalse;
      }
      if (ch1 != 'm') {
        force[i] = ZD(NParseSz(argv[2], pmSign), atof(argv[3]));
        if (force[i] < 0.0 || force[i] >= rDegMax) {
          ErrorValR("F", force[i]);
          return fFalse;
        }
        force[i] += rDegMax;
      } else {
        j = NParseSz(argv[2], pmObject);
        if (!FItem(j)) {
          ErrorValN("Fm", j);
          return fFalse;
        }
        k = NParseSz(argv[3], pmObject);
        if (!FItem(k)) {
          ErrorValN("Fm", k);
          return fFalse;
        }
        force[i] = (real)-(j * cObj + k + 1);
      }
      argc -= 3; argv += 3;
      break;

    case '+':
      if (argc > 1 && (i = atoi(argv[1])) != 0) {
        argc--; argv++;
      } else
        i = 1;
      us.dayDelta += i * (ch1 == 'y' ? 365 : (ch1 == 'm' ? 30 : 1));
      break;

    case chNull:
      if (ich <= 1)
        break;
      /* Fall thorugh */

    case '-':
      if (argc > 1 && (i = atoi(argv[1])) != 0) {
        argc--; argv++;
      } else
        i = 1;
      us.dayDelta -= i * (ch1 == 'y' ? 365 : (ch1 == 'm' ? 30 : 1));
      break;

    /* Switches for relationship and comparison charts: */

    case 'r':
      if (fAnd) {
        us.nRel = rcNone;
        break;
      } else if (FBetween(ch1, '1', '4')) {
        us.nRel = -(int)(ch1-'1');
        break;
      }
      i = 2 + 2*((ch1 == 'c' || ch1 == 'm') && ch2 == '0');
      if (argc <= i) {
        ErrorArgc("r");
        return fFalse;
      }
      if (ch1 == 'c')
        us.nRel = rcComposite;
      else if (ch1 == 'm')
        us.nRel = rcMidpoint;
      else if (ch1 == 'd')
        us.nRel = rcDifference;
#ifdef BIORHYTHM
      else if (ch1 == 'b')
        us.nRel = rcBiorhythm;
#endif
      else if (ch1 == '0')
        us.nRel = rcDual;
      else if (ch1 == 't')
        us.nRel = rcTransit;
      else if (ch1 == 'p') {
        us.nRel = rcProgress;
        us.nProgress = (ch2 == '0') + 2*(ch2 == '1');
      } else
        us.nRel = rcSynastry;
      ci = ciCore;
      ciCore = ciTwin;
      if (!FInputData(argv[2]))
        return fFalse;
      ciTwin = ciCore;
      ciCore = ci;
      if (!FInputData(argv[1]))
        return fFalse;
      if (i > 2) {
        us.nRatio1 = atoi(argv[3]);
        us.nRatio2 = atoi(argv[4]);
        if (us.nRatio1 == us.nRatio2)
          us.nRatio1 = us.nRatio2 = 1;
      }
      argc -= i; argv += i;
      break;

#ifdef TIME
    case 'y':
      if (argc <= 1) {
        ErrorArgc("y");
        return fFalse;
      }
      if (ch1 == 'd')
        us.nRel = rcDifference;
#ifdef BIORHYTHM
      else if (ch1 == 'b')
        us.nRel = rcBiorhythm;
#endif
      else if (ch1 == 't')
        us.nRel = rcTransit;
      else if (ch1 == 'p') {
        us.nRel = rcProgress;
        us.nProgress = (ch2 == '0') + 2*(ch2 == '1');
      } else
        us.nRel = rcDual;
      if (!FInputData(szNowCore))
        return fFalse;
      ciTwin = ciCore;
      if (!FInputData(argv[1]))
        return fFalse;
      argc--; argv++;
      break;
#endif

    /* Switches to access graphics options: */

    case 'k':
      if (ch1 == 'h') {
        SwitchF(us.fTextHTML);
        break;
      }
      if (ch1 == '1') {     /* Undocumented subswitch. */
        us.fAnsiColor = 2;
        us.fAnsiChar  = 1;
      } else {
        if (ch1 != '0')
          SwitchF(us.fAnsiColor);
        SwitchF(us.fAnsiChar);
      }
      break;

#ifdef GRAPH
    case 'X':
      if (us.fNoGraphics) {
        ErrorArgv("X");
        return fFalse;
      }
      i = NProcessSwitchesX(argc, argv, ich, fOr, fAnd, fNot);
      if (i < 0)
        return fFalse;
      SwitchF2(us.fGraphics);
      argc -= i; argv += i;
      break;

#ifdef WIN
    case 'W':
      i = NProcessSwitchesW(argc, argv, ich, fOr, fAnd, fNot);
      if (i < 0)
        return fFalse;
      argc -= i; argv += i;
      break;
#endif
#endif /* GRAPH */

    case '0':
      while (ch1 != chNull) {
        switch (ch1) {
        case 'o': us.fNoWrite    = fTrue; break;
        case 'i': us.fNoRead     = fTrue; break;
        case 'q': us.fNoQuit     = fTrue; break;
        case 'X': us.fNoGraphics = fTrue; break;
        case '~': us.fNoExp      = fTrue; break;
        }
        ch1 = argv[0][++ich];
      }
      break;

    case '?':    /* Common command line usage does the same as -H. */
      SwitchF(us.fSwitch);
      break;

    case ';':    /* The -; switch means don't process the rest of the line. */
      return fTrue;

    case '@':    /* The -@ switch is just a system flag indicator no-op. */
      break;

    case '.':                /* "-." is usually used to exit the -Q loop. */
      Terminate(tcForce);

#ifdef EXPRESS
    case '~':
      if (ch1 == '0') {
        SwitchF(us.fExpOff);
        break;
      }
      i = 1 + (ch1 == 'M');
      if (argc <= i) {
        ErrorArgc("~");
        return fFalse;
      }
      if (ch1 == 'a')
        us.szExpAsplist = SzPersist(argv[1]);
      else if (ch1 == 'E')
        us.szExpEph = SzPersist(argv[1]);
      else if (ch1 == 'd')
        us.szExpDay = SzPersist(argv[1]);
      else if (ch1 == 't')
        us.szExpTra = SzPersist(argv[1]);
      else if (ch1 == 'j')
        us.szExpInf = SzPersist(argv[1]);
      else if (ch1 == 'O')
        us.szExpObj = SzPersist(argv[1]);
      else if (ch1 == 'C')
        us.szExpHou = SzPersist(argv[1]);
      else if (ch1 == 'A')
        us.szExpAsp = SzPersist(argv[1]);
      else if (ch1 == 'k' && ch2 == 'O')
        us.szExpColObj = SzPersist(argv[1]);
      else if (ch1 == 'k' && ch2 == 'A')
        us.szExpColAsp = SzPersist(argv[1]);
      else if (ch1 == 'M') {
        j = atoi(argv[1]);
        if (!FBetween(j, 0, iLetterZ)) {
          ErrorValN("~M", j);
          return fFalse;
        }
        rgszExpMacro[j] = SzPersist(argv[2]);
      } else
        FParseExpression(argv[1]);
      argc -= i; argv += i;
      break;
#endif

    default:
      ErrorSwitch(argv[0]);
      return fFalse;
    }
    argc--; argv++;
  }
  return fTrue;
}


/*
******************************************************************************
** Main Program.
******************************************************************************
*/

/* Initialize program variables and tables that aren't done so at compile */
/* time. Called once when the program starts from main() or WinMain().    */

void InitProgram()
{
#ifdef WIN
  char sz[cchSzMax], *pch;
#endif
  int i;

  is.S = stdout;
  ClearB((pbyte)szStarCustom, sizeof(szStarCustom));
  for (i = 0; i < objMax; i++)
    szObjDisp[i] = szObjName[i];
  for (i = 1; i <= cAspect2; i++) {
    szAspectDisp[i]       = szAspectName[i];
    szAspectAbbrevDisp[i] = szAspectAbbrev[i];
    szAspectGlyphDisp[i]  = szAspectGlyph[i];
  }
#ifdef SWISS
  ClearB((pbyte)rgTypSwiss, sizeof(rgTypSwiss));
  ClearB((pbyte)rgPntSwiss, sizeof(rgPntSwiss));
  ClearB((pbyte)rgFlgSwiss, sizeof(rgFlgSwiss));
#endif
#ifdef GRAPH
  for (i = 0; i < cColor; i++)
    rgbbmp[i] = rgbbmpDef[i];
  for (i = 0; i < objMax+5; i++) {
    szDrawObject[i]  = szDrawObjectDef[i];
    szDrawObject2[i] = szDrawObjectDef2[i];
  }
  for (i = 1; i <= cAspect2; i++) {
    szDrawAspect[i]  = szDrawAspectDef[i];
    szDrawAspect2[i] = szDrawAspectDef2[i];
  }
#endif
#ifdef WIN
  GetModuleFileName(wi.hinst, sz, cchSzMax);
  for (pch = sz; *pch; pch++)
    ;
  if (pch - sz > 4 && NCompareSz(pch - 4, ".scr") == 0)
    wi.fSaverExt = fTrue;
#endif
}


/* Program is about to exit, so free memory that was allocated. */

void FinalizeProgram()
{
  char sz[cchSzDef];

  if (grid != NULL)
    DeallocateP(grid);
#ifdef ATLAS
  if (is.rgae != NULL)
    DeallocateP(is.rgae);
  if (is.rgzc != NULL)
    DeallocateP(is.rgzc);
  if (is.rgrun != NULL)
    DeallocateP(is.rgrun);
  if (is.rgrue != NULL)
    DeallocateP(is.rgrue);
#endif
#ifdef EXPRESS
  if (is.rgsTrieFun != NULL)
    DeallocateP(is.rgsTrieFun);
#endif
#ifdef GRAPH
  if (gi.bm != NULL)
    DeallocateP(gi.bm);
  if (gi.rgspace != NULL)
    DeallocateP(gi.rgspace);
#ifdef SWISS
  if (gi.rges != NULL)
    DeallocateP(gi.rges);
#endif
#endif // GRAPH
  if (is.cAlloc != 0) {
    sprintf(sz, "Number of memory allocations not freed before exiting: %d",
      is.cAlloc);
    PrintWarning(sz);
  }
}


#ifndef WIN
/* The main program, the starting point for Astrolog, follows. This routine */
/* basically consists of a loop, inside which we read a command line, and   */
/* go process it, before actually calling a routine to display astrology.   */

#ifdef SWITCHES
int main(int argc, char **argv)
{
#else
int main()
{
  int argc;
  char **argv;
#endif
  char szCommandLine[cchSzMax], *rgsz[MAXSWITCHES];
#ifdef BETA
  char szBeta[cchSzMax];
#endif

  /* Read in info from the astrolog.as file. */
  InitProgram();
#ifdef SWITCHES
  is.szProgName = argv[0];
#endif
  FProcessSwitchFile(DEFAULT_INFOFILE, NULL);
#ifdef BETA
  sprintf(szBeta, "This is a beta version of %s %s! "
    "That means changes are still being made and testing is not complete. "
    "If this is being run after %s %d, %d, "
    "it should be replaced with the finished release.\n\n",
    szAppName, szVersionCore, szMonth[ciSave.mon], ciSave.day, ciSave.yea);
  FieldWord(szBeta);
#endif

LBegin:
  is.fSzPersist = !us.fNoSwitches;
  if (us.fNoSwitches) {                             /* Go prompt for    */
    argc = NPromptSwitches(szCommandLine, rgsz);    /* switches if we   */
    argv = rgsz;                                    /* don't have them. */
    is.fSzPersist = fFalse;
  }
  is.szProgName = argv[0];
  if (FProcessSwitches(argc, argv)) {
    if (!us.fNoSwitches && us.fLoopInit) {
      us.fNoSwitches = fTrue;
      goto LBegin;
    }
    Action();
  }
  if (us.fLoop || us.fNoQuit) { /* If -Q in effect loop back and get switch */
    PrintL2();                  /* info for another chart to display.       */
    InitVariables();
    us.fLoop = us.fNoSwitches = fTrue;
    goto LBegin;
  }
  Terminate(tcOK);    /* The only standard place to exit Astrolog is here. */
  return 0;
}
#endif /* WIN */

/* astrolog.cpp */
