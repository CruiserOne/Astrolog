$ ! Astrolog (Version 7.30) File: makefile.com (VMS version)
$ !
$ ! IMPORTANT NOTICE: Astrolog and all chart display routines and anything
$ ! not enumerated elsewhere in this program are Copyright (C) 1991-2020 by
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
	ASTROLOG.obj, -
	ATLAS.obj, -
	CALC.obj, -
	CHARTS0.obj, -
	CHARTS1.obj, -
	CHARTS2.obj, -
	CHARTS3.obj, -
	DATA.obj, -
	GENERAL.obj, -
	INTRPRET.obj, -
	IO.obj, -
	MATRIX.obj, -
	PLACALC.obj, -
	PLACALC2.obj, -
	XCHARTS0.obj, -
	XCHARTS1.obj, -
	XCHARTS2.obj, -
	XDATA.obj, -
	XDEVICE.obj, -
	XGENERAL.obj, -
	XSCREEN.obj, -
	SWECL.obj, -
	SWEDATE.obj, -
	SWEHOUSE.obj, -
	SWEJPL.obj, -
	SWEMMOON.obj, -
	SWEMPLAN.obj, -
	SWEPH.obj, -
	SWEPHLIB.obj, -
	sys$input/opt
sys$share:decw$xlibshr/share
$ set nover
$ exit
