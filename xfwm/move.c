/*
 * This module is all original code 
 * by Rob Nation 
 * Copyright 1993, Robert Nation
 *     You may use this code for any purpose, as long as the original
 *     copyright remains in the source code and all documentation
 ****************************************************************************/

/***********************************************************************
 *
 * code for moving windows
 *
 ***********************************************************************/

#include "configure.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <X11/keysym.h>
#include "xfwm.h"
#include "menus.h"
#include "misc.h"
#include "parse.h"
#include "screen.h"
#include "module.h"
#include "constant.h"

extern XEvent Event;
extern int menuFromFrameOrWindowOrTitlebar;
extern XfwmWindow *Tmp_win;		/* the current xfwm window */
extern int _xfwm_deskwrap (int x_off, int y_off);
Bool NeedToResizeToo;

/*forward*/ 
int XFwmMoveWindow (XfwmWindow* tmp_win, int xl, int yt, int Width,
                     int Height, int desk, Bool opaque_move);

/****************************************************************************
 *
 * Start a window move operation
 *
 ****************************************************************************/
void
move_window (XEvent * eventp, Window w, XfwmWindow * tmp_win,
	     unsigned long context, char *action, int *Module)
{
  int FinalX, FinalY;
  int val1, val2, val1_unit, val2_unit, n;
  int startDesk;
  
  n = GetTwoArguments (action, &val1, &val2, &val1_unit, &val2_unit);

  if (DeferExecution (eventp, &w, &tmp_win, &context, MOVE, ButtonPress))
    return;

  startDesk = Scr.CurrentDesk;
  /* gotta have a window */
  w = tmp_win->frame;
  RaiseWindow (tmp_win);
  if (tmp_win->flags & ICONIFIED)
    {
      if (tmp_win->icon_pixmap_w != None)
	{
	  XUnmapWindow (dpy, tmp_win->icon_w);
	  w = tmp_win->icon_pixmap_w;
	}
      else
	w = tmp_win->icon_w;
    }

  if (n == 2)
    {
      FinalX = val1 * val1_unit / 100;
      FinalY = val2 * val2_unit / 100;
    }
  else
    InteractiveMove (&w, tmp_win, &FinalX, &FinalY, eventp);

  if (w == tmp_win->frame)
    {
      if (startDesk != Scr.CurrentDesk)
        Broadcast (M_NEW_DESK, 1, Scr.CurrentDesk, 0, 0, 0, 0, 0, 0);
      SetupFrame (tmp_win, FinalX, FinalY,
		  tmp_win->frame_width, tmp_win->frame_height, TRUE, TRUE);
      MapIt (tmp_win);
      SetFocus (tmp_win->w, tmp_win, 0);
      RaiseWindow (tmp_win);
      KeepOnTop ();
      XFlush (dpy);
    }
  else
    /* icon window */
    {
      tmp_win->flags |= ICON_MOVED;
      tmp_win->icon_x_loc = FinalX;
      tmp_win->icon_xl_loc = FinalX -
	(tmp_win->icon_w_width - tmp_win->icon_p_width) / 2;
      tmp_win->icon_y_loc = FinalY;
      Broadcast (M_ICON_LOCATION, 7, tmp_win->w, tmp_win->frame,
		 (unsigned long) tmp_win,
		 tmp_win->icon_x_loc, tmp_win->icon_y_loc,
		 tmp_win->icon_w_width, tmp_win->icon_w_height
		 + tmp_win->icon_p_height);
      XMoveWindow (dpy, tmp_win->icon_w,
		   tmp_win->icon_xl_loc, FinalY + tmp_win->icon_p_height);
      if (tmp_win->icon_pixmap_w != None)
	{
	  XMapWindow (dpy, tmp_win->icon_w);
	  XMoveWindow (dpy, tmp_win->icon_pixmap_w, tmp_win->icon_x_loc, FinalY);
	  XMapWindow (dpy, w);
	}
    }

  return;
}

