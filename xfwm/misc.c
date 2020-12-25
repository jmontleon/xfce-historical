
/****************************************************************************
 * This module is all original code 
 * by Rob Nation 
 * Copyright 1993, Robert Nation
 *     You may use this code for any purpose, as long as the original
 *     copyright remains in the source code and all documentation
 ****************************************************************************/

/****************************************************************************
 *
 * Assorted odds and ends
 *
 **************************************************************************/


#include "configure.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>

#include "xfwm.h"
#include <X11/Xatom.h>
#include "menus.h"
#include "misc.h"
#include "parse.h"
#include "screen.h"
#include "module.h"

#ifdef HAVE_IMLIB
#include <Imlib.h>
#endif

#ifdef HAVE_X11_EXTENSIONS_XINERAMA_H
#  include <X11/extensions/Xinerama.h>
#endif

#ifdef DMALLOC
#  include "dmalloc.h"
#endif

char NoName[] = "Untitled";	/* name if no name in XA_WM_NAME */
char NoClass[] = "NoClass";	/* Class if no res_class in class hints */
char NoResource[] = "NoResource";	/* Class if no res_name in class hints */

#ifdef HAVE_IMLIB
extern ImlibData *imlib_id;
#endif

int
check_existfile (char *filename)
{
  struct stat s;
  int status;

  if ((!filename) || (!strlen (filename)))
    return (0);

  status = stat (filename, &s);
  if (status == 0 && S_ISREG (s.st_mode))
    return (1);
  return (0);
}

/**************************************************************************
 * 
 * Releases dynamically allocated space used to store window/icon names
 *
 **************************************************************************/
void
free_window_names (XfwmWindow * tmp, Bool nukename, Bool nukeicon)
{

  if (!tmp)
    {
      xfwm_msg (WARN, "free_window_names", "Cannot unallocate");
      return;
    }

  if (nukename && nukeicon)
    {
      if ((tmp->name != NoName) && (tmp->name != NULL))
	{
	  free (tmp->name);
	}
      tmp->name = NULL;
      if ((tmp->icon_name != NoName) && (tmp->icon_name != NULL))
	{
	  free (tmp->icon_name);
	}
      tmp->icon_name = NULL;
    }
  else if (nukename)
    {
      if ((tmp->name != NoName) && (tmp->name != NULL))
	{
	  free (tmp->name);
	}
      tmp->name = NULL;
    }
  else
    {				/* if (nukeicon) */
      if ((tmp->icon_name != NoName) && (tmp->icon_name != NULL))
	{
	  free (tmp->icon_name);
	}
      tmp->icon_name = NULL;
    }

  return;
}

/***************************************************************************
 *
 * Handles destruction of a window 
 *
 ****************************************************************************/
