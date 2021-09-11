/*
** Astrolog (Version 7.30) File: placalc2.cpp
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

#include "placalc.h"


#ifdef PLACALC
/*
** ---------------------------------------------------------------
** | Copyright Astrodienst AG and Alois Treindl, 1989,1991,1993  |
** | The use of this source code is subject to regulations made  |
** | by Astrodienst Zurich. The code is NOT in the public domain.|
** |                                                             |
** | This copyright notice must not be changed or removed        |
** | by any user of this program.                                |
** ---------------------------------------------------------------
**
** Important changes:
** 11-jun-93 revision 1.12: fixed error which affected Mercury between -2100
** and -3100 (it jumped wildly).
*/

#ifdef ASTROLOG
// Given an object index and a Julian Day time, get its zodiac and latitude
// position (planetary longitude and latitude) of the object and its velocity
// and distance from the Earth or Sun. This basically just calls the Placalc
// calculation function to actually do it, but as this is the one routine
// called from Astrolog, this is the one routine which has knowledge of and
// uses both Astrolog and Placalc definitions, and does things such as
// translation to Placalc indices and formats.

flag FPlacalcPlanet(int ind, real jd, flag fHelio,
  real *obj, real *objalt, real *dir, real *space, real *diry, real *dirz)
{
  int iobj, flag;
  REAL8 jd_ad, rlng, rrad, rlat, rspeed;

  // Don't compute object if it's the same as the central object.
  if (ind <= oSun && (ind == oSun) == fHelio)
    return fFalse;

  // Convert Astrolog object index to Placalc index.
  if (ind <= oSun)
    iobj = SUN;
  else if (ind <= oPlu)
    iobj = ind-1;
  else if (ind == oChi)
    iobj = CHIRON;
  else if (FBetween(ind, oCer, oVes))
    iobj = ind - oCer + CERES;
  else if (ind == oNod)
    iobj = us.fTrueNode ? TRUE_NODE : MEAN_NODE;
  else if (ind == oLil)
    iobj = LILITH;
  else
    return fFalse;

  // Actually compute object position here.
  jd_ad = jd - JUL_OFFSET;
  flag = fHelio ? CALC_BIT_SPEED | CALC_BIT_HELIO : CALC_BIT_SPEED;
  jd_ad += deltat(jd_ad);
  if (calc(iobj, jd_ad, flag, &rlng, &rrad, &rlat, &rspeed) == OK) {
    *obj    = rlng;
    *objalt = rlat;
    *dir    = rspeed;
    *space  = rrad;
    *diry   = 0.0;
    *dirz   = 0.0;
    return fTrue;
  }
  return fFalse;
}
#endif // ASTROLOG


/***********************************************************
** $Header$
**
** definition module for planetary elements
** and disturbation coefficients
** version HP-UX C  for new version with stored outer planets
** 31-jul-88
** by Alois Treindl
**
** ---------------------------------------------------------------
** | Copyright Astrodienst Zurich AG and Alois Treindl, 1989.    |
** | The use of this source code is subject to regulations made |
** | by Astrodienst Zurich. The code is NOT in the public domain.|
** |                                                             |
** | This copyright notice must not be changed or removed        |
** | by any user of this program.                                |
** ---------------------------------------------------------------
**
***********************************************************/

/************************************************************
externally accessible globals, defined as extern in placalc.h
************************************************************/

REAL8 meanekl, ekl, nut;
struct elements el[MARS + 1];

/*
** In the elements degrees were kept as the units for the constants. This
** requires conversion to radians, when the actual calculations are performed.
** This approach is not the most efficient, but safer for development.
** Constant conversion could be done by writing all degree constants with
** value * DEGTORAD
*/

#define TIDAL_26  TRUE  /* decide wheter to use new or old lunar tidal
term; a consistent system of delta t must be
used */
#define MOON_TEST_CORR FALSE  /* to include more lunar terms in longitude */

REAL8 ekld[4] = {23.452294, -46.845, -.0059, 0.00181};
/* ecliptic with epoch1900, Ekd(0..3) in basic */

struct eledata pd[MARS + 1] = {
{/*earth*/  1.00000023, 365.25636042, EPOCH1900,
99.696678,  .9856473354,  1.089,    0,
101.220833, 6189.03,  1.63,   0.012,
0.01675104, -0.00004180,  -0.000000126,
0,    0,    0,    0,
0,    0,    0},
/*
** note 29 June 88 by Alois: G.M.Clemence, Astronomical Journal
** vol.53,p. 178 (1948) gives a correction to the perihel motion
** of -4.78" T, giving 6184.25 for the linear Term above. We have
** not yet applied this correction. It has been used in APAE 22,4
** on the motion of mars and does make an official impression.
*/
{/*moon*/ 0.0025955307, 27.321661,  EPOCH1900,
# if ! TIDAL_26
/*
** values from Improved Lunar Ephemeris, corresponding to tidal
** term -22.44"/cy and  consistent with delta t ~ 29.949 T*T
*/
270.4341638,  13.176396526808121, -4.08,  0.0068,
# endif
# if TIDAL_26
/*
** new values from Morrison 1979, with tidal term -26"/cy as
** stated in A.E. 1986 onwards, consistent with delta t ~ 44.3 T*T
** correction: -1.54" + 2.33" T - 1.78" T*T
*/
270.4337361,  13.176396544528099, -5.86,  0.0068,
# endif
334.329556, 14648522.52,  -37.17,   -0.045,
0.054900489,  0,    0,
259.183275, -6962911.23,  7.48 ,    0.008,
5.145388889,  0,    0},
{/*mercury*/  .3870986, 87.969252,  EPOCH1900,
178.179078, 4.0923770233, 1.084,    0,
75.89969722,  5599.76,  1.061,    0,
0.20561421, .00002046,   -.000000030,
47.145944444, 4266.75,  .626,   0,
7.0028805555, 6.699,    -.066},
{/*venus*/  .72333162,  224.700726, EPOCH1900,
342.767053, 1.6021687039, 1.1148,   0,
130.16383333, 5068.93,  -3.515,   0,
0.00682069, -.00004774, .000000091,
75.7796472223,3239.46,  1.476,    0,
3.3936305555, 3.621,    .0035},
{/*mars*/ 1.5236914620, 686.9296097,  EPOCH1900,
/* These are the corrected elements by Ross */
293.74762778, .524071163814,  1.1184,   0,
334.21820278, 6626.73,  .4675,    -0.0043,
0.09331290, .000092064, -.000000077,
48.786441667, 2775.57,  -.005,    -0.0192,
1.85033333, -2.430,   .0454}
};

