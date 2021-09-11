/*
** Astrolog (Version 7.30) File: charts0.cpp
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


/*
******************************************************************************
** Table Display Routines.
******************************************************************************
*/

// A subprocedure of the credit displayed below, this prints out one line of
// credit information on the screen. Given a string, it's displayed centered
// with left and right borders around it, in the given color.

void PrintW(CONST char *sz, int col)
{
  int i;

  if (!sz) {

    // Null string means print the top, bottom, or a separator row.
    if (col < 0)
      AnsiColor(kRedA);
    PrintCh((char)(col ? (col > 0 ? chSW : chNW) : chJE));
    PrintTab(chH, CREDITWIDTH);
    PrintCh((char)(col ? (col > 0 ? chSE : chNE) : chJW));
  } else {
    i = CchSz(sz);
    PrintCh(chV);
    PrintTab(' ', (CREDITWIDTH-i)/2 + (i&1));
    AnsiColor(col);
    PrintSz(sz);
    PrintTab(' ', (CREDITWIDTH-i)/2);
    AnsiColor(kRedA);
    PrintCh(chV);
  }
  PrintL();
}


// Display a list of credits showing those who helped create the various parts
// of Astrolog, as well as important copyright and version info, as displayed
// with the -Hc switch.

void DisplayCredits(void)
{
  char sz[cchSzDef], szT[cchSzDef];

  PrintW(NULL, -1);
#ifdef PC
  sprintf(szT, " for %s Windows", szArchCore);
#else
  *szT = chNull;
#endif
  sprintf(sz, "** %s version %s%s **", szAppName, szVersionCore, szT);
  PrintW(sz, kWhiteA);
  sprintf(sz, "Released %s - By Walter D. Pullen", szDateCore);
  PrintW(sz, kLtGrayA);
  PrintW(szAddressCore, kCyanA);
  PrintW(NULL, 0);
  PrintW(
    "Main ephemeris databases and calculation routines are from the library",
    kGreenA);
  PrintW("'Swiss Ephemeris' by Astrodienst AG, subject to license for Swiss",
    kGreenA);
  PrintW(
    "Ephemeris Free Edition at http://www.astro.com/swisseph. Old 'Placalc'",
    kGreenA);
  PrintW(
    "library and formulas are by Alois Treindl, also from Astrodienst AG.",
    kGreenA);
  PrintW("Original planetary calculation formulas were converted from",
    kDkGreenA);
  PrintW(
    "routines by James Neely, as listed in 'Manual of Computer Programming",
    kDkGreenA);
  PrintW(
    "for Astrologers' by Michael Erlewine, available from Matrix Software.",
    kDkGreenA);
  PrintW(
    "Atlas list of city locations from GeoNames: https://www.geonames.org/",
    kMagentaA);
  PrintW("Timezone and Daylight Saving Time date changes converted from",
    kPurpleA);
  PrintW("TZ database: https://data.iana.org/time-zones/tz-link.html",
    kPurpleA);
  PrintW("PostScript graphics routines by Brian D. Willoughby.", kYellowA);
  PrintW(
    "IMPORTANT: Astrolog is free software. You can distribute and/or modify",
    kLtGrayA);
  PrintW(
    "it under the terms of the GNU General Public License, as described at",
    kLtGrayA);
  PrintW("http://www.gnu.org and in the license.htm file included with the",
    kLtGrayA);
  PrintW("program. Astrolog is distributed without any warranty expressed or",
    kLtGrayA);
  PrintW(
    "implied of any kind. These license and copyright notices must not be",
    kLtGrayA);
  PrintW("changed or removed by any user or editor of the program.",
    kLtGrayA);
  PrintW(NULL, 0);
  PrintW("Special thanks to all those unmentioned, seen and unseen, who have",
    kBlueA);
  PrintW(
    "pointed out problems, suggested features, & sent many positive vibes! :)",
    kBlueA);
  PrintW(NULL, 1);
  AnsiColor(kDefault);
}


// Print out a command switch or keypress info line to the screen, as done
// with the -H switch or 'H' key in a graphic window. This is just printing
// out the string, except set the proper colors: Red for header lines, Green
// for individual switches or keys, dark green for subswitches, and White for
// the rest of the line telling what it does. Also prefix each switch with
// Unix's '-' or PC's '/', whichever is appropriate for system.

void PrintS(CONST char *sz)
{
  char ch, ch1, ch2, ch3, ch4;
  static char ch1Prev = chNull, ch2Prev = chNull, ch3Prev = chNull,
    ch4Prev = chNull;

  // Determine color for first part of line.
  ch1 = sz[1]; ch2 = sz[2]; ch3 = sz[3]; ch4 = sz[4];
  if (*sz != ' ')
    AnsiColor(kRedA);
  else if (ch1 != ' ')
    AnsiColor(ch1 == 'P' || ch2 != ch2Prev ||
      ((ch2 == 'Y' || ch2 == 'X' || ch2 == 'W' || ch2 == '~') &&
        ch3 != ch3Prev) ||
      (ch2 == 'Y' && ch3 == 'X' && ch4 != ch4Prev) ?
      kGreenA : kDkGreenA);
  else {
    AnsiColor(kDefault);
    PrintSz("   ");
  }

  // Print first part of line.
  while ((ch = *sz) && ch != ':' &&
    (ch1 != 'P' || (ch != ' ' || *(sz+1) != 't'))) {
    if (ch != '_')
      PrintCh(ch);
    else
      PrintCh(chSwitch);
    sz++;
  }
  if (*sz)
    PrintCh(*sz++);

  // Print rest of line in default color.
  AnsiColor(kDefault);
  while (ch = *sz) {
    if (ch != '_')
      PrintCh(ch);
    else
      PrintCh(chSwitch);
    sz++;
  }
  PrintL();

  if (ch1 != ' ') {
    ch1Prev = ch1; ch2Prev = ch2; ch3Prev = ch3; ch4Prev = ch4;
  }
}


// Print a list of every command switch that can be passed to the program, and
// a description of what it does. This is what the -H switch prints.

void DisplaySwitches(void)
{
  char sz[cchSzDef];

  sprintf(sz, "%s (version %s) command switches:", szAppName, szVersionCore);
  PrintS(sz);
  PrintS(" _H: Display this help list.");
  PrintS(" _Hc: Display program credits and copyrights.");
  PrintS(" _HC: Display names of zodiac signs and houses.");
  PrintS(" _HO: Display available planets and other celestial objects.");
  PrintS(" _HA: Display available aspects, their angles, and present orbs.");
#ifdef CONSTEL
  PrintS(" _HF: Display names of astronomical constellations.");
#endif
  PrintS(" _HS: Display information about planets in the solar system.");
  PrintS(" _H7: Display information about the esoteric seven Rays.");
#ifdef INTERPRET
  PrintS(" _HI: Display meanings of signs, houses, planets, and aspects.");
#endif
  sprintf(sz,
    " _He: Display all tables together (_Hc_H_Y_HX_HC_HO_HA%s_HS_H7%s).",
#ifdef CONSTEL
  "_HF",
#else
  "",
#endif
#ifdef INTERPRET
  "_HI");
#else
  "");
#endif
  PrintS(sz);
  PrintS(" _Q: Prompt for more command switches after display finished.");
#ifdef SWITCHES
  PrintS(" _Q0: Like _Q but prompt for additional switches on startup.");
#endif
  PrintS(" _M <1-48>: Run the specified command switch macro.");
  PrintS(" _M0 <1-48> <string>: Define the specified command switch macro.");
  PrintS(
    " _M[1-6][0] <strings>: Define macro(s) to run when chart calculated.");
  PrintS(" _Y: Display help list of less commonly used command switches.");
  PrintS("\nSwitches which determine the type of chart to display:");
  PrintS(" _v: Display list of object positions (chosen by default).");
  PrintS(" _v0: Like _v but express velocities relative to average speed.");
  PrintS(" _v3: Like _v but display decan information alongside positions.");
  PrintS(" _w [<rows>]: Display chart in a graphic house wheel format.");
  PrintS(" _w0 [..]: Like _w but reverse order of objects in houses 4..9.");
  PrintS(" _g: Display aspect and midpoint grid among planets.");
  PrintS(" _g0: Like _g but flag aspect configurations (e.g. Yods) too.");
  PrintS(" _gm: For comparison charts, show midpoints instead of aspects.");
  PrintS(
    " _ga: Like _g but indicate applying/separating instead of offset orbs.");
  PrintS(" _gp: Like _g but generate parallel and contraparallel aspects.");
  PrintS(" _a: Display list of all aspects ordered by influence.");
  PrintS(" _a0: Like _a but display aspect summary too.");
  PrintS(
    " _aa: Like _a but indicate applying/separating instead of offset orbs.");
  PrintS(" _ap: Like _a but do parallel and contraparallel aspects.");
  PrintS(
    " _a[jonOPACDm]: Sort aspects by power, orb, orb difference, 1st planet,");
  PrintS("  2nd planet, aspect, 1st position, 2nd position, midpoint.");
  PrintS(" _m: Display all object midpoints in sorted zodiac order.");
  PrintS(" _m0: Like _m but display midpoint summary too.");
  PrintS(" _ma: Like _m but show aspects from midpoints to planets as well.");
  PrintS(" _Z: Display planet locations with respect to the local horizon.");
  PrintS(" _Z0: Like _Z but express coordinates relative to polar center.");
  PrintS(" _Zd: Search day for object local rising and setting times.");
  PrintS(
    " _Zd[m,y,Y] [<years>]: Like _Zd but for entire month, year, or years.");
  PrintS(" _S: Display x,y,z coordinate positions of planets in space.");
  PrintS(" _l: Display Gauquelin sectors for each planet in chart.");
  PrintS(" _l0: Like _l but approximate sectors using Placidus cusps.");
  PrintS(" _j: Display astrological influences of each object in chart.");
  PrintS(" _j0: Like _j but include influences of each zodiac sign as well.");
  PrintS(" _7: Display Esoteric Astrology and Ray summary for chart.");
  PrintS(" _L [<step>]: Display astro-graph locations of planetary angles.");
  PrintS(" _L0 [<step> [<dist>]]: Like _L but list latitude crossings too.");
  PrintS(" _K: Display a calendar for given month.");
  PrintS(" _Ky: Like _K but display a calendar for the entire year.");
  PrintS(" _d [<step>]: Print all aspects and changes occurring in a day.");
  PrintS(" _dm: Like _d but print all aspects for the entire month.");
  PrintS(" _dy: Like _d but print all aspects for the entire year.");
  PrintS(" _dY <years>: Like _d but search within a number of years.");
  PrintS(" _dp <month> <year>: Print aspects within progressed chart.");
  PrintS(" _dpy <year>: Like _dp but search for aspects within entire year.");
  PrintS(" _dpY <year> <years>: Like _dp but search within number of years.");
  PrintS(" _dp[y]n: Search for progressed aspects in current month/year.");
  PrintS(" _D: Like _d but display aspects by influence instead of time.");
  PrintS(" _B: Like _d but graph all aspects occurring in a day.");
  PrintS(" _B[m,y,Y]: Like _B but for entire month, year, or five years.");
  PrintS(" _B0: Like _B but don't restrict fast moving objects from graph.");
  PrintS(" _E: Display planetary ephemeris for given month.");
  PrintS(" _Ey: Display planetary ephemeris for the entire year.");
  PrintS(" _EY <years>: Display planetary ephemeris for a number of years.");
  PrintS(" _E[]0 <step>: Display ephemeris times for days, months, or years.");
  PrintS(" _8: Display planetary moons chart showing placements and aspects.");
  PrintS(" _e: Display all charts (_v_w_g_a_m_Z_S_l_K_j_7_L_E_P_Zd_d_D_B_8).");
  PrintS(
    " _t <month> <year>: Compute all transits to natal planets in month.");
  PrintS(
    " _tp <month> <year>: Compute progressions to natal in month for chart.");
  PrintS(" _tr <month> <year>: Compute all returns in month for chart.");
  PrintS(" _t[p]d: <month> <day> <year>: Compute transits for a single day.");
  PrintS(" _t[p]y: <year>: Compute transits/progressions for entire year.");
  PrintS(" _t[p]Y: <year> <years>: Compute transits for a number of years.");
