/*
** Astrolog (Version 7.30) File: placalc.cpp
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

/* function calc():
** This is the main routine for computing a planets position.
** The function has several modes, which are controlled by bits in
** the parameter 'flag'. The normal mode (flag == 0) computes
** a planets apparent geocentric position in ecliptic coordinates relative to
** the true equinox of date, without speed
**
** Explanation of the arguments: see the functions header.
**
** Returns OK or ERR (if some planet out of time range). OK and ERR are
** defined in ourdef.h and must not be confused with TRUE and FALSE.
** OK and ERR are of type int, not of type PLABOOL.
**
** Bits used in flag:
** CALC_BIT_HELIO     0 = geocentric, 1 = heliocentric
** CALC_BIT_NOAPP       0 = apparent positions, 1 = true positions
** CALC_BIT_NONUT     0 = do nutation (true equinox of date)
** 1 = don't do nutation (mean equinox of date).
**
** CALC_BIT_SPEED     0 = don't calc speed,
** 1 = calc speed, takes quite long for moon
** (is observed only for moon, with other
** planets speed is cheap)
**
** Side effects and local memory:
** For doing heliocentric positions the fucntion must know the
** earth's position for the desired time t. It remembers the earth
** position so it does not have to recompute it each time a planet
** position is wanted for the same time t.
** It calls helup(t), which leaves as a side effect the global
** variables meanekl, ekl and nut for the time t.
**
** Functions called by calc():
** helup(t)
** hel(t)
** moon(t)
** togeo()
**
** Time range:
** The function can be used savely in the time range 5000 BC to
** 3000 AD. The stored ephemeris is available only for this time
** range, so Jupiter ... Pluto cannot be computed outside. The
** function will return results for the other planets also outside
** of this time range, but they become meaningless pretty soon
** before 5000 BC, because Newcombs time series expansions for the
** elements will not work anymore.
**
** pointers to the return variables:
** alng = ecliptic longitude in degrees
** arad = radius vector in AU (astronomic units)
** alat = ecliptic latitude in degrees
** alngspeed = speed of planet in degrees per day
**
** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
** The precision of the speed is quite limited.
** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
**
** For Sun, Mercury, Venus and Mars we take only the speed from
** the undisturbed Kepler orbit. For the Moon there is no
** reasonable undisturbed orbit and we derive the speed from
** its position at t + dt and t - dt. We need these
** moon positions anyway for the true node calculation.
** For the outer planets and Chiron we derive the precise
** speed from the stored ephemeris by high order inter-
** polation; the precision is limited for the geocentric
** case due to the limited precision of the earth's/sun's speed.
** Applications who need precise speeds should
** get them by calling calc() with slightly different times.
**
** Comment 7 May 1991 by Alois Treindl:
** Center of Earth versus Barycenter Earth-Moon:
** Brown's theory of the moon gives the moon's coordinates relative
** to the center of the earth. Newcomb's theory of the Sun gives the
** coordinates of the earth's center relative to the center of the Sun.
** This is what we need.
**
** How about the Mean Lunar Node?
** The orbital elements of the Sun in Newcomb's theory are given
** relative to the barycenter Earth-Moon; the reduction to geocentric
** is only applied after doing the Kepler ellipse calculation.
** Are the Lunar elements also relative to the barycenter??
** If yes:
** When we use the moon's mean node out of the elements, it is still
** as seen from the barycenter. Because the node is close to the
** earth, we would have to apply a considerable correction, which is of
** the order of 4000/384000 km or 35' (minutes of arc).
** Nobody has ever applied such a correction to the mean node.
**
** And the True Node?
** When we calculate the osculating orbital elements of the Moon (true node),
** are they relative to the barycenter or to the Earth's center?
** Our derivation of true node from the actual Moon positions considers
** the earth's center as the focal point of the osculating lunar ellipse.
** A more correct approach would first reduce the lunar position from
** geocentric to barycentric, then compute the orbital elements from
** the reduced positions, and then reduce the desired items
** (node, apogaeum, 'dark moon') to geocentric positions.
** No known astrological ephemeris has ever used such a correction, which is
** of the same order of magnitude as the correction to the meannode above.
** When the moon is going through the ecliptic, the geocenter, barycenter
** moon (and the node identical to the moon itself) line up; this is why
** the error does not show up in normal considerations.
*/

int calc(int planet, REAL8 jd_ad, int flag,
  REAL8 *alng, REAL8 *arad, REAL8 *alat, REAL8 *alngspeed)
