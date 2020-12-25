## Process this file with automake to produce Makefile.in

CLEANFILES=*~ *.bak core *.core *.o .depend .deps/* 

INCLUDES = @GTK_CFLAGS@ @GDK_IMLIB_CFLAGS@ -I../ -I../libs/ -DXFCE_DIR=\"@XFCE_DIR@\" \
	-DXFCE_LOCALE_DIR=\"@XFCE_LOCALE_DIR@\"

bin_PROGRAMS = xfclock

xfclock_DEPENDENCIES = ../libs/libxfce.a

xfclock_SOURCES = xfclock_cb.c \
		  xfclock_cb.h \
                  xfclock_colorsel.c \
                  xfclock_colorsel.h \
		  xfclock_config.c \
		  xfclock_config.h \
		  xfclock_menus.c \
		  xfclock_menus.h \
		  xfclock_style.c \
		  xfclock_style.h \
		  xfclock.c \
		  xfclock.h

xfclock_LDADD = ../libs/libxfce.a @GTK_LIBS@ @GDK_IMLIB_LIBS@ @INTLLIBS@ @LIBS@ \
	${X_LIBS} ${X_PRE_LIBS} -lXpm -lm

man_MANS=xfclock.1