void
moveLoop (XfwmWindow * tmp_win, int XOffset, int YOffset, int Width,
	  int Height, int *FinalX, int *FinalY, register Bool opaque_move,
	  Bool AddWindow)
{
  XGCValues gcv;
  Bool finished = False;
  Bool done;
  int xl, yt;
  int olddesk = tmp_win->Desk;
  int newdesk = tmp_win->Desk;

  XQueryPointer (dpy, Scr.Root, &JunkRoot, &JunkChild, &xl, &yt,
		 &JunkX, &JunkY, &JunkMask);
  *FinalX = xl += XOffset;
  *FinalY = yt += YOffset;

  if (!opaque_move)
    {
      gcv.line_width = ((tmp_win->boundary_width > 4)
			? (tmp_win->boundary_width - 2) : 2);
      XChangeGC (dpy, Scr.DrawGC, GCLineWidth, &gcv);

      MoveOutline (tmp_win, xl, yt, Width, Height);
    }

  while (!finished)
    {
      XNextEvent(dpy, &Event);
      StashEventTime (&Event);

      /* discard any extra motion events before a logical release */
      if (Event.type == MotionNotify)
	  while (XCheckMaskEvent (dpy, PointerMotionMask | ButtonMotionMask |
				  ButtonPressMask | ButtonRelease, &Event))
	    {
	      StashEventTime (&Event);
	      if (Event.type == ButtonRelease) break;
	    }

      done = FALSE;
      /* Handle a limited number of key press events to allow mouseless
       * operation */
      if (Event.type == KeyPress)
	Keyboard_shortcuts (&Event, ButtonRelease);
      switch (Event.type)
	{
	case FocusIn:
	case FocusOut:
	case EnterNotify:
	case LeaveNotify:
	  /* simply ignore those events */
	  break;
	case KeyPress:
	  /* simple code to bag out of move - CKH */
	  if (XLookupKeysym (&(Event.xkey), 0) == XK_Escape)
	    {
	      if (!opaque_move)
		MoveOutline (tmp_win, 0, 0, 0, 0);
	      *FinalX = tmp_win->frame_x;
	      *FinalY = tmp_win->frame_y;

              if (!(tmp_win->flags & ICONIFIED))
	        SnapMove (FinalX, FinalY, Width, Height, tmp_win);

	      finished = TRUE;
	    }
	  done = TRUE;
	  break;
	case ButtonPress:
	  XAllowEvents (dpy, ReplayPointer, CurrentTime);
	  if ((Event.xbutton.button == 1) && (Event.xbutton.state & ShiftMask))
	      NeedToResizeToo = True;
	  else
	      done = 1;
	  break;
	case ButtonRelease:
	  if (!opaque_move)
	    MoveOutline (tmp_win, 0, 0, 0, 0);
	  xl = Event.xmotion.x_root + XOffset;
	  yt = Event.xmotion.y_root + YOffset;

	  if (!(tmp_win->flags & ICONIFIED))
	    SnapMove (&xl, &yt, Width, Height, tmp_win);

	  *FinalX = xl;
	  *FinalY = yt;

	  done = TRUE;
	  finished = TRUE;
	  break;
	case MotionNotify:
	  xl = Event.xmotion.x_root;
	  yt = Event.xmotion.y_root;
	  /* redraw the rubberband */
	  xl += XOffset;
	  yt += YOffset;

	  if (!(tmp_win->flags & ICONIFIED))
	    SnapMove (&xl, &yt, Width, Height, tmp_win);

	  if (tmp_win->flags & ICONIFIED)
	    {
	      tmp_win->icon_x_loc = xl;
	      tmp_win->icon_xl_loc = xl -
	    	(tmp_win->icon_w_width - tmp_win->icon_p_width) / 2;
	      tmp_win->icon_y_loc = yt;
	      if (tmp_win->icon_pixmap_w != None)
	    	XMoveWindow (dpy, tmp_win->icon_pixmap_w,
	    		     tmp_win->icon_x_loc, yt);
	      else if (tmp_win->icon_w != None)
	    	XMoveWindow (dpy, tmp_win->icon_w, tmp_win->icon_xl_loc,
	    		     yt + tmp_win->icon_p_height);

	    }
          else
	    newdesk = XFwmMoveWindow (tmp_win, xl, yt, Width, Height, newdesk, opaque_move);
	  done = TRUE;
	  break;
	case DestroyNotify:
	  if (!opaque_move) MoveOutline (tmp_win, 0,0,0,0);
	  finished = TRUE;
	  done = TRUE;
	  *FinalX = xl;
	  *FinalY = yt;
	  XPutBackEvent(dpy, &Event);
	  break;
	default:
	  if (!opaque_move) MoveOutline (tmp_win, 0,0,0,0);
          DispatchEvent ();
	  if (!opaque_move) MoveOutline (tmp_win, xl, yt, Width, Height);
	  break;
	}
    }
    if (newdesk != olddesk)
      tmp_win->Desk = newdesk;
}

/****************************************************************************
 *
 * For menus, move, and resize operations, we can effect keyboard 
 * shortcuts by warping the pointer.
 *
 ****************************************************************************/
