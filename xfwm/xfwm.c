
/****************************************************************************
 * This module is all original code 
 * by Rob Nation 
 * Copyright 1993, Robert Nation
 *     You may use this code for any purpose, as long as the original
 *     copyright remains in the source code and all documentation
 ****************************************************************************/

#include "configure.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "xfwm.h"
#include "menus.h"
#include "misc.h"
#include "screen.h"
#include "parse.h"
#include "module.h"
#include "../xfce/constant.h"

#include <X11/Xproto.h>
#include <X11/Xatom.h>
/* need to get prototype for XrmUniqueQuark for XUniqueContext call */
#include <X11/Xresource.h>

#include <X11/extensions/shape.h>

#if defined (sparc) && defined (SVR4)
/* Solaris has sysinfo instead of gethostname.  */
#include <sys/systeminfo.h>
#endif

#define MAXHOSTNAME 255

#include "version.h"

int master_pid;			/* process number of 1st xfwm process */

ScreenInfo Scr;			/* structures for the screen */
Display *dpy;			/* which display are we talking to */

Window BlackoutWin = None;	/* window to hide window captures */

char *default_config_command = "Read .xfwmrc";
#define MAX_CFG_CMDS 10
static char *config_commands[MAX_CFG_CMDS];
static int num_config_commands = 0;

char *output_file = NULL;

XErrorHandler XfwmErrorHandler (Display *, XErrorEvent *);
XIOErrorHandler CatchFatal (Display *);
XErrorHandler CatchRedirectError (Display *, XErrorEvent *);
void newhandler (int sig);
void CreateCursors (void);
void ChildDied (int nonsense);
void SaveDesktopState (void);
void SetMWM_INFO (Window window);
void SetRCDefaults (void);
void StartupStuff (void);

XContext XfwmContext;		/* context for xfwm windows */
XContext MenuContext;		/* context for xfwm menus */

int JunkX = 0, JunkY = 0;
Window JunkRoot, JunkChild;	/* junk window */
unsigned int JunkWidth, JunkHeight, JunkBW, JunkDepth, JunkMask;

Boolean PPosOverride, Blackout = False;

char **g_argv;
int g_argc;

/* assorted gray bitmaps for decorative borders */
#define g_width 2
#define g_height 2
static char g_bits[] =
{0x02, 0x01};

#define l_g_width 4
#define l_g_height 2
static char l_g_bits[] =
{0x08, 0x02};

#define s_g_width 4
#define s_g_height 4

int ShapeEventBase, ShapeErrorBase;
Boolean ShapesSupported = False;

long isIconicState = 0;
extern XEvent Event;
Bool Restarting = False;
int fd_width, x_fd;
char *display_name = NULL;

/***********************************************************************
 *
 *  Procedure:
 *	main - start of xfwm
 *
 ***********************************************************************
 */