#ifdef TIME
  PrintS(" _t[py]n: Compute transits to natal planets for current time now.");
#endif
  PrintS(" _T <month> <day> <year>: Display transits ordered by influence.");
  PrintS(" _Tt <month> <day> <year> <time>: Like _T but specify time too.");
  PrintS(
    " _T[t]p <month> <day> <year>: Print progressions instead of transits.");
#ifdef TIME
  PrintS(" _T[p]n: Display transits ordered by influence for current date.");
#endif
  PrintS(" _V [..]: Like _t but graph all transits occurring during period.");
  PrintS(
    " _V[d,y,Y] [[<day>] <month>] <year>: Like _V for day, year, or 5 years.");
  PrintS(
    " _V[..]0: Like _V but don't restrict fast moving objects from graph.");
#ifdef ARABIC
  PrintS(" _P [<parts>]: Display list of Arabic parts and their positions.");
  PrintS(" _P0 [<parts>]: Like _P but display formulas with terms reversed.");
  PrintS(" _P[z,n,f]: Order parts by position, name, or formula.");
#endif
#ifdef ATLAS
  PrintS(" _N [<rows>]: Lookup chart location as city in atlas.");
  PrintS(" _Nl [<rows>]: Display nearest cities in atlas to chart location.");
  PrintS(
    " _Nz [<rows>]: Display all time changes in time zone of chart city.");
#endif
#ifdef INTERPRET
  PrintS(" _I [<columns>]: Print interpretation of selected charts.");
#endif
  PrintS("\nSwitches which affect how the chart parameters are obtained:");
#ifdef TIME
  PrintS(" _n: Compute chart for this exact moment using current time.");
  PrintS(" _n[d,m,y]: Compute chart for start of current day, month, year.");
#endif
  PrintS(" _z [<zone>]: Change the default time zone (for _d_E_t_q options).");
  PrintS(" _z0 [<offset>]: Change the default Daylight time setting.");
  PrintS(" _zl <long> <lat>: Change the default longitude & latitude.");
  PrintS(" _zv <elev>: Change the default elevation above sea level.");
  PrintS(" _zj <name> <place>: Change the default name and place strings.");
  PrintS(" _zt <time>: Set only the time of current chart.");
  PrintS(" _zd <date>: Set only the day of current chart.");
  PrintS(" _zm <month>: Set only the month of current chart.");
  PrintS(" _zy <year>: Set only the year of current chart.");
  PrintS(" _zi <name> <place>: Set name and place strings of current chart.");
#ifdef ATLAS
  PrintS(
    " _zL <city>: Lookup city in atlas and set location in current chart.");
  PrintS(
    " _zN <city>: Lookup city in atlas and set zone, Daylight, and location.");
#endif
  PrintS(" _q <month> <date> <year> <time>: Compute chart with defaults.");
  PrintS(" _qd <month> <date> <year>: Compute chart for noon on date.");
  PrintS(" _qm <month> <year>: Compute chart for first of month.");
  PrintS(" _qy <year>: Compute chart for first day of year.");
  PrintS(" _qa <month> <date> <year> <time> <zone> <long> <lat>:");
  PrintS("  Compute chart automatically given specified data.");
  PrintS(" _qb <month> <date> <year> <time> <daylight> <zone> <long> <lat>:");
  PrintS("  Like _qa but takes additional parameter for Daylight offset.");
  PrintS(" _qj <day>: Compute chart for time of specified Julian day.");
  PrintS(" _i <file>: Compute chart based on info in file.");
  PrintS(" _i[2-6] <file>: Load chart info into chart slots 2 through 6.");
  PrintS(" _o <file> [..]: Write parameters of current chart to file.");
  PrintS(" _o0 <file> [..]: Like _o but output planet/house positions.");
  PrintS(" _oa <file>: Write parameters of current chart to AAF format file.");
  PrintS(" _od <file>: Output program's current settings to switch file.");
  PrintS(" _os <file>, > <file>: Redirect output of text charts to file.");
  PrintS("\nSwitches which affect what information is used in a chart:");
  PrintS(" _R [<obj1> [<obj2> ..]]: Restrict specific bodies from displays.");
  PrintS(" _R0 [<obj1> ..]: Like _R but restrict everything first.");
  PrintS(" _R1 [<obj1> ..]: Like _R0 but unrestrict and show all objects.");
  PrintS(
    " _R[C,u,u0,8,U]: Restrict all cusps, Uranians, Dwarfs, moons, or stars.");
  PrintS(
    " _RT[0,1,C,u,u0,8,U] [..]: Restrict transiting planets in _t lists.");
  PrintS(" _RA [<asp1> ..]: Restrict specific aspects from displays.");
  PrintS(" _RO <obj>: Require object to be present in aspects.");
  PrintS(" _C: Include angular and non-angular house cusps in charts.");
  PrintS(" _u: Include Uranian/transneptunian bodies in charts.");
  PrintS(" _u0: Include Dwarf planets and related bodies in charts.");
  PrintS(" _u8: Include planetary moon bodies in charts.");
  PrintS(" _ub: Include planetary center of body (COB) objects in charts.");
  PrintS(" _U: Include locations of fixed background stars in charts.");
  PrintS(" _U[z,l,n,b,d,v]: Sort stars by zodiac position, latitude, name,");
  PrintS("   brightness, distance, or zodiac position velocity.");
  PrintS(" _A <0-18>: Specify the number of aspects to use in charts.");
  PrintS(
    " _A3: Aspects calculated by latitude combined with zodiac position.");
  PrintS(" _Ap: Orb limits apply to latitude as well as zodiac position.");
  PrintS(
    " _AP: Parallel aspects based on ecliptic not equatorial positions.");
  PrintS(" _Ao <aspect> <orb>: Specify maximum orb for an aspect.");
  PrintS(" _Am <planet> <orb>: Specify maximum orb allowed to a planet.");
  PrintS(" _Ad <planet> <orb>: Specify orb addition given to a planet.");
  PrintS(" _Aa <aspect> <angle>: Change the actual angle of an aspect.");
  PrintS("\nSwitches which affect how a chart is computed:");
#ifdef EPHEM
  PrintS(" _b: Use ephemeris files for more accurate location computations.");
#endif
  PrintS(" _b0: Display locations and times to the nearest second.");
#ifdef SWISS
  PrintS(
    " _bj: Use more accurate JPL ephemeris file instead of Swiss Ephemeris.");
  PrintS(
    " _bs: Use less accurate Moshier formulas instead of Swiss Ephemeris.");
#endif
#ifdef PLACALC
  PrintS(
    " _bp: Use less accurate Placalc ephemeris instead of Swiss Ephemeris.");
#endif
#ifdef MATRIX
  PrintS(" _bm: Use inaccurate Matrix formulas when ephemeris unavailable.");
  PrintS(" _bU: Use inaccurate Matrix formulas for fixed stars only.");
#endif
#ifdef JPLWEB
  PrintS(" _bJ: Use most accurate JPL Web query instead of Swiss Ephemeris.");
