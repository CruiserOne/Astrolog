/*
** Astrolog (Version 7.30) File: extern.h
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

/*
******************************************************************************
** Function Declarations.
******************************************************************************
*/

#ifdef PROTO
#define P(t) t
#else
#define P(t) ()
#endif

// From astrolog.cpp

extern void InitColors P((void));
extern void Action P((void));
extern void InitVariables P((void));
extern flag FProcessCommandLine P((char *));
extern int NParseCommandLine P((char *, char **));
extern int NPromptSwitches P((char *, char *[MAXSWITCHES]));
extern int NProcessSwitchesRare P((int, char **, int, flag, flag, flag));
extern flag FProcessSwitches P((int, char **));
extern void InitProgram P((void));
extern void FinalizeProgram P((flag));


// From data.cpp

#define MM ciCore.mon
#define DD ciCore.day
#define YY ciCore.yea
#define TT ciCore.tim
#define ZZ ciCore.zon
#define SS ciCore.dst
#define OO ciCore.lon
#define AA ciCore.lat

#define Mon ciMain.mon
#define Day ciMain.day
#define Yea ciMain.yea
#define Tim ciMain.tim
#define Zon ciMain.zon
#define Dst ciMain.dst
#define Lon ciMain.lon
#define Lat ciMain.lat

#define MonT ciTran.mon
#define DayT ciTran.day
#define YeaT ciTran.yea
#define TimT ciTran.tim
#define ZonT ciTran.zon
#define DstT ciTran.dst
#define LonT ciTran.lon
#define LatT ciTran.lat

#define planet    cp0.obj
#define planetalt cp0.alt
#define ret       cp0.dir
#define retalt    cp0.diralt
#define retlen    cp0.dirlen
#define chouse    cp0.cusp
#define chouse3   cp0.cusp3
#define inhouse   cp0.house

#define FIgnoreA(a) (ignorea[a] || rAspOrb[a] < 0.0)
#define DEFAULT_LOC DMS(122, 19, 55), DMS(47, 36, 22)
#define FProperGraph(o) !(!us.fGraphAll && (FCusp(o) || \
  (us.fInDayMonth && ((o) == oMoo || (us.fInDayYear && (o) <= oMar)))))

#define FCmSwissAny() (us.fEphemFiles && !us.fPlacalcPla)
#define FCmSwissEph() (us.fEphemFiles && !us.fPlacalcPla && us.nSwissEph <= 0)
#define FCmSwissMosh() (us.fEphemFiles && !us.fPlacalcPla && us.nSwissEph == 1)
#define FCmSwissJPL() (us.fEphemFiles && !us.fPlacalcPla && us.nSwissEph == 2)
#define FCmSwissStar() (us.fEphemFiles && !us.fPlacalcPla && !us.fMatrixStar)
#define FCmPlacalc() (us.fEphemFiles && us.fPlacalcPla)
#define FCmMatrix() (!us.fEphemFiles && us.fMatrixPla)
#define FCmJPLWeb() (us.fEphemFiles && !us.fPlacalcPla && us.nSwissEph >= 3)

extern US us;
extern IS is;
extern CI ciCore, ciMain, ciTwin, ciThre, ciFour, ciFive, ciHexa,
  ciTran, ciSave, ciGreg;
extern CP cp0, cp1, cp2, cp3, cp4, cp5, cp6;
extern CP * CONST rgpcp[cRing+1];
extern CI * CONST rgpci[cRing+1];

extern PT3R space[objMax];
extern real force[objMax];
extern GridInfo *grid;
extern int rgobjList[objMax], starname[cStar+1], kObjA[objMax];

extern byte ignore[objMax], ignore2[objMax], ignorea[cAspect+1],
  ignorez[arMax], ignore7[rrMax], pluszone[cSector+1];
extern real rAspAngle[cAspect+1], rAspOrb[cAspect+1], rObjOrb[oNorm+2],
  rObjAdd[oNorm+2];
extern int ruler1[oNorm+1], ruler2[oNorm+1], exalt[oNorm+1], rules[cSign+1],
  kMainA[9], kRainbowA[cRainbow+1], kElemA[cElem], kAspA[cAspect+1],
  kObjU[oNorm+2];
extern real rObjInf[oNorm1+6], rHouseInf[cSign+6], rAspInf[cAspect+1],
  rTransitInf[oNorm1+1];

#define kBlackA   kMainA[0]
#define kWhiteA   kMainA[1]
#define kLtGrayA  kMainA[2]
#define kDkGrayA  kMainA[3]
#define kMaroonA  kMainA[4]
#define kDkGreenA kMainA[5]
#define kDkCyanA  kMainA[6]
#define kDkBlueA  kMainA[7]
#define kMagentaA kMainA[8]

#define kRedA    kRainbowA[1]
#define kOrangeA kRainbowA[2]
#define kYellowA kRainbowA[3]
#define kGreenA  kRainbowA[4]
#define kCyanA   kRainbowA[5]
#define kBlueA   kRainbowA[6]
#define kPurpleA kRainbowA[7]

#define KStarA(mag) ((mag) < 1.0 ? kOrangeA : kMaroonA)
#define KStar2A(mag) \
  ((mag) < 2.0 ? kWhiteA : ((mag) < 4.0 ? kLtGrayA : kDkGrayA))

extern CONST char *szAppName, *szSignName[cSign+1], *szSignAbbrev[cSign+1],
  *szSignEnglish[cSign+1], *szHouseTradition[cSign+1], *szSystem[cSystem],
  *szAspectName[cAspect2+1], *szAspectAbbrev[cAspect2+1],
  *szAspectGlyph[cAspect2+1], *szAspectConfig[cAspConfig],
  *szElem[cElem], *szMode[3], *szMonth[cSign+1], *szDay[cWeek], *szZon[cZone],
  *szDir[4], *szSuffix[cSign+1], *szEphem[cmMax];