/* planet index as defined in placalc.h,
SUN = 0, MOON = 1 etc.
planet == -1 calc calculates only nut and ecl */
/* relative Astrodienst Juldate, ephemeris time.
Astrodienst Juldate is relative 31 Dec 1949, noon. */
/* See definition of flag bits above */
{
  struct rememberdat  /* time for which the datas are calculated */
    {REAL8 calculation_time, lng, rad, zet, lngspeed, radspeed, zetspeed;};
  static struct rememberdat earthrem =
    {HUGE8, HUGE8, HUGE8, HUGE8, HUGE8, HUGE8, HUGE8};
  static struct rememberdat moonrem  =
    {HUGE8, HUGE8, HUGE8, HUGE8, HUGE8, HUGE8, HUGE8};
  REAL8 c, s, x, knn, knv;
  REAL8 rp, zp; /* needed to call hel! */
  REAL8 *azet = alat;
  PLABOOL calc_geo, calc_helio, calc_apparent, calc_speed,
  calc_nut;

  /* helup checks whether it was already called with same time */
  helup (jd_ad);
  /* we could return now if we only wanted to compute ecl and nut */

  calc_helio =  flag & CALC_BIT_HELIO;
  calc_geo = ! calc_helio;
  calc_apparent = ! (flag & CALC_BIT_NOAPP);
  calc_nut = ! (flag & CALC_BIT_NONUT);
  calc_speed = flag & CALC_BIT_SPEED;
  /*
  ** it is necessary to compute EARTH in the following cases:
  ** heliocentric MOON or EARTH
  ** geocentric any planet except MOON or nodes or LILITH
  */
  if (calc_helio && (planet == MOON || planet == EARTH)
    || calc_geo && planet != MOON
    && planet != MEAN_NODE
    && planet != TRUE_NODE
    && planet != LILITH) {
    if (earthrem.calculation_time != jd_ad) {
      hel (EARTH, jd_ad, alng, arad, azet, alngspeed, &rp, &zp);
      /* store earthdata for geocentric calculation: */
      earthrem.lng = *alng;
      earthrem.rad = *arad;
      earthrem.zet = *azet;
      earthrem.lngspeed = *alngspeed;
      earthrem.radspeed = rp;
      earthrem.zetspeed = zp;
      earthrem.calculation_time = jd_ad;
    }
  }
  switch(planet) {

  case EARTH: /* has been already computed */
    *alng = earthrem.lng;
    *arad = earthrem.rad;
    *azet = earthrem.zet;
    *alngspeed = earthrem.lngspeed;
    rp = earthrem.radspeed;
    zp = earthrem.zetspeed;
    if (calc_geo) { /* SUN seen from earth */
      *alng = smod8360(*alng + 180.0);
      *azet = - *azet;
    }
    if (calc_apparent)
      *alng = *alng - 0.0057683 * (*arad) * (*alngspeed);
    break;

  case MOON:
    moon(alng, arad, azet);
    moonrem.lng = *alng;  /* moonrem will be used for TRUE_NODE */
    moonrem.rad = *arad;
    moonrem.zet = *azet;
    *alngspeed = 12;
    moonrem.calculation_time = jd_ad;
    if (calc_helio || calc_speed) {/* get a second moon position */
      REAL8 lng2, _rad2, zet2;
      helup(jd_ad + MOON_SPEED_INTERVAL);
      moon(&lng2, &_rad2, &zet2);
      helup(jd_ad);
      if (calc_helio) { /* moon as seen from sun */
        togeo(earthrem.lng, -earthrem.rad, moonrem.lng, moonrem.rad,
          moonrem.zet, alng, arad);
        togeo(earthrem.lng + MOON_SPEED_INTERVAL * earthrem.lngspeed,
          -(earthrem.rad + MOON_SPEED_INTERVAL * earthrem.radspeed),
          lng2, _rad2, zet2, &lng2, &_rad2);
      }
      *alngspeed =  diff8360(lng2, *alng) / MOON_SPEED_INTERVAL;
      /* rp = (rad2 - *arad) / MOON_SPEED_INTERVAL;       */
      /* zp = (zet2 - moonrem.zet) / MOON_SPEED_INTERVAL; */
    }
    *alat = RADTODEG * ASIN8(*azet / *arad);
    /*
    ** light time correction, not applied for moon or nodes;
    ** moon would have only term of ca. 0.02", see Expl.Sup.1961 p.109
    */
    break;

  case MERCURY:
  case VENUS:
  case MARS:
  case JUPITER:
  case SATURN:
  case URANUS:
  case NEPTUNE:
  case PLUTO:
  case CHIRON:
  case CERES:
  case PALLAS:
  case JUNO:
  case VESTA:
    if (hel(planet, jd_ad, alng, arad, azet, alngspeed, &rp, &zp) != OK)
      return ERR; /* outer planets can fail if out of ephemeris range */
    if (calc_geo) {       /* geocentric */
      REAL8 lng1, _rad1, lng2, _rad2;
      togeo(earthrem.lng, earthrem.rad, *alng, *arad, *azet, &lng1, &_rad1);
      togeo(earthrem.lng + earthrem.lngspeed,
      earthrem.rad + earthrem.radspeed,
      *alng + *alngspeed, *arad + rp, *azet + zp, &lng2, &_rad2);
      *alng = lng1;
      *arad = _rad1;
      *alngspeed = diff8360(lng2, lng1);
      /* rp = rad2 - rad1; */
    }
    *alat = RADTODEG * ASIN8(*azet / *arad);
    if (calc_apparent)
      *alng = *alng - 0.0057683 * (*arad) * (*alngspeed);
    break;

  case MEAN_NODE:
    *alng = smod8360(el[MOON].kn);
    /*
    * the distance of the node is the 'orbital parameter' p = a (1-e^2);
    * Our current use of the axis a is wrong, but is never used.
    */
    *arad = pd[MOON].axis;
    *alat = 0.0;
    *alngspeed = -0.053;
    break;

  case TRUE_NODE: {
    /* see comment 'Note 7 May 1991' above */
    REAL8 ln, rn, zn,
    lv, rv, zv,
    l1, r1, z1,
    xn, yn, xv, yv, r0, x0, y0;

    helup(jd_ad + NODE_INTERVAL);
    moon(&ln, &rn, &zn);
    helup(jd_ad - NODE_INTERVAL);
    moon(&lv, &rv, &zv);
    helup(jd_ad);
    if (moonrem.calculation_time != jd_ad)
      moon(&l1, &r1, &z1);
    else {  /* moon is already calculated */
      l1 = moonrem.lng;
      r1 = moonrem.rad;
      z1 = moonrem.zet;
    }
    rn = sqrt(rn * rn - zn * zn);
    rv = sqrt(rv * rv - zv * zv);
    r0 = sqrt(r1 * r1 - z1 * z1);
    xn = rn * COS8(DEGTORAD * ln);
    yn = rn * SIN8(DEGTORAD * ln);
    xv = rv * COS8(DEGTORAD * lv);
    yv = rv * SIN8(DEGTORAD * lv);
    x0 = r0 * COS8(DEGTORAD * l1);
    y0 = r0 * SIN8(DEGTORAD * l1);
    x = test_near_zero(x0 * yn - xn * y0);
    s = (y0 * zn - z1 * yn) / x;
    c = test_near_zero((x0 * zn - z1 * xn) / x);
    knn =  smod8360(RADTODEG * ATAN28(s, c)); /* = ATAN8(s / c) */
    x = test_near_zero(y0 * xv - x0 * yv);
    s = (yv * z1 - zv * y0) / x;
    c = test_near_zero((xv * z1 - zv * x0) / x);
    knv =  smod8360(RADTODEG * ATAN28(s, c));
    *alng = smod8360((knv + knn) / 2);
    /*
    ** the distance of the node is the 'orbital parameter' p = a (1-e^2);
    ** Our current use of the axis a is wrong.
    */
    *arad = pd[MOON].axis;
    *alat = 0.0;
    *alngspeed = diff8360(knn, knv) / NODE_INTERVAL;
    }
    break;

  case LILITH: {
    /*
    ** Added 22-Jun-93
    ** Lilith or Dark Moon is the empty focal point of the mean lunar ellipse.
    ** This is 180 degrees from the perihel.
    ** Because the lunar orbit is not in the ecliptic, it must be
    ** projected onto the ecliptic in the same way as the planetary orbits
    ** are (see for example Montenbruck, Grundlagen der Ephemeridenrechnung).
    **
    ** We compute the MEAN Lilith, not the TRUE one which would have to be
    ** derived in a similar way as the true node.
    ** For the radius vector of Lilith we use a simple formula;
    ** to get a precise value, the fact that the focal point of the ellipse
    ** is not at the center of the earth but at the barycenter moon-earth
    ** would have to be accounted for.
    ** For the speed we always return a constant: the T term from the
    ** lunar perihel.
    ** Joelle de Gravelaine publishes in her book "Lilith der schwarze Mond"
    ** (Astrodata, 1990) an ephemeris which gives noon (12.00) positions
    ** but does not project onto the ecliptic.
    ** This creates deviations
    */
    double arg_lat, lon, cosi;
    struct elements *e = &el[MOON];
    arg_lat = degnorm(e->pe - e->kn + 180.0);
    cosi = COSDEG(e->in);
    if (e->in == 0 || ABS8(arg_lat -  90.0) < TANERRLIMIT
      || ABS8(arg_lat - 270.0) < TANERRLIMIT) {
      lon = arg_lat;
    } else {
      lon = ATAN8(TANDEG(arg_lat) * cosi);
      lon = RADTODEG * lon;
      if (arg_lat > 90.0 && arg_lat < 270.0)  lon += 180.0;
    }
    lon = degnorm(lon + e->kn);
    *alng = lon;
    *alngspeed = 0.111404;  /* 6'41.05" per day */
    *arad = 2 * pd[MOON].axis * e->ex;
    /*
    ** To test Gravalaines error, return unprojected long in alat.
    ** the correct latitude would be:
    ** *alat = RADTODEG * ASIN8(SINDEG(arg_lat) * SINDEG(e->in));
    */
#ifdef ASTROLOG
    *alat = RADTODEG * ASIN8(SINDEG(arg_lat) * SINDEG(e->in));
#else
    *alat = degnorm(arg_lat + e->kn); /* unprojected longitude, no nut */
#endif
    }
    break;

  default:
    return ERR;
  } /* end switch */

  if (calc_nut)
    *alng += nut;
  *alng = smod8360(*alng);  /* normalize to circle */
  return OK;
}


