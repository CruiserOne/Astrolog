/*
** Astrolog (Version 7.30) File: astrolog.h
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
** Last code change made 9/8/2021.
*/

/*
** TO COMPILE: For most systems, especially Unix, Windows, and Macs, the only
** changes that should need to be made to the code are to edit or comment the
** #define's below to equal the particulars of your own system and locale.
**
** SYSTEM SECTION: These settings describe platform and hardware specifics.
** They are all required to be set properly or the program probably won't
** even compile. Some of these are technically optional and can be commented
** out even if your system would normally support them, e.g. the X11 graphics
** can be disabled even if you are running a system that supports X windows.
*/

//#define PC /* Comment out this #define if you have a Unix, Mac, or other */
           /* system that isn't a generic PC running DOS or MS Windows.  */

//#define MACOLD /* Comment out this #define if you're not compiling for an  */
               /* old pre-OSX Mac. Modern Mac systems should not use this. */

#define X11 /* Comment out this #define if you don't have X windows, or */
            /* else have them and don't wish to compile in X graphics.  */

//#define WIN /* Comment out this #define if you don't have MS Windows, or */
            /* else have them but want a command line version instead.   */

//#define WCLI /* Comment out this #define if you don't want to compile a    */
             /* command line Windows version that can still popup windows. */

//#define MACG /* Comment out this #define if you don't have a Mac, or else  */
             /* have one and don't wish to compile in Mac screen graphics. */
             /* Note modern Mac's may be able to compile with X11 instead. */

#define TIME /* Comment out this #define if your compiler can't take the  */
             /* calls to the 'time' or 'localtime' functions as in time.h */

#define SWITCHES /* Comment out this #define if your system can not handle */
                 /* parameters on the command line (such as Mac's).        */

#define ENVIRON /* Comment out this #define if your system doesn't have  */
                /* environment variables or can't compile calls to them. */

//#define ATOF /* Comment out this #define if you have a system in which  */
             /* 'atof' and related functions aren't defined in stdio.h, */
             /* such as most PC's, Linux, VMS compilers, and NeXT's.    */

#define PROTO /* Comment out this #define if you have an ancient compiler */
              /* which doesn't allow full Ansi function prototypes. This  */
              /* is for programmers only and has no effect on executable. */

/*
** FEATURES SECTION: These settings describe features that are always
** available to be compiled into the program no matter what platform or
** hardware is available. Their settings are always optional. Warning: If you
** disable any of these, you may need to remove switches from the default
** astrolog.as settings file, that correspond to features no longer available.
*/

#define GRAPH /* Comment out this #define if you don't want any graphics   */
              /* in the program. This switch allows at least generation of */
              /* bitmap files and must be set if any of the more advanced  */
              /* graphics feature additions are also compiled in.          */

#define SWISS /* Comment out this #define if you don't want the Swiss      */
              /* Ephemeris most accurate calculation features and formulas */
              /* to be compiled into the program (as accessed with -b).    */

#define PLACALC /* Comment out this #define if you don't want the Placalc */
                /* less accurate calculation features and formulas to be  */
                /* compiled into the program (as accessed with -bp).      */

#define MATRIX /* Comment out this #define if you don't want the Matrix  */
               /* much less accurate calculation formulas to be compiled */
               /* into the program (as accessed with -bm).               */

#define PS /* Comment out this #define if you don't want the ability to */
           /* generate charts in the PostScript graphics format.        */

#define META /* Comment out this #define if you don't want the ability to  */
             /* generate charts in the MS Windows metafile picture format. */

#define WIRE /* Comment out this #define if you don't want the ability to */
             /* generate charts in the Daedalus wireframe vector format.  */

#define ATLAS /* Comment out this #define if you don't want the built in  */
              /* city atlas and time zone change features in the program. */

#define INTERPRET /* Comment out this #define if you don't want the ability */
                  /* to display interpretations of the various chart types. */

#define ARABIC /* Comment out this #define if you don't want any chart     */
               /* lists that include Arabic parts included in the program. */

#define CONSTEL /* Comment out this #define if you don't want any of the */
                /* astronomical constellation charts in the program.     */

#define BIORHYTHM /* Comment out this #define if you don't want the    */
                  /* non-astrological biorhythm charts in the program. */

#define EXPRESS /* Comment out this #define if you don't want programmable */
                /* AstroExpression customization options in the program.   */

/*
** DATA CONFIGURATION SECTION: These settings describe particulars of
** your own location and where the program looks for certain info. It is
** recommended that these values be changed appropriately, although the
** program will still run if they are left alone. They may also be
** specified within the default settings file or in the program itself.
*/

#ifndef PC
#define DEFAULT_DIR "~/astrolog"
#else
#define DEFAULT_DIR "C:\\Astrolog"
#endif
  // Change this string to directory path program should look in for the
  // astrolog.as default file, if one is not in the executable directory, the
  // current directory, or in directories indicated by Astrolog environment
  // variables. For PC systems, use two backslashes instead of one forward
  // slash to divide subdirectories. For Unix systems, it may be necessary to
  // expand "~" to the full path. Files will still be found even if not set!

#define CHART_DIR DEFAULT_DIR
  // This string is the directory the program looks in for chart info files
  // (-i switch) if not in the executable or current directory. This is
  // normally the default dir above but may be changed to be somewhere else.

#define EPHE_DIR DEFAULT_DIR
  // This string is the directory the program looks in for the ephemeris files
  // as accessed with the -b switch. This is normally the default dir above
  // but may be changed to be somewhere else.

#define DEFAULT_LONG DM(122,20) 
#define DEFAULT_LAT  DM(47, 36)
  // Change numbers to longitude and latitude of your current location. Use
  // negative values for eastern or southern degrees.

#define DEFAULT_ZONE 8.00
  // Change this number to the time zone of your current location in hours
  // before (west of) UTC. Use negative values for eastern zones.

/*
** OPTIONAL CONFIGURATION SECTION: Although not necessary, one may like
** to change some of the values below: These constants affect some of
** the default parameters and related options. They may also be
** specified within the default settings file or in the program itself.
*/

#define DEFAULT_SYSTEM 0
  // Normally, Placidus houses are used (unless the user specifies otherwise).
  // If you want a different default system, change this number to a value
  // from 0-21 (values same as in -c switch).

#define DEFAULT_ASPECTS 5
  // Default number of aspects to include in charts.

#define DIVISIONS 48
  // Greater numbers means more accuracy but slower calculation, of exact
  // aspect and transit times.

#define DEFAULT_INFOFILE "astrolog.as"
  // Name of file to look in for default program settings (which will override
  // the compile time values here, if the file exists).

#define DEFAULT_ATLASFILE "atlas.as"
  // Name of file to look in for default atlas city and time zone list.

#define DEFAULT_TIMECHANGE "timezone.as"
  // Name of file to look in for default list of time zone changes.

#define BITMAP_EARTH "earth.bmp"
  // Name of file to look in for bitmap of world map.

#define ENVIRONALL "ASTROLOG"
#define ENVIRONVER "ASTR"
  // Name of environment variables to look in for chart, ephemeris, and
  // default files. The second name is a version specific variable which can
  // also have the current version appended to it before it is accessed.

#define WHEELCOLS 15    // Affects width of each house in wheel display.
#define WHEELROWS 11    // Max no. of objects that can be in a wheel house.
#define SCREENWIDTH 80  // Number of columns to print interpretations in.
#define MONTHSPACE 3    // Number of spaces between each calendar column.
#define MAXINDAY 200    // Max number of aspects or transits displayable.
#define MAXCROSS 750    // Max number of latitude crossings displayable.
#define BIODAYS 14      // Days to include in graphic biorhythms.
#define CREDITWIDTH 74  // Number of text columns in the -Hc credit screen.
#define MAXSWITCHES 100 // Max number of switch parameters per input line.
#define PSGUTTER 9      // Points of white space on PostScript page edge.

#ifdef GRAPH            // For graphics, this char affects how bitmaps are
#ifndef PC              // written. 'N' is written like with the 'bitmap
#define BITMAPMODE 'C'  // program, 'C' is compacted somewhat (files have
#else                   // less spaces), and 'V' is compacted even more.
#define BITMAPMODE 'B'  // 'A' means write as rectangular Ascii text file.
#endif                  // 'B' means write as Windows bitmap (.bmp) file.
#endif // GRAPH

/*
** By the time you reach here and the above values are customized as
** desired, Astrolog is ready to be compiled! Be sure to similarly
** change the values in the astrolog.as file, which will override any
** corresponding compile time values here. Don't change any of the
** values in the section below unless you know what you're doing.
*/

#ifdef GRAPH
#define BITMAPX    4096  // Maximum window size allowed.
#define BITMAPY    4096
#define BITMAPX1    180  // Minimum window size allowed.
#define BITMAPY1    180
#define DEFAULTX    600  // Default window size.
#define DEFAULTY    600
#define SIDESIZE    160  // Size of wheel chart information sidebar.
#define MAXMETA 8000000  // Max bytes allowed in a metafile.
#define METAMUL      12  // Metafile coordinate to chart pixel ratio.
#define PSMUL        11  // PostScript coordinate to chart pixel ratio.
#define WIREMUL      10  // Wireframe coordinate to chart pixel ratio.
#define CELLSIZE     14  // Size for each cell in the aspect grid.
#define DEFORB      7.0  // Min distance glyphs can be from each other.
#define MAXSCALE    400  // Max scale factor as passed to -Xs swtich.
#endif // GRAPH

// Ascii and other characters used to display text charts.
#define chH    (char)(us.fAnsiChar ? 196 : '-')
#define chV    (char)(us.fAnsiChar ? 179 : '|')
#define chC    (char)(us.fAnsiChar ? 197 : '|')
#define chNW   (char)(us.fAnsiChar ? 218 : '+')
#define chNE   (char)(us.fAnsiChar ? 191 : '+')
#define chSW   (char)(us.fAnsiChar ? 192 : '+')
#define chSE   (char)(us.fAnsiChar ? 217 : '+')
#define chJN   (char)(us.fAnsiChar ? 193 : '-')
#define chJS   (char)(us.fAnsiChar ? 194 : '-')
#define chJW   (char)(us.fAnsiChar ? 180 : '|')
#define chJE   (char)(us.fAnsiChar ? 195 : '|')
#define chDeg0 (char)(us.fAnsiChar ? 248 : ' ')
#define chDeg1 (char)(us.fAnsiChar ? 248 : ':')


