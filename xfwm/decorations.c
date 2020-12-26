/****************************************************************************
 * 
 * This is all original code by Robert Nation 
 * which reads motif mwm window manager
 * hints from a window, and makes necessary adjustments for xfwm. 
 *
 * Definitions of the hint structure and the constants are courtesy of
 * mitnits@bgumail.bgu.ac.il (Roman Mitnitski ), who sent this note,
 * after conferring with a friend at the OSF:
 * Hi, Rob.
 *
 *I'm happy to announce, that you can use motif public
 *headers in any way you can... I just got the letter from
 *my friend, it says literally:
 *
 *>    Hi.
 *>
 *> Yes, you can use motif public header files, in particular
 *> because there is NO limitation on inclusion of this files
 *> in your programms....Also, no one can put copyright to the NUMBERS
 *> (I mean binary flags for decorations) or DATA STRUCTURES
 *> (I mean little structure used by motif to pass description
 *> of the decorations to the mwm). Call it another name, if you are
 *> THAT MUCH concerned.
 *>
 *> You can even use the little piece of code I've passed to you - 
 *> we are talking about 10M distribution against two pages of code.
 *> Don't be silly.
 *> 
 *> Best wishes. 
 *> Eli.
 *
 *
 ****************************************************************************/

#include "configure.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xfwm.h"
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "screen.h"
#include "parse.h"
#include "menus.h"

extern Atom _XA_MwmAtom;

/* Motif  window hints */
typedef struct
  {
    CARD32 flags;
    CARD32 functions;
    CARD32 decorations;
    INT32 inputMode;
  }
PropMotifWmHints;

typedef struct _extended_hints
{
    int flags;
    int desktop;
} 
ExtendedHints;

typedef PropMotifWmHints PropMwmHints;

/* Motif window hints */
#define MWM_HINTS_FUNCTIONS           (1L << 0)
#define MWM_HINTS_DECORATIONS         (1L << 1)

/* bit definitions for MwmHints.functions */
#define MWM_FUNC_ALL            (1L << 0)
#define MWM_FUNC_RESIZE         (1L << 1)
#define MWM_FUNC_MOVE           (1L << 2)
#define MWM_FUNC_MINIMIZE       (1L << 3)
#define MWM_FUNC_MAXIMIZE       (1L << 4)
#define MWM_FUNC_CLOSE          (1L << 5)

/* bit definitions for MwmHints.decorations */
#define MWM_DECOR_ALL                 (1L << 0)
#define MWM_DECOR_BORDER              (1L << 1)
#define MWM_DECOR_RESIZEH             (1L << 2)
#define MWM_DECOR_TITLE               (1L << 3)
#define MWM_DECOR_MENU                (1L << 4)
#define MWM_DECOR_MINIMIZE            (1L << 5)
#define MWM_DECOR_MAXIMIZE            (1L << 6)

#define PROP_MOTIF_WM_HINTS_ELEMENTS  4
#define PROP_MWM_HINTS_ELEMENTS       PROP_MOTIF_WM_HINTS_ELEMENTS

/* bit definitions for OL hints; I just
 *  made these up, OL stores hints as atoms */
#define OL_DECOR_CLOSE                (1L << 0)
#define OL_DECOR_RESIZEH              (1L << 1)
#define OL_DECOR_HEADER               (1L << 2)
#define OL_DECOR_ICON_NAME            (1L << 3)
#define OL_DECOR_ALL                  (OL_DECOR_CLOSE | OL_DECOR_RESIZEH | OL_DECOR_HEADER | OL_DECOR_ICON_NAME)

#define EXTENDED_HINT_STICKY            (1<<0)
#define EXTENDED_HINT_ONTOP             (1<<1)
#define EXTENDED_HINT_ONBOTTOM          (1<<2)
#define EXTENDED_HINT_NEVER_USE_AREA    (1<<3)
#define EXTENDED_HINT_DESKTOP           (1<<4)

#define KDE_noDecoration              0
#define KDE_normalDecoration          1
#define KDE_tinyDecoration            2

extern XfwmWindow *Tmp_win;

