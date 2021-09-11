/*
** Astrolog (Version 7.30) File: xdevice.cpp
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


#ifdef GRAPH
/*
******************************************************************************
** Windows Bitmap Routines.
******************************************************************************
*/

#define cbPixelK 3
#define CbColmapRow(x) ((x)*cbPixelK + 3 & ~3)
#define CbColmap(x, y) ((y) * CbColmapRow(x))
#define zColmap 65535

// Functions to set or get a pixel within a 24 bit color bitmap.

INLINE long _IbXY(CONST Bitmap *b, int x, int y)
  { return y*(b->clRow << 2) + (x * cbPixelK); }
INLINE byte *_PbXY(CONST Bitmap *b, int x, int y)
  { return &(b->rgb)[_IbXY(b, x, y)]; }
INLINE void _SetRGB(byte *pb, int r, int g, int b)
  { *pb = b; *(pb+1) = g; *(pb+2) = r; }
INLINE KV _GetP(CONST byte *pb)
  { return (*pb << 16) | (*(pb+1) << 8) | *(pb+2); }
INLINE KV _GetXY(CONST Bitmap *b, int x, int y)
  { return _GetP(_PbXY(b, x, y)); }
INLINE void _GetRGB(CONST byte *pb, int *r, int *g, int *b)
  { *b = *pb; *g = *(pb+1); *r = *(pb+2); }
void BmpSetXY(Bitmap *b, int x, int y, KV kv)
  { _SetRGB(_PbXY(b, x, y), RgbR(kv), RgbG(kv), RgbB(kv)); }
KV BmpGetXY(Bitmap *b, int x, int y)
  { return _GetXY(b, x, y); }
void SetXY(int x, int y, KI ki)
  { if (!gi.fBmp) BmSet(gi.bm, x, y, ki); else BmpSetXY(&gi.bmp, x, y, ki); }
KI GetXY(int x, int y)
  { return !gi.fBmp ? FBmGet(gi.bm, x, y) : _GetXY(&gi.bmp, x, y); }
KI BmGetXY(int x, int y)
  { return !gi.fBmp ? FBmGet(gi.bm, x, y) : (_GetXY(&gi.bmp, x, y) > 0)*15; }


// Allocate or reallocate a 24 bit color bitmap to have a given size.

flag FAllocateBmp(Bitmap *b, int x, int y)
{
  char sz[cchSzDef];
  long cb;
  byte *rgb;

  if (x == b->x && y == b->y)
    return fTrue;
  if (x < 0 || y < 0 || x > zColmap || y > zColmap) {
    sprintf(sz, "Can't create color bitmap larger than %d by %d!\n",
      zColmap, zColmap);
    PrintError(sz);
    return fFalse;
  }
  cb = CbColmap(x, y);
  if (cb < 0) {
    sprintf(sz, "Can't allocate color bitmap of size %d by %d!\n", x, y);
    PrintError(sz);
    return fFalse;
  }
  if (cb != CbColmap(b->x, b->y)) {
    rgb = (byte *)PAllocate(cb, "color bitmap");
    if (rgb == NULL)
      return fFalse;
    if (b->rgb != NULL)
      DeallocateP(b->rgb);
    b->rgb = rgb;
  }
  b->x = x; b->y = y;
  b->clRow = CbColmapRow(x) >> 2;
  return fTrue;
}


// Load a bitmap from file into a 24 bit bitmap structure. This supports
// Windows bitmap files stored with 4, 8, 16, 24, or 32 bits per pixel.

flag FReadBmp(Bitmap *b, FILE *file, flag fNoHeader)
{
  byte *pb, bR, bG, bB, ch, ch2;
  KV rgkv[256], kv;
  int cbExtra, cb, x, y, z, k, i, m, n;
  long l;

  // BitmapFileHeader
  if (!fNoHeader) {
    ch = getbyte(); ch2 = getbyte();
    if (ch != 'B' || ch2 != 'M') {
      PrintError("This file does not look like a Windows bitmap.\n");
      return fFalse;
    }
    skiplong();
    skipword(); skipword();
    skiplong();
  }

  // BitmapInfo / BitmapInfoHeader
  cbExtra = getlong();
  x = NAbs((int)getlong()); y = NAbs((int)getlong());
  skipword(); z = getword();
  l = getlong();
  if (l != 0/*BI_RGB*/ && l != 3/*BI_BITFIELDS*/) {
    PrintError("This Windows bitmap can't be uncompressed.\n");
    return fFalse;
  }
  for (i = 0; i < 3; i++) {
    skiplong();
  }
  k = getlong(); skiplong();
  for (cbExtra -= 40; cbExtra > 0; cbExtra--)
    skipbyte();
  if (l == 3/*BI_BITFIELDS*/)
    for (i = 0; i < 3; i++) {
      skiplong();
    }
  if (!(z == 4 || z == 8 || z == 16 || z == 24 || z == 32)) {
    PrintError("This Windows bitmap has bad number of bits per pixel.\n");
    return fFalse;
  }

  // RgbQuad
  // Data
  if (!FAllocateBmp(b, x, y))
    return fFalse;

  // Figure out the bytes per row in this color bitmap.
  if (z == 32)
    cb = x << 2;
  else if (z == 24)
    cb = x * 3;
  else if (z == 16) {
    for (i = 0; i < 12; i++)
      skipbyte();
    cb = x << 1;
  } else {
    // Read in the color palette to translate indexes to RGB values.
    Assert(k <= 256);
    if (k == 0)
      k = (z == 8) ? 256 : 16;
    for (i = 0; i < k; i++) {
      bB = getbyte(); bG = getbyte(); bR = getbyte();
      skipbyte();
      rgkv[i] = Rgb(bR, bG, bB);
    }
    if (z == 8)
      cb = x;
    else
      cb = (x + 1) >> 1;
  }
  // Figure out the number of padding bytes after each row.
  cb = (4 - (cb & 3)) & 3;

  for (n = y-1; n >= 0; n--) {
    pb = _PbXY(b, 0, n);
    for (m = 0; m < x; m++) {
      if (z >= 24) {
        bB = getbyte(); bG = getbyte(); bR = getbyte();
        if (z == 32)
          skipbyte();
      } else {
        if (z == 16) {
          i = WRead(file);
          kv = Rgb((i >> 11) << 3, (i >> 5 & 63) << 2, (i & 31) << 3);
        } else if (z == 8) {
          ch = getbyte();
          kv = rgkv[ch];
        } else {
          if (!FOdd(m))
            ch = getbyte();
          i = FOdd(m) ? (ch & 15) : (ch >> 4);
          kv = rgkv[i];
        }
        bR = RgbR(kv); bG = RgbG(kv); bB = RgbB(kv);
      }
      _SetRGB(pb, bR, bG, bB);
      pb += cbPixelK;
    }
    for (m = 0; m < cb; m++)
      skipbyte();
  }
  return fTrue;
}


// Load a 24 bit bitmap given a filename.

flag FLoadBmp(CONST char *szFile, Bitmap *bmp, flag fNoHeader)
{
  FILE *file;
  flag fRet;

  file = FileOpen(szFile, 3, NULL);
  if (file == NULL)
    return fFalse;
  fRet = FReadBmp(bmp, file, fNoHeader);
  fclose(file);
  return fRet;
}


// Write a 24 bit bitmap to a previously opened file, in the 24 bit bitmap
// format used by Microsoft Windows for its .bmp extension files.

void WriteBmp2(CONST Bitmap *b, FILE *file)
{
  int x, y, cb;
  dword dw;

  cb = (4 - ((b->x*3) & 3)) & 3;
  // BitmapFileHeader
  PutByte('B'); PutByte('M');
  dw = 14+40 + 0 + b->y*(b->x*3+cb);
  PutLong(dw);
  PutWord(0); PutWord(0);
  PutLong(14+40 + 0);
  // BitmapInfo / BitmapInfoHeader
  PutLong(40);
  PutLong(b->x); PutLong(b->y);
  PutWord(1); PutWord(24);
  PutLong(0 /*BI_RGB*/); PutLong(0);
  PutLong(0); PutLong(0);
  PutLong(0); PutLong(0);
  // RgbQuad
  // Data
  for (y = b->y-1; y >= 0; y--) {
    for (x = 0; x < b->x; x++) {
      dw = _GetXY(b, x, y);
      PutByte(RgbB(dw)); PutByte(RgbG(dw)); PutByte(RgbR(dw));
    }
    for (x = 0; x < cb; x++)
      PutByte(0);
  }
}


// Set all pixels in a 24 bit bitmap to the specified RGB color value.

void BmpSetAll(Bitmap *b, KV kv)
{
  int x, y, nR, nG, nB;
  byte *pb;

  nR = RgbR(kv); nG = RgbG(kv); nB = RgbB(kv);
  for (y = 0; y < b->y; y++) {
    pb = _PbXY(b, 0, y);
    for (x = 0; x < b->x; x++) {
      _SetRGB(pb, nR, nG, nB);
      pb += cbPixelK;
    }
  }
}


// Copy a rectangle from one bitmap structure to a different rectangle in
// another, stretching pixels as needed. Like the Windows StretchBlt() API.

void BmpCopyBlock(CONST Bitmap *bs, int x1, int y1, int x2, int y2,
  Bitmap *bd, int x3, int y3, int x4, int y4)
{
  int xs = x2-x1+1, ys = y2-y1+1, xd = x4-x3+1, yd = y4-y3+1,
    x, y, xT, yT, nR, nG, nB;
  byte *pbDst;

  // Sanity checks of coordinate bounds, which shouldn't ever fail.
  Assert(FBetween(x1, 0, bs->x-1));
  Assert(FBetween(y1, 0, bs->y-1));
  Assert(FBetween(x2, 0, bs->x-1));
  Assert(FBetween(y2, 0, bs->y-1));
  Assert(FBetween(x3, 0, bd->x-1));
  Assert(FBetween(y3, 0, bd->y-1));
  Assert(FBetween(x4, 0, bd->x-1));
  Assert(FBetween(y4, 0, bd->y-1));

  for (y = y3; y <= y4; y++) {
    pbDst = _PbXY(bd, x3, y);
    yT = y1 + (y-y3) * ys / yd;
    for (x = x3; x <= x4; x++) {
      xT = x1 + (x-x3) * xs / xd;
      _GetRGB(_PbXY(bs, xT, yT), &nR, &nG, &nB);
      _SetRGB(pbDst, nR, nG, nB);
      pbDst += cbPixelK;
    }
  }
}


// Like BmpCopyBlock() but the source rectangle coordinates are reals instead
// of integers. Can be used to render only subsections of source pixels.