struct sdat _sd [SDNUM] = {
  114.50,   585.17493,
  109.856,  191.39977,
  148.031,  30.34583,
  284.716,  12.21794,
  114.508,  585.17656,
  -0.56,    359.99213,
  148.03,   30.34743,
  284.72,   12.2196,
  248.07,   1494.726615,
  359.44,   359.993595,
  109.86,   191.402867,
  148.02,   30.348930,
  114.503,  585.173715,
  359.444,  359.989285,
  148.021,  30.344620,
  284.716,  12.21669,
  148.0315, 30.34906264,
  284.7158, 12.22117085,
  220.1695, 4.284931111,
  291.8024, 2.184704167
};

REAL8 sa[SDNUM];

/*
** delta long = lampl * COS (lphase - arg) in seconds of arc
** delta rad  = rampl * COS (rphase - arg) in ninth place of log
** arg = j * sa (k) + i * ma (this planet)
** ma = mean anomaly
** sa = mean anomaly of disturbing planet, where this
** is taken from the aproximate value in sa[]
** For the COS (phase - arg) it is good enough to compute
** with 32 bit reals, because ampl and phase have only
** four to five significant digits.
** While saving constant space, it is costing execution time due
** to float/double conversions.
**
** In basic, all correction terms for sun, mercury, venus and mars
** were contained in one array K(0..142,0..6); Nk(N,0) contained
** the index of the first term of planet N and Nk(N,1) the number
** of terms for this planet. Here, we use a  0 in the first column
** kor.j to indicate the end of the table for a planet.
** K(*) was a basic INTEGER array, therefore the amplitudes and phases
** had to be expressed as
** K(i,2) = ampl. of longitude in 0.001 seconds of arc
** K(i,3) = phase of longitude in 0.01 degrees
** K(i,4) = ampl. of radius in 9th place of log
** K(i,5) = phase of radius in 0.01 degrees.
** Here we have converted the amplitude of long. to seconds of arc
** and the phases to degrees.
*/

