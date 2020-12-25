
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


/**********************************************************************
 *
 * Add a new window, put the titlbar and other stuff around
 * the window
 *
 **********************************************************************/
#include "configure.h"

#include <X11/Xatom.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "xfwm.h"
#include "misc.h"
#include "screen.h"
#include <X11/Xresource.h>
#include <X11/extensions/shape.h>
#include "module.h"
#include "session.h"

/* Used to parse command line of clients for specific desk requests. */
/* Todo: check for multiple desks. */
static XrmDatabase db;
static XrmOptionDescRec table[] =
{
  /* Want to accept "-workspace N" or -xrm "xfwm*desk:N" as options
   * to specify the desktop. I have to include dummy options that
   * are meaningless since Xrm seems to allow -w to match -workspace
   * if there would be no ambiguity. */
  {"-workspacf", "*junk", XrmoptionSepArg, (caddr_t) NULL},
  {"-workspace", "*desk", XrmoptionSepArg, (caddr_t) NULL},
  {"-xrn", NULL, XrmoptionResArg, (caddr_t) NULL},
  {"-xrm", NULL, XrmoptionResArg, (caddr_t) NULL},
};

extern void GetMwmHints (XfwmWindow *);
extern void GetExtHints (XfwmWindow *);
extern void GetKDEHints (XfwmWindow *);

/***********************************************************************
 *
 *  Procedure:
 *	AddWindow - add a new window to the xfwm list
 *
 *  Returned Value:
 *	(XfwmWindow *) - pointer to the XfwmWindow structure
 *
 *  Inputs:
 *	w	- the window id of the window to add
 *	iconm	- flag to tell if this is an icon manager window
 *
 ***********************************************************************/
