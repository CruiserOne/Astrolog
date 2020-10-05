/*
** Astrolog (Version 7.10) File: xgeneral.cpp
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
** Last code change made 9/30/2020.
*/

#include "astrolog.h"


#ifdef GRAPH
/*
******************************************************************************
** Core Graphic Procedures.
******************************************************************************
*/

/* Set the current color to use in drawing on the screen or bitmap array. */

void DrawColor(KI col)
{
#ifdef WINANY
  HPEN hpenT;
#endif
#ifdef MACG
  RGBColor kv;
#endif

  if (gi.fFile) {
#ifdef PS
    if (gs.ft == ftPS) {
      if (gi.kiCur != col) {
        PsStrokeForce();      // Render existing path with current color
        fprintf(gi.file, "%.2f %.2f %.2f c\n",
          (real)RGBR(rgbbmp[col])/255.0, (real)RGBG(rgbbmp[col])/255.0,
          (real)RGBB(rgbbmp[col])/255.0);
      }
    }
#endif
#ifdef META
    if (gs.ft == ftWmf)
      gi.kiLineDes = col;
#endif
#ifdef WIRE
    if (gs.ft == ftWire)
      gi.kiCur = col;
#endif
  }
#ifdef X11
  else
    XSetForeground(gi.disp, gi.gc, rgbind[col]);
#endif
#ifdef WINANY
  else {
    if (gi.kiCur != col) {
      hpenT = wi.hpen;
      wi.hpen = CreatePen(PS_SOLID, gi.nScaleT, (COLORREF)rgbbmp[col]);
      SelectObject(wi.hdc, wi.hpen);
      if (hpenT != (HPEN)NULL)
        DeleteObject(hpenT);
    }
  }
#endif
#ifdef MACG
  else {
    kv.red   = RGBR(rgbbmp[col]) << 8;
    kv.green = RGBG(rgbbmp[col]) << 8;
    kv.blue  = RGBB(rgbbmp[col]) << 8;
    RGBForeColor(&kv);
    RGBBackColor(&kv);
  }
#endif
  gi.kiCur = col;
}


/* Set a single point on the screen. This is the most basic graphic function */
/* and is called by all the more complex routines. Based on what mode we are */
/* in, we either set a cell in the bitmap array or a pixel on the window.    */

void DrawPoint(int x, int y)
{
  if (gi.fFile) {
    if (gs.ft == ftBmp) {
      // Force the coordinates to be within the bounds of the bitmap array.
      if (x < 0)
        x = 0;
      else if (x >= gs.xWin)
        x = gs.xWin-1;
      if (y < 0)
        y = 0;
      else if (y >= gs.yWin)
        y = gs.yWin-1;
      BmSet(gi.bm, x, y, gi.kiCur);
    }
#ifdef PS
    else if (gs.ft == ftPS) {
      DrawColor(gi.kiCur);
      PsLineCap(fTrue);
      fprintf(gi.file, "%d %d d\n", x, y);
      PsStroke(2);
    }
#endif
#ifdef META
    else if (gs.ft == ftWmf) {
      gi.kiFillDes = gi.kiCur;
      MetaSelect();
      MetaEllipse(x-gi.nPenWid/2, y-gi.nPenWid/2,
        x+gi.nPenWid/2, y+gi.nPenWid/2);
    }
#endif
#ifdef WIRE
    else
      WirePoint(x, y, gi.zDefault);
#endif
  }
#ifdef X11
  else
    XDrawPoint(gi.disp, gi.pmap, gi.gc, x, y);
#endif
#ifdef WIN
  else {
    if (wi.hdcPrint == hdcNil)
      SetPixel(wi.hdc, x, y, (COLORREF)rgbbmp[gi.kiCur]);
    else {
      MoveTo(wi.hdc, x,   y);
      LineTo(wi.hdc, x+1, y);
    }
  }
#endif
#ifdef MACG
  else {
    MoveTo(x, y); LineTo(x, y);
  }
#endif
#ifdef WCLI
  else
    SetPixel(wi.hdc, x, y, (COLORREF)rgbbmp[gi.kiCur]);
#endif
}


/* Draw dot a little larger than just a single pixel at specified location. */

void DrawSpot(int x, int y)
{
#ifdef PS
  if (gs.ft == ftPS) {
    PsLineWidth((int)(gi.rLineWid*3.0));
    DrawPoint(x, y);
    PsLineWidth((int)(gi.rLineWid/3.0));
    return;
  }
#endif
#ifdef META
  if (gs.ft == ftWmf) {
    gi.kiFillDes = gi.kiCur;
    MetaSelect();
    MetaEllipse(x-gi.nPenWid, y-gi.nPenWid, x+gi.nPenWid, y+gi.nPenWid);
    return;
  }
#endif
#ifdef WIRE
  if (gs.ft == ftWire) {
    WireLine(x-1, y,   0, x+1, y,   0);
    WireLine(x,   y-1, 0, x,   y+1, 0);
    return;
  }
#endif
  DrawPoint(x, y);
  DrawPoint(x, y-1);
  DrawPoint(x-1, y);
  DrawPoint(x+1, y);
  DrawPoint(x, y+1);
}


/* Draw a filled in block, defined by the corners of its rectangle. */

void DrawBlock(int x1, int y1, int x2, int y2)
{
  int x, y;
#ifdef MACG
  Rect rc;
#endif

  if (gi.fFile) {
    if (gs.ft == ftBmp) {
      // Force the coordinates to be within the bounds of the bitmap array.
      if (x1 < 0)
        x1 = 0;
      if (x2 >= gs.xWin)
        x2 = gs.xWin-1;
      if (y1 < 0)
        y1 = 0;
      else if (y2 >= gs.yWin)
        y2 = gs.yWin-1;
      // For bitmap, just fill in the array.
      for (y = y1; y <= y2; y++)
        for (x = x1; x <= x2; x++)
          BmSet(gi.bm, x, y, gi.kiCur);
    }
#ifdef PS
    else if (gs.ft == ftPS) {
      DrawColor(gi.kiCur);
      fprintf(gi.file, "%d %d %d %d rf\n",
        Max(x1-gi.nPenWid/4, 0), Max(y1-gi.nPenWid/4, 0),
        x2-x1+gi.nPenWid/4, y2-y1+gi.nPenWid/4);
    }
#endif
#ifdef META
    else if (gs.ft == ftWmf) {
      gi.kiFillDes = gi.kiCur;
      MetaSelect();
      MetaRectangle(x1-gi.nPenWid/2, y1-gi.nPenWid/2,
        x2+gi.nPenWid/2, y2+gi.nPenWid/2);
    }
#endif
#ifdef WIRE
    else {
      if (x1 == x2 || y1 == y2)
        WireLine(x1, y1, 0, x2, y2, 0);
    }
#endif
  }
#ifdef X11
  else
    XFillRectangle(gi.disp, gi.pmap, gi.gc, x1, y1, x2-x1+1, y2-y1+1);
#endif
#ifdef WINANY
  else {
    wi.hbrush = CreateSolidBrush((COLORREF)rgbbmp[gi.kiCur]);
    SelectObject(wi.hdc, wi.hbrush);
    PatBlt(wi.hdc, x1, y1, x2-x1 + gi.nScaleT, y2-y1 + gi.nScaleT, PATCOPY);
    SelectObject(wi.hdc, GetStockObject(NULL_BRUSH));
    DeleteObject(wi.hbrush);
  }
#endif
#ifdef MACG
  else {
    SetRect(&rc, x1, y1, x2+1, y2+1);
    EraseRect(&rc);
  }
#endif
}


