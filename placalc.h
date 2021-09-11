/*
** Astrolog (Version 7.30) File: placalc.h
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
/* "planet" is defined to be cp0.obj, but is often a local in Placalc code. */
#undef planet


#define EPHE_PATH EPHE_DIR
#define HUGE8 1.7E+308     /* biggest value for REAL8 */

#ifdef PLACALC
/************************************************************
$Header: placalc.h,v 1.4 93/03/22 10:08:39 alois Exp $
definitions and constants for planetary routines

ATTENTION: PLACALC USERS ON MSDOS:
See the note close to the end of this file regarding EPHE_PATH.

---------------------------------------------------------------
| Copyright Astrodienst AG and Alois Treindl, 1991, 1993.     |
| The use of this source code is subject to regulations made  |
| by Astrodienst Zurich. The code is NOT in the public domain.|
|                                                             |
| This copyright notice must not be changed or removed        |
| by any user of this program.                                |
---------------------------------------------------------------
************************************************************/

/************************************************************
$Header: ourdef.h,v 1.2 91/11/16 16:21:37 alois Exp $
definitions and constants for all Astrodienst C programs
contains only declarations and #defines, no global variables.
auto-dectection of MSDOS (TURBO_C or MS_C) or HPUNIX
************************************************************/

#ifdef MSDOS  /* already defined by some DOS compilers */
#undef MSDOS
#define MSDOS 1
#endif

#ifdef __TURBOC__ /* defined by turboc */
#ifndef MSDOS
#define MSDOS 1
#endif
#define TURBO_C
#endif

#if MSDOS
#define HPUNIX 0
#ifndef TURBO_C
#define MS_C /* assume Microsoft C compiler */
#endif
#else
#define MSDOS 0
#define HPUNIX 1
#ifndef _HPUX_SOURCE
#define _HPUX_SOURCE
#endif
#endif

#include <stdlib.h>
#if HPUNIX
#ifndef MACOLD
#ifndef NeXT
#ifndef PC
#include <unistd.h>
#endif
#endif
#endif /* MACOLD */
#endif /* HPUNIX */

#ifndef OK
#define OK 0
#define ERR (-1)
#endif


#define UCHP (unsigned char*)  /* used for casting *char  */
#define UCP (unsigned char*)  /* used for casting *char  */
#define SCP (char*)     /* used for casting *unsigned char */
#define UCHAR unsigned char

#if HPUNIX
#ifndef NeXT
#ifdef PC
#include <malloc.h>
#endif /* PC */
#endif
typedef double  REAL8;  /* real with at least 64 bit precision */
typedef float   REAL4;  /* real with at least 32 bit precision */
typedef long    INT4;   /* signed integer with at least 32 bit precision */
typedef unsigned long UINT4;
/* unsigned integer with at least 32 bit precision */
typedef int     INT2;   /* signed integer with at least 16 bit precision */
typedef int     INT1;   /* signed integer with at least 8  bit precision */
#ifndef dbd_VERSION     /* db_vista defines boolean in vista.h already */
typedef int     PLABOOL;
#endif
typedef unsigned short UINT2; /* unsigned 16 bits */
#define ABS4  abs   /* abs function for long */
#endif /* HPUNIX */

#if MSDOS
#ifdef TURBO_C
#include <alloc.h>    /* MSC needs malloc ! */
#else
#include <malloc.h>
#endif
typedef double  REAL8;  /* real with at least 64 bit precision */
typedef float   REAL4;  /* real with at least 32 bit precision */
typedef long    INT4;   /* signed integer with at least 32 bit precision */
typedef unsigned long UINT4;
                        /* unsigned integer with at least 32 bit precision */
typedef int     INT2;   /* signed integer with at least 16 bit precision */
typedef int     INT1;   /* signed integer with at least 8  bit precision */
typedef int     PLABOOL;
typedef unsigned int UINT2; /* unsigned 16 bits */
#define ABS4  labs    /* abs function for long */
#endif /* MSDOS */

#define COS8 RCos
#define SIN8 RSin
#define ASIN8 asin
#define TAN8 RTan
#define ATAN8 RAtn
#define ATAN28 atan2
#define EXP10(x) pow(10.0, (x))
#define ABS8(x) RAbs(x)

