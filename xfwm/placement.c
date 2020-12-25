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

#ifndef MIN
#define MIN(A,B) ((A)<(B)? (A):(B))
#endif
#ifndef MAX
#define MAX(A,B) ((A)>(B)? (A):(B))
#endif

int get_next_x (XfwmWindow * t, int x, int y);
int get_next_y (XfwmWindow * t, int y);
int test_fit (XfwmWindow * t, int test_x, int test_y, int aoimin);
void CleverPlacement (XfwmWindow * t, int *x, int *y);

/* The following factors represent the amount of area that these types of
 * windows are counted as.  For example, by default the area of ONTOP windows
 * is counted 5 times as much as normal windows.  So CleverPlacement will
 * cover 5 times as much area of another window before it will cover an ONTOP
 * window.  To treat ONTOP windows the same as other windows, set this to 1.
 * To really, really avoid putting windows under ONTOP windows, set this to a
 * high value, say 1000.  A value of 5 will try to avoid ONTOP windows if
 * practical, but if it saves a reasonable amount of area elsewhere, it will
 * place one there.  The same rules apply for the other "AVOID" factors.
 * (for CleverPlacement)
 */
#define AVOIDICON   0
#define AVOIDSTICKY 1
#define AVOIDONTOP  5

/* CleverPlacement by Anthony Martin <amartin@engr.csulb.edu>
 * This function will place a new window such that there is a minimum amount
 * of interference with other windows.  If it can place a window without any
 * interference, fine.  Otherwise, it places it so that the area of of
 * interference between the new window and the other windows is minimized */
void
CleverPlacement (XfwmWindow * t, int *x, int *y)
{
  int test_x = 0, test_y = 0;
  int xbest, ybest;
  int aoi, aoimin;		/* area of interference */

  aoi = aoimin = test_fit (t, test_x, test_y, -1);
  xbest = test_x;
  ybest = test_y;

  while ((aoi != 0) && (aoi != -1))
    {
      if (aoi > 0)		/* Windows interfere.  Try next x. */
	{
	  test_x = get_next_x (t, test_x, test_y);
	}
      else
	/* Out of room in x direction. Try next y. Reset x. */
	{
	  test_x = 0;
	  test_y = get_next_y (t, test_y);
	}
      aoi = test_fit (t, test_x, test_y, aoimin);
      if ((aoi >= 0) && (aoi < aoimin))
	{
	  xbest = test_x;
	  ybest = test_y;
	  aoimin = aoi;
	}
    }
  *x = xbest;
  *y = ybest;
}

int
get_next_x (XfwmWindow * t, int x, int y)
{
  int xnew;
  int xtest;
  XfwmWindow *testw;

  /* Test window at far right of screen */
  xnew = Scr.MyDisplayWidth;
  xtest = Scr.MyDisplayWidth - (t->frame_width + 2 * t->bw);
  if (xtest > x)
    xnew = MIN (xnew, xtest);
  /* Test the values of the right edges of every window */
  for (testw = Scr.XfwmRoot.next; testw != NULL; testw = testw->next)
    {
      if ((testw->Desk != Scr.CurrentDesk) || (testw == t))
	continue;
      if (testw->flags & ICONIFIED)
	{
	  if ((y < (testw->icon_p_height + testw->icon_w_height + testw->icon_y_loc)) &&
	      (testw->icon_y_loc < (t->frame_height + 2 * t->bw + y)))
	    {
	      xtest = testw->icon_p_width + testw->icon_x_loc;
	      if (xtest > x)
		xnew = MIN (xnew, xtest);
	      xtest = testw->icon_x_loc - (t->frame_width + 2 * t->bw);
	      if (xtest > x)
		xnew = MIN (xnew, xtest);
	    }
	}
      else if ((y < (testw->frame_height + 2 * testw->bw + testw->frame_y)) &&
	       (testw->frame_y < (t->frame_height + 2 * t->bw + y)))
	{
	  xtest = testw->frame_width + 2 * testw->bw + testw->frame_x;
	  if (xtest > x)
	    xnew = MIN (xnew, xtest);
	  xtest = testw->frame_x - (t->frame_width + 2 * t->bw);
	  if (xtest > x)
	    xnew = MIN (xnew, xtest);
	}
    }
  return xnew;
}
int
get_next_y (XfwmWindow * t, int y)
{
  int ynew;
  int ytest;
  XfwmWindow *testw;

  /* Test window at far bottom of screen */
  ynew = Scr.MyDisplayHeight;
  ytest = Scr.MyDisplayHeight - (t->frame_height + 2 * t->bw);
  if (ytest > y)
    ynew = MIN (ynew, ytest);
  /* Test the values of the bottom edge of every window */
  for (testw = Scr.XfwmRoot.next; testw != NULL; testw = testw->next)
    {
      if ((testw->Desk != Scr.CurrentDesk) || (testw == t))
	continue;
      if (testw->flags & ICONIFIED)
	{
	  ytest = testw->icon_p_height + testw->icon_w_height + testw->icon_y_loc;
	  if (ytest > y)
	    ynew = MIN (ynew, ytest);
	  ytest = testw->icon_y_loc - (t->frame_height + 2 * t->bw);
	  if (ytest > y)
	    ynew = MIN (ynew, ytest);
	}
      else
	{
	  ytest = testw->frame_height + 2 * testw->bw + testw->frame_y;
	  if (ytest > y)
	    ynew = MIN (ynew, ytest);
	  ytest = testw->frame_y - (t->frame_height + 2 * t->bw);
	  if (ytest > y)
	    ynew = MIN (ynew, ytest);
	}
    }
  return ynew;
}