void
main (int argc, char **argv)
{
  XSetWindowAttributes attributes;	/* attributes for create windows */
  void InternUsefulAtoms (void);
  void InitVariables (void);
  int i;
  extern int x_fd;
  int len;
  char *display_string;
  char message[255];
  Bool single = False;
  Bool option_error = FALSE;

  g_argv = argv;
  g_argc = argc;

  for (i = 1; i < argc; i++)
    {
      if (mystrncasecmp (argv[i], "-s", 2) == 0)
	{
	  single = True;
	}
      else if (mystrncasecmp (argv[i], "-d", 2) == 0)
	{
	  if (++i >= argc)
	    usage ();
	  display_name = argv[i];
	}
      else if (mystrncasecmp (argv[i], "-f", 2) == 0)
	{
	  if (++i >= argc)
	    usage ();
	  if (num_config_commands < MAX_CFG_CMDS)
	    {
	      config_commands[num_config_commands] =
		(char *) malloc (6 + strlen (argv[i]));
	      strcpy (config_commands[num_config_commands], "Read ");
	      strcat (config_commands[num_config_commands], argv[i]);
	      num_config_commands++;
	    }
	  else
	    {
	      xfwm_msg (ERR, "main", "only %d -f and -cmd parms allowed!", MAX_CFG_CMDS);
	    }
	}
      else if (mystrncasecmp (argv[i], "-cmd", 4) == 0)
	{
	  if (++i >= argc)
	    usage ();
	  if (num_config_commands < MAX_CFG_CMDS)
	    {
	      config_commands[num_config_commands] = strdup (argv[i]);
	      num_config_commands++;
	    }
	  else
	    {
	      xfwm_msg (ERR, "main", "only %d -f and -cmd parms allowed!", MAX_CFG_CMDS);
	    }
	}
      else if (mystrncasecmp (argv[i], "-outfile", 8) == 0)
	{
	  if (++i >= argc)
	    usage ();
	  output_file = argv[i];
	}
      else if (mystrncasecmp (argv[i], "-h", 2) == 0)
	{
	  usage ();
	  exit (0);
	}
      else if (mystrncasecmp (argv[i], "-blackout", 9) == 0)
	{
	  Blackout = True;
	}
      else if (mystrncasecmp (argv[i], "-version", 8) == 0)
	{
	  xfwm_msg (INFO, "main", "Xfwm Version %s compiled on %s at %s\n",
		    VERSION, __DATE__, __TIME__);
	}
      else
	{
	  xfwm_msg (ERR, "main", "Unknown option:  `%s'\n", argv[i]);
	  option_error = TRUE;
	}
    }

  if (option_error)
    {
      usage ();
    }

  newhandler (SIGINT);
  newhandler (SIGHUP);
  newhandler (SIGQUIT);
  newhandler (SIGTERM);
  signal (SIGUSR1, Restart);
  signal (SIGPIPE, DeadPipe);

  ReapChildren ();

  if (!(dpy = XOpenDisplay (display_name)))
    {
      xfwm_msg (ERR, "main", "can't open display %s", XDisplayName (display_name));
      exit (1);
    }
  Scr.screen = DefaultScreen (dpy);
  Scr.NumberOfScreens = ScreenCount (dpy);

  master_pid = getpid ();

  if (!single)
    {
      int myscreen = 0;
      char *cp;

      strcpy (message, XDisplayString (dpy));

      for (i = 0; i < Scr.NumberOfScreens; i++)
	{
	  if (i != Scr.screen && fork () == 0)
	    {
	      myscreen = i;

	      /*
	       * Truncate the string 'whatever:n.n' to 'whatever:n',
	       * and then append the screen number.
	       */
	      cp = strchr (message, ':');
	      if (cp != NULL)
		{
		  cp = strchr (cp, '.');
		  if (cp != NULL)
		    *cp = '\0';	/* truncate at display part */
		}
	      sprintf (message + strlen (message), ".%d", myscreen);
	      dpy = XOpenDisplay (message);
	      Scr.screen = myscreen;
	      Scr.NumberOfScreens = ScreenCount (dpy);

	      break;
	    }
	}
    }

  x_fd = XConnectionNumber (dpy);
  fd_width = GetFdWidth ();

  if (fcntl (x_fd, F_SETFD, 1) == -1)
    {
      xfwm_msg (ERR, "main", "close-on-exec failed");
      exit (1);
    }

  /*  Add a DISPLAY entry to the environment, incase we were started
   * with xfwm -display term:0.0
   */
  len = strlen (XDisplayString (dpy));
  display_string = safemalloc (len + 10);
  sprintf (display_string, "DISPLAY=%s", XDisplayString (dpy));
  putenv (display_string);
  /* Add a HOSTDISPLAY environment variable, which is the same as
   * DISPLAY, unless display = :0.0 or unix:0.0, in which case the full
   * host name will be used for ease in networking . */
  /* Note: Can't free the rdisplay_string after putenv, because it
   * becomes part of the environment! */
  if (strncmp (display_string, "DISPLAY=:", 9) == 0)
    {
      char client[MAXHOSTNAME], *rdisplay_string;
      mygethostname (client, MAXHOSTNAME);
      rdisplay_string = safemalloc (len + 14 + strlen (client));
      sprintf (rdisplay_string, "HOSTDISPLAY=%s:%s", client, &display_string[9]);
      putenv (rdisplay_string);
    }
  else if (strncmp (display_string, "DISPLAY=unix:", 13) == 0)
    {
      char client[MAXHOSTNAME], *rdisplay_string;
      mygethostname (client, MAXHOSTNAME);
      rdisplay_string = safemalloc (len + 14 + strlen (client));
      sprintf (rdisplay_string, "HOSTDISPLAY=%s:%s", client,
	       &display_string[13]);
      putenv (rdisplay_string);
    }
  else
    {
      char *rdisplay_string;
      rdisplay_string = safemalloc (len + 14);
      sprintf (rdisplay_string, "HOSTDISPLAY=%s", XDisplayString (dpy));
      putenv (rdisplay_string);
    }

  Scr.Root = RootWindow (dpy, Scr.screen);
  if (Scr.Root == None)
    {
      xfwm_msg (ERR, "main", "Screen %d is not a valid screen", (char *) Scr.screen);
      exit (1);
    }

  ShapesSupported = XShapeQueryExtension (dpy, &ShapeEventBase, &ShapeErrorBase);

  InternUsefulAtoms ();

  /* Make sure property priority colors is empty */
  XChangeProperty (dpy, Scr.Root, _XA_MIT_PRIORITY_COLORS,
		   XA_CARDINAL, 32, PropModeReplace, NULL, 0);


  XSetErrorHandler ((XErrorHandler) CatchRedirectError);
  XSetIOErrorHandler ((XIOErrorHandler) CatchFatal);
  XSelectInput (dpy, Scr.Root,
		LeaveWindowMask | EnterWindowMask | PropertyChangeMask |
		SubstructureRedirectMask | KeyPressMask |
		SubstructureNotifyMask |
		ButtonPressMask | ButtonReleaseMask);
  XSync (dpy, 0);

  XSetErrorHandler ((XErrorHandler) XfwmErrorHandler);

  BlackoutScreen ();		/* if they want to hide the capture/startup */

  CreateCursors ();
  InitVariables ();
  InitEventHandlerJumpTable ();
  initModules ();

  Scr.gray_bitmap =
    XCreateBitmapFromData (dpy, Scr.Root, g_bits, g_width, g_height);

  SetRCDefaults ();

  if (num_config_commands > 0)
    {
      int i;
      for (i = 0; i < num_config_commands; i++)
	{
	  ExecuteFunction (config_commands[i], NULL, &Event, C_ROOT, -1);
	  free (config_commands[i]);
	}
    }
  else
    {
      ExecuteFunction (default_config_command, NULL, &Event, C_ROOT, -1);
    }

  if (Scr.d_depth < 2)
    {
      Scr.gray_pixmap =
	XCreatePixmapFromBitmapData (dpy, Scr.Root, g_bits, g_width, g_height,
				   Scr.MenuColors.fore, Scr.MenuColors.back,
				     Scr.d_depth);
      Scr.light_gray_pixmap =
	XCreatePixmapFromBitmapData (dpy, Scr.Root, l_g_bits, l_g_width, l_g_height,
				   Scr.MenuColors.fore, Scr.MenuColors.back,
				     Scr.d_depth);
    }

  /* create a window which will accept the keyboard focus when no other 
   * windows have it */
  attributes.event_mask = KeyPressMask | FocusChangeMask;
  attributes.override_redirect = True;
  Scr.NoFocusWin = XCreateWindow (dpy, Scr.Root, -10, -10, 10, 10, 0, 0,
				  InputOnly, CopyFromParent,
				  CWEventMask | CWOverrideRedirect,
				  &attributes);
  XMapWindow (dpy, Scr.NoFocusWin);

  SetMWM_INFO (Scr.NoFocusWin);

  XSetInputFocus (dpy, Scr.NoFocusWin, RevertToParent, CurrentTime);

  XSync (dpy, 0);
  initPanFrames ();
  MyXGrabServer (dpy);

  checkPanFrames ();
  MyXUngrabServer (dpy);
  UnBlackoutScreen ();		/* if we need to remove blackout window */
  HandleEvents ();
  return;
}


