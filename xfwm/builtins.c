/****************************************************************************
 * This module is all original code 
 * by Rob Nation 
 * Copyright 1993, Robert Nation
 *     You may use this code for any purpose, as long as the original
 *     copyright remains in the source code and all documentation
 ****************************************************************************/

#include "configure.h"

#include <sys/types.h>
#include <sys/time.h>

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#include "xfwm.h"
#include "menus.h"
#include "misc.h"
#include "parse.h"
#include "screen.h"
#include "module.h"

static char *exec_shell_name = "/bin/sh";
char *ModulePath = "./";
/* button state strings must match the enumerated states */
static char *button_states[MaxButtonState] =
{"Active", "Inactive",};

extern void KillModuleByName (char *name);

/***********************************************************************
 *
 *  Procedure:
 *	DeferExecution - defer the execution of a function to the
 *	    next button press if the context is C_ROOT
 *
 *  Inputs:
 *      eventp  - pointer to XEvent to patch up
 *      w       - pointer to Window to patch up
 *      tmp_win - pointer to XfwmWindow Structure to patch up
 *	context	- the context in which the mouse button was pressed
 *	func	- the function to defer
 *	cursor	- the cursor to display while waiting
 *      finishEvent - ButtonRelease or ButtonPress; tells what kind of event to
 *                    terminate on.
 *
 ***********************************************************************/
int
DeferExecution (XEvent * eventp, Window * w, XfwmWindow ** tmp_win,
		unsigned long *context, int cursor, int FinishEvent)

{
  int done;
  int finished = 0;
  Window dummy;
  Window original_w;

  original_w = *w;

  if ((*context != C_ROOT) && (*context != C_NO_CONTEXT))
    {
      if ((FinishEvent == ButtonPress) || ((FinishEvent == ButtonRelease) &&
					   (eventp->type != ButtonPress)))
	{
	  return FALSE;
	}
    }
  if (!GrabEm (cursor))
    {
      XBell (dpy, Scr.screen);
      return True;
    }

  while (!finished)
    {
      done = 0;
      /* block until there is an event */
      XMaskEvent (dpy, ButtonPressMask | ButtonReleaseMask |
		  ExposureMask | KeyPressMask | VisibilityChangeMask |
		  ButtonMotionMask | PointerMotionMask	/* | EnterWindowMask | 
							 * LeaveWindowMask */ , eventp);
      StashEventTime (eventp);

      if (eventp->type == KeyPress)
	Keyboard_shortcuts (eventp, FinishEvent);
      if (eventp->type == FinishEvent)
	finished = 1;
      if (eventp->type == ButtonPress)
	{
	  XAllowEvents (dpy, ReplayPointer, CurrentTime);
	  done = 1;
	}
      if (eventp->type == ButtonRelease)
	done = 1;
      if (eventp->type == KeyPress)
	done = 1;

      if (!done)
	{
	  DispatchEvent ();
	}

    }

  *w = eventp->xany.window;
  if (((*w == Scr.Root) || (*w == Scr.NoFocusWin))
      && (eventp->xbutton.subwindow != (Window) 0))
    {
      *w = eventp->xbutton.subwindow;
      eventp->xany.window = *w;
    }
  if (*w == Scr.Root)
    {
      *context = C_ROOT;
      XBell (dpy, Scr.screen);
      UngrabEm ();
      return TRUE;
    }
  if (XFindContext (dpy, *w, XfwmContext, (caddr_t *) tmp_win) == XCNOENT)
    {
      *tmp_win = NULL;
      XBell (dpy, Scr.screen);
      UngrabEm ();
      return (TRUE);
    }

  if (*w == (*tmp_win)->Parent)
    *w = (*tmp_win)->w;

  if (original_w == (*tmp_win)->Parent)
    original_w = (*tmp_win)->w;

  /* this ugly mess attempts to ensure that the release and press
   * are in the same window. */
  if ((*w != original_w) && (original_w != Scr.Root) &&
      (original_w != None) && (original_w != Scr.NoFocusWin))
    if (!((*w == (*tmp_win)->frame) &&
	  (original_w == (*tmp_win)->w)))
      {
	*context = C_ROOT;
	XBell (dpy, Scr.screen);
	UngrabEm ();
	return TRUE;
      }

  *context = GetContext (*tmp_win, eventp, &dummy);

  UngrabEm ();
  return FALSE;
}

/**************************************************************************
 *
 * Moves focus to specified window 
 *
 *************************************************************************/
void
FocusOn (XfwmWindow * t, int DeIconifyOnly)
{
  int dx, dy;
  int cx, cy;
  int x, y;

  if (t == (XfwmWindow *) 0)
    return;

  if (t->Desk != Scr.CurrentDesk)
    {
      changeDesks (0, t->Desk);
    }

  if (t->flags & ICONIFIED)
    {
      cx = t->icon_xl_loc + t->icon_w_width / 2;
      cy = t->icon_y_loc + t->icon_p_height + ICON_HEIGHT / 2;
    }
  else
    {
      cx = t->frame_x + t->frame_width / 2;
      cy = t->frame_y + t->frame_height / 2;
    }

  dx = (cx + Scr.Vx) / Scr.MyDisplayWidth * Scr.MyDisplayWidth;
  dy = (cy + Scr.Vy) / Scr.MyDisplayHeight * Scr.MyDisplayHeight;

  MoveViewport (dx, dy, True);

  if (t->flags & ICONIFIED)
    {
      x = t->icon_xl_loc + t->icon_w_width / 2;
      y = t->icon_y_loc + t->icon_p_height + ICON_HEIGHT / 2;
    }
  else
    {
      x = t->frame_x;
      y = t->frame_y;
    }
  KeepOnTop ();

  /* If the window is still not visible, make it visible! */
  if (((t->frame_x + t->frame_height) < 0) || (t->frame_y + t->frame_width < 0) ||
    (t->frame_x > Scr.MyDisplayWidth) || (t->frame_y > Scr.MyDisplayHeight))
    {
      SetupFrame (t, 0, 0, t->frame_width, t->frame_height, False);
    }
  UngrabEm ();
  SetFocus (t->w, t, 0);
}



/**************************************************************************
 *
 * Moves pointer to specified window 
 *
 *************************************************************************/
void
WarpOn (XfwmWindow * t, int warp_x, int x_unit, int warp_y, int y_unit)
{
  int dx, dy;
  int cx, cy;
  int x, y;

  if (t == (XfwmWindow *) 0 || (t->flags & ICONIFIED && t->icon_w == None))
    return;

  if (t->Desk != Scr.CurrentDesk)
    {
      changeDesks (0, t->Desk);
    }

  if (t->flags & ICONIFIED)
    {
      cx = t->icon_xl_loc + t->icon_w_width / 2;
      cy = t->icon_y_loc + t->icon_p_height + ICON_HEIGHT / 2;
    }
  else
    {
      cx = t->frame_x + t->frame_width / 2;
      cy = t->frame_y + t->frame_height / 2;
    }

  dx = (cx + Scr.Vx) / Scr.MyDisplayWidth * Scr.MyDisplayWidth;
  dy = (cy + Scr.Vy) / Scr.MyDisplayHeight * Scr.MyDisplayHeight;

  MoveViewport (dx, dy, True);

  if (t->flags & ICONIFIED)
    {
      x = t->icon_xl_loc + t->icon_w_width / 2 + 2;
      y = t->icon_y_loc + t->icon_p_height + ICON_HEIGHT / 2 + 2;
    }
  else
    {
      if (x_unit != Scr.MyDisplayWidth)
	x = t->frame_x + 2 + warp_x;
      else
	x = t->frame_x + 2 + (t->frame_width - 4) * warp_x / 100;
      if (y_unit != Scr.MyDisplayHeight)
	y = t->frame_y + 2 + warp_y;
      else
	y = t->frame_y + 2 + (t->frame_height - 4) * warp_y / 100;
    }
  if (warp_x >= 0 && warp_y >= 0)
    {
      XWarpPointer (dpy, None, Scr.Root, 0, 0, 0, 0, x, y);
    }
  RaiseWindow (t);
  KeepOnTop ();

  /* If the window is still not visible, make it visible! */
  if (((t->frame_x + t->frame_height) < 0) || (t->frame_y + t->frame_width < 0) ||
    (t->frame_x > Scr.MyDisplayWidth) || (t->frame_y > Scr.MyDisplayHeight))
    {
      SetupFrame (t, 0, 0, t->frame_width, t->frame_height, False);
      XWarpPointer (dpy, None, Scr.Root, 0, 0, 0, 0, 2, 2);
    }
  UngrabEm ();
}



