
/****************************************************************************
 * This module is based on Twm, but has been siginificantly modified 
 * by Rob Nation
 ****************************************************************************/
/*
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/***********************************************************************
 *
 * xfwm per-screen data include file
 *
 ***********************************************************************/

#ifndef _SCREEN_
#define _SCREEN_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include "misc.h"
#include "menus.h"

#define SIZE_HINDENT 5
#define SIZE_VINDENT 3
#define MAX_WINDOW_WIDTH 32767
#define MAX_WINDOW_HEIGHT 32767


/* Cursor types */
#define POSITION 0		/* upper Left corner cursor */
#define TITLE_CURSOR 1		/* title-bar cursor */
#define DEFAULT 2		/* cursor for apps to inherit */
#define SYS 3			/* sys-menu and iconify boxes cursor */
#define MOVE 4			/* resize cursor */
#if defined(__alpha)
#ifdef WAIT
#undef WAIT
#endif /*WAIT */
#endif /*alpha */
#define WAIT 5			/* wait a while cursor */
#define MENU 6			/* menu cursor */
#define SELECT 7		/* dot cursor for f.move, etc. from menus */
#define DESTROY 8		/* skull and cross bones, f.destroy */
#define TOP 9
#define RIGHT 10
#define BOTTOM 11
#define LEFT 12
#define TOP_LEFT 13
#define TOP_RIGHT 14
#define BOTTOM_LEFT 15
#define BOTTOM_RIGHT 16
#define MAX_CURSORS 18

/* colormap focus styes */
#define COLORMAP_FOLLOWS_MOUSE 1	/* default */
#define COLORMAP_FOLLOWS_FOCUS 2


typedef struct
  {
    Window win;
    int isMapped;
  }
PanFrame;

typedef enum
  {
    /* button types */
    VectorButton,
    SimpleButton,
    GradButton,
    SolidButton
  }
ButtonFaceStyle;

#define ButtonFaceTypeMask      0x000F

/* button style flags (per-state) */
enum
  {

    /* specific style flags */
    /* justification bits (3.17 -> 4 bits) */
    HOffCenter = (1 << 4),
    HRight = (1 << 5),
    VOffCenter = (1 << 6),
    VBottom = (1 << 7),

    /* general style flags */
    UseTitleStyle = (1 << 8),
    UseBorderStyle = (1 << 9),
    FlatButton = (1 << 10),
    SunkButton = (1 << 11)
  };

/* border style flags (uses ButtonFace) */
enum
  {
    HiddenHandles = (1 << 8),
    NoInset = (1 << 9)
  };

typedef struct ButtonFace
  {
    ButtonFaceStyle style;
    union
      {
	Pixel back;
	struct
	  {
	    int npixels;
	    Pixel *pixels;
	  }
	grad;
      }
    u;
    struct vector_coords
      {
	int num;
	int x[20];
	int y[20];
	int line_style[20];
      }
    vector;
    struct ButtonFace *next;
  }
ButtonFace;

/* button style flags (per title button) */
enum
  {
    /* MWM function hint button assignments */
    MWMDecorMenu = (1 << 0),
    MWMDecorMinimize = (1 << 1),
    MWMDecorMaximize = (1 << 2),
    MWMDecorSticky = (1 << 3)
  };

enum ButtonState
  {
    Active, Inactive,
    MaxButtonState
  };

typedef struct
  {
    int flags;
    ButtonFace state[MaxButtonState];
  }
TitleButton;

typedef struct XfwmDecor
  {
    ColorPair HiColors;		/* standard fore/back colors */
    ColorPair HiRelief;
    ColorPair LoColors;		/* standard fore/back colors */
    ColorPair LoRelief;
    GC HiReliefGC;		/* GC for active window relief */
    GC HiShadowGC;		/* GC for active window shadow */
    GC LoReliefGC;		/* GC for inactive window relief */
    GC LoShadowGC;		/* GC for inactive window shadow */

    int TitleHeight;		/* height of the title bar window */
    MyFont WindowFont;		/* font structure for window titles */

    /* titlebar buttons */
    TitleButton left_buttons[2];
    TitleButton right_buttons[2];
    TitleButton titlebar;
    struct BorderStyle
      {
	ButtonFace active, inactive;
      }
    BorderStyle;
  }
XfwmDecor;