/*
** One shouldn't ever need to change anything below this line to compile.
*/

//#define BETA  // Uncomment to compile in beta message on startup.
#define ASTROLOG
#ifdef _DEBUG
#define DEBUG
#endif
#ifdef SWISS
#define EPHEM
#ifdef GRAPH
#define SWISSGRAPH
#endif
#endif
#ifdef PLACALC
#define EPHEM
#endif

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#include <stdio.h>
#ifndef ATOF
#include <stdlib.h>
#endif
#include <math.h>
#ifdef PC
#include <malloc.h>
#include <windows.h>
#endif
#ifdef TIME
#include <time.h>
#endif

#ifdef X11
#define ISG
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif
#ifdef WIN
#define ISG
#define WINANY
#include <windows.h>
#include <commdlg.h>
#include <objbase.h>
#include <comdef.h>
#include <comdefsp.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include "resource.h"
#endif
#ifdef MACG
#define ISG
#endif
#ifdef WCLI
#define ISG
#define WINANY
#include <windows.h>
#endif
#ifdef PC
#ifdef _WIN64
#define szArchCore "64 bit"
#else
#define szArchCore "32 bit"
#endif
#define INLINE __forceinline
#else
#define INLINE inline
#endif // PC

#ifdef PS
#define VECTOR
#endif
#ifdef META
#define VECTOR
#endif
#ifdef WIRE
#define VECTOR
#endif
#ifdef SWISS
#ifdef WINANY
#define JPLWEB
#endif
#endif // SWISS


/*
** Make sure only legal combinations of options are active.
*/

#ifdef PLACALC
#ifndef MATRIX
#error "If 'PLACALC' is defined 'MATRIX' must be too"
#endif
#endif // PLACALC

#ifdef MACOLD
#ifdef SWITCHES
#error "If 'MACOLD' is defined 'SWITCHES' must not be as well"
#endif
#ifdef ENVIRON
#error "If 'MACOLD' is defined 'ENVIRON' must not be as well"
#endif
#endif // MACOLD

#ifdef X11
#ifndef GRAPH
#error "If 'X11' is defined 'GRAPH' must be too"
#endif
#ifdef WIN
#error "If 'X11' is defined 'WIN' must not be as well"
#endif
#ifdef MACG
#error "If 'X11' is defined 'MACG' must not be as well"
#endif
#ifdef WCLI
#error "If 'X11' is defined 'WCLI' must not be as well"
#endif
#ifdef PC
#error "If 'X11' is defined 'PC' must not be as well"
#endif
#endif // X11

#ifdef WIN
#ifndef GRAPH
#error "If 'WIN' is defined 'GRAPH' must be too"
#endif
#ifdef X11
#error "If 'WIN' is defined 'X11' must not be as well"
#endif
#ifdef MACG
#error "If 'WIN' is defined 'MACG' must not be as well"
#endif
#ifdef WCLI
#error "If 'WIN' is defined 'WCLI' must not be as well"
#endif
#ifndef PC
#error "If 'WIN' is defined 'PC' must be too"
#endif
#endif // WIN

#ifdef MACG
#ifndef GRAPH
#error "If 'MACG' is defined 'GRAPH' must be too"
#endif
#ifdef X11
#error "If 'MACG' is defined 'X11' must not be as well"
#endif
#ifdef WIN
#error "If 'MACG' is defined 'WIN' must not be as well"
#endif
#ifdef WCLI
#error "If 'MACG' is defined 'WCLI' must not be as well"
#endif
#ifdef PC
#error "If 'MACG' is defined 'PC' must not be as well"
#endif
#endif // MACG

#ifdef WCLI
#ifndef GRAPH
#error "If 'WCLI' is defined 'GRAPH' must be too"
#endif
#ifdef X11
#error "If 'WCLI' is defined 'X11' must not be as well"
#endif
#ifdef WIN
#error "If 'WCLI' is defined 'WIN' must not be as well"
#endif
#ifdef MACG
#error "If 'WCLI' is defined 'MACG' must not be as well"
#endif
#ifndef PC
#error "If 'WCLI' is defined 'PC' must be too"
#endif
#endif // WCLI

#ifdef PS
#ifndef GRAPH
#error "If 'PS' is defined 'GRAPH' must be too"
#endif
#endif // PS

#ifdef META
#ifndef GRAPH
#error "If 'META' is defined 'GRAPH' must be too"
#endif
#endif // META

#ifdef WIRE
#ifndef GRAPH
#error "If 'WIRE' is defined 'GRAPH' must be too"
#endif
#endif // WIRE


/*
******************************************************************************
** Program Constants.
******************************************************************************
*/

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif
#define fFalse FALSE
#define fTrue  TRUE

#define szAppNameCore "Astrolog"
#define szVersionCore "7.30"
#define szVerCore     "730"
#define szDateCore    "September 2021"
#define szAddressCore \
  "Astara@msn.com - http://www.astrolog.org/astrolog.htm"
#define szNowCore     "now"
#define szTtyCore     "tty"
#define szSetCore     "set"
#define szNulCore     "nul"
#define szObjUnknown  "???"

#define cchSzDef   80
#define cchSzMax   255
#define cchSzLine  (cchSzMax*4)
#define dwCanary   0x87654321
#define nDegMax    360
#define nDegHalf   180
#define nMillion   1000000
#define nLarge     9999999
#define nNegative  -1000
#define yeaJ2G     1582
#define monJ2G     mOct
#define dayJ2G1    4
#define dayJ2G2    15
#define zonLMT     24.0
#define dstAuto    24.0
#define rStarLite  -1.46
#define rStarSpan  7.0
#define rStarNot   999.99

#define rSqr2      1.41421356237309504880
#define rSqr3      1.73205080756887729353
#define rPhi       1.61803398874989484820
#define rLog10     2.30258509299404568402
#define rLog101    4.61512051684125945088
#define rPi        3.14159265358979323846
#define rPi2       (rPi*2.0)
#define rPiHalf    (rPi/2.0)
#define rDegMax    360.0
#define rDegHalf   180.0
#define rDegQuad   90.0
#define rDegRad    (rDegHalf/rPi)
#define rMiToKm    1.609344
#define rFtToM     0.3048
#define rInToCm    2.54
#define rAUToKm    149597870.7
#define rLYToAU    63241.07708427
#define rPCToAU    206264.8062471
#define rDayInYear 365.24219
#define rEarthDist 149.59787
#define rEpoch2000 -24.736467
#define rJD2000    2451545.0
#define rAxis      23.44578889
#define rSatRingA  136780.0
#define rSatRingB  92000.0
#define rUraRing   51149.0
#define rSmall     (1.7453E-09)
#define rLarge     10000.0
#define rInvalid   (1.23456789E-09)
#define rRound     0.5

#define chNull     '\0'
#define chEscape   '\33'
#define chBell     '\7'
#define chReturn   '\r'
#define chTab      '\t'
#define chDelete   '\b'
#define chBreak    '\3'
#define chDeg2     '\260'
#define chRet      'R'
#define chRet2     'r'
#define chSep      ','
#define chSep2     ';'

// Array index limits

#define objMax     (cObj+1)
#define cCnstl     88
#define cZone      72
#define cSector    36
#define cPart      177
#define cWeek      7
#define cColor     16
#define cRainbow   7
#define cRay       7
#define cRing      6
#define xFont      6
#define yFont      10
#define xFont2     (xFont >> 1)
#define yFont2     (yFont >> 1)
#define xFontT     (xFont2 * gi.nScaleTextT2)
#define yFontT     (yFont2 * gi.nScaleTextT2)
#define xSideT     (SIDESIZE * gi.nScaleTextT2 >> 1)

// Atlas values

#define cchSzAtl 54
#define cchSzZon 13
#define icnewMax 252
#define icnusMax 51
#define icncaMax 13
#define icnUS    234  // United States
#define icnCA    38   // Canada
#define icnFR    76   // France
#define iznMax   425
#define ilistMax 200

// Object array index values

#define cPlanet oVes
#define cThing  oLil
#define oMain   10
#define oCore   21
#define cUran   9
#define cDwarf  9
#define cMoons  27
#define cCOB    5
#define cMoons2 (cMoons + cCOB)
#define cCust   (cUran + cDwarf + cMoons2)
#define cStar   47
#define cuspLo  (oCore+1)
#define cuspHi  (cuspLo+cSign-1)
#define uranLo  (cuspHi+1)
#define uranHi  (uranLo+cUran-1)
#define dwarfLo (uranHi+1)
#define dwarfHi (dwarfLo+cDwarf-1)
#define moonsLo (dwarfHi+1)
#define moonsHi (moonsLo+cMoons-1)
#define cobLo   (moonsHi+1)
#define cobHi   (cobLo+cCOB-1)
#define starLo  (cobHi+1)
#define starHi  (starLo+cStar-1)
#define custLo  uranLo
#define custHi  cobHi
#define oNorm   cobHi
#define oNorm1  starLo

// Month index values

enum _months {
  mJan = 1,
  mFeb = 2,
  mMar = 3,
  mApr = 4,
  mMay = 5,
  mJun = 6,
  mJul = 7,
  mAug = 8,
  mSep = 9,
  mOct = 10,
  mNov = 11,
  mDec = 12,
};

// Elements

enum _elements {
  eFir = 0,
  eEar = 1,
  eAir = 2,
  eWat = 3,
  cElem = 4,
};

// Zodiac signs

enum _signs {
  sAri = 1,
  sTau = 2,
  sGem = 3,
  sCan = 4,
  sLeo = 5,
  sVir = 6,
  sLib = 7,
  sSco = 8,
  sSag = 9,
  sCap = 10,
  sAqu = 11,
  sPis = 12,
  cSign = 12,
};

// Objects