void
Destroy (XfwmWindow * Tmp_win)
{
  int i;
  extern XfwmWindow *ButtonWindow;
  extern XfwmWindow *colormap_win;

  /*
   * Warning, this is also called by HandleUnmapNotify; if it ever needs to
   * look at the event, HandleUnmapNotify will have to mash the UnmapNotify
   * into a DestroyNotify.
   */
  if (!Tmp_win)
    return;

  XUnmapWindow (dpy, Tmp_win->frame);

  XSync (dpy, 0);

  if (Tmp_win == Scr.Hilite)
    Scr.Hilite = NULL;

  Broadcast (XFCE_M_DESTROY_WINDOW, 3, Tmp_win->w, Tmp_win->frame,
	     (unsigned long) Tmp_win, 0, 0, 0, 0);

  if (Scr.PreviousFocus == Tmp_win)
    Scr.PreviousFocus = NULL;

  if (ButtonWindow == Tmp_win)
    ButtonWindow = NULL;

  if (Tmp_win == Scr.Focus)
    {
      if (Tmp_win->next)
	SetFocus (Tmp_win->next->w, Tmp_win->next, 1);
      else if (Tmp_win->prev)
	SetFocus (Tmp_win->prev->w, Tmp_win->prev, 1);
      else
	SetFocus (Scr.NoFocusWin, NULL, 1);
    }

  if (Tmp_win == Scr.pushed_window)
    Scr.pushed_window = NULL;

  if (Tmp_win == colormap_win)
    colormap_win = NULL;

  XDestroyWindow (dpy, Tmp_win->frame);
  XDeleteContext (dpy, Tmp_win->frame, XfwmContext);
  XDestroyWindow (dpy, Tmp_win->Parent);
  XDeleteContext (dpy, Tmp_win->Parent, XfwmContext);
  XDeleteContext (dpy, Tmp_win->w, XfwmContext);

  if ((Tmp_win->icon_w) && (Tmp_win->flags & PIXMAP_OURS))
#ifdef HAVE_IMLIB
    Imlib_free_pixmap (imlib_id, Tmp_win->iconPixmap);
#else
    XFreePixmap (dpy, Tmp_win->iconPixmap);
#endif

  if (Tmp_win->icon_w)
    {
      XDestroyWindow (dpy, Tmp_win->icon_w);
      XDeleteContext (dpy, Tmp_win->icon_w, XfwmContext);
    }
  if ((Tmp_win->flags & ICON_OURS) && (Tmp_win->icon_pixmap_w != None))
    XDestroyWindow (dpy, Tmp_win->icon_pixmap_w);
  if (Tmp_win->icon_pixmap_w != None)
    XDeleteContext (dpy, Tmp_win->icon_pixmap_w, XfwmContext);

  if (Tmp_win->flags & TITLE)
    {
      XDeleteContext (dpy, Tmp_win->title_w, XfwmContext);
      for (i = 0; i < Scr.nr_left_buttons; i++)
	XDeleteContext (dpy, Tmp_win->left_w[i], XfwmContext);
      for (i = 0; i < Scr.nr_right_buttons; i++)
	if (Tmp_win->right_w[i] != None)
	  XDeleteContext (dpy, Tmp_win->right_w[i], XfwmContext);
    }
  if (Tmp_win->flags & BORDER)
    {
      for (i = 0; i < 4; i++)
	XDeleteContext (dpy, Tmp_win->sides[i], XfwmContext);
      for (i = 0; i < 4; i++)
	XDeleteContext (dpy, Tmp_win->corners[i], XfwmContext);
    }

  Tmp_win->prev->next = Tmp_win->next;
  if (Tmp_win->next != NULL)
    Tmp_win->next->prev = Tmp_win->prev;
  free_window_names (Tmp_win, True, True);
  if (Tmp_win->wmhints)
    XFree ((char *) Tmp_win->wmhints);
  /* removing NoClass change for now... */
  if (Tmp_win->class.res_name && Tmp_win->class.res_name != NoResource)
    XFree ((char *) Tmp_win->class.res_name);
  if (Tmp_win->class.res_class && Tmp_win->class.res_class != NoClass)
    XFree ((char *) Tmp_win->class.res_class);
  if (Tmp_win->mwm_hints)
    XFree ((char *) Tmp_win->mwm_hints);

  if (Tmp_win->cmap_windows != (Window *) NULL)
    XFree ((void *) Tmp_win->cmap_windows);

  free ((char *) Tmp_win);

  XSync (dpy, 0);
  return;
}

/**************************************************************************
 *
 * Removes expose events for a specific window from the queue 
 *
 *************************************************************************/
int
flush_expose (Window w)
{
  XEvent dummy;
  int i = 0;

  while (XCheckTypedWindowEvent (dpy, w, Expose, &dummy))
    i++;
  return i;
}



/***********************************************************************
 *
 *  Procedure:
 *	RestoreWithdrawnLocation
 * 
 *  Puts windows back where they were before xfwm took over 
 *
 ************************************************************************/
