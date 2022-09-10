# Astrolog (Version 7.50) File: Makefile (Unix version)
#
# IMPORTANT NOTICE: Astrolog and all chart display routines and anything
# not enumerated elsewhere in this program are Copyright (C) 1991-2022 by
# Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
# Permission is granted to freely use, modify, and distribute these
# routines provided these credits and notices remain unmodified with any
# altered or distributed versions of the program.
#
# First created 11/21/1991.
#
# This Makefile is included only for convenience. One could easily compile
# Astrolog on most Unix systems by hand with the command:
# % cc -c -O *.cpp; cc -o astrolog *.o -lm -lX11
# Generally, all that needs to be done to compile once astrolog.h has been
# edited, is compile each source file, and link them together with the math
# library, and if applicable, the main X library.
#
NAME = astrolog
OBJS = src/atlas.o \
       src/calc.o \
       src/charts0.o \
       src/charts1.o \
       src/charts2.o \
       src/charts3.o \
       src/data.o \
       src/express.o \
       src/general.o \
       src/intrpret.o \
       src/io.o \
       src/matrix.o \
       src/placalc.o \
       src/placalc2.o \
       src/xdata.o \
       src/xgeneral.o \
       src/xdevice.o \
       src/xcharts0.o \
       src/xcharts1.o \
       src/xcharts2.o \
       src/xscreen.o \
       src/swecl.o \
       src/swedate.o \
       src/swehouse.o \
       src/swejpl.o \
       src/swemmoon.o \
       src/swemplan.o \
       src/sweph.o \
       src/swephlib.o \
       src/astrolog.o

# If you don't have X windows, delete the "-lX11" part from the line below:
# If not compiling with GNUC, delete the "-ldl" part from the line below:
LIBS = -lm -lX11 -ldl -s
CPPFLAGS = -O -Wno-write-strings -Wno-narrowing -Wno-comment
RM = rm -f

$(NAME): $(OBJS)
	cc -o $(NAME) $(OBJS) $(LIBS)

clean:
	$(RM) $(OBJS) $(NAME)
#
