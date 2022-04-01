/*
** Astrolog (Version 7.40) File: intrpret.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2022 by
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
** Last code change made 3/31/2022.
*/

#include "astrolog.h"


#ifdef INTERPRET
/*
******************************************************************************
** Interpretation Routines.
******************************************************************************
*/

// This function is used by the interpretation routines to print out lines of
// text with newlines inserted just before the end of screen is reached.

void FieldWord(CONST char *sz)
{
  static char line[cchSzMax];
  static int cursor = 0;
  int ich = 0, i, j;
  char ch;

  // Display buffer if function called with a null string.

  if (sz == NULL)
    sz = "\n";
  else if (cursor > 0)
    line[cursor++] = ' ';
  loop {
    ch = sz[ich];
    if (ch == chNull)
      break;
    if (ch == '\n') {
      line[cursor] = 0;
      PrintSz(line); PrintL();
      cursor = 0;
      ich++;
      continue;
    }
    line[cursor] = ch;

    // When buffer overflows 'n' columns, display one line and start over.

    if (cursor >= us.nScreenWidth-1) {
      for (i = us.nScreenWidth-1; i > 2 && line[i] != ' '; i--)
        ;
      if (i <= 2)
        i = us.nScreenWidth-1;
      line[i] = 0;
      PrintSz(line); PrintL();
      line[0] = line[1] = ' ';
      for (j = 2; (line[j] = line[j+i-1]) != 0; j++)
        ;
      cursor -= (i-1);
    }
    ich++, cursor++;
  }
}


// Display a general interpretation of what each sign of the zodiac, house,
// and planet or object means. This is called to do the -HI switch table.

void InterpretGeneral(void)
{
  char sz[cchSzMax];
  int i, j;

  FieldWord(
    "Signs of the zodiac represent psychological characteristics.\n\n");
  for (i = 1; i <= cSign; i++) {
    AnsiColor(kSignA(i));
    sprintf(sz, "%s is", szSignName[i]); FieldWord(sz);
    sprintf(sz, "%s, and", szDesc[i]); FieldWord(sz);
    sprintf(sz, "%s.\n", szDesire[i]); FieldWord(sz);
  }
  AnsiColor(kDefault);

  PrintL();
  FieldWord("Houses represent different areas within one's life.\n\n");
  for (i = 1; i <= cSign; i++) {
    AnsiColor(kSignA(i));
    sprintf(sz, "The %d%s House is the area of life dealing with",
      i, szSuffix[i]); FieldWord(sz);
    sprintf(sz, "%s.\n", szLifeArea[i]); FieldWord(sz);
  }
  AnsiColor(kDefault);

  PrintL();
  FieldWord("Planets represent various parts of one's mind or self.\n\n");
  for (j = 0; j <= Min(is.nObj, oNorm); j++) {
    i = rgobjList[j];
    if (ignore[i] || FCusp(i))
      continue;
    AnsiColor(kObjA[i]);
    if (i <= oMoo || (FBetween(i, oNod, oCore) && i != oLil))
      FieldWord("The");
    sprintf(sz, "%s%s%s represents one's", i == oFor ? "Part of " : "",
      szObjDisp[i], i == oPal ? " Athena" : ""); FieldWord(sz);
    sprintf(sz, "%s.\n", szMindPart[i]); FieldWord(sz);
  }
  AnsiColor(kDefault);
}


// Display a general interpretation of what each aspect type means. This
// is called when printing the interpretation table in the -HI switch.

void InterpretAspectGeneral(void)
{
  char sz[cchSzMax];
  int i;

  PrintL();
  FieldWord("Aspects are different relationships between planets.\n\n");
  for (i = 1; i <= us.nAsp; i++) {
    if (!FInterpretAsp(i))
      continue;
    AnsiColor(kAspA[i]);
    sprintf(sz, "When planets are %s, one", szAspectName[i]);
    FieldWord(sz); sprintf(sz, szInteract[i], ""); FieldWord(sz);
    FieldWord("another.");
    if (szTherefore[i][0]) {
      sprintf(sz, "%s.", szTherefore[i]); FieldWord(sz);
    }
    FieldWord(NULL);
  }
  return;
}


// Print the interpretation of each planet in sign and house, as specified
// with the -I switch. This is basically array accessing combining the
// meanings of each planet, sign, and house, and a couple of other things.

void InterpretLocation(void)
{
  char sz[cchSzMax], c;
  int i, j;

  PrintL();
  for (i = 0; i <= is.nObj; i++) {
    if (ignore[i] || !FInterpretObj(i))
      continue;
    AnsiColor(kObjA[i]);
    j = SFromZ(planet[i]); c = *Dignify(i, j);
    sprintf(sz, "%s%s%s%s in %s", ret[i] < 0.0 ? "Retrograde " : "",
      i == oFor && szObjDisp[i] == szObjName[i] ? "Part of " : "",
      szObjDisp[i],
      i == oPal && szObjDisp[i] == szObjName[i] ? " Athena" : "",
      szSignName[j]);
    FieldWord(sz);
    sprintf(sz, "and %d%s House:", inhouse[i], szSuffix[inhouse[i]]);
    FieldWord(sz);
    sprintf(sz, "%s's", szPerson); FieldWord(sz);
    FieldWord(szMindPart[i]); FieldWord("is");
    if (((int)planet[i]) % 30 < 10)
      FieldWord("very");
    sprintf(sz, "%s, and", szDesc[j]); FieldWord(sz);
    sprintf(sz, "%s.", szDesire[j]); FieldWord(sz);
    FieldWord("Most often this manifests");
    if (ret[i] < 0.0 && i != oNod && i != oSou)
      FieldWord("in an independent, backward, introverted manner, and");
    FieldWord("in the area of life dealing with");
    sprintf(sz, "%s.", szLifeArea[inhouse[i]]); FieldWord(sz);

    // Extra information if planet is in its ruling, exalting, etc, sign.

    if (c == 'R')
      FieldWord("This is a major part of their psyche!");
    else if (c == 'd')
      FieldWord("(This bit plays only a minor part in their psyche.)");
    else if (c == 'X')
      FieldWord("It is easy for them to express this part of themselves.");
    else if (c == 'f')
      FieldWord(
        "It is difficult for them to express this part of themselves.");
    FieldWord(NULL);
  }
}


// Print an interpretation for a particular aspect in effect in a chart.

void InterpretAspectCore(int x, int asp, int y, int nOrb)
{
  char sz[cchSzMax];

  if (!FInterpretAsp(asp) || !FInterpretObj(x) || !FInterpretObj(y))
    return;
  AnsiColor(kAspA[asp]);
  sprintf(sz, "%s %s %s: %s's",
    szObjDisp[x], SzAspect(asp), szObjDisp[y], szPerson);
  FieldWord(sz); FieldWord(szMindPart[x]);
  sprintf(sz, szInteract[asp],
    szModify[Min(nOrb, 2)][asp-1]);
  FieldWord(sz);
  sprintf(sz, "their %s.", szMindPart[y]); FieldWord(sz);
  if (szTherefore[asp][0]) {
    sprintf(sz, "%s.", szTherefore[asp]); FieldWord(sz);
  }
  FieldWord(NULL);
}