struct kor earthkor[86+1] = {  /* 11-jul-88 all terms to 0.020" long */
  /* j i  lampl  lphase  rampl  rphase  k */
  -1, 1,  0.013, 243,    28,    335,    8,  /* mercury */
  -1, 3,  0.015, 357,    18,    267,    8,
  -1, 4,  0.023, 326,    5,     239,    8,
  -1, 0,  0.075, 296.6,  94,    205.0,  0,  /* venus */
  -1, 1,  4.838, 299.10, 2359,  209.08, 0,
  -1, 2,  0.074, 207.9,  69,    348.5,  0,
  -1, 3,  0.009, 249,    16,    330,    0,
  -2, 1,  .116,  148.90, 160,   58.40,  0,
  -2, 2,  5.526, 148.31, 6842,  58.32,  0,
  -2, 3,  2.497, 315.94, 869,   226.70, 0,
  -2, 4,  0.044, 311.4,  52,    38.8,   0,
  -3, 2,  0.013, 176,    21,    90,     0,
  -3, 3,  .666,  177.71, 1045,  87.57,  0,
  -3, 4,  1.559, -14.75, 1497,  255.25, 0,
  -3, 5,  1.024, 318.15, 194,   49.50,  0,
  -3, 6,  0.017, 315,    19,    43,     0,
  -4, 4,  .210,  206.20, 376,   116.28, 0,
  -4, 5,  .144,  195.40, 196,   105.20, 0,
  -4, 6,  .152,  -16.20, 94,    254.80, 0,
  -5, 5,  0.084, 235.6,  163,   145.4,  0,
  -5, 6,  0.037, 221.8,  59,    132.2,  0,
  -5, 7,  .123,  195.30, 141,   105.40, 0,
  -5, 8,  .154,  -.40,   26,    270.00, 0,
  -6, 6,  0.038, 264.1,  80,    174.3,  0,
  -6, 7,  0.014, 253,    25,    164,    0,
  -6, 8,  0.01,  230,    14,    135,    0,
  -6, 9,  0.014, 12,     12,    284,    0,
  -7, 7,  0.020, 294,    42,    203.5,  0,
  -7, 8,  0.006, 279,    12,    194,    0,
  -8, 8,  0.011, 322,    24,    234,    0,
  -8, 12, 0.042, 259.2,  44,    169.7,  0,
  -8, 14, 0.032, 48.8,   33,    138.7,  0,
  -9, 9,  0.006, 351,    13,    261,    0,
  1,  -1, .273,  217.70, 150,   127.70, 1,  /* mars */
  1,  0,  0.048, 260.3,  28,    347,    1,
  2,  -3, 0.041, 346,    52,    255.4,  1,
  2,  -2, 2.043, 343.89, 2057,  253.83, 1,
  2,  -1, 1.770, 200.40, 151,   295.00, 1,
  2,  0,  0.028, 148,    31,    234.3,  1,
  3,  -3, .129,  294.20, 168,   203.50, 1,
  3,  -2, .425,  -21.12, 215,   249.00, 1,
  4,  -4, 0.034, 71,     49,    339.7,  1,
  4,  -3, .500,  105.18, 478,   15.17,  1,
  4,  -2, .585,  -25.94, 105,   65.90,  1,
  5,  -4, 0.085, 54.6,   107,   324.6,  1,
  5,  -3, .204,  100.80, 89,    11.00,  1,
  6,  -5, 0.020, 186,    30,    95.7,   1,
  6,  -4, .154,  227.40, 139,   137.30, 1,
  6,  -3, .101,  96.30,  27,    188.00, 1,
  7,  -5, 0.049, 176.5,  60,    86.2,   1,
  7,  -4, .106,  222.70, 38,    132.90, 1,
  8,  -5, 0.052, 348.9,  45,    259.7,  1,
  8,  -4, 0.021, 215.2,  8,     310,    1,
  8,  -6, 0.010, 307,    15,    217,    1,
  9,  -6, 0.028, 298,    34,    208.1,  1,
  9,  -5, 0.062, 346,    17,    257,    1,
  10, -6, 0.019, 111,    15,    23,     1,
  11, -7, 0.017, 59,     20,    330,    1,
  11, -6, 0.044, 105.9,  9,     21,     1,
  13, -8, 0.013, 184,    15,    94,     1,
  13, -7, 0.045, 227.8,  5,     143,    1,
  15, -9, 0.021, 309,    22,    220,    1,
  17, -9, 0.026, 113,    0,     0,      1,
  1,  -2, .163,  198.60, 208,   112.00, 2,  /* jupiter */
  1,  -1, 7.208, 179.53, 7067,  89.55,  2,
  1,  0,  2.600, 263.22, 244,   -21.40, 2,
  1,  1,  0.073, 276.3,  80,    6.5,    2,
  2,  -3, 0.069, 80.8,   103,   350.5,  2,
  2,  -2, 2.731, 87.15,  4026,  -2.89,  2,
  2,  -1, 1.610, 109.49, 1459,  19.47,  2,
  2,  0,  0.073, 252.6,  8,     263,    2,
  3,  -3, .164,  170.50, 281,   81.20,  2,
  3,  -2, .556,  82.65,  803,   -7.44,  2,
  3,  -1, .210,  98.50,  174,   8.60,   2,
  4,  -4, 0.016, 259,    29,    170,    2,
  4,  -3, 0.044, 168.2,  74,    79.9,   2,
  4,  -2, 0.080, 77.7,   113,   347.7,  2,
  4,  -1, 0.023, 93,     17,    3,      2,
  5,  -2, 0.009, 71,     14,    343,    2,
  1,  -2, 0.011, 105,    15,    11,     3,  /* saturn */
  1,  -1, .419,  100.58, 429,   10.60,  3,
  1,  0,  .320,  269.46, 8,     -7.00,  3,
  2,  -2, .108,  290.60, 162,   200.60, 3,
  2,  -1, .112,  293.60, 112,   203.10, 3,
  3,  -2, 0.021, 289,    32,    200.1,  3,
  3,  -1, 0.017, 291,    17,    201,    3,
  ENDMARK
};

struct kor mercurykor[24+1] = {
  1,  -1, .711,  35.47,  491,  305.28, 4,
  2,  -3, .552,  161.15, 712,  71.12,  4,
  2,  -2, 2.100, 161.15, 2370, 71.19,  4,
  2,  -1, 3.724, 160.69, 899,  70.49,  4,
  2,  0,  .729,  159.76, 763,  250.00, 4,
  3,  -3, .431,  105.37, 541,  15.53,  4,
  3,  -2, 1.329, 104.78, 1157, 14.84,  4,
  3,  -1, .539,  278.95, 14,   282.00, 4,
  4,  -2, .484,  226.40, 234,  136.02, 4,
  5,  -4, .685,  -10.43, 849,  259.51, 4,
  5,  -3, 2.810, -10.14, 2954, 259.92, 4,
  5,  -2, 7.356, -12.22, 282,  255.43, 4,
  5,  -1, 1.471, -12.30, 1550, 77.75,  4,
  5,  0,  .375,  -12.29, 472,  77.70,  4,
  2,  -1, .443,  218.48, 256,  128.36, 5,
  4,  -2, .374,  151.81, 397,  61.63,  5,
  4,  -1, .808,  145.93, 13,   35.00,  5,
  1,  -1, .697,  181.07, 708,  91.38,  6,
  1,  0,  .574,  236.72, 75,   265.40, 6,
  2,  -2, .938,  36.98,  1185, 306.97, 6,
  2,  -1, 3.275, 37.00,  3268, 306.99, 6,
  2,  0,  .499,  31.91,  371,  126.90, 6,
  3,  -1, .353,  25.84,  347,  295.76, 6,
  2,  -1, .380,  239.87, 0,    0,      7,
  ENDMARK
};

struct kor venuskor[22+1] = {
  -1, 2,  .264,   -19.20, 175,  251.10, 8,
  -2, 5,  .361,   167.68, 55,   77.20,  8,
  1,  -1, 4.889,  119.11, 2246, 29.11,  9,
  2,  -2, 11.261, 148.23, 9772, 58.21,  9,
  3,  -3, 7.128,  -2.57,  8271, 267.42, 9,
  3,  -2, 3.446,  135.91, 737,  47.37,  9,
  4,  -4, 1.034,  26.54,  1426, 296.49, 9,
  4,  -3, .677,   165.32, 445,  75.70,  9,
  5,  -5, .330,   56.88,  510,  -33.36, 9,
  5,  -4, 1.575,  193.93, 1572, 104.21, 9,
  5,  -3, 1.439,  138.08, 162,  229.90, 9,
  6,  -6, .143,   84.40,  236,  -5.80,  9,
  6,  -5, .205,   44.20,  256,  314.20, 9,
  6,  -4, .176,   164.30, 70,   75.70,  9,
  8,  -5, .231,   180.00, 25,   75.00,  9,
  3,  -2, .673,   221.62, 717,  131.60, 10,
  3,  -1, 1.208,  237.57, 29,   149.00, 10,
  1,  -1, 2.966,  208.09, 2991, 118.09, 11,
  1,  0,  1.563,  268.31, 91,   -7.60,  11,
  2,  -2, .889,   145.16, 1335, 55.17,  11,
  2,  -1, .480,   171.01, 464,  80.95,  11,
  3,  -2, .169,   144.20, 250,  54.00,  11,
  ENDMARK
};