void BmpCopyBlock2(CONST Bitmap *bs, real x1, real y1, real x2, real y2,
  Bitmap *bd, int x3, int y3, int x4, int y4)
{
  int xd = x4-x3+1, yd = y4-y3+1, x, y, xT, yT, nR, nG, nB;
  real xs, ys;
  byte *pbDst;

  // Sanity checks of coordinate bounds, which shouldn't ever fail.
  Assert(FBetween(x1, 0.0, (real)bs->x - rSmall));
  Assert(FBetween(y1, 0.0, (real)bs->y - rSmall));
  Assert(FBetween(x2, 0.0, (real)bs->x - rSmall));
  Assert(FBetween(y2, 0.0, (real)bs->y - rSmall));
  Assert(FBetween(x3, 0, bd->x-1));
  Assert(FBetween(y3, 0, bd->y-1));
  Assert(FBetween(x4, 0, bd->x-1));
  Assert(FBetween(y4, 0, bd->y-1));

  xs = (x2-x1) / (real)xd;
  ys = (y2-y1) / (real)yd;
  for (y = y3; y <= y4; y++) {
    pbDst = _PbXY(bd, x3, y);
    yT = (int)(y1 + (real)(y-y3) * ys);
    for (x = x3; x <= x4; x++) {
      xT = (int)(x1 + (real)(x-x3) * xs);
      _GetRGB(_PbXY(bs, xT, yT), &nR, &nG, &nB);
      _SetRGB(pbDst, nR, nG, nB);
      pbDst += cbPixelK;
    }
  }
}


#ifdef WINANY
// Copy a 24 bit bitmap structure to a Windows DC. Since Astrolog's internal
// bitmap structure is the same as Windows, it can be done all at once.

void BmpCopyWin(CONST Bitmap *b, HDC hdc, int x, int y)
{
  BITMAPINFO bi;

  bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = cbPixelK << 3;
  bi.bmiHeader.biCompression = BI_RGB;
  bi.bmiHeader.biSizeImage = 0;
  bi.bmiHeader.biXPelsPerMeter = bi.bmiHeader.biYPelsPerMeter = 1000;
  bi.bmiHeader.biClrUsed = 0;
  bi.bmiHeader.biClrImportant = 0;
  bi.bmiColors[0].rgbBlue = bi.bmiColors[0].rgbGreen =
    bi.bmiColors[0].rgbRed = bi.bmiColors[0].rgbReserved = 0;
  bi.bmiHeader.biWidth  =  (b->x);
  bi.bmiHeader.biHeight = -(b->y);

  SetDIBitsToDevice(hdc, x, y, b->x, b->y, 0, 0, 0, b->y,
    b->rgb, (BITMAPINFO *)&bi, DIB_RGB_COLORS);
}
#endif


// Draw the background bitmap onto the specified 24 bit bitmap. Implements the
// -XI switch features.

flag FBmpDrawBack(Bitmap *bDest)
{
  Bitmap *b = &gi.bmpBack, *b2 = &gi.bmpBack2;
  int nTrans, x, y, x1, y1, x2, y2, x3, y3, x4, y4, nR, nG, nB, nRT, nGT, nBT;
  byte *pb, *pb2;
  KV kv;
  static KV kvLast = -1;
  static int nTransLast = 0, xLast = 0, yLast = 0;

  // Don't draw background if user doesn't want to.
  if (!gi.fBmp)
    return fFalse;

  // Don't draw background if entire chart will be covered with world map.
  if (gi.bmpWorld.rgb != NULL &&
    (gi.nMode == gAstroGraph || (gi.nMode == gWorldMap && !gs.fMollewide)))
    return fFalse;

  // Don't do anything if bitmap empty or transparent enough to be invisible.
  nTrans = (int)(gs.rBackPct * 256.0 / 100.0);
  if (b->rgb == NULL || nTrans <= 0)
    return fFalse;

  // Cache bitmap with proper percentage blend with current background color.
  kv = rgbbmp[gi.kiOff];
  if (b2->x != b->x || b2->y != b->y || kv != kvLast || nTrans != nTransLast) {
    if (!FAllocateBmp(b2, b->x, b->y))
      return fFalse;
    kvLast = kv;
    nTransLast = nTrans;
    nR = RgbR(kv); nG = RgbG(kv); nB = RgbB(kv);
    for (y = 0; y < b->y; y++) {
      pb = _PbXY(b, 0, y);
      pb2 = _PbXY(b2, 0, y);
      for (x = 0; x < b->x; x++) {
        _GetRGB(pb, &nRT, &nGT, &nBT);
        nRT = nR + ((nRT - nR) * nTrans >> 8);
        nGT = nG + ((nGT - nG) * nTrans >> 8);
        nBT = nB + ((nBT - nB) * nTrans >> 8);
        _SetRGB(pb2, nRT, nGT, nBT);
        pb += cbPixelK; pb2 += cbPixelK;
      }
    }
    xLast = yLast = 0;
  }

  // Determine source (on bitmap) and destination (on chart) rectangles.
  x1 = y1 = x3 = y3 = 0;
  x2 = gs.xWin; y2 = gs.yWin;
  x4 = b2->x; y4 = b2->y;
  if (gs.nBackOrient < 0) {
    if ((real)x2 / (real)y2 > (real)x4 / (real)y4) {
      x2 = y2 * x4 / y4;
      x1 = (gs.xWin - x2) >> 1;
    } else {
      y2 = x2 * y4 / x4;
      y1 = (gs.yWin - y2) >> 1;
    }
  } else if (gs.nBackOrient > 0) {
    if ((real)x4 / (real)y4 > (real)x2 / (real)y2) {
      x4 = y4 * x2 / y2;
      x3 = (b2->x - x4) >> 1;
    } else {
      y4 = x4 * y2 / x2;
      y3 = (b2->y - y4) >> 1;
    }
  }

  // For bitmaps, copy background to chart bitmap manually.
  if (gi.fFile || bDest != NULL) {
    if ((gs.ft == ftBmp && gi.fBmp) || bDest != NULL)
      BmpCopyBlock(&gi.bmpBack2, x3, y3, x3+x4-1, y3+y4-1,
        bDest != NULL ? bDest : &gi.bmp, x1, y1, x1+x2-1, y1+y2-1);
    return fTrue;
  }

#ifdef WINANY
  // For Windows, draw background bitmap on window using Windows API.
  if (wi.hdcBack == NULL) {
    wi.hdcBack = CreateCompatibleDC(wi.hdc);
    SetMapMode(wi.hdcBack, MM_TEXT);
    SetWindowOrg(wi.hdcBack, 0, 0); SetViewportOrg(wi.hdcBack, 0, 0);
  }

  if (b2->x != xLast || b2->y != yLast) {
    // If background has changed size, create a new Windows Bitmap for it.
    SelectObject(wi.hdcBack, wi.hbmpPrev);
    if (wi.hbmpBack != NULL)
      DeleteObject(wi.hbmpBack);
    wi.hbmpBack = CreateCompatibleBitmap(wi.hdc, b2->x, b2->y);
    if (wi.hbmpBack == NULL) {
      PrintError("Failed to create color background bitmap.");
      return fFalse;
    }
    wi.hbmpPrev = (HBITMAP)SelectObject(wi.hdcBack, wi.hbmpBack);
    xLast = b2->x; yLast = b2->y;
    BmpCopyWin(b2, wi.hdcBack, 0, 0);
  }
  SetStretchBltMode(wi.hdc, COLORONCOLOR);
  StretchBlt(wi.hdc, x1, y1, x2, y2, wi.hdcBack, x3, y3, x4, y4, SRCCOPY);
#endif
  return fTrue;
}


// Draw the world map bitmap upon the specified 24 bit bitmap. This draws the
// world in the appropriate projection for various Astrolog charts.