/*
 * ** StartupStuff
 * **
 * ** Does initial window captures and runs init/restart function
 */
void
StartupStuff (void)
{
  MenuRoot *mr;

  CaptureAllWindows ();
  MakeMenus ();

  if (Restarting)
    {
      mr = FindPopup ("RestartFunction");
      if (mr != NULL)
	ExecuteFunction ("Function RestartFunction", NULL, &Event, C_ROOT, -1);
    }
  else
    {
      mr = FindPopup ("InitFunction");
      if (mr != NULL)
	ExecuteFunction ("Function InitFunction", NULL, &Event, C_ROOT, -1);
    }
}				/* StartupStuff */


/***********************************************************************
 *
 *  Procedure:
 *      CaptureAllWindows
 *
 *   Decorates all windows at start-up and during recaptures
 *
 ***********************************************************************/

void
CaptureAllWindows (void)
{
  int i, j;
  unsigned int nchildren;
  Window root, parent, *children;
  XfwmWindow *tmp, *next;	/* temp xfwm window structure */
  Window w;
  unsigned long data[1];
  unsigned char *prop;
  Atom atype;
  int aformat;
  unsigned long nitems, bytes_remain;

  MyXGrabServer (dpy);

  if (!XQueryTree (dpy, Scr.Root, &root, &parent, &children, &nchildren))
    {
      MyXUngrabServer (dpy);
      return;
    }

  PPosOverride = True;

  if (!(Scr.flags & WindowsCaptured))	/* initial capture? */
    {
      /*
       * ** weed out icon windows
       */
      for (i = 0; i < nchildren; i++)
	{
	  if (children[i])
	    {
	      XWMHints *wmhintsp = XGetWMHints (dpy, children[i]);
	      if (wmhintsp)
		{
		  if (wmhintsp->flags & IconWindowHint)
		    {
		      for (j = 0; j < nchildren; j++)
			{
			  if (children[j] == wmhintsp->icon_window)
			    {
			      children[j] = None;
			      break;
			    }
			}
		    }
		  XFree ((char *) wmhintsp);
		}
	    }
	}
      /*
       * ** map all of the non-override, non-icon windows
       */
      for (i = 0; i < nchildren; i++)
	{
	  if (children[i] && MappedNotOverride (children[i]))
	    {
	      XUnmapWindow (dpy, children[i]);
	      Event.xmaprequest.window = children[i];
	      HandleMapRequestKeepRaised (BlackoutWin);
	    }
	}
      Scr.flags |= WindowsCaptured;
    }
  else
    /* must be recapture */
    {
      /* reborder all windows */
      tmp = Scr.XfwmRoot.next;
      for (i = 0; i < nchildren; i++)
	{
	  if (XFindContext (dpy, children[i], XfwmContext,
			    (caddr_t *) & tmp) != XCNOENT)
	    {
	      isIconicState = DontCareState;
	      if (XGetWindowProperty (dpy, tmp->w, _XA_WM_STATE, 0L, 3L, False,
				      _XA_WM_STATE,
			 &atype, &aformat, &nitems, &bytes_remain, &prop) ==
		  Success)
		{
		  if (prop != NULL)
		    {
		      isIconicState = *(long *) prop;
		      XFree (prop);
		    }
		}
	      next = tmp->next;
	      data[0] = (unsigned long) tmp->Desk;
	      XChangeProperty (dpy, tmp->w, _XA_WM_DESKTOP, _XA_WM_DESKTOP, 32,
			       PropModeReplace, (unsigned char *) data, 1);

	      XSelectInput (dpy, tmp->w, 0);
	      w = tmp->w;
	      XUnmapWindow (dpy, tmp->frame);
	      XUnmapWindow (dpy, w);
	      RestoreWithdrawnLocation (tmp, True);
	      Destroy (tmp);
	      Event.xmaprequest.window = w;
	      HandleMapRequestKeepRaised (BlackoutWin);
	      tmp = next;
	    }
	}
    }

  isIconicState = DontCareState;

  if (nchildren > 0)
    XFree ((char *) children);

  /* after the windows already on the screen are in place,
   * don't use PPosition */
  PPosOverride = False;
  KeepOnTop ();
  MyXUngrabServer (dpy);
  XSync (dpy, 0);		/* should we do this on initial capture? */
}

/*
 * ** SetRCDefaults
 * **
 * ** Sets some initial style values & such
 */