struct kor marskor[62+1] = {
  -1, 1,  .115,   65.84,  684,   156.14, 12,
  -1, 2,  .623,   246.03, 812,   155.77, 12,
  -1, 3,  6.368,  57.60,  556,   -32.06, 12,
  -1, 4,  .588,   57.24,  616,   147.28, 12,
  -2, 5,  .138,   39.18,  157,   309.39, 12,
  -2, 6,  .459,   217.58, 82,    128.10, 12,
  -1, -1, .106,   33.60,  141,   303.45, 13,
  -1, 0,  .873,   34.34,  1112,  304.05, 13,
  -1, 1,  8.559,  35.10,  6947,  304.45, 13,
  -1, 2,  13.966, 20.50,  2875,  113.20, 13,
  -1, 3,  1.487,  22.18,  1619,  112.38, 13,
  -1, 4,  .175,   22.46,  225,   112.15, 13,
  -2, 2,  .150,   18.96,  484,   266.42, 13,
  -2, 3,  7.355,  158.64, 6412,  68.62,  13,
  -2, 4,  4.905,  154.09, 1985,  244.70, 13,
  -2, 5,  .489,   154.39, 543,   244.50, 13,
  -3, 3,  .216,   111.06, 389,   21.10,  13,
  -3, 4,  .355,   110.64, 587,   19.17,  13,
  -3, 5,  2.641,  280.58, 2038,  190.60, 13,
  -3, 6,  .970,   276.06, 587,   6.75,   13,
  -3, 7,  .100,   276.20, 116,   6.40,   13,
  -4, 5,  .152,   232.48, 259,   142.60, 13,
  -4, 6,  .264,   230.47, 387,   139.75, 13,
  -4, 7,  1.156,  41.64,  749,   312.67, 13,
  -4, 8,  .259,   37.92,  205,   128.80, 13,
  -5, 8,  .172,   -8.99,  234,   260.70, 13,
  -5, 9,  .575,   164.48, 308,   74.60,  13,
  -6, 10, .115,   113.70, 145,   23.53,  13,
  -6, 11, .363,   285.69, 144,   196.00, 13,
  -7, 13, .353,   48.83,  85,    319.10, 13,
  -8, 15, 1.553,  170.14, 110,   81.00,  13,
  -8, 16, .148,   170.74, 154,   259.94, 13,
  -9, 17, .193,   293.70, 23,    22.80,  13,
  1,  -3, .382,   46.48,  521,   316.25, 14,
  1,  -2, 3.144,  46.78,  3894,  316.39, 14,
  1,  -1, 25.384, 48.96,  23116, 318.87, 14,
  1,  0,  3.732,  -17.62, 1525,  117.81, 14,
  1,  1,  .474,   -34.60, 531,   59.67,  14,
  2,  -4, .265,   192.88, 396,   103.12, 14,
  2,  -3, 2.108,  192.72, 3042,  102.89, 14,
  2,  -2, 16.035, 191.90, 22144, 101.99, 14,
  2,  -1, 21.869, 188.35, 16624, 98.33,  14,
  2,  0,  1.461,  189.66, 1478,  279.04, 14,
  2,  1,  .167,   191.04, 224,   280.81, 14,
  3,  -4, .206,   167.11, 338,   76.13,  14,
  3,  -3, 1.309,  168.27, 2141,  76.24,  14,
  3,  -2, 2.607,  228.41, 3437,  139.74, 14,
  3,  -1, 3.174,  207.20, 1915,  115.83, 14,
  3,  0,  .232,   207.78, 240,   298.06, 14,
  4,  -4, .178,   127.25, 322,   36.16,  14,
  4,  -3, .241,   200.69, 389,   110.02, 14,
  4,  -2, .330,   267.57, 413,   179.86, 14,
  4,  -1, .416,   221.88, 184,   128.17, 14,
  1,  -2, .155,   -38.20, 191,   231.58, 15,
  1,  -1, 1.351,  -34.10, 1345,  235.85, 15,
  1,  0,  .884,   288.05, 111,   39.90,  15,
  1,  1,  .132,   284.88, 144,   15.67,  15,
  2,  -2, .620,   35.15,  869,   305.30, 15,
  2,  -1, 1.768,  32.50,  1661,  302.51, 15,
  2,  0,  .125,   18.73,  103,   119.90, 15,
  3,  -2, .141,   47.59,  199,   318.06, 15,
  3,  -1, .281,   40.95,  248,   310.75, 15,
  ENDMARK
};

