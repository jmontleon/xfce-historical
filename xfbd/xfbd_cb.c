/*  xfbd
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
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include "my_intl.h"
#include "xfbd.h"
#include "xfbd_cb.h"
#include "constant.h"
#include "fileselect.h"
#include "fileutil.h"
#include "my_string.h"
#include "xfce-common.h"

void browse_cb( GtkWidget *widget, gpointer   data )
{
  char *fselect;
  
  fselect = gtk_entry_get_text(GTK_ENTRY(filename_entry));
  if (strlen(fselect) && existfile (fselect))
    fselect = open_fileselect (fselect);
  else
    fselect = (cleanup (open_fileselect (build_path(XFCE_BACKDROPS))));
  if (fselect)
    if (strlen (fselect))
      {
        gtk_entry_set_text(GTK_ENTRY(filename_entry), fselect);
        gtk_entry_set_position(GTK_ENTRY(filename_entry)  , 0);
	strcpy (backdrp, fselect);
	display_back (backdrp);
      }
}

void apply_cb( GtkWidget *widget, gpointer   data )
{
  strncpy(backdrp, cleanup ((char *)gtk_entry_get_text (GTK_ENTRY(filename_entry))), FILENAME_MAX - 1);
  if (!strlen (backdrp))
    strcpy (backdrp, NOBACK);
  display_back (backdrp);
  setroot (backdrp);
}

void cancel_cb( GtkWidget *widget, gpointer   data )
{
  free (backdrp);
  gtk_main_quit ();
  exit (0);
}

void ok_cb( GtkWidget *widget, gpointer   data )
{
  apply_cb (widget, data);
  writestr (backdrp);
  defroot (backdrp);
  free (backdrp);
  gtk_widget_hide (xfbd);
  gtk_widget_unrealize (xfbd);
  gtk_widget_destroy (xfbd);
  gtk_main_quit ();
  exit (0);
}

void clear_cb( GtkWidget *widget, gpointer   data )
{
  gtk_entry_set_text(GTK_ENTRY(filename_entry), NOBACK);
  gtk_entry_set_position(GTK_ENTRY(filename_entry)  , 0);
  strcpy (backdrp, NOBACK);
  display_back (backdrp);
}

gboolean delete_event( GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   data )
{
  cancel_cb (widget,data);
  return (TRUE);
}
