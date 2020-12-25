
/****************************************************************************
 * This module is all new
 * by Rob Nation 
 * A little of it is borrowed from ctwm.
 * Copyright 1993 Robert Nation. No restrictions are placed on this code,
 * as long as the copyright notice is preserved
 ****************************************************************************/
/***********************************************************************
 *
 * xfwm window-list popup code
 *
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <limits.h>

#include "configure.h"
#include "utils.h"

#include "xfwm.h"
#include "menus.h"
#include "misc.h"
#include "parse.h"
#include "screen.h"

#ifdef DMALLOC
#  include "dmalloc.h"
#endif

#define SHOW_GEOMETRY (1<<0)
#define SHOW_ALLDESKS (1<<1)
#define SHOW_NORMAL   (1<<2)
#define SHOW_ICONIC   (1<<3)
#define SHOW_STICKY   (1<<4)
#define SHOW_ONTOP    (1<<5)
#define DONT_SORT     (1<<6)
#define SHOW_ICONNAME (1<<7)
#define SHOW_ALPHABETIC (1<<8)
#define SHOW_EVERYTHING (SHOW_GEOMETRY | SHOW_ALLDESKS | SHOW_NORMAL | SHOW_ICONIC | SHOW_STICKY | SHOW_ONTOP)

/* Function to compare window title names
 */
static int globalFlags;
int
winCompare (const XfwmWindow ** a, const XfwmWindow ** b)
{
  if (globalFlags & SHOW_ICONNAME)
    return strcasecmp ((*a)->icon_name, (*b)->icon_name);
  else
    return strcasecmp ((*a)->name, (*b)->name);
}


/*
 * Change by PRB (pete@tecc.co.uk), 31/10/93.  Prepend a hot key
 * specifier to each item in the list.  This means allocating the
 * memory for each item (& freeing it) rather than just using the window
 * title directly.  */