struct m45dat m45[NUM_MOON_CORR] = {
  /*    l,     l',  F,  D,      Long,    Lat,         Par),*/
  { 0,   0, 0,   4,    13.902,     14.06,  0.2607},
  { 0,   0, 0,   2,    2369.912,   2373.36,     28.2333},
  { 1,   0, 0,   4,     1.979,      6.98,  0.0433},
  { 1,   0, 0,   2,     191.953,    192.72,  3.0861},
  { 1,   0, 0,   0,   22639.500,  22609.1,     186.5398},
  { 1,   0, 0,  -2,   -4586.465,  -4578.13,     34.3117},
  { 1,   0, 0,  -4, -38.428,    -38.64,  0.6008},
  { 1,   0, 0,  -6,  -0.393,     -1.43,  0.0086},
  { 0,   1, 0,   4,  -0.289,     -1.59, -0.0053},
  { 0,   1, 0,   2, -24.420,    -25.10, -0.3000},
  { 0,   1, 0,   0,    -668.146,   -126.98,     -0.3997},
  { 0,   1, 0,  -2,    -165.145,   -165.06,      1.9178},
  { 0,   1, 0,  -4,  -1.877,     -6.46,  0.0339},
  { 0,   0, 0,   3,   0.403,     -4.01,  0.0023},
  { 0,   0, 0,   1,    -125.154,   -112.79,     -0.9781},
  { 2,   0, 0,   4,   0.213,      1.02,  0.0054},
  { 2,   0, 0,   2,  14.387,     14.78,  0.2833},
  { 2,   0, 0,   0, 769.016,    767.96, 10.1657},
  { 2,   0, 0,  -2,    -211.656,   -152.53,     -0.3039},
  { 2,   0, 0,  -4, -30.773,    -34.07,  0.3722},
  { 2,   0, 0,  -6,  -0.570,     -1.40,  0.0109},
  { 1,   1, 0,   2,  -2.921,    -11.75, -0.0484},
  { 1,   1, 0,   0,    -109.673,   -115.18,     -0.9490},
  { 1,   1, 0,  -2,    -205.962,   -182.36,      1.4437},
  { 1,  1,  0,  -4,  -4.391,     -9.66,  0.0673},
  { 1,  -1, 0,  4,    0.283,      1.53,  0.0060},
  { 1,  -1, 0,  2,   14.577,     31.70,  0.2302},
  { 1,  -1, 0,  0,  147.687,    138.76,  1.1528},
  { 1,  -1, 0,  -2,  28.475,     23.59, -0.2257},
  { 1,  -1, 0,  -4,   0.636,      2.27, -0.0102},
  { 0,  2,  0,  2,   -0.189,     -1.68, -0.0028},
  { 0,  2,  0,  0,   -7.486,     -0.66, -0.0086},
  { 0,  2,  0,  -2,  -8.096,    -16.35,  0.0918},
  { 0,  0,  2,  2,   -5.741,     -0.04, -0.0009},
  { 0,  0,  2,  0,  -411.608,    -0.2,      -0.0124},
  { 0,  0,  2,  -2, -55.173,    -52.14, -0.1052},
  { 0,  0,  2,  -4,   0.025,     -1.67,  0.0031},
  { 1,  0,  0,  1,   -8.466,    -13.51, -0.1093},
  { 1,  0,  0,  -1,  18.609,      3.59,  0.0118},
  { 1,  0,  0,  -3,   3.215,      5.44, -0.0386},
  { 0,  1,  0,  1,   18.023,     17.93,  0.1494},
  { 0,  1,  0,  -1,   0.560,      0.32, -0.0037},
  { 3,  0,  0,  2,    1.060,      2.96,  0.0243},
  { 3,  0,  0,  0,   36.124,     50.64,  0.6215},
  { 3,  0,  0,  -2, -13.193,    -16.40, -0.1187},
  { 3,  0,  0,  -4,  -1.187,     -0.74,  0.0074},
  { 3,  0,  0,  -6,  -0.293,     -0.31,  0.0046},
  { 2,  1,  0,  2,   -0.290,     -1.45, -0.0051},
  { 2,  1,  0,  0,   -7.649,    -10.56, -0.1038},
  { 2,  1,  0,  -2,  -8.627,     -7.59, -0.0192},
  { 2,  1,  0,  -4,  -2.740,     -2.54,  0.0324},
  { 2,  -1, 0,  2,    1.181,      3.32,  0.0213},
  { 2,  -1, 0,  0,    9.703,     11.67,  0.1268},
  { 2,  -1, 0,  -2,  -2.494,     -1.17, -0.0017},
  { 2,  -1, 0,  -4,   0.360,      0.20, -0.0043},
  { 1,  2,  0,  0,   -1.167,     -1.25, -0.0106},
  { 1,  2,  0,  -2,  -7.412,     -6.12,  0.0484},
  { 1,  2,  0,  -4,  -0.311,     -0.65,  0.0044},
  { 1,  -2, 0,  2,    0.757,      1.82,  0.0112},
  { 1,  -2, 0,  0,    2.580,      2.32,  0.0196},
  { 1,  -2, 0,  -2,   2.533,      2.40, -0.0212},
  { 0,  3,  0,  -2,  -0.344,     -0.57,  0.0036},
  { 1,  0,  2,  2,   -0.992,     -0.02,  0},
  { 1,  0,  2,  0,  -45.099,     -0.02, -0.0010},
  { 1,  0,  2,  -2,  -0.179,     -9.52, -0.0833},
  { 1,  0,  -2, 2,   -6.382,     -3.37, -0.0481},
  { 1,  0,  -2, 0,   39.528,     85.13, -0.7136},
  { 1,  0,  -2, -2,   9.366,      0.71,     -0.0112},
  { 0,  1,  2,  0,    0.415,      0.10,    0.0013},
  { 0,  1,  2,  -2,  -2.152,     -2.26, -0.0066},
  { 0,  1,  -2, 2,   -1.440,     -1.30,  0.0014},
  { 0,  1,  -2, -2,   0.384,      0.0,   0.0},
  { 2,  0,  0,  1,   -0.586,     -1.20, -0.0100},
  { 2,  0,  0,  -1,   1.750,      2.01,  0.0155},
  { 2,  0,  0,  -3,   1.225,      0.91,     -0.0088},
  { 1,  1,  0,  1,    1.267,      1.52,  0.0164},
  { 1,  -1, 0,  -1,  -1.089,      0.55,      0},
  { 0,  0,  2,  -1,   0.584,      8.84,  0.0071},
  { 4,  0,  0,  0,    1.938,      3.60,  0.0401},
  { 4,  0,  0,  -2,  -0.952,     -1.58, -0.0130},
  { 3,  1,  0,  0,   -0.551,      0.94, -0.0097},
  { 3,  1,  0,  -2,  -0.482,     -0.57, -0.0045},
  { 3,  -1, 0,  0,    0.681,      0.96,      0.0115},
  { 2,  0,  2,  0,   -3.996,      0,   0.0004},
  { 2,  0,  2,  -2,   0.557,     -0.75,     -0.0090},
  { 2,  0,  -2, 2,   -0.459,     -0.38, -0.0053},
  { 2,  0,  -2, 0,   -1.298,      0.74,      0.0004},
  { 2,  0,  -2, -2,   0.538,      1.14, -0.0141},
  { 1,  1,  -2, -2,   0.426,      0.07,     -0.0006},
  { 1,  -1, 2,  0,   -0.304,      0.03,  0.0003},
  { 1,  -1, -2, 2,   -0.372,     -0.19, -0.0027},
  { 0,  0,  4,  0,    0.418,      0,   0},
  { 2,  -1, 0,  -1,  -0.352,     -0.37,     -0.0028}
};

