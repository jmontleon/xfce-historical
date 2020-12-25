
/****************************************************************************
 * This module is all original code 
 * by Rob Nation 
 * Copyright 1993, Robert Nation
 *     You may use this code for any purpose, as long as the original
 *     copyright remains in the source code and all documentation
 ****************************************************************************/

/***********************************************************************
 *
 * xfwm focus-setting code
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


/********************************************************************
 *
 * Sets the input focus to the indicated window.
 *
 **********************************************************************/

void
SetFocus (Window w, XfwmWindow * Fw, Bool FocusByMouse)
{
  int i;
  extern Time lastTimestamp;

  if ((FocusByMouse) && (Fw && Fw != Scr.Focus && Fw != &Scr.XfwmRoot))
    {
      XfwmWindow *tmp_win1, *tmp_win2;

      tmp_win1 = Fw->prev;
      tmp_win2 = Fw->next;

      if (tmp_win1)
	tmp_win1->next = tmp_win2;
      if (tmp_win2)
	tmp_win2->prev = tmp_win1;

      Fw->next = Scr.XfwmRoot.next;
      if (Scr.XfwmRoot.next)
	Scr.XfwmRoot.next->prev = Fw;
      Scr.XfwmRoot.next = Fw;
      Fw->prev = &Scr.XfwmRoot;
    }

  if (Scr.NumberOfScreens > 1)
    {
      XQueryPointer (dpy, Scr.Root, &JunkRoot, &JunkChild,
		     &JunkX, &JunkY, &JunkX, &JunkY, &JunkMask);
      if (JunkRoot != Scr.Root)
	{
	  if ((Scr.Ungrabbed != NULL) && (Scr.Options & ClickToFocus))
	    {
	      /* Need to grab buttons for focus window */
	      XSync (dpy, 0);
	      for (i = 0; i < 3; i++)
		if (Scr.buttons2grab & (1 << i))
		  {
		    XGrabButton (dpy, (i + 1), 0, Scr.Ungrabbed->frame, True,
			       ButtonPressMask, GrabModeSync, GrabModeAsync,
				 None, Scr.XfwmCursors[SYS]);
		    XGrabButton (dpy, (i + 1), AnyModifier, Scr.Ungrabbed->frame, True,
			       ButtonPressMask, GrabModeSync, GrabModeAsync,
				 None, Scr.XfwmCursors[SYS]);
		  }
	      Scr.Focus = NULL;
	      Scr.Ungrabbed = NULL;
	      XSetInputFocus (dpy, Scr.NoFocusWin, RevertToParent, lastTimestamp);
	    }
	  return;
	}
    }

  if ((Fw != NULL) && (Fw->Desk != Scr.CurrentDesk))
    {
      Fw = NULL;
      w = Scr.NoFocusWin;
    }

  if ((Scr.Ungrabbed != NULL) && (Scr.Ungrabbed != Fw) 
                              && (Scr.Options & (ClickToFocus | AutoRaiseWin)))
    {
      /* need to grab all buttons for window that we are about to
       * unfocus */
      XSync (dpy, 0);
      for (i = 0; i < 3; i++)
	if (Scr.buttons2grab & (1 << i)) {
	  XGrabButton (dpy, (i + 1), 0, Scr.Ungrabbed->frame, True,
		       ButtonPressMask, GrabModeSync, GrabModeAsync, None,
		       Scr.XfwmCursors[SYS]);
	  XGrabButton (dpy, (i + 1), AnyModifier, Scr.Ungrabbed->frame, True,
		       ButtonPressMask, GrabModeSync, GrabModeAsync, None,
		       Scr.XfwmCursors[SYS]);
        }
      Scr.Ungrabbed = NULL;
    }
  /* if we do click to focus, remove the grab on mouse events that
   * was made to detect the focus change */
  if ((Fw != NULL) && (Scr.Options & ClickToFocus))
    {
      for (i = 0; i < 3; i++)
	if (Scr.buttons2grab & (1 << i))
	  {
	    XUngrabButton (dpy, (i + 1), 0, Fw->frame);
            XUngrabButton (dpy, (i + 1), AnyModifier, Fw->frame);
	  }
      Scr.Ungrabbed = Fw;
    }

  if ((Fw) && (Fw->flags & ICONIFIED) && (Fw->icon_w))
    w = Fw->icon_w;

  if ((Fw) && (Fw->flags & Lenience))
    {
      XSetInputFocus (dpy, w, RevertToParent, lastTimestamp);
      Scr.Focus = Fw;
      Scr.UnknownWinFocused = None;
    }
  else
    {
      /* Window will accept input focus */
      XSetInputFocus (dpy, w, RevertToParent, lastTimestamp);
      Scr.Focus = Fw;
      Scr.UnknownWinFocused = None;
    }

  if ((Fw) && (Fw->flags & DoesWmTakeFocus))
    send_clientmessage (dpy, w, _XA_WM_TAKE_FOCUS, lastTimestamp);

  XSync (dpy, 0);

}