// Like InterpretAspectCore() but get data from the aspect grid. This is
// called from the InterpretGrid() and ChartAspect() routines.

void InterpretAspect(int x, int y)
{
  int nOrb;

  nOrb = NAbs(grid->v[x][y]) / (150*60);
  InterpretAspectCore(x, grid->n[x][y], y, nOrb);
}


// Print the interpretation of each aspect in the aspect grid, as specified
// with the -g -I switch. Again, this is done by basically array accessing of
// the meanings of the two planets in aspect and of the aspect itself.

void InterpretGrid(void)
{
  int i, j;

  for (i = 0; i < is.nObj; i++) if (!ignore[i])
    for (j = i+1; j <= is.nObj; j++) if (!ignore[j])
      InterpretAspect(i, j);
  AnsiColor(kDefault);
}


// Print an interpretation for a particular midpoint in effect in a chart.
// This is called from the ChartMidpoint() routine.

void InterpretMidpoint(int x, int y)
{
  char sz[cchSzMax];
  int n, i;
  real rT = 0.0;

  if (!FInterpretObj(x) || !FInterpretObj(y))
    return;
  n = grid->n[y][x];
  AnsiColor(kSignA(n));
  sprintf(sz, "%s midpoint %s in %s: The merging of %s's",
    szObjDisp[x], szObjDisp[y], szSignName[n], szPerson0);
  FieldWord(sz); FieldWord(szMindPart[x]);
  FieldWord("with their"); FieldWord(szMindPart[y]);
  FieldWord("is");
  if (grid->v[y][x]/3600 < 10)
    FieldWord("very");
  sprintf(sz, "%s, and", szDesc[n]); FieldWord(sz);
  sprintf(sz, "%s.", szDesire[n]); FieldWord(sz);
  FieldWord("Most often this manifests in");
  // Nodes are always retrograde, so that shouldn't contribute to text.
  if (!FBetween(x, oNod, oSou))
    rT += ret[x];
  if (!FBetween(y, oNod, oSou))
    rT += ret[y];
  if (rT < 0.0)
    FieldWord("an independent, backward, introverted manner, and");
  FieldWord("the area of life dealing with");
  i = NHousePlaceIn2D(ZFromS(n) + (real)grid->v[y][x]/3600.0);
  sprintf(sz, "%s.", szLifeArea[i]); FieldWord(sz);
  FieldWord(NULL);
}


// This is a subprocedure of ChartInDaySearch(). Print the interpretation for
// a particular instance of the various exciting events that can happen.

void InterpretInDay(int source, int aspect, int dest)
{
  char sz[cchSzMax];

  // Interpret object changing direction.

  if (aspect == aDir && FInterpretObj(source)) {
    AnsiColor(kObjA[source]);
    FieldWord("Energy representing"); FieldWord(szMindPart[source]);
    FieldWord("will tend to manifest in");
    FieldWord(dest ? "an independent, backward, introverted" :
      "the standard, direct, open");
    FieldWord("manner.\n");

  // Interpret object entering new sign.

  } else if (aspect == aSig) {
    AnsiColor(kObjA[source]);
    FieldWord("Energy representing"); FieldWord(szMindPart[source]);
    sprintf(sz, "will be %s,", szDesc[dest]);
    FieldWord(sz);
    sprintf(sz, "and it %s.\n", szDesire[dest]); FieldWord(sz);

  // Interpret aspect between transiting planets.

  } else if (FInterpretAsp(aspect) &&
    FInterpretObj(source) && FInterpretObj(dest)) {
    AnsiColor(kAspA[aspect]);
    FieldWord("Energy representing"); FieldWord(szMindPart[source]);
    sprintf(sz, szInteract[aspect], szModify[1][aspect-1]);
    FieldWord(sz);
    sprintf(sz, "energies of %s.", szMindPart[dest]); FieldWord(sz);
    if (szTherefore[aspect][0]) {
      if (aspect > aCon) {
        sprintf(sz, "%s.", szTherefore[aspect]); FieldWord(sz);
      } else
        FieldWord("They will affect each other prominently.");
    }
    FieldWord(NULL);
  }
}


// This is called from ChartTransitSearch() and ChartTransitInfluence(). Print
// the interpretation for a particular transit of a planet making an aspect to
// a natal object of a chart.

void InterpretTransit(int source, int aspect, int dest)
{
  char sz[cchSzMax];

  // Interpret transiting planet forming aspect.

  if (FInterpretObj(source) && FInterpretAsp(aspect) && FInterpretObj(dest)) {
    AnsiColor(kAspA[aspect]);
    FieldWord("Energy representing"); FieldWord(szMindPart[source]);
    sprintf(sz, szInteract[aspect], szModify[1][aspect-1]);
    FieldWord(sz);
    if (source != dest) {
      sprintf(sz, "%s's %s.", szPerson0, szMindPart[dest]);
    } else {
      sprintf(sz, "the same area inside %s's makeup.", szPerson0);
    }
    FieldWord(sz);
    if (szTherefore[aspect][0]) {
      if (aspect > aCon) {
        sprintf(sz, "%s.", szTherefore[aspect]); FieldWord(sz);
      } else
        FieldWord("This part of their psyche will be strongly influenced.");
    }
    FieldWord(NULL);

  // Interpret transiting planet changing 3D house.

  } else if (aspect == aHou && FInterpretObj(source)) {
    AnsiColor(kSignA(dest));
    FieldWord("Energy representing"); FieldWord(szMindPart[source]);
    sprintf(sz, "is now affecting %s's area of life dealing with %s.\n",
      szPerson0, szLifeArea[dest]);
    FieldWord(sz);
  }
}


// Print the interpretation of one person's planet in another's sign and
// house, in a synastry chart as specified with the -r switch combined with
// -I. This is very similar to the interpretation of the standard -v chart in
// InterpretLocation(), but treat the chart as a relationship here.

