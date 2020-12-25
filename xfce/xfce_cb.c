/*  gxfce
 *  Copyright (C) 1999 Olivier Fourdan (fourdan@csi.com)
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

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include "my_intl.h"
#include "xfce_cb.h"
#include "action.h"
#include "xfce.h"
#include "xfce-common.h"
#include "popup.h"
#include "xpmext.h"
#include "selects.h"
#include "configfile.h"
#include "fileutil.h"
#include "xfwm.h"
#include "xfce_main.h"
#include "setup.h"
#include "screen.h"
#include "my_string.h"
#include "minbutup.h"
#include "minbutdn.h"

static gint current_screen_selected = -1;
static gboolean use_action = TRUE; /* kind of semphor */

static char *day_names[] = { N_("Sunday"),
                             N_("Monday"),
                             N_("Tuesday"),
                             N_("Wednesday"),
                             N_("Thursday"),
                             N_("Friday"),
                             N_("Saturday") };

static char *month_names[] = { N_("January"),
                     	       N_("February"),
                     	       N_("March"),
                     	       N_("April"),
                     	       N_("May"),
                     	       N_("June"),
                     	       N_("July"),
                     	       N_("August"),
                     	       N_("September"),
                     	       N_("October"),
                     	       N_("November"),
                     	       N_("December") };

void iconify_cb(    GtkWidget *widget,
                    gpointer   data )
{
  XIconifyWindow(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(((GtkWidget *) data)->window), MY_GDK_SCREEN());
}

void quit_cb(      GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   data )
{
  writeconfig();
  if (my_yesno_dialog(_("Are you sure you want to Quit ?\nThis might log you off !"))) {
     gtk_main_quit();
     free_selects();
     quit_wm();
     exit(0);
  }
}

gint delete_event( GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   data )
{
  quit_cb (widget,event,data);
  return (TRUE);
}

void destroy_cb(   GtkWidget *widget,
                   gpointer   data )
{
    writeconfig();
    gtk_main_quit();
}

gint
select_modify_cb(  GtkWidget *widget,
                   GdkEventButton *event,
                   gpointer   data )
{
  if(event->button == 3) {
    open_action(action, (int) data);
    return TRUE;
  }
  return FALSE;
}

void popup_cb(      GtkWidget *widget,
                   gpointer   data )
{
  gint upositionx = 0;
  gint upositiony = 0;
  gint offsetx    = 0;
  gint offsety    = 0;
  gint uwidth	  = 0;
  gint uheight    = 0;
  gint udepth	  = 0;
  
  gdk_window_get_geometry (((GtkWidget *) widget)->window, &upositionx, &upositiony,
							    &uwidth, &uheight, &udepth );
  gdk_window_get_root_origin (((GtkWidget *) widget)->window, &offsetx, &offsety);
  toggle_popup_button(widget, (GtkPixmap *) popup_buttons.popup_pixmap[(int) data]);
  if (GTK_TOGGLE_BUTTON (widget)->active)
    show_popup_menu ((int) data, upositionx + offsetx + uwidth / 2,upositiony + offsety );
  else
    close_popup_menu ((int) data);
}

void select_cb(      GtkWidget *widget,
                   gpointer   data )
{
  exec_comm (get_command ((int) data), current_config.wm);
}

void update_screen(int i)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (screen_buttons.screen_button[i])))
    return;
  
  if (i == current_screen_selected)
    return;

  if (i >= current_config.visible_screen)
    my_show_message (_("You've selected a desktop which is not currently\nvisible in XFce panel."));
  else {
    use_action = FALSE;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (screen_buttons.screen_button[i]), TRUE);
    if (current_config.colorize_root)
      ApplyRootColor (pal, (current_config.gradient_root != 0), get_screen_color(i));
  }

  current_screen_selected = i;
  use_action = TRUE;
}

