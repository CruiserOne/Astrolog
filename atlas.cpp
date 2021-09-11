/*
** Astrolog (Version 7.30) File: atlas.cpp
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


#ifdef ATLAS
/*
******************************************************************************
** Atlas Tables.
******************************************************************************
*/

typedef struct _CountryOrRegionName {
  char szAbb[3];  // Two letter abbreviation of location
  char *szNam;    // Full name of location
} CN;

// List of world country/regions
CONST CN rgcnew[icnewMax] = {
{"AD", "Andorra"},
{"AE", "United Arab Emirates"},
{"AF", "Afghanistan"},
{"AG", "Antigua and Barbuda"},
{"AI", "Anguilla"},
{"AL", "Albania"},
{"AM", "Armenia"},
{"AN", "Netherlands Antilles"},
{"AO", "Angola"},
{"AQ", "Antarctica"},
{"AR", "Argentina"},
{"AS", "American Samoa"},
{"AT", "Austria"},
{"AU", "Australia"},
{"AW", "Aruba"},
{"AX", "Aland Islands"},
{"AZ", "Azerbaijan"},
{"BA", "Bosnia and Herzegovina"},
{"BB", "Barbados"},
{"BD", "Bangladesh"},
{"BE", "Belgium"},
{"BF", "Burkina Faso"},
{"BG", "Bulgaria"},
{"BH", "Bahrain"},
{"BI", "Burundi"},
{"BJ", "Benin"},
{"BL", "Saint Barthelemy"},
{"BM", "Bermuda"},
{"BN", "Brunei"},
{"BO", "Bolivia"},
{"BQ", "Bonaire, Saint Eustatius and Saba "},
{"BR", "Brazil"},
{"BS", "Bahamas"},
{"BT", "Bhutan"},
{"BV", "Bouvet Island"},
{"BW", "Botswana"},
{"BY", "Belarus"},
{"BZ", "Belize"},
{"CA", "Canada"},
{"CC", "Cocos Islands"},
{"CD", "Democratic Republic of the Congo"},
{"CF", "Central African Republic"},
{"CG", "Republic of the Congo"},
{"CH", "Switzerland"},
{"CI", "Ivory Coast"},
{"CK", "Cook Islands"},
{"CL", "Chile"},
{"CM", "Cameroon"},
{"CN", "China"},
{"CO", "Colombia"},
{"CR", "Costa Rica"},
{"CS", "Serbia and Montenegro"},
{"CU", "Cuba"},
{"CV", "Cabo Verde"},
{"CW", "Curacao"},
{"CX", "Christmas Island"},
{"CY", "Cyprus"},
{"CZ", "Czechia"},
{"DE", "Germany"},
{"DJ", "Djibouti"},
{"DK", "Denmark"},
{"DM", "Dominica"},
{"DO", "Dominican Republic"},
{"DZ", "Algeria"},
{"EC", "Ecuador"},
{"EE", "Estonia"},
{"EG", "Egypt"},
{"EH", "Western Sahara"},
{"ER", "Eritrea"},
{"ES", "Spain"},
{"ET", "Ethiopia"},
{"FI", "Finland"},
{"FJ", "Fiji"},
{"FK", "Falkland Islands"},
{"FM", "Micronesia"},
{"FO", "Faroe Islands"},
{"FR", "France"},
{"GA", "Gabon"},
{"GB", "United Kingdom"},
{"GD", "Grenada"},
{"GE", "Georgia"},
{"GF", "French Guiana"},
{"GG", "Guernsey"},
{"GH", "Ghana"},
{"GI", "Gibraltar"},
{"GL", "Greenland"},
{"GM", "Gambia"},
{"GN", "Guinea"},
{"GP", "Guadeloupe"},
{"GQ", "Equatorial Guinea"},
{"GR", "Greece"},
{"GS", "South Georgia and the South Sandwich Islands"},
{"GT", "Guatemala"},
{"GU", "Guam"},
{"GW", "Guinea-Bissau"},
{"GY", "Guyana"},
{"HK", "Hong Kong"},
{"HM", "Heard Island and McDonald Islands"},
{"HN", "Honduras"},
{"HR", "Croatia"},
{"HT", "Haiti"},
{"HU", "Hungary"},
{"ID", "Indonesia"},
{"IE", "Ireland"},
{"IL", "Israel"},
{"IM", "Isle of Man"},
{"IN", "India"},
{"IO", "British Indian Ocean Territory"},
{"IQ", "Iraq"},
{"IR", "Iran"},
{"IS", "Iceland"},
{"IT", "Italy"},
{"JE", "Jersey"},
{"JM", "Jamaica"},
{"JO", "Jordan"},
{"JP", "Japan"},
{"KE", "Kenya"},
{"KG", "Kyrgyzstan"},
{"KH", "Cambodia"},
{"KI", "Kiribati"},
{"KM", "Comoros"},
{"KN", "Saint Kitts and Nevis"},
{"KP", "North Korea"},
{"KR", "South Korea"},
{"KW", "Kuwait"},
{"KY", "Cayman Islands"},
{"KZ", "Kazakhstan"},
{"LA", "Laos"},
{"LB", "Lebanon"},
{"LC", "Saint Lucia"},
{"LI", "Liechtenstein"},
{"LK", "Sri Lanka"},
{"LR", "Liberia"},
{"LS", "Lesotho"},
{"LT", "Lithuania"},
{"LU", "Luxembourg"},
{"LV", "Latvia"},
{"LY", "Libya"},
{"MA", "Morocco"},
{"MC", "Monaco"},
{"MD", "Moldova"},
{"ME", "Montenegro"},
{"MF", "Saint Martin"},
{"MG", "Madagascar"},
{"MH", "Marshall Islands"},
{"MK", "North Macedonia"},
{"ML", "Mali"},
{"MM", "Myanmar"},
{"MN", "Mongolia"},
{"MO", "Macao"},
{"MP", "Northern Mariana Islands"},
{"MQ", "Martinique"},
{"MR", "Mauritania"},
{"MS", "Montserrat"},
{"MT", "Malta"},
{"MU", "Mauritius"},
{"MV", "Maldives"},
{"MW", "Malawi"},
{"MX", "Mexico"},
{"MY", "Malaysia"},
{"MZ", "Mozambique"},
{"NA", "Namibia"},
{"NC", "New Caledonia"},
{"NE", "Niger"},
{"NF", "Norfolk Island"},
{"NG", "Nigeria"},
{"NI", "Nicaragua"},
{"NL", "Netherlands"},
{"NO", "Norway"},
{"NP", "Nepal"},
{"NR", "Nauru"},
{"NU", "Niue"},
{"NZ", "New Zealand"},
{"OM", "Oman"},
{"PA", "Panama"},
{"PE", "Peru"},
{"PF", "French Polynesia"},
{"PG", "Papua New Guinea"},
{"PH", "Philippines"},
{"PK", "Pakistan"},
{"PL", "Poland"},
{"PM", "Saint Pierre and Miquelon"},
{"PN", "Pitcairn"},
{"PR", "Puerto Rico"},
{"PS", "Palestinian Territory"},
{"PT", "Portugal"},
{"PW", "Palau"},
{"PY", "Paraguay"},
{"QA", "Qatar"},
{"RE", "Reunion"},
{"RO", "Romania"},
{"RS", "Serbia"},
{"RU", "Russia"},
{"RW", "Rwanda"},
{"SA", "Saudi Arabia"},
{"SB", "Solomon Islands"},
{"SC", "Seychelles"},
{"SD", "Sudan"},
{"SE", "Sweden"},
{"SG", "Singapore"},
{"SH", "Saint Helena"},
{"SI", "Slovenia"},
{"SJ", "Svalbard and Jan Mayen"},
{"SK", "Slovakia"},
{"SL", "Sierra Leone"},
{"SM", "San Marino"},
{"SN", "Senegal"},
{"SO", "Somalia"},
{"SR", "Suriname"},
{"SS", "South Sudan"},
{"ST", "Sao Tome and Principe"},
{"SV", "El Salvador"},
{"SX", "Sint Maarten"},
{"SY", "Syria"},
{"SZ", "Eswatini"},
{"TC", "Turks and Caicos Islands"},
{"TD", "Chad"},
{"TF", "French Southern Territories"},
{"TG", "Togo"},
{"TH", "Thailand"},
{"TJ", "Tajikistan"},
{"TK", "Tokelau"},
{"TL", "Timor Leste"},
{"TM", "Turkmenistan"},
{"TN", "Tunisia"},
{"TO", "Tonga"},
{"TR", "Turkey"},
{"TT", "Trinidad and Tobago"},
{"TV", "Tuvalu"},
{"TW", "Taiwan"},
{"TZ", "Tanzania"},
{"UA", "Ukraine"},
{"UG", "Uganda"},
{"UM", "United States Minor Outlying Islands"},
{"US", "United States"},
{"UY", "Uruguay"},
{"UZ", "Uzbekistan"},
{"VA", "Vatican"},
{"VC", "Saint Vincent and the Grenadines"},
{"VE", "Venezuela"},
{"VG", "British Virgin Islands"},
{"VI", "U.S. Virgin Islands"},
{"VN", "Vietnam"},
{"VU", "Vanuatu"},
{"WF", "Wallis and Futuna"},
{"WS", "Samoa"},
{"XK", "Kosovo"},
{"YE", "Yemen"},
{"YT", "Mayotte"},
{"ZA", "South Africa"},
{"ZM", "Zambia"},
{"ZW", "Zimbabwe"},
};