/* Draw a rectangle on the screen with specified thickness. This is just   */
/* like DrawBlock() except that we are only drawing the edges of the area. */

void DrawBox(int x1, int y1, int x2, int y2, int xsiz, int ysiz)
{
#ifdef META
  if (gs.ft == ftWmf)
    // For thin boxes in metafiles, can just output one rectangle record
    // instead of drawing each side separately as have to do otherwise.
    if (xsiz <= 1 && ysiz <= 1) {
      gi.kiFillDes = kNull;          // Specify a hollow fill brush.
      MetaSelect();
      MetaRectangle(x1, y1, x2, y2);
      return;
    }
#endif
#ifdef WIRE
  if (gs.ft == ftWire) {
    DrawLine(x1, y1, x2, y1);
    DrawLine(x1, y1, x1, y2);
    DrawLine(x2, y1, x2, y2);
    DrawLine(x1, y2, x2, y2);
    return;
  }
#endif
  DrawBlock(x1, y1, x2, y1 + ysiz - 1);
  DrawBlock(x1, y1 + ysiz, x1 + xsiz - 1, y2 - ysiz);
  DrawBlock(x2 - xsiz + 1, y1 + ysiz, x2, y2 - ysiz);
  DrawBlock(x1, y2 - ysiz + 1, x2, y2);
}


#ifdef WINANY
/* Draw a character from the Astro or Wingdings fonts on the screen. Used  */
/* to draw sign, planet, and aspect glyphs from these fonts within charts. */

void WinDrawGlyph(char ch, int x, int y, int nFont)
{
  HFONT hfont, hfontPrev;
  SIZE size;
  char sz[2];
  KV kvSav;
  int nSav;

  hfont = CreateFont(12*gi.nScale, 0, 0, 0, 400, fFalse, fFalse, fFalse,
    nFont >= 1 ? ANSI_CHARSET : SYMBOL_CHARSET, OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH | FF_DECORATIVE,
    nFont == 2 ? "EnigmaAstrology" : (nFont == 1 ? "Astro" : "Wingdings"));
  if (hfont == NULL)
    return;
  hfontPrev = (HFONT)SelectObject(wi.hdc, hfont);
  sprintf(sz, "%c", ch);
  GetTextExtentPoint(wi.hdc, sz, 1, &size);
  kvSav = GetTextColor(wi.hdc);
  SetTextColor(wi.hdc, rgbbmp[gi.kiCur]);
  nSav = SetBkMode(wi.hdc, TRANSPARENT);
  TextOut(wi.hdc, x - (size.cx >> 1), y - (size.cy >> 1), sz, 1);
  SetBkMode(wi.hdc, nSav);
  SetTextColor(wi.hdc, kvSav);
  SelectObject(wi.hdc, hfontPrev);
  DeleteObject(hfont);
}


/* Draw a character from the Unicode Ariel font on the screen. Used to draw */
/* sign, planet, and aspect glyphs from this font within charts.            */

void WinDrawGlyphW(WCHAR wch, int x, int y, int nScale)
{
  HFONT hfont, hfontPrev;
  SIZE size;
  WCHAR wz[2];
  KV kvSav;
  int nSav;

  hfont = CreateFont(12*gi.nScale*nScale/100, 0, 0, 0, 400, fFalse, fFalse,
    fFalse, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    PROOF_QUALITY, VARIABLE_PITCH | FF_DECORATIVE, "Ariel");
  if (hfont == NULL)
    return;
  hfontPrev = (HFONT)SelectObject(wi.hdc, hfont);
  wz[0] = wch; wz[1] = chNull;
  GetTextExtentPointW(wi.hdc, wz, 1, &size);
  kvSav = GetTextColor(wi.hdc);
  SetTextColor(wi.hdc, rgbbmp[gi.kiCur]);
  nSav = SetBkMode(wi.hdc, TRANSPARENT);
  TextOutW(wi.hdc, x - (size.cx >> 1), y - (size.cy >> 1), wz, 1);
  SetBkMode(wi.hdc, nSav);
  SetTextColor(wi.hdc, kvSav);
  SelectObject(wi.hdc, hfontPrev);
  DeleteObject(hfont);
}


/* Clear and erase the entire graphics screen on Windows. */

void WinClearScreen(KI ki)
{
  wi.hbrush = CreateSolidBrush((COLORREF)rgbbmp[ki]);
  SelectObject(wi.hdc, wi.hbrush);
  PatBlt(wi.hdc, -gi.xOffset, -gi.yOffset, wi.xClient, wi.yClient, PATCOPY);
  SelectObject(wi.hdc, GetStockObject(NULL_BRUSH));
  DeleteObject(wi.hbrush);
}
#endif


/* Clear and erase the graphics screen or bitmap contents. */