/***********************************************************************
 *
 *  Procedure:
 *	(Un)Maximize a window.
 *
 ***********************************************************************/
void
Maximize (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	  unsigned long context, char *action, int *Module)
{
  int new_width, new_height, new_x, new_y;
  int val1, val2, val1_unit, val2_unit, n;

  if (DeferExecution (eventp, &w, &tmp_win, &context, SELECT, ButtonRelease))
    return;

  if (tmp_win == NULL)
    return;

  n = GetTwoArguments (action, &val1, &val2, &val1_unit, &val2_unit);
  if (n != 2)
    {
      val1 = 100;
      val2 = 100;
      val1_unit = Scr.MyDisplayWidth;
      val2_unit = Scr.MyDisplayHeight;
    }

  if (tmp_win->flags & MAXIMIZED)
    {
      tmp_win->flags &= ~MAXIMIZED;
      SetupFrame (tmp_win, tmp_win->orig_x, tmp_win->orig_y, tmp_win->orig_wd,
		  tmp_win->orig_ht, TRUE);
      SetBorder (tmp_win, True, True, True, None);
    }
  else
    {
      new_width = tmp_win->frame_width;
      new_height = tmp_win->frame_height;
      new_x = tmp_win->frame_x;
      new_y = tmp_win->frame_y;
      if (val1 > 0)
	{
	  new_width = val1 * val1_unit / 100 - 2;
	  new_x = 0;
	}
      if (val2 > 0)
	{
	  new_height = val2 * val2_unit / 100 - 2;
	  new_y = 0;
	}
      if ((val1 == 0) && (val2 == 0))
	{
	  new_x = 0;
	  new_y = 0;
	  new_height = Scr.MyDisplayHeight - 2;
	  new_width = Scr.MyDisplayWidth - 2;
	}
      tmp_win->flags |= MAXIMIZED;
      ConstrainSize (tmp_win, &new_width, &new_height);
      SetupFrame (tmp_win, new_x, new_y, new_width, new_height, TRUE);
      SetBorder (tmp_win, Scr.Hilite == tmp_win, True, True, None);
    }
}

MenuRoot *
FindPopup (char *action)
{
  char *tmp;
  MenuRoot *mr;

  GetNextToken (action, &tmp);

  if (tmp == NULL)
    return NULL;

  mr = Scr.AllMenus;
  while (mr != NULL)
    {
      if (mr->name != NULL)
	if (mystrcasecmp (tmp, mr->name) == 0)
	  {
	    free (tmp);
	    return mr;
	  }
      mr = mr->next;
    }
  free (tmp);
  return NULL;

}



void
Bell (XEvent * eventp, Window w, XfwmWindow * tmp_win, unsigned long context,
      char *action, int *Module)
{
  XBell (dpy, Scr.screen);
}


char *last_menu = NULL;
void
add_item_to_menu (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context,
		  char *action, int *Module)
{
  MenuRoot *mr;

  char *token, *rest, *item;

  rest = GetNextToken (action, &token);
  mr = FindPopup (token);
  if (mr == NULL)
    mr = NewMenuRoot (token, 0);
  if (last_menu != NULL)
    free (last_menu);
  last_menu = token;
  rest = GetNextToken (rest, &item);

  AddToMenu (mr, item, rest);
  free (item);

  MakeMenu (mr);
  return;
}


void
add_another_item (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context,
		  char *action, int *Module)
{
  MenuRoot *mr;

  char *rest, *item;

  if (last_menu == NULL)
    return;

  mr = FindPopup (last_menu);
  if (mr == NULL)
    return;
  rest = GetNextToken (action, &item);

  AddToMenu (mr, item, rest);
  free (item);

  MakeMenu (mr);
  return;
}

void
destroy_menu (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	      unsigned long context,
	      char *action, int *Module)
{
  MenuRoot *mr;

  char *token, *rest;

  rest = GetNextToken (action, &token);
  mr = FindPopup (token);
  if (mr == NULL)
    return;
  DestroyMenu (mr);
  return;

}

void
add_item_to_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context,
		  char *action, int *Module)
{
  MenuRoot *mr;

  char *token, *rest, *item;

  rest = GetNextToken (action, &token);
  mr = FindPopup (token);
  if (mr == NULL)
    mr = NewMenuRoot (token, 1);
  if (last_menu != NULL)
    free (last_menu);
  last_menu = token;
  rest = GetNextToken (rest, &item);

  AddToMenu (mr, item, rest);
  free (item);

  return;
}

void
Nop_func (XEvent * eventp, Window w, XfwmWindow * tmp_win, unsigned long context,
	  char *action, int *Module)
{

}



void
movecursor (XEvent * eventp, Window w, XfwmWindow * tmp_win, unsigned long context,
	    char *action, int *Module)
{
  int x, y, delta_x, delta_y, warp_x, warp_y;
  int val1, val2, val1_unit, val2_unit, n;

  n = GetTwoArguments (action, &val1, &val2, &val1_unit, &val2_unit);

  XQueryPointer (dpy, Scr.Root, &JunkRoot, &JunkChild,
		 &x, &y, &JunkX, &JunkY, &JunkMask);
  delta_x = 0;
  delta_y = 0;
  warp_x = 0;
  warp_y = 0;
  if (x >= Scr.MyDisplayWidth - 2)
    {
      delta_x = Scr.EdgeScrollX;
      warp_x = Scr.EdgeScrollX - 4;
    }
  if (y >= Scr.MyDisplayHeight - 2)
    {
      delta_y = Scr.EdgeScrollY;
      warp_y = Scr.EdgeScrollY - 4;
    }
  if (x < 2)
    {
      delta_x = -Scr.EdgeScrollX;
      warp_x = -Scr.EdgeScrollX + 4;
    }
  if (y < 2)
    {
      delta_y = -Scr.EdgeScrollY;
      warp_y = -Scr.EdgeScrollY + 4;
    }
  if (Scr.Vx + delta_x < 0)
    delta_x = -Scr.Vx;
  if (Scr.Vy + delta_y < 0)
    delta_y = -Scr.Vy;
  if (Scr.Vx + delta_x > Scr.VxMax)
    delta_x = Scr.VxMax - Scr.Vx;
  if (Scr.Vy + delta_y > Scr.VyMax)
    delta_y = Scr.VyMax - Scr.Vy;
  if ((delta_x != 0) || (delta_y != 0))
    {
      MoveViewport (Scr.Vx + delta_x, Scr.Vy + delta_y, True);
      XWarpPointer (dpy, Scr.Root, Scr.Root, 0, 0, Scr.MyDisplayWidth,
		    Scr.MyDisplayHeight,
		    x - warp_x,
		    y - warp_y);
    }
  XWarpPointer (dpy, Scr.Root, Scr.Root, 0, 0, Scr.MyDisplayWidth,
		Scr.MyDisplayHeight, x + val1 * val1_unit / 100 - warp_x,
		y + val2 * val2_unit / 100 - warp_y);
}


void
iconify_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context, char *action, int *Module)

{
  int val1;
  int val1_unit, n;

  if (DeferExecution (eventp, &w, &tmp_win, &context, SELECT,
		      ButtonRelease))
    return;

  n = GetOneArgument (action, (long *) &val1, &val1_unit);

  if (tmp_win->flags & ICONIFIED)
    {
      if (val1 <= 0)
	DeIconify (tmp_win);
    }
  else if (val1 >= 0)
	Iconify (tmp_win, eventp->xbutton.x_root - 5, eventp->xbutton.y_root - 5);
}