// List of USA states
CONST CN rgcnus[icnusMax] = {
{"AK", "Alaska"},
{"AL", "Alabama"},
{"AR", "Arkansas"},
{"AZ", "Arizona"},
{"CA", "California"},
{"CO", "Colorado"},
{"CT", "Connecticut"},
{"DC", "District of Columbia"},
{"DE", "Delaware"},
{"FL", "Florida"},
{"GA", "Georgia"},
{"HI", "Hawaii"},
{"IA", "Iowa"},
{"ID", "Idaho"},
{"IL", "Illinois"},
{"IN", "Indiana"},
{"KS", "Kansas"},
{"KY", "Kentucky"},
{"LA", "Louisiana"},
{"MA", "Massachusetts"},
{"MD", "Maryland"},
{"ME", "Maine"},
{"MI", "Michigan"},
{"MN", "Minnesota"},
{"MO", "Missouri"},
{"MS", "Mississippi"},
{"MT", "Montana"},
{"NC", "North Carolina"},
{"ND", "North Dakota"},
{"NE", "Nebraska"},
{"NH", "New Hampshire"},
{"NJ", "New Jersey"},
{"NM", "New Mexico"},
{"NV", "Nevada"},
{"NY", "New York"},
{"OH", "Ohio"},
{"OK", "Oklahoma"},
{"OR", "Oregon"},
{"PA", "Pennsylvania"},
{"RI", "Rhode Island"},
{"SC", "South Carolina"},
{"SD", "South Dakota"},
{"TN", "Tennessee"},
{"TX", "Texas"},
{"UT", "Utah"},
{"VA", "Virginia"},
{"VT", "Vermont"},
{"WA", "Washington"},
{"WI", "Wisconsin"},
{"WV", "West Virginia"},
{"WY", "Wyoming"},
};

// List of Canada provinces
CONST CN rgcnca[icncaMax] = {
{"AB", "Alberta"},
{"BC", "British Columbia"},
{"MB", "Manitoba"},
{"NB", "New Brunswick"},
{"NL", "Newfoundland and Labrador"},
{"NS", "Nova Scotia"},
{"NT", "Northwest Territories"},
{"NU", "Nunavut"},
{"ON", "Ontario"},
{"PE", "Prince Edward Island"},
{"QC", "Quebec"},
{"SK", "Saskatchewan"},
{"YT", "Yukon"},
};

