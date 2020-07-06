/*
** Astrolog (Version 7.00) File: xdevice.cpp
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


#ifdef GRAPH
/*
******************************************************************************
** Bitmap File Routines.
******************************************************************************
*/

/* Write the bitmap array to a previously opened file in a format that   */
/* can be read in by the Unix X commands bitmap and xsetroot. The 'mode' */
/* parameter defines how much white space is put in the file.            */

void WriteXBitmap(FILE *file, CONST char *name, char mode)
{
  int x, y, i, temp = 0;
  uint value;
  char szT[cchSzDef], *pchStart, *pchEnd;

  /* Determine variable name from filename. */
  sprintf(szT, "%s", name);
  for (pchEnd = szT; *pchEnd != chNull; pchEnd++)
    ;
  for (pchStart = pchEnd; pchStart > szT &&
    *(pchStart-1) != '/' && *(pchStart-1) != '\\'; pchStart--)
    ;
  for (pchEnd = pchStart; *pchEnd != chNull && *pchEnd != '.'; pchEnd++)
    ;
  *pchEnd = chNull;

  /* Output file header. */
  fprintf(file, "#define %s_width %d\n" , pchStart, gs.xWin);
  fprintf(file, "#define %s_height %d\n", pchStart, gs.yWin);
  fprintf(file, "static %s %s_bits[] = {",
    mode != 'V' ? "char" : "short", pchStart);
  for (y = 0; y < gs.yWin; y++) {
    x = 0;
    do {

      /* Process each row, eight columns at a time. */
      if (y + x > 0)
        fprintf(file, ",");
      if (temp == 0)
        fprintf(file, "\n%s",
          mode == 'N' ? "  " : (mode == 'C' ? " " : ""));
      value = 0;
      for (i = (mode != 'V' ? 7 : 15); i >= 0; i--)
        value = (value << 1) + (!(FBmGet(gi.bm, x+i, y)^
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

      /* Is it time to skip to the next line while writing the file yet? */
      if ((mode == 'N' && temp >= 12) ||
          (mode == 'C' && temp >= 15) ||
          (mode == 'V' && temp >= 11))
        temp = 0;
      x += (mode != 'V' ? 8 : 16);
    } while (x < gs.xWin);
  }
  fprintf(file, "};\n");
}


/* Write the bitmap array to a previously opened file in a simple boolean    */
/* Ascii rectangle, one char per pixel, where '#' represents an off bit and  */
/* '-' an on bit. The output format is identical to the format generated by  */
/* the Unix bmtoa command, and it can be converted into a bitmap with atobm. */

void WriteAscii(FILE *file)
{
  int x, y, i;

  for (y = 0; y < gs.yWin; y++) {
    for (x = 0; x < gs.xWin; x++) {
      i = FBmGet(gi.bm, x, y);
      if (gs.fColor)
        putc(ChHex(i), file);
      else
        putc(i ? '-' : '#', file);
    }
    putc('\n', file);
  }
}


/* Write the bitmap array to a previously opened file in the bitmap format  */
/* used in Microsoft Windows for its .bmp extension files. This is a pretty */
/* efficient format, only requiring a small header, and one bit per pixel   */
/* for monochrome graphics, or four bits per pixel for full color.          */

void WriteBmp(FILE *file)
{
  int x, y;
  dword value;

  /* BitmapFileHeader */
  PutByte('B'); PutByte('M');
  PutLong(14+40 + (gs.fColor ? 64 : 8) +
    (long)4*gs.yWin*(((gs.xWin-1) >> (gs.fColor ? 3 : 5))+1));
  PutWord(0); PutWord(0);
  PutLong(14+40 + (gs.fColor ? 64 : 8));
  /* BitmapInfo / BitmapInfoHeader */
  PutLong(40);
  PutLong(gs.xWin); PutLong(gs.yWin);
  PutWord(1); PutWord(gs.fColor ? 4 : 1);
  PutLong(0 /*BI_RGB*/); PutLong(0);
  PutLong(0); PutLong(0);
  PutLong(0); PutLong(0);
  /* RgbQuad */
  if (gs.fColor)
    for (x = 0; x < 16; x++) {
      PutByte(RGBB(rgbbmp[x])); PutByte(RGBG(rgbbmp[x]));
      PutByte(RGBR(rgbbmp[x])); PutByte(0);
    }
  else {
    PutLong(0);
    PutByte(255); PutByte(255); PutByte(255); PutByte(0);
  }
  /* Data */
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


/* Begin the work of creating a graphics file. Prompt for a filename if */
/* need be, and if valid, create the file and open it for writing.      */

void BeginFileX()
{
  char sz[cchSzMax];

  if (us.fNoWrite)
    return;
#ifdef WIN
  if (gi.szFileOut == NULL)
    return;
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
#endif /* WIN */

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
    /* If autosaving in potentially rapid succession, ensure the file isn't */
    /* being opened by some other application before saving over it again.  */
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
}


/* Finish up the work of creating a graphics file. This basically consists */
/* of just calling the appropriate routine to actually write the data in   */
/* memory to a file for bitmaps and metafiles, although for PostScript we  */
/* just close file as we were already writing while creating the chart.    */

void EndFileX()
{
  if (gs.ft == ftBmp && gi.file != NULL) {
    PrintProgress("Writing chart bitmap to file.");
    if (gs.chBmpMode == 'B')
      WriteBmp(gi.file);
    else if (gs.chBmpMode == 'A')
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
  if (gi.file != NULL)
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

/* Table of PostScript header alias lines used by the program. */

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
" 0 0 1 0 360 arc setmatrix stroke}bind def\n";


/* Write a command to flush the PostScript buffer. */

void PsStrokeForce()
{
  if (gi.cStroke > 0) {              /* render any existing path */
    fprintf(gi.file, "stroke\n");
    gi.cStroke = 0;
    gi.xPen = -1;                    /* Invalidate PolyLine cache */
  }
}


/* Indicate that a certain number of PostScript commands have been done. */

void PsStroke(int n)
{
  gi.cStroke += n;
  if (gi.cStroke > 2000)    /* Whenever we reach a certain limit, flush. */
    PsStrokeForce();
}


/* Set the type of line end to be used by PostScript commands. If linecap */
/* is true, then the line ends are rounded, otherwise they are squared.   */

void PsLineCap(flag fLineCap)
{
  if (fLineCap != gi.fLineCap) {
    PsStrokeForce();
    fprintf(gi.file, "%d setlinecap\n", fLineCap);
    gi.fLineCap = fLineCap;
  }
}


/* Set the dash length to be used by PostScript line commands. */

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


/* Set a linewidth size to be used by PostScript figure primitive commands. */

void PsLineWidth(int linewidth)
{
  if ((real)linewidth != gi.rLineWid) {
    PsStrokeForce();
    fprintf(gi.file, "%d setlinewidth\n", linewidth);
    gi.rLineWid = (real)linewidth;
  }
}


/* Set a system font and size to be used by PostScript text commands. */

void PsFont(int psfont)
{
  int z;

  if (psfont != gi.nFont && gs.fFont) {
    if (psfont <= 2) {
      z = psfont == 1 ? 32*PSMUL : 23*PSMUL;
      fprintf(gi.file, "/Astro[%d 0 0 -%d 0 0]sf\n", z, z);
    } else if (psfont == 3) {
      z = 26*PSMUL;
      fprintf(gi.file, "/Times-Roman[%d 0 0 -%d 0 0]sf\n", z, z);
    } else {
      z = 10*PSMUL;
      fprintf(gi.file, "/Courier[%d 0 0 -%d 0 0]sf\n", z, z);
    }
    gi.nFont = psfont;
  }
}


/* Prompt the user for the name of a file to write the PostScript file to */
/* (if not already specified), open it, and write out file header info.   */

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
  PsFont(2);
  fprintf(gi.file, "gsave\n");
  PsLineWidth(gi.nPenWid/2);
  gi.xPen = -1;
  PrintProgress("Creating PostScript chart file.");
}


/* Write out trailing information to the PostScript file. */

void PsEnd()
{
  PsStrokeForce();
  if (gi.fEps)
    fprintf(gi.file, "%%%%EOF\n");
  else {
    fprintf(gi.file, "showpage\n");
    fprintf(gi.file, "%%%%PageTrailer\n");
    fprintf(gi.file, "%%%%Trailer\n");
    fprintf(gi.file, "%%%%DocumentFonts: Times-Roman\n");
    if (gs.fFont) {
      fprintf(gi.file, "%%%%+ Courier\n");
      fprintf(gi.file, "%%%%+ Astro\n");
    }
  }
}
#endif /* PS */


#ifdef META
/*
******************************************************************************
** Metafile Routines.
******************************************************************************
*/

/* Output one 16 bit or 32 bit value into the metafile buffer stream. */

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


/* Output any necessary metafile records to make the current actual     */
/* settings of line color, fill color, etc, be those that we know are   */
/* desired. This is generally called by the primitives routines before  */
/* any figure record is actually written into a metafile. We wait until */
/* the last moment before changing any settings to ensure that we don't */
/* output any unnecessary records, e.g. two select colors in a row.     */

void MetaSelect()
{
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
  gi.xPen = -1;    /* Invalidate PolyLine cache */
}


/* Output initial metafile header information into our metafile buffer. */
/* We also setup and create all pen, brush, and font objects that may   */
/* possibly be used in the generation and playing of the picture.       */

void MetaInit()
{
  int i, j, k;

  gi.pwMetaCur = (word *)gi.bm;
  /* Placeable Metaheader */
  MetaLong(0x9AC6CDD7L);
  MetaWord(0);      /* Not used */
  MetaWord(0); MetaWord(0);
  MetaWord(gs.xWin); MetaWord(gs.yWin);
  MetaWord(gs.xWin/6);                     /* Units per inch */
  MetaLong(0L);     /* Not used */
  MetaWord(0x9AC6 ^ 0xCDD7 ^ gs.xWin ^ gs.yWin ^ gs.xWin/6);  /* Checksum */
  /* Metaheader */
  MetaWord(1);                      /* Metafile type                    */
  MetaWord(9);                      /* Size of header in words          */
  MetaWord(0x300);                  /* Windows version                  */
  MetaLong(0L);                     /* Size of entire metafile in words */
  MetaWord(16*5+1+(gs.fFont>0)*4);  /* Number of objects in metafile    */
  MetaLong(17L);                    /* Size of largest record in words  */
  MetaWord(0);                      /* Not used                         */
  /* Setup */
  MetaEscape(17);
  MetaLong(LFromBB('A', 's', 't', 'r'));  /* "Astr" */
  MetaWord(4);                            /* Creator */
  MetaLong(14L);                          /* Bytes in string */
  MetaLong(LFromBB('A', 's', 't', 'r'));  /* "Astr" */
  MetaLong(LFromBB('o', 'l', 'o', 'g'));  /* "olog" */
  MetaLong(LFromBB(' ', szVerCore[0], '.', szVerCore[1]));  /* Version */
  MetaWord(WFromBB(szVerCore[2], 0));     /* Version part 2 */
  MetaSaveDc();
  MetaWindowOrg(0, 0);
  MetaWindowExt(gs.xWin, gs.yWin);
  MetaBkMode(1 /* Transparent */);
  /* Colors */
  for (j = 1; j <= 4; j++)
    for (i = 0; i < 16; i++) {
      k = j <= 1 ? gi.nPenWid : 0;
      MetaCreatePen(j <= 2 ? 0 : j-2 /* PS_SOLID; PS_DASH; PS_DOT */,
        k, rgbbmp[i]);
    }
  for (i = 0; i < 16; i++) {
    MetaCreateBrush(0 /* BS_SOLID */, rgbbmp[i]);
  }
  MetaCreateBrush(1 /* BS_NULL */, 0L);
  /* Fonts */
  if (gs.fFont) {
    MetaCreateFont(5, 0, -8*gi.nScale, 2 /* Symbol Charset */);
    MetaWord(WFromBB(1 /* Draft */, 1 | 0x10 /* Fixed | Roman */));
    MetaLong(LFromBB('W', 'i', 'n', 'g'));
    MetaLong(LFromBB('d', 'i', 'n', 'g'));
    MetaWord(WFromBB('s', 0));

    MetaCreateFont(8, 0, -6*gi.nScale, 0 /* Ansi Charset */);
    MetaWord(WFromBB(0 /* Default */, 2 | 0x10 /* Variable | Roman */));
    MetaLong(LFromBB('T', 'i', 'm', 'e'));
    MetaLong(LFromBB('s', ' ', 'N', 'e'));
    MetaLong(LFromBB('w', ' ', 'R', 'o'));
    MetaLong(LFromBB('m', 'a', 'n', 0));

    MetaCreateFont(6, 6*METAMUL, 10*METAMUL, 0 /* Ansi Charset */);
    MetaWord(WFromBB(1 /* Draft */, 1 | 0x30 /* Fixed | Modern */));
    MetaLong(LFromBB('C', 'o', 'u', 'r'));
    MetaLong(LFromBB('i', 'e', 'r', ' '));
    MetaLong(LFromBB('N', 'e', 'w', 0));

    MetaCreateFont(3, 0, -11*gi.nScale, 0 /* Ansi Charset */);
    MetaWord(WFromBB(0 /* Default */, 2 | 0 /* Variable | Don't Care */));
    MetaLong(LFromBB('A', 's', 't', 'r'));
    MetaLong(WFromBB('o', 0));
  }
  gi.kiLineAct = gi.kiFillAct = gi.nFontAct = gi.kiTextAct = gi.nAlignAct = -1;
}


/* Output trailing records to indicate the end of the metafile and then */
/* actually write out the entire buffer to the specifed file.           */

void WriteMeta(FILE *file)
{
  word *w;
#if FALSE
  int i;

  for (i = 16*5+1+(gs.fFont>0)*4; i >= 0; i--) {
    MetaDeleteObject(i);
  }
#endif
  MetaRestoreDc();
  MetaRecord(3, 0);    /* End record */
  *(long *)(gi.bm + 22 + 6) =
    ((long)((pbyte)gi.pwMetaCur - gi.bm) - 22) / 2;
  for (w = (word *)gi.bm; w < gi.pwMetaCur; w++) {
    PutWord(*w);
  }
}
#endif /* META */


#ifdef WIRE
/*
******************************************************************************
** Daedalus Wireframe File Routines.
******************************************************************************
*/

/* Write the wireframe file in memory to a previously opened file in the    */
/* Daedalus wireframe format. This usually consists of coordinates for each */
/* line segment, but can also include changes to the default color.         */

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

      /* Output one line segment. */
      x1 = (short)pw[0]; y1 = (short)pw[1]; z1 = (short)pw[2];
      x2 = (short)pw[3]; y2 = (short)pw[4]; z2 = (short)pw[5];
      fprintf(file, "%d %d %d %d %d %d\n", x1, y1, z1, x2, y2, z2);
      pw += 6;
    } else {

      /* Output a color change. */
      if (gs.fColor) {
        n = pw[1];
        if (n < cColor) {
          kv = rgbbmp[n];
          if (kv != rgbbmpDef[n])
            fprintf(file, "Rgb %d %d %d\n", RGBR(kv), RGBG(kv), RGBB(kv));
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


/* Add a single 16 bit number to the current wireframe file. */

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


/* Add a solid line to current wireframe file, specified by its endpoints. */

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


/* Add an octahedron of a given radius to the current wireframe file. These */
/* shapes are used to mark the exact locations of planets in the scene.     */

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
/* Add a fixed star to the current wireframe file. */

void WireStar(int x, int y, int z, ES *pes)
{
  char *pch = pes->sz;
  int n;

  /* Determine star color. */
  if (gs.fColor) {
    if (pes->ki != kDefault)
      n = pes->ki;
    else {
      n = 255 - (int)((pes->mag - rStarLite) / rStarSpan * 224.0);
      n = Min(n, 255); n = Max(n, 32);
    }
    DrawColor(n);
  }

  /* Draw star point. */
  if (!FOdd(gs.nAllStar)) {
    WireLine(x-1, y, z, x+1, y, z);
    WireLine(x, y-1, z, x, y+1, z);
    WireLine(x, y, z-1, x, y, z+1);
  } else
    WireOctahedron(x, y, z, 3 * gi.nScaleT);

  /* Draw star's name label. */
  if (!gs.fLabel || gs.nAllStar < 2)
    return;
  gi.zDefault = z + 8*gi.nScaleT;
  DrawSz(pes->pchBest, x, y, dtCent);
}
#endif


/* Given longitude and latitude values on a globe, return the 3D pixel     */
/* coordinates corresponding to them. In other words, project the globe in */
/* the 3D environment, and return where our coordinates got projected to.  */
/* Like FGlobeCalc() except for 3D wireframe format.                       */

void WireGlobeCalc(real x1, real y1, int *u, int *v, int *w, int rz, real deg)
{
  real lonMC, latMC;

  /* Compute coordinates for a general globe invoked with -XG switch. */

  if (gi.nMode == gSphere) {
    /* Chart sphere coordinates are relative to the local horizon. */
    lonMC = Tropical(is.MC); latMC = 0.0;
    EclToEqu(&lonMC, &latMC);
    x1 = Mod(rDegMax - (x1 + lonMC) + rDegQuad);
    y1 = rDegQuad - y1;
    EquToLocal(&x1, &y1, rDegQuad - Lat);
    y1 = rDegQuad - y1;
  }

  x1 = Mod(x1+deg);      /* Shift by current globe rotation value. */
  if (gs.rTilt != 0.0) {
    /* Do another coordinate shift if the globe's equator is tilted any. */
    y1 = rDegQuad - y1;
    CoorXform(&x1, &y1, gs.rTilt);
    x1 = Mod(x1); y1 = rDegQuad - y1;
  }
  *u = (int)((real)rz*RSinD(y1)*RSinD(x1)-rRound);
  *v = (int)((real)rz*RSinD(y1)*RCosD(x1)-rRound);
  *w = (int)((real)rz*RCosD(y1)-rRound);
}


/* Given longitude and latitude values, return the 3D pixel coordinates   */
/* corresponding to them. Like FMapCalc() except for 3D wireframe format. */

void WireMapCalc(real x1, real y1, int *xp, int *yp, int *zp, flag fSky,
  real lonMC, real rT, int rz, real deg)
{
  if (!fSky)
    x1 = lonMC - x1;
  if (x1 < 0.0)
    x1 += rDegMax;
  if (x1 > rDegHalf)
    x1 -= rDegMax;
  x1 = Mod(rDegHalf - rT - x1);
  y1 = rDegQuad - y1;
  WireGlobeCalc(x1, y1, xp, yp, zp, rz, deg);
}


/* Draw a globe, for either the world or the constellations. We shift the */
/* chart by specified rotational and tilt values, and may plot on the     */
/* chart each planet at its zenith position on Earth or location in       */
/* constellations. Like DrawMap() except for 3D wireframe format.         */

void WireDrawGlobe(flag fSky, real deg)
{
  char *nam, *loc, *lin, chCmd;
  int X[objMax], Y[objMax], Z[objMax], M[objMax], N[objMax], O[objMax],
    rz, lon, lat, unit = 12*gi.nScale,
    x, y, z, xold, yold, zold, m, n, o, u, v, w, i, j, k, l, nScl = gi.nScale;
  flag fNext = fTrue;
  real planet1[objMax], planet2[objMax], x1, y1, rT;
#ifdef CONSTEL
  CONST char *pch;
  flag fBlank;
  int isz = 0, nC, xT, yT, xDelta, yDelta, xLo, xHi, yLo, yHi;
#endif
#ifdef SWISS
  ES es;
#endif
#ifdef ATLAS
  real rgzon[iznMax], zon;
  CI ci;
#endif

  /* Set up some variables. */
  rz = Min(gs.xWin/2, gs.yWin/2);
  if (gi.nMode == gSphere)
    rz -= 7*gi.nScale;

  loop {

    /* Get the next chunk of data to process. Get the starting position, */
    /* map it to the screen, and set the drawing color appropriately.    */

    if (fNext) {
      fNext = fFalse;

      /* For constellations, get data for the next constellation shape. */

      if (fSky) {
#ifdef CONSTEL
        isz++;
        if (isz > cCnstl)
          break;
        DrawColor(gi.nMode == gSphere ? kPurpleB :
          (gs.fAlt ? kBlueB : kDkBlueB));
        pch = szDrawConstel[isz];
        lon = nDegMax -
          (((pch[2]-'0')*10+(pch[3]-'0'))*15+(pch[4]-'0')*10+(pch[5]-'0'));
        lat = 90-((pch[6] == '-' ? -1 : 1)*((pch[7]-'0')*10+(pch[8]-'0')));
        if (lon >= nDegMax)
          lon -= nDegMax;
        pch += 9;
        xLo = xHi = xT = xold = x = lon;
        yLo = yHi = yT = yold = y = lat;
        nC = 0;
        WireGlobeCalc((real)x, (real)y, &m, &n, &o, rz, rDegMax - deg);
        k = l = fTrue;
#else
        ;
#endif

      /* For world maps, get data for the next coastline piece. */

      } else {
        if (!FReadWorldData(&nam, &loc, &lin))
          break;
        i = nam[0]-'0';
        DrawColor(!gs.fAlt && !gs.fColorHouse ? gi.kiGray :
          (i ? kRainbowB[i] : kDkBlueB));
        lon = (loc[0] == '+' ? 1 : -1)*
          ((loc[1]-'0')*100 + (loc[2]-'0')*10 + (loc[3]-'0'));
        lat = (loc[4] == '+' ? 1 : -1)*((loc[5]-'0')*10 + (loc[6]-'0'));
        x = 180-lon;
        y = 90-lat;
        WireGlobeCalc((real)x, (real)y, &m, &n, &o, rz, deg);
        k = l = fTrue;
      }
    }

    /* Get the next unit from the string to draw on the screen as a line. */

    if (fSky) {

      /* For constellations we have a cache of how long we should keep    */
      /* going in the previous direction, as say "u5" for up five should  */
      /* move our pointer up five times without advancing string pointer. */

#ifdef CONSTEL
      if (nC <= 0) {
        if (!(chCmd = *pch)) {
          fNext = fTrue;
          if (gs.fText) {

            /* If we've reached the end of current constellation, compute */
            /* the center location in it based on lower and upper bounds  */
            /* we've maintained, and print the name of the constel there. */

            xT = xLo + (xHi - xLo)*(szDrawConstel[isz][0]-'1')/8;
            yT = yLo + (yHi - yLo)*(szDrawConstel[isz][1]-'1')/8;
            if (xT < 0)
              xT += nDegMax;
            else if (xT > nDegMax)
              xT -= nDegMax;
            WireGlobeCalc((real)xT, (real)yT, &x, &y, &z, rz, deg);
            DrawColor(gi.nMode == gSphere || !gs.fAlt ? gi.kiGray : kDkGreenB);
            gi.zDefault = z;
            DrawSz(szCnstlAbbrev[isz], x, y, dtCent);
          }
          continue;
        }
        pch++;

        /* Get the next direction and distance from constellation string. */

        if (fBlank = (chCmd == 'b'))
          chCmd = *pch++;
        xDelta = yDelta = 0;
        switch (chCmd) {
        case 'u': yDelta = -1; break;    /* Up    */
        case 'd': yDelta =  1; break;    /* Down  */
        case 'l': xDelta = -1; break;    /* Left  */
        case 'r': xDelta =  1; break;    /* Right */
        case 'U': yDelta = -1; nC = (yT-1)%10+1;    break;  /* Up until    */
        case 'D': yDelta =  1; nC = 10-yT%10;       break;  /* Down until  */
        case 'L': xDelta = -1; nC = (xT+599)%15+1;  break;  /* Left until  */
        case 'R': xDelta =  1; nC = 15-(xT+600)%15; break;  /* Right until */
        default: PrintError("Bad wire draw.");        /* Shouldn't happen. */
        }
        if (chCmd >= 'a')
          nC = NFromPch(&pch);  /* Figure out how far to draw. */
      }
      nC--;
      xT += xDelta; x += xDelta;
      yT += yDelta; y += yDelta;
      if (fBlank) {
        xold = x; yold = y;    /* We occasionally want to move the pointer */
        l = fFalse;            /* without drawing the line on the screen.  */
        continue;
      }
      if (xT < xLo)         /* Maintain our bounding rectangle for this */
        xLo = xT;           /* constellation if we crossed over it any. */
      else if (xT > xHi)
        xHi = xT;
      if (yT < yLo)
        yLo = yT;
      else if (yT > yHi)
        yHi = yT;
#else
      ;
#endif

    } else {

      /* Get the next unit from the much simpler world map strings. */

      if (!(chCmd = *lin)) {
        fNext = fTrue;
        continue;
      }
      lin++;

      /* Each unit is exactly one character in the coastline string. */

      if (chCmd == 'L' || chCmd == 'H' || chCmd == 'G')
        x--;
      else if (chCmd == 'R' || chCmd == 'E' || chCmd == 'F')
        x++;
      if (chCmd == 'U' || chCmd == 'H' || chCmd == 'E')
        y--;
      else if (chCmd == 'D' || chCmd == 'G' || chCmd == 'F')
        y++;
    }

    /* Transform map coordinates to screen coordinates and draw a line. */

    while (x >= nDegMax)    /* Take care of coordinate wrap around. */
      x -= nDegMax;
    while (x < 0)
      x += nDegMax;

    /* For globes, we have to go do a complicated transformation. */

    WireGlobeCalc((real)x, (real)y, &u, &v, &w, rz, deg);
    if (l && !(fSky && (x > xold || y > yold)))
      WireLine(m, n, o, u, v, w);
    m = u; n = v; o = w;
    l = fTrue;
    xold = x; yold = y;
  }

  /* Now, if we are in an appropriate bonus chart mode, draw each planet at */
  /* its zenith or visible location on the globe, if not hidden.            */

  if (gs.fAlt || gi.nMode == gAstroGraph || gi.nMode == gSphere)
    return;
  rT = gs.fConstel ? rDegHalf : Lon;
  if (rT < 0.0)
    rT += rDegMax;
  for (i = 0; i <= cObj; i++) {
    planet1[i] = Tropical(planet[i]);
    planet2[i] = planetalt[i];
    EclToEqu(&planet1[i], &planet2[i]);    /* Calculate zenith long. & lat. */
  }

  /* Compute screen coordinates of each object, if it's even visible. */

  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    WireMapCalc(planet1[i], planet2[i], &u, &v, &w, fSky,
      planet1[oMC], rT, rz, deg);
    X[i] = u;
    Y[i] = v;
    Z[i] = w;
    M[i] = X[i]; N[i] = Y[i]; O[i] = Z[i]-unit/2;
  }

  /* Draw ecliptic equator and zodiac sign wedges. */

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
          /* Coordinates for zodiac sign wedge longitude lines. */
          j = l*30; k = i;
        } else {
          /* Coordinates for ecliptic equator latitude line. */
          j = i+90 + (l < -1)*180; k = 0;
        }
        x1 = Tropical((real)j);
        y1 = (real)k;
        EclToEqu(&x1, &y1);
        WireMapCalc(x1, y1, &u, &v, &w, fSky, planet1[oMC],
          rT, rz, deg);
        WirePoint(u, v, w);
      }
    }
  }

  /* Draw Earth's equator. */

  if (gs.fEquator) {
    DrawColor(kPurpleB);
    for (i = 0; i < nDegMax; i++) {
      x1 = (real)i; y1 = 90.0;
      WireGlobeCalc(x1, y1, &j, &k, &l, rz, deg);
      WirePoint(j, k, l);
    }
  }

  /* Draw chart latitude and plot chart location. */

  if (us.fLatitudeCross && !fSky) {
    DrawColor(kMagentaB);
    for (i = 0; i < nDegMax; i++) {
      x1 = (real)i; y1 = rDegQuad - Lat;
      WireGlobeCalc(x1, y1, &j, &k, &l, rz, deg);
      WirePoint(j, k, l);
    }
    x1 = Mod(rDegHalf - Lon); y1 = rDegQuad - Lat;
    WireGlobeCalc(x1, y1, &j, &k, &l, rz, deg);
    WireOctahedron(j, k, l, gi.nScale);
  }

#ifdef SWISS
  /* Draw extra stars. */

  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      x1 = es.lon; y1 = es.lat;
      x1 = Tropical(x1);
      EclToEqu(&x1, &y1);
      WireMapCalc(x1, y1, &j, &k, &l, fSky, planet1[oMC], rT, rz, deg);
      WireStar(j, k, l, &es);
    }
  }

  /* Draw extra asteroids. */

  if (gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fFalse);
    while (SwissComputeAsteroid(is.T, &es, fFalse)) {
      x1 = es.lon; y1 = es.lat;
      x1 = Tropical(x1);
      EclToEqu(&x1, &y1);
      WireMapCalc(x1, y1, &j, &k, &l, fSky, planet1[oMC], rT, rz, deg);
      WireStar(j, k, l, &es);
    }
  }