/* helio to geocentric conversion */

void togeo(REAL8 lngearth, REAL8 radearth, REAL8 lng, REAL8 rad, REAL8 zet,
  REAL8 *alnggeo, REAL8 *aradgeo)
{
  REAL8 r1, x, y;

  r1 = sqrt(rad * rad - zet * zet);
  x = r1 * COS8(DEGTORAD * lng) - radearth * COS8(DEGTORAD * lngearth);
  y = r1 * SIN8(DEGTORAD * lng) - radearth * SIN8(DEGTORAD * lngearth);
  *aradgeo = sqrt(x * x + y * y + zet * zet);
  x = test_near_zero(x);
  *alnggeo = smod8360(RADTODEG * ATAN28(y, x));
}


/*
** helup()
** prepares the orbital elements and the disturbation arguments for the
** inner planets and the moon. helup(t) is called by hel() and by calc().
** helup() returns its results in global variables.
** helup() remembers the t it has been called with before and does
** not recalculate its results when it is called more than once with
** the same t.
*/

void helup(REAL8 jd_ad)
{
  int i;
  static REAL8 thelup = HUGE8;  /* is initialized only once at load time */
  struct elements *e = el;      /* pointer to el[i] */
  struct elements *ee = el;     /* pointer to el[EARTH] */
  struct eledata  *d = pd;      /* pointer to pd[i] */
  REAL8 td, ti, ti2, tj1, tj2, tj3;

  if (thelup == jd_ad)
    return; /* if already calculated then return */

  for (i = SUN; i <= MARS; i++, d++, e++) {
    td = jd_ad - d->epoch;
    ti = e->tj = td / 36525.0;  /* julian centuries from epoch */
    ti2 = ti * ti;
    tj1 = ti / 3600.0;  /* used when coefficients are in seconds of arc */
    tj2 = ti * tj1;
    tj3 = ti * tj2;
    e->lg = mod8360(d->lg0 + d->lg1 * td  + d->lg2 * tj2 + d->lg3 * tj3);
    /* also with moon lg1 *td is exact to 10e-8 degrees within 5000 years */
    e->pe = mod8360(d->pe0 + d->pe1 * tj1 + d->pe2 * tj2 + d->pe3 * tj3);
    e->ex = d->ex0 + d->ex1 * ti + d->ex2 * ti2;
    e->kn = mod8360(d->kn0 + d->kn1 * tj1 + d->kn2 * tj2 + d->kn3 * tj3);
    e->in = d->in0 + d->in1 * tj1 + d->in2 * tj2;
    e->ma = smod8360(e->lg - e->pe);

    if (i == MOON) {
      /* calculate ekliptic according Newcomb, APAE VI,
      ** and nutation according Exp.Suppl. 1961, identical
      ** with Mark Potttenger elemnut()
      ** all terms >= 0.01" only .
      ** The 1984 IAU Theory of Nutation, as published in
      ** AE 1984 suppl. has not yet been implemented
      ** because it would mean to use other elements of
      ** moon and sun */

      REAL8 mnode, mlong2, slong2, mg, sg, d2;

      mnode  = DEGTORAD * e->kn;        /* moon's mean node */
      mlong2 = DEGTORAD * 2.0 * e->lg;  /* 2 x moon's mean longitude */
      mg     = DEGTORAD * e->ma;        /* moon's mean anomaly (g1) */
      slong2 = DEGTORAD * 2.0 * ee->lg; /* 2 x sun's mean longitude (L), with
                                        the phase 180 deg earth-sun irrelevant
                                        because 2 x 180 = 360 deg */
      sg     = DEGTORAD * ee->ma; /* sun's mean anomaly = earth's */
      d2     = mlong2 - slong2;   /* 2 x elongation of moon from sun */
      meanekl = ekld[0] + ekld[1] * tj1 + ekld[2] * tj2 + ekld[3] * tj3;
      ekl = meanekl +
        (9.2100 * COS8(mnode)
        - 0.0904 * COS8(2.0 * mnode)
        + 0.0183 * COS8(mlong2 - mnode)
        + 0.0884 * COS8(mlong2)
        + 0.0113 * COS8(mlong2 + mg)
        + 0.5522 * COS8(slong2)
        + 0.0216 * COS8(slong2 + sg)) / 3600.0;
        nut = ((-17.2327 - 0.01737 * ti) * SIN8(mnode)
        + 0.2088 * SIN8(2.0 * mnode)
        + 0.0675 * SIN8(mg)
        - 0.0149 * SIN8(mg - d2)
        - 0.0342 * SIN8(mlong2 - mnode)
        + 0.0114 * SIN8(mlong2 - mg)
        - 0.2037 * SIN8(mlong2)
        - 0.0261 * SIN8(mlong2 + mg)
        + 0.0124 * SIN8(slong2 - mnode)
        + 0.0214 * SIN8(slong2 - sg)
        - 1.2729 * SIN8(slong2)
        - 0.0497 * SIN8(slong2 + sg)
        + 0.1261 * SIN8(sg)) / 3600.0;
    }
  }

  /* calculate the arguments sa[] for the disturbation terms */
  ti = (jd_ad - EPOCH1850) / 365.25;  /* julian years from 1850 */
  for (i = 0; i < SDNUM; i++)
    sa [i] = mod8360(_sd [i].sd0 + _sd [i].sd1 * ti);
  /*
  ** sa[2] += 0.3315 * SIN8 (DEGTORAD *(133.9099 + 38.39365 * el[SUN].tj));
  **
  ** correction of jupiter perturbation argument for sun from Pottenger;
  ** creates only .03" and 1e-7 rad, not applied because origin unclear */
  thelup = jd_ad;               /* note the last helup time */
}