enum _objects {
  oEar = 0,
  oSun = 1,
  oMoo = 2,
  oMer = 3,
  oVen = 4,
  oMar = 5,
  oJup = 6,
  oSat = 7,
  oUra = 8,
  oNep = 9,
  oPlu = 10,
  oChi = 11,
  oCer = 12,
  oPal = 13,
  oJun = 14,
  oVes = 15,
  oNod = 16,
  oSou = 17,
  oLil = 18,
  oFor = 19,
  oVtx = 20,
  oEP  = 21,
  oAsc = (cuspLo-1 + 1),
  o2nd = (cuspLo-1 + 2),
  o3rd = (cuspLo-1 + 3),
  oNad = (cuspLo-1 + 4),
  o5th = (cuspLo-1 + 5),
  o6th = (cuspLo-1 + 6),
  oDes = (cuspLo-1 + 7),
  o8th = (cuspLo-1 + 8),
  o9th = (cuspLo-1 + 9),
  oMC  = (cuspLo-1 + 10),
  o11h = (cuspLo-1 + 11),
  o12h = (cuspLo-1 + 12),
  oVul = (uranLo + 0),
  oVlk = (uranLo + 7),
  oHyg = (dwarfLo + 0),
  oPho = (dwarfLo + 1),
  oEri = (dwarfLo + 2),
  oHau = (dwarfLo + 3),
  oMak = (dwarfLo + 4),
  oGon = (dwarfLo + 5),
  oQua = (dwarfLo + 6),
  oSed = (dwarfLo + 7),
  oOrc = (dwarfLo + 8),
  oUrT = (moonsLo + 14),
  oJuC = (cobLo + 0),
  oSaC = (cobLo + 1),
  oUrC = (cobLo + 2),
  oNeC = (cobLo + 3),
  oPlC = (cobLo + 4),
  oOri = (starLo-1 + 10),
  oAlr = (starLo-1 + 45),
  oAnd = (starLo-1 + 47),
  cObj = 130,
};

// Aspects

enum _aspects {
  aLen = -6,  // Direction change (distance)
  aAlt = -5,  // Direction change (latitude)
  aHou = -4,  // 3D House change
  aDeg = -3,  // Degree change
  aDir = -2,  // Direction change (longitude)
  aSig = -1,  // Sign change
  aCon = 1,
  aOpp = 2,
  aSqu = 3,
  aTri = 4,
  aSex = 5,
  aInc = 6,
  aSSx = 7,
  aSSq = 8,
  aSes = 9,
  aQui = 10,
  aBQn = 11,
  aSQn = 12,
  aSep = 13,
  aNov = 14,
  aBNv = 15,
  aBSp = 16,
  aTSp = 17,
  aQNv = 18,
  cAspect = 18,
  cAspect2 = cAspect + aOpp,
};

// House systems

enum _housesystem {
  hsPlacidus      = 0,
  hsKoch          = 1,
  hsEqual         = 2,
  hsCampanus      = 3,
  hsMeridian      = 4,
  hsRegiomontanus = 5,
  hsPorphyry      = 6,
  hsMorinus       = 7,
  hsTopocentric   = 8,
  hsAlcabitius    = 9,
  hsKrusinski     = 10,
  hsEqualMC       = 11,
  hsSinewaveRatio = 12,
  hsSinewaveDelta = 13,
  hsWhole         = 14,
  hsVedic         = 15,
  hsSripati       = 16,
  hsHorizon       = 17,
  hsAPC           = 18,
  hsCarter        = 19,
  hsSunshine      = 20,
  hsSavardA       = 21,
  hsNull          = 22,
  // New experimental house systems follow:
  hsWholeMC       = 23,
  hsVedicMC       = 24,
  hsEqualBalanced = 25,
  hsWholeBalanced = 26,
  hsVedicBalanced = 27,
  hsEqualEP       = 28,
  hsWholeEP       = 29,
  hsVedicEP       = 30,
  hsEqualVertex   = 31,
  hsWholeVertex   = 32,
  hsVedicVertex   = 33,
  cSystem = 34,
};

// House system models

enum _housemodel {
  hmEcliptic = 0,   // Standard 2D: Equator of houses is ecliptic
  hmPrime    = 1,   // 3D: Equator of houses is prime vertical
  hmHorizon  = 2,   // 3D: Equator of houses is local horizon
  hmEquator  = 3,   // 3D: Equator of houses is celestial equator
  cMethod = 3,
};

// Progressed chart types

enum _progressiontype {
  ptCast     = 0,
  ptSolarArc = 1,
  ptMixed    = 2,
};

// Biorhythm cycle constants

#define brPhy 23.0
#define brEmo 28.0
#define brInt 33.0

// Relationship chart modes

enum _relationshipchart {
  rcNone       = 0,
  rcSynastry   = 1,
  rcComposite  = 2,
  rcMidpoint   = 3,
  rcDifference = 4,
  rcBiorhythm  = 5,
  rcDual       = -1,
  rcTriWheel   = -2,
  rcQuadWheel  = -3,
  rcQuinWheel  = -4,
  rcHexaWheel  = -5,
  rcTransit    = -6,
  rcProgress   = -7,
};

// Aspect configurations

enum _aspectconfigurations {
  acS3 = 0,  // Stellium (3 planets)
  acGT = 1,  // Grand Trine
  acTS = 2,  // T-Square
  acY  = 3,  // Yod
  acGC = 4,  // Grand Cross
  acC  = 5,  // Cradle
  acMR = 6,  // Mystic Rectangle
  acS4 = 7,  // Stellium (4 planets)
  cAspConfig = 8,
};

// Aspect sorting methods

enum _aspectsorting {
  asj = 0,  // By power
  aso = 1,  // By orb (+/- merged)
  asn = 2,  // By orb (+/- separated)
  asO = 3,  // By 1st object name
  asP = 4,  // By 2nd object name
  asA = 5,  // By aspect
  asC = 6,  // By 1st object position
  asD = 7,  // By 2nd object position
  asM = 8,  // By midpoint
};

// Angle restrictions

enum _angles {
  arAsc = 0,  // Ascendant
  arMC  = 1,  // Midheaven
  arDes = 2,  // Descendant
  arIC  = 3,  // Nadir
  arMax = 4,
};

// Eclipse types

enum _eclipses {
  etUndefined = -1, // Not checked
  etNone      = 0,  // No eclipse
  etPenumbra  = 1,  // Penumbral eclipse
  etPenumbra2 = 2,  // Total penumbral eclipse
  etPartial   = 3,  // Partial eclipse
  etAnnular   = 4,  // Annular eclipse
  etTotal     = 5,  // Total eclipse
  etMax       = 6,
};

// Rulership restrictions

enum _rulerships {
  rrStd = 0,  // Standard exoteric
  rrEso = 1,  // Esoteric
  rrHie = 2,  // Hierarchical
  rrExa = 3,  // Exaltation
  rrRay = 4,  // Ray rulership
  rrMax = 5,
};

// Graphics chart modes

enum _graphicschart {
  gWheel      = 1,
  gHouse      = 2,
  gGrid       = 3,
  gHorizon    = 4,
  gOrbit      = 5,
  gSector     = 6,
  gCalendar   = 7,
  gDisposit   = 8,
  gEsoteric   = 9,
  gAstroGraph = 10,
  gEphemeris  = 11,
  gRising     = 12,
  gLocal      = 13,
  gTraTraGra  = 14,
  gTraNatGra  = 15,
  gSphere     = 16,
  gWorldMap   = 17,
  gGlobe      = 18,
  gPolar      = 19,
  gTelescope  = 20,
  gBiorhythm  = 21,
#ifdef WIN
  gAspect     = 22,
  gMidpoint   = 23,
  gArabic     = 24,
  gMoons      = 25,
  gTraTraTim  = 26,
  gTraTraInf  = 27,
  gTraNatTim  = 28,
  gTraNatInf  = 29,
  gSign       = 30,
  gObject     = 31,
  gHelpAsp    = 32,
  gConstel    = 33,
  gPlanet     = 34,
  gRay        = 35,
  gMeaning    = 36,
  gSwitch     = 37,
  gObscure    = 38,
  gKeystroke  = 39,
  gCredit     = 40,
  gMax        = 41,
#endif
};

// Colors

enum _colors {
  kReverse = -2,
  kDefault = -1,
  kBlack   = 0,
  kMaroon  = 1,
  kDkGreen = 2,
  kOrange  = 3,
  kDkBlue  = 4,
  kPurple  = 5,
  kDkCyan  = 6,
  kLtGray  = 7,
  kDkGray  = 8,
  kRed     = 9,
  kGreen   = 10,
  kYellow  = 11,
  kBlue    = 12,
  kMagenta = 13,
  kCyan    = 14,
  kWhite   = 15,
  kElement = 16,
  kRay     = 17,
  kStar    = 18,
  kPlanet  = 19,
  kNull    = 16,
};

// Arabic parts

enum _arabicparts {
  apFor = 0,
  apSpi = 1,
};

// Calculation methods

enum _calculationmethod {
  cmSwiss   = 0,
  cmMoshier = 1,
  cmJPL     = 2,
  cmPlacalc = 3,
  cmMatrix  = 4,
  cmJPLWeb  = 5,
  cmNone    = 6,
  cmMax     = 7,
};

// Draw text formatting flags

enum _drawtext {
  dtCent   = 0x0,   // Default: Center text at coordinates
  dtLeft   = 0x1,   // Left justify text at X coordinate
  dtTop    = 0x2,   // Y coordinate is top of text
  dtBottom = 0x4,   // Y coordinate is bottom of text
  dtErase  = 0x8,   // Erase background behind text
  dtScale  = 0x10,  // Scale text by -Xs character scale
  dtScale2 = 0x20,  // Scale text by -XS text scale
};

// User string parse modes

enum _parsemode {
  pmMon    = 1,
  pmDay    = 2,
  pmYea    = 3,
  pmTim    = 4,
  pmDst    = 5,
  pmZon    = 6,
  pmLon    = 7,
  pmLat    = 8,
  pmDist   = 9,
  pmElv    = 10,
  pmLength = 11,
  pmObject = 12,
  pmAspect = 13,
  pmSystem = 14,
  pmSign   = 15,
  pmColor  = 16,
  pmRGB    = 17,
  pmWeek   = 18,
};

// File types

enum _filetype {
  ftNone = 0,
  ftBmp  = 1,
  ftPS   = 2,
  ftWmf  = 3,
  ftWire = 4,
};