flag FBmpDrawMap()
{
  Bitmap *bmp = &gi.bmp;
  int nScl = 1, xc, yc, zc, x1, x2, y1, y2, xi, yi, n, n2;
  real deg = Mod(rDegMax - gs.rRot), lonS, latS, rxc, ryc, rzc,
    lon, lat, lat0, rT, rLen, sint, cost, sina, cosa;
  KV kv;

  // Do nothing if not drawing bitmaps, or if the Earth bitmap fails to load.
  if (!gi.fBmp || (gi.fFile && gs.ft != ftBmp))
    return fFalse;
  if (gi.bmpWorld.rgb == NULL && !FLoadBmp(BITMAP_EARTH, &gi.bmpWorld, fFalse))
    return fFalse;
#ifdef WINANY
  if (!gi.fFile) {
    if (!FAllocateBmp(&wi.bmpWin, gs.xWin, gs.yWin))
      return fFalse;
    bmp = &wi.bmpWin;
  }
#endif

  // Compute center coordinates and horizontal map dimensions.
  xc = (gs.xWin >> 1) - !FOdd(gs.xWin); yc = (gs.yWin >> 1) - !FOdd(gs.yWin);
  zc = Max(xc, yc);
  rxc = (real)xc; ryc = (real)yc; rzc = (real)zc;
  x1 = (int)((real)gi.bmpWorld.x * deg / rDegMax);
  x2 = (int)((real)gs.xWin       * deg / rDegMax);

  // Draw map on a -XW rectangular world map.
  if (gi.nMode == gAstroGraph || (gi.nMode == gWorldMap && !gs.fMollewide)) {
    if (x1 == 0 || x2 == 0)
      BmpCopyBlock(&gi.bmpWorld, 0, 0, gi.bmpWorld.x-1, gi.bmpWorld.y-1,
        bmp, 0, 0, gs.xWin-1, gs.yWin-1);
    else {
      BmpCopyBlock(&gi.bmpWorld, 0, 0, x1-1, gi.bmpWorld.y-1,
        bmp, gs.xWin-x2-1, 0, gs.xWin-1, gs.yWin-1);
      BmpCopyBlock(&gi.bmpWorld, x1, 0, gi.bmpWorld.x-1, gi.bmpWorld.y-1,
        bmp, 0, 0, gs.xWin-x2, gs.yWin-1);
    }

  // Draw map on a -XW0 Mollewide projection world map.
  } else if (gi.nMode == gWorldMap && gs.fMollewide) {
    if (!FBmpDrawBack(bmp))
      BmpSetAll(bmp, rgbbmp[gi.kiOff]);
    for (y2 = 0; y2 < gs.yWin; y2++) {
      y1 = y2 * gi.bmpWorld.y / gs.yWin;
      rT = RMollewide((real)y2 * rDegHalf / (real)gs.yWin - rDegQuad);
      n = (gs.xWin - (int)(rT * (real)gs.xWin / rDegHalf)) >> 1;
      if (x1 == 0 || x2 == 0)
        BmpCopyBlock(&gi.bmpWorld, 0, y1, gi.bmpWorld.x-1, y1,
          bmp, n, y2, gs.xWin-1-n, y2);
      else {
        n2 = (int)(rT * (real)(x2 - xc) / rDegHalf);
        n2 += xc;
        BmpCopyBlock(&gi.bmpWorld, 0, y1, x1-1, y1,
          bmp, gs.xWin-n2-1, y2, gs.xWin-1-n, y2);
        BmpCopyBlock(&gi.bmpWorld, x1, y1, gi.bmpWorld.x-1, y1,
          bmp, n, y2, gs.xWin-n2, y2);
      }
    }

  // Draw map on a -XP polar globe.
  } else if (gi.nMode == gPolar) {
    if (!FBmpDrawBack(bmp))
      BmpSetAll(bmp, rgbbmp[gi.kiOff]);
    lonS = Tropical(planet[oSun]);
    latS = planetalt[oSun];
    EclToEqu(&lonS, &latS);
    lonS = Mod(lonS - is.lonMC + rDegHalf - Lon);
    for (y1 = 0; y1 < gs.yWin; y1++) {
      yi = !FOdd(gs.yWin) && y1 > yc;
      for (x1 = 0; x1 < gs.xWin; x1++) {
        xi = !FOdd(gs.xWin) && x1 > xc;
        n  = xc - x1 + xi;
        n2 = yc - y1 + yi;
        if (xc > yc)
          n2 = n2 * xc / yc;
        else if (yc > xc)
          n = n * yc / xc;
        n = Sq(n) + Sq(n2);
        if (n > Sq(zc))
          continue;
        lat = RAsinD(RSqr((real)n) / rzc) * 2.0;
        if (gs.fSouth)
          lat = rDegHalf - lat;
        lon = RAngleD(x1 - xc, y1 - yc);
        lon = Mod(270.0 - gs.rRot + (!gs.fSouth ? -lon : lon));
        if (gs.fEcliptic) {
          lon = Tropical(lon);
          lat = rDegQuad - lat;
          EclToEqu(&lon, &lat);
          lon = Mod(lon - is.lonMC + rDegHalf - Lon);
          lat = rDegQuad - lat;
        }
        x2 = (int)(lon * ((real)gi.bmpWorld.x - rSmall) / rDegMax);
        y2 = (int)(lat * ((real)gi.bmpWorld.y - rSmall) / rDegHalf);
        kv = _GetXY(&gi.bmpWorld, x2, y2);
        if (gs.fMollewide &&
          SphDistance(lonS, latS, lon, rDegQuad - lat) > rDegQuad)
          kv = Rgb(RgbR(kv) / 3, RgbG(kv) / 3, RgbB(kv) / 3);
        BmpSetXY(bmp, x1, y1, kv);
      }
    }

  // Draw map on a -XG globe.
  } else if (gi.nMode == gGlobe) {
    if (!FBmpDrawBack(bmp))
      BmpSetAll(bmp, rgbbmp[gi.kiOff]);
    if (gs.rTilt != 0.0) {
      sint = RSinD(-gs.rTilt);
      cost = RCosD(-gs.rTilt);
    }
    lonS = Tropical(planet[oSun]);
    latS = planetalt[oSun];
    EclToEqu(&lonS, &latS);
    lonS = Mod(lonS - is.lonMC + rDegHalf - Lon);
    for (y1 = 0; y1 < gs.yWin; y1++) {
      yi = !FOdd(gs.yWin) && y1 > yc;
      rT = (ryc - (real)y1) / ryc;
      if (rT < -1.0)    // Roundoff may put it slightly outside Acos range.
        rT = -1.0;
      else if (rT > 1.0)
        rT = 1.0;
      lat0 = RAcosD(rT);
      n = xc; n2 = yc - y1;
      if (xc > yc)
        n2 = n2 * xc / yc;
      else if (yc > xc)
        n = n * yc / xc;
      rT = (real)(Sq(n) - Sq(n2));
      rLen = rT >= 0.0 ? RSqr(rT) : rSmall;
      if (rLen < rSmall)
        rLen = 1.0;
      sina = RSinD(rDegQuad - lat0);
      cosa = RCosD(rDegQuad - lat0);
      for (x1 = 0; x1 < gs.xWin; x1++) {
        xi = !FOdd(gs.xWin) && x1 > xc;
        n  = xc - x1 + xi;
        n2 = yc - y1 + yi;
        if (xc > yc)
          n2 = n2 * xc / yc;
        else if (yc > xc)
          n = n * yc / xc;
        n = Sq(n) + Sq(n2);
        if (n > Sq(zc))
          continue;
        lon = (rxc - (real)x1) / rxc;
        rT = lon / rLen * rzc;
        if (rT < -1.0)    // Roundoff may put it slightly outside Acos range.
          rT = -1.0;
        else if (rT > 1.0)
          rT = 1.0;
        lon = Mod(RAcosD(rT));
        lat = lat0;
        if (gs.rTilt != 0.0) {
          lat = rDegQuad - lat;
          CoorXformFast(&lon, &lat, RSinD(lon), RCosD(lon),
            sina, cosa, sint, cost);
          lat = rDegQuad - lat;
        }
        lon = Mod(lon - gs.rRot);
        if (gs.fEcliptic) {
          lon = Tropical(lon);
          lat = rDegQuad - lat;
          EclToEqu(&lon, &lat);
          lon = Mod(lon - is.lonMC + rDegHalf - Lon);
          lat = rDegQuad - lat;
        }
        x2 = (int)(lon * ((real)gi.bmpWorld.x - rSmall) / rDegMax);
        y2 = (int)(lat * ((real)gi.bmpWorld.y - rSmall) / rDegHalf);
        kv = _GetXY(&gi.bmpWorld, x2, y2);
        if (gs.fMollewide &&
          SphDistance(lonS, latS, lon, rDegQuad - lat) > rDegQuad)
          kv = Rgb(RgbR(kv) / 3, RgbG(kv) / 3, RgbB(kv) / 3);
        BmpSetXY(bmp, x1, y1, kv);
      }
    }
  }

#ifdef WINANY
  if (!gi.fFile)
    BmpCopyWin(bmp, wi.hdc, 0, 0);
#endif
  return fTrue;
}


// Draw a subsection of the world map bitmap upon a section of the specified
// 24 bit bitmap. Called from the graphic -Nl switch local space chart.

flag FBmpDrawMap2(int x1, int y1, int x2, int y2,
  real rx1, real ry1, real rx2, real ry2)
{
  Bitmap *bmp = &gi.bmp;
  int x12;
  real rx, ry, x3, y3, x4, y4, x34;

  if (!gi.fBmp || (gi.fFile && gs.ft != ftBmp))
    return fFalse;
  if (gi.bmpWorld.rgb == NULL && !FLoadBmp(BITMAP_EARTH, &gi.bmpWorld, fFalse))
    return fFalse;
#ifdef WINANY
  if (!gi.fFile) {
    if (!FAllocateBmp(&wi.bmpWin, gs.xWin, gs.yWin))
      return fFalse;
    bmp = &wi.bmpWin;
  }
#endif

  rx = (real)gi.bmpWorld.x / rDegMax; ry = (real)gi.bmpWorld.y / rDegHalf;
  BmpSetAll(bmp, rgbbmp[gi.kiOff]);
  rx1 = Mod(rx1); rx2 = Mod(rx2);
  x3 = rx1 * rx; y3 = ry1 * ry;
  x4 = rx2 * rx; y4 = ry2 * ry;
  if (x3 <= x4) {
    // In most cases, just copy the entire rectangle all at once.
    BmpCopyBlock2(&gi.bmpWorld, x3, y3, x4, y4, bmp, x1, y1, x2, y2);
  } else {
    // If viewport spans 180W/E, then have to copy twice from Earth bitmap.
    x34 = (real)gi.bmpWorld.x - rSmall;
    x12 = x1 + (int)((real)(x2-x1+1) * (x34 - x3) / (x34 - x3 + x4));
    BmpCopyBlock2(&gi.bmpWorld, x3, y3, x34, y4, bmp, x1,    y1, x12, y2);
    BmpCopyBlock2(&gi.bmpWorld, 0,  y3, x4,  y4, bmp, x12+1, y1, x2,  y2);
  }

#ifdef WINANY
  if (!gi.fFile)
    BmpCopyWin(bmp, wi.hdc, 0, 0);
#endif
  return fTrue;
}


/*
******************************************************************************
** Bitmap File Routines.
******************************************************************************
*/

// Write the bitmap array to a previously opened file in a format that can be
// read in by the Unix X11 commands bitmap and xsetroot. The 'mode' parameter
// defines how much white space is put in the file.

void WriteXBitmap(FILE *file, CONST char *name, char mode)
{
  int x, y, i, temp = 0;
  uint value;
  char szT[cchSzDef], *pchStart, *pchEnd;

  // Determine variable name from filename.
  sprintf(szT, "%s", name);
  for (pchEnd = szT; *pchEnd != chNull; pchEnd++)
    ;
  for (pchStart = pchEnd; pchStart > szT &&
    *(pchStart-1) != '/' && *(pchStart-1) != '\\'; pchStart--)
    ;
  for (pchEnd = pchStart; *pchEnd != chNull && *pchEnd != '.'; pchEnd++)
    ;
  *pchEnd = chNull;

  // Output file header.
  fprintf(file, "#define %s_width %d\n" , pchStart, gs.xWin);
  fprintf(file, "#define %s_height %d\n", pchStart, gs.yWin);
  fprintf(file, "static %s %s_bits[] = {",
    mode != 'V' ? "char" : "short", pchStart);
  for (y = 0; y < gs.yWin; y++) {
    x = 0;
    do {

      // Process each row, eight columns at a time.
      if (y + x > 0)
        fprintf(file, ",");
      if (temp == 0)
        fprintf(file, "\n%s",
          mode == 'N' ? "  " : (mode == 'C' ? " " : ""));
      value = 0;
      for (i = (mode != 'V' ? 7 : 15); i >= 0; i--)
        value = (value << 1) + (!(BmGetXY(x+i, y)^
          (gs.fInverse*15))^gs.fInverse && (x + i < gs.xWin));
      if (mode == 'N')
        putc(' ', file);
      fprintf(file, "0x");
      if (mode == 'V')
        fprintf(file, "%c%c",
          ChHex(value >> 12), ChHex((value >> 8) & 15));
      fprintf(file, "%c%c",
        ChHex((value >> 4) & 15), ChHex(value & 15));
      temp++;

      // Is it time to skip to the next line while writing the file yet?
      if ((mode == 'N' && temp >= 12) ||
          (mode == 'C' && temp >= 15) ||
          (mode == 'V' && temp >= 11))
        temp = 0;
      x += (mode != 'V' ? 8 : 16);
    } while (x < gs.xWin);
  }
  fprintf(file, "};\n");
}


// Write the bitmap array to a previously opened file in a simple boolean
// Ascii rectangle, one char per pixel, in which '#' represents an off bit and
// '-' an on bit. The output format is identical to the format generated by
// the Unix bmtoa command, and it can be converted into a bitmap with atobm.

void WriteAscii(FILE *file)
{
  int x, y, i;

  for (y = 0; y < gs.yWin; y++) {
    for (x = 0; x < gs.xWin; x++) {
      i = BmGetXY(x, y);
      if (gs.fColor)
        putc(ChHex(i), file);
      else
        putc(i ? '-' : '#', file);
    }
    putc('\n', file);
  }
}


// Write the bitmap array to a previously opened file in the bitmap format
// used in Microsoft Windows for its .bmp extension files. This is a pretty
// efficient format, only requiring a small header, and one bit per pixel
// for monochrome graphics, or four bits per pixel for 16 color bitmaps.