void
raise_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		unsigned long context, char *action, int *Module)
{
  char *junk, *junkC;
  unsigned long junkN;
  int junkD;

  if (DeferExecution (eventp, &w, &tmp_win, &context, SELECT, ButtonRelease))
    return;

  if (tmp_win)
    RaiseWindow (tmp_win);

  if (LookInList (Scr.TheList, tmp_win->name, &tmp_win->class, &junk,
		  &junkD, &junkD, &junkD, &junkC, &junkC, &junkN)
                  & STAYSONTOP_FLAG)
    tmp_win->flags |= ONTOP;
  KeepOnTop ();
}

void
lower_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		unsigned long context, char *action, int *Module)
{
  if (DeferExecution (eventp, &w, &tmp_win, &context, SELECT, ButtonRelease))
    return;

  LowerWindow (tmp_win);

  tmp_win->flags &= ~ONTOP;
}

void
destroy_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context, char *action, int *Module)
{
  if (DeferExecution (eventp, &w, &tmp_win, &context, DESTROY, ButtonRelease))
    return;

  if (XGetGeometry (dpy, tmp_win->w, &JunkRoot, &JunkX, &JunkY,
		    &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth) == 0)
    Destroy (tmp_win);
  else
    XKillClient (dpy, tmp_win->w);
  XSync (dpy, 0);
}

void
delete_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		 unsigned long context, char *action, int *Module)
{
  if (DeferExecution (eventp, &w, &tmp_win, &context, DESTROY, ButtonRelease))
    return;

  if (tmp_win->flags & DoesWmDeleteWindow)
    {
      send_clientmessage (dpy, tmp_win->w, _XA_WM_DELETE_WINDOW, CurrentTime);
      return;
    }
  else
    XBell (dpy, Scr.screen);
  XSync (dpy, 0);
}

void
close_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		unsigned long context, char *action, int *Module)
{
  if (DeferExecution (eventp, &w, &tmp_win, &context, DESTROY, ButtonRelease))
    return;

  if (tmp_win->flags & DoesWmDeleteWindow)
    {
      send_clientmessage (dpy, tmp_win->w, _XA_WM_DELETE_WINDOW, CurrentTime);
      return;
    }
  else if (XGetGeometry (dpy, tmp_win->w, &JunkRoot, &JunkX, &JunkY,
			 &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth) == 0)
    Destroy (tmp_win);
  else
    XKillClient (dpy, tmp_win->w);
  XSync (dpy, 0);
}

void
restart_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context, char *action, int *Module)
{
  Done (1, action);
}

void
exec_setup (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	    unsigned long context, char *action, int *Module)
{
  char *arg = NULL;

  action = GetNextToken (action, &arg);

  if (arg && (strcmp (arg, "") != 0))	/* specific shell was specified */
    {
      exec_shell_name = strdup (arg);
    }
  else
    /* no arg, so use $SHELL -- not working??? */
    {
      if (getenv ("SHELL"))
	exec_shell_name = strdup (getenv ("SHELL"));
      else
	exec_shell_name = strdup ("/bin/sh");	/* if $SHELL not set, use default */
    }
}

void
exec_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	       unsigned long context, char *action, int *Module)
{
  char *cmd = NULL;

  /* if it doesn't already have an 'exec' as the first word, add that
   * to keep down number of procs started */
  /* need to parse string better to do this right though, so not doing this
   * for now... */
  if (0 && mystrncasecmp (action, "exec", 4) != 0)
    {
      cmd = (char *) safemalloc (strlen (action) + 6);
      strcpy (cmd, "exec ");
      strcat (cmd, action);
    }
  else
    {
      cmd = strdup (action);
    }
  /* Use to grab the pointer here, but the fork guarantees that
   * we wont be held up waiting for the function to finish,
   * so the pointer-gram just caused needless delay and flashing
   * on the screen */
  /* Thought I'd try vfork and _exit() instead of regular fork().
   * The man page says that its better. */
  /* Not everyone has vfork! */
  if (!(fork ()))		/* child process */
    {
      if (execl (exec_shell_name, exec_shell_name, "-c", cmd, NULL) == -1)
	{
	  xfwm_msg (ERR, "exec_function", "execl failed (%s)", strerror (errno));
	  exit (100);
	}
    }
  free (cmd);
  return;
}

void
refresh_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context, char *action, int *Module)
{
  XSetWindowAttributes attributes;
  unsigned long valuemask;

  valuemask = CWOverrideRedirect | CWBackingStore | CWSaveUnder | CWBackPixmap;
  attributes.override_redirect = True;
  attributes.save_under = False;
  attributes.background_pixmap = None;
  attributes.backing_store = NotUseful;
  w = XCreateWindow (dpy, Scr.Root, 0, 0,
		     (unsigned int) Scr.MyDisplayWidth,
		     (unsigned int) Scr.MyDisplayHeight,
		     (unsigned int) 0,
		     CopyFromParent, (unsigned int) CopyFromParent,
		     (Visual *) CopyFromParent, valuemask,
		     &attributes);
  XMapWindow (dpy, w);
  XDestroyWindow (dpy, w);
  XFlush (dpy);
}


void
refresh_win_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		      unsigned long context, char *action, int *Module)
{
  XSetWindowAttributes attributes;
  unsigned long valuemask;

  if (DeferExecution (eventp, &w, &tmp_win, &context, SELECT, ButtonRelease))
    return;

  valuemask = CWOverrideRedirect | CWBackingStore | CWSaveUnder | CWBackPixmap;
  attributes.override_redirect = True;
  attributes.save_under = False;
  attributes.background_pixmap = None;
  attributes.backing_store = NotUseful;
  w = XCreateWindow (dpy,
		 (context == C_ICON) ? (tmp_win->icon_w) : (tmp_win->frame),
		     0, 0,
		     (unsigned int) Scr.MyDisplayWidth,
		     (unsigned int) Scr.MyDisplayHeight,
		     (unsigned int) 0,
		     CopyFromParent, (unsigned int) CopyFromParent,
		     (Visual *) CopyFromParent, valuemask,
		     &attributes);
  XMapWindow (dpy, w);
  XDestroyWindow (dpy, w);
  XFlush (dpy);
}

void
stick_function (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		unsigned long context, char *action, int *Module)
{
  if (DeferExecution (eventp, &w, &tmp_win, &context, SELECT, ButtonRelease))
    return;

  if (tmp_win->flags & STICKY)
    {
      tmp_win->flags &= ~STICKY;
    }
  else
    {
      tmp_win->flags |= STICKY;
    }
  BroadcastConfig (M_CONFIGURE_WINDOW, tmp_win);
  SetTitleBar (tmp_win, (Scr.Hilite == tmp_win), True);
  SetBorder (tmp_win, True, True, True, None);
}

void
wait_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	   unsigned long context, char *action, int *Module)
{
  Bool done = False;
  extern XfwmWindow *Tmp_win;

  while (!done)
    {
      if (My_XNextEvent (dpy, &Event))
	{
	  DispatchEvent ();
	  if (Event.type == MapNotify)
	    {
	      if ((Tmp_win) && (matchWildcards (action, Tmp_win->name) == True))
		done = True;
	      if ((Tmp_win) && (Tmp_win->class.res_class) &&
		(matchWildcards (action, Tmp_win->class.res_class) == True))
		done = True;
	      if ((Tmp_win) && (Tmp_win->class.res_name) &&
		  (matchWildcards (action, Tmp_win->class.res_name) == True))
		done = True;
	    }
	}
    }
}


