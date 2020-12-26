## Process this file with automake to produce Makefile.in

CLEANFILES=*~ *.bak core *.core *.o .depend .deps/* 

INCLUDES = @GTK_CFLAGS@ @GDK_IMLIB_CFLAGS@ -I../ -I../libs/ @X_CFLAGS@ \
	-DXFCE_DIR=\"@XFCE_DIR@\" -DXFCE_LOCALE_DIR=\"@XFCE_LOCALE_DIR@\"

bin_PROGRAMS = xfbd

xfbd_DEPENDENCIES = ../libs/libxfce.a

xfbd_SOURCES = \
	xfbd.c xfbd.h \
	xfbd_cb.c xfbd_cb.h
	
xfbd_LDADD = ../libs/libxfce.a @GTK_LIBS@ @GDK_IMLIB_LIBS@ @INTLLIBS@ @LIBS@ \
	${X_LIBS} ${X_PRE_LIBS} -lXpm -lm

man_MANS=xfbd.1