XfwmWindow *
AddWindow (Window w)
{
  XfwmWindow *tmp_win;		/* new xfwm window structure */
  unsigned long valuemask;	/* mask for create windows */
  XSetWindowAttributes attributes;	/* attributes for create windows */
  int i, x, y, width, height;
  int a, b;
  char *value;
  unsigned long tflag;
  int Desk, border_width;
  extern XfwmWindow *colormap_win;
  char *forecolor = NULL, *backcolor = NULL;
  int client_argc;
  char **client_argv = NULL, *str_type;
  Bool status;
  XrmValue rm_value;
  XTextProperty text_prop;
  extern Bool PPosOverride;
  Atom atype;
  int aformat;
  unsigned long nitems, bytes_remain;
  unsigned char *prop;

  XSync (dpy, 0);
  /* allocate space for the xfwm window */
  tmp_win = (XfwmWindow *) calloc (1, sizeof (XfwmWindow));
  if (tmp_win == (XfwmWindow *) 0)
    {
      return NULL;
    }
  tmp_win->flags = 0;
  tmp_win->w = w;
  tmp_win->icon_arranged = False;
  
  tmp_win->cmap_windows = (Window *) NULL;

  if (!PPosOverride)
    if (XGetGeometry (dpy, tmp_win->w, &JunkRoot, &JunkX, &JunkY,
		      &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth) == 0)
      {
	free ((char *) tmp_win);
	return (NULL);
      }
  tmp_win->name = NoName;
  if (XGetWMName (dpy, tmp_win->w, &text_prop) != 0)
    {
      if (text_prop.format == 8)
        { /* Multi-byte string */
          char **text_list = NULL;
          int text_list_num;
          if (XmbTextPropertyToTextList(dpy, &text_prop,
                                &text_list, &text_list_num) == Success)
            {
	      if (text_list)
	        {
                  tmp_win->name = (char *) malloc(strlen (text_list[0]) + 1);
                  strcpy (tmp_win->name, text_list[0]);
                  XFreeStringList(text_list);
	        } 
            }
          else
            tmp_win->name = (char *) text_prop.value;
        }
      else
        tmp_win->name = (char *) text_prop.value;
    }

  /* removing NoClass change for now... */
  tmp_win->class.res_name = NoResource;
  tmp_win->class.res_class = NoClass;
  XGetClassHint (dpy, tmp_win->w, &tmp_win->class);
  if (tmp_win->class.res_name == NULL)
    tmp_win->class.res_name = NoResource;
  if (tmp_win->class.res_class == NULL)
    tmp_win->class.res_class = NoClass;

  FetchWmProtocols (tmp_win);
  FetchWmColormapWindows (tmp_win);
  if (!(XGetWindowAttributes (dpy, tmp_win->w, &(tmp_win->attr))))
    tmp_win->attr.colormap = Scr.XfwmRoot.attr.colormap;

  tmp_win->wmhints = XGetWMHints (dpy, tmp_win->w);

  if (XGetTransientForHint (dpy, tmp_win->w, &tmp_win->transientfor))
    tmp_win->flags |= TRANSIENT;
  else
    tmp_win->flags &= ~TRANSIENT;

  tmp_win->old_bw = tmp_win->attr.border_width;

  if (ShapesSupported)
    {
      int xws, yws, xbs, ybs;
      unsigned wws, hws, wbs, hbs;
      int boundingShaped, clipShaped;

      XShapeSelectInput (dpy, tmp_win->w, ShapeNotifyMask);
      XShapeQueryExtents (dpy, tmp_win->w,
			  &boundingShaped, &xws, &yws, &wws, &hws,
			  &clipShaped, &xbs, &ybs, &wbs, &hbs);
      tmp_win->wShaped = boundingShaped;
    }

  tflag = LookInList (Scr.TheList, tmp_win->name, &tmp_win->class, &value,
		      &Desk, &border_width, &forecolor, &backcolor, &tmp_win->buttons);


  tmp_win->title_height = GetDecor (tmp_win, TitleHeight) + tmp_win->bw;

  GetMwmHints (tmp_win);
  GetExtHints (tmp_win);
  GetKDEHints (tmp_win);

  SelectDecor (tmp_win, tflag, border_width);

  tmp_win->flags |= tflag & ALL_COMMON_FLAGS;

  /* find a suitable icon pixmap */
  if (tflag & ICON_FLAG)
    {
      /* an icon was specified */
      tmp_win->icon_bitmap_file = value;
    }
  else if ((tmp_win->wmhints)
	   && (tmp_win->wmhints->flags & (IconWindowHint | IconPixmapHint)))
    {
      /* window has its own icon */
      tmp_win->icon_bitmap_file = NULL;
    }
  else
    {
      /* use default icon */
      tmp_win->icon_bitmap_file = Scr.DefaultIcon;
    }

  GetWindowSizeHints (tmp_win);

  /* Tentative size estimate */
  tmp_win->frame_width = tmp_win->attr.width + 2 * tmp_win->boundary_width;
  tmp_win->frame_height = tmp_win->attr.height + tmp_win->title_height +
    2 * tmp_win->boundary_width;

  ConstrainSize (tmp_win, &tmp_win->frame_width, &tmp_win->frame_height);

  /* Find out if the client requested a specific desk on the command line. */
  if (XGetCommand (dpy, tmp_win->w, &client_argv, &client_argc))
    {
      XrmParseCommand (&db, table, 4, "xfwm", &client_argc, client_argv);
      status = XrmGetResource (db, "xfwm.desk", "Xfwm.Desk", &str_type, &rm_value);
      if ((status == True) && (rm_value.size != 0))
	{
	  Desk = atoi (rm_value.addr);
	  tflag |= STARTSONDESK_FLAG;
	}
      XrmDestroyDatabase (db);
      db = NULL;
      XFreeStringList (client_argv);
    }

  if (!PlaceWindow (tmp_win, tflag, Desk))
    return NULL;

  /* Load Session Management saved values */
  MatchWinToSM(tmp_win);

  /* If restarting, retrieve parameters saved as Atoms */
 if ((XGetWindowProperty (dpy, tmp_win->w, _XA_XFWM_FLAGS, 0L, 1L, True,
                          _XA_XFWM_FLAGS, &atype, &aformat, &nitems,
                          &bytes_remain, &prop)) == Success)
   {
     if (prop != NULL)
       {
         tmp_win->flags |= (*(unsigned long *) prop) & (STICKY | ICON_MOVED | SHADED);
         XFree (prop);
       }
   }
  
 if ((XGetWindowProperty (dpy, tmp_win->w, _XA_XFWM_ICONPOS_X, 0L, 1L, True,
                          _XA_XFWM_ICONPOS_X, &atype, &aformat, &nitems,
                          &bytes_remain, &prop)) == Success)
   {
     if (prop != NULL)
       {
         tmp_win->icon_x_loc = *(int *) prop;
         XFree (prop);
       }
   }

 if ((XGetWindowProperty (dpy, tmp_win->w, _XA_XFWM_ICONPOS_Y, 0L, 1L, True,
                          _XA_XFWM_ICONPOS_Y, &atype, &aformat, &nitems,
                          &bytes_remain, &prop)) == Success)
   {
     if (prop != NULL)
       {
         tmp_win->icon_y_loc = *(int *) prop;
         XFree (prop);
       }
   }

  /*
   * Make sure the client window still exists.  We don't want to leave an
   * orphan frame window if it doesn't.  Since we now have the server
   * grabbed, the window can't disappear later without having been
   * reparented, so we'll get a DestroyNotify for it.  We won't have
   * gotten one for anything up to here, however.
   */
  XSync (dpy, 0);
  if (XGetGeometry (dpy, w, &JunkRoot, &JunkX, &JunkY,
		    &JunkWidth, &JunkHeight,
		    &JunkBW, &JunkDepth) == 0)
    {
      free ((char *) tmp_win);
      return (NULL);
    }

  XSetWindowBorderWidth (dpy, tmp_win->w, 0);

  tmp_win->icon_name = NULL;
  if (XGetWMIconName (dpy, tmp_win->w, &text_prop) != 0) 
    {
      if (text_prop.format == 8)
	{ /* Multi-byte string */
	  char **text_list = NULL;
	  int text_list_num;
	  if (XmbTextPropertyToTextList(dpy, &text_prop,
                        	&text_list, &text_list_num) == Success)
            {
              if (text_list)
	        {
                  tmp_win->icon_name = (char *) malloc(strlen (text_list[0]) + 1);
                  strcpy (tmp_win->icon_name, text_list[0]);
                  XFreeStringList(text_list);
	        }
            }
	  else
            tmp_win->icon_name = (char *) text_prop.value;
	}
      else
	tmp_win->icon_name = (char *) text_prop.value;
    }
  else
    tmp_win->icon_name = tmp_win->name;
  
  if (tmp_win->icon_name == (char *) NULL)
    tmp_win->icon_name = tmp_win->name;

  tmp_win->flags &= ~ICONIFIED;
  tmp_win->flags &= ~ICON_UNMAPPED;
  tmp_win->flags &= ~MAXIMIZED;

  tmp_win->TextPixel = Scr.MenuColors.fore;

  if (forecolor != NULL)
    {
      XColor color;

      if ((XParseColor (dpy, Scr.XfwmRoot.attr.colormap, forecolor, &color))
	  && (XAllocColor (dpy, Scr.XfwmRoot.attr.colormap, &color)))
	{
	  tmp_win->TextPixel = color.pixel;
	}
    }
  tmp_win->BackPixel = GetDecor (tmp_win, LoColors.back);
  attributes.background_pixel = tmp_win->BackPixel;
  valuemask = CWBackPixel;

  /* add the window into the xfwm list */
  tmp_win->next = Scr.XfwmRoot.next;
  if (Scr.XfwmRoot.next != NULL)
    Scr.XfwmRoot.next->prev = tmp_win;
  tmp_win->prev = &Scr.XfwmRoot;
  Scr.XfwmRoot.next = tmp_win;

  /* create windows */
  tmp_win->frame_x = tmp_win->attr.x + tmp_win->old_bw - tmp_win->bw;
  tmp_win->frame_y = tmp_win->attr.y + tmp_win->old_bw - tmp_win->bw;

  tmp_win->frame_width = tmp_win->attr.width + 2 * tmp_win->boundary_width;
  tmp_win->frame_height = tmp_win->attr.height + tmp_win->title_height +
    2 * tmp_win->boundary_width;
  tmp_win->shade_x = tmp_win->frame_x;
  tmp_win->shade_y = tmp_win->frame_y;
  tmp_win->shade_width = tmp_win->frame_width;
  tmp_win->shade_height = tmp_win->title_height + 2 * tmp_win->boundary_width;
  ConstrainSize (tmp_win, &tmp_win->frame_width, &tmp_win->frame_height);

  valuemask |= CWCursor | CWEventMask;
  attributes.cursor = Scr.XfwmCursors[DEFAULT];
  attributes.event_mask = (SubstructureRedirectMask | 
  		           ButtonPressMask | ButtonReleaseMask | 
                           EnterWindowMask | LeaveWindowMask | 
                           ExposureMask);

  tmp_win->frame =
    XCreateWindow (dpy, Scr.Root, tmp_win->frame_x, tmp_win->frame_y,
		   tmp_win->frame_width, tmp_win->frame_height,
		   tmp_win->bw, CopyFromParent, InputOutput,
		   CopyFromParent,
		   (valuemask & ~CWBackPixel),
		   &attributes);

  attributes.save_under = FALSE;
  attributes.backing_store = WhenMapped;
  attributes.event_mask = (ButtonPressMask | ButtonReleaseMask | 
                           ExposureMask);

  valuemask |= CWBackingStore;
  tmp_win->title_x = tmp_win->title_y = 0;
  tmp_win->title_w = 0;
  tmp_win->title_width = tmp_win->frame_width - 2 * tmp_win->corner_width
    - 3 + tmp_win->bw;
  if (tmp_win->title_width < 1)
    tmp_win->title_width = 1;
  if (tmp_win->flags & BORDER)
    {
      /* Just dump the windows any old place and left SetupFrame take
       * care of the mess */
      for (i = 0; i < 4; i++)
	{
	  attributes.cursor = Scr.XfwmCursors[TOP_LEFT + i];
	  tmp_win->corners[i] =
	    XCreateWindow (dpy, tmp_win->frame, 0, 0,
			   tmp_win->corner_width, tmp_win->corner_width,
			   0, CopyFromParent, InputOutput,
			   CopyFromParent,
			   valuemask,
			   &attributes);
	}
    }
    
  if (tmp_win->flags & TITLE)
    {
      tmp_win->title_x = tmp_win->boundary_width + tmp_win->title_height + 1;
      tmp_win->title_y = tmp_win->boundary_width;
      attributes.cursor = Scr.XfwmCursors[TITLE_CURSOR];
      tmp_win->title_w =
	XCreateWindow (dpy, tmp_win->frame, tmp_win->title_x, tmp_win->title_y,
		       tmp_win->title_width, tmp_win->title_height, 0,
		       CopyFromParent, InputOutput, CopyFromParent,
		       (valuemask & ~CWBackPixel), &attributes);
      attributes.cursor = Scr.XfwmCursors[SYS];
      for (i = 2; i >= 0; i--)
	{
	  if ((i < Scr.nr_left_buttons) && (tmp_win->left_w[i] > 0))
	    {
	      tmp_win->left_w[i] =
		XCreateWindow (dpy, tmp_win->frame, tmp_win->title_height * i, 0,
			    tmp_win->title_height, tmp_win->title_height, 0,
			       CopyFromParent, InputOutput,
			       CopyFromParent,
			       valuemask,
			       &attributes);
	    }
	  else
	    tmp_win->left_w[i] = None;

	  if ((i < Scr.nr_right_buttons) && (tmp_win->right_w[i] > 0))
	    {
	      tmp_win->right_w[i] =
		XCreateWindow (dpy, tmp_win->frame,
			       tmp_win->title_width -
			       tmp_win->title_height * (i + 1),
			       0, tmp_win->title_height,
			       tmp_win->title_height,
			       0, CopyFromParent, InputOutput,
			       CopyFromParent,
			       valuemask,
			       &attributes);
	    }
	  else
	    tmp_win->right_w[i] = None;
	}
    }

  if (tmp_win->flags & BORDER)
    {
      for (i = 0; i < 4; i++)
	{
	  attributes.cursor = Scr.XfwmCursors[TOP + i];
	  tmp_win->sides[i] =
	    XCreateWindow (dpy, tmp_win->frame, 0, 0, tmp_win->boundary_width,
			   tmp_win->boundary_width, 0, CopyFromParent,
			   InputOutput, CopyFromParent,
			   (valuemask & ~CWBackPixel),
			   &attributes);
	}
    }


  XMapSubwindows (dpy, tmp_win->frame);

  XReparentWindow (dpy, tmp_win->w, tmp_win->frame, 0, 0);

  valuemask = (CWEventMask | CWDontPropagate | CWBackingStore);
  attributes.event_mask = (StructureNotifyMask | 
  		           PropertyChangeMask |
			   VisibilityChangeMask | 
			   ColormapChangeMask | 
                           EnterWindowMask | LeaveWindowMask | 
                           FocusChangeMask);

  attributes.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask;

  XChangeWindowAttributes (dpy, tmp_win->w, valuemask, &attributes);
  tmp_win->name = NoName;
  if (XGetWMName (dpy, tmp_win->w, &text_prop) != 0)
    {
      if (text_prop.format == 8)
        { /* Multi-byte string */
          char **text_list = NULL;
          int text_list_num;
          if (XmbTextPropertyToTextList(dpy, &text_prop,
                                &text_list, &text_list_num) == Success)
            {
	      if (text_list)
	        {
                  tmp_win->name = (char *) malloc(strlen (text_list[0]) + 1);
                  strcpy (tmp_win->name, text_list[0]);
                  XFreeStringList(text_list);
	        } 
            }
          else
            tmp_win->name = (char *) text_prop.value;
        }
      else
        tmp_win->name = (char *) text_prop.value;
    }

  XAddToSaveSet (dpy, tmp_win->w);

  /*
   * Reparenting generates an UnmapNotify event, followed by a MapNotify.
   * Set the map state to FALSE to prevent a transition back to
   * WithdrawnState in HandleUnmapNotify.  Map state gets set correctly
   * again in HandleMapNotify.
   */
  tmp_win->flags &= ~MAPPED;
  x = tmp_win->frame_x;
  tmp_win->frame_x = 0;
  y = tmp_win->frame_y;
  tmp_win->frame_y = 0;
  width = tmp_win->frame_width;
  tmp_win->frame_width = 0;
  height = tmp_win->frame_height;
  tmp_win->frame_height = 0;
  SetupFrame (tmp_win, x, y, width, height, TRUE, TRUE);

  /* wait until the window is iconified and the icon window is mapped
   * before creating the icon window 
   */
  tmp_win->icon_w = None;
  GrabButtons (tmp_win);
  GrabKeys (tmp_win);
  XSaveContext (dpy, tmp_win->w, XfwmContext, (caddr_t) tmp_win);
  XSaveContext (dpy, tmp_win->frame, XfwmContext, (caddr_t) tmp_win);
  if (tmp_win->flags & TITLE)
    {
      XSaveContext (dpy, tmp_win->title_w, XfwmContext, (caddr_t) tmp_win);
      for (i = 0; i < Scr.nr_left_buttons; i++)
	XSaveContext (dpy, tmp_win->left_w[i], XfwmContext, (caddr_t) tmp_win);
      for (i = 0; i < Scr.nr_right_buttons; i++)
	if (tmp_win->right_w[i] != None)
	  XSaveContext (dpy, tmp_win->right_w[i], XfwmContext,
			(caddr_t) tmp_win);
    }
  if (tmp_win->flags & BORDER)
    {
      for (i = 0; i < 4; i++)
	{
	  XSaveContext (dpy, tmp_win->sides[i], XfwmContext, (caddr_t) tmp_win);
	  XSaveContext (dpy, tmp_win->corners[i], XfwmContext, (caddr_t) tmp_win);
	}
    }

  RaiseWindow (tmp_win);
  KeepOnTop();

  XGetGeometry (dpy, tmp_win->w, &JunkRoot, &JunkX, &JunkY,
		&JunkWidth, &JunkHeight, &JunkBW, &JunkDepth);
  XTranslateCoordinates (dpy, tmp_win->frame, Scr.Root, JunkX, JunkY,
			 &a, &b, &JunkChild);
  tmp_win->xdiff -= a;
  tmp_win->ydiff -= b;
  
  XGrabButton (dpy, AnyButton, 0, tmp_win->frame, True,
               ButtonPressMask, GrabModeSync, GrabModeAsync, None,
               None);
  XGrabButton (dpy, AnyButton, AnyModifier, tmp_win->frame, True,
               ButtonPressMask, GrabModeSync, GrabModeAsync, None,
               None);
  
  BroadcastConfig (M_ADD_WINDOW, tmp_win);

  BroadcastName (M_WINDOW_NAME, tmp_win->w, tmp_win->frame,
		 (unsigned long) tmp_win, tmp_win->name);
  BroadcastName (M_ICON_NAME, tmp_win->w, tmp_win->frame,
		 (unsigned long) tmp_win, tmp_win->icon_name);
  if (tmp_win->icon_bitmap_file != NULL &&
      tmp_win->icon_bitmap_file != Scr.DefaultIcon)
    BroadcastName (M_ICON_FILE, tmp_win->w, tmp_win->frame,
		   (unsigned long) tmp_win, tmp_win->icon_bitmap_file);
  BroadcastName (M_RES_CLASS, tmp_win->w, tmp_win->frame,
		 (unsigned long) tmp_win, tmp_win->class.res_class);
  BroadcastName (M_RES_NAME, tmp_win->w, tmp_win->frame,
		 (unsigned long) tmp_win, tmp_win->class.res_name);

  FetchWmProtocols (tmp_win);
  FetchWmColormapWindows (tmp_win);
  if (!(XGetWindowAttributes (dpy, tmp_win->w, &(tmp_win->attr))))
    tmp_win->attr.colormap = Scr.XfwmRoot.attr.colormap;
  InstallWindowColormaps (colormap_win);

  XSync(dpy,0);
  return (tmp_win);
}