void DrawClearScreen()
{
#ifdef PS
  if (gs.ft == ftPS) {
    // For PostScript charts first output page orientation information.
    if (!gi.fEps) {
      if (gs.nOrient == 0)
        gs.nOrient = gs.xWin > gs.yWin ? -1 : 1;
      if (gs.nOrient < 0) {
        // Values chartx and charty are reversed for Landscape mode.
        fprintf(gi.file, "%d %d translate\n",
          ((int)(gs.xInch*72.0+rRound) + gs.yWin)/2,
          ((int)(gs.yInch*72.0+rRound) + gs.xWin)/2);
        fprintf(gi.file, "-90 rotate\n");
      } else {
        // Most charts are in Portrait mode.
        fprintf(gi.file, "%d %d translate\n",
          ((int)(gs.xInch*72.0+rRound) - gs.xWin)/2,
          ((int)(gs.yInch*72.0+rRound) + gs.yWin)/2);
      }
    } else
      fprintf(gi.file, "0 %d translate\n", gs.yWin);
    fprintf(gi.file, "1 -1 scale\n");
    gs.nScale *= PSMUL; gs.xWin *= PSMUL; gs.yWin *= PSMUL; gi.nScale *= PSMUL;
    fprintf(gi.file, "1 %d div dup scale\n", PSMUL);
  }
#endif
#ifdef META
  if (gs.ft == ftWmf)
    MetaInit();        // For metafiles first go write header information.
#endif

  // Don't actually erase the screen if the -Xj switch is in effect.
  if (gs.fJetTrail)
    return;

  DrawColor(gi.kiOff);
#ifdef WINANY
  // For Windows charts clear entire window, not just the chart area.
  if (!gi.fFile)
    WinClearScreen(gi.kiCur);
  else
#endif /* WINANY */
    DrawBlock(0, 0, gs.xWin - 1, gs.yWin - 1);    // Clear bitmap screen.
}


/* Draw a line on the screen, specified by its endpoints. In addition, we */
/* have specified a skip factor, which allows us to draw dashed lines.    */

void DrawDash(int x1, int y1, int x2, int y2, int skip)
{
  int x = x1, y = y1, i = 0,
    dx = x2 - x1, dy = y2 - y1, xInc, yInc, xInc2, yInc2, d, dInc, z, zMax;

  if (skip < 0)
    skip = 0;
  else if (gs.nDashMax >= 0 && skip > gs.nDashMax)
    skip = gs.nDashMax;
#ifdef ISG
  if (!gi.fFile) {
    if (!skip) {
#ifdef X11
      // For non-dashed X window lines, have the Xlib do it.

      XDrawLine(gi.disp, gi.pmap, gi.gc, x1, y1, x2, y2);
      // Some XDrawLine implementations don't draw the last pixel.
      XDrawPoint(gi.disp, gi.pmap, gi.gc, x2, y2);
#endif
#ifdef WINANY
      // For Windows lines, have to manually draw the last pixel.

      MoveTo(wi.hdc, x1, y1);
      LineTo(wi.hdc, x2, y2);
      SetPixel(wi.hdc, x2, y2, (COLORREF)rgbbmp[gi.kiCur]);
#endif
#ifdef MACG
      MoveTo(x1, y1);
      LineTo(x2, y2);
#endif
      return;
    }
#ifdef WIN
    if (skip && wi.hdcPrint != hdcNil)
      skip = (skip + 1)*METAMUL - 1;
#endif
  }
#endif /* ISG */

#ifdef PS
  if (gs.ft == ftPS) {

    // For PostScript charts can save file size if we output a LineTo command
    // when the start vertex is the same as the end vertex of the previous
    // line drawn, instead of writing out both vertices.

    PsLineCap(fTrue);
    PsDash(skip);
    if (gi.xPen != x1 || gi.yPen != y1)
      fprintf(gi.file, "%d %d %d %d l\n", x1, y1, x2, y2);
    else
      fprintf(gi.file, "%d %d t\n", x2, y2);
    gi.xPen = x2; gi.yPen = y2;
    PsStroke(2);
    return;
  }
#endif

#ifdef META
  if (gs.ft == ftWmf) {

    // For metafile charts can save file size for consecutive lines sharing
    // endpoints by consolidating them into a PolyLine.

    if (gi.xPen != x1 || gi.yPen != y1) {
      if (x1 != x2 || y1 != y2) {
        gi.kiLineDes = (gi.kiLineDes & 15) + 16*(skip > 3 ? 3 : skip);
        MetaSelect();
        gi.pwPoly = gi.pwMetaCur;
        MetaRecord(8, 0x325);      // Polyline
        MetaWord(2); MetaWord(x1); MetaWord(y1);
      } else {
        DrawPoint(x1, y1);
        return;
      }
    } else {
      *gi.pwPoly += 2;
      (*(gi.pwPoly+3))++;
      // Note: We should technically update the max record size in the file
      // header here too, but it doesn't seem necessary.
    }
    MetaWord(x2); MetaWord(y2);
    gi.xPen = x2; gi.yPen = y2;
    return;
  }
#endif

#ifdef WIRE
  if (gs.ft == ftWire) {
    // Solid non-dashed lines are supported in wireframe format.
    if (skip == 0) {
      WireLine(x1, y1, gi.zDefault, x2, y2, gi.zDefault);
      return;
    }
  }
#endif

  // If none of the above cases hold, then have to draw line dot by dot.

  // Determine slope.
  if (NAbs(dx) >= NAbs(dy)) {
    xInc = NSgn(dx); yInc = 0;
    xInc2 = 0; yInc2 = NSgn(dy);
    zMax = NAbs(dx); dInc = NAbs(dy);
    d = zMax - (!FOdd(dx) && x1 > x2);
  } else {
    xInc = 0; yInc = NSgn(dy);
    xInc2 = NSgn(dx); yInc2 = 0;
    zMax = NAbs(dy); dInc = NAbs(dx);
    d = zMax - (!FOdd(dy) && y1 > y2);
  }
  d >>= 1;

  // Loop over long axis, adjusting short axis for slope as needed.
  for (z = 0; z <= zMax; z++) {
    if (i < 1)
      DrawPoint(x, y);
    i = i < skip ? i+1 : 0;
    x += xInc; y += yInc; d += dInc;
    if (d >= zMax) {
      x += xInc2; y += yInc2; d -= zMax;
    }
  }
}


/* Draw a normal line on the screen; however, if the x coordinates are close */
/* to either of the two given bounds, then we assume that the line runs off  */
/* one side and reappears on the other, so draw the appropriate two lines    */
/* instead. This is used by the Ley line and astro-graph routines, which     */
/* draw lines running around the world and hence off the edges of the maps.  */

void DrawWrap(int x1, int y1, int x2, int y2, int xmin, int xmax)
{
  int xmid, ymid, i, j, k;

  if (x1 < 0) {           // Special case for drawing world map.
    DrawPoint(x2, y2);
    return;
  }
  j = (xmin < 0);    // Negative xmin means always draw forward.
  if (j)
    neg(xmin);
  xmid = (xmax-xmin) / 2;
  if (j)
    k = (x1 < x2 ? xmid*7 : xmid)/4;
  else
    k = xmid;

  // If endpoints aren't near opposite edges, just draw the line and return.
  if (NAbs(x2-x1) < k) {
    DrawLine(x1, y1, x2, y2);
    return;
  }
  if ((i = (xmax-xmin+1) + (!j && x1 < xmid ? x1-x2 : x2-x1)) == 0)
    i = 1;

  // Determine vertical coordinate where our line runs off edges of screen.
  ymid = y1+(int)((real)(y2-y1)*
    (!j && x1 < xmid ? (real)(x1-xmin) : (real)(xmax-x1))/(real)i + rRound);
  DrawLine(x1, y1, !j && x1 < xmid ? xmin : xmax, ymid);
  DrawLine(j || x2 < xmid ? xmin : xmax, ymid, x2, y2);
}