// List of time zones
CONST char *rgszzn[iznMax] = {
"Africa/Abidjan",
"Africa/Accra",
"Africa/Addis_Ababa",
"Africa/Algiers",
"Africa/Asmara",
"Africa/Bamako",
"Africa/Bangui",
"Africa/Banjul",
"Africa/Bissau",
"Africa/Blantyre",
"Africa/Brazzaville",
"Africa/Bujumbura",
"Africa/Cairo",
"Africa/Casablanca",
"Africa/Ceuta",
"Africa/Conakry",
"Africa/Dakar",
"Africa/Dar_es_Salaam",
"Africa/Djibouti",
"Africa/Douala",
"Africa/El_Aaiun",
"Africa/Freetown",
"Africa/Gaborone",
"Africa/Harare",
"Africa/Johannesburg",
"Africa/Juba",
"Africa/Kampala",
"Africa/Khartoum",
"Africa/Kigali",
"Africa/Kinshasa",
"Africa/Lagos",
"Africa/Libreville",
"Africa/Lome",
"Africa/Luanda",
"Africa/Lubumbashi",
"Africa/Lusaka",
"Africa/Malabo",
"Africa/Maputo",
"Africa/Maseru",
"Africa/Mbabane",
"Africa/Mogadishu",
"Africa/Monrovia",
"Africa/Nairobi",
"Africa/Ndjamena",
"Africa/Niamey",
"Africa/Nouakchott",
"Africa/Ouagadougou",
"Africa/Porto-Novo",
"Africa/Sao_Tome",
"Africa/Tripoli",
"Africa/Tunis",
"Africa/Windhoek",
"America/Adak",
"America/Anchorage",
"America/Anguilla",
"America/Antigua",
"America/Araguaina",
"America/Argentina/Buenos_Aires",
"America/Argentina/Catamarca",
"America/Argentina/Cordoba",
"America/Argentina/Jujuy",
"America/Argentina/La_Rioja",
"America/Argentina/Mendoza",
"America/Argentina/Rio_Gallegos",
"America/Argentina/Salta",
"America/Argentina/San_Juan",
"America/Argentina/San_Luis",
"America/Argentina/Tucuman",
"America/Argentina/Ushuaia",
"America/Aruba",
"America/Asuncion",
"America/Atikokan",
"America/Bahia",
"America/Bahia_Banderas",
"America/Barbados",
"America/Belem",
"America/Belize",
"America/Blanc-Sablon",
"America/Boa_Vista",
"America/Bogota",
"America/Boise",
"America/Cambridge_Bay",
"America/Campo_Grande",
"America/Cancun",
"America/Caracas",
"America/Cayenne",
"America/Cayman",
"America/Chicago",
"America/Chihuahua",
"America/Costa_Rica",
"America/Creston",
"America/Cuiaba",
"America/Curacao",
"America/Danmarkshavn",
"America/Dawson",
"America/Dawson_Creek",
"America/Denver",
"America/Detroit",
"America/Dominica",
"America/Edmonton",
"America/Eirunepe",
"America/El_Salvador",
"America/Fort_Nelson",
"America/Fortaleza",
"America/Glace_Bay",
"America/Godthab",
"America/Goose_Bay",
"America/Grand_Turk",
"America/Grenada",
"America/Guadeloupe",
"America/Guatemala",
"America/Guayaquil",
"America/Guyana",
"America/Halifax",
"America/Havana",
"America/Hermosillo",
"America/Indiana/Indianapolis",
"America/Indiana/Knox",
"America/Indiana/Marengo",
"America/Indiana/Petersburg",
"America/Indiana/Tell_City",
"America/Indiana/Vevay",
"America/Indiana/Vincennes",
"America/Indiana/Winamac",
"America/Inuvik",
"America/Iqaluit",
"America/Jamaica",
"America/Juneau",
"America/Kentucky/Louisville",
"America/Kentucky/Monticello",
"America/Kralendijk",
"America/La_Paz",
"America/Lima",
"America/Los_Angeles",
"America/Lower_Princes",
"America/Maceio",
"America/Managua",
"America/Manaus",
"America/Marigot",
"America/Martinique",
"America/Matamoros",
"America/Mazatlan",
"America/Menominee",
"America/Merida",
"America/Metlakatla",
"America/Mexico_City",
"America/Miquelon",
"America/Moncton",
"America/Monterrey",
"America/Montevideo",
"America/Montserrat",
"America/Nassau",
"America/New_York",
"America/Nipigon",
"America/Nome",
"America/Noronha",
"America/North_Dakota/Beulah",
"America/North_Dakota/Center",
"America/North_Dakota/New_Salem",
"America/Ojinaga",
"America/Panama",
"America/Pangnirtung",
"America/Paramaribo",
"America/Phoenix",
"America/Port-au-Prince",
"America/Port_of_Spain",
"America/Porto_Velho",
"America/Puerto_Rico",
"America/Punta_Arenas",
"America/Rainy_River",
"America/Rankin_Inlet",
"America/Recife",
"America/Regina",
"America/Resolute",
"America/Rio_Branco",
"America/Santarem",
"America/Santiago",
"America/Santo_Domingo",
"America/Sao_Paulo",
"America/Scoresbysund",
"America/Sitka",
"America/St_Barthelemy",
"America/St_Johns",
"America/St_Kitts",
"America/St_Lucia",
"America/St_Thomas",
"America/St_Vincent",
"America/Swift_Current",
"America/Tegucigalpa",
"America/Thule",
"America/Thunder_Bay",
"America/Tijuana",
"America/Toronto",
"America/Tortola",
"America/Vancouver",
"America/Whitehorse",
"America/Winnipeg",
"America/Yakutat",
"America/Yellowknife",
"Antarctica/Casey",
"Antarctica/Davis",
"Antarctica/DumontDUrville",
"Antarctica/Macquarie",
"Antarctica/Mawson",
"Antarctica/McMurdo",
"Antarctica/Palmer",
"Antarctica/Rothera",
"Antarctica/Syowa",
"Antarctica/Troll",
"Antarctica/Vostok",
"Arctic/Longyearbyen",
"Asia/Aden",
"Asia/Almaty",
"Asia/Amman",
"Asia/Anadyr",
"Asia/Aqtau",
"Asia/Aqtobe",
"Asia/Ashgabat",
"Asia/Atyrau",
"Asia/Baghdad",
"Asia/Bahrain",
"Asia/Baku",
"Asia/Bangkok",
"Asia/Barnaul",
"Asia/Beirut",
"Asia/Bishkek",
"Asia/Brunei",
"Asia/Chita",
"Asia/Choibalsan",
"Asia/Colombo",
"Asia/Damascus",
"Asia/Dhaka",
"Asia/Dili",
"Asia/Dubai",
"Asia/Dushanbe",
"Asia/Famagusta",
"Asia/Gaza",
"Asia/Hebron",
"Asia/Ho_Chi_Minh",
"Asia/Hong_Kong",
"Asia/Hovd",
"Asia/Irkutsk",
"Asia/Jakarta",
"Asia/Jayapura",
"Asia/Jerusalem",
"Asia/Kabul",
"Asia/Kamchatka",
"Asia/Karachi",
"Asia/Kathmandu",
"Asia/Khandyga",
"Asia/Kolkata",
"Asia/Krasnoyarsk",
"Asia/Kuala_Lumpur",
"Asia/Kuching",
"Asia/Kuwait",
"Asia/Macau",
"Asia/Magadan",
"Asia/Makassar",
"Asia/Manila",
"Asia/Muscat",
"Asia/Nicosia",
"Asia/Novokuznetsk",
"Asia/Novosibirsk",
"Asia/Omsk",
"Asia/Oral",
"Asia/Phnom_Penh",
"Asia/Pontianak",
"Asia/Pyongyang",
"Asia/Qatar",
"Asia/Qostanay",
"Asia/Qyzylorda",
"Asia/Riyadh",
"Asia/Sakhalin",
"Asia/Samarkand",
"Asia/Seoul",
"Asia/Shanghai",
"Asia/Singapore",
"Asia/Srednekolymsk",
"Asia/Taipei",
"Asia/Tashkent",
"Asia/Tbilisi",
"Asia/Tehran",
"Asia/Thimphu",
"Asia/Tokyo",
"Asia/Tomsk",
"Asia/Ulaanbaatar",
"Asia/Urumqi",
"Asia/Ust-Nera",
"Asia/Vientiane",
"Asia/Vladivostok",
"Asia/Yakutsk",
"Asia/Yangon",
"Asia/Yekaterinburg",
"Asia/Yerevan",
"Atlantic/Azores",
"Atlantic/Bermuda",
"Atlantic/Canary",
"Atlantic/Cape_Verde",
"Atlantic/Faroe",
"Atlantic/Madeira",
"Atlantic/Reykjavik",
"Atlantic/South_Georgia",
"Atlantic/St_Helena",
"Atlantic/Stanley",
"Australia/Adelaide",
"Australia/Brisbane",
"Australia/Broken_Hill",
"Australia/Currie",
"Australia/Darwin",
"Australia/Eucla",
"Australia/Hobart",
"Australia/Lindeman",
"Australia/Lord_Howe",
"Australia/Melbourne",
"Australia/Perth",
"Australia/Sydney",
"Europe/Amsterdam",
"Europe/Andorra",
"Europe/Astrakhan",
"Europe/Athens",
"Europe/Belgrade",
"Europe/Berlin",
"Europe/Bratislava",
"Europe/Brussels",
"Europe/Bucharest",
"Europe/Budapest",
"Europe/Busingen",
"Europe/Chisinau",
"Europe/Copenhagen",
"Europe/Dublin",
"Europe/Gibraltar",
"Europe/Guernsey",
"Europe/Helsinki",
"Europe/Isle_of_Man",
"Europe/Istanbul",
"Europe/Jersey",
"Europe/Kaliningrad",
"Europe/Kiev",
"Europe/Kirov",
"Europe/Lisbon",
"Europe/Ljubljana",
"Europe/London",
"Europe/Luxembourg",
"Europe/Madrid",
"Europe/Malta",
"Europe/Mariehamn",
"Europe/Minsk",
"Europe/Monaco",
"Europe/Moscow",
"Europe/Oslo",
"Europe/Paris",
"Europe/Podgorica",
"Europe/Prague",
"Europe/Riga",
"Europe/Rome",
"Europe/Samara",
"Europe/San_Marino",
"Europe/Sarajevo",
"Europe/Saratov",
"Europe/Simferopol",
"Europe/Skopje",
"Europe/Sofia",
"Europe/Stockholm",
"Europe/Tallinn",
"Europe/Tirane",
"Europe/Ulyanovsk",
"Europe/Uzhgorod",
"Europe/Vaduz",
"Europe/Vatican",
"Europe/Vienna",
"Europe/Vilnius",
"Europe/Volgograd",
"Europe/Warsaw",
"Europe/Zagreb",
"Europe/Zaporozhye",
"Europe/Zurich",
"Indian/Antananarivo",
"Indian/Chagos",
"Indian/Christmas",
"Indian/Cocos",
"Indian/Comoro",
"Indian/Kerguelen",
"Indian/Mahe",
"Indian/Maldives",
"Indian/Mauritius",
"Indian/Mayotte",
"Indian/Reunion",
"Pacific/Apia",
"Pacific/Auckland",
"Pacific/Bougainville",
"Pacific/Chatham",
"Pacific/Chuuk",
"Pacific/Easter",
"Pacific/Efate",
"Pacific/Enderbury",
"Pacific/Fakaofo",
"Pacific/Fiji",
"Pacific/Funafuti",
"Pacific/Galapagos",
"Pacific/Gambier",
"Pacific/Guadalcanal",
"Pacific/Guam",
"Pacific/Honolulu",
"Pacific/Kiritimati",
"Pacific/Kosrae",
"Pacific/Kwajalein",
"Pacific/Majuro",
"Pacific/Marquesas",
"Pacific/Midway",
"Pacific/Nauru",
"Pacific/Niue",
"Pacific/Norfolk",
"Pacific/Noumea",
"Pacific/Pago_Pago",
"Pacific/Palau",
"Pacific/Pitcairn",
"Pacific/Pohnpei",
"Pacific/Port_Moresby",
"Pacific/Rarotonga",
"Pacific/Saipan",
"Pacific/Tahiti",
"Pacific/Tarawa",
"Pacific/Tongatapu",
"Pacific/Wake",
"Pacific/Wallis",
};


/*
******************************************************************************
** Atlas Procedures.
******************************************************************************
*/

// Compose a string to display a city. Include the country/region it lies in,
// along with the US state or Canada province, if in those countries.

char *SzCity(int iae)
{
  static char szCity[cchSzMax];
  CONST char *pchState;
  int icn, istate;

  icn = is.rgae[iae].icn;
  istate = is.rgae[iae].istate;
  pchState = istate < 0 ? "" :
    (icn == icnUS ? rgcnus[istate].szAbb : rgcnca[istate].szAbb);
  // Country US displays as "USA" instead of full name "United States"
  sprintf(szCity, "%s, %s%s%s", is.rgae[iae].szNam, pchState,
    *pchState ? ", " : "", icn == icnUS ? "USA" : rgcnew[icn].szNam);
  return szCity;
}


// Lookup an abbreviation for a country/region or state/province in a list of
// abbreviations. Used when parsing city names which include them.

int ILookupCN(CONST char *szAbb, CONST CN *rgcn, int ccn)
{
  int icn;
  CONST char *pch;

  if (CchSz(szAbb) != 2)  // Abbreviations must be 2 letters long.
    return -1;
  for (icn = 0; icn < ccn; icn++) {
    pch = rgcn[icn].szAbb;
    if (ChCap(szAbb[0]) == pch[0] && ChCap(szAbb[1]) == pch[1])
      return icn;
  }
  return -1;
}


// Load the atlas information file if it hasn't been loaded yet. Return
// whether loading has succeeded, i.e. whether city list is allocated.

flag FEnsureAtlas()
{
  if (is.rgae != NULL)
    return fTrue;
  if (!FProcessSwitchFile(DEFAULT_ATLASFILE, NULL))
    return fFalse;
  return is.rgae != NULL;
}


// Load the time zone change information file if it hasn't been loaded yet.
// Return whether loading has succeeded, i.e. whether the necessary zone
// change and zone Daylight Saving rules arrays have been allocated.

flag FEnsureTimezoneChanges()
{
  if (is.rgzc != NULL && is.rgrun != NULL && is.rgrue != NULL)
    return fTrue;
  if (!FProcessSwitchFile(DEFAULT_TIMECHANGE, NULL))
    return fFalse;
  return is.rgzc != NULL && is.rgrun != NULL && is.rgrue != NULL;
}


