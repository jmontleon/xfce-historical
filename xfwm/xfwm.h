
/****************************************************************************
 * This module is based on Twm, but has been siginificantly modified 
 * by Rob Nation 
 ****************************************************************************/
/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/


/***********************************************************************
 * xfwm include file
 ***********************************************************************/

#ifndef _XFWM_
#define _XFWM_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#ifdef HAVE_IMLIB
#include <Imlib.h>
#endif

#ifndef WithdrawnState
#define WithdrawnState 0
#endif

/* use PanFrames! this replaces the 3 pixel margin with PanFrame windows
 * it should not be an option, once it works right. HEDU 2/2/94 */
#define PAN_FRAME_THICKNESS 2	/* or just 1 ? */

/* the maximum number of mouse buttons xfwm knows about */
/* don't think that upping this to 5 will make everything
 * hunky-dory with 5 button mouses */
#define MAX_BUTTONS 5

#include <X11/Intrinsic.h>

#ifdef SIGNALRETURNSINT
#define SIGNAL_T int
#define SIGNAL_RETURN return 0
#else
#define SIGNAL_T void
#define SIGNAL_RETURN return
#endif

#define BW 1			/* border width */
#define BOUNDARY_WIDTH 7	/* border width */
#define CORNER_WIDTH 16		/* border width */

#define HEIGHT_EXTRA 4		/* Extra height for texts in popus */
#define HEIGHT_EXTRA_TITLE 4	/* Extra height for underlining title */
#define HEIGHT_SEPARATOR 4	/* Height of separator lines */

#define SCROLL_REGION 2		/* region around screen edge that */
				/* triggers scrolling */

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define NULLSTR ((char *) NULL)

/* contexts for button presses */
#define C_NO_CONTEXT (1<<0)
#define C_WINDOW     (1<<1)
#define C_TITLE	     (1<<2)
#define C_ICON	     (1<<3)
#define C_ROOT	     (1<<4)
#define C_FRAME	     (1<<5)
#define C_SIDEBAR    (1<<6)
#define C_L1         (1<<7)
#define C_L2         (1<<8)
#define C_L3         (1<<9)
#define C_L4         (1<<10)
#define C_L5         (1<<11)
#define C_R1         (1<<12)
#define C_R2         (1<<13)
#define C_R3         (1<<14)
#define C_R4         (1<<15)
#define C_R5         (1<<16)
#define C_RALL       (C_R1|C_R2|C_R3|C_R4|C_R5)
#define C_LALL       (C_L1|C_L2|C_L3|C_L4|C_L5)
#define C_ALL   (C_WINDOW|C_TITLE|C_ICON|C_ROOT|C_FRAME|C_SIDEBAR|\
                 C_L1|C_L2|C_L3|C_L4|C_L5|C_R1|C_R2|C_R3|C_R4|C_R5)

#define XFWM_HALT    0  /* Quit */
#define XFWM_RESTART 1  /* Restart */
#define XFWM_END     2  /* Session mgt terminating */

typedef struct MyFont
  {
    XFontStruct *font;		/* font structure */
    XFontSet fontset;           /* fontset for multi-language */
    int height;			/* height of the font */
    int y;			/* Y coordinate to draw characters */
  }
MyFont;

typedef struct ColorPair
  {
    Pixel fore;
    Pixel back;
  }
ColorPair;

/* for each window that is on the display, one of these structures
 * is allocated and linked into a list 
 */