/* This routine, and its companion below, clips a line defined by its  */
/* endpoints to either above some line y=c, or below some line y=c. By */
/* passing in parameters in different orders, we can clip to vertical  */
/* lines, too. These are used by the DrawClip() routine below.         */

void ClipLesser(int *x1, int *y1, int *x2, int *y2, int s)
{
  if (*y2 - *y1)
    *x1 -= NMultDiv(*y1 - s, *x2 - *x1, *y2 - *y1);
  *y1 = s;
}

void ClipGreater(int *x1, int *y1, int *x2, int *y2, int s)
{
  if (*y2 - *y1)
    *x1 += NMultDiv(s - *y1, *x2 - *x1, *y2 - *y1);
  *y1 = s;
}


/* Draw a line on the screen. This is just like DrawLine() routine earlier; */
/* however, first clip the endpoints to the given viewport before drawing.  */

flag FDrawClip(int x1, int y1, int x2, int y2, int xl, int yl, int xh, int yh,
  int skip, int *x0, int *y0)
{
  if ((x1 < xl && x2 < xl) || (y1 < yl && y2 < yl) ||  // Skip if outside
    (x1 > xh && x2 > xh) || (y1 > yh && y2 > yh))      // bounding box.
    return fFalse;
  if (x1 < xl)
    ClipLesser (&y1, &x1, &y2, &x2, xl);    // Check left side of window.
  if (x2 < xl)
    ClipLesser (&y2, &x2, &y1, &x1, xl);
  if (y1 < yl)
    ClipLesser (&x1, &y1, &x2, &y2, yl);    // Check top side of window.
  if (y2 < yl)
    ClipLesser (&x2, &y2, &x1, &y1, yl);
  if (x1 > xh)
    ClipGreater(&y1, &x1, &y2, &x2, xh);    // Check right of window.
  if (x2 > xh)
    ClipGreater(&y2, &x2, &y1, &x1, xh);
  if (y1 > yh)
    ClipGreater(&x1, &y1, &x2, &y2, yh);    // Check bottom of window.
  if (y2 > yh)
    ClipGreater(&x2, &y2, &x1, &y1, yh);
  if (x1 < xl || x2 < xl || y1 < yl || y2 < yl ||  // Skip if not inside
    x1 > xh || x2 > xh || y1 > yh || y2 > yh)      // bounding box.
    return fFalse;
  DrawDash(x1, y1, x2, y2, skip);           // Go draw the line.

  // Return coordinates at which drawn line crosses bounding box.
  if (x0 != NULL) {
    if (x1 == xl || x1 == xh || y1 == yl || y1 == yh) {
      *x0 = x1; *y0 = y1;
      return fTrue;
    }
    if (x2 == xl || x2 == xh || y2 == yl || y2 == yh) {
      *x0 = x2; *y0 = y2;
      return fTrue;
    }
  }
  return fFalse;
}


/* Draw a circle or ellipse inside the given bounding rectangle. */

void DrawEllipse(int x1, int y1, int x2, int y2)
{
  int x, y, rx, ry, m, n, u, v, i;
#ifdef MACG
  Rect rc;
#endif

  if (gi.fFile) {
    x = (x1+x2)/2; y = (y1+y2)/2; rx = (x2-x1)/2; ry = (y2-y1)/2;
    if (gs.ft == ftBmp || gs.ft == ftWire) {
      m = x + rx; n = y;
      for (i = 0; i <= nDegMax; i += DEGINC) {
        u = x + (int)(((real)rx+rRound)*RCosD((real)i));
        v = y + (int)(((real)ry+rRound)*RSinD((real)i));
        DrawLine(m, n, u, v);
        m = u; n = v;
      }
    }
#ifdef PS
    else if (gs.ft == ftPS) {
      PsLineCap(fFalse);
      PsStrokeForce();
      PsDash(0);
      fprintf(gi.file, "%d %d %d %d el\n", rx, ry, x, y);
    }
#endif
#ifdef META
    else {
      gi.kiFillDes = kNull;    // Specify a hollow fill brush.
      MetaSelect();
      MetaEllipse(x1+gi.nPenWid/3, y1+gi.nPenWid/3,
        x2+1+gi.nPenWid/3, y2+1+gi.nPenWid/3);
    }
#endif
  }
#ifdef X11
  else
    XDrawArc(gi.disp, gi.pmap, gi.gc, x1, y1, x2-x1, y2-y1, 0, nDegMax*64);
#endif
#ifdef WINANY
  else
    Ellipse(wi.hdc, x1, y1, x2+1, y2+1);
#endif
#ifdef MACG
  else {
    SetRect(&rc, x1, y1, x2, y2);
    FrameOval(&rc);
  }
#endif
}


/* Draw a filled in circle or ellipse inside a bounding rectangle. */