// Font index

enum _fontindex {
  fiAstrolog = 0,
  fiWingding = 1,
  fiAstro    = 2,
  fiEnigma   = 3,
  fiHamburg  = 4,
  fiAstronom = 5,
  fiCourier  = 6,
  fiConsolas = 7,
  fiArial    = 8,
  cFont = 9,
};

// Letters

enum _letter {
  iLetterQ = 17,
  iLetterR = 18,
  iLetterS = 19,
  iLetterT = 20,
  iLetterU = 21,
  iLetterV = 22,
  iLetterW = 23,
  iLetterX = 24,
  iLetterY = 25,
  iLetterZ = 26,
  cLetter = 26,
};

// Termination codes

enum _terminationcode {
  tcError = -1,
  tcOK    = 0,
  tcFatal = 1,
  tcForce = 2,
};


/*
******************************************************************************
** Macro Functions.
******************************************************************************
*/

#define BLo(w) ((byte)(w))
#define BHi(w) ((byte)((word)(w) >> 8 & 0xFF))
#define WLo(l) ((word)(dword)(l))
#define WHi(l) ((word)((dword)(l) >> 16 & 0xFFFF))
#define WFromBB(bLo, bHi) ((word)BLo(bLo) | (word)((byte)(bHi)) << 8)
#define LFromWW(wLo, wHi) ((dword)WLo(wLo) | (dword)((word)(wHi)) << 16)
#define LFromBB(b1, b2, b3, b4) LFromWW(WFromBB(b1, b2), WFromBB(b3, b4))
#define Rgb(bR, bG, bB) \
  (((dword)(bR)) | ((dword)(bG)<<8) | ((dword)(bB)<<16))
#define RgbR(l) BLo(l)
#define RgbG(l) BHi(l)
#define RgbB(l) ((byte)((dword)(l) >> 16 & 0xFF))
#define ChHex(n) (char)((n) < 10 ? '0' + (n) : 'a' + (n) - 10)

#define Max(v1, v2) ((v1) > (v2) ? (v1) : (v2))
#define Min(v1, v2) ((v1) < (v2) ? (v1) : (v2))
#define NSgn(n) ((n) < 0 ? -1 : (n) > 0)
#define NSgn2(n) ((n) < 0 ? -1 : 1)
#define RSgn2(r) ((r) < 0.0 ? -1.0 : 1.0)
#define FOdd(n) ((n) & 1)
#define FBetween(v, v1, v2) ((v) >= (v1) && (v) <= (v2))
#define FSameR(r1, r2) (RAbs((r1) - (r2)) < rSmall)
#define FNearR(r1, r2) (RAbs((r1) - (r2)) < 0.001)
#define ChCap(ch) (FUncapCh(ch) ? (ch) - 'a' + 'A' : (ch))
#define ChUncap(ch) (FCapCh(ch) ? (ch) - 'A' + 'a' : (ch))
#define FCapCh(ch) FBetween(ch, 'A', 'Z')
#define FUncapCh(ch) FBetween(ch, 'a', 'z')
#define FNumCh(ch) FBetween(ch, '0', '9')
#define NHex(ch) ((int)((ch) <= '9' ? (ch) - '0' : (ch) - 'a' + 10) & 15)
#define NHex2(ch) ((int)((ch) <= '9' ? (ch) - '0' : (ch) - 'A' + 10) & 15)
#define FSzSet(sz) ((sz) != NULL && *(sz) != chNull)
#define NMultDiv(n1, n2, n3) ((int)((long)(n1) * (n2) / (n3)))
#define Ratio(v1, v2, v3) ((v1) + ((v2) - (v1)) * (v3))
#define ZFromS(s) ((real)(((s)-1)*30))
#define SFromZ(r) (((int)(r))/30+1)
#define RFromD(r) ((r)/rDegRad)
#define DFromR(r) ((r)*rDegRad)
#define GFromO(o) ((rDegMax - (o))/10.0)
#define RAbs(r) fabs(r)
#define NAbs(n) abs(n)
#define RFloor(r) floor(r)
#define NFloor(r) ((int)RFloor(r))
#define RFract(r) ((r) - RFloor(r))
#define Sq(n) ((n)*(n))
#define RSqr(r) sqrt(r)
#define RLog(r) log(r)
#define RLog10(r) (RLog(r) / rLog10)
#define RSin(r) sin(r)
#define RCos(r) cos(r)
#define RTan(r) tan(r)
#define RAsin(r) asin(r)
#define RAcos(r) acos(r)
#define RAtn(r) atan(r)
#define RSinD(r) RSin(RFromD(r))
#define RCosD(r) RCos(RFromD(r))
#define RTanD(r) RTan(RFromD(r))
#define RAsinD(r) DFromR(RAsin(r))
#define RAcosD(r) DFromR(RAcos(r))
#define RAtnD(r) DFromR(RAtn(r))
#define NSinD(nR, nD) ((int)((real)(nR)*RSinD((real)nD)))
#define NCosD(nR, nD) ((int)((real)(nR)*RCosD((real)nD)))
#define RAngleD(x, y) DFromR(RAngle(x, y))
#define VAngleD(v1, v2) DFromR(VAngle(v1, v2))
#define RLength2(x, y) RSqr(Sq(x) + Sq(y))
#define RLength3(x, y, z) RSqr(Sq(x) + Sq(y) + Sq(z))
#define RStarBright(b1, d1, d2) \
  ((d1) == 0.0 ? (b1) : ((b1) + 5.0*RLog10((d2) / (d1))))

#define FItem(obj)    FBetween(obj, 0, cObj)
#define FNorm(obj)    FBetween(obj, 0, oNorm)
#define FCusp(obj)    FBetween(obj, cuspLo, cuspHi)
#define FAngle(obj)   (FCusp(obj) && ((obj)-cuspLo)%3 == 0)
#define FMinor(obj)   (FCusp(obj) && ((obj)-cuspLo)%3 != 0)
#define FUranian(obj) FBetween(obj, uranLo, uranHi)
#define FDwarf(obj)   FBetween(obj, dwarfLo, dwarfHi)
#define FMoons(obj)   FBetween(obj, moonsLo, moonsHi)
#define FCob(obj)     FBetween(obj, cobLo, cobHi)
#define FCust(obj)    FBetween(obj, custLo, custHi)
#define FStar(obj)    FBetween(obj, starLo, starHi)
#define FObject(obj)  ((obj) <= cPlanet || (obj) >= uranLo)
#define FThing(obj)   ((obj) <= cThing || (obj) >= uranLo)
#define FHelio(obj)   (FNorm(obj) && FObject(obj) && !FGeo(obj))
#define FNodal(obj)   FBetween(obj, oNod, oLil)
#define FGeo(obj)     ((obj) == oMoo || FNodal(obj))
#define FAspect(asp)  FBetween(asp, 1, cAspect)
#define FAspect2(asp) FBetween(asp, 1, cAspect2)
#define FSector(s)    FBetween(s, 1, cSector)
#define ChDashF(f)    (f ? '=' : '_')
#define SzNumF(f)     (f ? "1 " : "0 ")
#define ChDst(dst)    (dst == 0.0 ? 'S' : (dst == 1.0 ? 'D' : \
  (dst != dstAuto ? 'A' : (is.fDst ? 'D' : 'S'))))
#define DayInYear(yea) (365-28+DayInMonth(2, yea))
#define FChSwitch(ch) ((ch) == '-' || (ch) == '/' || (ch) == '_' || \
  (ch) == '=' || (ch) == ':')

#define FValidMon(mon) FBetween(mon, 1, cSign)
#define FValidDay(day, mon, yea) ((day) >= 1 && (day) <= DayInMonth(mon, yea))
#define FValidYea(yea) FBetween(yea, -nLarge, nLarge)
#define FValidTim(tim) ((tim) > -2.0 && (tim) < 24.0)
#define FValidDst(dst) FValidZon(dst)
#define FValidZon(zon) FBetween(zon, -24.0, 24.0)
#define FValidLon(lon) FBetween(lon, -rDegHalf, rDegHalf)
#define FValidLat(lat) FBetween(lat, -rDegQuad, rDegQuad)
#define FValidWeek(day) FBetween(day, 0, cWeek-1)
#define FValidObj(obj) FBetween(obj, 0, cObj)
#define FValidSign(n) FBetween(n, 1, cSign)
#define FValidAspect(asp) FBetween(asp, 0, cAspect)
#define FValidSystem(n) FBetween(n, 0, cSystem-1)
#define FValidMethod(n) FBetween(n, 0, cMethod)
#define FValidDivision(n) FBetween(n, 1, 2880)
#define FValidOffset(r) FBetween(r, -rDegMax, rDegMax)
#define FValidCenter(obj) (FBetween(obj, oEar, cObj) && FThing(obj))
#define FValidHarmonic(r) FBetween(r, -10000000.0, 10000000.0)
#define FValidWheel(n) FBetween(n, 0, WHEELROWS)
#define FValidAstrograph(n) ((n) > 0 && (n) < 90)
#define FValidAstrograph2(n) FBetween(n, 0, 40000)
#define FValidPart(n) FBetween(n, 1, cPart)
#define FValidDwad(n) FBetween(n, 0, 12)
#define FValidBioday(n) FBetween(n, 1, 199)
#define FValidScreen(n) FBetween(n, 20, 200)
#define FValidMacro(n) FBetween(n, 1, 48)
#define FValidGlyphs(n) FBetween(n, 0, 2232)
#define FValidDecaType(n) FBetween(n, 0, 2)
#define FValidDecaSize(n) FBetween(n, 0, 100)
#define FValidDecaLine(n) FBetween(n, 1, 1000)
#define FValidGrid(n) FBetween(n, 0, cObj)
#define FValidEsoteric(n) FBetween(n, 1, 32000)
#define FValidScale(n) (FBetween(n, 100, MAXSCALE) && (n)%100 == 0)
#define FValidScaleText(n) (FBetween(n, 100, MAXSCALE) && (n)%50 == 0)
#define FValidBackPct(r) FBetween(r, 0.0, 100.0)
#define FValidBackOrient(n) FBetween(n, -1, 1)
#define FValidGraphX(x) (FBetween(x, BITMAPX1, BITMAPX) || (x) == 0)
#define FValidGraphY(y) (FBetween(y, BITMAPY1, BITMAPY) || (y) == 0)
#define FValidRotation(n) FBetween(n, 0, rDegMax-rSmall)
#define FValidTilt(n) FBetween(n, -rDegQuad, rDegQuad)
#define FValidColor(n) FBetween(n, 0, cColor-1)
#define FValidColor2(n) FBetween(n, 0, cColor-1 + 2)
#define FValidColorS(n) (FValidColor(n) || (n) == kStar)
#define FValidColorM(n) (FValidColor(n) || (n) == kPlanet)
#define FValidBmpmode(ch) \
  ((ch) == 'N' || (ch) == 'C' || (ch) == 'V' || (ch) == 'A' || (ch) == 'B')