extern CONST int rgAspConfig[cAspConfig];
extern CONST real rZon[cZone];
extern CONST char *szObjName[objMax+4], *szCnstlName[cCnstl+1],
  *szCnstlAbbrev[cCnstl+1], *szCnstlMeaning[cCnstl+1],
  *szCnstlGenitive[cCnstl+1];
extern CONST real rStarBrightMatrix[cStar+1], rStarData[cStar*6];
extern char *szMindPart[oNorm+1], *szDesc[cSign+1], *szDesire[cSign+1],
  *szLifeArea[cSign+1], *szInteract[cAspect+1], *szTherefore[cAspect+1],
  *szModify[3][cAspect];
extern CONST StrLook rgObjName[], rgSystem[], rgAspectName[];
extern CONST StrLookR rgZodiacOffset[];
extern CONST char *szEclipse[etMax], rgchEclipse[etMax+1];
extern real rStarBrightDef[cStar+1], rStarBright[cStar+1],
  rStarDistDef[cStar+1], rStarDist[cStar+1];
extern char *szStarCustom[cStar+1];
extern CONST char *szObjDisp[objMax], *szAspectDisp[cAspect2+1],
  *szAspectAbbrevDisp[cAspect2+1], *szAspectGlyphDisp[cAspect2+1];

extern CONST real rObjDist[oNorm+1], rObjYear[oNorm+1], rObjDay[oNorm+1],
  rObjMass[oPlu+1], rObjAxis[oPlu+1];
extern real rObjDiam[oNorm+1];
extern CONST int cSatellite[oPlu+1], nMooMap[6][8];
extern CONST AI ai[cPart];

extern char *szMacro[48], *szWheel[cRing+1];
extern CONST char *szColor[cColor+4], *szColorHTML[cColor];

extern int rgObjRay[oNorm+1], rgSignRay[cSign+1], rgSignRay2[cSign+1][cRay+1],
  rgObjEso1[oNorm+1], rgObjEso2[oNorm+1], rgObjHie1[oNorm+1],
  rgObjHie2[oNorm+1], rgSignEso1[cSign+1], rgSignEso2[cSign+1],
  rgSignHie1[cSign+1], rgSignHie2[cSign+1], kRayA[cRay+2];
extern CONST char *szRayName[cRay+1], *szRayWill[cRay+1];


// From general.cpp

#define RgAllocate(n, t, sz) ((t *)PAllocate((n) * sizeof(t), sz))
#define PrintAltitude(deg) PrintSz(SzAltitude(deg))
#define FEqCI(ci1, ci2) (\
  ci1.mon == ci2.mon && ci1.day == ci2.day && ci1.yea == ci2.yea && \
  ci1.tim == ci2.tim && ci1.dst == ci2.dst && ci1.zon == ci2.zon && \
  ci1.lon == ci2.lon && ci1.lat == ci2.lat && \
  (ci1.nam == ci2.nam || NCompareSz(ci1.nam, ci2.nam) == 0) && \
  (ci1.loc == ci2.loc || NCompareSz(ci1.loc, ci2.loc) == 0))

#define PtSet(pt, a, b, c) pt.x = a; pt.y = b; pt.z = c;
#define PtLen(pt) RLength3(pt.x, pt.y, pt.z)
#define PtMul(pt, r) pt.x *= (r); pt.y *= (r); pt.z *= (r);
#define PtDiv(pt, r) pt.x /= (r); pt.y /= (r); pt.z /= (r);
#define PtAdd2(pt, pt2) pt.x += pt2.x; pt.y += pt2.y; pt.z += pt2.z;
#define PtSub2(pt, pt2) pt.x -= pt2.x; pt.y -= pt2.y; pt.z -= pt2.z;
#define PtNeg2(pt, pt2) pt.x = -pt2.x; pt.y = -pt2.y; pt.z = -pt2.z;
#define PtVec(pt, pt1, pt2) pt = pt2; PtSub2(pt, pt1);
#define PtDot(pt1, pt2) (pt1.x*pt2.x + pt1.y*pt2.y + pt1.z*pt2.z)
#define PtCross(pt, pt1, pt2) pt.x = pt1.y*pt2.z - pt1.z*pt2.y; \
  pt.y = pt1.z*pt2.x - pt1.x*pt2.z; pt.z = pt1.x*pt2.y - pt1.y*pt2.x;

