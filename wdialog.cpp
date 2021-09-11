/*
** Astrolog (Version 7.30) File: wdialog.cpp
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
** Dialog Utility Functions.
******************************************************************************
*/

// Set the contents of the given edit control in a dialog to a string.

void SetEditSz(HWND hdlg, int id, CONST char *sz)
{
  while (*sz == ' ')    // Strip off any extra leading spaces.
    sz++;
  SetEdit(id, sz);
}


// Set the contents of the given edit control in a dialog to a floating point
// value, with at most 'n' significant fractional digits.

void SetEditR(HWND hdlg, int id, real r, int n)
{
  char sz[cchSzDef];

  FormatR(sz, r, n);
  SetEdit(id, sz);
}


// Set the contents of four combo controls and their dropdowns in a dialog
// indicating month, day, year, and time fields to the given values.

void SetEditMDYT(HWND hdlg, int idMon, int idDay, int idYea, int idTim,
  int mon, int day, int yea, real tim)
{
  char sz[cchSzDef];
  int i;

  ClearCombo(idMon);
  ClearCombo(idDay);
  ClearCombo(idYea);
  ClearCombo(idTim);
  if (!FValidMon(mon))
    mon = 1;
  sprintf(sz, "%.3s", szMonth[mon]);
  SetEdit(idMon, sz);
  for (i = 1; i <= cSign; i++)
    SetCombo(idMon, szMonth[i]);
  if (!FValidDay(day, mon, yea))
    day = 1;
  SetEditN(idDay, day);
  for (i = 0; i <= 25; i += 5) {
    sprintf(sz, "%d", Max(i, 1)); SetCombo(idDay, sz);
  }
  SetEditN(idYea, yea);
  for (i = 2015; i <= 2025; i++) {
    sprintf(sz, "%d", i); SetCombo(idYea, sz);
  }
  sprintf(sz, "%s", SzTim(tim));
  SetEditSz(hdlg, idTim, sz);
  SetCombo(idTim, "Midnight");
  SetCombo(idTim, (char *)(us.fEuroTime ? "6:00" : "6:00am"));
  SetCombo(idTim, "Noon");
  SetCombo(idTim, (char *)(us.fEuroTime ? "18:00" : "6:00pm"));
}


// Set the contents of four combo controls in a dialog indicating daylight,
// time zone, longitude, and latitude fields to the given values.

void SetEditSZOA(HWND hdlg, int idDst, int idZon, int idLon, int idLat,
  real dst, real zon, real lon, real lat)
{
  char sz[cchSzDef];
  int i;
  flag fT;

  ClearCombo(idDst);
  ClearCombo(idZon);
  ClearCombo(idLon);
  ClearCombo(idLat);
  if (dst == 0.0 || dst == 1.0 || dst == dstAuto)
    sprintf(sz, "%s",
      dst == 0.0 ? "No" : (dst == 1.0 ? "Yes" : "Autodetect"));
  else
    sprintf(sz, "%s", SzZone(dst));
  SetEdit(idDst, sz);
  SetCombo(idDst, "No"); SetCombo(idDst, "Yes");
  sprintf(sz, "%s", SzZone(zon));
  SetEdit(idZon, (char *)(sz[0] == '+' ? &sz[1] : sz));

  // For the time zone dropdown, fill it out with all abbreviations of three
  // letters that don't reference daylight or war time.
  for (i = 0; i < cZone; i++) {
    if (szZon[i][1] && szZon[i][1] != 'D' && szZon[i][1] != 'W' &&
      szZon[i][2] && szZon[i][2] != 'D') {
      if (rZon[i] != zonLMT)
        sprintf(sz, "%s %s", SzZone(rZon[i]), szZon[i]);
      else
        sprintf(sz, "%s", SzZone(rZon[i]));
      SetCombo(idZon, sz);
    }
  }
  fT = us.fAnsiChar; us.fAnsiChar = fFalse;
  sprintf(sz, "%s", SzLocation(lon, lat));
  us.fAnsiChar = fT;
  i = 7 + is.fSeconds*3;
  sz[i] = chNull;
  SetEditSz(hdlg, idLon, &sz[0]);
  SetCombo(idLon, "122W20"); SetCombo(idLon, "0E00");
  SetEditSz(hdlg, idLat, &sz[i+1]);
  SetCombo(idLat, "47N36"); SetCombo(idLat, "0S00");
}


// Set the contents of a combo control in a dialog indicating a color field
// field to the given value, and fill its dropdown with the color list.

void SetEditColor(HWND hdlg, int id, KI ki, int nExtra)
{
  int i, iMax = cColor + (nExtra > 0)*(nExtra+1);

  SetEdit(id, szColor[ki]);
  for (i = 0; i < iMax; i++) {
    SetCombo(id, szColor[i]);
    if (i == kWhite && nExtra >= 2)
      i += nExtra;
  }
}


// Return the contents of a dialog edit control as an integer value.

int GetEditN(HWND hdlg, int id)
{
  char sz[cchSzMax];

  GetEdit(id, sz);
  return NFromSz(sz);
}


// Return the contents of a dialog edit control as a floating point value.

real GetEditR(HWND hdlg, int id)
{
  char sz[cchSzMax];

  GetEdit(id, sz);
  return RFromSz(sz);
}


// Return which radio button in a radio button group is set.

int GetRadio(HWND hdlg, int id1, int id2)
{
  int i;

  for (i = id1; i <= id2; i++)
    if (GetCheck(i))
      return i - id1;
  return 0;
}


// Bring up an error box indicating an illegal value for a dialog field.

void ErrorEnsure(int n, CONST char *sz)
{
  char szT[cchSzDef];

  sprintf(szT, "The value %d is not a valid %s setting.", n, sz);
  PrintWarning(szT);
}


#ifdef ATLAS
// Shared WM_COMMAND processor for atlas and time change related button
// commands. Called from DlgInfo() and DlgDefault().

flag FDlgInfoAtlas(HWND hdlg, WORD wParam, flag fDefault)
{
  CI ci;
  char sz[cchSzMax];
  int dcDst, dcZon, dcLon, dcLat, deLoc, ilist, i, j;
  real lon, lat;

  // Variables different between the Chart Info and Defaults dialogs.
  dcDst = (fDefault ? dcDeDst : dcInDst);
  dcZon = (fDefault ? dcDeZon : dcInZon);
  dcLon = (fDefault ? dcDeLon : dcInLon);
  dcLat = (fDefault ? dcDeLat : dcInLat);
  deLoc = (fDefault ? deDeLoc : deInLoc);
  ilist = (fDefault ? 11 : 14);

  // "Lookup City" button:
  if (wParam == dbInCity) {
    GetEdit(deLoc, sz);
    ClearList(dlIn);
    if (!DisplayAtlasLookup(sz, (size_t)hdlg, &ilist))
      PrintWarning("Couldn't get atlas data!");

  // "Nearby Cities" button:
  } else if (wParam == dbInCoor) {
    GetEdit(dcLon, sz); lon = RParseSz(sz, pmLon);
    GetEdit(dcLat, sz); lat = RParseSz(sz, pmLat);
    ClearList(dlIn);
    if (!DisplayAtlasNearby(lon, lat, (size_t)hdlg, &ilist, fFalse))
      PrintWarning("Couldn't get atlas data!");

  // "Time Changes" button:
  } else if (wParam == dbInChan) {
    GetEdit(deLoc, sz);
    if (!DisplayAtlasLookup(sz, 0, &i)) {
      PrintWarning("Please have a valid city in the 'Location' field first.");
      return fTrue;
    }
    ClearList(dlIn);
    ci = ciMain;
    GetEdit(dcInYea, sz); ci.yea = NParseSz(sz, pmYea);
    if (!DisplayTimezoneChanges(is.rgae[i].izn, (size_t)hdlg, &ci))
      PrintWarning("Couldn't get time zone data!");

  // "Apply Info" button:
  } else if (wParam == dbInAppl) {
    j = (int)SendDlgItemMessage(hdlg, dlIn, LB_GETCURSEL, 0, 0); 
    i = (int)SendDlgItemMessage(hdlg, dlIn, LB_GETITEMDATA, j, 0);
    if (i < 0) {
      GetEdit(deLoc, sz);
      if (!DisplayAtlasLookup(sz, 0, &i)) {
        PrintWarning("Please have a valid city selected in 'Atlas Lookups', "
          "or a valid city already in the 'Location' field.");
        return fTrue;
      }
    }
    SetEditSz(hdlg, deLoc, SzCity(i));
    ci = ciMain;
    if (!fDefault) {
      GetEdit(dcInMon, sz); ci.mon = NParseSz(sz, pmMon);
      GetEdit(dcInDay, sz); ci.day = NParseSz(sz, pmDay);
      GetEdit(dcInYea, sz); ci.yea = NParseSz(sz, pmYea);
      GetEdit(dcInTim, sz); ci.tim = RParseSz(sz, pmTim);
    }
    if (!DisplayTimezoneChanges(is.rgae[i].izn, 0, &ci))
      PrintWarning("Couldn't get time zone data!");
    SetEditSZOA(hdlg, dcDst, dcZon, dcLon, dcLat,
      ci.dst, ci.zon, is.rgae[i].lon, is.rgae[i].lat);
  }
  return fFalse;
}
#endif


/*
******************************************************************************
** Windows Dialogs.
******************************************************************************
*/

// Bring up the Windows standard file open dialog, and process the command
// file specified if any. This is called from the File \ Open Chart and
// File / Open Chart #2 menu commands.