typedef struct ScreenInfo
  {

    unsigned long screen;
    int d_depth;		/* copy of DefaultDepth(dpy, screen) */
    int NumberOfScreens;	/* number of screens on display */
    int MyDisplayWidth;		/* my copy of DisplayWidth(dpy, screen) */
    int MyDisplayHeight;	/* my copy of DisplayHeight(dpy, screen) */

    XfwmWindow XfwmRoot;	/* the head of the xfwm window list */
    Window Root;		/* the root window */
    Window NoFocusWin;		/* Window which will own focus when no other
				 * windows have it */
    PanFrame PanFrameTop, PanFrameLeft, PanFrameRight, PanFrameBottom;

    Pixmap gray_bitmap;		/* dark gray pattern for shaded out menu items */
    Pixmap gray_pixmap;		/* dark gray pattern for inactive borders */
    Pixmap light_gray_pixmap;	/* light gray pattern for inactive borders */
    Pixmap sticky_gray_pixmap;	/* light gray pattern for sticky borders */

    Binding *AllBindings;
    MenuRoot *AllMenus;

    int root_pushes;		/* current push level to install root
				 * colormap windows */
    XfwmWindow *pushed_window;	/* saved window to install when pushes drops
				 * to zero */
    Cursor XfwmCursors[MAX_CURSORS];

    name_list *TheList;		/* list of window names with attributes */
    char *DefaultIcon;		/* Icon to use when no other icons are found */

    ColorPair MenuColors;
    ColorPair MenuSelColors;
    ColorPair MenuRelief;

    MyFont StdFont;		/* font structure */
    MyFont IconFont;		/* for icon labels */
    MyFont WindowFont;		/* font structure for window titles */

    GC TransMaskGC;		/* GC for transparency masks */
    GC DrawGC;			/* GC to draw lines for move and resize */
    GC MenuGC;
    GC MenuSelGC;
    GC MenuReliefGC;
    GC MenuShadowGC;
    GC ScratchGC1;
    GC ScratchGC2;
    GC ScratchGC3;
    int SizeStringWidth;	/* minimum width of size window */
    int CornerWidth;		/* corner width for decoratedwindows */
    int BoundaryWidth;		/* frame width for decorated windows */
    int NoBoundaryWidth;	/* frame width for decorated windows */

    XfwmDecor DefaultDecor;	/* decoration style(s) */

    int nr_left_buttons;	/* number of left-side title-bar buttons */
    int nr_right_buttons;	/* number of right-side title-bar buttons */

    XfwmWindow *Hilite;		/* the xfwm window that is highlighted 
				 * except for networking delays, this is the
				 * window which REALLY has the focus */
    XfwmWindow *Focus;		/* Last window which Xfwm gave the focus to 
				 * NOT the window that really has the focus */
    Window UnknownWinFocused;	/* None, if the focus is nowhere or on an xfwm
				 * * managed window. Set to id of otherwindow 
				 * * with focus otherwise */
    XfwmWindow *Ungrabbed;
    XfwmWindow *PreviousFocus;	/* Window which had focus before xfwm stole it
				 * to do moves/menus/etc. */
    int EntryHeight;		/* menu entry height */
    int EdgeScrollX;		/* #pixels to scroll on screen edge */
    int EdgeScrollY;		/* #pixels to scroll on screen edge */
    unsigned char buttons2grab;	/* buttons to grab in click to focus mode */
    unsigned long flags;
    int NumBoxes;
    int randomx;		/* values used for randomPlacement */
    int randomy;
    XfwmWindow *LastWindowRaised;	/* Last window which was raised. Used for raise
					 * lower func. */
    int VxMax;			/* Max location for top left of virt desk */
    int VyMax;
    int Vx;			/* Current loc for top left of virt desk */
    int Vy;

    int ClickTime;		/*Max button-click delay for Function built-in */
    int AutoRaiseDelay;         /* Delay between setting focus and raising win*/
    int ScrollResistance;	/* resistance to scrolling in desktop */
    int MoveResistance;		/* res to moving windows over viewport edge */
    int OpaqueSize;
    int CurrentDesk;		/* The current desktop number */
    int ColormapFocus;		/* colormap focus style */
    int iconbox;		/* 0 = top, 1 = left, 2 = bottom 3 = right */

    /*
     * ** some additional global options which will probably become window
     * ** specific options later on:
     */
    int SmartPlacementIsClever;
    int ClickToFocusPassesClick;
    unsigned int Options;
  }
ScreenInfo;

/* 
 * Macro which gets specific decor or default decor.
 * This saves an indirection in case you don't want
 * the UseDecor mechanism.
 */
#define GetDecor(window,part) (Scr.DefaultDecor.part)

/* some protos for the decoration structures */
void LoadDefaultLeftButton (ButtonFace * bf, int i);
void LoadDefaultRightButton (ButtonFace * bf, int i);
void LoadDefaultButton (ButtonFace * bf, int i);
void ResetAllButtons (XfwmDecor * fl);
void InitXfwmDecor (XfwmDecor * fl);
void DestroyXfwmDecor (XfwmDecor * fl);

extern ScreenInfo Scr;

/* for the flags value - these used to be seperate Bool's */
#define WindowsCaptured            (1)
#define EdgeWrapX                 (64)	/* Should EdgeScroll wrap around? */
#define EdgeWrapY                (128)
#endif /* _SCREEN_ */
