/*
** Astrolog (Version 7.30) File: wdriver.cpp
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


#ifdef WIN
/*
******************************************************************************
** Windows Command Processing.
******************************************************************************
*/

// Process one command line switch passed to the program dealing with the
// Windows features. This is just like the processing of each switch in the
// main program, however here each switch has been prefixed with an 'W'.

int NProcessSwitchesW(int argc, char **argv, int pos,
  flag fOr, flag fAnd, flag fNot)
{
  int darg = 0, xo, yo, i;
  char sz[cchSzMax], ch1, ch2;
  RECT rc;

  ch1 = argv[0][pos+1];
  ch2 = ch1 != chNull ? argv[0][pos+2] : chNull;
  switch (argv[0][pos]) {
  case chNull:
    if (FErrorArgc("W", argc, 1))
      return tcError;
    i = NFromSz(argv[1]);
    PostMessage(wi.hwnd, WM_COMMAND, i, 0L);
    darg++;
    break;

  case 'N':
    if (FErrorArgc("WN", argc, 1))
      return tcError;
    i = NFromSz(argv[1]);
    if (FErrorValN("WN", !FValidTimer(i), i, 0))
      return tcError;
    wi.nTimerDelay = i;
    darg++;
    break;

  case 'M':
    if (FErrorArgc("WM", argc, 2))
      return tcError;
    i = NFromSz(argv[1]);
    if (FErrorValN("WM", !FValidMacro(i), i, 1))
      return tcError;
    i--;
    sprintf(sz, "%s\t%sF%d", argv[2], i < 12 ? "" : (i < 24 ? "Shift+" :
      (i < 36 ? "Ctrl+" : "Alt+")), i % 12 + 1);
    ModifyMenu(wi.hmenu, (WORD)(cmdMacro01 + i), MF_BYCOMMAND | MF_STRING,
      (WORD)(cmdMacro01 + i), sz);
    darg += 2;
    break;

  case 'n':
    SwitchF(wi.fNoUpdate);
    break;

  case 'h':
    SwitchF(wi.fHourglass);
    break;

  case 't':
    SwitchF(wi.fNoPopup);
    break;

  case 'w':
    if (FErrorArgc("Ww", argc, 2))
      return tcError;
    GetWindowRect(wi.hwnd, &rc);
    xo = NFromSz(argv[1]);
    yo = NFromSz(argv[2]);
    SetWindowPos(wi.hwnd, HWND_NOTOPMOST, xo, yo,
      rc.right - rc.left, rc.bottom - rc.top, 0);
    darg += 2;
    break;

  case 'B':
    if (FErrorArgc("WB", argc, 2))
      return tcError;
    wi.xScroll = max(0, min(NFromSz(argv[1]), nScrollDiv));
    wi.yScroll = max(0, min(NFromSz(argv[2]), nScrollDiv));
    SetScrollPos(wi.hwnd, SB_HORZ, wi.xScroll, fTrue);
    SetScrollPos(wi.hwnd, SB_VERT, wi.yScroll, fTrue);
    darg += 2;
    break;

  case 'T':
    if (FErrorArgc("WT", argc, 1))
      return tcError;
    if (*argv[1])
      sprintf(sz, "%s %s: %s", szAppName, szVersionCore, argv[1]);
    else
      sprintf(sz, "%s %s", szAppName, szVersionCore);
    SetWindowText(wi.hwnd, sz);
    darg++;
    break;

  case 'o':
    if (ch1 == '0' || ch2 == '0') {
      SwitchF(wi.fAutoSaveNum);
      wi.nAutoSaveNum = 0;
    }
    if (ch1 == '3' || ch2 == '3')
      SwitchF(wi.fAutoSaveWire);
    SwitchF(wi.fAutoSave);
    break;

  case 'S':
    if (ch1 == 'g')
      FCreateProgramGroup(fFalse);
    else if (ch1 == 'G')
      FCreateProgramGroup(fTrue);
    else if (ch1 == 'd')
      FCreateDesktopIcon();
    else if (ch1 == 'x')
      FRegisterExtensions();
    else if (ch1 == 'u')
      FUnregisterExtensions();
    break;

  case 'Z':
    SwitchF(wi.fSaverRun);
    break;

  default:
    ErrorSwitch(argv[0]);
    return tcError;
  }
  // 'darg' contains the value to be added to argc when we return.
  return darg;
}


// Launch an external application and have it open the specified file.

void BootExternal(CONST char *szApp, CONST char *szFile)
{
  char szCmd[cchSzMax], szPath[cchSzMax];

  if (us.fNoRead) {
    PrintWarning("File reading is disabled.");
    return;
  }
  if (FileOpen(szFile, 2, szPath) != NULL) {
    if (szApp != NULL) {
      sprintf(szCmd, "%s %s", szApp, szPath);
      WinExec(szCmd, SW_SHOW);
    } else
      ShellExecute(NULL, NULL, szPath, NULL, NULL, SW_SHOW);
  } else {
    sprintf(szCmd, "File '%s' not found!", szFile);
    PrintWarning(szCmd);
  }
}


// Given a relationship chart mode, return the menu command that sets it.

int CmdFromRc(int rc)
{
  switch (rc) {
  case rcTriWheel: case rcQuadWheel: case rcQuinWheel: case rcHexaWheel:
  // Fall through
  case rcDual:       return cmdRelComparison;
  case rcSynastry:   return cmdRelSynastry;
  case rcComposite:  return cmdRelComposite;
  case rcMidpoint:   return cmdRelMidpoint;
  case rcDifference: return cmdRelDate;
  case rcBiorhythm:  return cmdRelBiorhythm;
  case rcTransit:    return cmdRelTransit;
  case rcProgress:   return cmdRelProgressed;
  default:           return cmdRelNo;
  }
}


// Change relationship chart modes. Given a new mode, we put a bullet by its
// menu command, and erase the bullet by the menu command for the old mode.

void SetRel(int rc)
{
  CI ciT;

  if (us.nRel == rcMidpoint) {  // Restore chart when leave midpoint mode.
    ciT = ciMain;
    ciCore = ciMain = ciSave;
    ciSave = ciT;
  }
  if (rc == rcMidpoint)         // Remember chart so can restore it later.
    ciSave = ciMain;
  us.nRel = rc;
  RadioMenu(cmdRelBiorhythm, cmdRelTransit, CmdFromRc(rc));
  wi.fCast = fTrue;
}


// Display and process the specified right click context popup menu.