void DrawEllipse2(int x1, int y1, int x2, int y2)
{
  int rx, ry, r, i, j, iT, jT, q, qLo, qHi, m1, m2, n1, n2, x, y;
#ifdef MACG
  Rect rc;
#endif

  if (gi.fFile) {
    x = (x1 + x2) >> 1; y = (y1 + y2) >> 1;
    rx = NAbs(x2 - x1) >> 1; ry = NAbs(y2 - y1) >> 1;
    if (gs.ft == ftBmp || gs.ft == ftWire) {
      // This efficient algorithm has no calculations more complicated than
      // addition (and multiplication to proportion in case of an ellipse).
      if (rx == 0 || ry == 0) {
        DrawBlock(x1, y1, x2, y2);
        return;
      }
      r = Max(rx, ry);
      j = r; q = r+1; qLo = 1; qHi = r+r-1;
      m1 = (x1 + x2) >> 1; m2 = m1 + FOdd(x2 - x1);
      n1 = (y1 + y2) >> 1; n2 = n1 + FOdd(y2 - y1);
      for (i = 0; i <= r; i++) {
        iT = (rx <= ry) ? i : i * ry / rx;
        jT = (rx >= ry) ? j : j * rx / ry;
        DrawBlock(m1 - jT, n1 - iT, m2 + jT, n1 - iT);
        DrawBlock(m1 - jT, n2 + iT, m2 + jT, n2 + iT);
        q -= qLo;
        while (q < 0) {
          q += qHi;
          qHi -= 2;
          j--;
        }
        qLo += 2;
      }
    }
#ifdef PS
    else if (gs.ft == ftPS) {
      PsLineCap(fFalse);
      PsStrokeForce();
      PsDash(0);
      fprintf(gi.file, "%d %d %d %d ef\n", rx, ry, x, y);
    }
#endif
#ifdef META
    else {
      gi.kiFillDes = gi.kiCur;    // Specify a solid fill brush.
      MetaSelect();
      MetaEllipse(x1+gi.nPenWid/3, y1+gi.nPenWid/3,
        x2+1+gi.nPenWid/3, y2+1+gi.nPenWid/3);
    }
#endif
  }
#ifdef X11
  else
    XFillArc(gi.disp, gi.pmap, gi.gc, x1, y1, x2-x1, y2-y1, 0, nDegMax*64);
#endif
#ifdef WINANY
  else {
    wi.hbrush = CreateSolidBrush((COLORREF)rgbbmp[gi.kiCur]);
    SelectObject(wi.hdc, wi.hbrush);
    Ellipse(wi.hdc, x1, y1, x2+1, y2+1);
    SelectObject(wi.hdc, GetStockObject(NULL_BRUSH));
    DeleteObject(wi.hbrush);
  }
#endif
#ifdef MACG
  else {
    SetRect(&rc, x1, y1, x2, y2);
    PaintOval(&rc);
  }
#endif
}


// Fast version of rotating that assumes the slow trigonometry values have
// already been computed. Useful when rotating many points by the same angle.

#define RotateR2(x1, y1, x2, y2, rS, rC) x2 = (x1)*(rC) - (y1)*(rS); \
  y2 = (y1)*(rC) + (x1)*(rS)
#define RotateR2Init(rS, rC, d) rS = RSinD(d); rC = RCosD(d)

/* Draw a filled in circle or ellipse inside a bounding rectangle, which */
/* has a proportion of it in phase or shadow, like a crescent Moon.      */

void DrawCrescent(int x1, int y1, int x2, int y2, real rProp, real rRotate,
  KI ki0, KI ki1)
{
  real xc, yc, rSq, dxy, rx, ry, dx, dy, dx2, dy2, rS, rC;
  int x, y, xPrev;
  KI ki, kiPrev;

  // Can only draw up to a half Moon crescent. Any greater means to draw a
  // crescent coming in from the inverse angle with the two colors reversed.
  if (rProp <= 0.5)
    rProp = 1.0 - rProp*2.0;
  else {
    rProp = (rProp - 0.5)*2.0;
    rRotate += rDegHalf;
    SwapN(ki0, ki1);
  }

  // Initialize variables.
  xc = (real)(x1 + x2) / 2.0; yc = (real)(y1 + y2) / 2.0;
  rx = (real)(x2 - x1) / 2.0; ry = (real)(y2 - y1) / 2.0;
  rSq = Sq(rx) + rx/2.0;
  dxy = rx / ry;
  RotateR2Init(rS, rC, rRotate);
  x1 = Max(x1, 0); x2 = Min(x2, gs.xWin-1);
  y1 = Max(y1, 0); y2 = Min(y2, gs.yWin-1);

  // Draw one row of the ellipse at a time.
  for (y = y1; y <= y2; y++) {
    xPrev = ki = kiPrev = ~0;
    for (x = x1; x <= x2; x++) {
      dx = (real)x - xc;
      dy = (real)y - yc;
      if (rx != ry)
        dy *= dxy;
      if (Sq(dx) + Sq(dy) > rSq) {
        if (xPrev != ~0)
          break;
        continue;
      }
      if (rRotate != 0.0) {
        RotateR2(dx, dy, dx2, dy2, rS, rC);
      } else {
        dx2 = dx; dy2 = dy;
      }
      ki = ki1;
      if (dx2 > 0.0) {
        dx2 /= rProp;
        if (Sq(dx2) + Sq(dy2) > rSq)
          ki = ki0;
      }
      // Don't draw anything in this row until the color changes.
      if (xPrev == ~0)
        xPrev = x;
      if (kiPrev == ~0)
        kiPrev = ki;
      if (ki != kiPrev) {
        DrawColor(kiPrev);
        DrawBlock(xPrev, y, x-1, y);
        kiPrev = ki;
        xPrev = x;
      }
    }
    // Draw the rest of the color in this row, if any.
    if (ki != ~0) {
      DrawColor(ki);
      DrawBlock(xPrev, y, x-1, y);
    }
  }
}


/* Print a string of text on the graphic window at specified location. To  */
/* do this we either use Astrolog's own "font" (6x10) and draw each letter */
/* separately, or else specify system fonts for PostScript and metafiles.  */

void DrawSz(CONST char *sz, int x, int y, int dt)
{
  int s = gi.nScale, c = gi.kiCur, cch, i;

  cch = CchSz(sz);
  if (dt & dtScale2)
    gi.nScale = gi.nScaleTextT;
  else if (!(dt & dtScale))
    gi.nScale = gi.nScaleT;
  x += gi.nScale;
  if (!(dt & dtLeft))
    x -= cch*xFont*gi.nScale/2;
  if (dt & dtBottom)
    y -= (yFont-3)*gi.nScale;
  else if (!(dt & dtTop))
    y -= yFont*gi.nScale/2;
  if (dt & dtErase) {
    DrawColor(gi.kiOff);
    DrawBlock(x, y, x+xFont*gi.nScale*cch-1, y+(yFont-2)*gi.nScale);
  }
  DrawColor(c);
#ifdef PS
  if (gs.ft == ftPS && gs.nFont > 0) {
    PsFont(4);
    fprintf(gi.file, "%d %d(%s)center\n",
      x + xFont*gi.nScale*cch/2, y + yFont*gi.nScale/2, sz);
    gi.nScale = s;
    return;
  }
#endif
  while (*sz) {
#ifdef META
    if (gs.ft == ftWmf && gs.nFont > 0) {
      gi.nFontDes = 3;
      gi.kiTextDes = gi.kiCur;
      gi.nAlignDes = 0x6 | 0 /* Center | Top */;
      MetaSelect();
      MetaTextOut(x, y, 1);
      MetaWord(WFromBB(*sz, 0));
    } else
#endif
    {
      i = (uchar)*sz-' ';
      if (i < 256-32)
        DrawTurtle(szDrawCh[i], x, y);
    }
    x += xFont*gi.nScale;
    sz++;
  }
  gi.nScale = s;
}