void WriteBmp(FILE *file)
{
  int x, y;
  dword value;

  // BitmapFileHeader
  PutByte('B'); PutByte('M');
  PutLong(14+40 + (gs.fColor ? 64 : 8) +
    (long)4*gs.yWin*(((gs.xWin-1) >> (gs.fColor ? 3 : 5))+1));
  PutWord(0); PutWord(0);
  PutLong(14+40 + (gs.fColor ? 64 : 8));
  // BitmapInfo / BitmapInfoHeader
  PutLong(40);
  PutLong(gs.xWin); PutLong(gs.yWin);
  PutWord(1); PutWord(gs.fColor ? 4 : 1);
  PutLong(0 /*BI_RGB*/); PutLong(0);
  PutLong(0); PutLong(0);
  PutLong(0); PutLong(0);
  // RgbQuad
  if (gs.fColor)
    for (x = 0; x < 16; x++) {
      PutByte(RgbB(rgbbmp[x])); PutByte(RgbG(rgbbmp[x]));
      PutByte(RgbR(rgbbmp[x])); PutByte(0);
    }
  else {
    PutLong(0);
    PutByte(255); PutByte(255); PutByte(255); PutByte(0);
  }
  // Data
  for (y = gs.yWin-1; y >= 0; y--) {
    value = 0;
    for (x = 0; x < gs.xWin; x++) {
      if ((x & (gs.fColor ? 7 : 31)) == 0 && x > 0) {
        PutLong(value);
        value = 0;
      }
      if (gs.fColor)
        value |= (dword)FBmGet(gi.bm, x, y) << ((x & 7 ^ 1) << 2);
      else
        if (FBmGet(gi.bm, x, y))
          value |= (dword)1 << (x & 31 ^ 7);
    }
    PutLong(value);
  }
}


// Begin the work of creating a graphics file. Prompt for a filename if need
// be, and if valid, create the file and open it for writing.

flag BeginFileX()
{
  char sz[cchSzMax];

  if (us.fNoWrite)
    return fFalse;
#ifdef WIN
  if (gi.szFileOut == NULL)
    return fFalse;
#endif

#ifndef WIN
  if (gi.szFileOut == NULL && ((gs.ft == ftBmp && gs.chBmpMode == 'B') ||
#ifdef PS
    gi.fEps ||
#endif
    gs.ft == ftWmf || gs.ft == ftWire)) {
    sprintf(sz, "(It is recommended to specify an extension of '.%s'.)\n",
      gs.ft == ftBmp ? "bmp" :
#ifdef WIRE
      (gs.ft == ftWire ? "dw" :
#endif
#ifdef PS
      (gi.fEps ? "eps" : "wmf")
#else
      "wmf"
#endif
#ifdef WIRE
      )
#endif
      );
    PrintSzScreen(sz);
  }
#endif // WIN

  loop {
#ifndef WIN
    if (gi.szFileOut == NULL) {
      sprintf(sz, "Enter name of file to write %s to",
        gs.ft == ftBmp ? "bitmap" : (gs.ft == ftPS ? "PostScript" :
        (gs.ft == ftWmf ? "metafile" : "wireframe")));
      InputString(sz, sz);
      gi.szFileOut = SzPersist(sz);
   }
#else
    // If autosaving in potentially rapid succession, ensure the file isn't
    // being opened by some other application before saving over it again.
    if (wi.fAutoSave) {
      if (wi.hMutex == NULL)
        wi.hMutex = CreateMutex(NULL, fFalse, szAppName);
      if (wi.hMutex != NULL)
        WaitForSingleObject(wi.hMutex, 1000);
    }
#endif
    gi.file = fopen(gi.szFileOut, (gs.ft == ftBmp && gs.chBmpMode != 'B') ||
      gs.ft == ftPS || gs.ft == ftWire ? "w" : "wb");
    if (gi.file != NULL)
      break;
#ifdef WIN
    if (wi.fAutoSave)
      break;
#endif
    sprintf(sz, "Couldn't create output file: %s", gi.szFileOut);
    PrintWarning(sz);
    gi.szFileOut = NULL;
#ifdef WIN
    break;
#endif
  }
  return gi.file != NULL;
}


// Finish up the work of creating a graphics file. This basically consists of
// just calling the appropriate routine to actually write the data in memory
// to a file for bitmaps and metafiles, although for PostScript just close the
// file as were already writing while creating the chart.

void EndFileX()
{
  if (gi.file == NULL)
    return;
  if (gs.ft == ftBmp) {
    PrintProgress("Writing chart bitmap to file.");
    if (gs.chBmpMode == 'B') {
      if (!gi.fBmp)
        WriteBmp(gi.file);
      else
        WriteBmp2(&gi.bmp, gi.file);
    } else if (gs.chBmpMode == 'A')
      WriteAscii(gi.file);
    else
      WriteXBitmap(gi.file, gi.szFileOut, gs.chBmpMode);
  }
#ifdef PS
  else if (gs.ft == ftPS)
    PsEnd();
#endif
#ifdef META
  else if (gs.ft == ftWmf) {
    PrintProgress("Writing metafile to file.");
    WriteMeta(gi.file);
  }
#endif
#ifdef WIRE
  else if (gs.ft == ftWire) {
    PrintProgress("Writing wireframe to file.");
    WriteWire(gi.file);
  }
#endif
  fclose(gi.file);
#ifdef WIN
  if (wi.fAutoSave && wi.hMutex != NULL)
    ReleaseMutex(wi.hMutex);
  if (wi.wCmd == cmdSaveWallTile || wi.wCmd == cmdSaveWallCenter ||
    wi.wCmd == cmdSaveWallStretch || wi.wCmd == cmdSaveWallFit ||
    wi.wCmd == cmdSaveWallFill) {
    WriteProfileString("Desktop", "TileWallpaper",
      wi.wCmd == cmdSaveWallTile ? "1" : "0");
    WriteProfileString("Desktop", "WallpaperStyle",
      wi.wCmd == cmdSaveWallStretch ? "2" : (wi.wCmd == cmdSaveWallFit ? "6" :
      (wi.wCmd == cmdSaveWallFill ? "10" : "0")));
    SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, gi.szFileOut,
      SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
    wi.wCmd = 0;
  }
#endif
}


#ifdef PS
/*
******************************************************************************
** PostScript File Routines.
******************************************************************************
*/

// Table of PostScript header alias lines used by the program.

CONST char szPsFunctions[] =
"/languagelevel where{pop languagelevel}{1}ifelse"
" 2 lt{\n"
"/sf{exch findfont exch"
" dup type/arraytype eq{makefont}{scalefont}ifelse setfont}bind def\n"
"/rf{gsave newpath\n"
"4 -2 roll moveto"
" dup 0 exch rlineto exch 0 rlineto neg 0 exch rlineto closepath\n"
"fill grestore}bind def\n"
"/rc{newpath\n"
"4 -2 roll moveto"
" dup 0 exch rlineto exch 0 rlineto neg 0 exch rlineto closepath\n"
"clip newpath}bind def\n"
"}{/sf/selectfont load def/rf/rectfill load def"
"/rc/rectclip load def}ifelse\n"
"/center{0 begin gsave dup 4 2 roll"
" translate newpath 0 0 moveto"
" false charpath flattenpath pathbbox"
" /URy exch def/URx exch def/LLy exch def/LLx exch def"
" URx LLx sub 0.5 mul LLx add neg URy LLy sub 0.5 mul LLy add neg"
" 0 0 moveto rmoveto"
" show grestore end}bind def\n"
"/center load 0 4 dict put\n"
"/c{setrgbcolor}bind def\n"
"/d{moveto 0 0 rlineto}bind def\n"
"/l{4 2 roll moveto lineto}bind def\n"
"/t{lineto}bind def\n"
"/el{newpath matrix currentmatrix 5 1 roll translate scale"
" 0 0 1 0 360 arc setmatrix stroke}bind def\n"
"/ef{newpath matrix currentmatrix 5 1 roll translate scale"
" 0 0 1 0 360 arc fill setmatrix stroke}bind def\n";


// Write a command to flush the PostScript buffer.

void PsStrokeForce()
{
  if (gi.cStroke > 0) {              // Render any existing path
    fprintf(gi.file, "stroke\n");
    gi.cStroke = 0;
    gi.xPen = -1;                    // Invalidate PolyLine cache
  }
}


// Indicate that a certain number of PostScript commands have been done.

void PsStroke(int n)
{
  gi.cStroke += n;
  if (gi.cStroke > 2000)    // Whenever we reach a certain limit, flush.
    PsStrokeForce();
}


// Set the type of line end to be used by PostScript commands. If linecap is
// true, then the line ends are rounded, otherwise they are squared.

void PsLineCap(flag fLineCap)
{
  if (fLineCap != gi.fLineCap) {
    PsStrokeForce();
    fprintf(gi.file, "%d setlinecap\n", fLineCap);
    gi.fLineCap = fLineCap;
  }
}


// Set the dash length to be used by PostScript line commands.

void PsDash(int dashoff)
{
  if (dashoff != gi.nDash) {
    PsStrokeForce();
    if (dashoff)
      fprintf(gi.file, "[%d %d", PSMUL, dashoff * PSMUL);
    else
      fprintf(gi.file, "[");
    fprintf(gi.file, "]0 setdash\n");
    gi.nDash = dashoff;
  }
}


// Set a linewidth size to be used by PostScript figure primitive commands.

void PsLineWidth(int linewidth)
{
  if ((real)linewidth != gi.rLineWid) {
    PsStrokeForce();
    fprintf(gi.file, "%d setlinewidth\n", linewidth);
    gi.rLineWid = (real)linewidth;
  }
}


// Set a system font and size to be used by PostScript text commands.

void PsFont(int nFont)
{
  CONST char *szFont;
  int z;

  if (nFont == gi.nFontPS || gs.nFont == 0)
    return;
  szFont = rgszFontName[nFont];
  z = PSMUL*gi.nScale;
  if (nFont == fiAstrolog) {
    szFont = "Times-Roman"; z = 4*PSMUL*gi.nScaleText;
  } else if (nFont == fiCourier) {
    szFont = "Courier"; z = 5*PSMUL*gi.nScaleText;
  }
  fprintf(gi.file, "/%s[%d 0 0 -%d 0 0]sf\n", szFont, z, z);
  gi.nFontPS = nFont;
}


// Write out initial file header information to the PostScript file.

void PsBegin()
{
  fprintf(gi.file, "%%!PS-Adobe-2.0");
  if (gi.fEps)
    fprintf(gi.file, " EPSF-2.0");
  fprintf(gi.file, "\n%%%%Title: %s\n", gi.szFileOut);
  fprintf(gi.file, "%%%%Creator: %s %s\n", szAppName, szVersionCore);
  fprintf(gi.file, "%%%%CreationDate: %s\n", szDateCore);
  if (gi.fEps) {
    fprintf(gi.file, "%%%%BoundingBox: 0 0 %d %d\n", gs.xWin, gs.yWin);
    fprintf(gi.file, "%%%%EndComments\n");
    fprintf(gi.file, "%%%%BeginSetup\n");
    fprintf(gi.file, szPsFunctions, 6 * PSMUL, 6 * PSMUL);
    fprintf(gi.file, "%%%%EndSetup\n");
    fprintf(gi.file, "0 0 %d %d rc\n", gs.xWin, gs.yWin);
  } else {
    fprintf(gi.file, "%%%%Pages: 1 1\n");
    fprintf(gi.file, "%%%%DocumentFonts: (atend)\n");
    fprintf(gi.file, "%%%%BoundingBox: %d %d %d %d\n", PSGUTTER, PSGUTTER,
      (int)(gs.xInch*72.0+rRound)-PSGUTTER,
      (int)(gs.yInch*72.0+rRound)-PSGUTTER);
    fprintf(gi.file, "%%%%EndComments\n");
    fprintf(gi.file, "%%%%BeginProcSet: common\n");
    fprintf(gi.file, szPsFunctions, 6 * PSMUL, 6 * PSMUL);
    fprintf(gi.file, "%%%%EndProcSet\n");
    fprintf(gi.file, "%%%%Page: 1 1\n");
  }
  PsFont(fiAstrolog);
  fprintf(gi.file, "gsave\n");
  PsLineWidth(!gs.fThick ? gi.nPenWid/2 : gi.nPenWid*2);
  gi.xPen = -1;
  PrintProgress("Creating PostScript chart file.");
}