void DoPopup(int imenu, HWND hwnd, LPARAM lParam)
{
  HMENU hmenu;
  POINT pt;
  int x, y;

  x = WLo(lParam);
  y = WHi(lParam);
  pt.x = x; pt.y = y;
  ClientToScreen(hwnd, &pt);
  hmenu = LoadMenu(NULL, MAKEINTRESOURCE(imenu));
  hmenu = GetSubMenu(hmenu, 0);

  switch (imenu) {
  case menuV:
    CheckPopup(cmdChartModify,     gi.nMode == gHouse);
    CheckPopup(cmdHouseSet3D,      us.fHouse3D);
    CheckPopup(cmdHouseSetVedic,   us.fVedic);
    CheckPopup(cmdGraphicsEquator, !gs.fEquator);
    CheckPopup(cmdGraphicsAspect,  gs.fLabelAsp);
    CheckPopup(cmdGraphicsModify,  gs.fAlt);
    CheckPopup(cmdGraphicsHouse,   !gs.fHouseExtra);
    CheckPopup(cmdGraphicsSidebar, !us.fVelocity);
    break;
  case menuA:
    CheckPopup(cmdParallel,       us.fParallel);
    CheckPopup(cmdApplying,       us.fAppSep);
    CheckPopup(cmdGraphicsAspect, gs.fLabelAsp);
    CheckPopup(cmdGraphicsModify, gs.fAlt);
    break;
  case menuZ:
    CheckPopup(cmdChartModify,     us.fPrimeVert);
    CheckPopup(cmdConstellation,   gs.fConstel);
    CheckPopup(cmdHouseSet3D,      !us.fHouse3D);
    CheckPopup(cmdGraphicsAllStar, gs.fAllStar);
    CheckPopup(cmdHouseSetVedic,   us.fVedic);
    CheckPopup(cmdGraphicsHouse,   gs.fHouseExtra);
    CheckPopup(cmdGraphicsEquator, gs.fEquator);
    CheckPopup(cmdGraphicsLabel,   gs.fLabel);
    CheckPopup(cmdGraphicsModify,  !gs.fAlt);
    CheckPopup(cmdGraphicsAxis,    gs.fEcliptic);
    break;
  case menuS:
    CheckPopup(cmdHouseSet3D,      us.fHouse3D);
    CheckPopup(cmdGraphicsAllStar, gs.fAllStar);
    CheckPopup(cmdGraphicsHouse,   !gs.fHouseExtra);
    CheckPopup(cmdGraphicsEquator, !gs.fEquator);
    CheckPopup(cmdGraphicsAspect,  gs.fLabelAsp);
    CheckPopup(cmdGraphicsLabel,   gs.fLabel);
    CheckPopup(cmdGraphicsModify,  !gs.fAlt);
    break;
  case menuH:
    CheckPopup(cmdGraphicsEquator, !gs.fEquator);
    CheckPopup(cmdGraphicsModify,  gs.fAlt);
    CheckPopup(cmdGraphicsAspect,  gs.fLabelAsp);
    CheckPopup(cmdHouseSetVedic,   us.fVedic);
    CheckPopup(cmdGraphicsHouse,   !gs.fHouseExtra);
    CheckPopup(cmdGraphicsSidebar, !us.fVelocity);
    break;
  case menuK:
    CheckPopup(cmdChartModify,    us.fCalendarYear);
    CheckPopup(cmdGraphicsAspect, gs.fLabelAsp);
    CheckPopup(cmdGraphicsModify, gs.fAlt);
    CheckPopup(cmdGraphicsText,   !gs.fText);
    CheckPopup(cmdGraphicsLabel,  gs.fLabel);
    break;
  case menuJ:
    CheckPopup(cmdGraphicsModify, !gs.fAlt);
    CheckPopup(cmdHouseSetVedic,  us.fVedic);
    CheckPopup(cmdGraphicsHouse,  gs.fHouseExtra);
    break;
  case menu7:
    CheckPopup(cmdChartModify,     us.nEphemYears != 0);
    CheckPopup(cmdSecond,          us.fSeconds);
    CheckPopup(cmdGraphicsModify,  gs.fAlt);
    CheckPopup(cmdGraphicsLabel,   gs.fLabel);
    CheckPopup(cmdGraphicsEquator, !gs.fEquator);
    break;
  case menuL:
    CheckPopup(cmdChartModify,    us.fLatitudeCross);
    CheckPopup(cmdGraphicsModify, gs.fAlt);
    break;
  case menuE:
    CheckPopup(cmdChartModify,     us.nEphemYears != 0);
    CheckPopup(cmdSecond,          us.fSeconds);
    CheckPopup(cmdParallel,        us.fParallel);
    CheckPopup(cmdGraphicsModify,  gs.fAlt);
    CheckPopup(cmdGraphicsLabel,   gs.fLabel);
    CheckPopup(cmdGraphicsEquator, !gs.fEquator);
    break;
  case menuZd:
    CheckPopup(cmdSecond,         us.fSeconds);
    CheckPopup(cmdGraphicsBmp,    gi.fBmp);
    CheckPopup(cmdGraphicsLabel,  gs.fLabel);
    CheckPopup(cmdGraphicsCity,   gs.fLabelCity);
    CheckPopup(cmdGraphicsModify, gs.fAlt);
    break;
  case menuN:
    CheckPopup(cmdHouseSetVedic,   !us.fVedic);
    CheckPopup(cmdGraphicsEquator, gs.fEquator);
    CheckPopup(cmdGraphicsHouse,   gs.fHouseExtra);
    CheckPopup(cmdGraphicsCity,    !gs.fLabelCity);
    CheckPopup(cmdGraphicsAspect,  gs.fLabelAsp);
    CheckPopup(cmdGraphicsModify,  !gs.fAlt);
    break;
  case menuD:
    CheckPopup(cmdChartModify,    us.fGraphAll);
    CheckPopup(cmdGraphicsModify, !gs.fAlt);
    break;
  case menuY:
    CheckPopup(cmdGraphicsModify, gs.fAlt);
    break;
  case menuXX:
    CheckPopup(cmdChartModify,     gs.fSouth);
    CheckPopup(cmdGraphicsModify,  gs.fAlt);
    CheckPopup(cmdGraphicsSidebar, !us.fVelocity);
    CheckPopup(cmdHouseSetVedic,   !us.fVedic);
    CheckPopup(cmdGraphicsHouse,   !gs.fHouseExtra);
    CheckPopup(cmdHouseSet3D,      !us.fHouse3D);
    CheckPopup(cmdGraphicsAspect,  gs.fLabelAsp);
    CheckPopup(cmdGraphicsEquator, gs.fEquator);
    CheckPopup(cmdConstellation,   gs.fConstel);
    CheckPopup(cmdGraphicsAllStar, gs.fAllStar);
    CheckPopup(cmdGraphicsAxis,    gs.fEcliptic);
    break;
  case menuXG:
    CheckPopup(cmdGraphicsModify,  !gs.fAlt);
    CheckPopup(cmdConstellation,   gs.fConstel);
    CheckPopup(cmdGraphicsHouse,   gs.fHouseExtra);
    CheckPopup(cmdGraphicsEquator, gs.fEquator);
    CheckPopup(cmdGraphicsAllStar, gs.fAllStar);
    CheckPopup(cmdGraphicsAxis,    gs.fEcliptic);
    CheckPopup(cmdGraphicsCity,    gs.fLabelCity);
    CheckPopup(cmdGraphicsAspect,  gs.fLabelAsp);
    break;
  case menuXZ:
    CheckPopup(cmdConstellation,   gs.fConstel);
    CheckPopup(cmdHouseSetVedic,   us.fVedic);
    CheckPopup(cmdGraphicsHouse,   gs.fHouseExtra);
    CheckPopup(cmdHouseSet3D,      !us.fHouse3D);
    CheckPopup(cmdGraphicsEquator, gs.fEquator);
    CheckPopup(cmdGraphicsAspect,  !gs.fLabelAsp);
    CheckPopup(cmdChartModify,     !us.fLatitudeCross);
    CheckPopup(cmdGraphicsLabel,   gs.fLabel);
    CheckPopup(cmdGraphicsModify,  !gs.fAlt);
    CheckPopup(cmdGraphicsAllStar, gs.fAllStar);
    CheckPopup(cmdGraphicsCity,    gs.fLabelCity);
    CheckPopup(cmdGraphicsAxis,    gs.fEcliptic);
    break;
  }

  TrackPopupMenu(hmenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
  DestroyMenu(hmenu);
}


// The main program, the starting point of Astrolog for Windows, follows. This
// is like the "main" function in standard C. The program initializes here,
// then spins in a tight message processing loop until it terminates.

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  LPSTR lpszCmdLine, int nCmdShow)
{
  MSG msg;
  WNDCLASS wndclass;
  char ch;
#ifdef BETA
  char sz[cchSzMax];
#endif

  // Set up the window class shared by all instances of Astrolog.

  wi.hinst = hInstance;
  if (!hPrevInstance) {
    ClearB((pbyte)&wndclass, sizeof(WNDCLASS));
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNWINDOW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = wi.hinst;
    wndclass.hIcon = LoadIcon(wi.hinst, MAKEINTRESOURCE(icon));
    wndclass.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wndclass.lpszMenuName = MAKEINTRESOURCE(menu);
    wndclass.lpszClassName = szAppName;
    if (!RegisterClass(&wndclass)) {
      PrintError("The window class could not be registered.");
      return -1L;
    }
  }

  // Create the actual window to be used and drawn on by this instance.

  wi.hmenu = LoadMenu(wi.hinst, MAKEINTRESOURCE(menu));
#ifndef PS
  DeleteMenu(wi.hmenu, cmdSavePS, MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdCopyPS, MF_BYCOMMAND);
#endif
#ifndef META
  DeleteMenu(wi.hmenu, cmdSavePicture, MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdCopyPicture, MF_BYCOMMAND);
#endif
#ifndef WIRE
  DeleteMenu(wi.hmenu, cmdSaveWire, MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdCopyWire, MF_BYCOMMAND);
#endif
#ifndef INTERPRET
  DeleteMenu(wi.hmenu, cmdInterpret,   MF_BYCOMMAND);
  DeleteMenu(wi.hmenu, cmdHelpMeaning, MF_BYCOMMAND);
#endif
#ifndef ARABIC
  DeleteMenu(wi.hmenu, cmdChartArabic, MF_BYCOMMAND);
#endif
#ifndef CONSTEL
  DeleteMenu(wi.hmenu, cmdConstellation, MF_BYCOMMAND);
#endif
#ifndef BIORHYTHM
  DeleteMenu(wi.hmenu, cmdRelBiorhythm, MF_BYCOMMAND);
#endif
#ifndef ATLAS
  DeleteMenu(wi.hmenu, cmdGraphicsCity, MF_BYCOMMAND);
#endif
  wi.hwndMain = CreateWindow(
    szAppName,
    szAppNameCore " " szVersionCore,
    WS_CAPTION |
    WS_SYSMENU |
    WS_MINIMIZEBOX |
    WS_MAXIMIZEBOX |
    WS_THICKFRAME |
    WS_VSCROLL |
    WS_HSCROLL |
    WS_CLIPCHILDREN |
    WS_OVERLAPPED,
    CW_USEDEFAULT, CW_USEDEFAULT,
    CW_USEDEFAULT, CW_USEDEFAULT,
    (HWND)NULL,
    wi.hmenu,
    wi.hinst,
    (LPSTR)NULL);
  if (wi.hwndMain == (HWND)NULL) {
    PrintError("The window could not be created.");
    return -1L;
  }

  // Set up some globals that can't be initialized at compile time.

  CoInitialize(NULL);
  InitProgram();
  ofn.hwndOwner = wi.hwndMain;
  ofn.lpstrFile = szFileName;
  ofn.lpstrFileTitle = szFileTitle;
  prd.hwndOwner = wi.hwndMain;
  wi.haccel = LoadAccelerators(wi.hinst, MAKEINTRESOURCE(accelerator));
  wi.kiPen = gi.kiLite;

  // Check for being used as a Windows screen saver.

  if (lpszCmdLine[0] == '/') {
    ch = lpszCmdLine[1];
    if (ch == 's') {
      wi.fSaverRun = fTrue;
      inv(us.fSidereal);     // Because "/s" will also toggle -s switch.
    } else if (ch == 'c' || ch == 'a')
      wi.fSaverCfg = fTrue;
    else if (ch == 'p')
      return -1;
    if (wi.fSaverRun || wi.fSaverCfg) {
      // By default animate forward a minute at a time, 10 times per second.
      gs.nAnim = 2;
      wi.fHourglass = fFalse;
      wi.nTimerDelay = 100;
    }
  }

  // Process the astrolog.as file and the Windows command line.

  FProcessSwitchFile(DEFAULT_INFOFILE, NULL);
  FProcessCommandLine(lpszCmdLine);
  ciTran = ciHexa = ciFive = ciFour = ciThre = ciTwin = ciCore;

  // Actually bring up and display the window for the first time.

  ResizeWindowToChart();
  ShowWindow(wi.hwndMain, nCmdShow);
  RedoMenu();
  wi.lTimer = SetTimer(wi.hwnd, 1, wi.nTimerDelay, NULL);
  if (wi.fSaverRun) {
    // By default have the image in the middle of the full screen window.
    ShowCursor(fFalse);
    wi.xScroll = wi.yScroll = nScrollDiv >> 1;
    NWmCommand(cmdSizeWindowFull);
  }
#ifdef BETA
  sprintf(sz, "This is a beta version of %s %s! "
    "That means changes are still being made and testing is not complete. "
    "If this is being run after %s %d, %d, "
    "it should be replaced with the finished release.",
    szAppName, szVersionCore, szMonth[ciSave.mon], ciSave.day, ciSave.yea);
  PrintWarning(sz);
#endif

  // Process window messages until the program is told to terminate.

  while (GetMessage(&msg, (HWND)NULL, 0, 0)) {
    if (!TranslateAccelerator(wi.hwndMain, wi.haccel, &msg))
      TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  wi.hwndMain = NULL;

  // Cleanup and exit Astrolog for Windows.

  FinalizeProgram(fFalse);
  if (wi.hMutex != NULL)
    CloseHandle(wi.hMutex);
  SelectObject(wi.hdcBack, wi.hbmpPrev);
  if (wi.hbmpBack != NULL)
    DeleteObject(wi.hbmpBack);
  DeleteDC(wi.hdcBack);
  UnregisterClass(szAppName, wi.hinst);
  return (int)msg.wParam;
}


// This is the main message processor for the Astrolog window. Given a user
// input or other message, do the appropriate action and updates.

LRESULT API WndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
  HDC hdc;
  HPEN hpen, hpenOld;
  HBRUSH hbr;
  RECT rc;
  int iParam, x, y;

  wi.hwnd = hwnd;
  switch (wMsg) {

    // A command, most likely a menu option, was given.

    case WM_COMMAND:
      if (wi.fSaverRun)
        goto LClose;
      switch (NWmCommand((WORD)wParam)) {
      case 1:
        goto LClose;
      case -1:
        return DefWindowProc(hwnd, wMsg, wParam, lParam);
      default:
        iParam = (int)wParam & 0xFFFF;
        wi.fSkipSystem = (iParam == cmdMacro40/*Alt+F4*/);
        ProcessState();
      }
      break;

    // A special system command was given. Suppress its action if it was
    // invoked by a hotkey which ran a macro, to avoid both effects happening.

    case WM_SYSCOMMAND:
      iParam = (int)wParam & 0xFFF0;
      if (wi.fSkipSystem && (iParam == SC_KEYMENU || iParam == SC_CLOSE)) {
        wi.fSkipSystem = fFalse;
        break;
      }
      return DefWindowProc(hwnd, wMsg, wParam, lParam);

    // When a part of a window is uncovered, Windows quickly blanks it out,
    // usually with white, before having the app redraw that section. Most
    // apps don't do anything here, however Astrolog can quickly draw with
    // a more appropriate color since we know our chart's background color.

    case WM_ERASEBKGND:
      if (!(wi.fNoUpdate & 1) && !wi.fBuffer) {
        GetClipBox((HDC)wParam, &rc);
        hbr = CreateSolidBrush(rgbbmp[gi.kiOff]);
        FillRect((HDC)wParam, &rc, hbr);
        DeleteObject(hbr);
      }
      return fTrue;

    // The window was just created. Setup the scrollbars.

    case WM_CREATE:
      SetScrollRange(hwnd, SB_HORZ, 0, nScrollDiv, fFalse);
      SetScrollRange(hwnd, SB_VERT, 0, nScrollDiv, fFalse);
      break;

    // The window has been resized. Change the chart size if need be.

    case WM_SIZE:
      wi.xClient = LOWORD(lParam);
      wi.yClient = HIWORD(lParam);
      if (wi.fWindowChart || wi.fChartWindow) {
        gs.xWin = wi.xClient; gs.yWin = wi.yClient;
      }
      break;

    // All or part of the window needs to be redrawn. Go do so.

    case WM_PAINT:
      if (!(wi.fNoUpdate & 1))
        FRedraw();
      break;

    // The mouse has been left clicked or dragged over the window.

    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
      x = WLo(lParam);
      y = WHi(lParam);
      if (wMsg == WM_MOUSEMOVE) {
        if (wi.fSaverRun) {
          if (!wi.fMoved) {
            wi.fMoved = fTrue;
            wi.xMouse = WLo(lParam); wi.yMouse = WHi(lParam);
            break;
          }
          // Exit screensaver if mouse moves more than 4 pixels from start.
          if (NAbs(wi.xMouse - WLo(lParam)) +
              NAbs(wi.yMouse - WHi(lParam)) > 4)
            goto LClose;
          break;
        }

        // Dragging with right mouse down rotates and tilts globes.
        if ((wParam & MK_RBUTTON) != 0 && us.fGraphics && (fMap ||
          gi.nMode == gLocal || gi.nMode == gSphere || gi.nMode == gGlobe ||
          gi.nMode == gPolar || gi.nMode == gTelescope)) {
          gs.rRot += (real)(x-WLo(wi.lParamRC)) * rDegHalf / (real)gs.xWin *
            (gi.nMode == gLocal || gi.nMode == gTelescope ? -gi.zViewRatio :
            1.0);
          gs.rTilt += (real)(y-WHi(wi.lParamRC)) * rDegHalf / (real)gs.yWin *
            (gi.nMode == gLocal || gi.nMode == gTelescope ? gi.zViewRatio :
            (gi.nMode == gGlobe ? -1.0 : 1.0));
          while (gs.rRot >= rDegMax)
            gs.rRot -= rDegMax;
          while (gs.rRot < 0.0)
            gs.rRot += rDegMax;
          while (gs.rTilt > rDegQuad)
            gs.rTilt = rDegQuad;
          while (gs.rTilt < -rDegQuad)
            gs.rTilt = -rDegQuad;
          if (gi.nMode == gLocal || gi.nMode == gTelescope)
            gs.objTrack = -1;
          wi.lParamRC = lParam;
          wi.fRedraw = fTrue;
          wi.fMoved = fTrue;
          ProcessState();
          break;
        }

        // Treat dragging with left mouse down as a Shift+left click.
        if ((wParam & MK_LBUTTON) == 0 ||
          (wParam & MK_SHIFT) || (wParam & MK_CONTROL))
          break;
        wParam = MK_SHIFT;
      }
      if (wi.fSaverRun)
        goto LClose;

      // Alt+click on a world map chart means relocate the chart there.
      if (wMsg == WM_LBUTTONDOWN && GetKeyState(VK_MENU) < 0) {
        if (fMap && !gs.fConstel && !gs.fMollewide) {
          Lon = rDegHalf -
            Mod((real)(x-gi.xOffset) / (real)gs.xWin*rDegMax - gs.rRot);
          if (Lon < -rDegHalf)
            Lon = -rDegHalf;
          else if (Lon > rDegHalf)
            Lon = rDegHalf;
          Lat = rDegQuad-(real)(y-gi.yOffset)/(real)gs.yWin*rDegHalf;
          if (Lat < -rDegQuad)
            Lat = -rDegQuad;
          else if (Lat > rDegQuad)
            Lat = rDegQuad;
          wi.xMouse = -1;
          ciCore = ciMain;
          wi.fCast = fTrue;
          ProcessState();
        }
        break;
      }
      hdc = GetDC(hwnd);
      hpen = (HPEN)CreatePen(PS_SOLID, !gs.fThick ? 0 : 2,
        (COLORREF)rgbbmp[wi.kiPen]);
      hpenOld = (HPEN)SelectObject(hdc, hpen);

      // Ctrl+click means draw a rectangle. Ctrl+Shift+click does ellipse.
      if (wParam & MK_CONTROL) {
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        if (wParam & MK_SHIFT)
          Ellipse(hdc, wi.xMouse, wi.yMouse, x, y);
        else
          Rectangle(hdc, wi.xMouse, wi.yMouse, x, y);

      // Shift+click means draw a line from the last to current position.
      } else if (wParam & MK_SHIFT) {
        if (wi.xMouse >= 0) {
          MoveTo(hdc, wi.xMouse, wi.yMouse);
          LineTo(hdc, x, y);
          if (wMsg == WM_MOUSEMOVE) {
            wi.xMouse = x; wi.yMouse = y;
          }
        }

      // A simple click means set a pixel and remember that location.
      } else {
        SetPixel(hdc, x, y, (COLORREF)rgbbmp[wi.kiPen]);
        wi.xMouse = x; wi.yMouse = y;
      }
      SelectObject(hdc, hpenOld);
      DeleteObject(hpen);
      ReleaseDC(hwnd, hdc);
      break;

    // The mouse has been right clicked on the window.

    case WM_RBUTTONDOWN:
      if (wi.fSaverRun)
        goto LClose;
      wi.fMoved = fFalse;
      if (us.fGraphics) {
        if (fMap || gi.nMode == gLocal || gi.nMode == gSphere ||
          gi.nMode == gGlobe || gi.nMode == gPolar || gi.nMode == gTelescope)
          wi.lParamRC = lParam;
        // Most graphics modes show popup right when mouse button clicked.
        if (gi.nMode == gWheel || gi.nMode == gHouse)
          DoPopup(menuV, hwnd, lParam);
        else if (gi.nMode == gGrid)
          DoPopup(menuA, hwnd, lParam);
        else if (gi.nMode == gHorizon)
          DoPopup(menuZ, hwnd, lParam);
        else if (gi.nMode == gOrbit)
          DoPopup(menuS, hwnd, lParam);
        else if (gi.nMode == gSector)
          DoPopup(menuH, hwnd, lParam);
        else if (gi.nMode == gCalendar)
          DoPopup(menuK, hwnd, lParam);
        else if (gi.nMode == gDisposit)
          DoPopup(menuJ, hwnd, lParam);
        else if (gi.nMode == gEsoteric)
          DoPopup(menu7, hwnd, lParam);
        else if (gi.nMode == gEphemeris)
          DoPopup(menuE, hwnd, lParam);
        else if (gi.nMode == gRising)
          DoPopup(menuZd, hwnd, lParam);
        else if (gi.nMode == gTraTraGra || gi.nMode == gTraNatGra)
          DoPopup(menuD, hwnd, lParam);
        else if (gi.nMode == gBiorhythm)
          DoPopup(menuY, hwnd, lParam);
      }
      break;

    // The right mouse button was clicked, but has now been lifted.

    case WM_RBUTTONUP:
      if (!wi.fMoved && us.fGraphics) {
        // Some graphics modes can't show popup until mouse button lifted,
        // because can click and drag right mouse in window to rotate chart.
        if (gi.nMode == gAstroGraph)
          DoPopup(menuL, hwnd, lParam);
        else if (gi.nMode == gLocal)
          DoPopup(menuN, hwnd, lParam);
        else if (gi.nMode == gTelescope)
          DoPopup(menuXZ, hwnd, lParam);
        else if (gi.nMode == gSphere)
          DoPopup(menuXX, hwnd, lParam);
        else if (gi.nMode == gWorldMap || gi.nMode == gGlobe ||
          gi.nMode == gPolar)
          DoPopup(menuXG, hwnd, lParam);
      }
      break;

    // A timer message is received at a defined regular interval.

    case WM_TIMER:
      if (gs.nAnim < 1 || wi.fPause)
        break;
      Animate(gs.nAnim, wi.nDir);  // Update chart if animation mode on.
      wi.fRedraw = fTrue;
      ProcessState();
      break;

    // Define the minimum and maximum size the window may be resized to.

    case WM_GETMINMAXINFO:
      ((MINMAXINFO FAR *)lParam)->ptMinTrackSize.x = BITMAPX1;
      ((MINMAXINFO FAR *)lParam)->ptMinTrackSize.y = BITMAPY1;
      ((MINMAXINFO FAR *)lParam)->ptMaxTrackSize.x = BITMAPX;
      ((MINMAXINFO FAR *)lParam)->ptMaxTrackSize.y = BITMAPY;
      break;

    // The horizontal scrollbar was clicked on or used in some way.

    case WM_HSCROLL:
      if (wi.fSaverRun)
        goto LClose;
      x = wi.xScroll;
      switch (LOWORD(wParam)) {
      case SB_LEFT:
        wi.xScroll = 0;
        break;
      case SB_RIGHT:
        wi.xScroll = nScrollDiv;
        break;
      case SB_LINEUP:
        wi.xScroll--;
        break;
      case SB_LINEDOWN:
        wi.xScroll++;
        break;
      case SB_PAGEUP:
        wi.xScroll -= nScrollPage;
        break;
      case SB_PAGEDOWN:
        wi.xScroll += nScrollPage;
        break;
      case SB_THUMBPOSITION:
        wi.xScroll = HIWORD(wParam);
        break;
      default:
        return fFalse;
      }
      wi.xScroll = max(0, min(wi.xScroll, nScrollDiv));
      if (wi.xScroll == x)
        break;
      SetScrollPos(hwnd, SB_HORZ, wi.xScroll, fTrue);
      wi.fRedraw = fTrue;
      ProcessState();
      break;

    // The vertical scrollbar was clicked on or used in some way.

    case WM_VSCROLL:
      if (wi.fSaverRun)
        goto LClose;
      y = wi.yScroll;
      switch(LOWORD(wParam)) {
      case SB_TOP:
        wi.yScroll = 0;
        break;
      case SB_BOTTOM:
        wi.yScroll = nScrollDiv;
        break;
      case SB_LINEUP:
        wi.yScroll--;
        break;
      case SB_LINEDOWN:
        wi.yScroll++;
        break;
      case SB_PAGEUP:
        wi.yScroll -= nScrollPage;
        break;
      case SB_PAGEDOWN:
        wi.yScroll += nScrollPage;
        break;
      case SB_THUMBPOSITION:
        wi.yScroll = HIWORD(wParam);
        break;
      default:
        return fFalse;
      }
      wi.yScroll = max(0, min(wi.yScroll, nScrollDiv));
      if (wi.yScroll == y)
        break;
      SetScrollPos(hwnd, SB_VERT, wi.yScroll, fTrue);
      wi.fRedraw = fTrue;
      ProcessState();
      break;

    // The window is being terminated. Clean up and free program data.

    case WM_CLOSE:
LClose:
      if (us.fNoQuit) {
        PrintWarning("Program exiting is not allowed now.");
        break;
      }
      if (wi.hpen != (HPEN)NULL)
        DeleteObject(wi.hpen);
      if (wi.lTimer != 0)
        KillTimer(hwnd, 1);
      if (hwnd == wi.hwndMain)
        PostQuitMessage(0);
      break;

    // Messages not processed here are handled by Windows in a default way.

    default:
      return DefWindowProc(hwnd, wMsg, wParam, lParam);
    }
  return fFalse;
}