#endif

#ifdef ATLAS
  /* Draw locations of cities from atlas. */

  if (gs.fLabelCity && !fSky && FEnsureAtlas()) {
    if (!gs.fLabelAsp)
      DrawColor(kOrangeB);
    if (gs.nLabelCity >= 4) {
      // Initialize array of current offsets for each time zone area.
      ci = ciMain;
#ifdef WIN
      l = wi.fNoPopup; wi.fNoPopup = fTrue;
#endif
      for (i = 0; i < iznMax; i++) {
        DisplayTimezoneChanges(i, 0, &ci);
        rgzon[i] = ci.zon - ci.dst;
      }
#ifdef WIN
      wi.fNoPopup = l;
#endif
    }
    for (i = 0; i < is.cae; i++) {
      x1 = nDegHalf - is.rgae[i].lon;
      y1 = rDegQuad - is.rgae[i].lat;
      WireGlobeCalc(x1, y1, &u, &v, &w, rz, deg);
      if (gs.fLabelAsp) {
        if (gs.nLabelCity < 3) {
          l = is.rgae[i].icn;
          if (gs.nLabelCity == 2 && (l == icnUS || l == icnCA))
            DrawColor(kRainbowB[is.rgae[i].istate % 7 + 1]);
          else
            DrawColor(kRainbowB[l == icnUS ? 6 :
              (l == icnFR ? 5 : l % 7 + 1)]);
        } else if (FEnsureTimezoneChanges()) {
          l = is.rgae[i].izn;
          if (gs.nLabelCity == 3)
            zon = ZondefFromIzn(l);
          else
            zon = rgzon[l];
          if (zon == zonLMT)
            DrawColor(kDkGrayB);
          else if (zon == RFloor(zon)) {
            l = (int)(zon + rDegMax) % 6;
            DrawColor(kRainbowB[l + (l > 0) + 1]);
          } else
            DrawColor(kMagentaB);
        }
      }
      WirePoint(u, v, w);
    }
  }