extern void SwapR P((real *, real *));
extern int CchSz P((CONST char *));
extern int NCompareSz P((CONST char *, CONST char *));
extern int NCompareSzI P((CONST char *, CONST char *));
extern flag FEqRgch P((CONST char *, CONST char *, int, flag));
extern flag FMatchSz P((CONST char *, CONST char *));
extern CONST char *SzInList P((CONST char *, CONST char *, int *));
extern void ClearB P((pbyte, int));
extern void CopyRgb P((byte *, byte *, int));
extern void CopyRgchToSz P((CONST char *, int, char *, int));
extern real RSgn P((real));
extern real RAngle P((real, real));
extern real VAngle P((CONST PT3R *, CONST PT3R *));
extern real Mod P((real));
extern long Dvd P((long, long));
extern int SzLookup P((CONST StrLook *, CONST char *));
extern flag FCompareSzSubI P((CONST char *, CONST char *));
extern void FormatR P((char *, real, int));
extern KV KvBlend P((KV, KV, real));
extern KV KvHue P((real));
extern int Mod12 P((int));
extern real DecToDeg P((real));
extern real DegToDec P((real));
extern real MinDistance P((real, real));
extern real MinDifference P((real, real));
extern real Midpoint P((real, real));
extern real SphDistance P((real, real, real, real));
extern void SphRatio P((real, real, real, real, real, real *, real *));
extern char *Dignify P((int, int));
extern void EnsureRay P((void));
extern void EnsureStarBright P((void));
extern int DayInMonth P((int, int));
extern int DaysInMonth P((int, int));
extern int DayOfWeek P((int, int, int));
extern int AddDay P((int, int, int, int));
extern real GetOrb P((int, int, int));
extern CONST char *SzAspect P((int));
extern CONST char *SzAspectAbbrev P((int));
extern void SetCentric P((int));
extern int ObjOrbit P((int));
extern int ObjMoons P((int));
extern real RObjDiam P((int));
extern void Terminate P((int));
extern void PrintSz P((CONST char *));
extern void PrintCh P((char));
extern void PrintSzScreen P((CONST char *));
extern void PrintProgress P((CONST char *));
extern void PrintNotice P((CONST char *));
extern void PrintWarning P((CONST char *));
extern void PrintError P((CONST char *));
extern flag FErrorArgc P((CONST char *, int, int));
extern flag FErrorValN P((CONST char *, flag, int, int));
extern flag FErrorValR P((CONST char *, flag, real, int));
extern void ErrorArgv P((CONST char *));
extern void ErrorSwitch P((CONST char *));
extern void ErrorEphem P((CONST char *, long));
extern void PrintTab P((char, int));
extern void AnsiColor P((int));
extern void PrintZodiac P((real));
extern char *SzZodiac P((real));
extern char *SzAltitude P((real));
extern char *SzDegree P((real));
extern char *SzHMS P((int));
extern char *SzDate P((int, int, int, int));
extern char *SzTime P((int, int, int));
extern char *SzTim P((real));
extern char *SzZone P((real));
extern char *SzLocation P((real, real));
extern char *SzElevation P((real));
extern char *SzLength P((real));
extern void GetTimeNow P((int *, int *, int *, real *, real, real));
extern int NFromAltN P((int));
extern char *SzProcessProgname P((char *));
extern char *SzPersist P((char *));
extern pbyte PAllocate P((long, CONST char *));
extern void DeallocateP P((void *));
#ifdef DEBUG
extern void Assert P((flag));
#else
#define Assert(f)
#endif


// From io.cpp

#define getbyte() BRead(file)

extern FILE *FileOpen P((CONST char *, int, char *));
extern byte BRead P((FILE *));
extern word WRead P((FILE *));
extern dword LRead P((FILE *));
extern flag FProcessSwitchFile P((CONST char *, FILE *));
extern flag FOutputData P((void));
extern flag FProcessAAFFile P((CONST char *, FILE *));
extern flag FOutputAAFFile P((void));
extern flag FOutputSettings P((void));
extern int NFromSz P((CONST char *));
extern real RFromSz P((CONST char *));
extern int NParseSz P((CONST char *, int));
extern real RParseSz P((CONST char *, int));
extern void InputString P((CONST char *, char *));
extern int NInputRange P((CONST char *, int, int, int));
extern real RInputRange P((CONST char *, real, real, int));
extern flag FInputData P((CONST char *));
extern flag GetJPLHorizons P((int,
  real *, real *, real *, real *, real *, real *, char *));


// From calc.cpp

#define JulianDayFromTime(t) ((t)*36525.0+2415020.0)
#define NHousePlaceIn2D(deg) NHousePlaceIn(deg, 0.0)
#define EclToEqu(Z, L) CoorXform(Z, L, is.OB)
#define EquToEcl(Z, L) CoorXform(Z, L, -is.OB)
#define EquToLocal(Z, L, T) CoorXform(Z, L, T)
#define Tropical(deg) ((deg) - is.rSid)
#define Untropical(deg) ((deg) + is.rSid)
#define AdjustRestrictions() for (is.nObj = cObj; is.nObj >= 0 && \
  ignore[is.nObj] && ignore2[is.nObj]; is.nObj--);

extern long MdyToJulian P((int, int, int));
extern real MdytszToJulian P((int, int, int, real, real, real));
extern void JulianToMdy P((real, int *, int *, int *));
extern real RHousePlaceIn3DCore P((real, real));
extern real RHousePlaceIn3D P((real, real));
extern int NHousePlaceIn P((real, real));
extern void ComputeInHouses P((void));
extern void ComputeHouses P((int));
extern void ComputeStars P((real, real));
extern real Decan P((real));
extern real Navamsa P((real));
extern void RecToPol P((real, real, real *, real *));
extern void SphToRec P((real, real, real, real *, real *, real *));
extern void RecToSph3 P((real, real, real, real *, real *));
extern void CoorXform P((real *, real *, real));
extern void CoorXformFast P((real *, real *,
  real, real, real, real, real, real));
extern void ComputeEphem P((real));
extern real CastChart P((int));
extern void CastSectors P((void));
extern flag FEnsureGrid P((void));
extern flag FAcceptAspect P((int, int, int));
extern int GetAspect P((CONST real *, CONST real *, CONST real *,
  CONST real *, CONST real *, CONST real *, int, int, real *));
extern int GetParallel P((CONST real *, CONST real *, CONST real *,
  CONST real *, CONST real *, CONST real *, int, int, real *));
extern flag FCreateGrid P((flag));
extern flag FCreateGridRelation P((flag));
extern int NCheckEclipse P((int, int, real *));
extern int NCheckEclipseLunar P((int, int, real *));
extern void CreateElemTable P((ET *));