// This is called after some action has been done that probably changed the
// chart state, such as a menu command was run. Update anything needing it.

void ProcessState()
{
  int cmd;

  if (wi.fCast)            // Recasting a chart implies redrawing it too.
    wi.fRedraw = fTrue;

  // If the chart type was changed, clear all the setting flags, then set the
  // appropriate core switch settings based on the new chart type.

  if (wi.nMode) {
    ClearB((pbyte)&us.fListing,
      (int)((pbyte)&us.fVelocity - (pbyte)&us.fListing));
    ClearB((pbyte)&us.fCredit,
      (int)((pbyte)&us.fLoop - (pbyte)&us.fCredit));
    us.nArabic = gi.nMode = 0;
    switch (wi.nMode) {
      case gBiorhythm:
      case gWheel:      us.fListing       = fTrue; break;
      case gHouse:      us.fWheel         = fTrue; break;
      case gGrid:       us.fGrid          = fTrue; break;
      case gHorizon:    us.fHorizon       = fTrue; break;
      case gOrbit:      us.fOrbit         = fTrue; break;
      case gSector:     us.fSector        = fTrue; break;
      case gAstroGraph: us.fAstroGraph    = fTrue; break;
      case gEphemeris:  us.fEphemeris     = fTrue; break;
      case gRising:     us.fHorizonSearch = fTrue; break;
      case gLocal:      us.fAtlasNear     = fTrue; break;
      case gSphere:     gi.nMode = gSphere;    break;
      case gWorldMap:   gi.nMode = gWorldMap;  break;
      case gGlobe:      gi.nMode = gGlobe;     break;
      case gPolar:      gi.nMode = gPolar;     break;
      case gTelescope:  gi.nMode = gTelescope; break;
      case gCalendar:   us.fCalendar   = fTrue; break;
      case gDisposit:   us.fInfluence  = fTrue; break;
      case gEsoteric:   us.fEsoteric   = fTrue; break;
      case gAspect:     us.fAspList    = fTrue; break;
      case gMidpoint:   us.fMidpoint   = fTrue; break;
      case gArabic:     us.nArabic     = 1;     break;
      case gMoons:      us.fMoonChart  = fTrue; break;
      case gTraTraTim:  us.fInDay      = fTrue; break;
      case gTraTraInf:  us.fInDayInf   = fTrue; break;
      case gTraTraGra:  us.fInDayGra   = fTrue; break;
      case gTraNatTim:  us.fTransit    = fTrue; break;
      case gTraNatInf:  us.fTransitInf = fTrue; break;
      case gTraNatGra:  us.fTransitGra = fTrue; break;
      case gSign:       us.fSign       = fTrue; break;
      case gObject:     us.fObject     = fTrue; break;
      case gHelpAsp:    us.fAspect     = fTrue; break;
      case gConstel:    us.fConstel    = fTrue; break;
      case gPlanet:     us.fOrbitData  = fTrue; break;
      case gRay:        us.fRay        = fTrue; break;
      case gMeaning:    us.fMeaning    = fTrue; break;
      case gSwitch:     us.fSwitch     = fTrue; break;
      case gObscure:    us.fSwitchRare = fTrue; break;
      case gKeystroke:  us.fKeyGraph   = fTrue; break;
      case gCredit:     us.fCredit     = fTrue; break;
    }
    cmd = rgcmdMode[wi.nMode];
    if (cmd != wi.cmdCur) {
      if (wi.cmdCur > 0)
        CheckMenu(wi.cmdCur, fFalse);
      if (cmd > 0)
        RadioMenu(cmd, cmd, cmd);
      wi.cmdCur = cmd;
    }
    wi.nMode = 0;
    wi.fRedraw = fTrue;
  }

  if (wi.fMenuAll) {       // Redetermine all menu checks if need be.
    RedoMenu();
    wi.fMenu = fTrue;
  }
  if (wi.fMenu) {          // Update menu if we added/removed check marks.
    DrawMenuBar(wi.hwnd);
    wi.fMenu = fFalse;
  }
  if (wi.fRedraw)          // Send the window a redraw message if need be.
    RedrawWindow(wi.hwnd, NULL, (HRGN)NULL, RDW_INVALIDATE);
}


