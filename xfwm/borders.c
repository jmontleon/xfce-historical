
/****************************************************************************
 * This module is all original code 
 * by Rob Nation 
 * Copyright 1993, Robert Nation
 *     You may use this code for any purpose, as long as the original
 *     copyright remains in the source code and all documentation
 ****************************************************************************/


/***********************************************************************
 *
 * xfwm window border drawing code
 *
 ***********************************************************************/

#include "configure.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>


#include "xfwm.h"
#include "menus.h"
#include "misc.h"
#include "parse.h"
#include "screen.h"
#include "module.h"

#include <X11/extensions/shape.h>

void DrawButton (XfwmWindow * t,
		 Window win,
		 int W,
		 int H,
		 ButtonFace * bf,
		 GC ReliefGC, GC ShadowGC,
		 Boolean inverted,
		 int stateflags);

void DrawLinePattern (Window win,
		      GC ReliefGC,
		      GC ShadowGC,
		      struct vector_coords *coords,
		      int w, int h);
void
RelieveBand (Window win, int x, int y, int w, int h, GC Hilite, GC Shadow);

#define GetButtonState(window) (onoroff ? Active : Inactive)

/* macro to change window background color/pixmap */
#define ChangeWindowColor(window,valuemask) {				\
        if(NewColor)							\
        {								\
          XChangeWindowAttributes(dpy,window,valuemask, &attributes);	\
          XClearWindow(dpy,window);					\
        }								\
      }

extern Window PressedW;
XGCValues Globalgcv;
unsigned long Globalgcm;
/****************************************************************************
 *
 * Redraws the windows borders
 *
 ****************************************************************************/