void
SetRCDefaults ()
{
  /* set up default colors, fonts, etc */
  char *defaults[] =
  {
    "XORValue 65535",
    "ModulePath $PATH",
    "ActiveColor white #aeb2c3",
    "InactiveColor black #aeb2c3",
    "TitleStyle Active Solid #c7686b",
    "TitleStyle Inactive Solid #606060",
    "CursorColor red white",
    "MenuColor  black #aeb2c3 white #8091a4",
    "IconFont fixed",
    "WindowFont fixed",
    "MenuFont fixed",
    "DeskTopSize 1 1",
    "Style \"*\" Title, BorderWidth 5",
    "Style \"*\" RandomPlacement, SmartPlacement",
    "Style \"XFce*\" Sticky",
    "Style \"XFbd*\" Sticky",
    "Style \"Popup*\" Sticky",
    "AddToMenu __builtin_menu__ \"Builtin Menu\" Title",
    "+ \"Terminal\" Exec xterm",
    "+ \"\" Nop",
    "+ \"Exit XFWM\" Quit",
    "AddToFunc WindowListFunc \"I\" WindowId $0 Iconify -1",
    "+ \"I\" WindowId $0 Focus",
    "+ \"I\" WindowId $0 Raise",
    "+ \"I\" WindowId $0 WarpToWindow 5p 5p",
    "AddToFunc \"InitFunction\"",
    "+ \"I\" Module xfce",
    "+ \"I\" Desk 0 4",
    "+ \"I\" Exec xfbd -d",
    "AddToFunc \"RestartFunction\"",
    "+ \"I\" Module xfce",
    "+ \"I\" Desk 0 4",
    "+ \"I\" Exec xfbd -d",
    "AddToFunc \"__Iconfunc__\"",
    "+ \"M\" Move",
    "+ \"D\" Iconify",
    "AddToFunc \"__Raisemove__\"",
    "+ \"C\" Raise",
    "+ \"M\" Raise",
    "+ \"M\" Move",
    "AddToFunc \"__Raiseresize__\"",
    "+ \"C\" Raise",
    "+ \"M\" Raise",
    "+ \"M\" Resize",
    "Mouse 0 T   A Function __Raisemove__",
    "Mouse 0 1   A Close",
    "Mouse 0 2   A Maximize",
    "Mouse 0 3   A Stick",
    "Mouse 0 4   A Iconify",
    "Mouse 0 FS  A __Raiseresize__",
    "Mouse 1 R   A Popup __builtin_menu__",
    "Mouse 0 I   A Function __Iconfunc__",
    "Mouse 3 R   A WindowList",
    NULL
  };
  int i = 0;

  while (defaults[i])
    {
      ExecuteFunction (defaults[i], NULL, &Event, C_ROOT, -1);
      i++;
    }
}				/* SetRCDefaults */

/***********************************************************************
 *
 *  Procedure:
 *	MappedNotOverride - checks to see if we should really
 *		put a xfwm frame on the window
 *
 *  Returned Value:
 *	TRUE	- go ahead and frame the window
 *	FALSE	- don't frame the window
 *
 *  Inputs:
 *	w	- the window to check
 *
 ***********************************************************************/

int
MappedNotOverride (Window w)
{
  XWindowAttributes wa;
  Atom atype;
  int aformat;
  unsigned long nitems, bytes_remain;
  unsigned char *prop;

  isIconicState = DontCareState;

  if ((w == Scr.NoFocusWin) || (!XGetWindowAttributes (dpy, w, &wa)))
    return False;

  if (XGetWindowProperty (dpy, w, _XA_WM_STATE, 0L, 3L, False, _XA_WM_STATE,
		&atype, &aformat, &nitems, &bytes_remain, &prop) == Success)
    {
      if (prop != NULL)
	{
	  isIconicState = *(long *) prop;
	  XFree (prop);
	}
    }
  if (wa.override_redirect == True)
    {
      XSelectInput (dpy, w, FocusChangeMask);
    }
  return (((isIconicState == IconicState) || (wa.map_state != IsUnmapped)) &&
	  (wa.override_redirect != True));
}


/***********************************************************************
 *
 *  Procedure:
 *	InternUsefulAtoms:
 *            Dont really know what it does
 *
 ***********************************************************************
 */
Atom _XA_MIT_PRIORITY_COLORS;
Atom _XA_WM_CHANGE_STATE;
Atom _XA_WM_STATE;
Atom _XA_WM_COLORMAP_WINDOWS;
extern Atom _XA_WM_PROTOCOLS;
Atom _XA_WM_TAKE_FOCUS;
Atom _XA_WM_DELETE_WINDOW;
Atom _XA_WM_DESKTOP;
Atom _XA_MwmAtom;
Atom _XA_MOTIF_WM;

Atom _XA_OL_WIN_ATTR;
Atom _XA_OL_WT_BASE;
Atom _XA_OL_WT_CMD;
Atom _XA_OL_WT_HELP;
Atom _XA_OL_WT_NOTICE;
Atom _XA_OL_WT_OTHER;
Atom _XA_OL_DECOR_ADD;
Atom _XA_OL_DECOR_DEL;
Atom _XA_OL_DECOR_CLOSE;
Atom _XA_OL_DECOR_RESIZE;
Atom _XA_OL_DECOR_HEADER;
Atom _XA_OL_DECOR_ICON_NAME;