// Given a command, process it, changing any appropriate program settings.
// Return values are: 0 meaning it was one of Astrolog's menu commands and
// action was taken, 1 meaning it's the special end program command, or
// -1 meaning it's not a custom command and Windows should deal with it.

int NWmCommand(WORD wCmd)
{
  char sz[cchSzDef];
  DLGPROC dlgproc;
  RECT rc;
  POINT pt;
  int i;
  long l;
  real r;
  flag fGraphicsSav, fT;

  wi.wCmd = wCmd;
  fGraphicsSav = us.fGraphics;
  switch (wCmd) {

  // File Menu

  case cmdOpenChart:
    wi.nDlgChart = 1;
    DlgOpenChart();
    break;

  case cmdOpenChart2:
    wi.nDlgChart = 2;
    DlgOpenChart();
    break;

  case cmdSaveChart:
  case cmdSavePositions:
  case cmdSaveAAF:
  case cmdSaveSettings:
  case cmdSaveText:
  case cmdSaveBitmap:
#ifdef META
  case cmdSavePicture:
#endif
#ifdef PS
  case cmdSavePS:
#endif
#ifdef WIRE
  case cmdSaveWire:
#endif
  case cmdSaveWallTile:
  case cmdSaveWallCenter:
  case cmdSaveWallStretch:
  case cmdSaveWallFit:
  case cmdSaveWallFill:
    DlgSaveChart();
    break;

  case cmdOpenBackground:
    wi.nDlgChart = 0;
    if (DlgOpenChart())
      gi.fBmp = fTrue;
    break;

  case cmdOpenWorld:
    wi.nDlgChart = -1;
    DlgOpenChart();
    break;

  case cmdPrint:
    if (!DlgPrint())
      PrintWarning("The printing was not completed successfully.");
    break;

  case cmdPrintSetup:
    l = prd.Flags;
    prd.Flags |= PD_PRINTSETUP;
    PrintDlg((LPPRINTDLG)&prd);
    prd.Flags = l;
    break;

  case cmdFileExit:
    return 1;

  // Edit Menu

  case cmdCommand:
    WiDoDialog(DlgCommand, dlgCommand);
    break;

  case cmdColor:
    WiDoDialog(DlgColor, dlgColor);
    break;

  case cmdCopyText:
    is.szFileScreen = szFileTemp;
    us.fGraphics = fFalse;
    wi.fRedraw = fTrue;
    break;

  case cmdCopyBitmap:
#ifdef META
  case cmdCopyPicture:
#endif
#ifdef PS
  case cmdCopyPS:
#endif
#ifdef WIRE
  case cmdCopyWire:
#endif
    if (us.fNoWrite)
      break;
    gi.szFileOut = szFileTemp;
    switch (wCmd) {
    case cmdCopyBitmap:  gs.ft = ftBmp; gs.chBmpMode = 'B'; break;
    case cmdCopyPicture: gs.ft = ftWmf;  break;
    case cmdCopyPS:      gs.ft = ftPS;   break;
    case cmdCopyWire:    gs.ft = ftWire; break;
    }
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdPaste:
    FFilePaste();
    wi.fCast = fTrue;
    break;

  // View Menu

  case cmdGraphics:
    inv(us.fGraphics);
    wi.fRedraw = fTrue;
    break;

  case cmdColoredText:
    inv(us.fAnsiColor); inv(us.fAnsiChar);
    WiCheckMenu(cmdColoredText, us.fAnsiColor);
    us.fGraphics = fFalse;
    wi.fRedraw = fTrue;
    break;

  case cmdWinBuffer:
    inv(wi.fBuffer);
    WiCheckMenu(cmdWinBuffer, wi.fBuffer);
    break;

  case cmdWinRedraw:
    if (wi.fNoUpdate)
      wi.fNoUpdate = 2;
    wi.fMenuAll = wi.fRedraw = fTrue;
    break;

  case cmdWinClear:
    fT = gs.fJetTrail;
    gs.fJetTrail = fFalse;
    wi.hdc = GetDC(wi.hwnd);
    if (us.fGraphics)
      DrawClearScreen();
    else
      TextClearScreen();
    ReleaseDC(wi.hwnd, wi.hdc);
    gs.fJetTrail = fT;
    break;

  case cmdWinHourglass:
    inv(wi.fHourglass);
    WiCheckMenu(cmdWinHourglass, wi.fHourglass);
    break;

  case cmdChartResizesWindow:
    inv(wi.fChartWindow);
    WiCheckMenu(cmdChartResizesWindow, wi.fChartWindow);
    wi.fRedraw = fTrue;
    break;

  case cmdWindowResizesChart:
    inv(wi.fWindowChart);
    WiCheckMenu(cmdWindowResizesChart, wi.fWindowChart);
    wi.fRedraw = fTrue;
    break;

  case cmdSizeChartToWindow:
    gs.xWin = wi.xClient;
    gs.yWin = wi.yClient;
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdSizeWindowToChart:
    ResizeWindowToChart();
    break;

  case cmdSizeWindowFull:
    inv(wi.fWindowFull);
    if (wi.fWindowFull) {
      GetWindowRect(wi.hwnd, &wi.rcFull);
      wi.rcFull.right -= wi.rcFull.left; wi.rcFull.bottom -= wi.rcFull.top;
      pt.x = pt.y = 0;
      ClientToScreen(wi.hwnd, &pt);
      GetWindowRect(GetDesktopWindow(), &rc);
      if (!SetWindowPosition(rc.left + (wi.rcFull.left - pt.x),
        rc.top + (wi.rcFull.top - pt.y),
        rc.right + (wi.rcFull.right - wi.xClient),
        rc.bottom + (wi.rcFull.bottom - wi.yClient))) {
        PrintWarning("Failed to enter full screen mode.\n"
          "You may need to run Astrolog with greater permissions for full "
          "screen mode to succeed.");
        wi.fWindowFull = fFalse;
        break;
      }
    } else {
      if (wi.rcFull.left < 0)
        wi.rcFull.left = 0;
      if (wi.rcFull.top < 0)
        wi.rcFull.top = 0;
      SetWindowPosition(wi.rcFull.left, wi.rcFull.top,
        wi.rcFull.right, wi.rcFull.bottom);
    }
    WiCheckMenu(cmdSizeWindowFull, wi.fWindowFull);
    break;

  case cmdScrollPageUp:
    PostMessage(wi.hwnd, WM_VSCROLL, SB_PAGEUP, 0);
    break;

  case cmdScrollPageDown:
    PostMessage(wi.hwnd, WM_VSCROLL, SB_PAGEDOWN, 0);
    break;

  case cmdScrollHome:
    PostMessage(wi.hwnd, WM_HSCROLL, SB_THUMBPOSITION, 0);
    PostMessage(wi.hwnd, WM_VSCROLL, SB_THUMBPOSITION, 0);
    break;

  case cmdScrollEnd:
    PostMessage(wi.hwnd, WM_HSCROLL, LFromWW(SB_THUMBPOSITION, nScrollDiv), 0);
    PostMessage(wi.hwnd, WM_VSCROLL, LFromWW(SB_THUMBPOSITION, nScrollDiv), 0);
    break;

#ifdef INTERPRET
  case cmdInterpret:
    inv(us.fInterpret);
    WiCheckMenu(cmdInterpret, us.fInterpret);
    us.fGraphics = fFalse;
    wi.fRedraw = fTrue;
    break;
#endif

  case cmdSecond:
    inv(us.fSeconds);
    is.fSeconds = us.fSeconds;
    WiCheckMenu(cmdSecond, us.fSeconds);
    wi.fRedraw = fTrue;
    break;

  case cmdParallel:
    inv(us.fParallel);
    WiCheckMenu(cmdParallel, us.fParallel);
    wi.fRedraw = fTrue;
    break;

  case cmdApplying:
    inv(us.fAppSep);
    WiCheckMenu(cmdApplying, us.fAppSep);
    wi.fRedraw = fTrue;
    break;

  // Info Menu

  case cmdSetInfo:
    wi.nDlgChart = 1;
    WiDoDialog(DlgInfo, dlgInfo);
    break;

#ifdef TIME
  case cmdNow:
    FInputData(szNowCore);
    wi.fCast = fTrue;
    break;
#endif

  case cmdDefaultInfo:
    WiDoDialog(DlgDefault, dlgDefault);
    break;

  case cmdSetInfo2:
    wi.nDlgChart = 2;
    WiDoDialog(DlgInfo, dlgInfo);
    break;

  case cmdSetInfoAll:
    WiDoDialog(DlgInfoAll, dlgInfoAll);
    break;

  case cmdRelNo:
  case cmdRelComparison:
    SetRel(us.nRel ? rcNone : rcDual);
    break;

  case cmdRelSynastry:
    SetRel(rcSynastry);
    break;

  case cmdRelComposite:
    SetRel(rcComposite);
    break;

  case cmdRelMidpoint:
    SetRel(rcMidpoint);
    break;

  case cmdRelDate:
    SetRel(rcDifference);
    gi.nMode = gWheel;
    us.fGraphics = fFalse;
    break;

#ifdef BIORHYTHM
  case cmdRelBiorhythm:
    SetRel(rcBiorhythm);
    gi.nMode = gBiorhythm;
    break;
#endif

  case cmdRelTransit:
    SetRel(rcTransit);
    break;

  case cmdRelProgressed:
    SetRel(rcProgress);
    break;

  // Settings Menu

  case cmdSidereal:
    inv(us.fSidereal);
    WiCheckMenu(cmdSidereal, us.fSidereal);
    wi.fCast = fTrue;
    break;

  case cmdHeliocentric:
    SetCentric(!us.objCenter);
    WiCheckMenu(cmdHeliocentric, us.objCenter != oEar);
    wi.fCast = fTrue;
    break;

  case cmdHouse00:
  case cmdHouse01:
  case cmdHouse02:
  case cmdHouse03:
  case cmdHouse04:
  case cmdHouse05:
  case cmdHouse06:
  case cmdHouse07:
  case cmdHouse08:
  case cmdHouse09:
  case cmdHouse10:
  case cmdHouse11:
  case cmdHouse12:
  case cmdHouse13:
  case cmdHouse14:
  case cmdHouse15:
  case cmdHouse16:
  case cmdHouse17:
  case cmdHouse18:
  case cmdHouse19:
  case cmdHouse20:
  case cmdHouse21:
  case cmdHouse22:
    us.nHouseSystem = (int)(wCmd - cmdHouse00);
    WiRadioMenu(cmdHouse00, cmdHouse00 + cSystem - 1, wCmd);
    wi.fCast = fTrue;
    break;

  case cmdHouseSetSolar:
    us.objOnAsc = us.objOnAsc ? 0 : oSun+1;
    WiCheckMenu(cmdHouseSetSolar, us.objOnAsc);
    wi.fCast = fTrue;
    break;

  case cmdHouseSet3D:
    inv(us.fHouse3D);
    WiCheckMenu(cmdHouseSet3D, us.fHouse3D);
    if (us.fGraphics) {
      if (gi.nMode == gAstroGraph) {
        wi.nMode = gGlobe;
        if (gs.fAlt) {
          gs.fAlt = fFalse;
          WiCheckMenu(cmdGraphicsModify, fFalse);
        }
      } else if (gi.nMode == gGlobe && !gs.fAlt)
        wi.nMode = gAstroGraph;
    }
    wi.fCast = fTrue;
    break;

  case cmdHouseSetDecan:
    inv(us.fDecan);
    WiCheckMenu(cmdHouseSetDecan, us.fDecan);
    wi.fCast = fTrue;
    break;

  case cmdHouseSetDwad:
    inv(us.nDwad);
    WiCheckMenu(cmdHouseSetDwad, us.nDwad > 0);
    wi.fCast = fTrue;
    break;

  case cmdHouseSetFlip:
    inv(us.fFlip);
    WiCheckMenu(cmdHouseSetFlip, us.fFlip);
    wi.fCast = fTrue;
    break;

  case cmdHouseSetGeodetic:
    inv(us.fGeodetic);
    WiCheckMenu(cmdHouseSetGeodetic, us.fGeodetic);
    wi.fCast = fTrue;
    break;

  case cmdHouseSetVedic:
    inv(us.fVedic);
    WiCheckMenu(cmdHouseSetVedic, us.fVedic);
    wi.fRedraw = fTrue;
    break;

  case cmdHouseSetNavamsa:
    inv(us.fNavamsa);
    WiCheckMenu(cmdHouseSetNavamsa, us.fNavamsa);
    wi.fCast = fTrue;
    break;

  case cmdAspect:
    WiDoDialog(DlgAspect, dlgAspect);
    break;

  case cmdObject:
    WiDoDialog(DlgObject, dlgObject);
    break;

  case cmdObject2:
    WiDoDialog(DlgObject2, dlgObject2);
    break;

  case cmdRes:
  case cmdResTransit:
    WiDoDialog(DlgRestrict, dlgRestrict);
    break;

  case cmdStar:
    WiDoDialog(DlgStar, dlgStar);
    break;

  case cmdChartMoons:
    wi.nMode = gMoons;
    us.fGraphics = fFalse;
    break;

  case cmdMoons:
    WiDoDialog(DlgMoons, dlgMoons);
    break;

  case cmdObjectM:
    WiDoDialog(DlgObjectM, dlgObjectM);
    break;

  case cmdResMoons:
    inv(us.fMoons);
    for (i = moonsLo; i <= moonsHi; i++)
      ignore[i] = !us.fMoons || !ignore[i];
    AdjustRestrictions();
    WiCheckMenu(cmdResMoons, us.fMoons);
    wi.fCast = fTrue;
    break;

  case cmdResCOB:
    inv(us.fCOB);
    for (i = cobLo; i <= cobHi; i++)
      ignore[i] = !us.fCOB || !ignore[i];
    AdjustRestrictions();
    WiCheckMenu(cmdResCOB, us.fCOB);
    wi.fCast = fTrue;
    break;

  case cmdCustom:
    WiDoDialog(DlgCustom, dlgCustom);
    break;

  case cmdResMinor:
    for (i = oChi; i <= oEP; i++)
      if (i != oNod)
        inv(ignore[i]);
    AdjustRestrictions();
    WiCheckMenu(cmdResMinor, !ignore[oChi]);
    wi.fCast = fTrue;
    break;

  case cmdResCusp:
    inv(us.fCusp);
    for (i = cuspLo; i <= cuspHi; i++)
      ignore[i] = !us.fCusp || !ignore[i];
    AdjustRestrictions();
    WiCheckMenu(cmdResCusp, us.fCusp);
    wi.fCast = fTrue;
    break;

  case cmdResUranian:
    inv(us.fUranian);
    for (i = uranLo; i <= uranHi; i++)
      ignore[i] = !us.fUranian || !ignore[i];
    AdjustRestrictions();
    WiCheckMenu(cmdResUranian, us.fUranian);
    wi.fCast = fTrue;
    break;

  case cmdResDwarf:
    inv(us.fDwarf);
    for (i = dwarfLo; i <= dwarfHi; i++)
      ignore[i] = !us.fDwarf || !ignore[i];
    AdjustRestrictions();
    WiCheckMenu(cmdResDwarf, us.fDwarf);
    wi.fCast = fTrue;
    break;

  case cmdResStar:
    us.nStar = !us.nStar;
    for (i = starLo; i <= starHi; i++)
      ignore[i] = !us.nStar || !ignore[i];
    AdjustRestrictions();
    WiCheckMenu(cmdResStar, us.nStar);
    wi.fCast = fTrue;
    break;

  case cmdSettingCalc:
    WiDoDialog(DlgCalc, dlgCalc);
    break;

  case cmdDisplay:
    WiDoDialog(DlgDisplay, dlgDisplay);
    break;

  // Chart Menu

  case cmdChartList:
    wi.nMode = gWheel;
    break;

  case cmdChartWheel:
    wi.nMode = gHouse;
    break;

  case cmdChartGrid:
    wi.nMode = gGrid;
    break;

  case cmdChartAspect:
    wi.nMode = gAspect;
    us.fGraphics = fFalse;
    break;

  case cmdChartMidpoint:
    wi.nMode = gMidpoint;
    us.fGraphics = fFalse;
    break;

  case cmdChartHorizon:
    wi.nMode = gHorizon;
    break;

  case cmdChartOrbit:
    wi.nMode = gOrbit;
    break;

  case cmdChartSector:
    wi.nMode = gSector;
    break;

  case cmdChartCalendar:
    wi.nMode = gCalendar;
    break;

  case cmdChartInfluence:
    wi.nMode = gDisposit;
    break;

  case cmdChartEsoteric:
    wi.nMode = gEsoteric;
    break;

  case cmdChartAstroGraph:
    wi.nMode = gAstroGraph;
    if (us.fGraphics && us.fHouse3D) {
      wi.nMode = gGlobe;
      if (gs.fAlt) {
        gs.fAlt = fFalse;
        WiCheckMenu(cmdGraphicsModify, fFalse);
      }
    }
    break;

  case cmdChartEphemeris:
    wi.nMode = gEphemeris;
    break;

#ifdef ARABIC
  case cmdChartArabic:
    wi.nMode = gArabic;
    us.fGraphics = fFalse;
    break;
#endif

  case cmdChartRising:
    wi.nMode = gRising;
    break;

  case cmdChartLocal:
    wi.nMode = gLocal;
    break;

  case cmdProgress:
    WiDoDialog(DlgProgress, dlgProgress);
    break;

  case cmdTransit:
    WiDoDialog(DlgTransit, dlgTransit);
    break;

  case cmdChartSettings:
    WiDoDialog(DlgChart, dlgChart);
    break;

  // Graphics Menu

  case cmdChartSphere:
    wi.nMode = gSphere;
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdChartMap:
    wi.nMode = gWorldMap;
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdChartGlobe:
    wi.nMode = gGlobe;
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdChartPolar:
    wi.nMode = gPolar;
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdChartTelescope:
    wi.nMode = gTelescope;
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsReverse:
    inv(gs.fInverse);
    WiCheckMenu(cmdGraphicsReverse, gs.fInverse);
    InitColorPalette(gs.fInverse);
    wi.fRedraw = fTrue;
    break;

  case cmdGraphicsMonochrome:
    inv(gs.fColor);
    WiCheckMenu(cmdGraphicsMonochrome, !gs.fColor);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsSquare:
    SquareX(&gs.xWin, &gs.yWin, fTrue);
    ResizeWindowToChart();
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdScale1:
  case cmdScale2:
  case cmdScale3:
  case cmdScale4:
    gs.nScale = (int)(wCmd - cmdScale1) + 1;
    gs.nScale *= 100;
    WiRadioMenu(cmdScale1, cmdScale2, wCmd);
    wi.fRedraw = fTrue;
    break;

  case cmdScaleDecrease:
    if (gs.nScale > 100) {
      gs.nScale -= 100;
      WiRadioMenu(cmdScale1, cmdScale4, cmdScale1 + gs.nScale/100 - 1);
      wi.fRedraw = fTrue;
    }
    break;

  case cmdScaleIncrease:
    if (gs.nScale < MAXSCALE) {
      gs.nScale += 100;
      WiRadioMenu(cmdScale1, cmdScale4, cmdScale1 + gs.nScale/100 - 1);
      wi.fRedraw = fTrue;
    }
    break;

  case cmdGraphicsBorder:
    inv(gs.fBorder);
    WiCheckMenu(cmdGraphicsBorder, gs.fBorder);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsText:
    inv(gs.fText);
    WiCheckMenu(cmdGraphicsText, gs.fText);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsSidebar:
    inv(us.fVelocity);
    if (!us.fVelocity) {
      gs.fText = fTrue;
      WiCheckMenu(cmdGraphicsText, fTrue);
    }
    WiCheckMenu(cmdGraphicsSidebar, !us.fVelocity);
    if (wi.nMode != gWheel && gi.nMode != gHouse && gi.nMode != gSector &&
      gi.nMode != gSphere)
      wi.nMode = gWheel;
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsThick:
    inv(gs.fThick);
    WiCheckMenu(cmdGraphicsThick, gs.fThick);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsLabel:
    inv(gs.fLabel);
    WiCheckMenu(cmdGraphicsLabel, gs.fLabel);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsAspect:
    inv(gs.fLabelAsp);
    WiCheckMenu(cmdGraphicsAspect, gs.fLabelAsp);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

#ifdef CONSTEL
  case cmdConstellation:
    if (gi.nMode != gHorizon && gi.nMode != gSphere &&
      gi.nMode != gGlobe && gi.nMode != gPolar && gi.nMode != gTelescope)
      wi.nMode = gWorldMap;
    inv(gs.fConstel);
    WiCheckMenu(cmdConstellation, gs.fConstel);
    us.fGraphics = wi.fRedraw = fTrue;
    break;
#endif

  case cmdGraphicsAllStar:
    inv(gs.fAllStar);
    WiCheckMenu(cmdGraphicsAllStar, gs.fAllStar);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsHouse:
    inv(gs.fHouseExtra);
    WiCheckMenu(cmdGraphicsHouse, gs.fHouseExtra);
    wi.fCast = fTrue;
    break;

  case cmdGraphicsEquator:
    inv(gs.fEquator);
    WiCheckMenu(cmdGraphicsEquator, gs.fEquator);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsCity:
    inv(gs.fLabelCity);
    WiCheckMenu(cmdGraphicsCity, gs.fLabelCity);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsBmp:
    if (!gi.fBmp && gi.bmpBack.rgb == NULL &&
      gi.nMode != gLocal && gi.nMode != gAstroGraph && gi.nMode != gRising &&
      gi.nMode != gWorldMap && gi.nMode != gGlobe && gi.nMode != gPolar)
      wi.nMode = gGlobe;
    inv(gi.fBmp);
    WiCheckMenu(cmdGraphicsBmp, gi.fBmp);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsAxis:
    inv(gs.fEcliptic);
    WiCheckMenu(cmdGraphicsAxis, gs.fEcliptic);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdTiltZero:
    if (gi.nMode != gTelescope && gi.nMode != gSphere && gi.nMode != gGlobe)
      wi.nMode = gGlobe;
    if (gs.rTilt != 0.0) {
      gs.rTilt = 0.0;
      wi.fRedraw = fTrue;
    }
    us.fGraphics = fTrue;
    break;

  case cmdTiltNorth:
  case cmdTiltSouth:
    if (gi.nMode != gLocal &&
      gi.nMode != gSphere && gi.nMode != gGlobe && gi.nMode != gTelescope)
      wi.nMode = gGlobe;
    r = (real)NAbs(wi.nDir) *
      (gi.nMode == gTelescope || gi.nMode == gLocal ? gi.zViewRatio : 1.0);
    if (wCmd == cmdTiltNorth) {
      if (gs.rTilt > -rDegQuad) {
        gs.rTilt -= r;
        if (gs.rTilt < -rDegQuad)
          gs.rTilt = -rDegQuad;
        wi.fRedraw = fTrue;
      }
    } else {
      if (gs.rTilt < rDegQuad) {
        gs.rTilt += r;
        if (gs.rTilt > rDegQuad)
          gs.rTilt = rDegQuad;
        wi.fRedraw = fTrue;
      }
    }
    if (gi.nMode == gTelescope)
      gs.objTrack = -1;
    us.fGraphics = fTrue;
    break;

  case cmdRotateWest:
  case cmdRotateEast:
    if (gi.nMode != gLocal && gi.nMode != gAstroGraph &&
      gi.nMode != gSphere && gi.nMode != gWorldMap && gi.nMode != gGlobe &&
      gi.nMode != gPolar && gi.nMode != gTelescope)
      wi.nMode = gGlobe;
    r = (real)NAbs(wi.nDir) *
      (gi.nMode == gTelescope || gi.nMode == gLocal ? gi.zViewRatio : 1.0);
    if (wCmd == cmdRotateWest) {
      gs.rRot += r;
      if (gs.rRot >= rDegMax)
        gs.rRot -= rDegMax;
    } else {
      gs.rRot -= r;
      if (gs.rRot < 0)
        gs.rRot += rDegMax;
    }
    if (gi.nMode == gTelescope)
      gs.objTrack = -1;
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdZoomOut:
  case cmdZoomIn:
    if (gi.nMode != gOrbit && gi.nMode != gLocal && gi.nMode != gTelescope)
      wi.nMode = gOrbit;
    if (gs.rspace < rSmall)
      gs.rspace = (real)(1 << (4-gi.nScale/gi.nScaleT));
    if (wCmd == cmdZoomIn) {
      if (gs.rspace > 0.0001)
        gs.rspace /= 2.0;
    } else {
      if (gs.rspace < rDegQuad)
        gs.rspace *= 2.0;
    }
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdGraphicsModify:
    inv(gs.fAlt);
    WiCheckMenu(cmdGraphicsModify, gs.fAlt);
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdChartModify:
    inv(us.fGridMidpoint);
    inv(us.fPrimeVert);
    inv(us.fCalendarYear);
    inv(us.fLatitudeCross);
    inv(us.nEphemYears);
    inv(us.fGraphAll);
    inv(gs.fSouth);
    inv(gs.fMollewide);
    gi.nMode = (gi.nMode == gWheel ? gHouse :
      (gi.nMode == gHouse ? gWheel : gi.nMode));
    us.fGraphics = wi.fRedraw = fTrue;
    break;

  case cmdPen00:
  case cmdPen01:
  case cmdPen02:
  case cmdPen03:
  case cmdPen04:
  case cmdPen05:
  case cmdPen06:
  case cmdPen07:
  case cmdPen08:
  case cmdPen09:
  case cmdPen10:
  case cmdPen11:
  case cmdPen12:
  case cmdPen13:
  case cmdPen14:
  case cmdPen15:
    wi.kiPen = (int)(wCmd - cmdPen00);
    WiRadioMenu(cmdPen00, cmdPen15, wCmd);
    break;

  case cmdSettingGraphics:
    WiDoDialog(DlgGraphics, dlgGraphics);
    break;

  // Animate Menu

  case cmdAnimateNo:
    neg(gs.nAnim);
    WiCheckMenu(cmdAnimateNo, gs.nAnim > 0);
    if (gs.nAnim > 0) {
      WiCheckMenu(cmdWinBuffer, fTrue);
      wi.fBuffer = wi.fRedraw = fTrue;
    }
    break;

  case cmdAnimateNow:
  case cmdAnimateNo1:
  case cmdAnimateNo2:
  case cmdAnimateNo3:
  case cmdAnimateNo4:
  case cmdAnimateNo5:
  case cmdAnimateNo6:
  case cmdAnimateNo7:
  case cmdAnimateNo8:
  case cmdAnimateNo9:
    gs.nAnim = (gs.nAnim < 0 ? -1 : 1)*((int)(wCmd - cmdAnimateNo1) + 1);
    WiRadioMenu(cmdAnimateNo1, cmdAnimateNow, wCmd);
    break;

  case cmdAnimateF1:
  case cmdAnimateF2:
  case cmdAnimateF3:
  case cmdAnimateF4:
  case cmdAnimateF5:
  case cmdAnimateF6:
  case cmdAnimateF7:
  case cmdAnimateF8:
  case cmdAnimateF9:
    wi.nDir = (wi.nDir > 0 ? 1 : -1)*(int)(wCmd - cmdAnimateF1) + 1;
    WiRadioMenu(cmdAnimateF1, cmdAnimateF9, wCmd);
    break;

  case cmdAnimateReverse:
    neg(wi.nDir);
    WiCheckMenu(cmdAnimateReverse, wi.nDir < 0);
    if (gs.nAnim < 0) {
      neg(gs.nAnim);
      WiCheckMenu(cmdWinBuffer, fTrue);
      WiCheckMenu(cmdAnimateNo, fTrue);
      wi.fBuffer = fTrue;
    }
    wi.fRedraw = fTrue;
    break;

  case cmdAnimatePause:
    inv(wi.fPause);
    WiCheckMenu(cmdAnimatePause, wi.fPause);
    break;

  case cmdTimedExposure:
    inv(gs.fJetTrail);
    WiCheckMenu(cmdTimedExposure, gs.fJetTrail);
    break;

  case cmdStepForward:
    Animate(NAbs(gs.nAnim) >= 10 ? 4 : gs.nAnim, NAbs(wi.nDir));
    wi.fCast = fTrue;
    break;

  case cmdStepBackward:
    Animate(NAbs(gs.nAnim) >= 10 ? 4 : gs.nAnim, -NAbs(wi.nDir));
    wi.fCast = fTrue;
    break;

  case cmdStore:
    ciSave = ciMain;
    break;

  case cmdRecall:
    ciMain = ciCore = ciSave;
    wi.fCast = fTrue;
    break;

  // Help Menu

  case cmdDocHelpfile:
    BootExternal(NULL, "astrolog.htm");
    break;

  case cmdDocUpdate:
    BootExternal(NULL, "changes.htm");
    break;

  case cmdDocLicense:
    BootExternal(NULL, "license.htm");
    break;

  case cmdDocWebsite:
    BootExternal(NULL, "astrolog.url");
    break;

  case cmdDocWebsite2:
    BootExternal(NULL, "astrlog2.url");
    break;

  case cmdDocDefault:
    BootExternal("Notepad", DEFAULT_INFOFILE);
    break;

  case cmdDocAtlas:
    BootExternal("Notepad", DEFAULT_ATLASFILE);
    break;

  case cmdDocTimezone:
    BootExternal("Notepad", DEFAULT_TIMECHANGE);
    break;

  case cmdDocStar:
    BootExternal("Notepad", "sefstars.txt");
    break;

  case cmdDocOrbit:
    BootExternal("Notepad", "seorbel.txt");
    break;

  case cmdHelpSign:
    wi.nMode = gSign;
    us.fGraphics = fFalse;
    break;

  case cmdHelpObject:
    wi.nMode = gObject;
    us.fGraphics = fFalse;
    break;

  case cmdHelpAspect:
    wi.nMode = gHelpAsp;
    us.fGraphics = fFalse;
    break;

#ifdef CONSTEL
  case cmdHelpConstellation:
    wi.nMode = gConstel;
    us.fGraphics = fFalse;
    break;
#endif

  case cmdHelpPlanetInfo:
    wi.nMode = gPlanet;
    us.fGraphics = fFalse;
    break;

  case cmdHelpRay:
    wi.nMode = gRay;
    us.fGraphics = fFalse;
    break;

#ifdef INTERPRET
  case cmdHelpMeaning:
    wi.nMode = gMeaning;
    us.fGraphics = fFalse;
    break;
#endif

  case cmdHelpSwitch:
    wi.nMode = gSwitch;
    us.fGraphics = fFalse;
    break;

  case cmdHelpObscure:
    wi.nMode = gObscure;
    us.fGraphics = fFalse;
    break;

  case cmdHelpKeystroke:
    wi.nMode = gKeystroke;
    us.fGraphics = fFalse;
    break;

  case cmdHelpCredit:
    wi.nMode = gCredit;
    us.fGraphics = fFalse;
    break;

  case cmdSetupAll:
  case cmdSetupUser:
    if (!us.fNoWrite)
      FCreateProgramGroup(wCmd == cmdSetupAll);
    break;

  case cmdSetupDesktop:
    if (!us.fNoWrite)
      FCreateDesktopIcon();
    break;

  case cmdSetupExtension:
    if (!us.fNoWrite)
      FRegisterExtensions();
    break;

  case cmdUnsetup:
    if (!us.fNoWrite)
      FUnregisterExtensions();
    break;

  case cmdHelpAbout:
    WiDoDialog(DlgAbout, dlgAbout);
    break;

  default:
    if (FBetween(wCmd, cmdMacro01, cmdMacro48)) {
      i = (int)(wCmd - cmdMacro01);
      if (szMacro[i]) {
        FProcessCommandLine(szMacro[i]);
        wi.fCast = wi.fMenuAll = fTrue;
      } else if (i == 0/*F1*/ || i == 39/*Alt+F4*/) {
        // F1 shows help if macro not already defined for it.
        if (i == 0)
          BootExternal(NULL, "astrolog.htm");
        else if (i == 39)
          return -1;
      } else {
        sprintf(sz, "Macro number %d is not defined.", i+1);
        PrintWarning(sz);
      }
      break;
    }
    return -1;
  }

  if (us.fNoGraphics)
    us.fGraphics = fFalse;
  if (us.fGraphics != fGraphicsSav)
    WiCheckMenu(cmdGraphics, us.fGraphics);
  return 0;
}


