/*

   ORIGINAL FILE NAME : main.c

   ********************************************************************
   *                                                                  *
   *           X F C E  - Written by O. Fourdan (c) 1997              *
   *                                                                  *
   *           This software is absolutely free of charge             *
   *                                                                  *
   ********************************************************************

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR (O. FOURDAN) BE 
   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include "forms.h"
#include "XFCE.h"
#include "startup.h"
#include "pulldown.h"
#include "addicon.h"
#include "info.h"
#include "screen.h"
#include "command.h"
#include "defcolor.h"
#include "color.h"
#include "configfile.h"
#include "constant.h"
#include "sendinfo.h"
#include "XFCE_icon.h"
#include "menu_icon.h"
#include "extern.h"
#include "selects.h"
#include "resources.h"

char *logo = "/XFCE.logo";

void 
initmenus (void)
{
  Pixmap icon_m, mask_m;
  unsigned w, h;
  int i;

  icon_m = fl_create_from_pixmapdata (fl_root, menu_icon, &w, &h, &mask_m, 0, 0, 0);
  for (i = 0; i < NBMENUS; i++)
    menus[i] = create_form_menu (i, icon_m, mask_m);
  /*
     Allocate memory for the command lines of front panel icons 
   */
  alloc_selects ();
}

XErrorHandler 
ErrorHandler (Display * dpy, XErrorEvent * event)
{
  /*
     Ring the bell to warn user 
   */
  XBell (fl_display, 0);
  if ((event->error_code == BadWindow) ||
      (event->request_code == X_GetGeometry) ||
      (event->error_code == BadDrawable) ||
      (event->request_code == X_SetInputFocus) ||
      (event->request_code == X_ChangeWindowAttributes) ||
      (event->request_code == X_GrabButton) ||
      (event->request_code == X_ChangeWindowAttributes) ||
      (event->request_code == X_InstallColormap))
    return (0);

  fprintf (stderr, "XLib internal error\n");
  end_XFCE (1);
  return (0);
}

void 
reap (int sig)
{
  /*
     This avoid those ugly zombies 
   */
  signal (SIGCHLD, SIG_DFL);
  while ((wait3 (NULL, WNOHANG, NULL)) > 0);
  signal (SIGCHLD, reap);
}

void 
handlesignal (int sig)
{
  /*
     No time to writetoconfig(); Avoid trucation ? 
   */
  end_XFCE (0);
}

void 
assume_event (unsigned long type, unsigned long *body)
{
  int i;
  switch (type)
    {
    case M_NEW_DESK:
      i = (long) body[0];
      if ((i >= 0) && (i < NBSCREENS))
	update_screen (i);
      break;
    default:
      break;
    }
}

void 
deadpipe (int fd, void *d)
{
  handlesignal (0);
}

void 
watchpipe (int fd, void *d)
{
  unsigned long header[HEADER_SIZE];
  unsigned long *body;

  if (readpacket (fd, header, &body) > 0)
    {
      assume_event (header[1], body);
      free (body);
    }
}

int 
initforfvwm (int argc, char *argv[])
{
  if (argc < 6)
    {				/*
				   XFCE has not been lauched as an FVWM module 
				 */
      return (0);
    }

  /*
     Load pipe 
   */
  fd[0] = atoi (argv[1]);
  fd[1] = atoi (argv[2]);
  /*
     Reading in pipe cannot be blocking in this case 
   */
  /*
     fcntl(fd[0],F_SETFL,O_NDELAY); 
   */
  fcntl (fd[1], F_SETFL, O_NDELAY);

  /*
     We're are probably using FVWM 
   */
  return (1);
}

int 
main (int argc, char *argv[])
{
  FL_IOPT flopt;
  Pixmap icon, mask;
  unsigned w, h;
  int version, revision;

  if (FL_INCLUDE_VERSION != fl_library_version (&version, &revision))
    {
      fprintf (stderr, "XFCE : Header and library version don't match\n");
      fprintf (stderr, "   Header: %d.%d\n", FL_VERSION, FL_REVISION);
      fprintf (stderr, "  Library: %d.%d\n", version, revision);
      exit (1);
    }

  fl_initialize (&argc, argv, "XFce", 0, 0);
  flopt.borderWidth = -BORDERWIDTH;
  /*
     Borderwidth cannot be changed thru Resources ... Nah !! 
   */
  fl_set_defaults (FL_PDBorderWidth, &flopt);
  create_resources (&rxfce);
  load_resources (&rxfce);

#ifdef USE_SET_FONT_NAME
  fl_set_font_name (XFTINY, XFCE_TINY);
  fl_set_font_name (XFREG, XFCE_REGULAR);
  fl_set_font_name (XFBOLD, XFCE_BOLD);
  fl_set_font_name (XFBIG, XFCE_BIG);
  fl_set_font_name (XFTIT, XFCE_TITLE);
#endif
  palette = newpal ();
  initpal (palette);
  defpal (palette);
  loadpal (palette);
  applypal (palette);

  XSetErrorHandler ((XErrorHandler) ErrorHandler);
  open_startup ();
  fl_set_goodies_font (FL_BOLD_STYLE, FL_NORMAL_SIZE);
  fl_set_oneliner_font (FL_BOLD_STYLE, FL_SMALL_SIZE);
  fl_set_oneliner_color (FL_BLUE, FL_WHEAT);
  icon = fl_create_from_pixmapdata (fl_root, XFCE_icon, &w, &h, &mask, 0, 0, 0);
  FVWM = initforfvwm (argc, argv);
  if (FVWM)
    fl_set_cursor (fl_root, XFCE_CURS);
  initmenus ();
  fd_XFCE = create_form_XFCE (icon, mask);
  fd_addiconform = create_form_addiconform (icon, mask);
  fd_info = create_form_info (icon, mask);
  fd_def_command = create_form_def_command (icon, mask);
  fd_def_screen = create_form_def_screen (icon, mask);
  fd_def_color = create_form_def_color (icon, mask);
  /*
     Read configuration file 
   */
  readconfig ();
  /*
     Switch to screen 0 by default 
   */
  if (FVWM)
    switch_to_screen (0);
  close_startup ();

  /*
     Hanldle system signals 
   */
  signal (SIGPIPE, handlesignal);
  signal (SIGINT, handlesignal);
  signal (SIGTERM, handlesignal);
  signal (SIGQUIT, handlesignal);
  signal (SIGABRT, handlesignal);
  signal (SIGBUS, handlesignal);
  signal (SIGUSR1, handlesignal);
  signal (SIGUSR2, handlesignal);
  signal (SIGCHLD, reap);

  /*
     show the first form 
   */

  fl_show_form (fd_XFCE->XFCE, FL_PLACE_POSITION + FL_FIX_SIZE, FL_TRANSIENT, "XFCE");
  fl_set_cursor (fd_XFCE->XFCE->window, XFCE_CURS);
  /*
     Some usefull I/O callbacks (new in version 1.2.4) 
   */
  if (FVWM)
    {
      fl_add_io_callback (fd[1], FL_READ, watchpipe, 0);
      fl_add_io_callback (fd[1], FL_EXCEPT, deadpipe, 0);
      fl_add_io_callback (fd[0], FL_EXCEPT, deadpipe, 0);
    }

  fl_do_forms ();
  writetoconfig ();
  return (0);
}
