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
#include <string.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "my_intl.h"
#include "xfcolor.h"
#include "setup.h"
#include "setup_cb.h"
#include "colorselect.h"
#include "fileselect.h"
#include "xfce.h"
#include "xfce_cb.h"
#include "popup.h"
#include "xfce_main.h"
#include "xfce-common.h"
#include "xpmext.h"
#include "fontselection.h"
#include "configfile.h"
#include "xfwm.h"
#include "constant.h"

void color_button_cb( GtkWidget *widget,
                     gpointer   data )
{
  open_colorselect(temp_pal, (int) data);
  makepalstyle(temp_pal, (int) data);
  apply_pal_colortable (temp_pal);
  pal_changed = TRUE;
}

void setup_ok_cb( GtkWidget *widget,
                     gpointer   data )
{
  cursor_wait(setup);
  if (pal_changed) {
    copyvaluepal(pal, temp_pal);
    savepal(pal);
    create_gtkrc_file(pal);
    applypal (pal, gxfce);
    applypal_to_all();
  }
  get_setup_values();
  if (prev_visible_screen != current_config.visible_screen)
    update_gxfce_screen_buttons(current_config.visible_screen);
  if (current_config.colorize_root)
    ApplyRootColor (pal, (current_config.gradient_root != 0), get_screen_color(get_current_screen()));
  if (prev_visible_popup != current_config.visible_popup)
    update_gxfce_popup_buttons(current_config.visible_popup);
  if (prev_panel_icon_size != current_config.select_icon_size)
    update_gxfce_size();
  if (prev_popup_icon_size != current_config.popup_icon_size)
    update_popup_size();
  if (current_config.wm == XFWM) {
    apply_wm_colors(pal);
    apply_wm_fonts();
    apply_wm_iconpos();
    apply_wm_options();
  }
  while (repaint_in_progress());
  my_flush_events ();
  gtk_main_quit ();
  cursor_reset(setup);
  gtk_widget_hide (setup);
  writeconfig();
}

void setup_apply_cb( GtkWidget *widget,
                     gpointer   data )
{
  cursor_wait(setup);
  if (pal_changed) {
    copyvaluepal(pal, temp_pal);
    savepal(pal);
    create_gtkrc_file(pal);
    applypal (pal, gxfce);
    applypal_to_all();
  }
  get_setup_values();
  if (prev_visible_screen != current_config.visible_screen)
    update_gxfce_screen_buttons(current_config.visible_screen);
  if (current_config.colorize_root)
    ApplyRootColor (pal, (current_config.gradient_root != 0), get_screen_color(get_current_screen()));
  if (prev_visible_popup != current_config.visible_popup)
    update_gxfce_popup_buttons(current_config.visible_popup);
  if (prev_panel_icon_size != current_config.select_icon_size)
    update_gxfce_size();
  if (prev_popup_icon_size != current_config.popup_icon_size)
    update_popup_size();
  if (current_config.wm == XFWM) {
    apply_wm_colors(pal);
    apply_wm_fonts();
    apply_wm_iconpos();
    apply_wm_options();
  }
  pal_changed = FALSE;
  prev_panel_icon_size = current_config.select_icon_size;
  prev_popup_icon_size = current_config.popup_icon_size;
  prev_visible_screen = current_config.visible_screen;
  prev_visible_popup = current_config.visible_popup;
  writeconfig();
  cursor_reset(setup);
}

void setup_cancel_cb( GtkWidget *widget,
                     gpointer   data )
{
  while (repaint_in_progress());
  my_flush_events ();
  gtk_main_quit ();
  gtk_widget_hide (setup);
}

gboolean
setup_delete_event( GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   data )
{
  setup_cancel_cb (widget,data);
  return (TRUE);
}

void setup_default_cb( GtkWidget *widget,
                     gpointer   data )
{
  int i;
  defpal (temp_pal);
  for (i = 0; i < NB_XFCE_COLORS; i++)
    makepalstyle(temp_pal, i);
  apply_pal_colortable (temp_pal);
  pal_changed = TRUE;
}

void setup_loadpal_cb( GtkWidget *widget,
                       gpointer   data )
{
  char *t;
  int i;
  
  t = open_fileselect (build_path(XFCE_PAL));
  if (t) loadnamepal(temp_pal, t);
  for (i = 0; i < NB_XFCE_COLORS; i++)
    makepalstyle(temp_pal, i);
  apply_pal_colortable (temp_pal);
  pal_changed = TRUE;
}

void setup_savepal_cb( GtkWidget *widget,
                       gpointer   data )
{
  char *t;
  
  t = open_fileselect (build_path(XFCE_PAL));
  if (t) savenamepal(temp_pal, t);
}

void xfce_font_cb( GtkWidget *widget,
                       gpointer   data )
{
  char *s = NULL;
  s = open_fontselection (gtk_entry_get_text(GTK_ENTRY(setup_options.setup_font_xfce_entry)));
  if ((s) && strlen(s)) {
      gtk_entry_set_text(GTK_ENTRY(setup_options.setup_font_xfce_entry), s);
      set_font(temp_pal, s);
      pal_changed = TRUE;
    }
}

void toggle_repaint_checkbutton_cb( GtkWidget *widget,
                       gpointer   data )
{
  if (!(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (setup_options.setup_repaint_checkbutton))))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (setup_options.setup_gradient_checkbutton), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET (setup_options.setup_gradient_checkbutton), 
                           (DEFAULT_DEPTH >= 8) 
			     && (gtk_toggle_button_get_active(
			           GTK_TOGGLE_BUTTON (setup_options.setup_repaint_checkbutton))));
}

void toggle_focusmode_checkbutton_cb( GtkWidget *widget,
                       gpointer   data )
{
  if ((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (setup_options.setup_focusmode_checkbutton))))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (setup_options.setup_autoraise_checkbutton), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET (setup_options.setup_autoraise_checkbutton), 
                           (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (setup_options.setup_focusmode_checkbutton))));

}

void xfwm_titlefont_cb( GtkWidget *widget,
                       gpointer   data )
{
  char *s = NULL;
  s = open_fontselection (gtk_entry_get_text(GTK_ENTRY(setup_options.setup_font_title_entry)));
  if ((s) && strlen(s))
    gtk_entry_set_text(GTK_ENTRY(setup_options.setup_font_title_entry), s);
}

void xfwm_menufont_cb( GtkWidget *widget,
                       gpointer   data )
{
  char *s = NULL;
  s = open_fontselection (gtk_entry_get_text(GTK_ENTRY(setup_options.setup_font_menu_entry)));
  if ((s) && strlen(s))
    gtk_entry_set_text(GTK_ENTRY(setup_options.setup_font_menu_entry), s);
}

void xfwm_iconfont_cb( GtkWidget *widget,
                       gpointer   data )
{
  char *s = NULL;
  s = open_fontselection (gtk_entry_get_text(GTK_ENTRY(setup_options.setup_font_icon_entry)));
  if ((s) && strlen(s))
    gtk_entry_set_text(GTK_ENTRY(setup_options.setup_font_icon_entry), s);
}