// For each menu command that can have a check mark by it, determine its state
// and set or clear appropriately. This is called when the program is first
// started, and after commands that may change many settings that can't be
// kept track of, such as running macro commands or script files.

void API RedoMenu()
{
  int nMode = 0, cmd;

  CheckMenu(cmdGraphics, us.fGraphics);
  CheckMenu(cmdColoredText, us.fAnsiColor);
  CheckMenu(cmdWinBuffer, wi.fBuffer);
  CheckMenu(cmdWinHourglass, wi.fHourglass);
  CheckMenu(cmdChartResizesWindow, wi.fChartWindow);
  CheckMenu(cmdWindowResizesChart, wi.fWindowChart);
  CheckMenu(cmdSizeWindowFull, wi.fWindowFull);
#ifdef INTERPRET
  CheckMenu(cmdInterpret, us.fInterpret);
#endif
  CheckMenu(cmdSecond, us.fSeconds);
  CheckMenu(cmdApplying, us.fAppSep);
  CheckMenu(cmdParallel, us.fParallel);
  RadioMenu(cmdRelBiorhythm, cmdRelTransit, CmdFromRc(us.nRel));
  CheckMenu(cmdSidereal, us.fSidereal);
  CheckMenu(cmdHeliocentric, us.objCenter != oEar);
  // Note this may not check anything due to other house systems after hsNull.
  RadioMenu(cmdHouse00, cmdHouse00 + hsNull, cmdHouse00 + us.nHouseSystem);
  CheckMenu(cmdHouseSetSolar, us.objOnAsc);
  CheckMenu(cmdHouseSet3D, us.fHouse3D);
  CheckMenu(cmdHouseSetDecan, us.fDecan);
  CheckMenu(cmdHouseSetDwad, us.nDwad > 0);
  CheckMenu(cmdHouseSetFlip, us.fFlip);
  CheckMenu(cmdHouseSetGeodetic, us.fGeodetic);
  CheckMenu(cmdHouseSetVedic, us.fVedic);
  CheckMenu(cmdHouseSetNavamsa, us.fNavamsa);
  CheckMenu(cmdResMinor, !ignore[oChi]);
  CheckMenu(cmdResCusp, us.fCusp);
  CheckMenu(cmdResUranian, us.fUranian);
  CheckMenu(cmdResDwarf, us.fDwarf);
  CheckMenu(cmdResMoons, us.fMoons);
  CheckMenu(cmdResCOB, us.fCOB);
  CheckMenu(cmdResStar, us.nStar);
  CheckMenu(cmdProgress, us.fProgress);
#ifdef CONSTEL
  CheckMenu(cmdConstellation, gs.fConstel);
#endif
  CheckMenu(cmdGraphicsAllStar, gs.fAllStar);
  CheckMenu(cmdGraphicsHouse, gs.fHouseExtra);
  CheckMenu(cmdGraphicsEquator, gs.fEquator);
  CheckMenu(cmdGraphicsCity, gs.fLabelCity);
  CheckMenu(cmdGraphicsBmp, gi.fBmp);
  CheckMenu(cmdGraphicsAxis, gs.fEcliptic);
  CheckMenu(cmdGraphicsReverse, gs.fInverse);
  CheckMenu(cmdGraphicsMonochrome, !gs.fColor);
  CheckMenu(cmdGraphicsBorder, gs.fBorder);
  CheckMenu(cmdGraphicsText, gs.fText);
  CheckMenu(cmdGraphicsSidebar, !us.fVelocity);
  CheckMenu(cmdGraphicsLabel, gs.fLabel);
  CheckMenu(cmdGraphicsThick, gs.fThick);
  RadioMenu(cmdScale1, cmdScale4, cmdScale1 + gs.nScale/100 - 1);
  CheckMenu(cmdGraphicsModify, gs.fAlt);
  RadioMenu(cmdPen00, cmdPen15, cmdPen00 + wi.kiPen);
  CheckMenu(cmdAnimateNo, gs.nAnim > 0);
  RadioMenu(cmdAnimateNo1, cmdAnimateNow, cmdAnimateNo + NAbs(gs.nAnim));
  RadioMenu(cmdAnimateF1, cmdAnimateF9, cmdAnimateF1 + NAbs(wi.nDir) - 1);
  CheckMenu(cmdAnimateReverse, wi.nDir < 0);
  CheckMenu(cmdAnimatePause, wi.fPause);
  CheckMenu(cmdTimedExposure, gs.fJetTrail);
  wi.fMenuAll = fFalse;

  if (us.fGraphics)
    nMode = FBetween(gi.nMode, gSphere, gTelescope) ? gi.nMode :
      DetectGraphicsChartMode();
  else if (us.fCredit)     nMode = gCredit;
  else if (us.fSwitch)     nMode = gSwitch;
  else if (us.fSwitchRare) nMode = gObscure;
  else if (us.fKeyGraph)   nMode = gKeystroke;
  else if (us.fSign)       nMode = gSign;
  else if (us.fObject)     nMode = gObject;
  else if (us.fAspect)     nMode = gHelpAsp;
  else if (us.fConstel)    nMode = gConstel;
  else if (us.fOrbitData)  nMode = gPlanet;
  else if (us.fRay)        nMode = gRay;
  else if (us.fMeaning)    nMode = gMeaning;
  else if (us.fListing)    nMode = gWheel;
  else if (us.fWheel)      nMode = gHouse;
  else if (us.fGrid)       nMode = gGrid;
  else if (us.fAspList)    nMode = gAspect;
  else if (us.fMidpoint)   nMode = gMidpoint;
  else if (us.fHorizon)    nMode = gHorizon;
  else if (us.fOrbit)      nMode = gOrbit;
  else if (us.fSector)     nMode = gSector;
  else if (us.fInfluence)  nMode = gDisposit;
  else if (us.fEsoteric)   nMode = gEsoteric;
  else if (us.fAstroGraph) nMode = gAstroGraph;
  else if (us.fCalendar)   nMode = gCalendar;
  else if (us.fEphemeris)  nMode = gEphemeris;
  else if (us.nArabic)     nMode = gArabic;
  else if (us.fHorizonSearch) nMode = gRising;
  else if (us.fAtlasNear)  nMode = gLocal;
  else if (us.fMoonChart)  nMode = gMoons;
  else if (us.fInDay || us.fInDayInf || us.fInDayGra ||
    us.fTransit || us.fTransitInf || us.fTransitGra)
    nMode = gTraTraGra;
  cmd = rgcmdMode[nMode];
  if (cmd != wi.cmdCur) {
    if (wi.cmdCur > 0)
      CheckMenu(wi.cmdCur, fFalse);
    if (cmd > 0)
      RadioMenu(cmd, cmd, cmd);
    wi.cmdCur = cmd;
  }
}