void
SetBorder (XfwmWindow * t, Bool onoroff, Bool force, Bool Mapped,
	   Window expose_win)
{
  int y, i, x;
  GC ReliefGC, ShadowGC;
  Pixel BorderColor, BackColor;
  Pixmap BackPixmap, TextColor;
  Bool NewColor = False;
  XSetWindowAttributes attributes;
  unsigned long valuemask;
  static unsigned int corners[4];
  Window w;

  corners[0] = TOP_HILITE | LEFT_HILITE;
  corners[1] = TOP_HILITE | RIGHT_HILITE;
  corners[2] = BOTTOM_HILITE | LEFT_HILITE;
  corners[3] = BOTTOM_HILITE | RIGHT_HILITE;

  if (!t)
    return;

  if (onoroff)
    {

      if ((!force) && (Scr.Hilite == t))
	return;

      if ((Scr.Hilite != t) || (force))
	NewColor = True;

      /* make sure that the previously highlighted window got unhighlighted */
      if ((Scr.Hilite != t) && (Scr.Hilite != NULL))
	SetBorder (Scr.Hilite, False, False, True, None);

      /* set the keyboard focus */
      if ((Mapped) && (t->flags & MAPPED) && (Scr.Hilite != t))
	w = t->w;
      else if ((t->flags & ICONIFIED) &&
	       (Scr.Hilite != t) && (!(t->flags & SUPPRESSICON)))
	w = t->icon_w;
      Scr.Hilite = t;

      TextColor = GetDecor (t, HiColors.fore);
      BackPixmap = Scr.gray_pixmap;
      BackColor = GetDecor (t, HiColors.back);
      ReliefGC = GetDecor (t, HiReliefGC);
      ShadowGC = GetDecor (t, HiShadowGC);
      BorderColor = GetDecor (t, HiRelief.back);
    }
  else
    {

      if ((!force) && (Scr.Hilite != t))
	return;

      if (Scr.Hilite == t)
	{
	  Scr.Hilite = NULL;
	  NewColor = True;
	}
      if (force)
	NewColor = True;
      TextColor = GetDecor (t, LoColors.fore);
      BackPixmap = Scr.light_gray_pixmap;
      BackColor = GetDecor (t, LoColors.back);
      ReliefGC = GetDecor (t, LoReliefGC);
      ShadowGC = GetDecor (t, LoShadowGC);
      BorderColor = GetDecor (t, LoRelief.back);
    }

  if (t->flags & ICONIFIED)
    {
      DrawIconWindow (t);
      return;
    }
  valuemask = CWBorderPixel;
  attributes.border_pixel = BorderColor;
  if (Scr.d_depth < 2)
    {
      attributes.background_pixmap = BackPixmap;
      valuemask |= CWBackPixmap;
    }
  else
    {
      attributes.background_pixel = BackColor;
      valuemask |= CWBackPixel;
    }

  if (t->flags & (TITLE | BORDER))
    {
      XSetWindowBorder (dpy, t->Parent, BorderColor);
      XSetWindowBorder (dpy, t->frame, BorderColor);
    }
  if (t->flags & TITLE)
    {
      XChangeWindowAttributes(dpy,t->title_w,valuemask, &attributes);
      for (i = 0; i < Scr.nr_left_buttons; ++i)
	{
	  if (t->left_w[i] != None)
	    {
	      enum ButtonState bs = GetButtonState (t->left_w[i]);
	      ButtonFace *bf = &GetDecor (t, left_buttons[i].state[bs]);
	      ChangeWindowColor (t->left_w[i], valuemask);
	      if (flush_expose (t->left_w[i]) || (expose_win == t->left_w[i]) ||
		  (expose_win == None)
		  || NewColor
		)
		{
		  int inverted = PressedW == t->left_w[i];
		  XClearWindow (dpy, t->left_w[i]);
		  if (bf->style & UseTitleStyle)
		    {
		      ButtonFace *tsbf = &GetDecor (t, titlebar.state[bs]);
		      for (; tsbf; tsbf = tsbf->next)
			DrawButton (t, t->left_w[i],
				    t->title_height, t->title_height,
				    tsbf, ReliefGC, ShadowGC,
			     inverted, GetDecor (t, left_buttons[i].flags));
		    }
		  for (; bf; bf = bf->next)
		    DrawButton (t, t->left_w[i],
				t->title_height, t->title_height,
				bf, ReliefGC, ShadowGC,
			     inverted, GetDecor (t, left_buttons[i].flags));

		}
	    }
	}
      for (i = 0; i < Scr.nr_right_buttons; ++i)
	{
	  if (t->right_w[i] != None)
	    {
	      enum ButtonState bs = GetButtonState (t->right_w[i]);
	      ButtonFace *bf = &GetDecor (t, right_buttons[i].state[bs]);
	      ChangeWindowColor (t->right_w[i], valuemask);
	      if (flush_expose (t->right_w[i]) || (expose_win == t->right_w[i]) ||
		  (expose_win == None)
		  || NewColor
		)
		{
		  int inverted = PressedW == t->right_w[i];
		  XClearWindow (dpy, t->right_w[i]);
		  if (bf->style & UseTitleStyle)
		    {
		      ButtonFace *tsbf = &GetDecor (t, titlebar.state[bs]);
		      for (; tsbf; tsbf = tsbf->next)
			DrawButton (t, t->right_w[i],
				    t->title_height, t->title_height,
				    tsbf, ReliefGC, ShadowGC,
			    inverted, GetDecor (t, right_buttons[i].flags));
		    }
		  for (; bf; bf = bf->next)
		    DrawButton (t, t->right_w[i],
				t->title_height, t->title_height,
				bf, ReliefGC, ShadowGC,
			    inverted, GetDecor (t, right_buttons[i].flags));

		}
	    }
	}
      SetTitleBar (t, onoroff, False); 
    }

  if (t->flags & BORDER)
    {
      /* draw relief lines */
      y = t->frame_height - 2 * t->corner_width;
      x = t->frame_width - 2 * t->corner_width + t->bw;

      for (i = 0; i < 4; i++)
	{
	  int vertical = i % 2;
	  ChangeWindowColor (t->sides[i], valuemask);
	  if ((flush_expose (t->sides[i])) || (expose_win == t->sides[i]) ||
	      (expose_win == None) || NewColor)
	    {
	      GC sgc, rgc;

	      rgc = ReliefGC;
	      sgc = ShadowGC;
	      RelieveWindow (t, t->sides[i], 0, 0,
			     ((vertical) ? t->boundary_width : x),
			     ((vertical) ? y : t->boundary_width),
			     rgc, sgc, (0x0001 << i)
		);
	    }
	  ChangeWindowColor (t->corners[i], valuemask);
	  if ((flush_expose (t->corners[i])) || (expose_win == t->corners[i]) ||
	      (expose_win == None) || NewColor)
	    {
	      GC rgc, sgc;

	      rgc = ReliefGC;
	      sgc = ShadowGC;
	      RelieveWindow (t, t->corners[i], 0, 0, t->corner_width,
		      ((i / 2) ? t->corner_width + t->bw : t->corner_width),
			     rgc, sgc, corners[i]);

	    }
	}
    }
  else
    /* no decorative border */
    {
      /* for mono - put a black border on 
       * for color, make it the color of the decoration background */
      if (t->boundary_width < 2)
	{
	  flush_expose (t->frame);
	  if (Scr.d_depth < 2)
	    {
	      XSetWindowBorder (dpy, t->frame, TextColor);
	      XSetWindowBorder (dpy, t->Parent, TextColor);
	      XSetWindowBackgroundPixmap (dpy, t->frame, BackPixmap);
	      XClearWindow (dpy, t->frame);
	      XSetWindowBackgroundPixmap (dpy, t->Parent, BackPixmap);
	      XClearWindow (dpy, t->Parent);
	    }
	  else
	    {
	      XSetWindowBorder (dpy, t->frame, BorderColor);
	      XClearWindow (dpy, t->frame);
	      XSetWindowBackground (dpy, t->Parent, BorderColor);
	      XSetWindowBorder (dpy, t->Parent, BorderColor);
	      XClearWindow (dpy, t->Parent);
	      XSetWindowBorder (dpy, t->w, BorderColor);
	    }
	}
      else
	{
	  GC rgc, sgc;

	  XSetWindowBorder (dpy, t->Parent, BorderColor);
	  XSetWindowBorder (dpy, t->frame, BorderColor);

	  rgc = ReliefGC;
	  sgc = ShadowGC;
	  ChangeWindowColor (t->frame, valuemask);
	  if ((flush_expose (t->frame)) || (expose_win == t->frame) ||
	      (expose_win == None) || NewColor)
	    {
	      if (t->boundary_width > 2)
		{
		  RelieveWindow (t, t->frame, 0, 0, t->frame_width + t->bw,
				 t->frame_height + t->bw, rgc, sgc,
				 TOP_HILITE | LEFT_HILITE | RIGHT_HILITE |
				 BOTTOM_HILITE);
		}
	      else
		{
		  RelieveWindow (t, t->frame, 0, 0, t->frame_width + t->bw,
				 t->frame_height + t->bw, rgc, rgc,
				 TOP_HILITE | LEFT_HILITE | RIGHT_HILITE |
				 BOTTOM_HILITE);
		}
	    }
	  else
	    {
	      XSetWindowBackground (dpy, t->Parent, BorderColor);
	    }
	}
    }
}

