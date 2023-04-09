@AS760
; Astrolog Maze game - A script file for Astrolog 7.60
; By Walter D. Pullen, Astara@msn.com, http://www.astrolog.org/astrolog.htm

-M0 1 "-YYT 'Welcome to Astrolog Mazes! :)\nUse w/a/s/d keys to move the dot through the Maze to the finish.\n\nPress F1 to display this help text.\nPress F2 to create new Maze.\nPress F3 to create smaller Maze.\nPress F4 to create larger Maze.\nPress F5 to restart current Maze.\n'"
-M0 2 "-n ~1 'Macro 2 Macro 3'"
-M0 3 "~1 'If Gt @d 1 =d Dec @d' -M 2"
-M0 4 "~1 'If Lt @d 25 =d Inc @d' -M 2"
-M0 5 "-n ~1 '=a 1 =b 0 =k 0'"
-M0 6 "~1 '=a Dec @a If Macro 6 =a Inc @a'"
-M0 7 "~1 '=a Inc @a If Macro 6 =a Dec @a'"
-M0 8 "~1 '=b Dec @b If Macro 6 =b Inc @b'"
-M0 9 "~1 '=b Inc @b If Macro 6 =b Dec @b'"
-M0 10 "-n -n1 ~1 '=l 0'"
-M0 11 "~1 '=u Int Mul Sub JulianT @t 8640000.0 =x Div @u 100 =y Int Mod @u 100' -YYT 'Congratulations, you made it through the Maze in \X.\Y seconds! :)\n\nHave an a-Maze-ing day! :)\n'"
-M0 12 "_Wh"

~M 1 "DCol K_Green For %y 0 Dec @c For %x 0 Dec @c Do2 =v Add @h Mul @x @e =w Add @h Mul @y @e If Not Var Add 27 Add Mul @y @c @x DBlock Sub @v @f Sub @w @f Add @v Dec @f Add @w Dec @f"
~M 2 "=c Inc Mul @d 2 =g Dec Mul @d @d =t ?: @k JulianT Tim =s RndSeed Mul Fract Mul @t 10000.0 10000.0 For %y 0 Dec @c For %x 0 Dec @c = Add 27 Add Mul @y @c @x 0"
~M 3 "=a 1 =b 1 = Add 27 Add Mul @b @c @a 1 While @g Macro 4 = Inc 27 1 =a Sub @c 2 =b Dec @c Macro 6 = @z 1 =a 1 =b 0"
~M 4 "=r Rnd 0 3 =x @a =y @b IfElse Odd @r =x Add @x ?: Lte @r 1 2 -2 =y Add @y ?: Lte @r 0 2 -2 If And Tween @x 0 Dec @c Tween @y 0 Dec @c Macro 5"
~M 5 "=z Add 27 Add Mul @y @c @x If Not Var @z Do2 = @z 1 = Add 27 Add Mul >> Add @y @b 1 @c >> Add @x @a 1 1 =g Dec @g =a @x =b @y"
~M 6 "If Gte @b @c Switch 10 =z Add 27 ?: Lt @b 0 0 Add Mul @b @c @a Not Var @z"

~Q2 "=f Inc Div Dec Min _Xwx _Xwy Mul @d 12 =e Mul @f 2 =h Div Sub Min _Xwx _Xwy Mul Dec @c @e 2 DCol K_Red =v Add @h Mul @a @e =w Add @h Mul @b @e DDisk Sub @v @f Sub @w @f Add @v Dec @f Add @w Dec @f Macro 1"
~Q3 "If Not @k Do2 =k 1 Switch 1 =t JulianT If Not @l Do =l 1 Switch 11"
~XQ "If Equ @z 119 =z 208 If Equ @z 97 =z 206 If Equ @z 115 =z 209 If Equ @z 100 =z 207"
~WQ "If Equ @z 40082 =z 40152 If Equ @z 40131 =z 40150 If Equ @z 40281 =z 40153 If Equ @z 40086 =z 40151"

~1 "=d 10 =f 5 =l 1 If WIN Switch 12"
=b0 =Xe
-M 2