// Load atlas information from an open file, consisting of the specified
// number of city entries. Implements the -YY command switch.

flag FLoadAtlas(FILE *file, int cae)
{
  char szLine[cchSzMax], szAbb[cchSzDef], *pch;
  int i, j;
  AtlasEntry *pae;

#ifdef DEBUG
  // Ensure country/region list abbreviations are in sorted order
  for (i = 0; i < icnewMax-1; i++)
    Assert(NCompareSz(rgcnew[i].szAbb, rgcnew[i+1].szAbb) < 0);
  for (i = 0; i < icnusMax-1; i++)
    Assert(NCompareSz(rgcnus[i].szAbb, rgcnus[i+1].szAbb) < 0);
  for (i = 0; i < icncaMax-1; i++)
    Assert(NCompareSz(rgcnca[i].szAbb, rgcnca[i+1].szAbb) < 0);
  for (i = 0; i < iznMax-1; i++)
    Assert(NCompareSz(rgszzn[i], rgszzn[i+1]) < 0);
  // Ensure certain countries are located where expected
  Assert(NCompareSz(rgcnew[icnUS].szAbb, "US") == 0);  // USA
  Assert(NCompareSz(rgcnew[icnCA].szAbb, "CA") == 0);  // Canada
  Assert(NCompareSz(rgcnew[icnFR].szAbb, "FR") == 0);  // France
#endif

  // Free previous city list if present, and allocate new list.
  is.cae = 0;
  if (is.rgae != NULL) {
    DeallocateP(is.rgae);
    is.rgae = NULL;
  }
  is.rgae = RgAllocate(cae, AtlasEntry, "atlas");
  if (is.rgae == NULL)
    return fFalse;

  for (i = 0; i < cae; i++) {
    pae = &is.rgae[i];
    fgets(szLine, cchSzMax, file);

    // Parse location coordinates
    sscanf(szLine, "%lf%lf", &pae->lon, &pae->lat);
    for (pch = szLine, j = 0; j < 2; j++) {
      while (*pch && *pch != '\t')
        pch++;
      if (*pch == '\t')
        pch++;
    }

    // Parse country/region or state abbreviation
    ClearB((pbyte)&szAbb, sizeof(szAbb));
    szAbb[0] = *pch;
    if (*pch)
      szAbb[1] = *++pch;
    while (*pch && *pch != '\t')
      pch++;
    if (*pch == '\t')
      pch++;
    if (FCapCh(szAbb[0])) {
      pae->icn = ILookupCN(szAbb, rgcnew, icnewMax);
      j = -1;
    } else {
      j = ILookupCN(szAbb, rgcnus, icnusMax);
      if (j >= 0)
        pae->icn = icnUS;
      else {
        j = ILookupCN(szAbb, rgcnca, icncaMax);
        pae->icn = j >= 0 ? icnCA : -1;
      }
    }
    pae->istate = j;

    // Parse city name
    for (j = 0; j < cchSzAtl-1 && pch[j] >= ' '; j++)
      pae->szNam[j] = pch[j];
    pae->szNam[j] = chNull;
    if (pae->icn < 0) {
      sprintf(szLine,
        "Atlas error: City %d in unknown country/region: '%s'\n", i, szAbb);
      PrintError(szLine);
      return fFalse;
    }
    while (*pch && *pch != '\t')
      pch++;
    if (*pch == '\t')
      pch++;

    // Parse time zone name
    if (*pch < ' ')
      j = i > 0 ? pae[-1].izn : iznMax;
    else {
      for (j = 0; pch[j] >= ' '; j++)
        ;
      pch[j] = chNull;
      for (j = 0; j < iznMax; j++)
        if (NCompareSz(pch, rgszzn[j]) == 0)
          break;
    }
    if (j >= iznMax) {
      sprintf(szLine,
        "Atlas error: City %d in unknown time zone: '%s'\n", i, pch);
      PrintError(szLine);
      return fFalse;
    }
    pae->izn = j;
  }
  is.cae = cae;
  return fTrue;
}


// Parse an Hours:Minutes:Seconds (HMS) time value, returning the total
// number of seconds. For example, -12hr 30min 15sec or "-12:30:15" maps to
// -(12 hr*60*60 + 30 min*60 + 15 sec) = 45015 seconds total.

int NParseHMS(CONST char *sz)
{
  int n;
  flag fNeg;

  fNeg = (*sz == '-');
  // Parse hours.
  n = atoi(sz)*3600;
  if (fNeg)
    neg(n);
  while (*sz > ' ' && *sz != ':')
    sz++;
  // If a colon present after first number, parse minutes.
  if (*sz == ':') {
    n += atoi(++sz)*60;
    while (*sz > ' ' && *sz != ':')
      sz++;
    // If a second colon present, parse seconds.
    if (*sz == ':')
      n += atoi(++sz);
  }
  if (fNeg)
    neg(n);
  return n;
}


// Load zone change rules from an open file, consisting of the specified
// number of rules, each consisting of a sublist of rule entries (total rule
// entries also specified). Implements the -YY1 command switch.

flag FLoadZoneRules(FILE *file, int irunMax, int irueMax)
{
  char szLine[cchSzMax], szErr[cchSzMax], *pch, *pchT, chT;
  int i, j, crue, n;
  RuleName *prun;
  RuleEntry *prue;

  // Free previous rule lists if present, and allocate new lists.
  is.crun = is.crue = 0;
  if (is.rgrun != NULL) {
    DeallocateP(is.rgrun);
    is.rgrun = NULL;
  }
  is.rgrun = RgAllocate(irunMax+1, RuleName, "timezone rule names");
  if (is.rgrun == NULL)
    return fFalse;
  ClearB((pbyte)is.rgrun, sizeof(RuleName) * (irunMax+1));
  if (is.rgrue != NULL) {
    DeallocateP(is.rgrue);
    is.rgrue = NULL;
  }
  is.rgrue = RgAllocate(irueMax, RuleEntry, "timezone rule entries");
  if (is.rgrue == NULL)
    return fFalse;
  ClearB((pbyte)is.rgrue, sizeof(RuleEntry) * irueMax);

  // Read in each rule.
  for (i = 0; i < irunMax; i++) {
    prun = &is.rgrun[i];
    fgets(szLine, cchSzMax, file);
    for (pch = szLine; *pch > ' '; pch++)
      ;
    if (*pch)
      *pch++ = chNull;
    if (CchSz(szLine) >= cchSzZon) {
      sprintf(szErr, "Zone rule error: Rule %d (%s) is too long a string, "
        "which exceeds string length limit of %d\n",
        i, szLine, cchSzZon-1);
      PrintError(szErr);
      return fFalse;
    }
    for (j = 0; j < cchSzZon-1 && szLine[j] >= ' '; j++)
      prun->szNam[j] = szLine[j];
    prun->szNam[j] = chNull;
    crue = atoi(pch);
    prun[1].irue = prun->irue + crue;
    if (prun[1].irue > irueMax) {
      sprintf(szErr, "Zone rule error: Rule %d (%s) has %d entries, "
        "which exceed total entry limit of %d\n",
        i, prun->szNam, crue, irueMax);
      PrintError(szErr);
      return fFalse;
    }

    // Read in the current rule's list of rule entries.
    for (j = 0; j < crue; j++) {
      prue = &is.rgrue[prun->irue + j];
      fgets(szLine, cchSzMax, file);

      // Parse year range at which rule entry applies.
      prue->yea1 = atoi(szLine);
      for (pch = szLine; *pch && *pch > ' '; pch++)
        ;
      while (*pch && *pch <= ' ')
        pch++;
      if (*pch == 'o') // "only"
        prue->yea2 = prue->yea1;
      else if (*pch == 'm') // "max"
        prue->yea2 = 9999;
      else
        prue->yea2 = atoi(pch);
      while (*pch > ' ')
        pch++;
      while (*pch && *pch <= ' ')
        pch++;

      // Parse month at which Daylight Time offset changes.
      for (pchT = pch; *pch > ' '; pch++)
        ;
      if (*pch)
        *pch++ = chNull;
      n = NParseSz(pchT, pmMon);
      if (!FValidMon(n)) {
        sprintf(szErr,
          "Zone rule error: Bad month in entry %d of rule %d: '%s'\n",
          j, i, pchT);
        PrintError(szErr);
        return fFalse;
      }
      prue->mon = n;
      while (*pch && *pch <= ' ')
        pch++;

      // Parse day specification at which Daylight Time offset changes.
      for (pchT = pch; *pch > ' '; pch++)
        ;
      if (*pch)
        *pch++ = chNull;
      n = atoi(pchT);
      if (n > 0) {
        Assert(prue->daytype == 0);
        prue->dayweek = -1;
        prue->daynum = n;
      } else if (*pchT == 'l') {
        prue->daytype = 1;
        prue->daynum = -1;
        n = NParseSz(pchT + 4, pmWeek);
        if (!FValidWeek(n)) {
          sprintf(szErr, "Zone rule error: "
            "Bad lastDay of week in entry %d of rule %d: '%s'\n", j, i, pchT);
          PrintError(szErr);
          return fFalse;
        }
        prue->dayweek = n;
      } else {
        chT = pchT[3];
        pchT[3] = chNull;
        n = NParseSz(pchT, pmWeek);
        if (!FValidWeek(n)) {
          sprintf(szErr, "Zone rule error: "
            "Bad Day==X of week in entry %d of rule %d: '%s'\n", j, i, pchT);
          PrintError(szErr);
          return fFalse;
        }
        prue->dayweek = n;
        if (!((chT == '>' || chT == '<') && pchT[4] == '=')) {
          sprintf(szErr, "Zone rule error: "
            "Day operator not >= or <= in entry %d of rule %d: '%s'\n",
            j, i, pchT);
          PrintError(szErr);
          return fFalse;
        }
        prue->daytype = 2 + (chT == '<');
        prue->daynum = atoi(pchT + 5);
      }

      // Parse time at which Daylight Time offset changes.
      while (*pch && *pch <= ' ')
        pch++;
      prue->tim = NParseHMS(pch);
      while (*pch > ' ')
        pch++;
      if (pch[-1] == 'u')
        prue->timtype = 2;
      else if (pch[-1] == 's')
        prue->timtype = 1;
      while (*pch && *pch <= ' ')
        pch++;

      // Parse new Daylight Time offset.
      prue->dst = NParseHMS(pch);
    }
  }

  // Sanity check results.
  if (is.rgrun[irunMax].irue != irueMax) {
    sprintf(szErr, "Zone rule error: The %d rules have %d entries, "
      "which differs from total rule entry limit of %d\n",
      irunMax, is.rgrun[irunMax].irue, irueMax);
    PrintError(szErr);
    return fFalse;
  }
  is.crun = irunMax;
  is.crue = irueMax;
  return fTrue;
}


