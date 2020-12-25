/*

   ORIGINAL FILE NAME : constant.h

   ********************************************************************
   *                                                                  *
   *           X F C E  - Written by O. Fourdan (c) 1997              *
   *                                                                  *
   *           This software is absolutely free of charge             *
   *                                                                  *
   ********************************************************************

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR (O. FOURDAN) BE 
   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
 */



#ifndef __CONSTANT_H__
#define __CONSTANT_H__

#include "forms.h"
#include "X11/cursorfont.h"

#include "stdio.h"		/*
				   To get FILENAME_MAX from standard include file 
				 */

#ifndef FL_COORD
  #define FL_COORD FL_Coord
#endif

/*
   Some font sizes and styles 
 */

#define XFCE_TINY_SIZE		10
#define XFCE_NORMAL_SIZE 	12
#define XFCE_MEDIUM_SIZE	14
#define XFCE_HUGE_SIZE		18

#define XFTINY	FL_MAXFONTS - 1
#define XFREG 	FL_MAXFONTS - 2
#define XFBOLD	FL_MAXFONTS - 3
#define XFBIG 	FL_MAXFONTS - 4
#define XFTIT 	FL_MAXFONTS - 5

#define XFCE_BORDER	FL_UP_BOX
#define XFCE_BUTSTYLE       XFBOLD
#define XFCE_INACTIVELABEL  XFBOLD + FL_ENGRAVED_STYLE
#define XFCE_ACTIVELABEL    XFBOLD + FL_SHADOW_STYLE

#define ALLWAYS_SAVE 	1

   #define XFCE_CURS    XC_left_ptr
   #define MENU_CURS	XC_right_ptr
/*
   #define XFCE_CURS	XC_top_left_arrow
   #define MENU_CURS	XC_arrow
 */

#ifndef USEMWMHINTS		/* Not suitable for Fvwm95 nor Afterstep which */
#define USEMWMHINTS	1	/* supply such terrible support for MWM hints */
#endif				/* Use 0 if your WM does not display borders */

#ifndef BACKUPEXT
#define BACKUPEXT 	".bak"
#endif

#ifndef DEFAULT_SHELL
#define DEFAULT_SHELL 	"/bin/sh"
#endif

#ifndef PAL_PATTERN
#define PAL_PATTERN 	"*"
#endif

#ifndef XFCE_DIR
#define XFCE_DIR        "/var/XFCE"
#endif

#ifndef XFCE_PAL
#define XFCE_PAL 	XFCE_DIR"/palettes"
#endif

#ifndef XFCE_BACKDROPS
#define XFCE_BACKDROPS 	XFCE_DIR"/backdrops"
#endif

#ifndef XFCE_ICONS
#define XFCE_ICONS 	XFCE_DIR"/icons"
#endif

#ifndef XFCE_SOUNDS
#define XFCE_SOUNDS 	XFCE_DIR"/sounds"
#endif

#ifndef XBINDIR
#define XBINDIR 	"/usr/X11/bin"
#endif

#ifndef NBMAXITEMS
#define NBMAXITEMS 	15
#endif

#ifndef MAXSTRLEN
#define MAXSTRLEN 	FILENAME_MAX
#endif

#ifndef BUTTONCHANGE
#define BUTTONCHANGE 	3
#endif

#ifndef BORDERWIDTH
#define BORDERWIDTH 	1
#endif

#ifndef HILIGHTWIDTH
#define HILIGHTWIDTH	0 
#endif

#ifndef HILIGHTSELECT
  #if BORDERWIDTH>=2
    #define HILIGHTSELECT	1
  #else
    #define HILIGHTSELECT	0
  #endif
#endif

#define NBSCREENS   	4	/* Don't change this */
#define NBMENUS   	6	/* Don't change this */
#define NBSELECTS 	7	/* Don't change this */
#define NB_XFCE_COLORS 	8	/* Don't change this */
#define NB_PANEL_ICONS 	13	/* Don't change this */
#define XFCE_COL1	FL_FREE_COL1	/*  Don't change this */
#define XFCE_COL2	FL_FREE_COL2	/*  Don't change this */
#define XFCE_COL3	FL_FREE_COL3	/*  Don't change this */
#define XFCE_COL4	FL_FREE_COL4	/*  Don't change this */
#define XFCE_COL5	FL_FREE_COL5	/*  Don't change this */
#define XFCE_COL6	FL_FREE_COL6	/*  Don't change this */
#define XFCE_COL7	FL_FREE_COL7	/*  Don't change this */
#define XFCE_COL8	FL_FREE_COL8	/*  Don't change this */
#define XFCE_COLB	FL_FREE_COL9	/*  Don't change this */
#define XFCE_COL9	FL_FREE_COL10	/*  Don't change this */
#define XFCE_COL10	FL_FREE_COL11	/*  Don't change this */


#endif /*
          __CONSTANT_H__ 
        */