flag API DlgOpenChart()
{
  char sz[cchSzDef];
  CI ciT;
  HCURSOR hcurOld;

  // Setup dialog title and settings and get the name of a file from it.
  if (us.fNoRead) {
    PrintWarning("File input is disabled.");
    return fFalse;
  }
  if (wi.nDlgChart > 0) {
    sprintf(sz, "Open Chart #%d", wi.nDlgChart);
    if (wi.nDlgChart < 2)
      sz[10] = chNull;
    ofn.lpstrFilter = "Astrolog Files (*.as)\0*.as\0"
      "Astrological Exchange Files (*.aaf)\0*.aaf\0All Files (*.*)\0*.*\0";
    sprintf(szFileName, "*.as");
  } else {
    sprintf(sz, "Open %s", wi.nDlgChart == 0 ? "Background" : "World Map");
    ofn.lpstrFilter = "Windows Bitmaps (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
    sprintf(szFileName, "*.bmp");
  }
  ofn.lpstrTitle = sz;
  if (!GetOpenFileName((LPOPENFILENAME)&ofn))
    return fFalse;

  // Process the given file based on what open command is being run.
  if (wi.nDlgChart <= 0) {
    if (wi.fHourglass)
      hcurOld = SetCursor(LoadCursor((HINSTANCE)NULL, IDC_WAIT));
    FLoadBmp(ofn.lpstrFileTitle,
      wi.nDlgChart == 0 ? &gi.bmpBack : &gi.bmpWorld, fFalse);
    if (wi.fHourglass)
      SetCursor(hcurOld);
    wi.fRedraw = fTrue;
    return fTrue;
  }
  ciT = ciCore;
  FInputData(ofn.lpstrFileTitle);
  if (wi.nDlgChart <= 1)
    cp1 = cp0;
  else {
    if (wi.nDlgChart == 2) {
      ciTwin = ciCore;
      cp2 = cp0;
    } else if (wi.nDlgChart == 3) {
      ciThre = ciCore;
      cp3 = cp0;
    } else if (wi.nDlgChart == 4) {
      ciFour = ciCore;
      cp4 = cp0;
    } else if (wi.nDlgChart == 5) {
      ciFive = ciCore;
      cp5 = cp0;
    } else {
      ciHexa = ciCore;
      cp6 = cp0;
    }
    ciCore = ciT;
  }

  wi.fCast = fTrue;
  return fTrue;
}


// Bring up the Windows standard file save dialog, get the name of a file from
// the user, and save to it either right away or set variables to ensure it
// will be done later. This is called from all six File / Save menu commands:
// Save Info, Positions, Text, Bitmap, Picture, PostScript, and Wireframe.

flag API DlgSaveChart()
{
  char *pch;

  // Setup dialog title and settings and get the name of a file from it.
  if (us.fNoWrite) {
    PrintWarning("File output is disabled.");
    return fFalse;
  }
  ofn.lpstrFilter = "Astrolog Files (*.as)\0*.as\0All Files (*.*)\0*.*\0";
  ofn.lpstrDefExt = "as";
  sprintf(szFileName, "*.as");
  switch (wi.wCmd) {
  case cmdSaveChart:
    ofn.lpstrTitle = "Save Chart Info";
    break;
  case cmdSavePositions:
    ofn.lpstrTitle = "Save Chart Positions";
    break;
  case cmdSaveText:
    if (!us.fTextHTML) {
      ofn.lpstrTitle = "Save Chart Text";
      ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
      ofn.lpstrDefExt = "txt";
      sprintf(szFileName, "*.txt");
    } else {
      ofn.lpstrTitle = "Save Chart HTML Text";
      ofn.lpstrFilter = "HTML Files (*.htm)\0*.txt\0All Files (*.*)\0*.*\0";
      ofn.lpstrDefExt = "htm";
      sprintf(szFileName, "*.htm");
    }
    break;
  case cmdSaveBitmap:
    ofn.lpstrTitle = "Save Chart Bitmap";
    ofn.lpstrFilter =
      "Windows Bitmaps (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "bmp";
    sprintf(szFileName, "*.bmp");
    break;
  case cmdSavePicture:
    ofn.lpstrTitle = "Save Chart Picture";
    ofn.lpstrFilter =
      "Windows Metafiles (*.wmf)\0*.wmf\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "wmf";
    sprintf(szFileName, "*.wmf");
    break;
  case cmdSavePS:
    ofn.lpstrTitle = "Save Chart PostScript";
    ofn.lpstrFilter =
      "PostScript Text (*.eps)\0*.eps\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "eps";
    sprintf(szFileName, "*.eps");
    break;
  case cmdSaveWire:
    ofn.lpstrTitle = "Save Chart Wireframe";
    ofn.lpstrFilter =
      "Daedalus Wireframes (*.dw)\0*.dw\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "dw";
    sprintf(szFileName, "*.dw");
    break;
  case cmdSaveAAF:
    ofn.lpstrTitle = "Save Chart AAF Format";
    ofn.lpstrFilter =
      "Astrological Exchange Files (*.aaf)\0*.aaf\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "aaf";
    sprintf(szFileName, "*.aaf");
    break;
  case cmdSaveSettings:
    ofn.lpstrTitle = "Save Settings";
    sprintf(szFileName, "%s", DEFAULT_INFOFILE);
    break;
  }
  if (wi.wCmd != cmdSaveWallTile && wi.wCmd != cmdSaveWallCenter &&
    wi.wCmd != cmdSaveWallStretch && wi.wCmd != cmdSaveWallFit &&
    wi.wCmd != cmdSaveWallFill) {
    if (!GetSaveFileName((LPOPENFILENAME)&ofn))
      return fFalse;
  } else {
    if (ofn.lpstrFile == NULL || ofn.lpstrFile == szFileName)
      ofn.lpstrFile = (char *)PAllocate(cchSzMax, "filename");
    if (ofn.lpstrFile == NULL)
      return fFalse;
    GetWindowsDirectory(ofn.lpstrFile, cchSzMax);
    pch = ofn.lpstrFile + CchSz(ofn.lpstrFile);
    sprintf(pch, "%c%s%s", '\\', szAppName, ".bmp");
  }

  // Saving chart info, position, or setting command files can be done here.
  // Saving actual chart output isn't done until the next redraw.
  is.szFileOut = gi.szFileOut = ofn.lpstrFile;
  switch (wi.wCmd) {
  case cmdSaveChart:
    us.fWritePos = fFalse;
    return FOutputData();
  case cmdSavePositions:
    us.fWritePos = fTrue;
    return FOutputData();
  case cmdSaveText:
    is.szFileScreen = ofn.lpstrFile;
    us.fGraphics = fFalse;
    wi.fRedraw = fTrue;
    break;
  case cmdSaveBitmap:
  case cmdSaveWallTile:
  case cmdSaveWallCenter:
  case cmdSaveWallStretch:
  case cmdSaveWallFit:
  case cmdSaveWallFill:
    gs.ft = ftBmp;
    us.fGraphics = wi.fRedraw = fTrue;
    break;
  case cmdSavePicture:
    gs.ft = ftWmf;
    us.fGraphics = wi.fRedraw = fTrue;
    break;
  case cmdSavePS:
    gs.ft = ftPS;
    us.fGraphics = wi.fRedraw = fTrue;
    break;
  case cmdSaveWire:
    gs.ft = ftWire;
    us.fGraphics = wi.fRedraw = fTrue;
    break;
  case cmdSaveAAF:
    return FOutputAAFFile();
  case cmdSaveSettings:
    return FOutputSettings();
  }
  return fTrue;
}


// Bring up the Windows standard print dialog, receive any printing settings
// from the user, and proceed to print the current graphics or text chart as
// displayed in the window. Called from the File / Print command.

flag API DlgPrint()
{
  DLGPROC lpAbortDlg;
  ABORTPROC lpAbortProc;
  HDC hdc;
  LPDEVMODE lpDevMode = NULL;
  LPDEVNAMES lpDevNames;
  LPSTR lpszDriverName;
  LPSTR lpszDeviceName;
  LPSTR lpszPortName;
  DOCINFO doci;
  int xPrint, yPrint;
  int xClient, yClient;
  flag fInverse;

  // Bring up the Windows print dialog.
  wi.fNoUpdate = fFalse;
  if (!PrintDlg((LPPRINTDLG)&prd))
    return fTrue;

  // Get the printer DC.
  if (prd.hDC)
    hdc = prd.hDC;
  else {
    // If the dialog didn't just return the DC, try to make one manually.
    if (!prd.hDevNames)
      return fFalse;
    lpDevNames = (LPDEVNAMES)GlobalLock(prd.hDevNames);
    lpszDriverName = (LPSTR)lpDevNames + lpDevNames->wDriverOffset;
    lpszDeviceName = (LPSTR)lpDevNames + lpDevNames->wDeviceOffset;
    lpszPortName = (LPSTR)lpDevNames + lpDevNames->wOutputOffset;
    GlobalUnlock(prd.hDevNames);
    if (prd.hDevMode)
      lpDevMode = (LPDEVMODE)GlobalLock(prd.hDevMode);
    hdc = CreateDC(lpszDriverName, lpszDeviceName, lpszPortName,
      /*(LPSTR)*/lpDevMode);
    if (prd.hDevMode && lpDevMode)
      GlobalUnlock(prd.hDevMode);
  }
  if (prd.hDevNames) {
    GlobalFree(prd.hDevNames);
    prd.hDevNames = (HGLOBAL)NULL;
  }
  if (prd.hDevMode) {
    GlobalFree(prd.hDevMode);
    prd.hDevMode = (HGLOBAL)NULL;
  }

  // Setup the Abort dialog and start the print job.
  lpAbortDlg = (DLGPROC)MakeProcInstance((FARPROC)DlgAbort, wi.hinst);
  lpAbortProc = (ABORTPROC)MakeProcInstance((FARPROC)DlgAbortProc, wi.hinst);
  SetAbortProc(hdc, lpAbortProc);
  doci.cbSize = sizeof(DOCINFO);
  doci.lpszDocName = szAppName;
  doci.lpszOutput = NULL;
  if (StartDoc(hdc, &doci) < 0) {
    FreeProcInstance(lpAbortDlg);
    FreeProcInstance(lpAbortProc);
    DeleteDC(hdc);
    return fFalse;
  }
  wi.fAbort = FALSE;
  wi.hwndAbort = CreateDialog(wi.hinst, MAKEINTRESOURCE(dlgAbort), wi.hwnd,
    lpAbortDlg);
  if (!wi.hwndAbort)
    return fFalse;
  ShowWindow(wi.hwndAbort, SW_NORMAL);
  EnableWindow(wi.hwnd, fFalse);
  StartPage(hdc);

  //  Scale the chart to the page.
  if (us.fGraphics) {
    gs.xWin *= METAMUL; gs.yWin *= METAMUL; gs.nScale *= METAMUL;
    fInverse = gs.fInverse;
    if (us.fSmartSave)
      gs.fInverse = fTrue;
  }
  SetMapMode(hdc, MM_ANISOTROPIC);       // So SetViewportExt can be called.
  xPrint = GetDeviceCaps(hdc, HORZRES);
  yPrint = GetDeviceCaps(hdc, VERTRES);
  SetViewportOrg(hdc, 0, 0);
  SetViewportExt(hdc, xPrint, yPrint);
  xClient = wi.xClient; yClient = wi.yClient;
  wi.xClient = gs.xWin;
  wi.yClient = NMultDiv(wi.xClient, yPrint, xPrint);
  if (gs.yWin > wi.yClient) {
    wi.yClient = gs.yWin;
    wi.xClient = NMultDiv(wi.yClient, xPrint, yPrint);
  }
  wi.hdcPrint = hdc;

  FRedraw();    // Actually go "draw" the chart to the printer.

  // Restore globals that were temporarily changed to print above.
  wi.hdcPrint = hdcNil;
  wi.xClient = xClient; wi.yClient = yClient;
  if (us.fGraphics) {
    gs.xWin /= METAMUL; gs.yWin /= METAMUL; gs.nScale /= METAMUL;
    gs.fInverse = fInverse;
  }

  // Finalize and cleanup everything.
  if (!wi.fAbort) {
    EndPage(hdc);
    EndDoc(hdc);
  }
  EnableWindow(wi.hwnd, fTrue);
  DestroyWindow(wi.hwndAbort);
  FreeProcInstance(lpAbortDlg);
  FreeProcInstance(lpAbortProc);
  DeleteDC(hdc);
  return fTrue;
}