void
Keyboard_shortcuts (XEvent * Event, int ReturnEvent)
{
  int x, y, x_root, y_root;
  int move_size, x_move, y_move;
  KeySym keysym;

  /* Pick the size of the cursor movement */
  move_size = Scr.EntryHeight;
  if (Event->xkey.state & ControlMask)
    move_size = 1;
  if (Event->xkey.state & ShiftMask)
    move_size = 100;

  keysym = XLookupKeysym (&Event->xkey, 0);

  x_move = 0;
  y_move = 0;
  switch (keysym)
    {
    case XK_Up:
    case XK_k:
    case XK_p:
      y_move = -move_size;
      break;
    case XK_Down:
    case XK_n:
    case XK_j:
      y_move = move_size;
      break;
    case XK_Left:
    case XK_b:
    case XK_h:
      x_move = -move_size;
      break;
    case XK_Right:
    case XK_f:
    case XK_l:
      x_move = move_size;
      break;
    case XK_Return:
    case XK_space:
      /* beat up the event */
      Event->type = ReturnEvent;
      break;
    case XK_Escape:
      /* simple code to bag out of move - CKH */
      /* return keypress event instead */
      Event->type = KeyPress;
      Event->xkey.keycode = XKeysymToKeycode (Event->xkey.display, keysym);
      break;
    default:
      break;
    }
  XQueryPointer (dpy, Scr.Root, &JunkRoot, &Event->xany.window,
		 &x_root, &y_root, &x, &y, &JunkMask);

  if ((x_move != 0) || (y_move != 0))
    {
      /* beat up the event */
      XWarpPointer (dpy, None, Scr.Root, 0, 0, 0, 0, x_root + x_move,
		    y_root + y_move);

      /* beat up the event */
      Event->type = MotionNotify;
      Event->xkey.x += x_move;
      Event->xkey.y += y_move;
      Event->xkey.x_root += x_move;
      Event->xkey.y_root += y_move;
    }
}


void
InteractiveMove (Window * win, XfwmWindow * tmp_win, int *FinalX, int *FinalY, XEvent * eventp)
{
  extern int Stashed_X, Stashed_Y;
  int origDragX, origDragY, DragX, DragY, DragWidth, DragHeight;
  int XOffset, YOffset;
  Window w;

  w = *win;

  InstallRootColormap ();
  if (menuFromFrameOrWindowOrTitlebar)
    {
      XWarpPointer (dpy, None, Scr.Root, 0, 0, 0, 0, Stashed_X, Stashed_Y);
      XFlush (dpy);
    }

  DragX = eventp->xbutton.x_root;
  DragY = eventp->xbutton.y_root;

  if (!GrabEm (MOVE))
    {
      XBell (dpy, Scr.screen);
      return;
    }

  XGetGeometry (dpy, w, &JunkRoot, &origDragX, &origDragY,
		(unsigned int *) &DragWidth, (unsigned int *) &DragHeight,
		&JunkBW, &JunkDepth);

  DragWidth += JunkBW;
  DragHeight += JunkBW;
  XOffset = origDragX - DragX;
  YOffset = origDragY - DragY;

  if (!(Scr.Options & MoveOpaqueWin) && !(tmp_win->flags & ICONIFIED))
    MyXGrabServer (dpy);
  moveLoop (tmp_win, XOffset, YOffset, DragWidth, DragHeight, FinalX, FinalY,
      (Scr.Options & MoveOpaqueWin) || (tmp_win->flags & ICONIFIED), False);
  if (!(Scr.Options & MoveOpaqueWin) && !(tmp_win->flags & ICONIFIED))
    MyXUngrabServer (dpy);
  UninstallRootColormap ();

  UngrabEm ();
}

inline void
domove(XfwmWindow* tmp_win, int xl, int yt, int Width, 
       int Height, Bool opaque_move)
{
  XWindowChanges frame_wc;
  unsigned long frame_mask;

  if (!opaque_move)
    MoveOutline (tmp_win, xl, yt, Width, Height);
  else
   {
     frame_wc.x = tmp_win->frame_x = tmp_win->shade_x = xl;
     frame_wc.y = tmp_win->frame_y = tmp_win->shade_y = yt;
     frame_mask = (CWX | CWY);
     XConfigureWindow (dpy, tmp_win->frame, frame_mask, &frame_wc);
     /*
     if (!(tmp_win->flags & SHADED))
       sendclient_event(tmp_win, xl, yt, Width, Height);
      */
   }
}