void
RestoreWithdrawnLocation (XfwmWindow * tmp, Bool restart)
{
  int a, b, w2, h2;
  unsigned int bw, mask;
  XWindowChanges xwc;

  if (!tmp)
    return;

  if (XGetGeometry (dpy, tmp->w, &JunkRoot, &xwc.x, &xwc.y,
		    &JunkWidth, &JunkHeight, &bw, &JunkDepth))
    {
      XTranslateCoordinates (dpy, tmp->frame, Scr.Root, xwc.x, xwc.y,
			     &a, &b, &JunkChild);
      xwc.x = a + tmp->xdiff;
      xwc.y = b + tmp->ydiff;
      xwc.border_width = tmp->old_bw;
      mask = (CWX | CWY | CWBorderWidth);

      /* We can not assume that the window is currently on the screen.
       * Although this is normally the case, it is not always true.  The
       * most common example is when the user does something in an
       * application which will, after some amount of computational delay,
       * cause the window to be unmapped, but then switches screens before
       * this happens.  The XTranslateCoordinates call above will set the
       * window coordinates to either be larger than the screen, or negative.
       * This will result in the window being placed in odd, or even
       * unviewable locations when the window is remapped.  The followin code
       * forces the "relative" location to be within the bounds of the display.
       *
       * gpw -- 11/11/93
       *
       * Unfortunately, this does horrendous things during re-starts, 
       * hence the "if(restart)" clause (RN) 
       *
       * Also, fixed so that it only does this stuff if a window is more than
       * half off the screen. (RN)
       */

      if (!restart)
	{
	  /* Don't mess with it if its partially on the screen now */
	  if ((tmp->frame_x < 0) || (tmp->frame_y < 0) ||
	      (tmp->frame_x >= Scr.MyDisplayWidth) ||
	      (tmp->frame_y >= Scr.MyDisplayHeight))
	    {
	      w2 = (tmp->frame_width >> 1);
	      h2 = (tmp->frame_height >> 1);
	      if ((xwc.x < -w2) || (xwc.x > (Scr.MyDisplayWidth - w2)))
		{
		  xwc.x = xwc.x % Scr.MyDisplayWidth;
		  if (xwc.x < -w2)
		    xwc.x += Scr.MyDisplayWidth;
		}
	      if ((xwc.y < -h2) || (xwc.y > (Scr.MyDisplayHeight - h2)))
		{
		  xwc.y = xwc.y % Scr.MyDisplayHeight;
		  if (xwc.y < -h2)
		    xwc.y += Scr.MyDisplayHeight;
		}
	    }
	}
      XReparentWindow (dpy, tmp->w, Scr.Root, xwc.x, xwc.y);

      if ((tmp->flags & ICONIFIED) && (!(tmp->flags & SUPPRESSICON)))
	{
	  if (tmp->icon_w)
	    XUnmapWindow (dpy, tmp->icon_w);
	  if (tmp->icon_pixmap_w)
	    XUnmapWindow (dpy, tmp->icon_pixmap_w);
	}

      XConfigureWindow (dpy, tmp->w, mask, &xwc);
      if (!restart)
	XSync (dpy, 0);
    }
}

/***************************************************************************
 *
 * Start/Stops the auto-raise timer
 *
 ****************************************************************************/
void
SetTimer (int delay)
{
  struct itimerval value;

  value.it_value.tv_usec = 1000 * (delay % 1000);
  value.it_value.tv_sec = delay / 1000;
  value.it_interval.tv_usec = 0;
  value.it_interval.tv_sec = 0;
  setitimer (ITIMER_REAL, &value, NULL);
}

/****************************************************************************
 *
 * Records the time of the last processed event. Used in XSetInputFocus
 *
 ****************************************************************************/
Time lastTimestamp = CurrentTime;	/* until Xlib does this for us */

Bool
StashEventTime (XEvent * ev)
{
  Time NewTimestamp = CurrentTime;

  switch (ev->type)
    {
    case KeyPress:
    case KeyRelease:
      NewTimestamp = ev->xkey.time;
      break;
    case ButtonPress:
    case ButtonRelease:
      NewTimestamp = ev->xbutton.time;
      break;
    case MotionNotify:
      NewTimestamp = ev->xmotion.time;
      break;
    case EnterNotify:
    case LeaveNotify:
      NewTimestamp = ev->xcrossing.time;
      break;
    case PropertyNotify:
      NewTimestamp = ev->xproperty.time;
      break;
    case SelectionClear:
      NewTimestamp = ev->xselectionclear.time;
      break;
    case SelectionRequest:
      NewTimestamp = ev->xselectionrequest.time;
      break;
    case SelectionNotify:
      NewTimestamp = ev->xselection.time;
      break;
    default:
      return False;
    }
  /* Only update is the new timestamp is later than the old one, or
   * if the new one is from a time at least 30 seconds earlier than the
   * old one (in which case the system clock may have changed) */
  if ((NewTimestamp > lastTimestamp)
      || ((lastTimestamp - NewTimestamp) > 30000))
    lastTimestamp = NewTimestamp;

  return True;
}


int
GetTwoArguments (char *action, int *val1, int *val2, int *val1_unit,
		 int *val2_unit, int x, int y)
{
  char c1, c2;
  int n;

  *val1 = 0;
  *val2 = 0;
  *val1_unit = MyDisplayWidth (x, y);
  *val2_unit = MyDisplayHeight (x, y);

  n = sscanf (action, "%d %d", val1, val2);
  if (n == 2)
    return 2;

  c1 = 's';
  c2 = 's';
  n = sscanf (action, "%d%c %d%c", val1, &c1, val2, &c2);

  if (n != 4)
    return 0;

  if ((c1 == 'p') || (c1 == 'P'))
    *val1_unit = 100;

  if ((c2 == 'p') || (c2 == 'P'))
    *val2_unit = 100;

  return 2;
}