/****************************************************************************
 * 
 * Reads the property MOTIF_WM_HINTS
 *
 *****************************************************************************/
void
GetMwmHints (XfwmWindow * t)
{
  int actual_format;
  Atom actual_type;
  unsigned long nitems, bytesafter;

  if (XGetWindowProperty (dpy, t->w, _XA_MwmAtom, 0L, 20L, False,
			  _XA_MwmAtom, &actual_type, &actual_format, &nitems,
		  &bytesafter, (unsigned char **) &t->mwm_hints) == Success)
    {
      if (nitems >= PROP_MOTIF_WM_HINTS_ELEMENTS)
	  return;
      XFree((char *)t->mwm_hints);
    }

  t->mwm_hints = NULL;
}

/****************************************************************************
 * 
 * Reads the openlook properties _OL_WIN_ATTR, _OL_DECOR_ADD, _OL_DECOR_DEL
 *
 * _OL_WIN_ATTR - the win_type field is the either the first atom if the
 * property has length three or the second atom if the property has
 * length five.  It can be any of:
 *     _OL_WT_BASE (no changes)
 *     _OL_WT_CMD (no minimize decoration)
 *     _OL_WT_HELP (no minimize, maximize or resize handle decorations)
 *     _OL_WT_NOTICE (no minimize, maximize, system menu, resize handle
 *         or titlebar decorations)
 *     _OL_WT_OTHER (no minimize, maximize, system menu, resize handle
 *         or titlebar decorations)
 * In addition, if the _OL_WIN_ATTR property is in the three atom format
 * or if the type is _OL_WT_OTHER, then the icon name is not displayed
 * (same behavior as olvwm).
 * 
 * _OL_DECOR_ADD or _OL_DECOR_DEL - indivdually add or remove minimize
 * button (_OL_DECOR_CLOSE), resize handles (_OL_DECOR_RESIZE), title bar
 * (_OL_DECOR_HEADER), or icon name (_OL_DECOR_ICON_NAME).
 * 
 * The documentation for the Open Look hints was taken from "Advanced X
 * Window Application Programming", Eric F. Johnson and Kevin Reichard
 * (M&T Books), and the olvwm source code (available at ftp.x.org in
 * /R5contrib).
 *****************************************************************************/