#define FValidTimer(n) FBetween(n, 1, 32000)
#define FValidTelescope(obj) (FItem(obj) || (obj) == -1)

#define DM(d, m) ((d) + (m)/60.0)
#define DMS(d, m, s) (DM(d, m) + (s)/3600.0)
#define ZD(z, d) ((real)(((z)-1)*30) + (d))
#define ZDMS(z, d, m, s) ((real)(((z)-1)*30) + DMS(d, m, s))
#define HM(h, m) ((h) + (m)/60.0)
#define HMS(h, m, s) (HM(h, m) + (s)/3600.0)

#define kSignA(s) kObjA[cuspLo-1+(s)]
#define kSignB(s) kObjB[cuspLo-1+(s)]
#define kModeA(m) kElemA[(m) <= 1 ? (m) : eWat]
#define kModeB(m) kElemB[(m) <= 1 ? (m) : eWat]
#define FInterpretObj(obj) ((obj) <= oNorm && szMindPart[obj][0])
#define FInterpretAsp(asp) ((asp) > 0 && szInteract[asp][0])
#define szPerson  (ciMain.nam[0] ? ciMain.nam : "This person")
#define szPerson0 (ciMain.nam[0] ? ciMain.nam : "the person")
#define szPerson1 (ciMain.nam[0] ? ciMain.nam : "Person1")
#define szPerson2 (ciTwin.nam[0] ? ciTwin.nam : "Person2")
#define FIgnore(i) ignore[i]
#define FIgnore2(i) ignore2[i]
#define FAllow(i) (us.objRequire < 0 || (i) == us.objRequire)
#define FNoTimeOrSpace(ci) ((ci).mon == -1)

#define loop for (;;)
#define inv(v) v = !(v)
#define neg(v) v = -(v)
#define PrintL() PrintCh('\n')
#define PrintL2() PrintSz("\n\n")
#define PrintF(sz) fprintf(file, "%s", sz)
#define PrintFSz() PrintF(sz)
#define SwapN(n1, n2) (n1)^=(n2)^=(n1)^=(n2)
#define FSwitchF(f) ((((f) || fOr) && !fAnd) != fNot)
#define FSwitchF2(f) (((f) || (fOr || fNot)) && !fAnd)
#define SwitchF(f) f = FSwitchF(f)
#define SwitchF2(f) f = FSwitchF2(f)
#define SetCI(ci, M, D, Y, T, S, Z, O, A) \
  ci.mon = M; ci.day = D; ci.yea = Y; \
  ci.tim = T; ci.dst = S; ci.zon = Z; ci.lon = O; ci.lat = A

#define CONST const
#define PAllocateCore(cb) malloc(cb)
#define DeallocatePCore(p) free(p)
#ifndef PC
#define chDirSep '/'
#define chSwitch '-'
#ifndef MACOLD
#define ldTime 2440588L
#else
#define ldTime 2416481L
#endif
#else // PC
#define chDirSep '\\'
#define chSwitch '/'
#define ldTime 2440588L
#endif // PC

#ifdef GRAPH
#ifdef WINANY
#define API FAR PASCAL
#define hdcNil ((HDC)NULL)
#define SetWindowOrg(hdc, x, y) SetWindowOrgEx(hdc, x, y, NULL)
#define SetWindowExt(hdc, x, y) SetWindowExtEx(hdc, x, y, NULL)
#define SetViewportOrg(hdc, x, y) SetViewportOrgEx(hdc, x, y, NULL)
#define SetViewportExt(hdc, x, y) SetViewportExtEx(hdc, x, y, NULL)
#define SetWindowPosition(xo, yo, xs, ys) SetWindowPos(wi.hwnd, \
  wi.fWindowFull ? GetTopWindow(NULL) : HWND_NOTOPMOST, xo, yo, xs, ys, 0)
#define MoveTo(hdc, x, y) MoveToEx(hdc, x, y, NULL)
#endif

// Should an object in the outer wheel be restricted?
#define FProper2(i) (!(us.nRel == rcTransit ? ignore2[i] : ignore[i]))

// Are particular coordinates on the chart?
#define FInRect(x, y, x1, y1, x2, y2) \
  ((x) >= (x1) && (x) < (x2) && (y) >= (y1) && (y) < (y2))
#define FOnWin(X, Y) FInRect((X), (Y), 0, 0, gs.xWin, gs.yWin)

// Get a coordinate based on chart radius, a fraction, and (co)sin value.
#define POINT0(U, R, S) ((int)(RFloor((U)*(R)+rRound)*(S)))
#define POINT1(U, R, S) ((int)((RFloor((U)*(R)+rRound)+0.25)*(S)))
#define POINT2(U, R, S) ((int)((RFloor((U)*(R)+rRound)-0.25)*(S)))

// Determine (co)sine factors based on zodiac angle and chart orientation.
#define PX(A) RCosD(A)
#define PY(A) RSinD(A)
#define PZ(A) PlaceInX(A)

// Compute Mollewide projection in pixel scale given latitude.
#define RMollewide(y) RSqr((real)Sq(180*nScl) - 4.0*Sq((y)*(real)nScl))
#define NMollewide(y) \
  ((int)(RSqr((real)(Sq(180*nScl) - 4*Sq((y)*nScl))) + rRound))

// Do settings indicate the current chart should have the info sidebar?
#define fSidebar ((gi.nMode == gWheel || gi.nMode == gHouse || \
  gi.nMode == gSector || gi.nMode == gSphere) && gs.fText && !us.fVelocity)

// Is the current chart most properly displayed as a square graphic?
#define fSquare \
  (gi.nMode == gWheel || gi.nMode == gHouse || gi.nMode == gGrid || \
  (gi.nMode == gHorizon && us.fPrimeVert) || gi.nMode == gDisposit || \
  gi.nMode == gOrbit || gi.nMode == gSector || gi.nMode == gSphere || \
  gi.nMode == gGlobe || gi.nMode == gPolar)

// Does the current chart have to be displayed in a map rectangle?
#define fMap \
  (gi.nMode == gAstroGraph || gi.nMode == gWorldMap)

// Do settings indicate the current chart should have an outer border?
#define fDrawBorder \
  ((gs.fBorder || gi.nMode == gGrid) && gi.nMode != gTraTraGra && \
  gi.nMode != gTraNatGra && gi.nMode != gGlobe && gi.nMode != gPolar && \
  (gi.nMode != gWorldMap || !gs.fMollewide))

// Do settings indicate current chart should have chart info at its bottom?
#define fDrawText \
  (gs.fText && gi.nMode != gCalendar && !((gi.nMode == gWorldMap || \
  gi.nMode == gGlobe || gi.nMode == gPolar) && (gs.fAlt || gs.fConstel)) && \
  !((gi.nMode == gWheel || gi.nMode == gHouse || gi.nMode == gSector || \
  gi.nMode == gSphere) && !us.fVelocity))
#endif // GRAPH


/*
******************************************************************************
** Type Definitions.
******************************************************************************
*/

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef long word4;
typedef double real;
typedef unsigned char uchar;
typedef unsigned int uint;
typedef int flag;
typedef byte * pbyte;

typedef int KI;
#ifdef GRAPH
typedef unsigned long KV;
#endif // GRAPH
typedef short * TRIE;

typedef struct _StrLook {
  char *sz;
  int isz;
} StrLook;

typedef struct _StrLookR {
  char *sz;
  real r;
} StrLookR;

typedef struct _PT2S {
  word x;
  word y;
} PT2S;

typedef struct _PT3R {
  real x;
  real y;
  real z;
} PT3R;

typedef struct _CIRC {
  int xc;  // Center coordinates of circle
  int yc;
  int xr;  // Radius of circle
  int yr;
} CIRC;

typedef struct _TELE {
  real xCent;   // Center pixel coordinates of chart
  real yCent;
  real xBase;   // Chart coordinates of center
  real yBase;
  real xScale;  // Scale of chart
  real yScale;
  real lon;     // Final coordinates used for point
  real lat;
} TELE;

typedef struct _GridInfo {
  byte n[objMax][objMax];
  int  v[objMax][objMax];
} GridInfo;

typedef struct _CrossInfo {
  real lat[MAXCROSS];
  real lon[MAXCROSS];
  int obj1[MAXCROSS];
  int obj2[MAXCROSS];
} CrossInfo;

typedef struct _InDayInfo {
  short source;  // First planet
  short aspect;  // The aspect first planet makes with second planet
  int dest;      // Larger type because may store fraction of degree
  char mon;      // Month of event
  char day;      // Day of event
  short yea;     // Year of event
  real time;     // Time of event in hours
  real pos1;     // Zodiac position of first planet
  real pos2;     // Zodiac position of second planet
  char ret1;     // Sign of first planet's velocity
  char ret2;     // Sign of second planet's velocity
} InDayInfo;

typedef struct _AtlasEntry {
  real lon;              // Longitude of city
  real lat;              // Latitude of city
  short icn;             // Country or region of city
  short istate;          // State or province of city, if US or CA
  char szNam[cchSzAtl];  // Name of city
  short izn;             // Time zone area of city
} AtlasEntry;

typedef struct _TimezoneChange {
  int zon;      // Time zone value (in seconds before UTC)
  int irun;     // Daylight Saving rule (if any)
  int dst;      // Daylight offset to always use (if no rule)
  short yea;    // Year time zone value ends
  char mon;     // Month time zone value ends
  char day;     // Day time zone value ends
  int tim;      // Time that time zone value ends (in seconds)
  int timtype;  // Type of time (0=local, 1=standard, 2=UTC)
} ZoneChange;

