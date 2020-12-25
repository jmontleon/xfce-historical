/*
 * SnapXY, snapxy
 * Copyright 1999 Khadiyd Idris <khadx@francemel.com>
 *
 */

/****************************************************************************
 * This module is all new
 * by Rob Nation 
 *
 * This code does smart-placement initial window placement stuff
 *
 * Copyright 1994 Robert Nation. No restrictions are placed on this code,
 * as long as the copyright notice is preserved . No guarantees or
 * warrantees of any sort whatsoever are given or implied or anything.
 ****************************************************************************/

#include "configure.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "xfwm.h"
#include "menus.h"
#include "misc.h"
#include "parse.h"
#include "screen.h"

#ifdef DMALLOC
#  include "dmalloc.h"
#endif

#ifndef MIN
#define MIN(A,B) ((A)<(B)? (A):(B))
#endif
#ifndef MAX
#define MAX(A,B) ((A)>(B)? (A):(B))
#endif

/**************************************************************************
 *
 * Handles initial placement and sizing of a new window
 * Returns False in the event of a lost window.
 *
 **************************************************************************/
Bool
PlaceWindow (XfwmWindow * tmp_win, unsigned long tflag, int Desk)
{
  XfwmWindow *t;
  int gravx, gravy;		/* gravity signs for positioning */
  extern Bool PPosOverride;

  GetGravityOffsets (tmp_win, &gravx, &gravy);


  /* Select a desk to put the window on (in list of priority):
   * 1. Sticky Windows stay on the current desk.
   * 2. Windows specified with StartsOnDesk go where specified
   * 3. Put it on the desk it was on before the restart.
   * 4. Transients go on the same desk as their parents.
   * 5. Window groups stay together (completely untested)
   */
  tmp_win->Desk = Scr.CurrentDesk;
  if (tflag & STICKY_FLAG)
    tmp_win->Desk = Scr.CurrentDesk;
  else if (tflag & STARTSONDESK_FLAG)
    tmp_win->Desk = Desk;
  else
    {
      Atom atype;
      int aformat;
      unsigned long nitems, bytes_remain;
      unsigned char *prop;

      if ((tmp_win->wmhints) && (tmp_win->wmhints->flags & WindowGroupHint) &&
	  (tmp_win->wmhints->window_group != None) &&
	  (tmp_win->wmhints->window_group != Scr.Root))
	{
	  /* Try to find the group leader or another window
	   * in the group */
	  for (t = Scr.XfwmRoot.next; t != NULL; t = t->next)
	    {
	      if ((t->w == tmp_win->wmhints->window_group) ||
		  ((t->wmhints) && (t->wmhints->flags & WindowGroupHint) &&
		   (t->wmhints->window_group ==
		    tmp_win->wmhints->window_group)))
		tmp_win->Desk = t->Desk;
	    }
	}
      if ((tmp_win->flags & TRANSIENT) && (tmp_win->transientfor != None) &&
	  (tmp_win->transientfor != Scr.Root))
	{
	  /* Try to find the parent's desktop */
	  for (t = Scr.XfwmRoot.next; t != NULL; t = t->next)
	    {
	      if (t->w == tmp_win->transientfor)
		tmp_win->Desk = t->Desk;
	    }
	}

      if ((XGetWindowProperty (dpy, tmp_win->w, _XA_WM_DESKTOP, 0L, 1L, True,
			       _XA_WM_DESKTOP, &atype, &aformat, &nitems,
			       &bytes_remain, &prop)) == Success)
	{
	  if (prop != NULL)
	    {
	      tmp_win->Desk = *(unsigned long *) prop;
	      XFree (prop);
	    }
	}
    }

  /* Desk has been selected, now pick a location for the window */
  if (!(tmp_win->flags & TRANSIENT) &&
      !(tmp_win->hints.flags & USPosition) &&
      ((tflag & NO_PPOSITION_FLAG) ||
       !(tmp_win->hints.flags & PPosition)) &&
      !(PPosOverride) &&
      !((tmp_win->wmhints) &&
	(tmp_win->wmhints->flags & StateHint) &&
	(tmp_win->wmhints->initial_state == IconicState)))
    {
      tmp_win->attr.x = Scr.randomx - tmp_win->old_bw;
      tmp_win->attr.y = Scr.randomy - tmp_win->old_bw;
      if ((Scr.randomx += GetDecor (tmp_win, TitleHeight)) > Scr.MyDisplayWidth / 2)
        Scr.randomx = GetDecor (tmp_win, TitleHeight);
      if ((Scr.randomy += 2 * GetDecor (tmp_win, TitleHeight)) > Scr.MyDisplayHeight / 2)
        Scr.randomy = 2 * GetDecor (tmp_win, TitleHeight);
      tmp_win->frame_x = tmp_win->attr.x + tmp_win->old_bw - tmp_win->bw;
      tmp_win->frame_y = tmp_win->attr.y + tmp_win->old_bw - tmp_win->bw;

      if (tmp_win->frame_x + tmp_win->frame_width +
          2 * tmp_win->boundary_width > Scr.MyDisplayWidth)
        {
          tmp_win->attr.x = Scr.MyDisplayWidth - tmp_win->attr.width
            - tmp_win->old_bw + tmp_win->bw - 2 * tmp_win->boundary_width;
          Scr.randomx = 0;
        }
      if (tmp_win->frame_y + 2 * tmp_win->boundary_width +
          tmp_win->title_height + tmp_win->frame_height >
          Scr.MyDisplayHeight)
        {
          tmp_win->attr.y = Scr.MyDisplayHeight - tmp_win->attr.height
            - tmp_win->old_bw + tmp_win->bw - tmp_win->title_height -
            2 * tmp_win->boundary_width;;
          Scr.randomy = 0;
        }

      tmp_win->xdiff = tmp_win->attr.x - tmp_win->bw;
      tmp_win->ydiff = tmp_win->attr.y - tmp_win->bw;
    }
  else
    {
      /* the USPosition was specified, or the window is a transient, 
       * or it starts iconic so place it automatically */

      tmp_win->xdiff = tmp_win->attr.x;
      tmp_win->ydiff = tmp_win->attr.y;
      /* put it where asked, mod title bar */
      /* if the gravity is towards the top, move it by the title height */
      tmp_win->attr.y -= gravy * (tmp_win->bw - tmp_win->old_bw);
      tmp_win->attr.x -= gravx * (tmp_win->bw - tmp_win->old_bw);
      if (gravy > 0)
	tmp_win->attr.y -=
	  2 * tmp_win->boundary_width + tmp_win->title_height;
      if (gravx > 0)
	tmp_win->attr.x -= 2 * tmp_win->boundary_width;
    }
  /* Just in case the window title is off the screen */
  if (tmp_win->attr.y < (tmp_win->bw - tmp_win->old_bw))
    tmp_win->attr.y = tmp_win->bw - tmp_win->old_bw;
  if (tmp_win->attr.x < (tmp_win->bw - tmp_win->old_bw))
    tmp_win->attr.x = tmp_win->bw - tmp_win->old_bw;
  return True;
}