//                                      ATGCLVLSSCAPc
CONST char szSignFontEnigma[cSign+3] = "1234567890-=+";

/* Draw the glyph of a sign at particular coordinates on the screen.    */
/* To do this we either use Astrolog's turtle vector representation or  */
/* we may specify a system font character for PostScript and metafiles. */

void DrawSign(int i, int x, int y)
{
#ifdef WINANY
  int nFont = gs.nFont/1000%10;

  if (!gi.fFile && nFont > 0) {
    if (nFont == 1)
      WinDrawGlyph('^' + i - 1, x, y, 0);
    else if (nFont == 2)
      WinDrawGlyph('A' + i - 1, x, y, 1);
    else if (nFont == 3)
      WinDrawGlyph(szSignFontEnigma[i == sCap && gs.nGlyphs/1000 > 1 ? cSign :
        i-1], x, y-gi.nScale, 2);
    else
      WinDrawGlyphW(0x2648 + i - 1, x, y, 100);
    return;
  }
#endif
#ifdef PS
  if (gs.ft == ftPS && gs.nFont > 0) {
    PsFont(1);
    fprintf(gi.file, "%d %d(%c)center\n", x, y, 'A' + i - 1);
    return;
  }
#endif
#ifdef META
  if (gs.ft == ftWmf && gs.nFont > 0) {
    gi.nFontDes = 1;
    gi.kiTextDes = gi.kiCur;
    gi.nAlignDes = 0x6 | 0x8 /* Center | Bottom */;
    MetaSelect();
    MetaTextOut(x, y+4*gi.nScale, 1);
    MetaWord(WFromBB('^' + i - 1, 0));
    return;
  }
#endif
  if (i == sCap && gs.nGlyphs/1000 > 1)
    i = cSign+1;
  if (gi.nScale % 3 == 0 && szDrawSign3[i][0]) {
    gi.nScale /= 3;
    DrawTurtle(szDrawSign3[i], x, y);  // Special extra hi-res sign glyphs.
    gi.nScale *= 3;
  } else if (!FOdd(gi.nScale) && szDrawSign2[i][0]) {
    gi.nScale >>= 1;
    DrawTurtle(szDrawSign2[i], x, y);  // Special hi-res sign glyphs.
    gi.nScale <<= 1;
  } else
    DrawTurtle(szDrawSign[i], x, y);
}


/* Draw the number of a house at particular coordinates on the screen. */
/* We either use a turtle vector or write a number in a system font.   */

void DrawHouse(int i, int x, int y)
{
#ifdef WINANY
  int nFont = gs.nFont/100%10;

  if (!gi.fFile && nFont > 0) {
    WinDrawGlyphW(0x2460 + i - 1, x, y, 100);
    return;
  }
#endif
#ifdef PS
  if (gs.ft == ftPS && gs.nFont > 0) {
    PsFont(3);
    fprintf(gi.file, "%d %d(%d)center\n", x, y, i);
    return;
  }
#endif
#ifdef META
  if (gs.ft == ftWmf && gs.nFont > 0) {
    gi.nFontDes = 2;
    gi.kiTextDes = gi.kiCur;
    gi.nAlignDes = 0x6 | 0x8 /* Center | Bottom */;
    MetaSelect();
    MetaTextOut(x, y+3*gi.nScale, 1 + (i>9));
    MetaWord(WFromBB(i > 9 ? '1' : '0'+i, i > 9 ? '0'+i-10 : 0));
    return;
  }
#endif
  if (gi.nScale % 3 == 0 && szDrawHouse3[i][0]) {
    gi.nScale /= 3;
    DrawTurtle(szDrawHouse3[i], x, y);  // Special extra hi-res house numbers.
    gi.nScale *= 3;
  } else if (!FOdd(gi.nScale) && szDrawHouse2[i][0]) {
    gi.nScale >>= 1;
    DrawTurtle(szDrawHouse2[i], x, y);  // Special hi-res house numbers.
    gi.nScale <<= 1;
  } else
    DrawTurtle(szDrawHouse[i], x, y);
}


#ifdef VECTOR
//                                        ESMMVMJSUNPccpjvnslpveA23I56D89M12
CONST char szObjectFontAstro[cuspHi+2] = ";QRSTUVWXYZ     <> ?  a  c     b  ";
CONST char szObjectFontEnigma[uranHi+2] =
// ESMMVMJSUNPcc___p___j___vnslpveA23I56D89M12___vchzkaavp
  "eabcdfghijkw_\373\374\300{},   A        M  \311yz!@#$%&";
#endif
#ifdef WINANY
CONST WCHAR wzObjectFontUnicode[oCore+1] = {
  0x2295, 0x2609, 0x263d, 0x263f, 0x2640, 0x2642, 0x2643, 0x2644, 0x2645,
  0x2646, 0x2647, 0/*26b7*/, 0/*26b3*/, 0/*26b4*/, 0/*26b5*/, 0/*26b6*/,
  0x260a, 0x260b, 0/*26b8*/, 0/*2297*/, 0, 0};
#endif

/* Draw the glyph of an object at particular coordinates on the screen. */