typedef struct XfwmWindow
  {
    struct XfwmWindow *next;	/* next xfwm window */
    struct XfwmWindow *prev;	/* prev xfwm window */
    Window w;			/* the child window */
    int old_bw;			/* border width before reparenting */
    Window frame;		/* the frame window */
    Window title_w;		/* the title bar window */
    Window sides[4];
    Window corners[4];		/* Corner pieces */
    int nr_left_buttons;
    int nr_right_buttons;
    Window left_w[3];
    Window right_w[3];
    Window icon_w;		/* the icon window */
    Window icon_pixmap_w;	/* the icon window */
    int wShaped;		/* is this a shaped window */
    int frame_x;		/* x position of frame */
    int frame_y;		/* y position of frame */
    int frame_width;		/* width of frame */
    int frame_height;		/* height of frame */
    int boundary_width;
    int corner_width;
    int bw;
    int title_x;
    int title_y;
    int title_height;		/* height of the title bar */
    int title_width;		/* width of the title bar */
    int icon_x_loc;		/* icon window x coordinate */
    int icon_xl_loc;		/* icon label window x coordinate */
    int icon_y_loc;		/* icon window y coordiante */
    int icon_w_width;		/* width of the icon window */
    int icon_w_height;		/* height of the icon window */
    int icon_t_width;		/* width of the icon title window */
    int icon_p_width;		/* width of the icon pixmap window */
    int icon_p_height;		/* height of the icon pixmap window */
    int shade_x;		/* shaded window x coordinate */
    int shade_y;		/* shaded window y coordiante */
    int shade_width;		/* width of the shaded window */
    int shade_height;		/* height of the shaded window */
    Bool icon_arranged;		/* internal flag */
    Pixmap iconPixmap;		/* pixmap for the icon */
    int iconDepth;		/* Drawable depth for the icon */
    Pixmap icon_maskPixmap;	/* pixmap for the icon mask */
    char *name;			/* name of the window */
    char *icon_name;		/* name of the icon */
    XWindowAttributes attr;	/* the child window attributes */
    XSizeHints hints;		/* normal hints */
    XWMHints *wmhints;		/* WM hints */
    XClassHint class;
    int Desk;			/* Tells which desktop this window is on */
    int FocusDesk;		/* Where (if at all) was it focussed */
    int DeIconifyDesk;		/* Desk to deiconify to, for StubbornIcons */
    Window transientfor;

    unsigned long flags;
    
    char *icon_bitmap_file;

    int orig_x;			/* unmaximized x coordinate */
    int orig_y;			/* unmaximized y coordinate */
    int orig_wd;		/* unmaximized window width */
    int orig_ht;		/* unmaximized window height */

    int xdiff, ydiff;		/* used to restore window position on exit */
    int *mwm_hints;
    int ol_hints;
    int kde_hints;
    int functions;
    Window *cmap_windows;	/* Colormap windows property */
    int number_cmap_windows;	/* Should generally be 0 */
    Pixel TextPixel;
    Pixel BackPixel;
    unsigned long buttons;
  }
XfwmWindow;

/***************************************************************************
 * window flags definitions 
 ***************************************************************************/

/* Basic flags (field flags) */
#define STARTICONIC             (1<<0)
#define ONTOP                   (1<<1)	/* does window stay on top */
#define STICKY                  (1<<2)	/* Does window stick to glass? */
#define WINDOWLISTSKIP          (1<<3)
#define SUPPRESSICON            (1<<4)
#define NOICON_TITLE            (1<<5)
#define Lenience                (1<<6)
#define StickyIcon              (1<<7)
#define CirculateSkipIcon       (1<<8)
#define CirculateSkip           (1<<9)
#define SHOW_ON_MAP             (1<<10)
#define BORDER                  (1<<11)
#define TITLE                   (1<<12)
#define MAPPED                  (1<<13)
#define ICONIFIED               (1<<14)
#define TRANSIENT               (1<<15)
#define RAISED                  (1<<16)
#define VISIBLE                 (1<<17)
#define ICON_OURS               (1<<18)
#define PIXMAP_OURS             (1<<19)
#define SHAPED_ICON             (1<<20)
#define MAXIMIZED               (1<<21)
#define DoesWmTakeFocus		(1<<22)
#define DoesWmDeleteWindow	(1<<23)
#define ICON_MOVED              (1<<24)
#define ICON_UNMAPPED           (1<<25)
#define MAP_PENDING             (1<<26)
#define RAISEDWIN               (1<<27) /* Used for autoraise facility */
#define SHADED                  (1<<28)
#define WM_NAME_CHANGED         (1<<29)