#define TANERRLIMIT 1.0E-10     /* used to check for arguments close to pi */
#define NEAR_ZERO   1.0E-16     /* used to compare for divisors close to 0 */
#define BIGREAL     1.0E+38

#define DEGTORAD 0.0174532925199433
#define RADTODEG 57.2957795130823

typedef INT4 centisec;   /* centiseconds used for angles and times */
#define CS   (centisec)  /* use for casting */
#define CSEC centisec    /* use for typing */

#define DEG     (360000L)   /* degree expressed in centiseconds */
#define DEG7_30 (2700000L)  /* 7.5 degrees */
#define DEG15   (15 * DEG)
#define DEG24   (24 * DEG)
#define DEG30   (30 * DEG)
#define DEG60   (60 * DEG)
#define DEG90   (90 * DEG)
#define DEG120  (120 * DEG)
#define DEG150  (150 * DEG)
#define DEG180  (180 * DEG)
#define DEG270  (270 * DEG)
#define DEG360  (360 * DEG)

#define CSTORAD 4.84813681109536E-08 /* centisec to rad: pi / 180 /3600/100 */
#define RADTOCS 2.06264806247096E+07 /* rad to centisec 180*3600*100/pi */

#define DEG2MSEC 3600000.0  /* degree to milliseconds */
#define DEG2CSEC 360000.0   /* degree to centiseconds */

#define SEC2CSEC 100  /* seconds to centiseconds */

#define CS2DEG    (1.0/360000.0)  /* centisec to degree */
#define CS2CIRCLE (CS2DEG/360.0)  /* centisec to circle */
#define AU2INT    1e7             /* factor for long storage of A.U. */

#define CSMIN 6000L
#define CSSEC 100L

#define SINDEG(x) sin(DEGTORAD * (x))
#define COSDEG(x) cos(DEGTORAD * (x))
#define TANDEG(x) tan(DEGTORAD * (x))
#define SINCS(x)  sin((double)(CSTORAD * (x)))
#define COSCS(x)  cos((double)(CSTORAD * (x)))
#define TANCS(x)  tan((double)(CSTORAD * (x)))

/*************************************************************
Exported functions:

In all functions the variable jd_ad indicates the use of
Astrodienst relative julian days, and jd the use of absolute
julian days.
*************************************************************/

extern int lrz_file_posit(double, FILE **);
extern int ast_file_posit(double, FILE **);
extern int chi_file_posit(double, FILE **);
extern int outer_hel(int, REAL8, REAL8 *, REAL8 *, REAL8 *, REAL8 *, REAL8 *,
  REAL8 *);
extern void longreorder(UCHAR *, int);
extern int inpolq(int, int, double, double[], double *, double *);

extern int nacalc();
extern int calcserv();
extern void helup(REAL8);
extern void togeo(REAL8, REAL8, REAL8, REAL8, REAL8, REAL8 *, REAL8 *);
extern int calc(int, REAL8, int, REAL8 *, REAL8 *, REAL8 *, REAL8 *);
extern int hel(int, REAL8, REAL8 *, REAL8 *, REAL8 *, REAL8 *, REAL8 *,
  REAL8 *);
extern int moon(REAL8 *, REAL8 *, REAL8 *);
extern REAL8 sidtime();
extern REAL8 smod8360(REAL8);
extern REAL8 mod8360(REAL8);
extern REAL8 diff8360(REAL8, REAL8);
extern REAL8 test_near_zero(REAL8);
extern REAL8 deltat(double);
extern void to_mean_ekl();
extern int afl2planet();
extern void disturb(register struct kor *, REAL8 *, REAL8 *, REAL8, REAL8,
  REAL8);
extern REAL8 fnu(REAL8, REAL8, REAL8);

/*************************************************************
definitions
*************************************************************/