#endif

  /* Draw ecliptic equator and zodiac sign wedges. */

  if (us.fHouse3D) {
    if (!gs.fColorSign)
      DrawColor(kDkGreenB);
    for (l = -2; l < cSign; l++) {
      if (gs.fColorSign && l >= 0)
        DrawColor(kSignB(l+1));
      for (i = -90; i < 90; i++) {
        if (gs.fColorSign && l < 0 && i % 30 == 0)
          DrawColor(kSignB((i+90)/30 + (l < -1)*6 + 1));
        if (l >= 0) {
          /* Coordinates for zodiac sign wedge longitude lines. */
          j = l*30; k = i;
        } else {
          /* Coordinates for ecliptic equator latitude line. */
          j = i+90 + (l < -1)*180; k = 0;
        }
        x1 = Tropical((real)j);
        y1 = (real)k;
        EclToEqu(&x1, &y1);
        WireMapCalc(x1, y1, &u, &v, &w, fSky, planet1[oMC], rT, rz, deg);
        WireOctahedron(u, v, w, unit/2);
      }
    }
  }

  /* Draw MC, IC, Asc, and Des lines for each object, as great circles */
  /* around the globe. The result is a 3D astro-graph chart.           */

  if (!fSky) for (i = 0; i <= cObj; i++) if (FProper(i)) {
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
        WireMapCalc(x1, y1, &x, &y, &z, fSky, planet1[oMC], rT, rz, deg);
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
        WireMapCalc(x1, y1, &x, &y, &z, fSky, planet1[oMC], rT, rz, deg);
        if (j > 0)
          WireLine(xold, yold, zold, x, y, z);
        xold = x; yold = y; zold = z;
      }
    }
  }

  /* Now that we have the coordinates of each object, draw their glyphs. */

  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    gi.zDefault = O[i];
    DrawObject(i, M[i], N[i]);
    DrawColor(kObjB[i]);
    WireOctahedron(X[i], Y[i], Z[i], gi.nScale);
  }
}


