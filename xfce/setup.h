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

#ifndef __SETUP_H__
#define __SETUP_H__

#include "xfcolor.h"
#include <gtk/gtk.h>

typedef struct
  {
    GtkWidget *setup_color_button[NB_XFCE_COLORS];
    GtkWidget *setup_color_button_frame[NB_XFCE_COLORS];
  }
color_button;

typedef struct
  {
    GtkWidget *setup_palette_load_button;
    GtkWidget *setup_palette_save_button;
    GtkWidget *setup_palette_default_button;
    GtkWidget *setup_repaint_checkbutton;
    GtkWidget *setup_gradient_checkbutton;
    GtkWidget *setup_tearoff_checkbutton;
    GtkObject *setup_numscreens_spinbutton_adj;
    GtkWidget *setup_numscreens_spinbutton;
    GtkObject *setup_numpopups_spinbutton_adj;
    GtkWidget *setup_numpopups_spinbutton;
    GtkWidget *setup_panelicons_large;
    GtkWidget *setup_panelicons_medium;
    GtkWidget *setup_panelicons_small;
    GtkWidget *setup_popupicons_large;
    GtkWidget *setup_popupicons_medium;
    GtkWidget *setup_popupicons_small;
    GtkWidget *setup_font_xfce_entry;
    GtkWidget *setup_font_xfce_font_button;
    GtkWidget *setup_focusmode_checkbutton;
    GtkWidget *setup_autoraise_checkbutton;
    GtkWidget *setup_opaquemove_checkbutton;
    GtkWidget *setup_gradient_activetitle;
    GtkWidget *setup_gradient_inactivetitle;
    GtkWidget *setup_iconpos_topbutton;
    GtkWidget *setup_iconpos_leftbutton;
    GtkWidget *setup_iconpos_botbutton;
    GtkWidget *setup_iconpos_rightbutton;
    GtkWidget *setup_font_title_entry;
    GtkWidget *setup_font_icon_entry;
    GtkWidget *setup_font_menu_entry;
    GtkWidget *setup_font_title_button;
    GtkWidget *setup_font_icon_button;
    GtkWidget *setup_font_menu_button;
    GtkWidget *setup_ok_button;
    GtkWidget *setup_apply_button;
    GtkWidget *setup_cancel_button;
  }
setup_option;

XFCE_palette * temp_pal;

color_button  color_buttons;
setup_option setup_options;

int prev_panel_icon_size;
int prev_popup_icon_size;
int prev_visible_screen;
int prev_visible_popup;
gboolean pal_changed;

void
apply_pal_colortable (XFCE_palette * pal);

GtkWidget*
create_setup_colortable (GtkWidget *toplevel, XFCE_palette * pal);

GtkWidget* 
create_setup (XFCE_palette * pal);

void
show_setup (XFCE_palette * pal);

void
get_setup_values(void);

#endif