// Load time zone changes from an open file, consisting of the specified
// number of zones, each consisting of a sublist of zone change entries
// (total entries also specified). Implements the -YY2 command switch.

int rgznChange[iznMax], rgizcChange[iznMax+1];

flag FLoadZoneChanges(FILE *file, int izcnMax, int izceMax)
{
  char szLine[cchSzMax], szErr[cchSzMax], *pch, *pchT;
  int i, j, izn, czn, n;
  flag rgfUsed[iznMax];
  ZoneChange *pzc;

  // Free previous zone change entry list if present, and allocate new list.
  is.czcn = is.czce = 0;
  if (is.rgzc != NULL) {
    DeallocateP(is.rgzc);
    is.rgzc = NULL;
  }
  is.rgzc = RgAllocate(izceMax, ZoneChange, "timezone changes");
  if (is.rgzc == NULL)
    return fFalse;
  ClearB((pbyte)is.rgzc, sizeof(ZoneChange) * izceMax);
  ClearB((pbyte)rgfUsed, sizeof(rgfUsed));
  rgznChange[0] = 0;

  // Read in each zone change area.
  for (i = 0; i < izcnMax; i++) {
    fgets(szLine, cchSzMax, file);
    for (pch = szLine; *pch > ' '; pch++)
      ;
    if (*pch)
      *pch++ = chNull;
    for (izn = 0; izn < iznMax; izn++) {
      if (NCompareSz(szLine, rgszzn[izn]) == 0)
        break;
    }
    if (izn >= iznMax) {
      sprintf(szErr, "Zone change error: Zone %d unknown: '%s'\n", i, szLine);
      PrintError(szErr);
      return fFalse;
    }
    rgznChange[i] = izn;
    czn = atoi(pch);
    rgizcChange[i+1] = rgizcChange[i] + czn;
    if (rgizcChange[i+1] > izceMax) {
      sprintf(szErr, "Zone change error: Zone %d (%s) has %d entries, "
        "which exceed total entry limit of %d\n",
        i, rgszzn[izn], czn, izceMax);
      PrintError(szErr);
      return fFalse;
    }

    // Read in the current zone's list of zone change entries.
    for (j = 0; j < czn; j++) {
      pzc = &is.rgzc[rgizcChange[i] + j];
      fgets(szLine, cchSzMax, file);

      // Parse offset to use for this time zone.
      for (pch = szLine; *pch > ' '; pch++)
        ;
      if (*pch)
        *pch++ = chNull;
      pzc->zon = NParseHMS(szLine);
      while (*pch <= ' ')
        pch++;

      // Parse Daylight Saving rule to apply during timeframe.
      for (pchT = pch; *pch > ' '; pch++)
        ;
      if (*pch)
        *pch++ = chNull;
      pzc->irun = -1;
      pzc->dst = nLarge;
      for (n = 0; n < is.crun; n++)
        if (NCompareSz(pchT, is.rgrun[n].szNam) == 0) {
          pzc->irun = n;
          rgfUsed[n] = fTrue;
          break;
        }
      if (pzc->irun < 0) {
        if (FNumCh(*pchT) || (*pchT == '-' && pchT[1] > ' '))
          pzc->dst = NParseHMS(pchT);
        else if (*pchT != '-') {
          sprintf(szErr,
            "Zone change error: Unknown rule in entry %d of zone %d: '%s'\n",
            j, i, pchT);
          PrintError(szErr);
          return fFalse;
        }
      }
      pzc->yea = 9999;
      pzc->mon = 1; pzc->day = 1; pzc->tim = 0; pzc->timtype = 0;

      // Parse year if present, else assume lasts forever.
      while (*pch && *pch <= ' ')
        pch++;
      if (*pch == chNull)
        continue;
      pzc->yea = atoi(pch);
      while (*pch > ' ')
        pch++;

      // Parse month if present, else assume January.
      while (*pch && *pch <= ' ')
        pch++;
      if (*pch == chNull)
        continue;
      for (pchT = pch; *pch > ' '; pch++)
        ;
      if (*pch)
        *pch++ = chNull;
      n = NParseSz(pchT, pmMon);
      if (!FValidMon(n)) {
        sprintf(szErr,
          "Zone change error: Bad month in entry %d of zone %d: '%s'\n",
          j, i, pchT);
        PrintError(szErr);
        return fFalse;
      }
      pzc->mon = n;

      // Parse day if present, else assume 1st of month.
      while (*pch && *pch <= ' ')
        pch++;
      if (*pch == chNull)
        continue;
      pzc->day = atoi(pch);
      while (*pch > ' ')
        pch++;

      // Parse time if present, else assume midnight local time.
      while (*pch && *pch <= ' ')
        pch++;
      if (*pch == chNull)
        continue;
      pzc->tim = NParseHMS(pch);
      while (*pch > ' ')
        pch++;
      if (pch[-1] == 'u')
        pzc->timtype = 2;
      else if (pch[-1] == 's')
        pzc->timtype = 1;
    }
  }

  // Sanity check results.
  for (i = 0; i < is.crun; i++)
    if (!rgfUsed[i]) {
      sprintf(szErr, "Zone change error: Rule %d (%s) is never used "
        "by any zone change entry.\n", i, is.rgrun[i].szNam);
      PrintError(szErr);
      return fFalse;
    }
  if (rgizcChange[izcnMax] != izceMax) {
    sprintf(szErr, "Zone change error: The %d zones have %d entries, "
      "which differs from total entry limit of %d\n",
      izcnMax, rgizcChange[izcnMax], izceMax);
    PrintError(szErr);
    return fFalse;
  }
  is.czcn = izcnMax;
  is.czce = izceMax;
  return fTrue;
}


// Load time zone links from an open file, consisting of the specified number
// of time zones from atlas.as, and how each maps to a zone change area
// specified above with -YY2. Implements the -YY3 command switch.

int mpznzc[iznMax];