/* Generate a chart depicting an aerial view of the solar system in space, */
/* with all the planets drawn around the Sun, and the specified central    */
/* in the middle. Like XChartOrbit() except for 3D wireframe format.       */

void WireChartOrbit()
{
  int x[objMax], y[objMax], z[objMax], zWin, xd, yd, i, j, k;
  real sx, sz, xp, yp, zp, xp2, yp2, zp2;
#ifdef SWISS
  ES es, *pes1, *pes2;
  int xT, yT, zT, x2, y2, z2;
#endif

  /* Compute coordinates of planets. */
  i = gi.nScale/gi.nScaleT;
  sz = gs.rspace > 0.0 ? gs.rspace : (i <= 1 ? 90.0 : (i == 2 ? 30.0 :
    (i == 3 ? 6.0 : (gi.nScaleText <= 1 ? 1.0 : 0.006))));
  zWin = Min(gs.xWin, gs.yWin);
  sx = (real)zWin/sz;
  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    xp = space[i].x; yp = space[i].y; zp = space[i].z;
    if (us.nStar > 0) {
      xp /= rLYToAU; yp /= rLYToAU; zp /= rLYToAU;
    }
    if (us.fHouse3D)
      OrbitPlot(&xp, &yp, &zp, sz, i, &space[oEar]);
    x[i] = -(int)(xp*sx); y[i] = (int)(yp*sx); z[i] = (int)(zp*sx);
  }

  /* Draw zodiac lines. */
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

  /* Draw lines connecting planets which have aspects between them. */
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

  /* Prepare to draw orbital trails. */
  if (gs.cspace > 0 && gi.rgspace == NULL) {
    gi.rgspace = (PT3R *)PAllocate(sizeof(PT3R)*oNorm1*gs.cspace, "orbits");
    if (gi.rgspace == NULL)
      return;
  }

  /* Draw planets. */
  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    DrawColor(kObjB[i]);
    if (!gs.fAlt || i > oVes)
      j = 3 * gi.nScaleT;
    else
      j = (int)(RLog10(rObjDiam[i]) * (real)gi.nScaleT);
    WireOctahedron(x[i], y[i], z[i], j);
    gi.zDefault = z[i] + (j + 5*gi.nScaleT);
    DrawObject(i, x[i], y[i]);

    /* Draw orbital trails. */
    for (j = 0; j < gi.cspace-1; j++) {
      k = (gi.ispace - gi.cspace + j + gs.cspace) % gs.cspace;
      k = k*oNorm1 + i;
      xp = gi.rgspace[k].x; yp = gi.rgspace[k].y; zp = gi.rgspace[k].z;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, &zp, sz, i, &gi.rgspace[k - i + oEar]);

      k = (gi.ispace - gi.cspace + j + 1 + gs.cspace) % gs.cspace;
      k = k*oNorm1 + i;
      xp2 = gi.rgspace[k].x; yp2 = gi.rgspace[k].y; zp2 = gi.rgspace[k].z;
      if (us.fHouse3D)
        OrbitPlot(&xp2, &yp2, &zp2, sz, i, &gi.rgspace[k - i + oEar]);

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
  /* Draw extra stars. */
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

  /* Draw extra asteroids. */
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