# if MOON_TEST_CORR
/* moon additional correction terms */
struct m5dat {
  REAL8 lng;
  int  i0,i1,i2,i3;
} m5[] = {
  /*    lng,  l,     l',  F,  D,      */
  0.127,  0,  0,  0,  6,
  -0.151, 0,  2,  0,  -4,
  -0.085, 0,  0,  2,  4,
  0.150,  0,  1,  0,  3,
  -0.091, 2,  1,  0,  -6,
  -0.103, 0,  3,  0,  0,
  -0.301, 1,  0,  2,  -4,
  0.202,  1,  0,  -2, -4,
  0.137,  1,  1,  0,  -1,
  0.233,  1,  1,  0,  -3,
  -0.122, 1,  -1, 0,  1,
  -0.276, 1,  -1, 0,  -3,
  0.255,  0,  0,  2,  1,
  0.254,  0,  0,  2,  -3,
  -0.100, 3,  1,  0,  -4,
  -0.183, 3,  -1, 0,  -2,
  -0.297, 2,  2,  0,  -2,
  -0.161, 2,  2,  0,  -4,
  0.197,  2,  -2, 0,  0,
  0.254,  2,  -2, 0,  -2,
  -0.250, 1,  3,  0,  -2,
  -0.123, 2,  0,  2,  2,
  0.173,  2,  0,  -2, -4,
  0.263,  1,  1,  2,  0,
  0.130,  3,  0,  0,  -1,
  0.113,  5,  0,  0,  0,
  0.092,  3,  0,  2,  -2,
  0,  99, 0,  0,  0 /* end mark */
};
# endif /* MOON_TEST_CORR */


/* solution of the Kepler equation, return rad */
/* t = mean anomaly in degrees                 */
/* ex = excentricity of orbit                  */
/* err = maximum error in degrees              */

REAL8 fnu(REAL8 t, REAL8 ex, REAL8 err)
{
  REAL8 u0, delta;

  t *= DEGTORAD;
  u0 = t;
  err *= DEGTORAD;
  delta = 1;
  while (ABS8(delta) >= err) {
    delta = (t + ex * SIN8(u0) - u0) / (1 - ex * COS8(u0));
    u0 += delta;
  }
  return u0;
}


/* x MOD 360.0, so that x in 0..360 */

REAL8 smod8360(REAL8 x)
{
  while (x >= 360.0)
    x -= 360.0;
  while (x < 0.0)
    x += 360.0;
  return x;
}


/* x MOD 360.0, so that x in 0..360 */

REAL8 mod8360(REAL8 x)
{
  if (x >= 0 && x < 360.0)
    return x;
  return x - 360.0 * RFloor(x / 360.0);
}


/* a - b on a 360 degree circle, result -180..180 */

REAL8 diff8360(REAL8 a, REAL8 b)
{
  REAL8 d;

  d = a - b;
  if (d >= 180.0)
    return d - 360.0;
  if (d < -180.0)
    return d + 360.0;
  return d;
}


REAL8 test_near_zero(REAL8 x)
{
  if (ABS8(x) >= NEAR_ZERO)
    return x;
  if (x < 0)
    return -NEAR_ZERO;
  return NEAR_ZERO;
}


/*
** p points to memory filled with long values; for
** each of the values the seqeuence of the four bytes
** has to be reversed, to translate HP-UX and VAX
** ordering to MSDOS/Turboc ordering
*/

void longreorder(UCHAR *p, int n)
{
  int i;
  unsigned char c0, c1, c2, c3;
  static int orderinit = 0;
  unsigned short test;

  if (!orderinit) {
    test = 0x0001;
    orderinit = (*(unsigned char *)(&test)) ? 1 : -1;
  }
  if (orderinit < 0)
    return;
  for (i = 0; i < n; i += 4, p += 4) {
    c0 = *p;
    c1 = *(p + 1);
    c2 = *(p + 2);
    c3 = *(p + 3);
    *p = c3;
    *(p + 1) = c2;
    *(p + 2) = c1;
    *(p + 3) = c0;
  }
}


