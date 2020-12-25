## Process this file with automake to produce Makefile.in

CLEANFILES=*~ *.bak core *.core *.o .depend .deps/* 

INCLUDES = @GTK_CFLAGS@ @GDK_IMLIB_CFLAGS@ -I../ -I../libs/ -DXFCE_DIR=\"@XFCE_DIR@\" \
	-DXFCE_LOCALE_DIR=\"@XFCE_LOCALE_DIR@\"

bin_PROGRAMS = xfmouse

xfmouse_DEPENDENCIES = ../libs/libxfce.a

xfmouse_SOURCES = \
	xfmouse_cb.c xfmouse_cb.h \
	xfmouse.c xfmouse.h \
	xfmouse.xpm

xfmouse_LDADD = ../libs/libxfce.a @GTK_LIBS@ @GDK_IMLIB_LIBS@ @INTLLIBS@ @LIBS@ \
	${X_LIBS} ${X_PRE_LIBS} -lXpm -lm

man_MANS=xfmouse.1
