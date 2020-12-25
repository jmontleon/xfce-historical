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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include "my_intl.h"
#include "mygtkclock.h"
#include "xfce_main.h"
#include "xfce-common.h"
#include "move.h"
#include "xfce.h"
#include "xfce_cb.h"
#include "xfcolor.h"
#include "constant.h"
#include "xpmext.h"
#include "configfile.h"
#include "xfwm.h"
#include "selects.h"
#include "my_tooltips.h"

#include "xfce_icon.h"
#include "mininf.h"
#include "minipnt.h"
#include "minilock.h"
#include "defaulticon.h"
#include "handle.h"
#include "closepix.h"
#include "iconify.h"
#include "minbutup.h"

#define PANEL_ICON_SIZE ((current_config.select_icon_size == 0) ? SMALL_PANEL_ICONS : \
                           ((current_config.select_icon_size == 2) ? LARGE_PANEL_ICONS : MEDIUM_PANEL_ICONS))

char *screen_names[] = {N_("One"), N_("Two"), N_("Three"), N_("Four"), N_("Five"),
                        N_("Six"), N_("Seven"), N_("Eight"), N_("Nine"), N_("Ten"),
			N_("Eleven"), N_("Twelve"), N_("Thirteen"), N_("Fourteen"), N_("Fifteen"),
			N_("Sixteen"), N_("Seventeen"), N_("Eighteen"), N_("Nineteen"), N_("Twenty")};
int screen_colors[] = {2, 5, 4, 6};

enum {
  TARGET_STRING,
  TARGET_ROOTWIN,
  TARGET_URL
};

static GtkTargetEntry select_target_table[] = {
  { "STRING",     0, TARGET_STRING },
  { "text/plain", 0, TARGET_STRING },
  { "text/uri-list", 0, TARGET_URL },
};

static guint n_select_targets = sizeof(select_target_table) / sizeof(select_target_table[0]);

GtkWidget *
gxfce_clock_make_popup (MyGtkClock *clock) 
{
  if (!gxfce_clock_popup_menu)
  {
    gxfce_clock_popup_menu = gtk_menu_new ();
    if (!gxfce_clock_popup_menu) return (GtkWidget *) NULL;
  }
  
  gxfce_clock_digital_mode = gtk_check_menu_item_new_with_label(_("Digital mode"));
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM(gxfce_clock_digital_mode), 1);

  gtk_menu_append (GTK_MENU(gxfce_clock_popup_menu), gxfce_clock_digital_mode);
  gtk_signal_connect (GTK_OBJECT(gxfce_clock_digital_mode), "activate",
        	      GTK_SIGNAL_FUNC(clock_digital_cb),
        	      (gpointer) clock);
                      
  gtk_widget_show (gxfce_clock_digital_mode);

  gxfce_clock_hrs_mode = gtk_check_menu_item_new_with_label(_("24 hours mode"));
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM(gxfce_clock_hrs_mode), 1);

  gtk_menu_append (GTK_MENU(gxfce_clock_popup_menu), gxfce_clock_hrs_mode);
  gtk_signal_connect (GTK_OBJECT(gxfce_clock_hrs_mode), "activate",
        	      GTK_SIGNAL_FUNC(clock_hrs_cb),
        	      (gpointer) clock);
                     
  gtk_widget_show (gxfce_clock_hrs_mode);
  return gxfce_clock_popup_menu;
}

void
update_gxfce_clock(void)
{
  my_gtk_clock_set_mode(MY_GTK_CLOCK (gxfce_clock), 
        current_config.digital_clock ? MY_GTK_CLOCK_DIGITAL : MY_GTK_CLOCK_ANALOG);
  my_gtk_clock_show_military(MY_GTK_CLOCK (gxfce_clock), current_config.hrs_mode);
}

void
update_gxfce_coord(GtkWidget * toplevel, int *x, int *y)
{
  GtkRequisition requisition;

  if ((*x < 0) || (*y < 0)) {
    gtk_widget_size_request (GTK_WIDGET (toplevel), &requisition);
    *x = (gdk_screen_width() - requisition.width) / 2;
    *y = gdk_screen_height() - requisition.height - 20;
    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
  }
}

void
update_gxfce_size(void)
{
  int i;
  
  for (i = 0; i < NBPOPUPS; i++)
    gtk_widget_set_usize (popup_buttons.popup_button[i], PANEL_ICON_SIZE + 4, 16);
    
  for (i = 0; i < NBSELECTS; i++)
    gtk_widget_set_usize (select_buttons.select_button[i], PANEL_ICON_SIZE, PANEL_ICON_SIZE);

  gtk_widget_set_usize (gxfce_move_left_pixmap, PANEL_ICON_SIZE - 16 + 4, 16);
  gtk_widget_set_usize (gxfce_move_right_pixmap, PANEL_ICON_SIZE - 16 + 4, 16);

  gtk_widget_set_usize (gxfce_clock_event, PANEL_ICON_SIZE, PANEL_ICON_SIZE);

  setup_icon ();
}