/****************************************************************************
 *
 *  Redraws buttons (veliaa@rpi.edu)
 *
 ****************************************************************************/
void
DrawButton (XfwmWindow * t, Window win, int w, int h,
	    ButtonFace * bf, GC ReliefGC, GC ShadowGC,
	    Boolean inverted, int stateflags)
{
  register int type = bf->style & ButtonFaceTypeMask;

  switch (type)
    {
    case SimpleButton:
      break;

    case SolidButton:
      XSetWindowBackground (dpy, win, bf->u.back);
      flush_expose (win);
      XClearWindow (dpy, win);
      break;

    case VectorButton:
      if ((stateflags & MWMDecorMaximize)
	  && (t->flags & MAXIMIZED))
	DrawLinePattern (win,
			 ShadowGC, ReliefGC,
			 &bf->vector,
			 w, h);
      else if ((stateflags & MWMDecorSticky)
	       && (t->flags & STICKY))
	DrawLinePattern (win,
			 ShadowGC, ReliefGC,
			 &bf->vector,
			 w, h);
      else
	DrawLinePattern (win,
			 ReliefGC, ShadowGC,
			 &bf->vector,
			 w, h);
      break;

    case GradButton:
      {
	XRectangle bounds;
	bounds.x = bounds.y = 0;
	bounds.width = w;
	bounds.height = h;
	flush_expose (win);

	{
	  register int i = 0, dw = bounds.width / bf->u.grad.npixels + 1;
	  while (i < bf->u.grad.npixels)
	    {
	      register unsigned short x = i * bounds.width / bf->u.grad.npixels;
	      XSetForeground (dpy, Scr.TransMaskGC, bf->u.grad.pixels[i++]);
	      XFillRectangle (dpy, win, Scr.TransMaskGC,
			      bounds.x + x, bounds.y,
			      dw, bounds.height);
	    }
	}
      }
      break;

    default:
      xfwm_msg (ERR, "DrawButton", "unknown button type");
      break;
    }
}