/***********************************************************************
 *
 *  Procedure:
 *	GrabButtons - grab needed buttons for the window
 *
 *  Inputs:
 *	tmp_win - the xfwm window structure to use
 *
 ***********************************************************************/
void
GrabButtons (XfwmWindow * tmp_win)
{
  Binding *MouseEntry;

  MouseEntry = Scr.AllBindings;
  while (MouseEntry != (Binding *) 0)
    {
      if ((MouseEntry->Action != NULL) && (MouseEntry->Context & C_WINDOW)
	  && (MouseEntry->IsMouse == 1))
	{
	  if (MouseEntry->Button_Key > 0)
	    {
	      XGrabButton (dpy, MouseEntry->Button_Key, MouseEntry->Modifier,
			   tmp_win->w,
			   True, ButtonPressMask | ButtonReleaseMask,
			   GrabModeAsync, GrabModeAsync, None,
			   None);
	    }
	  else
	    {
	      XGrabButton (dpy, AnyButton, MouseEntry->Modifier,
			   tmp_win->w,
			   True, ButtonPressMask | ButtonReleaseMask,
			   GrabModeAsync, GrabModeAsync, None,
			   None);
	    }
	}
      MouseEntry = MouseEntry->NextBinding;
    }
  return;
}

/***********************************************************************
 *
 *  Procedure:
 *	GrabKeys - grab needed keys for the window
 *
 *  Inputs:
 *	tmp_win - the xfwm window structure to use
 *
 ***********************************************************************/