// Message loop function for the printing abort dialog. Loops until printing
// is completed or the user hits cancel, returning which result.

flag API DlgAbortProc(HDC hdc, int nCode)
{
  MSG msg;

  if (wi.hwndAbort == (HWND)NULL)
    return fTrue;
  while (!wi.fAbort && PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE))
    if (!IsDialogMessage(wi.hwndAbort, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  return !wi.fAbort;
}


// Processing function for the printing abort modeless dialog, as brought up
// temporarily when printing via the File / Print menu command.

BOOL API DlgAbort(HWND hdlg, uint message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
  case WM_INITDIALOG:
    SetFocus(GetDlgItem(hdlg, IDCANCEL));
    return fFalse;

  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      wi.fAbort = fTrue;
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the command switch entry dialog, as brought up with
// the Edit / Enter Command Line menu command.

flag API DlgCommand(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];

  switch (message) {
  case WM_INITDIALOG:
    SetCheck(dxCo_e, !us.fExpOff);
    SetFocus(GetDlgItem(hdlg, deCo));
#ifndef EXPRESS
    ShowWindow(GetDlgItem(hdlg, dxCo_e), SW_HIDE);
#endif
    return fFalse;

  case WM_COMMAND:
    if (wParam == IDOK) {
      us.fExpOff = !GetCheck(dxCo_e);
      GetEdit(deCo, sz);
      FProcessCommandLine(sz);
      wi.fCast = wi.fMenuAll = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the color customization dialog, as brought up with
// the View / Set Colors menu command.

flag API DlgColor(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i, j, k, l;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 0; i < cColor; i++) {
      j = ikPalette[i];
      SetEditColor(hdlg, dck00 + i, j <= 0 ? kMainA[-j] : kRainbowA[j], 0);
    }
    for (i = 0; i < cElem; i++)
      SetEditColor(hdlg, dce0 + i, kElemA[i], 0);
    for (i = 1; i <= cRay; i++)
      SetEditColor(hdlg, dcr1 - 1 + i, kRayA[i], 0);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      for (k = 0; k <= 1; k++) {
        for (i = 0; i < cColor; i++) {
          GetEdit(dck00 + i, sz);
          l = NParseSz(sz, pmColor);
          if (k) {
            j = ikPalette[i];
            if (j <= 0)
              kMainA[-j] = l;
            else
              kRainbowA[j] = l;
          } else
            EnsureN(l, FValidColor(l), "palette color");
        }
        for (i = 0; i < cElem; i++) {
          GetEdit(dce0 + i, sz);
          l = NParseSz(sz, pmColor);
          if (k)
            kElemA[i] = l;
          else
            EnsureN(l, FValidColor(l), "element color");
        }
        for (i = 1; i <= cRay; i++) {
          GetEdit(dcr1 - 1 + i, sz);
          l = NParseSz(sz, pmColor);
          if (k)
            kRayA[i] = l;
          else
            EnsureN(l, FValidColor(l), "aspect color");
        }
      }
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the chart info entry dialog, as brought up by both
// the Info / Set Chart Info and Info / Set Chart #2 menu commands.

flag API DlgInfo(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  CI ci;
  char sz[cchSzMax];

  switch (message) {
  case WM_INITDIALOG:
    ci = *rgpci[wi.nDlgChart];
    if (wi.nDlgChart > 1) {
      sprintf(sz, "Enter Chart #%d Info", wi.nDlgChart);
      SetWindowText(hdlg, sz);
    }
    SetEditSz(hdlg, deInNam, ci.nam);
    SetEditSz(hdlg, deInLoc, ci.loc);
LInit:
    SetEditMDYT(hdlg, dcInMon, dcInDay, dcInYea, dcInTim,
      ci.mon, ci.day, ci.yea, ci.tim);
    SetEditSZOA(hdlg, dcInDst, dcInZon, dcInLon, dcInLat,
      ci.dst != dstAuto ? ci.dst : (real)is.fDst, ci.zon, ci.lon, ci.lat);
    SetFocus(GetDlgItem(hdlg, dcInMon));
#ifndef ATLAS
    ShowWindow(GetDlgItem(hdlg, dbInCity), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dbInCoor), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dbInChan), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dbInAppl), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dlIn), SW_HIDE);
#endif
    return fFalse;

  case WM_COMMAND:
    if (wParam == dbInNow || wParam == dbInSet) {
#ifdef TIME
      if (wParam == dbInNow) {
        GetTimeNow(&ci.mon, &ci.day, &ci.yea, &ci.tim, us.dstDef, us.zonDef);
        ci.dst = us.dstDef; ci.zon = us.zonDef;
        ci.lon = us.lonDef; ci.lat = us.latDef;
      } else
#endif
        ci = ciSave;
      goto LInit;
    }
#ifdef ATLAS
    if (FDlgInfoAtlas(hdlg, wParam, fFalse))
      break;
#endif
    if (wParam == IDOK) {
      GetEdit(dcInMon, sz); ci.mon = NParseSz(sz, pmMon);
      GetEdit(dcInDay, sz); ci.day = NParseSz(sz, pmDay);
      GetEdit(dcInYea, sz); ci.yea = NParseSz(sz, pmYea);
      GetEdit(dcInTim, sz); ci.tim = RParseSz(sz, pmTim);
      GetEdit(dcInDst, sz); ci.dst = RParseSz(sz, pmDst);
      GetEdit(dcInZon, sz); ci.zon = RParseSz(sz, pmZon);
      GetEdit(dcInLon, sz); ci.lon = RParseSz(sz, pmLon);
      GetEdit(dcInLat, sz); ci.lat = RParseSz(sz, pmLat);
      EnsureN(ci.mon, FValidMon(ci.mon), "month");
      EnsureN(ci.yea, FValidYea(ci.yea), "year");
      EnsureN(ci.day, FValidDay(ci.day, ci.mon, ci.yea), "day");
      EnsureR(ci.tim, FValidTim(ci.tim), "time");
      EnsureR(ci.dst, FValidZon(ci.dst), "daylight saving");
      EnsureR(ci.zon, FValidZon(ci.zon), "time zone");
      EnsureR(ci.lon, FValidLon(ci.lon), "longitude");
      EnsureR(ci.lat, FValidLat(ci.lat), "latitude");
      GetEdit(deInNam, sz);
      ci.nam = SzPersist(sz);
      GetEdit(deInLoc, sz);
      ci.loc = SzPersist(sz);
      *rgpci[wi.nDlgChart] = ci;
      if (wi.nDlgChart == 1)
        ciCore = ci;
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the default chart info dialog, as brought up with
// the Info / Default Chart Info menu command.

flag API DlgDefault(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  CI ci;
  char sz[cchSzMax];

  switch (message) {
  case WM_INITDIALOG:
    SetEditSZOA(hdlg, dcDeDst, dcDeZon, dcDeLon, dcDeLat,
      us.dstDef, us.zonDef, us.lonDef, us.latDef);
    SetCombo(dcDeDst, "Autodetect");
    SetEditN(dcDeCor, (int)us.lTimeAddition);
    SetCombo(dcDeCor, "60"); SetCombo(dcDeCor, "0"); SetCombo(dcDeCor, "-60");
    SetEditSz(hdlg, dcDeElv, SzElevation(us.elvDef));
    SetCombo(dcDeElv, "0m"); SetCombo(dcDeElv, "1000ft");
    SetEditSz(hdlg, deDeNam, us.namDef);
    SetEditSz(hdlg, deDeLoc, us.locDef);
    SetFocus(GetDlgItem(hdlg, dcDeDst));
#ifndef ATLAS
    ShowWindow(GetDlgItem(hdlg, dbInCity), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dbInCoor), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dbInChan), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dbInAppl), SW_HIDE);
    ShowWindow(GetDlgItem(hdlg, dlIn), SW_HIDE);
#endif
    return fFalse;

  case WM_COMMAND:
    if (wParam == IDOK) {
      GetEdit(dcDeDst, sz); ci.dst = RParseSz(sz, pmDst);
      GetEdit(dcDeZon, sz); ci.zon = RParseSz(sz, pmZon);
      GetEdit(dcDeLon, sz); ci.lon = RParseSz(sz, pmLon);
      GetEdit(dcDeLat, sz); ci.lat = RParseSz(sz, pmLat);
      GetEdit(dcDeCor, sz); us.lTimeAddition = NFromSz(sz);
      GetEdit(dcDeElv, sz); us.elvDef = RParseSz(sz, pmElv);
      EnsureR(ci.dst, FValidZon(ci.dst), "daylight saving");
      EnsureR(ci.zon, FValidZon(ci.zon), "time zone");
      EnsureR(ci.lon, FValidLon(ci.lon), "longitude");
      EnsureR(ci.lat, FValidLat(ci.lat), "latitude");
      us.dstDef = ci.dst; us.zonDef = ci.zon;
      us.lonDef = ci.lon; us.latDef = ci.lat;
      GetEdit(deDeNam, sz); us.namDef = SzPersist(sz);
      GetEdit(deDeLoc, sz); us.locDef = SzPersist(sz);
      wi.fCast = fTrue;
    }
#ifdef ATLAS
    if (FDlgInfoAtlas(hdlg, wParam, fTrue))
      break;
#endif
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the charts #3 through #6 info entry dialog, as
// brought up by the Info / Charts #3 through #6 menu command.

flag API DlgInfoAll(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  DLGPROC dlgproc;
  int i;

  switch (message) {
  case WM_INITDIALOG:
    i = us.nRel;
    if (i > rcDual)
      i = 0;
    else if (i < rcHexaWheel)
      i = rcDual;
    SetRadio(dr01-i, dr01, dr06);
    return fTrue;

  case WM_COMMAND:
    if (FBetween(wParam, dbIa_o1, dbIa_o6)) {
      wi.nDlgChart = wParam - dbIa_o1 + 1;
      DlgOpenChart();
    } else if (FBetween(wParam, dbIa_i1, dbIa_i6)) {
      wi.nDlgChart = wParam - dbIa_i1 + 1;
      WiDoDialog(DlgInfo, dlgInfo);
    }
    if (wParam == IDOK) {
      i = GetRadio(hdlg, dr01, dr06) + 1;
      SetRel(-(i-1));
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the aspect settings dialog, as brought up with the
// Setting / Aspect Settings menu command.

flag API DlgAspect(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i, j, k;
  real r;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 1; i <= cAspect; i++) {
      SetCheck(dxa01 - 1 + i, ignorea[i]);
      SetEditR(hdlg, deo01 - 1 + i, rAspOrb[i], -6);
      SetEditR(hdlg, dea01 - 1 + i, rAspAngle[i], -6);
      SetEditR(hdlg, dei01 - 1 + i, rAspInf[i], 2);
      SetEditColor(hdlg, dck01 - 1 + i, kAspA[i], 0);
    }
    return fTrue;

  case WM_COMMAND:
    switch (wParam) {
    case dbAs_RA0:
      for (i = 1; i <= cAspect; i++)
        SetCheck(dxa01 - 1 + i, fTrue);
      break;
    case dbAs_RA1:
      for (i = 1; i <= cAspect; i++)
        SetCheck(dxa01 - 1 + i, fFalse);
      break;
    case dbAs_RA:
      for (i = 1; i <= 5; i++)
        SetCheck(dxa01 - 1 + i, !GetCheck(dxa01 - 1 + i));
      break;
    }

    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i = 1; i <= cAspect; i++) {
          r = GetEditR(hdlg, deo01 - 1 + i);
          if (j)
            rAspOrb[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "orb");
          r = GetEditR(hdlg, dea01 - 1 + i);
          if (j)
            rAspAngle[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "angle");
          if (j) {
            r = GetEditR(hdlg, dei01 - 1 + i);
            rAspInf[i] = r;
          }
          GetEdit(dck01 - 1 + i, sz);
          k = NParseSz(sz, pmColor);
          if (j)
            kAspA[i] = k;
          else
            EnsureN(k, FValidColor(k), "color");
        }
      }
      for (i = 1; i <= cAspect; i++)
        ignorea[i] = GetCheck(dxa01 - 1 + i);
      for (us.nAsp = cAspect; us.nAsp > 0 && ignorea[us.nAsp]; us.nAsp--)
        ;
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the object settings dialog, as brought up with the
// Setting / Object Settings menu command.

flag API DlgObject(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i, j, k;
  real r;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 0; i <= oCore; i++) {
      SetEditR(hdlg, deo01 + i, rObjOrb[i], -2);
      SetEditR(hdlg, dea01 + i, rObjAdd[i], -1);
      SetEditR(hdlg, dei01 + i, rObjInf[i], -2);
      SetEditColor(hdlg, dck00 + i, kObjU[i], 1);
    }
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i = 0; i <= oCore; i++) {
          r = GetEditR(hdlg, deo01 + i);
          if (j)
            rObjOrb[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "max orb");
          r = GetEditR(hdlg, dea01 + i);
          if (j)
            rObjAdd[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "orb addition");
          r = GetEditR(hdlg, dei01 + i);
          if (j)
            rObjInf[i] = r;
          GetEdit(dck00 + i, sz);
          k = NParseSz(sz, pmColor);
          if (j)
            kObjU[i] = k;
          else
            EnsureN(k, FValidColor2(k), "color");
        }
      }
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the cusp and Uranian object settings dialog, as
// brought up with the Setting / More Object Settings menu command.

flag API DlgObject2(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i0, i, j, k;
  real r;

  switch (message) {
  case WM_INITDIALOG:
    for (i0 = oAsc; i0 <= dwarfHi+1; i0++) {
      i = (i0 <= dwarfHi ? i0 : starLo);
      SetEditR(hdlg, deo01 - oAsc + i0, rObjOrb[i], -2);
      SetEditR(hdlg, dea01 - oAsc + i0, rObjAdd[i], -1);
      SetEditR(hdlg, dei01 - oAsc + i0, rObjInf[i], -2);
      SetEditColor(hdlg, dck00 - oAsc + i0, kObjU[i], 1 + (i == starLo));
    }
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i0 = oAsc; i0 <= dwarfHi+1; i0++) {
          i = (i0 <= dwarfHi ? i0 : starLo);
          r = GetEditR(hdlg, deo01 - oAsc + i0);
          if (j)
            rObjOrb[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "max orb");
          r = GetEditR(hdlg, dea01 - oAsc + i0);
          if (j)
            rObjAdd[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "orb addition");
          r = GetEditR(hdlg, dei01 - oAsc + i0);
          if (j)
            rObjInf[i] = r;
          GetEdit(dck00 - oAsc + i, sz);
          k = NParseSz(sz, pmColor);
          if (j)
            kObjU[i] = k;
          else
            EnsureN(k, i < starLo ? FValidColor2(k) : FValidColorS(k),
              "color");
        }
      }
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the planetary moons object settings dialog, as
// brought up with the Setting / Planetary Moons / Moon Object Settings menu
// command.

flag API DlgObjectM(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i, j, k;
  real r;

  switch (message) {
  case WM_INITDIALOG:
    for (i = moonsLo; i <= cobHi; i++) {
      SetEditR(hdlg, deo01 - moonsLo + i, rObjOrb[i], -2);
      SetEditR(hdlg, dea01 - moonsLo + i, rObjAdd[i], -1);
      SetEditR(hdlg, dei01 - moonsLo + i, rObjInf[i], -2);
      SetEditColor(hdlg, dck00 - moonsLo + i, kObjU[i], 3);
    }
    SetCheck(dxMo_Ym, us.fMoonMove);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i = moonsLo; i <= cobHi; i++) {
          r = GetEditR(hdlg, deo01 - moonsLo + i);
          if (j)
            rObjOrb[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "max orb");
          r = GetEditR(hdlg, dea01 - moonsLo + i);
          if (j)
            rObjAdd[i] = r;
          else
            EnsureR(r, r >= -rDegMax && r <= rDegMax, "orb addition");
          r = GetEditR(hdlg, dei01 - moonsLo + i);
          if (j)
            rObjInf[i] = r;
          GetEdit(dck00 - moonsLo + i, sz);
          k = NParseSz(sz, pmColor);
          if (j)
            kObjU[i] = k;
          else
            EnsureN(k, FValidColorM(k), "color");
        }
      }
      us.fMoonMove = GetCheck(dxMo_Ym);
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the custom objects dialog, as brought up with the
// Setting / Planetary Moons / Object Customization menu command.

flag API DlgCustom(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int i, j;
#ifdef SWISS
  char *pch, *pchEnd;
  int k, l;
  real rT;
#endif

  switch (message) {
  case WM_INITDIALOG:
    for (i = custLo; i <= custHi; i++) {
      SetEdit(den01 - custLo + i, szObjDisp[i]);
#ifdef SWISS
      j = rgTypSwiss[i - custLo];
      k = rgObjSwiss[i - custLo];
      if (j != 2)
        sprintf(sz, "%s%d",
          j <= 0 ? "h" : (j == 1 ? "" : (j == 3 ? "m" : "j")), k);
      else
        sprintf(sz, "%.3s", szObjName[k]);
      for (pch = sz; *pch; pch++)
        ;
      j = rgPntSwiss[i - custLo];
      k = rgFlgSwiss[i - custLo];
      if (j > 0 || k > 0)
        *pch++ = ' ';
      if (j > 0)
        *pch++ = (j == 1 ? 'n' : (j == 2 ? 's' : (j == 3 ? 'p' : 'a')));
      if (k &  1) *pch++ = 'H';
      if (k &  2) *pch++ = 'S';
      if (k &  4) *pch++ = 'B';
      if (k &  8) *pch++ = 'N';
      if (k & 16) *pch++ = 'T';
      if (k & 32) *pch++ = 'V';
      *pch = chNull;
      SetEdit(ded01 - custLo + i, sz);
#endif
    }
#ifndef SWISS
    ShowWindow(GetDlgItem(hdlg, dbCu_l), SW_HIDE);
    for (i = custLo; i <= custHi; i++)
      ShowWindow(GetDlgItem(hdlg, ded01 - custLo + i), SW_HIDE);
#endif
    return fTrue;

  case WM_COMMAND:
#ifdef SWISS
    if (wParam == dbCu_l)
      for (i = custLo; i <= custHi; i++) {
        GetEdit(den01 - custLo + i, sz);
        if (*sz && NCompareSz(sz, szObjUnknown) != 0)
          continue;
        GetEdit(ded01 - custLo + i, sz);
        for (pch = sz; *pch; pch++)
          ;
        pchEnd = pch;
        for (pch--; pch > sz && *pch >= 'A'; pch--)
          ;
        if (pch >= sz && *pch < '0')
          *pch = chNull;
        pch = sz;
        k = (*pch == 'h' ? 0 : (*pch == 'm' ? 3 : (*pch == 'j' ? 4 :
          (FNumCh(*pch) ? 1 : 2))));
        if (k == 0 || k >= 3)
          pch++;
        l = (k == 2 ? NParseSz(pch, pmObject) : NFromSz(pch));
        switch (k) {
        case 0:
          SwissGetObjName(sz, -l);
          break;
        case 1:
          SwissGetObjName(sz, l);
          break;
        case 2:
          sprintf(sz, "%s", szObjName[l]);
          break;
        case 3:
          l = ObjMoons(l);
          sprintf(sz, "%s", l >= 0 ? szObjName[l] : szObjUnknown);
          break;
        case 4:
#ifdef JPLWEB
          if (!GetJPLHorizons(l, &rT, &rT, &rT, &rT, &rT, &rT, sz))
#endif
            sprintf(sz, "%s", szObjUnknown);
          break;
        }
        SetEdit(den01 - custLo + i, sz);
      }
#endif

    if (wParam == IDOK) {
      for (j = 0; j <= 1; j++) {
        for (i = custLo; i <= custHi; i++) {
#ifdef SWISS
          GetEdit(ded01 - custLo + i, sz);
          for (pch = sz; *pch; pch++)
            ;
          pchEnd = pch;
          for (pch--; pch > sz && *pch >= 'A'; pch--)
            ;
          if (pch >= sz && *pch < '0')
            *pch = chNull;
          pch = sz;
          k = (*pch == 'h' ? 0 : (*pch == 'm' ? 3 : (*pch == 'j' ? 4 :
            (FNumCh(*pch) ? 1 : 2))));
          if (k == 0 || k >= 3)
            pch++;
          l = (k == 2 ? NParseSz(pch, pmObject) : NFromSz(pch));
          EnsureN(l, l > 0 || k >= 4, "definition");
#endif
          if (j) {
#ifdef SWISS
            rgTypSwiss[i - custLo] = k;
            rgObjSwiss[i - custLo] = l;
            k = l = 0;
            for (pch = pchEnd-1; pch > sz && *pch >= 'A'; pch--) {
              if (*pch == 'n') k = 1;
              if (*pch == 's') k = 2;
              if (*pch == 'p') k = 3;
              if (*pch == 'a') k = 4;
              if (*pch == 'H') l |= 1;
              if (*pch == 'S') l |= 2;
              if (*pch == 'B') l |= 4;
              if (*pch == 'N') l |= 8;
              if (*pch == 'T') l |= 16;
              if (*pch == 'V') l |= 32;
            }
            rgPntSwiss[i - custLo] = k;
            rgFlgSwiss[i - custLo] = l;
#endif
            GetEdit(den01 - custLo + i, sz);
            if (NCompareSz(sz, szObjDisp[i]))
              szObjDisp[i] = SzPersist(sz);
          }
        }
      }
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the object restrictions dialog, as invoked with
// both the Setting / Restrictions and Transit Restrictions menu commands.

flag API DlgRestrict(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  byte *lpb, *lpb2;
  int i;

  switch (message) {
  case WM_INITDIALOG:
    if (wi.wCmd == cmdRes)
      lpb = ignore;
    else {
      SetWindowText(hdlg, "Transit Object Restrictions");
      SetDlgItemText(hdlg, dbRT, "Copy &From Standard Restriction Set");
      lpb = ignore2;
    }
    for (i = 0; i <= dwarfHi; i++)
      SetCheck(dx01 + i, lpb[i]);
    return fTrue;

  case WM_COMMAND:
    switch (wParam) {
    case dbRe_R0:
      for (i = 0; i <= dwarfHi; i++)
        SetCheck(dx01 + i, fTrue);
      break;
    case dbRe_R1:
      for (i = 0; i <= dwarfHi; i++)
        SetCheck(dx01 + i, fFalse);
      break;
    case dbRe_R:
      for (i = oMain+1; i <= oCore; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbRe_RC:
      for (i = cuspLo; i <= cuspHi; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbRe_Ru:
      for (i = uranLo; i <= uranHi; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbRe_Ry:
      for (i = dwarfLo; i <= dwarfHi; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbRT:
      lpb2 = wi.wCmd == cmdRes ? ignore2 : ignore;
      for (i = 0; i <= dwarfHi; i++)
        SetCheck(dx01 + i, lpb2[i]);
      break;
    }

    if (wParam == IDOK) {
      lpb = wi.wCmd == cmdRes ? ignore : ignore2;
      for (i = 0; i <= dwarfHi; i++)
        lpb[i] = GetCheck(dx01 + i);
      if (!us.fCusp) {
        for (i = cuspLo; i <= cuspHi; i++)
          if (!ignore[i] || !ignore2[i]) {
            us.fCusp = fTrue;
            WiCheckMenu(cmdResCusp, fTrue);
            break;
          }
      } else {
        for (i = cuspLo; i <= cuspHi; i++)
          if (!ignore[i] || !ignore2[i])
            break;
        if (i > cuspHi) {
          us.fCusp = fFalse;
          WiCheckMenu(cmdResCusp, fFalse);
        }
      }
      if (!us.fUranian) {
        for (i = uranLo; i <= uranHi; i++)
          if (!ignore[i] || !ignore2[i]) {
            us.fUranian = fTrue;
            WiCheckMenu(cmdResUranian, fTrue);
            break;
          }
      } else {
        for (i = uranLo; i <= uranHi; i++)
          if (!ignore[i] || !ignore2[i])
            break;
        if (i > uranHi) {
          us.fUranian = fFalse;
          WiCheckMenu(cmdResUranian, fFalse);
        }
      }
      if (!us.fDwarf) {
        for (i = dwarfLo; i <= dwarfHi; i++)
          if (!ignore[i] || !ignore2[i]) {
            us.fDwarf = fTrue;
            WiCheckMenu(cmdResDwarf, fTrue);
            break;
          }
      } else {
        for (i = dwarfLo; i <= dwarfHi; i++)
          if (!ignore[i] || !ignore2[i])
            break;
        if (i > dwarfHi) {
          us.fDwarf = fFalse;
          WiCheckMenu(cmdResDwarf, fFalse);
        }
      }
      AdjustRestrictions();
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the star restrictions dialog, as brought up with
// the Setting / Star Restrictions menu command.

flag API DlgStar(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  int i;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 0; i < cStar; i++)
      SetCheck(dx01 + i, ignore[starLo + i]);
    return fTrue;

  case WM_COMMAND:
    switch (wParam) {
    case dbSt_RU0:
      for (i = 0; i < cStar; i++)
        SetCheck(dx01 + i, fTrue);
      break;
    case dbSt_RU1:
      for (i = 0; i < cStar; i++)
        SetCheck(dx01 + i, fFalse);
      break;
    }

    if (wParam == IDOK) {
      for (i = 0; i < cStar; i++)
        ignore[starLo + i] = GetCheck(dx01 + i);
      if (!us.nStar) {
        for (i = starLo; i <= starHi; i++)
          if (!ignore[i]) {
            us.nStar = fTrue;
            WiCheckMenu(cmdResStar, fTrue);
            break;
          }
      } else {
        for (i = starLo; i <= starHi; i++)
          if (!ignore[i])
            break;
        if (i > starHi) {
          us.nStar = fFalse;
          WiCheckMenu(cmdResStar, fFalse);
        }
      }
      AdjustRestrictions();
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the planetary moons restrictions dialog, as brought
// up with the Setting / Planetary Moons / Moon Restrictions menu command.

flag API DlgMoons(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  int i;

  switch (message) {
  case WM_INITDIALOG:
    for (i = 0; i < cMoons2; i++)
      SetCheck(dx01 + i, ignore[moonsLo + i]);
    return fTrue;

  case WM_COMMAND:
    switch (wParam) {
    case dbMo_Rm0:
      for (i = 0; i < cMoons2; i++)
        SetCheck(dx01 + i, fTrue);
      break;
    case dbMo_Rm1:
      for (i = 0; i < cMoons2; i++)
        SetCheck(dx01 + i, fFalse);
      break;
    case dbMo_Mar:
      for (i = 0; i <= 1; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbMo_Jup:
      for (i = 2; i <= 5; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbMo_Sat:
      for (i = 6; i <= 13; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbMo_Ura:
      for (i = 14; i <= 18; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbMo_Nep:
      for (i = 19; i <= 21; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbMo_Plu:
      for (i = 22; i <= 26; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    case dbMo_COB:
      for (i = 27; i <= 31; i++)
        SetCheck(dx01 + i, !GetCheck(dx01 + i));
      break;
    }

    if (wParam == IDOK) {
      for (i = 0; i < cMoons2; i++)
        ignore[moonsLo + i] = GetCheck(dx01 + i);
      if (!us.fMoons) {
        for (i = moonsLo; i <= moonsHi; i++)
          if (!ignore[i] || !ignore2[i]) {
            us.fMoons = fTrue;
            WiCheckMenu(cmdResMoons, fTrue);
            break;
          }
      } else {
        for (i = moonsLo; i <= moonsHi; i++)
          if (!ignore[i] || !ignore2[i])
            break;
        if (i > moonsHi) {
          us.fMoons = fFalse;
          WiCheckMenu(cmdResMoons, fFalse);
        }
      }
      if (!us.fCOB) {
        for (i = cobLo; i <= cobHi; i++)
          if (!ignore[i] || !ignore2[i]) {
            us.fCOB = fTrue;
            WiCheckMenu(cmdResCOB, fTrue);
            break;
          }
      } else {
        for (i = cobLo; i <= cobHi; i++)
          if (!ignore[i] || !ignore2[i])
            break;
        if (i > cobHi) {
          us.fCOB = fFalse;
          WiCheckMenu(cmdResCOB, fFalse);
        }
      }
      AdjustRestrictions();
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the standard calculation settings dialog, as
// brought up with the Setting / Calculation Settings menu command.

flag API DlgCalc(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax], *pch;
  real rs, rx;
  int nh, n4, n1, i;

  switch (message) {
  case WM_INITDIALOG:
    SetEditR(hdlg, dcSe_s, us.rZodiacOffset, 6);
    for (i = 0; *rgZodiacOffset[i].sz; i++) {
      FormatR(sz, rgZodiacOffset[i].r, 6);
      for (pch = sz; *pch; pch++)
        ;
      sprintf(pch, " %s", rgZodiacOffset[i].sz);
      SetCombo(dcSe_s, sz);
      if (us.rZodiacOffset == rgZodiacOffset[i].r)
        SetEdit(dcSe_s, sz);
    }
    SetEditR(hdlg, deSe_x, us.rHarmonic, -6);
    SetEditN(deSe_4, us.nDwad);
    SetCheck(dxSe_Yh, us.fBarycenter);
    SetCheck(dxSe_Yn, us.fTrueNode);
    SetCheck(dxSe_Yc0, us.fHouseAngle);
    SetCheck(dxSe_ys, us.fSidereal2);
    SetCheck(dxSe_Yn0, us.fNoNutation);
    SetEdit(deSe_h, szObjName[us.objCenter]);
    SetRadio(us.objOnAsc == 0 ? dr01 : (us.objOnAsc > 0 ? dr02 : dr03),
      dr01, dr03);
    SetEdit(deSe_1, szObjName[us.objOnAsc == 0 ? oSun : NAbs(us.objOnAsc)-1]);
    SetCheck(dxSe_10, us.fSolarWhole);
    SetCheck(dxSe_sr0, us.fEquator2);
    SetCheck(dxSe_sr, us.fEquator);
    SetCheck(dxSe_yt, us.fTruePos);
    SetCheck(dxSe_yv, us.fTopoPos);
    SetCheck(dxSe_A3, us.fAspect3D);
    SetCheck(dxSe_Ap, us.fAspectLat);
    SetCheck(dxSe_c3, us.fHouse3D);
    SetRadio(dr03 + us.nHouse3D, dr04, dr06);
#ifdef SWISS
    SetCombo(dcSe_b, szEphem[cmSwiss]);
    SetCombo(dcSe_b, szEphem[cmMoshier]);
    SetCombo(dcSe_b, szEphem[cmJPL]);
#endif
#ifdef JPLWEB
    if (!us.fNoNetwork)
      SetCombo(dcSe_b, szEphem[cmJPLWeb]);
#endif
#ifdef PLACALC
    SetCombo(dcSe_b, szEphem[cmPlacalc]);
#endif
#ifdef MATRIX
    SetCombo(dcSe_b, szEphem[cmMatrix]);
#endif
    SetCombo(dcSe_b, szEphem[cmNone]);
    i = FCmSwissEph() ? cmSwiss : (FCmSwissMosh() ? cmMoshier :
      (FCmSwissJPL() ? cmJPL : (FCmPlacalc() ? cmPlacalc :
      (FCmMatrix() ? cmMatrix : (FCmJPLWeb() ? cmJPLWeb : cmNone)))));
    SetEdit(dcSe_b, szEphem[i]);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      rs = GetEditR(hdlg, dcSe_s);
      GetEdit(deSe_h, sz); nh = NParseSz(sz, pmObject);
      rx = GetEditR(hdlg, deSe_x);
      n4 = GetEditN(hdlg, deSe_4);
      GetEdit(deSe_1, sz); n1 = NParseSz(sz, pmObject);
      EnsureR(rs, FValidOffset(rs), "zodiac offset");
      EnsureN(nh, FValidCenter(nh), "central planet");
      EnsureR(rx, FValidHarmonic(rx), "harmonic factor");
      EnsureN(n4, FValidDwad(n4), "dwad nesting");
      EnsureN(n1, FItem(n1), "Solar chart planet");
      us.rZodiacOffset = rs;
      SetCentric(nh);
      WiCheckMenu(cmdHeliocentric, us.objCenter != oEar);
      us.rHarmonic = rx;
      us.nDwad = n4;
      WiCheckMenu(cmdHouseSetDwad, us.nDwad > 0);
      us.fBarycenter = GetCheck(dxSe_Yh);
      us.fTrueNode = GetCheck(dxSe_Yn);
      us.fHouseAngle = GetCheck(dxSe_Yc0);
      us.fSidereal2 = GetCheck(dxSe_ys);
      us.fNoNutation = GetCheck(dxSe_Yn0);
      us.fAspect3D = GetCheck(dxSe_A3);
      us.fAspectLat = GetCheck(dxSe_Ap);
      us.objOnAsc = GetCheck(dr01) ? 0 : (GetCheck(dr02) ? n1+1 : -n1-1);
      WiCheckMenu(cmdHouseSetSolar, us.objOnAsc);
      us.fSolarWhole = GetCheck(dxSe_10);
      us.fEquator2 = GetCheck(dxSe_sr0);
      us.fEquator = GetCheck(dxSe_sr);
      us.fTruePos = GetCheck(dxSe_yt);
      us.fTopoPos = GetCheck(dxSe_yv);
      us.fHouse3D = GetCheck(dxSe_c3);
      us.nHouse3D = GetRadio(hdlg, dr04, dr06) + 1;
      WiCheckMenu(cmdHouseSet3D, us.fHouse3D);
      wi.fCast = fTrue;
      GetEdit(dcSe_b, sz);
      us.fEphemFiles = us.fPlacalcPla = us.fMatrixPla = fFalse;
      us.nSwissEph = 0;
#ifdef SWISS
      if (FMatchSz(sz, szEphem[cmSwiss])) {
        us.fEphemFiles = fTrue; us.nSwissEph = 0;
      } else if (FMatchSz(sz, szEphem[cmMoshier])) {
        us.fEphemFiles = fTrue; us.nSwissEph = 1;
      } else if (FMatchSz(sz, szEphem[cmJPL])) {
        us.fEphemFiles = fTrue; us.nSwissEph = 2;
      } else if (FMatchSz(sz, szEphem[cmJPLWeb])) {
        us.fEphemFiles = fTrue; us.nSwissEph = 3;
      }
#endif
#ifdef PLACALC
      if (FMatchSz(sz, szEphem[cmPlacalc]))
        us.fEphemFiles = us.fPlacalcPla = fTrue;
#endif
#ifdef MATRIX
      if (FMatchSz(sz, szEphem[cmMatrix]))
        us.fMatrixPla = fTrue;
#endif
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the display settings dialog, as brought up with the
// Setting / Display Settings menu command.

flag API DlgDisplay(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int na, ni, nro, i;

  switch (message) {
  case WM_INITDIALOG:
    SetCheck(dxDi_Yr, us.fRound);
    SetCheck(dxDi_Yd, us.fEuroDate);
    SetCheck(dxDi_Yt, us.fEuroTime);
    SetCheck(dxDi_Yv, us.fEuroDist);
    SetCheck(dxDi_YC, us.fSmartCusp);
    SetCheck(dxDi_Y8, us.fClip80);
    SetCheck(dxDi_YR0_s, us.fIgnoreSign);
    SetCheck(dxDi_YR0_d, us.fIgnoreDir);
    SetCheck(dxDi_YR1_l, us.fIgnoreDiralt);
    SetCheck(dxDi_YR1_d, us.fIgnoreDirlen);
    if (us.objRequire >= 0)
      SetEdit(deDi_RO, szObjName[us.objRequire]);
    else
      SetEdit(deDi_RO, "None");
    SetCheck(dxDi_YOO, us.fSmartSave);
    SetCheck(dxDi_kh, us.fTextHTML);
    SetCheck(dxDi_Yo, us.fWriteOld);
    SetCheck(dxDi_YXf, gs.nFont > 0);
    SetEdit(deDi_YXp0_x, SzLength(gs.xInch));
    SetEdit(deDi_YXp0_y, SzLength(gs.yInch));
    SetRadio(gs.nOrient == 0 ? dr03 : (gs.nOrient > 0 ? dr01 : dr02),
      dr01, dr03);
    SetRadio(dr04 + us.nDegForm, dr04, dr07);
    for (i = 0; i < 5; i++)
      SetCheck(dxDi_r0 + i, ignore7[i]);
    SetEditN(deDi_A, us.nAsp);
    SetEditN(deDi_I, us.nScreenWidth);
    SetCheck(dxDi_Yu, us.fEclipse);
    SetCheck(dxDi_Yu0, !us.fEclipseAny);
    for (i = 0; i < 4; i++)
      SetCheck(dxDi_z0 + i, ignorez[i]);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      GetEdit(deDi_A, sz); na = NParseSz(sz, pmAspect);
      ni = GetEditN(hdlg, deDi_I);
      GetEdit(deDi_RO, sz); nro = NParseSz(sz, pmObject);
      EnsureN(na, FValidAspect(na), "aspect count");
      EnsureN(ni, FValidScreen(ni), "text columns");
      EnsureN(nro, FItem(nro) || nro == -1, "required object");
      us.fRound = GetCheck(dxDi_Yr);
      us.fEuroDate = GetCheck(dxDi_Yd);
      us.fEuroTime = GetCheck(dxDi_Yt);
      us.fEuroDist = GetCheck(dxDi_Yv);
      us.fSmartCusp = GetCheck(dxDi_YC);
      us.fClip80 = GetCheck(dxDi_Y8);
      us.fIgnoreSign = GetCheck(dxDi_YR0_s);
      us.fIgnoreDir = GetCheck(dxDi_YR0_d);
      us.fIgnoreDiralt = GetCheck(dxDi_YR1_l);
      us.fIgnoreDirlen = GetCheck(dxDi_YR1_d);
      us.objRequire = nro;
      us.fSmartSave = GetCheck(dxDi_YOO);
      us.fTextHTML = GetCheck(dxDi_kh);
      us.fWriteOld = GetCheck(dxDi_Yo);
      gs.nFont = GetCheck(dxDi_YXf) * gi.nFontPrev;
      GetEdit(deDi_YXp0_x, sz); gs.xInch = RParseSz(sz, pmLength);
      GetEdit(deDi_YXp0_y, sz); gs.yInch = RParseSz(sz, pmLength);
      gs.nOrient = GetCheck(dr03) ? 0 : (GetCheck(dr01) ? 1 : -1);
      us.nDegForm = GetCheck(dr04) ? 0 : (GetCheck(dr05) ? 1 :
        (GetCheck(dr06) ? 2 : 3));
      for (i = 0; i < 5; i++)
        ignore7[i] = GetCheck(dxDi_r0 + i);
      for (i = us.nAsp + 1; i <= na; i++)
        ignorea[i] = fFalse;
      for (i = na + 1; i <= cAspect; i++)
        ignorea[i] = fTrue;
      us.nAsp = na;
      us.nScreenWidth = ni;
      us.fEclipse = GetCheck(dxDi_Yu);
      us.fEclipseAny = !GetCheck(dxDi_Yu0);
      for (i = 0; i < 4; i++)
        ignorez[i] = GetCheck(dxDi_z0 + i);
      if (!ignore7[rrRay])
        EnsureRay();
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the transit chart dialog, as brought up with the
// Chart / Transits menu command.

flag API DlgTransit(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int mon, day, yea, nty, nd, n1, n2;
  real tim;

  switch (message) {
  case WM_INITDIALOG:
    if (us.fInDay)           n1 = 1;
    else if (us.fInDayInf)   n1 = 2;
    else if (us.fInDayGra)   n1 = 3;
    else if (us.fTransit)    n1 = 4;
    else if (us.fTransitInf) n1 = 5;
    else if (us.fTransitGra) n1 = 6;
    else                     n1 = 0;
    SetRadio(dr01 + n1, dr01, dr07);
    SetEditMDYT(hdlg, dcTrMon, dcTrDay, dcTrYea, dcTrTim,
      MonT, DayT, YeaT, TimT);
    n2 = us.fInDayMonth + us.fInDayYear +
      us.fInDayYear*(NAbs(us.nEphemYears) > 1);
    SetRadio(dr08 + n2, dr08, dr11);
    SetEditN(deTr_tY, us.nEphemYears);
    SetCheck(dxTr_p, is.fProgress);
    SetCheck(dxTr_r, is.fReturn);
    SetCheck(dxTr_g, us.fGraphAll);
    SetEditN(deTr_d, us.nDivision);
    SetFocus(GetDlgItem(hdlg, dcTrMon));
    return fFalse;

  case WM_COMMAND:
#ifdef TIME
    if (wParam == dbTr_tn) {
      GetTimeNow(&mon, &day, &yea, &tim, us.dstDef, us.zonDef);
      SetEditMDYT(hdlg, dcTrMon, dcTrDay, dcTrYea, dcTrTim,
        mon, day, yea, tim);
    }
#endif

    if (wParam == IDOK) {
      GetEdit(dcTrMon, sz); mon = NParseSz(sz, pmMon);
      GetEdit(dcTrDay, sz); day = NParseSz(sz, pmDay);
      GetEdit(dcTrYea, sz); yea = NParseSz(sz, pmYea);
      GetEdit(dcTrTim, sz); tim = RParseSz(sz, pmTim);
      nty = GetEditN(hdlg, deTr_tY);
      nd = GetEditN(hdlg, deTr_d);
      EnsureN(mon, FValidMon(mon), "month");
      EnsureN(yea, FValidYea(yea), "year");
      EnsureN(day, FValidDay(day, mon, yea), "day");
      EnsureR(tim, FValidTim(tim), "time");
      EnsureN(nd, FValidDivision(nd), "searching divisions");
      SetCI(ciTran, mon, day, yea, tim,
        us.dstDef, us.zonDef, us.lonDef, us.latDef);
      us.nEphemYears = nty;
      is.fProgress = GetCheck(dxTr_p);
      is.fReturn   = GetCheck(dxTr_r);
      us.fGraphAll = GetCheck(dxTr_g);
      us.nDivision = nd;
      for (n1 = dr01; n1 < dr07; n1++)
        if (GetCheck(n1))
          break;
      n1 -= dr01;
      switch (n1) {
      case 1: wi.nMode = gTraTraTim; break;
      case 2: wi.nMode = gTraTraInf; break;
      case 3: wi.nMode = gTraTraGra; break;
      case 4: wi.nMode = gTraNatTim; break;
      case 5: wi.nMode = gTraNatInf; break;
      case 6: wi.nMode = gTraNatGra; break;
      default: wi.nMode = gWheel;
      }
      n2 = GetCheck(dr08) ? 0 : (GetCheck(dr09) ? 1 :
        (GetCheck(dr10) ? 2 : 3));
      us.fInDayMonth = n2 >= 1;
      us.fInDayYear = us.fInDayMonth && n2 >= 2;
      if (n2 == 2 && NAbs(us.nEphemYears) > 1)
        us.nEphemYears = 0;
      if (n1 == 3 || n1 == 6)
        us.nEphemYears = (n2 <= 2 ? 1 : (nty <= 1 ? 5 : nty));
      else {
        us.fGraphics = fFalse;
        if (n1 == 2 || n1 == 5) {
          wi.fCast = fTrue;
          if (n1 == 2)
            us.fProgress = is.fProgress;
        }
      }
      wi.fRedraw = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the progression settings dialog, as brought up with
// the Chart / Progressions menu command.

flag API DlgProgress(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int mon, day, yea;
  real tim, rd, rC;

  switch (message) {
  case WM_INITDIALOG:
    SetCheck(dxPr_p, us.fProgress);
    SetRadio(dr01 + us.nProgress, dr01, dr03);
    SetEditR(hdlg, dcPr_pd, us.rProgDay, -5);
    FormatR(sz, rDayInYear * rDegMax, -3);
    SetCombo(dcPr_pd, sz);
    FormatR(sz, rDayInYear, -5);
    SetCombo(dcPr_pd, sz);
    SetCombo(dcPr_pd, "29.530588");
    SetCombo(dcPr_pd, "27.321661");
    SetEditR(hdlg, dcPr_pC, us.rProgCusp, -5);
    SetCombo(dcPr_pC, "1");
    SetCombo(dcPr_pC, sz);
    SetEditMDYT(hdlg, dcPrMon, dcPrDay, dcPrYea, dcPrTim,
      MonT, DayT, YeaT, TimT);
    SetFocus(GetDlgItem(hdlg, dcPrMon));
    return fFalse;

  case WM_COMMAND:
#ifdef TIME
    if (wParam == dbPr_pn) {
      GetTimeNow(&mon, &day, &yea, &tim, us.dstDef, us.zonDef);
      SetEditMDYT(hdlg, dcPrMon, dcPrDay, dcPrYea, dcPrTim,
        mon, day, yea, tim);
    }
#endif

    if (wParam == IDOK) {
      rd = GetEditR(hdlg, dcPr_pd);
      rC = GetEditR(hdlg, dcPr_pC);
      GetEdit(dcPrMon, sz); mon = NParseSz(sz, pmMon);
      GetEdit(dcPrDay, sz); day = NParseSz(sz, pmDay);
      GetEdit(dcPrYea, sz); yea = NParseSz(sz, pmYea);
      GetEdit(dcPrTim, sz); tim = RParseSz(sz, pmTim);
      EnsureR(rd, rd != 0.0, "degree per day");
      EnsureR(rC, rC != 0.0, "cusp move ratio");
      EnsureN(mon, FValidMon(mon), "month");
      EnsureN(yea, FValidYea(yea), "year");
      EnsureN(day, FValidDay(day, mon, yea), "day");
      EnsureR(tim, FValidTim(tim), "time");
      us.fProgress = GetCheck(dxPr_p);
      WiCheckMenu(cmdProgress, us.fProgress);
      us.nProgress = GetCheck(dr01) ? ptCast :
        (GetCheck(dr02) ? ptSolarArc : ptMixed);
      us.rProgDay = rd;
      us.rProgCusp = rC;
      SetCI(ciTran, mon, day, yea, tim,
        us.dstDef, us.zonDef, us.lonDef, us.latDef);
      is.JDp = MdytszToJulian(MonT, DayT, YeaT, TimT, us.dstDef, us.zonDef);
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the chart subsettings dialog, as brought up with
// the Chart / Chart Settings menu command.

flag API DlgChart(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int nw, nl, nl2, np, nn, yb, nT;
  flag f;

  switch (message) {
  case WM_INITDIALOG:
    SetCheck(dxCh_v0, us.fVelocity);
    SetEditN(deCh_w,  us.nWheelRows);
    SetCheck(dxCh_w0, us.fWheelReverse);
    SetCheck(dxCh_g0, us.fGridConfig);
    SetCheck(dxCh_gm, us.fGridMidpoint);
    SetCheck(dxCh_a0, us.fAspSummary);
    SetCheck(dxCh_m0, us.fMidSummary);
    SetCheck(dxCh_ma, us.fMidAspect);
    SetCheck(dxCh_Z0, us.fPrimeVert);
    SetCheck(dxCh_l,  us.fSectorApprox);
    SetCheck(dxCh_Ky, us.fCalendarYear);
    SetCheck(dxCh_j0, us.fInfluenceSign);
    SetEditN(deCh_L,  us.nAstroGraphStep);
    SetCheck(dxCh_L0, us.fLatitudeCross);
    sprintf(sz, "%d %s", us.nAstroGraphDist, us.fEuroDist ? "km" : "mi");
    SetEdit(deCh_L2, sz);
    SetCheck(dxCh_Ey, us.nEphemYears != 0);
    SetEditN(deCh_P,  us.nArabicParts);
    SetCheck(dxCh_P0, us.fArabicFlip);
    SetEditN(deCh_Nl, us.nAtlasList);
    SetEditN(deCh_Yb, us.nBioday);
    switch (us.nStar) {
    case 'z': nT = dr02; break;
    case 'l': nT = dr03; break;
    case 'n': nT = dr04; break;
    case 'b': nT = dr05; break;
    case 'd': nT = dr06; break;
    case 'v': nT = dr07; break;
    default:  nT = dr01;
    }
    SetRadio(nT, dr01, dr07);
    switch (us.nArabic) {
    case 'z': nT = dr09; break;
    case 'n': nT = dr10; break;
    case 'f': nT = dr11; break;
    default:  nT = dr08;
    }
    SetRadio(nT, dr08, dr11);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      nw = GetEditN(hdlg, deCh_w);
      nl = GetEditN(hdlg, deCh_L);
      nl2 = GetEditN(hdlg, deCh_L2);
      np = GetEditN(hdlg, deCh_P);
      nn = GetEditN(hdlg, deCh_Nl);
      yb = GetEditN(hdlg, deCh_Yb);
      EnsureN(nw, FValidWheel(nw), "wheel row");
      EnsureN(nl, FValidAstrograph(nl), "astro-graph step");
      EnsureN(nl2, nl2 >= 0, "latitude crossing count");
      EnsureN(np, FValidPart(np), "Arabic part");
      EnsureN(nn, nn >= 0, "nearest city count");
      EnsureN(yb, FValidBioday(yb), "Biorhythm days");
      f = GetCheck(dxCh_v0);
      if (us.fVelocity != f) {
        us.fVelocity = f;
        WiCheckMenu(cmdGraphicsSidebar, !f);
      }
      us.nWheelRows = nw;
      us.fWheelReverse = GetCheck(dxCh_w0);
      us.fGridConfig = GetCheck(dxCh_g0);
      us.fGridMidpoint = GetCheck(dxCh_gm);
      us.fAspSummary = GetCheck(dxCh_a0);
      us.fMidSummary = GetCheck(dxCh_m0);
      us.fMidAspect = GetCheck(dxCh_ma);
      us.fPrimeVert = GetCheck(dxCh_Z0);
      us.fSectorApprox = GetCheck(dxCh_l);
      us.fCalendarYear = GetCheck(dxCh_Ky);
      us.fInfluenceSign = GetCheck(dxCh_j0);
      us.nAstroGraphStep = nl;
      us.fLatitudeCross = GetCheck(dxCh_L0);
      us.nAstroGraphDist = nl2;
      us.nEphemYears = GetCheck(dxCh_Ey);
      us.nArabicParts = np;
      us.fArabicFlip = GetCheck(dxCh_P0);
      us.nAtlasList = nn;
      us.nBioday = yb;
      if (us.nStar)
        us.nStar = GetCheck(dr02) ? 'z' : (GetCheck(dr03) ? 'l' :
          (GetCheck(dr04) ? 'n' : (GetCheck(dr05) ? 'b' :
          (GetCheck(dr06) ? 'd' : (GetCheck(dr07) ? 'v' : fTrue)))));
      if (us.nArabic)
        us.nArabic = GetCheck(dr09) ? 'z' : (GetCheck(dr10) ? 'n' :
          (GetCheck(dr11) ? 'f' : fTrue));
      wi.fCast = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


CONST char *rgszFontDisp[cFont] = {szAppNameCore, "Wingdings", "Astro",
  "Enigma", "Hamburg", "Astronomicon", "Courier New", "Consolas", "Arial"};

// Processing function for the graphic settings dialog, as brought up with the
// Graphics / Graphics Settings menu command.

flag API DlgGraphics(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];
  int nx, ny, ns, nS, ng, nwn, nxz, nx1, nyxv, is, i, j;
  real rI, rxw, rxg;

  switch (message) {
  case WM_INITDIALOG:
    SetEditN(deGr_Xw_x, gs.xWin);
    SetEditN(deGr_Xw_y, gs.yWin);
    for (is = 100; is <= 400; is += 50) {
      sprintf(sz, "%d", is);
      if (is % 100 == 0)
        SetCombo(dcGr_Xs, sz);
      SetCombo(dcGr_XSS, sz);
    }
    SetEditN(dcGr_Xs, gs.nScale);
    SetEditN(dcGr_XSS, gs.nScaleText);
    SetEditR(hdlg, deGr_XI1, gs.rBackPct, 1);
    SetCheck(dxGr_XQ, gs.fKeepSquare);
    SetEditN(deGr_YXg, gs.nGridCell);
    SetEditR(hdlg, deGr_XW, gs.rRot, 2);
    SetEditR(hdlg, deGr_XG, gs.rTilt, 2);
    SetCheck(dxGr_XP0, gs.fSouth);
    SetCheck(dxGr_XW0, gs.fMollewide);
    SetEditN(deGr_WN, wi.nTimerDelay);
    SetCheck(dxGr_XN, gs.fAnimMap);
    if (gs.objTrack >= 0)
      SetEdit(deGr_XZ, szObjName[gs.objTrack]);
    else
      SetEdit(deGr_XZ, "None");
    SetCheck(dxGr_Wn, wi.fNoUpdate);
    SetRadio(gs.objLeft > 0 ? dr02 :
      (gs.objLeft < 0 ? dr03 : dr01), dr01, dr03);
    SetEdit(deGr_X1, szObjName[gs.objLeft == 0 ? oSun : NAbs(gs.objLeft)-1]);
    SetRadio(dr04 + gs.nDecaType, dr04, dr06);
    SetEditN(deGr_YXv, gs.nDecaSize);
    for (i = dcGr_Xf0; i <= dcGr_Xf4; i++)
      for (j = 0; j < cFont; j++) {
        if (rgszFontAllow[i-dcGr_Xf0][j] < '0')
          continue;
        SetCombo(i, rgszFontDisp[j]);
      }
    SetEdit(dcGr_Xf0, rgszFontDisp[gs.nFont/10000]);
    SetEdit(dcGr_Xf1, rgszFontDisp[gs.nFont/1000%10]);
    SetEdit(dcGr_Xf2, rgszFontDisp[gs.nFont/100%10]);
    SetEdit(dcGr_Xf3, rgszFontDisp[gs.nFont/10%10]);
    SetEdit(dcGr_Xf4, rgszFontDisp[gs.nFont%10]);
    SetRadio(drg1 + (gs.nGlyphs/1000 - 1), drg1, drg2);
    SetRadio(drg3 + (gs.nGlyphs/100%10 - 1), drg3, drg4);
    SetRadio(drg5 + (gs.nGlyphs/10%10 - 1), drg5, drg7);
    SetRadio(drg8 + (gs.nGlyphs%10 - 1), drg8, drg9);
    SetRadio(dr07 + gs.nDecaFill, dr07, dr09);
    SetFocus(GetDlgItem(hdlg, deGr_Xw_x));
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK) {
      nx = GetEditN(hdlg, deGr_Xw_x);
      ny = GetEditN(hdlg, deGr_Xw_y);
      ns = GetEditN(hdlg, dcGr_Xs);
      nS = GetEditN(hdlg, dcGr_XSS);
      rI = GetEditR(hdlg, deGr_XI1);
      ng = GetEditN(hdlg, deGr_YXg);
      rxw = GetEditR(hdlg, deGr_XW);
      rxg = GetEditR(hdlg, deGr_XG);
      nwn = GetEditN(hdlg, deGr_WN);
      GetEdit(deGr_XZ, sz); nxz = NParseSz(sz, pmObject);
      nyxv = GetEditN(hdlg, deGr_YXv);
      GetEdit(deGr_X1, sz); nx1 = NParseSz(sz, pmObject);
      EnsureN(nx, FValidGraphX(nx), "horizontal size");
      EnsureN(ny, FValidGraphY(ny), "vertical size");
      EnsureN(ns, FValidScale(ns), "character scale");
      EnsureN(nS, FValidScaleText(nS), "text scale");
      EnsureR(rI, FValidBackPct(rI), "background transparency");
      EnsureN(ng, FValidGrid(ng), "grid cell");
      EnsureR(rxw, FValidRotation(rxw), "horizontal rotation");
      EnsureR(rxg, FValidTilt(rxg), "vertical tilt");
      EnsureN(nwn, FValidTimer(nwn), "animation delay");
      EnsureN(nxz, FValidTelescope(nxz), "telescope planet");
      EnsureN(nx1, FItem(nx1), "rotation planet");
      EnsureN(nyxv, FValidDecaSize(nyxv), "wheel corners coverage");
      if (gs.xWin != nx || gs.yWin != ny) {
        gs.xWin = nx; gs.yWin = ny;
        if (wi.fWindowChart)
          ResizeWindowToChart();
      }
      gs.nScale = ns; gs.nScaleText = nS;
      gs.rBackPct = rI;
      gs.fKeepSquare = GetCheck(dxGr_XQ);
      gs.nGridCell = ng;
      gs.rRot = rxw; gs.rTilt = rxg;
      if (wi.nTimerDelay != (UINT)nwn) {
        wi.nTimerDelay = nwn;
        if (wi.lTimer != 0)
          KillTimer(wi.hwnd, 1);
        wi.lTimer = SetTimer(wi.hwnd, 1, wi.nTimerDelay, NULL);
      }
      gs.fSouth = GetCheck(dxGr_XP0);
      gs.fMollewide = GetCheck(dxGr_XW0);
      gs.fAnimMap = GetCheck(dxGr_XN);
      gs.objTrack = nxz;
      wi.fNoUpdate = GetCheck(dxGr_Wn);
      gs.objLeft = GetCheck(dr01) ? 0 : (GetCheck(dr02) ? nx1+1 : -nx1-1);
      gs.nDecaType = GetRadio(hdlg, dr04, dr06);
      gs.nDecaSize = nyxv;
      gs.nFont = 0;
      for (i = dcGr_Xf0; i <= dcGr_Xf4; i++) {
        GetEdit(i, sz);
        // Astro font in slot #2 gets checked first, since it's a substring.
        for (j = 2; j < cFont; j += (j == 2 ? -2 : (j == 1 ? 2 : 1))) {
          if (rgszFontAllow[i-dcGr_Xf0][j] >= '0' &&
            FMatchSz(sz, rgszFontDisp[j])) {
            gs.nFont = gs.nFont * 10 + j;
            break;
          }
        }
        if (j >= cFont)    // If no match, default to "Astrolog".
          gs.nFont *= 10;
      }
      gs.nGlyphs = (GetRadio(hdlg, drg1, drg2) + 1) * 1000 +
        (GetRadio(hdlg, drg3, drg4) + 1) * 100 +
        (GetRadio(hdlg, drg5, drg7) + 1) * 10 +
        (GetRadio(hdlg, drg8, drg9) + 1);
      gs.nDecaFill = GetRadio(hdlg, dr07, dr09);
      if (gs.nFont != 0)
        gi.nFontPrev = gs.nFont;
      us.fGraphics = wi.fRedraw = wi.fMenuAll = fTrue;
    }
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}


// Processing function for the about dialog, showing copyrights and credits,
// as brought up with the Help / About Astrolog menu command.

flag API DlgAbout(HWND hdlg, uint message, WORD wParam, LONG lParam)
{
  char sz[cchSzMax];

  switch (message) {
  case WM_INITDIALOG:
#ifdef DEBUG
    SetWindowText(hdlg, "About Astrolog (DEBUG)");
#endif
    sprintf(sz, "%s version %s for %s Windows",
      szAppName, szVersionCore, szArchCore);
    SetDlgItemText(hdlg, ds1, sz);
    sprintf(sz, "Released %s", szDateCore);
    SetDlgItemText(hdlg, ds2, sz);
    return fTrue;

  case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL) {
      EndDialog(hdlg, fTrue);
      return fTrue;
    }
    break;
  }
  return fFalse;
}
#endif // WIN

/* wdialog.cpp */