int
GetOneArgument (char *action, long *val1, int *val1_unit, int x, int y)
{
  char c1;
  int n;

  *val1 = 0;
  *val1_unit = MyDisplayWidth (x, y);

  n = sscanf (action, "%ld", val1);
  if (n == 1)
    return 1;

  c1 = '%';
  n = sscanf (action, "%ld%c", val1, &c1);

  if (n != 2)
    return 0;

  if ((c1 == 'p') || (c1 == 'P'))
    *val1_unit = 100;

  return 1;
}

/*
 * A handy function to grab a button along with its modifier, ignoring other "fake"
 * modifiers
 */
void
MyXGrabButton (Display * dpi, unsigned int button, unsigned int modifiers,
	       Window grab_window, Bool owner_events, unsigned int event_mask,
	       int pointer_mode, int keyboard_mode, Window confine_to,
	       Cursor cursor)
{
  if ((modifiers == AnyModifier) || (modifiers == 0))
    {
      XGrabButton (dpi, button, modifiers,
		   grab_window, owner_events, event_mask,
		   pointer_mode, keyboard_mode, confine_to, cursor);
    }
  else
    {
      XGrabButton (dpi, button, modifiers,
		   grab_window, owner_events, event_mask,
		   pointer_mode, keyboard_mode, confine_to, cursor);
      XGrabButton (dpi, button, modifiers | ScrollLockMask,
		   grab_window, owner_events, event_mask,
		   pointer_mode, keyboard_mode, confine_to, cursor);
      XGrabButton (dpi, button, modifiers | NumLockMask,
		   grab_window, owner_events, event_mask,
		   pointer_mode, keyboard_mode, confine_to, cursor);
      XGrabButton (dpi, button, modifiers | CapsLockMask,
		   grab_window, owner_events, event_mask,
		   pointer_mode, keyboard_mode, confine_to, cursor);
      XGrabButton (dpi, button, modifiers | ScrollLockMask | NumLockMask,
		   grab_window, owner_events, event_mask,
		   pointer_mode, keyboard_mode, confine_to, cursor);
      XGrabButton (dpi, button, modifiers | ScrollLockMask | CapsLockMask,
		   grab_window, owner_events, event_mask,
		   pointer_mode, keyboard_mode, confine_to, cursor);
      XGrabButton (dpi, button, modifiers | CapsLockMask | NumLockMask,
		   grab_window, owner_events, event_mask,
		   pointer_mode, keyboard_mode, confine_to, cursor);
      XGrabButton (dpi, button,
		   modifiers | ScrollLockMask | CapsLockMask | NumLockMask,
		   grab_window, owner_events, event_mask, pointer_mode,
		   keyboard_mode, confine_to, cursor);
    }
}

void
MyXUngrabButton (Display * dpi, unsigned int button,
		 unsigned int modifiers, Window grab_window)
{
  if ((modifiers == AnyModifier) || (modifiers == 0))
    {
      XUngrabButton (dpi, button, modifiers, grab_window);
    }
  else
    {
      XUngrabButton (dpi, button, modifiers, grab_window);
      XUngrabButton (dpi, button, modifiers | ScrollLockMask, grab_window);
      XUngrabButton (dpi, button, modifiers | NumLockMask, grab_window);
      XUngrabButton (dpi, button, modifiers | CapsLockMask, grab_window);
      XUngrabButton (dpi, button, modifiers | ScrollLockMask | NumLockMask,
		     grab_window);
      XUngrabButton (dpi, button, modifiers | ScrollLockMask | CapsLockMask,
		     grab_window);
      XUngrabButton (dpi, button, modifiers | CapsLockMask | NumLockMask,
		     grab_window);
      XUngrabButton (dpi, button,
		     modifiers | ScrollLockMask | CapsLockMask | NumLockMask,
		     grab_window);
    }
}

/*
 * Idem for keys
 */