void
do_windowList (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	       unsigned long context, char *action, int *Module)
{
  MenuRoot *mr;
  XfwmWindow *t;
  XfwmWindow **windowList;
  int numWindows;
  int ii;
  char *tname = NULL;
  char loc[40], *name = NULL;
  int dwidth, dheight;
  char tlabel[50] = "";
  int last_desk_done = INT_MIN;
  int next_desk;
  char *t_hot = NULL;		/* Menu label with hotkey added */
  char scut = '0';		/* Current short cut key */
  char *line = NULL, *tok = NULL;
  int desk = Scr.CurrentDesk;
  int flags = SHOW_EVERYTHING;
  char *func = NULL;

  if (action && *action)
    {
      line = strdup (action);	/* local copy */
      /* parse args */
      while (line && *line)
	{
	  tok = GetToken (&line);

	  if (StrEquals (tok, "Function"))
	    func = GetToken (&line);
	  else if (StrEquals (tok, "Desk"))
	    {
	      desk = atoi (GetToken (&line));
	      flags &= ~SHOW_ALLDESKS;
	    }
	  else if (StrEquals (tok, "CurrentDesk"))
	    {
	      desk = Scr.CurrentDesk;
	      flags &= ~SHOW_ALLDESKS;
	    }
	  else if (StrEquals (tok, "Unsorted"))
	    flags |= DONT_SORT;
	  else if (StrEquals (tok, "UseIconName"))
	    flags |= SHOW_ICONNAME;
	  else if (StrEquals (tok, "NoGeometry"))
	    flags &= ~SHOW_GEOMETRY;
	  else if (StrEquals (tok, "Geometry"))
	    flags |= SHOW_GEOMETRY;
	  else if (StrEquals (tok, "NoIcons"))
	    flags &= ~SHOW_ICONIC;
	  else if (StrEquals (tok, "Icons"))
	    flags |= SHOW_ICONIC;
	  else if (StrEquals (tok, "OnlyIcons"))
	    flags = SHOW_ICONIC;
	  else if (StrEquals (tok, "NoNormal"))
	    flags &= ~SHOW_NORMAL;
	  else if (StrEquals (tok, "Normal"))
	    flags |= SHOW_NORMAL;
	  else if (StrEquals (tok, "OnlyNormal"))
	    flags = SHOW_NORMAL;
	  else if (StrEquals (tok, "NoSticky"))
	    flags &= ~SHOW_STICKY;
	  else if (StrEquals (tok, "Sticky"))
	    flags |= SHOW_STICKY;
	  else if (StrEquals (tok, "OnlySticky"))
	    flags = SHOW_STICKY;
	  else if (StrEquals (tok, "NoOnTop"))
	    flags &= ~SHOW_ONTOP;
	  else if (StrEquals (tok, "OnTop"))
	    flags |= SHOW_ONTOP;
	  else if (StrEquals (tok, "OnlyOnTop"))
	    flags = SHOW_ONTOP;
	  else
	    {
	      xfwm_msg (ERR, "WindowList", "Unknown option '%s'", tok);
	    }
	}
    }

  globalFlags = flags;
  if (flags & SHOW_GEOMETRY)
    {
      sprintf (tlabel, "Desk: %d\tGeometry", desk);
    }
  else
    {
      sprintf (tlabel, "Desk: %d", desk);
    }
  mr = NewMenuRoot (tlabel, 0);

  next_desk = 0;

  numWindows = 0;
  for (t = Scr.XfwmRoot.next; t != NULL; t = t->next)
    {
      numWindows++;
    }
  windowList = malloc (numWindows * sizeof (t));
  if (windowList == NULL)
    return;

  ii = 0;
  for (t = Scr.XfwmRoot.next; t != NULL; t = t->next)
    {
      windowList[ii] = t;
      ii++;
    }

  while (next_desk != INT_MAX)
    {
      /* Sort window list by desktop number */
      if ((flags & SHOW_ALLDESKS) && !(flags & DONT_SORT))
	{
	  next_desk = INT_MAX;
	  for (ii = 0; ii < numWindows; ii++)
	    {
	      t = windowList[ii];
	      if ((t->Desk > last_desk_done) && (t->Desk < next_desk))
		next_desk = t->Desk;
	    }
	}
      if (!(flags & SHOW_ALLDESKS))
	{
	  if (last_desk_done == INT_MIN)
	    next_desk = desk;
	  else
	    next_desk = INT_MAX;
	}
      last_desk_done = next_desk;
      for (ii = 0; ii < numWindows; ii++)
	{
	  t = windowList[ii];
	  if ((t->Desk == next_desk) && (!(t->flags & WINDOWLISTSKIP)))
	    {
	      if (!(flags & SHOW_ICONIC) && (t->flags & ICONIFIED))
		continue;	/* don't want icons - skip */
	      if (!(flags & SHOW_STICKY) && (t->flags & STICKY))
		continue;	/* don't want sticky ones - skip */
	      if (!(flags & SHOW_ONTOP) && (t->flags & ONTOP))
		continue;	/* don't want ontop ones - skip */
	      if (!(flags & SHOW_NORMAL) &&
		  !((t->flags & ICONIFIED) ||
		    (t->flags & STICKY) || (t->flags & ONTOP)))
		continue;	/* don't want "normal" ones - skip */

	      if (++scut == ('9' + 1))
		scut = 'A';	/* Next shortcut key */
	      if (flags & SHOW_ICONNAME)
		name = t->icon_name;
	      else
		name = t->name;
	      t_hot = safemalloc (strlen (name) + 48);
	      if (t->flags & ICONIFIED)
	        sprintf (t_hot, "&%c.  (%s)", scut, name);	
	      else
	        sprintf (t_hot, "&%c.  %s ", scut, name);	

	      if (flags & SHOW_GEOMETRY)
		{
		  tname = safemalloc (80);
		  tname[0] = 0;
		  sprintf (loc, "[%d] ", t->Desk + 1);
		  strcpy (tname, loc);

		  dheight =
		    t->frame_height - t->title_height - 2 * t->boundary_width;
		  dwidth = t->frame_width - 2 * t->boundary_width;

		  dwidth -= t->hints.base_width;
		  dheight -= t->hints.base_height;

		  dwidth /= t->hints.width_inc;
		  dheight /= t->hints.height_inc;

		  sprintf (loc, "%d", dwidth);
		  strcat (tname, loc);
		  sprintf (loc, "x%d", dheight);
		  strcat (tname, loc);
		  if (t->frame_x >= 0)
		    sprintf (loc, "+%d", t->frame_x);
		  else
		    sprintf (loc, "%d", t->frame_x);
		  strcat (tname, loc);
		  if (t->frame_y >= 0)
		    sprintf (loc, "+%d", t->frame_y);
		  else
		    sprintf (loc, "%d", t->frame_y);
		  strcat (tname, loc);

		  strcat (t_hot, "\t");
		  strcat (t_hot, tname);
		}
	      if (func)
		sprintf (tlabel, "%s %ld", func, t->w);
	      else
		sprintf (tlabel, "WindowListFunc %ld", t->w);
	      AddToMenu (mr, t_hot, tlabel);
	      if (t_hot)
		free (t_hot);
	      if (tname)
		free (tname);
	    }
	}
    }

  free (windowList);

  MakeMenu (mr);

  /* and1000@cam.ac.uk, 27/6/96 */
  do_menu (mr, eventp->type == ButtonPress);

  DestroyMenu (mr);
}