GtkWidget*
create_gxfce_popup_button (GtkWidget *toplevel, XFCE_palette * pal, gint nbr)
{
  popup_buttons.popup_button[nbr] = gtk_toggle_button_new ();
  gtk_widget_set_name (popup_buttons.popup_button[nbr], "gxfce_popup_panel");
  gtk_object_set_data (GTK_OBJECT (toplevel), "gxfce_popup_panel", popup_buttons.popup_button[nbr]);
  gtk_widget_show (popup_buttons.popup_button[nbr]);
  gtk_widget_set_usize (popup_buttons.popup_button[nbr], PANEL_ICON_SIZE + 4, 16);
  gtk_widget_set_style(popup_buttons.popup_button[nbr], pal->cm[1]);

  popup_buttons.popup_pixmap[nbr] = MyCreateFromPixmapData (popup_buttons.popup_button[nbr], minbutup);
  if (popup_buttons.popup_pixmap[nbr] == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_set_name (popup_buttons.popup_pixmap[nbr], "gxfce_popup_button_pixmap");
  gtk_object_set_data (GTK_OBJECT (toplevel), "gxfce_popup_button_pixmap", popup_buttons.popup_pixmap[nbr]);
  gtk_widget_show (popup_buttons.popup_pixmap[nbr]);
  gtk_container_add (GTK_CONTAINER (popup_buttons.popup_button[nbr]), popup_buttons.popup_pixmap[nbr]);
  gtk_widget_set_style(popup_buttons.popup_pixmap[nbr], pal->cm[1]);
  gtk_signal_connect (GTK_OBJECT (popup_buttons.popup_button[nbr]), "clicked",
                             GTK_SIGNAL_FUNC (popup_cb),
                             (gpointer) nbr);

  return popup_buttons.popup_button[nbr];
}

GtkWidget*
create_gxfce_select_button (GtkWidget *toplevel, XFCE_palette * pal, gint nbr)
{
  select_buttons.select_button[nbr] = gtk_button_new ();
  gtk_widget_set_name (select_buttons.select_button[nbr], "gxfce_select_button");
  gtk_object_set_data (GTK_OBJECT (toplevel), "gxfce_select_button", select_buttons.select_button[nbr]);
  gtk_button_set_relief((GtkButton *) select_buttons.select_button[nbr], GTK_RELIEF_NONE);
  gtk_widget_show (select_buttons.select_button[nbr]);
  gtk_widget_set_usize (select_buttons.select_button[nbr], PANEL_ICON_SIZE, PANEL_ICON_SIZE);
  gtk_container_border_width (GTK_CONTAINER (select_buttons.select_button[nbr]), 2);
  /* gtk_widget_set_style(select_buttons.select_button[nbr], pal->cm[7]); */
  select_buttons.select_tooltips[nbr] = my_tooltips_new(current_config.tooltipsdelay);
  gtk_tooltips_set_tip (select_buttons.select_tooltips[nbr],select_buttons.select_button[nbr],"None", "ContextHelp/buttons/?");
  gtk_object_set_data (GTK_OBJECT (toplevel), "tooltips", select_buttons.select_tooltips[nbr]);

  gtk_drag_dest_set (select_buttons.select_button[nbr],
		     GTK_DEST_DEFAULT_ALL,
		     select_target_table, n_select_targets,
		     GDK_ACTION_COPY | GDK_ACTION_MOVE);

  gtk_signal_connect (GTK_OBJECT (select_buttons.select_button[nbr]), "drag_data_received",
		      GTK_SIGNAL_FUNC (select_drag_data_received), (gpointer) nbr);
  
  gtk_widget_set_events(select_buttons.select_button[nbr], 
                        gtk_widget_get_events (select_buttons.select_button[nbr]) | GDK_BUTTON3_MASK);

  gtk_signal_connect( GTK_OBJECT(select_buttons.select_button[nbr]), "button_press_event",
                              GTK_SIGNAL_FUNC(select_modify_cb),
			      (gpointer) nbr);
  gtk_signal_connect (GTK_OBJECT (select_buttons.select_button[nbr]), "clicked",
                             GTK_SIGNAL_FUNC (select_cb),
                             (gpointer) nbr);
		      
  select_buttons.select_pixmap[nbr] = MyCreateFromPixmapData (select_buttons.select_button[nbr], defaulticon);
  if (select_buttons.select_pixmap[nbr] == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_set_name (select_buttons.select_pixmap[nbr], "gxfce_select_button_pixmap");
  gtk_object_set_data (GTK_OBJECT (toplevel), "gxfce_select_button_pixmap", select_buttons.select_pixmap[nbr]);
  gtk_widget_show (select_buttons.select_pixmap[nbr]);
  gtk_container_add (GTK_CONTAINER (select_buttons.select_button[nbr]), select_buttons.select_pixmap[nbr]);
  gtk_widget_set_style(select_buttons.select_pixmap[nbr], pal->cm[1]);
  
  return select_buttons.select_button[nbr];
}

GtkWidget*
create_gxfce_popup_select_group (GtkWidget *toplevel, GtkWidget *popup, GtkWidget *select)
{
  GtkWidget *vbox;

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox, "button_vbox");
  gtk_object_set_data (GTK_OBJECT (toplevel), "button_vbox", vbox);
  gtk_widget_show (vbox);
  gtk_widget_set_usize (vbox, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), popup,  TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), select, FALSE, FALSE, 0);
  
  return vbox;
}

char *
get_gxfce_screen_label(int i)
{
  return (char *) GTK_LABEL(screen_buttons.screen_label[i])->label;
}