#ifdef SWISS
extern int rgObjSwiss[cCust], rgTypSwiss[cCust], rgPntSwiss[cCust],
  rgFlgSwiss[cCust];

extern flag FSwissPlanet
  P((int, real, int, real *, real *, real *, real *, real *, real *));
extern void SwissHouse P((real, real, real, int,
  real *, real *, real *, real *, real *, real *, real *, real *));
extern void SwissComputeStars P((real, flag));
extern flag SwissComputeStar P((real, ES *));
extern flag SwissComputeAsteroid P((real, ES *, flag));
extern void SwissGetObjName P((char *, int));
extern flag FSwissPlanetData P((real, int, real *, real *, real *));
extern double SwissJulDay P((int, int, int, real, int));
extern void SwissRevJul P((real, int, int *, int *, int *, double *));
#endif


#ifdef MATRIX
// From matrix.cpp

#define IoeFromObj(obj) \
  ((obj) < oMoo ? 0 : ((obj) <= cPlanet ? (obj)-2 : (obj)-uranLo+cPlanet-2))

extern OE rgoe[oVes+cUran-2];

extern long MatrixMdyToJulian P((int, int, int));
extern void MatrixJulianToMdy P((real, int *, int *, int *));
extern real ProcessInput P((void));
extern void PolToRec P((real, real, real *, real *));
extern real RecToSph P((real, real, real));
extern void ComputeVariables P((real *));
extern real CuspMidheaven P((void));
extern real CuspAscendant P((void));
extern real CuspEastPoint P((void));
extern void HousePlacidus P((void));
extern void HouseKoch P((void));
extern void HouseCampanus P((void));
extern void HouseMeridian P((void));
extern void HouseRegiomontanus P((void));
extern void HouseMorinus P((void));
extern void HouseTopocentric P((void));
extern real ReadThree P((real, real, real));
extern void RecToSph2 P((real, real, real, real *, real *, real *));
extern void ErrorCorrect P((int, real *, real *, real *));
extern void ProcessPlanet P((int, real));
extern void ComputePlanets P((void));
extern void ComputeLunar P((real *, real *, real *, real *));
#endif


#ifdef PLACALC
// From placalc2.cpp

extern flag FPlacalcPlanet
  P((int, real, flag, real *, real *, real *, real *, real *, real *));
extern double julday P((int, int, int, double, int));
extern void revjul P((double, int, int *, int *, int *, double *));
#endif


// From charts0.cpp

extern void PrintW P((CONST char *, int));
extern void DisplayCredits P((void));
extern void PrintS P((CONST char *));
extern void DisplaySwitches P((void));
extern void DisplaySwitchesRare P((void));
extern void PrintObjects P((void));
extern void PrintAspects P((void));
extern void PrintSigns P((void));
#ifdef CONSTEL
extern CONST char *GetSzGenitive P((char *, CONST char *));
extern void GetSzConstel P((char *, int));
extern void PrintConstellations P((void));
#endif
extern void PrintOrbit P((void));
extern int NCompareSzPart P((int, int));
extern void DisplayArabic P((void));
#ifdef GRAPH
extern void DisplayKeysX P((void));
extern void DisplaySwitchesX P((void));
#ifdef WIN
extern void DisplaySwitchesW P((void));
#endif
#endif // GRAPH
extern flag FPrintTables P((void));


// From charts1.cpp

extern void PrintHeader P((void));
extern void ChartListing P((void));
extern void ChartGrid P((void));
extern void DisplayAspectConfigs P((void));
extern void ChartWheel P((void));
extern void PrintAspectSummary P((int *, int *, int, real));
extern void ChartAspect P((void));
extern void PrintAspectsToPoint P((real, int, real, char *));
extern void PrintMidpointSummary P((int *, int, long));
extern void ChartMidpoint P((void));
extern void ChartHorizon P((void));
extern void ChartOrbit P((void));
extern void ChartSector P((void));
extern void ChartAstroGraph P((void));
extern void PrintChart P((flag));


// From charts2.cpp

#define RBiorhythm(day, rate) (RSin(((day)/(rate))*rPi2)*100.0)

extern void ChartListingRelation P((void));
extern void ChartGridRelation P((void));
extern void ChartAspectRelation P((void));
extern void ChartMidpointRelation P((void));
extern void CastRelation P((void));
extern void PrintInDayEvent P((int, int, int, int));
extern void PrintAspect P((int, real, int, int, int, real, int, char));
extern void ChartInDayInfluence P((void));
extern void ChartTransitInfluence P((flag));
extern void ChartTransitGraph P((flag, flag));
extern void EclToHoriz P((real *, real *, real, real, real, real));
extern void ChartCalendarMonth P((void));
extern void ChartCalendarYear P((void));
extern void DisplayRelation P((void));


// From charts3.cpp

#define FAspectVoid(obj1, obj2, asp) (FBetween(asp, aCon, aSex) && \
  ((obj1) == oMoo || (obj2) == oMoo) && (obj1) <= oPlu && (obj2) <= oPlu)

extern void ChartInDaySearch P((flag));
extern void ChartTransitSearch P((flag));
extern void ChartInDayHorizon P((void));
extern void ChartEphemeris P((void));


// From intrpret.cpp

#define RObjInf(i) rObjInf[Min(i, oNorm1)]
#define RTransitInf(i) rTransitInf[Min(i, oNorm1)]