void
InternUsefulAtoms (void)
{
  /* 
   * Create priority colors if necessary.
   */
  _XA_MIT_PRIORITY_COLORS = XInternAtom (dpy, "_MIT_PRIORITY_COLORS", False);
  _XA_WM_CHANGE_STATE = XInternAtom (dpy, "WM_CHANGE_STATE", False);
  _XA_WM_STATE = XInternAtom (dpy, "WM_STATE", False);
  _XA_WM_COLORMAP_WINDOWS = XInternAtom (dpy, "WM_COLORMAP_WINDOWS", False);
  _XA_WM_PROTOCOLS = XInternAtom (dpy, "WM_PROTOCOLS", False);
  _XA_WM_TAKE_FOCUS = XInternAtom (dpy, "WM_TAKE_FOCUS", False);
  _XA_WM_DELETE_WINDOW = XInternAtom (dpy, "WM_DELETE_WINDOW", False);
  _XA_WM_DESKTOP = XInternAtom (dpy, "WM_DESKTOP", False);
  _XA_MwmAtom = XInternAtom (dpy, "_MOTIF_WM_HINTS", False);
  _XA_MOTIF_WM = XInternAtom (dpy, "_MOTIF_WM_INFO", False);

  _XA_OL_WIN_ATTR = XInternAtom (dpy, "_OL_WIN_ATTR", False);
  _XA_OL_WT_BASE = XInternAtom (dpy, "_OL_WT_BASE", False);
  _XA_OL_WT_CMD = XInternAtom (dpy, "_OL_WT_CMD", False);
  _XA_OL_WT_HELP = XInternAtom (dpy, "_OL_WT_HELP", False);
  _XA_OL_WT_NOTICE = XInternAtom (dpy, "_OL_WT_NOTICE", False);
  _XA_OL_WT_OTHER = XInternAtom (dpy, "_OL_WT_OTHER", False);
  _XA_OL_DECOR_ADD = XInternAtom (dpy, "_OL_DECOR_ADD", False);
  _XA_OL_DECOR_DEL = XInternAtom (dpy, "_OL_DECOR_DEL", False);
  _XA_OL_DECOR_CLOSE = XInternAtom (dpy, "_OL_DECOR_CLOSE", False);
  _XA_OL_DECOR_RESIZE = XInternAtom (dpy, "_OL_DECOR_RESIZE", False);
  _XA_OL_DECOR_HEADER = XInternAtom (dpy, "_OL_DECOR_HEADER", False);
  _XA_OL_DECOR_ICON_NAME = XInternAtom (dpy, "_OL_DECOR_ICON_NAME", False);
  return;
}

/***********************************************************************
 *
 *  Procedure:
 *	newhandler: Installs new signal handler
 *
 ************************************************************************/
void
newhandler (int sig)
{
  if (signal (sig, SIG_IGN) != SIG_IGN)
    signal (sig, SigDone);
}


/*************************************************************************
 * Restart on a signal
 ************************************************************************/
void
Restart (int nonsense)
{
  Done (1, *g_argv);
  SIGNAL_RETURN;
}

/***********************************************************************
 *
 *  Procedure:
 *	CreateCursors - Loads xfwm cursors
 *
 ***********************************************************************
 */
void
CreateCursors (void)
{
  /* define cursors */
  Scr.XfwmCursors[POSITION] = XCreateFontCursor (dpy, XC_top_left_corner);
  Scr.XfwmCursors[DEFAULT] = XCreateFontCursor (dpy, XFCE_CURS);
  Scr.XfwmCursors[SYS] = XCreateFontCursor (dpy, MENU_CURS);
  Scr.XfwmCursors[TITLE_CURSOR] = XCreateFontCursor (dpy, XFCE_CURS);
  Scr.XfwmCursors[MOVE] = XCreateFontCursor (dpy, XC_fleur);
  Scr.XfwmCursors[MENU] = XCreateFontCursor (dpy, MENU_CURS);
  Scr.XfwmCursors[WAIT] = XCreateFontCursor (dpy, XC_watch);
  Scr.XfwmCursors[SELECT] = XCreateFontCursor (dpy, XC_plus);
  Scr.XfwmCursors[DESTROY] = XCreateFontCursor (dpy, XC_plus);
  Scr.XfwmCursors[LEFT] = XCreateFontCursor (dpy, XC_left_side);
  Scr.XfwmCursors[RIGHT] = XCreateFontCursor (dpy, XC_right_side);
  Scr.XfwmCursors[TOP] = XCreateFontCursor (dpy, XC_top_side);
  Scr.XfwmCursors[BOTTOM] = XCreateFontCursor (dpy, XC_bottom_side);
  Scr.XfwmCursors[TOP_LEFT] = XCreateFontCursor (dpy, XC_top_left_corner);
  Scr.XfwmCursors[TOP_RIGHT] = XCreateFontCursor (dpy, XC_top_right_corner);
  Scr.XfwmCursors[BOTTOM_LEFT] = XCreateFontCursor (dpy, XC_bottom_left_corner);
  Scr.XfwmCursors[BOTTOM_RIGHT] = XCreateFontCursor (dpy, XC_bottom_right_corner);
}

/***********************************************************************
 *
 *  LoadDefaultLeftButton -- loads default left button # into 
 *		assumes associated button memory is already free
 * 
 ************************************************************************/
void
LoadDefaultLeftButton (ButtonFace * bf, int i)
{
  bf->style = VectorButton;
  switch (i % 2)
    {
    case 0:
      bf->vector.x[0] = 20;
      bf->vector.y[0] = 40;
      bf->vector.line_style[0] = 0;
      bf->vector.x[1] = 80;
      bf->vector.y[1] = 40;
      bf->vector.line_style[1] = 0;
      bf->vector.x[2] = 80;
      bf->vector.y[2] = 60;
      bf->vector.line_style[2] = 1;
      bf->vector.x[3] = 20;
      bf->vector.y[3] = 60;
      bf->vector.line_style[3] = 1;
      bf->vector.x[4] = 20;
      bf->vector.y[4] = 40;
      bf->vector.line_style[4] = 0;
      bf->vector.num = 5;
      break;
    case 1:
      bf->vector.x[0] = 30;
      bf->vector.y[0] = 20;
      bf->vector.line_style[0] = 1;
      bf->vector.x[1] = 70;
      bf->vector.y[1] = 20;
      bf->vector.line_style[1] = 1;
      bf->vector.x[2] = 70;
      bf->vector.y[2] = 40;
      bf->vector.line_style[2] = 0;
      bf->vector.x[3] = 55;
      bf->vector.y[3] = 40;
      bf->vector.line_style[3] = 0;
      bf->vector.x[4] = 55;
      bf->vector.y[4] = 80;
      bf->vector.line_style[4] = 0;
      bf->vector.x[5] = 45;
      bf->vector.y[5] = 80;
      bf->vector.line_style[5] = 0;
      bf->vector.x[6] = 45;
      bf->vector.y[6] = 40;
      bf->vector.line_style[6] = 1;
      bf->vector.x[7] = 30;
      bf->vector.y[7] = 40;
      bf->vector.line_style[7] = 0;
      bf->vector.x[8] = 30;
      bf->vector.y[8] = 20;
      bf->vector.line_style[8] = 1;
      bf->vector.num = 9;
      break;
    }
}

