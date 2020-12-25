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

#ifndef __ACTION_H__
#define __ACTION_H__
#include "xfcolor.h"

  GtkWidget *action_icon_optionmenu;
  GtkWidget *action_icon_optionmenu_menu;
  GtkWidget *action_command_entry;
  GtkWidget *action_icon_label;
  GtkWidget *action_icon_browse_button;
  GtkWidget *action_ok_button;

  guint action_signal_id1;
  guint action_signal_id2;

GtkWidget* create_action (XFCE_palette *);
GtkWidget * action_addto_choice(char *name);
int action_get_choice_selected(void);
void action_set_choice_selected(int index);
void open_action(GtkWidget *action, int nbr);

#endif
