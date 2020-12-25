## Process this file with automake to produce Makefile.in

CLEANFILES=*~ *.bak core *.core *.o .depend .deps/* 

INCLUDES = @GTK_CFLAGS@ @GDK_IMLIB_CFLAGS@ @GDK_PIXBUF_CFLAGS@ -I../ -I../libs/ \
	-DXFCE_DIR=\"@XFCE_DIR@\" -DXFCE_LOCALE_DIR=\"@XFCE_LOCALE_DIR@\" \
	-DXFCE_CONFDIR=\"@XFCE_CONFDIR@\"

bin_PROGRAMS = xfsound
bin_SCRIPTS  = xfplay

xfsound_DEPENDENCIES = ../libs/libxfce.a ../libs/libxfwm.a

xfsound_SOURCES = \
	xfsound_cb.c xfsound_cb.h \
	xfsound.c xfsound.h \
	xfdsp.c xfdsp.h

xfsound_LDADD = ../libs/libxfce.a @GTK_LIBS@ @GDK_IMLIB_LIBS@ @GDK_PIXBUF_LIBS@ \
	@INTLLIBS@ @LIBS@ ../libs/libxfwm.a ${X_LIBS} ${X_PRE_LIBS} -lXpm -lm

man_MANS=xfsound.1