/* options flags (apply to all windows) */
#define ClickToFocus            (1<<0)
#define AnimateWin              (1<<1)
#define MoveOpaqueWin           (1<<2)
#define AutoRaiseWin            (1<<3)
#define ResizeOpaqueWin         (1<<4)
#define SessionMgt              (1<<5)
#define BitmapButtons           (1<<6)


#define ALL_COMMON_FLAGS (STARTICONIC|ONTOP|STICKY|WINDOWLISTSKIP| \
			  SUPPRESSICON|NOICON_TITLE|Lenience|StickyIcon| \
			  CirculateSkipIcon|CirculateSkip|SHOW_ON_MAP)


/* flags to suppress/enable title bar buttons */
#define BUTTON1     1
#define BUTTON2     2
#define BUTTON3     4
#define BUTTON4     8
#define BUTTON5    16
#define BUTTON6    32

#include <stdlib.h>
extern void Reborder (void);
extern void SigDone (int);
extern void Restart (int nonsense);
extern void Done (int, char *);
extern void BlackoutScreen (void);
extern void UnBlackoutScreen (void);

extern int master_pid;

extern Display *dpy;

#ifdef HAVE_IMLIB
extern ImlibData *imlib_id;
#endif

extern XContext XfwmContext;

extern Window BlackoutWin;

extern Boolean ShapesSupported;

extern Window JunkRoot, JunkChild;
extern int JunkX, JunkY;
extern unsigned int JunkWidth, JunkHeight, JunkBW, JunkDepth, JunkMask;

extern Atom _XA_MIT_PRIORITY_COLORS;
extern Atom _XA_WM_CHANGE_STATE;
extern Atom _XA_WM_STATE;
extern Atom _XA_WM_COLORMAP_WINDOWS;
extern Atom _XA_WM_PROTOCOLS;
extern Atom _XA_WM_TAKE_FOCUS;
extern Atom _XA_WM_SAVE_YOURSELF;
extern Atom _XA_WM_DELETE_WINDOW;
extern Atom _XA_WM_DESKTOP;
extern Atom _XA_XFWM_FLAGS;
extern Atom _XA_XFWM_ICONPOS_X;
extern Atom _XA_XFWM_ICONPOS_Y;
extern Atom _XA_OL_WIN_ATTR;
extern Atom _XA_OL_WT_BASE;
extern Atom _XA_OL_WT_CMD;
extern Atom _XA_OL_WT_HELP;
extern Atom _XA_OL_WT_NOTICE;
extern Atom _XA_OL_WT_OTHER;
extern Atom _XA_OL_DECOR_ADD;
extern Atom _XA_OL_DECOR_DEL;
extern Atom _XA_OL_DECOR_CLOSE;
extern Atom _XA_OL_DECOR_RESIZE;
extern Atom _XA_OL_DECOR_HEADER;
extern Atom _XA_OL_DECOR_ICON_NAME;
extern Atom _XA_WIN_WORKSPACE_COUNT;
extern Atom _XA_WIN_WORKSPACE;

extern Atom _XA_WM_WINDOW_ROLE;
extern Atom _XA_WM_CLIENT_LEADER;
extern Atom _XA_SM_CLIENT_ID;

extern unsigned int KeyMask;
extern unsigned int ButtonMask;
extern unsigned int ButtonKeyMask;
extern unsigned int AltMask;
extern unsigned int MetaMask;
extern unsigned int NumLockMask;
extern unsigned int ScrollLockMask;
extern unsigned int SuperMask;
extern unsigned int HyperMask;
    
#endif /* _XFWM_ */