void
GetOlHints (XfwmWindow * t)
{
  int actual_format;
  Atom actual_type;
  unsigned long nitems, bytesafter;
  Atom *hints;
  int i;
  Atom win_type;

  t->ol_hints = OL_DECOR_ALL;

  if (XGetWindowProperty (dpy, t->w, _XA_OL_WIN_ATTR, 0L, 20L, False,
		     _XA_OL_WIN_ATTR, &actual_type, &actual_format, &nitems,
			  &bytesafter, (unsigned char **) &hints) == Success)
    {
      if (nitems > 0)
	{
	  if (nitems == 3)
	    win_type = hints[0];
	  else
	    win_type = hints[1];

	  /* got this from olvwm and sort of mapped it to
	   * XFWM/MWM hints */
	  if (win_type == _XA_OL_WT_BASE)
	    t->ol_hints = OL_DECOR_ALL;
	  else if (win_type == _XA_OL_WT_CMD)
	    t->ol_hints = OL_DECOR_ALL & ~OL_DECOR_CLOSE;
	  else if (win_type == _XA_OL_WT_HELP)
	    t->ol_hints = OL_DECOR_ALL & ~(OL_DECOR_CLOSE | OL_DECOR_RESIZEH);
	  else if (win_type == _XA_OL_WT_NOTICE)
	    t->ol_hints = OL_DECOR_ALL & ~(OL_DECOR_CLOSE | OL_DECOR_RESIZEH |
				      OL_DECOR_HEADER | OL_DECOR_ICON_NAME);
	  else if (win_type == _XA_OL_WT_OTHER)
	    t->ol_hints = 0;
	  else
	    t->ol_hints = OL_DECOR_ALL;

	  if (nitems == 3)
	    t->ol_hints &= ~OL_DECOR_ICON_NAME;
	}

      XFree (hints);
    }

  if (XGetWindowProperty (dpy, t->w, _XA_OL_DECOR_ADD, 0L, 20L, False,
			  XA_ATOM, &actual_type, &actual_format, &nitems,
			  &bytesafter, (unsigned char **) &hints) == Success)
    {
      for (i = 0; i < nitems; i++)
	{
	  if (hints[i] == _XA_OL_DECOR_CLOSE)
	    t->ol_hints |= OL_DECOR_CLOSE;
	  else if (hints[i] == _XA_OL_DECOR_RESIZE)
	    t->ol_hints |= OL_DECOR_RESIZEH;
	  else if (hints[i] == _XA_OL_DECOR_HEADER)
	    t->ol_hints |= OL_DECOR_HEADER;
	  else if (hints[i] == _XA_OL_DECOR_ICON_NAME)
	    t->ol_hints |= OL_DECOR_ICON_NAME;
	}
      XFree (hints);
    }

  if (XGetWindowProperty (dpy, t->w, _XA_OL_DECOR_DEL, 0L, 20L, False,
			  XA_ATOM, &actual_type, &actual_format, &nitems,
			  &bytesafter, (unsigned char **) &hints) == Success)
    {
      for (i = 0; i < nitems; i++)
	{
	  if (hints[i] == _XA_OL_DECOR_CLOSE)
	    t->ol_hints &= ~OL_DECOR_CLOSE;
	  else if (hints[i] == _XA_OL_DECOR_RESIZE)
	    t->ol_hints &= ~OL_DECOR_RESIZEH;
	  else if (hints[i] == _XA_OL_DECOR_HEADER)
	    t->ol_hints &= ~OL_DECOR_HEADER;
	  else if (hints[i] == _XA_OL_DECOR_ICON_NAME)
	    t->ol_hints &= ~OL_DECOR_ICON_NAME;
	}
      XFree (hints);
    }

}

void
GetExtHints(XfwmWindow * t)
{
   Atom 	       a1, a3;
   unsigned long       lnum, ldummy;
   int  	       dummy;
   ExtendedHints       *eh;

   a1 = XInternAtom(dpy, "WM_EXTENDED_HINTS", False);
   eh = NULL;
   if (XGetWindowProperty(dpy, t->w, a1, 0, 
                      sizeof(ExtendedHints)/sizeof(unsigned long), 
                      False, a1, &a3, &dummy, &lnum,
		      &ldummy, (unsigned char **)&eh) == Success)
   {
      if (eh)
	{
	   if (eh->flags & EXTENDED_HINT_STICKY)
	     t->flags |= STICKY;
	   if (eh->flags & EXTENDED_HINT_ONTOP)
	     t->flags |= ONTOP;
	   if (eh->flags & EXTENDED_HINT_ONBOTTOM)
	     t->flags &= ~ONTOP;
	   if (eh->flags & EXTENDED_HINT_NEVER_USE_AREA)
	     ; /* Not implemented */
	   if (eh->flags & EXTENDED_HINT_DESKTOP)
	     ; /* Not implemented */
	   XFree(eh);
	}
    }
}

void
GetKDEHints(XfwmWindow * t)
{
   Atom                a, dummy_a;
   unsigned long       lnum, ldummy;
   int  	       dummy, status;
   long                *p = 0;

   a = XInternAtom(dpy, "KWM_WIN_DECORATION", False);
   t->kde_hints = -1;
   status = XGetWindowProperty(dpy, t->w, a, 0L, 1L, False, a, &dummy_a, &dummy,
                    &lnum, &ldummy, (unsigned char **)&p);
   if ((status == Success) && (p != 0)) {
     t->kde_hints = (p[0] & 3);
     if (lnum == 0)
       XFree((void*) *p);
     XFree((char *) p);
   }
}

/****************************************************************************
 * 
 * Interprets the property MOTIF_WM_HINTS, sets decoration and functions
 * accordingly
 *
 *****************************************************************************/