void
flip_focus_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		 unsigned long context, char *action, int *Module)
{
  if (DeferExecution (eventp, &w, &tmp_win, &context, SELECT, ButtonRelease))
    return;

  /* Reorder the window list */
  if (Scr.Focus)
    {
      if (Scr.Focus->next)
	Scr.Focus->next->prev = Scr.Focus->prev;
      if (Scr.Focus->prev)
	Scr.Focus->prev->next = Scr.Focus->next;
      Scr.Focus->next = Scr.XfwmRoot.next;
      Scr.Focus->prev = &Scr.XfwmRoot;
      if (Scr.XfwmRoot.next)
	Scr.XfwmRoot.next->prev = Scr.Focus;
      Scr.XfwmRoot.next = Scr.Focus;
    }
  if (tmp_win != Scr.Focus)
    {
      if (tmp_win->next)
	tmp_win->next->prev = tmp_win->prev;
      if (tmp_win->prev)
	tmp_win->prev->next = tmp_win->next;
      tmp_win->next = Scr.XfwmRoot.next;
      tmp_win->prev = &Scr.XfwmRoot;
      if (Scr.XfwmRoot.next)
	Scr.XfwmRoot.next->prev = tmp_win;
      Scr.XfwmRoot.next = tmp_win;
    }

  FocusOn (tmp_win, 0);

}


void
focus_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	    unsigned long context, char *action, int *Module)
{
  if (DeferExecution (eventp, &w, &tmp_win, &context, SELECT, ButtonRelease))
    return;

  FocusOn (tmp_win, 0);
}


void
warp_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	   unsigned long context, char *action, int *Module)
{
  int val1_unit, val2_unit, n;
  int val1, val2;

  if (DeferExecution (eventp, &w, &tmp_win, &context, SELECT, ButtonRelease))
    return;

  n = GetTwoArguments (action, &val1, &val2, &val1_unit, &val2_unit);

  if (n == 2)
    WarpOn (tmp_win, val1, val1_unit, val2, val2_unit);
  else
    WarpOn (tmp_win, 0, 0, 0, 0);
}


void
popup_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	    unsigned long context, char *action, int *Module)
{
  MenuRoot *menu;
  extern int menuFromFrameOrWindowOrTitlebar;

  menu = FindPopup (action);
  if (menu == NULL)
    {
      xfwm_msg (ERR, "popup_func", "No such menu %s", action);
      return;
    }
  ActiveItem = NULL;
  ActiveMenu = NULL;
  menuFromFrameOrWindowOrTitlebar = FALSE;
  do_menu (menu, 0);
}

void
staysup_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	      unsigned long context, char *action, int *Module)
{
  MenuRoot *menu;
  extern int menuFromFrameOrWindowOrTitlebar;
  char *default_action = NULL, *menu_name = NULL;
  extern int menu_aborted;

  action = GetNextToken (action, &menu_name);
  GetNextToken (action, &default_action);
  menu = FindPopup (menu_name);
  if (menu == NULL)
    {
      if (menu_name != NULL)
	{
	  xfwm_msg (ERR, "staysup_func", "No such menu %s", menu_name);
	  free (menu_name);
	}
      if (default_action != NULL)
	free (default_action);
      return;
    }
  ActiveItem = NULL;
  ActiveMenu = NULL;
  menuFromFrameOrWindowOrTitlebar = FALSE;

  /* See bottom of windows.c for rationale behind this */
  if (eventp->type == ButtonPress)
    do_menu (menu, 1);
  else
    do_menu (menu, 0);

  if (menu_name != NULL)
    free (menu_name);
  if ((menu_aborted) && (default_action != NULL))
    ExecuteFunction (default_action, tmp_win, eventp, context, *Module);
  if (default_action != NULL)
    free (default_action);
}


void
quit_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	   unsigned long context, char *action, int *Module)
{
  if (master_pid != getpid ())
    kill (master_pid, SIGTERM);
  Done (0, NULL);
}

void
quit_screen_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context, char *action, int *Module)
{
  Done (0, NULL);
}

void
echo_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	   unsigned long context, char *action, int *Module)
{
  if (action && *action)
    {
      int len = strlen (action);
      if (action[len - 1] == '\n')
	action[len - 1] = '\0';
      xfwm_msg (INFO, "Echo", action);
    }
}

void
raiselower_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		 unsigned long context, char *action, int *Module)
{
  char *junk, *junkC;
  unsigned long junkN;
  int junkD;

  if (DeferExecution (eventp, &w, &tmp_win, &context, SELECT, ButtonRelease))
    return;
  if (tmp_win == NULL)
    return;

  if ((tmp_win == Scr.LastWindowRaised) ||
      (tmp_win->flags & VISIBLE))
    {
      LowerWindow (tmp_win);
      tmp_win->flags &= ~ONTOP;
    }
  else
    {
      RaiseWindow (tmp_win);
      if (LookInList (Scr.TheList, tmp_win->name, &tmp_win->class, &junk,
		    &junkD, &junkD, &junkD, &junkC, &junkC, &junkN)
                     & STAYSONTOP_FLAG)
	tmp_win->flags |= ONTOP;
      KeepOnTop ();
    }
}

void
SetEdgeScroll (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	       unsigned long context, char *action, int *Module)
{
  int val1, val2, val1_unit, val2_unit, n;

  n = GetTwoArguments (action, &val1, &val2, &val1_unit, &val2_unit);
  if (n != 2)
    {
      xfwm_msg (ERR, "SetEdgeScroll", "EdgeScroll requires two arguments");
      return;
    }

  /*
   * ** if edgescroll >1000 and < 100000m
   * ** wrap at edges of desktop (a "spherical" desktop)
   */
  if (val1 >= 1000)
    {
      val1 /= 1000;
      Scr.flags |= EdgeWrapX;
    }
  else
    {
      Scr.flags &= ~EdgeWrapX;
    }
  if (val2 >= 1000)
    {
      val2 /= 1000;
      Scr.flags |= EdgeWrapY;
    }
  else
    {
      Scr.flags &= ~EdgeWrapY;
    }

  Scr.EdgeScrollX = val1 * val1_unit / 100;
  Scr.EdgeScrollY = val2 * val2_unit / 100;

  checkPanFrames ();
}

void
SetEdgeResistance (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		   unsigned long context, char *action, int *Module)
{
  int val1, val2, val1_unit, val2_unit, n;

  n = GetTwoArguments (action, &val1, &val2, &val1_unit, &val2_unit);
  if (n != 2)
    {
      xfwm_msg (ERR, "SetEdgeResistance", "EdgeResistance requires two arguments");
      return;
    }

  Scr.ScrollResistance = val1;
  Scr.MoveResistance = val2;
}

void
SetColormapFocus (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context, char *action, int *Module)
{
  if (mystrncasecmp (action, "FollowsFocus", 12) == 0)
    {
      Scr.ColormapFocus = COLORMAP_FOLLOWS_FOCUS;
    }
  else if (mystrncasecmp (action, "FollowsMouse", 12) == 0)
    {
      Scr.ColormapFocus = COLORMAP_FOLLOWS_MOUSE;
    }
  else
    {
      xfwm_msg (ERR, "SetColormapFocus",
	      "ColormapFocus requires 1 arg: FollowsFocus or FollowsMouse");
      return;
    }
}

void
SetClick (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	  unsigned long context, char *action, int *Module)
{
  int val1;
  int val1_unit, n;

  n = GetOneArgument (action, (long *) &val1, &val1_unit);
  if (n != 1)
    {
      xfwm_msg (ERR, "SetClick", "ClickTime requires 1 argument");
      return;
    }

  Scr.ClickTime = val1;
}

void
SetXOR (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	unsigned long context, char *action, int *Module)
{
  int val1;
  int val1_unit, n;
  XGCValues gcv;
  unsigned long gcm;

  n = GetOneArgument (action, (long *) &val1, &val1_unit);
  if (n != 1)
    {
      xfwm_msg (ERR, "SetXOR", "XORValue requires 1 argument");
      return;
    }

  gcm = GCFunction | GCLineWidth | GCForeground | GCSubwindowMode;
  gcv.function = GXxor;
  gcv.line_width = 0;
  /* use passed in value, or try to calculate appropriate value if 0 */
  /* ctwm method: */
  /* gcv.foreground = (val1)?(val1):((((unsigned long) 1) << Scr.d_depth) - 1); */
  /* Xlib programming manual suggestion: */
  gcv.foreground = (val1) ? (val1) : (BlackPixel (dpy, Scr.screen) ^ WhitePixel (dpy, Scr.screen));
  gcv.subwindow_mode = IncludeInferiors;
  Scr.DrawGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);
}