/*****************************************************
$Header: deltat.c,v 1.10 93/01/27 14:37:06 alois Exp $
deltat.c
deltat(t): returns delta t (in julian days) from universal time t
is included by users
ET = UT +  deltat

---------------------------------------------------------------
| Copyright Astrodienst Zurich AG and Alois Treindl, 1989.    |
| The use of this source code is subject to regulations made  |
| by Astrodienst Zurich. The code is NOT in the public domain.|
|                                                             |
| This copyright notice must not be changed or removed        |
| by any user of this program.                                |
---------------------------------------------------------------

******************************************************/

double deltat(double jd_ad)
{
  static short int dt[] = { /* in centiseconds */
  /*
  ** dt from 1637 to 2000, as tabulated in A.E.
  ** the values 1620 - 1636 are not taken, as they fit
  ** badly the parabola 25.5 t*t for the next range. The
  ** best crossing point to switch to the parabola is
  ** 1637, where we have fitted the value for continuity
  */
  6780, 6500, 6300,
  6200, 6000, 5800, 5700, 5500,
  5400, 5300, 5100, 5000, 4900,
  4800, 4700, 4600, 4500, 4400,
  4300, 4200, 4100, 4000, 3800, /* 1655 - 59 */
  3700, 3600, 3500, 3400, 3300,
  3200, 3100, 3000, 2800, 2700,
  2600, 2500, 2400, 2300, 2200,
  2100, 2000, 1900, 1800, 1700,
  1600, 1500, 1400, 1400, 1300,
  1200, 1200, 1100, 1100, 1000,
  1000, 1000, 900,  900,  900,
  900,  900,  900,  900,  900,
  900,  900,  900,  900,  900,  /* 1700 - 1704 */
  900,  900,  900,  1000, 1000,
  1000, 1000, 1000, 1000, 1000,
  1000, 1000, 1100, 1100, 1100,
  1100, 1100, 1100, 1100, 1100,
  1100, 1100, 1100, 1100, 1100,
  1100, 1100, 1100, 1100, 1200, /* 1730 - 1734 */
  1200, 1200, 1200, 1200, 1200,
  1200, 1200, 1200, 1200, 1300,
  1300, 1300, 1300, 1300, 1300,
  1300, 1400, 1400, 1400, 1400,
  1400, 1400, 1400, 1500, 1500,
  1500, 1500, 1500, 1500, 1500, /* 1760 - 1764 */
  1600, 1600, 1600, 1600, 1600,
  1600, 1600, 1600, 1600, 1600,
  1700, 1700, 1700, 1700, 1700,
  1700, 1700, 1700, 1700, 1700,
  1700, 1700, 1700, 1700, 1700,
  1700, 1700, 1600, 1600, 1600, /* 1790 - 1794 */
  1600, 1500, 1500, 1400, 1400,
  1370, 1340, 1310, 1290, 1270, /* 1800 - 1804 */
  1260, 1250, 1250, 1250, 1250,
  1250, 1250, 1250, 1250, 1250,
  1250, 1250, 1240, 1230, 1220,
  1200, 1170, 1140, 1110, 1060,
  1020, 960,  910,  860,  800,
  750,  700,  660,  630,  600,  /* 1830 - 1834 */
  580,  570,  560,  560,  560,
  570,  580,  590,  610,  620,
  630,  650,  660,  680,  690,
  710,  720,  730,  740,  750,
  760,  770,  770,  780,  780,
  788,  782,  754,  697,  640,  /* 1860 - 1864 */
  602,  541,  410,  292,  182,
  161,  10, -102, -128, -269,
  -324, -364, -454, -471, -511,
  -540, -542, -520, -546, -546,
  -579, -563, -564, -580, -566,
  -587, -601, -619, -664, -644, /* 1890 - 1894 */
  -647, -609, -576, -466, -374,
  -272, -154, -2, 124,  264,
  386,  537,  614,  775,  913,
  1046, 1153, 1336, 1465, 1601,
  1720, 1824, 1906, 2025, 2095,
  2116, 2225, 2241, 2303, 2349, /* 1920 - 1924 */
  2362, 2386, 2449, 2434, 2408,
  2402, 2400, 2387, 2395, 2386,
  2393, 2373, 2392, 2396, 2402,
  2433, 2483, 2530, 2570, 2624,
  2677, 2728, 2778, 2825, 2871,
  2915, 2957, 2997, 3036, 3072, /* 1950 - 1954 */
  3107, 3135, 3168, 3218, 3268,
  3315, 3359, 3400, 3447, 3503,
  3573, 3654, 3743, 3829, 3920,
  4018, 4117, 4223, 4337, 4449,
  4548, 4646, 4752, 4853, 4959,
  5054, 5138, 5217, 5296, 5379, /* 1980 - 1984 */
  5434, 5487, 5532, 5582, 5630, /* 1985 - 89 from AE 1991 */
  5686, 5757, 5900, 5900, 6000, /* AE 1993 and extrapol */
  6050, 6100, 6150, 6200, 6250, /* 1995 - 1999 */
  6300};          /* 2000 */
  double yr, cy, delta;
  long iyr, i;
  yr = (jd_ad + 18262) / 365.25 + 100.0;    /*  year  relative 1800 */
  cy = yr / 100;
  iyr =  (long) (RFloor(yr) + 1800);   /* truncated to integer, rel 0 */
#if TIDAL_26    /* Stephenson formula only when 26" tidal
  term in lunar motion */
  if (iyr >= 1637  && iyr < 2000) {
  i = iyr - 1637;
  delta = dt[i] * 0.01 + (dt[i+1] - dt[i]) * (yr - RFloor(yr)) * 0.01;
  } else if (iyr >= 2000) { /* parabola, fitted at value[2000] */
  delta = 25.5 * cy * cy  - 25.5 * 4 + 63.00;
  } else if (iyr >= 948) {  /* from 948 - 1637 use parabola */
  delta = 25.5 * cy * cy;
  } else {  /* before 984 use other parabola */
  delta = 1361.7  + 320 * cy + 44.3 * cy * cy;  /* fits at 948 */
  }
#else    /* use Clemence value + 5 sec before 1690, new dt afterwards */
  cy -= 1;  /* epoch 1900 */
  if (iyr >= 1690  && iyr < 2000) {
  i = iyr - 1637;
  delta = dt[i] * 0.01 + (dt[i+1] - dt[i]) * (yr - RFloor(yr)) * 0.01;
  } else if (iyr >= 2000) { /* parabola, fitted at value[2000] */
  delta = 29.949 * cy * cy  - 29.949 * 4 + 63.0;
  } else {
  delta = 5 + 24.349 + 72.3165 * cy + 29.949 * cy * cy; /* fits at 1690 */
  }
#endif
  return delta / 86400.0;
}