/*
* planet index numbers, used to identify a planet in calc() and
* other related functions.
*/
#define SUN       0   /* used synonymously for earth too */
#define EARTH     0
#define MOON      1
#define MERCURY   2
#define VENUS     3
#define MARS      4
#define JUPITER   5
#define SATURN    6
#define URANUS    7
#define NEPTUNE   8
#define PLUTO     9
#define MEAN_NODE 10
#define TRUE_NODE 11
#define CHIRON    12
#define LILITH    13
#define CERES     14
#define PALLAS    15
#define JUNO      16
#define VESTA     17

/*************************************************************
exported variables
*************************************************************/

extern REAL8 meanekl;
extern REAL8 ekl;
extern REAL8 nut;

extern struct elements { /* actual elements at time thelup */
  REAL8 tj,     /* centuries from epoch */
  lg,     /* mean longitude in degrees of arc*/
  pe,     /* longitude of the perihelion in degrees of arc*/
  ex,     /* excentricity in degrees of arc*/
  kn,     /* longitude of node in degrees of arc*/
  in,     /* inclination of the orbit in degrees of arc*/
  ma;     /* mean anomaly in degrees of arc*/
} el [MARS + 1];

extern char *ephe_path;

/*
* in a bitlist flag each planet is represented by a bit;
* all 13 defined planets can be called at once with
* LILITH is not automatically included
*/
#define CALC_ALL_PLANET_BITS ((1 << 13) - 1) /* bits 0..12 set */

#define J2000 2451545.0 /* Epoch of JPL ephemeris DE200, absolute */
#define J1950  2433282.423  /* Epoch of JPL ephemeris DE102 */
#define JUL_OFFSET 2433282.0  /* offset of Astrodienst relative Julian date */
#define EPOCH1850 -36524.0  /* jupiter,saturn 0 jan 1850, 12:00 */
#define EPOCH1900 -18262.0  /* inner planets  0 jan 1900, 12:00 */
#define EPOCH1950  0.0    /* pluto    0 jan 1950, 12:00 */
                /* this is the origin of the Astrodienst
                   relative julian calendar system */
#define EPOCH1960 3653.0 /* uranus,neptune 1 jan 1960, 12:00 */

#define ENDMARK 99 /* used to mark the end of disturbation terms */

#define NODE_INTERVAL 0.005        /* days, = 7m20s */
#define MOON_SPEED_INTERVAL 0.0001 /* 8.64 seconds later */

/*
* flag bits used in calc and calcserv
*/
#define CALC_BIT_HELIO 1   /* geo/helio */
#define CALC_BIT_NOAPP 2   /* apparent/true positions */
#define CALC_BIT_NONUT 4   /* true eq. of date/ mean equ. of date */
#define CALC_BIT_EPHE  8   /* universal/ephemeris time */
#define CALC_BIT_SPEED 16  /* without/with speed */
#define CALC_BIT_BETA  32  /* without/with latitude */
#define CALC_BIT_RGEO  64  /* without/with relative rgeo */
#define CALC_BIT_RAU   128 /* without/with real radius */
#define CALC_BIT_MUST_USE_EPHE 256 /* epheserv may not use calc */
#define CALC_BIT_MAY_USE_EPHE  512 /* calcserv may use ephread */

#define EPHE_STEP 80              /* days step in LRZ ephe */
#define EPHE_DAYS_PER_FILE 100000 /* days per ephe file */
#define EPHE_OUTER "LRZ5_"        /* file name prefix */
#define EPHE_OUTER_BSIZE  60      /* blocksize */
#define EPHE_CHIRON "CHI_"        /* file name prefix */
#define EPHE_CHIRON_BSIZE 12      /* blocksize */
#define EPHE_ASTER "CPJV_"        /* file name prefix */
#define EPHE_ASTER_BSIZE  48      /* blocksize */