// Write out trailing information to the PostScript file.

void PsEnd()
{
  int i;

  PsStrokeForce();
  if (gi.fEps)
    fprintf(gi.file, "%%%%EOF\n");
  else {
    fprintf(gi.file, "showpage\n");
    fprintf(gi.file, "%%%%PageTrailer\n");
    fprintf(gi.file, "%%%%Trailer\n");
    fprintf(gi.file, "%%%%DocumentFonts: Times-Roman\n");
    if (gs.nFont > 0)
      for (i = 1; i < cFont; i++)
        fprintf(gi.file, "%%%%+ %s\n",
          i != fiCourier ? rgszFontName[i] : "Courier");
  }
}
#endif // PS


#ifdef META
/*
******************************************************************************
** Metafile Routines.
******************************************************************************
*/

// Output one 16 bit or 32 bit value into the metafile buffer stream.

void MetaWord(word w)
{
  char sz[cchSzDef];

  if ((pbyte)gi.pwMetaCur - gi.bm >= gi.cbMeta) {
    sprintf(sz, "Metafile would be more than %ld bytes.", gi.cbMeta);
    PrintError(sz);
    Terminate(tcFatal);
  }
  *gi.pwMetaCur = w;
  gi.pwMetaCur++;
}

void MetaLong(long l)
{
  MetaWord(WLo(l));
  MetaWord(WHi(l));
}


// Output a string into the metafile buffer stream.

void MetaSz(CONST char *sz)
{
  while (*sz) {
    MetaWord(WFromBB(sz[0], sz[1]));
    if (sz[1] == chNull)
      break;
    sz += 2;
  }
}


// Output any necessary metafile records to make the current actual settings
// of line color, fill color, etc, be those that are desired. This is
// generally called by the primitives routines before any figure record is
// actually written into a metafile. Wait until the last moment before
// changing any settings to ensure that unnecessary records aren't output,
// e.g. two select colors in a row.

void MetaSelect()
{
  if (gi.pwPoly != NULL) {        // Invalidate PolyLine cache
    gi.pwPoly = NULL;
    MetaPoint(gi.xPen, gi.yPen, rgbbmp[gi.kiPoly]);
    gi.xPen = -1;    
  }
  if (gi.kiLineDes != gi.kiLineAct) {
    MetaSelectObject(gi.kiLineDes);
    gi.kiLineAct = gi.kiLineDes;
  }
  if (gi.kiFillDes != gi.kiFillAct) {
    MetaSelectObject(16*4 + gi.kiFillDes);
    gi.kiFillAct = gi.kiFillDes;
  }
  if (gi.nFontDes != gi.nFontAct) {
    MetaSelectObject(16*5 + gi.nFontDes);
    gi.nFontAct = gi.nFontDes;
  }
  if (gi.kiTextDes != gi.kiTextAct) {
    MetaTextColor(rgbbmp[gi.kiTextDes]);
    gi.kiTextAct = gi.kiTextDes;
  }
  if (gi.nAlignDes != gi.nAlignAct) {
    MetaTextAlign(gi.nAlignDes);
    gi.nAlignAct = gi.nAlignDes;
  }
}


// Output initial metafile header information into the metafile buffer. Also
// setup and create all pen, brush, and font objects that may possibly be used
// in the generation and playing of the picture.

void MetaInit()
{
  int i, j, k;

  gi.pwMetaCur = (word *)gi.bm;
  // Placeable Metaheader
  MetaLong(0x9AC6CDD7L);
  MetaWord(0);                             // Not used
  MetaWord(0); MetaWord(0);
  MetaWord(gs.xWin); MetaWord(gs.yWin);
  MetaWord(gs.xWin/6);                     // Units per inch
  MetaLong(0L);                            // Not used
  MetaWord(0x9AC6 ^ 0xCDD7 ^ gs.xWin ^ gs.yWin ^ gs.xWin/6);  // Checksum
  // Metaheader
  MetaWord(1);                             // Metafile type
  MetaWord(9);                             // Size of header in words
  MetaWord(0x300);                         // Windows version
  MetaLong(0L);                            // Size of entire metafile in words
  MetaWord(16*5+1+(gs.nFont>0)*(cFont-1)); // Number of objects in metafile
  MetaLong(17L);                           // Size of largest record in words
  MetaWord(0);                             // Not used
  // Setup
  MetaEscape(17);
  MetaLong(LFromBB('A', 's', 't', 'r'));  // "Astr"
  MetaWord(4);                            // Creator
  MetaLong(14L);                          // Bytes in string
  MetaLong(LFromBB('A', 's', 't', 'r'));  // "Astr"
  MetaLong(LFromBB('o', 'l', 'o', 'g'));  // "olog"
  MetaLong(LFromBB(' ', szVerCore[0], '.', szVerCore[1]));  // Version
  MetaWord(WFromBB(szVerCore[2], 0));     // Version part 2
  MetaSaveDc();
  MetaWindowOrg(0, 0);
  MetaWindowExt(gs.xWin, gs.yWin);
  MetaBkMode(1 /* Transparent */);
  // Colors
  for (j = 1; j <= 4; j++)
    for (i = 0; i < 16; i++) {
      k = (j <= 1 ? gi.nPenWid * (1 + gs.fThick) : 0);
      MetaCreatePen(j <= 2 ? 0 : j-2 /* PS_SOLID; PS_DASH; PS_DOT */,
        k, rgbbmp[i]);
    }
  for (i = 0; i < 16; i++) {
    MetaCreateBrush(0 /* BS_SOLID */, rgbbmp[i]);
  }
  MetaCreateBrush(1 /* BS_NULL */, 0L);
  // Fonts
  if (gs.nFont > 0)
    for (i = 1; i < cFont; i++) {
      j = (CchSz(rgszFontName[i]) + 1) >> 1;
      MetaCreateFont(j, 0, i < fiCourier ? -METAMUL*gi.nScale : yFontT,
        i == fiWingding ? 2 /* Symbol Charset */ : 0 /* Ansi Charset */);
      j = (i == fiWingding || i >= fiCourier ?
        1 /* Draft */ : 0 /* Default */);
      k = (i == fiWingding || i == fiCourier || i == fiConsolas ?
          1 /* Fixed */ : 2 /* Variable */) |
        (i <= fiWingding ? 0x10 /* Roman */ :
          (i >= fiCourier ? 0x30 /* Modern */ : 0 /* Don't Care */));
      MetaWord(WFromBB(j, k));
      MetaSz(rgszFontName[i]);
    }
  gi.kiLineAct = gi.kiFillAct = gi.nFontAct = gi.kiTextAct = gi.nAlignAct = -1;
}


// Output trailing records to indicate the end of the metafile and then
// actually write out the entire buffer to the specifed file.

void WriteMeta(FILE *file)
{
  word *w;
#if FALSE
  int i;

  for (i = 16*5+1+(gs.nFont > 0)*4; i >= 0; i--) {
    MetaDeleteObject(i);
  }
#endif
  MetaRestoreDc();
  MetaRecord(3, 0);    // End record
  *(long *)(gi.bm + 22 + 6) =
    ((long)((pbyte)gi.pwMetaCur - gi.bm) - 22) / 2;
  for (w = (word *)gi.bm; w < gi.pwMetaCur; w++) {
    PutWord(*w);
  }
}
#endif // META


#ifdef WIRE
/*
******************************************************************************
** Daedalus Wireframe File Routines.
******************************************************************************
*/

// Write the wireframe file in memory to a previously opened file in the
// Daedalus wireframe format. This usually consists of coordinates for each
// line segment, but can also include changes to the default color.

void WriteWire(FILE *file)
{
  word *pw = (word *)gi.bm;
  int x1, y1, z1, x2, y2, z2, n;
  KV kv;

  if (file == NULL)
    return;
  fprintf(file, "DW#\n%d\n", gi.cWire);
  while (pw < gi.pwWireCur) {
    if (*pw != 32768) {

      // Output one line segment.
      x1 = (short)pw[0]; y1 = (short)pw[1]; z1 = (short)pw[2];
      x2 = (short)pw[3]; y2 = (short)pw[4]; z2 = (short)pw[5];
      fprintf(file, "%d %d %d %d %d %d\n", x1, y1, z1, x2, y2, z2);
      pw += 6;
    } else {

      // Output a color change.
      if (gs.fColor) {
        n = pw[1];
        if (n < cColor) {
          kv = rgbbmp[n];
          if (kv != rgbbmpDef[n])
            fprintf(file, "Rgb %d %d %d\n", RgbR(kv), RgbG(kv), RgbB(kv));
          else if (n != kOrange)
            fprintf(file, "%s\n", szColor[n]);
          else
            fprintf(file, "Maize\n");
        } else {
          if (gs.fInverse)
            n = 255 - n;
          fprintf(file, "GrayN %d\n", n);
        }
      }
      pw += 2;
    }
  }
}


// Add a single 16 bit number to the current wireframe file.

void WireNum(int n)
{
  char sz[cchSzDef];

  if ((pbyte)gi.pwWireCur - gi.bm >= gi.cbWire) {
    sprintf(sz, "Wireframe would be more than %ld bytes.", gi.cbWire);
    PrintError(sz);
    Terminate(tcFatal);
  }
  *gi.pwWireCur = (word)n;
  gi.pwWireCur++;
}


// Add a solid line to current wireframe file, specified by its endpoints.

void WireLine(int x1, int y1, int z1, int x2, int y2, int z2)
{
  if (!FBetween(x1, -32767, 32767) || !FBetween(x2, -32767, 32767))
    return;
  if (!FBetween(y1, -32767, 32767) || !FBetween(y2, -32767, 32767))
    return;
  if (!FBetween(z1, -32767, 32767) || !FBetween(z2, -32767, 32767))
    return;

  if (gi.kiInFile != gi.kiCur) {
    gi.kiInFile = gi.kiCur;
    WireNum(32768);
    WireNum(gi.kiCur);
  }
  WireNum(x1); WireNum(y1); WireNum(z1);
  WireNum(x2); WireNum(y2); WireNum(z2);
  gi.cWire++;
}


// Add a small but still visible dot to the current wireframe file.

void WireSpot(int x, int y, int z)
{
  WireLine(x-1, y, z, x+1, y, z);
  WireLine(x, y-1, z, x, y+1, z);
  WireLine(x, y, z-1, x, y, z+1);
}


// Add an octahedron of a given radius to the current wireframe file. These
// shapes are used to mark the exact locations of planets in the scene.

void WireOctahedron(int x, int y, int z, int r)
{
  int rgx[4], rgy[4], i;

  rgx[0] = rgx[3] = x-r; rgx[1] = rgx[2] = x+r;
  rgy[0] = rgy[1] = y-r; rgy[2] = rgy[3] = y+r;
  for (i = 0; i < 4; i++) {
    WireLine(rgx[i], rgy[i], z, x, y, z-r);
    WireLine(rgx[i], rgy[i], z, x, y, z+r);
    WireLine(rgx[i], rgy[i], z, rgx[i+1 & 3], rgy[i+1 & 3], z);
  }
}