#endif
  PrintS(" _c <value>: Select a different system of house division.");
  PrintS("  0 = Placidus, 1 = Koch, 2 = Equal, 3 = Campanus, 4 = Meridian,");
  PrintS("  5 = Regiomontanus, 6 = Porphyry, 7 = Morinus, 8 = Topocentric,");
  PrintS("  9 = Alcabitius, 10 = Krusinski, 11 = Equal (Midheaven),");
  PrintS("  12 = Pullen Sinusoidal Ratio, 13 = Pullen Sinusoidal Delta,");
  PrintS("  14 = Whole, 15 = Vedic, 16 = Sripati, 17 = Horizon, 18 = APC,");
  PrintS(
    "  19 = Carter Poli Equatorial, 20 = Sunshine, 21 = Savard-A, 22 = Null.");
  PrintS(
    " _c3 [0-3]: Place in houses using latitude as well as zodiac position.");
  PrintS(" _s [..]: Compute a sidereal instead of standard tropical chart.");
  PrintS(" _sr: Compute right ascension locations relative to equator.");
  PrintS(
    " _sr0: Like _sr but only display declinations instead of latitudes.");
  PrintS(
    " _s[z,h,n,d]: Display as zodiac, hr/min, Nakshatras, or 0-360 degrees.");
  PrintS(" _h [<objnum>]: Compute positions centered on specified object.");
  PrintS(
    " _p <month> <day> <year>: Cast secondary progressed chart for date.");
  PrintS(" _p0 <month> <day> <year>: Cast solar arc chart for date.");
  PrintS(" _p1 <month> <day> <year>: Like _p but with solar arc cusps only.");
  PrintS(" _p[0]t <month> <day> <year> <time>: Like _p but specify time too.");
#ifdef TIME
  PrintS(" _p[0]n: Cast progressed chart based on current date now.");
#endif
  PrintS(
    " _pd <days>: Set num of days to progress / day (default 365.24219).");
  PrintS(
    " _pC <days>: Set factor to use when progressing cusps (default 1.0).");
  PrintS(" _x <value>: Cast harmonic chart based on specified factor.");
  PrintS(" _1 [<objnum>]: Cast chart with specified object on Ascendant.");
  PrintS(" _2 [<objnum>]: Cast chart with specified object on Midheaven.");
  PrintS(" _3: Display objects in their zodiac decan positions.");
  PrintS(" _4 [<nest>]: Display objects in their (nested) dwad positions.");
  PrintS(" _f: Display houses as sign positions (flip them).");
  PrintS(" _G: Compute houses based on geographic location only.");
  PrintS(" _J: Display wheel charts in Vedic format.");
  PrintS(" _9: Display objects in their zodiac navamsa positions.");
  PrintS(" _F <objnum> <sign> <deg>: Force object's position to be value.");
  PrintS(" _Fm <objnum> <obj1> <obj2>: Force object's position to midpoint.");
  PrintS(" _+ [<days>]: Cast chart for specified num of days in the future.");
  PrintS(" _- [<days>]: Cast chart for specified num of days in the past.");
  PrintS(" _+[m,y] [<value>]: Cast chart for num of months/years in future.");
  PrintS("\nSwitches for relationship and comparison charts:");
  PrintS(" _r <file1> <file2>: Compute a relationship synastry chart.");
  PrintS(" _rc <file1> <file2>: Compute a composite chart.");
  PrintS(" _rm <file1> <file2>: Compute a time space midpoint chart.");
  PrintS(" _r[c,m]0 <file1> <file2> <ratio1> <ratio2>: Weighted chart.");
  PrintS(" _rd <file1> <file2>: Print time span between files' dates.");
#ifdef BIORHYTHM
  PrintS(" _rb <file1> <file2>: Display biorhythm for file1 at time file2.");
#endif
  PrintS(" _r0 <file1> <file2>: Keep the charts separate in comparison.");
  PrintS(" _rp[0] <file1> <file2>: Like _r0 but do file1 progr. to file2.");
  PrintS(" _rt <file1> <file2>: Like _r0 but treat file2 as transiting.");
#ifdef GRAPH
  PrintS(" _r[2-6]: Make graphics wheel chart tri-wheel, quad-wheel, etc.");
#endif
#ifdef TIME
  PrintS(" _y <file>: Display current house transits for particular chart.");
#ifdef BIORHYTHM
  PrintS(" _y[b,d,p,t] <file>: Like _r0 but compare to current time now.");
#endif
#endif // TIME
  PrintS("\nSwitches to access graphics options:");
  PrintS(" _k: Display text charts using Ansi characters and color.");
  PrintS(" _k0: Like _k but only use special characters, not Ansi color.");
  PrintS(" _kh: Text charts saved to file use HTML instead of Ansi codes.");

  // If graphics features are compiled in, call an additional procedure to
  // display the command switches offered dealing with graphics options.

#ifdef GRAPH
  DisplaySwitchesX();
#ifdef WIN
  DisplaySwitchesW();  // Windows version has its own set of switches too.
#endif
#endif // GRAPH
}


// Print a list of the obscure command switches that can be passed to the
// program and a description of them. This is what the -Y switch prints.