typedef struct _TimezoneRuleName {
  char szNam[cchSzZon];  // Name of rule
  int irue;              // Start index of rule entries
} RuleName;

typedef struct _TimezoneRuleEntry {
  short yea1;    // Start year rule applies to
  short yea2;    // End year rule applies to
  char mon;      // Month in each year rule takes place
  char daytype;  // Type of day (0=num, 1=lastDOW, 2=DOW>=num, 3=DOW<=num)
  char daynum;   // Day within month rule takes place
  char dayweek;  // Day of week (DOW) rule takes place
  int tim;       // Time within day rule takes place
  int timtype;   // Type of time (0=local, 1=standard, 2=UTC)
  int dst;       // Rule applies this Daylight offset (in seconds before UTC)
} RuleEntry;

#ifdef GRAPH
typedef struct _ObjDraw {
  int obj;  // The object to draw
  int x;    // Horizontal pixel position of object
  int y;    // Vertical pixel position of object
  int yg;   // Vertical pixel position of glyph
  KV kv;    // Color to use when drawing object
  flag f;   // Whether to draw object at all
} ObjDraw;
#endif

typedef word * TransGraInfo[objMax][objMax][cAspect+1];

typedef struct _UserSettings {

  // Chart types
  flag fListing;        // -v
  flag fWheel;          // -w
  flag fGrid;           // -g
  flag fAspList;        // -a
  flag fMidpoint;       // -m
  flag fHorizon;        // -Z
  flag fOrbit;          // -S
  flag fSector;         // -l
  flag fInfluence;      // -j
  flag fEsoteric;       // -7
  flag fAstroGraph;     // -L
  flag fCalendar;       // -K
  flag fInDay;          // -d
  flag fInDayInf;       // -D
  flag fEphemeris;      // -E
  flag fHorizonSearch;  // -Zd
  flag fTransit;        // -t
  flag fTransitInf;     // -T
  flag fInDayGra;       // -B
  flag fTransitGra;     // -V
  flag fAtlasLook;      // -N
  flag fAtlasNear;      // -Nl
  flag fZoneChange;     // -Nz
  flag fMoonChart;      // -8

  // Chart suboptions
  flag fVelocity;       // -v0
  flag fListDecan;      // -v3
  flag fWheelReverse;   // -w0
  flag fGridConfig;     // -g0
  flag fGridMidpoint;   // -gm
  flag fAppSep;         // -ga
  flag fParallel;       // -gp
  flag fAspSummary;     // -a0
  flag fMidSummary;     // -m0
  flag fMidAspect;      // -ma
  flag fPrimeVert;      // -Z0
  flag fSectorApprox;   // -l0
  flag fInfluenceSign;  // -j0
  flag fLatitudeCross;  // -L0
  flag fCalendarYear;   // -Ky
  flag fInDayMonth;     // -dm
  flag fInDayYear;      // -dy
  flag fGraphAll;       // -B0
  flag fArabicFlip;     // -P0

  // Table chart types
  flag fCredit;          // -Hc
  flag fSwitch;          // -H
  flag fSwitchRare;      // -Y
  flag fKeyGraph;        // -HX
  flag fSign;            // -HC
  flag fObject;          // -HO
  flag fAspect;          // -HA
  flag fConstel;         // -HF
  flag fOrbitData;       // -HS
  flag fRay;             // -H7
  flag fMeaning;         // -HI

  // Main flags
  flag fLoop;        // -Q
  flag fSidereal;    // -s
  flag fCusp;        // -C
  flag fUranian;     // -u
  flag fDwarf;       // -u0
  flag fMoons;       // -u8
  flag fCOB;         // -uc
  flag fProgress;    // Are we doing a -p progressed chart?
  flag fInterpret;   // Is -I interpretation switch in effect?
  flag fHouse3D;     // -c3
  flag fAspect3D;    // -A3
  flag fAspectLat;   // -Ap
  flag fParallel2;   // -AP
  flag fDecan;       // -3
  flag fFlip;        // -f
  flag fGeodetic;    // -G
  flag fVedic;       // -J
  flag fNavamsa;     // -9
  flag fEphemFiles;  // -b
  flag fWriteFile;   // -o
  flag fAnsiColor;   // -k
  flag fGraphics;    // -X

  // Main subflags
  flag fNoSwitches;
  flag fLoopInit;    // -Q0
  flag fSeconds;     // -b0
  flag fPlacalcAst;  // -ba
  flag fPlacalcPla;  // -bp
  flag fMatrixPla;   // -bm
  flag fMatrixStar;  // -bU
  flag fEquator;     // -sr
  flag fEquator2;    // -sr0
  flag fWritePos;    // -o0
  flag fWriteDef;    // -od
  flag fWriteAAF;    // -oa
  flag fAnsiChar;    // -k0
  flag fTextHTML;    // -kh
  flag fSolarWhole;  // -10

  // Obscure flags
  flag fTruePos;      // -YT
  flag fTopoPos;      // -YV
  flag fBarycenter;   // -Yh
  flag fMoonMove;     // -Ym
  flag fSidereal2;    // -Ys
  flag fTrueNode;     // -Yn
  flag fNoNutation;   // -Yn0
  flag fEuroDate;     // -Yd
  flag fEuroTime;     // -Yt
  flag fEuroDist;     // -Yv
  flag fRound;        // -Yr
  flag fSmartCusp;    // -YC
  flag fSmartSave;    // -YO
  flag fClip80;       // -Y8
  flag fWriteOld;     // -Yo
  flag fHouseAngle;   // -Yc
  flag fPolarAsc;     // -Yp
  flag fEclipse;      // -Yu
  flag fEclipseAny;   // -Yu0
  flag fObjRotWhole;  // -Y10
  flag fIgnoreSign;   // -YR0
  flag fIgnoreDir;    // -YR0
  flag fIgnoreDiralt; // -YR1
  flag fIgnoreDirlen; // -YR1
  flag fIgnoreAuto;   // -YRh
  flag fStarsList;    // -YRU0
  flag fStarMagDist;  // -YUb
  flag fStarMagAbs;   // -YUb0
  flag fNoWrite;      // -0o
  flag fNoRead;       // -0i
  flag fNoQuit;       // -0q
  flag fNoGraphics;   // -0X
  flag fNoNetwork;    // -0n
  flag fNoExp;        // -0~
  flag fExpOff;       // -~0

  // Value settings
  int   nAspectSort;   // -a
  int   nEphemYears;   // -Ey
  int   nEphemRate;    // -E0
  int   nEphemFactor;  // -E0
  int   nArabic;       // -P
  int   nRel;          // What relationship chart is in effect, if any?
  int   nSwissEph;     // -bs
  int   nHouseSystem;  // -c
  int   nHouse3D;      // -c3
  int   nAsp;          // -A
  int   objCenter;     // -h
  int   nDwad;         // -4
  int   nStar;         // -U
  real  rHarmonic;     // Harmonic chart value passed to -x switch.
  int   objOnAsc;      // Planet value passed to -1 or -2 switch.
  int   objRequire;    // Required object passed to -RO switch.
  int   dayDelta;      // -+, --
  int   nDegForm;      // -s
  int   nProgress;     // -p0
  int   nDivision;     // -d
  int   nScreenWidth;  // -I
  real  dstDef;        // -z0
  real  zonDef;        // -z
  real  lonDef;        // -zl
  real  latDef;        // -zl
  real  elvDef;        // -zv
  char *namDef;        // -zj
  char *locDef;        // -zj
  char *rgszPath[10];  // -Yi
  char *szAstColor;    // -YkE
  char *szStarsColor;  // -YkU
  char *szStarsList;   // -YRU

  // Value subsettings
  int   nWheelRows;       // Number of rows per house to use for -w wheel.
  int   nAstroGraphStep;  // Latitude step rate passed to -L switch.
  int   nAstroGraphDist;  // Maximum crossing distance passed to -L0 switch.
  int   nArabicParts;     // Arabic parts to include value passed to -P.
  int   nAtlasList;       // Number of rows to display value passed to -N.
  real  rZodiacOffset;    // Position shifting value passed to -s switch.
  real  rProgDay;         // Progression day value passed to -pd switch.
  real  rProgCusp;        // Progression cusp ratio value passed to -pC.
  int   nRatio1;          // Chart ratio factors passed to -rc or -rm.
  int   nRatio2;
  int   nScrollRow;       // -YQ
  int   cSequenceLine;    // -Yq
  long  lTimeAddition;    // -Yz
  real  rDeltaT;          // -Yz0
  real  rObjAddition;     // -YzO
  real  rCuspAddition;    // -YzC
  int   objRot1;          // -Y1
  int   objRot2;          // -Y1
  int   nArabicNight;     // -YP
  int   nBioday;          // -Yb
  int   nSignDiv;         // -YRd

  // AstroExpression hooks
  char *szExpConfig;      // -~g
  char *szExpAspList;     // -~a
  char *szExpAspSumm;     // -~a0
  char *szExpMid;         // -~m
  char *szExpMidAsp;      // -~ma
  char *szExpInf;         // -~j
  char *szExpInf0;        // -~j0
  char *szExpCross;       // -~L
  char *szExpEph;         // -~E
  char *szExpDay;         // -~d
  char *szExpVoid;        // -~dv
  char *szExpTra;         // -~t
  char *szExpPart;        // -~P
  char *szExpObj;         // -~O
  char *szExpHou;         // -~C
  char *szExpAsp;         // -~A
  char *szExpProg;        // -~p
  char *szExpProg0;       // -~p0
  char *szExpColObj;      // -~kO
  char *szExpColAsp;      // -~kA
  char *szExpColFill;     // -~kv
  char *szExpFontSig;     // -~F
  char *szExpFontHou;     // -~FC
  char *szExpFontObj;     // -~FO
  char *szExpFontAsp;     // -~FA
  char *szExpSort;        // -~v
  char *szExpDecan;       // -~v3
  char *szExpSidebar;     // -~Xt
  char *szExpStar;        // -~U
  char *szExpAst;         // -~U0
  char *szExpCast1;       // -~q1
  char *szExpCast2;       // -~q2
  char *szExpDisp1;       // -~Q1
  char *szExpDisp2;       // -~Q2
} US;