void
MyXGrabKey (Display * dpi, int keycode, unsigned int modifiers,
	    Window grab_window, Bool owner_events,
	    int pointer_mode, int keyboard_mode)
{
  if ((modifiers == AnyModifier) || (modifiers == 0))
    {
      XGrabKey (dpi, keycode, modifiers,
		grab_window, owner_events, pointer_mode, keyboard_mode);
    }
  else
    {
      XGrabKey (dpi, keycode, modifiers,
		grab_window, owner_events, pointer_mode, keyboard_mode);
      XGrabKey (dpi, keycode, modifiers | ScrollLockMask,
		grab_window, owner_events, pointer_mode, keyboard_mode);
      XGrabKey (dpi, keycode, modifiers | NumLockMask,
		grab_window, owner_events, pointer_mode, keyboard_mode);
      XGrabKey (dpi, keycode, modifiers | CapsLockMask,
		grab_window, owner_events, pointer_mode, keyboard_mode);
      XGrabKey (dpi, keycode, modifiers | ScrollLockMask | NumLockMask,
		grab_window, owner_events, pointer_mode, keyboard_mode);
      XGrabKey (dpi, keycode, modifiers | ScrollLockMask | CapsLockMask,
		grab_window, owner_events, pointer_mode, keyboard_mode);
      XGrabKey (dpi, keycode, modifiers | CapsLockMask | NumLockMask,
		grab_window, owner_events, pointer_mode, keyboard_mode);
      XGrabKey (dpi, keycode,
		modifiers | ScrollLockMask | CapsLockMask | NumLockMask,
		grab_window, owner_events, pointer_mode, keyboard_mode);
    }
}

void
MyXUngrabKey (Display * dpi, int keycode,
	      unsigned int modifiers, Window grab_window)
{
  if ((modifiers == AnyModifier) || (modifiers == 0))
    {
      XUngrabKey (dpi, keycode, modifiers, grab_window);
    }
  else
    {
      XUngrabKey (dpi, keycode, modifiers, grab_window);
      XUngrabKey (dpi, keycode, modifiers | ScrollLockMask, grab_window);
      XUngrabKey (dpi, keycode, modifiers | NumLockMask, grab_window);
      XUngrabKey (dpi, keycode, modifiers | CapsLockMask, grab_window);
      XUngrabKey (dpi, keycode, modifiers | ScrollLockMask | NumLockMask,
		  grab_window);
      XUngrabKey (dpi, keycode, modifiers | ScrollLockMask | CapsLockMask,
		  grab_window);
      XUngrabKey (dpi, keycode, modifiers | CapsLockMask | NumLockMask,
		  grab_window);
      XUngrabKey (dpi, keycode,
		  modifiers | ScrollLockMask | CapsLockMask | NumLockMask,
		  grab_window);
    }
}

/*****************************************************************************
 *
 * Grab the pointer and keyboard
 *
 ****************************************************************************/
Bool
GrabEm (int cursor)
{
  int i = 0, val = 0;
  unsigned int mask;
  Cursor vs = None;

  XSync (dpy, 0);
  vs = ((cursor >= 0) ? Scr.XfwmCursors[cursor] : None);
  /* move the keyboard focus prior to grabbing the pointer to
   * eliminate the enterNotify and exitNotify events that go
   * to the windows */
  if (Scr.PreviousFocus == NULL)
    Scr.PreviousFocus = Scr.Focus;
  SetFocus (Scr.NoFocusWin, NULL, 0);
  mask =
    ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | PointerMotionMask
    | EnterWindowMask | LeaveWindowMask;
  while ((i < 1000)
	 && (val =
	     XGrabPointer (dpy, Scr.Root, True, mask, GrabModeAsync,
			   GrabModeAsync, Scr.Root, vs,
			   CurrentTime) != GrabSuccess))
    {
      i++;
      /* If you go too fast, other windows may not get a change to release
       * any grab that they have. */
      sleep_a_little (1000);
    }

  /* If we fall out of the loop without grabbing the pointer, its
   * time to give up */
  XSync (dpy, 0);
  if (val != GrabSuccess)
    {
      return False;
    }
  return True;
}


/*****************************************************************************
 *
 * UnGrab the pointer and keyboard
 *
 ****************************************************************************/
void
UngrabEm ()
{
  Window w;

  XSync (dpy, 0);
  XUngrabPointer (dpy, CurrentTime);

  if (Scr.PreviousFocus != NULL)
    {
      w = Scr.PreviousFocus->w;

      /* if the window still exists, focus on it */
      if (w)
	{
	  SetFocus (w, Scr.PreviousFocus, 0);
	}
      Scr.PreviousFocus = NULL;
    }
  XSync (dpy, 0);
}



/****************************************************************************
 *
 * Keeps the "StaysOnTop" windows on the top of the pile.
 * This is achieved by clearing a flag for OnTop windows here, and waiting
 * for a visibility notify on the windows. Exeption: OnTop windows which are
 * obscured by other OnTop windows, which need to be raised here.
 *
 ****************************************************************************/