#ifdef DEBUG
// Determine whether a Windows font is installed. Have to enumarate all fonts
// on the system, and compare them to the font being searched for.

static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *pelf,
  NEWTEXTMETRICEX *pntm, int nFontType, LPARAM lParam)
{
  LPARAM *pl = (LPARAM *)lParam;

  *pl = 1;
  return fTrue;
}

flag FFontInstalled(CONST char *sz)
{
  LOGFONT lf;
  LPARAM lParam = 0;

  ClearB((pbyte)&lf, sizeof(lf));
  lf.lfCharSet = DEFAULT_CHARSET;
  strcpy(lf.lfFaceName, sz);
  EnumFontFamiliesEx(GetDC(NULL), &lf, (FONTENUMPROC)EnumFontFamExProc,
    (LPARAM)&lParam, 0);
  return lParam ? fTrue : fFalse;
}
#endif


// Implements the Paste menu command.

flag FFilePaste(void)
{
  char szTmp[cchSzMax];
  HFILE hfile;
  LONG lSize;
  HGLOBAL hglobal;
  byte *hpb;
  flag fText;

  // Check for a bitmap or text on the Windows clipboard, the two formats
  // that Astrolog knows how to paste.
  if (!OpenClipboard(wi.hwnd))
    return fFalse;
  if (IsClipboardFormatAvailable(CF_DIB))
    fText = fFalse;
  else if (IsClipboardFormatAvailable(CF_TEXT))
    fText = fTrue;
  else {
    PrintWarning("There is nothing on the clipboard to paste.\n");
    return fFalse;
  }

  // Get a memory buffer containing the Windows clipboard.
  hglobal = GetClipboardData(fText ? CF_TEXT : CF_DIB);
  if (hglobal == (HGLOBAL)NULL)
    return fFalse;

  // Create a temporary file.
  sprintf(szTmp, "%s.tmp", szAppName);
  hfile = _lcreat(szTmp, 0);
  if (hfile == HFILE_ERROR)
    return fFalse;

  // Save the contents of the memory buffer to that temporary file.
  hpb = (byte *)GlobalLock(hglobal);
  lSize = (LONG)GlobalSize(hglobal);
  _hwrite(hfile, (char *)hpb, lSize);
  _lclose(hfile);
  GlobalUnlock(hglobal);

  // Load the file's contents into the program.
  if (fText)
    FInputData(szTmp);
  else
    FLoadBmp(szTmp, &gi.bmpBack, fTrue);
  _unlink(szTmp);
  CloseClipboard();
  return fTrue;
}