typedef struct _InternalSettings {
  flag fHaveInfo;      // Do we need to prompt user for chart info?
  flag fDst;           // Has Daylight Saving Time been autodetected?
  flag fProgress;      // Are we doing a chart involving progression?
  flag fReturn;        // Are we doing a transit chart for returns?
  flag fMult;          // Have we already printed at least one text chart?
  flag fSeconds;       // Do we print locations to nearest second?
  flag fSzPersist;     // Are parameter strings persistent when processing?
  flag fSzInteract;    // Are we in middle of chart so some setting fixed?
  flag fNoEphFile;     // Have we already had a ephem file not found error?
  flag fSwissPathSet;  // Has the Swiss Ephemeris path been set yet?
  char *szProgName;    // The name and path of the executable running.
  char *rgszLine[9];   // The command lines to run before each -Yq chart.
  char *szFileScreen;  // The file to send text output to as passed to -os.
  char *szFileOut;     // The output chart filename string as passed to -o.
  char **rgszComment;  // Points to any comment strings after -o filename.
  int nContext;        // Context of current or most recent chart cast.
  int nObj;            // Index of highest unrestricted object.
  int cszComment;      // The number of strings after -o that are comments.
  int cchRow;          // The current row text charts have scrolled to.
  int cchCol;          // The current column text charts are printing at.
  int cchColMax;       // Max column current text chart has printed at.
  int nHTML;           // HTML text output context mode for -kh.
  int nHouseSystem;    // Actual house system used to compute cusps for -c.
  int nWheelRows;      // Actual number of rows per house to use for -w.
  int cae;             // Number of atlas entries of city locations loaded.
  int czcn;            // Number of time zone change areas loaded.
  int czce;            // Total number of change entries in all zone areas.
  int crun;            // Number of time zone Daylight rule categories.
  int crue;            // Total number of rule entries in all categories.
  int cAlloc;          // Number of memory allocations currently allocated.
  int cAllocTotal;     // Total memory allocations allocated this session.
  int cbAllocSize;     // Total bytes in all memory allocations allocated.
  real rOff;           // Offset between sidereal and tropical zodiacs.
  real rSid;           // Sidereal offset degrees to be added to locations.
  real JD;             // Fractional Julian day for current chart.
  real JDp;            // Julian day that a progressed chart indicates.
  real Tp;             // Julian time used for progressed chart cusps.
  real lonMC;          // 0 longitude converted to equatorial coordinates.
  real latMC;          // 0 latitude converted to equatorial coordinates.
  TRIE rgsTrieFun;     // Trie tree of tokens for AstroExpression parsing.
  AtlasEntry *rgae;    // List of atlas entries for city coordinates.
  ZoneChange *rgzc;    // List of time zone change entries for zone areas.
  RuleName *rgrun;     // List of Daylight Saving change rule names.
  RuleEntry *rgrue;    // List of all Daylight Saving change rule entries.
  real *rgzonCol;      // Cache of time zone offsets for each zone area.
  FILE *fileIn;        // The switch file currently being read from.
  FILE *S;             // File to write text to.
  real T;              // Julian time for chart.
  real MC;             // Midheaven at chart time.
  real Asc;            // Ascendant at chart time.
  real EP;             // East Point at chart time.
  real Vtx;            // Vertex at chart time.
  real RA;             // Right ascension at time.
  real OB;             // Obliquity of ecliptic.
  real rDeltaT;        // Delta-T at chart time, in days.
  real jdDeltaT;       // JD for cached Delta-T offset above.
  real rNut;           // Nutation offset.
} IS;

typedef struct _ChartInfo {
  int mon;    // Month
  int day;    // Day
  int yea;    // Year
  real tim;   // Time in hours
  real dst;   // Daylight offset
  real zon;   // Time zone
  real lon;   // Longitude
  real lat;   // Latitude
  char *nam;  // Name for chart
  char *loc;  // Name of location
} CI;

typedef struct _ChartPositions {
  real obj[objMax];     // The zodiac positions.
  real alt[objMax];     // Ecliptic declination.
  real dir[objMax];     // Retrogradation velocity.
  real diralt[objMax];  // Latitude velocity.
  real dirlen[objMax];  // Distance velocity.
  real cusp[cSign+1];   // House cusp positions.
  real cusp3[cSign+1];  // 3D house cusp positions.
  int house[objMax];    // House each object is in.
} CP;

#ifdef SWISS
typedef struct _ExtraStar {
  real lon;          // Zodiac position.
  real lat;          // Vertical latitude.
  real dir;          // Velocity or speed.
  real mag;          // Magnitude or brightness.
  PT3R space;        // Coordinates in 3D space.
  char sz[cchSzDef]; // Buffer for name of star.
  char *pchNam;      // Star's classic name.
  char *pchDes;      // Star's designation name.
  char *pchBest;     // Best available name.
  KI ki;             // Color to use for star.
} ES;
#endif

#ifdef GRAPH
typedef struct _Bitmap {
  int x;      // Horizontal pixel size of bitmap
  int y;      // Vertical pixel size of bitmap
  int clRow;  // Longs per row in bitmap
  byte *rgb;  // Bytes of bitmap bits
} Bitmap;

typedef struct _GraphicsSettings {
  int ft;            // File type being created (-Xb, -Xp, -XM, or -X3).
  flag fPSComplete;  // Is PostScript file not encapsulated (-Xp0 set).
  flag fColor;       // Are we drawing a color chart (-Xm not set).
  flag fInverse;     // Are we drawing in reverse video (-Xr set).
  flag fRoot;        // Are we drawing on the X11 background (-XB set).
  flag fText;        // Are we printing chart info on chart (-Xt set).
  int nFont;         // Which fonts to use for sign/house/obj/asp (-YXf).
  flag fAlt;         // Are we drawing in alternate mode (-Xi set).
  flag fBorder;      // Are we drawing borders around charts (-Xu set).
  flag fLabel;       // Are we labeling objects in charts (-Xl not set).
  flag fLabelAsp;    // Are we drawing aspect glyphs on lines (-XA set).
  flag fLabelCity;   // Are we plotting cities on maps (-XL set).
  flag fJetTrail;    // Are we not clearing screen on updates (-Xj set).
  flag fConstel;     // Are we drawing maps as constellations (-XF set).
  flag fSouth;       // Are we focus on south hemisphere (-XX0/XP0 set).
  flag fMollewide;   // Are we drawing maps scaled correctly (-XW0 set).
  flag fEquator;     // Are we showing equator on maps/globes (-Xe set).
  flag fEcliptic;    // Are we drawing oriented to ecliptic (-YXe set).
  flag fAllStar;     // Are we drawing all sefstars.txt stars (-XU set).
  flag fHouseExtra;  // Are we showing additional house info (-XC set).
  flag fPrintMap;    // Are we printing globe names on draw (-XPv set).
  flag fKeepSquare;  // Are we preserving chart aspect ratio (-XQ set).
  flag fAnimMap;     // Are we animating map instead of time (-XN set).
  flag fThick;       // Are we drawing thicker lines in charts (-Xx set).
  int xWin;          // Current hor. size of graphic chart (-Xw).
  int yWin;          // Current ver. size of graphic chart (-Xw).
  int nAnim;         // Current animation mode jump rate (-Xn).
  int nScale;        // Current character scale factor (-Xs).
  int nScaleText;    // Current graphics text scale factor (-XS).
  int nAllStar;      // Extra star size and labelling (-XU).
  int nAstLo;        // Extra asteroid min ephemeris file (-XE).
  int nAstHi;        // Extra asteroid max ephemeris file (-XE).
  int nAstLabel;     // How extra asteroids get labeled (-XE).
  int nLabelCity;    // How city plottings get colored (-XL).
  int objLeft;       // Current object to place on Asc (-X1).
  real rRot;         // Current rotation degree of globe (-XG).
  real rTilt;        // Current vertical tilt of rotating globe (-XG).
  int objTrack;      // Object being telescope tracked, if any (-XZ).
  char chBmpMode;    // Current bitmap file type (-Xb).
  real rBackPct;     // Background image transparency percentage (-XI).
  int nBackOrient;   // Background image wallpaper orientation (-XI).
  int nOrient;       // PostScript paper orientation indicator.
  real xInch;        // PostScript horizontal paper size inches.
  real yInch;        // PostScript vertical paper size inches.
  char *szDisplay;   // Current X11 display name (-Xd).
  int nDecaType;     // Type of wheel chart decoration (-YXv).
  int nDecaSize;     // Size of wheel chart decoration (-YXv).
  int nDecaLine;     // Lines in wheel chart decoration (-YXv).
  int nDecaFill;     // Fill method for wheel chart sections (-Xv).
  char *szSidebar;   // Extra text to append to bottom of sidebar (-YXt)
  int nGridCell;     // Number of cells in -g grids (-YXg).
  real rspace;       // Radius in AU of -S orbit chart (-YXS).
  int cspace;        // Number of -S orbit trails allowed (-YXj).
  int zspace;        // Height diff of each orbit trail (-YXj0).
  int nRayWidth;     // Column width in -7 esoteric chart (-YX7).
  int nGlyphs;       // Settings for what gylphs to use (-YXG).
  flag fColorSign;   // More color for sign boundaries. (-YXk).
  flag fColorHouse;  // More color for house boundaries. (-YXk0).
  flag fAltPalette;  // Use alternate palette for white backgrounds (-YXK0).
  int nDashMax;      // Maximum dash allowed for lines (-YXA).
  int nTriangles;    // Triangles/cubes grid to draw on maps, if any (-YXW).
  char *szStarsLin;  // Names of extra stars for linking (-YXU).
  char *szStarsLnk;  // Indexes of star pairs to link up (-YXU).
} GS;

