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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "my_intl.h"
#include "constant.h"
#include "xfcolor.h"
#include "xfce_main.h"
#include "xfce.h"
#include "xfce_cb.h"
#include "xfce-common.h"
#include "popup.h"
#include "xpmext.h"
#include "popup_cb.h"
#include "configfile.h"
#include "my_tooltips.h"

#include "menu_icon.h"
#include "addiconpix.h"
#include "defaulticon.h"


#define POPUP_ICON_SIZE ((current_config.popup_icon_size == 0) ? SMALL_POPUP_ICONS : \
                           ((current_config.popup_icon_size == 2) ? LARGE_POPUP_ICONS : MEDIUM_POPUP_ICONS))

enum {
  TARGET_STRING,
  TARGET_ROOTWIN,
  TARGET_URL
};

static GtkTargetEntry popup_target_table[] = {
  { "STRING",     0, TARGET_STRING },
  { "text/plain", 0, TARGET_STRING },
  { "text/uri-list", 0, TARGET_URL },
};

static guint n_popup_targets = sizeof(popup_target_table) / sizeof(popup_target_table[0]);

GtkWidget*
create_popup_item (GtkWidget *toplevel, XFCE_palette * pal, gint nbr_menu, gint nbr_item)
{
  GtkWidget *hbox;
  
  popup_menus[nbr_menu].popup_buttons[nbr_item].label   = (char *) malloc (sizeof (char) * MAXSTRLEN + 1);
  popup_menus[nbr_menu].popup_buttons[nbr_item].pixfile = (char *) malloc (sizeof (char) * MAXSTRLEN + 1);
  popup_menus[nbr_menu].popup_buttons[nbr_item].command = (char *) malloc (sizeof (char) * MAXSTRLEN + 1);

  strcpy (popup_menus[nbr_menu].popup_buttons[nbr_item].label,   "Empty");
  strcpy (popup_menus[nbr_menu].popup_buttons[nbr_item].pixfile, "Default");
  strcpy (popup_menus[nbr_menu].popup_buttons[nbr_item].command, "");

  popup_menus[nbr_menu].popup_buttons[nbr_item].item_button = gtk_button_new ();
  gtk_widget_set_name (popup_menus[nbr_menu].popup_buttons[nbr_item].item_button, "popup_button");
  gtk_object_set_data (GTK_OBJECT (toplevel), "popup_button", popup_menus[nbr_menu].popup_buttons[nbr_item].item_button);
  gtk_button_set_relief((GtkButton *) popup_menus[nbr_menu].popup_buttons[nbr_item].item_button, GTK_RELIEF_NONE);
  /* gtk_widget_set_style(popup_menus[nbr_menu].popup_buttons[nbr_item].item_button, pal->cm[7]); */
  gtk_widget_show (popup_menus[nbr_menu].popup_buttons[nbr_item].item_button);

  popup_menus[nbr_menu].popup_buttons[nbr_item].item_tooltip = my_tooltips_new();
  gtk_tooltips_set_tip (popup_menus[nbr_menu].popup_buttons[nbr_item].item_tooltip,
                        popup_menus[nbr_menu].popup_buttons[nbr_item].item_button,
			popup_menus[nbr_menu].popup_buttons[nbr_item].command,
			"ContextHelp/buttons/?");
  gtk_object_set_data (GTK_OBJECT (toplevel), "tooltips", popup_menus[nbr_menu].popup_buttons[nbr_item].item_tooltip);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox, "hbox");
  gtk_object_set_data (GTK_OBJECT (toplevel), "hbox", hbox);
  /* gtk_widget_set_style(hbox, pal->cm[7]); */
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (popup_menus[nbr_menu].popup_buttons[nbr_item].item_button), hbox);

  popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap_frame = gtk_frame_new (NULL);
  gtk_widget_set_name (popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap_frame, "frame");
  gtk_object_set_data (GTK_OBJECT (toplevel), "frame", popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap_frame);
  /* gtk_widget_set_style(popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap_frame, pal->cm[7]); */
  gtk_widget_show (popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap_frame);
  gtk_box_pack_start (GTK_BOX (hbox), popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap_frame, FALSE, TRUE, 0);
  gtk_widget_set_usize (popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap_frame, POPUP_ICON_SIZE, POPUP_ICON_SIZE);
  gtk_frame_set_shadow_type (GTK_FRAME (popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap_frame), GTK_SHADOW_IN);

  popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap = MyCreateFromPixmapData (popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap_frame, defaulticon);
  if (popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_set_name (popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap, "popup_pixmap");
  gtk_object_set_data (GTK_OBJECT (toplevel), "popup_pixmap", popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap);
  /* gtk_widget_set_style(popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap, pal->cm[7]); */
  gtk_widget_show (popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap);
  gtk_container_add (GTK_CONTAINER (popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap_frame), popup_menus[nbr_menu].popup_buttons[nbr_item].item_pixmap);

  popup_menus[nbr_menu].popup_buttons[nbr_item].item_label = gtk_label_new (popup_menus[nbr_menu].popup_buttons[nbr_item].label);
  gtk_widget_set_name (popup_menus[nbr_menu].popup_buttons[nbr_item].item_label, "popup_label");
  gtk_object_set_data (GTK_OBJECT (toplevel), "popup_label", popup_menus[nbr_menu].popup_buttons[nbr_item].item_label);
  /* gtk_widget_set_style(popup_menus[nbr_menu].popup_buttons[nbr_item].item_label, pal->cm[7]); */
  gtk_widget_show (popup_menus[nbr_menu].popup_buttons[nbr_item].item_label);
  gtk_box_pack_start (GTK_BOX (hbox), popup_menus[nbr_menu].popup_buttons[nbr_item].item_label, TRUE, TRUE, 0);
  gtk_widget_set_usize (popup_menus[nbr_menu].popup_buttons[nbr_item].item_label, 120, 0);
  gtk_label_set_justify (GTK_LABEL (popup_menus[nbr_menu].popup_buttons[nbr_item].item_label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (popup_menus[nbr_menu].popup_buttons[nbr_item].item_label), 0.1, 0.5);

  gtk_drag_dest_set (popup_menus[nbr_menu].popup_buttons[nbr_item].item_button,
		     GTK_DEST_DEFAULT_ALL,
		     popup_target_table, n_popup_targets,
		     GDK_ACTION_COPY | GDK_ACTION_MOVE);

  gtk_signal_connect (GTK_OBJECT (popup_menus[nbr_menu].popup_buttons[nbr_item].item_button), "drag_data_received",
		      GTK_SIGNAL_FUNC (popup_entry_drag_data_received), (gpointer) (nbr_menu * NBMAXITEMS + nbr_item));

  gtk_widget_set_events(popup_menus[nbr_menu].popup_buttons[nbr_item].item_button, 
                        gtk_widget_get_events (popup_menus[nbr_menu].popup_buttons[nbr_item].item_button) | GDK_BUTTON3_MASK);
  gtk_signal_connect( GTK_OBJECT(popup_menus[nbr_menu].popup_buttons[nbr_item].item_button), "button_press_event",
			    GTK_SIGNAL_FUNC(popup_entry_modify_cb),
        		    (gpointer) (nbr_menu * NBMAXITEMS + nbr_item));
  gtk_signal_connect (GTK_OBJECT (popup_menus[nbr_menu].popup_buttons[nbr_item].item_button), "clicked",
			   GTK_SIGNAL_FUNC (popup_entry_cb),
			   (gpointer) (nbr_menu * NBMAXITEMS + nbr_item));

  return popup_menus[nbr_menu].popup_buttons[nbr_item].item_button;
}

GtkWidget*
create_popup_menu (XFCE_palette *pal, gint nbr)
{
  GtkWidget *popup_mainframe;
  GtkWidget *popup_vbox;
  GtkWidget *popup_addicon_button;
  GtkTooltips *popup_addicon_tooltip;
  GtkWidget *popup_addicon_hbox;
  GtkWidget *popup_addicon_icon_pixmap;
  GtkWidget *popup_addicon_icon_label;
  GtkWidget *popup_tearoff_hseparator;
  GtkTooltips *popup_tearoff_tooltip;
  gint i;
  
  popup_menus[nbr].detach = FALSE;
  popup_menus[nbr].entries = 0;
  popup_menus[nbr].uposx = 0;
  popup_menus[nbr].uposy = 0; 
  popup_menus[nbr].lastpx = 0;
  popup_menus[nbr].lastpy = 0;

  popup_menus[nbr].popup_toplevel = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_policy (GTK_WINDOW (popup_menus[nbr].popup_toplevel), FALSE, FALSE, TRUE);
  gtk_widget_realize (popup_menus[nbr].popup_toplevel);
  gtk_widget_set_name (popup_menus[nbr].popup_toplevel, "popup");
  gtk_object_set_data (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "popup", popup_menus[nbr].popup_toplevel);
  gtk_window_set_title (GTK_WINDOW (popup_menus[nbr].popup_toplevel), "Popup Menu");
  /* gtk_widget_set_style(popup_menus[nbr].popup_toplevel, pal->cm[7]); */

  popup_mainframe = gtk_frame_new (NULL);
  gtk_widget_set_name (popup_mainframe, "popup_mainframe");
  gtk_object_set_data (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "popup_mainframe", popup_mainframe);
  /* gtk_widget_set_style(popup_mainframe, pal->cm[7]); */
  gtk_widget_show (popup_mainframe);
  gtk_container_add (GTK_CONTAINER (popup_menus[nbr].popup_toplevel), popup_mainframe);
  gtk_frame_set_shadow_type (GTK_FRAME (popup_mainframe), GTK_SHADOW_OUT);

  popup_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (popup_vbox, "popup_vbox");
  gtk_object_set_data (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "popup_vbox", popup_vbox);
  /* gtk_widget_set_style(popup_vbox, pal->cm[7]); */
  gtk_widget_show (popup_vbox);
  gtk_container_add (GTK_CONTAINER (popup_mainframe), popup_vbox);

  popup_menus[nbr].popup_addicon_frame = gtk_frame_new (NULL);
  gtk_widget_set_name (popup_menus[nbr].popup_addicon_frame, "popup_addicon_frame");
  gtk_object_set_data (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "popup_menus[nbr].popup_addicon_frame", popup_menus[nbr].popup_addicon_frame);
  /* gtk_widget_set_style(popup_menus[nbr].popup_addicon_frame, pal->cm[7]); */
  gtk_widget_show (popup_menus[nbr].popup_addicon_frame);
  gtk_box_pack_start (GTK_BOX (popup_vbox), popup_menus[nbr].popup_addicon_frame, FALSE, TRUE, 0);
  gtk_widget_set_usize (popup_menus[nbr].popup_addicon_frame, 0, 0);

  popup_addicon_button = gtk_button_new ();
  gtk_widget_set_name (popup_addicon_button, "popup_addicon_button");
  gtk_object_set_data (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "popup_addicon_button", popup_addicon_button);
  gtk_button_set_relief((GtkButton *) popup_addicon_button, GTK_RELIEF_NONE);
  /* gtk_widget_set_style(popup_addicon_button, pal->cm[7]); */
  gtk_widget_show (popup_addicon_button);
  gtk_container_add (GTK_CONTAINER (popup_menus[nbr].popup_addicon_frame), popup_addicon_button);

  popup_addicon_tooltip = my_tooltips_new();
  gtk_tooltips_set_tip (popup_addicon_tooltip, popup_addicon_button, _("Add a new entry..."), "ContextHelp/buttons/?");
  gtk_object_set_data (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "tooltips", popup_addicon_tooltip);

  popup_addicon_hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (popup_addicon_hbox, "popup_addicon_hbox");
  gtk_object_set_data (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "popup_addicon_hbox", popup_addicon_hbox);
  gtk_widget_show (popup_addicon_hbox);
  gtk_container_add (GTK_CONTAINER (popup_addicon_button), popup_addicon_hbox);

  popup_menus[nbr].popup_addicon_icon_frame = gtk_frame_new (NULL);
  gtk_widget_set_name (popup_menus[nbr].popup_addicon_icon_frame, "popup_addicon_icon_frame");
  gtk_object_set_data (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "popup_addicon_icon_frame", popup_menus[nbr].popup_addicon_icon_frame);
  /* gtk_widget_set_style(popup_menus[nbr].popup_addicon_icon_frame, pal->cm[7]); */
  gtk_widget_show (popup_menus[nbr].popup_addicon_icon_frame);
  gtk_box_pack_start (GTK_BOX (popup_addicon_hbox), popup_menus[nbr].popup_addicon_icon_frame, FALSE, TRUE, 0);
  gtk_widget_set_usize (popup_menus[nbr].popup_addicon_icon_frame, POPUP_ICON_SIZE, POPUP_ICON_SIZE);
  gtk_frame_set_shadow_type (GTK_FRAME (popup_menus[nbr].popup_addicon_icon_frame), GTK_SHADOW_IN);

  popup_addicon_icon_pixmap = MyCreateFromPixmapData (popup_menus[nbr].popup_addicon_icon_frame, addiconpix);
  if (popup_addicon_icon_pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_set_name (popup_addicon_icon_pixmap, "popup_addicon_icon_pixmap");
  gtk_object_set_data (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "popup_addicon_icon_pixmap", popup_addicon_icon_pixmap);
  /* gtk_widget_set_style(popup_addicon_icon_pixmap, pal->cm[7]); */
  gtk_widget_show (popup_addicon_icon_pixmap);
  gtk_container_add (GTK_CONTAINER (popup_menus[nbr].popup_addicon_icon_frame), popup_addicon_icon_pixmap);

  popup_addicon_icon_label = gtk_label_new (_("Add icon..."));
  gtk_widget_set_name (popup_addicon_icon_label, "popup_addicon_icon_label");
  gtk_object_set_data (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "popup_addicon_icon_label", popup_addicon_icon_label);
  /* gtk_widget_set_style(popup_addicon_icon_label, pal->cm[7]); */
  gtk_widget_show (popup_addicon_icon_label);
  gtk_box_pack_start (GTK_BOX (popup_addicon_hbox), popup_addicon_icon_label, TRUE, TRUE, 0);
  gtk_widget_set_usize (popup_addicon_icon_label, 120, -2);
  gtk_label_set_justify (GTK_LABEL (popup_addicon_icon_label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (popup_addicon_icon_label), 0.1, 0.5);

  for (i=0; i < NBMAXITEMS; i++)
    gtk_box_pack_start (GTK_BOX (popup_vbox), create_popup_item (popup_menus[nbr].popup_toplevel, pal, nbr, i), FALSE, TRUE, 0);

  popup_menus[nbr].popup_detach = gtk_button_new ();
  gtk_widget_set_name (popup_menus[nbr].popup_detach, "popup_tearoff_button");
  gtk_object_set_data (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "popup_tearoff_button", popup_menus[nbr].popup_detach);
  gtk_button_set_relief((GtkButton *) popup_menus[nbr].popup_detach, GTK_RELIEF_NONE);
  /* gtk_widget_set_style(popup_menus[nbr].popup_detach, pal->cm[7]); */
  gtk_widget_show (popup_menus[nbr].popup_detach);
  gtk_box_pack_start (GTK_BOX (popup_vbox), popup_menus[nbr].popup_detach, FALSE, TRUE, 0);

  popup_tearoff_tooltip = my_tooltips_new();
  gtk_tooltips_set_tip (popup_tearoff_tooltip, popup_menus[nbr].popup_detach, _("Click to tearoff popup menu"), "ContextHelp/buttons/?");
  gtk_object_set_data (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "tooltips", popup_tearoff_tooltip);

  popup_tearoff_hseparator = gtk_hseparator_new ();
  gtk_widget_set_name (popup_tearoff_hseparator, "popup_tearoff_hseparator");
  gtk_object_set_data (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "popup_tearoff_hseparator", popup_tearoff_hseparator);
  /* gtk_widget_set_style(popup_tearoff_hseparator, pal->cm[7]); */
  gtk_widget_show (popup_tearoff_hseparator);
  gtk_container_add (GTK_CONTAINER (popup_menus[nbr].popup_detach), popup_tearoff_hseparator);

  gtk_signal_connect (GTK_OBJECT (popup_menus[nbr].popup_toplevel), "delete_event",
                      GTK_SIGNAL_FUNC (delete_popup_cb), (gpointer) nbr);
 
  gtk_signal_connect (GTK_OBJECT (popup_menus[nbr].popup_detach), "clicked",
                             GTK_SIGNAL_FUNC (detach_cb),
                             (gpointer) nbr);

  gtk_signal_connect (GTK_OBJECT (popup_addicon_button), "clicked",
			   GTK_SIGNAL_FUNC (popup_addicon_cb),
			   (gpointer) nbr);

  gtk_drag_dest_set (popup_addicon_button,
		     GTK_DEST_DEFAULT_ALL,
		     popup_target_table, n_popup_targets,
		     GDK_ACTION_COPY | GDK_ACTION_MOVE);

  gtk_signal_connect (GTK_OBJECT (popup_addicon_button), "drag_data_received",
		      GTK_SIGNAL_FUNC (popup_addicon_drag_data_received), (gpointer) nbr);

  set_icon (popup_menus[nbr].popup_toplevel, "Popup Menu", menu_icon);

  update_popup_entries(nbr);

  return popup_menus[nbr].popup_toplevel;
}

void
create_all_popup_menus (XFCE_palette *pal, GtkWidget *main)
{
  gint i;
  
  for (i = 0; i < NBPOPUPS; i++)
  {
     create_popup_menu (pal, i);
     gtk_window_set_transient_for(GTK_WINDOW (popup_menus[i].popup_toplevel), GTK_WINDOW (main));
  }
}

void
show_popup_menu (gint nbr, gint x, gint y)
{
  gint uwidth  = 0;
  gint uheight = 0;
  gint px      = 0;
  gint py      = 0;
  gint i;
  GtkRequisition requisition;
  
  if (!GTK_WIDGET_REALIZED(popup_menus[nbr].popup_toplevel))
    gtk_widget_realize (popup_menus[nbr].popup_toplevel);
  if ((popup_menus[nbr].entries == 0) || (popup_menus[nbr].detach))
    gtk_widget_hide (popup_menus[nbr].popup_detach);
  else
    if (current_config.detach_menu)
      gtk_widget_show (popup_menus[nbr].popup_detach);
    else
      gtk_widget_hide (popup_menus[nbr].popup_detach);

  if (popup_menus[nbr].entries == NBMAXITEMS)
    gtk_widget_hide (popup_menus[nbr].popup_addicon_frame);
  else
    gtk_widget_show (popup_menus[nbr].popup_addicon_frame);

  if ((x > 0) && (y > 0)) {
    px = x;
    py = y;
    popup_menus[nbr].lastpx = x;
    popup_menus[nbr].lastpy = y;
  }
  else
    if (!popup_menus[nbr].detach) {
      px = popup_menus[nbr].lastpx;
      py = popup_menus[nbr].lastpy;
    }
  gtk_widget_size_request ((GtkWidget *) popup_menus[nbr].popup_toplevel, &requisition);
  uwidth  = requisition.width;
  uheight = requisition.height;
  if (!popup_menus[nbr].detach)
    gtk_widget_set_uposition (popup_menus[nbr].popup_toplevel, ((px - uwidth / 2) > 0) ? (px - uwidth / 2) : 0, ((py - uheight) > 0) ? (py - uheight) : 0);
  else
    gtk_widget_set_uposition (popup_menus[nbr].popup_toplevel, popup_menus[nbr].uposx,
                                                               popup_menus[nbr].uposy);
  
  gdk_window_set_decorations (((GtkWidget *)popup_menus[nbr].popup_toplevel)->window, ((popup_menus[nbr].detach) ? (GDK_DECOR_TITLE | GDK_DECOR_MENU) : 0));
  gtk_widget_show (popup_menus[nbr].popup_toplevel);
  for (i = 0 ; i < NBMAXITEMS; i++)
    {
      gtk_button_leave (GTK_BUTTON (popup_menus[nbr].popup_buttons[i].item_button));
    }
}

void
hide_popup_menu (gint nbr)
{
  gdk_window_get_root_origin (((GtkWidget *) popup_menus[nbr].popup_toplevel)->window,
                               &popup_menus[nbr].uposx, 
			       &popup_menus[nbr].uposy);
  gtk_widget_hide (popup_menus[nbr].popup_toplevel);
  gtk_widget_unrealize (popup_menus[nbr].popup_toplevel);
  gtk_widget_realize (popup_menus[nbr].popup_toplevel);
}

void
close_popup_menu (gint nbr)
{
  hide_popup_menu(nbr);
  popup_menus[nbr].detach = FALSE;
}

void
update_popup_entries(gint nbr)
{
   gint i;

   for (i = 0; i < popup_menus[nbr].entries; i++) {
     gtk_widget_show (popup_menus[nbr].popup_buttons[i].item_button);
   }
   for (i = popup_menus[nbr].entries; i < NBMAXITEMS; i++) {
     gtk_widget_hide (popup_menus[nbr].popup_buttons[i].item_button);
   }
}

void
update_popup_size(void)
{
  int i, j;
  GdkPixmap *pixmap = NULL;
  GdkBitmap *mask = NULL;
  char * pixfile;
  
  for (i = 0; i < NBPOPUPS; i++) {
    gtk_widget_set_usize (popup_menus[i].popup_addicon_icon_frame, POPUP_ICON_SIZE, POPUP_ICON_SIZE);
    for (j = 0; j < NBMAXITEMS; j++) {
      gtk_widget_set_usize (popup_menus[i].popup_buttons[j].item_pixmap_frame, POPUP_ICON_SIZE, POPUP_ICON_SIZE);
      if (j < popup_menus[i].entries) {
	pixfile = popup_menus[i].popup_buttons[j].pixfile;
	if (check_file_exists ("/", pixfile)) {
	  pixmap = MyCreateGdkPixmapFromFile(pixfile, popup_menus[i].popup_buttons[j].item_pixmap_frame, &mask);
	}
	else {
	  pixmap = MyCreateGdkPixmapFromData (defaulticon, popup_menus[i].popup_buttons[j].item_pixmap_frame, &mask);
	}
	gtk_pixmap_set (GTK_PIXMAP(popup_menus[i].popup_buttons[j].item_pixmap), pixmap, mask);
      }
    }
  }
}

void
set_entry(gint menu, gint entry, char *label, char *pixfile, char *command)
{
  GdkPixmap *pixmap = NULL;
  GdkBitmap *mask = NULL;
  
  strcpy (popup_menus[menu].popup_buttons[entry].label  , label  );
  strcpy (popup_menus[menu].popup_buttons[entry].pixfile, pixfile);
  strcpy (popup_menus[menu].popup_buttons[entry].command, command);
  
  if (check_file_exists ("/", pixfile)) {
    pixmap = MyCreateGdkPixmapFromFile(pixfile, popup_menus[menu].popup_buttons[entry].item_pixmap_frame, &mask);
  }
  else {
    pixmap = MyCreateGdkPixmapFromData (defaulticon, popup_menus[menu].popup_buttons[entry].item_pixmap_frame, &mask);
  }

  gtk_pixmap_set (GTK_PIXMAP(popup_menus[menu].popup_buttons[entry].item_pixmap), pixmap, mask);
  
  gtk_label_set_text (GTK_LABEL(popup_menus[menu].popup_buttons[entry].item_label), label);

  gtk_tooltips_set_tip (popup_menus[menu].popup_buttons[entry].item_tooltip,
                        popup_menus[menu].popup_buttons[entry].item_button,
			popup_menus[menu].popup_buttons[entry].command,
			"ContextHelp/buttons/?");
}

void
set_like_entry(gint menu_s, gint entry_s, gint menu_d, gint entry_d)
{
  GdkPixmap *pixmap = NULL;
  GdkBitmap *mask = NULL;
  
  strcpy (popup_menus[menu_d].popup_buttons[entry_d].label  , popup_menus[menu_s].popup_buttons[entry_s].label  );
  strcpy (popup_menus[menu_d].popup_buttons[entry_d].pixfile, popup_menus[menu_s].popup_buttons[entry_s].pixfile);
  strcpy (popup_menus[menu_d].popup_buttons[entry_d].command, popup_menus[menu_s].popup_buttons[entry_s].command);
  
  gtk_pixmap_get (GTK_PIXMAP(popup_menus[menu_s].popup_buttons[entry_s].item_pixmap), &pixmap, &mask);
  gtk_pixmap_set (GTK_PIXMAP(popup_menus[menu_d].popup_buttons[entry_d].item_pixmap), pixmap, mask);
  
  gtk_label_set_text (GTK_LABEL(popup_menus[menu_d].popup_buttons[entry_d].item_label), popup_menus[menu_s].popup_buttons[entry_s].label);

  gtk_tooltips_set_tip (popup_menus[menu_d].popup_buttons[entry_d].item_tooltip,
                        popup_menus[menu_d].popup_buttons[entry_d].item_button,
			popup_menus[menu_d].popup_buttons[entry_d].command,
			"ContextHelp/buttons/?");
}

void
add_popup_entry(gint nbr, char *label, char *pixfile, char *command)
{
  if ((popup_menus[nbr].entries < NBMAXITEMS) && label && pixfile && command) {
     set_entry(nbr, popup_menus[nbr].entries,label, pixfile, command);
     popup_menus[nbr].entries++;
  }
  update_popup_entries(nbr);
}

void
remove_popup_entry(gint nbr_menu, gint entry)
{
  gint i;

  for (i = entry; i < (NBMAXITEMS - 1); i++) {
    set_like_entry(nbr_menu, i + 1, nbr_menu, i);
  }
  set_entry(nbr_menu, (NBMAXITEMS - 1), "Empty", "Default", "");
  popup_menus[nbr_menu].entries--;
  update_popup_entries(nbr_menu);
}

char *
get_popup_entry_label(gint menu, gint entry)
{
  return popup_menus[menu].popup_buttons[entry].label;
}

char *
get_popup_entry_command(gint menu, gint entry)
{
  return popup_menus[menu].popup_buttons[entry].command;
}

char *
get_popup_entry_icon(gint menu, gint entry)
{
  return popup_menus[menu].popup_buttons[entry].pixfile;
}

int
get_popup_menu_entries(gint menu)
{
   return popup_menus[menu].entries;
}
