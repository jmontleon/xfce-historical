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

#ifndef __XFCE_H__
#define __XFCE_H__

#include "xfcolor.h"

typedef struct
  {
    GtkWidget *gxfce_central_frame;
    GtkWidget *screen_frame[NBSCREENS];
    GtkWidget *screen_button[NBSCREENS];
    GtkWidget *screen_label[NBSCREENS];
  }
XFCE_screen_buttons;

typedef struct
  {
    GtkWidget *popup_vbox[NBPOPUPS];
    GtkWidget *popup_button[NBPOPUPS];
    GtkWidget *popup_pixmap[NBPOPUPS];
  }
XFCE_popup_buttons;

typedef struct
  {
    GtkTooltips *select_tooltips[NBSELECTS + 1];
    GtkWidget *select_button[NBSELECTS + 1];
    GtkWidget *select_pixmap[NBSELECTS + 1];
  }
XFCE_select_buttons;

XFCE_screen_buttons screen_buttons;
XFCE_popup_buttons popup_buttons;
XFCE_select_buttons select_buttons;

GtkWidget *gxfce_move_left_pixmap;
GtkWidget *gxfce_move_right_pixmap;
GtkWidget *gxfce_clock_event;
guint clock_tooltip_timer;

extern char *screen_names[];

void       update_gxfce_coord(GtkWidget *, int *, int *);
void       update_gxfce_size(void);
char *     get_gxfce_screen_label(int);
void       set_gxfce_screen_label(int, char *);
int        get_screen_color(int);
GtkWidget* create_gxfce_screen_buttons (GtkWidget *, XFCE_palette *);
void       update_gxfce_screen_buttons (gint);
void       update_gxfce_popup_buttons (gint);

GtkWidget* create_gxfce (XFCE_palette *);

#endif
