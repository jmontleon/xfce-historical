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

#ifndef __MODIFY_CB_H__
#define __MODIFY_CB_H__

#include <gtk/gtk.h>

void modify_cancel_cb( GtkWidget *widget,
                     gpointer   data );

gboolean
modify_delete_event( GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   data );
		   
void modify_browse_command_cb( GtkWidget *widget,
                               gpointer   data );

void modify_browse_icon_cb( GtkWidget *widget,
                            gpointer   data );

void modify_remove_cb( GtkWidget *widget,
                            gpointer   data );

void modify_change_cb( GtkWidget *widget,
                            gpointer   data );

void modify_add_cb( GtkWidget *widget,
                            gpointer   data );

#endif