/***********************************************************************
 *
 *  LoadDefaultRightButton -- loads default left button # into
 *		assumes associated button memory is already free
 * 
 ************************************************************************/
void
LoadDefaultRightButton (ButtonFace * bf, int i)
{
  bf->style = VectorButton;
  switch (i % 2)
    {
    case 0:
      bf->vector.x[0] = 20;
      bf->vector.y[0] = 20;
      bf->vector.line_style[0] = 1;
      bf->vector.x[1] = 80;
      bf->vector.y[1] = 20;
      bf->vector.line_style[1] = 1;
      bf->vector.x[2] = 80;
      bf->vector.y[2] = 80;
      bf->vector.line_style[2] = 0;
      bf->vector.x[3] = 20;
      bf->vector.y[3] = 80;
      bf->vector.line_style[3] = 0;
      bf->vector.x[4] = 20;
      bf->vector.y[4] = 20;
      bf->vector.line_style[4] = 1;
      bf->vector.num = 5;
      break;
    case 1:
      bf->vector.x[0] = 39;
      bf->vector.y[0] = 39;
      bf->vector.line_style[0] = 0;
      bf->vector.x[1] = 61;
      bf->vector.y[1] = 39;
      bf->vector.line_style[1] = 0;
      bf->vector.x[2] = 61;
      bf->vector.y[2] = 61;
      bf->vector.line_style[2] = 1;
      bf->vector.x[3] = 39;
      bf->vector.y[3] = 61;
      bf->vector.line_style[3] = 1;
      bf->vector.x[4] = 39;
      bf->vector.y[4] = 39;
      bf->vector.line_style[4] = 0;
      bf->vector.num = 5;
      break;
    }
}

/***********************************************************************
 *
 *  LoadDefaultButton -- loads default button # into button structure
 *		assumes associated button memory is already free
 * 
 ************************************************************************/
void
LoadDefaultButton (ButtonFace * bf, int i)
{
  int n = i / 2;
  if ((n * 2) == i)
    {
      if (--n < 0)
	n = 2;
      LoadDefaultRightButton (bf, n);
    }
  else
    LoadDefaultLeftButton (bf, n);
}

extern void FreeButtonFace (Display * dpy, ButtonFace * bf);

/***********************************************************************
 *
 *  ResetAllButtons -- resets all buttons to defaults
 *                 destroys existing buttons
 * 
 ************************************************************************/
void
ResetAllButtons (XfwmDecor * fl)
{
  int i = 0;
  for (; i < 2; ++i)
    {
      int j;

      FreeButtonFace (dpy, &fl->left_buttons[i].state[0]);
      FreeButtonFace (dpy, &fl->right_buttons[i].state[0]);

      LoadDefaultLeftButton (&fl->left_buttons[i].state[0], i);
      LoadDefaultRightButton (&fl->right_buttons[i].state[0], i);

      for (j = 1; j < MaxButtonState; ++j)
	{
	  FreeButtonFace (dpy, &fl->left_buttons[i].state[j]);
	  FreeButtonFace (dpy, &fl->right_buttons[i].state[j]);

	  fl->left_buttons[i].state[j] = fl->left_buttons[i].state[0];
	  fl->right_buttons[i].state[j] = fl->right_buttons[i].state[0];
	}
    }
  fl->left_buttons[0].flags |= MWMDecorMenu;
  fl->left_buttons[1].flags |= MWMDecorSticky;
  fl->right_buttons[1].flags |= MWMDecorMinimize;
  fl->right_buttons[0].flags |= MWMDecorMaximize;
}

/***********************************************************************
 *
 *  DestroyXfwmDecor -- frees all memory assocated with an XfwmDecor
 *	structure, but does not free the XfwmDecor itself
 * 
 ************************************************************************/
void
DestroyXfwmDecor (XfwmDecor * fl)
{
  int i;
  /* reset to default button set (frees allocated mem) */
  ResetAllButtons (fl);
  for (i = 0; i < 3; ++i)
    {
      int j = 0;
      for (; j < MaxButtonState; ++j)
	FreeButtonFace (dpy, &fl->titlebar.state[i]);
    }
  FreeButtonFace (dpy, &fl->BorderStyle.active);
  FreeButtonFace (dpy, &fl->BorderStyle.inactive);
  if (fl->LoReliefGC != NULL)
    {
      XFreeGC (dpy, fl->LoReliefGC);
      fl->LoReliefGC = NULL;
    }
  if (fl->LoShadowGC != NULL)
    {
      XFreeGC (dpy, fl->LoShadowGC);
      fl->LoShadowGC = NULL;
    }
  if (fl->HiReliefGC != NULL)
    {
      XFreeGC (dpy, fl->HiReliefGC);
      fl->HiReliefGC = NULL;
    }
  if (fl->HiShadowGC != NULL)
    {
      XFreeGC (dpy, fl->HiShadowGC);
      fl->HiShadowGC = NULL;
    }
}