void
GrabKeys (XfwmWindow * tmp_win)
{
  Binding *tmp;
  for (tmp = Scr.AllBindings; tmp != NULL; tmp = tmp->NextBinding)
    {
      if ((tmp->Context & (C_WINDOW | C_TITLE | C_RALL | C_LALL | C_SIDEBAR)) &&
	  (tmp->IsMouse == 0))
	{
	  XGrabKey (dpy, tmp->Button_Key, tmp->Modifier, tmp_win->frame, True,
		    GrabModeAsync, GrabModeAsync);
	  if (tmp->Modifier != AnyModifier)
	    {
	      XGrabKey (dpy, tmp->Button_Key, tmp->Modifier | LockMask,
			tmp_win->frame, True,
			GrabModeAsync, GrabModeAsync);
              if (NumLockMask != 0)
	        {
	          XGrabKey (dpy, tmp->Button_Key, tmp->Modifier | NumLockMask,
			tmp_win->frame, True,
			GrabModeAsync, GrabModeAsync);
	          XGrabKey (dpy, tmp->Button_Key, tmp->Modifier | NumLockMask | LockMask,
			tmp_win->frame, True,
			GrabModeAsync, GrabModeAsync);
	        }
              if (ScrollLockMask != 0)
	        {
	          XGrabKey (dpy, tmp->Button_Key, tmp->Modifier | ScrollLockMask,
			tmp_win->frame, True,
			GrabModeAsync, GrabModeAsync);
	          XGrabKey (dpy, tmp->Button_Key, tmp->Modifier | ScrollLockMask | LockMask,
			tmp_win->frame, True,
			GrabModeAsync, GrabModeAsync);
	        }
              if ((NumLockMask != 0) && (ScrollLockMask != 0))
	        {
	          XGrabKey (dpy, tmp->Button_Key, tmp->Modifier | NumLockMask | ScrollLockMask,
			tmp_win->frame, True,
			GrabModeAsync, GrabModeAsync);
	          XGrabKey (dpy, tmp->Button_Key, tmp->Modifier | NumLockMask | ScrollLockMask | LockMask,
			tmp_win->frame, True,
			GrabModeAsync, GrabModeAsync);
	        }
	    }
	}
    }
  return;
}