// This important routine is the bottleneck to redraw the window and call into
// the program to draw or do action with a particular chart type.

flag API FRedraw(void)
{
  // Local variables used in drawing on the screen.
  PAINTSTRUCT ps;
  HDC hdcWin;
  HCURSOR hcurOld;
  HBITMAP hbmp, hbmpOld;
  HFONT hfontOld = NULL;
  char szFile[cchSzDef];
  int nScrollRow, i;
  flag fSmartText = fFalse, fAnsiColor, fAnsiChar,
    fSmartHTML = fFalse, fInverse;

  // Local variables used for copying to the Windows clipboard.
  HFILE hfile;
  LONG lSize, l;
  HGLOBAL hglobal, hmfp;
  byte *hpb;
  METAFILEPICT mfp;
  HMETAFILE hmf;
  int cfid;
  char *pch;

  if (us.fSmartSave && (wi.wCmd == cmdSaveText || wi.wCmd == cmdCopyText)) {
    if (!us.fTextHTML)
      fSmartText = fTrue;
    else
      fSmartHTML = fTrue;
  }
  if (wi.fHourglass)
    hcurOld = SetCursor(LoadCursor((HINSTANCE)NULL, IDC_WAIT));
  ClearB((pbyte)&ps, sizeof(PAINTSTRUCT));
  if (wi.hdcPrint != hdcNil)
    wi.hdc = wi.hdcPrint;
  else {
    hdcWin = BeginPaint(wi.hwnd, &ps);
    if (wi.fBuffer) {
      wi.hdc = CreateCompatibleDC(hdcWin);
      hbmp = CreateCompatibleBitmap(hdcWin, wi.xClient, wi.yClient);
      hbmpOld = (HBITMAP)SelectObject(wi.hdc, hbmp);
      if (gs.fJetTrail)
        BitBlt(wi.hdc, 0, 0, wi.xClient, wi.yClient, hdcWin, 0, 0, SRCCOPY);
    } else
      wi.hdc = hdcWin;
  }

  if (us.fGraphics) {
    // Set up a graphics chart.
    if (wi.fWindowChart && wi.hdcPrint == hdcNil) {
      gs.xWin = wi.xClient; gs.yWin = wi.yClient;
    }
    gi.nScale = gs.nScale/100;
    gi.nScaleText = gs.nScaleText/50;
    if (gs.nFont/10000 == 0)
      gi.nScaleText &= ~1;
    gi.kiCur = -1;
  } else {
    // Set up a text chart.
    gi.xOffset = gi.yOffset = 0;
    SetWindowOrg(wi.hdc, 0, 0);
    SetWindowExt(wi.hdc, wi.xClient, wi.yClient);
    SetBkMode(wi.hdc, TRANSPARENT);
    if (wi.hdcPrint == hdcNil)
      TextClearScreen();
    FBmpDrawBack(NULL);
    i = gs.nScale/100;
    wi.xChar = i < 2 ? 6 : (i < 3 ? 8 : (i < 4 ? 10 : 12));
    wi.yChar = i < 2 ? 8 : (i < 3 ? 12 : (i < 4 ? 18 : 16));
    wi.hfont = CreateFont(wi.yChar /*nHeight*/, wi.xChar /*nWidth*/,
      0 /*nEscapement*/, 0 /*nOrientation*/, FW_DONTCARE,
      0 /*fbItalic*/, 0 /*fbUnderline*/, 0 /*fbStrikeOut*/,
      DEFAULT_CHARSET, OUT_RASTER_PRECIS, CLIP_DEFAULT_PRECIS,
      DRAFT_QUALITY, FIXED_PITCH | FF_DONTCARE, "Terminal");
    hfontOld = (HFONT)SelectObject(wi.hdc, wi.hfont);
    // If printing, set the number of text rows per page.
    if (wi.hdcPrint != hdcNil) {
      nScrollRow = us.nScrollRow;
      us.nScrollRow = wi.yClient / wi.yChar;
      if (us.fSmartSave)
        fSmartHTML = fTrue;
    }
    if (wi.wCmd == cmdCopyText && us.fTextHTML)
      is.nHTML = -1;
  }
  if (fSmartText) {
    fAnsiColor = us.fAnsiColor; fAnsiChar = us.fAnsiChar;
    us.fAnsiColor = fFalse; us.fAnsiChar = fFalse;
  } else if (fSmartHTML) {
    fInverse = gs.fInverse; fAnsiChar = us.fAnsiChar;
    gs.fInverse = fTrue; us.fAnsiChar = fFalse;
    InitColorPalette(1);
  }

  Action();    // Actually go and create the chart here.

  // Create the chart again if also autosaving chart to file.

  if (wi.fAutoSave && us.fGraphics &&
    (!wi.fAutoSaveWire ? gs.ft != ftBmp : gs.ft != ftWire)) {
    gs.ft = !wi.fAutoSaveWire ? ftBmp : ftWire;
    if (!wi.fAutoSaveNum)
      gi.szFileOut = !wi.fAutoSaveWire ? szFileAutoCore : "astrolog.dw";
    else {
      gi.szFileOut = szFile;
      sprintf(szFile, "ast%05d.%s",
        wi.nAutoSaveNum, !wi.fAutoSaveWire ? "bmp" : "dw");
      wi.nAutoSaveNum++;
    }
    Action();
    if (gs.ft == ftWire) {
      gs.xWin /= WIREMUL; gs.yWin /= WIREMUL; gs.nScale /= WIREMUL;
    }
    gi.szFileOut = NULL;
    gs.ft = ftNone;
  }

  // Cleanup and copy from the buffer to the screen if need be.

  if (fSmartText) {
    us.fAnsiColor = fAnsiColor; us.fAnsiChar = fAnsiChar;
  } else if (fSmartHTML) {
    gs.fInverse = fInverse; us.fAnsiChar = fAnsiChar;
    InitColorPalette(fInverse);
  }
  if (!us.fGraphics) {
    if (wi.hdcPrint != hdcNil)
      us.nScrollRow = nScrollRow;
    SelectObject(wi.hdc, hfontOld);
    DeleteObject(wi.hfont);
  }
  if (wi.hdcPrint == hdcNil) {
    if (wi.fBuffer) {
      BitBlt(hdcWin, 0, 0, wi.xClient, wi.yClient,
        wi.hdc, -gi.xOffset, -gi.yOffset, SRCCOPY);
      SelectObject(wi.hdc, hbmpOld);
      DeleteObject(hbmp);
      DeleteDC(wi.hdc);
    }
    EndPaint(wi.hwnd, &ps);
  }
  if (wi.fHourglass)
    SetCursor(hcurOld);

  // If all text was scrolled off the top of the screen, scroll up.
  // If all text was scrolled off the left of the screen, scroll left.

  if (!us.fGraphics && is.S == stdout) {
    if (is.cchRow - wi.yScroll * 10 < 0)
      PostMessage(wi.hwnd, WM_VSCROLL,
        LFromWW(SB_THUMBPOSITION, is.cchRow / 10 - 2), 0);
    if (is.cchColMax - wi.xScroll * 10 < 0)
      PostMessage(wi.hwnd, WM_HSCROLL,
        LFromWW(SB_THUMBPOSITION, is.cchColMax / 10 - 2), 0);
  }

  // Sometimes creating a chart means saving it to a file instead of drawing
  // it on screen. If were in file mode, cleanup things here.

  if (is.szFileScreen != NULL || gs.ft != ftNone) {
    is.szFileScreen = NULL;
    if (gs.ft == ftWmf) {
      gs.xWin /= METAMUL; gs.yWin /= METAMUL; gs.nScale /= METAMUL;
    } else if (gs.ft == ftPS) {
      gs.xWin /= PSMUL; gs.yWin /= PSMUL; gs.nScale /= PSMUL;
    } else if (gs.ft == ftWire) {
      gs.xWin /= WIREMUL; gs.yWin /= WIREMUL; gs.nScale /= WIREMUL;
    }
    gs.ft = ftNone;

    // To copy charts to the clipboard, Astrolog saves the chart to a temp
    // file, then copies the contents of that file to the clipboard.

    if (wi.wCmd == cmdCopyText ||
      wi.wCmd == cmdCopyBitmap || wi.wCmd == cmdCopyPicture ||
      wi.wCmd == cmdCopyPS || wi.wCmd == cmdCopyWire) {
      hfile = _lopen(szFileTemp, OF_READ);
      if (hfile == HFILE_ERROR)
        return fFalse;
      lSize = _llseek(hfile, 0, 2);
      // For bitmap and metafile charts, skip over the file header bytes.
      l = wi.wCmd == cmdCopyBitmap ? sizeof(BITMAPFILEHEADER) :
        (wi.wCmd == cmdCopyPicture ? 22 : 0);
      lSize -= l;
      hglobal = GlobalAlloc(GMEM_MOVEABLE, lSize);
      if (hglobal == (HGLOBAL)NULL)
        return fFalse;
      hpb = (byte *)GlobalLock(hglobal);
      _llseek(hfile, l, 0);
      _hread(hfile, hpb, lSize);
      _lclose(hfile);
      if (l == 0 && us.fTextHTML) {
        // Mark HTML clipboard fragment end character indexes.
        pch = (char *)(hpb + 39);
        sprintf(pch, "%08d", lSize - 1);
        *(pch + 8) = '\n';
        pch = (char *)(hpb + 87);
        sprintf(pch, "%08d", lSize - 34);
        *(pch + 8) = '\n';
      }
      GlobalUnlock(hglobal);
      if (!OpenClipboard(wi.hwnd))
        return fFalse;
      EmptyClipboard();
      if (l == 0) {
        if (!us.fTextHTML)
          SetClipboardData(CF_TEXT, hglobal);
        else {
          cfid = RegisterClipboardFormat("HTML Format");
          if (cfid != 0)
            SetClipboardData(cfid, hglobal);
        }
      } else if (wi.wCmd == cmdCopyBitmap) {
        SetClipboardData(CF_DIB, hglobal);
      } else {
        // For metafiles special structure with pointer to picture data needs
        // to be allocated and used.
        mfp.mm = MM_ISOTROPIC;
        mfp.xExt = (gs.xWin-1)*METAMUL*2;
        mfp.yExt = (gs.yWin-1)*METAMUL*2;
        hpb = (byte *)GlobalLock(hglobal);
        hmf = SetMetaFileBitsEx(lSize, hpb);
        GlobalUnlock(hglobal);
        mfp.hMF = hmf;
        hmfp = GlobalAlloc(GMEM_MOVEABLE, sizeof(METAFILEPICT));
        if (hmfp == (HGLOBAL)NULL)
          return fFalse;
        hpb = (byte *)GlobalLock(hmfp);
        *(METAFILEPICT FAR *)hpb = mfp;
        GlobalUnlock(hmfp);
        SetClipboardData(CF_METAFILEPICT, hmfp);
      }
      CloseClipboard();
      _unlink(szFileTemp);
      wi.wCmd = 0;
    }
    ProcessState();
  }
  if (fSmartText || fSmartHTML)
    wi.wCmd = 0;

  wi.fRedraw = fFalse;
  if (wi.fNoUpdate)
    wi.fNoUpdate = fTrue;
  return fTrue;
}