/***********************************************************************
 *
 *  InitXfwmDecor -- initializes an XfwmDecor structure to defaults
 * 
 ************************************************************************/
void
InitXfwmDecor (XfwmDecor * fl)
{
  int i;
  ButtonFace tmpbf;

  fl->HiReliefGC = NULL;
  fl->HiShadowGC = NULL;
  fl->LoReliefGC = NULL;
  fl->LoShadowGC = NULL;


  /* initialize title-bar button styles */
  tmpbf.style = SimpleButton;
  tmpbf.next = NULL;
  for (i = 0; i < 2; ++i)
    {
      int j = 0;
      for (; j < MaxButtonState; ++j)
	{
	  fl->left_buttons[i].state[j] =
	    fl->right_buttons[i].state[j] = tmpbf;
	}
    }

  /* reset to default button set */
  ResetAllButtons (fl);

  /* initialize title-bar styles */
  fl->titlebar.flags = 0;

  for (i = 0; i < MaxButtonState; ++i)
    {
      fl->titlebar.state[i].style = SimpleButton;
      fl->titlebar.state[i].next = NULL;
    }

  /* initialize border texture styles */
  fl->BorderStyle.active.style = SimpleButton;
  fl->BorderStyle.inactive.style = SimpleButton;
  fl->BorderStyle.active.next = NULL;
  fl->BorderStyle.inactive.next = NULL;
}

/***********************************************************************
 *
 *  Procedure:
 *	InitVariables - initialize xfwm variables
 *
 ************************************************************************/
void
InitVariables (void)
{
  XfwmContext = XUniqueContext ();
  MenuContext = XUniqueContext ();

  /* initialize some lists */
  Scr.AllBindings = NULL;
  Scr.AllMenus = NULL;
  Scr.TheList = NULL;

  Scr.DefaultIcon = NULL;


  /* create graphics contexts */
  CreateGCs ();

  Scr.d_depth = DefaultDepth (dpy, Scr.screen);
  Scr.XfwmRoot.w = Scr.Root;
  Scr.XfwmRoot.next = 0;
  XGetWindowAttributes (dpy, Scr.Root, &(Scr.XfwmRoot.attr));
  Scr.root_pushes = 0;
  Scr.pushed_window = &Scr.XfwmRoot;
  Scr.XfwmRoot.number_cmap_windows = 0;


  Scr.MyDisplayWidth = DisplayWidth (dpy, Scr.screen);
  Scr.MyDisplayHeight = DisplayHeight (dpy, Scr.screen);

  Scr.NoBoundaryWidth = 1;
  Scr.BoundaryWidth = BOUNDARY_WIDTH;
  Scr.CornerWidth = CORNER_WIDTH;
  Scr.Hilite = NULL;
  Scr.Focus = NULL;
  Scr.Ungrabbed = NULL;

  Scr.StdFont.font = NULL;

  Scr.VxMax = 2 * Scr.MyDisplayWidth;
  Scr.VyMax = 2 * Scr.MyDisplayHeight;
  Scr.Vx = Scr.Vy = 0;
  {
    Atom atype;
    int aformat;
    unsigned long nitems, bytes_remain;
    unsigned char *prop;

    Scr.CurrentDesk = 0;
    if ((XGetWindowProperty (dpy, Scr.Root, _XA_WM_DESKTOP, 0L, 1L, True,
			     _XA_WM_DESKTOP, &atype, &aformat, &nitems,
			     &bytes_remain, &prop)) == Success)
      {
	if (prop != NULL)
	  {
	    Restarting = True;
	    Scr.CurrentDesk = *(unsigned long *) prop;
	  }
      }
  }

  Scr.EdgeScrollX = Scr.EdgeScrollY = 0;
  Scr.ScrollResistance = Scr.MoveResistance = 0;
  Scr.OpaqueSize = 100;
  Scr.ClickTime = 150;
  Scr.ColormapFocus = COLORMAP_FOLLOWS_MOUSE;

  /* set major operating modes */
  Scr.NumBoxes = 0;

  Scr.randomx = Scr.randomy = 0;
  Scr.buttons2grab = 7;

  InitXfwmDecor (&Scr.DefaultDecor);

  Scr.SmartPlacementIsClever = False;
  Scr.ClickToFocusPassesClick = True;
  Scr.ClickToFocusRaises = True;
  Scr.MouseFocusClickRaises = False;

  /* Not the right place for this, should only be called once somewhere .. */
  InitPictureCMap (dpy, Scr.Root);

  return;
}

/***********************************************************************
 *
 *  Procedure:
 *	Reborder - Removes xfwm border windows
 *
 ************************************************************************/