flag FLoadZoneLinks(FILE *file, int czl)
{
  char szLine[cchSzMax], szErr[cchSzMax], *pch, *szFrom, *szTo;
  int i, iznFrom, iznTo, izn, izc;

  // Initialize list to invalid values (clear any previous data).
  for (i = 0; i < iznMax; i++)
    mpznzc[i] = -1;

  // Read in each link.
  for (i = 0; i < czl; i++) {
    fgets(szLine, cchSzMax, file);
    for (pch = szFrom = szLine; *pch > ' '; pch++)
      ;
    if (*pch)
      *pch++ = chNull;
    szTo = pch;
    while (*pch > ' ')
      pch++;
    *pch = chNull;

    // Ensure "from" string is a valid time zone from atlas.as
    for (iznFrom = 0; iznFrom < iznMax; iznFrom++) {
      if (NCompareSz(szFrom, rgszzn[iznFrom]) == 0)
        break;
    }
    if (iznFrom >= iznMax) {
      sprintf(szErr, "Zone link error: Link %d from unknown: '%s'\n",
        i, szFrom);
      PrintError(szErr);
      return fFalse;
    }

    // Ensure "to" string is a valid time zone from atlas.as
    for (iznTo = 0; iznTo < iznMax; iznTo++) {
      if (NCompareSz(szTo, rgszzn[iznTo]) == 0)
        break;
    }
    if (iznTo >= iznMax) {
      sprintf(szErr, "Zone link error: Link %d to unknown: '%s'\n", i, szTo);
      PrintError(szErr);
      return fFalse;
    }

    // Ensure "from" string zone hasn't been defined with zone change entries.
    for (izc = 0; izc < is.czcn; izc++)
      if (iznFrom == rgznChange[izc])
        break;
    if (izc < is.czcn) {
      sprintf(szErr, "Zone link error: Link %d redirects defined: '%s'\n",
        i, rgszzn[iznFrom]);
      PrintError(szErr);
      return fFalse;
    }

    // Ensure "to" string zone has been defined with zone change entries.
    for (izc = 0; izc < is.czcn; izc++)
      if (iznTo == rgznChange[izc])
        break;
    if (izc >= is.czcn) {
      sprintf(szErr, "Zone link error: Link %d to undefined: '%s'\n",
        i, rgszzn[iznTo]);
      PrintError(szErr);
      return fFalse;
    }
    mpznzc[iznFrom] = izc;
  }

  // Compose mapping list from time zone index to zone change index.
  for (izn = 0; izn < iznMax; izn++) {
    if (mpznzc[izn] >= 0)
      continue;
    for (izc = 0; izc < is.czcn; izc++)
      if (izn == rgznChange[izc])
        break;
    if (izc >= is.czcn) {
      sprintf(szErr, "Zone link error: Zone %d undefined: '%s'\n",
        izn, rgszzn[izn]);
      PrintError(szErr);
      return fFalse;
    }
    mpznzc[izn] = izc;
  }
  return fTrue;
}


// Lookup a city in the atlas. Display a list of matches in text or in a
// Windows dialog. Implements the -N switch and "Lookup City" button.

flag DisplayAtlasLookup(CONST char *szIn, size_t lDialog, int *piae)
{
  AtlasEntry *pae;
  char szCity[cchSzMax], sz[cchSzMax], *pch1, *pch2, *pch;
  int rgiae[ilistMax], rgn[ilistMax], ilistHi,
    clist = 0, icn, istateUS, istateCA, iae, nPower, i, j, fSav;
  flag fTimezoneChanges;
  real zon;
#ifdef WIN
  HWND hdlg = (HWND)lDialog;
#endif

  if (!FEnsureAtlas())
    return fFalse;
  ilistHi = (lDialog != 0 ? *piae : (piae != NULL ? 1 :
    (us.nAtlasList > 0 ? Min(us.nAtlasList, ilistMax) : ilistMax)));

  // Parse city, along with comma separated state/province and country/region.
  for (pch1 = szCity, pch2 = (char *)szIn; *pch1 = *pch2; pch1++, pch2++)
    ;
  for (pch1 = szCity; *pch1 && *pch1 != ','; pch1++)
    ;
  if (*pch1)
    *pch1++ = chNull;
  while (*pch1 && *pch1 <= ' ')
    pch1++;
  for (pch2 = pch1; *pch2 && *pch2 != ','; pch2++)
    ;
  if (*pch2)
    *pch2++ = chNull;
  while (*pch2 && *pch2 <= ' ')
    pch2++;
  if (*pch2 == chNull) {
    pch2 = pch1;
    pch1 = "";
  }

  // Determine the country/region, and state/province, if any.
  icn = istateUS = istateCA = -1;
  if (*pch2) {
    for (j = 0; j < icnewMax; j++)
      if (NCompareSzI(pch2, rgcnew[j].szAbb) == 0 ||
          NCompareSzI(pch2, rgcnew[j].szNam) == 0) {
        icn = j;
        break;
      }
    if (*pch1 == chNull)
      pch1 = pch2;
  }
  // Allow "USA" in addition to just country/region abbreviation "US".
  if (icn == -1 && NCompareSzI(pch2, "USA") == 0)
    icn = icnUS;
  if (*pch1) {
    for (j = 0; j < icnusMax; j++)
      if (NCompareSzI(pch1, rgcnus[j].szAbb) == 0 ||
          NCompareSzI(pch1, rgcnus[j].szNam) == 0) {
        istateUS = j;
        break;
      }
    if (istateUS < 0) for (j = 0; j < icncaMax; j++)
      if (NCompareSzI(pch1, rgcnca[j].szAbb) == 0 ||
          NCompareSzI(pch1, rgcnca[j].szNam) == 0) {
        istateCA = j;
        break;
      }
  }

  // Loop over all cities in atlas, seeing how well they match input string.
  for (iae = 0; iae < is.cae; iae++) {
    pae = &is.rgae[iae];
    nPower = 0;
    if (NCompareSzI(szCity, pae->szNam) == 0) {
      // Exact match of entire name = 10 points.
      nPower = 10;
    } else {
      for (j = 0; pae->szNam[j]; j++)
        if (FCompareSzSubI(szCity, &pae->szNam[j]))
          break;
      // Substring match = 1 point.
      // Substring match at start and/or end of word = +1 point each.
      if (pae->szNam[j] != chNull)
        nPower = 1 + (j == 0 || pae->szNam[j-1] < 'A') +
          (pae->szNam[j + CchSz(szCity)] < 'A');
    }
    // Input has to at least be substring of city, to be any match.
    if (nPower <= 0)
      continue;
    // Ensure country/region and/or state/province matches are at top of list.
    if (icn >= 0 && icn == pae->icn)
      nPower += 100;
    if ((istateUS >= 0 && istateUS == pae->istate && pae->icn == icnUS) ||
        (istateCA >= 0 && istateCA == pae->istate && pae->icn == icnCA))
      nPower += 200;
    // Insert match in match list, in order sorted by power.
    for (i = 0; i < clist; i++)
      if (nPower > rgn[i])
        break;
    if (i >= ilistHi)
      continue;
    if (i < clist)
      for (j = Min(clist+1, ilistHi-1); j > i; j--) {
        rgiae[j] = rgiae[j-1];
        rgn[j] = rgn[j-1];
      }
    rgiae[i] = iae;
    rgn[i] = nPower;
    if (clist < ilistHi)
      clist++;
  }

  // Display header.
  fTimezoneChanges = FEnsureTimezoneChanges();
  if (lDialog != 0) {
    fSav = us.fAnsiChar; us.fAnsiChar = 2;
  } else {
    if (piae != NULL) {

      // If piae set, then just return name and coordinates of best city.
      if (clist < 1)
        return fFalse;
      i = rgiae[0];
      pae = &is.rgae[i];
      if (fTimezoneChanges) {
        zon = ZondefFromIzn(pae->izn);
        ciCore.zon = zon;
      }
      ciCore.lon = pae->lon;
      ciCore.lat = pae->lat;
      ciCore.loc = SzPersist(SzCity(rgiae[0]));
      *piae = i;
      return fTrue;
    }
    AnsiColor(kWhite);
    sprintf(sz, "Cities in atlas matching: %s\n", szIn); PrintSz(sz);
    AnsiColor(kDefault);
  }

  // Display list of matches.
  for (i = 0; i < clist; i++) {
    if (lDialog == 0) {
      sprintf(sz, "%3d: ", i+1);
      PrintSz(sz);
    }
    pae = &is.rgae[rgiae[i]];
    pch = SzLocation(pae->lon, pae->lat);
    while (*pch == ' ')
      pch++;
    if (fTimezoneChanges) {
      zon = ZondefFromIzn(pae->izn);
      sprintf(sz, "%s (%s, %s)", SzCity(rgiae[i]), pch, SzZone(zon));
    } else
      sprintf(sz, "%s (%s)", SzCity(rgiae[i]), pch);
#ifdef WIN
    if (hdlg != NULL) {
      SetListN(dlIn, sz, rgiae[i], j);
      continue;
    }
#endif
    PrintSz(sz);
    PrintL();
  }

  // It's possible no matches were found.
  if (clist <= 0) {
#ifdef WIN
    if (hdlg != NULL) {
      SetListN(dlIn, "(No matches found)", -1, j);
    } else
#endif
      PrintSz("No matches found.");
  }
  if (lDialog != 0)
    us.fAnsiChar = fSav;
  return fTrue;
}


// Given a location, display a list of cities from the atlas nearest to it.
// Display it in text or in a Windows dialog. Implements the -Nl switch.