void
SetDeskSize (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	     unsigned long context, char *action, int *Module)
{
  int val1, val2, val1_unit, val2_unit, n;

  n = GetTwoArguments (action, &val1, &val2, &val1_unit, &val2_unit);
  if (n != 2)
    {
      xfwm_msg (ERR, "SetDeskSize", "DesktopSize requires two arguments");
      return;
    }
  if ((val1_unit != Scr.MyDisplayWidth) ||
      (val2_unit != Scr.MyDisplayHeight))
    {
      xfwm_msg (ERR, "SetDeskSize", "DeskTopSize arguments should be unitless");
    }

  Scr.VxMax = val1;
  Scr.VyMax = val2;
  Scr.VxMax = Scr.VxMax * Scr.MyDisplayWidth - Scr.MyDisplayWidth;
  Scr.VyMax = Scr.VyMax * Scr.MyDisplayHeight - Scr.MyDisplayHeight;
  if (Scr.VxMax < 0)
    Scr.VxMax = 0;
  if (Scr.VyMax < 0)
    Scr.VyMax = 0;
  Broadcast (M_NEW_PAGE, 5, Scr.Vx, Scr.Vy, Scr.CurrentDesk, Scr.VxMax, Scr.VyMax, 0, 0);

  checkPanFrames ();
}

void
setModulePath (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	       unsigned long context, char *action, int *Module)
{
  static char *ptemp = NULL;
  char *tmp;

  if (ptemp == NULL)
    ptemp = ModulePath;

  if ((ModulePath != ptemp) && (ModulePath != NULL))
    free (ModulePath);
  tmp = stripcpy (action);
  ModulePath = envDupExpand (tmp, 0);
  free (tmp);
}

void
SetHiColor (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	    unsigned long context, char *action, int *Module)
{
  XGCValues gcv;
  unsigned long gcm;
  char *hifore = NULL, *hiback = NULL;
  extern char *white, *black;
  XfwmWindow *hilight;
  XfwmDecor *fl = &Scr.DefaultDecor;

  action = GetNextToken (action, &hifore);
  GetNextToken (action, &hiback);

  if (Scr.d_depth > 2)
    {
      if (hifore != NULL)
	{
	  fl->HiColors.fore = GetColor (hifore);
	}
      if (hiback != NULL)
	{
	  fl->HiColors.back = GetColor (hiback);
	}
      fl->HiRelief.fore = GetHilite (fl->HiColors.back);
      fl->HiRelief.back = GetShadow (fl->HiColors.back);
    }
  else
    {
      fl->HiColors.back = GetColor (white);
      fl->HiColors.fore = GetColor (black);
      fl->HiRelief.back = GetColor (black);
      fl->HiRelief.fore = GetColor (white);
    }
  if (hifore)
    free (hifore);
  if (hiback)
    free (hiback);
  gcm = GCFunction | GCPlaneMask | GCGraphicsExposures | GCLineWidth | GCForeground |
    GCBackground;
  gcv.foreground = fl->HiRelief.fore;
  gcv.background = fl->HiRelief.back;
  gcv.fill_style = FillSolid;
  gcv.plane_mask = AllPlanes;
  gcv.function = GXcopy;
  gcv.graphics_exposures = False;
  gcv.line_width = 0;
  if (fl->HiReliefGC != NULL)
    {
      XFreeGC (dpy, fl->HiReliefGC);
    }
  fl->HiReliefGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);

  gcv.foreground = fl->HiRelief.back;
  gcv.background = fl->HiRelief.fore;
  if (fl->HiShadowGC != NULL)
    {
      XFreeGC (dpy, fl->HiShadowGC);
    }
  fl->HiShadowGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);

  if ((Scr.flags & WindowsCaptured) && (Scr.Hilite != NULL))
    {
      hilight = Scr.Hilite;
      SetBorder (Scr.Hilite, False, True, True, None);
      SetBorder (hilight, True, True, True, None);
    }
}

void
SetLoColor (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	    unsigned long context, char *action, int *Module)
{
  XGCValues gcv;
  unsigned long gcm;
  char *lofore = NULL, *loback = NULL;
  extern char *white, *black;
  XfwmDecor *fl = &Scr.DefaultDecor;

  action = GetNextToken (action, &lofore);
  GetNextToken (action, &loback);
  if (Scr.d_depth > 2)
    {
      if (lofore != NULL)
	{
	  fl->LoColors.fore = GetColor (lofore);
	}
      if (loback != NULL)
	{
	  fl->LoColors.back = GetColor (loback);
	}
      fl->LoRelief.back = GetShadow (fl->LoColors.back);
      fl->LoRelief.fore = GetHilite (fl->LoColors.back);
    }
  else
    {
      fl->LoColors.back = GetColor (white);
      fl->LoColors.fore = GetColor (black);
      fl->LoRelief.back = GetColor (black);
      fl->LoRelief.fore = GetColor (white);
    }
  if (lofore)
    free (lofore);
  if (loback)
    free (loback);
  gcm = GCFunction | GCPlaneMask | GCGraphicsExposures | GCLineWidth | GCForeground |
    GCBackground;
  gcv.foreground = fl->LoRelief.fore;
  gcv.background = fl->LoRelief.back;
  gcv.fill_style = FillSolid;
  gcv.plane_mask = AllPlanes;
  gcv.function = GXcopy;
  gcv.graphics_exposures = False;
  gcv.line_width = 0;
  if (fl->LoReliefGC != NULL)
    {
      XFreeGC (dpy, fl->LoReliefGC);
    }
  fl->LoReliefGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);

  gcv.foreground = fl->LoRelief.back;
  gcv.background = fl->LoRelief.fore;
  if (fl->LoShadowGC != NULL)
    {
      XFreeGC (dpy, fl->LoShadowGC);
    }
  fl->LoShadowGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);
}

void
SafeDefineCursor (Window w, Cursor cursor)
{
  if (w)
    XDefineCursor (dpy, w, cursor);
}

void
CursorColor (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
	     unsigned long context, char *action, int *Module)
{
  char *hifore = NULL, *hiback = NULL;
  XColor back, fore;

  action = GetNextToken (action, &hifore);
  action = GetNextToken (action, &hiback);
  if (!hifore || !hiback)
    {
      xfwm_msg (ERR, "CursorColor", "Requires 2 parameters");
      return;
    }

  if (Scr.d_depth > 2)
    {
      fore = GetXColor (hifore);
      back = GetXColor (hiback);
      XRecolorCursor (dpy, Scr.XfwmCursors[POSITION], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[DEFAULT], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[SYS], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[TITLE_CURSOR], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[MOVE], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[MENU], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[WAIT], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[SELECT], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[DESTROY], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[LEFT], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[RIGHT], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[TOP], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[BOTTOM], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[TOP_LEFT], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[TOP_RIGHT], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[BOTTOM_LEFT], &fore, &back);
      XRecolorCursor (dpy, Scr.XfwmCursors[BOTTOM_RIGHT], &fore, &back);
    }
}

