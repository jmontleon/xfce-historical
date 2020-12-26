/*  gxfce
 *  Copyright (C) 1999 Olivier Fourdan <fourdan@xfce.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

#ifdef HAVE_GDK_IMLIB
  #include <gdk_imlib.h>
#endif

#include "my_intl.h"
#include "xfce.h"
#include "setup.h"
#include "fileutil.h"
#include "modify.h"
#include "action.h"
#include "screen.h"
#include "info.h"
#include "popup.h"
#include "startup.h"
#include "xfce_main.h"
#include "xfce-common.h"
#include "xfce_cb.h"
#include "move.h"
#include "xfcolor.h"
#include "configfile.h"
#include "selects.h"
#include "xfwm.h"
#include "xpmext.h"
#include "gnome_protocol.h"
#include "my_tooltips.h"

extern gint check_mail(gpointer data);

char **g_argv;

void 
reap (int sig)
{
  signal (SIGCHLD, SIG_DFL);
  while (waitpid (-1, NULL, WNOHANG) > 0);
  signal (SIGCHLD, reap);
}

int
main (int argc, char *argv[])
{
  int i;
  char gtkrc_file[MAXSTRLEN + 1];
  
  g_argv = argv;
  
  signal_setup ();

  gtk_set_locale ();
  bindtextdomain(PACKAGE, XFCE_LOCALE_DIR);
  textdomain (PACKAGE);
  gtk_init (&argc, &argv);

  /* The GC for the outline move */
  CreateDrawGC (GDK_ROOT_PARENT());

#ifdef HAVE_GDK_IMLIB
  gdk_imlib_init();
/* Get gdk to use imlib's visual and colormap */
  gtk_widget_push_visual(gdk_imlib_get_visual());
  gtk_widget_push_colormap(gdk_imlib_get_colormap());  
#endif

  /* Create atoms for a GNOME complaint Window Manager */
  create_gnome_atoms();
  
  fcntl(ConnectionNumber(GDK_DISPLAY()), F_SETFD, 1);
  signal (SIGCHLD, reap);
  
  initconfig(&current_config);

  /* Initialisation of GtkStyle palettes */
  pal = newpal ();
  defpal (pal);
  loadpal (pal);
  applypal (pal, NULL);

  current_config.wm = initwm (argc, argv);
  
  startup = create_startup ();
  open_startup(startup);

  /* 
    Make sure startup screen is mapped on screen :
    pro  :  decrease system load at startup
    cons :  slow down startup phase
  */
  for (i = 0; i < 20; i++)
    {
       update_events();
       my_sleep(50);
    }
  update_events();

  setup    = create_setup (pal);
  modify   = create_modify (pal);
  action   = create_action (pal);
  screen   = create_screen (pal);
  info_scr = create_info (pal);
  gxfce    = create_gxfce (pal);

  create_all_popup_menus (pal, gxfce);
  alloc_selects ();
  readconfig();  
  update_gxfce_screen_buttons(current_config.visible_screen);
  gnome_set_desk_count(current_config.visible_screen);
  update_gxfce_popup_buttons(current_config.visible_popup);
  update_gxfce_size();
  update_popup_size();
  update_delay_tooltips(current_config.tooltipsdelay);
  update_gxfce_coord(gxfce, &current_config.panel_x, &current_config.panel_y);
  update_gxfce_clock();
  gtk_widget_set_uposition (gxfce, current_config.panel_x, current_config.panel_y);

  gtk_widget_show (gxfce);
  gnome_sticky(gxfce->window);

  if (!current_config.wm)
    gnome_set_root_event(GTK_SIGNAL_FUNC (handle_gnome_workspace_event_cb));
  
  switch_to_screen(pal, 0);
  cursor_wait(gxfce);
  init_watchpipe();

  reg_xfce_app(gxfce, pal);
  close_startup (startup);
  
  /* Build path to gtkrc file in user's home directory */
  strcpy (gtkrc_file, (char *) getenv ("HOME"));
  strcat (gtkrc_file, "/.gtkrc");

  /* Update all running GTK+ apps if ~/.gtkrc is not present */

  if (!existfile (gtkrc_file))
    {
      create_gtkrc_file(pal);
      applypal_to_all ();
    }
  
  /* Apply user's preferences to the Window Manager */

  if (current_config.wm == XFWM) {
    apply_wm_colors(pal);
    apply_wm_fonts();
    apply_wm_iconpos();
    apply_wm_options();
    apply_wm_snapsize();
    startup_wm_modules();
  }

  /* Add a hook for the mail-check function. */
  gtk_timeout_add(MAILCHECK_PERIOD, check_mail, NULL);

  cursor_reset(gxfce);
  gtk_main ();
  return 0;
}