void DrawObject(int obj, int x, int y)
{
  char szGlyph[4];
  flag fNoText = fFalse;
  int col;
#ifdef WINANY
  int nFont = gs.nFont/10%10;
#endif

  if (!gs.fLabel)    // If inhibiting labels, then do nothing.
    return;
  if (obj < 0) {
    obj = -obj-1;
    fNoText = fTrue;
  }

  col = kObjB[obj];
#ifdef EXPRESS
  if (!us.fExpOff && FSzSet(us.szExpColObj)) {
    ExpSetN(iLetterY, obj);
    ExpSetN(iLetterZ, col);
    ParseExpression(us.szExpColObj);
    col = NExpGet(iLetterZ);
    if (!FValidColor(col))
      col = 0;
  }
#endif
  DrawColor(col);

#ifdef WINANY
  if (!gi.fFile) {
    if (nFont == 2 && obj <= cuspHi && szObjectFontAstro[obj] != ' ') {
      WinDrawGlyph(szObjectFontAstro[obj], x, y, 1);
      return;
    } else if (nFont == 3 && obj <= uranHi && szObjectFontEnigma[obj] != ' ') {
      WinDrawGlyph(szObjectFontEnigma[obj], x, y-gi.nScale, 2);
      return;
    } else if (nFont == 4 && obj <= oCore && wzObjectFontUnicode[obj] != 0) {
      WinDrawGlyphW(wzObjectFontUnicode[obj], x, y,
        FBetween(obj, oMer, oMar) ? 95 : 135);
      return;
    }
  }
#endif
#ifdef PS
  if (gs.ft == ftPS && gs.nFont == 1 &&
    obj < uranLo && szObjectFontAstro[obj] != ' ') {
    PsFont(2);
    fprintf(gi.file, "%d %d(%c)center\n", x, y, szObjectFontAstro[obj]);
    return;
  }
#endif
#ifdef META
  if (gs.ft == ftWmf && gs.nFont/10%10 == 2 &&
    obj < uranLo && szObjectFontAstro[obj] != ' ') {
    gi.nFontDes = 4;
    gi.kiTextDes = gi.kiCur;
    gi.nAlignDes = 0x6 | 0x8 /* Center | Bottom */;
    MetaSelect();
    MetaTextOut(x, y+5*gi.nScale, 1);
    MetaWord(WFromBB(szObjectFontAstro[obj], 0));
    return;
  }
#endif

  // Adjust glyph to alternate versions of it, if necessary.
  if (szDrawObject[obj] == szDrawObjectDef[obj]) {
    if (obj == oUra) {
      if ((gs.nGlyphs/100)%10 > 1)
        obj = cObj + 1;
    } else if (obj == oPlu) {
      if ((gs.nGlyphs/10)%10 > 1)
        obj = cObj + (((gs.nGlyphs/10)%10 > 2) ? 4 : 2);
    } else if (obj == oLil) {
      if (gs.nGlyphs%10 > 1)
        obj = cObj + 3;
    } else if (us.fHouseAngle && FAngle(obj))
      obj = cObj + 5 + ((obj - cuspLo) / 3);
  }

  // Draw the object's glyph.
  if (FOdd(gi.nScale) || !szDrawObject2[obj][0]) {
    if (ChCap(szDrawObject[obj][0]) != 'T') {
      DrawTurtle(szDrawObject[obj], x, y);
      return;
    }
  } else {
    if (ChCap(szDrawObject2[obj][0]) != 'T') {
      // Draw special hi-res object glyph.
      gi.nScale >>= 1;
      DrawTurtle(szDrawObject2[obj], x, y);
      gi.nScale <<= 1;
      return;
    }
  }

  // Normally can just draw the glyph, however some objects don't have glyphs
  // (like stars) so for these draw their three letter abbreviation.

  if (fNoText)
    return;
  sprintf(szGlyph, "%.3s", szObjDisp[obj]);
#ifdef CONSTEL
  // If doing constellations, give stars more correct astronomical names.

  if (gs.fConstel) {
    if (obj == oOri && szObjDisp[oOri] == szObjName[oOri])
      sprintf(szGlyph, "Aln");    // Alnilam, normally "Orion"
    else if (obj == oAnd && szObjDisp[oAnd] == szObjName[oAnd])
      sprintf(szGlyph, "M31");    // M31, normally "Andromeda"
  }
#endif
  DrawSz(szGlyph, x, y, dtCent | dtScale2);
}


#ifdef SWISS
/* Set a single point on the screen, whose color is a grayscale based on */
/* the passed in star magnitude. This is one of the few areas in the     */
/* program that works with more than a 16 color palette.                 */

void DrawStar(int x, int y, ES *pes)
{
#ifdef WINANY
  KV kv;
  int n;

  // Determine star color.
  if (!gs.fColor)
    kv = rgbbmp[gi.kiCur];
  else if (pes->ki != kDefault)
    kv = rgbbmp[pes->ki];
  else {
    n = 255 - (int)((pes->mag - rStarLite) / rStarSpan * 224.0);
    n = Min(n, 255); n = Max(n, 32);
    if (gs.fInverse)
      n = 255 - n;
    kv = Rgb(n, n, n);
  }

  // Draw star point.
  if (!gi.fFile
#ifdef WIN
    && wi.hdcPrint == hdcNil
#endif
    ) {
    SetPixel(wi.hdc, x, y, (COLORREF)kv);
    if (FOdd(gs.nAllStar)) {
      SetPixel(wi.hdc, x, y-1, (COLORREF)kv);
      SetPixel(wi.hdc, x-1, y, (COLORREF)kv);
      SetPixel(wi.hdc, x+1, y, (COLORREF)kv);
      SetPixel(wi.hdc, x, y+1, (COLORREF)kv);
    }
    goto LAfter;
  }
#endif /* WINANY */
  if (pes->ki != kDefault)
    DrawColor(pes->ki);
  else
    DrawColor(KStarB(pes->mag));
  if (!FOdd(gs.nAllStar))
    DrawPoint(x, y);
  else
    DrawSpot(x, y);

  // Draw star's name label.
#ifdef WINANY
LAfter:
#endif
  if (!gs.fLabel || gs.nAllStar < 2)
    return;
  if (pes->ki != kDefault)
    DrawColor(pes->ki);
  else
    DrawColor(KStarB(pes->mag));
  DrawSz(pes->pchBest, x, y + 9*gi.nScaleT, dtCent);
}
#endif


#ifdef VECTOR
//                                          C_OSTSisssqbssnbbtqPC
CONST char szAspectFontAstro[cAspect2+1] = "!\"#$'&%()+*         ";
CONST char szAspectFontEnigma[cAspect2+1] =
// COSTSisssq___b___ss___n___b___b___tqPC
  "BCEDFHGIJK\316\325N\334\321\332\333|OP";
#endif
#ifdef WINANY
CONST WCHAR wzAspectFontUnicode[cAspect2] = {
  0x260c, 0x260d, 0x25a1, 0x25b3, 0x04ff, 0, 0, 0, 0,
  'Q', 0x00b1, 0, 0, 0, 0, 0, 0, 0};
#endif

/* Draw the glyph of an aspect at particular coordinates on the screen. */
/* Again we either use Astrolog's turtle vector or a system Astro font. */