void InterpretSynastry(void)
{
  char sz[cchSzMax], c;
  int i, j;

  PrintL();
  for (i = 0; i <= is.nObj; i++) {
    if (ignore[i] || !FInterpretObj(i))
      continue;
    AnsiColor(kObjA[i]);
    j = SFromZ(planet[i]); c = *Dignify(i, j);
    sprintf(sz, "%s%s%s%s in %s,", ret[i] < 0.0 ? "Retrograde " : "",
      i == oFor && szObjDisp[i] == szObjName[i] ? "Part of " : "",
      szObjDisp[i],
      i == oPal && szObjDisp[i] == szObjName[i] ? " Athena" : "",
      szSignName[j]);
    FieldWord(sz);
    sprintf(sz, "in their %d%s House:", inhouse[i], szSuffix[inhouse[i]]);
    FieldWord(sz);
    sprintf(sz, "%s's", szPerson2); FieldWord(sz);
    FieldWord(szMindPart[i]); FieldWord("is");
    if (((int)planet[i]) % 30 < 10)
      FieldWord("very");
    sprintf(sz, "%s, and", szDesc[j]); FieldWord(sz);
    sprintf(sz, "%s.", szDesire[j]); FieldWord(sz);
    FieldWord("This");
    if (ret[i] < 0.0 && !FBetween(i, oNod, oSou))
      FieldWord(
        "manifests in an independent, backward, introverted manner, and");
    sprintf(sz, "affects %s in the area of life dealing with %s.",
      szPerson1, szLifeArea[inhouse[i]]); FieldWord(sz);

    // Extra information if planet is in its ruling, exalting, etc, sign.

    if (c == 'R') {
      sprintf(sz, "This is a major part of %s's psyche!", szPerson2);
      FieldWord(sz);
    } else if (c == 'd') {
      sprintf(sz, "(This bit plays only a minor part in %s's psyche.)",
        szPerson2);
      FieldWord(sz);
    } else if (c == 'X') {
      sprintf(sz, "%s is affected harmoniously in this way.", szPerson1);
      FieldWord(sz);
    } else if (c == 'f') {
      sprintf(sz, "%s is affected discordantly in this way.", szPerson1);
      FieldWord(sz);
    }
    FieldWord(NULL);
  }
}


// Print an interpretation for a particular aspect in effect in a comparison
// relationship chart. This is called from the InterpretGridRelation() and the
// ChartAspectRelation() routines.

void InterpretAspectRelation(int x, int y)
{
  char sz[cchSzMax];
  int asp;

  asp = grid->n[y][x];
  if (!FInterpretAsp(asp) || !FInterpretObj(x) || !FInterpretObj(y))
    return;
  AnsiColor(kAspA[asp]);
  sprintf(sz, "%s %s %s: %s's",
    szObjDisp[x], SzAspect(asp), szObjDisp[y], szPerson1);
  FieldWord(sz); FieldWord(szMindPart[x]);
  sprintf(sz, szInteract[asp],
    szModify[Min(NAbs(grid->v[y][x])/(150*60), 2)][asp-1]);
  FieldWord(sz);
  sprintf(sz, "%s's %s.", szPerson2, szMindPart[y]); FieldWord(sz);
  if (szTherefore[asp][0]) {
    if (asp != aCon) {
      sprintf(sz, "%s.", szTherefore[asp]); FieldWord(sz);
    } else
      FieldWord("These parts affect each other prominently.");
  }
  FieldWord(NULL);
}


// Print the interpretation of each aspect in the relationship aspect grid, as
// specified with the -r0 -g -I switch combination.

void InterpretGridRelation(void)
{
  int i, j;

  for (i = 0; i <= is.nObj; i++) if (!ignore[i])
    for (j = 0; j <= is.nObj; j++) if (!ignore[j])
      InterpretAspectRelation(i, j);
  AnsiColor(kDefault);
}


// Print the interpretation of a midpoint in the relationship grid, as
// specified with the -r0 -m -I switch combination.

void InterpretMidpointRelation(int x, int y)
{
  char sz[cchSzMax];
  int n;

  if (!FInterpretObj(x) || !FInterpretObj(y))
    return;
  n = grid->n[y][x];
  AnsiColor(kSignA(n));
  sprintf(sz, "%s midpoint %s in %s: The merging of %s's",
    szObjDisp[x], szObjDisp[y], szSignName[n], szPerson1);
  FieldWord(sz); FieldWord(szMindPart[x]);
  sprintf(sz, "with %s's", szPerson2); FieldWord(sz);
  FieldWord(szMindPart[y]); FieldWord("is");
  if (grid->v[y][x]/3600 < 10)
    FieldWord("very");
  sprintf(sz, "%s, and", szDesc[n]); FieldWord(sz);
  sprintf(sz, "%s.", szDesire[n]); FieldWord(sz);
  if (cp1.dir[x]+cp2.dir[y] < 0.0 &&
    x != oNod && y != oNod && x != oSou && y != oSou) {
    FieldWord("Most often this manifests in "
      "an independent, backward, introverted manner.");
  }
  FieldWord(NULL);
}


CONST char *szAngle[cElem] = {"project", "feel within",
  "attract and receive", "be seen as"};

// Print an interpretation for a latitude crossing in effect in an astro-graph
// chart. This is called from the ChartAstroGraph() routine.

void InterpretAstroGraph(int obj1, int cusp1, int obj2, int cusp2)
{
  char sz[cchSzMax];
  int c1 = (cusp1 - oAsc) / 3, c2 = (cusp2 - oAsc) / 3;

  if (!FInterpretObj(obj1) || !FInterpretObj(obj2))
    return;
  FieldWord("Near this location"); 
  if (us.nRel >= rcNone) {
    FieldWord(szPerson0);
    FieldWord("can more easily");
    sprintf(sz, "%s their %s, and also %s their %s.",
      szAngle[c1], szMindPart[obj1], szAngle[c2], szMindPart[obj2]);
  } else {
    FieldWord(szPerson1);
    FieldWord("can more easily");
    sprintf(sz,
      "%s their %s, at the same time %s can more easily %s their %s.",
      szAngle[c1], szMindPart[obj1], szPerson2, szAngle[c2], szMindPart[obj2]);
  }
  FieldWord(sz);
  FieldWord(NULL);
}


/*
******************************************************************************
** Esoteric Interpretation Routines.
******************************************************************************
*/

#define cRayArea 5

CONST char *rgEsoRayArea[cRayArea] =
  {"Physical", "Astral", "Mental", "Personality", "Soul"};

CONST char *rgEsoMantra1[cSign+1] = {"",
  "Let form again be sought.",
  "Let struggle be undismayed.",
  "Let instability do its work.",
  "Let isolation be the rule and yet the crowd exists.",
  "Let other forms exist, I rule.",
  "Let matter reign.",
  "Let choice be made.",
  "Let Maya flourish and let deception rule.",
  "Let food be sought.",
  "Let ambition rule and let the door stand wide.",
  "Let desire in form be ruler.",
  "Go forth into matter."};
CONST char *rgEsoMantra2[cSign+1] = {"",
  "I come forth and from the plane of mind I rule.",
  "I see and when the eye is opened, all is light.",
  "I see my other self, and in the waning of that self, I grow and glow.",
  "I build a lighted house and therein dwell.",
  "I am That, and That am I.",
  "I am the mother and the child, I God, I matter am. "
    "Christ in you, the hope of glory.",
  "I choose the way which lies between the two great lines of force.",
  "Warrior I am, and from the battle I emerge triumphant!",
  "I see the goal, I reach that goal, and then I see another.",
  "Lost am I in light supernal, yet on that light I turn my back.",
  "Water of life am I, poured forth for thirsty men.",
  "I leave the Father's house, and turning back, I save."};