/* Translate to chart 3D coordinates, that indicate how to compose a 3D   */
/* chart sphere, for the -XX wireframe chart. Inputs may be local horizon */
/* altitude and azimuth coordinates, local horizon prime vertical, local  */
/* horizon meridian, zodiac position and latitude, or Earth coordinates.  */

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
  azi = Mod(azi + Lon + rDegQuad);
  CoorXform(&azi, &alt, rDegQuad - Lat);
  WireSphereLocal(azi + rDegQuad, -alt, zr, xp, yp, zp);
}


/* Draw a chart sphere (like a chart wheel but in 3D) as done with the -XX */
/* switch. Like XChartSphere() except for 3D wireframe format.             */

void WireChartSphere()
{
  char sz[cchSzDef];
  int rgx[objMax], rgy[objMax], rgz[objMax], zGlyph, zGlyph2,
    cChart, iChart, zr, xo = 0, yo = 0, zo = 0, xp, yp, zp, i, j, k, k2, nSav;
  flag fNoHorizon;
  real lonMC, latMC;
  CONST CP *pcp;
  CP cpSav;
#ifdef SWISS
  ES es, *pes1, *pes2;
#endif

  /* Initialize variables. */
  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;

  fNoHorizon = ignorez[0] && ignorez[1] && ignorez[2] && ignorez[3];
  zGlyph = 7*gi.nScale; zGlyph2 = 14*gi.nScale;
  zr = Min(gs.xWin >> 1, gs.yWin >> 1) - zGlyph;
  cChart = 1 +
    (us.nRel <= rcDual) + (us.nRel <= rcTriWheel) + (us.nRel <= rcQuadWheel);

  lonMC = Tropical(is.MC); latMC = 0.0;
  EclToEqu(&lonMC, &latMC);
  latMC = Lat;
  is.lonMC = lonMC; is.latMC = latMC;

  /* Draw constellations. */
  if (gs.fConstel) {
    neg(gs.rTilt);
    WireDrawGlobe(fTrue, rDegMax - gs.rRot);
    neg(gs.rTilt);
  }

  /* Draw horizon. */
  if (!fNoHorizon || (!gs.fHouseExtra && !us.fHouse3D)) {
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

  /* Draw Earth's equator. */
  if (gs.fEquator) {
    DrawColor(kPurpleB);
    for (i = 0; i <= nDegMax; i++) {
      WireSphereEarth((real)i, 0.0, zr, &xp, &yp, &zp);
      if (i > 0)
        WireLine(xo, yo, zo, xp, yp, zp);
      xo = xp; yo = yp; zo = zp;
    }
  }

  /* Draw prime vertical. */
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

  /* Draw house wedges and meridian. */
  if (!gs.fColorHouse)
    DrawColor(kDkGreenB);
  for (j = 30; j < nDegMax; j += (j != 150 ? 30 : 60)) {
    if (!(!gs.fHouseExtra && !us.fHouse3D) &&
      !((j == 90 || j == 270) && !fNoHorizon))
      continue;
    if (gs.fColorHouse)
      DrawColor(kSignB(j/30 + 1));
    for (i = -90; i <= 90; i++) {
      WireSpherePrime((real)j, (real)i, zr, &xp, &yp, &zp);
      if (i > -90) {
        WireLine(xo, yo, zo, xp, yp, zp);
        if ((j == 90 || j == 270) && i % 30 != 0) {
          k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
          for (k2 = -k; k2 <= k; k2 += (k << 1)) {
            WireSphereMeridian((real)(j > 180 ? i+180 : 360-i), (real)k2 / 2.0,
              zr, &xo, &yo, &zo);
            WireLine(xo, yo, zo, xp, yp, zp);
          }
        }
      }
      xo = xp; yo = yp; zo = zp;
    }
  }
  if (!gs.fHouseExtra && us.fHouse3D)
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

  /* Draw sign wedges. */
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

  /* Label signs. */
  if (!us.fVedic) {
    nSav = gi.nScale;
    gi.nScale = gi.nScaleText * gi.nScaleT;
    for (j = -80; j <= 80; j += 160)
      for (i = 1; i <= cSign; i++) {
        WireSphereZodiac((real)(i*30-15), (real)j, zr, &xp, &yp, &zp);
        DrawColor(gs.fColorSign ? kSignB(i) : kDkBlueB);
        gi.zDefault = zp;
        DrawSign(i, xp, yp);
      }
    gi.nScale = nSav;
  }

  /* Label houses. */
  if (!gs.fHouseExtra) {
    nSav = gi.nScale;
    gi.nScale = gi.nScaleText * gi.nScaleT;
    for (j = -82; j <= 82; j += 164)
      for (i = 1; i <= cSign; i++) {
        WireSpherePrime((real)(i*30-15), (real)j, zr, &xp, &yp, &zp);
        DrawColor(gs.fColorHouse ? kSignB(i) : kDkGreenB);
        gi.zDefault = zp;
        DrawHouse(i, xp, yp);
      }
    gi.nScale = nSav;
  }

  /* Label directions. */
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
  /* Draw extra stars. */
  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      WireSphereZodiac(es.lon, es.lat, zr, &xp, &yp, &zp);
      WireStar(xp, yp, zp, &es);
    }
    DrawColor(gi.kiLite);
    EnumStarsLines(fTrue, NULL, NULL);
    while (EnumStarsLines(fFalse, &pes1, &pes2)) {
      WireSphereZodiac(pes1->lon, pes1->lat, zr, &xo, &yo, &zo);
      WireSphereZodiac(pes2->lon, pes2->lat, zr, &xp, &yp, &zp);
      WireLine(xo, yo, zo, xp, yp, zp);
    }
  }

  /* Draw extra asteroids. */
  if (gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fFalse);
    while (SwissComputeAsteroid(is.T, &es, fFalse)) {
      WireSphereZodiac(es.lon, es.lat, zr, &xp, &yp, &zp);
      WireStar(xp, yp, zp, &es);
    }
  }