void
set_gxfce_screen_label(int i, char *label)
{
  if (label && strlen(label))
    gtk_label_set_text (GTK_LABEL(screen_buttons.screen_label[i]), label);
  else
    gtk_label_set_text (GTK_LABEL(screen_buttons.screen_label[i]), _(screen_names[i]));
}

int
get_screen_color(int idx)
{
  return screen_colors[((idx >= 0) ? idx : 0) % 4];
}

GtkWidget*
create_gxfce_screen_buttons (GtkWidget *toplevel, XFCE_palette * pal)
{
  GSList *desktops_table_group = NULL;
  GtkWidget *desktops_table;
  gint i;

  desktops_table = gtk_table_new (2, ((NBSCREENS + 1) / 2), FALSE);

  for (i = 0; i < NBSCREENS; i++) {
    screen_buttons.screen_frame[i] = gtk_frame_new (NULL);
    gtk_widget_set_name (screen_buttons.screen_frame[i], "screen_frame");
    gtk_object_set_data (GTK_OBJECT (toplevel), "screen_frame", screen_buttons.screen_frame[i]);
    gtk_frame_set_shadow_type (GTK_FRAME (screen_buttons.screen_frame[i]), GTK_SHADOW_IN);
    gtk_widget_set_usize (screen_buttons.screen_frame[i], 0, 0);
    gtk_container_border_width (GTK_CONTAINER (screen_buttons.screen_frame[i]), 2);
    screen_buttons.screen_button[i] = gtk_radio_button_new (desktops_table_group);
    desktops_table_group = gtk_radio_button_group (GTK_RADIO_BUTTON (screen_buttons.screen_button[i]));
    gtk_widget_set_name (screen_buttons.screen_button[i], screen_names[i]);
    gtk_object_set_data (GTK_OBJECT (toplevel), screen_names[i], screen_buttons.screen_button[i]);
    gtk_button_set_relief((GtkButton *)screen_buttons.screen_button[i], GTK_RELIEF_HALF);
    gtk_widget_set_usize (screen_buttons.screen_button[i], 0, 0);
    gtk_widget_show (screen_buttons.screen_button[i]);
    gtk_container_add (GTK_CONTAINER (screen_buttons.screen_frame[i]), screen_buttons.screen_button[i]);
    screen_buttons.screen_label[i] = gtk_label_new (_(screen_names[i]));
    gtk_misc_set_alignment (GTK_MISC (screen_buttons.screen_label[i]), 0, 0.5);
    gtk_container_add (GTK_CONTAINER (screen_buttons.screen_button[i]), screen_buttons.screen_label[i]);
    gtk_widget_show (screen_buttons.screen_label[i]);
    gtk_table_attach (GTK_TABLE (desktops_table), screen_buttons.screen_frame[i], (i / 2), (i / 2) + 1, (i % 2), (i % 2) + 1,
                      (GtkAttachOptions) GTK_EXPAND | GTK_FILL | GTK_SHRINK, (GtkAttachOptions) GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
    gtk_container_border_width (GTK_CONTAINER (screen_buttons.screen_button[i]), 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (screen_buttons.screen_button[i]), (i == 0));
    gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (screen_buttons.screen_button[i]), FALSE);
    /* gtk_widget_set_style(screen_buttons.screen_frame[i], pal->cm[7]); */
    gtk_widget_set_style(screen_buttons.screen_button[i], pal->cm[get_screen_color(i)]);
    gtk_widget_set_style(screen_buttons.screen_label[i], pal->cm[get_screen_color(i)]);
    gtk_widget_set_events(screen_buttons.screen_button[i], 
                        gtk_widget_get_events (screen_buttons.screen_button[i]) | GDK_BUTTON3_MASK);
    gtk_signal_connect( GTK_OBJECT(screen_buttons.screen_button[i]), "button_press_event",
                              GTK_SIGNAL_FUNC(screen_modify_cb),
			      (gpointer) i);
    gtk_signal_connect (GTK_OBJECT (screen_buttons.screen_button[i]), "clicked",
                             GTK_SIGNAL_FUNC (screen_cb),
                             (gpointer) i);
  }
  return desktops_table;
}

void
update_gxfce_screen_buttons (gint visible)
{
  gint i, true_visible;
  
  true_visible = ((visible + 1) / 2) * 2;
  if (true_visible < 2) true_visible = 2;
  if (true_visible > 10) true_visible = 10;  
  for (i = 0; i < true_visible; i++) {
    gtk_widget_show (screen_buttons.screen_frame[i]);
  }
  for (i = true_visible; i < NBSCREENS; i++) {
    gtk_widget_hide (screen_buttons.screen_frame[i]);
  }
  gtk_widget_set_usize (screen_buttons.gxfce_central_frame, 80 + 40 * true_visible, 0);
  update_config_screen_visible(true_visible);
}

void
update_gxfce_popup_buttons (gint visible)
{
  gint i, true_visible;
  
  true_visible = ((visible > NBPOPUPS) ? NBPOPUPS : visible);
  for (i = 0; i < true_visible; i++) {
    gtk_widget_show (popup_buttons.popup_vbox[i]);
  }
  for (i = true_visible; i < NBPOPUPS; i++) {
    gtk_widget_hide (popup_buttons.popup_vbox[i]);
  }
}