#ifdef SWISS
// Add a fixed star to the current wireframe file.

void WireStar(int x, int y, int z, ES *pes)
{
  char *pch = pes->sz;
  int n;

  // Determine star color.
  if (gs.fColor) {
    if (pes->ki != kDefault)
      n = pes->ki;
    else {
      n = 255 - (int)((pes->mag - rStarLite) / rStarSpan * 224.0);
      n = Min(n, 255); n = Max(n, 32);
    }
    DrawColor(n);
  }

  // Draw star point.
  if (!FOdd(gs.nAllStar))
    WireSpot(x, y, z);
  else
    WireOctahedron(x, y, z, 3 * gi.nScaleT);

  // Draw star's name label.
  if (!gs.fLabel || gs.nAllStar < 2)
    return;
  gi.zDefault = z + 8*gi.nScaleT;
  DrawSz(pes->pchBest, x, y, dtCent);
}
#endif


// Given longitude and latitude values on a globe, return the 3D pixel
// coordinates corresponding to them. In other words, project the globe in
// the 3D environment, and return where our coordinates got projected to.
// Like FGlobeCalc() except for 3D wireframe format.

void WireGlobeCalc(real x1, real y1, int *u, int *v, int *w, int rz, real deg)
{
  // Compute coordinates for a general globe invoked with -XG switch.

  if (gi.nMode == gSphere) {
    // Chart sphere coordinates are relative to the local horizon.
    x1 = Mod(rDegMax - (x1 + is.lonMC) + rDegQuad);
    y1 = rDegQuad - y1;
    EquToLocal(&x1, &y1, rDegQuad - Lat);
    y1 = rDegQuad - y1;
  }

  x1 = Mod(x1+deg);       // Shift by current globe rotation value.
  if (gs.rTilt != 0.0) {
    // Do another coordinate shift if the globe's equator is tilted any.
    y1 = rDegQuad - y1;
    CoorXform(&x1, &y1, gs.rTilt);
    x1 = Mod(x1); y1 = rDegQuad - y1;
  }
  *u = (int)((real)rz*RSinD(y1)*RSinD(x1)-rRound);
  *v = (int)((real)rz*RSinD(y1)*RCosD(x1)-rRound);
  *w = (int)((real)rz*RCosD(y1)-rRound);
}


// Given longitude and latitude values, return the 3D pixel coordinates
// corresponding to them. Like FMapCalc() except for 3D wireframe format.

void WireMapCalc(real x1, real y1, int *xp, int *yp, int *zp, flag fSky,
  real rT, int rz, real deg)
{
  if (!fSky)
    x1 = is.lonMC - x1;
  if (x1 < 0.0)
    x1 += rDegMax;
  if (x1 > rDegHalf)
    x1 -= rDegMax;
  x1 = Mod(rDegHalf - rT - x1);
  y1 = rDegQuad - y1;
  WireGlobeCalc(x1, y1, xp, yp, zp, rz, deg);
}


// Draw a globe, for either the world or the constellations. Shift the chart
// by specified rotational and tilt values, and may plot on the chart each
// planet at its zenith position on Earth or location in constellations. Like
// DrawMap() except for 3D wireframe format.

void WireDrawGlobe(flag fSky, real deg)
{
  int X[objMax], Y[objMax], Z[objMax], M[objMax], N[objMax], O[objMax],
    rz, unit = 12*gi.nScale,
    x, y, z, xold, yold, zold, m, n, o, u, v, w, i, j, k, l;
  real planet1[objMax], planet2[objMax], x1, y1, rT;
#ifdef SWISS
  ES es;
#endif

  // Set up some variables.
  rz = Min(gs.xWin/2, gs.yWin/2);
  if (gi.nMode == gSphere)
    rz -= 7*gi.nScale;

  // Draw the map (either a constellation map, or a world map).

  rT = fSky ? rDegMax - deg : deg;
#ifdef CONSTEL
  if (fSky)
    EnumConstelLines(NULL, NULL, NULL, NULL, NULL);
  else
#endif
    EnumWorldLines(NULL, NULL, NULL, NULL, NULL);
  while (
#ifdef CONSTEL
    fSky ? EnumConstelLines(&xold, &yold, &x, &y, &i) :
#endif
    EnumWorldLines(&xold, &yold, &x, &y, &i)) {
    if (fSky) {
      if (i > 0)
        DrawColor(gi.nMode == gSphere || !gs.fAlt ? gi.kiGray : kDkGreenB);
      else
        DrawColor(gi.nMode == gSphere ? kPurpleB :
          (gs.fAlt ? kBlueB : kDkBlueB));
    } else {
      if (i >= 0)
        DrawColor(!gs.fAlt && !gs.fColorHouse ? gi.kiGray :
          (i ? kRainbowB[i] : kDkBlueB));
    }
    // For globes, have to do a complicated transformation.
    WireGlobeCalc((real)xold, (real)yold, &m, &n, &o, rz, rT);
    if (fSky && i > 0) {
      gi.zDefault = o;
      DrawSz(szCnstlAbbrev[i], m, n, dtCent | dtScale2);
      continue;
    }
    WireGlobeCalc((real)x, (real)y, &u, &v, &w, rz, rT);
    WireLine(m, n, o, u, v, w);
  }

  // Now, if in an appropriate bonus chart mode, draw each planet at its
  // zenith or visible location on the globe, if not hidden.

  if (gs.fAlt || gi.nMode == gAstroGraph || gi.nMode == gSphere)
    return;
  rT = gs.fConstel ? rDegHalf : Lon;
  if (rT < 0.0)
    rT += rDegMax;
  j = Max(is.nObj, oMC);
  for (i = 0; i <= j; i++) {
    planet1[i] = Tropical(planet[i]);
    planet2[i] = planetalt[i];
    EclToEqu(&planet1[i], &planet2[i]);    // Calculate zenith long. & lat.
  }

  // Compute screen coordinates of each object, if it's even visible.

  for (i = 0; i <= is.nObj; i++) if (FProper(i)) {
    WireMapCalc(planet1[i], planet2[i], &u, &v, &w, fSky, rT, rz, deg);
    X[i] = u;
    Y[i] = v;
    Z[i] = w;
    M[i] = X[i]; N[i] = Y[i]; O[i] = Z[i]-unit/2;
  }

  // Draw ecliptic equator and zodiac sign wedges.

  if (gs.fHouseExtra) {
    if (!gs.fColorSign)
      DrawColor(kDkGreenB);
    for (l = -2; l < cSign; l++) {
      if (gs.fColorSign && l >= 0)
        DrawColor(kSignB(l+1));
      for (i = -90; i <= 90; i++) {
        if (gs.fColorSign && l < 0 && i % 30 == 0)
          DrawColor(kSignB((i+90)/30 + (l < -1)*6 + 1));
        if (l >= 0) {
          // Coordinates for zodiac sign wedge longitude lines.
          j = l*30; k = i;
        } else {
          // Coordinates for ecliptic equator latitude line.
          j = i+90 + (l < -1)*180; k = 0;
        }
        x1 = Tropical((real)j);
        y1 = (real)k;
        EclToEqu(&x1, &y1);
        WireMapCalc(x1, y1, &u, &v, &w, fSky, rT, rz, deg);
        WireSpot(u, v, w);
      }
    }
  }

  // Draw Earth's equator.

  if (gs.fEquator) {
    DrawColor(kPurpleB);
    for (i = 0; i < nDegMax; i++) {
      x1 = (real)i; y1 = 90.0;
      WireGlobeCalc(x1, y1, &j, &k, &l, rz, deg);
      WireSpot(j, k, l);
    }
  }

  // Draw chart latitude and plot chart location.

  if (us.fLatitudeCross && !fSky) {
    DrawColor(kMagentaB);
    for (i = 0; i < nDegMax; i++) {
      x1 = (real)i; y1 = rDegQuad - Lat;
      WireGlobeCalc(x1, y1, &j, &k, &l, rz, deg);
      WireSpot(j, k, l);
    }
    x1 = Mod(rDegHalf - Lon); y1 = rDegQuad - Lat;
    WireGlobeCalc(x1, y1, &j, &k, &l, rz, deg);
    WireOctahedron(j, k, l, gi.nScale);
  }

#ifdef SWISS
  // Draw extra stars.

  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      x1 = es.lon; y1 = es.lat;
      x1 = Tropical(x1);
      EclToEqu(&x1, &y1);
      WireMapCalc(x1, y1, &j, &k, &l, fSky, rT, rz, deg);
      WireStar(j, k, l, &es);
    }
  }

  // Draw extra asteroids.

  if (gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fFalse);
    while (SwissComputeAsteroid(is.T, &es, fFalse)) {
      x1 = es.lon; y1 = es.lat;
      x1 = Tropical(x1);
      EclToEqu(&x1, &y1);
      WireMapCalc(x1, y1, &j, &k, &l, fSky, rT, rz, deg);
      WireStar(j, k, l, &es);
    }
  }
#endif

#ifdef ATLAS
  // Draw locations of cities from atlas.

  if (gs.fLabelCity && !fSky && FEnsureAtlas()) {
    if (!gs.fLabelAsp)
      DrawColor(kOrangeB);
    KiCity(-1);
    for (i = 0; i < is.cae; i++) {
      x1 = nDegHalf - is.rgae[i].lon;
      y1 = rDegQuad - is.rgae[i].lat;
      WireGlobeCalc(x1, y1, &u, &v, &w, rz, deg);
      if (gs.fLabelAsp)
        DrawColor(KiCity(i));
      WireSpot(u, v, w);
    }
  }
#endif

  // Draw MC, IC, Asc, and Des lines for each object, as great circles around
  // the globe. The result is a 3D astro-graph chart.

  if (!fSky) for (i = 0; i <= is.nObj; i++) if (FProper(i)) {
    for (k = 0; k < 4; k++) {
      if (ignorez[!k ? arDes : (k == 1 ? arIC : (k == 2 ? arAsc : arMC))])
        continue;
      DrawColor(kObjB[!k ? oDes : (k == 1 ? oNad : (k == 2 ? oAsc : oMC))]);
      for (j = 0; j <= nDegHalf; j++) {
        if (FOdd(k)) {
          x1 = planet1[i] + (k == 3 ? 0.0 : rDegHalf);
          y1 = (real)(j - 90);
        } else {
          l = j + (k == 0)*nDegHalf + 90;
          if (l >= nDegMax)
            l -= nDegMax;
          x1 = (real)l; y1 = 0.0;
          CoorXform(&x1, &y1, rDegQuad - planet2[i]);
          x1 += planet1[i] + rDegQuad;
        }
        WireMapCalc(x1, y1, &x, &y, &z, fSky, rT, rz, deg);
        if (j > 0)
          WireLine(xold, yold, zold, x, y, z);
        xold = x; yold = y; zold = z;
      }
    }
    // Draw astro-graph lines for minor house cusps, if unrestricted.
    for (k = 1; k <= cSign; k++) {
      if (FIgnore(cuspLo-1 + k) ||
        (k == sAri && chouse[k] == is.Asc) ||
        (k == sCap && chouse[k] == is.MC)  ||
        (k == sLib && chouse[k] == Mod(is.Asc + rDegHalf)) ||
        (k == sCan && chouse[k] == Mod(is.MC  + rDegHalf)))
        continue;
      DrawColor(kObjB[cuspLo-1 + k]);
      for (j = 0; j <= nDegHalf; j++) {
        x1 = 0.0; y1 = (real)j;
        CoorXform(&x1, &y1, rDegQuad - chouse3[k]);
        x1 = Mod(x1 + rDegQuad);
        CoorXform(&x1, &y1, rDegQuad + planet2[i]);
        x1 = Mod(x1 - rDegQuad + planet1[i]);
        WireMapCalc(x1, y1, &x, &y, &z, fSky, rT, rz, deg);
        if (j > 0)
          WireLine(xold, yold, zold, x, y, z);
        xold = x; yold = y; zold = z;
      }
    }
  }

  // Now that we have the coordinates of each object, draw their glyphs.

  for (i = 0; i <= is.nObj; i++) if (FProper(i)) {
    gi.zDefault = O[i];
    DrawObject(i, M[i], N[i]);
    DrawColor(kObjB[i]);
    WireOctahedron(X[i], Y[i], Z[i], gi.nScale);
  }
}