/*
** hel()
** Computes the heliocentric positions for all planets except the moon.
** The outer planets from Jupiter onwards, including Chiron, are
** actually done by a subsequent call to outer_hel() which takes
** exactly the same parameters.
** hel() does true position relative to the mean ecliptic and equinox
** of date. Nutation is not added and must be done so by the caller.
** The latitude of the Sun (max. 0.5") is neglected and always returned
** as zero.
**
** return: OK or ERR
*/

int hel(int planet, REAL8 t, REAL8 *al, REAL8 *ar, REAL8 *az, REAL8 *alp,
  REAL8 *arp, REAL8 *azp)
/* planet index as defined by placalc.h */
/* relative juliand date, ephemeris time */
/* Now come 6 pointers to return values. */
/* longitude in degrees */
/* radius in AU */
/* distance from ecliptic in AU */
/* speed in longitude, degrees per day */
/* speed in radius, AU per day */
/* speed in z, AU per day */
{
  register struct elements *e;
  register struct eledata  *d;
  REAL8 lk = 0.0;
  REAL8 rk = 0.0;
  REAL8 b, h1, sini, sinv, cosi, cosu, cosv, man, truanom, esquare,
    k8, u, up, v, vp;

  if (planet >= JUPITER)
    return (outer_hel(planet, t, al, ar, az, alp, arp, azp));
  if (planet < SUN || planet == MOON)
    return ERR;

  e = &el[planet];
  d = &pd[planet];
  sini = SIN8(DEGTORAD * e->in);
  cosi = COS8(DEGTORAD * e->in);
  esquare = sqrt((1.0 + e->ex) / (1.0 - e->ex)); /* H6 in old version */
  man = e->ma;
  if (planet == EARTH)  /* some longperiodic terms in mean longitude */
    man += (0.266 * SIN8 (DEGTORAD * (31.8 + 119.0 * e->tj))
      + 6.40 * SIN8(DEGTORAD * (231.19 + 20.2 * e->tj))
      + (1.882-0.016*e->tj) * SIN8(DEGTORAD * (57.24 + 150.27 * e->tj))
      ) / 3600.0;
  if (planet == MARS)  /* some longperiodic terms */
    man += (0.606 * SIN8(DEGTORAD * (212.87 + e->tj * 119.051))
      + 52.490 * SIN8(DEGTORAD * (47.48 + e->tj * 19.771))
      +  0.319 * SIN8(DEGTORAD * (116.88 + e->tj * 773.444))
      +  0.130 * SIN8(DEGTORAD * (74 + e->tj * 163))
      +  0.280 * SIN8(DEGTORAD * (300 + e->tj * 40.8))
      -  (37.05 +13.5 * e->tj)
      ) / 3600.0;
  u = fnu(man, e->ex, 0.0000003); /* error 0.001" returns radians */
  cosu = COS8(u);
  h1 = 1 - e->ex * cosu;
  *ar = d->axis * h1;
  if (ABS8(rPi - u) < TANERRLIMIT)
    truanom = u; /* very close to aphel */
  else
    truanom = 2.0 * ATAN8(esquare * TAN8(u * 0.5)); /* true anomaly, rad*/
  v = smod8360(truanom * RADTODEG + e->pe - e->kn); /* argument of latitude */
  if (sini == 0.0 || ABS8(v -  90.0) < TANERRLIMIT
    || ABS8(v - 270.0) < TANERRLIMIT) {
    *al = v;
  } else {
    *al = RADTODEG * ATAN8(TAN8(v * DEGTORAD) * cosi);
    if (v > 90.0 && v < 270.0)  *al += 180.0;
  }
  *al = smod8360(*al + e->kn);
  sinv = SIN8(v * DEGTORAD);
  cosv = COS8(v * DEGTORAD);
  *az = *ar * sinv * sini;
  b = ASIN8(sinv * sini);     /* latitude in radians */
  k8 = cosv / COS8(b) * sini;
  up = 360.0 / d->period / h1;    /* du/dt degrees/day */
  if (ABS8(rPi - u) < TANERRLIMIT)
    vp = up / esquare;  /* speed at aphel */
  else
    vp = up * esquare * (1 + COS8 (truanom)) / (1 + cosu);
  /* dv/dt degrees/day */
  *arp = d->axis * up * DEGTORAD * SIN8(u) * e->ex;
  /* dr/dt AU/day */
  *azp = *arp * sinv * sini + *ar * vp * DEGTORAD * cosv * sini;  /* dz/dt */
  *alp = vp / cosi * (1 - k8 * k8);

  /* now come the disturbations */

  switch (planet) {
    REAL8 am, mma, ema, u2;

  case EARTH:
    /*
    ** earth has some special moon values and a disturbation series due to the
    ** planets. The moon stuff is due to the fact, that the mean elements
    ** give the coordinates of the earth-moon barycenter. By adding the
    ** corrections we effectively reduce to the center of the earth.
    ** We neglect the correction in latitude, which is about 0.5", because
    ** for astrological purposes we want the Sun to have latitude zero.
    */
    am = DEGTORAD * smod8360(el[MOON].lg - e->lg + 180.0); /* degrees */
    mma = DEGTORAD * el[MOON].ma;
    ema = DEGTORAD * e->ma;
    u2 = 2.0 * DEGTORAD * (e->lg - 180.0 - el[MOON].kn); /* 2u' */
    lk = 6.454 * SIN8(am)
      + 0.013 * SIN8(3.0 * am)
      + 0.177 * SIN8(am + mma)
      - 0.424 * SIN8(am - mma)
      + 0.039 * SIN8(3.0 * am - mma)
      - 0.064 * SIN8(am + ema)
      + 0.172 * SIN8(am - ema)
      - 0.013 * SIN8(am - mma - ema)
      - 0.013 * SIN8(u2);
    rk = 13360 * COS8(am)
      + 30    * COS8(3.0 * am)
      + 370   * COS8(am + mma)
      - 1330  * COS8(am - mma)
      + 80    * COS8(3.0 * am - mma)
      - 140   * COS8(am + ema)
      + 360   * COS8(am - ema)
      - 30    * COS8(am - mma - ema)
      + 30    * COS8(u2);
    /* long periodic term from mars 15g''' - 8g'', Vol 6 p19, p24 */
    lk += 0.202 * SIN8(DEGTORAD * (315.6 + 893.3 * e->tj));
    disturb(earthkor, al, ar, lk, rk, man);
    break;

  case MERCURY:  /* only normal disturbation series */
    disturb(mercurykor, al, ar, 0.0, 0.0, man);
    break;

  case VENUS:  /* some longperiod terms and normal series */
    lk = (2.761 - 0.22*e->tj) * SIN8(DEGTORAD * (237.24 + 150.27 * e->tj))
    + 0.269 * SIN8(DEGTORAD * (212.2  + 119.05 * e->tj))
    - 0.208 * SIN8(DEGTORAD * (175.8  + 1223.5 * e->tj));
    /* make seconds */
    disturb(venuskor, al, ar, lk, 0.0, man);
    break;

  case MARS:  /* only normal disturbation series */
    disturb(marskor, al, ar, 0.0, 0.0, man);
    break;
  }
  return OK;
}