#endif

  /* Determine set of planet data to use. */
  for (iChart = 1; iChart <= cChart; iChart++) {
    FProcessCommandLine(szWheelX[iChart]);
    if (iChart <= 1)
      pcp = rgpcp[us.nRel <= rcDual];
    else
      pcp = rgpcp[iChart];
    cpSav = cp0;
    cp0 = *pcp;

  /* Draw planet glyphs, and spots for actual local location. */
  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    WireSphereZodiac(planet[i], planetalt[i], zr, &xp, &yp, &zp);
    rgx[i] = xp; rgy[i] = yp; rgz[i] = zp;
    gi.zDefault = rgz[i] - zGlyph2;
    DrawObject(i, rgx[i], rgy[i]);
    DrawColor(kObjB[i]);
    WireOctahedron(rgx[i], rgy[i], rgz[i], gi.nScale);
  }

  /* Draw lines connecting planets which have aspects between them. */
  if (!FCreateGrid(fFalse))
    return;
  for (j = cObj; j >= 1; j--)
    for (i = j-1; i >= 0; i--)
      if (grid->n[i][j] && FProper(i) && FProper(j)) {
        DrawColor(kAspB[grid->n[i][j]]);
        WireLine(rgx[i], rgy[i], rgz[i], rgx[j], rgy[j], rgz[j]);
      }

    cp0 = cpSav;
  } /* iChart */
  FProcessCommandLine(szWheelX[0]);

  /* Draw center point. */
  DrawColor(gi.kiOn);
  WireOctahedron(0, 0, 0, gi.nScale * 2);
}
#endif /* WIRE */
#endif /* GRAPH */

/* xdevice.cpp */
