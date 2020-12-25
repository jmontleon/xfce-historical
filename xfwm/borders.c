
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

#ifdef DMALLOC
#  include "dmalloc.h"
#endif

void DrawButton (XfwmWindow * t,
		 Window win,
		 int W,
		 int H,
		 ButtonFace * bf,
		 GC ReliefGC, GC ShadowGC, Boolean inverted, int stateflags);

void DrawLinePattern (Window win,
		      GC ReliefGC,
		      GC ShadowGC,
		      struct vector_coords *coords, int w, int h);

void
RelieveRoundedRectangle (Window win,
			 int x, int y, int w, int h, GC Hilite, GC Shadow);

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
  int y, i, x, fh;
  GC ReliefGC, ShadowGC;
  Pixel BorderColor, BackColor;
  Pixmap TextColor;
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

  if (t->flags & SHADED)
    fh = t->title_height + 2 * t->boundary_width;
  else
    fh = t->frame_height;

  if (onoroff)
    {
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
      BackColor = GetDecor (t, HiColors.back);
      ReliefGC = GetDecor (t, HiReliefGC);
      ShadowGC = GetDecor (t, HiShadowGC);
      BorderColor = GetDecor (t, HiRelief.back);
    }
  else
    {

      if (Scr.Hilite == t)
	{
	  Scr.Hilite = NULL;
	  NewColor = True;
	}
      if (force)
	NewColor = True;
      TextColor = GetDecor (t, LoColors.fore);
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

  attributes.background_pixel = BackColor;
  valuemask |= CWBackPixel;

  if (t->flags & TITLE)
    {
      for (i = 0; i < Scr.nr_left_buttons; ++i)
	{
	  if (t->left_w[i] != None)
	    {
	      enum ButtonState bs = GetButtonState (t->left_w[i]);
	      ButtonFace *bf = &GetDecor (t, left_buttons[i].state[bs]);
	      ChangeWindowColor (t->left_w[i], valuemask);
	      if (flush_expose (t->left_w[i]) || (expose_win == t->left_w[i])
		  || (expose_win == None) || NewColor)
		{
		  int inverted = PressedW == t->left_w[i];
		  if (bf->style & UseTitleStyle)
		    {
		      ButtonFace *tsbf = &GetDecor (t, titlebar.state[bs]);
		      for (; tsbf; tsbf = tsbf->next)
			DrawButton (t, t->left_w[i],
				    t->title_height, t->title_height,
				    tsbf, ReliefGC, ShadowGC,
				    inverted, GetDecor (t,
							left_buttons
							[i].flags));
		    }
		  for (; bf; bf = bf->next)
		    DrawButton (t, t->left_w[i],
				t->title_height, t->title_height,
				bf, ReliefGC, ShadowGC,
				inverted, GetDecor (t,
						    left_buttons[i].flags));

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
	      if (flush_expose (t->right_w[i])
		  || (expose_win == t->right_w[i]) || (expose_win == None)
		  || NewColor)
		{
		  int inverted = PressedW == t->right_w[i];
		  if (bf->style & UseTitleStyle)
		    {
		      ButtonFace *tsbf = &GetDecor (t, titlebar.state[bs]);
		      for (; tsbf; tsbf = tsbf->next)
			DrawButton (t, t->right_w[i],
				    t->title_height, t->title_height,
				    tsbf, ReliefGC, ShadowGC,
				    inverted, GetDecor (t,
							right_buttons
							[i].flags));
		    }
		  for (; bf; bf = bf->next)
		    DrawButton (t, t->right_w[i],
				t->title_height, t->title_height,
				bf, ReliefGC, ShadowGC,
				inverted, GetDecor (t,
						    right_buttons[i].flags));

		}
	    }
	}
      if (flush_expose (t->title_w) || (expose_win == t->title_w)
	  || (expose_win == None) || NewColor)
	SetTitleBar (t, onoroff, False);
    }

  if (t->flags & BORDER)
    {
      /* draw relief lines */
      y = fh - 2 * t->corner_width;
      x = t->frame_width - 2 * t->corner_width + t->bw;
      for (i = 0; i < 4; i++)
	{
	  int vertical = i % 2;
	  if ((flush_expose (t->sides[i])) || (expose_win == t->sides[i]) ||
	      (expose_win == None) || NewColor)
	    {
	      GC sgc, rgc;

	      rgc = ReliefGC;
	      sgc = ShadowGC;
	      RelieveWindow (t, t->sides[i], 0, 0,
			     ((vertical) ? t->boundary_width : x),
			     ((vertical) ? y : t->boundary_width),
			     rgc, sgc, (0x0001 << i));
	    }
	  ChangeWindowColor (t->corners[i], valuemask);
	  if ((flush_expose (t->corners[i])) || (expose_win == t->corners[i])
	      || (expose_win == None) || NewColor)
	    {
	      GC rgc, sgc;

	      rgc = ReliefGC;
	      sgc = ShadowGC;
	      RelieveWindow (t, t->corners[i], 0, 0, t->corner_width,
			     ((i / 2) ? t->corner_width +
			      t->bw : t->corner_width), rgc, sgc, corners[i]);

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
  int type = bf->style & ButtonFaceTypeMask;
  GC BackGC = NULL;

  if (w - t->boundary_width + t->bw <= 0)
    return;

  switch (type)
    {
    case SimpleButton:
      break;

    case SolidButton:
      {
	XRectangle bounds;
	bounds.x = bounds.y = 0;
	bounds.width = w;
	bounds.height = h;
	flush_expose (win);
	BackGC =
	  ((Scr.Hilite == t) ? GetDecor (t, HiBackGC) :
	   GetDecor (t, LoBackGC));

	XSetForeground (dpy, Scr.TransMaskGC, bf->u.back);
	if (w > h)
	  {
	    XFillRectangle (dpy, win, BackGC, 0, 0, bounds.height / 2,
			    bounds.height);

	    XFillArc (dpy, win, Scr.TransMaskGC, 0,
		      0, bounds.height, bounds.height - 1, 90 * 64, 180 * 64);
	    XFillRectangle (dpy, win, Scr.TransMaskGC,
			    bounds.height / 2, 1,
			    bounds.width - bounds.height, bounds.height - 1);

	    XFillRectangle (dpy, win, BackGC,
			    bounds.width - (bounds.height / 2), 0,
			    bounds.height / 2, bounds.height);

	    XFillArc (dpy, win, Scr.TransMaskGC,
		      bounds.width - bounds.height - 1, 0, bounds.height,
		      bounds.height - 1, 90 * 64, -180 * 64);
	  }
	else
	  XFillRectangle (dpy, win, Scr.TransMaskGC, 0, 0, w, h - 1);
      }
      break;

    case VectorButton:
      BackGC =
	((Scr.Hilite == t) ? GetDecor (t, HiBackGC) : GetDecor (t, LoBackGC));
      if (((stateflags & MWMDecorMaximize) && (t->flags & MAXIMIZED))
	  || ((stateflags & DecorSticky) && (t->flags & STICKY))
	  || ((stateflags & DecorShaded) && (t->flags & SHADED)))
	{
	  if (Scr.Options & BitmapButtons)
	    {
	      XSetClipOrigin (dpy, BackGC, (w - 16) / 2, (h - 16) / 2);
	      XCopyPlane (dpy, bf->bitmap_pressed, win, BackGC, 0, 0, 15, 15,
			  (w - 16) / 2, (h - 16) / 2, 1);
	    }
	  else
	    {
	      DrawLinePattern (win, ShadowGC, ReliefGC, &bf->vector, w, h);
	    }
	}
      else
	{
	  if (Scr.Options & BitmapButtons)
	    {
	      XSetClipOrigin (dpy, BackGC, (w - 16) / 2, (h - 16) / 2);
	      XCopyPlane (dpy, bf->bitmap, win, BackGC, 0, 0, 15, 15,
			  (w - 16) / 2, (h - 16) / 2, 1);
	    }
	  else
	    {
	      DrawLinePattern (win, ReliefGC, ShadowGC, &bf->vector, w, h);
	    }
	}

      if (inverted)
	RelieveRectangle (win, 0, 0, w - 1, h - 1, ShadowGC, ReliefGC);

      break;

    case GradButton:
      {
	XRectangle bounds;
	bounds.x = bounds.y = 0;
	bounds.width = w;
	bounds.height = h;
	flush_expose (win);
	BackGC =
	  ((Scr.Hilite == t) ? GetDecor (t, HiBackGC) :
	   GetDecor (t, LoBackGC));

	{
	  int i = 0, dw =
	    ((w > h) ? bounds.width : 0) / bf->u.grad.npixels + 1;
	  XSetForeground (dpy, Scr.TransMaskGC, bf->u.grad.pixels[i]);
	  if (w > h)
	    {
	      XFillRectangle (dpy, win, BackGC, 0, 0, bounds.height / 2,
			      bounds.height);

	      XFillArc (dpy, win, Scr.TransMaskGC, 0,
			0,
			bounds.height, bounds.height - 1, 90 * 64, 180 * 64);
	    }
	  while (i < bf->u.grad.npixels)
	    {
	      unsigned short x = ((w > h) ? (bounds.height / 2) : 0)
		+ i * (bounds.width -
		       ((w > h) ? bounds.height : 0)) / bf->u.grad.npixels;
	      XSetForeground (dpy, Scr.TransMaskGC, bf->u.grad.pixels[i++]);
	      XFillRectangle (dpy, win, Scr.TransMaskGC,
			      bounds.x + x, bounds.y + 1,
			      dw, bounds.height - 2);
	    }
	  if (w > h)
	    {
	      XFillRectangle (dpy, win, BackGC,
			      bounds.width - (bounds.height / 2), 0,
			      bounds.height / 2, bounds.height);

	      XFillArc (dpy, win, Scr.TransMaskGC,
			bounds.width - bounds.height - 1, 0, bounds.height,
			bounds.height - 1, 90 * 64, -180 * 64);
	    }
	}
      }
      break;

    default:
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
  ButtonFace *bf;

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
      XFontSet fontset = GetDecor (t, WindowFont.fontset);
      if (fontset)
	{
	  XRectangle rect1, rect2;
	  XmbTextExtents (fontset, t->name, strlen (t->name), &rect1, &rect2);
	  w = rect2.width;
	}
      else
	w =
	  XTextWidth (GetDecor (t, WindowFont.font), t->name,
		      strlen (t->name));
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

  if (GetDecor (t, WindowFont.font))
    {
      NewFontAndColor (GetDecor (t, WindowFont.font->fid), Forecolor,
		       BackColor);
    }
  else
    {
      NewFontAndColor (0, Forecolor, BackColor);
    }

  bf = &GetDecor (t, titlebar.state[title_state]);
  /* draw compound titlebar (veliaa@rpi.edu) */
  for (; bf; bf = bf->next)
    DrawButton (t, t->title_w, t->title_width, t->title_height,
		bf, ShadowGC, ReliefGC, True, 0);
  if (t->name != (char *) NULL)
    {
      XFontSet fontset = GetDecor (t, WindowFont.fontset);
      if (fontset)
	XmbDrawString (dpy, t->title_w, fontset, Scr.ScratchGC3, hor_off,
		       GetDecor (t, WindowFont.y) + 1,
		       t->name, strlen (t->name));
      else
	XDrawString (dpy, t->title_w, Scr.ScratchGC3, hor_off,
		     GetDecor (t, WindowFont.y) + 1,
		     t->name, strlen (t->name));
    }
  RelieveRoundedRectangle (t->title_w, 0, 0, t->title_width, t->title_height,
			   ShadowGC, ReliefGC);
  if (!onoroff)
    RelieveRoundedRectangle (t->title_w, 1, 1, t->title_width - 2,
			     t->title_height - 2, ReliefGC, ShadowGC);
}

void
RelieveRoundedRectangle (Window win, int x, int y, int w, int h, GC Hilite,
			 GC Shadow)
{
  if (w <= 0)
    return;
  if (w > h)
    {
      XDrawLine (dpy, win, Hilite, x + h / 2, y, w + x - h / 2, y);
      XDrawArc (dpy, win, Hilite, x, y, h, h - 1, 90 * 64, 135 * 64);
      XDrawArc (dpy, win, Shadow, x, y, h, h - 1, 270 * 64, -45 * 64);

      XDrawLine (dpy, win, Shadow, x + h / 2, h + y - 1, w + x - h / 2,
		 h + y - 1);
      XDrawArc (dpy, win, Shadow, w + x - h - 1, y, h, h - 1, 270 * 64,
		135 * 64);
      XDrawArc (dpy, win, Hilite, w + x - h - 1, y, h, h - 1, 90 * 64,
		-45 * 64);
    }
  else
    RelieveRectangle (win, x, y, w, h, Hilite, Shadow);
}

/****************************************************************************
 *
 *  Draws a little pattern within a window (more complex)
 *
 ****************************************************************************/
void
DrawLinePattern (Window win,
		 GC ReliefGC,
		 GC ShadowGC, struct vector_coords *coords, int w, int h)
{
  int i = 1;

  for (; i < coords->num; ++i)
    {
      XDrawLine (dpy, win,
		 coords->line_style[i] ? ReliefGC : ShadowGC,
		 (w * coords->x[i - 1] / 100.0 + .5) - 1,
		 (h * coords->y[i - 1] / 100.0 + .5) - 1,
		 (w * coords->x[i] / 100.0 + .5) - 1,
		 (h * coords->y[i] / 100.0 + .5) - 1);
    }
}

/****************************************************************************
 *
 *  Draws the relief pattern around a window
 *
 ****************************************************************************/
void
RelieveWindow (XfwmWindow * t, Window win,
	       int x, int y, int w, int h,
	       GC ReliefGC, GC ShadowGC, int hilite)
{
  XSegment seg[4];
  GC BackGC = NULL;
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
      BackGC =
	((Scr.Hilite == t) ? GetDecor (t, HiBackGC) : GetDecor (t, LoBackGC));
      switch (hilite)
	{
	case LEFT_HILITE:
	  XFillRectangle (dpy, win, BackGC, x + 2, y, w - 1, h + y + 1);
	  i = 0;
	  seg[i].x1 = x;
	  seg[i].y1 = y;
	  seg[i].x2 = x;
	  seg[i++].y2 = h + y;
	  XDrawSegments (dpy, win, Scr.BlackGC, seg, i);
	  i = 0;
	  seg[i].x1 = x + 1;
	  seg[i].y1 = y;
	  seg[i].x2 = x + 1;
	  seg[i++].y2 = h + y;
	  XDrawSegments (dpy, win, ReliefGC, seg, i);
	  break;

	case TOP_HILITE:
	  XFillRectangle (dpy, win, BackGC, x, y + 2, w + x + 1, h - 1);
	  i = 0;
	  seg[i].x1 = x;
	  seg[i].y1 = y;
	  seg[i].x2 = w + x;
	  seg[i++].y2 = y;
	  XDrawSegments (dpy, win, Scr.BlackGC, seg, i);
	  i = 0;
	  seg[i].x1 = x;
	  seg[i].y1 = y + 1;
	  seg[i].x2 = w + x;
	  seg[i++].y2 = y + 1;
	  XDrawSegments (dpy, win, ReliefGC, seg, i);
	  break;

	case RIGHT_HILITE:
	  XFillRectangle (dpy, win, BackGC, x, y, w + x - 2, h + y + 1);
	  i = 0;
	  seg[i].x1 = w + x - 1;
	  seg[i].y1 = y;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = h + y;
	  XDrawSegments (dpy, win, Scr.BlackGC, seg, i);
	  i = 0;
	  seg[i].x1 = w + x - 2;
	  seg[i].y1 = y;
	  seg[i].x2 = w + x - 2;
	  seg[i++].y2 = h + y;
	  XDrawSegments (dpy, win, ShadowGC, seg, i);
	  break;

	case BOTTOM_HILITE:
	  XFillRectangle (dpy, win, BackGC, x, y, w + x + 1, h + y - 2);
	  i = 0;
	  seg[i].x1 = x;
	  seg[i].y1 = h + y - 1;
	  seg[i].x2 = w + x;
	  seg[i++].y2 = h + y - 1;
	  XDrawSegments (dpy, win, Scr.BlackGC, seg, i);
	  i = 0;
	  seg[i].x1 = x;
	  seg[i].y1 = h + y - 2;
	  seg[i].x2 = w + x;
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
	  seg[i].x1 = x;
	  seg[i].y1 = y;
	  seg[i].x2 = x;
	  seg[i++].y2 = h + y - 1;
	  seg[i].x1 = x;
	  seg[i].y1 = y;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = y;
	  XDrawSegments (dpy, win, Scr.BlackGC, seg, i);
	  i = 0;
	  seg[i].x1 = x + 1;
	  seg[i].y1 = y + 1;
	  seg[i].x2 = x + 1;
	  seg[i++].y2 = h + y;
	  seg[i].x1 = x + 1;
	  seg[i].y1 = y + 1;
	  seg[i].x2 = w + x;
	  seg[i++].y2 = y + 1;
	  XDrawSegments (dpy, win, ReliefGC, seg, i);
	  break;

	case 2:
	  i = 0;
	  i = 0;
	  seg[i].x1 = x;
	  seg[i].y1 = y + 1;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = y + 1;
	  XDrawSegments (dpy, win, ReliefGC, seg, i);
	  i = 0;
	  seg[i].x1 = w + x - 2;
	  seg[i].y1 = y + 3;
	  seg[i].x2 = w + x - 2;
	  seg[i++].y2 = h + y;
	  XDrawSegments (dpy, win, ShadowGC, seg, i);
	  i = 0;
	  seg[i].x1 = w + x - 1;
	  seg[i].y1 = y + 1;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = h + y;
	  seg[i].x1 = x;
	  seg[i].y1 = y;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = y;
	  XDrawSegments (dpy, win, Scr.BlackGC, seg, i);
	  break;

	case 3:
	  i = 0;
	  seg[i].x1 = x + 1;
	  seg[i].y1 = y;
	  seg[i].x2 = x + 1;
	  seg[i++].y2 = h + y - 1;
	  XDrawSegments (dpy, win, ReliefGC, seg, i);
	  i = 0;
	  seg[i].x1 = x;
	  seg[i].y1 = h + y - 1;
	  seg[i].x2 = w + x - 1;
	  seg[i++].y2 = h + y - 1;
	  seg[i].x1 = x;
	  seg[i].y1 = y;
	  seg[i].x2 = x;
	  seg[i++].y2 = h + y - 1;
	  XDrawSegments (dpy, win, Scr.BlackGC, seg, i);
	  i = 0;
	  seg[i].x1 = x + 3;
	  seg[i].y1 = h + y - 2;
	  seg[i].x2 = w + x;
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
	  XDrawSegments (dpy, win, Scr.BlackGC, seg, i);
	  i = 0;
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

      seg[i].x1 = x;
      seg[i].y1 = y;

      seg[i].x2 = w + x - 1;
      seg[i++].y2 = y;

      seg[i].x1 = x;
      seg[i].y1 = y;

      seg[i].x2 = x;
      seg[i++].y2 = h + y - 1;

      XDrawSegments (dpy, win, ReliefGC, seg, i);

      i = 0;

      seg[i].x1 = x;
      seg[i].y1 = y + h - 1;

      seg[i].x2 = w + x - 1;
      seg[i++].y2 = y + h - 1;

      seg[i].x1 = x + w - 1;
      seg[i].y1 = y;

      seg[i].x2 = x + w - 1;
      seg[i++].y2 = y + h - 1;

      XDrawSegments (dpy, win, Scr.BlackGC, seg, i);
      i = 0;

      seg[i].x1 = x + 2;
      seg[i].y1 = y + h - 2;

      seg[i].x2 = w + x - 2;
      seg[i++].y2 = y + h - 2;

      seg[i].x1 = x + w - 2;
      seg[i].y1 = y + 2;

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
SetupFrame (XfwmWindow * tmp_win, int x, int y, int w, int h,
	    Bool sendEvent, Bool broadcast)
{
  XWindowChanges winwc;
  Bool Resized = False, Moved = False;
  int xwidth, ywidth, left, right, horig;
  int cx, cy, i;
  unsigned long winmask;

  horig = h;

  if (tmp_win->flags & TITLE)
    tmp_win->title_height = GetDecor (tmp_win, TitleHeight) + tmp_win->bw;

  if (tmp_win->flags & SHADED)
    h = tmp_win->title_height + 2 * tmp_win->boundary_width;

  /* if windows is not being maximized, save size in case of maximization */
  if (!(tmp_win->flags & MAXIMIZED))
    {
      tmp_win->orig_x = x;
      tmp_win->orig_y = y;
      tmp_win->orig_wd = w;
      tmp_win->orig_ht = horig;
    }
  if (x >= Scr.MyDisplayWidth - 16)
    x = Scr.MyDisplayWidth - 16;
  if (y >= Scr.MyDisplayHeight - 16)
    y = Scr.MyDisplayHeight - 16;
  if (x <= 16 - w)
    x = 16 - w;
  if (y <= 16 - h)
    y = 16 - h;

  if ((w != tmp_win->frame_width) || (h != tmp_win->frame_height))
    Resized = True;
  if ((x != tmp_win->frame_x || y != tmp_win->frame_y))
    Moved = True;

  left = tmp_win->nr_left_buttons;
  right = tmp_win->nr_right_buttons;

  tmp_win->title_width = w -
    (left + right) * tmp_win->title_height
    - 2 * tmp_win->boundary_width + tmp_win->bw;

  if (tmp_win->title_width < 1)
    tmp_win->title_width = 1;

  /*
   * According to the July 27, 1988 ICCCM draft, we should send a
   * "synthetic" ConfigureNotify event to the client if the window
   * was moved but not resized.
   */
  if (Moved && !Resized)
    sendEvent = True;

  winmask = (CWX | CWY | CWWidth | CWHeight);
  if (Resized)
    {
      if (tmp_win->flags & TITLE)
	{
	  tmp_win->title_x = tmp_win->boundary_width +
	    (left) * tmp_win->title_height;
	  if (tmp_win->title_x >= w - tmp_win->boundary_width)
	    tmp_win->title_x = -10;
	  tmp_win->title_y = tmp_win->boundary_width;

	  winwc.width = tmp_win->title_width;

	  winwc.height = tmp_win->title_height;
	  winwc.x = tmp_win->title_x;
	  winwc.y = tmp_win->title_y;
	  XMoveResizeWindow (dpy, tmp_win->title_w, winwc.x, winwc.y, winwc.width,
			     winwc.height);
          /*
	  XConfigureWindow(dpy, tmp_win->title_w, winmask, &winwc);
           */


	  winwc.height = tmp_win->title_height;
	  winwc.width = tmp_win->title_height;

	  winwc.y = tmp_win->boundary_width;
	  winwc.x = tmp_win->boundary_width;
	  for (i = 0; i < Scr.nr_left_buttons; i++)
	    {
	      if (tmp_win->left_w[i] != None)
		{
		  if (winwc.x + tmp_win->title_height <
		      w - tmp_win->boundary_width)
		    XMoveResizeWindow (dpy, tmp_win->left_w[i], winwc.x, winwc.y,
				       winwc.width, winwc.height);
                    /*
		    XConfigureWindow(dpy, tmp_win->left_w[i], winmask, &winwc);
                     */
		  else
		    {
		      winwc.x = -tmp_win->title_height;
		      XMoveResizeWindow (dpy, tmp_win->left_w[i], winwc.x,
					 winwc.y, winwc.width, winwc.height);
                      /*
		      XConfigureWindow(dpy, tmp_win->left_w[i], winmask, &winwc);
                       */
		    }
		  winwc.x += tmp_win->title_height;
		}
	    }

	  winwc.x = w - tmp_win->boundary_width + tmp_win->bw;
	  for (i = 0; i < Scr.nr_right_buttons; i++)
	    {
	      if (tmp_win->right_w[i] != None)
		{
		  winwc.x -= tmp_win->title_height;
		  if (winwc.x > tmp_win->boundary_width)
		    XMoveResizeWindow (dpy, tmp_win->right_w[i], winwc.x, winwc.y,
				       winwc.width, winwc.height);
                    /*
		    XConfigureWindow(dpy, tmp_win->right_w[i], winmask, &winwc);
                     */
		  else
		    {
		      winwc.x = -tmp_win->title_height;
		      XMoveResizeWindow (dpy, tmp_win->right_w[i], winwc.x,
					 winwc.y, winwc.width, winwc.height);
                      /*
		      XConfigureWindow(dpy, tmp_win->right_w[i], winmask, &winwc);
                       */
		    }
		}
	    }
	}

      if (tmp_win->flags & BORDER)
	{
	  tmp_win->corner_width =
	    GetDecor (tmp_win,
		      TitleHeight) + tmp_win->bw + tmp_win->boundary_width;

	  if (w < 2 * tmp_win->corner_width)
	    tmp_win->corner_width = w / 3;
	  if ((h < 2 * tmp_win->corner_width))
	    tmp_win->corner_width = h / 3;
	  xwidth = w - 2 * tmp_win->corner_width + tmp_win->bw;
	  ywidth = h - 2 * tmp_win->corner_width;
	  if (xwidth < 2)
	    xwidth = 2;
	  if (ywidth < 2)
	    ywidth = 2;

	  for (i = 0; i < 4; i++)
	    {
	      if (i == 0)
		{
		  winwc.x = tmp_win->corner_width;
		  winwc.y = 0;
		  winwc.height = tmp_win->boundary_width;
		  winwc.width = xwidth;
		}
	      else if (i == 1)
		{
		  winwc.x = w - tmp_win->boundary_width + tmp_win->bw;
		  winwc.y = tmp_win->corner_width;
		  winwc.width = tmp_win->boundary_width;
		  winwc.height = ywidth;

		}
	      else if (i == 2)
		{
		  winwc.x = tmp_win->corner_width;
		  winwc.y = h - tmp_win->boundary_width + tmp_win->bw;
		  winwc.height = tmp_win->boundary_width + tmp_win->bw;
		  winwc.width = xwidth;
		}
	      else
		{
		  winwc.x = 0;
		  winwc.y = tmp_win->corner_width;
		  winwc.width = tmp_win->boundary_width;
		  winwc.height = ywidth;
		}
	      XMoveResizeWindow (dpy, tmp_win->sides[i], winwc.x, winwc.y,
				 winwc.width, winwc.height);
              /*
	      XConfigureWindow(dpy, tmp_win->sides[i], winmask, &winwc);
               */
	    }

	  winwc.width = tmp_win->corner_width;
	  winwc.height = tmp_win->corner_width;
	  for (i = 0; i < 4; i++)
	    {
	      if (i % 2)
		winwc.x = w - tmp_win->corner_width + tmp_win->bw;
	      else
		winwc.x = 0;

	      if (i / 2)
		winwc.y = h - tmp_win->corner_width;
	      else
		winwc.y = 0;

              XMoveResizeWindow (dpy, tmp_win->corners[i], winwc.x, winwc.y,
				 winwc.width, winwc.height);
	      /*
	      XConfigureWindow(dpy, tmp_win->corners[i], winmask, &winwc);
               */
	    }
	}
    }

  tmp_win->attr.width = w - 2 * tmp_win->boundary_width;
  tmp_win->attr.height = horig - tmp_win->title_height
    - 2 * tmp_win->boundary_width;
  /* may need to omit the -1 for shaped windows, next two lines */
  cx = tmp_win->boundary_width - tmp_win->bw;
  cy = tmp_win->title_height + tmp_win->boundary_width - tmp_win->bw;

  XResizeWindow (dpy, tmp_win->w, tmp_win->attr.width, tmp_win->attr.height);
  XMoveResizeWindow (dpy, tmp_win->Parent, cx, cy,
		     tmp_win->attr.width, tmp_win->attr.height);
  /* 
   * fix up frame and assign size/location values in tmp_win
   */
  winwc.x = tmp_win->frame_x = tmp_win->shade_x = x;
  winwc.y = tmp_win->frame_y = tmp_win->shade_y = y;
  winwc.width = tmp_win->frame_width = tmp_win->shade_width = w;
  tmp_win->shade_height = tmp_win->title_height + 2 * tmp_win->boundary_width;
  tmp_win->frame_height = horig;
  winwc.height = h;
  /* 
  XConfigureWindow (dpy, tmp_win->frame, winmask, &winwc);
   */
  XMoveResizeWindow (dpy, tmp_win->frame, winwc.x, winwc.y,
		     winwc.width, winwc.height);

  if (ShapesSupported)
    {
      if ((Resized) && (tmp_win->wShaped))
	SetShape (tmp_win, w);
    }
  if (sendEvent)
    sendclient_event (tmp_win, x, y, w, horig);
  if (broadcast)
    BroadcastConfig (XFCE_M_CONFIGURE_WINDOW, tmp_win);
}

/****************************************************************************
 *
 * Sets up the shaped window borders 
 * 
 ****************************************************************************/
void
SetShape (XfwmWindow * tmp_win, int w)
{
  if ((ShapesSupported) && (tmp_win->wShaped))
    {
      XRectangle rect;

      XShapeCombineShape (dpy, tmp_win->frame, ShapeBounding,
			  tmp_win->boundary_width,
			  tmp_win->title_height + tmp_win->boundary_width,
			  tmp_win->w, ShapeBounding, ShapeSet);
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