void disturb(register struct kor *k, REAL8 *al, REAL8 *ar, REAL8 lk, REAL8 rk,
  REAL8 man)
/* ENDMARK-terminated array of struct kor */
/* longitude in degrees, use a pointer to return value */
/* radius in AU */
/* longitude correction in SECONDS OF ARC
   function can be called with an lk and rk already
   != 0, but no value is returned */
/* radius correction in units of 9th place of log r */
/* mean anomaly of planet */
{
  REAL8 arg;
  while (k->j != ENDMARK) {
    arg = k->j * sa[k->k] + k->i * man;
    lk += k->lampl * COS8(DEGTORAD * (k->lphase - arg));
    rk += k->rampl * COS8(DEGTORAD * (k->rphase - arg));
    k++;
  }
  *ar *= EXP10(rk * 1.0E-9);  /* 10^rk */
  *al += lk / 3600.0;
}


int moon(REAL8 *al, REAL8 *ar, REAL8 *az)  /* return OK or ERR */
{
  REAL8 a1,a2,a3,a4,a5,a6,a7,a8,a9,c2,c4,arg,b,d,f,dgc,dlm,dpm,dkm,dls;
  REAL8 ca, cb, cd, f_2d, f_4d, g1c,lk,lk1,man,ms,nib,s,sinarg,sinp,sk;
  REAL8 t, tb, t2c, r2rad, i1corr, i2corr, dlid;
  int i;
  struct elements *e;
  struct m45dat   *mp;
#if MOON_TEST_CORR
  struct m5dat    *m5p;
#endif
  e = &el[MOON];
  t = e->tj * 36525;  /* days from epoch 1900 */

  /* new format table II, parameters in full rotations of 360 degrees */
  r2rad = 360.0 * DEGTORAD;
  tb  = t * 1e-12;  /* units of 10^12 */
  t2c = t * t * 1e-16;  /* units of 10^16 */
  a1 = SIN8(r2rad * (0.53733431 -  10104982 * tb + 191 * t2c));
  a2 = SIN8(r2rad * (0.71995354 - 147094228 * tb +  43 * t2c));
  c2 = COS8(r2rad * (0.71995354 - 147094228 * tb +  43 * t2c));
  a3 = SIN8(r2rad * (0.14222222 +   1536238 * tb));
  a4 = SIN8(r2rad * (0.48398132 - 147269147 * tb +  43 * t2c));
  c4 = COS8(r2rad * (0.48398132 - 147269147 * tb +  43 * t2c));
  a5 = SIN8(r2rad * (0.52453688 - 147162675 * tb +  43 * t2c));
  a6 = SIN8(r2rad * (0.84536324 -  11459387 * tb));
  a7 = SIN8(r2rad * (0.23363774 +   1232723 * tb + 191 * t2c));
  a8 = SIN8(r2rad * (0.58750000 +   9050118 * tb));
  a9 = SIN8(r2rad * (0.61043085 -  67718733 * tb));

  dlm = 0.84 * a3 + 0.31 * a7 + 14.27 * a1 + 7.261  * a2 + 0.282 * a4
    + 0.237 * a6;
  dpm = -2.1  * a3 - 2.076  * a2 - 0.840 * a4 - 0.593 * a6;
  dkm = 0.63 * a3 + 95.96 * a2 + 15.58 * a4 + 1.86 * a5;
  dls = -6.4  * a3 - 0.27 * a8 - 1.89  * a6 + 0.20 * a9;
  dgc = (-4.318 * c2 - 0.698 * c4) / 3600.0 / 360.0;  /* in revolutions */
  dgc = (1.000002708 + 139.978 * dgc);  /* in this form used later */
  man = DEGTORAD * (e->ma + (dlm - dpm) / 3600.0);
  /* man with periodic and secular corr. */
  ms  = DEGTORAD * (el[EARTH].ma + dls / 3600.0);
  f   = DEGTORAD * (e->lg - e->kn + (dlm - dkm) / 3600.0);
  d   = DEGTORAD * (e->lg + 180 - el[EARTH].lg + (dlm - dls) / 3600.0);

  lk = lk1 = sk = sinp = nib = g1c = 0;
  i1corr = 1.0 - 6.8320E-8 * t;
  i2corr = dgc * dgc; /* i2 occurs only as -2, 2 */
  for (i = 0, mp = m45; i < NUM_MOON_CORR; i++, mp++) {
    /* arg = mp->i0 * man + mp->i1 * ms + mp->i2 * f + mp->i3 * d; */
    arg = mp->i0 * man;
    arg += mp->i3 * d;
    arg += mp->i2 * f;
    arg += mp->i1 * ms;
    sinarg = SIN8(arg);
    /*
    ** now apply corrections due to changes in constants;
    ** we correct only terms in l' (i1) and F (i2), not in l (i0), because
    ** the latter are < 0.05"
    ** We don't apply corrections  for cos(arg), i.e. for parallax
    */
    if (mp->i1 != 0) {  /* i1 can be -2, -1, 0, 1, 2 */
      sinarg *= i1corr;
      if  (mp->i1 == 2 || mp->i1 == -2)
        sinarg *= i1corr;
    }
    if (mp->i2 != 0)  /* i2 can be -2, 0, 2 */
      sinarg *= i2corr;
    lk += mp->lng * sinarg;
    sk += mp->lat * sinarg;
    sinp += mp->par * COS8 (arg) ;
  }

#if MOON_TEST_CORR  /* optionally add more lunar longitudes  */
  for (m5p = m5; m5p->i0 != 99; m5p++) {  /* i0 = 99 is end mark */
    arg = m5p->i0 * man + m5p->i1 * ms + m5p->i2 * f + m5p->i3 * d;
    sinarg = SIN8(arg);
    lk1 += m5p->lng * sinarg;
  }
#endif

  /*
  ** now compute some planetary terms in longitude, list i delta;
  ** we take all > 0.5" and neglect secular terms in the arguments. These
  ** produce phase errors > 10 degrees only after 3000 years.
  */
  dlid =  0.822 * SIN8 (r2rad * (0.32480 - 0.0017125594 * t));
  dlid += 0.307 * SIN8 (r2rad * (0.14905 - 0.0034251187 * t));
  dlid += 0.348 * SIN8 (r2rad * (0.68266 - 0.0006873156 * t));
  dlid += 0.662 * SIN8 (r2rad * (0.65162 + 0.0365724168 * t));
  dlid += 0.643 * SIN8 (r2rad * (0.88098 - 0.0025069941 * t));
  dlid += 1.137 * SIN8 (r2rad * (0.85823 + 0.0364487270 * t));
  dlid += 0.436 * SIN8 (r2rad * (0.71892 + 0.0362179180 * t));
  dlid += 0.327 * SIN8 (r2rad * (0.97639 + 0.0001734910 * t));

  /* without nutation */
  *al = smod8360(e->lg + (dlm + lk + lk1 + dlid) / 3600.0);

  /* solar Terms in latitude Nibeta */
  f_2d = f - 2.0 * d;
  f_4d = f - 4.0 * d;
  nib += -526.069 * SIN8(                   f_2d);
  nib +=   -3.352 * SIN8(                   f_4d);
  nib +=   44.297 * SIN8( man             + f_2d);
  nib +=   -6.000 * SIN8( man             + f_4d);
  nib +=   20.599 * SIN8(-man             + f   );
  nib +=  -30.598 * SIN8(-man             + f_2d);
  nib +=  -24.649 * SIN8(-2*man           + f   );
  nib +=   -2.000 * SIN8(-2*man           + f_2d);
  nib +=  -22.571 * SIN8(          ms     + f_2d);
  nib +=   10.985 * SIN8(         -ms     + f_2d);

  /* new gamma1C from 29 Jul 88, all terms > 0.4 " in table III, code 2 */
  g1c += -0.725 * COS8(          d);
  g1c +=  0.601 * COS8(      2 * d);
  g1c +=  0.394 * COS8(      3 * d);
  g1c += -0.445 * COS8(man                   + 4 * d);
  g1c +=  0.455 * COS8(man                   + 1 * d);
  g1c +=  5.679 * COS8(2 * man               - 2 * d);
  g1c += -1.300 * COS8(3 * man                      );
  g1c += -1.302 * COS8(            ms               );
  g1c += -0.416 * COS8(            ms        - 4 * d);
  g1c += -0.740 * COS8(        2 * ms        - 2 * d);
  g1c +=  0.787 * COS8(    man +   ms        + 2 * d);
  g1c +=  0.461 * COS8(    man +   ms               );
  g1c +=  2.056 * COS8(    man +   ms        - 2 * d);
  g1c += -0.471 * COS8(    man +   ms        - 4 * d);
  g1c += -0.443 * COS8(   -man +   ms        + 2 * d);
  g1c +=  0.679 * COS8(   -man +   ms               );
  g1c += -1.540 * COS8(   -man +   ms        - 2 * d);

  s =  f + sk / 3600.0 * DEGTORAD;
  ca = 18519.7 + g1c;
  cb = -0.000336992 * ca * dgc * dgc * dgc;
  cd = ca / 18519.7;
  b = (ca * SIN8(s) * dgc  + cb * SIN8(3.0 * s) + cd * nib) / 3600.0;

  /* we neglect the planetary terms in latitude, code 4 in table III */

  sinp = (sinp + 3422.451);
  /*
  ** Improved lunar ephemeris and APAE until ca. 1970 had here
  ** 3422.54 as constant of moon's sine parallax.
  ** The difference can be applied by direct addition of 0.089" to
  ** our parallax results.
  **
  ** To get the radius in A.U. from the sine parallax,
  ** we use 1964 IAU value 8.794" for solar parallax.
  ** sinp is still in seconds of arc.
  ** To calculate moon parallax in " it would be:
  ** p = sinp (1  + sinp * sinp * 3.917405E-12)
  ** based on the formula p = sinp + 1/6 sinp^3
  ** and taking into account the conversion of " to radians.
  ** The semidiameter of the moon is: (Expl.Suppl. 61, p 109)
  ** s = 0.0796 + 0.272446 * p
  */

  *ar = 8.794 / sinp;
  *az = *ar * SIN8(DEGTORAD * b);
  return OK;
}