/****************************************************************************
 *
 *  Redraws just the title bar
 *
 ****************************************************************************/
void
SetTitleBar (XfwmWindow * t, Bool onoroff, Bool NewTitle)
{
  int hor_off, w;
  enum ButtonState title_state;
  ButtonFaceStyle tb_style;
  int tb_flags;
  GC ReliefGC, ShadowGC;
  Pixel Forecolor, BackColor;

  if (!t)
    return;
  if (!(t->flags & TITLE))
    return;

  if (onoroff)
    {
      Forecolor = GetDecor (t, HiColors.fore);
      BackColor = GetDecor (t, HiColors.back);
      ReliefGC = GetDecor (t, HiReliefGC);
      ShadowGC = GetDecor (t, HiShadowGC);
    }
  else
    {
      Forecolor = GetDecor (t, LoColors.fore);
      BackColor = GetDecor (t, LoColors.back);
      ReliefGC = GetDecor (t, LoReliefGC);
      ShadowGC = GetDecor (t, LoShadowGC);
    }

  flush_expose (t->title_w);

  if (t->name != (char *) NULL)
    {
      w = XTextWidth (GetDecor (t, WindowFont.font), t->name, strlen (t->name));
      if (w > t->title_width - 12)
	w = t->title_width - 4;
      if (w < 0)
	w = 0;
    }
  else
    w = 0;

  title_state = GetButtonState (t->title_w);
  tb_style = GetDecor (t, titlebar.state[title_state].style);
  tb_flags = GetDecor (t, titlebar.flags);
  hor_off = 10;

  NewFontAndColor (GetDecor (t, WindowFont.font->fid), Forecolor, BackColor);

  /* the next bit tries to minimize redraw based upon compilation options (veliaa@rpi.edu) */
  /* for mono, we clear an area in the title bar where the window
   * title goes, so that its more legible. For color, no need */
  if (Scr.d_depth < 2)
    {
      RelieveWindow (t, t->title_w, 0, 0, hor_off - 2, t->title_height,
		     ReliefGC, ShadowGC, BOTTOM_HILITE);
      RelieveWindow (t, t->title_w, hor_off + w + 2, 0,
		     t->title_width - w - hor_off - 2, t->title_height,
		     ReliefGC, ShadowGC, BOTTOM_HILITE);
      XFillRectangle (dpy, t->title_w,
		      (PressedW == t->title_w ? ShadowGC : ReliefGC),
		      hor_off - 2, 0, w + 4, t->title_height);

      XDrawLine (dpy, t->title_w, ShadowGC, hor_off + w + 1, 0, hor_off + w + 1,
		 t->title_height);
      if (t->name != (char *) NULL)
	XDrawString (dpy, t->title_w, Scr.ScratchGC3, hor_off,
		     GetDecor (t, WindowFont.y) + 1,
		     t->name, strlen (t->name));
    }
  else
    {
      ButtonFace *bf = &GetDecor (t, titlebar.state[title_state]);
      /* draw compound titlebar (veliaa@rpi.edu) */
      for (; bf; bf = bf->next)
	    DrawButton (t, t->title_w, t->title_width, t->title_height,
			bf, ShadowGC, ReliefGC, True, 0);
      if (t->name != (char *) NULL)
	XDrawString (dpy, t->title_w, Scr.ScratchGC3, hor_off,
		     GetDecor (t, WindowFont.y) + 1,
		     t->name, strlen (t->name));
      if (onoroff)
         RelieveRectangle(t->title_w,0,0,t->title_width,t->title_height,
			    ShadowGC,ReliefGC);
    }
}

/****************************************************************************
 *
 *  Draws a little pattern within a window (more complex)
 *
 ****************************************************************************/