CONST char *rgEsoLight[cSign+1] = {"",
  "The Light of Life Itself.",
  "The penetrating Light of the Path.",
  "The Light of Interplay.",
  "The Light within the form.",
  "The Light of the Soul.",
  "The blended dual Light.",
  "The Light that moves to rest.",
  "The Light of Day.",
  "A beam of directed, focused Light.",
  "The Light of Initiation.",
  "The Light that shines on Earth, across the sea.",
  "The Light of the World."};
CONST char *rgEsoLabor[cSign+1] = {"",
  "Capture of the Man-eating Mares",
  "Capture of the Cretan Bull",
  "Gathering the Golden Apples of the Hesperides",
  "Capture of the Doe or Hind",
  "Slaying of the Nemean Lion",
  "Seizing the Girdle of Hippolyte",
  "Capture of the Erymanthian Boar",
  "Destroying the Lernaean Hydra",
  "Killing the Stymphalian Birds",
  "Slaying of Cerberus, Guardian of Hades",
  "Cleansing the Augean Stables",
  "Capture of the Red Cattle of Geryon"};
CONST char *rgEsoLesson[cSign+1] = {"",
  "Mastering the right use of the mind and life force with unselfish intent, "
    "and using the will to gain control and eliminate wrong thought, speech,"
    "and actions",
  "Learning to intelligently express will impulsed by love, refining the "
    "will to good, and achieving harmony by fostering right human "
    "relationships",
  "Understanding the dual aspect of the mind as it mediates between the "
    "higher and lower expressions, producing changes needed for the evolving "
    "consciousness",
  "To develop intuition and a sensitive response to the environing conditions "
    "and circumstances, and psychic development of both the lower and higher "
    "senses",
  "To unite the mind and heart in the will-to-illumine, and developing the "
    "ability to consciously express spiritual will, purpose and intent",
  "Learning to nurture intelligence, wisdom, and pure reason, and to nurture "
    "the soul, using matter (the physical body) as the guardian of the hidden "
    "Christ deep within",
  "To develop the right use of the mind to discriminate between the pairs of "
    "opposites and find balance on the narrow razor-edged path between the "
    "two",
  "Learning to attack inner demons and cleanse the personality, and "
    "transmuting desire into aspiration and establishing right relations with "
    "the soul",
  "Refocusing and reorienting the personality to a higher goal through right "
    "thought, right speech and right actions, achieving one-pointed direction "
    "of the soul",
  "Expanding spiritual will, and identifying with the will of God and the "
    "Divine Plan and using it in humility and impersonal service to all of "
    "humanity",
  "Awakening to group interest and individual responsibilities to group life, "
    "and beginning to live a life of unselfish service to humanity",
  "Learning to overcome the human and express the divine through meditation, "
    "mediation, and the universal love of right relationships"};
CONST char *rgEsoHou1[cSign+1] = {"",
  "Self, personality, physical body",
  "Substance, resources, values",
  "Integration, self-expression",
  "Home, domestic relationships",
  "Creativity, pleasure, children",
  "Service, health",
  "Relationships",
  "Endings, regeneration",
  "Spiritual integration, exploration",
  "Career, goals",
  "Friendships, teamwork",
  "Dreams, fears, fantasies"};
CONST char *rgEsoHou2[cSign+1] = {"",
  "fostering soul expression and control of the personality",
  "fostering spiritual values and higher uses of resources",
  "fostering soul expression on the higher mental planes",
  "revealing past karmic ties to help determine the soul's purpose",
  "fostering soul's creativity and development of spiritual will",
  "fostering integration of personality and soul, and developing spiritual "
    "service",
  "establishing viable spiritual relationships with the soul and soul group",
  "fostering transmutation of physical desires into spiritual purpose",
  "fostering spiritual enlightenment, and the Ancient Wisdom",
  "fostering development of spiritual responsibility, and preparation for "
    "spiritual initiation",
  "of group service opportunities for the good of all Humanity",
  "for developing the ability to transmute fears and dispel illusion"};
CONST char *rgEsoObj[oNorm1] = {
  "Spiritual duty and potential, service to humanity",
  "Personality path, creative gifts, key to integrating personality and soul",
  "Repositiy for the past, karmic ties to form, prison of the soul",
  "Illumination of lower mind, tapping intuition, mediator between "
    "personality and soul",
  "Emerging love principle through the mind, right human relations, "
    "resolution of polarities",
  "Motivation, spiritual warrior, establishing relations between opposites",
  "Beneficient method of growth, seeking greater truths",
  "Opportunity through crisis, destruction of crystallized forms through "
    "wise choices",
  "Mediator of the soul, collective interrelatedness, intuition to "
    "inspiration, leading the soul to the final path",
  "Compassion and unity, unconditional love, mysticism, self-sacrifice",
  "Transformation, deep inner truths, personality surrendering to the soul",
  "Emotional or physical vulnerability, compassionate healer, spiritual "
    "lessons, the bridge between personality and soul",
  "Seasonal cycles, environmental issues, right distribution of wheat, food, "
    "grains",
  "Wisdom of right human relations, fighting for righteous causes",
  "Protector and counselor of state, divine marriage between personality and "
    "soul",
  "Right ordering of domesticity and the family, right human relations "
    "within the family",
  "Using new experiences to achieve spiritual goals",
  "Using past experience to achieve spiritual goals",
  "",
  "Cooperation of the soul and personality to overcome illusion",
  "Completion of karmic events, more than likely from past lives",
  "",
  "Purpose of soul ray, point where your soul left off last life, new "
    "qualities to develop",
  "", "", "", "", "", "", "", "", "", "", "",
  "Spiritual will, forging change and fusing new forms",
  "", "", "", "", "", "", "", "",
  "Health service for humanitarian reasons",
  "",
  "Inner conviction, striving to reach spiritual goal",
  "Birth of the Christ child within",
  "Path to spiritual renewal, Christ consciousness",
  "", "",
  "Respecting all life through right actions",
  ""};

// Print an Esoteric Astrology interpretation of each planet in sign (and
// house), along with Ray chart clues based on astrological influences, as
// specified with the -7 -I switches.