void DisplaySwitchesRare(void)
{
  char sz[cchSzDef];

  sprintf(sz, "%s (version %s) obscure command switches:",
    szAppName, szVersionCore);
  PrintS(sz);
  PrintS(" _Y: Display this help list.");
  PrintS(" _YT: Compute true positions in space instead of apparent in sky.");
  PrintS(
    " _YV: Compute topocentric positions instead of from center of body.");
  PrintS(" _Yh: Compute location of solar system barycenter instead of Sun.");
  PrintS(" _Ym: Position planetary moons around current central object.");
  PrintS(" _Ys: Sidereal zodiac positions are in plane of solar system.");
  PrintS(" _Yn: Compute location of true instead of mean nodes and Lilith.");
  PrintS(" _Yn0: Don't consider nutation in tropical zodiac positions.");
  PrintS(" _Yu: Display eclipse and occultation information in charts.");
  PrintS(" _Yu0: Like _Yu but detect maximum eclipse anywhere on Earth.");
  PrintS(" _Yd: Display dates in D/M/Y instead of M/D/Y format.");
  PrintS(" _Yt: Display times in 24 hour instead of am/pm format.");
  PrintS(" _Yv: Display distance in metric instead of imperial units.");
  PrintS(" _Yr: Round positions to nearest unit instead of crop fraction.");
  PrintS(" _YC: Automatically ignore insignificant house cusp aspects.");
  PrintS(" _YO: Automatically adjust settings when exporting and printing.");
  PrintS(" _Y8: Clip text charts at the rightmost (e.g. 80th) column.");
  PrintS(" _YQ <rows>: Pause text scrolling after a page full has printed.");
  PrintS(
    " _Yq[0-9] <strings>: Define command lines to run and show in sequence.");
  PrintS(" _Yi[0-9] <path>: Specify directory to search within for files.");
  PrintS(" _Yo: Output chart info and position files in old style format.");
  PrintS(" _Yc: Angular cusp objects are house positions instead of angles.");
  PrintS(" _Yp: Fix polar houses by preserving Ascendant instead of MC.");
  PrintS(" _Yz <min>: Forward clock by amount for current moment charts.");
  PrintS(" _Yz0 <sec>: Set seconds of Delta-T to always use for charts.");
  PrintS(" _YzO <hr>: Forward object positions by amount for all charts.");
  PrintS(" _YzC <hr>: Forward cusp positions by amount for all charts.");
  PrintS(
    " _Y1[0] <obj1> <obj2>: Rotate planets so one is at other's position.");
  PrintS(" _Yl <1-36>: Toggle plus zone status of sector for sector chart.");
#ifdef ARABIC
  PrintS(" _YP <-1,0,1>: Set how Arabic parts are computed for night charts.");
#endif
  PrintS(" _Yb <days>: Set number of days to span for biorhythm chart.");
#ifdef SWISS
  PrintS(" _Ye <obj> <index>: Change orbit of Uranian to external formula.");
  PrintS(
    " _Yeb <obj> <index>: Change orbit of Uranian to external ephemeris.");
  PrintS(
    " _Yem <obj> <index>: Change orbit of Uranian to planet moon ephemeris.");
  PrintS(" _YeO <obj1> <obj2>: Change orbit of Uranian to internal planet.");
#ifdef JPLWEB
  PrintS(
    " _Yej <obj> <index>: Change orbit of Uranian to JPL Horizons Web query.");
#endif
  PrintS(" _Ye[..]n <obj> <index>: Change Uranian to North Node of object.");
  PrintS(" _Ye[..]s <obj> <index>: Change Uranian to South Node of object.");
  PrintS(" _Ye[..]a <obj> <index>: Change Uranian to aphelion of object.");
  PrintS(" _Ye[..]p <obj> <index>: Change Uranian to perihelion of object.");
  PrintS(
    " _Ye[..]HSBNTV <obj> <index>: Toggle heliocentric, sidereal zodiac,");
  PrintS(
    "  barycentric, true node, true position, or topocentric for object.");
#endif
#ifdef MATRIX
  PrintS(" _YE <obj> <semi-major axis> <eccentricity (3)> <inclination (3)>");
  PrintS("  <perihelion (3)> <ascending node (3)> <time offset (3)>:");
  PrintS("  Change orbit of object to be the given elements.");
#endif
#ifdef SWISS
  PrintS(" _YU <obj> <name>: Change position of star to sefstars.txt entry.");
#endif
  PrintS(
    " _YUb: Adjust star brightness to apparent magnitude based on distance.");
  PrintS(
    " _YUb0: Set brightness to distance independent absolute magnitude.");
  PrintS(" _YS <obj> <size>: Set diameter of object to be specified size.");
  PrintS(
    " _YR <obj1> <obj2> <flag1>..<flag2>: Set restrictions for object range.");
  PrintS(
    " _YRT <obj1> <obj2> <flag1>..<flag2>: Transit restrictions for range.");
  PrintS(
    " _YR0 <flag1> <flag2>: Set restrictions for sign, direction changes.");
  PrintS(
    " _YR1 <flag1> <flag2>: Set restrictions for latitude, distance events.");
  PrintS(
    " _YRZ <rise> <zenith> <set> <nadir>: Set restrictions for _Zd chart.");
  PrintS(
    " _YR7 <ruler> <exalt> <eso> <hier> <ray>: Set rulership restrictions.");
  PrintS(
    " _YRd <div>: Set divisions within signs to search for degree changes.");
  PrintS(" _YRh: Don't auto(un)restrict central planet when changing it.");
  PrintS(" _YRU[0] <starlist>: Restrict or focus on list of extra stars.");
  PrintS(" _YAo <asp1> <asp2> <orb1>..<orb2>: Set aspect orbs for range.");
  PrintS(" _YAm <obj1> <obj2> <orb1>..<orb2>: Set max planet orbs for range.");
  PrintS(
    " _YAd <obj1> <obj2> <orb1>..<orb2>: Set planet orb additions for range.");
  PrintS(
    " _YAa <asp1> <asp2> <ang1>..<ang2>: Set planet aspect angles for range.");
  PrintS(
    " _YAD <asp> <name> <abbrev> <glyph>: Customize display names of aspect.");
  PrintS(
    " _Yj <obj1> <obj2> <inf1>..<inf2>: Set influences for object range.");
  PrintS(
    " _YjC <cusp1> <cusp2> <inf1>..<inf2>: Set influences for house cusps.");
  PrintS(
    " _YjA <asp1> <asp2> <inf1>..<inf2>: Set influences for aspect range.");
  PrintS(
    " _YjT <obj1> <obj2> <inf1>..<inf2>: Set transit influences for range.");
  PrintS(
    " _Yj0 <inf1> <inf2> <inf3> <inf4>: Set influences given to planets");
  PrintS("  in ruling sign, exalted sign, ruling house, exalted house.");
  PrintS(
    " _Yj7 <inf1> <inf2> <inf3> <inf4> <inf5> <inf6>: Set influences for in");
  PrintS(
    "  esoteric, hierarchical, Ray ruling sign, plus same for ruling house.");
  PrintS(" _YJ <obj> <sign> <cosign>: Set sign planet rules and co-rules.");
  PrintS(" _YJ0 <obj> <sign>: Set zodiac sign given planet exalts in.");
  PrintS(" _YJ7 <obj> <sign> <cosign>: Set signs planet esoterically rules.");
  PrintS(
    " _YJ70 <obj> <sign> <cosign>: Set signs planet hierarchically rules.");
  PrintS(" _Y7O <obj1> <obj2> <ray1>..<ray2>: Customize object rays.");
  PrintS(" _Y7C <sign1> <sign2> <rays1>..<rays2>: Customize sign rays.");
  PrintS(" _YI <obj> <string>: Customize interpretation for object.");
  PrintS(
    " _YIa <sign> <string>: Customize interpretation adjective for sign.");
  PrintS(" _YIv <sign> <string>: Customize interpretation verb for sign.");
  PrintS(" _YIC <house> <string>: Customize interpretation for house.");
  PrintS(" _YIA <asp> <string>: Customize interpretation for aspect.");
  PrintS(" _YIA0 <asp> <string>: Customize aspect interpretation statement.");
  PrintS(" _YkO <obj1> <obj2> <col1>..<col2>: Customize planet colors.");
  PrintS(" _YkC <fir> <ear> <air> <wat>: Customize element colors.");
  PrintS(" _YkA <asp1> <asp2> <col1>..<col2>: Customize aspect colors.");
  PrintS(" _Yk7 <1..7> <1..7> <col1>..<col2>: Customize Ray colors.");
  PrintS(" _Yk0 <1..7> <1..7> <col1>..<col2>: Customize 'rainbow' colors.");
  PrintS(" _Yk <0..8> <0..8> <col1>..<col2>: Customize 'general' colors.");
#ifdef SWISS
  PrintS(" _YkU <starlist>: Customize list of extra star colors.");
  PrintS(" _YkE <astlist>: Customize list of extra asteroid colors.");
#endif
  PrintS(" _YD <obj> <name>: Customize display name of object.");
  PrintS(
    " _YF <obj> <deg><sign><min> <deg><min> <velocity> <au>: Set position.");
#ifdef GRAPH
  PrintS("\nSwitches to access obscure graphics options:");
  PrintS(" _YXG <0-2><0-2><0-3><0-2>: Select among different graphic glyphs");
  PrintS("  for Capricorn, Uranus, Pluto, and Lilith.");
  PrintS(" _YXD <obj> <string1> <string2>: Customize glyphs for planet.");
  PrintS(" _YXA <asp> <string1> <string2>: Customize glyphs for aspect.");
  PrintS(" _YXv <type> [<size> [<lines>]]: Set wheel chart decoration.");
  PrintS(" _YXt <string>: Display extra text in wheel chart sidebar.");
  PrintS(" _YXg <cells>: Set number of cells for graphic aspect grid.");
  PrintS(" _YXS <au>: Set radius of graphic solar system orbit chart.");
  PrintS(" _YXj <num>: Set number of graphic orbit trails to remember.");
  PrintS(" _YXj0 <step>: Set vertical step rate for graphic orbit trails.");
  PrintS(" _YX7 <inf>: Set influence width for graphic esoteric ephemeris.");
  PrintS(" _YXk: Use more color for sign boundaries in graphics charts.");
  PrintS(
    " _YXk0: Use more color for house boundaries in graphics charts too.");
  PrintS(" _YXK <col> <rgb>: Customize RGB value of color index.");
  PrintS(" _YXK0: Use alternate color palette for white background mode.");
  PrintS(" _YXe: Align certain charts to plane of ecliptic.");
  PrintS(" _YXa <num>: Set limit to dashedness in aspect lines drawn.");
#ifdef SWISS
  PrintS(" _YXU <starlist> <linklist>: Define lines between extra stars.");
  PrintS(" _YXU0 <starlist> <linklist>: Append instead of replace lines.");
#endif
  PrintS(" _YXW <num>: Draw triangles or cubes grid over world maps.");
  PrintS(" _YXf <0-8><0-8><0-8><0-8><0-8>: Set font usage in graphic charts.");
  PrintS(" _YXp <-1,0,1>: Set paper orientation for PostScript files.");
  PrintS(" _YXp0 <hor> <ver>: Set paper size for PostScript files.");
#endif // GRAPH
  PrintS("\nSwitches to access obscure system options:");
  PrintS(" _YB: Make a beep sound at the time this switch is processed.");
  PrintS(" _YY <rows>: Load atlas list of city locations from current file.");
  PrintS(
    " _YY1 <rules> <entries>: Load Daylight Time rules from current file.");
  PrintS(" _YY2 <zones> <entries>: Load time zone change lists from file.");
  PrintS(
    " _YY3 <rows>: Load atlas time zone to zone change mappings from file.");
  PrintS(" _0[o,i,q,X,n,~]: Permanently disable file output, file input,");
  PrintS("  program exiting, all graphics, internet, or AstroExpressions.");
  PrintS(" _;: Ignore rest of command line and treat it as a comment.");
#ifdef EXPRESS
  PrintS("\nSwitches to define AstroExpressions:");
  PrintS(" _~ <string>: Display result of string parsed as AstroExpression.");
  PrintS(" _~g <string>: Set filter for aspect configurations.");
  PrintS(" _~a <string>: Set adjustment for aspect list powers.");
  PrintS(" _~a0 <string>: Set notification for aspect list summary.");
  PrintS(" _~m <string>: Set filter for midpoint display.");
  PrintS(" _~ma <string>: Set filter for displaying aspects to midpoints.");
  PrintS(" _~j <string>: Set adjustment for object influence.");
  PrintS(" _~j0 <string>: Set adjustment for sign influence.");
  PrintS(" _~L <string>: Set filter for astro-graph latitude crossings.");
  PrintS(" _~E <string>: Set filter for text ephemeris lines.");
  PrintS(" _~P <string>: Set filter for Arabic parts display.");
  PrintS(" _~d <string>: Set filter for transit to transit events.");
  PrintS(" _~dv <string>: Set adjustment for void of course determinations.");
  PrintS(" _~t <string>: Set filter for transit to natal events.");
  PrintS(" _~O <string>: Set adjustment for object calculations.");
  PrintS(" _~C <string>: Set adjustment for house cusp calculations.");
  PrintS(" _~A <string>: Set adjustment for aspect orbs.");
  PrintS(" _~p[0] <string>: Set adjustment for progression offset.");
  PrintS(" _~kO <string>: Set adjustment for object colors.");
  PrintS(" _~kA <string>: Set adjustment for aspect colors.");
  PrintS(" _~kv <string>: Set adjustment for wheel chart fill colors.");
  PrintS(" _~F[O,C,A] <string>: "
    "Set adjustment for sign/object/house/aspect fonts.");
  PrintS(" _~v <string>: Set adjustment for object display ordering.");
  PrintS(" _~v3 <string>: Set adjustment for wheel chart decan markings.");
  PrintS(" _~Xt <string>: Set notification before sidebar drawn.");
  PrintS(" _~U <string>: Set filter for extra stars.");
  PrintS(" _~U0 <string>: Set filter for extra asteroids.");
  PrintS(" _~q[1-2] <string>: Set notification before/after chart cast.");
  PrintS(" _~Q[1-2] <string>: Set notification before/after chart displayed.");
  PrintS(" _~M <0-26> <string>: Define the specified AstroExpression macro.");
  PrintS(" _~1 <string>: Simply parse AstroExpression (don't show result).");
  PrintS(" _~0: Disable all automatic AstroExpression checks in the program.");
#endif
}


// Print out a list of the various objects - planets, asteroids, house 
// cusps, stars - recognized by the program, and their index values. This
// is displayed when the -HO switch is invoked. For some objects, display
// additional information, e.g. ruling signs for planets, brightnesses and
// positions in the sky for fixed stars, etc.

