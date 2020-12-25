## Process this file with automake to produce Makefile.in

CLEANFILES=*~ *.bak core *.core *.o .depend .deps/* 

INCLUDES = -I../libs/

bin_PROGRAMS = xfpager

xfpager_DEPENDENCIES = ../libs/libxfwm.a

xfpager_SOURCES = \
	x_pager.c \
	XfwmPager.c \
	XfwmPager.h

xfpager_LDADD=  ../libs/libxfwm.a ${X_LIBS} ${X_PRE_LIBS} -lXext -lX11 -lXpm -lm

man_MANS=xfpager.1
