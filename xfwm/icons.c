/****************************************************************************
 * This module is mostly all new
 * by Rob Nation 
 * A little of it is borrowed from ctwm.
 * Copyright 1993 Robert Nation. No restrictions are placed on this code,
 * as long as the copyright notice is preserved
 ****************************************************************************/
/***********************************************************************
 *
 * xfwm icon code
 *
 ***********************************************************************/

#include "configure.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#ifdef NeXT
#include <fcntl.h>
#endif

#ifdef HAVE_IMLIB
#include <Imlib.h>
#endif

#include <X11/Intrinsic.h>
#include <X11/xpm.h>
#include "xfwm.h"
#include "menus.h"
#include "misc.h"
#include "parse.h"
#include "screen.h"
#include "module.h"
#include "default_icon.h"

#include <X11/extensions/shape.h>

#ifdef DMALLOC
#  include "dmalloc.h"
#endif

void GrabIconButtons (XfwmWindow *, Window);
void GrabIconKeys (XfwmWindow *, Window);

/****************************************************************************
 *
 * Creates an icon window as needed
 *
 ****************************************************************************/
void
CreateIconWindow (XfwmWindow * tmp_win, int def_x, int def_y)
{
  int final_x, final_y;
  unsigned long valuemask;	/* mask for create windows */
  XSetWindowAttributes attributes;	/* attributes for create windows */

  tmp_win->flags |= ICON_OURS;
  tmp_win->flags &= ~PIXMAP_OURS;
  tmp_win->flags &= ~SHAPED_ICON;
  tmp_win->icon_pixmap_w = None;
  tmp_win->iconPixmap = None;
  tmp_win->iconDepth = 0;

  if (tmp_win->flags & SUPPRESSICON)
    return;

  tmp_win->icon_p_height = 0;
  tmp_win->icon_p_width = 0;

  /* First, check for a monochrome bitmap */
  if (tmp_win->icon_bitmap_file != NULL)
    GetBitmapFile (tmp_win);

  /* Next, check for a color pixmap */
  if ((tmp_win->icon_bitmap_file != NULL) &&
      (tmp_win->icon_p_height == 0) && (tmp_win->icon_p_width == 0))
    GetXPMFile (tmp_win);

  /* See if the app supplies its own icon window */
  if ((tmp_win->icon_p_height == 0) && (tmp_win->icon_p_width == 0) &&
      (tmp_win->wmhints) && (tmp_win->wmhints->flags & IconWindowHint))
    GetIconWindow (tmp_win);

  /* Finally, try to get icon bitmap from the application */
  if ((tmp_win->icon_p_height == 0) && (tmp_win->icon_p_width == 0) &&
      (tmp_win->wmhints) && (tmp_win->wmhints->flags & IconPixmapHint))
    GetIconBitmap (tmp_win);

  if ((tmp_win->icon_p_height == 0) && (tmp_win->icon_p_width == 0))
    GetXPMFile (tmp_win);

  /* figure out the icon window size */
  if (!(tmp_win->flags & NOICON_TITLE) || (tmp_win->icon_p_height == 0))
    {
      XFontSet fontset = Scr.IconFont.fontset;
      if (fontset)
	{
	  XRectangle rect1, rect2;
	  XmbTextExtents (fontset, tmp_win->icon_name,
			  strlen (tmp_win->icon_name), &rect1, &rect2);
	  tmp_win->icon_t_width = rect2.width;
	}
      else
	tmp_win->icon_t_width = XTextWidth (Scr.IconFont.font,
					    tmp_win->icon_name,
					    strlen (tmp_win->icon_name));
      tmp_win->icon_w_height = ICON_HEIGHT;
    }
  else
    {
      tmp_win->icon_t_width = 0;
      tmp_win->icon_w_height = 0;
    }
  if ((tmp_win->flags & ICON_OURS) && (tmp_win->icon_p_height > 0))
    {
      tmp_win->icon_p_width += 4;
      tmp_win->icon_p_height += 4;
    }

  if (tmp_win->icon_p_width == 0)
    tmp_win->icon_p_width = tmp_win->icon_t_width + 6;
  tmp_win->icon_w_width = tmp_win->icon_p_width;

  final_x = def_x;
  final_y = def_y;
  if (final_x < 0)
    final_x = 0;
  if (final_y < 0)
    final_y = 0;

  if (final_x + tmp_win->icon_w_width >= Scr.MyDisplayWidth)
    final_x = Scr.MyDisplayWidth - tmp_win->icon_w_width - 1;
  if (final_y + tmp_win->icon_w_height >= Scr.MyDisplayHeight)
    final_y = Scr.MyDisplayHeight - tmp_win->icon_w_height - 1;

  tmp_win->icon_x_loc = final_x;
  tmp_win->icon_xl_loc = final_x;
  tmp_win->icon_y_loc = final_y;

  /* clip to fit on screen */
  attributes.background_pixel = Scr.MenuColors.back;
  valuemask = CWBorderPixel | CWCursor | CWEventMask | CWBackPixel;
  attributes.border_pixel = Scr.MenuColors.fore;
  attributes.cursor = Scr.XfwmCursors[DEFAULT];
  attributes.event_mask = (ButtonPressMask | ButtonReleaseMask |
			   VisibilityChangeMask |
			   ExposureMask | KeyPressMask | EnterWindowMask |
			   FocusChangeMask);
  if (!(tmp_win->flags & NOICON_TITLE) || (tmp_win->icon_p_height == 0))
    tmp_win->icon_w =
      XCreateWindow (dpy, Scr.Root, final_x, final_y + tmp_win->icon_p_height,
		     tmp_win->icon_w_width, tmp_win->icon_w_height, 0,
		     CopyFromParent,
		     CopyFromParent, CopyFromParent, valuemask, &attributes);

  if ((tmp_win->flags & ICON_OURS) && (tmp_win->icon_p_width > 0) &&
      (tmp_win->icon_p_height > 0))
    {
      tmp_win->icon_pixmap_w =
	XCreateWindow (dpy, Scr.Root, final_x, final_y, tmp_win->icon_p_width,
		       tmp_win->icon_p_height, 0, CopyFromParent,
		       CopyFromParent, CopyFromParent, valuemask,
		       &attributes);
    }
  else
    {
      attributes.event_mask = (ButtonPressMask | ButtonReleaseMask |
			       VisibilityChangeMask |
			       KeyPressMask | EnterWindowMask |
			       FocusChangeMask | LeaveWindowMask);

      valuemask = CWEventMask;
      XChangeWindowAttributes (dpy, tmp_win->icon_pixmap_w,
			       valuemask, &attributes);
    }


  if (ShapesSupported && (tmp_win->flags & SHAPED_ICON))
    {
      XShapeCombineMask (dpy, tmp_win->icon_pixmap_w, ShapeBounding, 2, 2,
			 tmp_win->icon_maskPixmap, ShapeSet);
    }

  if (tmp_win->icon_w != None)
    {
      XSaveContext (dpy, tmp_win->icon_w, XfwmContext, (caddr_t) tmp_win);
      XDefineCursor (dpy, tmp_win->icon_w, Scr.XfwmCursors[DEFAULT]);
      GrabIconButtons (tmp_win, tmp_win->icon_w);
      GrabIconKeys (tmp_win, tmp_win->icon_w);
    }
  if (tmp_win->icon_pixmap_w != None)
    {
      XSaveContext (dpy, tmp_win->icon_pixmap_w, XfwmContext,
		    (caddr_t) tmp_win);
      XDefineCursor (dpy, tmp_win->icon_pixmap_w, Scr.XfwmCursors[DEFAULT]);
      GrabIconButtons (tmp_win, tmp_win->icon_pixmap_w);
      GrabIconKeys (tmp_win, tmp_win->icon_pixmap_w);
    }
  return;
}