flag DisplayAtlasNearby(real lon, real lat, size_t lDialog, int *piae,
  flag fAstroGraph)
{
  AtlasEntry *pae;
  char sz[cchSzMax], *pch;
  int rgiae[ilistMax], rgn[ilistMax], ilistHi, clist = 0, iae, nDist,
    i, j, fSav;
  flag fTimezoneChanges;
  real rDist, zon;
#ifdef WIN
  HWND hdlg = (HWND)lDialog;
#endif

  if (!FEnsureAtlas())
    return fFalse;
  ilistHi = (lDialog != 0 ? *piae : (piae != NULL ? 1 :
    (us.nAtlasList > 0 ? Min(us.nAtlasList, ilistMax) : ilistMax)));

  // Loop over all cities in atlas, computing their distance to location.
  for (iae = 0; iae < is.cae; iae++) {
    pae = &is.rgae[iae];
    rDist = SphDistance(lon, lat, pae->lon, pae->lat) / 360.0 *
      (us.fEuroDist ? 40075.0 : 24901.0);
    nDist = (int)rDist;
    for (i = 0; i < clist; i++)
      if (nDist < rgn[i])
        break;
    if (i >= ilistHi)
      continue;
    // Insert city in list, in order sorted by nearness.
    if (i < clist)
      for (j = Min(clist+1, ilistHi-1); j > i; j--) {
        rgiae[j] = rgiae[j-1];
        rgn[j] = rgn[j-1];
      }
    rgiae[i] = iae;
    rgn[i] = nDist;
    if (clist < ilistHi)
      clist++;
  }

  // Display header.
  if (lDialog != 0) {
    fSav = us.fAnsiChar; us.fAnsiChar = 2;
  } else {
    if (piae != NULL) {
      // If piae set, then just return index of nearest city.
      if (clist <= 0)
        return fFalse;
      *piae = rgiae[0];
    }
    if (!fAstroGraph) {
      AnsiColor(kWhite);
      sprintf(sz, "Cities in atlas nearest: %s\n", SzLocation(lon, lat));
      PrintSz(sz);
    }
    AnsiColor(kDefault);
  }

  // Display list of cities.
  fTimezoneChanges = !fAstroGraph && FEnsureTimezoneChanges();
  for (i = 0; i < clist; i++) {
    if (fAstroGraph && rgn[i] >= us.nAstroGraphDist)
      break;
    if (lDialog == 0) {
      sprintf(sz, "%3d: ", i+1);
      PrintSz(sz);
    }
    pae = &is.rgae[rgiae[i]];
    pch = SzLocation(pae->lon, pae->lat);
    while (*pch == ' ')
      pch++;
    if (fTimezoneChanges) {
      zon = ZondefFromIzn(pae->izn);
      sprintf(sz, "%d %s: %s (%s, %s)", rgn[i], us.fEuroDist ? "km" : "mi",
        SzCity(rgiae[i]), pch, SzZone(zon));
    } else
      sprintf(sz, "%d %s: %s (%s)", rgn[i], us.fEuroDist ? "km" : "mi",
        SzCity(rgiae[i]), pch);
#ifdef WIN
    if (hdlg != NULL) {
      SetListN(dlIn, sz, rgiae[i], j);
      continue;
    }
#endif
    PrintSz(sz);
    PrintL();
  }

  if (lDialog != 0)
    us.fAnsiChar = fSav;
  return fTrue;
}


// Sanitize a time, in which the individual parameters may be out of range.
// For example, 25:00 on 32 Dec 2020 gets converted to 1:00 on 2 Jan 2021.

void AdjustTime(int *mon, int *day, int *yea, int *tim)
{
  int cd;

  // Move time within range, adjusting day if need be.
  while (*tim < 0) {
    *tim += 3600*24; *day -= 1;
  }
  while (*tim >= 3600*24) {
    *tim -= 3600*24; *day += 1;
  }

  // Move day within range, adjusting month if need be.
  while (*day < 1) {
    *mon -= 1;
    while (*mon < 1) {
      *mon += cSign;
      *yea -= 1;
    }
    *day += DayInMonth(*mon, *yea);
  }
  loop {
    cd = DayInMonth(*mon, *yea);
    if (*day <= cd)
      break;
    *mon += 1;
    while (*mon > cSign) {
      *mon -= cSign;
      *yea += 1;
    }
    *day -= cd;
  }
}


#define ichngMax 4
#define RTim(tim) ((real)(tim) / 3600.0)

// Set the Daylight Time and time zone values in a chart, since the period
// they apply to has been determined. However, if near the endpoints of the
// period, make sure the time isn't actually ambiguous or invalid.

flag FSetDstZon(CI *ci, int izn,
  int mon, int day, int yea, int tim, int zon, int doff,
  int monPrev, int dayPrev, int yeaPrev, int timPrev, int dstPrev,
    int zonPrev, int doffPrev)
{
  char sz[cchSzMax*2];

  if (!(yea > ci->yea || (yea == ci->yea && (mon > ci->mon ||
    (mon == ci->mon && (day > ci->day ||
    (day == ci->day && RTim(tim) > ci->tim)))))))
    return fFalse;

  if (ci->yea == yea && ci->mon == mon && ci->day == day &&
    RTim(tim) - ci->tim <= RTim(NAbs(doff)) && doff < 0) {
    sprintf(sz, "Unknown whether Daylight Time is in effect!\n"
      "On %s at %s local time in zone %s (%s), "
      "clocks 'fall back' by %s hour.\n"
      "Clock times within that period are ambiguous, "
      "since they're repeated after falling back.\n",
      SzDate(mon, day, yea, fFalse),
      SzTime(tim / 3600, tim / 60 % 60, us.fSeconds ? tim % 60 : -1),
      rgszzn[izn], SzZone(RTim(zon)), SzHMS(doff));
    PrintWarning(sz);
  }
  if (ci->yea == yeaPrev && ci->mon == monPrev && ci->day == dayPrev &&
    ci->tim - RTim(timPrev) < RTim(doffPrev) && doffPrev > 0) {
    sprintf(sz, "Unknown whether Daylight Time is in effect!\n"
      "On %s at %s local time in zone %s (%s),"
      "clocks 'spring forward' by %s hour.\n"
      "Clock times within the gap are invalid, "
      "since they're skipped when springing forward.\n",
      SzDate(monPrev, dayPrev, yeaPrev, fFalse),
      SzTime(timPrev / 3600, timPrev / 60 % 60,
        us.fSeconds ? timPrev % 60 : -1),
      rgszzn[izn], SzZone(RTim(zon)), SzHMS(doffPrev));
    PrintWarning(sz);
  }

  ci->dst = RTim(dstPrev);
  ci->zon = RTim(zonPrev != nLarge ? zonPrev : zon);
  return fTrue;
}


// Given a time zone area, display a list of time zone and Daylight Saving
// time changes within it. Display it in text or within a Windows dialog.
// Implements the -Nz switch. Can also do the important task of determining
// the time zone and Daylight Time setting for a particular time.