#ifdef INTERPRET
extern void FieldWord P((CONST char *));
extern void InterpretGeneral P((void));
extern void InterpretAspectGeneral P((void));
extern void InterpretLocation P((void));
extern void InterpretAspectCore P((int, int, int, int));
extern void InterpretAspect P((int, int));
extern void InterpretGrid P((void));
extern void InterpretMidpoint P((int, int));
extern void InterpretInDay P((int, int, int));
extern void InterpretTransit P((int, int, int));
extern void InterpretSynastry P((void));
extern void InterpretAspectRelation P((int, int));
extern void InterpretGridRelation P((void));
extern void InterpretMidpointRelation P((int, int));
extern void InterpretAstroGraph P((int, int, int, int));
#endif
extern void SortRank P((real *, int *, int, flag));
extern void ComputeInfluence P((real[oNorm+1], real[oNorm+1]));
extern void ChartInfluence P((void));


#ifdef ATLAS
// From atlas.cpp

extern char *SzCity P((int));
extern flag FEnsureAtlas P((void));
extern flag FEnsureTimezoneChanges P((void));
extern flag FLoadAtlas P((FILE *, int));
extern flag FLoadZoneRules P((FILE *, int, int));
extern flag FLoadZoneChanges P((FILE *, int, int));
extern flag FLoadZoneLinks P((FILE *, int));
extern flag DisplayAtlasLookup P((CONST char *, size_t, int *));
extern flag DisplayAtlasNearby P((real, real, size_t, int *, flag));
extern flag DisplayTimezoneChanges P((int, size_t, CI *));
extern real ZondefFromIzn P((int));
#endif


#ifdef EXPRESS
// From express.cpp

#define ParseExpression(sz) NParseExpression(sz)
extern char *rgszExpMacro[cLetter+1];

extern int ILookupTrie P((CONST TRIE, CONST char *, int, flag));
extern long NParseExpression P((CONST char *));
extern real RParseExpression P((CONST char *));
extern flag ShowParseExpression P((CONST char *));
extern int NExpGet P((int));
extern real RExpGet P((int));
extern void ExpSetN P((int, int));
extern void ExpSetR P((int, real));
extern char *PchFormatExpression P((char *, int));
extern void ExpInit P((void));
#endif


#ifdef GRAPH
// From xdata.cpp

extern GS gs;
extern GI gi;

#ifdef X11
extern XSizeHints hint;
extern char xkey[];
extern KV rgbind[cColor];
extern KV fg, bg;
#endif
#ifdef WIN
extern CONST int ikPalette[cColor];
extern CONST int rgcmdMode[gMax];
#endif
#ifdef WCLI
extern WI wi;
#endif
extern char *szWheelX[cRing+1];

extern CONST KV rgbbmpDef[cColor], rgbbmpDef2[cColor];
extern KV rgbbmp[cColor];
extern KI kMainB[9], kRainbowB[cRainbow+1], kElemB[cElem], kAspB[cAspect+1],
  kObjB[objMax], kRayB[cRay+2];
extern CONST char *rgszFontName[cFont], rgszFontAllow[5][cFont+1];
extern CONST char
  *szDrawSign[cSign+2], *szDrawSign2[cSign+2], *szDrawSign3[cSign+2],
  *szDrawObjectDef[objMax+9], *szDrawObjectDef2[objMax+9],
  *szDrawHouse[cSign+1], *szDrawHouse2[cSign+1], *szDrawHouse3[cSign+1],
  *szDrawAspectDef[cAspect2+1], *szDrawAspectDef2[cAspect2+1],
  *szDrawCh[256-32], *szWorldData[62*3], *szDrawConstel[cCnstl+1];
extern CONST char *szDrawObject[objMax+9], *szDrawObject2[objMax+9],
  *szDrawAspect[cAspect2+1], *szDrawAspect2[cAspect2+1];

#define kBlackB   kMainB[0]
#define kWhiteB   kMainB[1]
#define kLtGrayB  kMainB[2]
#define kDkGrayB  kMainB[3]
#define kMaroonB  kMainB[4]
#define kDkGreenB kMainB[5]
#define kDkCyanB  kMainB[6]
#define kDkBlueB  kMainB[7]
#define kMagentaB kMainB[8]

#define kRedB    kRainbowB[1]
#define kOrangeB kRainbowB[2]
#define kYellowB kRainbowB[3]
#define kGreenB  kRainbowB[4]
#define kCyanB   kRainbowB[5]
#define kBlueB   kRainbowB[6]
#define kPurpleB kRainbowB[7]

#define KStarB(mag) \
  ((mag) < 2.0 ? gi.kiOn : ((mag) < 4.0 ? gi.kiLite : gi.kiGray))


// From xgeneral.cpp

#define BBmGet(rg, x, y) ((rg)[(y)*gi.cbBmpRow + ((x) >> 1)])
#define FBmGet(rg, x, y) (BBmGet(rg, x, y) >> (((x)&1^1) << 2) & 15)
#define BmSet(rg, x, y, o) BBmGet(rg, x, y) = BBmGet(rg, x, y) & \
  15 << (((x)&1) << 2) | (o) << (((x)&1^1) << 2)

#define DrawEdge(x1, y1, x2, y2) \
  DrawBox(x1, y1, x2, y2, gi.nScaleT, gi.nScaleT)
#define DrawEdgeAll() DrawEdge(0, 0, gs.xWin-1-gs.fThick, gs.yWin-1-gs.fThick)
#define DrawLine(x1, y1, x2, y2) DrawDash(x1, y1, x2, y2, 0)
#define DrawClip(x1, y1, x2, y2, xl, yl, xh, yh, skip) \
  FDrawClip(x1, y1, x2, y2, xl, yl, xh, yh, skip, NULL, NULL)