void screen_cb(      GtkWidget *widget,
                     gpointer   data )
{
  if (!use_action) {
    return;
  }
  
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    return;
  
  if ((int) data == current_screen_selected)
    return;
  
  if ((int) data < current_config.visible_screen)
    switch_to_screen(pal, (int) data);
  current_screen_selected = (int) data;
}

gint 
screen_modify_cb(      GtkWidget *widget,
                     GdkEventButton *event,
                     gpointer   data )
{
  if(event->button == 3) {
    open_screen(screen, (int) data);
    return TRUE;
  }
  return FALSE;
}

void setup_cb(      GtkWidget *widget,
                     gpointer   data )
{
  show_setup (data);
}

void info_cb(      GtkWidget *widget,
                     gpointer   data )
{
  gtk_widget_show (info_scr);
}

void
toggle_popup_button(GtkWidget *widget, GtkPixmap *pix)
{
  static GdkPixmap *pixmap_up = NULL;
  static GdkBitmap *mask_up = NULL;
  static GdkPixmap *pixmap_dn = NULL;
  static GdkBitmap *mask_dn = NULL;
  GtkStyle *style;

  style = gtk_widget_get_style( widget );
  if (!(pixmap_up && mask_up))
    pixmap_up = MyCreateGdkPixmapFromData (minbutup, widget, &mask_up);
  if (!(pixmap_dn && mask_dn))
    pixmap_dn = MyCreateGdkPixmapFromData (minbutdn, widget, &mask_dn); 
					   
  if (GTK_IS_PIXMAP (pix))  gtk_pixmap_set (pix, (GTK_TOGGLE_BUTTON (widget)->active) ? pixmap_dn : pixmap_up, 
                                                 (GTK_TOGGLE_BUTTON (widget)->active) ? mask_dn : mask_up);
}

int
get_current_screen(void)
{
  return ((current_screen_selected >= 0) ? current_screen_selected : 0);
}

void
set_current_screen(int i)
{
  current_screen_selected = i;
}

void  
select_drag_data_received  (GtkWidget          *widget,
			    GdkDragContext     *context,
			    gint                x,
			    gint                y,
			    GtkSelectionData   *data,
			    guint               info,
			    guint               time,
                            gpointer            cbdata )
{
  GList *fnames, *fnp;
  guint count;
  char * execute, * cmd;
  fnames = gnome_uri_list_extract_filenames((char *)data->data);
  count = g_list_length(fnames);
  if (count > 0) {
      execute = (char *) malloc (MAXSTRLEN);
      cmd = get_command((int) cbdata);
      for (fnp = fnames; fnp; fnp = fnp->next, count--) {
	   snprintf (execute, MAXSTRLEN - 1, "%s %s", cmd,  (char *)(fnp->data));
	   exec_comm (execute, current_config.wm);
      }
      free (execute);
  }
  gnome_uri_list_free_strings(fnames);
  gtk_drag_finish (context, (count > 0), (context->action == GDK_ACTION_MOVE), time);
}
  
gint
update_gxfce_date_timer (GtkWidget *widget)
{
  time_t ticks;
  struct tm *tm;
  static gint mday = -1;
  static gint wday = -1;
  static gint mon  = -1;
  static gint year = -1;
  char date_s[255];
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  ticks = time(0);
  tm = localtime(&ticks);
  if ((mday != tm->tm_mday)       ||
      (wday != tm->tm_wday)       ||
      (mon  != tm->tm_mon)        ||
      (year != tm->tm_year))
    {
      mday = tm->tm_mday;
      wday = tm->tm_wday;
      mon  = tm->tm_mon;
      year = tm->tm_year;
      snprintf ( date_s, 255, "%s, %u %s %u", _(day_names[wday]), mday, _(month_names[mon]), year + 1900);

      gtk_tooltips_set_tip (gtk_tooltips_data_get(GTK_WIDGET (widget))->tooltips,
                            GTK_WIDGET (widget),
			    date_s,
			    "ContextHelp/buttons/?");
    }
  return TRUE;
}