/********************************************
About the format of the ephemeris files
----------------------------------------
We use currently ephemeris files with steps of 80 days.
There are 1250 "records" in each file, so that one file
spans 100000 days. We have three types of ephemeris files:
LRZ5_nn for the outer planets Jupiter ... Pluto,
CHI_nn for Chiron, and CPJV_nn for the four asteroids Ceres ... Vesta.
nn is an expression derived from the first julian daynumber on the
file. Jd 2100'000 to 2199'920 is on file LRZ5_21, CHI_21, and CPJV_21;
for negative Jd we use the filenames LRZ5_Mxx  with M indicating the minus.

Given the jd for which you want the ephemeris, it is simple to build
the filename and use fseek() within the file to go where the data is.
This is done by the functions lrz_file_posit() and chi_file_posit().

The stored coordinates are for each date and planet:
L = ecliptic longitude relative to mean exquinox of date,
in units of milliseconds of acr (1/3600000 degree) as type long;
R = radius vector, units of 10-7 AU, as type long.
Z = disctance of ecliptic; Z = R * sin(latitude); in units of 10-7 AU,
as type long.
The data is stored in the byte ordering of the Astrodienst HPUX machines,
which is most significant byte first. It is not the same byte ordering
as on Intel processors; the function longreorder() converts between
the disk file format and the internal format of MSDOS machines.

For LRZ5- files, we have 60-byte records LRZ(Jupiter),LRZ(Saturn),
LRZ(Uranus), LRZ(Neptune), LRZ(Pluto).
For CHI- files we have 12-byte records LRZ.
For CPJV- files we have 48-byte records.

************************************************/


/*************************************************************
placalc2.c definitions
*************************************************************/

#define SDNUM 20
#define NUM_MOON_CORR 93

struct eledata {
REAL8 axis,   /* mean distance, in a.u., A(N) in basic */
period,   /* days for one revolution, P(N) in basic */
epoch,    /* relative juldate of epoch, Ep(N) in basic */
/* T = distance to epoch in jul.centuries 36525 day*/
lg0,lg1,lg2,lg3,/* deg(epoch), degree/day, seconds/T^2,seconds/T^3 */
/* Pd(N,0..2) in basic, lg3 was not present */
pe0,pe1,pe2,pe3,/* deg(epoch), seconds/T,  seconds/T^2,seconds/T^3 */
/* Pd(N,3..5) in basic, pe3 was not present */
ex0,ex1,ex2,  /* ecl(epoch), 1/T, 1/T^2 */
/* Pd(N,6..8) in basic */
kn0,kn1,kn2,kn3,/* node(epoch),seconds/T,  seconds/T^2,seconds/T^3 */
/* Pd(N,9..11) in basic, kn3 was not present */
in0,in1,in2;    /* incl(epoch),1/T, 1/T^2 */
/* Pd(N,12..14) in basic */
};

struct kor {
  int     j, i;
  REAL8   lampl;  /* amplitude of disturbation in long, seconds of arc */
  REAL8 lphase;   /* phase of disturbation in long, degrees */
  INT4    rampl;  /* ampl. of disturbation in radius, 9th place of log */
  REAL8   rphase; /* phase of disturbation in radius, degrees */
  int     k;      /* index into disturbing planet anomaly table sa[] */
};

struct sdat {   /* 0..19 mean anomalies of disturbing planets
  Sd(0..19,0..1) in basic */
  REAL8 sd0,  /* mean anomaly at epoch 1850 */
  sd1;  /* degrees/year */
};

/* moon correction data; revised 30-jul-88: all long. to 0.3" */
struct m45dat {
  int  i0,i1,i2,i3;
  REAL8 lng,lat,par;
};

extern struct eledata pd[MARS + 1];
extern struct kor earthkor[86+1];
extern struct kor mercurykor[24+1];
extern struct kor venuskor[22+1];
extern struct kor marskor[62+1];
extern struct sdat _sd [SDNUM];
extern struct m45dat m45[NUM_MOON_CORR];
extern REAL8 ekld[4];
extern REAL8 sa[SDNUM];
extern double degnorm(double);

/*******************************************
$Header: astrolib.h,v 1.2 91/11/16 16:21:02 alois Exp $
astrolib.h

*******************************************/

/* macros for bit operations */

#define clear_bit(v,bit_nr) ((v) & ~(1L << (bit_nr)))
#define set_bit(v,bit_nr)   ((v) | (1L << (bit_nr)))
#define bit(bit_nr)         (1L << (bit_nr))
#define check_bit(v,bit_nr) ((v) & (1L << (bit_nr)))

#endif /* PLACALC */

/* placalc.h */