void
SetMenuColor (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	      unsigned long context, char *action, int *Module)
{
  XGCValues gcv;
  unsigned long gcm;
  char *fore = NULL, *back = NULL, *selfore = NULL, *selback = NULL;
  extern char *white, *black;

  action = GetNextToken (action, &fore);
  action = GetNextToken (action, &back);
  action = GetNextToken (action, &selfore);
  action = GetNextToken (action, &selback);

  if (Scr.d_depth > 2)
    {
      if (fore != NULL)
	{
	  Scr.MenuColors.fore = GetColor (fore);
	}
      if (back != NULL)
	{
	  Scr.MenuColors.back = GetColor (back);
	}
      if (selfore != NULL)
	{
	  Scr.MenuSelColors.fore = GetColor (selfore);
	}
      if (back != NULL)
	{
	  Scr.MenuSelColors.back = GetColor (selback);
	}
      Scr.MenuRelief.back = GetShadow (Scr.MenuColors.back);
      Scr.MenuRelief.fore = GetHilite (Scr.MenuColors.back);
    }
  else
    {
      Scr.MenuColors.back = GetColor (white);
      Scr.MenuColors.fore = GetColor (black);
      Scr.MenuRelief.back = GetColor (black);
      Scr.MenuRelief.fore = GetColor (white);
      Scr.MenuSelColors.back = GetColor (white);
      Scr.MenuSelColors.fore = GetColor (black);
    }

  gcm = GCFunction | GCPlaneMask | GCGraphicsExposures | GCLineWidth | GCForeground | GCBackground;

  gcv.foreground = Scr.MenuRelief.fore;
  gcv.background = Scr.MenuRelief.back;
  gcv.fill_style = FillSolid;
  gcv.plane_mask = AllPlanes;
  gcv.function = GXcopy;
  gcv.graphics_exposures = False;
  gcv.line_width = 0;
  if (Scr.MenuReliefGC != NULL)
    XChangeGC (dpy, Scr.MenuReliefGC, gcm, &gcv);
  else
    Scr.MenuReliefGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);

  gcv.foreground = Scr.MenuRelief.back;
  gcv.background = Scr.MenuRelief.fore;
  if (Scr.MenuShadowGC != NULL)
    XChangeGC (dpy, Scr.MenuShadowGC, gcm, &gcv);
  else
    Scr.MenuShadowGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);

  gcv.foreground = Scr.MenuColors.fore;
  gcv.background = Scr.MenuColors.back;
  if (Scr.MenuGC != NULL)
    XChangeGC (dpy, Scr.MenuGC, gcm, &gcv);
  else
    Scr.MenuGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);

  gcv.foreground = Scr.MenuSelColors.fore;
  gcv.background = Scr.MenuSelColors.back;
  if (Scr.MenuSelGC != NULL)
    XChangeGC (dpy, Scr.MenuSelGC, gcm, &gcv);
  else
    Scr.MenuSelGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);
  MakeMenus ();
  if (fore != NULL)
    free (fore);
  if (back != NULL)
    free (back);
  if (selfore != NULL)
    free (selfore);
  if (selback != NULL)
    free (selback);
}

void 
SetMenuFont (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	     unsigned long context, char *action, int *Module)
{
  XGCValues gcv;
  unsigned long gcm;
  char *font = NULL;

  action = GetNextToken (action, &font);

  if ((font == NULL) ||
      (Scr.StdFont.font = GetFontOrFixed (dpy, font)) == NULL)
    {
      fprintf (stderr,
	     "[SetMenuFont]: Error -- Couldn't load font '%s' or 'fixed'\n",
	       (font == NULL) ? ("NULL") : (font));
      exit (1);
    }
  Scr.StdFont.height = Scr.StdFont.font->ascent + Scr.StdFont.font->descent;
  Scr.StdFont.y = Scr.StdFont.font->ascent;
  Scr.EntryHeight = Scr.StdFont.height + HEIGHT_EXTRA;

  gcm = GCFont;
  gcv.font = Scr.StdFont.font->fid;

  if (Scr.MenuReliefGC != NULL)
    XChangeGC (dpy, Scr.MenuReliefGC, gcm, &gcv);
  else
    Scr.MenuReliefGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);

  if (Scr.MenuShadowGC != NULL)
    XChangeGC (dpy, Scr.MenuShadowGC, gcm, &gcv);
  else
    Scr.MenuShadowGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);

  if (Scr.MenuGC != NULL)
    XChangeGC (dpy, Scr.MenuGC, gcm, &gcv);
  else
    Scr.MenuGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);

  if (Scr.MenuSelGC != NULL)
    XChangeGC (dpy, Scr.MenuSelGC, gcm, &gcv);
  else
    Scr.MenuSelGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);
  MakeMenus ();
  free (font);
}

Boolean ReadButtonFace (char *s, ButtonFace * bf, int button, int verbose);
void FreeButtonFace (Display * dpy, ButtonFace * bf);

char *ReadTitleButton (char *s, TitleButton * tb, Boolean append, int button);

void
SetTitleStyle (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	       unsigned long context, char *action, int *Module)
{
  char *prev = action;
  XfwmDecor *fl = &Scr.DefaultDecor;

  fl->titlebar.flags |= HOffCenter;
  fl->titlebar.flags &= ~HRight;
  ReadTitleButton (prev, &fl->titlebar, False, -1);
}

void
LoadIconFont (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	      unsigned long context, char *action, int *Module)
{
  char *font;
  XfwmWindow *tmp;

  action = GetNextToken (action, &font);

  if ((Scr.IconFont.font = GetFontOrFixed (dpy, font)) == NULL)
    {
      xfwm_msg (ERR, "LoadIconFont", "Couldn't load font '%s' or 'fixed'\n",
		font);
      free (font);
      return;
    }
  Scr.IconFont.height =
    Scr.IconFont.font->ascent + Scr.IconFont.font->descent;
  Scr.IconFont.y = Scr.IconFont.font->ascent;

  free (font);
  tmp = Scr.XfwmRoot.next;
  while (tmp != NULL)
    {
      RedoIconName (tmp);

      if (tmp->flags & ICONIFIED)
	{
	  DrawIconWindow (tmp);
	}
      tmp = tmp->next;
    }
}

void
LoadWindowFont (XEvent * eventp, Window win, XfwmWindow * tmp_win,
		unsigned long context, char *action, int *Module)
{
  char *font;
  XfwmWindow *tmp, *hi;
  int x, y, w, h, extra_height;
  XFontStruct *newfont;
  XfwmDecor *fl = &Scr.DefaultDecor;

  action = GetNextToken (action, &font);

  if ((newfont = GetFontOrFixed (dpy, font)) != NULL)
    {
      fl->WindowFont.font = newfont;
      fl->WindowFont.height =
	fl->WindowFont.font->ascent + fl->WindowFont.font->descent;
      fl->WindowFont.y = fl->WindowFont.font->ascent;
      extra_height = fl->TitleHeight;
      fl->TitleHeight = fl->WindowFont.font->ascent + fl->WindowFont.font->descent + 3;
      extra_height -= fl->TitleHeight;
      tmp = Scr.XfwmRoot.next;
      hi = Scr.Hilite;
      while (tmp != NULL)
	{
	  if (!(tmp->flags & TITLE)
	    )
	    {
	      tmp = tmp->next;
	      continue;
	    }
	  x = tmp->frame_x;
	  y = tmp->frame_y;
	  w = tmp->frame_width;
	  h = tmp->frame_height - extra_height;
	  tmp->frame_x = 0;
	  tmp->frame_y = 0;
	  tmp->frame_height = 0;
	  tmp->frame_width = 0;
	  SetupFrame (tmp, x, y, w, h, True);
	  SetTitleBar (tmp, True, True);
	  SetTitleBar (tmp, False, True);
	  tmp = tmp->next;
	}
      SetTitleBar (hi, True, True);

    }
  else
    {
      xfwm_msg (ERR, "LoadWindowFont", "Couldn't load font '%s' or 'fixed'\n",
		font);
    }

  free (font);
}

void
FreeButtonFace (Display * dpy, ButtonFace * bf)
{
  switch (bf->style)
    {
    case GradButton:
      free (bf->u.grad.pixels);
      bf->u.grad.pixels = NULL;
      break;
    default:
      break;
    }
  if (bf->next)
    {
      FreeButtonFace (dpy, bf->next);
      free (bf->next);
    }
  bf->next = NULL;
  bf->style = SimpleButton;
}

/*****************************************************************************
 * 
 * Reads a button face line into a structure (veliaa@rpi.edu)
 *
 ****************************************************************************/