/*
******************************************************************************
** Windows Setup.
******************************************************************************
*/

// Create a Windows shortcut file, a link pointing to another file. Called
// from FCreateProgramGroup() and FCreateDesktopIcon() to setup program.

flag FCreateShortcut(CONST char *szDir, CONST char *szName,
  CONST char *szFile, CONST char *szDesc, int nIcon)
{
  IShellLinkA *pisl = NULL;
  IPersistFile *pipf = NULL;
  char sz[cchSzMax], *pch;
  WCHAR wsz[cchSzMax];
  HRESULT hr;
  flag fRet = fFalse;

  // Define the link data.
  hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
    IID_IShellLink, (LPVOID *)&pisl);
  if (FAILED(hr))
    goto LExit;
  GetModuleFileName(wi.hinst, sz, cchSzMax);
  if (nIcon >= 0) {
    hr = pisl->SetIconLocation(sz, nIcon);
    if (FAILED(hr))
      goto LExit;
  }
  for (pch = sz; *pch; pch++)
    ;
  while (pch > sz && *pch != '\\')
    pch--;
  *pch = chNull;
  hr = pisl->SetWorkingDirectory(sz);
  if (FAILED(hr))
    goto LExit;
  sprintf(pch, "\\%s", szFile);
  hr = pisl->SetPath(sz);
  if (FAILED(hr))
    goto LExit;
  if (szDesc == NULL)
    szDesc = szName;
  hr = pisl->SetDescription(szDesc);
  if (FAILED(hr))
    goto LExit;

  // Save it to a link file.
  hr = pisl->QueryInterface(IID_IPersistFile, (LPVOID *)&pipf);
  if (FAILED(hr))
    goto LExit;
  sprintf(sz, "%s", szDir);
  for (pch = sz; *pch; pch++)
    ;
  sprintf(pch, "\\%s.lnk", szName);
  MultiByteToWideChar(CP_ACP, 0, sz, -1, wsz, cchSzMax);
  hr = pipf->Save(wsz, fTrue);
  if (FAILED(hr))
    goto LExit;

  fRet = fTrue;
LExit:
  if(pipf)
    pipf->Release();
  if(pisl)
    pisl->Release();
  return fRet;
}


// Delete a Windows shortcut file, a link pointing to another file. Called
// from FCreateProgramGroup() to setup the program.

void DeleteShortcut(CONST char *szDir, CONST char *szFile)
{
  char sz[cchSzMax];

  sprintf(sz, "%s\\%s.lnk", szDir, szFile);
  _unlink(sz);
}


// Add an icon pointing to the Astrolog executable on the Windows desktop.

flag FCreateDesktopIcon()
{
  char szDir[cchSzMax], szName[cchSzMax];
  LPITEMIDLIST pidl;
  flag fRet;

  SHGetSpecialFolderLocation(wi.hwnd, CSIDL_DESKTOPDIRECTORY, &pidl);
  SHGetPathFromIDList(pidl, szDir);
  sprintf(szName, "%s %s", szAppName, szVersionCore);
  fRet = FCreateShortcut(szDir, szName,
    "astrolog.exe", "Astrolog executable", 0);
  if (!fRet)
    PrintError("Failed to create desktop icon.");
  return fRet;
}


// Create a Windows program group folder containing icons for Astrolog.

flag FCreateProgramGroup(flag fAll)
{
  char szDir[cchSzMax], szName[cchSzMax], *pch;
  LPITEMIDLIST pidl;

  // Create program group folder.
  SHGetSpecialFolderLocation(wi.hwnd,
    fAll ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS, &pidl);
  SHGetPathFromIDList(pidl, szDir);
  for (pch = szDir; *pch; pch++)
    ;
  sprintf(pch, "\\Astrolog");
  if (!CreateDirectory(szDir, NULL)) {
    if (GetLastError() != ERROR_ALREADY_EXISTS)
      goto LError;
  }

  // Delete old previous version shortcuts from folder.
  DeleteShortcut(szDir, "Astrolog 6.00");
  DeleteShortcut(szDir, "Astrolog 6.10");
  DeleteShortcut(szDir, "Astrolog 6.20");
  DeleteShortcut(szDir, "Astrolog 6.30");
  DeleteShortcut(szDir, "Astrolog 6.40");
  DeleteShortcut(szDir, "Astrolog 6.50");
  DeleteShortcut(szDir, "Astrolog 7.00");
  DeleteShortcut(szDir, "Astrolog 7.10");
  DeleteShortcut(szDir, "Astrolog 7.20");

  // Add main shortcuts in folder.
  sprintf(szName, "%s %s", szAppName, szVersionCore);
  if (!FCreateShortcut(szDir, szName,
    "astrolog.exe", "Astrolog executable", -1))
    goto LError;
  if (!FCreateShortcut(szDir, "Astrolog documentation",
    "astrolog.htm", NULL, -1))
    goto LError;
  if (!FCreateShortcut(szDir, "Astrolog changes", "changes.htm", NULL, -1))
    goto LError;
  if (!FCreateShortcut(szDir, "Astrolog website", "astrolog.url", NULL, 3))
    goto LError;

  return fTrue;
LError:
  PrintError("Failed to create program group.\n"
    "Astrolog can still be run from other icons or directly from its folder.");
  return fFalse;
}


// Add info about Astrolog specific file extensions to the Windows registry.
// Could edit HKCR\ instead of HKCU\Software\Classes\ to affect all users,
// but that would require Administrator priviledges on modern systems.

flag FRegisterExtensions()
{
  HKEY hkey;
  char szExe[cchSzMax], szIco[cchSzMax], *pch;

  GetModuleFileName(wi.hinst, szExe, cchSzMax);
  sprintf(szIco, "%s,1", szExe);
  for (pch = szExe; *pch; pch++)
    ;
  sprintf(pch, "%s", " %1");

  // Define .as extension and point it to Astrolog File Type.
  if (RegCreateKey(HKEY_CURRENT_USER,
    "Software\\Classes\\.as", &hkey) != ERROR_SUCCESS)
    goto LError;
  if (RegSetValue(hkey, NULL, REG_SZ, "Astrolog.as", 11) != ERROR_SUCCESS)
    goto LError;
  RegCloseKey(hkey);

  // Set File Type name for .as extensions.
  if (RegCreateKey(HKEY_CURRENT_USER,
    "Software\\Classes\\Astrolog.as", &hkey) != ERROR_SUCCESS)
    goto LError;
  if (RegSetValue(hkey, NULL, REG_SZ, "Astrolog Settings", 17) !=
    ERROR_SUCCESS)
    goto LError;
  RegCloseKey(hkey);

  // Make .as extension files be opened by Astrolog.
  if (RegCreateKey(HKEY_CURRENT_USER,
    "Software\\Classes\\Astrolog.as\\shell\\open\\command",
    &hkey) != ERROR_SUCCESS)
    goto LError;
  if (RegSetValue(hkey, NULL, REG_SZ, szExe, CchSz(szExe)) != ERROR_SUCCESS)
    goto LError;
  RegCloseKey(hkey);

  // Make .as extension files be edited by Windows Notepad.
  if (RegCreateKey(HKEY_CURRENT_USER,
    "Software\\Classes\\Astrolog.as\\shell\\edit\\command",
    &hkey) != ERROR_SUCCESS)
    goto LError;
  if (RegSetValue(hkey, NULL, REG_SZ, "Notepad %1", 10) != ERROR_SUCCESS)
    goto LError;
  RegCloseKey(hkey);

  // Set icon for .as extension files.
  if (RegCreateKey(HKEY_CURRENT_USER,
    "Software\\Classes\\Astrolog.as\\DefaultIcon", &hkey) != ERROR_SUCCESS)
    goto LError;
  if (RegSetValue(hkey, NULL, REG_SZ, szIco, CchSz(szIco)) != ERROR_SUCCESS)
    goto LError;
  RegCloseKey(hkey);

  if (wi.wCmd == cmdSetupExtension)
    PrintNotice("Registered Astrolog as owner of file extension \".as\".\n");
  return fTrue;
LError:
  PrintError("Failed to register Astrolog file extensions.\n"
    "This error is ignorable and all features of Astrolog will still work.");
  return fFalse;
}


// Remove Astrolog specific data from the Windows registry.

flag FUnregisterExtensions()
{
  // Delete .as extension pointing to Astrolog File Type.
  if (SHDeleteKey(HKEY_CURRENT_USER,
    "Software\\Classes\\.as") != ERROR_SUCCESS)
    goto LError;

  // Delete icon for .as extension files.
  if (SHDeleteKey(HKEY_CURRENT_USER,
    "Software\\Classes\\Astrolog.as") != ERROR_SUCCESS)
    goto LError;

  if (wi.wCmd == cmdUnsetup)
    PrintNotice("Unregistered Astrolog as owner of file extension \".as\".\n");
  return fTrue;
LError:
  PrintError("Failed to unregister Astrolog file extensions.");
  return fFalse;
}
#endif // WIN

/* wdriver.cpp */
