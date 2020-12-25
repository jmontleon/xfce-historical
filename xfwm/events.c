/****************************************************************************
 * This module is based on Twm, but has been siginificantly modified 
 * by Rob Nation 
 ****************************************************************************/
/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/


/***********************************************************************
 *
 * xfwm event handling
 *
 ***********************************************************************/

#include "configure.h"

#ifdef ISC
#include <sys/bsdtypes.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
/* Some people say that AIX and AIXV3 need 3 preceding underscores, other say
 * no. I'll do both */
#if defined ___AIX || defined _AIX || defined __QNX__ || defined ___AIXV3 || defined AIXV3 || defined _SEQUENT_
#include <sys/select.h>
#endif

#include "xfwm.h"
#include <X11/Xatom.h>
#include "menus.h"
#include "misc.h"
#include "parse.h"
#include "screen.h"
#include "xfwm.h"
#include <X11/extensions/shape.h>
#include "module.h"
#include "constant.h"

unsigned int mods_used = (ShiftMask | ControlMask | Mod1Mask |
			  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
extern int menuFromFrameOrWindowOrTitlebar;

int Context = C_NO_CONTEXT;	/* current button press context */
int Button = 0;
XfwmWindow *ButtonWindow;	/* button press window structure */
XEvent Event;			/* the current event */
XfwmWindow *Tmp_win;		/* the current xfwm window */

int last_event_type = 0;
Window last_event_window = 0;
volatile int alarmed;

extern int ShapeEventBase;
void HandleShapeNotify (void);

Window PressedW;

/*
 * ** LASTEvent is the number of X events defined - it should be defined
 * ** in X.h (to be like 35), but since extension (eg SHAPE) events are
 * ** numbered beyond LASTEvent, we need to use a bigger number than the
 * ** default, so let's undefine the default and use 256 instead.
 */
#undef LASTEvent
#ifndef LASTEvent
#define LASTEvent 256
#endif /* !LASTEvent */
typedef void (*PFEH) ();
PFEH EventHandlerJumpTable[LASTEvent];

void enterAlarm(int nonsense)
{
  alarmed = True;
  signal(SIGALRM, enterAlarm);
}

/*
 * ** Procedure:
 * **   InitEventHandlerJumpTable
 */
void
InitEventHandlerJumpTable (void)
{
  int i;

  for (i = 0; i < LASTEvent; i++)
    {
      EventHandlerJumpTable[i] = NULL;
    }
  EventHandlerJumpTable[Expose] = HandleExpose;
  EventHandlerJumpTable[DestroyNotify] = HandleDestroyNotify;
  EventHandlerJumpTable[MapRequest] = HandleMapRequest;
  EventHandlerJumpTable[MapNotify] = HandleMapNotify;
  EventHandlerJumpTable[UnmapNotify] = HandleUnmapNotify;
  EventHandlerJumpTable[ButtonPress] = HandleButtonPress;
  EventHandlerJumpTable[EnterNotify] = HandleEnterNotify;
  EventHandlerJumpTable[LeaveNotify] = HandleLeaveNotify;
  EventHandlerJumpTable[FocusIn] = HandleFocusIn;
  EventHandlerJumpTable[FocusOut] = HandleFocusOut;
  EventHandlerJumpTable[ConfigureRequest] = HandleConfigureRequest;
  EventHandlerJumpTable[ClientMessage] = HandleClientMessage;
  EventHandlerJumpTable[PropertyNotify] = HandlePropertyNotify;
  EventHandlerJumpTable[KeyPress] = HandleKeyPress;
  EventHandlerJumpTable[VisibilityNotify] = HandleVisibilityNotify;
  EventHandlerJumpTable[ColormapNotify] = HandleColormapNotify;
  EventHandlerJumpTable[MotionNotify] = HandleMotionNotify;
  if (ShapesSupported)
    EventHandlerJumpTable[ShapeEventBase + ShapeNotify] = HandleShapeNotify;
}

/***********************************************************************
 *
 *  Procedure:
 *	DispatchEvent - handle a single X event stored in global var Event
 *
 ************************************************************************/
void
DispatchEvent ()
{
  Window w = Event.xany.window;

  StashEventTime (&Event);

  if (XFindContext (dpy, w, XfwmContext, (caddr_t *) & Tmp_win) == XCNOENT)
    Tmp_win = NULL;
  last_event_type = Event.type;
  last_event_window = w;

  if (EventHandlerJumpTable[Event.type])
    (*EventHandlerJumpTable[Event.type]) ();

  return;
}


/***********************************************************************
 *
 *  Procedure:
 *	HandleEvents - handle X events
 *
 ************************************************************************/
void
HandleEvents ()
{
  while (TRUE)
    {
      last_event_type = 0;
      if (My_XNextEvent (dpy, &Event))
	{
	  DispatchEvent ();
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	Find the Xfwm context for the Event.
 *
 ************************************************************************/
int
GetContext (XfwmWindow * t, XEvent * e, Window * w)
{
  int Context, i;

  if (!t)
    return C_ROOT;

  Context = C_NO_CONTEXT;
  *w = e->xany.window;

  if (*w == Scr.NoFocusWin)
    return C_ROOT;

  /* Since key presses and button presses are grabbed in the frame
   * when we have re-parented windows, we need to find out the real
   * window where the event occured */
  if ((e->type == KeyPress) && (e->xkey.subwindow != None))
    *w = e->xkey.subwindow;

  if ((e->type == ButtonPress) && (e->xbutton.subwindow != None) &&
    ((e->xbutton.subwindow == t->w) || (e->xbutton.subwindow == t->Parent)))
    *w = e->xbutton.subwindow;

  if (*w == Scr.Root)
    Context = C_ROOT;
  if (t)
    {
      if (*w == t->title_w)
	Context = C_TITLE;
      if ((*w == t->w) || (*w == t->Parent))
	Context = C_WINDOW;
      if (*w == t->icon_w)
	Context = C_ICON;
      if (*w == t->icon_pixmap_w)
	Context = C_ICON;
      if (*w == t->frame)
	Context = C_SIDEBAR;
      for (i = 0; i < 4; i++)
	if (*w == t->corners[i])
	  {
	    Context = C_FRAME;
	    Button = i;
	  }
      for (i = 0; i < 4; i++)
	if (*w == t->sides[i])
	  {
	    Context = C_SIDEBAR;
	    Button = i;
	  }
      for (i = 0; i < Scr.nr_left_buttons; i++)
	{
	  if (*w == t->left_w[i])
	    {
	      Context = (1 << i) * C_L1;
	      Button = i;
	    }
	}
      for (i = 0; i < Scr.nr_right_buttons; i++)
	{
	  if (*w == t->right_w[i])
	    {
	      Context = (1 << i) * C_R1;
	      Button = i;
	    }
	}
    }
  return Context;
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleFocusIn - handles focus in events
 *
 ************************************************************************/
void
HandleFocusIn ()
{
  XEvent d;
  Window w;

  w = Event.xany.window;
  while (XCheckTypedEvent (dpy, FocusIn, &d))
    {
      w = d.xany.window;
    }
  if (XFindContext (dpy, w, XfwmContext, (caddr_t *) & Tmp_win) == XCNOENT)
    {
      Tmp_win = NULL;
    }

  if (!Tmp_win)
    {
      if (w != Scr.NoFocusWin)
	{
	  Scr.UnknownWinFocused = w;
	}
      else
	{
	  SetBorder (Scr.Hilite, False, True, True, None);
	  Broadcast (M_FOCUS_CHANGE, 5, 0, 0, 0,
		     Scr.DefaultDecor.HiColors.fore,
		     Scr.DefaultDecor.HiColors.back,
		     0, 0);
	  if (Scr.ColormapFocus == COLORMAP_FOLLOWS_FOCUS)
	    {
	      if ((Scr.Hilite) && (!(Scr.Hilite->flags & ICONIFIED)))
		{
		  InstallWindowColormaps (Scr.Hilite);
		}
	      else
		{
		  InstallWindowColormaps (NULL);
		}
	    }

	}
    }
  else if (Tmp_win != Scr.Hilite)
    {
      SetBorder (Tmp_win, True, True, True, None);
      Broadcast (M_FOCUS_CHANGE, 5, Tmp_win->w,
		 Tmp_win->frame, (unsigned long) Tmp_win,
		 GetDecor (Tmp_win, HiColors.fore),
		 GetDecor (Tmp_win, HiColors.back),
		 0, 0);
      if (Scr.ColormapFocus == COLORMAP_FOLLOWS_FOCUS)
	{
	  if ((Scr.Hilite) && (!(Scr.Hilite->flags & ICONIFIED)))
	    {
	      InstallWindowColormaps (Scr.Hilite);
	    }
	  else
	    {
	      InstallWindowColormaps (NULL);
	    }
	}
    }
}

void
HandleFocusOut()
{
    Window focusBug;
    int rt;

    XGetInputFocus(dpy, &focusBug, &rt);
    if ((Tmp_win == NULL) && (Scr.Focus != NULL) && (focusBug == None))
    {
        XSetInputFocus(dpy, Scr.Focus->w, RevertToParent, CurrentTime);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleKeyPress - key press event handler
 *
 ************************************************************************/
void
HandleKeyPress ()
{
  Binding *key;
  unsigned int modifier;
  modifier = vmod((Event.xkey.state) & KeyMask);
  ButtonWindow = Tmp_win;

  Context = GetContext (Tmp_win, &Event, &PressedW);
  PressedW = None;

  /* Here's a real hack - some systems have two keys with the
   * same keysym and different keycodes. This converts all
   * the cases to one keycode. */
  Event.xkey.keycode =
    XKeysymToKeycode (dpy, XKeycodeToKeysym (dpy, Event.xkey.keycode, 0));

  for (key = Scr.AllBindings; key != NULL; key = key->NextBinding)
    {
      if ((key->Button_Key == Event.xkey.keycode) &&
	  ((vmod(key->Modifier) == modifier) ||
	   (key->Modifier == AnyModifier)) &&
	  (key->Context & Context) &&
	  (key->IsMouse == 0))
	{
	  ExecuteFunction (key->Action, Tmp_win, &Event, Context, -1);
	  return;
	}
    }

  /* if we get here, no function key was bound to the key.  Send it
   * to the client if it was in a window we know about.
   */
  if (Tmp_win)
    {
      if (Event.xkey.window != Tmp_win->w)
	{
	  Event.xkey.window = Tmp_win->w;
	  XSendEvent (dpy, Tmp_win->w, False, KeyPressMask, &Event);
	}
    }

  ButtonWindow = NULL;
}


/***********************************************************************
 *
 *  Procedure:
 *	HandlePropertyNotify - property notify event handler
 *
 ***********************************************************************/
#define MAX_NAME_LEN 200L	/* truncate to this many */
#define MAX_ICON_NAME_LEN 200L	/* ditto */

void
HandlePropertyNotify ()
{
  XTextProperty text_prop;

  if (Event.xproperty.atom == _XA_WIN_WORKSPACE_COUNT)
        {
          Atom atype;
          int aformat;
          unsigned long nitems, bytes_remain;
          unsigned char* prop;

          if ((XGetWindowProperty (dpy, Scr.Root, _XA_WIN_WORKSPACE_COUNT, 0L, 1L, False,
			         XA_CARDINAL, &atype, &aformat, &nitems,
			         &bytes_remain, &prop)) == Success)
            {
	      if (prop != NULL)
	        {
	          Scr.ndesks = *(unsigned long *) prop;
	        }
            }
            
          return;
        }

  if ((!Tmp_win) || (XGetGeometry (dpy, Tmp_win->w, &JunkRoot, &JunkX, &JunkY,
			&JunkWidth, &JunkHeight, &JunkBW, &JunkDepth) == 0))
    return;

  switch (Event.xproperty.atom)
    {
    case XA_WM_NAME:
      if (!XGetWMName (dpy, Tmp_win->w, &text_prop))
	return;

      free_window_names (Tmp_win, True, False);
      Tmp_win->name = NULL;
      if (text_prop.format == 8)
        { /* Multi-byte string */
          char **text_list = NULL;
          int text_list_num;
          if (XmbTextPropertyToTextList(dpy, &text_prop,
                                &text_list, &text_list_num) == Success)
            {
	      if (text_list)
	        {
                  Tmp_win->name = (char *) malloc(strlen (text_list[0]) + 1);
                  strcpy (Tmp_win->name, text_list[0]);
                  XFreeStringList(text_list);
	        }
            }
          else
            Tmp_win->name = (char *) text_prop.value;
        }
      else
        Tmp_win->name = (char *) text_prop.value;

      if (Tmp_win->name == NULL)
	Tmp_win->name = NoName;
      BroadcastName (M_WINDOW_NAME, Tmp_win->w, Tmp_win->frame,
		     (unsigned long) Tmp_win, Tmp_win->name);

      /* fix the name in the title bar */
      if (!(Tmp_win->flags & ICONIFIED))
	SetTitleBar (Tmp_win, (Scr.Hilite == Tmp_win), True);

      /*
       * if the icon name is NoName, set the name of the icon to be
       * the same as the window 
       */
      if (Tmp_win->icon_name == NoName)
	{
	  Tmp_win->icon_name = Tmp_win->name;
	  BroadcastName (M_ICON_NAME, Tmp_win->w, Tmp_win->frame,
			 (unsigned long) Tmp_win, Tmp_win->icon_name);
	  RedoIconName (Tmp_win);
	}
      break;

    case XA_WM_ICON_NAME:
      if (!XGetWMIconName (dpy, Tmp_win->w, &text_prop))
	return;
      free_window_names (Tmp_win, False, True);
      Tmp_win->icon_name = Tmp_win->icon_name;
      if (text_prop.format == 8)
        { /* Multi-byte string */
          char **text_list = NULL;
          int text_list_num;
          if (XmbTextPropertyToTextList(dpy, &text_prop,
                                &text_list, &text_list_num) == Success)
            {
	      if (text_list)
	        {
                  Tmp_win->icon_name = (char *) malloc(strlen (text_list[0]) + 1);
                  strcpy (Tmp_win->icon_name, text_list[0]);
                  XFreeStringList(text_list);
	        }
            }
          else
            Tmp_win->icon_name = (char *) text_prop.value;
        }
      else
        Tmp_win->icon_name = (char *) text_prop.value;
      if (Tmp_win->icon_name == NULL)
	Tmp_win->icon_name = Tmp_win->name;
      BroadcastName (M_ICON_NAME, Tmp_win->w, Tmp_win->frame,
		     (unsigned long) Tmp_win, Tmp_win->icon_name);
      RedoIconName (Tmp_win);
      break;

    case XA_WM_HINTS:
      if (Tmp_win->wmhints)
	XFree ((char *) Tmp_win->wmhints);
      Tmp_win->wmhints = XGetWMHints (dpy, Event.xany.window);

      if (Tmp_win->wmhints == NULL)
	return;

      if ((Tmp_win->wmhints->flags & IconPixmapHint) ||
	  (Tmp_win->wmhints->flags & IconWindowHint))
	if (Tmp_win->icon_bitmap_file == Scr.DefaultIcon)
	  Tmp_win->icon_bitmap_file = (char *) 0;

      if ((Tmp_win->wmhints->flags & IconPixmapHint) ||
	  (Tmp_win->wmhints->flags & IconWindowHint))
	{
	  if (!(Tmp_win->flags & SUPPRESSICON))
	    {
	      if (Tmp_win->icon_w)
		XDestroyWindow (dpy, Tmp_win->icon_w);
	      XDeleteContext (dpy, Tmp_win->icon_w, XfwmContext);
	      if (Tmp_win->flags & ICON_OURS)
		{
		  if (Tmp_win->icon_pixmap_w != None)
		    {
		      XDestroyWindow (dpy, Tmp_win->icon_pixmap_w);
		      XDeleteContext (dpy, Tmp_win->icon_pixmap_w, XfwmContext);
		    }
		}
	      else
		XUnmapWindow (dpy, Tmp_win->icon_pixmap_w);
	    }
	  Tmp_win->icon_w = None;
	  Tmp_win->icon_pixmap_w = None;
	  Tmp_win->iconPixmap = (Window) NULL;
	  if (Tmp_win->flags & ICONIFIED)
	    {
	      Tmp_win->flags &= ~ICONIFIED;
	      Tmp_win->flags &= ~ICON_UNMAPPED;
	      CreateIconWindow (Tmp_win, Tmp_win->icon_x_loc, Tmp_win->icon_y_loc);
	      Broadcast (M_ICONIFY, 7, Tmp_win->w, Tmp_win->frame,
			 (unsigned long) Tmp_win,
			 Tmp_win->icon_x_loc,
			 Tmp_win->icon_y_loc,
			 Tmp_win->icon_w_width,
			 Tmp_win->icon_w_height);
	      BroadcastConfig (M_CONFIGURE_WINDOW, Tmp_win);

	      if (!(Tmp_win->flags & SUPPRESSICON))
		{
		  LowerWindow (Tmp_win);
		  AutoPlace (Tmp_win, False);
		  if (Tmp_win->Desk == Scr.CurrentDesk)
		    {
		      if (Tmp_win->icon_w)
			XMapWindow (dpy, Tmp_win->icon_w);
		      if (Tmp_win->icon_pixmap_w != None)
			XMapWindow (dpy, Tmp_win->icon_pixmap_w);
		    }
		}
	      Tmp_win->flags |= ICONIFIED;
	      DrawIconWindow (Tmp_win);
	    }
	}
      break;

    case XA_WM_NORMAL_HINTS:
      {
	GetWindowSizeHints (Tmp_win);
	BroadcastConfig (M_CONFIGURE_WINDOW, Tmp_win);
      }
      break;
    default:
      if (Event.xproperty.atom == _XA_WM_PROTOCOLS)
	FetchWmProtocols (Tmp_win);
      else if (Event.xproperty.atom == _XA_WM_COLORMAP_WINDOWS)
	{
	  FetchWmColormapWindows (Tmp_win);	/* frees old data */
	  ReInstallActiveColormap ();
	}
      else if (Event.xproperty.atom == _XA_WM_STATE)
	{
	  if ((Tmp_win != NULL) && (Tmp_win == Scr.Focus) 
                                && (Scr.Options & (ClickToFocus | AutoRaiseWin)))
	    {
	      Scr.Focus = NULL;
              if (Scr.Options & ClickToFocus)
  	        SetFocus (Tmp_win->w, Tmp_win, 0);
              else
  	        SetFocus (Tmp_win->w, Tmp_win, 1);
	    }
	}
      break;
    }
}


/***********************************************************************
 *
 *  Procedure:
 *	HandleClientMessage - client message event handler
 *
 ************************************************************************/
void
HandleClientMessage ()
{
  XEvent button;

  if ((Event.xclient.message_type == _XA_WM_CHANGE_STATE) &&
      (Tmp_win) && (Event.xclient.data.l[0] == IconicState) &&
      !(Tmp_win->flags & ICONIFIED))
    {
      XQueryPointer (dpy, Scr.Root, &JunkRoot, &JunkChild,
		     &(button.xmotion.x_root),
		     &(button.xmotion.y_root),
		     &JunkX, &JunkY, &JunkMask);
      button.type = 0;
      ExecuteFunction ("Iconify", Tmp_win, &button, C_FRAME, -1);
      return;
    }

  /*
   * ** CKH - if we get here, it was an unknown client message, so send
   * ** it to the client if it was in a window we know about.  I'm not so
   * ** sure this should be done or not, since every other window manager
   * ** I've looked at doesn't.  But it might be handy for a free drag and
   * ** drop setup being developed for Linux.
   */
  if (Tmp_win)
    {
      if (Event.xclient.window != Tmp_win->w)
	{
	  Event.xclient.window = Tmp_win->w;
	  XSendEvent (dpy, Tmp_win->w, False, NoEventMask, &Event);
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleExpose - expose event handler
 *
 ***********************************************************************/
void
HandleExpose ()
{
  if (Event.xexpose.count != 0)
    return;

  if (Tmp_win)
    {
      if ((Event.xany.window == Tmp_win->title_w))
	{
	  SetTitleBar (Tmp_win, (Scr.Hilite == Tmp_win), False);
	}
      else
	{
	  SetBorder (Tmp_win, (Scr.Hilite == Tmp_win), True, True, Event.xany.window);
	}
    }
  return;
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleDestroyNotify - DestroyNotify event handler
 *
 ***********************************************************************/
void
HandleDestroyNotify ()
{
  if (Tmp_win)
    Destroy (Tmp_win);
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleMapRequest - MapRequest event handler
 *
 ************************************************************************/
void
HandleMapRequest ()
{
  HandleMapRequestKeepRaised (None);
}

void
HandleMapRequestKeepRaised (Window KeepRaised)
{
  extern long isIconicState;

  Event.xany.window = Event.xmaprequest.window;

  if (XFindContext (dpy, Event.xany.window, XfwmContext,
		    (caddr_t *) & Tmp_win) == XCNOENT)
    Tmp_win = NULL;

  XFlush (dpy);

  if (!Tmp_win)
    {
      Tmp_win = AddWindow (Event.xany.window);
      if (Tmp_win == NULL)
	return;
    }

  if (!(Tmp_win->flags & ICONIFIED))
    {
      int state;

      if (Tmp_win->wmhints && (Tmp_win->wmhints->flags & StateHint))
	state = Tmp_win->wmhints->initial_state;
      else
	state = NormalState;

      if (Tmp_win->flags & STARTICONIC)
	state = IconicState;

      if (isIconicState != DontCareState)
	state = isIconicState;

      MyXGrabServer (dpy);

      switch (state)
	{
	case DontCareState:
	case NormalState:
	case InactiveState:
	default:
	  if (Tmp_win->Desk == Scr.CurrentDesk)
	    {
	      XMapWindow (dpy, Tmp_win->w);
	      XMapWindow (dpy, Tmp_win->frame);
	      Tmp_win->flags |= MAP_PENDING;
	      SetMapStateProp (Tmp_win, NormalState);
	      if((!Scr.Focus) && (Scr.Options & ClickToFocus))
		  SetFocus(Tmp_win->w,Tmp_win,1);
	    }
	  else
	    {
	      XMapWindow (dpy, Tmp_win->w);
	      SetMapStateProp (Tmp_win, NormalState);
	    }
	  break;

	case IconicState:
	  if (Tmp_win->wmhints)
	      Iconify (Tmp_win, Tmp_win->wmhints->icon_x, Tmp_win->wmhints->icon_y);
	  else
	      Iconify (Tmp_win, 0, 0);
	  break;
	}

      XSync (dpy, 0);
      MyXUngrabServer (dpy);
    }
  else
    {
      DeIconify (Tmp_win);
      RaiseWindow(Tmp_win);
      if (Tmp_win->flags & SHADED)
        Unshade(Tmp_win);
      SetFocus(Tmp_win->w,Tmp_win,1);
    }
    KeepOnTop ();
}


/***********************************************************************
 *
 *  Procedure:
 *	HandleMapNotify - MapNotify event handler
 *
 ***********************************************************************/
void
HandleMapNotify ()
{
  if (!Tmp_win)
    {
      if ((Event.xmap.override_redirect == True) &&
	  (Event.xmap.window != Scr.NoFocusWin))
	{
	  XSelectInput (dpy, Event.xmap.window, FocusChangeMask);
	  Scr.UnknownWinFocused = Event.xmap.window;
	}
      return;
    }

  if (Event.xmap.event != Event.xmap.window)
    return;

  MyXGrabServer (dpy);
  if (Tmp_win->icon_w)
    XUnmapWindow (dpy, Tmp_win->icon_w);
  if (Tmp_win->icon_pixmap_w != None)
    XUnmapWindow (dpy, Tmp_win->icon_pixmap_w);
  XMapSubwindows (dpy, Tmp_win->frame);

  if (Tmp_win->Desk == Scr.CurrentDesk)
      XMapWindow (dpy, Tmp_win->frame);

  if (Tmp_win->flags & ICONIFIED)
      Broadcast (M_DEICONIFY, 3, Tmp_win->w, Tmp_win->frame,
	       (unsigned long) Tmp_win, 0, 0, 0, 0);
  else
      Broadcast (M_MAP, 3, Tmp_win->w, Tmp_win->frame,
		 (unsigned long) Tmp_win, 0, 0, 0, 0);
  if ((Scr.Options & ClickToFocus) && (Scr.Focus))
    SetFocus (Tmp_win->w, Tmp_win, 1);
  if ((!(Tmp_win->flags & (BORDER | TITLE))) && (Tmp_win->boundary_width < 2))
    SetBorder (Tmp_win, False, True, True, Tmp_win->frame);
  XSync (dpy, 0);
  MyXUngrabServer (dpy);
  XFlush (dpy);
  Tmp_win->flags |= MAPPED;
  Tmp_win->flags &= ~MAP_PENDING;
  Tmp_win->flags &= ~ICONIFIED;
  Tmp_win->flags &= ~ICON_UNMAPPED;
}


/***********************************************************************
 *
 *  Procedure:
 *	HandleUnmapNotify - UnmapNotify event handler
 *
 ************************************************************************/
void
HandleUnmapNotify ()
{
  int dstx, dsty;
  Window dumwin;
  XEvent dummy;
  extern XfwmWindow *colormap_win;
  int weMustUnmap;

  /*
   * Don't ignore events as described below.
   */
  if ((Event.xunmap.event != Event.xunmap.window) &&
      (Event.xunmap.event != Scr.Root || !Event.xunmap.send_event))
    {
      return;
    }

  weMustUnmap = 0;
  if (!Tmp_win)
    {
      Event.xany.window = Event.xunmap.window;
      weMustUnmap = 1;
      if (XFindContext (dpy, Event.xany.window,
			XfwmContext, (caddr_t *) & Tmp_win) == XCNOENT)
	Tmp_win = NULL;
    }

  if (!Tmp_win)
    return;

  if (weMustUnmap)
    XUnmapWindow (dpy, Event.xunmap.window);

  if (Tmp_win == Scr.Hilite)
    Scr.Hilite = NULL;

  if (Scr.PreviousFocus == Tmp_win)
    Scr.PreviousFocus = NULL;

  if ((Tmp_win == Scr.Focus) && (Scr.Options & ClickToFocus))
    {
      if (Tmp_win->next)
	SetFocus(Tmp_win->next->w,Tmp_win->next, 1);
      else
	SetFocus (Scr.NoFocusWin, NULL, 1);
    }

  if (Scr.Focus == Tmp_win)
    SetFocus (Scr.NoFocusWin, NULL, 1);

  if (Tmp_win == Scr.pushed_window)
    Scr.pushed_window = NULL;

  if (Tmp_win == colormap_win)
    colormap_win = NULL;

  if (!(Tmp_win->flags & MAPPED) && !(Tmp_win->flags & ICONIFIED) && !(Tmp_win->flags & SHADED))
      return;

  MyXGrabServer (dpy);

  if (XCheckTypedWindowEvent (dpy, Event.xunmap.window, DestroyNotify, &dummy))
    {
      Destroy (Tmp_win);
      MyXUngrabServer (dpy);
      return;
    }

  /*
   * The program may have unmapped the client window, from either
   * NormalState or IconicState.  Handle the transition to WithdrawnState.
   *
   * We need to reparent the window back to the root (so that xfwm exiting 
   * won't cause it to get mapped) and then throw away all state (pretend 
   * that we've received a DestroyNotify).
   */
  if (XTranslateCoordinates (dpy, Event.xunmap.window, Scr.Root,
			     0, 0, &dstx, &dsty, &dumwin))
    {
      XEvent ev;
      Bool reparented;
      reparented = XCheckTypedWindowEvent (dpy, Event.xunmap.window,
					   ReparentNotify, &ev);
      SetMapStateProp (Tmp_win, WithdrawnState);
      if (reparented)
	{
	  if (Tmp_win->old_bw)
	    XSetWindowBorderWidth (dpy, Event.xunmap.window, Tmp_win->old_bw);
	  if ((!(Tmp_win->flags & SUPPRESSICON)) &&
	   (Tmp_win->wmhints && (Tmp_win->wmhints->flags & IconWindowHint)))
	    XUnmapWindow (dpy, Tmp_win->wmhints->icon_window);
	}
      else
	RestoreWithdrawnLocation (Tmp_win, False);
      XRemoveFromSaveSet (dpy, Event.xunmap.window);
      XSelectInput (dpy, Event.xunmap.window, NoEventMask);
      Destroy (Tmp_win);	/* do not need to mash event before */
    }				/* else window no longer exists and we'll get a destroy notify */
  MyXUngrabServer (dpy);

  XFlush (dpy);
}


/***********************************************************************
 *
 *  Procedure:
 *	HandleButtonPress - ButtonPress event handler
 *
 ***********************************************************************/
void
HandleButtonPress ()
{
  unsigned int modifier;
  Binding *MouseEntry;
  Window x;
  int LocalContext;

  /* click to focus stuff goes here */
  if (Scr.Options & AutoRaiseWin)
    SetTimer(0);
  if ((Tmp_win) && (Tmp_win != Scr.Ungrabbed) && (Scr.Options & ClickToFocus))
    {
      SetFocus (Tmp_win->w, Tmp_win, 1);
      RaiseWindow (Tmp_win);
      if(!(Tmp_win->flags & ICONIFIED))
      {
         XSync(dpy,0);
         XAllowEvents(dpy,ReplayPointer,CurrentTime);
         XSync(dpy,0);
         return;
      }
    }
  else if ((Tmp_win) && !(Scr.Options & ClickToFocus) &&
           (Event.xbutton.window == Tmp_win->frame))
  {
    if ((Tmp_win != Scr.LastWindowRaised) &&
        (GetContext(Tmp_win,&Event, &PressedW) == C_WINDOW))
      RaiseWindow (Tmp_win);
    XSync(dpy,0);
    XAllowEvents(dpy,ReplayPointer,CurrentTime);
    XSync(dpy,0);
    return;
  }

  XSync (dpy, 0);
  XAllowEvents (dpy, ReplayPointer, CurrentTime);
  XSync (dpy, 0);

  Context = GetContext (Tmp_win, &Event, &PressedW);
  LocalContext = Context;
  x = PressedW;
  if (Context == C_TITLE)
    SetTitleBar (Tmp_win, (Scr.Hilite == Tmp_win), False);
  else
    SetBorder (Tmp_win, (Scr.Hilite == Tmp_win), True, True, PressedW);

  ButtonWindow = Tmp_win;

  /* we have to execute a function or pop up a menu
   */

  modifier = vmod((Event.xkey.state) & KeyMask);
  /* need to search for an appropriate mouse binding */
  for (MouseEntry = Scr.AllBindings; MouseEntry != NULL;
       MouseEntry = MouseEntry->NextBinding)
    {
      if (((MouseEntry->Button_Key == Event.xbutton.button) ||
	   (MouseEntry->Button_Key == 0)) &&
	  (MouseEntry->Context & Context) &&
	  ((vmod(MouseEntry->Modifier) == modifier) ||
	   (MouseEntry->Modifier == AnyModifier)) &&
	  (MouseEntry->IsMouse == 1))
	{
	  /* got a match, now process it */
	  ExecuteFunction (MouseEntry->Action, Tmp_win, &Event, Context, -1);
	  break;
	}
    }

  PressedW = None;
  if (LocalContext != C_TITLE)
    SetBorder (ButtonWindow, (Scr.Hilite == ButtonWindow), True, True, x);
  else
    SetTitleBar (ButtonWindow, (Scr.Hilite == ButtonWindow), False);
  ButtonWindow = NULL;
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleEnterNotify - EnterNotify event handler
 *
 ************************************************************************/
void
HandleEnterNotify ()
{
  XEnterWindowEvent *ewp = &Event.xcrossing;
  XEvent d;

  /* look for a matching leaveNotify which would nullify this enterNotify */
  if (XCheckTypedWindowEvent (dpy, ewp->window, LeaveNotify, &d))
    {
      StashEventTime (&d);
      if ((d.xcrossing.mode == NotifyNormal) &&
	  (d.xcrossing.detail != NotifyInferior))
	return;
    }

  /* multi screen? */
  if (Event.xany.window == Scr.Root)
    {
      if ((!Scr.Focus) || (!Scr.Options & ClickToFocus))
	SetFocus (Scr.NoFocusWin, NULL, 1);
      if (Scr.ColormapFocus == COLORMAP_FOLLOWS_MOUSE)
	InstallWindowColormaps (NULL);
      return;
    }

  /* make sure its for one of our windows */
  if (!Tmp_win)
    return;

  if(!(Scr.Options & ClickToFocus))
    {
      SetFocus(Tmp_win->w,Tmp_win,1);
      if (((Scr.Options & AutoRaiseWin) && (Scr.AutoRaiseDelay > 0)) 
         && (!(Tmp_win->flags & ICONIFIED) && !(Tmp_win->flags & RAISEDWIN)))
	 SetTimer(Scr.AutoRaiseDelay);
    }
  if (Scr.ColormapFocus == COLORMAP_FOLLOWS_MOUSE)
    {
      if ((!(Tmp_win->flags & ICONIFIED)) && (Event.xany.window == Tmp_win->w))
	InstallWindowColormaps (Tmp_win);
      else
	InstallWindowColormaps (NULL);
    }
  return;
}


/***********************************************************************
 *
 *  Procedure:
 *	HandleLeaveNotify - LeaveNotify event handler
 *
 ************************************************************************/
void
HandleLeaveNotify ()
{
  /* If we leave the root window, then we're really moving
   * another screen on a multiple screen display, and we
   * need to de-focus and unhighlight to make sure that we
   * don't end up with more than one highlighted window at a time */
  if (Event.xcrossing.window == Scr.Root)
    {
      if (Scr.Options & AutoRaiseWin)
        SetTimer(0);
      if (Event.xcrossing.mode == NotifyNormal)
	{
	  if (Event.xcrossing.detail != NotifyInferior)
	    {
	      if (Scr.Focus != NULL)
		SetFocus (Scr.NoFocusWin, NULL, 1);
	      if (Scr.Hilite != NULL)
		SetBorder (Scr.Hilite, False, True, True, None);
	    }
	}
    }
}


/***********************************************************************
 *
 *  Procedure:
 *	HandleConfigureRequest - ConfigureRequest event handler
 *
 ************************************************************************/
void
HandleConfigureRequest ()
{
  XWindowChanges xwc;
  unsigned long xwcm;
  int x, y, width, height;
  XConfigureRequestEvent *cre = &Event.xconfigurerequest;
  /*
   * Event.xany.window is Event.xconfigurerequest.parent, so Tmp_win will
   * be wrong
   */
  Event.xany.window = cre->window;	/* mash parent field */
  if (XFindContext (dpy, cre->window, XfwmContext, (caddr_t *) & Tmp_win) == XCNOENT)
    Tmp_win = NULL;

  if (!Tmp_win || (Tmp_win->icon_w == cre->window))
    {
      xwcm = cre->value_mask &
	(CWX | CWY | CWWidth | CWHeight | CWBorderWidth);
      xwc.x = cre->x;
      xwc.y = cre->y;
      if ((Tmp_win) && ((Tmp_win->icon_w == cre->window)))
	{
	  Tmp_win->icon_xl_loc = cre->x;
	  Tmp_win->icon_x_loc = cre->x +
	    (Tmp_win->icon_w_width - Tmp_win->icon_p_width) / 2;
	  Tmp_win->icon_y_loc = cre->y - Tmp_win->icon_p_height;
	  if (!(Tmp_win->flags & ICON_UNMAPPED))
	    Broadcast (M_ICON_LOCATION, 7, Tmp_win->w, Tmp_win->frame,
		       (unsigned long) Tmp_win,
		       Tmp_win->icon_x_loc, Tmp_win->icon_y_loc,
		       Tmp_win->icon_w_width,
		       Tmp_win->icon_w_height + Tmp_win->icon_p_height);
	}
      xwc.width = cre->width;
      xwc.height = cre->height;
      xwc.border_width = cre->border_width;
      XConfigureWindow (dpy, Event.xany.window, xwcm, &xwc);

      if (Tmp_win)
	{
	  xwc.x = Tmp_win->icon_x_loc;
	  xwc.y = Tmp_win->icon_y_loc - Tmp_win->icon_p_height;
	  xwcm = cre->value_mask & (CWX | CWY);
	  if (Tmp_win->icon_pixmap_w != None)
	    XConfigureWindow (dpy, Tmp_win->icon_pixmap_w, xwcm, &xwc);
	  xwc.x = Tmp_win->icon_x_loc;
	  xwc.y = Tmp_win->icon_y_loc;
	  xwcm = cre->value_mask & (CWX | CWY);
	  if (Tmp_win->icon_w != None)
	    XConfigureWindow (dpy, Tmp_win->icon_w, xwcm, &xwc);
	}
      return;
    }

  if (cre->value_mask & CWStackMode)
    {
      XfwmWindow *otherwin;

      xwc.sibling = (((cre->value_mask & CWSibling) &&
		      (XFindContext (dpy, cre->above, XfwmContext,
				     (caddr_t *) & otherwin) == XCSUCCESS))
		     ? otherwin->frame : cre->above);
      xwc.stack_mode = cre->detail;
      XConfigureWindow (dpy, Tmp_win->frame,
			cre->value_mask & (CWSibling | CWStackMode), &xwc);
      XSync (dpy, False);
    }

  if (ShapesSupported)
    {
      int xws, yws, xbs, ybs;
      unsigned wws, hws, wbs, hbs;
      int boundingShaped, clipShaped;

      XShapeQueryExtents (dpy, Tmp_win->w, &boundingShaped, &xws, &yws, &wws,
			  &hws, &clipShaped, &xbs, &ybs, &wbs, &hbs);
      Tmp_win->wShaped = boundingShaped;
    }

  /* Don't modify frame_XXX fields before calling SetupWindow! */
  x = Tmp_win->frame_x;
  y = Tmp_win->frame_y;
  width = Tmp_win->frame_width;
  height = Tmp_win->frame_height;

  /* for restoring */
  if (cre->value_mask & CWBorderWidth)
      Tmp_win->old_bw = cre->border_width;

  if (cre->value_mask & CWX)
    x = cre->x - Tmp_win->boundary_width - Tmp_win->bw;
  if (cre->value_mask & CWY)
    y = cre->y - Tmp_win->boundary_width - Tmp_win->title_height - Tmp_win->bw;
  if (cre->value_mask & CWWidth)
    width = cre->width + 2 * Tmp_win->boundary_width;
  if (cre->value_mask & CWHeight)
    height = cre->height + Tmp_win->title_height + 2 * Tmp_win->boundary_width;

  SetupFrame (Tmp_win, x, y, width, height, FALSE, TRUE);
}

/***********************************************************************
 *
 *  Procedure:
 *      HandleShapeNotify - shape notification event handler
 *
 ***********************************************************************/
void
HandleShapeNotify (void)
{
  if (ShapesSupported)
    {
      XShapeEvent *sev = (XShapeEvent *) & Event;

      if (!Tmp_win)
	return;
      if (sev->kind != ShapeBounding)
	return;
      Tmp_win->wShaped = sev->shaped;
      SetShape (Tmp_win, Tmp_win->frame_width);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleVisibilityNotify - record fully visible windows for
 *      use in the RaiseLower function and the OnTop type windows.
 *
 ************************************************************************/
void
HandleVisibilityNotify ()
{
  XVisibilityEvent *vevent = (XVisibilityEvent *) & Event;

  if (Tmp_win)
    {
      if (vevent->state == VisibilityUnobscured)
	Tmp_win->flags |= VISIBLE;
      else
	Tmp_win->flags &= ~VISIBLE;

      if (((vevent->state == VisibilityPartiallyObscured) ||
	   (vevent->state == VisibilityFullyObscured)) &&
	  (Tmp_win->flags & ONTOP) && (Tmp_win->flags & RAISED) && !(Tmp_win->flags & ICONIFIED))
	{
	  RaiseWindow (Tmp_win);
	  Tmp_win->flags &= ~RAISED;
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *     HandleRootMotionNotify - Root-MotionNotify event handler
 *
 ************************************************************************/
int _xfwm_deskwrap (int horz, int vert)
{
  register int ndesks = Scr.ndesks;
  register int desk = Scr.CurrentDesk;

  if (!ndesks) ndesks = 1;
    
  /*assumes that the desk-layout is alway 2 rows in height -gud */
  
  if (horz) 
  {
     desk += horz*2;
     if (desk >= ndesks) { desk -= ndesks; }
     if (desk < 0) { desk += ndesks; }
  }
  if (vert) 
  {
     desk ^= 1;
  }

  return desk;
}

void
sendclient_event(XfwmWindow *tmp_win, int x, int y, int w, int h)
{
  XEvent client_event;

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
  client_event.xconfigure.above = tmp_win->frame;
  client_event.xconfigure.override_redirect = False;
  XSendEvent (dpy, tmp_win->w, False, StructureNotifyMask, &client_event);
}

void
HandleRootMotionNotify ()
{
  if (Scr.SnapSize < MINRESISTANCE) 
    {
      return;
    }
  else
    { /* check if the pointer is pushed onto the screen border,
         if so, increment the corresponding X or Y counter (until Snapsize),
         otherwise reset them to null.
      */
           if (Event.xmotion.x_root == 0) { Scr.EdgeScrollX--; }
      else if (Event.xmotion.y_root == 0) { Scr.EdgeScrollY--; }
      else if (Event.xmotion.x_root >= Scr.MyDisplayWidth-1)  { Scr.EdgeScrollX++; }
      else if (Event.xmotion.y_root >= Scr.MyDisplayHeight-1) { Scr.EdgeScrollY++; }
      else { Scr.EdgeScrollY = Scr.EdgeScrollX = 0; }
      
      /* now check if some EdgeCounter has reached Snapsize, so that
         we shall emit signals to switch to the next desk (computed in 
         the deskwrap-helper)
      */
      if (Scr.EdgeScrollX <= (-EDGEFACTOR * Scr.SnapSize)) 
        { /* move desktop left */
          changeDesks (0, _xfwm_deskwrap(-1,0), 1, 1);
          Scr.EdgeScrollX = Scr.EdgeScrollY = 0;
          XWarpPointer (dpy, None, None, 0,0,0,0, +(Scr.MyDisplayWidth-Scr.SnapSize),(0));
	  return;
        }
      if (Scr.EdgeScrollX >= (EDGEFACTOR * Scr.SnapSize)) 
        { /* move desktop right */
          changeDesks (0, _xfwm_deskwrap(+1,0), 1, 1);
          Scr.EdgeScrollX = Scr.EdgeScrollY = 0;
          XWarpPointer (dpy, None, None, 0,0,0,0, -(Scr.MyDisplayWidth-Scr.SnapSize),(0));
	  return;
        }
      if (Scr.EdgeScrollY <= (-EDGEFACTOR * Scr.SnapSize)) 
        { /* move desktop upper */
          changeDesks (0, _xfwm_deskwrap(0,-1), 1, 1);
          Scr.EdgeScrollX = Scr.EdgeScrollY = 0;
          XWarpPointer (dpy, None, None, 0,0,0,0, (0),+(Scr.MyDisplayHeight-Scr.SnapSize));
	  return;
        }
      if (Scr.EdgeScrollY >= (EDGEFACTOR * Scr.SnapSize)) 
        { /* move desktop lower */
          changeDesks (0, _xfwm_deskwrap(0,+1), 1, 1);
          Scr.EdgeScrollX = Scr.EdgeScrollY = 0;
          XWarpPointer (dpy, None, None, 0,0,0,0, (0),-(Scr.MyDisplayHeight-Scr.SnapSize));
	  return;
        }
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleMotionNotify - MotionNotify event handler
 *
 ************************************************************************/
void
HandleMotionNotify ()
{
  /* multi screen? */
  if (Event.xany.window == Scr.Root)
    {
      HandleRootMotionNotify ();
    }
  return;
}

/***************************************************************************
 *
 * Waits for next X event, or for an auto-raise timeout.
 *
 ****************************************************************************/
int
My_XNextEvent (Display * dpy, XEvent * event)
{
  extern int fd_width, x_fd;
  struct itimerval value;
  fd_set in_fdset, out_fdset;
  Window child;
  Window targetWindow;
  int i, count;
  int retval;

  /* Do this IMMEDIATELY prior to select, to prevent any nasty
   * queued up X events from just hanging around waiting to be
   * flushed */
  /* Do this prior to the select() call, in case the timer already expired,
   * in which case the select would never return. */
  XFlush (dpy);
  if (XPending (dpy))
    {
      XNextEvent (dpy, event);
      StashEventTime (event);
      return 1;
    }

  if(alarmed)
    {
      alarmed = False;
      XQueryPointer(dpy,Scr.Root,&JunkRoot,&child,&JunkX,&JunkY,&JunkX, 
		    &JunkY,&JunkMask );
      if((Scr.Focus != NULL) && (child == Scr.Focus->frame))
	{
	  if(!(Scr.Focus->flags & VISIBLE))
	    { 
               SetTimer(0);
	       RaiseWindow(Scr.Focus);
	    }
	}
      return 0;
    }
  
  getitimer(ITIMER_REAL,&value);

  ReapChildren ();

  FD_ZERO (&in_fdset);
  FD_SET (x_fd, &in_fdset);
  FD_ZERO (&out_fdset);
  for (i = 0; i < npipes; i++)
    {
      if (readPipes[i] >= 0)
	{
	  FD_SET (readPipes[i], &in_fdset);
	}
    }

  for (i = 0; i < npipes; i++)
    {
      if (pipeQueue[i] != NULL)
	{
	  FD_SET (writePipes[i], &out_fdset);
	}
    }

  XFlush (dpy);
#ifdef __hpux
  retval = select (fd_width, (int *) &in_fdset, (int *) &out_fdset, 0, NULL);
#else
  retval = select (fd_width, &in_fdset, &out_fdset, 0, NULL);
#endif

  /* Check for module input. */
  for (i = 0; i < npipes; i++)
    {
      if (readPipes[i] >= 0)
	{
	  if ((retval > 0) && (FD_ISSET (readPipes[i], &in_fdset)))
	    {
	      if ((count =
		   read (readPipes[i], &targetWindow, sizeof (Window))) > 0)
		{
		  HandleModuleInput (targetWindow, i);
		}
	      if (count <= 0)
		{
		  KillModule (i, 10);
		}
	    }
	}
      if (writePipes[i] >= 0)
	{
	  if ((retval > 0) && (FD_ISSET (writePipes[i], &out_fdset)))
	    {
	      FlushQueue (i);
	    }
	}
    }
  return 0;
}