void
KeepOnTop ()
{
  XfwmWindow *t;

  /* flag that on-top windows should be re-raised */
  for (t = Scr.XfwmRoot.next; t != NULL; t = t->next)
    {
      if ((t->flags & ONTOP) && !(t->flags & VISIBLE)
	  && !(t->flags & ICONIFIED))
	{
	  RaiseWindow (t);
	  t->flags &= ~RAISED;
	}
      else
	t->flags |= RAISED;
    }
}


/**************************************************************************
 * 
 * Unmaps a window on transition to a new desktop
 *
 *************************************************************************/
void
UnmapIt (XfwmWindow * t)
{
  XWindowAttributes winattrs;
  unsigned long eventMask;

  XGetWindowAttributes (dpy, t->w, &winattrs);
  eventMask = winattrs.your_event_mask;
  XSelectInput (dpy, t->w, eventMask & ~StructureNotifyMask);
  if (t->flags & ICONIFIED)
    {
      if (t->icon_pixmap_w != None)
	XUnmapWindow (dpy, t->icon_pixmap_w);
      if (t->icon_w != None)
	XUnmapWindow (dpy, t->icon_w);
    }
  else if (t->flags & (MAPPED | MAP_PENDING))
    {
      XUnmapWindow (dpy, t->w);
      XUnmapWindow (dpy, t->frame);
    }
  XSelectInput (dpy, t->w, eventMask);
}

/**************************************************************************
 * 
 * Maps a window on transition to a new desktop
 *
 *************************************************************************/
void
MapIt (XfwmWindow * t)
{
  XWindowAttributes winattrs;
  unsigned long eventMask;

  XGetWindowAttributes (dpy, t->w, &winattrs);
  eventMask = winattrs.your_event_mask;
  XSelectInput (dpy, t->w, eventMask & ~StructureNotifyMask);
  if (t->flags & ICONIFIED)
    {
      if (t->icon_pixmap_w != None)
	XMapWindow (dpy, t->icon_pixmap_w);
      if (t->icon_w != None)
	XMapWindow (dpy, t->icon_w);
    }
  else if (t->flags & MAPPED)
    {
      XMapWindow (dpy, t->w);
      XMapWindow (dpy, t->Parent);
      XMapWindow (dpy, t->frame);
      t->flags |= MAP_PENDING;
    }
  XSelectInput (dpy, t->w, eventMask);
}

void
RaiseWindow (XfwmWindow * t)
{
  XfwmWindow *t2;
  int count, i;
  Window *wins;

  /* raise the target, at least */
  count = 1;
  Broadcast (XFCE_M_RAISE_WINDOW, 3, t->w, t->frame, (unsigned long) t, 0, 0,
	     0, 0);

  MyXGrabServer (dpy);
  for (t2 = Scr.XfwmRoot.next; t2 != NULL; t2 = t2->next)
    {
      t2->flags &= ~RAISEDWIN;
      if ((t2->flags & ONTOP) && !(t2->flags & ICONIFIED))
	count++;
      if ((t2->flags & TRANSIENT) && (t2->transientfor == t->w) && (t2 != t))
	{
	  count++;
	  Broadcast (XFCE_M_RAISE_WINDOW, 3, t2->w, t2->frame,
		     (unsigned long) t2, 0, 0, 0, 0);

	  if ((t2->flags & ICONIFIED) && (!(t2->flags & SUPPRESSICON)))
	    {
	      count += 2;
	    }
	}
    }
  if ((t->flags & ICONIFIED) && (!(t->flags & SUPPRESSICON)))
    {
      count += 2;
    }

  wins = (Window *) safemalloc (count * sizeof (Window));

  i = 0;

  /* ONTOP windows on top */
  for (t2 = Scr.XfwmRoot.next; t2 != NULL; t2 = t2->next)
    {
      if ((t2->flags & ONTOP) && !(t2->flags & ICONIFIED) && (t2 != t) &&
          !((t->flags & TRANSIENT) && (t->transientfor == t2->w)))
	{
	  Broadcast (XFCE_M_RAISE_WINDOW, 3, t2->w, t2->frame,
		     (unsigned long) t2, 0, 0, 0, 0);
	  wins[i++] = t2->frame;
	}
    }

  /* now raise transients */
  for (t2 = Scr.XfwmRoot.next; t2 != NULL; t2 = t2->next)
    {
      if ((t2->flags & TRANSIENT) && (t2->transientfor == t->w) && (t2 != t))
	{
	  wins[i++] = t2->frame;
	  if ((t2->flags & ICONIFIED) && (!(t2->flags & SUPPRESSICON)))
	    {
	      if (!(t2->flags & NOICON_TITLE))
		wins[i++] = t2->icon_w;
	      if (!(t2->icon_pixmap_w))
		wins[i++] = t2->icon_pixmap_w;
	    }
	}
    }
  if ((t->flags & ICONIFIED) && (!(t->flags & SUPPRESSICON)))
    {
      if (!(t->flags & NOICON_TITLE))
	wins[i++] = t->icon_w;
      if (t->icon_pixmap_w)
	wins[i++] = t->icon_pixmap_w;
    }

  if (!(t->flags & ONTOP))
    {
      wins[i++] = t->frame;
      Scr.LastWindowRaised = t;
    }

  if (i > 0)
    XRaiseWindow (dpy, wins[0]);

  XRestackWindows (dpy, wins, i);
  free (wins);

  t->flags |= RAISEDWIN;
  XSync (dpy, 0);
  MyXUngrabServer (dpy);

}