void
Reborder (void)
{
  XfwmWindow *tmp;		/* temp xfwm window structure */

  /* put a border back around all windows */
  MyXGrabServer (dpy);

  InstallWindowColormaps (&Scr.XfwmRoot);	/* force reinstall */
  for (tmp = Scr.XfwmRoot.next; tmp != NULL; tmp = tmp->next)
    {
      RestoreWithdrawnLocation (tmp, True);
      XUnmapWindow (dpy, tmp->frame);
      XDestroyWindow (dpy, tmp->frame);
    }

  MyXUngrabServer (dpy);
  XSetInputFocus (dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
  XSync (dpy, 0);

}

/***********************************************************************
 *
 *  Procedure:
 *	Done - cleanup and exit xfwm
 *
 ***********************************************************************
 */
void
SigDone (int nonsense)
{
  Done (0, NULL);
  SIGNAL_RETURN;
}

void
Done (int restart, char *command)
{
  MenuRoot *mr;

  MoveViewport (0, 0, False);

  mr = FindPopup ("ExitFunction");
  if (mr != NULL)
    ExecuteFunction ("Function ExitFunction", NULL, &Event, C_ROOT, -1);

  /* Close all my pipes */
  ClosePipes ();

  Reborder ();

  if (restart)
    {
      SaveDesktopState ();	/* I wonder why ... */

      /* Really make sure that the connection is closed and cleared! */
      XSelectInput (dpy, Scr.Root, 0);
      XSync (dpy, 0);
      XCloseDisplay (dpy);

      {
	char *my_argv[10];
	int i, done, j;

	i = 0;
	j = 0;
	done = 0;
	while ((g_argv[j] != NULL) && (i < 8))
	  {
	    if (strcmp (g_argv[j], "-s") != 0)
	      {
		my_argv[i] = g_argv[j];
		i++;
		j++;
	      }
	    else
	      j++;
	  }
	if (strstr (command, "xfwm") != NULL)
	  my_argv[i++] = "-s";
	while (i < 10)
	  my_argv[i++] = NULL;

	/* really need to destroy all windows, explicitly,
	 * not sleep, but this is adequate for now */
	sleep (1);
	ReapChildren ();
	execvp (command, my_argv);
      }
      xfwm_msg (ERR, "Done", "Call of '%s' failed!!!! (restarting '%s' instead)",
		command,
		g_argv[0]);
      execvp (g_argv[0], g_argv);	/* that _should_ work */
      xfwm_msg (ERR, "Done", "Call of '%s' failed!!!!", g_argv[0]);
    }
  else
    {
      XCloseDisplay (dpy);
      exit (0);
    }
}

XErrorHandler
CatchRedirectError (Display * dpy, XErrorEvent * event)
{
  xfwm_msg (ERR, "CatchRedirectError", "another WM is running");
  exit (1);
}

/***********************************************************************
 *
 *  Procedure:
 *	CatchFatal - Shuts down if the server connection is lost
 *
 ************************************************************************/
XIOErrorHandler
CatchFatal (Display * dpy)
{
  /* No action is taken because usually this action is caused by someone
   * using "xlogout" to be able to switch between multiple window managers
   */
  ClosePipes ();
  exit (1);
}

/***********************************************************************
 *
 *  Procedure:
 *	XfwmErrorHandler - displays info on internal errors
 *
 ************************************************************************/
XErrorHandler
XfwmErrorHandler (Display * dpy, XErrorEvent * event)
{
  extern int last_event_type;

  /* some errors are acceptable, mostly they're caused by 
   * trying to update a lost  window */
  if ((event->error_code == BadWindow) || (event->request_code == X_GetGeometry) ||
      (event->error_code == BadDrawable) || (event->request_code == X_SetInputFocus) ||
      (event->request_code == X_GrabButton) ||
      (event->request_code == X_ChangeWindowAttributes) ||
      (event->request_code == X_InstallColormap))
    return 0;


  xfwm_msg (ERR, "XfwmErrorHandler", "*** internal error ***");
  xfwm_msg (ERR, "XfwmErrorHandler", "Request %d, Error %d, EventType: %d",
	    event->request_code,
	    event->error_code,
	    last_event_type);
  return 0;
}

void
usage (void)
{
  fprintf (stderr, "\nXfwm Version %s Usage:\n\n", VERSION);
  fprintf (stderr, "  %s [-d dpy] [-f config_cmd] [-s] [-blackout] [-version] [-h]\n\n", g_argv[0]);
}

/****************************************************************************
 *
 * Save Desktop State
 *
 ****************************************************************************/
void
SaveDesktopState ()
{
  XfwmWindow *t;
  unsigned long data[1];

  for (t = Scr.XfwmRoot.next; t != NULL; t = t->next)
    {
      data[0] = (unsigned long) t->Desk;
      XChangeProperty (dpy, t->w, _XA_WM_DESKTOP, _XA_WM_DESKTOP, 32,
		       PropModeReplace, (unsigned char *) data, 1);
    }

  data[0] = (unsigned long) Scr.CurrentDesk;
  XChangeProperty (dpy, Scr.Root, _XA_WM_DESKTOP, _XA_WM_DESKTOP, 32,
		   PropModeReplace, (unsigned char *) data, 1);

  XSync (dpy, 0);
}


void
SetMWM_INFO (Window window)
{
}

void
BlackoutScreen ()
{
  XSetWindowAttributes attributes;
  unsigned long valuemask;

  if (Blackout && (BlackoutWin == None))
    {
      /* blackout screen */
      attributes.border_pixel = BlackPixel (dpy, Scr.screen);
      attributes.background_pixel = BlackPixel (dpy, Scr.screen);
      attributes.bit_gravity = NorthWestGravity;
      attributes.override_redirect = True;	/* is override redirect needed? */
      valuemask = CWBorderPixel |
	CWBackPixel |
	CWBitGravity |
	CWOverrideRedirect;
      BlackoutWin = XCreateWindow (dpy, Scr.Root, 0, 0,
				   DisplayWidth (dpy, Scr.screen),
				   DisplayHeight (dpy, Scr.screen), 0,
				   CopyFromParent,
				   CopyFromParent, CopyFromParent,
				   valuemask, &attributes);
      XMapWindow (dpy, BlackoutWin);
      XSync (dpy, 0);
    }
}				/* BlackoutScreen */

void
UnBlackoutScreen ()
{
  if (Blackout && (BlackoutWin != None))
    {
      XDestroyWindow (dpy, BlackoutWin);	/* unblacken the screen */
      XSync (dpy, 0);
      BlackoutWin = None;
    }
}				/* UnBlackoutScreen */