/***********************************************************************
 *
 *  Procedure:
 *	FetchWMProtocols - finds out which protocols the window supports
 *
 *  Inputs:
 *	tmp - the xfwm window structure to use
 *
 ***********************************************************************/
void
FetchWmProtocols (XfwmWindow * tmp)
{
  unsigned long flags = 0L;
  Atom *protocols = NULL, *ap;
  int i, n;
  Atom atype;
  int aformat;
  unsigned long bytes_remain, nitems;

  if (tmp == NULL)
    return;
  /* First, try the Xlib function to read the protocols.
   * This is what Twm uses. */
  if (XGetWMProtocols (dpy, tmp->w, &protocols, &n))
    {
      for (i = 0, ap = protocols; i < n; i++, ap++)
	{
	  if (*ap == (Atom) _XA_WM_TAKE_FOCUS)
	    flags |= DoesWmTakeFocus;
	  if (*ap == (Atom) _XA_WM_DELETE_WINDOW)
	    flags |= DoesWmDeleteWindow;
	}
      if (protocols)
	XFree ((char *) protocols);
    }
  else
    {
      /* Next, read it the hard way. mosaic from Coreldraw needs to 
       * be read in this way. */
      if ((XGetWindowProperty (dpy, tmp->w, _XA_WM_PROTOCOLS, 0L, 10L, False,
			       _XA_WM_PROTOCOLS, &atype, &aformat, &nitems,
			       &bytes_remain,
			       (unsigned char **) &protocols)) == Success)
	{
	  for (i = 0, ap = protocols; i < nitems; i++, ap++)
	    {
	      if (*ap == (Atom) _XA_WM_TAKE_FOCUS)
		flags |= DoesWmTakeFocus;
	      if (*ap == (Atom) _XA_WM_DELETE_WINDOW)
		flags |= DoesWmDeleteWindow;
	    }
	  if (protocols)
	    XFree ((char *) protocols);
	}
    }
  tmp->flags |= flags;
  return;
}