/*******************************************
$Header: d2l.c,v 1.9 91/11/16 16:24:20 alois Exp $

double to long with rounding, no overflow check
*************************************/
long d2l(double x)
{
  if (x >=0)
    return ((long) (x + 0.5));
  else
    return (-(long) (0.5 - x));
}


/*
* $Header$
*
* A collection of useful functions for centisec calculations.

---------------------------------------------------------------
| Copyright Astrodienst Zurich AG and Alois Treindl, 1991.    |
| The use of this source code is subject to regulations made  |
| by Astrodienst Zurich. The code is NOT in the public domain.|
|                                                             |
| This copyright notice must not be changed or removed        |
| by any user of this program.                                |
---------------------------------------------------------------
*******************************************************/

double degnorm(double p)
{
  if (p < 0)
    do {
      p += 360.0;
    } while (p < 0);
  else if (p >= 360.0)
    do {
      p -= 360.0;
    } while (p >= 360.0);
  return (p);
}


/*********************************************************
$Header: julday.c,v 1.9 91/11/16 16:25:06 alois Exp $
*********************************************************/

/*
** This function returns the absolute Julian day number (JD)
** for a given calendar date.
** The aruguments are a calendar date: day, month, year as integers,
** hour as double with decimal fraction.
** If gregflag = 1, Gregorian calendar is assumed, gregflag = 0
** Julian calendar is assumed.
**
** The Julian day number is system of numbering all days continously
** within the time range of known human history. It should be familiar
** for every astrological or astronomical programmer. The time variable
** in astronomical theories is usually expressed in Julian days or
** Julian centuries (36525 days per century) relative to some start day;
** the start day is called 'the epoch'.
** The Julian day number is a double representing the number of
** days since JD = 0.0 on 1 Jan -4712, 12:00 noon.
** Midnight has always a JD with fraction .5, because traditionally
** the astronomical day started at noon.
**
** NOTE: The Julian day number is named after the monk Julianus. It must
** not be confused with the Julian calendar system, which is named after
** Julius Cesar, the Roman politician who introduced this calendar.
**
** Original author: Marc Pottenger, Los Angeles.
** with bug fix for year < -4711   15-aug-88 by Alois Treindl
**
** References: Oliver Montenbruck, Grundlagen der Ephemeridenrechnung,
**             Verlag Sterne und Weltraum (1987), p.49 ff
**
** related functions: revjul() reverse Julian day number: compute the
**             calendar date from a given JD
*/

double julday(int month, int day, int year, double hour, int gregflag)
{
  double jd, u, u0, u1, u2;

  u = year;
  if (month < 3)
    u -=1;
  u0 = u + 4712.0;
  u1 = month + 1.0;
  if (u1 < 4)
    u1 += 12.0;
  jd = RFloor(u0*365.25)
    + RFloor(30.6*u1+0.000001)
    + day + hour/24.0 - 63.5;
  if (gregflag) {
    u2 = RFloor(ABS8(u) / 100) - RFloor(ABS8(u) / 400);
    if (u < 0.0)
      u2 = -u2;
    jd = jd - u2 + 2;
    if ((u < 0.0) && (u/100 == RFloor(u/100)) && (u/400 != RFloor(u/400)))
      jd -= 1;
  }
  return jd;
}


/*********************************************************
$Header: revjul.c,v 1.9 91/11/16 16:25:37 alois Exp $
*********************************************************/

/*
** revjul() is the inverse function to julday(), see the description there.
** Arguments are julian day number, calendar flag (0=julian, 1=gregorian)
** return values are the calendar day, month, year and the hour of
** the day with decimal fraction (0 .. 23.999999).
**
** Original author Mark Pottenger, Los Angeles.
** with bug fix for year < -4711 16-aug-88 Alois Treindl
*/

void revjul(double jd, int gregflag,
  int *jmon, int *jday, int *jyear, double *jut)
{
  double u0, u1, u2, u3, u4;

  u0 = jd + 32082.5;
  if (gregflag) {
    u1 = u0 + RFloor(u0/36525.0) - RFloor(u0/146100.0) - 38.0;
    if (jd >= 1830691.5) u1 +=1;
    u0 = u0 + RFloor(u1/36525.0) - RFloor(u1/146100.0) - 38.0;
  }
  u2 = RFloor(u0 + 123.0);
  u3 = RFloor((u2 - 122.2) / 365.25);
  u4 = RFloor((u2 - RFloor(365.25 * u3)) / 30.6001);
  *jmon = (int)(u4-1.0);
  if (*jmon > 12)
    *jmon -= 12;
  *jday = (int)(u2 - RFloor(365.25 * u3) - RFloor(30.6001 * u4));
  *jyear = (int)(u3 + RFloor((u4 - 1.9999) / 12.0) - 4800.0);
  *jut = (jd - RFloor(jd + 0.5) + 0.5) * 24.0;
}
#endif /* PLACALC */

/* placalc2.cpp */