void PrintObjects(void)
{
  char sz[cchSzDef];
  int i, j, k, l;
#ifdef SWISSGRAPH
  ES es;
#endif

  sprintf(sz, "%s planets and objects:\n", szAppName); PrintSz(sz);
  PrintSz("No. Name       Rulers   Detriments Exalt Fall ");
  PrintSz("Esoteric Hierarchical Ray\n\n");
  for (l = 0; l <= oNorm; l++) {
    i = rgobjList[l];
    if (ignore[i])
      continue;
    AnsiColor(kObjA[i]);
    sprintf(sz, "%2d %-12s", i, szObjDisp[i]); PrintSz(sz);

    // Print rulerships and exaltations for the planets.

    j = ruler1[i]; k = ruler2[i];
    if (j) {
      sprintf(sz, "%.3s", szSignName[j]); PrintSz(sz);
    } else
      PrintSz("   ");
    PrintSz(" ");
    if (k) {
      sprintf(sz, "%.3s", szSignName[k]); PrintSz(sz);
    } else
      PrintSz("   ");
    PrintSz("  ");
    if (j) {
      sprintf(sz, "%.3s", szSignName[Mod12(j+6)]); PrintSz(sz);
    } else
      PrintSz("   ");
    PrintSz(" ");
    if (k) {
      sprintf(sz, "%.3s", szSignName[Mod12(k+6)]); PrintSz(sz);
    } else
      PrintSz("   ");
    PrintTab(' ', 4);

    j = exalt[i];
    if (j) {
      sprintf(sz, "%.3s", szSignName[j]); PrintSz(sz);
    } else
      PrintSz("   ");
    PrintSz("   ");
    if (j) {
      sprintf(sz, "%.3s", szSignName[Mod12(j+6)]); PrintSz(sz);
    } else
      PrintSz("   ");
    PrintSz("  ");

    j = rgObjEso1[i]; k = rgObjEso2[i];
    if (j) {
      sprintf(sz, "%.3s", szSignName[j]); PrintSz(sz);
    } else
      PrintSz("   ");
    PrintSz(" ");
    if (k) {
      sprintf(sz, "%.3s", szSignName[k]); PrintSz(sz);
    } else
      PrintSz("   ");
    PrintSz("  ");
    j = rgObjHie1[i]; k = rgObjHie2[i];
    if (j) {
      sprintf(sz, "%.3s", szSignName[j]); PrintSz(sz);
    } else
      PrintSz("   ");
    PrintSz(" ");
    if (k) {
      sprintf(sz, "%.3s", szSignName[k]); PrintSz(sz);
    } else
      PrintSz("   ");

    if (rgObjRay[i]) {
      PrintTab(' ', 6);
      sprintf(sz, "%d", rgObjRay[i]); PrintSz(sz);
    }
    PrintL();
  }

  // Now, if -U in effect, read in and display stars in specified order.

  if (us.nStar) {
    CastChart(0);
    for (i = starLo; i <= starHi; i++) if (!ignore[i]) {
      j = rgobjList[i];
      AnsiColor(kObjA[j]);
      sprintf(sz, "%2d %-12s", i, szObjDisp[j]); PrintSz(sz);
      sprintf(sz, "Star #%2d  ", i-oNorm); PrintSz(sz);
      PrintZodiac(planet[j]);
      PrintTab(' ', 3);
      PrintAltitude(planetalt[j]);
      AnsiColor(kObjA[j]);
      sprintf(sz, " %5.2f\n", rStarBright[j-oNorm]); PrintSz(sz);
    }
  }
  AnsiColor(kDefault);

#ifdef SWISSGRAPH
  // Print extra stars.
  if (gs.fAllStar) {
    PrintL();
    SwissComputeStar(0.0, NULL);
    for (i = 1; SwissComputeStar(is.T, &es); i++) {
      AnsiColor(es.ki != kDefault ? es.ki : KStar2A(es.mag));
      if (es.mag == rStarNot)
        es.mag = 99.99;
      sprintf(sz, "%4d %-8.8s ", i, es.pchDes); PrintSz(sz);
      PrintZodiac(es.lon);
      PrintCh(' ');
      PrintAltitude(es.lat);
      AnsiColor(es.ki != kDefault ? es.ki : KStar2A(es.mag));
      sprintf(sz, " %5.2f%s%s", es.mag, *es.pchNam ? " " : "", es.pchNam);
      PrintSz(sz);
      PrintL();
    }
  }

  // Print extra asteroids.
  if (!gs.fAllStar && gs.nAstLo > 0) {
    PrintL();
    SwissComputeAsteroid(0.0, NULL, fFalse);
    for (i = gs.nAstLo; SwissComputeAsteroid(is.T, &es, fFalse); i++) {
      AnsiColor(es.ki != kDefault ? es.ki : kDefault);
      sprintf(sz, "%6d ", i); PrintSz(sz);
      PrintZodiac(es.lon);
      PrintCh(' ');
      PrintAltitude(es.lat);
      sprintf(sz, " %s\n", es.sz); PrintSz(sz);
    }
  }
#endif
}


// Print out a list of all the aspects recognized by the program, and info
// about them: Their names, index numbers, degree angles, present orbs, and
// the description of their glyph. This gets displayed by the -HA switch.

void PrintAspects(void)
{
  char sz[cchSzDef], *pch;
  int i, iMax, i2;

  iMax = !us.fParallel ? cAspect : aOpp;
  sprintf(sz,
    "%s aspects:\nNo. Name         Abbrev. ", szAppName); PrintSz(sz);
  PrintSz("Angle    Orb          Description of glyph\n\n");
  for (i = 1; i <= iMax; i++) {
    i2 = !us.fParallel ? i : i + cAspect;
    AnsiColor(kAspA[i]);

    // Print aspect angle.
    sprintf(sz, "%2d %-15s(%s) %6.2f",
      i, szAspectDisp[i2], szAspectAbbrevDisp[i2], rAspAngle[i]);
    for (pch = sz; *pch; pch++)
      ;
    while (*(--pch) == '0')
      *pch = ' ';
    if (*pch == '.')
      *pch = ' ';
    PrintSz(sz);

    // Print aspect orb.
    sprintf(sz, " +/- %f", rAspOrb[i]);
    for (pch = sz; *pch; pch++)
      ;
    while (*(--pch) == '0')
      *pch = chNull;
    if (*pch == '.')
      *pch = chNull;
    PrintSz(sz);
    sprintf(sz, " degrees - %s\n", szAspectGlyphDisp[i2]); PrintSz(sz);
  }
  AnsiColor(kDefault);
}


// Print out a list of the 12 signs and houses of the zodiac, and their
// standard and traditional names, as done when the -HC switch is invoked.

void PrintSigns(void)
{
  char sz[cchSzDef];
  int i, j;

  sprintf(sz, "%s signs and houses:\n", szAppName); PrintSz(sz);
  PrintSz("Sign        English name     House  Traditional name");
  PrintTab(' ', 19); PrintSz("Ruler\n\n");
  for (i = 1; i <= cSign; i++) {
    AnsiColor(kSignA(i));
    sprintf(sz, "%-12sthe %-14s%2d%s  House of %-24s  %s\n",
      szSignName[i], szSignEnglish[i], i, szSuffix[i], szHouseTradition[i],
      szObjDisp[rules[i]]);
    PrintSz(sz);
  }
  AnsiColor(kDefault);

  // Print Rays, esoteric rulerships, and exaltations of signs.
  PrintSz("\nNo. Sign        Rays  Exoteric   Esoteric   Hierarchical "
    "Exalt Detriment  Fall\n\n");
  for (i = 1; i <= cSign; i++) {
    AnsiColor(kSignA(i));
    sprintf(sz, "%2d  %-11s%5d ", i, szSignName[i], rgSignRay[i]);
    PrintSz(sz);

    for (j = 1; j <= oNorm; j++)
      if (ruler1[j] == i || ruler2[j] == i) {
        sprintf(sz, " %-4.4s", szObjDisp[j]); PrintSz(sz); break;
      }
    if (j > oNorm) PrintSz("     ");
    for (j++; j <= oPlu; j++)
      if (ruler1[j] == i || ruler2[j] == i) {
        sprintf(sz, " %-4.4s", szObjDisp[j]); PrintSz(sz); break;
      }
    if (j > oPlu) PrintSz("     ");
    PrintSz(" ");
    for (j = 0; j <= oNorm; j++)
      if (rgObjEso1[j] == i && !FCusp(j)) {
        sprintf(sz, " %-4.4s", szObjDisp[j]); PrintSz(sz); break;
      }
    if (j > oNorm) PrintSz("     ");
    for (j = 0; j <= oNorm; j++)
      if (rgObjEso2[j] == i && !FCusp(j)) {
        sprintf(sz, " %-4.4s", szObjDisp[j]); PrintSz(sz); break;
      }
    if (j > oNorm) PrintSz("     ");
    PrintSz(" ");
    for (j = 0; j <= oNorm; j++)
      if (rgObjHie1[j] == i && !FCusp(j)) {
        sprintf(sz, " %-4.4s", szObjDisp[j]); PrintSz(sz); break;
      }
    if (j > oNorm) PrintSz("     ");
    for (j = 0; j <= oNorm; j++)
      if (rgObjHie2[j] == i && !FCusp(j)) {
        sprintf(sz, " %-4.4s", szObjDisp[j]); PrintSz(sz); break;
      }
    if (j > oNorm) PrintSz("     ");

    PrintSz("   ");
    for (j = 0; j <= oNorm; j++)
      if (exalt[j] == i && (FBetween(j, oSun, oPlu) || !ignore[j])) {
        sprintf(sz, " %-4.4s", szObjDisp[j]); PrintSz(sz); break;
      }
    if (j > oNorm) PrintSz("     ");

    PrintSz(" ");
    for (j = 1; j <= oNorm; j++)
      if (Mod12(ruler1[j]+6) == i || (ruler2[j] && Mod12(ruler2[j]+6) == i)) {
        sprintf(sz, " %-4.4s", szObjDisp[j]); PrintSz(sz); break;
      }
    if (j > oNorm) PrintSz("     ");
    for (j++; j <= oPlu; j++)
      if (Mod12(ruler1[j]+6) == i || (ruler2[j] && Mod12(ruler2[j]+6) == i)) {
        sprintf(sz, " %-4.4s", szObjDisp[j]); PrintSz(sz); break;
      }
    if (j > oPlu) PrintSz("     ");
    PrintSz(" ");
    for (j = 0; j <= oNorm; j++)
      if (exalt[j] && Mod12(exalt[j]+6) == i &&
        (FBetween(j, oSun, oPlu) || !ignore[j])) {
        sprintf(sz, " %-4.4s", szObjDisp[j]); PrintSz(sz); break;
      }
    PrintL();
  }
  AnsiColor(kDefault);
}