void InterpretEsoteric()
{
  char sz[cchSzMax*2], szName[cchSzDef], *pch;
  int i, j, sig, hou, ray;
  int rgcRay[cRay+1], rgcObjRay[oVul+1][cRay+1], rgnSort[cRay+1],
    rgcTot[cRayArea], *rgRules, *pcRay = rgcRay, bod, nObj, nDec, nLin, k, l;
  flag fIgnore7Sav[rrMax];

  for (i = 0; i < rrMax; i++) {
    fIgnore7Sav[i] = ignore7[i];
    ignore7[i] = fFalse;
  }

  // Determine Ray chart
  PrintSz("Ray chart clues based on astrological influences only:\n");
  EnsureRay();
  for (bod = cRayArea-1; bod >= 0; bod--) {
    ClearB((pbyte)rgcRay, sizeof(rgcRay));
    ClearB((pbyte)rgcObjRay, sizeof(rgcObjRay));
    for (i = 0; i <= oVul; i++) if (!ignore[i]) {
      switch (bod) {
      case 0: nObj = (i == oMoo ? 2 : (i == oMar || i == oEar ? 1 : 0)); break;
      case 1: nObj = (i == oNep ? 2 : (i == oJup              ? 1 : 0)); break;
      case 2: nObj = (i == oVen ? 2 : (i == oMer              ? 1 : 0)); break;
      case 3: nObj = (i == oSun ? 2 : (i == oMar || i == oSat ? 1 : 0)); break;
      case 4: nObj = (i == oAsc ? 2 : (i == oVul || i == oUra ? 1 : 0)); break;
      }
      if (nObj <= 0)
        continue;
      for (l = (us.fListDecan ? 3 : 0); l >= 0; l--) {
        // l: 0=sign, 1=sign's decanate, 2=decanate's sign, 3=dec's decanate
        nDec = (l <= 0 ? 9 : (l >= 3 ? 1 : 3));
        for (k = 0; k < 6; k++) {
          // k: 0=sign, 1=exo ruler of sign, 2=eso ruler of sign
          // k: 3=(veiled) sign, 4=exo ruler of v.sign, 5=eso ruler of v.sign
          nLin = 1;
          if (k <= 0 || k == 3)
            nLin *= 2;
          if ((bod == 4 && k >= 3) || (bod < 4 && k < 3))
            nLin *= 2;            
          if (k <= 0 || k == 3)
            j = i;
          else {
            // Look at Ray of dispositor planet
            rgRules = (k < 3 ? (k == 1 ? rules : rgSignEso1) :
              (k == 4 ? rules2 : rgSignEso2));
            sig = SFromZ(!FOdd(l) ? planet[i] : Decan(planet[i]));
            j = rgRules[sig];
            if (j < 0)
              continue;
            ray = rgObjRay[j];
            if (ray > 0)
              rgcObjRay[i][ray] += nObj * nLin * nDec;
          }
          // Look at Rays of dispositor planet's sign
          if (ignore[j])
            continue;
          sig = SFromZ(l < 3 ? planet[j] : Decan(planet[j]));
          for (ray = 1; ray <= cRay; ray++) {
            j = rgSignRay2[sig][ray];
            if (j > 0)
              rgcObjRay[i][ray] += nObj * nLin * nDec;
          }
        }
      }
      for (ray = 1; ray <= cRay; ray++)
        rgcRay[ray] += rgcObjRay[i][ray];
    }
    // Some Rays more probable for certain vehicles, so get additional factor.
    switch (bod) {
    case 0: pcRay[1] *= 2; pcRay[3] *= 5; pcRay[7] *= 6; break;
    case 1: pcRay[1] *= 4; pcRay[2] *= 5; pcRay[6] *= 6; pcRay[3] /= 2; break;
    case 2: pcRay[1] *= 3; pcRay[3] *= 3; pcRay[4] *= 3; pcRay[5] *= 6; break;
    }
    // Sort Rays by most points to least points
    for (ray = 1; ray <= cRay; ray++)
      rgnSort[ray] = ray;
    for (ray = 2; ray <= cRay; ray++) {
      j = ray-1;
      while (j >= 1 &&
        rgcRay[rgnSort[j]] < rgcRay[rgnSort[j+1]]) {
        SwapN(rgnSort[j], rgnSort[j+1]);
        j--;
      }
    }
    rgcTot[bod] = rgnSort[1];
    // Print all seven Rays and their points for this vehicle.
    AnsiColor(kRayA[rgnSort[1]]);
    sprintf(sz, "%-4.4s Ray:", rgEsoRayArea[bod]); PrintSz(sz);
    k = 0;
    for (ray = 1; ray <= cRay; ray++)
      k += rgcRay[ray];
    if (k == 0)
      k = 1;
    for (ray = 1; ray <= cRay; ray++) {
      sprintf(sz, " R%d (%2d%%)%s", rgnSort[ray], rgcRay[rgnSort[ray]] *
        100 / k, ray < cRay ? "," : ""); PrintSz(sz);
    }
    PrintL();
  }
  PrintL();

#ifdef EXPRESS
  // Send top Ray chart Rays to AstroExpression if one set.
  if (!us.fExpOff && FSzSet(us.szExpEso)) {
    for (bod = 0; bod < cRayArea; bod++)
      ExpSetN(iLetterZ - cRayArea + 1 + bod, rgcTot[bod]);
    ParseExpression(us.szExpEso);
  }
#endif

  // Interpret each planet in sign and house placement
  for (i = 0; i <= is.nObj; i++) {
    if (ignore[i])
      continue;
    AnsiColor(kObjA[i]);
    sprintf(szName, "%s%s%s", i == oFor && szObjDisp[i] == szObjName[i] ?
      "Part of " : "", szObjDisp[i],
      i == oPal && szObjDisp[i] == szObjName[i] ? " Athena" : "");
    sig = SFromZ(planet[i]);
    hou = inhouse[i];
    sprintf(sz, "%s in %s%s", szName, szSignName[sig],
      us.fInfluenceSign ? "" : ":\n");
    FieldWord(sz);
    if (us.fInfluenceSign) {
      sprintf(sz, "and %d%s house:\n", hou, szSuffix[hou]);
      FieldWord(sz);
    }
    // Planet
    if (i <= oNorm && *rgEsoObj[i]) {
      sprintf(sz, "%s esoteric meaning: %s.\n", szName, rgEsoObj[i]);
      FieldWord(sz);
    }
    ray = rgObjRay[i];
    if (ray > 0) {
      sprintf(sz, "%s is Ray %d (%s), the \"Will to %s\".\n",
        szName, ray, szRayName[ray], szRayWill[ray]);
      AnsiColor(kRayA[ray]); FieldWord(sz);
    }
    // Sign
    AnsiColor(kSignA(sig));
    sprintf(sz, "%s esoteric lesson: %s.\n", szSignName[sig],
      rgEsoLesson[sig]);
    FieldWord(sz);
    sprintf(sz, "%s mundane mantram: \"%s\"\n",
      szSignName[sig], rgEsoMantra1[sig]);
    FieldWord(sz);
    sprintf(sz, "%s esoteric mantram: \"%s\"\n",
      szSignName[sig], rgEsoMantra2[sig]);
    FieldWord(sz);
    sprintf(sz, "%s Light is: \"%s\"\n", szSignName[sig], rgEsoLight[sig]);
    FieldWord(sz);
    sprintf(sz, "%s Labor of Hercules: %s.\n",
      szSignName[sig], rgEsoLabor[sig]);
    FieldWord(sz);
    j = rules[sig];
    if (j >= 0) {
      sprintf(sz, "%s is exoterically ruled by %s%s%s", szSignName[sig],
        j <= oMoo ? "the " : "", szObjDisp[j], rules2[sig] >= 0 ? "" : ".");
      FieldWord(sz);
      if (rules2[sig] >= 0) {
        sprintf(sz, "and %s.", szObjDisp[rules2[sig]]);
        FieldWord(sz);
      }
    }
    j = rgSignEso1[sig];
    if (j >= 0) {
      sprintf(sz, "%s is esoterically ruled by %s%s%s",
        szSignName[sig], j <= oMoo ? "the " : "", szObjDisp[j],
        rgSignEso2[sig] >= 0 ? "" : ".");
      FieldWord(sz);
      if (rgSignEso2[sig] >= 0) {
        sprintf(sz, "veiling %s.", szObjDisp[rgSignEso2[sig]]);
        FieldWord(sz);
      }
    }
    if (rules[sig] > 0 || rgSignEso1[sig] > 0)
      FieldWord(NULL);
    for (j = 1; j <= cRay; j++) if (rgSignRay2[sig][j] > 0) {
      sprintf(sz, "%s is Ray %d (%s), the \"Will to %s\".\n",
        szSignName[sig], j, szRayName[j], szRayWill[j]);
      AnsiColor(kRayA[j]); FieldWord(sz);
    }
    // House
    if (us.fInfluenceSign) {
      AnsiColor(kSignA(hou));
      sprintf(sz, "%d%s house characteristics: %s. It's an environment %s.\n",
        hou, szSuffix[hou], rgEsoHou1[hou], rgEsoHou2[hou]);
      FieldWord(sz);
      for (j = 1; j <= cRay; j++) if (rgSignRay2[hou][j] > 0) {
        sprintf(sz, "%d%s house (%s) is Ray %d (%s), the \"Will to %s\".\n",
          hou, szSuffix[hou], szSignName[hou], j, szRayName[j], szRayWill[j]);
        AnsiColor(kRayA[j]); FieldWord(sz);
      }
    }
    // Rulerships
    pch = Dignify(i, sig)+1;
    if (pch[rrRay] == 'Y') {
      sprintf(sz, "Ray rulership! Both %s and %s are Ray %d.\n",
        szName, szSignName[sig], ray);
      AnsiColor(kRayA[ray]); FieldWord(sz);
    } else if (pch[rrRay] == 'z') {
      sprintf(sz, "Ray debilitation! Both %s and %s (opposite %s) are Ray "
        "%d.\n", szName, szSignName[Mod12(sig+6)], szSignName[sig], ray);
      AnsiColor(kRayA[ray]); FieldWord(sz);
    }
    AnsiColor(kObjA[i]);
    if (pch[rrStd] == 'R') {
      sprintf(sz, "Standard rulership! %s", szName);
      FieldWord(sz);
      if (rules2[sig] >= 0) {
        sprintf(sz, "(with %s)",
          szObjDisp[(i == rules2[sig] ? rules : rules2)[sig]]);
        FieldWord(sz);
      }
      sprintf(sz, "exoterically rules %s.\n", szSignName[sig]);
      FieldWord(sz);
    } else if (pch[rrStd] == 'd') {
      sprintf(sz, "Standard debilitation! %s is exoterically debilitated in "
        "%s.\n", szName, szSignName[sig]);
      FieldWord(sz);
    }
    if (pch[rrEso] == 'S') {
      sprintf(sz, "Esoteric rulership! %s", szName);
      FieldWord(sz);
      if (rgSignEso2[sig] >= 0) {
        sprintf(sz, "(veiling %s)", szObjDisp[rgSignEso2[sig]]);
        FieldWord(sz);
      }
      sprintf(sz, "esoterically rules %s.\n", szSignName[sig]);
      FieldWord(sz);
    } else if (pch[rrEso] == 's') {
      sprintf(sz, "Esoteric debilitation! %s is esoterically debilitated in "
        "%s.\n", szName, szSignName[sig]);
      FieldWord(sz);
    }
    if (pch[rrHie] == 'H') {
      sprintf(sz, "Hierarchical rulership! %s", szName);
      FieldWord(sz);
      if (rgSignHie2[sig] >= 0) {
        sprintf(sz, "(veiling %s)", szObjDisp[rgSignHie2[sig]]);
        FieldWord(sz);
      }
      sprintf(sz, "Hierarchically rules %s.\n", szSignName[sig]);
      FieldWord(sz);
    } else if (pch[rrHie] == 'h') {
      sprintf(sz, "Hierarchical debilitation! %s is Hierarchically "
        "debilitated in %s.\n", szName, szSignName[sig]);
      FieldWord(sz);
    }
    if (us.fInfluenceSign) {
      pch = Dignify(i, hou)+1;
      if (pch[rrRay] == 'Y') {
        sprintf(sz, "House Ray rulership! Both %s and %d%s house (%s) are "
          "Ray %d.\n", szName, hou, szSuffix[hou], szSignName[hou], ray);
        AnsiColor(kRayA[ray]); FieldWord(sz);
      } else if (pch[rrRay] == 'z') {
        sprintf(sz, "House Ray debilitation! Both %s and %d%s house (%s, "
          "opposite %s) are Ray %d.\n", szName, hou, szSuffix[hou],
          szSignName[Mod12(hou+6)], szSignName[hou], ray);
        AnsiColor(kRayA[ray]); FieldWord(sz);
      }
    }
    FieldWord(NULL);
  }

  for (i = 0; i < rrMax; i++)
    ignore7[i] = fIgnore7Sav[i];
}
#endif // INTERPRET


