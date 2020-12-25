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
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#include "forms.h"
#include "XFCE.h"
#include "startup.h"
#include "pulldown.h"
#include "addicon.h"
#include "info.h"
#include "screen.h"
#include "command.h"
#include "my_string.h"
#include "defcolor.h"
#include "color.h"
#include "configfile.h"
#include "constant.h"
#include "sendinfo.h"
#include "mwmdecor.h"
#include "xfwm_cmd.h"
#include "XFCE_icon.h"
#include "menu_icon.h"
#include "move.h"
#include "extern.h"
#include "selects.h"
#include "resources.h"
#include "version.h"


char *logo = "/XFce2.logo";
Atom _XA_MwmAtom;

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
  fprintf(stderr, "XFCE: Signal caught, exiting...\n");
  exit (0);
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
  fprintf(stderr, "XFCE: Communication pipe closed. Exiting...\n");
  exit (0);
}

void 
watchpipe (int fd, void *d)
{
  unsigned long header[HEADER_SIZE];
  unsigned long *body;
  /*
      An ugly and dirty hack to cause a SIGPIPE signal on Linux 2.2 Kernel
   */
  sendinfo (d, NOP_CMD, 0);
  
  /*
      And now, the real stuff...
   */
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
				   XFCE has not been launched as an FVWM module 
				 */
      return (0);
    }

  /*
     Load pipe 
   */
  fd[0] = atoi (argv[1]);
  fd[1] = atoi (argv[2]);
  /*
     if -nowm is defined, don't use XFwm interface
   */
  if (argc > 7)
    if (!my_strncasecmp(argv[7], "-nowm", strlen("-nowm")))
      return (1);

  /*
     Are we using XFwm or another FVWM's compatible Window Manager ?
   */
  if (argc > 6)
    if (!my_strncasecmp(argv[6], "XFwm", strlen("XFwm")))
      return (2);

  return (1);
}

int 
main (int argc, char *argv[])
{
  FL_IOPT flopt;
  Pixmap icon, mask;
  unsigned w, h;
  int version, revision;
  char *xfce_lang, *env_value, *classname;

  if (FL_INCLUDE_VERSION != fl_library_version (&version, &revision))
    {
      fprintf (stderr, "XFCE : Header and library version don't match\n");
      fprintf (stderr, "   Header: %d.%d\n", FL_VERSION, FL_REVISION);
      fprintf (stderr, "  Library: %d.%d\n", version, revision);
      exit (1);
    }

  /*
     Hanldle system signals 
   */
  signal (SIGPIPE, handlesignal);
  signal (SIGINT,  handlesignal);
  signal (SIGTERM, handlesignal);
  signal (SIGQUIT, handlesignal);
  signal (SIGABRT, handlesignal);
  signal (SIGBUS,  handlesignal);
  signal (SIGUSR1, handlesignal);
  signal (SIGUSR2, handlesignal);
  signal (SIGCHLD, reap);

  /* 
     Initialization Stuff
   */

  flopt.borderWidth = -BORDERWIDTH;
  fl_set_defaults (FL_PDBorderWidth, &flopt);
  xfce_lang = (char *) malloc (3 * sizeof (char));
  env_value = (char *) getenv ("XFCE_LANG");
  if (env_value) {
    strncpy (xfce_lang, env_value, 2);
    xfce_lang[2] = '\0';
  }
  else
    xfce_lang[0] = '\0';
  
  classname = (char *) malloc (8 * sizeof (char));
  strcpy (classname, "XFce");
  if ((xfce_lang) && (strlen (xfce_lang))) {
    strcat (classname, "-");
    strcat (classname, xfce_lang);
  }

  if (!fl_initialize (&argc, argv, classname, 0, 0)) 
    exit (1);

  create_resources (&rxfce);
  load_resources (&rxfce);

  setlocale (LC_ALL, "");

  free (xfce_lang);
  free (classname);

  /*
     New in v. 2.0.2 : Fonts are taken from the resources
   */
  fl_set_font_name (XFTINY, rxfce.tinyfont);
  fl_set_font_name (XFREG,  rxfce.regularfont);
  fl_set_font_name (XFBOLD, rxfce.boldfont);
  fl_set_font_name (XFBIG,  rxfce.bigfont);
  fl_set_font_name (XFTIT,  rxfce.titlefont);

  FVWM = initforfvwm (argc, argv);
  palette = newpal ();
  initpal (palette);
  defpal (palette);
  loadpal (palette);
  applypal (palette);
  fl_set_cursor_color(XFCE_CURS, palcolor(palette, 0), FL_WHITE);
  fl_set_cursor_color(MENU_CURS, palcolor(palette, 0), FL_WHITE);
  
  if (FVWM == 2)
     apply_wm_colors(fd, palette);

  XSetErrorHandler ((XErrorHandler) ErrorHandler);
  _XA_MwmAtom=XInternAtom(fl_display,"_MOTIF_WM_HINTS",False);
  open_startup ();
  fl_set_goodies_font (FL_BOLD_STYLE, FL_NORMAL_SIZE);
  fl_set_oneliner_font (FL_NORMAL_STYLE, FL_NORMAL_SIZE);
  fl_set_oneliner_color (FL_BLUE, FL_WHEAT);
  fl_setpup_default_cursor(MENU_CURS);
  fl_set_cursor (fl_root, XFCE_CURS);

  icon = fl_create_from_pixmapdata (fl_root, XFCE_icon, &w, &h, &mask, 0, 0, 0);
  initmenus ();
  fd_XFCE = create_form_XFCE (icon, mask, (fl_scrw <= 800));
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
     Create GC for outline move
   */
  CreateDrawGC();

  /*
     show the first form 
   */
  fl_prepare_form_window (fd_XFCE->XFCE, FL_PLACE_POSITION + FL_FIX_SIZE, FL_FULLBORDER, "XFce Main Panel");
  defdecor(fl_display, fd_XFCE->XFCE->window, MWM_DECOR_BORDER);
  close_startup ();

  /*
     Some usefull I/O callbacks (new in version 1.2.4) 
   */
  if (FVWM == 2) {
     sendinfo(fd, (CLICKTOFOCUS ? FOCUSCLICK_CMD : FOCUSMOUSE_CMD) , 0);
     sendinfo(fd, (OPAQUEMOVE ? OPAQUEMOVEON_CMD : OPAQUEMOVEOFF_CMD), 0);
     sendinfo(fd, (AUTORAISE ? AUTORAISEON_CMD : AUTORAISEOFF_CMD), 0);
  }

  fl_show_form_window(fd_XFCE->XFCE);
  fl_set_cursor (fd_XFCE->XFCE->window, XFCE_CURS);

  if (FVWM)
    {
      fl_add_io_callback (fd[0], FL_EXCEPT, deadpipe, 0);
      fl_add_io_callback (fd[1], FL_READ || FL_EXCEPT, watchpipe, &fd[0]);
      switch_to_screen (0);
      update_screen (0);
    }

  fl_do_forms ();
  writetoconfig ();
  return (0);
}