Boolean
ReadButtonFace (char *s, ButtonFace * bf, int button, int verbose)
{
  int offset;
  char style[256], *file;
  char *action = s;

  if (sscanf (s, "%s%n", style, &offset) < 1)
    {
      if (verbose)
	xfwm_msg (ERR, "ReadButtonFace", "error in face: %s", action);
      return False;
    }

  s += offset;

  FreeButtonFace (dpy, bf);

  /* determine button style */
  if (mystrncasecmp (style, "Simple", 6) == 0)
    {
      bf->style = SimpleButton;
    }
  else if (mystrncasecmp (style, "Default", 7) == 0)
    {
      int b = -1, n = sscanf (s, "%d%n", &b, &offset);

      if (n < 1)
	{
	  if (button == -1)
	    {
	      if (verbose)
		xfwm_msg (ERR, "ReadButtonFace",
			  "need default button number to load");
	      return False;
	    }
	  b = button;
	}
      s += offset;
      if ((b > 0) && (b <= 10))
	LoadDefaultButton (bf, b);
      else
	{
	  if (verbose)
	    xfwm_msg (ERR, "ReadButtonFace",
		      "button number out of range: %d", b);
	  return False;
	}
    }
  else if (mystrncasecmp (style, "Solid", 5) == 0)
    {
      s = GetNextToken (s, &file);
      if (file && *file)
	{
	  bf->style = SolidButton;
	  bf->u.back = GetColor (file);
	}
      else
	{
	  if (verbose)
	    xfwm_msg (ERR, "ReadButtonFace",
		      "no color given for Solid face type: %s",
		      action);
	  return False;
	}
      free (file);
    }
  else if (mystrncasecmp (style, "Gradient", 8) == 0)
    {
      char *item, **s_colors;
      int npixels, nsegs, i, sum, *perc;
      Pixel *pixels;

      npixels = ((Scr.d_depth > 8) ? 32 : 8);

      if (!(s = GetNextToken (s, &item)) || (item == NULL))
	{
	  if (verbose)
	    xfwm_msg (ERR, "ReadButtonFace",
		      "incomplete gradient style");
	  return False;
	}

      if (!(isdigit (*item)))
	{
	  s_colors = (char **) safemalloc (sizeof (char *) * 2);
	  perc = (int *) safemalloc (sizeof (int));
	  nsegs = 1;
	  s_colors[0] = item;
	  s = GetNextToken (s, &s_colors[1]);
	  perc[0] = 100;
	}
      else
	{
	  nsegs = atoi (item);
	  free (item);
	  if (nsegs < 1)
	    nsegs = 1;
	  if (nsegs > 128)
	    nsegs = 128;
	  s_colors = (char **) safemalloc (sizeof (char *) * (nsegs + 1));
	  perc = (int *) safemalloc (sizeof (int) * nsegs);
	  for (i = 0; i <= nsegs; ++i)
	    {
	      s = GetNextToken (s, &s_colors[i]);
	      if (i < nsegs)
		{
		  s = GetNextToken (s, &item);
		  if (item)
		    perc[i] = atoi (item);
		  else
		    perc[i] = 0;
		  free (item);
		}
	    }
	}

      for (i = 0, sum = 0; i < nsegs; ++i)
	sum += perc[i];

      if (sum != 100)
	{
	  if (verbose)
	    xfwm_msg (ERR, "ReadButtonFace",
		      "multi gradient lenghts must sum to 100");
	  for (i = 0; i <= nsegs; ++i)
	    free (s_colors[i]);
	  free (s_colors);
	  return False;
	}

      if (npixels < 2)
	npixels = 2;
      if (npixels > 128)
	npixels = 128;

      pixels = AllocNonlinearGradient (s_colors, perc, nsegs, npixels);
      for (i = 0; i <= nsegs; ++i)
	free (s_colors[i]);
      free (s_colors);

      if (!pixels)
	{
	  if (verbose)
	    xfwm_msg (ERR, "ReadButtonFace",
		      "couldn't create gradient");
	  return False;
	}

      bf->u.grad.pixels = pixels;
      bf->u.grad.npixels = npixels;

      bf->style = GradButton;
    }
  else
    {
      if (verbose)
	xfwm_msg (ERR, "ReadButtonFace",
		  "unknown style %s: %s", style, action);
      return False;
    }
  return True;
}


/*****************************************************************************
 * 
 * Reads a title button description (veliaa@rpi.edu)
 *
 ****************************************************************************/
char *
ReadTitleButton (char *s, TitleButton * tb, Boolean append, int button)
{
  char *end = NULL, *spec;
  ButtonFace tmpbf;
  enum ButtonState bs = MaxButtonState;
  int i = 0, all = 0, pstyle = 0;

  while (isspace (*s))
    ++s;
  for (; i < MaxButtonState; ++i)
    if (mystrncasecmp (button_states[i], s,
		       strlen (button_states[i])) == 0)
      {
	bs = i;
	break;
      }
  if (bs != MaxButtonState)
    s += strlen (button_states[bs]);
  else
    all = 1;
  while (isspace (*s))
    ++s;
  spec = s;

  while (isspace (*spec))
    ++spec;
  /* setup temporary in case button read fails */
  tmpbf.style = SimpleButton;
  tmpbf.next = NULL;

  if (ReadButtonFace (spec, &tmpbf, button, True))
    {
      int b = all ? 0 : bs;
      FreeButtonFace (dpy, &tb->state[b]);
      tb->state[b] = tmpbf;
      if (all)
	for (i = 1; i < MaxButtonState; ++i)
	  ReadButtonFace (spec, &tb->state[i], button, False);
    }
  if (pstyle)
    {
      free (spec);
      ++end;
      while (isspace (*end))
	++end;
    }
  return end;
}


/*****************************************************************************
 * 
 * Updates window decoration styles (veliaa@rpi.edu)
 *
 ****************************************************************************/
void
UpdateDecor (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
	     unsigned long context, char *action, int *Module)
{
  XfwmWindow *fw = Scr.XfwmRoot.next;

  for (; fw != NULL; fw = fw->next)
    {
      {
	SetBorder (fw, True, True, True, None);
	SetBorder (fw, False, True, True, None);
      }
    }
  SetBorder (Scr.Hilite, True, True, True, None);
}


/*****************************************************************************
 * 
 * Changes a button decoration style (changes by veliaa@rpi.edu)
 *
 ****************************************************************************/
#define SetButtonFlag(a) \
        do { \
             int i; \
             if (multi) { \
               if (multi&1) \
                 for (i=0;i<2;++i) \
                   if (set) \
                     fl->left_buttons[i].flags |= (a); \
                   else \
                     fl->left_buttons[i].flags &= ~(a); \
               if (multi&2) \
                 for (i=0;i<2;++i) \
                   if (set) \
                     fl->right_buttons[i].flags |= (a); \
                   else \
                     fl->right_buttons[i].flags &= ~(a); \
             } else \
                 if (set) \
                   tb->flags |= (a); \
                 else \
                   tb->flags &= ~(a); } while (0)

/**************************************************************************
 *
 * Direction = 1 ==> "Next" operation
 * Direction = -1 ==> "Previous" operation 
 * Direction = 0 ==> operation on current window (returns pass or fail)
 *
 **************************************************************************/