#ifdef CONSTEL
// Given the standard 'noun' form of a constellation string, convert it to
// its genitive or posessive form. Some standard rules are used but a special
// instructions string is passed for special cases.

CONST char *GetSzGenitive(char *szGen, CONST char *szInst)
{
  char *pch, ch1, ch2;
  int cch;

  pch = szGen + CchSz(szGen);
  if (*szInst == ' ')            // Instructions starting with a space or
    szInst++;                    // that are empty means no special case.
  else if (*szInst) {
    cch = *szInst - '0';
    if (cch < 10) {          // Instructions starting with a number means
      szInst++;              // hack off that many characters of string.
      pch -= cch;
    }
    while (*szInst > '9')    // Instructions end with characters to append.
      *pch++ = *szInst++;
    *pch = chNull;
    return szInst;
  }
  ch1 = *(pch-1);
  ch2 = *(pch-2);
  if (ch1 == 'a') {           // Standard rule: 'a' ending -> 'ae'.
    *pch++ = 'e';
    *pch = chNull;
  } else if (ch1 == 's' && ch2 == 'u') {     // 'us' ending -> 'i'.
    *(pch-2) = 'i';
    *(pch-1) = chNull;
  } else if (ch1 == 'm' && ch2 == 'u') {     // 'um' ending -> 'i'.
    *(pch-2) = 'i';
    *(pch-1) = chNull;
  } else if (ch1 == 'x')     // Standard rule: 'x' ending -> 'cis'.
    sprintf(pch-1, "cis");
  return szInst;
}


// Given a constellation index, fill out a string with the genitive or
// posessive form of its name. This basically just calls GetSzGenitive()
// above, however it has to be called twice for names having two words.

void GetSzConstel(char *szGen, int i)
{
  char sz1[cchSzDef], sz2[cchSzDef], *pchSpace;
  CONST char *szInst;

  sprintf(szGen, "%s", szCnstlName[i]);
  for (pchSpace = szGen; *pchSpace && *pchSpace != ' '; pchSpace++)
    ;
  szInst = szCnstlGenitive[i];
  if (*pchSpace == chNull) {
    GetSzGenitive(szGen, szInst);
    return;
  }
  *pchSpace = chNull;
  if (szInst[0] == '!') {
    GetSzGenitive(szGen, szInst+1);
    return;
  }
  sprintf(sz1, "%s", szGen);
  sprintf(sz2, "%s", pchSpace+1);
  szInst = GetSzGenitive(sz1, szInst);
  GetSzGenitive(sz2, szInst);
  sprintf(szGen, "%s %s", sz1, sz2);
}


// Print out a list of the 88 constellations used in astronomy, in their
// standard, English, and genitive forms, as invoked with the -HF switch.

void PrintConstellations(void)
{
  int i, j = eWat;
  char szGen[cchSzDef], sz[cchSzDef], chLast = chNull;

  sprintf(sz, "%s constellations:\n", szAppName); PrintSz(sz);
  PrintSz("No. Name                Abbrev.   "
    "Meaning            Genitive form\n\n");
  for (i = 1; i <= cCnstl; i++) {
    if (szCnstlName[i][0] != chLast) {
      chLast = szCnstlName[i][0];
      j = (j + 1) & 3;
      AnsiColor(kElemA[j]);
    }
    sprintf(sz, "%2d: %-19s (%s) ", i, szCnstlName[i], szCnstlAbbrev[i]);
    PrintSz(sz);
    if (szCnstlMeaning[i][0] == ' ')
      sprintf(sz, "%-22s", szCnstlMeaning[i]+1);
    else
      sprintf(sz, "the %-18s", szCnstlMeaning[i]);
    PrintSz(sz);
    GetSzConstel(szGen, i);
    sprintf(sz, " (%s)\n", szGen); PrintSz(sz);
  }
  AnsiColor(kDefault);
}
#endif // CONSTEL


// Print out a list of the planets in the solar system (and our Moon),
// listing astronomical info on them, as invoked with the -HS switch.

void PrintOrbit(void)
{
  char sz[cchSzDef];
  int i, j;
  real r;

  // Fill out star data arrays if necessary.
  if (us.nStar)
    CastChart(0);

  // Loop over each object and print its data.
  sprintf(sz, "%s planets:\n", szAppName); PrintSz(sz);
  PrintSz("   Name: Distance   Year Diameter     Day       Mass Density  "
    "Axis Satellites\n\n");
  for (i = 0; i <= is.nObj; i++) {
    if (ignore[i] || !FThing(i))
      continue;
    j = rgobjList[i];
    AnsiColor(kObjA[j]);
    sprintf(sz, "%7.7s: %8.4f", szObjDisp[j],
      j < starLo ? rObjDist[j]/rObjDist[oEar] : rStarDist[j-oNorm]/rLYToAU);
    PrintSz(sz);
    if (j < starLo) {
      if (rObjYear[j] < 1000.0)
        sprintf(sz, " %6.2f", rObjYear[j]);
      else
        sprintf(sz, " %6.0f", rObjYear[j]);
      PrintSz(sz);
      if (j <= oVes || j >= dwarfLo) {
        if (!us.fEuroDist)
          sprintf(sz, " %8.4f", rObjDiam[j]/rObjDiam[oEar]);
        else {
          if (rObjDiam[j] < 1000000.0)
            sprintf(sz, " %8.1f", rObjDiam[j]);
          else
            sprintf(sz, " %8.0f", rObjDiam[j]);
        }
        PrintSz(sz);
        sprintf(sz, " %7.2f", rObjDay[j]); PrintSz(sz);
        if (j <= oPlu) {
          r = rObjDiam[j]*1000.0/2.0;
          sprintf(sz, " %10.3f %7.2f %5.2f %10d", rObjMass[j],
            (rObjMass[j]*5.974E24/1000.0)/((4.0/3.0)*(r*r*r)*rPi),
            rObjAxis[j], cSatellite[j]);
          PrintSz(sz);
        }
      }
    }
    PrintL();
  }
  AnsiColor(kDefault);
}


// Print out a list of the esoteric Rays, their names and keywords, and the
// signs and planets that map to each Ray, as invoked with the -H7 switch.

void PrintRay()
{
  char sz[cchSzDef];
  int i, j, c;

  EnsureRay();
  sprintf(sz, "%s Rays:\nRay  Name", szAppName); PrintSz(sz);
  PrintTab(' ', 24);
  PrintSz("Will to    Signs       Slice  Planets\n\n");
  for (i = 1; i <= cRay; i++) {
    AnsiColor(kRayA[i]);
    sprintf(sz, "%d  %-28s  %-9s ", i, szRayName[i], szRayWill[i]);
    PrintSz(sz);
    c = 0;
    for (j = 1; j <= cSign; j++)
      if (rgSignRay2[j][i]) {
        sprintf(sz, " %3.3s", szSignName[j]); PrintSz(sz);
        c += rgSignRay2[j][i];
      }
    sprintf(sz, "%6.2f ", (real)c / 420.0); PrintSz(sz);
    for (j = 0; j <= oNorm; j++) {
      // Dwarf and other minor body Rays are only included if unrestricted.
      if (rgObjRay[j] == i && !FCusp(j) &&
        (j <= oPlu || j == oVul || !ignore[j])) {
        sprintf(sz, " %-4.4s", szObjDisp[j]); PrintSz(sz);
      }
    }
    PrintL();
  }
  AnsiColor(kDefault);
}


#ifdef ARABIC
// Compare the strings corresponding to two Arabic part formulas. Like
// NCompareSz(), this returns 0 if they are equal, a positive value if the
// first is greater, and negative if the second is greater.

int NCompareSzPart(int ap1, int ap2)
{
  char *pch1, *pch2;
  int ich;

  pch1 = ai[ap1].form; pch2 = ai[ap2].form;
  for (ich = 0; pch1[ich] && pch1[ich] == pch2[ich]; ich++) {
    if (!us.fArabicFlip) {

      // If formulas are being displayed in alternate form, we need to
      // effectively swap two sections in the string and then compare.

      if (ich == 2)
        ich = 5;
      else if (ich == 8)
        ich = 2;
      else if (ich == 5)
        ich = 8;
    }
  }
  return pch1[ich] - pch2[ich];
}


// Print out a list of all the Arabic parts in the current chart, computing
// their positions first, as brought up with the -P switch.