void
SelectDecor (XfwmWindow * t, unsigned long tflags, int border_width)
{
  int decor, i;
  PropMwmHints *prop;

  if (!(tflags & BW_FLAG))
    border_width = Scr.NoBoundaryWidth;

  for (i = 0; i < 2; i++)
    {
      t->left_w[i] = 1;
      t->right_w[i] = 1;
    }

  decor = MWM_DECOR_ALL;
  t->functions = MWM_FUNC_ALL;
  if (t->mwm_hints)
    {
      prop = (PropMwmHints *) t->mwm_hints;
      if (tflags & MWM_DECOR_FLAG)
	if (prop->flags & MWM_HINTS_DECORATIONS)
	  decor = prop->decorations;
      if (tflags & MWM_FUNCTIONS_FLAG)
	if (prop->flags & MWM_HINTS_FUNCTIONS)
	  t->functions = prop->functions;
    }

  /* functions affect the decorations! if the user says
   * no iconify function, then the iconify button doesn't show
   * up. */
  if (t->functions & MWM_FUNC_ALL)
    {
      /* If we get ALL + some other things, that means to use
       * ALL except the other things... */
      t->functions &= ~MWM_FUNC_ALL;
      t->functions = (MWM_FUNC_RESIZE | MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE |
		    MWM_FUNC_MAXIMIZE | MWM_FUNC_CLOSE) & (~(t->functions));
    }
  if ((tflags & MWM_FUNCTIONS_FLAG) && (t->flags & TRANSIENT))
    {
      t->functions &= ~(MWM_FUNC_MAXIMIZE | MWM_FUNC_MINIMIZE);
    }

  if (decor & MWM_DECOR_ALL)
    {
      /* If we get ALL + some other things, that means to use
       * ALL except the other things... */
      decor &= ~MWM_DECOR_ALL;
      decor = (MWM_DECOR_BORDER | MWM_DECOR_RESIZEH | MWM_DECOR_TITLE |
	       MWM_DECOR_MENU | MWM_DECOR_MINIMIZE | MWM_DECOR_MAXIMIZE)
	& (~decor);
    }

  /* now remove any functions specified in the OL hints */
  if (!(t->ol_hints & OL_DECOR_CLOSE))
    t->functions &= ~MWM_FUNC_MINIMIZE;
  if (!(t->ol_hints & OL_DECOR_RESIZEH))
    t->functions &= ~(MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE);
  if (!(t->ol_hints & OL_DECOR_HEADER))
    t->functions &= ~(MWM_DECOR_MENU | MWM_FUNC_MINIMIZE |
		      MWM_FUNC_MAXIMIZE | MWM_DECOR_TITLE);
  if (!(t->ol_hints & OL_DECOR_ICON_NAME))
    t->flags |= NOICON_TITLE;
      
  if (!(t->functions & MWM_FUNC_RESIZE))
    decor &= ~MWM_DECOR_RESIZEH;
  if (!(t->functions & MWM_FUNC_MINIMIZE))
    decor &= ~MWM_DECOR_MINIMIZE;
  if (!(t->functions & MWM_FUNC_MAXIMIZE))
    decor &= ~MWM_DECOR_MAXIMIZE;
  if (decor & (MWM_DECOR_MENU | MWM_DECOR_MINIMIZE | MWM_DECOR_MAXIMIZE))
    decor |= MWM_DECOR_TITLE;

  if (tflags & NOTITLE_FLAG)
    decor &= ~MWM_DECOR_TITLE;

  if (tflags & NOBORDER_FLAG)
    decor &= ~MWM_DECOR_RESIZEH;

  if ((tflags & MWM_DECOR_FLAG) && (t->flags & TRANSIENT)) 
    {
      decor &= ~(MWM_DECOR_MAXIMIZE | MWM_DECOR_MINIMIZE);
    }

  if (t->kde_hints == KDE_normalDecoration) {
    decor        |= MWM_DECOR_ALL;
    t->functions |= MWM_FUNC_ALL;
  }
  else if (t->kde_hints == KDE_noDecoration){
    decor &= ~(MWM_DECOR_BORDER | MWM_DECOR_RESIZEH | MWM_DECOR_TITLE |
	       MWM_DECOR_MENU | MWM_DECOR_MINIMIZE | MWM_DECOR_MAXIMIZE);
    t->functions &= ~MWM_FUNC_ALL;
  }
  else if (t->kde_hints == KDE_tinyDecoration) {
    decor        &= ~(MWM_DECOR_MAXIMIZE | MWM_DECOR_MINIMIZE |
                      MWM_DECOR_TITLE | MWM_DECOR_RESIZEH);
    t->functions &= ~(MWM_DECOR_MENU | MWM_FUNC_MINIMIZE |
		      MWM_FUNC_MAXIMIZE | MWM_DECOR_TITLE);
  }

  if (ShapesSupported)
    {
      if (t->wShaped)
	decor &= ~(BORDER | MWM_DECOR_RESIZEH);
    }

  t->flags &= ~(BORDER | TITLE);
  t->boundary_width = 0;
  t->corner_width = 0;
  t->title_height = 0;
  if (decor & MWM_DECOR_BORDER)
    {
        t->boundary_width = border_width;
    }
  if (decor & MWM_DECOR_TITLE)
    {
      t->flags |= TITLE;
      t->title_height = GetDecor (t, TitleHeight);
    }
  if (decor & MWM_DECOR_RESIZEH)
    {
      t->flags |= BORDER;
      t->boundary_width = border_width;
      t->corner_width = GetDecor (t, TitleHeight) + t->boundary_width;
    }
  if (!(decor & MWM_DECOR_MENU))
    {
      for (i = 0; i < 2; ++i)
	{
	  if (GetDecor (t, left_buttons[i].flags) & MWMDecorMenu)
	    t->left_w[i] = None;
	  if (GetDecor (t, right_buttons[i].flags) & MWMDecorMenu)
	    t->right_w[i] = None;
	}
    }
  if (!(decor & MWM_DECOR_MINIMIZE))
    {
      for (i = 0; i < 2; ++i)
	{
	  if (GetDecor (t, left_buttons[i].flags) & MWMDecorMinimize)
	    t->left_w[i] = None;
	  if (GetDecor (t, right_buttons[i].flags) & MWMDecorMinimize)
	    t->right_w[i] = None;
	}
    }
  if (!(decor & MWM_DECOR_MAXIMIZE))
    {
      for (i = 0; i < 2; ++i)
	{
	  if (GetDecor (t, left_buttons[i].flags) & MWMDecorMaximize)
	    t->left_w[i] = None;
	  if (GetDecor (t, right_buttons[i].flags) & MWMDecorMaximize)
	    t->right_w[i] = None;
	}
    }
  if (t->buttons & BUTTON1)
    t->left_w[0] = None;
  if (t->buttons & BUTTON3)
    t->left_w[1] = None;

  if (t->buttons & BUTTON2)
    t->right_w[0] = None;
  if (t->buttons & BUTTON4)
    t->right_w[1] = None;

  t->nr_left_buttons = Scr.nr_left_buttons;
  t->nr_right_buttons = Scr.nr_right_buttons;

  for (i = 0; i < Scr.nr_left_buttons; i++)
    if (t->left_w[i] == None)
      t->nr_left_buttons--;

  for (i = 0; i < Scr.nr_right_buttons; i++)
    if (t->right_w[i] == None)
      t->nr_right_buttons--;

  t->bw = 0;
  if (t->boundary_width <= 0)
      t->boundary_width = 0;
  if (t->title_height > 0)
    t->title_height += t->bw;
  if (t->boundary_width == 0)
    t->flags &= ~BORDER;
}