XfwmWindow *
Circulate (char *action, int Direction, char **restofline)
{
  int l, pass = 0;
  XfwmWindow *fw, *found = NULL;
  char *t, *tstart, *name = NULL, *expression, *condition, *prev_condition = NULL;
  char *orig_expr;
  Bool needsCurrentDesk = 0;
  Bool needsCurrentPage = 0;

  Bool needsName = 0;
  Bool needsNotName = 0;
  Bool useCirculateHit = (Direction) ? 0 : 1;	/* override for Current [] */
  Bool useCirculateHitIcon = (Direction) ? 0 : 1;
  unsigned long onFlags = 0;
  unsigned long offFlags = 0;

  l = 0;

  if (action == NULL)
    return NULL;

  t = action;
  while (isspace (*t) && (*t != 0))
    t++;
  if (*t == '[')
    {
      t++;
      tstart = t;

      while ((*t != 0) && (*t != ']'))
	{
	  t++;
	  l++;
	}
      if (*t == 0)
	{
	  xfwm_msg (ERR, "Circulate", "Conditionals require closing brace");
	  return NULL;
	}

      *restofline = t + 1;

      orig_expr = expression = safemalloc (l + 1);
      strncpy (expression, tstart, l);
      expression[l] = 0;
      expression = GetNextToken (expression, &condition);
      while ((condition != NULL) && (strlen (condition) > 0))
	{
	  if (mystrcasecmp (condition, "Iconic") == 0)
	    onFlags |= ICONIFIED;
	  else if (mystrcasecmp (condition, "!Iconic") == 0)
	    offFlags |= ICONIFIED;
	  else if (mystrcasecmp (condition, "Visible") == 0)
	    onFlags |= VISIBLE;
	  else if (mystrcasecmp (condition, "!Visible") == 0)
	    offFlags |= VISIBLE;
	  else if (mystrcasecmp (condition, "Sticky") == 0)
	    onFlags |= STICKY;
	  else if (mystrcasecmp (condition, "!Sticky") == 0)
	    offFlags |= STICKY;
	  else if (mystrcasecmp (condition, "Maximized") == 0)
	    onFlags |= MAXIMIZED;
	  else if (mystrcasecmp (condition, "!Maximized") == 0)
	    offFlags |= MAXIMIZED;
	  else if (mystrcasecmp (condition, "Transient") == 0)
	    onFlags |= TRANSIENT;
	  else if (mystrcasecmp (condition, "!Transient") == 0)
	    offFlags |= TRANSIENT;
	  else if (mystrcasecmp (condition, "Raised") == 0)
	    onFlags |= RAISED;
	  else if (mystrcasecmp (condition, "!Raised") == 0)
	    offFlags |= RAISED;
	  else if (mystrcasecmp (condition, "CurrentDesk") == 0)
	    needsCurrentDesk = 1;
	  else if (mystrcasecmp (condition, "CurrentPage") == 0)
	    {
	      needsCurrentDesk = 1;
	      needsCurrentPage = 1;
	    }
	  else if (mystrcasecmp (condition, "CurrentPageAnyDesk") == 0 ||
		   mystrcasecmp (condition, "CurrentScreen") == 0)
	    needsCurrentPage = 1;
	  else if (mystrcasecmp (condition, "CirculateHit") == 0)
	    useCirculateHit = 1;
	  else if (mystrcasecmp (condition, "CirculateHitIcon") == 0)
	    useCirculateHitIcon = 1;
	  else if (!needsName && !needsNotName)		/* only 1st name to avoid mem leak */
	    {
	      name = condition;
	      condition = NULL;
	      if (name[0] == '!')
		{
		  needsNotName = 1;
		  name++;
		}
	      else
		needsName = 1;
	    }
	  if (prev_condition)
	    free (prev_condition);
	  prev_condition = condition;
	  expression = GetNextToken (expression, &condition);
	}
      if (prev_condition != NULL)
	free (prev_condition);
      if (orig_expr != NULL)
	free (orig_expr);
    }
  else
    *restofline = t;

  if (Scr.Focus != NULL)
    {
      if (Direction == 1)
	fw = Scr.Focus->prev;
      else if (Direction == -1)
	fw = Scr.Focus->next;
      else
	fw = Scr.Focus;
    }
  else
    fw = Scr.XfwmRoot.prev;

  while ((pass < 3) && (found == NULL))
    {
      while ((fw != NULL) && (found == NULL) && (fw != &Scr.XfwmRoot))
	{
	  /* Make CirculateUp and CirculateDown take args. by Y.NOMURA */
	  if ((onFlags & fw->flags) == onFlags &&
	      (offFlags & fw->flags) == 0 &&
	      (useCirculateHit || !(fw->flags & CirculateSkip)) &&
	      ((useCirculateHitIcon && fw->flags & ICONIFIED) ||
	       !(fw->flags & CirculateSkipIcon && fw->flags & ICONIFIED)) &&
	      (!needsCurrentDesk || fw->Desk == Scr.CurrentDesk) &&
	      (!needsCurrentPage || (fw->frame_x < Scr.MyDisplayWidth &&
				     fw->frame_y < Scr.MyDisplayHeight &&
				     fw->frame_x + fw->frame_width > 0 &&
				     fw->frame_y + fw->frame_height > 0)
	      ) &&
	      ((!needsName ||
		matchWildcards (name, fw->name) ||
		matchWildcards (name, fw->icon_name) ||
		(fw->class.res_class && matchWildcards (name, fw->class.res_class)) ||
	   (fw->class.res_name && matchWildcards (name, fw->class.res_name))
	       ) &&
	       (!needsNotName ||
		!(matchWildcards (name, fw->name) ||
		  matchWildcards (name, fw->icon_name) ||
		  (fw->class.res_class && matchWildcards (name, fw->class.res_class)) ||
	   (fw->class.res_name && matchWildcards (name, fw->class.res_name))
		)
	       )
	      ))
	    found = fw;
	  else
	    {
	      if (Direction == 1)
		fw = fw->prev;
	      else
		fw = fw->next;
	    }
	  if (Direction == 0)
	    {
	      if (needsName)
		free (name);
	      else if (needsNotName)
		free (name - 1);
	      return found;
	    }
	}
      if ((fw == NULL) || (fw == &Scr.XfwmRoot))
	{
	  if (Direction == 1)
	    {
	      /* Go to end of list */
	      fw = &Scr.XfwmRoot;
	      while ((fw) && (fw->next != NULL))
		{
		  fw = fw->next;
		}
	    }
	  else
	    {
	      /* GO to top of list */
	      fw = Scr.XfwmRoot.next;
	    }
	}
      pass++;
    }
  if (needsName)
    free (name);
  else if (needsNotName)
    free (name - 1);
  return found;

}

void
PrevFunc (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
	  unsigned long context, char *action, int *Module)
{
  XfwmWindow *found;
  char *restofline;

  found = Circulate (action, -1, &restofline);
  if (found != NULL)
    {
      ExecuteFunction (restofline, found, eventp, C_WINDOW, *Module);
    }

}

void
NextFunc (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
	  unsigned long context, char *action, int *Module)
{
  XfwmWindow *found;
  char *restofline;

  found = Circulate (action, 1, &restofline);
  if (found != NULL)
    {
      ExecuteFunction (restofline, found, eventp, C_WINDOW, *Module);
    }

}

void
NoneFunc (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
	  unsigned long context, char *action, int *Module)
{
  XfwmWindow *found;
  char *restofline;

  found = Circulate (action, 1, &restofline);
  if (found == NULL)
    {
      ExecuteFunction (restofline, NULL, eventp, C_ROOT, *Module);
    }
}

void
CurrentFunc (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
	     unsigned long context, char *action, int *Module)
{
  XfwmWindow *found;
  char *restofline;

  found = Circulate (action, 0, &restofline);
  if (found != NULL)
    {
      ExecuteFunction (restofline, found, eventp, C_WINDOW, *Module);
    }
}

void
WindowIdFunc (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
	      unsigned long context, char *action, int *Module)
{
  XfwmWindow *found = NULL, *t;
  char *restofline, *num;
  unsigned long win;

  restofline = strdup (action);
  num = GetToken (&restofline);
  win = (unsigned long) strtol (num, NULL, 0);	/* SunOS doesn't have strtoul */
  for (t = Scr.XfwmRoot.next; t != NULL; t = t->next)
    {
      if (t->w == win)
	{
	  found = t;
	  break;
	}
    }
  if (found)
    {
      ExecuteFunction (restofline, found, eventp, C_WINDOW, *Module);
    }
  if (restofline)
    free (restofline);
}


void
module_zapper (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
	       unsigned long context, char *action, int *Module)
{
  char *condition;

  GetNextToken (action, &condition);
  KillModuleByName (condition);
  free (condition);
}

/***********************************************************************
 *
 *  Procedure:
 *	Reborder - Removes xfwm border windows
 *
 ************************************************************************/
void
Recapture (XEvent * eventp, Window junk, XfwmWindow * tmp_win,
	   unsigned long context, char *action, int *Module)
{
  BlackoutScreen ();		/* if they want to hide the recapture */
  CaptureAllWindows ();
  UnBlackoutScreen ();
}