/***********************************************************************
 *
 *  Procedure:
 *	GetWindowSizeHints - gets application supplied size info
 *
 *  Inputs:
 *	tmp - the xfwm window structure to use
 *
 ***********************************************************************/
void
GetWindowSizeHints (XfwmWindow * tmp)
{
  long supplied = 0;

  if (!XGetWMNormalHints (dpy, tmp->w, &tmp->hints, &supplied))
    tmp->hints.flags = 0;

  /* Beat up our copy of the hints, so that all important field are
   * filled in! */
  if (tmp->hints.flags & PResizeInc)
    {
      if (tmp->hints.width_inc == 0)
	tmp->hints.width_inc = 1;
      if (tmp->hints.height_inc == 0)
	tmp->hints.height_inc = 1;
    }
  else
    {
      tmp->hints.width_inc = 1;
      tmp->hints.height_inc = 1;
    }

  /*
   * ICCCM says that PMinSize is the default if no PBaseSize is given,
   * and vice-versa.
   */

  if (!(tmp->hints.flags & PBaseSize))
    {
      if (tmp->hints.flags & PMinSize)
	{
	  tmp->hints.base_width = tmp->hints.min_width;
	  tmp->hints.base_height = tmp->hints.min_height;
	}
      else
	{
	  tmp->hints.base_width = 0;
	  tmp->hints.base_height = 0;
	}
    }
  if (!(tmp->hints.flags & PMinSize))
    {
      tmp->hints.min_width = tmp->hints.base_width;
      tmp->hints.min_height = tmp->hints.base_height;
    }
  if (!(tmp->hints.flags & PMaxSize))
    {
      tmp->hints.max_width = MAX_WINDOW_WIDTH;
      tmp->hints.max_height = MAX_WINDOW_HEIGHT;
    }
  if (tmp->hints.max_width < tmp->hints.min_width)
    tmp->hints.max_width = MAX_WINDOW_WIDTH;
  if (tmp->hints.max_height < tmp->hints.min_height)
    tmp->hints.max_height = MAX_WINDOW_HEIGHT;

  /* Zero width/height windows are bad news! */
  if (tmp->hints.min_height <= 0)
    tmp->hints.min_height = 1;
  if (tmp->hints.min_width <= 0)
    tmp->hints.min_width = 1;

  if (!(tmp->hints.flags & PWinGravity))
    {
      tmp->hints.win_gravity = NorthWestGravity;
      tmp->hints.flags |= PWinGravity;
    }
}