void DisplayArabic(void)
{
  real rPart[cPart], rBit[3], rCur;
  char sz[cchSzDef], *pch, ch;
  int iPart[cPart], h, i, j, k, l;

  PrintSz("Num."); PrintTab(' ', 20); PrintSz("Name Position");
  PrintTab(' ', 1 + 4 * is.fSeconds);
  PrintSz("House Formula              Flip Type\n");

  // Calculate the zodiac positions of all the parts.

  for (i = 0; i < cPart; i++) {
    rPart[i] = -rDegMax;
    if (i >= us.nArabicParts)
      goto LNextPart;
    for (j = 0; j < 3; j++) {
      pch = &ai[i].form[j*3];
      ch = pch[1];
      if (ch == ' ')
        k = oAsc;
      else if (ch == 'F')
        k = -apFor;
      else if (ch == 'S')
        k = -apSpi;
      else
        k = (ch-'0') * 10 + (pch[2]-'0');
      ch = *pch;
      if (ch == 'h')       // House cusp
        rCur = chouse[k];
      else if (ch == 'r')  // Ruler of house cusp
        rCur = planet[rules[SFromZ(chouse[k])]];
      else if (ch == 'j')  // 10 degrees after house cusp
        rCur = chouse[k] + 10.0;
      else if (ch == 'H')  // House cusp planet lies within
        rCur = chouse[inhouse[k]];
      else if (ch == 'R')  // Ruler of planet's house
        rCur = planet[rules[SFromZ(chouse[inhouse[k]])]];
      else if (ch == 'D')  // Dispositor / ruler of planet's position
        rCur = planet[rules[SFromZ(planet[k])]];
      else if (FBetween(ch, '0', '3'))
        rCur = (real)((ch-'0') * 100 + k);
      else {
        if (k < 1) {
          rCur = rPart[-k];
          if (rCur < 0.0)
            goto LNextPart;
        } else {
          if (ignore[k] && (us.fCusp || !FCusp(k)))
            goto LNextPart;
          else
            rCur = planet[k];
        }
      }
      rBit[j] = rCur;
    }
    rCur = rBit[1] - rBit[2];
    if (us.nArabicNight < 0 || (ai[i].form[9] == 'F' &&
      inhouse[oSun] < sLib && us.nArabicNight == 0))
      neg(rCur);
    rCur = Mod(rCur + rBit[0]);
    rPart[i] = rCur;
LNextPart:
    iPart[i] = i;
  }

  // Sort parts to figure out what order to display them in.

  if (us.nArabic > 1) for (i = 1; i < cPart; i++) {
    j = i-1;

    // Compare part zodiac locations for -Pz switch.
    if (us.nArabic == 'z') while (j >= 0 &&
      rPart[iPart[j]] > rPart[iPart[j+1]]) {
      SwapN(iPart[j], iPart[j+1]);
      j--;

    // Compare part names for -Pn switch.
    } else if (us.nArabic == 'n') while (j >= 0 && NCompareSz(
      ai[iPart[j]].name, ai[iPart[j+1]].name) > 0) {
      SwapN(iPart[j], iPart[j+1]);
      j--;

    // Compare part formulas for -Pf switch.
    } else if (us.nArabic == 'f') while (j >= 0 && NCompareSzPart(
      iPart[j], iPart[j+1]) > 0) {
      SwapN(iPart[j], iPart[j+1]);
      j--;
    }
  }

  // Display the positions and formulas of the parts.

  for (h = i = 0; i < cPart; i++) {
    l = iPart[i];
    if (rPart[l] < 0.0)
      continue;
#ifdef EXPRESS
    // Skip this part if AstroExpression says to do so.
    if (!us.fExpOff && FSzSet(us.szExpPart)) {
      ExpSetN(iLetterY, l);
      ExpSetR(iLetterZ, rPart[l]);
      if (!NParseExpression(us.szExpPart))
        continue;
      rPart[l] = Mod(RExpGet(iLetterZ));
    }
#endif
    sprintf(sz, "%3d: %23.23s ", ++h, ai[l].name); PrintSz(sz);
    PrintZodiac(rPart[l]);
    j = NHousePlaceIn2D(rPart[l]);
    sprintf(sz, " [%2d%s] ", j, szSuffix[j]);
    AnsiColor(kSignA(j)); PrintSz(sz); AnsiColor(kDefault);
    PrintCh('(');
    for (j = 0; j < 3; j++) {
      k = j < 1 || us.fArabicFlip ? j : 3-j;
      pch = &ai[l].form[k*3];
      ch = pch[1];
      if (ch == ' ')
        k = oAsc;
      else if (ch == 'F')
        k = -apFor;
      else if (ch == 'S')
        k = -apSpi;
      else
        k = (ch-'0') * 10 + (pch[2]-'0');
      ch = *pch;
      if (k < 1) {
        AnsiColor(kObjA[oFor]);
        sprintf(sz, "%3.3s", ai[-k].name); PrintSz(sz);
      } else {
        if (ch == ' ' || ch == 'H' || ch == 'R' || ch == 'D') {
          AnsiColor(kSignA(ruler1[k]));
          sprintf(sz, "%.3s", szObjDisp[k]);
        } else if (FBetween(ch, '0', '3')) {
          k = (ch-'0') * 100 + k;
          AnsiColor(kSignA(k/30+1));
          sprintf(sz, "%2d%.3s", k%30, szSignName[k/30+1]);
        } else {
          AnsiColor(kSignA(k));
          sprintf(sz, "%3d", k);
        }
        PrintSz(sz);
      }
      AnsiColor(kDefault);
      if (sz[3] == chNull) {
        PrintCh(' ');
        switch (ch) {
        case 'h': ch = ' '; break;  // House cusp
        case 'r': ch = 'R'; break;  // Ruler of house cusp
        case 'j': ch = '+'; break;  // 10 degrees after house cusp
        default: ch = *pch;
        }
        PrintCh(ch);
      }
      if (j < 2) {
        sprintf(sz, " %c ", (j < 1 == us.fArabicFlip) ? '+' : '-');
        PrintSz(sz);
      }
    }
    PrintCh(' ');
    ch = ai[l].form[9];
    switch (ch) {
    case 'F': PrintSz("Y"); break;
    default: AnsiColor(kWhiteA); PrintSz("N"); AnsiColor(kDefault); break;
    }
    PrintSz(") ");
    ch = ai[l].form[10];
    switch (ch) {
    case 'C': AnsiColor(kElemA[eWat]); PrintSz("Comm."); break;
    case 'E': AnsiColor(kElemA[eFir]); PrintSz("Event"); break;
    case 'H': AnsiColor(kElemA[eEar]); PrintSz("Hora."); break;
    }
    AnsiColor(kDefault);
    PrintL();

    // If the -ma switch is set, determine and display each aspect from one of
    // the planets to the current part, and the aspect's orb.

    if (us.fMidAspect)
      PrintAspectsToPoint(rPart[l], -1, 0.0, "Part at");
  }
}
#endif // ARABIC


#ifdef GRAPH
// Print a list of every key that one can press in a graphics window to do a
// certain function, and a description of what it does, as displayed when one
// presses the 'H' or '?' key, and as shown with the -HX switch.

void DisplayKeysX(void)
{
  char sz[cchSzDef];

  sprintf(sz, "%s graphics screen key press options (version %s):",
    szAppName, szVersionCore);
  PrintS(sz);
  PrintS(" Press '?' to display this list of key options.");
  PrintS(" Press 'p' to toggle pause status on or off.");
  PrintS(" Press 'x' to toggle foreground/background colors on screen.");
  PrintS(" Press 'm' to toggle color/monochrome display on screen.");
  PrintS(" Press 'i' to toggle status of the minor chart modification.");
  PrintS(" Press 't' to toggle header info on current chart on screen.");
  PrintS(" Press 'b' to toggle drawing of a border around the chart.");
  PrintS(" Press 'l' to toggle labeling of object points in chart.");
  PrintS(" Press 'k' to toggle labeling of glyphs on aspect lines.");
  PrintS(" Press 'j' to toggle not clearing screen between chart updates.");
  PrintS(" Press 'v' to display current chart positions on text screen.");
  PrintS(" Press 'R','C','u','y','`','~','U' to toggle restriction of minor");
  PrintS(
    "    objects, cusps, Uranians, Dwarfs, moons, body centers, and stars.");
  PrintS(" Press 'c' to toggle relationship comparison chart mode.");
  PrintS(" Press 's', 'h', 'a', 'f', 'g', 'z', to toggle status of sidereal");
  PrintS("    zodiac, heliocentric charts, 3D houses, domal charts, decan");
  PrintS("    charts, and vedic format wheel charts.");
  PrintS(" Press 'O' and 'o' to recall/store a previous chart from memory.");
#ifdef X11
  PrintS(" Press 'B' to save current window contents to root background.");
#else
  PrintS(" Press 'B' to resize chart display to full size of screen.");
#endif
  PrintS(" Press 'Q' to resize chart display to a square.");
  PrintS(" Press '<' and '>' to decrease/increase the scale size of the");
  PrintS("    glyphs and the size of world map.");
  PrintS(" Press '[' and ']' to decrease/increase tilt in globe display.");
  PrintS(" Press '{' and '}' to rotate left/right one degree in globe.");
  PrintS(" Press '+' and '-' to add/subtract a day from current chart.");
#ifdef TIME
  PrintS(" Press 'n' to set chart information to current time now.");
#endif
  PrintS(" Press 'N' to toggle animation status on or off. Charts will");
  PrintS("    be updated to current status and globes will rotate.");
  PrintS(" Press '!'-'(' to begin updating current chart by adding times.");
  PrintS("    !: seconds, @: minutes, #: hours, $: days, %: months,");
  PrintS("    ^: years, &: decades, *: centuries, (: millennia.");
  PrintS(" Press 'r' to reverse direction of time-lapse or animation.");
  PrintS(" Press '1'-'9' to set rate of animation to 'n' degrees, etc.");
  PrintS(" Press 'V','A','Z','S','H','K','J','L','E','X','W','G','P','T' to");
  PrintS(
    "    switch to normal (_v), grid (_g), local horizon (_Z), space (_S),");
  PrintS(
    "    sector (_l), calendar (_K), dispositor (_j), astro-graph (_L),");
  PrintS(
    "    ephemeris (_E), chart sphere (_XX), world map (_XW), globe (_XG),");
  PrintS("    polar (_XP), and telescope (_XZ) modes.");
  PrintS(" Press 'Y' to switch to biorhythm relationship chart mode.");
  PrintS(" Press '0' to toggle between _Z,_Z0 & _XW,_XW0 & _E,_Ey modes.");
#ifdef CONSTEL
  PrintS(" Press 'F' to toggle between world and constellation map modes.");
#endif
  PrintS(" Press 'd' to toggle display of house information in map modes.");
  PrintS(" Press 'e' to toggle display of Earth's equator in map modes.");
  PrintS(" Press 'w' to toggle drawing world maps in detailed bitmap mode.");
  PrintS(" Press 'space' to force redraw of current graphics display.");
  PrintS(" Press 'del' to clear the graphics screen and not redraw.");
  PrintS(" Press 'enter' to input a command line of program switches.");
  PrintS(" Press 'escape' to close graphics window and exit program.");
#ifdef X11
  PrintL();
  PrintS(" Left   mouse button: Scribble lines over chart in window.");
  PrintS(" Middle mouse button: Print coordinates of pointer on world map.");
  PrintS(" Right  mouse button: Close window and exit the program.");
#endif
#ifdef WINANY
  PrintL();
  PrintS(" Left  mouse button: Scribble lines over chart in window.");
  PrintS(" Right mouse button: Drag maps or show chart's context menu.");
#endif
}


