## Process this file with automake to produce Makefile.in

CLEANFILES=*~ *.bak core *.core *.o .depend .deps/* 

INCLUDES = -I../ -I../libs/ @X_CFLAGS@ -DXFCE_DIR=\"@XFCE_DIR@\" \
	-DXFCE_LOCALE_DIR=\"@XFCE_LOCALE_DIR@\"

bin_PROGRAMS = xfgnome

xfgnome_DEPENDENCIES = ../libs/libxfwm.a

xfgnome_SOURCES = xfgnome.c xfgnome.h

xfgnome_LDADD=  ../libs/libxfwm.a ${X_LIBS} ${X_PRE_LIBS} -lXext -lX11 -lXpm -lm

man_MANS=xfgnome.1