void
LowerWindow (XfwmWindow * t)
{
  XLowerWindow (dpy, t->frame);

  Broadcast (XFCE_M_LOWER_WINDOW, 3, t->w, t->frame, (unsigned long) t, 0, 0,
	     0, 0);

  if ((t->flags & ICONIFIED) && (!(t->flags & SUPPRESSICON)))
    {
      XLowerWindow (dpy, t->icon_w);
      XLowerWindow (dpy, t->icon_pixmap_w);
    }
  Scr.LastWindowRaised = (XfwmWindow *) 0;
}

/*
 * ** xfwm_msg: used to send output from xfwm to files and or stderr/stdout
 * **
 * ** type -> DBG == Debug, ERR == Error, INFO == Information, WARN == Warning
 * ** id -> name of function, or other identifier
 */
void
xfwm_msg (int type, char *id, char *msg, ...)
{
  char *typestr;
  va_list args;

  switch (type)
    {
    case DBG:
      typestr = "Debug =";
      break;
    case ERR:
      typestr = "Error =";
      break;
    case WARN:
      typestr = "Warning =";
      break;
    case INFO:
    default:
      typestr = "";
      break;
    }

  va_start (args, msg);

  fprintf (stderr, "XFWM message (type %s): %s ", id, typestr);
  vfprintf (stderr, msg, args);
  fprintf (stderr, "\n");

  if (type == ERR)
    {
      char tmp[1024];		/* I hate to use a fixed length but this will do for now */
      sprintf (tmp, "XFWM message (type %s): %s ", id, typestr);
      vsprintf (tmp + strlen (tmp), msg, args);
      tmp[strlen (tmp) + 1] = '\0';
      tmp[strlen (tmp)] = '\n';
      BroadcastName (XFCE_M_ERROR, 0, 0, 0, tmp);
    }

  va_end (args);
}				/* xfwm_msg */

/* Xinerama handling routines */

#if defined(HAVE_X11_EXTENSIONS_XINERAMA_H) || defined(EMULATE_XINERAMA)
static int findhead(int x, int y)
{
  extern XineramaScreenInfo *xinerama_infos;
  extern int xinerama_heads;
  extern Bool enable_xinerama;
  static int cache_x;
  static int cache_y;
  static int cache_head;
  static Bool cache_init = False;
  
  int head, closest_head;
  int dx, dy;
  int center_x, center_y;
  int distsquare, min_distsquare;
  
  /* Cache system */
  if ((cache_init) && (x == cache_x) && (y == cache_y))
    return (cache_head);
    
  /* Okay, now we consider the cache has been initialized */
  cache_init = True;
  cache_x = x;
  cache_y = y;
    
  if ((xinerama_heads == 0) || (xinerama_infos == NULL) || (!enable_xinerama))
    /* Xinerama extensions are disabled */
    {
      cache_head = 0;
      return (0);
    }
    
  for (head = 0; head < xinerama_heads; head++)
    {
      if ((xinerama_infos[head].x_org <= x)
	  && ((xinerama_infos[head].x_org + xinerama_infos[head].width) > x)
          && (xinerama_infos[head].y_org <= y)
	  && ((xinerama_infos[head].y_org + xinerama_infos[head].height) > y))
        {
          cache_head = head;
          return (head);
	}
    }
  /* No head has been eligible, use the closest one*/

  center_x = xinerama_infos[0].x_org + (xinerama_infos[0].width / 2);
  center_y = xinerama_infos[0].y_org + (xinerama_infos[0].height / 2);

  dx = x - center_x;
  dy = y - center_y;
  
  min_distsquare = (dx * dx) + (dy * dy);
  closest_head   = 0;

  for (head = 1; head < xinerama_heads; head++)
    {
      center_x = xinerama_infos[head].x_org + (xinerama_infos[head].width / 2);
      center_y = xinerama_infos[head].y_org + (xinerama_infos[head].height / 2);

      dx = x - center_x;
      dy = y - center_y;
      
      distsquare = (dx * dx) + (dy * dy);
      
      if (distsquare < min_distsquare)
        {
          min_distsquare = distsquare;
          closest_head   = head;
        }
    }
  cache_head = closest_head;
  return (closest_head);
}
#endif