/*
** outer_hel()
** Computes the position of Jupiter, Saturn, Uranus, Neptune, Pluto and
** Chiron by reading our stored ephemeris in steps of 80 (!) days and
** applying a high order interpolation to it. The interpolation errors are
** less than 0.01" seconds of arc.
** The stored ephemeris is packed in a special format consisting of
** 32 bit numbers; it has been created on the Astrodienst Unix system
** by numerical integration with routines provided originally by Marc
** Pottenger, USA, which we improved for better long term precision.
** Because the Unix system uses a different byte order than the MSDOS
** systems, the bytes must be reordered for MSDOS after reading from
** the binary files.
**
** outer_hel() takes the same parameters as hel().
** It returns the same type of values.
**
** The access to the ephemeris files is done in the functions chi_file_posit()
** and lrz_file_posit().
*/

int outer_hel(int planet, REAL8 jd_ad, REAL8 *al, REAL8 *ar, REAL8 *az,
  REAL8 *alp, REAL8 *arp, REAL8 *azp)
/* jd_ad Astrodienst relative Julian ephemeris time */
{
  static FILE *outerfp = NULL, *chironfp = NULL, *asterfp = NULL;
  static double last_j0_outer = HUGE8;
  static double last_j0_chiron = HUGE8;
  static double last_j0_aster = HUGE8;
  static long icoord[6][5][3], chicoord[6][3], ascoord[6][4][3];
  REAL8 j0, jd, jfrac;
  REAL8 l[6], r[6], z[6];
  int n, order, p;

  if ((planet < JUPITER || planet > PLUTO) && planet != CHIRON &&
    (planet < CERES || planet > VESTA))
    return ERR;
  jd = jd_ad + JUL_OFFSET;
  j0 = RFloor((jd - 0.5) / EPHE_STEP) * EPHE_STEP + 0.5;
  jfrac = (jd - j0) / EPHE_STEP;
  if (planet == CHIRON) {
    if (last_j0_chiron != j0) {
      for (n = 0; n < 6; n++) { /* read 6 days */
        jd = j0 + (n - 2) * EPHE_STEP;
        if (chi_file_posit(jd, &chironfp) != OK)
          return ERR;
        fread(&chicoord[n][0], sizeof(word4), 3, chironfp);
        longreorder((UCHAR *)&chicoord[n][0], 3*4);
      }
      last_j0_chiron = j0;
    }
    for (n = 0; n < 6; n++) {
      l[n] = chicoord[n][0] / DEG2MSEC;
      r[n] = chicoord[n][1] / AU2INT;
      z[n] = chicoord[n][2] / AU2INT;
    }
  } else if (planet >= CERES && planet <= VESTA) {
    if (last_j0_aster != j0) {  /* read all 4 asteroids for 6 steps */
      for (n = 0; n < 6; n++) {
        jd = j0 + (n - 2) * EPHE_STEP;
        if (ast_file_posit(jd, &asterfp) != OK)
          return ERR;
        fread(&ascoord[n][0][0], sizeof(word4), 12, asterfp);
        longreorder((UCHAR *)&ascoord[n][0][0], 12*4);
      }
      last_j0_aster = j0;
    }
    p = planet - CERES;
    for (n = 0; n < 6; n++) {
      l[n] = ascoord[n][p][0] / DEG2MSEC;
      r[n] = ascoord[n][p][1] / AU2INT;
      z[n] = ascoord[n][p][2] / AU2INT;
    }
  } else {  /* an outerplanet */
    if (last_j0_outer != j0) {  /* read all 5 planets for 6 steps */
      for (n = 0; n < 6; n++) {
        jd = j0 + (n - 2) * EPHE_STEP;
        if (lrz_file_posit(jd, &outerfp) != OK)
          return ERR;
        fread(&icoord[n][0][0], sizeof(word4), 15, outerfp);
        longreorder((UCHAR *)&icoord[n][0][0], 15*4);
      }
      last_j0_outer = j0;
    }
    p = planet - JUPITER;
    for (n = 0; n < 6; n++) {
      l[n] = icoord[n][p][0] / DEG2MSEC;
      r[n] = icoord[n][p][1] / AU2INT;
      z[n] = icoord[n][p][2] / AU2INT;
    }
  }
  if (planet > SATURN)
    order = 3;
  else
    order = 5;
  inpolq(2, order, jfrac, l, al, alp);
  *alp /= EPHE_STEP;
  inpolq(2, order, jfrac, r, ar, arp);
  *arp /= EPHE_STEP;
  inpolq(2, order, jfrac, z, az, azp);
  *azp /= EPHE_STEP;
  return OK;
}