// Generate a chart depicting an aerial view of the solar system in space,
// with all the planets drawn around the Sun, and the specified central body
// in the middle. Like XChartOrbit() except for 3D wireframe format.

void WireChartOrbit()
{
  int x[objMax], y[objMax], z[objMax], zWin, xd, yd, i, j, k;
  real sx, sz, xp, yp, zp, xp2, yp2, zp2;
#ifdef SWISS
  ES es, *pes1, *pes2;
  int xT, yT, zT, x2, y2, z2;
#endif

  // Compute coordinates of planets.
  i = gi.nScale/gi.nScaleT;
  sz = gs.rspace > 0.0 ? gs.rspace : (i <= 1 ? 90.0 : (i == 2 ? 30.0 :
    (i == 3 ? 6.0 : (gi.nScaleText/2 <= 1 ? 1.0 : 0.006))));
  zWin = Min(gs.xWin, gs.yWin);
  sx = (real)zWin/sz;
  for (i = 0; i <= is.nObj; i++) if (FProper(i)) {
    xp = space[i].x; yp = space[i].y; zp = space[i].z;
    if (us.nStar > 0) {
      xp /= rLYToAU; yp /= rLYToAU; zp /= rLYToAU;
    }
    if (us.fHouse3D)
      OrbitPlot(&xp, &yp, &zp, sz, i, space);
    x[i] = -(int)(xp*sx); y[i] = (int)(yp*sx); z[i] = (int)(zp*sx);
  }

  // Draw zodiac lines.
  if (!gs.fHouseExtra) {
    k = zWin;
    if (!gs.fColorSign)
      DrawColor(kLtGrayB);
    for (i = 0; i < cSign; i++) {
      j = i+1;
      if (gs.fColorSign)
        DrawColor(kSignB(j));
      xd = NCosD(k, nDegHalf-i*30); yd = NSinD(k, nDegHalf-i*30);
      WireLine(0, 0, 0, xd, yd, 0);
      xd = NCosD(k, nDegHalf-i*30-15); yd = NSinD(k, nDegHalf-i*30-15);
      DrawSign(j, xd, yd);
    }
  }

  // Draw lines connecting planets which have aspects between them.
  if (!gs.fEquator && us.nAsp > 0) {
    if (!FCreateGrid(fFalse))
      return;
    for (j = oNorm; j >= 1; j--)
      for (i = j-1; i >= 0; i--)
        if (grid->n[i][j] && FProper(i) && FProper(j)) {
          DrawColor(kAspB[grid->n[i][j]]);
          WireLine(x[i], y[i], z[i], x[j], y[j], z[j]);
        }
  }

  // Prepare to draw orbital trails.
  if (gs.cspace > 0 && gi.rgspace == NULL) {
    gi.rgspace = RgAllocate(oNorm1*gs.cspace, PT3R, "orbits");
    if (gi.rgspace == NULL)
      return;
  }

  // Draw planets.
  for (i = 0; i <= is.nObj; i++) if (FProper(i)) {
    DrawColor(kObjB[i]);
    if (!gs.fAlt || i > oVes)
      j = 0;
    else
      j = (int)(RLog10(RObjDiam(i)) * (real)gi.nScaleT);
    j = Max(j, 3 * gi.nScaleT);
    WireOctahedron(x[i], y[i], z[i], j);
    gi.zDefault = z[i] + (j + 5*gi.nScaleT);
    DrawObject(i, x[i], y[i]);

    // Draw orbital trails.
    for (j = 0; j < gi.cspace-1; j++) {
      k = (gi.ispace - gi.cspace + j + gs.cspace) % gs.cspace;
      k = k*oNorm1 + i;
      xp = gi.rgspace[k].x; yp = gi.rgspace[k].y; zp = gi.rgspace[k].z;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, &zp, sz, i, &gi.rgspace[k - i]);

      k = (gi.ispace - gi.cspace + j + 1 + gs.cspace) % gs.cspace;
      k = k*oNorm1 + i;
      xp2 = gi.rgspace[k].x; yp2 = gi.rgspace[k].y; zp2 = gi.rgspace[k].z;
      if (us.fHouse3D)
        OrbitPlot(&xp2, &yp2, &zp2, sz, i, &gi.rgspace[k - i]);

      k = gi.cspace - j;
      WireLine(-(int)(xp*sx), (int)(yp*sx), (int)(zp*sx) - k*gs.zspace,
        -(int)(xp2*sx), (int)(yp2*sx), (int)(zp2*sx) - (k-1)*gs.zspace);
    }
    if (gs.cspace < 0 && i <= oNep) {
      j = !FGeo(i) ? oSun : oEar;
      if (i == j || FIgnore(j))
        continue;
      k = (int)RLength2((real)(x[i] - x[j]), (real)(y[i] - y[j]));
      DrawColor(kObjB[i]);
      gi.zDefault = z[i];
      DrawCircle(x[j], y[j], k, k);
    }
  }
  OrbitRecord();

#ifdef SWISS
  // Draw extra stars.
  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      xp = es.space.x / rLYToAU; yp = es.space.y / rLYToAU;
      zp = es.space.z / rLYToAU;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, &zp, sz, -1, NULL);
      xT = -(int)(xp*sx); yT = (int)(yp*sx); zT = (int)(zp*sx);
      WireStar(xT, yT, zT, &es);
    }

    // Draw constellation lines between stars.
    DrawColor(gi.kiLite);
    EnumStarsLines(fTrue, NULL, NULL);
    while (EnumStarsLines(fFalse, &pes1, &pes2)) {
      xp = pes1->space.x / rLYToAU; yp = pes1->space.y / rLYToAU;
      zp = pes1->space.z / rLYToAU;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, &zp, sz, -1, NULL);
      xT = -(int)(xp*sx); yT = (int)(yp*sx); zT = (int)(zp*sx);
      xp = pes2->space.x / rLYToAU; yp = pes2->space.y / rLYToAU;
      zp = pes2->space.z / rLYToAU;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, &zp, sz, -1, NULL);
      x2 = -(int)(xp*sx); y2 = (int)(yp*sx); z2 = (int)(zp*sx);
      WireLine(xT, yT, zT, x2, y2, z2);
    }
  }

  // Draw extra asteroids.
  if (!gs.fAllStar && gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fFalse);
    while (SwissComputeAsteroid(is.T, &es, fFalse)) {
      xp = es.space.x; yp = es.space.y; zp = es.space.z;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, &zp, sz, -1, NULL);
      xT = -(int)(xp*sx); yT = (int)(yp*sx); zT = (int)(zp*sx);
      WireStar(xT, yT, zT, &es);
    }
  }
#endif
}


// Translate to chart 3D coordinates, that indicate how to compose a 3D chart
// sphere, for the -XX wireframe chart. Inputs may be local horizon altitude
// and azimuth coordinates, local horizon prime vertical, local horizon
// meridian, zodiac position and latitude, or Earth coordinates.

void WireSphereLocal(real azi, real alt, int zr, int *xp, int *yp, int *zp)
{
  if (gs.fEcliptic) {
    azi = Mod(azi - rDegQuad); neg(alt);
    CoorXform(&azi, &alt, is.latMC - rDegQuad);
    azi = Mod(is.lonMC - azi + rDegQuad);
    EquToEcl(&azi, &alt);
    azi = rDegMax - Untropical(azi); neg(alt);
  }
  azi = Mod(rDegQuad*3 - (azi + gs.rRot));
  if (gs.rTilt != 0.0)
    CoorXform(&azi, &alt, gs.rTilt);
  *xp = (int)((real)zr * RCosD(alt) * RSinD(azi) - rRound);
  *yp = (int)((real)zr * RCosD(alt) * RCosD(azi) - rRound);
  *zp = -(int)((real)zr * RSinD(alt) - rRound);
}

void WireSpherePrime(real azi, real alt, int zr, int *xp, int *yp, int *zp)
{
  CoorXform(&azi, &alt, rDegQuad);
  WireSphereLocal(azi + rDegQuad, alt, zr, xp, yp, zp);
}

void WireSphereMeridian(real azi, real alt, int zr, int *xp, int *yp, int *zp)
{
  azi = Mod(azi + rDegQuad);
  CoorXform(&azi, &alt, rDegQuad);
  WireSphereLocal(azi, alt, zr, xp, yp, zp);
}

void WireSphereZodiac(real lon, real lat, int zr, int *xp, int *yp, int *zp)
{
  real lonT, latT;

  lonT = Tropical(lon); latT = lat;
  EclToEqu(&lonT, &latT);
  lonT = Mod(is.lonMC - lonT + rDegQuad);
  EquToLocal(&lonT, &latT, rDegQuad - is.latMC);
  WireSphereLocal(lonT + rDegQuad, -latT, zr, xp, yp, zp);
}

void WireSphereEarth(real azi, real alt, int zr, int *xp, int *yp, int *zp)
{
  azi = Mod(-azi);
  CoorXform(&azi, &alt, rDegQuad - Lat);
  WireSphereLocal(azi + rDegQuad, -alt, zr, xp, yp, zp);
}


// Draw a chart sphere (like a chart wheel but in 3D) as done with the -XX
// switch. Like XChartSphere() except for 3D wireframe format.