#define DrawLineX(x1, x2, y) DrawBlock(x1, y, x2, y)
#define DrawLineY(x, y1, y2) DrawBlock(x, y1, x, y2)
#define DrawEllipse(x1, y1, x2, y2) DrawArc(x1, y1, x2, y2, 0.0, 0.0, rDegMax)
#define DrawCircle(x, y, xr, yr) \
  DrawEllipse((x)-(xr), (y)-(yr), (x)+(xr), (y)+(yr))
#define DrawCircle2(x, y, xr, yr) \
  DrawEllipse2((x)-(xr), (y)-(yr), (x)+(xr), (y)+(yr))

extern void DrawColor P((KI));
extern void DrawPoint P((int, int));
extern void DrawSpot P((int, int));
extern void DrawBlock P((int, int, int, int));
extern void DrawBox P((int, int, int, int, int, int));
extern void WinClearScreen P((KI));
extern void DrawClearScreen P((void));
extern void DrawDash P((int, int, int, int, int));
extern void DrawWrap P((int, int, int, int, int, int));
extern void ClipLesser P((int *, int *, int *, int *, int));
extern void ClipGreater P((int *, int *, int *, int *, int));
extern flag FDrawClip P((int, int, int, int, int, int, int, int, int,
  int *, int *));
extern void DrawArc P((int, int, int, int, real, real, real));
extern void DrawEllipse2 P((int, int, int, int));
extern void DrawCrescent P((int, int, int, int, real, real, KI, KI));
extern void DrawFill P((int, int, KV));
extern void DrawSz P((CONST char *, int, int, int));
extern void DrawSign P((int, int, int));
extern void DrawHouse P((int, int, int));
extern void DrawObject P((int, int, int));
#ifdef SWISS
extern void DrawStar P((int, int, ES *));
#endif
extern void DrawAspect P((int, int, int));
extern int NFromPch P((CONST char **));
extern void DrawTurtle P((CONST char *, int, int));
extern KI KiCity P((int));


// From xdevice.cpp

#define PutByte(n) putc((byte)(n), file)
#define PutWord(n) PutByte(BLo(n)); PutByte(BHi(n))
#define PutLong(n) PutWord(WLo(n)); PutWord(WHi(n))
#define getword() WRead(file)
#define getlong() LRead(file)
#define skipbyte() ch = getbyte()
#define skipword() skipbyte(); skipbyte()
#define skiplong() skipword(); skipword()

#ifdef PS
#define PsEscape(ch) ((ch) == '(' || (ch) == ')' || (ch) == '\\' ? "\\" : "")
#endif
#ifdef META
// Macros to output the various metafile commands used.
#define MetaRecord(S, R) MetaLong((long)(S)); MetaWord(R)
#define MetaSelectObject(O) MetaRecord(4, 0x12D); MetaWord(O)
#define MetaDeleteObject(O) MetaRecord(4, 0x1F0); MetaWord(O)
#define MetaSaveDc() MetaRecord(3, 0x01E)
#define MetaRestoreDc() MetaRecord(4, 0x127); MetaWord((word)-1)
#define MetaWindowOrg(X, Y) MetaRecord(5, 0x20B); MetaWord(Y); MetaWord(X)
#define MetaWindowExt(X, Y) MetaRecord(5, 0x20C); MetaWord(Y); MetaWord(X)
#define MetaCreatePen(S, W, C) MetaRecord(8, 0x2FA); MetaWord(S); \
  MetaWord(W); MetaWord(W); MetaLong(C)
#define MetaCreateBrush(S, C) MetaRecord(7, 0x2FC); \
  MetaWord(S); MetaLong(C); MetaWord(0 /* Not used */);
#define MetaCreateFont(S, X, Y, C) MetaRecord(12+(S), 0x2FB); MetaWord(Y); \
  MetaWord(X); MetaWord(0 /* Angle */); MetaWord(0 /* Not used */); \
  MetaWord(400 /* Normal Weight */); MetaWord(0 /* Italic, Underline */); \
  MetaWord(WFromBB(0 /* Strikeout */, C)); \
  MetaWord(WFromBB(4 /* TrueType */, 0 /* Clip */))
#define MetaBkMode(M) MetaRecord(4, 0x102); MetaWord(M)
#define MetaTextAlign(A) MetaRecord(4, 0x12E); MetaWord(A)
#define MetaTextColor(C) MetaRecord(5, 0x209); MetaLong(C);
#define MetaTextOut(X, Y, S) MetaRecord(7+((S)+1)/2, 0xA32); \
  MetaWord(Y); MetaWord(X); MetaWord(S); MetaWord(0 /* ETO */)
#define MetaPoint(X, Y, C) MetaRecord(7, 0x41F); MetaLong(C); \
  MetaWord(Y); MetaWord(X);
#define MetaRectangle(X1, Y1, X2, Y2) MetaRecord(7, 0x41B); \
  MetaWord(Y2); MetaWord(X2); MetaWord(Y1); MetaWord(X1)
#define MetaEllipse(X1, Y1, X2, Y2) MetaRecord(7, 0x418); \
  MetaWord(Y2); MetaWord(X2); MetaWord(Y1); MetaWord(X1)
#define MetaFill(X, Y, C) MetaRecord(8, 0x548); \
  MetaWord(1 /* FLOODFILLSURFACE */); MetaLong(C); MetaWord(Y); MetaWord(X);
#define MetaEscape(S) MetaRecord(S, 0x626); \
  MetaWord(15 /* MFCOMMENT */); MetaWord(((S)-5)*2 /* Bytes in comment */);
#endif // META

#define WirePoint(x, y, z) WireLine(x, y, z, x, y, z)