GtkWidget*
create_gxfce (XFCE_palette *pal)
{
  GtkWidget *gxfce;
  GtkWidget *gxfce_mainframe;
  GtkWidget *gxfce_hbox1;
  GtkWidget *gxfce_hbox2;
  GtkWidget *gxfce_vbox1;
  GtkWidget *gxfce_hbox_moveleft;
  GtkWidget *gxfce_close_button;
  GtkWidget *gxfce_close_pixmap;
  GtkWidget *gxfce_move_frame_left;
  GtkWidget *gxfce_move_event_left;
  GtkWidget *gxfce_clock_frame;
  GtkTooltips *gxfce_clock_tooltip;
  GtkWidget *gxfce_central_vbox;
  GtkWidget *gxfce_central_hbox;
  GtkWidget *gxfce_central_vbox_left;
  GtkWidget *desktops_table;
  GtkWidget *gxfce_info;
  GtkWidget *gxfce_info_pixmap;
  GtkTooltips *gxfce_info_tooltip;
  GtkWidget *gxfce_central_vbox_right;
  GtkWidget *gxfce_setup;
  GtkWidget *gxfce_setup_pixmap;
  GtkTooltips *gxfce_setup_tooltip;
  GtkWidget *gxfce_quit;
  GtkWidget *gxfce_quit_label;
  GtkTooltips *gxfce_quit_tooltip;
  GtkWidget *gxfce_hbox3;
  GtkWidget *gxfce_hbox_moveright;
  GtkWidget *gxfce_move_frame_right;
  GtkWidget *gxfce_move_event_right;
  GtkWidget *gxfce_iconify_button;
  GtkWidget *gxfce_iconify_pixmap;

  int i;  
  int nbselects = NBSELECTS;

  gxfce = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_policy (GTK_WINDOW (gxfce), FALSE, FALSE, TRUE);
  gtk_widget_set_name (gxfce, "gxfce");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce", gxfce);
  gtk_window_set_title (GTK_WINDOW (gxfce), "XFce Main Panel");
  gtk_widget_realize (gxfce);
  gdk_window_set_decorations (gxfce->window, 0);
  /* gtk_widget_set_style(gxfce, pal->cm[7]);  */

  gxfce_mainframe = gtk_frame_new (NULL);
  gtk_widget_set_name (gxfce_mainframe, "gxfce_mainframe");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_mainframe", gxfce_mainframe);
  gtk_widget_show (gxfce_mainframe);
  gtk_container_add (GTK_CONTAINER (gxfce), gxfce_mainframe);
  gtk_frame_set_shadow_type (GTK_FRAME (gxfce_mainframe), GTK_SHADOW_OUT);
  /* gtk_widget_set_style(gxfce_mainframe, pal->cm[7]); */
  
  gxfce_hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (gxfce_hbox1, "gxfce_hbox1");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_hbox1", gxfce_hbox1);
  gtk_widget_show (gxfce_hbox1);
  gtk_container_add (GTK_CONTAINER (gxfce_mainframe), gxfce_hbox1);
  gtk_widget_set_usize (gxfce_hbox1, 0, 0);
  gtk_container_border_width (GTK_CONTAINER (gxfce_hbox1), 2);

  gxfce_hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (gxfce_hbox2, "gxfce_hbox2");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_hbox2", gxfce_hbox2);
  gtk_widget_show (gxfce_hbox2);
  gtk_box_pack_start (GTK_BOX (gxfce_hbox1), gxfce_hbox2, TRUE, TRUE, 0);

  gxfce_vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (gxfce_vbox1, "gxfce_vbox1");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_vbox1", gxfce_vbox1);
  gtk_widget_show (gxfce_vbox1);
  gtk_box_pack_start (GTK_BOX (gxfce_hbox2), gxfce_vbox1, TRUE, TRUE, 0);

  gxfce_hbox_moveleft = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (gxfce_hbox_moveleft, "gxfce_hbox_moveleft");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_hbox_moveleft", gxfce_hbox_moveleft);
  gtk_widget_show (gxfce_hbox_moveleft);
  gtk_widget_set_usize (gxfce_hbox_moveleft, 0, 0);

  gxfce_close_button = gtk_button_new ();
  gtk_widget_set_name (gxfce_close_button, "gxfce_close_button");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_close_button", gxfce_close_button);
  gtk_widget_show (gxfce_close_button);
  gtk_box_pack_start (GTK_BOX (gxfce_hbox_moveleft), gxfce_close_button, TRUE, TRUE, 0);
  gtk_widget_set_usize (gxfce_close_button, 16, 16);
  gtk_widget_set_style(gxfce_close_button, pal->cm[1]);

  gxfce_close_pixmap = MyCreateFromPixmapData (gxfce_close_button, closepix);
  if (gxfce_close_pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_set_name (gxfce_close_pixmap, "gxfce_close_pixmap");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_close_pixmap", gxfce_close_pixmap);
  gtk_widget_show (gxfce_close_pixmap);
  gtk_container_add (GTK_CONTAINER (gxfce_close_button), gxfce_close_pixmap);
  gtk_widget_set_style(gxfce_close_button, pal->cm[1]);

  gxfce_move_frame_left = gtk_frame_new (NULL);
  gtk_widget_set_name (gxfce_move_frame_left, "gxfce_move_frame_left");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_move_frame_left", gxfce_move_frame_left);
  gtk_widget_show (gxfce_move_frame_left);
  gtk_box_pack_start (GTK_BOX (gxfce_hbox_moveleft), gxfce_move_frame_left, TRUE, TRUE, 0);
  gtk_widget_set_usize (gxfce_move_frame_left, 0, 16);
  gtk_frame_set_shadow_type (GTK_FRAME (gxfce_move_frame_left), GTK_SHADOW_OUT);
  gtk_widget_set_style(gxfce_move_frame_left, pal->cm[1]);

  gxfce_move_event_left = gtk_event_box_new ();
  gtk_widget_set_name (gxfce_move_event_left, "gxfce_move_event_left");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_move_event_left", gxfce_move_event_left);
  gtk_widget_show (gxfce_move_event_left);
  gtk_container_add (GTK_CONTAINER (gxfce_move_frame_left), gxfce_move_event_left);
  gtk_widget_set_style(gxfce_move_event_left, pal->cm[1]);

  gxfce_move_left_pixmap = MyCreateFromPixmapData (gxfce_move_event_left, handle);
  if (gxfce_move_left_pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_set_name (gxfce_move_left_pixmap, "gxfce_move_left_pixmap");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_move_left_pixmap", gxfce_move_left_pixmap);
  gtk_widget_show (gxfce_move_left_pixmap);
  gtk_container_add (GTK_CONTAINER (gxfce_move_event_left), gxfce_move_left_pixmap);
  gtk_widget_set_usize (gxfce_move_left_pixmap, PANEL_ICON_SIZE - 16 + 4, 16);
  gtk_widget_set_style(gxfce_move_left_pixmap, pal->cm[1]);

  gxfce_clock_event = gtk_event_box_new ();
  gtk_widget_set_name (gxfce_clock_event, "gxfce_clock_event");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_clock_event", gxfce_clock_event);
  gtk_container_border_width (GTK_CONTAINER (gxfce_clock_event), 5);
  gtk_widget_show (gxfce_clock_event);
  /* gtk_widget_set_style(gxfce_clock_event, pal->cm[7]); */

  gxfce_clock_frame = gtk_frame_new (NULL);
  gtk_widget_set_name (gxfce_clock_frame, "gxfce_clock_frame");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_clock_frame", gxfce_clock_frame);
  gtk_container_add (GTK_CONTAINER (gxfce_clock_event), gxfce_clock_frame);
  gtk_frame_set_shadow_type (GTK_FRAME(gxfce_clock_frame), GTK_SHADOW_ETCHED_IN);
  gtk_container_border_width (GTK_CONTAINER (gxfce_clock_frame), 0);
  gtk_widget_set_usize (gxfce_clock_frame, PANEL_ICON_SIZE, PANEL_ICON_SIZE);
  /* gtk_widget_set_style(gxfce_clock_frame, pal->cm[7]); */
  gtk_widget_show (gxfce_clock_frame);
  gxfce_clock_tooltip = my_tooltips_new(current_config.tooltipsdelay);
  gtk_tooltips_set_tip (gxfce_clock_tooltip,gxfce_clock_event,"", "ContextHelp/buttons/?");
  gtk_object_set_data (GTK_OBJECT (gxfce), "tooltips", gxfce_clock_tooltip);
  update_gxfce_date_timer (gxfce_clock_event);
  clock_tooltip_timer = gtk_timeout_add (30000 /* 30 secs */,
                                         (GtkFunction) update_gxfce_date_timer,
                                         (gpointer) gxfce_clock_event);

  gxfce_clock = my_gtk_clock_new();
  gtk_container_add (GTK_CONTAINER (gxfce_clock_frame), gxfce_clock);
  gtk_widget_set_style(gxfce_clock, pal->cm[2]);
  my_gtk_clock_set_relief (MY_GTK_CLOCK(gxfce_clock), FALSE);
  my_gtk_clock_show_secs(MY_GTK_CLOCK (gxfce_clock), FALSE);
  my_gtk_clock_show_ampm(MY_GTK_CLOCK (gxfce_clock), FALSE);
  gtk_widget_show (gxfce_clock);
  gxfce_clock_make_popup (MY_GTK_CLOCK(gxfce_clock));

  gtk_box_pack_start (GTK_BOX (gxfce_hbox2), 
                      create_gxfce_popup_select_group(gxfce, 
                                                      gxfce_hbox_moveleft,
						      gxfce_clock_event),
						      TRUE, TRUE, 0);

  /* The items on the left */

  for (i = 0; i < NBPOPUPS; i += 2) {
  gtk_box_pack_start (GTK_BOX (gxfce_hbox2), 
      popup_buttons.popup_vbox[i] = create_gxfce_popup_select_group(gxfce, 
                                      create_gxfce_popup_button(gxfce, pal, i),
				      create_gxfce_select_button(gxfce, pal, (( i == 6 ) ? NBPOPUPS : i))),
				      TRUE, TRUE, 0);
  }

  /* The central panel */

  gxfce_central_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (gxfce_central_vbox, "gxfce_central_vbox");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_central_vbox", gxfce_central_vbox);
  gtk_widget_show (gxfce_central_vbox);
  gtk_box_pack_start (GTK_BOX (gxfce_hbox1), gxfce_central_vbox, TRUE, TRUE, 0);
  /* gtk_widget_set_style(gxfce_central_vbox, pal->cm[7]); */

  screen_buttons.gxfce_central_frame = gtk_frame_new (NULL);
  gtk_widget_set_name (screen_buttons.gxfce_central_frame, "gxfce_central_frame");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_central_frame", screen_buttons.gxfce_central_frame);
  gtk_widget_show (screen_buttons.gxfce_central_frame);
  gtk_box_pack_start (GTK_BOX (gxfce_central_vbox), screen_buttons.gxfce_central_frame, TRUE, TRUE, 0);
  gtk_widget_set_usize (screen_buttons.gxfce_central_frame, 0, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (screen_buttons.gxfce_central_frame), GTK_SHADOW_OUT);
  /* gtk_widget_set_style(screen_buttons.gxfce_central_frame, pal->cm[7]); */

  gxfce_central_hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (gxfce_central_hbox, "gxfce_central_hbox");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_central_hbox", gxfce_central_hbox);
  gtk_widget_show (gxfce_central_hbox);
  gtk_container_add (GTK_CONTAINER (screen_buttons.gxfce_central_frame), gxfce_central_hbox);

  gxfce_central_vbox_left = gtk_vbox_new (TRUE, 0);
  gtk_widget_set_name (gxfce_central_vbox_left, "gxfce_central_vbox_left");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_central_vbox_left", gxfce_central_vbox_left);
  gtk_widget_show (gxfce_central_vbox_left);
  gtk_box_pack_start (GTK_BOX (gxfce_central_hbox), gxfce_central_vbox_left, FALSE, TRUE, 0);

  select_buttons.select_button[NBSELECTS] = select_buttons.select_button[NBSELECTS] = gtk_button_new ();
  gtk_widget_set_name (select_buttons.select_button[NBSELECTS], "gxfce_select_button");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_select_button", select_buttons.select_button[NBSELECTS]);
  gtk_button_set_relief((GtkButton *) select_buttons.select_button[NBSELECTS], GTK_RELIEF_NONE);
  gtk_widget_show (select_buttons.select_button[NBSELECTS]);
  gtk_widget_set_usize (select_buttons.select_button[NBSELECTS], 35, 28);
  gtk_box_pack_start (GTK_BOX (gxfce_central_vbox_left), select_buttons.select_button[NBSELECTS], FALSE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (select_buttons.select_button[NBSELECTS]), 2);
  /* gtk_widget_set_style(select_buttons.select_button[NBSELECTS], pal->cm[7]); */
  select_buttons.select_tooltips[NBSELECTS] = my_tooltips_new(current_config.tooltipsdelay);
  gtk_tooltips_force_window(select_buttons.select_tooltips[NBSELECTS]);
  gtk_tooltips_set_tip (select_buttons.select_tooltips[NBSELECTS],select_buttons.select_button[NBSELECTS], "None", "ContextHelp/buttons/?");
  gtk_object_set_data (GTK_OBJECT (gxfce), "tooltips", select_buttons.select_tooltips[NBSELECTS]);

  select_buttons.select_pixmap[NBSELECTS] = MyCreateFromPixmapData (select_buttons.select_button[NBSELECTS], minilock);
  if (select_buttons.select_pixmap[NBSELECTS] == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_set_name (select_buttons.select_pixmap[NBSELECTS], "gxfce_select_button_pixmap");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_select_button_pixmap", select_buttons.select_pixmap[NBSELECTS]);
  gtk_widget_show (select_buttons.select_pixmap[NBSELECTS]);
  gtk_container_add (GTK_CONTAINER (select_buttons.select_button[NBSELECTS]), select_buttons.select_pixmap[NBSELECTS]);
  /* gtk_widget_set_style(select_buttons.select_pixmap[NBSELECTS], pal->cm[7]); */

  gxfce_info = gtk_button_new ();
  gtk_widget_set_name (gxfce_info, "gxfce_info");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_info", gxfce_info);
  gtk_button_set_relief((GtkButton *) gxfce_info, GTK_RELIEF_NONE);
  gtk_widget_set_usize (gxfce_info, 35, 28);
  gtk_widget_show (gxfce_info);
  gtk_box_pack_start (GTK_BOX (gxfce_central_vbox_left), gxfce_info, FALSE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (gxfce_info), 2);
  /* gtk_widget_set_style(gxfce_info, pal->cm[7]); */

  gxfce_info_tooltip = my_tooltips_new(current_config.tooltipsdelay);
  gtk_tooltips_set_tip (gxfce_info_tooltip, gxfce_info, _("About..."), "ContextHelp/buttons/?");
  gtk_object_set_data (GTK_OBJECT (gxfce), "tooltips", gxfce_info_tooltip);

  gxfce_info_pixmap = MyCreateFromPixmapData (gxfce_info, mininf);
  if (gxfce_info_pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_set_name (gxfce_info_pixmap, "gxfce_info_pixmap");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_info_pixmap", gxfce_info_pixmap);
  gtk_widget_show (gxfce_info_pixmap);
  gtk_container_add (GTK_CONTAINER (gxfce_info), gxfce_info_pixmap);
  /* gtk_widget_set_style(gxfce_info_pixmap, pal->cm[7]); */

  desktops_table = create_gxfce_screen_buttons(gxfce, pal);
  gtk_widget_set_name (desktops_table, "desktops_table");
  gtk_object_set_data (GTK_OBJECT (gxfce), "desktops_table", desktops_table);
  gtk_widget_show (desktops_table);
  gtk_box_pack_start (GTK_BOX (gxfce_central_hbox), desktops_table, TRUE, TRUE, 0);

  gxfce_central_vbox_right = gtk_vbox_new (TRUE, 0);
  gtk_widget_set_name (gxfce_central_vbox_right, "gxfce_central_vbox_right");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_central_vbox_right", gxfce_central_vbox_right);
  gtk_widget_show (gxfce_central_vbox_right);
  gtk_box_pack_start (GTK_BOX (gxfce_central_hbox), gxfce_central_vbox_right, FALSE, TRUE, 0);

  gxfce_setup = gtk_button_new ();
  gtk_widget_set_name (gxfce_setup, "gxfce_setup");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_setup", gxfce_setup);
  gtk_button_set_relief((GtkButton *) gxfce_setup, GTK_RELIEF_NONE);
  gtk_widget_set_usize (gxfce_setup, 35, 28);
  gtk_widget_show (gxfce_setup);
  gtk_box_pack_start (GTK_BOX (gxfce_central_vbox_right), gxfce_setup, FALSE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (gxfce_setup), 2);
  /* gtk_widget_set_style(gxfce_setup, pal->cm[7]); */

  gxfce_setup_tooltip = my_tooltips_new(current_config.tooltipsdelay);
  gtk_tooltips_set_tip (gxfce_setup_tooltip, gxfce_setup, _("Setup..."), "ContextHelp/buttons/?");
  gtk_object_set_data (GTK_OBJECT (gxfce), "tooltips", gxfce_setup_tooltip);

  gxfce_setup_pixmap = MyCreateFromPixmapData (gxfce_setup, minipnt);
  if (gxfce_setup_pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_set_name (gxfce_setup_pixmap, "gxfce_setup_pixmap");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_setup_pixmap", gxfce_setup_pixmap);
  gtk_widget_show (gxfce_setup_pixmap);
  gtk_container_add (GTK_CONTAINER (gxfce_setup), gxfce_setup_pixmap);
  /* gtk_widget_set_style(gxfce_setup_pixmap, pal->cm[7]); */

  gxfce_quit = gtk_button_new ();
  gtk_widget_set_name (gxfce_quit, "gxfce_quit");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_quit", gxfce_quit);
  gtk_button_set_relief((GtkButton *) gxfce_quit, GTK_RELIEF_NONE);
  gtk_widget_show (gxfce_quit);
  gtk_box_pack_start (GTK_BOX (gxfce_central_vbox_right), gxfce_quit, FALSE, FALSE, 0);
  gtk_container_border_width (GTK_CONTAINER (gxfce_quit), 2);
  /* gtk_widget_set_style(gxfce_quit, pal->cm[7]); */

  gxfce_quit_tooltip = my_tooltips_new(current_config.tooltipsdelay);
  gtk_tooltips_set_tip (gxfce_quit_tooltip, gxfce_quit, _("Whoooops !"), "ContextHelp/buttons/?");
  gtk_object_set_data (GTK_OBJECT (gxfce), "tooltips", gxfce_quit_tooltip);

  gxfce_quit_label = gtk_accel_label_new (_("Quit"));
  gtk_widget_set_name (gxfce_quit_label, "gxfce_quit_label");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_quit_label", gxfce_quit_label);
  gtk_widget_show (gxfce_quit_label);
  gtk_container_add (GTK_CONTAINER (gxfce_quit), gxfce_quit_label);

  gxfce_hbox3 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (gxfce_hbox3, "gxfce_hbox3");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_hbox3", gxfce_hbox3);
  gtk_widget_show (gxfce_hbox3);
  gtk_box_pack_start (GTK_BOX (gxfce_hbox1), gxfce_hbox3, TRUE, TRUE, 0);


  /* The items on the right */

  for (i = NBPOPUPS - 1; i > 0; i -= 2) {
  gtk_box_pack_start (GTK_BOX (gxfce_hbox3), 
     popup_buttons.popup_vbox[i] = create_gxfce_popup_select_group(gxfce, 
  				     create_gxfce_popup_button(gxfce, pal, i),
        			     create_gxfce_select_button(gxfce, pal, i)),
        			     TRUE, TRUE, 0);
  }

  gxfce_hbox_moveright = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (gxfce_hbox_moveright, "gxfce_hbox_moveright");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_hbox_moveright", gxfce_hbox_moveright);
  gtk_widget_show (gxfce_hbox_moveright);
  gtk_widget_set_usize (gxfce_hbox_moveright, 0, 0);

  gxfce_move_frame_right = gtk_frame_new (NULL);
  gtk_widget_set_name (gxfce_move_frame_right, "gxfce_move_frame_right");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_move_frame_right", gxfce_move_frame_right);
  gtk_widget_show (gxfce_move_frame_right);
  gtk_box_pack_start (GTK_BOX (gxfce_hbox_moveright), gxfce_move_frame_right, TRUE, TRUE, 0);
  gtk_widget_set_usize (gxfce_move_frame_right, 0, 16);
  gtk_frame_set_shadow_type (GTK_FRAME (gxfce_move_frame_right), GTK_SHADOW_OUT);
  gtk_widget_set_style(gxfce_move_frame_right, pal->cm[1]);

  gxfce_move_event_right = gtk_event_box_new ();
  gtk_widget_set_name (gxfce_move_event_right, "gxfce_move_event_right");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_move_event_right", gxfce_move_event_right);
  gtk_widget_show (gxfce_move_event_right);
  gtk_container_add (GTK_CONTAINER (gxfce_move_frame_right), gxfce_move_event_right);
  gtk_widget_set_style(gxfce_move_event_right, pal->cm[1]);

  gxfce_move_right_pixmap = MyCreateFromPixmapData (gxfce_move_event_right, handle);
  if (gxfce_move_right_pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_set_name (gxfce_move_right_pixmap, "gxfce_move_right_pixmap");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_move_right_pixmap", gxfce_move_right_pixmap);
  gtk_widget_show (gxfce_move_right_pixmap);
  gtk_container_add (GTK_CONTAINER (gxfce_move_event_right), gxfce_move_right_pixmap);
  gtk_widget_set_usize (gxfce_move_right_pixmap, PANEL_ICON_SIZE - 16 + 4, 16);
  gtk_widget_set_style(gxfce_move_right_pixmap, pal->cm[1]);

  gxfce_iconify_button = gtk_button_new ();
  gtk_widget_set_name (gxfce_iconify_button, "gxfce_iconify_button");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_iconify_button", gxfce_iconify_button);
  gtk_widget_show (gxfce_iconify_button);
  gtk_box_pack_start (GTK_BOX (gxfce_hbox_moveright), gxfce_iconify_button, TRUE, TRUE, 0);
  gtk_widget_set_usize (gxfce_iconify_button, 16, 16);
  gtk_widget_set_style(gxfce_iconify_button, pal->cm[1]);

  gxfce_iconify_pixmap = MyCreateFromPixmapData (gxfce_iconify_button, iconify);
  if (gxfce_iconify_pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_set_name (gxfce_iconify_pixmap, "gxfce_iconify_pixmap");
  gtk_object_set_data (GTK_OBJECT (gxfce), "gxfce_iconify_pixmap", gxfce_iconify_pixmap);
  gtk_widget_show (gxfce_iconify_pixmap);
  gtk_container_add (GTK_CONTAINER (gxfce_iconify_button), gxfce_iconify_pixmap);
  gtk_widget_set_style(gxfce_iconify_pixmap, pal->cm[1]);
  gtk_box_pack_start (GTK_BOX (gxfce_hbox3), 
                      create_gxfce_popup_select_group(gxfce, 
                                                      gxfce_hbox_moveright,
						      create_gxfce_select_button(gxfce, pal, 6)),
						      TRUE, TRUE, 0);
  /* Signal handling */

  gtk_signal_connect (GTK_OBJECT (gxfce_iconify_button), "clicked",
                      GTK_SIGNAL_FUNC (iconify_cb), gxfce);

  gtk_signal_connect (GTK_OBJECT (gxfce), "delete_event",
                          GTK_SIGNAL_FUNC (delete_event), NULL);

  gtk_signal_connect (GTK_OBJECT (gxfce), "destroy",
                      GTK_SIGNAL_FUNC (destroy_cb), NULL);


  gtk_signal_connect  (GTK_OBJECT (gxfce_quit), "clicked",
                       GTK_SIGNAL_FUNC (quit_cb),
                       GTK_OBJECT (gxfce));

  gtk_signal_connect  (GTK_OBJECT (gxfce_close_button), "clicked",
                       GTK_SIGNAL_FUNC (quit_cb),
                       GTK_OBJECT (gxfce));

  gtk_signal_connect (GTK_OBJECT (gxfce_setup), "clicked",
                      GTK_SIGNAL_FUNC (setup_cb),
                      pal);

  gtk_signal_connect (GTK_OBJECT (gxfce_info), "clicked",
                      GTK_SIGNAL_FUNC (info_cb),
                      GTK_OBJECT (gxfce));

  gtk_signal_connect (GTK_OBJECT(select_buttons.select_button[NBSELECTS]), "button_press_event",
                      GTK_SIGNAL_FUNC(select_modify_cb),
		      (gpointer) nbselects);

  gtk_signal_connect (GTK_OBJECT (select_buttons.select_button[NBSELECTS]), "clicked",
                      GTK_SIGNAL_FUNC (select_cb),
                      (gpointer) nbselects);
 
  gtk_signal_connect (GTK_OBJECT (gxfce_clock_event), "button_press_event",
                      GTK_SIGNAL_FUNC (gxfce_clock_show_popup_cb),
                      (gpointer) gxfce_clock);

  gtk_signal_connect (GTK_OBJECT (gxfce_move_event_left), "button_press_event",
                      GTK_SIGNAL_FUNC (move_event_pressed),
		      gxfce_clock);

  create_move_button (gxfce_move_event_left, gxfce);

  gtk_signal_connect (GTK_OBJECT (gxfce_move_event_left), "button_release_event",
                      GTK_SIGNAL_FUNC (move_event_released),
		      gxfce_clock);

  gtk_signal_connect (GTK_OBJECT (gxfce_move_event_right), "button_press_event",
                      GTK_SIGNAL_FUNC (move_event_pressed),
		      gxfce_clock);

  create_move_button (gxfce_move_event_right, gxfce);

  gtk_signal_connect (GTK_OBJECT (gxfce_move_event_right), "button_release_event",
                      GTK_SIGNAL_FUNC (move_event_released),
		      gxfce_clock);

  set_icon (gxfce, "XFce Main Panel", xfce_icon);

  return gxfce;
}