flag DisplayTimezoneChanges(int iznIn, size_t lDialog, CI *ci)
{
  char sz[cchSzMax*2], sz1[cchSzMax], sz2[cchSzDef], sz3[cchSzDef];
  int rgmon[ichngMax], rgday[ichngMax], rgtim[ichngMax], rgiru[ichngMax],
    izn, izcn, izce, czce,
    mon, day, yea, tim, dst, zon, off, doff,
    monPrev, dayPrev, yeaPrev, timPrev, dstPrev, zonPrev, offPrev, doffPrev,
    iyea, yea2, irue, crue, cn, ici, idMon, dd, i, j, k;
  ZoneChange *pzc, *pzc2;
  RuleEntry *pru;
#ifdef WIN
  HWND hdlg = (HWND)lDialog;
#endif

  if (!FEnsureTimezoneChanges())
    return fFalse;

  // Loop over all possible time zone areas, or just the one specified.
  for (izn = (iznIn < 0 ? 0 : iznIn); izn < (iznIn < 0 ? iznMax : iznIn+1);
    izn++) {

  // Display header for this time zone area.
  if (iznIn < 0 && izn > 0)
    PrintL();
  izcn = mpznzc[izn];
  if (lDialog == 0) {
    if (ci == NULL) {
      AnsiColor(kWhiteA);
      sprintf(sz, "Time changes within zone: %s", rgszzn[izn]); PrintSz(sz);
      i = rgznChange[izcn];
      if (i != izn) {
        sprintf(sz, " (same as %s)", rgszzn[i]); PrintSz(sz);
      }
      PrintL();
      AnsiColor(kDefault);
    }
  }
#ifdef WIN
  else {
    sprintf(sz, "Time changes within zone: %s", rgszzn[izn]);
    SetListN(dlIn, sz, -1, k);
  }
#endif
  izce = rgizcChange[izcn];
  czce = rgizcChange[izcn+1] - izce;
  cn = off = doff = 0;
  mon = day = yea = tim = dst = zon = nLarge;

  // Loop over all time zone change entries within this time zone area.
  for (i = 0; i < czce-1; i++) {
    monPrev = mon; dayPrev = day; yeaPrev = yea; timPrev = tim;
    dstPrev = dst; zonPrev = zon;
    offPrev = off; doffPrev = doff;
    pzc = &is.rgzc[izce + i];
    pzc2 = &pzc[1];
    mon = pzc->mon; day = pzc->day; yea = pzc->yea; tim = pzc->tim;
    dst = pzc2->dst < nLarge ? pzc2->dst : 0;
    zon = pzc2->zon;
    if (pzc->timtype == 1)
      tim += dst;
    else if (pzc->timtype == 2)
      tim -= (zon - dst);
    AdjustTime(&mon, &day, &yea, &tim);
    if (cn <= 0 && zon % (60*15))
      continue;
    off = zon - dst;
    doff = offPrev - off;
    if (doff == 0 && dst == dstPrev && zon == zonPrev)
      goto LSkip;
    cn++;
    if (ci != NULL && lDialog == 0) {
      // If ci set, then just set correct time zone and Daylight offset.
      // If ci date before any time zones were defined, then default to LMT.
      if (cn <= 1 && (yea > ci->yea || (yea == ci->yea && (mon > ci->mon ||
        (mon == ci->mon && (day > ci->day ||
        (day == ci->day && RTim(tim) > ci->tim))))))) {
        ci->dst = 0.0; ci->zon = zonLMT;
        return fTrue;
      }
      if (FSetDstZon(ci, izn, mon, day, yea, tim, zon, doff,
        monPrev, dayPrev, yeaPrev, timPrev, dstPrev, zonPrev, doffPrev))
        return fTrue;
      goto LSkip;
    }

    // Display when the time zone offset changes.
    if (cn > 1)
      sprintf(sz1, "Change clock %s to", SzHMS(doff));
    else
      sprintf(sz1, "Define clock to be");
    sz2[0] = sz3[0] = chNull;
    if (dst != 0 && (dst != 3600 || us.fSeconds))
      sprintf(sz2, " %s", SzHMS(dst));
    if (zon != zonPrev)
      sprintf(sz3, " & Zone %s", SzZone(RTim(zon)));
    sprintf(sz, "%s %s: %s: %s%s%s", SzDate(mon, day, yea, fFalse),
      SzTime(tim / 3600, tim / 60 % 60, us.fSeconds ? tim % 60 : -1),
      sz1, dst == 0 ? "ST" : "DT", sz2, sz3);
#ifdef WIN
    if (lDialog != 0) {
      sprintf(sz1, "%.3s %s", szDay[DayOfWeek(mon, day, yea)], sz);
      SetListN(dlIn, sz1, -1, k);
      goto LSkip;
    }
#endif
    sprintf(sz1, "%3d: ", cn); PrintSz(sz1);
    k = DayOfWeek(mon, day, yea);
    AnsiColor(kRainbowA[k + 1]);
    sprintf(sz2, "%.3s ", szDay[k]); PrintSz(sz2);
    AnsiColor(kWhiteA);
    PrintSz(sz);
    PrintL();

LSkip:
    if (pzc2->irun < 0)
      continue;

    // A rule applies to this time change entry. Use it to check for changes.
    Assert(pzc2->irun < is.crun);
    irue = is.rgrun[pzc2->irun].irue;
    crue = is.rgrun[pzc2->irun + 1].irue - irue;
    yea2 = Min(pzc2->yea, Max(2025, Min(2080,
      (ci != NULL ? ci->yea : Yea) + 5)));

    // Loop over each year within the time change entry.
    for (iyea = pzc->yea; iyea <= yea2; iyea++) {
      ici = 0;

      // Loop over each entry within rule, checking if it applies this year.
      for (j = 0; j < crue; j++) {
        pru = &is.rgrue[irue + j];
        if (!FBetween(iyea, pru->yea1, pru->yea2))
          continue;
        if (ici >= ichngMax) {
          sprintf(sz, "Zone rule warning: "
            "Too many changes in rule %s within year %d.\n",
            is.rgrun[pzc2->irun].szNam, iyea);
          PrintWarning(sz);
          break;
        }
        yea = iyea;
        mon = pru->mon;
        if (pru->daytype <= 0) {
          day = pru->daynum;
        } else if (pru->daytype == 1) {
          idMon = DayInMonth(mon, yea);
          dd = DayOfWeek(mon, idMon, yea) - pru->dayweek;
          if (dd < 0)
            dd += cWeek;
          day = idMon - dd;
        } else {
          idMon = pru->daynum;
          dd = DayOfWeek(mon, idMon, yea);
          if (pru->daytype == 2) {
            dd = pru->dayweek - dd;
            if (dd < 0)
              dd += cWeek;
            day = idMon + dd;
          } else {
            dd = dd - pru->dayweek;
            if (dd < 0)
              dd += cWeek;
            day = idMon - dd;
          }
        }
        tim = pru->tim;
        k = yea;
        AdjustTime(&mon, &day, &k, &tim);
        // Rule doesn't apply if it's outside ZoneChange endpoint times.
        if (yea == pzc->yea && (mon < pzc->mon || (mon == pzc->mon &&
          (day < pzc->day || (day == pzc->day && tim < pzc->tim)))))
          continue;
        if (yea == pzc2->yea && (mon > pzc2->mon || (mon == pzc2->mon &&
          (day > pzc2->day || (day == pzc2->day && tim >= pzc2->tim)))))
          continue;
        rgmon[ici] = mon; rgday[ici] = day; rgtim[ici] = tim;
        rgiru[ici] = irue + j;
        ici++;
      }

      // Sort the list of changes found in order by date/time.
      for (j = 1; j < ici; j++) {
        k = j-1;
        while (k >= 0 && (rgmon[k] > rgmon[j] ||
          (rgmon[k] == rgmon[j] && (rgday[k] > rgday[j] ||
          (rgday[k] == rgday[j] && rgtim[k] > rgtim[j]))))) {
          SwapN(rgmon[k], rgmon[j]);
          SwapN(rgday[k], rgday[j]);
          SwapN(rgtim[k], rgtim[j]);
          SwapN(rgiru[k], rgiru[j]);
          k--;
        }
      }

      // Loop over the list of Daylight Saving changes found.
      for (j = 0; j < ici; j++) {
        monPrev = mon; dayPrev = day; yeaPrev = yea; timPrev = tim;
        dstPrev = dst; zonPrev = zon;
        offPrev = off; doffPrev = doff;
        pru = &is.rgrue[rgiru[j]];
        mon = rgmon[j]; day = rgday[j]; tim = rgtim[j];
        dst = pru->dst;
        off = zon - dst;
        if (pru->timtype == 1)
          tim += dstPrev;
        else if (pru->timtype == 2)
          tim -= (zon - dstPrev);
        AdjustTime(&mon, &day, &yea, &tim);
        doff = offPrev - off;
        if (ci != NULL && lDialog == 0) {
          // If ci set, then just set correct time zone and Daylight offset.
          if (FSetDstZon(ci, izn, mon, day, yea, tim, zon, doff,
            monPrev, dayPrev, yeaPrev, timPrev, dstPrev, zonPrev, doffPrev))
            return fTrue;
          continue;
        }
        if (doff == 0)
          continue;

        // Display when the Daylight Saving offset changes.
        cn++;
        sz2[0] = chNull;
        if (dst != 0 && (dst != 3600 || us.fSeconds))
          sprintf(sz2, " %s", SzHMS(dst));
        sprintf(sz1, "%s %s: Change clock %s to: %s%s",
          SzDate(mon, day, yea, fFalse),
          SzTime(tim / 3600, tim / 60 % 60, us.fSeconds ? tim % 60 : -1),
          SzHMS(doff),
          dst == 0 ? "ST" : "DT", sz2);
#ifdef WIN
        if (lDialog != 0) {
          sprintf(sz, "%.3s %s", szDay[DayOfWeek(mon, day, yea)], sz1);
          SetListN(dlIn, sz, -1, k);
          continue;
        }
#endif
        AnsiColor(kDefault);
        sprintf(sz, "%3d: ", cn); PrintSz(sz);
        k = DayOfWeek(mon, day, yea);
        AnsiColor(kRainbowA[k + 1]);
        sprintf(sz, "%.3s ", szDay[k]); PrintSz(sz);
        AnsiColor(dst == 0 ? kDkGray : kLtGray);
        PrintSz(sz1);
        PrintL();
      }
    }
  } // i
  } // izn

  // If ci date after all time zones are defined, then go with final values.
  if (ci != NULL && lDialog == 0) {
    ci->dst = RTim(dst); ci->zon = RTim(zon);
  }
  return fTrue;
}


// Return the default time zone for a time zone area. That means the latest
// offset in the zone change list corresponding to this time zone area.

real ZondefFromIzn(int izn)
{
  int izc, zon;

  izc = rgizcChange[mpznzc[izn] + 1] - 1;
  zon = is.rgzc[izc].zon;
  return RTim(zon);
}
#endif // ATLAS

/* atlas.cpp */