/*
** quicker Everett interpolation, after Pottenger
** version 9 Jul 1988 by Alois Treindl
** return OK or ERR.
*/

int inpolq(int n, int o, double p, double x[], double *axu, double *adxu)
/* interpolate between x[n] and x[n-1], at argument n+p */
/* order of interpolation, maximum 5 */
/* argument , intervall [0..1] */
/* array of function values, x[n-o]..x[n+o] must exist */
/* pointer for storage of result */
/* pointer for storage of dx/dt  */
{
  static double q, q2, q3, q4, q5, p2, p3, p4, p5, u, u0, u1, u2;
  static double lastp = 9999;
  double dm2, dm1, d0, dp1, dp2,
    d2m1, d20, d2p1, d2p2, d30, d3p1, d3p2, d4p1, d4p2;
  double offset = 0.0;

  if (lastp != p) {
    q  = 1.0-p;
    q2 = q*q;
    q3 = (q+1.0)*q*(q-1.0)/6.0;       /* q - 1 over 3; u5 */
    p2 = p*p;
    p3 = (p+1.0)*p*(p-1.0)/6.0;       /* p - 1 over 3; u8 */
    u  = (3.0*p2-1.0)/6.0;
    u0 = (3.0*q2-1.0)/6.0;
    q4 = q2*q2;                       /* f5 */
    p4 = p2*p2;                       /* f4 */
    u1 = (5.0*p4-15.0*p2+4.0)/120.0;  /* u1 */
    u2 = (5.0*q4-15.0*q2+4.0)/120.0;  /* u2 */
    q5 = q3*(q+2.0)*(q-2.0)/20.0;     /* q - 2 over 5; u6 */
    p5 = (p+2.0)*p3*(p-2.0)/20.0;     /* p - 2 over 5; u9 */
    lastp = p;
  }
  dm1 = x[n] - x[n-1];
  if (dm1 > 180.0)
    dm1 -= 360.0;
  if (dm1 < -180.0)
    dm1 += 360.0;
  d0 = x[n+1] - x[n];
  if (d0 > 180.0) {
    d0 -= 360.0;
    offset = 360.0;
  }
  if (d0 < -180.0) {
    d0 += 360.0;
    offset = -360.0;
  }
  dp1 = x[n+2] - x[n+1];
  if (dp1 > 180.0)
    dp1 -= 360.0;
  if (dp1 < -180.0)
    dp1 += 360.0;
  d20  = d0 - dm1;    /* f8 */
  d2p1 = dp1 - d0;    /* f9 */

  /* Everett interpolation 3rd order */

  *axu = q*(x[n] + offset) + q3*d20 + p*x[n+1] + p3*d2p1;
  *adxu = d0 + u*d2p1 - u0*d20;
  if (o > 3) {    /* 5th order */
    dm2 = x[n-1] - x[n-2];
    if (dm2 > 180.0)
      dm2 -= 360.0;
    if (dm2 < -180.0)
      dm2 += 360.0;
    dp2 = x[n+3] - x[n+2];
    if (dp2 > 180.0)
      dp2 -= 360.0;
    if (dp2 < -180.0)
      dp2 += 360.0;
    d2m1 = dm1 - dm2;
    d2p2 = dp2 - dp1;
    d30  = d20 - d2m1;
    d3p1 = d2p1 - d20;
    d3p2 = d2p2 - d2p1;
    d4p1 = d3p1 - d30;     /* f7 */
    d4p2 = d3p2 - d3p1;    /* f */
    *axu  += p5*d4p2 + q5*d4p1;
    *adxu += u1*d4p2 - u2*d4p1;
  }
  return OK;
}