/****************************************************************************
 * 
 * Checks the function described in menuItem mi, and sees if it
 * is an allowed function for window Tmp_Win,
 * according to the motif way of life.
 * 
 * This routine is used to determine whether or not to grey out menu items.
 *
 ****************************************************************************/
int
check_allowed_function (MenuItem * mi)
{
  /* Complex functions are a little tricky... ignore them for now */

  if ((Tmp_win) &&
    (!(Tmp_win->flags & DoesWmDeleteWindow)) && (mi->func_type == F_DELETE))
    return 0;

  /* Move is a funny hint. Keeps it out of the menu, but you're still allowed
   * to move. */
  if ((mi->func_type == F_MOVE) && (Tmp_win) && (!(Tmp_win->functions & MWM_FUNC_MOVE)))
    return 0;

  if ((mi->func_type == F_RESIZE) && (Tmp_win) &&
      (!(Tmp_win->functions & MWM_FUNC_RESIZE)))
    return 0;

  if ((mi->func_type == F_ICONIFY) && (Tmp_win) &&
      (!(Tmp_win->flags & ICONIFIED)) &&
      (!(Tmp_win->functions & MWM_FUNC_MINIMIZE)))
    return 0;

  if ((mi->func_type == F_MAXIMIZE) && (Tmp_win) &&
      (!(Tmp_win->functions & MWM_FUNC_MAXIMIZE)))
    return 0;

  if ((mi->func_type == F_DELETE) && (Tmp_win) &&
      (!(Tmp_win->functions & MWM_FUNC_CLOSE)))
    return 0;

  if ((mi->func_type == F_CLOSE) && (Tmp_win) &&
      (!(Tmp_win->functions & MWM_FUNC_CLOSE)))
    return 0;

  if ((mi->func_type == F_DESTROY) && (Tmp_win) &&
      (!(Tmp_win->functions & MWM_FUNC_CLOSE)))
    return 0;

  if (mi->func_type == F_FUNCTION)
    {
      /* Hard part! What to do now? */
      /* Hate to do it, but for lack of a better idea,
       * check based on the menu entry name */
      if ((Tmp_win) && (!(Tmp_win->functions & MWM_FUNC_MOVE)) &&
	  (mystrncasecmp (mi->item, MOVE_STRING, strlen (MOVE_STRING)) == 0))
	return 0;

      if ((Tmp_win) && (!(Tmp_win->functions & MWM_FUNC_RESIZE)) &&
	  (mystrncasecmp (mi->item, RESIZE_STRING1, strlen (RESIZE_STRING1)) == 0))
	return 0;

      if ((Tmp_win) && (!(Tmp_win->functions & MWM_FUNC_RESIZE)) &&
	  (mystrncasecmp (mi->item, RESIZE_STRING2, strlen (RESIZE_STRING2)) == 0))
	return 0;

      if ((Tmp_win) && (!(Tmp_win->functions & MWM_FUNC_MINIMIZE)) &&
	  (!(Tmp_win->flags & ICONIFIED)) &&
	  (mystrncasecmp (mi->item, MINIMIZE_STRING, strlen (MINIMIZE_STRING)) == 0))
	return 0;

      if ((Tmp_win) && (!(Tmp_win->functions & MWM_FUNC_MINIMIZE)) &&
	  (mystrncasecmp (mi->item, MINIMIZE_STRING2, strlen (MINIMIZE_STRING2)) == 0))
	return 0;

      if ((Tmp_win) && (!(Tmp_win->functions & MWM_FUNC_MAXIMIZE)) &&
	  (mystrncasecmp (mi->item, MAXIMIZE_STRING, strlen (MAXIMIZE_STRING)) == 0))
	return 0;

      if ((Tmp_win) && (!(Tmp_win->functions & MWM_FUNC_CLOSE)) &&
      (mystrncasecmp (mi->item, CLOSE_STRING1, strlen (CLOSE_STRING1)) == 0))
	return 0;

      if ((Tmp_win) && (!(Tmp_win->functions & MWM_FUNC_CLOSE)) &&
      (mystrncasecmp (mi->item, CLOSE_STRING2, strlen (CLOSE_STRING2)) == 0))
	return 0;

      if ((Tmp_win) && (!(Tmp_win->functions & MWM_FUNC_CLOSE)) &&
      (mystrncasecmp (mi->item, CLOSE_STRING3, strlen (CLOSE_STRING3)) == 0))
	return 0;

      if ((Tmp_win) && (!(Tmp_win->functions & MWM_FUNC_CLOSE)) &&
      (mystrncasecmp (mi->item, CLOSE_STRING4, strlen (CLOSE_STRING4)) == 0))
	return 0;

    }
  return 1;
}