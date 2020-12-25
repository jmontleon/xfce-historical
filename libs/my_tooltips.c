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
#include "constant.h"

static GtkStyle *tooltipstyle = NULL;

GtkTooltips*     
my_tooltips_new (void)
{
  gint i;
  GtkTooltips* res;
  
  if (!tooltipstyle) {
    tooltipstyle = gtk_style_new();
    for (i = 0; i < 5; i++)
    {
      tooltipstyle->fg[i].red	  = 0;
      tooltipstyle->fg[i].blue    = 0;
      tooltipstyle->fg[i].green   = 0;
      tooltipstyle->bg[i].red	  = 63750;
      tooltipstyle->bg[i].blue    = 58650;
      tooltipstyle->bg[i].green   = 61200;
      tooltipstyle->text[i].red   = 0;
      tooltipstyle->text[i].blue  = 0;
      tooltipstyle->text[i].green = 0;
      tooltipstyle->base[i].red   = 63750;
      tooltipstyle->base[i].blue  = 58650;
      tooltipstyle->base[i].green = 61200;
    }
    gdk_font_unref(tooltipstyle->font);
    gtk_widget_push_style(tooltipstyle);
    if ((tooltipstyle->font = gdk_font_load(TIPFONT)) == NULL)
      tooltipstyle->font = gdk_font_load("fixed");
    gtk_widget_pop_style();
  }
  res = gtk_tooltips_new();
  gtk_tooltips_force_window(res);
  gtk_widget_set_style(GTK_WIDGET (res->tip_window), tooltipstyle);
  
  return res;
}