// Print a list of every command switch dealing with the graphics features
// that can be passed to the program, and a description of what it does.
// This is part of what the -H switch prints, if graphics were compiled in.

void DisplaySwitchesX(void)
{
  PrintS(" _X: Create a graphics chart instead of displaying it as text.");
#ifdef ISG
  PrintS(" _Xb: Create bitmap file instead of putting graphics on screen.");
#endif
  PrintS(" _Xb[n,c,v,a,b,w]: Set bitmap file output mode to X11 normal,");
  PrintS("  X11 compacted, X11 very compact, Ascii (bmtoa), Windows bitmap");
  PrintS("  compact (16 color palette), or Windows bitmap (24 bit colors).");
#ifdef PS
  PrintS(" _Xp: Create PostScript vector graphic instead of bitmap file.");
  PrintS(" _Xp0: Like _Xp but create complete instead of encapsulated file.");
#endif
#ifdef META
  PrintS(" _XM[0]: Create Windows metafile vector graphic instead of bitmap.");
#endif
#ifdef WIRE
  PrintS(" _X3: Create Daedalus wireframe vector file instead of bitmap.");
#endif
  PrintS(" _Xo <file>: Write output bitmap or graphic to specified file.");
#ifdef X11
  PrintS(" _XB: Display X chart on root instead of in a separate window.");
#endif
  PrintS(" _XI <file>: Display bitmap as background behind graphics charts.");
  PrintS(
    " _XI0 <trans> <pos>: Set transparency and positioning of background.");
  PrintS(" _XIW <file>: Replace world map bitmap used for world map charts.");
  PrintS(" _Xm: Create monochrome graphic instead of one in color.");
  PrintS(" _Xr: Create chart graphic in reversed colors (white background).");
#ifdef X11
  PrintS(" _Xw <hor> [<ver>], _ge[..]: Change the size of chart graphic.");
#else
  PrintS(" _Xw <hor> [<ver>]: Change the size of chart graphic.");
#endif
  PrintS(" _Xs <100,200,300,400>: Change the size of map or characters by %.");
  PrintS(
    " _XS <100,150,200,300,400>: Change size of graphics chart text by %.");
  PrintS(" _XQ: Ensure square charts remain so regardless of bitmap size.");
  PrintS(" _Xi: Create chart graphic in slightly modified form.");
  PrintS(" _Xt: Inhibit display of chart info at bottom of graphic.");
  PrintS(" _Xu: Inhibit display of a border around graphic.");
  PrintS(" _Xx: Draw thicker lines in graphics charts.");
  PrintS(" _Xl: Inhibit labeling of object points in chart graphic.");
  PrintS(" _XA: Draw aspect glyphs over aspect lines in charts.");
  PrintS(" _Xj: Don't clear screen between chart updates, drawing trails.");
  PrintS(" _Xe: Draw Earth's equator in certain charts.");
  PrintS(" _XU: Draw all stars from sefstars.txt file in certain charts.");
  PrintS(
    " _XU[0-3]: Like _XU but set whether to show larger star dot and name.");
  PrintS(" _XE <low> <high>: Draw range of asteroids in certain charts.");
  PrintS(
    " _XE[0-3] [..]: Like _XE but set whether to label ast number and name.");
#ifdef ATLAS
  PrintS(" _XL: Plot city locations from atlas on world map.");
  PrintS(" _XL[1-5]: Like _XL but set how to color cities (when _XA is on).");
#endif
  PrintS(" _XC: Draw house boundaries or alternate info in certain charts.");
  PrintS(" _X1 <object>: Rotate wheel charts so object is at left edge.");
  PrintS(" _X2 <object>: Rotate wheel charts so object is at top edge.");
#ifdef X11
  PrintS(" _Xd <name>, _di[..] <name>: Open X window on specified display.");
#endif
  PrintS(" _Xv <0-2>: Set fill style for wedge areas in wheel charts.");
  PrintS(
    " _XX[0] [<degrees> [<degrees>]]: Display chart sphere instead of wheel.");
  PrintS(" _XW: Simply display an image of the world map.");
  PrintS(" _XW0: Like _XW but do a non-rectangular Mollewide projection.");
  PrintS(
    " _XG[0] [<degrees> [<degrees>]]: Display image of world as a globe.");
  PrintS(
    " _XP[0] [<degrees>]: Like _XG but create globe from a polar projection.");
  PrintS(" _XZ [<object>]: Display telescope chart zoomed in on part of sky.");
#ifdef CONSTEL
  PrintS(" _XF: Display maps as constellations on the celestial sphere.");
#endif
#ifdef ISG
  PrintS(" _Xn [<mode>]: Start up chart or globe display in animation mode.");
  PrintS(" _XN: Map animates chart time instead of rotating map itself.");
#endif
  PrintS(" _XM[1-6][0] <strings>: Define macro(s) to run when chart drawn.");
#ifdef ISG
  PrintS(" _HX: Display list of key press options for screen graphics.");
#endif
}


#ifdef WIN
// Print a list of every command switch dealing with the Windows features
// that can be passed to the program, and a description of what it does.
// This is part of what the -H switch prints in the MS Windows version.

void DisplaySwitchesW(void)
{
  PrintS("\nSwitches to access Windows options:");
  PrintS(" _W <value>: Run given Windows menu command internally.");
  PrintS(" _WN <1-32000>: Set animation update delay in milliseconds.");
  PrintS(" _WM <1-48> <text>: Set Windows menu text for macro command.");
  PrintS(" _Wn: Don't redraw screen until user forces update.");
  PrintS(" _Wh: Set hourglass cursor when redrawing chart.");
  PrintS(" _Wt: Don't display warning and error popup messages.");
  PrintS(" _Ww <hor> <ver>: Set upper left coordinates of window.");
  PrintS(" _WB <0-24> <0-24>: Set window scrollbar positions.");
  PrintS(" _WT <string>: Set title bar text of Astrolog window.");
  PrintS(" _Wo: Continually autosave graphics screen to bitmap file.");
  PrintS(" _Wo0: Continually autosave graphics screen to numbered files.");
  PrintS(" _Wo3: Autosave graphics screen to wireframe instead of bitmap.");
  PrintS(" _WSg: Setup Windows program group, for current user only.");
  PrintS(" _WSG: Setup Windows program group, for all users.");
  PrintS(" _WSd: Setup Windows desktop icon for program.");
  PrintS(" _WSx: Setup registering Windows file extensions for program.");
  PrintS(" _WSu: Unregister Windows file extensions for program.");
  PrintS(" _WZ: Treat program as screen saver, and exit next user input.");
}
#endif
#endif // GRAPH


// This is the dispatch procedure for all the generic table information
// routines, such as those displaying the -H switch list, the list of signs,
// objects, default interpretations, and so on not requiring a date or time.

flag FPrintTables(void)
{
#ifdef WIN
  if (us.fGraphics)
    return fFalse;
#endif
  if (us.fCredit) {
    DisplayCredits();
    is.fMult = fTrue;
  }
  if (us.fSwitch) {
    if (is.fMult)
      PrintL2();
    DisplaySwitches();
    is.fMult = fTrue;
  }
  if (us.fSwitchRare) {
    if (is.fMult)
      PrintL2();
    DisplaySwitchesRare();
    is.fMult = fTrue;
  }
#ifdef GRAPH
  if (us.fKeyGraph) {
    if (is.fMult)
      PrintL2();
    DisplayKeysX();
    is.fMult = fTrue;
  }
#endif
  if (us.fSign) {
    if (is.fMult)
      PrintL2();
    PrintSigns();
    is.fMult = fTrue;
  }
  if (us.fObject) {
    if (is.fMult)
      PrintL2();
    PrintObjects();
    is.fMult = fTrue;
  }
  if (us.fAspect) {
    if (is.fMult)
      PrintL2();
    PrintAspects();
    is.fMult = fTrue;
  }
#ifdef CONSTEL
  if (us.fConstel) {
    if (is.fMult)
      PrintL2();
    PrintConstellations();
    is.fMult = fTrue;
  }
#endif
  if (us.fOrbitData) {
    if (is.fMult)
      PrintL2();
    PrintOrbit();
    is.fMult = fTrue;
  }
  if (us.fRay) {
    if (is.fMult)
      PrintL2();
    PrintRay();
    is.fMult = fTrue;
  }
#ifdef INTERPRET
  if (us.fMeaning) {
    if (is.fMult)
      PrintL2();
    InterpretGeneral();
    InterpretAspectGeneral();
    is.fMult = fTrue;
  }
#endif

  // If we also already have enough information to generate a chart, then go
  // on and do so, else exit. (So things like "-H -i file" will work, but
  // things like just "-H" will print and exit right away.)

#ifndef WIN
  return is.fMult && !is.fHaveInfo;
#else
  return is.fMult;
#endif
}

/* charts0.cpp */