extern void BmpSetXY P((Bitmap *, int, int, KV));
extern KV BmpGetXY P((Bitmap *, int, int));
extern void SetXY P((int, int, KI));
extern KI GetXY P((int, int));
extern KI BmGetXY P((int, int));
extern flag FAllocateBmp P((Bitmap *, int, int));
extern flag FLoadBmp P((CONST char *, Bitmap *, flag));
extern void BmpCopyBlock P((CONST Bitmap *, int, int, int, int,
  Bitmap *, int, int, int, int));
#ifdef WINANY
extern void BmpCopyWin P((CONST Bitmap *, HDC, int, int));
#endif
extern flag FBmpDrawBack P((Bitmap *));
extern flag FBmpDrawMap P((void));
extern flag FBmpDrawMap2 P((int, int, int, int, real, real, real, real));
extern void WriteXBitmap P((FILE *, CONST char *, char));
extern void WriteAscii P((FILE *));
extern void WriteBmp P((FILE *));
extern flag BeginFileX P((void));
extern void EndFileX P((void));
extern void PsStrokeForce P((void));
extern void PsStroke P((int));
extern void PsLineCap P((flag));
extern void PsDash P((int));
extern void PsLineWidth P((int));
extern void PsFont P((int));
extern void PsBegin P((void));
extern void PsEnd P((void));
extern void MetaWord P((word));
extern void MetaLong P((long));
extern void MetaSz P((CONST char *));
extern void MetaSelect P((void));
extern void MetaInit P((void));
extern void WriteMeta P((FILE *));
extern void WriteWire P((FILE *));
extern void WireLine P((int, int, int, int, int, int));
extern void WireDrawGlobe P((flag, real));
extern void WireChartOrbit P((void));
extern void WireChartSphere P((void));


// From xcharts0.cpp

extern int DrawPrint P((CONST char *, int, int));
extern void DrawSidebar P((void));
extern void DrawFillWheel P((int, int, int, int));
extern void DrawWheel P((real *, real *, int, int, real, real,
  real, real, real));
extern void DrawRing P((int, int, real *, real *, int, int, real,
  real, real, real, real, real, real, real));
extern void DrawObjects P((ObjDraw *, int, int));
extern void DrawAspectLine
  P((int, int, int, int, real, real, real, real, real));
extern flag EnumWorldLines P((int *, int *, int *, int *, int *));
#ifdef CONSTEL
extern flag EnumConstelLines P((int *, int *, int *, int *, int *));
#endif
extern flag FReadWorldData P((CONST char **, CONST char **, CONST char **));
extern flag FGlobeCalc P((real, real, int *, int *, int, int, int, int, int));
#ifdef SWISS
extern flag EnumStarsLines(flag, ES **, ES **);
#endif

extern void DrawLeyLine P((real, real, real, real));
extern void DrawLeyLines P((int));
extern void DrawMap P((flag, flag, real));
extern void DrawChartX P((void));


// From xcharts1.cpp

extern void LocToHorizon P((real, real, int, int, int, int, int *, int *));
extern void EquToHorizon P((real, real, int, int, int, int, int *, int *));
extern void EclToHorizon P((real, real, int, int, int, int, int *, int *));
extern void LocToHorizonSky P((real, real, CONST CIRC *, int *, int *));
extern void EquToHorizonSky P((real, real, CONST CIRC *, int *, int *));
extern void EclToHorizonSky P((real, real, CONST CIRC *, int *, int *));
extern void LocToTelescope P((real, real, TELE *, int *, int *, real*, real*));
extern void EquToTelescope P((real, real, TELE *, int *, int *, real*, real*));
extern void EclToTelescope P((real, real, TELE *, int *, int *, real*, real*));

extern void XChartWheel P((void));
extern void XChartAstroGraph P((void));
extern void XChartGrid P((void));
extern void XChartHorizon P((void));
extern void XChartHorizonSky P((void));
extern void XChartTelescope P((void));
extern void XChartLocal P((void));
extern void OrbitPlot P((real *, real *, real *, real, int, PT3R *));
extern void OrbitRecord P((void));
extern void XChartOrbit P((void));
extern void XChartSector P((void));
extern void DrawArrow P((int, int, int, int));
extern void XChartDispositor P((void));
extern flag DrawCalendarAspect P((InDayInfo *, int, int, int));
extern void XChartCalendar P((void));
extern void XChartSphere P((void));


// From xcharts2.cpp

extern flag FProper P((int));
extern void FillSymbolRing P((real *, real));
extern void FillSymbolLine P((real *));
extern real PlaceInX P((real));
extern real HousePlaceInX P((real, real));
extern void XChartWheelRelation P((void));
extern void XChartWheelMulti P((void));
extern void XChartGridRelation P((void));
extern void XChartEphemeris P((void));
extern void XChartEsoteric P((void));
extern void XChartTransit P((flag, flag));
extern flag XChartRising P((void));
extern void XChartBiorhythm P((void));


// From xscreen.cpp

extern void InitColorPalette P((int));
extern void InitColorsX P((void));
#ifdef ISG
extern void ResizeWindowToChart P((void));
extern void BeginX P((void));
extern void AddTime P((CI *, int, int));
extern void Animate P((int, int));
extern void CommandLineX P((void));
extern void SquareX P((int *, int *, flag));
extern void InteractX P((void));
extern void EndX P((void));
#endif
extern int NProcessSwitchesX P((int, char **, int, flag, flag, flag));
extern int NProcessSwitchesRareX P((int, char **, int, flag, flag, flag));
extern int DetectGraphicsChartMode P((void));
extern flag FActionX P((void));
#endif // GRAPH


#ifdef WIN
// From wdriver.cpp