/*
******************************************************************************
** Chart Influence Routines.
******************************************************************************
*/

CONST char *szElemHouse[cElem] = {"Dharma", "Artha", "Kama", "Moksha"};
CONST char *szModeHouse[3] = {"Angular", "Succedent", "Cadent"};

// This is a subprocedure of ChartInfluence(). Based on the values in the
// array parameter 'value', store numbers in array 'rank' reflecting the
// relative order, e.g. value[x] 2nd greatest array value -> rank[x] = 2.

void SortRank(real *value, int *rank, int size, flag fObj)
{
  int h, i, j, k;
  real r;

  if (!fObj)
    value[0] = -1.0;
  for (i = 0; i <= size; i++)
    rank[i] = -1;
  for (h = 0, i = 0; h <= size; h++) {
    if (fObj && FIgnore(h))
      continue;
    i++;
    k = 0;
    r = -1.0;
    for (j = 0; j <= size; j++) {
      if (fObj && FIgnore(j))
        continue;
      if (value[j] > r && rank[j] < 0) {
        k = j;
        r = value[k];
      }
    }

    // 'k' is the current position of the 'i'th place planet.
    rank[k] = i;
  }
}


// Determine the influence of each planet's position and aspects. Called from
// the ChartInfluence() routine for the -j chart, and the ChartEsoteric()
// routine for the -7 chart which also makes use of planet influences.

