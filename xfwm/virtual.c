
#include "configure.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "xfwm.h"
#include "menus.h"
#include "misc.h"
#include "parse.h"
#include "screen.h"
#include "module.h"

#ifdef DMALLOC
#  include "dmalloc.h"
#endif

/**************************************************************************
 * 
 * Move to a new desktop
 *
 *************************************************************************/
void
changeDesks_func (XEvent * eventp, Window w, XfwmWindow * tmp_win,
		  unsigned long context, char *action, int *Module)
{
  int n, val1, val1_unit, val2, val2_unit;

  n = GetTwoArguments (action, &val1, &val2, &val1_unit, &val2_unit, 0, 0);
  changeDesks (val1, val2, 1, 1, 1);
}

void
changeDesks (int val1, int val2, Bool handle_focus, Bool broadcast, Bool grab)
{

  int oldDesk;
  static XfwmWindow *StickyWin = 0;
  XfwmWindow *FocusWin = 0, *MouseWin, *t;
  Window w;

  oldDesk = Scr.CurrentDesk;


  if (val1 != 0)
    {
      Scr.CurrentDesk = Scr.CurrentDesk + val1;
      if (Scr.CurrentDesk < 0)
	Scr.CurrentDesk = Scr.ndesks - 1;
      else if (Scr.CurrentDesk > Scr.ndesks - 1)
	Scr.CurrentDesk = 0;
    }
  else
    {
      Scr.CurrentDesk = val2;
    }
    
  if (Scr.CurrentDesk > Scr.ndesks - 1)
    Scr.CurrentDesk = oldDesk;
    
  if (Scr.CurrentDesk == oldDesk)
    return;

  if (grab)
    MyXGrabServer (dpy);

  if (broadcast)
    Broadcast (XFCE_M_NEW_DESK, 1, Scr.CurrentDesk, 0, 0, 0, 0, 0, 0);
  /* Scan the window list, mapping windows on the new Desk,
   * unmapping windows on the old Desk */
  for (t = Scr.XfwmRoot.next; t != NULL; t = t->next)
    {
      /* Only change mapping for non-sticky windows */
      if (!((t->flags & ICONIFIED) && (t->flags & StickyIcon)) &&
	  (!(t->flags & STICKY)) && (!(t->flags & ICON_UNMAPPED)))
	{
	  if (t->Desk == oldDesk)
	    {
	      if (Scr.Focus == t)
		t->FocusDesk = oldDesk;
	      else
		t->FocusDesk = -1;
	      UnmapIt (t);
	    }
	  else if (t->Desk == Scr.CurrentDesk)
	    {
	      MapIt (t);
	      if (t->FocusDesk == Scr.CurrentDesk)
		{
		  FocusWin = t;
		}
	    }
	}
      else
	{
	  /* Window is sticky */
	  t->Desk = Scr.CurrentDesk;
	  if (Scr.Focus == t)
	    {
	      t->FocusDesk = oldDesk;
	      StickyWin = t;
	    }
	}
      /* If its an icon, and its sticking, autoplace it so
       * that it doesn't wind up on top a a stationary
       * icon */

      if (((t->flags & STICKY) || (t->flags & StickyIcon)) &&
	  (t->flags & ICONIFIED) && (!(t->flags & ICON_MOVED)) &&
	  (!(t->flags & ICON_UNMAPPED)) && (!(t->flags & STARTICONIC)))
	AutoPlace (t, False);
    }
  
  if (handle_focus)
    {
      MouseWin = NULL;
      if (!(Scr.Options & ClickToFocus))
        {
          XQueryPointer (dpy, Scr.Root, &JunkRoot, &w, &JunkX, &JunkY, 
                         &JunkX, &JunkY, &JunkMask);
          if (XFindContext (dpy, w, XfwmContext, (caddr_t *) &MouseWin) == XCNOENT)
            {
              MouseWin = NULL;
            }
        }

      if ((MouseWin) && !(Scr.Options & ClickToFocus))
        SetFocus (MouseWin->w, MouseWin, 0);
      else if (FocusWin)
	SetFocus (FocusWin->w, FocusWin, 0);
      else if (StickyWin && (StickyWin->flags && STICKY))
	SetFocus (StickyWin->w, StickyWin, 1);
      else
	SetFocus (Scr.NoFocusWin, NULL, 1);
    }
  if (grab)
    MyXUngrabServer (dpy);
  XSync (dpy, 0);
}



/**************************************************************************
 * 
 * Move to a new desktop
 *
 *************************************************************************/
void
changeWindowsDesk (XEvent * eventp, Window w, XfwmWindow * t,
		   unsigned long context, char *action, int *Module)
{
  int val1, val2;
  int val1_unit, val2_unit, n;

  if (DeferExecution (eventp, &w, &t, &context, SELECT, ButtonRelease))
    return;

  if (t == NULL)
    return;

  n = GetTwoArguments (action, &val1, &val2, &val1_unit, &val2_unit, 0, 0);

  if (n != 2)
    {
      n = GetOneArgument (action, (long *) &val2, &val2_unit, 0, 0);
      val1 = 0;
    }

  if (val1 != 0)
    val1 += t->Desk;
  else
    val1 = val2;

  if (val1 == t->Desk)
    return;

  sendWindowsDesk (val1, t);
}

void sendWindowsDesk (int desk, XfwmWindow *t)
{
  /* Only change mapping for non-sticky windows */
  if (!((t->flags & ICONIFIED) && (t->flags & StickyIcon)) &&
      (!(t->flags & STICKY)) && (!(t->flags & ICON_UNMAPPED)))
    {
      if (t->Desk == Scr.CurrentDesk)
	{
	  t->Desk = desk;
	  UnmapIt (t);
	  /* If its an icon, auto-place it */
	  if (t->flags & ICONIFIED)
	    {
	      if ((!(t->flags & STARTICONIC)) && (!(t->flags & ICON_MOVED)))
		AutoPlace (t, False);
	      LowerWindow (t);
	    }
	}
      else if (desk == Scr.CurrentDesk)
	{
	  t->Desk = desk;
	  /* If its an icon, auto-place it */
	  if (t->flags & ICONIFIED)
	    {
	      if ((!(t->flags & STARTICONIC)) && (!(t->flags & ICON_MOVED)))
		AutoPlace (t, False);
	      LowerWindow (t);
	    }
	  MapIt (t);
	}
      else
	t->Desk = desk;
    }
  BroadcastConfig (XFCE_M_CONFIGURE_WINDOW, t);
}