/************************************************************************
 *
 *  Procedure:
 *	GetGravityOffsets - map gravity to (x,y) offset signs for adding
 *		to x and y when window is mapped to get proper placement.
 * 
 ************************************************************************/
struct _gravity_offset
{
  int x, y;
};

void
GetGravityOffsets (XfwmWindow * tmp, int *xp, int *yp)
{
  static struct _gravity_offset gravity_offsets[11] = {
    {0, 0},			/* ForgetGravity */
    {-1, -1},			/* NorthWestGravity */
    {0, -1},			/* NorthGravity */
    {1, -1},			/* NorthEastGravity */
    {-1, 0},			/* WestGravity */
    {0, 0},			/* CenterGravity */
    {1, 0},			/* EastGravity */
    {-1, 1},			/* SouthWestGravity */
    {0, 1},			/* SouthGravity */
    {1, 1},			/* SouthEastGravity */
    {0, 0},			/* StaticGravity */
  };
  int g = ((tmp->hints.flags & PWinGravity)
	   ? tmp->hints.win_gravity : NorthWestGravity);

  if (g < ForgetGravity || g > StaticGravity)
    *xp = *yp = 0;
  else
    {
      *xp = (int) gravity_offsets[g].x;
      *yp = (int) gravity_offsets[g].y;
    }
  return;
}

#define CHANGE_HORZ 1
#define CHANGE_VERT 2
#define INTERSECT(t1,t2,t3,t4) (((t1 >= t3) && (t1 <= t4)) || ((t2 >= t3) && (t2 <= t4)) || \
                                ((t3 >= t1) && (t3 <= t2)) || ((t4 >= t1) && (t4 <= t2)))
#define BORNE(t1) ((t1 < Scr.SnapSize) && (t1 > -Scr.SnapSize))
#define INSIDE(x1,y1,x2,y2,x3,y3) ((x1 >= x2) && (x1 <= x3) && (y1 >= y2) && (y1 <= y3))


int
snapxy (int *x, int *y, int minx, int miny, int maxx, int maxy)
{
  int left, right, top, bottom;
  int changed = 0;

  if (!INSIDE (*x, *y, minx - Scr.SnapSize, miny - Scr.SnapSize,
	       maxx + Scr.SnapSize, maxy + Scr.SnapSize))
    return (0);

  left = minx - *x;
  if (left < 0)
    left = -left;

  top = miny - *y;
  if (top < 0)
    top = -top;

  right = maxx - *x;
  if (right < 0)
    right = -right;

  bottom = maxy - *y;
  if (bottom < 0)
    bottom = -bottom;

  if (right < Scr.SnapSize)
    {
      *x = maxx;
      changed |= CHANGE_HORZ;
    }
  if (bottom < Scr.SnapSize)
    {
      *y = maxy;
      changed |= CHANGE_VERT;
    }
  if (left < Scr.SnapSize)
    {
      *x = minx;
      changed |= CHANGE_HORZ;
    }
  if (top < Scr.SnapSize)
    {
      *y = miny;
      changed |= CHANGE_VERT;
    }
  return (changed);
}