void WireChartSphere()
{
  char sz[cchSzDef];
  int rgx[objMax], rgy[objMax], rgz[objMax], zGlyph, zGlyph2,
    cChart, iChart, zr, xo = 0, yo = 0, zo = 0, xp, yp, zp, i, j, k, k2, nSav;
  flag fHouse3D = !us.fHouse3D, fNoHorizon;
  real rT;
  CONST CP *pcp;
  CP cpSav;
#ifdef SWISS
  ES es, *pes1, *pes2;
#endif

  // Initialize variables.
  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;

  fNoHorizon = ignorez[0] && ignorez[1] && ignorez[2] && ignorez[3];
  zGlyph = 7*gi.nScale; zGlyph2 = 14*gi.nScale;
  zr = Min(gs.xWin >> 1, gs.yWin >> 1) - zGlyph;
  cChart = 1 +
    (us.nRel <= rcDual) + (us.nRel <= rcTriWheel) + (us.nRel <= rcQuadWheel);
  is.latMC = Lat;

  // Draw constellations.
  if (gs.fConstel) {
    neg(gs.rTilt);
    WireDrawGlobe(fTrue, rDegMax - gs.rRot);
    neg(gs.rTilt);
  }

  // Draw horizon.
  if (!fNoHorizon || (!gs.fHouseExtra && fHouse3D)) {
    if (!gs.fColorHouse)
      DrawColor(gi.kiOn);
    for (i = 0; i <= nDegMax; i++) {
      if (gs.fColorHouse && (i == 0 || i == nDegHalf))
        DrawColor(kSignB(i ? sLib : sAri));
      WireSphereLocal((real)i, 0.0, zr, &xp, &yp, &zp);
      if (i > 0) {
        WireLine(xo, yo, zo, xp, yp, zp);
        k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
        for (j = -k; j <= k; j += (k << 1)) {
          WireSphereLocal((real)i, (real)j / 2.0, zr, &xo, &yo, &zo);
          WireLine(xo, yo, zo, xp, yp, zp);
        }
      }
      xo = xp; yo = yp; zo = zp;
    }
  }

  // Draw Earth's equator.
  if (gs.fEquator) {
    DrawColor(kPurpleB);
    for (i = 0; i <= nDegMax; i++) {
      WireSphereEarth((real)i, 0.0, zr, &xp, &yp, &zp);
      if (i > 0)
        WireLine(xo, yo, zo, xp, yp, zp);
      xo = xp; yo = yp; zo = zp;
    }
  }

  // Draw prime vertical.
  if (!fNoHorizon) {
    if (!gs.fColorHouse)
      DrawColor(gi.kiGray);
    for (i = 0; i <= nDegMax; i++) {
      if (gs.fColorHouse)
        DrawColor(kSignB((i-1)/30 + 1));
      WireSpherePrime((real)i, 0.0, zr, &xp, &yp, &zp);
      if (i > 0) {
        WireLine(xo, yo, zo, xp, yp, zp);
        k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
        for (j = -k; j <= k; j += (k << 1)) {
          WireSpherePrime((real)i, (real)j / 2.0, zr, &xo, &yo, &zo);
          WireLine(xo, yo, zo, xp, yp, zp);
        }
      }
      xo = xp; yo = yp; zo = zp;
    }
  }

  // Draw 3D house wedges and meridian.
  if (!gs.fColorHouse)
    DrawColor(kDkGreenB);
  for (j = -1; j <= cSign; j++) {
    if (!(!gs.fHouseExtra && fHouse3D) && !(j <= 0 && !fNoHorizon))
      continue;
    if (fHouse3D &&
       ((j == sCap && chouse[j] == is.MC)  ||
        (j == sCan && chouse[j] == Mod(is.MC  + rDegHalf))))
      continue;
    if (fHouse3D && us.nHouse3D == hmPrime &&
       ((j == sAri && chouse[j] == is.Asc) ||
        (j == sLib && chouse[j] == Mod(is.Asc + rDegHalf))))
      continue;
    if (fHouse3D && us.nHouse3D == hmHorizon &&
       ((j == sAri && chouse[j] == Mod(is.Vtx + rDegHalf)) ||
        (j == sLib && chouse[j] == is.Vtx)))
      continue;
    if (gs.fColorHouse) {
      k = j > 0 ? j : (j < 0 ? sCan : sCap);
      DrawColor(kSignB(k));
    }
    rT = j > 0 ? chouse3[j] : (j < 0 ? rDegQuad : 270.0);
    for (i = -90; i <= 90; i++) {
      if (j <= 0 || us.nHouse3D == hmPrime)
        WireSpherePrime(rT, (real)i, zr, &xp, &yp, &zp);
      else if (us.nHouse3D == hmHorizon)
        WireSphereLocal(rDegQuad - rT, (real)i, zr, &xp, &yp, &zp);
      else
        WireSphereEarth(rT, (real)i, zr, &xp, &yp, &zp);
      if (i > -90) {
        WireLine(xo, yo, zo, xp, yp, zp);
        if (j <= 0) {
          k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
          for (k2 = -k; k2 <= k; k2 += (k << 1)) {
            WireSphereMeridian((real)(j == 0 ? i+180 : 360-i), (real)k2 / 2.0,
              zr, &xo, &yo, &zo);
            WireLine(xo, yo, zo, xp, yp, zp);
          }
        }
      }
      xo = xp; yo = yp; zo = zp;
    }
  }

  // Draw 2D house wedges.
  if (!gs.fHouseExtra && !fHouse3D)
    for (i = 1; i <= cSign; i++) {
      if (gs.fColorHouse)
        DrawColor(kSignB(i));
      for (j = -90; j <= 90; j++) {
        WireSphereZodiac(chouse[i], (real)j, zr, &xp, &yp, &zp);
        if (j > -90)
          WireLine(xo, yo, zo, xp, yp, zp);
        xo = xp; yo = yp; zo = zp;
      }
    }

  // Draw sign wedges.
  if (!us.fVedic) {
    if (!gs.fColorSign)
      DrawColor(kDkBlueB);
    for (i = 0; i <= nDegMax; i++) {
      if (gs.fColorSign)
        DrawColor(kSignB((i-1)/30 + 1));
      WireSphereZodiac((real)i, 0.0, zr, &xp, &yp, &zp);
      if (i > 0) {
        WireLine(xo, yo, zo, xp, yp, zp);
        if (i % 30 != 0) {
          k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
          for (j = -k; j <= k; j += (k << 1)) {
            WireSphereZodiac((real)i, (real)j / 2.0, zr, &xo, &yo, &zo);
            WireLine(xo, yo, zo, xp, yp, zp);
          }
        }
      }
      xo = xp; yo = yp; zo = zp;
    }
    for (i = 0; i < nDegMax; i += 30) {
      if (gs.fColorSign)
        DrawColor(kSignB(i/30 + 1));
      for (j = -90; j <= 90; j++) {
        WireSphereZodiac((real)i, (real)j, zr, &xp, &yp, &zp);
        if (j > -90)
          WireLine(xo, yo, zo, xp, yp, zp);
        xo = xp; yo = yp; zo = zp;
      }
    }
  }

  // Label signs.
  if (!us.fVedic) {
    nSav = gi.nScale;
    gi.nScale = gi.nScaleTextT;
    for (j = -80; j <= 80; j += 160)
      for (i = 1; i <= cSign; i++) {
        WireSphereZodiac((real)(i*30-15), (real)j, zr, &xp, &yp, &zp);
        DrawColor(gs.fColorSign ? kSignB(i) : kDkBlueB);
        gi.zDefault = zp;
        DrawSign(i, xp, yp);
      }
    gi.nScale = nSav;
  }

  // Label houses.
  if (!gs.fHouseExtra) {
    nSav = gi.nScale;
    gi.nScale = gi.nScaleTextT;
    for (j = -82; j <= 82; j += 164)
      for (i = 1; i <= cSign; i++) {
        WireSpherePrime((real)(i*30-15), (real)j, zr, &xp, &yp, &zp);
        DrawColor(gs.fColorHouse ? kSignB(i) : kDkGreenB);
        gi.zDefault = zp;
        DrawHouse(i, xp, yp);
      }
    gi.nScale = nSav;
  }

  // Label directions.
  if (!fNoHorizon) {
    k = zGlyph >> 1;
    for (i = 0; i < nDegMax; i += 90) {
      j = i / 90;
      WireSphereLocal((real)i, 0.0, zr, &xp, &yp, &zp);
      DrawColor(kObjB[oAsc + ((j + 3) & 3)*3]);
      if (!ignorez[(1 - j) & 3])
        WireLine(0, 0, 0, xp, yp, zp);
      if (gs.fColorHouse)
        DrawColor(gi.kiOn);
      WireSphereLocal((real)i, 0.0, zr + k, &xp, &yp, &zp);
      sprintf(sz, "%c", szDir[j][0]);
      gi.zDefault = zp + gi.nScale;
      DrawSz(sz, xp, yp, dtCent);
    }
    for (j = -90; j <= 90; j += nDegHalf) {
      WireSphereLocal(0.0, (real)j, zr + k, &xp, &yp, &zp);
      DrawColor(gs.fColorHouse ? gi.kiOn : (kObjB[j <= 0 ? oMC : oNad]));
      sprintf(sz, "%c", j <= 0 ? 'Z' : 'N');
      gi.zDefault = zp + gi.nScale;
      DrawSz(sz, xp, yp, dtCent);
    }
  }

#ifdef SWISS
  // Draw extra stars.
  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      WireSphereZodiac(es.lon, es.lat, zr, &xp, &yp, &zp);
      WireStar(xp, yp, zp, &es);
    }

    // Draw constellation lines between stars.
    DrawColor(gi.kiLite);
    EnumStarsLines(fTrue, NULL, NULL);
    while (EnumStarsLines(fFalse, &pes1, &pes2)) {
      WireSphereZodiac(pes1->lon, pes1->lat, zr, &xo, &yo, &zo);
      WireSphereZodiac(pes2->lon, pes2->lat, zr, &xp, &yp, &zp);
      WireLine(xo, yo, zo, xp, yp, zp);
    }
  }

  // Draw extra asteroids.
  if (gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fFalse);
    while (SwissComputeAsteroid(is.T, &es, fFalse)) {
      WireSphereZodiac(es.lon, es.lat, zr, &xp, &yp, &zp);
      WireStar(xp, yp, zp, &es);
    }
  }
#endif

  // Determine set of planet data to use.
  for (iChart = 1; iChart <= cChart; iChart++) {
    FProcessCommandLine(szWheelX[iChart]);
    if (iChart <= 1)
      pcp = rgpcp[us.nRel <= rcDual];
    else
      pcp = rgpcp[iChart];
    cpSav = cp0;
    cp0 = *pcp;

  // Draw planet glyphs, and spots for actual local location.
  for (i = 0; i <= is.nObj; i++) if (FProper(i)) {
    WireSphereZodiac(planet[i], planetalt[i], zr, &xp, &yp, &zp);
    rgx[i] = xp; rgy[i] = yp; rgz[i] = zp;
    gi.zDefault = rgz[i] - zGlyph2;
    DrawObject(i, rgx[i], rgy[i]);
    DrawColor(kObjB[i]);
    WireOctahedron(rgx[i], rgy[i], rgz[i], gi.nScale);
  }

  // Draw lines connecting planets which have aspects between them.
  if (!FCreateGrid(fFalse))
    return;
  for (j = is.nObj; j >= 1; j--)
    for (i = j-1; i >= 0; i--)
      if (grid->n[i][j] && FProper(i) && FProper(j)) {
        DrawColor(kAspB[grid->n[i][j]]);
        WireLine(rgx[i], rgy[i], rgz[i], rgx[j], rgy[j], rgz[j]);
      }

    cp0 = cpSav;
  } // iChart
  FProcessCommandLine(szWheelX[0]);

  // Draw center point.
  DrawColor(gi.kiOn);
  WireOctahedron(0, 0, 0, gi.nScale * 2);
}
#endif // WIRE
#endif // GRAPH

/* xdevice.cpp */