extern WI wi;
extern OPENFILENAME ofn;
extern PRINTDLG prd;
extern char szFileName[cchSzMaxFile], szFileTitle[cchSzMaxFile], *szFileTemp;

#define TextClearScreen() WinClearScreen(gs.fInverse ? kWhiteA : kBlackA)
#define CheckMenu(cmd, f) \
  CheckMenuItem(wi.hmenu, (uint)cmd, f ? MF_CHECKED : MF_UNCHECKED);
#define CheckPopup(cmd, f) \
  CheckMenuItem(hmenu, (uint)cmd, f ? MF_CHECKED : MF_UNCHECKED);
#define WiCheckMenu(cmd, f) CheckMenu(cmd, f); wi.fMenu = fTrue
#define RadioMenu(cmd1, cmd2, i) \
  CheckMenuRadioItem(wi.hmenu, (uint)cmd1, (uint)cmd2, i, MF_BYCOMMAND);
#define WiRadioMenu(cmd1, cmd2, i) RadioMenu(cmd1, cmd2, i); wi.fMenu = fTrue
#define WiDoDialog(pfn, dlg) \
  dlgproc = (DLGPROC)MakeProcInstance(pfn, wi.hinst); \
  DialogBox(wi.hinst, MAKEINTRESOURCE(dlg), wi.hwnd, dlgproc); \
  FreeProcInstance((FARPROC)dlgproc)

#define SetCheck(id, f) CheckDlgButton(hdlg, id, f)
#define SetRadio(id, idLo, idHi) CheckRadioButton(hdlg, idLo, idHi, id)
#define SetEdit(id, sz) SetDlgItemText(hdlg, id, (LPCSTR)sz)
#define SetEditN(id, n) SetDlgItemInt(hdlg, id, n, fTrue)
#define SetList(id, sz) \
  SendDlgItemMessage(hdlg, id, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)sz)
#define SetListN(id, sz, n, v) \
  v = SetList(id, sz); \
  SendDlgItemMessage(hdlg, id, LB_SETITEMDATA, v, (LPARAM)n);
#define ClearList(id) SendDlgItemMessage(hdlg, id, LB_RESETCONTENT, 0, 0);
#define SetCombo(id, sz) \
  SendDlgItemMessage(hdlg, id, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)sz)
#define ClearCombo(id) SendDlgItemMessage(hdlg, id, CB_RESETCONTENT, 0, 0);
#define GetCheck(id) IsDlgButtonChecked(hdlg, id)
#define GetEdit(id, sz) GetDlgItemText(hdlg, id, sz, cchSzMax)
#define EnsureN(n, f, sz) if (!(f)) { ErrorEnsure(n, sz); return fTrue; }
#define EnsureR(r, f, sz) EnsureN((int)r, f, sz)

extern LRESULT API WndProc P((HWND, UINT, WPARAM, LPARAM));
extern int NProcessSwitchesW P((int, char **, int, flag, flag, flag));
extern void BootExternal P((CONST char *, CONST char *));
extern WORD WCmdFromRc P((int));
extern void SetRel P((int));
extern void ProcessState P((void));
extern int NWmCommand P((WORD));
extern void API RedoMenu P((void));
extern flag FFilePaste P((void));
extern flag API FRedraw P((void));
extern flag FCreateDesktopIcon P((void));
extern flag FCreateProgramGroup P((flag));
extern flag FRegisterExtensions P((void));
extern flag FUnregisterExtensions P((void));


// From wdialog.cpp

extern void SetEditSz(HWND, int, CONST char *);
extern void SetEditR(HWND, int, real, int);
extern void SetEditMDYT(HWND, int, int, int, int, int, int, int, real);
extern void SetEditSZOA(HWND, int, int, int, int, real, real, real, real);
extern void SetEditColor(HWND, int, KI);
extern int GetEditN(HWND, int);
extern real GetEditR(HWND, int);
extern void ErrorEnsure(int, CONST char *);
extern flag API DlgOpenChart P((void));
extern flag API DlgSaveChart P((void));
extern flag API DlgPrint P((void));
extern flag API DlgAbortProc P((HDC, int));
extern BOOL API DlgAbort P((HWND, uint, WPARAM, LPARAM));
extern flag API DlgCommand P((HWND, uint, WORD, LONG));
extern flag API DlgColor P((HWND, uint, WORD, LONG));
extern flag API DlgInfo P((HWND, uint, WORD, LONG));
extern flag API DlgDefault P((HWND, uint, WORD, LONG));
extern flag API DlgInfoAll P((HWND, uint, WORD, LONG));
extern flag API DlgAspect P((HWND, uint, WORD, LONG));
extern flag API DlgObject P((HWND, uint, WORD, LONG));
extern flag API DlgObject2 P((HWND, uint, WORD, LONG));
extern flag API DlgObjectM P((HWND, uint, WORD, LONG));
extern flag API DlgCustom P((HWND, uint, WORD, LONG));
extern flag API DlgRestrict P((HWND, uint, WORD, LONG));
extern flag API DlgStar P((HWND, uint, WORD, LONG));
extern flag API DlgMoons P((HWND, uint, WORD, LONG));
extern flag API DlgCalc P((HWND, uint, WORD, LONG));
extern flag API DlgDisplay P((HWND, uint, WORD, LONG));
extern flag API DlgTransit P((HWND, uint, WORD, LONG));
extern flag API DlgProgress P((HWND, uint, WORD, LONG));
extern flag API DlgChart P((HWND, uint, WORD, LONG));
extern flag API DlgGraphics P((HWND, uint, WORD, LONG));
extern flag API DlgAbout P((HWND, uint, WORD, LONG));
#endif // WIN

/* extern.h */