int
snapxywh (int *x, int *y, int w, int h, int minx, int miny, int maxx,
	  int maxy, int inside)
{
  int changed = 0;

  if (INTERSECT (*y, *y + h, miny, maxy)
      && BORNE ((*x + w) - (inside ? maxx : minx)))
    {
      *x = (inside ? maxx : minx) - w;
      changed |= CHANGE_HORZ;
    }

  if (INTERSECT (*x, *x + w, minx, maxx)
      && BORNE ((*y + h) - (inside ? maxy : miny)))
    {
      *y = (inside ? maxy : miny) - h;
      changed |= CHANGE_VERT;
    }

  if (INTERSECT (*x, *x + w, minx, maxx)
      && BORNE (*y - (inside ? miny : maxy)))
    {
      *y = (inside ? miny : maxy);
      changed |= CHANGE_VERT;
    }

  if (INTERSECT (*y, *y + h, miny, maxy)
      && BORNE (*x - (inside ? minx : maxx)))
    {
      *x = (inside ? minx : maxx);
      changed |= CHANGE_HORZ;
    }

  return (changed);
}

void
SnapXY (int *x, int *y, XfwmWindow * skip)
{
  XfwmWindow *t;
  int nx, ny, xch, ych, he;
  int changed = 0;

  if (!Scr.SnapSize)
    return;

  xch = 0;
  ych = 0;

  nx = *x;
  ny = *y;

  changed = snapxy (&nx, &ny, 0, 0, Scr.MyDisplayWidth, Scr.MyDisplayHeight);

  if (changed & CHANGE_HORZ)
    {
      *x = nx;
      xch = 1;
    }

  if (changed & CHANGE_VERT)
    {
      *y = ny;
      ych = 1;
    }

  if (xch && ych)
    return;

  for (t = Scr.XfwmRoot.next; t != NULL; t = t->next)
    {
      if ((t == skip) || (t->Desk != skip->Desk) || (t->flags & ICONIFIED))
	continue;
      nx = *x;
      ny = *y;
      if (t->flags & SHADED)
	he = t->title_height + 2 * t->boundary_width;
      else
	he = t->frame_height;

      changed |= snapxy (&nx, &ny, t->frame_x, t->frame_y,
			 t->frame_x + t->frame_width, t->frame_y + he);

      if ((changed & CHANGE_HORZ) && (!xch))
	{
	  *x = nx;
	  xch = 1;
	}
      if ((changed & CHANGE_VERT) && (!ych))
	{
	  *y = ny;
	  ych = 1;
	}
      if (xch && ych)
	return;
    }
}

void
SnapXYWH (int *x, int *y, int w, int h, XfwmWindow * skip)
{
  XfwmWindow *t;
  int nx, ny, xch, ych, he;
  int changed = 0;

  if (!Scr.SnapSize)
    return;

  xch = 0;
  ych = 0;

  nx = *x;
  ny = *y;

  changed = snapxywh (&nx, &ny, w, h, 0, 0, Scr.MyDisplayWidth, Scr.MyDisplayHeight, 1	/* inside */
    );

  if (changed & CHANGE_HORZ)
    {
      *x = nx;
      xch = 1;
    }

  if (changed & CHANGE_VERT)
    {
      *y = ny;
      ych = 1;
    }

  if (xch && ych)
    return;

  for (t = Scr.XfwmRoot.next; t != NULL; t = t->next)
    {
      if ((t == skip) || (t->Desk != skip->Desk) || (t->flags & ICONIFIED))
	continue;
      nx = *x;
      ny = *y;
      if (t->flags & SHADED)
	he = t->title_height + 2 * t->boundary_width;
      else
	he = t->frame_height;

      changed |= snapxywh (&nx, &ny, w, h, t->frame_x, t->frame_y,
			   t->frame_x + t->frame_width, t->frame_y + he,
			   0 /* inside */ );

      if ((changed & CHANGE_HORZ) && (!xch))
	{
	  *x = nx;
	  xch = 1;
	}
      if ((changed & CHANGE_VERT) && (!ych))
	{
	  *y = ny;
	  ych = 1;
	}
      if (xch && ych)
	return;
    }
}

void
SnapMove (int *FinalX, int *FinalY, int Width, int Height, XfwmWindow * skip)
{
  int xw, yh;

  if (!Scr.SnapSize)
    return;

  xw = *FinalX;
  yh = *FinalY;

  SnapXYWH (&xw, &yh, Width, Height, skip);

  if (xw != *FinalX)
    *FinalX = xw;
  if (yh != *FinalY)
    *FinalY = yh;
}