/***********************************************************************
 *
 *  Procedure:
 *	LookInList - look through a list for a window name, or class
 *
 *  Returned Value:
 *	the ptr field of the list structure or NULL if the name 
 *	or class was not found in the list
 *
 *  Inputs:
 *	list	- a pointer to the head of a list
 *	name	- a pointer to the name to look for
 *	class	- a pointer to the class to look for
 *
 ***********************************************************************/
unsigned long
LookInList (name_list * list, char *name, XClassHint * class,
	    char **value,
	    int *Desk, int *border_width,
	    char **forecolor, char **backcolor,
	    unsigned long *buttons)
{
  name_list *nptr;
  unsigned long retval = 0;

  *value = NULL;
  *forecolor = NULL;
  *backcolor = NULL;
  *Desk = 0;
  *buttons = 0;
  *border_width = 0;

  /* look for the name first */
  for (nptr = list; nptr != NULL; nptr = nptr->next)
    {
      if (class)
	{
	  /* first look for the res_class  (lowest priority) */
	  if (matchWildcards (nptr->name, class->res_class) == TRUE)
	    {
	      if (nptr->value != NULL)
		*value = nptr->value;
	      if (nptr->off_flags & STARTSONDESK_FLAG)
		*Desk = nptr->Desk;
	      if (nptr->off_flags & BW_FLAG)
		*border_width = nptr->border_width;
	      if (nptr->off_flags & FORE_COLOR_FLAG)
		*forecolor = nptr->ForeColor;
	      if (nptr->off_flags & BACK_COLOR_FLAG)
		*backcolor = nptr->BackColor;
	      retval |= nptr->off_flags;
	      retval &= ~(nptr->on_flags);
	      *buttons |= nptr->off_buttons;
	      *buttons &= ~(nptr->on_buttons);
	    }

	  /* look for the res_name next */
	  if (matchWildcards (nptr->name, class->res_name) == TRUE)
	    {
	      if (nptr->value != NULL)
		*value = nptr->value;
	      if (nptr->off_flags & STARTSONDESK_FLAG)
		*Desk = nptr->Desk;
	      if (nptr->off_flags & FORE_COLOR_FLAG)
		*forecolor = nptr->ForeColor;
	      if (nptr->off_flags & BACK_COLOR_FLAG)
		*backcolor = nptr->BackColor;
	      if (nptr->off_flags & BW_FLAG)
		*border_width = nptr->border_width;
	      retval |= nptr->off_flags;
	      retval &= ~(nptr->on_flags);
	      *buttons |= nptr->off_buttons;
	      *buttons &= ~(nptr->on_buttons);
	    }
	}
      /* finally, look for name matches */
      if (matchWildcards (nptr->name, name) == TRUE)
	{
	  if (nptr->value != NULL)
	    *value = nptr->value;
	  if (nptr->off_flags & STARTSONDESK_FLAG)
	    *Desk = nptr->Desk;
	  if (nptr->off_flags & FORE_COLOR_FLAG)
	    *forecolor = nptr->ForeColor;
	  if (nptr->off_flags & BACK_COLOR_FLAG)
	    *backcolor = nptr->BackColor;
	  if (nptr->off_flags & BW_FLAG)
	    *border_width = nptr->border_width;
	  retval |= nptr->off_flags;
	  retval &= ~(nptr->on_flags);
	  *buttons |= nptr->off_buttons;
	  *buttons &= ~(nptr->on_buttons);
	}
    }
  return retval;
}