/****************************************************************************
 *
 * Draws the icon window
 *
 ****************************************************************************/
void
DrawIconWindow (XfwmWindow * Tmp_win)
{
  Pixel TextColor = 0;
  Pixel BackColor = 0;
  GC Shadow, Relief;
  int x;

  if (Tmp_win->icon_w != None)
    flush_expose (Tmp_win->icon_w);
  if (Tmp_win->icon_pixmap_w != None)
    flush_expose (Tmp_win->icon_pixmap_w);

  if (Scr.Hilite == Tmp_win)
    {
      Relief = GetDecor (Tmp_win, HiReliefGC);
      Shadow = GetDecor (Tmp_win, HiShadowGC);
      TextColor = GetDecor (Tmp_win, HiColors.fore);
      BackColor = GetDecor (Tmp_win, HiColors.back);
      if (Tmp_win->icon_w != None)
	XSetWindowBackground (dpy, Tmp_win->icon_w,
			      GetDecor (Tmp_win,
					titlebar.state[Active].u.back));
      /* resize the icon name window */
      if (Tmp_win->icon_w != None)
	{
	  Tmp_win->icon_w_width = Tmp_win->icon_t_width + 6;
	  if (Tmp_win->icon_w_width < Tmp_win->icon_p_width)
	    Tmp_win->icon_w_width = Tmp_win->icon_p_width;
	  Tmp_win->icon_xl_loc = Tmp_win->icon_x_loc -
	    (Tmp_win->icon_w_width - Tmp_win->icon_p_width) / 2;
	  /* start keep label on screen. dje 8/7/97 */
	  if (Tmp_win->icon_xl_loc < 0)
	    {			/* if new loc neg (off left edge) */
	      Tmp_win->icon_xl_loc = 0;	/* move to edge */
	    }
	  else
	    {			/* if not on left edge */
	      /* if (new loc + width) > screen width (off edge on right) */
	      if ((Tmp_win->icon_xl_loc + Tmp_win->icon_w_width) >
		  Scr.MyDisplayWidth)
		{		/* off right */
		  /* position up against right edge */
		  Tmp_win->icon_xl_loc =
		    Scr.MyDisplayWidth - Tmp_win->icon_w_width;
		}
	      /* end keep label on screen. dje 8/7/97 */
	    }
	}
    }
  else
    {
      Relief = GetDecor (Tmp_win, LoReliefGC);
      Shadow = GetDecor (Tmp_win, LoShadowGC);
      TextColor = GetDecor (Tmp_win, LoColors.fore);
      BackColor = GetDecor (Tmp_win, LoColors.back);
      if (Tmp_win->icon_w != None)
	XSetWindowBackground (dpy, Tmp_win->icon_w,
			      GetDecor (Tmp_win,
					titlebar.state[Inactive].u.back));
      /* resize the icon name window */
      if (Tmp_win->icon_w != None)
	{
	  Tmp_win->icon_w_width = Tmp_win->icon_p_width;
	  Tmp_win->icon_xl_loc = Tmp_win->icon_x_loc;
	}
    }
  if ((Tmp_win->flags & ICON_OURS) && (Tmp_win->icon_pixmap_w != None))
    XSetWindowBackground (dpy, Tmp_win->icon_pixmap_w, BackColor);
  /* write the icon label */
  if (Scr.IconFont.font)
    {
      NewFontAndColor (Scr.IconFont.font->fid, TextColor, BackColor);
    }
  else
    {
      NewFontAndColor (0, TextColor, BackColor);
    }

  if (Tmp_win->icon_pixmap_w != None)
    XMoveWindow (dpy, Tmp_win->icon_pixmap_w, Tmp_win->icon_x_loc,
		 Tmp_win->icon_y_loc);
  if (Tmp_win->icon_w != None)
    {
      Tmp_win->icon_w_height = ICON_HEIGHT;
      XMoveResizeWindow (dpy, Tmp_win->icon_w, Tmp_win->icon_xl_loc,
			 Tmp_win->icon_y_loc + Tmp_win->icon_p_height,
			 Tmp_win->icon_w_width, ICON_HEIGHT);

      XClearWindow (dpy, Tmp_win->icon_w);
    }

  if ((Tmp_win->iconPixmap != None) && (!(Tmp_win->flags & SHAPED_ICON)))
    RelieveWindow (Tmp_win, Tmp_win->icon_pixmap_w, 0, 0,
		   Tmp_win->icon_p_width, Tmp_win->icon_p_height,
		   Relief, Shadow, FULL_HILITE);

  /* need to locate the icon pixmap */
  if (Tmp_win->iconPixmap != None)
    {
      if (Tmp_win->iconDepth == Scr.d_depth)
	{
	  XCopyArea (dpy, Tmp_win->iconPixmap, Tmp_win->icon_pixmap_w,
		     Scr.ScratchGC3, 0, 0, Tmp_win->icon_p_width - 4,
		     Tmp_win->icon_p_height - 4, 2, 2);
	}
      else
	XCopyPlane (dpy, Tmp_win->iconPixmap, Tmp_win->icon_pixmap_w,
		    Scr.ScratchGC3, 0, 0, Tmp_win->icon_p_width - 4,
		    Tmp_win->icon_p_height - 4, 2, 2, 1);
    }

  if (Tmp_win->icon_w != None)
    {
      XFontSet fontset = Scr.IconFont.fontset;
      /* text position */
      x = (Tmp_win->icon_w_width - Tmp_win->icon_t_width) / 2;
      if (x < 3)
	x = 3;

      if (fontset)
	XmbDrawString (dpy, Tmp_win->icon_w, fontset, Scr.ScratchGC3, x,
		       Tmp_win->icon_w_height - Scr.IconFont.height +
		       Scr.IconFont.y - 3,
		       Tmp_win->icon_name, strlen (Tmp_win->icon_name));
      else
	XDrawString (dpy, Tmp_win->icon_w, Scr.ScratchGC3, x,
		     Tmp_win->icon_w_height - Scr.IconFont.height +
		     Scr.IconFont.y - 3,
		     Tmp_win->icon_name, strlen (Tmp_win->icon_name));
      RelieveRectangle (Tmp_win->icon_w, 0, 0, Tmp_win->icon_w_width,
			ICON_HEIGHT, Shadow, Relief);
      if (!(Scr.Hilite == Tmp_win))
	RelieveRectangle (Tmp_win->icon_w, 1, 1, Tmp_win->icon_w_width - 2,
			  ICON_HEIGHT - 2, Relief, Shadow);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	RedoIconName - procedure to re-position the icon window and name
 *
 ************************************************************************/
void
RedoIconName (XfwmWindow * Tmp_win)
{
  XFontSet fontset = Scr.IconFont.fontset;

  if (Tmp_win->flags & SUPPRESSICON)
    return;

  if (Tmp_win->icon_w == (int) NULL)
    return;

  if (fontset)
    {
      XRectangle rect1, rect2;
      XmbTextExtents (fontset, Tmp_win->icon_name,
		      strlen (Tmp_win->icon_name), &rect1, &rect2);
      Tmp_win->icon_t_width = rect2.width;
    }
  else
    Tmp_win->icon_t_width = XTextWidth (Scr.IconFont.font, Tmp_win->icon_name,
					strlen (Tmp_win->icon_name));
  /* clear the icon window, and trigger a re-draw via an expose event */
  if (Tmp_win->flags & ICONIFIED)
    XClearArea (dpy, Tmp_win->icon_w, 0, 0, 0, 0, True);
  return;
}

/***********************************************************************
 *
 *  Procedure:
 *	AutoPlace - Find a home for an icon
 *
 ************************************************************************/

#define TEST_1 \
      (((Scr.iconbox == 0) || (Scr.iconbox == 2)) \
      ? \
          ((test_y + temp_h) < (Scr.MyDisplayHeight+base_y)) \
      : \
          ((test_x + temp_w) < (Scr.MyDisplayWidth+base_x)))

#define TEST_2 \
      (((Scr.iconbox == 0) || (Scr.iconbox == 2)) \
      ? \
          ((test_x + temp_w) < (Scr.MyDisplayWidth+base_x)) \
      : \
          ((test_y + temp_h) < (Scr.MyDisplayHeight+base_y)))

/* Test overlapping windows */
#define OVERLAP(x1, y1, x2, y2, x3, y3, x4, y4) \
   (((x1 >= x3) && (x1 <= x4) && (y1 >= y3) && (y1 <= y4)) || \
    ((x2 >= x3) && (x2 <= x4) && (y2 >= y3) && (y2 <= y4)) || \
    ((x1 >= x3) && (x1 <= x4) && (y2 >= y3) && (y2 <= y4)) || \
    ((x2 >= x3) && (x2 <= x4) && (y1 >= y3) && (y1 <= y4)) || \
    ((x3 >= x1) && (x3 <= x2) && (y3 >= y1) && (y3 <= y2)) || \
    ((x4 >= x1) && (x4 <= x2) && (y4 >= y1) && (y4 <= y2)) || \
    ((x3 >= x1) && (x3 <= x2) && (y4 >= y1) && (y4 <= y2)) || \
    ((x4 >= x1) && (x4 <= x2) && (y3 >= y1) && (y3 <= y2)))


Bool
CheckIconPlace (XfwmWindow * t)
{
  XfwmWindow *tw;
  Bool loc_ok = True;

  tw = Scr.XfwmRoot.next;
  MyXGrabServer (dpy);
  while ((tw != (XfwmWindow *) 0) && (loc_ok == True))
    {
      if (tw->Desk == t->Desk)
	{
	  if ((tw->flags & ICONIFIED) &&
	      (tw->icon_w || tw->icon_pixmap_w) && (tw != t))
	    {
	      if (OVERLAP (t->icon_x_loc - Scr.iconspacing,
			   t->icon_y_loc - Scr.iconspacing,
			   t->icon_x_loc + t->icon_p_width + Scr.iconspacing,
			   t->icon_y_loc + t->icon_w_height +
			   t->icon_p_height + Scr.iconspacing, tw->icon_x_loc,
			   tw->icon_y_loc, tw->icon_x_loc + tw->icon_p_width,
			   tw->icon_y_loc + tw->icon_w_height +
			   tw->icon_p_height))
		{
		  loc_ok = False;
		}
	    }
	}
      tw = tw->next;
    }
  MyXUngrabServer (dpy);
  return (loc_ok);
}

void
AutoPlace (XfwmWindow * t, Bool rearrange)
{
  int test_x = 0, test_y = 0, tw, th, tx, ty, temp_h, temp_w;
  int base_x, base_y;
  int width, height;
  XfwmWindow *test_window;
  Bool loc_ok;
  int real_x = 10, real_y = 10;
  int step_x, step_y;

  width = t->icon_p_width;
  height = t->icon_w_height + t->icon_p_height;

  switch (Scr.iconbox)
    {
    case 1:
      base_x = 0;
      base_y = 0;
      step_x = Scr.icongrid;
      step_y = Scr.icongrid;
      break;
    case 2:
      base_x = 0;
      base_y =
	((int) (Scr.MyDisplayHeight - height) / Scr.icongrid) * Scr.icongrid;
      step_x = Scr.icongrid;
      step_y = -Scr.icongrid;
      break;
    case 3:
      base_x =
	((int) (Scr.MyDisplayWidth - width) / Scr.icongrid) * Scr.icongrid;
      base_y = 0;
      step_x = -Scr.icongrid;
      step_y = Scr.icongrid;
      break;
    default:
      base_x = 0;
      base_y = 0;
      step_x = Scr.icongrid;
      step_y = Scr.icongrid;
      break;
    }
  if (t->flags & ICON_MOVED)
    {
      /* just make sure the icon is on this screen */
      t->icon_x_loc = t->icon_x_loc % Scr.MyDisplayWidth + base_x;
      t->icon_y_loc = t->icon_y_loc % Scr.MyDisplayHeight + base_y;
      if (t->icon_x_loc < 0)
	t->icon_x_loc += Scr.MyDisplayWidth;
      if (t->icon_y_loc < 0)
	t->icon_y_loc += Scr.MyDisplayHeight;
      if (t->icon_x_loc > Scr.MyDisplayWidth)
	t->icon_x_loc -= Scr.MyDisplayWidth;
      if (t->icon_y_loc > Scr.MyDisplayHeight)
	t->icon_y_loc -= Scr.MyDisplayHeight;
    }
  loc_ok = False;

  /* check all boxes in order */
  if ((Scr.iconbox == 0) || (Scr.iconbox == 2))
    test_y = base_y;
  else
    test_x = base_x;

  temp_h = height;
  temp_w = width;
  MyXGrabServer (dpy);
  while (TEST_1 && (!loc_ok))
    {
      if ((Scr.iconbox == 0) || (Scr.iconbox == 2))
	test_x = base_x;
      else
	test_y = base_y;
      while (TEST_2 && (!loc_ok))
	{
	  real_x = test_x;
	  real_y = test_y;
	  loc_ok = True;
	  test_window = Scr.XfwmRoot.next;
	  while ((test_window != (XfwmWindow *) 0) && (loc_ok == True))
	    {
	      if (test_window->Desk == t->Desk)
		{
		  if ((test_window->flags & ICONIFIED) &&
		      (test_window->icon_w || test_window->icon_pixmap_w) &&
		      (test_window != t))
		    {
		      tw = test_window->icon_p_width;
		      th =
			test_window->icon_p_height +
			test_window->icon_w_height;
		      tx = test_window->icon_x_loc;
		      ty = test_window->icon_y_loc;

		      if (OVERLAP (tx - Scr.iconspacing,
				   ty - Scr.iconspacing,
				   tx + tw + Scr.iconspacing,
				   ty + th + Scr.iconspacing,
				   real_x,
				   real_y,
				   real_x + width,
				   real_y + height) &&
			  !(rearrange && !(test_window->icon_arranged)
			    && !(test_window->flags & ICON_MOVED)))
			{
			  loc_ok = False;
			}
		    }
		}
	      test_window = test_window->next;
	    }
	  if ((Scr.iconbox == 0) || (Scr.iconbox == 2))
	    test_x += step_x;
	  else
	    test_y += step_y;
	}
      if ((Scr.iconbox == 0) || (Scr.iconbox == 2))
	test_y += step_y;
      else
	test_x += step_x;
    }
  if (loc_ok == False)
    return;
  t->icon_x_loc = real_x;
  t->icon_y_loc = real_y;

  if ((t->icon_pixmap_w) && (t->Desk == Scr.CurrentDesk))
    XMoveWindow (dpy, t->icon_pixmap_w, t->icon_x_loc, t->icon_y_loc);

  t->icon_w_width = t->icon_p_width;
  t->icon_xl_loc = t->icon_x_loc;

  if ((t->icon_w != None) && (t->Desk == Scr.CurrentDesk))
    XMoveResizeWindow (dpy, t->icon_w, t->icon_xl_loc,
		       t->icon_y_loc + t->icon_p_height,
		       t->icon_w_width, ICON_HEIGHT);
  MyXUngrabServer (dpy);
  Broadcast (XFCE_M_ICON_LOCATION, 7, t->w, t->frame,
	     (unsigned long) t,
	     t->icon_x_loc, t->icon_y_loc,
	     t->icon_w_width, t->icon_w_height + t->icon_p_height);
}

/***********************************************************************
 *
 *  Procedure:
 *	GrabIconButtons - grab needed buttons for the icon window
 *
 *  Inputs:
 *	tmp_win - the xfwm window structure to use
 *
 ***********************************************************************/
void
GrabIconButtons (XfwmWindow * tmp_win, Window w)
{
  MyXGrabButton (dpy, AnyButton, 0, w, True,
		 ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
  MyXGrabButton (dpy, AnyButton, AnyModifier, w, True,
		 ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
  return;
}

/***********************************************************************
 *
 *  Procedure:
 *	GrabIconKeys - grab needed keys for the icon window
 *
 *  Inputs:
 *	tmp_win - the xfwm window structure to use
 *
 ***********************************************************************/
void
GrabIconKeys (XfwmWindow * tmp_win, Window w)
{
  Binding *tmp;
  for (tmp = Scr.AllBindings; tmp != NULL; tmp = tmp->NextBinding)
    {
      if ((tmp->Context & C_ICON) && (tmp->IsMouse == 0))
	MyXGrabKey (dpy, tmp->Button_Key, tmp->Modifier, w, True,
		    GrabModeAsync, GrabModeAsync);
    }
  return;
}

/****************************************************************************
 *
 * Looks for a monochrome icon bitmap file
 *
 ****************************************************************************/
void
GetBitmapFile (XfwmWindow * tmp_win)
{
  int HotX, HotY;
  int res;

  res = 0;
  if (check_existfile (tmp_win->icon_bitmap_file))
    res = (XReadBitmapFile (dpy, Scr.Root, tmp_win->icon_bitmap_file,
			    (unsigned int *) &tmp_win->icon_p_width,
			    (unsigned int *) &tmp_win->icon_p_height,
			    &tmp_win->iconPixmap,
			    &HotX, &HotY) != BitmapSuccess);

  if (!res)
    {
      tmp_win->icon_p_width = 0;
      tmp_win->icon_p_height = 0;
    }
}

/****************************************************************************
 *
 * Looks for a color XPM icon file
 *
 ****************************************************************************/
void
GetXPMFile (XfwmWindow * tmp_win)
{
  int res;
#ifdef HAVE_IMLIB
  ImlibImage *im;

  res = 0;
  im = Imlib_load_image (imlib_id, tmp_win->icon_bitmap_file);
  if (!im)
    {
      im = Imlib_create_image_from_xpm_data (imlib_id, default_icon);
    }
  if (im)
    {

      if ((im->rgb_width > 48) || (im->rgb_height > 48))
	{
	  res = Imlib_render (imlib_id, im, 48, 48);
	  tmp_win->icon_p_width = tmp_win->icon_p_height = 48;
	}
      else
	{
	  res = Imlib_render (imlib_id, im, im->rgb_width, im->rgb_height);
	  tmp_win->icon_p_width = im->rgb_width;
	  tmp_win->icon_p_height = im->rgb_height;
	}

      tmp_win->iconPixmap = Imlib_move_image (imlib_id, im);
      tmp_win->icon_maskPixmap = Imlib_move_mask (imlib_id, im);
      Imlib_kill_image (imlib_id, im);
    }
#else
  XWindowAttributes root_attr;
  XpmAttributes xpm_attributes;
  static XpmColorSymbol none_color = { NULL, "None", (Pixel) 0 };

  XGetWindowAttributes (dpy, Scr.Root, &root_attr);
  xpm_attributes.colormap = root_attr.colormap;
  xpm_attributes.closeness = 40000;	/* Allow for "similar" colors */
  xpm_attributes.valuemask = XpmSize | XpmColormap | XpmCloseness;
  xpm_attributes.colorsymbols = &none_color;
  xpm_attributes.numsymbols = 1;

  res = 0;
  if (check_existfile (tmp_win->icon_bitmap_file))
    res = (XpmReadFileToPixmap (dpy, Scr.Root, tmp_win->icon_bitmap_file,
				&tmp_win->iconPixmap,
				&tmp_win->icon_maskPixmap,
				&xpm_attributes) == XpmSuccess);
  if (!res)
    res = (XpmCreatePixmapFromData (dpy, Scr.Root, default_icon,
				    &tmp_win->iconPixmap,
				    &tmp_win->icon_maskPixmap,
				    &xpm_attributes) == XpmSuccess);

  if (res)
    {
      tmp_win->icon_p_width = xpm_attributes.width;
      tmp_win->icon_p_height = xpm_attributes.height;
    }
#endif
  if (res)
    {
      tmp_win->flags |= PIXMAP_OURS;
      tmp_win->iconDepth = Scr.d_depth;
      if (ShapesSupported && tmp_win->icon_maskPixmap)
	tmp_win->flags |= SHAPED_ICON;
    }
}

/****************************************************************************
 *
 * Looks for an application supplied icon window
 *
 ****************************************************************************/
void
GetIconWindow (XfwmWindow * tmp_win)
{
  /* We are guaranteed that wmhints is non-null when calling this
   * routine */
  if (XGetGeometry (dpy, tmp_win->wmhints->icon_window, &JunkRoot,
		    &JunkX, &JunkY, (unsigned int *) &tmp_win->icon_p_width,
		    (unsigned int *) &tmp_win->icon_p_height,
		    &JunkBW, &JunkDepth) == 0)
    {
      xfwm_msg (ERR, "GetIconWindow", "Help! Bad Icon Window!");
    }
  tmp_win->icon_p_width += JunkBW << 1;
  tmp_win->icon_p_height += JunkBW << 1;
  /*
   * Now make the new window the icon window for this window,
   * and set it up to work as such (select for key presses
   * and button presses/releases, set up the contexts for it,
   * and define the cursor for it).
   */
  tmp_win->icon_pixmap_w = tmp_win->wmhints->icon_window;
  if (ShapesSupported)
    {
      if (tmp_win->wmhints->flags & IconMaskHint)
	{
	  tmp_win->flags |= SHAPED_ICON;
	  tmp_win->icon_maskPixmap = tmp_win->wmhints->icon_mask;
	}
    }
  /* Make sure that the window is a child of the root window ! */
  /* Olwais screws this up, maybe others do too! */
  XReparentWindow (dpy, tmp_win->icon_pixmap_w, Scr.Root, 0, 0);
  tmp_win->flags &= ~ICON_OURS;
}


/****************************************************************************
 *
 * Looks for an application supplied bitmap or pixmap
 *
 ****************************************************************************/
void
GetIconBitmap (XfwmWindow * tmp_win)
{
  /* We are guaranteed that wmhints is non-null when calling this
   * routine */
  XGetGeometry (dpy, tmp_win->wmhints->icon_pixmap, &JunkRoot, &JunkX, &JunkY,
		(unsigned int *) &tmp_win->icon_p_width,
		(unsigned int *) &tmp_win->icon_p_height, &JunkBW,
		&JunkDepth);
  tmp_win->iconPixmap = tmp_win->wmhints->icon_pixmap;
  tmp_win->iconDepth = JunkDepth;
  if (ShapesSupported)
    {
      if (tmp_win->wmhints->flags & IconMaskHint)
	{
	  tmp_win->flags |= SHAPED_ICON;
	  tmp_win->icon_maskPixmap = tmp_win->wmhints->icon_mask;
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	DeIconify a window
 *
 ***********************************************************************/
void
DeIconify (XfwmWindow * tmp_win)
{
  XfwmWindow *t;

  if (!tmp_win)
    return;

  if (tmp_win->Desk == Scr.CurrentDesk)
    Animate (tmp_win->icon_x_loc,
	     tmp_win->icon_y_loc,
	     tmp_win->icon_w_width,
	     tmp_win->icon_w_height,
	     tmp_win->frame_x,
	     tmp_win->frame_y,
	     tmp_win->frame_width,
	     tmp_win->flags & SHADED ?
	     tmp_win->title_height + 2 * tmp_win->boundary_width :
	     tmp_win->frame_height);
  /* now de-iconify transients */
  MyXGrabServer (dpy);
  for (t = Scr.XfwmRoot.next; t != NULL; t = t->next)
    {
      if ((t == tmp_win) ||
	  ((t->flags & TRANSIENT) && (t->transientfor == tmp_win->w)))
	{
	  t->flags |= MAPPED;
	  if (Scr.Hilite == t)
	    SetBorder (t, False, True, True, None);

	  if (!(t->flags & SHADED))
	    {
	      XMapWindow (dpy, t->w);
	      XMapWindow (dpy, t->Parent);
	    }
	  if (t->Desk == Scr.CurrentDesk)
	    {
	      XMapWindow (dpy, t->frame);
	      t->flags |= MAP_PENDING;
	    }
	  SetMapStateProp (t, NormalState);
	  t->flags &= ~ICONIFIED;
	  t->flags &= ~ICON_UNMAPPED;
	  t->flags &= ~STARTICONIC;
	  if (!(t->flags & SHADED))
	    XRaiseWindow (dpy, t->w);
	  if (t->icon_w)
	    XUnmapWindow (dpy, t->icon_w);
	  if (t->icon_pixmap_w)
	    XUnmapWindow (dpy, t->icon_pixmap_w);
	  Broadcast (XFCE_M_DEICONIFY, 3, t->w, t->frame, (unsigned long) t,
		     0, 0, 0, 0);
	}
    }

  if (!(tmp_win->flags & SHADED))
    {
      RaiseWindow (tmp_win);
      FocusOn (tmp_win, 1);
    }
  MyXUngrabServer (dpy);
  return;
}


/****************************************************************************
 *
 * Iconifies the selected window
 *
 ****************************************************************************/
void
Iconify (XfwmWindow * tmp_win, int def_x, int def_y)
{
  XfwmWindow *t;
  XWindowAttributes winattrs;
  unsigned long eventMask;

  if (!tmp_win)
    return;

  XGetWindowAttributes (dpy, tmp_win->w, &winattrs);
  eventMask = winattrs.your_event_mask;
  if ((tmp_win->Desk == Scr.CurrentDesk) && (tmp_win)
      && (tmp_win == Scr.Hilite))
    {
      if (tmp_win->next)
	SetFocus (tmp_win->next->w, tmp_win->next, 1);
      else if (tmp_win->prev)
	SetFocus (tmp_win->prev->w, tmp_win->prev, 1);
    }

  /* iconify transients first */
  MyXGrabServer (dpy);
  for (t = Scr.XfwmRoot.next; t != NULL; t = t->next)
    {
      if ((t == tmp_win) ||
	  ((t->flags & TRANSIENT) && (t->transientfor == tmp_win->w)))
	{
	  /*
	   * Prevent the receipt of an UnmapNotify, since that would
	   * cause a transition to the Withdrawn state.
	   */
	  t->flags &= ~MAPPED;
	  XSelectInput (dpy, t->w, eventMask & ~StructureNotifyMask);
	  XUnmapWindow (dpy, t->w);
	  XSelectInput (dpy, t->w, eventMask);
	  XUnmapWindow (dpy, t->frame);
	  t->DeIconifyDesk = t->Desk;
	  if (t->icon_w)
	    XUnmapWindow (dpy, t->icon_w);
	  if (t->icon_pixmap_w)
	    XUnmapWindow (dpy, t->icon_pixmap_w);

	  SetMapStateProp (t, IconicState);
	  SetBorder (t, False, False, False, None);
	  if (t != tmp_win)
	    {
	      t->flags |= ICONIFIED | ICON_UNMAPPED;

	      Broadcast (XFCE_M_ICONIFY, 7, t->w, t->frame,
			 (unsigned long) t,
			 -10000, -10000,
			 t->icon_w_width,
			 t->icon_w_height + t->icon_p_height);
	      BroadcastConfig (XFCE_M_CONFIGURE_WINDOW, t);
	    }
	}
    }
  MyXUngrabServer (dpy);
  if (tmp_win->icon_w == None)
    {
      if (tmp_win->flags & ICON_MOVED)
	CreateIconWindow (tmp_win, tmp_win->icon_x_loc, tmp_win->icon_y_loc);
      else
	CreateIconWindow (tmp_win, def_x, def_y);
    }
  tmp_win->icon_arranged = False;
  /* if no pixmap we want icon width to change to text width every iconify */
  if ((tmp_win->icon_w != None) && (tmp_win->icon_pixmap_w == None))
    {
      XFontSet fontset = Scr.IconFont.fontset;
      if (fontset)
	{
	  XRectangle rect1, rect2;
	  XmbTextExtents (fontset, tmp_win->icon_name,
			  strlen (tmp_win->icon_name), &rect1, &rect2);
	  tmp_win->icon_t_width = rect2.width;
	}
      else
	tmp_win->icon_t_width =
	  XTextWidth (Scr.IconFont.font, tmp_win->icon_name,
		      strlen (tmp_win->icon_name));
      tmp_win->icon_p_width = tmp_win->icon_t_width + 6;
      tmp_win->icon_w_width = tmp_win->icon_p_width;
    }

  if ((!(tmp_win->flags & STARTICONIC)) &&
      (!(tmp_win->flags & ICON_MOVED) || !CheckIconPlace (tmp_win)))
    AutoPlace (tmp_win, False);
  tmp_win->flags |= ICONIFIED;
  tmp_win->flags &= ~ICON_UNMAPPED;
  Broadcast (XFCE_M_ICONIFY, 7, tmp_win->w, tmp_win->frame,
	     (unsigned long) tmp_win,
	     tmp_win->icon_x_loc, tmp_win->icon_y_loc,
	     tmp_win->icon_w_width,
	     tmp_win->icon_w_height + tmp_win->icon_p_height);
  BroadcastConfig (XFCE_M_CONFIGURE_WINDOW, tmp_win);


  if (tmp_win->Desk == Scr.CurrentDesk)
    {
      LowerWindow (tmp_win);
      if (!(tmp_win->flags & STARTICONIC))
	Animate (tmp_win->frame_x,
		 tmp_win->frame_y,
		 tmp_win->frame_width,
		 tmp_win->flags & SHADED ?
		 tmp_win->title_height + 2 * tmp_win->boundary_width :
		 tmp_win->frame_height,
		 tmp_win->icon_x_loc,
		 tmp_win->icon_y_loc,
		 tmp_win->icon_w_width, tmp_win->icon_w_height);

      if (tmp_win->icon_w != None)
	XMapWindow (dpy, tmp_win->icon_w);

      if (tmp_win->icon_pixmap_w != None);
      XMapWindow (dpy, tmp_win->icon_pixmap_w);
      KeepOnTop ();

      if ((tmp_win->Desk == Scr.CurrentDesk) && (tmp_win == Scr.Focus))
	{
	  if (Scr.PreviousFocus == Scr.Focus)
	    Scr.PreviousFocus = NULL;
	  if (tmp_win->next)
	    SetFocus (tmp_win->next->w, tmp_win->next, 1);
	  else if (tmp_win->prev)
	    SetFocus (tmp_win->prev->w, tmp_win->prev, 1);
	  else
	    SetFocus (Scr.NoFocusWin, NULL, 1);
	}
    }

  return;
}



/****************************************************************************
 *
 * This is used to tell applications which windows on the screen are
 * top level appication windows, and which windows are the icon windows
 * that go with them.
 *
 ****************************************************************************/
void
SetMapStateProp (XfwmWindow * tmp_win, int state)
{
  unsigned long data[2];	/* "suggested" by ICCCM version 1 */

  data[0] = (unsigned long) state;
  data[1] = (unsigned long) tmp_win->icon_w;
/*  data[2] = (unsigned long) tmp_win->icon_pixmap_w; */

  XChangeProperty (dpy, tmp_win->w, _XA_WM_STATE, _XA_WM_STATE, 32,
		   PropModeReplace, (unsigned char *) data, 2);
  return;
}
