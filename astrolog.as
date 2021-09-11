@AD730  ; Astrolog 7.30 default settings file astrolog.as

-z 8:00W                ; Default time zone     [hours W or E of UTC   ]
-z0 Autodetect          ; Default Daylight time [0 standard, 1 daylight]
-zl 122W19:59 47N36:35  ; Default location      [longitude and latitude]
-zv 167ft               ; Default elevation     [in feet or meters     ]
-zj "Current moment now" "Seattle, WA, USA" ; Default name and location

-Yz 0   ; Time minute addition to be used if "now" charts are offset.
;-n      ; Comment out this line to not start with chart for "now".

_s      ; Zodiac selection          ["_s" is tropical, "=s" is sidereal]
:s 0.0  ; Zodiac offset value       [Change "0.0" to desired ayanamsa  ]
:sz     ; Zodiac display format     ["z" is sign, "d" is 0-360 deg, etc]
-A 5    ; Number of aspects         [Change "5" to desired number      ]
-c Plac ; House system              [Change "Plac" to desired system   ]
_c3     ; 3D house boundaries       ["=c3" is 3D houses, "_c3" is 2D   ]
_k      ; Ansi color text           ["=k" is color, "_k" is monochrome ]
:d 48   ; Searching divisions       [Change "48" to desired divisions  ]
_b0     ; Print zodiac seconds      ["_b0" to minute, "=b0" to second  ]
=b      ; Use ephemeris files       ["=b" uses them, "_b" doesn't      ]
:w 0    ; Wheel chart text rows     [Change "0" to desired wheel rows  ]
:I 80   ; Text screen columns       [Change "80" to desired columns    ]
-YQ 0   ; Text screen scroll limit  [Change "24" or set to "0" for none]
_Yn     ; Which Nodes and Lilith    ["_Yn" shows mean, "=Yn" shows true]
_Yd     ; European date format      ["_Yd" is MDY, "=Yd" is DMY        ]
_Yt     ; European time format      ["_Yt" is AM/PM, "=Yt" is 24 hour  ]
_Yv     ; European length units     ["_Yv" is imperial, "=Yv" is metric]
_Yr     ; Show rounded positions    ["=Yr" rounds, "_Yr" doesn't       ]
=YC     ; Smart cusp displays       ["=YC" is smart, "_YC" is normal   ]
=YO     ; Smart copy and printing   ["=YO" does it smart, "_YO" doesn't]
=Y8     ; Clip text to end of line  ["=Y8" clips, "_Y8" doesn't clip   ]
-YP 0   ; Arabic part formula       ["1" is fixed, "0" checks if night ]
=Yu0    ; Show eclipse information  ["=Yu0" shows, "_Yu0" doesn't show ]
=0n     ; Internet Web queries      ["=0n" disables them, "_0n" allows ]


; FILE PATHS (-Yi1 through -Yi9):
; For example, point -Yi1 to ephemeris dir, -Yi2 to chart files dir, etc.

-Yi1 "C:\ASTROLOG\EPH"
-Yi2 "C:\ASTROLOG\DAT"


; DEFAULT RESTRICTIONS:
;  0-10: Ear Sun Moo Mer Ven Mar Jup Sat Ura Nep Plu
; 11-21: Chi Cer Pal Jun Ves Nor Sou Lil For Ver EP
; 22-33: Asc 2nd 3rd Nad 5th 6th Des 8th 9th MC 11th 12th
; 34-42: Vul Cup Had Zeu Kro Apo Adm Vulk Pos
; 43-51: Hyg Pho Eri Hau Mak Gon Qua Sed Orc
; 52-83: Planetary moons
; 84-130: Fixed stars

-YR 0 10     1 0 0 0 0 0 0 0 0 0 0    ; Planets
-YR 11 21    1 1 1 1 1 0 1 1 1 1 1    ; Minor planets
-YR 22 33    0 1 1 1 1 1 1 1 1 0 1 1  ; House cusps
-YR 34 42    1 1 1 1 1 1 1 1 1        ; Uranians
-YR 43 51    1 1 1 1 1 1 1 1 1        ; Dwarfs

; DEFAULT TRANSIT RESTRICTIONS:

-YRT 0 10    1 0 1 0 0 0 0 0 0 0 0    ; Planets
-YRT 11 21   1 1 1 1 1 0 1 1 1 1 1    ; Minor planets
-YRT 22 33   1 1 1 1 1 1 1 1 1 1 1 1  ; House cusps
-YRT 34 42   1 1 1 1 1 1 1 1 1        ; Uranians
-YRT 43 51   1 1 1 1 1 1 1 1 1        ; Dwarfs

-YR0 0 0  ; Restrict sign changes, direction changes
-YR1 1 1  ; Restrict latitude direction changes, distance direction changes

-YR7 0 1 1 0 1  ; Restrict rulerships: std, esoteric, hierarch, exalt, ray


; DEFAULT ASPECT ORBS:
;  1- 5: Con Opp Squ Tri Sex
;  6-11: Inc SSx SSq Ses Qui BQn
; 12-18: SQn Sep Nov BNv BSp TSp QNv

-YAo 1 5     7 7 7 7 6      ; Major aspects
-YAo 6 11    3 3 3 3 2 2    ; Minor aspects
-YAo 12 18   1 1 1 1 1 1 1  ; Obscure aspects

; DEFAULT MAX PLANET ASPECT ORBS:

-YAm 0 10    360 360 360 360 360 360 360 360 360 360 360      ; Planets
-YAm 11 21   360 360 360 360 360   2   2   2 360 360   2      ; Minor planets
-YAm 22 33   360 360 360 360 360 360 360 360 360 360 360 360  ; Cusp objects
-YAm 34 42   360 360 360 360 360 360 360 360 360              ; Uranians
-YAm 43 51   360 360 360 360 360 360 360 360 360              ; Dwarfs
-YAm 84 84     2                                              ; Fixed stars

; DEFAULT PLANET ASPECT ORB ADDITIONS:

-YAd 0 10    1 1 1 0 0 0 0 0 0 0 0    ; Planets
-YAd 11 21   0 0 0 0 0 0 0 0 0 0 0    ; Minor planets
-YAd 22 33   0 0 0 0 0 0 0 0 0 0 0 0  ; Cusp objects
-YAd 34 42   0 0 0 0 0 0 0 0 0        ; Uranians
-YAd 43 51   0 0 0 0 0 0 0 0 0        ; Dwarfs
-YAd 84 84   0                        ; Fixed stars


; DEFAULT INFLUENCES:

-Yj 0 10    30 30 25 10 10 10 10 10  8  8  8     ; Planets
-Yj 11 21    6  5  5  5  5  5  5  4  4  4  4     ; Minor planets
-Yj 22 33   20 10 10 10 10 10 10 10 10 15 10 10  ; Cusp objects
-Yj 34 42    4  3  3  3  3  3  3  3  3           ; Uranians
-Yj 43 51    3  3  3  3  3  3  3  3  3           ; Dwarfs
-Yj 84 84    2                                   ; Fixed stars

-YjC 1 12   20 0 0 10 0 0 5 0 0 15 0 0  ; Houses

-YjA 1 5    1.0 0.8 0.8 0.6 0.6          ; Major aspects
-YjA 6 11   0.4 0.4 0.2 0.2 0.2 0.2      ; Minor aspects
-YjA 12 18  0.1 0.1 0.1 0.1 0.1 0.1 0.1  ; Obscure aspects

; DEFAULT TRANSIT INFLUENCES:

-YjT 0 10   10 10  4  8  9 20 30 35 40 45 50  ; Planets
-YjT 11 21  30 15 15 15 15 30 30  1  1  1  1  ; Minor planets
-YjT 34 42   2 50 50 50 50 50 50 50 50        ; Uranians
-YjT 43 51  15 30 50 50 50 50 50 50 50        ; Dwarfs
-YjT 84 84  60                                ; Fixed stars

-Yj0 20 10 15 5  ; In ruling sign, exalted sign, ruling house, exalted house
-Yj7 10 10 10 5 5 5  ; In Esoteric, Hierarchical, Ray ruling (signs, houses)


; DEFAULT RAYS:

-Y7C 1 12   17 4 2 37 15 26 3 4 456 137 5 26  ; Signs
-Y7O 0 10   3 2 4 4 5 6 2 3 7 6 1             ; Planets
-Y7O 34 42  1 0 0 0 0 0 0 0 0                 ; Uranians
-Y7O 43 51  0 0 3 2 1 4 7 6 5                 ; Dwarfs


; DEFAULT COLORS:
; Black, White, Gray, LtGray, Red, Orange, Yellow, Green, Cyan, Blue, Purple,
; Magenta, Maroon, DkGreen, DkCyan, DkBlue; Element, Ray, Star, Planet

-YkO 0 10   Yel Ele Ele Ele Ele Ele Ele Ele Ele Ele Ele      ; Planet colors
-YkO 11 21  Mag Mag Mag Mag Mag DkC DkC DkC DkC DkC DkC      ; Minor colors
-YkO 22 33  Ele Ele Ele Ele Ele Ele Ele Ele Ele Ele Ele Ele  ; Cusp colors
-YkO 34 42  Pur Pur Pur Pur Pur Pur Pur Pur Pur              ; Uranian colors
-YkO 43 51  Mag Mag Pur Pur Pur Pur Pur Pur Pur              ; Dwarf colors
-YkO 52 63  Pla Pla Pla Pla Pla Pla Pla Pla Pla Pla Pla Pla  ; Moons
-YkO 64 75  Pla Pla Pla Pla Pla Pla Pla Pla Pla Pla Pla Pla  ; Moons
-YkO 76 83  Pla Pla Pla Pla Pla Pla Pla Pla                  ; Moons
-YkO 84 84  Sta                                              ; Fixed stars

-YkA 1 5    Yel Blu Red Gre Cya          ; Major aspect colors
-YkA 6 11   Mag Mag Ora Ora DkC DkC      ; Minor aspect colors
-YkA 12 18  DkC Mar Pur Pur Mar Mar Pur  ; Obscure aspect colors

-YkC        Red Yel Gre Blu                      ; Element colors
-Yk7 1 7    Red Blu Gre Yel Ora Mag Pur          ; Ray colors
-Yk0 1 7    Red Ora Yel Gre Cya Blu Pur          ; Rainbow colors
-Yk  0 8    Bla Whi LtG Gra Mar DkG DkC DkB Mag  ; Main colors


; GRAPHICS DEFAULTS:

=Xm              ; Color charts       ["=Xm" is color, "_Xm" is monochrome]
_Xr              ; Reverse background ["_Xr" is black, "=Xr" is white     ]
:Xw 600 600      ; Default X and Y resolution (not including sidebar)
:Xs 200          ; Character scale     [100-400]
:XS 100          ; Graphics text scale [100-400]
=XQ              ; Square charts ["=XQ" forces square, "_XQ" allows rectangle]
=Xu              ; Chart border  ["=Xu" shows border, "_Xu" doesn't show     ]
:Xv 1            ; Wheel fill    ["0" for none, "1" for standard, "2" rainbow]
_Xx              ; Thicker lines ["=Xx" is thicker, "_Xx" is thinner         ]
:Xbw             ; Bitmap file type   ["Xbw" is Windows .bmp, "Xbn" is X11   ]
:YXG 1111        ; Glyph selections   [Capricorn, Uranus, Pluto, Lilith]
:YXg 0           ; Aspect grid cells  ["0" for autodetect  ]
:YXS 0.0         ; Orbit radius in AU ["0.0" for autodetect]
:YXj 0           ; Orbit trail count
:YX7 600         ; Esoteric ray column influence width
:YXf 00000       ; System fonts to use [text, signs, houses, planets, aspects]
:YXp 0           ; PostScript paper orientation ["-1" portrait, "1" landscape]
:YXp0 8.5in 11in ; PostScript paper X and Y sizes

_X               ; Graphics chart display ["_X" is text, "=X" is graphics]

; astrolog.as