void DrawAspect(int asp, int x, int y)
{
#ifdef WINANY
  int nFont = gs.nFont%10;
#endif

  if (us.fParallel && asp <= aOpp)
    asp += cAspect;
#ifdef WINANY
  if (!gi.fFile) {
    if (nFont == 2 && szAspectFontAstro[asp-1] != ' ') {
      WinDrawGlyph(szAspectFontAstro[asp-1], x, y, 1);
      return;
    } else if (nFont == 3 && szAspectFontEnigma[asp-1] != ' ') {
      WinDrawGlyph(szAspectFontEnigma[asp-1], x, y-gi.nScale, 2);
      return;
    } else if (nFont == 4 && wzAspectFontUnicode[asp-1] != 0) {
      WinDrawGlyphW(wzAspectFontUnicode[asp-1], x, y, 100);
      return;
    }
  }
#endif
#ifdef PS
  if (gs.ft == ftPS && gs.nFont == 1 && szAspectFontAstro[asp-1] != ' ') {
    PsFont(2);
    fprintf(gi.file, "%d %d(%s%c)center\n", x, y,
      asp == aSSq || asp == aSes ? "\\" : "", szAspectFontAstro[asp-1]);
    return;
  }
#endif
#ifdef META
  if (gs.ft == ftWmf && gs.nFont/10%10 == 2 &&
    szAspectFontAstro[asp-1] != ' ') {
    gi.nFontDes = 4;
    gi.kiTextDes = gi.kiCur;
    gi.nAlignDes = 0x6 | 0x8 /* Center | Bottom */;
    MetaSelect();
    MetaTextOut(x, y+5*gi.nScale, 1);
    MetaWord(WFromBB(szAspectFontAstro[asp-1], 0));
    return;
  }
#endif
  if (FOdd(gi.nScale) || !szDrawAspect2[asp][0])
    DrawTurtle(szDrawAspect[asp], x, y);
  else {
    gi.nScale >>= 1;
    DrawTurtle(szDrawAspect2[asp], x, y);  // Special hi-res aspect glyphs.
    gi.nScale <<= 1;
  }
}


/* Convert a string segment to a positive number, updating the string to  */
/* point beyond the number chars. Return 1 if the string doesn't point to */
/* a numeric value. This is used by the DrawTurtle() routine to extract   */
/* motion vector quantities from draw strings, e.g. the "12" in "U12".    */

int NFromPch(CONST char **str)
{
  int num = 0, i = 0;

  loop {
    if (**str < '0' || **str > '9')
      return num > 0 ? num : (i < 1 ? 1 : 0);
    num = num*10+(**str)-'0';
    (*str)++;
    i++;
  }
}


/* This routine is used to draw complicated objects composed of lots of line */
/* segments on the screen, such as all the glyphs and coastline pieces. It   */
/* is passed in a string of commands defining what to draw in relative       */
/* coordinates. This is a copy of the format of the BASIC draw command found */
/* in PC's. For example, "U5R10D5L10" means go up 5 dots, right 10, down 5,  */
/* and left 10 - draw a box twice as wide as it is high.                     */

void DrawTurtle(CONST char *sz, int x0, int y0)
{
  int i, x, y, deltax, deltay;
  flag fBlank, fNoupdate;
  char szErr[cchSzDef], chCmd;

  gi.xTurtle = x0; gi.yTurtle = y0;
  while (chCmd = ChCap(*sz)) {
    sz++;

    // 'B' prefixing a command means just move the cursor, and don't draw.

    if (fBlank = (chCmd == 'B')) {
      chCmd = ChCap(*sz);
      sz++;
    }

    // 'N' prefixing a command means don't update cursor when done drawing.

    if (fNoupdate = (chCmd == 'N')) {
      chCmd = ChCap(*sz);
      sz++;
    }

    // Process the eight directional commands.

    switch (chCmd) {
    case 'U': deltax =  0; deltay = -1; break;      // Up
    case 'D': deltax =  0; deltay =  1; break;      // Down
    case 'L': deltax = -1; deltay =  0; break;      // Left
    case 'R': deltax =  1; deltay =  0; break;      // Right
    case 'E': deltax =  1; deltay = -1; break;      // NorthEast
    case 'F': deltax =  1; deltay =  1; break;      // SouthEast
    case 'G': deltax = -1; deltay =  1; break;      // SouthWest
    case 'H': deltax = -1; deltay = -1; break;      // NorthWest
    default:
      deltax = deltay = 0;
      sprintf(szErr, "Bad draw turtle action character: '%c'", chCmd);
      PrintError(szErr);
    }
    x = gi.xTurtle;
    y = gi.yTurtle;
    i = NFromPch(&sz)*gi.nScale;    // Figure out how far to draw.
    if (fBlank) {
      gi.xTurtle += deltax*i;
      gi.yTurtle += deltay*i;
    } else {
      gi.xTurtle += deltax*i;
      gi.yTurtle += deltay*i;
      DrawLine(x, y, gi.xTurtle, gi.yTurtle);
      if (fNoupdate) {
        gi.xTurtle = x;
        gi.yTurtle = y;
      }
    }
  }
}


#ifdef ATLAS
// Return the color to use for displaying an atlas city.

int KiCity(int iae)
{
  real zon;
  KI ki = kOrangeB;
  int i;
  CI ci;
#ifdef WIN
  flag fSav;
#endif

  if (!FEnsureAtlas())
    return fFalse;

  // Initialize array of offsets for each time zone area, at chart's time.
  if (iae < 0) {
    if (gs.nLabelCity == 4 && FEnsureTimezoneChanges()) {
      if (is.rgzonCol == NULL) {
        is.rgzonCol = RgAllocate(iznMax, real, "timezone color");
        if (is.rgzonCol == NULL)
          return -1;
      }
      ci = ciMain;
#ifdef WIN
      fSav = wi.fNoPopup; wi.fNoPopup = fTrue;
#endif
      for (i = 0; i < iznMax; i++) {
        DisplayTimezoneChanges(i, 0, &ci);
        is.rgzonCol[i] = ci.zon - ci.dst;
      }
#ifdef WIN
      wi.fNoPopup = fSav;
#endif
    }
    return ki;
  }

  // Determine color of this city.
  if (gs.nLabelCity <= 2) {
    i = is.rgae[iae].icn;
    if (gs.nLabelCity == 2 && (i == icnUS || i == icnCA))
      ki = kRainbowB[is.rgae[iae].istate % 7 + 1];
    else
      ki = kRainbowB[i == icnUS ? 6 : (i == icnFR ? 5 : i % 7 + 1)];
  } else if (gs.nLabelCity <= 4 && FEnsureTimezoneChanges()) {
    i = is.rgae[iae].izn;
    if (gs.nLabelCity == 4 && is.rgzonCol != NULL)
      zon = is.rgzonCol[i];
    else
      zon = ZondefFromIzn(i);
    if (zon == zonLMT)
      ki = kDkGrayB;
    else if (zon == RFloor(zon)) {
      i = (int)(zon + rDegMax) % 6;
      ki = kRainbowB[i + (i > 0) + 1];
    } else
      ki = kMagentaB;
  } else
    ki = kRainbowB[iae % 7 + 1];
  return ki;
}
#endif
#endif /* GRAPH */

/* xgeneral.cpp */