typedef struct _GraphicsInternal {
  int nMode;          // Current type of chart to create.
  flag fMono;         // Is this a monochrome display.
  int kiCur;          // Current color drawing with.
  pbyte bm;           // Pointer to allocated memory.
  int cbBmpRow;       // Horizontal size of bitmap array in memory.
  char *szFileOut;    // Current name of bitmap file (-Xo).
  FILE *file;         // Actual file handle writing graphics to.
  real rAsc;          // Degree to be at left edge in wheel charts.
  flag fFile;         // Are we making a graphics file.
  flag fDidSphere;    // Has a chart sphere been drawn once yet?
  real zViewRatio;    // Offset to adjust chart view (based on zoom).
  int nScale;         // Scale ratio, e.g. percentage / 100.
  int nScaleText;     // Text scale ratio, i.e. percentage / 50.
  int nScaleT;        // Internal units per pixel (1 for screen).
  int nScaleTextT;    // Internal text scale, i.e. nScaleText * nScaleT.
  int nScaleTextT2;   // Twice internal text scale, i.e. nScaleTextT * 2.
  int nGridCell;      // Actual number of cells in -g grids.
  int nPenWid;        // Pen width to use when creating metafiles.
  int nFontPrev;      // System fonts to restore when turning fonts on.
  KI kiOn;            // Foreground color.
  KI kiOff;           // Background color.
  KI kiLite;          // Hilight color.
  KI kiGray;          // A "dim" color.
  int xOffset;        // Viewport origin.
  int yOffset;
  int xTurtle;        // Current coordinates of drawing pen.
  int yTurtle;
  int xPen;           // Cached coordinates where last line ended.
  int yPen;
  PT3R *rgspace;      // List of orbit trail coordinates (-S -X).
  int ispace;         // Index of most recent coordinate (-S -X).
  int cspace;         // Coordinates within table so far (-S -X).
  int *rgzCalendar;   // Aspect coordinates in calendar (-K -X).
  flag fBmp;          // Are 24 bit bitmaps being used? (-Xbw set).
  Bitmap bmp;         // Bitmap storing chart contents, sized appropriately.
  Bitmap bmpBack;     // Bitmap storing background, as loaded from file.
  Bitmap bmpBack2;    // Bitmap storing background, at current transparency.
  Bitmap bmpWorld;    // Bitmap storing world map, as loaded from file.
  Bitmap bmpRising;   // Bitmap storing rising chart, as drawn within it.
#ifdef SWISS
  ES *rges;           // List of extra star coordinates (-YXU).
  int cStarsLin;      // Count of extra star coordinates (-YXU).
#endif
#ifdef X11
  Display *disp;      // The opened X11 display (-Xd).
  GC gc, pmgc;
  Pixmap pmap, icon;
  Window wind, root;
  int screen;
  int depth;          // Number of active color bits.
#endif
#ifdef PS             // Variables used by the PostScript generator.
  flag fEps;          // Are we doing Encapsulated PostScript.
  int cStroke;        // Number of items drawn without flushing.
  flag fLineCap;      // Are line ends rounded instead of square.
  int nDash;          // How much long are dashes in lines drawn.
  int nFontPS;        // What system font are we drawing text in.
  real rLineWid;      // How wide are lines, et al, drawn with.
#endif
#ifdef META           // Variables used by the metafile generator.
  word *pwMetaCur;    // Current mem position when making metafile.
  long cbMeta;        // Maximum size allowed for metafile.
  word *pwPoly;       // Position for start of current polyline.
  KI kiPoly;          // Line color for current polyline.
  KI kiLineAct;       // Desired and actual line color.
  KI kiLineDes;
  KI kiFillAct;       // Desired and actual fill color.
  KI kiFillDes;
  int nFontAct;       // Desired and actual text font.
  int nFontDes;
  KI kiTextAct;       // Desired and actual text color.
  KI kiTextDes;
  int nAlignAct;      // Desired/actual text alignment.
  int nAlignDes;
#endif
#ifdef WIRE           // Variables used by the wireframe generator.
  word *pwWireCur;    // Current memory position when doing wireframe.
  long cbWire;
  int cWire;          // Number of lines in wireframe file.
  KI kiInFile;        // Actual line color currently in file.
  int zDefault;       // Default elevation for 2D drawing.
#endif
#ifdef MACG
  WindowPtr wpAst;
  Rect rcDrag;
  Rect rcBounds;
#endif
} GI;
#endif // GRAPH

typedef struct _ArabicInfo {
  char *form;                 // The formula to calculate it.
  char *name;                 // The name of the Arabic part.
} AI;

typedef struct _ElementTable {
  int coSum;             // Total objects considered.
  int coHemi;            // Number that can be in hemispheres.
  int coSign[cSign];     // Number of objects in each sign.
  int coHouse[cSign];    // Number of objects in each house.
  int coElemMode[4][3];  // Objects in each elem/mode combo.
  int coElem[4];         // Objects in each element.
  int coMode[3];         // Objects in each sign mode.
  int coModeH[3];        // Objects in each house mode.
  int coYang;            // Objects in Fire/Air signs.
  int coYin;             // Objects in Earth/Water signs.
  int coLearn;           // Objects in first six signs.
  int coShare;           // Objects in last six signs.
  int coAsc;             // Objects in Eastern houses.
  int coDes;             // Objects in Western houses.
  int coMC;              // Objects in Southern houses.
  int coIC;              // Objects in Northern houses.
} ET;

typedef struct _OrbitalElements {
  real ma0, ma1, ma2;  // Mean anomaly.
  real ec0, ec1, ec2;  // Eccentricity.
  real sma;            // Semi-major axis.
  real ap0, ap1, ap2;  // Argument of perihelion.
  real an0, an1, an2;  // Ascending node.
  real in0, in1, in2;  // Inclination.
} OE;

#define szFileJPLCore  "astrolog.jpl"
#ifdef WIN
#define szFileTempCore "astrolog.tmp"
#define szFileAutoCore "astrolog.bmp"
#define nScrollDiv 24
#define nScrollPage 6
#define cchSzMaxFile 128

typedef struct _WindowInternal {
  HINSTANCE hinst;   // Instance of the Astrolog window class.
  HWND hwndMain;     // The outer created frame window.
  HWND hwnd;         // The current window being dealt with.
  HMENU hmenu;       // The Astrolog main menu bar.
  HACCEL haccel;     // Keyboard accelerator or shortcut table.
  HDC hdc;           // The current DC bring drawn upon.
  HDC hdcPrint;      // The current DC being printed upon.
  HDC hdcBack;       // The current DC for the background bitmap.
  HWND hwndAbort;    // Window of the printing abort dialog.
  HPEN hpen;         // Pen with the current line color.
  HBRUSH hbrush;     // Fill if any with the current color.
  HFONT hfont;       // Font of current text size being printed.
  HBITMAP hbmpBack;  // Bitmap for the background image.
  HBITMAP hbmpPrev;  // Bitmap to restore after using background.
  HANDLE hMutex;     // To ensure output file isn't already open.
  size_t lTimer;     // Identifier for the animation timer.
  short xScroll;     // Horizontal & vertical scrollbar position.
  short yScroll;
  short xClient;     // Horizontal & vertical window size.
  short yClient;
  int xChar;         // Horizontal & vertical font character size.
  int yChar;
  int xMouse;        // Horizontal & vertical mouse position.
  int yMouse;
  WORD wCmd;         // The currently invoked menu command.
  LPARAM lParamRC;   // Coordinates where right click originated.
  int nMode;         // New chart type to switch to if any.
  int cmdCur;        // Menu command for current chart type.
  flag fMenu;        // Do we need to repaint the menu bar?
  flag fMenuAll;     // Do we need to redetermine all menu checks?
  flag fRedraw;      // Do we need to redraw the screen?
  flag fCast;        // Do we need to recast the chart positions?
  flag fAbort;       // Did the user cancel printing in progress?
  flag fMoved;       // Has mouse been dragged since down click?
  flag fSkipSystem;  // If should suppress the next system command.
  int nDlgChart;     // Which chart to set in Open or Info dialog.
  flag fWindowFull;  // Is the window in full screen mode?
  RECT rcFull;       // Former window coordinates if full screen.
  flag fSaverExt;    // Is executable .scr screen saver extension?
  flag fSaverCfg;    // Is screen saver in configuration mode?
  Bitmap bmpWin;     // Bitmap storing contents to be copied to window.

  // Window user settings.
  flag fPause;         // Is animation paused?
  flag fBuffer;        // Are we drawing updates off screen?
  flag fHourglass;     // Bring up hourglass cursor on redraws?
  flag fChartWindow;   // Does chart change cause window resize?
  flag fWindowChart;   // Does window resize cause chart change?
  flag fNoUpdate;      // Do we not automatically update screen?
  flag fNoPopup;       // Do we not display warning messages?
  flag fSaverRun;      // Do we run Astrolog as a screen saver?
  flag fAutoSave;      // Are we saving chart after win draw?
  flag fAutoSaveNum;   // Autosave charts are incremental files?
  flag fAutoSaveWire;  // Autosave wireframe instead of bitmap?
  int nAutoSaveNum;    // Number of incremental bitmap save.
  KI kiPen;            // The current pen scribble color.
  int nDir;            // Animation step factor and direction.
  UINT nTimerDelay;    // Milliseconds between animation draws.
} WI;
#endif

#ifdef WCLI
typedef struct _WindowInternal {
  HINSTANCE hinst;  // Instance of the Astrolog window class.
  HWND hwndMain;    // The outer created frame window.
  HWND hwnd;        // The current window being dealt with.
  HDC hdc;          // The current DC bring drawn upon.
  HDC hdcBack;      // The current DC for the background bitmap.
  HPEN hpen;        // Pen with the current line color.
  HBRUSH hbrush;    // Fill if any with the current color.
  HBITMAP hbmpBack; // Bitmap for the background image.
  HBITMAP hbmpPrev; // Bitmap to restore after using background.
  short xClient;    // Horizontal & vertical window size.
  short yClient;
  flag fDoRedraw;
  flag fDoResize;
  flag fDoCast;
  flag fWndclass;
  int xMouse;       // Horizontal & vertical mouse position.
  int yMouse;
  LPARAM lParamRC;  // Coordinates where right click originated.
  Bitmap bmpWin;    // Bitmap storing contents to be copied to window.
  KI kiPen;         // The current pen scribble color.
} WI;
#endif

#include "extern.h"

/* astrolog.h */
