/*  gxfce
 *  Copyright (C) 1999 Olivier Fourdan (fourdan@xfce.org)
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
#include <string.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "my_intl.h"
#include "colorselect.h"
#include "xfcolor.h"

#ifdef DMALLOC
#  include "dmalloc.h"
#endif

static GtkWidget *colorselect;
static gdouble selcolor[4];
static XFCE_palette *p_tmp;
static int color_index_tmp;

static void
ok_cb (GtkWidget * twidget, gpointer data)
{
  gtk_color_selection_get_color (GTK_COLOR_SELECTION
				 (GTK_COLOR_SELECTION_DIALOG (colorselect)->
				  colorsel), selcolor);
  set_palcolor (p_tmp, color_index_tmp, selcolor);
  gtk_widget_destroy (colorselect);
  gtk_main_quit ();
}

static void
cancel_cb (GtkWidget * twidget, gpointer data)
{
  gtk_widget_destroy (colorselect);
  gtk_main_quit ();
}

static gboolean
delete_event (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  gtk_widget_destroy (colorselect);
  gtk_main_quit ();
  return (TRUE);
}

void
open_colorselect (XFCE_palette * p, int color_index)
{
  GtkWidget *colorselect_ok_button;
  GtkWidget *colorselect_cancel_button;
  GtkWidget *colorselect_help_button;
  GtkAccelGroup *accel_group;

  p_tmp = p;
  color_index_tmp = color_index;

  colorselect = gtk_color_selection_dialog_new (_("Select Color"));
  gtk_widget_set_name (colorselect, "colorselect");
  gtk_object_set_data (GTK_OBJECT (colorselect), "colorselect", colorselect);
  gtk_container_border_width (GTK_CONTAINER (colorselect), 10);
  GTK_WINDOW (colorselect)->type = GTK_WINDOW_DIALOG;
  gtk_color_selection_set_update_policy (GTK_COLOR_SELECTION
					 (GTK_COLOR_SELECTION_DIALOG
					  (colorselect)->colorsel),
					 GTK_UPDATE_CONTINUOUS);
  gtk_window_position (GTK_WINDOW (colorselect), GTK_WIN_POS_MOUSE);

  accel_group = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (colorselect), accel_group);


  colorselect_ok_button = GTK_COLOR_SELECTION_DIALOG (colorselect)->ok_button;
  gtk_widget_set_name (colorselect_ok_button, "colorselect_ok_button");
  gtk_object_set_data (GTK_OBJECT (colorselect), "colorselect_ok_button",
		       colorselect_ok_button);
  gtk_widget_show (colorselect_ok_button);
  GTK_WIDGET_SET_FLAGS (colorselect_ok_button, GTK_CAN_DEFAULT);

  colorselect_cancel_button =
    GTK_COLOR_SELECTION_DIALOG (colorselect)->cancel_button;
  gtk_widget_set_name (colorselect_cancel_button,
		       "colorselect_cancel_button");
  gtk_object_set_data (GTK_OBJECT (colorselect), "colorselect_cancel_button",
		       colorselect_cancel_button);
  gtk_widget_show (colorselect_cancel_button);
  GTK_WIDGET_SET_FLAGS (colorselect_cancel_button, GTK_CAN_DEFAULT);

  colorselect_help_button =
    GTK_COLOR_SELECTION_DIALOG (colorselect)->help_button;
  gtk_widget_set_name (colorselect_help_button, "colorselect_help_button");
  gtk_object_set_data (GTK_OBJECT (colorselect), "colorselect_help_button",
		       colorselect_help_button);
  /* gtk_widget_show (colorselect_help_button); */
  GTK_WIDGET_UNSET_FLAGS (colorselect_help_button, GTK_CAN_FOCUS);
  GTK_WIDGET_SET_FLAGS (colorselect_help_button, GTK_CAN_DEFAULT);
  gtk_widget_set_sensitive (colorselect_help_button, FALSE);
  gtk_widget_hide (colorselect_help_button);

  gtk_widget_add_accelerator (colorselect_ok_button, "clicked", accel_group,
			      GDK_Return, 0, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (colorselect_ok_button, "clicked", accel_group,
			      GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  gtk_widget_add_accelerator (colorselect_cancel_button, "clicked",
			      accel_group, GDK_Escape, 0, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (colorselect_cancel_button, "clicked",
			      accel_group, GDK_c, GDK_CONTROL_MASK,
			      GTK_ACCEL_VISIBLE);

  gtk_signal_connect (GTK_OBJECT (colorselect_ok_button), "clicked",
		      GTK_SIGNAL_FUNC (ok_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (colorselect_cancel_button), "clicked",
		      GTK_SIGNAL_FUNC (cancel_cb), NULL);

  gtk_signal_connect (GTK_OBJECT (colorselect), "delete_event",
		      GTK_SIGNAL_FUNC (delete_event), NULL);

  gtk_signal_connect (GTK_OBJECT (colorselect), "destroy",
		      GTK_SIGNAL_FUNC (delete_event), NULL);

  set_selcolor (p, color_index, selcolor);
  gtk_color_selection_set_color (GTK_COLOR_SELECTION
				 (GTK_COLOR_SELECTION_DIALOG (colorselect)->
				  colorsel), selcolor);
  gtk_window_set_modal (GTK_WINDOW (colorselect), TRUE);
  gtk_widget_show (colorselect);
  gtk_main ();
}
