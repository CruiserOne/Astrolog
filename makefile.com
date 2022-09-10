$ ! Astrolog (Version 7.50) File: makefile.com (VMS version)
$ !
$ ! IMPORTANT NOTICE: Astrolog and all chart display routines and anything
$ ! not enumerated elsewhere in this program are Copyright (C) 1991-2021 by
$ ! Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
$ ! Permission is granted to freely use, modify, and distribute these
$ ! routines provided these credits and notices remain unmodified with any
$ ! altered or distributed versions of the program.
$ !
$ ! This script originally created by Max Calvani.
$ !
$ set ver
$ define X11 decw$include
$ define lnk$library sys$library:vaxcrtl
$ CC ASTROLOG
$ CC ATLAS
$ CC CALC
$ CC CHARTS0
$ CC CHARTS1
$ CC CHARTS2
$ CC CHARTS3
$ CC DATA
$ CC EXPRESS
$ CC GENERAL
$ CC INTRPRET
$ CC IO
$ CC MATRIX
$ CC PLACALC
$ CC PLACALC2
$ CC XCHARTS0
$ CC XCHARTS1
$ CC XCHARTS2
$ CC XDATA
$ CC XDEVICE
$ CC XGENERAL
$ CC XSCREEN
$ CC SWECL
$ CC SWEDATE
$ CC SWEHOUSE
$ CC SWEJPL
$ CC SWEMMOON
$ CC SWEMPLAN
$ CC SWEPH
$ CC SWEPHLIB
$ link/exe=astrolog.exe -
	SRC\ASTROLOG.obj, -
	SRC\ATLAS.obj, -
	SRC\CALC.obj, -
	SRC\CHARTS0.obj, -
	SRC\CHARTS1.obj, -
	SRC\CHARTS2.obj, -
	SRC\CHARTS3.obj, -
	SRC\DATA.obj, -
	SRC\GENERAL.obj, -
	SRC\INTRPRET.obj, -
	SRC\IO.obj, -
	SRC\MATRIX.obj, -
	SRC\PLACALC.obj, -
	SRC\PLACALC2.obj, -
	SRC\XCHARTS0.obj, -
	SRC\XCHARTS1.obj, -
	SRC\XCHARTS2.obj, -
	SRC\XDATA.obj, -
	SRC\XDEVICE.obj, -
	SRC\XGENERAL.obj, -
	SRC\XSCREEN.obj, -
	SRC\SWECL.obj, -
	SRC\SWEDATE.obj, -
	SRC\SWEHOUSE.obj, -
	SRC\SWEJPL.obj, -
	SRC\SWEMMOON.obj, -
	SRC\SWEMPLAN.obj, -
	SRC\SWEPH.obj, -
	SRC\SWEPHLIB.obj, -
	sys$input/opt
sys$share:decw$xlibshr/share
$ set nover
$ exit