int
test_fit (XfwmWindow * t, int x11, int y11, int aoimin)
{
  XfwmWindow *testw;
  int x12, x21, x22;
  int y12, y21, y22;
  int xl, xr, yt, yb;		/* xleft, xright, ytop, ybottom */
  int aoi = 0;			/* area of interference */
  int anew;
  int avoidance_factor;

  x12 = x11 + t->frame_width + 2 * t->bw;
  y12 = y11 + t->frame_height + 2 * t->bw;

  if (y12 > Scr.MyDisplayHeight)	/* No room in y direction */
    return -1;
  if (x12 > Scr.MyDisplayWidth)	        /* No room in x direction */
    return -1;
  for (testw = Scr.XfwmRoot.next; testw != NULL; testw = testw->next)
    {
      if ((testw == t) || (testw->Desk != Scr.CurrentDesk))
	continue;
      if ((testw->flags & ICONIFIED) &&
	  (testw->icon_w))
	{
	  if (testw->flags & ICON_UNMAPPED)
	    continue;
	  x21 = testw->icon_x_loc;
	  y21 = testw->icon_y_loc;
	  x22 = x21 + testw->icon_p_width;
	  y22 = y21 + testw->icon_p_height + testw->icon_w_height;
	}
      else
	{
	  x21 = testw->frame_x;
	  y21 = testw->frame_y;
	  x22 = x21 + testw->frame_width + 2 * testw->bw;
	  y22 = y21 + testw->frame_height + 2 * testw->bw;
	}
      if ((x11 < x22) && (x12 > x21) &&
	  (y11 < y22) && (y12 > y21))
	{
	  /* Windows interfere */
	  xl = MAX (x11, x21);
	  xr = MIN (x12, x22);
	  yt = MAX (y11, y21);
	  yb = MIN (y12, y22);
	  anew = (xr - xl) * (yb - yt);
	  if (testw->flags & ICONIFIED)
	    avoidance_factor = AVOIDICON;
	  else if (testw->flags & ONTOP)
	    avoidance_factor = AVOIDONTOP;
	  else if (testw->flags & STICKY)
	    avoidance_factor = AVOIDSTICKY;
	  else
	    avoidance_factor = 1;
	  anew *= avoidance_factor;
	  aoi += anew;
	  if ((aoi > aoimin) && (aoimin != -1))
	    return aoi;
	}
    }
  return aoi;
}


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
  int xl = -1, yt, DragWidth, DragHeight;
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
	      (t->wmhints->window_group == tmp_win->wmhints->window_group)))
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
  /*
   *  If
   *     o  the window is a transient, or
   * 
   *     o  a USPosition was requested
   * 
   *   then put the window where requested.
   *
   *   If RandomPlacement was specified,
   *       then place the window in a psuedo-random location
   */
  if (!(tmp_win->flags & TRANSIENT) &&
      !(tmp_win->hints.flags & USPosition) &&
      ((tflag & NO_PPOSITION_FLAG) ||
       !(tmp_win->hints.flags & PPosition)) &&
      !(PPosOverride) &&
      !((tmp_win->wmhints) &&
	(tmp_win->wmhints->flags & StateHint) &&
	(tmp_win->wmhints->initial_state == IconicState)))
    {
      /* Get user's window placement, unless RandomPlacement is specified */
      if (tflag & RANDOM_PLACE_FLAG)
	{
	  if (tflag & SMART_PLACE_FLAG)
	    CleverPlacement (tmp_win, &xl, &yt);
	  if (xl < 0)
	    {
	      /* plase window in a random location */
	      tmp_win->attr.x = Scr.randomx - tmp_win->old_bw;
	      tmp_win->attr.y = Scr.randomy - tmp_win->old_bw;
	      if ((Scr.randomx += GetDecor (tmp_win, TitleHeight)) > Scr.MyDisplayWidth / 2)
		Scr.randomx = GetDecor (tmp_win, TitleHeight);
	      if ((Scr.randomy += 2 * GetDecor (tmp_win, TitleHeight)) > Scr.MyDisplayHeight / 2)
		Scr.randomy = 2 * GetDecor (tmp_win, TitleHeight);
	    }
	  else
	    {
	      tmp_win->attr.x = xl - tmp_win->old_bw + tmp_win->bw;
	      tmp_win->attr.y = yt - tmp_win->old_bw + tmp_win->bw;
	    }
	  /* patches 11/93 to try to keep the window on the
	   * screen */
	  tmp_win->frame_x = tmp_win->attr.x + tmp_win->old_bw - tmp_win->bw;
	  tmp_win->frame_y = tmp_win->attr.y + tmp_win->old_bw - tmp_win->bw;

	  if (tmp_win->frame_x + tmp_win->frame_width +
	      2 * tmp_win->boundary_width > Scr.MyDisplayWidth)
	    {
	      tmp_win->attr.x = Scr.MyDisplayWidth - tmp_win->attr.width
		- tmp_win->old_bw + tmp_win->bw - 2 * tmp_win->boundary_width;
	      Scr.randomx = 0;
	    }
	  if (tmp_win->frame_y + 2 * tmp_win->boundary_width + tmp_win->title_height
	      + tmp_win->frame_height > Scr.MyDisplayHeight)
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
	  xl = -1;
	  yt = -1;
	  if (tflag & SMART_PLACE_FLAG)
	    CleverPlacement (tmp_win, &xl, &yt);
	  if (xl < 0)
	    {
	      if (GrabEm (POSITION))
		{
		  /* Grabbed the pointer - continue */
		  MyXGrabServer (dpy);
		  if (XGetGeometry (dpy, tmp_win->w, &JunkRoot, &JunkX, &JunkY,
				    (unsigned int *) &DragWidth,
				    (unsigned int *) &DragHeight,
				    &JunkBW, &JunkDepth) == 0)
		    {
		      free ((char *) tmp_win);
		      MyXUngrabServer (dpy);
		      return False;
		    }
		  DragWidth = tmp_win->frame_width;
		  DragHeight = tmp_win->frame_height;

		  moveLoop (tmp_win, 0, 0, DragWidth, DragHeight,
			    &xl, &yt, False, True);
		  MyXUngrabServer (dpy);
		  UngrabEm ();
		}
	      else
		{
		  /* couldn't grab the pointer - better do something */
		  XBell (dpy, Scr.screen);
		  xl = 0;
		  yt = 0;
		}
	    }
	  tmp_win->attr.y = yt - tmp_win->old_bw + tmp_win->bw;
	  tmp_win->attr.x = xl - tmp_win->old_bw + tmp_win->bw;
	  tmp_win->xdiff = xl;
	  tmp_win->ydiff = yt;
	}
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
	tmp_win->attr.y -= 2 * tmp_win->boundary_width + tmp_win->title_height;
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
  static struct _gravity_offset gravity_offsets[11] =
  {
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

  if (!INSIDE(*x, *y, minx - Scr.SnapSize, miny - Scr.SnapSize, 
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
snapxywh (int *x, int *y, int w, int h, int minx, int miny, int maxx, int maxy, int inside)
{
  int changed = 0;
  
  if (INTERSECT (*y, *y + h, miny, maxy) && BORNE ((*x + w) - (inside ? maxx : minx)))
  {
    *x = (inside ? maxx : minx) - w;
    changed |= CHANGE_HORZ;
  }

  if (INTERSECT (*x, *x + w, minx, maxx) && BORNE ((*y + h) - (inside ? maxy : miny)))
  {
    *y = (inside ? maxy : miny) - h;
    changed |= CHANGE_VERT;
  }

  if (INTERSECT (*x, *x + w, minx, maxx) && BORNE (*y - (inside ? miny : maxy)))
  {
    *y = (inside ? miny : maxy);
    changed |= CHANGE_VERT;
  }

  if (INTERSECT (*y, *y + h, miny, maxy) && BORNE (*x - (inside ? minx : maxx)))
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
	      t->frame_x + t->frame_width,
	      t->frame_y + he);

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

  changed = snapxywh (&nx, &ny, w, h, 0, 0, Scr.MyDisplayWidth, Scr.MyDisplayHeight, 1 /* inside */);

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
	      t->frame_x + t->frame_width, t->frame_y + he, 0 /* inside */);

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
SnapMove (int *FinalX, int *FinalY, int Width, int Height,
	  XfwmWindow * skip)
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