/*
** position lrz file at proper position for julian date jd;
** Return OK or ERR.  Version for outer planets.
*/

int lrz_file_posit(double jd, FILE **lrzfpp)
/* full Julian day number, not Astrodienst relative */
/* pointer to file pointer; this function
   opens or closes the ephemeris file, and caller
   should keep it open while using it. The caller
   should close it when he is finished with using
   the placalc() package. */
{
  int filenr;
  long posit, jlong;
  char fname[cchSzDef];
  static int open_lrznr = -10000; /* local memory to remember whether
    an already open file is the one with
    the correct number for this date */

  jlong = (long)RFloor(jd);
  filenr = (int)(jlong / EPHE_DAYS_PER_FILE);
  if (jlong < 0 && filenr * EPHE_DAYS_PER_FILE != jlong)
    filenr--;
  posit = jlong - filenr * EPHE_DAYS_PER_FILE;
  posit = (posit / (int)EPHE_STEP) * EPHE_OUTER_BSIZE;
  if (*lrzfpp == NULL || open_lrznr != filenr) { /* no or wrong open file */
    open_lrznr = -10000;
    sprintf(fname, "%s%s%d", EPHE_OUTER, filenr < 0 ? "M" : "", abs(filenr));
    if (*lrzfpp != NULL)
      fclose(*lrzfpp);
    *lrzfpp = FileOpen(fname, 2, NULL);
    if (*lrzfpp == NULL) {
      ErrorEphem(fname, -1);
      return ERR;
    }
    open_lrznr = filenr;
  }
  if (fseek(*lrzfpp, posit, 0) == 0)
    return OK;
  ErrorEphem(fname, posit);
  return ERR; /* this fseek error occurs only with incomplete files */
}


/*
** position cpjv file at proper position for julian date jd;
** Return OK or ERR.  Version for asteroids.
** Sister function to lrz_file_posit().
*/

int ast_file_posit(double jd, FILE **astfpp)
/* full Julian day number, not Astrodienst relative */
/* pointer to file pointer; this function
   opens or closes the ephemeris file, and caller
   should keep it open while using it. */
{
  int filenr;
  long posit, jlong;
  char fname[cchSzDef];
  static int open_astnr = -10000; /* local memory to remember whether
    an already open file is the one with
    the correct number for this date */

  jlong = (long)RFloor(jd);
  filenr = (int)(jlong / EPHE_DAYS_PER_FILE);
  if (jlong < 0 && filenr * EPHE_DAYS_PER_FILE != jlong)
    filenr--;
  posit = jlong - filenr * EPHE_DAYS_PER_FILE;
  posit = (posit / (int)EPHE_STEP) * EPHE_ASTER_BSIZE;
  if (*astfpp == NULL || open_astnr != filenr) { /* no or wrong open file */
    open_astnr = -10000;
    sprintf(fname, "%s%s%d", EPHE_ASTER, filenr < 0 ? "M" : "", abs(filenr));
    if (*astfpp != NULL)
      fclose(*astfpp);
    *astfpp = FileOpen(fname, 2, NULL);
    if (*astfpp == NULL) {
      ErrorEphem(fname, -1);
      return ERR;
    }
    open_astnr = filenr;
  }
  if (fseek(*astfpp, posit, 0) == 0)
    return OK;
  ErrorEphem(fname, posit);
  return ERR; /* this fseek error occurs only with incomplete files */
}


/*
** position chiron file at proper position for julian date jd;
** Return OK or ERR. Version for Chiron.
*/

int chi_file_posit(double jd, FILE **lrzfpp)
/* full Julian day number, not Astrodienst relative */
/* pointer to file pointer; this function
   opens or closes the ephemeris file, and caller
   should keep it open while using it */
{
  int filenr;
  long posit, jlong;
  char fname[cchSzDef];
  static int open_lrznr = -10000; /* local memory to remember whether
    an already open file is the one with
    the correct number for this date */

  jlong = (long)RFloor(jd);
  filenr = (int)(jlong / EPHE_DAYS_PER_FILE);
  if (jlong < 0 && filenr * EPHE_DAYS_PER_FILE != jlong)
    filenr--;
  posit = jlong - filenr * EPHE_DAYS_PER_FILE;
  posit = (posit / (int)EPHE_STEP) * EPHE_CHIRON_BSIZE;
  if (*lrzfpp == NULL || open_lrznr != filenr) { /* no or wrong open file */
    open_lrznr = -10000;
    sprintf(fname, "%s%s%d", EPHE_CHIRON, filenr < 0 ? "M" : "", abs(filenr));
    if (*lrzfpp != NULL)
      fclose(*lrzfpp);
    *lrzfpp = FileOpen(fname, 2, NULL);
    if (*lrzfpp == NULL) {
      ErrorEphem(fname, -1);
      return ERR;
    }
    open_lrznr = filenr;
  }
  if (fseek (*lrzfpp, posit, 0) == 0)
    return OK;
  ErrorEphem(fname, posit);
  return ERR; /* this fseek error occurs only with incomplete files */
}
#endif /* PLACALC */

/* placalc.cpp */