int MyDisplayHeight (int x, int y)
{
#if defined(HAVE_X11_EXTENSIONS_XINERAMA_H) || defined(EMULATE_XINERAMA)
  extern XineramaScreenInfo *xinerama_infos;
  extern int xinerama_heads;
  extern Bool enable_xinerama;
  int head;

  if ((xinerama_heads == 0) || (xinerama_infos == NULL) || (!enable_xinerama))
    /* Xinerama extensions are disabled */
    return (Scr.MyDisplayHeight);

  head = findhead (x, y);    
  return (xinerama_infos[head].height);
#else
  return (Scr.MyDisplayHeight);
#endif
}

int MyDisplayWidth (int x, int y)
{
#if defined(HAVE_X11_EXTENSIONS_XINERAMA_H) || defined(EMULATE_XINERAMA)
  extern XineramaScreenInfo *xinerama_infos;
  extern int xinerama_heads;
  extern Bool enable_xinerama;
  int head;
  
  if ((xinerama_heads == 0) || (xinerama_infos == NULL) || (!enable_xinerama))
    /* Xinerama extensions are disabled */
    return (Scr.MyDisplayWidth);
    
  head = findhead (x, y);    
  return (xinerama_infos[head].width);
#else
  return (Scr.MyDisplayWidth);
#endif
}

int MyDisplayX (int x, int y)
{
#if defined(HAVE_X11_EXTENSIONS_XINERAMA_H) || defined(EMULATE_XINERAMA)
  extern XineramaScreenInfo *xinerama_infos;
  extern int xinerama_heads;
  extern Bool enable_xinerama;
  int head;
  
  if ((xinerama_heads == 0) || (xinerama_infos == NULL) || (!enable_xinerama))
    /* Xinerama extensions are disabled */
    return (0);
    
  head = findhead (x, y);    
  return (xinerama_infos[head].x_org);
#else
  return (0);
#endif
}


int MyDisplayY (int x, int y)
{
#if defined(HAVE_X11_EXTENSIONS_XINERAMA_H) || defined(EMULATE_XINERAMA)
  extern XineramaScreenInfo *xinerama_infos;
  extern int xinerama_heads;
  extern Bool enable_xinerama;
  int head;
  
  if ((xinerama_heads == 0) || (xinerama_infos == NULL) || (!enable_xinerama))
    /* Xinerama extensions are disabled */
    return (0);
    
  head = findhead (x, y);    
  return (xinerama_infos[head].y_org);
#else
  return (0);
#endif
}

int MyDisplayMaxX (int x, int y)
{
#if defined(HAVE_X11_EXTENSIONS_XINERAMA_H) || defined(EMULATE_XINERAMA)
  extern XineramaScreenInfo *xinerama_infos;
  extern int xinerama_heads;
  extern Bool enable_xinerama;
  int head;
  
  if ((xinerama_heads == 0) || (xinerama_infos == NULL) || (!enable_xinerama))
    /* Xinerama extensions are disabled */
    return (Scr.MyDisplayWidth);
    
  head = findhead (x, y);    
  return (xinerama_infos[head].x_org + xinerama_infos[head].width);
#else
  return (Scr.MyDisplayWidth);
#endif
}

int MyDisplayMaxY (int x, int y)
{
#if defined(HAVE_X11_EXTENSIONS_XINERAMA_H) || defined(EMULATE_XINERAMA)
  extern XineramaScreenInfo *xinerama_infos;
  extern int xinerama_heads;
  extern Bool enable_xinerama;
  int head;
  
  if ((xinerama_heads == 0) || (xinerama_infos == NULL) || (!enable_xinerama))
    /* Xinerama extensions are disabled */
    return (Scr.MyDisplayHeight);

  head = findhead (x, y);    
  return (xinerama_infos[head].y_org + xinerama_infos[head].height);
#else
  return (Scr.MyDisplayHeight);
#endif
}