void ComputeInfluence(real power1[objMax], real power2[objMax])
{
  int i, j, k, l;
  real x;
  char *c;

  for (i = 0; i <= is.nObj; i++)
    power1[i] = power2[i] = 0.0;

  // First, for each object, find its power based on its placement alone.

  for (i = 0; i <= is.nObj; i++) if (!FIgnore(i)) {
    j = SFromZ(planet[i]);
    power1[i] += RObjInf(i);               // Influence of planet itself.
    power1[i] += rHouseInf[inhouse[i]];    // Influence of house it's in.
    x = 0.0;
    c = Dignify(i, j);
    if (c[rrStd+1] == 'R') x += rObjInf[oNorm1+1];  // Planets in signs they
    if (c[rrExa+1] == 'X') x += rObjInf[oNorm1+2];  // rule or are exalted
    if (c[rrEso+1] == 'S') x += rObjInf[oNorm1+3];  // in have influence.
    if (c[rrHie+1] == 'H') x += rObjInf[oNorm1+4];
    if (c[rrRay+1] == 'Y') x += rObjInf[oNorm1+5];
    c = Dignify(i, inhouse[i]);
    if (c[rrStd+1] == 'R') x += rHouseInf[cSign+1];  // Planets in houses
    if (c[rrExa+1] == 'X') x += rHouseInf[cSign+2];  // aligned with sign
    if (c[rrEso+1] == 'S') x += rHouseInf[cSign+3];  // ruled or exalted
    if (c[rrHie+1] == 'H') x += rHouseInf[cSign+4];  // in have influence.
    if (c[rrRay+1] == 'Y') x += rHouseInf[cSign+5];
    power1[i] += x;
    x = RObjInf(i)/2.0;
    if (!ignore7[rrStd]) {
      // Planet ruling sign and house current planet is in, gets influence.
      k = rules[j];           if (k > 0 && i != k) power1[k] += x;
      k = rules2[j];          if (k > 0 && i != k) power1[k] += x;
      k = rules[inhouse[i]];  if (k > 0 && i != k) power1[k] += x;
      k = rules2[inhouse[i]]; if (k > 0 && i != k) power1[k] += x;
    }
    if (!ignore7[rrEso]) {
      k = rgSignEso1[j];          if (k > 0 && i != k) power1[k] += x;
      k = rgSignEso2[j];          if (k > 0 && i != k) power1[k] += x;
      k = rgSignEso1[inhouse[i]]; if (k > 0 && i != k) power1[k] += x;
      k = rgSignEso2[inhouse[i]]; if (k > 0 && i != k) power1[k] += x;
    }
    if (!ignore7[rrHie]) {
      k = rgSignHie1[j];          if (k > 0 && i != k) power1[k] += x;
      k = rgSignHie2[j];          if (k > 0 && i != k) power1[k] += x;
      k = rgSignHie1[inhouse[i]]; if (k > 0 && i != k) power1[k] += x;
      k = rgSignHie2[inhouse[i]]; if (k > 0 && i != k) power1[k] += x;
    }
  }
  for (i = 1; i <= cSign; i++) {         // Various planets get influence
    j = SFromZ(chouse[i]);               // if house cusps fall in signs
    power1[rules[j]] += rHouseInf[i];    // they rule.
    if (!ignore7[rrEso]) {
      k = rgSignEso1[j]; if (k) power1[k] += rHouseInf[i];
      k = rgSignEso2[j]; if (k) power1[k] += rHouseInf[i];
    }
    if (!ignore7[rrHie]) {
      k = rgSignHie1[j]; if (k) power1[k] += rHouseInf[i];
      k = rgSignHie2[j]; if (k) power1[k] += rHouseInf[i];
    }
  }

  // Second, for each object, find its power based on aspects it makes.

  if (!FCreateGrid(fFalse))
    return;
  for (j = 0; j <= is.nObj; j++) if (!FIgnore(j))
    for (i = 0; i <= is.nObj; i++) if (!FIgnore(i) && i != j) {
      k = grid->n[Min(i, j)][Max(i, j)];
      if (k) {
        l = grid->v[Min(i, j)][Max(i, j)];
        power2[j] += rAspInf[k]*rObjInf[i]*
          (1.0-RAbs((real)l)/3600.0/GetOrb(i, j, k));
      }
    }

#ifdef EXPRESS
  // Adjust object powers if AstroExpression set to do so.

  if (!us.fExpOff && FSzSet(us.szExpInf))
    for (i = 0; i <= is.nObj; i++) if (!FIgnore(i)) {
      ExpSetN(iLetterX, i);
      ExpSetR(iLetterY, power1[i]);
      ExpSetR(iLetterZ, power2[i]);
      ParseExpression(us.szExpInf);
      power1[i] = RExpGet(iLetterY);
      power2[i] = RExpGet(iLetterZ);
    }
#endif
}


// Print out a list of power values and relative rankings, based on the
// placements of the planets, and their aspects in the aspect grid, as
// specified with the -j "find influences" switch.