int
XFwmMoveWindow (XfwmWindow* tmp_win, int xl, int yt, int Width,
                int Height, int desk, Bool opaque_move)
{
  int newdesk = desk;
  if (Scr.SnapSize < MINRESISTANCE)
    { /* do the standard stuff */
      domove(tmp_win, xl, yt, Width, Height, opaque_move);
      return (tmp_win->Desk);
    }
  else
    { /* check if the pointer is pushed onto the screen border,
         if so, increment the corresponding X or Y counter (until Snapsize),
         otherwise reset them to null.
      */
           if (Event.xmotion.x_root == 0) { Scr.EdgeScrollX--; }
      else if (Event.xmotion.y_root == 0) { Scr.EdgeScrollY--; }
      else if (Event.xmotion.x_root >= Scr.MyDisplayWidth-1) { Scr.EdgeScrollX++; }
      else if (Event.xmotion.y_root >= Scr.MyDisplayHeight-1) { Scr.EdgeScrollY++; }
      else { Scr.EdgeScrollY = Scr.EdgeScrollX = 0; }
      
      /* now check if some EdgeCounter has reached Snapsize, so that
         we shall emit signals to switch to the next desk (computed in 
         the deskwrap-helper)
      */
      if (Scr.EdgeScrollX <= -Scr.SnapSize) 
        { /* move desktop left */
          Scr.EdgeScrollX = Scr.EdgeScrollY = 0;
          XWarpPointer (dpy, None, None, 0,0,0,0, +(Scr.MyDisplayWidth-Scr.SnapSize),(0));
	  if (!opaque_move)
	    MoveOutline (tmp_win, 0, 0, 0, 0);
          newdesk = _xfwm_deskwrap(-1,0);
	  if (opaque_move)
            tmp_win->Desk = newdesk;
          changeDesks (0, newdesk, 0, 0);
          DispatchEvent ();
	  domove(tmp_win, xl+(Scr.MyDisplayWidth-Scr.SnapSize), yt,Width, Height, opaque_move);
	  return (newdesk);
        }
      if (Scr.EdgeScrollX >= +Scr.SnapSize) 
        { /* move desktop right */
          Scr.EdgeScrollX = Scr.EdgeScrollY = 0;
          XWarpPointer (dpy, None, None, 0,0,0,0, -(Scr.MyDisplayWidth-Scr.SnapSize),(0));
	  if (!opaque_move)
	    MoveOutline (tmp_win, 0, 0, 0, 0);
          newdesk = _xfwm_deskwrap(+1,0);
	  if (opaque_move)
            tmp_win->Desk = newdesk;
          changeDesks (0, newdesk, 0, 0);
          DispatchEvent ();
          domove(tmp_win, xl-(Scr.MyDisplayWidth-Scr.SnapSize), yt,Width, Height, opaque_move);
	  return (newdesk);
        }
      if (Scr.EdgeScrollY <= -Scr.SnapSize) 
        { /* move desktop upper */
          Scr.EdgeScrollX = Scr.EdgeScrollY = 0;
          XWarpPointer (dpy, None, None, 0,0,0,0, (0),+(Scr.MyDisplayHeight-Scr.SnapSize));
	  if (!opaque_move)
	    MoveOutline (tmp_win, 0, 0, 0, 0);
          newdesk = _xfwm_deskwrap(0,-1);
	  if (opaque_move)
            tmp_win->Desk = newdesk;
          changeDesks (0, newdesk, 0, 0);
          DispatchEvent ();
	  domove(tmp_win, xl, yt+(Scr.MyDisplayHeight-Scr.SnapSize),Width, Height, opaque_move);
	  return (newdesk);
        }
      if (Scr.EdgeScrollY >= +Scr.SnapSize) 
        { /* move desktop lower */
          Scr.EdgeScrollX = Scr.EdgeScrollY = 0;
          XWarpPointer (dpy, None, None, 0,0,0,0, (0),-(Scr.MyDisplayHeight-Scr.SnapSize));
	  if (!opaque_move)
	    MoveOutline (tmp_win, 0, 0, 0, 0);
          newdesk = _xfwm_deskwrap(0,+1);
	  if (opaque_move)
            tmp_win->Desk = newdesk;
          changeDesks (0, newdesk, 0, 0);
          DispatchEvent ();
	  domove(tmp_win, xl, yt-(Scr.MyDisplayHeight-Scr.SnapSize),Width, Height, opaque_move);
	  return (newdesk);
        }
      domove(tmp_win, xl, yt, Width, Height, opaque_move);
      return (newdesk);
    }
  return (newdesk);
}


