## Process this file with automake to produce Makefile.in

CLEANFILES=*~ *.bak core *.core *.o .depend .deps/* 

INCLUDES = @GTK_CFLAGS@ @GDK_IMLIB_CFLAGS@ -I../ -I../libs/ -DXFCE_DIR=\"@XFCE_DIR@\" \
	-DXFCE_LOCALE_DIR=\"@XFCE_LOCALE_DIR@\"

noinst_LIBRARIES = libxfce.a libxfwm.a

libxfce_a_SOURCES = \
	colorselect.c colorselect.h \
	constant.h \
	empty.h \
	my_intl.h \
	gtkinvisible.h \
	fileselect.c fileselect.h \
	fileutil.c fileutil.h \
	fontselection.c fontselection.h \
	gnome_protocol.c gnome_protocol.h \
	lightdark.h \
	my_string.c my_string.h \
	my_tooltips.c my_tooltips.h \
	mygtkclock.c mygtkclock.h \
	sendinfo.c sendinfo.h \
	xfce-common.c xfce-common.h \
	xfcolor.c xfcolor.h \
	xpmext.c xpmext.h \
	snprintf.c snprintf.h

libxfwm_a_SOURCES = \
	ClientMsg.c \
	GetFont.c \
	Grab.c \
	Module.c \
	Parse.c \
	Picture.c \
	Strings.c \
	System.c \
	alpha_header.h \
	configure.h \
	envvar.c  \
	module.h \
	safemalloc.c \
	sleep.c \
	sun_headers.h \
	utils.h \
	wild.c