void ChartInfluence(void)
{
  real power[objMax], power1[objMax], power2[objMax],
    total, total1, total2;
  int rank[objMax], rank1[objMax], rank2[objMax], i, j;
  char sz[cchSzDef];

  ComputeInfluence(power1, power2);

  // Calculate total power of each planet.

  total = total1 = total2 = 0.0;
  for (i = 0; i <= is.nObj; i++) if (!FIgnore(i)) {
    power[i] = power1[i]+power2[i]; total1 += power1[i]; total2 += power2[i];
  }
  total = total1+total2;

  // Finally, determine ranks of the arrays, then print everything out.

  SortRank(power1, rank1, is.nObj, fTrue);
  SortRank(power2, rank2, is.nObj, fTrue);
  SortRank(power,  rank,  is.nObj, fTrue);
  PrintSz("  Planet:    Position      Aspects    Total Rank  Percent\n");
  for (j = 0; j <= is.nObj; j++) {
    i = rgobjList[j];
    if (FIgnore(i))
      continue;
    AnsiColor(kObjA[i]);
    sprintf(sz, "%8.8s: ", szObjDisp[i]); PrintSz(sz);
    sprintf(sz, "%6.1f (%2d) +%6.1f (%2d) =%7.1f (%2d) /%6.1f%%\n",
      power1[i], rank1[i], power2[i], rank2[i], power[i], rank[i],
      total > 0.0 ? power[i]/total*100.0 : 0.0); PrintSz(sz);
  }
  AnsiColor(kDefault);
  sprintf(sz, "   Total: %6.1f      +%6.1f      =%7.1f      / 100.0%%\n",
    total1, total2, total); PrintSz(sz);

  // Now, print out a list of power values and relative rankings, based on the
  // power of each sign of the zodiac, as indicated by the placement of the
  // planets above, in the chart, as specified with the -j0 switch.

  if (!us.fInfluenceSign)
    return;
  for (i = 1; i <= cSign; i++)
    power1[i] = power2[i] = 0.0;

  // For each sign, determine its power based on the power of the objects.

  for (i = 0; i <= is.nObj; i++) if (!FIgnore(i)) {
    power1[SFromZ(planet[i])] += power[i] / 2.0;
    if (us.fListDecan)
      power1[SFromZ(Decan(planet[i]))] += power[i] / 6.0;
    if (us.fSectorApprox)
      power1[inhouse[i]]        += power[i] / 4.0;
    if (!ignore7[rrStd]) {
      power1[ruler1[i]]         += power[i] / 3.0;
      if (ruler2[i])
        power1[ruler2[i]]       += power[i] / 3.0;
    }
    if (!ignore7[rrEso]) {
      power1[rgSignEso1[i]]     += power[i] / 3.0;
      if (rgSignEso2[i])
        power1[rgSignEso2[i]]   += power[i] / 3.0;
    }
    if (!ignore7[rrEso]) {
      power1[rgSignHie1[i]]     += power[i] / 3.0;
      if (rgSignHie2[i])
        power1[rgSignHie2[i]]   += power[i] / 3.0;
    }
  }

#ifdef EXPRESS
  // Adjust sign powers if AstroExpression set to do so.

  if (!us.fExpOff && FSzSet(us.szExpInf0))
    for (i = 1; i <= cSign; i++) {
      ExpSetN(iLetterX, 0);
      ExpSetN(iLetterY, i);
      ExpSetR(iLetterZ, power1[i]);
      ParseExpression(us.szExpInf0);
      power1[i] = RExpGet(iLetterZ);
    }
#endif

  total1 = 0.0;
  for (i = 1; i <= cSign; i++)
    total1 += power1[i];
  if (total1 > 0.0)
    for (i = 1; i <= cSign; i++)
      power1[i] *= total/total1;
  total1 = total;

  // Again, determine ranks in the array, and print everything out.

  SortRank(power1, rank1, cSign, fFalse);
  PrintSz(
    "\n       Sign:  Power Rank  Percent  -   Element  Power  Percent\n");
  for (i = 1; i <= cSign; i++) {
    AnsiColor(kSignA(i));
    sprintf(sz, "%11.11s: ", szSignName[i]); PrintSz(sz);
    sprintf(sz, "%6.1f (%2d) /%6.1f%%", power1[i],
      rank1[i], total1 > 0.0 ? power1[i]/total1*100.0 : 0.0); PrintSz(sz);
    if (i <= 4) {
      sprintf(sz, "  -%9.9s:", szElem[i-1]); PrintSz(sz);
      total2 = 0.0;
      for (j = 1; j < cSign; j += 4)
        total2 += power1[i-1+j];
      sprintf(sz, "%7.1f /%6.1f%%", total2,
        total1 > 0.0 ? total2/total1*100.0 : 0.0); PrintSz(sz);
    } else if (i == 6) {
      AnsiColor(kDefault);
      PrintSz("  -      Mode  Power  Percent");
    } else if (i >= 7 && i <= 9) {
      AnsiColor(kModeA(i-7));
      sprintf(sz, "  -%9.9s:", szMode[i-7]); PrintSz(sz);
      total2 = 0.0;
      for (j = 1; j < cSign; j += 3)
        total2 += power1[i-7+j];
      sprintf(sz, "%7.1f /%6.1f%%", total2,
        total1 > 0.0 ? total2/total1*100.0 : 0.0); PrintSz(sz);
    }
    PrintL();
  }
  AnsiColor(kDefault);
  sprintf(sz, "      Total:%7.1f      / 100.0%%\n", total1); PrintSz(sz);

  // For each house, determine its power based on the power of the objects.

  if (us.fSectorApprox)
    return;
  for (i = 1; i <= cSign; i++)
    power2[i] += rObjInf[oAsc + i - 1];
  for (i = 0; i <= is.nObj; i++) if (!FIgnore(i))
    power2[inhouse[i]] += power[i];

#ifdef EXPRESS
  // Adjust house powers if AstroExpression set to do so.

  if (!us.fExpOff && FSzSet(us.szExpInf0))
    for (i = 1; i <= cSign; i++) {
      ExpSetN(iLetterX, 1);
      ExpSetN(iLetterY, i);
      ExpSetR(iLetterZ, power2[i]);
      ParseExpression(us.szExpInf0);
      power2[i] = RExpGet(iLetterZ);
    }
#endif

  total2 = 0.0;
  for (i = 1; i <= cSign; i++)
    total2 += power2[i];
  if (total2 > 0.0 && total > 0.0) {
    for (i = 1; i <= cSign; i++)
      power2[i] *= total/total2;
    total2 = total;
  }

  // Again, determine ranks in the array, and print everything out.

  SortRank(power2, rank1, cSign, fFalse);
  PrintSz(
    "\nHouse:  Power Rank  Percent  -    Element  Power  Percent\n");
  for (i = 1; i <= cSign; i++) {
    AnsiColor(kSignA(i));
    sprintf(sz, "%3d%s: ", i, szSuffix[i]); PrintSz(sz);
    sprintf(sz, "%6.1f (%2d) /%6.1f%%", power2[i],
      rank1[i], total2 > 0.0 ? power2[i]/total2*100.0 : 0.0); PrintSz(sz);
    if (i <= 4) {
      sprintf(sz, "  - %9.9s:", szElemHouse[i-1]); PrintSz(sz);
      total1 = 0.0;
      for (j = 1; j < cSign; j += 4)
        total1 += power2[i-1+j];
      sprintf(sz, "%7.1f /%6.1f%%", total1,
        total1 > 0.0 ? total1/total2*100.0 : 0.0); PrintSz(sz);
    } else if (i == 6) {
      AnsiColor(kDefault);
      PrintSz("  -       Mode  Power  Percent");
    } else if (i >= 7 && i <= 9) {
      AnsiColor(kModeA(i-7));
      sprintf(sz, "  - %9.9s:", szModeHouse[i-7]); PrintSz(sz);
      total1 = 0.0;
      for (j = 1; j < cSign; j += 3)
        total1 += power2[i-7+j];
      sprintf(sz, "%7.1f /%6.1f%%", total1,
        total1 > 0.0 ? total1/total2*100.0 : 0.0); PrintSz(sz);
    }
    PrintL();
  }
  AnsiColor(kDefault);
  sprintf(sz, "Total:%7.1f      / 100.0%%\n", total2); PrintSz(sz);
}

/* intrpret.cpp */