void
DrawLinePattern (Window win,
		 GC ReliefGC,
		 GC ShadowGC,
		 struct vector_coords *coords,
		 int w, int h)
{
  int i = 1;
  for (; i < coords->num; ++i)
    {
      XDrawLine (dpy, win,
		 coords->line_style[i] ? ReliefGC : ShadowGC,
		 w * coords->x[i - 1] / 100,
		 h * coords->y[i - 1] / 100,
		 w * coords->x[i] / 100,
		 h * coords->y[i] / 100);
    }
}

/****************************************************************************
 *
 *  Draws the relief pattern around a window
 *
 ****************************************************************************/
inline void
RelieveWindow (XfwmWindow * t, Window win,
	       int x, int y, int w, int h,
	       GC ReliefGC, GC ShadowGC, int hilite)
{
  XSegment seg[4];
  int i;
  int edge;

  edge = 0;
  if ((win == t->sides[0]) || (win == t->sides[1]) ||
      (win == t->sides[2]) || (win == t->sides[3]))
    edge = -1;
  else if (win == t->corners[0])
    edge = 1;
  else if (win == t->corners[1])
    edge = 2;
  else if (win == t->corners[2])
    edge = 3;
  else if (win == t->corners[3])
    edge = 4;

  /* window sides */
  if (edge == -1)
    {
      switch (hilite)
	{
	case LEFT_HILITE:
	  i = 0;
	  seg[i].x1 = x + 1;
	  seg[i].y1 = y;
	  seg[i].x2 = x + 1;
	  seg[i++].y2 = h + y - 1;
	  XDrawSegments (dpy, win, ReliefGC, seg, i);
	  break;

	case TOP_HILITE:
	  i = 0;
	  seg[i].x1 = x;
	  seg[i].y1 = y + 1;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = y + 1;
	  XDrawSegments (dpy, win, ReliefGC, seg, i);
	  break;

	case RIGHT_HILITE:
	  i = 0;
	  seg[i].x1 = w + x - 1;
	  seg[i].y1 = y;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = h + y - 1;
	  seg[i].x1 = w + x - 2;
	  seg[i].y1 = y;
	  seg[i].x2 = w + x - 2;
	  seg[i++].y2 = h + y - 1;
	  XDrawSegments (dpy, win, ShadowGC, seg, i);
	  break;

	case BOTTOM_HILITE:
	  i = 0;
	  seg[i].x1 = x;
	  seg[i].y1 = h + y - 1;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = h + y - 1;
	  seg[i].x1 = x;
	  seg[i].y1 = h + y - 2;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = h + y - 2;
	  XDrawSegments (dpy, win, ShadowGC, seg, i);
	  break;

	}
      return;
    }

  /* corners */
  if (edge >= 1 && edge <= 4)
    {
      switch (edge)
	{
	case 1:
	  i = 0;
	  seg[i].x1 = x + 1;
	  seg[i].y1 = y + 1;
	  seg[i].x2 = x + 1;
	  seg[i++].y2 = h + y - 1;
	  seg[i].x1 = x + 1;
	  seg[i].y1 = y + 1;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = y + 1;
	  XDrawSegments (dpy, win, ReliefGC, seg, i);
	  break;

	case 2:
	  i = 0;
	  seg[i].x1 = x;
	  seg[i].y1 = y + 1;
	  seg[i].x2 = w + x - 2;
	  seg[i++].y2 = y + 1;
	  XDrawSegments (dpy, win, ReliefGC, seg, i);
	  i = 0;
	  seg[i].x1 = w + x - 1;
	  seg[i].y1 = y;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = h + y - 1;
	  seg[i].x1 = w + x - 2;
	  seg[i].y1 = y + 1;
	  seg[i].x2 = w + x - 2;
	  seg[i++].y2 = h + y - 1;
	  XDrawSegments (dpy, win, ShadowGC, seg, i);
	  break;

	case 3:
	  i = 0;
	  seg[i].x1 = x + 1;
	  seg[i].y1 = y;
	  seg[i].x2 = x + 1;
	  seg[i++].y2 = h + y - 2;
	  XDrawSegments (dpy, win, ReliefGC, seg, i);
	  i = 0;
	  seg[i].x1 = x;
	  seg[i].y1 = h + y - 1;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = h + y - 1;
	  seg[i].x1 = x + 1;
	  seg[i].y1 = h + y - 2;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = h + y - 2;
	  XDrawSegments (dpy, win, ShadowGC, seg, i);
	  break;

	case 4:
	  i = 0;
	  seg[i].x1 = w + x - 1;
	  seg[i].y1 = y;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = h + y - 1;
	  seg[i].x1 = x;
	  seg[i].y1 = h + y - 1;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = h + y - 1;
	  seg[i].x1 = w + x - 2;
	  seg[i].y1 = y;
	  seg[i].x2 = w + x - 2;
	  seg[i++].y2 = h + y - 2;
	  seg[i].x1 = x;
	  seg[i].y1 = h + y - 2;
	  seg[i].x2 = w + x - 2;
	  seg[i++].y2 = h + y - 2;
	  XDrawSegments (dpy, win, ShadowGC, seg, i);
	  break;

	}
      return;
    }

  if (!edge)
    {
      i = 0;

      seg[i].x1 = x + 1;
      seg[i].y1 = y + 1;

      seg[i].x2 = w + x - 2;
      seg[i++].y2 = y + 1;

      seg[i].x1 = x + 1;
      seg[i].y1 = y + 1;

      seg[i].x2 = x + 1;
      seg[i++].y2 = h + y - 2;

      XDrawSegments (dpy, win, ReliefGC, seg, i);

      i = 0;

      seg[i].x1 = x;
      seg[i].y1 = y + h - 1;

      seg[i].x2 = w + x - 1;
      seg[i++].y2 = y + h - 1;

      seg[i].x1 = x + 1;
      seg[i].y1 = y + h - 2;

      seg[i].x2 = w + x - 2;
      seg[i++].y2 = y + h - 2;

      seg[i].x1 = x + w - 1;
      seg[i].y1 = y;

      seg[i].x2 = x + w - 1;
      seg[i++].y2 = y + h - 1;

      seg[i].x1 = x + w - 2;
      seg[i].y1 = y + 1;

      seg[i].x2 = x + w - 2;
      seg[i++].y2 = y + h - 2;

      XDrawSegments (dpy, win, ShadowGC, seg, i);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *      Setupframe - set window sizes, this was called from either
 *              AddWindow, EndResize, or HandleConfigureNotify.
 *
 *  Inputs:
 *      tmp_win - the XfwmWindow pointer
 *      x       - the x coordinate of the upper-left outer corner of the frame
 *      y       - the y coordinate of the upper-left outer corner of the frame
 *      w       - the width of the frame window w/o border
 *      h       - the height of the frame window w/o border
 *
 *  Special Considerations:
 *      This routine will check to make sure the window is not completely
 *      off the display, if it is, it'll bring some of it back on.
 *
 *      The tmp_win->frame_XXX variables should NOT be updated with the
 *      values of x,y,w,h prior to calling this routine, since the new
 *      values are compared against the old to see whether a synthetic
 *      ConfigureNotify event should be sent.  (It should be sent if the
 *      window was moved but not resized.)
 *
 ************************************************************************/

void
SetupFrame (XfwmWindow * tmp_win, int x, int y, int w, int h, Bool sendEvent)
{
  XEvent client_event;
  XWindowChanges frame_wc, xwc;
  unsigned long frame_mask, xwcm;
  int cx, cy, i;
  Bool Resized = False, Moved = False;
  int xwidth, ywidth, left, right;

  /* if windows is not being maximized, save size in case of maximization */
  if (!(tmp_win->flags & MAXIMIZED)
    )
    {
      tmp_win->orig_x = x;
      tmp_win->orig_y = y;
      tmp_win->orig_wd = w;
      tmp_win->orig_ht = h;
    }
  if (x >= Scr.MyDisplayWidth + Scr.VxMax - Scr.Vx - 16)
    x = Scr.MyDisplayWidth + Scr.VxMax - Scr.Vx - 16;
  if (y >= Scr.MyDisplayHeight + Scr.VyMax - Scr.Vy - 16)
    y = Scr.MyDisplayHeight + Scr.VyMax - Scr.Vy - 16;

  if ((w != tmp_win->frame_width) || (h != tmp_win->frame_height))
    Resized = True;
  if ((x != tmp_win->frame_x || y != tmp_win->frame_y))
    Moved = True;

  /*
   * According to the July 27, 1988 ICCCM draft, we should send a
   * "synthetic" ConfigureNotify event to the client if the window
   * was moved but not resized.
   */
  if (Moved && !Resized)
    sendEvent = True;

  if (Resized)
    {
      left = tmp_win->nr_left_buttons;
      right = tmp_win->nr_right_buttons;

      if (tmp_win->flags & TITLE)
	tmp_win->title_height = GetDecor (tmp_win, TitleHeight) + tmp_win->bw;

      tmp_win->title_width = w -
	(left + right) * tmp_win->title_height
	- 2 * tmp_win->boundary_width + tmp_win->bw;


      if (tmp_win->title_width < 1)
	tmp_win->title_width = 1;

      if (tmp_win->flags & TITLE)
	{
	  xwcm = CWWidth | CWX | CWY | CWHeight;
	  tmp_win->title_x = tmp_win->boundary_width +
	    (left) * tmp_win->title_height;
	  if (tmp_win->title_x >= w - tmp_win->boundary_width)
	    tmp_win->title_x = -10;
	  tmp_win->title_y = tmp_win->boundary_width;

	  xwc.width = tmp_win->title_width;

	  xwc.height = tmp_win->title_height;
	  xwc.x = tmp_win->title_x;
	  xwc.y = tmp_win->title_y;
	  XConfigureWindow (dpy, tmp_win->title_w, xwcm, &xwc);


	  xwcm = CWX | CWY | CWHeight | CWWidth;
	  xwc.height = tmp_win->title_height;
	  xwc.width = tmp_win->title_height;

	  xwc.y = tmp_win->boundary_width;
	  xwc.x = tmp_win->boundary_width;
	  for (i = 0; i < Scr.nr_left_buttons; i++)
	    {
	      if (tmp_win->left_w[i] != None)
		{
		  if (xwc.x + tmp_win->title_height < w - tmp_win->boundary_width)
		    XConfigureWindow (dpy, tmp_win->left_w[i], xwcm, &xwc);
		  else
		    {
		      xwc.x = -tmp_win->title_height;
		      XConfigureWindow (dpy, tmp_win->left_w[i], xwcm, &xwc);
		    }
		  xwc.x += tmp_win->title_height;
		}
	    }

	  xwc.x = w - tmp_win->boundary_width + tmp_win->bw;
	  for (i = 0; i < Scr.nr_right_buttons; i++)
	    {
	      if (tmp_win->right_w[i] != None)
		{
		  xwc.x -= tmp_win->title_height;
		  if (xwc.x > tmp_win->boundary_width)
		    XConfigureWindow (dpy, tmp_win->right_w[i], xwcm, &xwc);
		  else
		    {
		      xwc.x = -tmp_win->title_height;
		      XConfigureWindow (dpy, tmp_win->right_w[i], xwcm, &xwc);
		    }
		}
	    }
	}

      if (tmp_win->flags & BORDER)
	{
	  tmp_win->corner_width = GetDecor (tmp_win, TitleHeight) + tmp_win->bw +
	    tmp_win->boundary_width;

	  if (w < 2 * tmp_win->corner_width)
	    tmp_win->corner_width = w / 3;
	  if ((h < 2 * tmp_win->corner_width)
	    )
	    tmp_win->corner_width = h / 3;
	  xwidth = w - 2 * tmp_win->corner_width + tmp_win->bw;
	  ywidth = h - 2 * tmp_win->corner_width;
	  xwcm = CWWidth | CWHeight | CWX | CWY;
	  if (xwidth < 2)
	    xwidth = 2;
	  if (ywidth < 2)
	    ywidth = 2;

	  for (i = 0; i < 4; i++)
	    {
	      if (i == 0)
		{
		  xwc.x = tmp_win->corner_width;
		  xwc.y = 0;
		  xwc.height = tmp_win->boundary_width;
		  xwc.width = xwidth;
		}
	      else if (i == 1)
		{
		  xwc.x = w - tmp_win->boundary_width + tmp_win->bw;
		  xwc.y = tmp_win->corner_width;
		  xwc.width = tmp_win->boundary_width;
		  xwc.height = ywidth;

		}
	      else if (i == 2)
		{
		  xwc.x = tmp_win->corner_width;
		  xwc.y = h - tmp_win->boundary_width + tmp_win->bw;
		  xwc.height = tmp_win->boundary_width + tmp_win->bw;
		  xwc.width = xwidth;
		}
	      else
		{
		  xwc.x = 0;
		  xwc.y = tmp_win->corner_width;
		  xwc.width = tmp_win->boundary_width;
		  xwc.height = ywidth;
		}
	      XConfigureWindow (dpy, tmp_win->sides[i], xwcm, &xwc);
	    }

	  xwcm = CWX | CWY | CWWidth | CWHeight;
	  xwc.width = tmp_win->corner_width;
	  xwc.height = tmp_win->corner_width;
	  for (i = 0; i < 4; i++)
	    {
	      if (i % 2)
		xwc.x = w - tmp_win->corner_width + tmp_win->bw;
	      else
		xwc.x = 0;

	      if (i / 2)
		xwc.y = h - tmp_win->corner_width;
	      else
		xwc.y = 0;

	      XConfigureWindow (dpy, tmp_win->corners[i], xwcm, &xwc);
	    }

	}
    }
  tmp_win->attr.width = w - 2 * tmp_win->boundary_width;
  tmp_win->attr.height = h - tmp_win->title_height
    - 2 * tmp_win->boundary_width;
  /* may need to omit the -1 for shaped windows, next two lines */
  cx = tmp_win->boundary_width - tmp_win->bw;
  cy = tmp_win->title_height + tmp_win->boundary_width - tmp_win->bw;

  XResizeWindow (dpy, tmp_win->w, tmp_win->attr.width,
		 tmp_win->attr.height);
  XMoveResizeWindow (dpy, tmp_win->Parent, cx, cy,
		     tmp_win->attr.width, tmp_win->attr.height);
  /* 
   * fix up frame and assign size/location values in tmp_win
   */
  frame_wc.x = tmp_win->frame_x = x;
  frame_wc.y = tmp_win->frame_y = y;
  frame_wc.width = tmp_win->frame_width = w;
  frame_wc.height = tmp_win->frame_height = h;
  frame_mask = (CWX | CWY | CWWidth | CWHeight);
  XConfigureWindow (dpy, tmp_win->frame, frame_mask, &frame_wc);

  if (ShapesSupported)
    {
      if ((Resized) && (tmp_win->wShaped))
	{
	  SetShape (tmp_win, w);
	}
    }
  XSync (dpy, 0);
  if (sendEvent
    )
    {
      client_event.type = ConfigureNotify;
      client_event.xconfigure.display = dpy;
      client_event.xconfigure.event = tmp_win->w;
      client_event.xconfigure.window = tmp_win->w;

      client_event.xconfigure.x = x + tmp_win->boundary_width;
      client_event.xconfigure.y = y + tmp_win->title_height +
	tmp_win->boundary_width;
      client_event.xconfigure.width = w - 2 * tmp_win->boundary_width;
      client_event.xconfigure.height = h - 2 * tmp_win->boundary_width -
	tmp_win->title_height;

      client_event.xconfigure.border_width = tmp_win->bw;
      /* Real ConfigureNotify events say we're above title window, so ... */
      /* what if we don't have a title ????? */
      client_event.xconfigure.above = tmp_win->frame;
      client_event.xconfigure.override_redirect = False;
      XSendEvent (dpy, tmp_win->w, False, StructureNotifyMask, &client_event);
    }
    XSync (dpy, 0);

  BroadcastConfig (M_CONFIGURE_WINDOW, tmp_win);
}


/****************************************************************************
 *
 * Sets up the shaped window borders 
 * 
 ****************************************************************************/
void
SetShape (XfwmWindow * tmp_win, int w)
{
  if (ShapesSupported)
    {
      XRectangle rect;

      XShapeCombineShape (dpy, tmp_win->frame, ShapeBounding,
			  tmp_win->boundary_width,
			  tmp_win->title_height + tmp_win->boundary_width,
			  tmp_win->w,
			  ShapeBounding, ShapeSet);
      if (tmp_win->title_w)
	{
	  /* windows w/ titles */
	  rect.x = tmp_win->boundary_width;
	  rect.y = tmp_win->title_y;
	  rect.width = w - 2 * tmp_win->boundary_width + tmp_win->bw;
	  rect.height = tmp_win->title_height;


	  XShapeCombineRectangles (dpy, tmp_win->frame, ShapeBounding,
				   0, 0, &rect, 1, ShapeUnion, Unsorted);
	}
    }
}
