
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

#ifdef DMALLOC
#  include "dmalloc.h"
#endif


/********************************************************************
 *
 * Sets the input focus to the indicated window.
 *
 **********************************************************************/

void
SetFocus (Window w, XfwmWindow * Fw, Bool FocusByMouse)
{
  extern Time lastTimestamp;

  XSync (dpy, 0);
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
	  if (Scr.Focus != NULL)
	    {
	      Scr.Focus = NULL;
	      XSetInputFocus (dpy, Scr.NoFocusWin, RevertToParent,
			      lastTimestamp);
	    }
	  XSync (dpy, 0);
	  return;
	}
    }

  if ((Fw != NULL) && (Fw->Desk != Scr.CurrentDesk))
    {
      Fw = NULL;
      w = Scr.NoFocusWin;
    }

  if ((Fw) && (Fw->flags & ICONIFIED) && (Fw->icon_w))
    w = Fw->icon_w;

  XSetInputFocus (dpy, w, RevertToParent, lastTimestamp);
  Scr.Focus = Fw;
  Scr.UnknownWinFocused = None;

  if ((Fw) && (Fw->flags & DoesWmTakeFocus))
    send_clientmessage (dpy, w, _XA_WM_TAKE_FOCUS, lastTimestamp);

  XSync (dpy, 0);
}
