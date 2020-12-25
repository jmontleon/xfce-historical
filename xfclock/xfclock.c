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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <time.h>
#include "my_intl.h"
#include "mygtkclock.h"
#include "xfcolor.h"
#include "xfce-common.h"

#include "xfclock_icon.xpm"

gint
update_xfclock_date_timer (GtkWidget *widget)
{
  time_t ticks;
  struct tm *tm;
  static gint mday = -1;
  static gint mon  = -1;
  static gint year = -1;
  gint cal_day, cal_mon, cal_year;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
  g_return_val_if_fail (GTK_IS_CALENDAR (widget), FALSE);

  ticks = time(0);
  tm = localtime(&ticks);
  gtk_calendar_get_date(GTK_CALENDAR (widget), &cal_year, &cal_mon, &cal_day);
  if ((mday     != tm->tm_mday) ||
      (mon      != tm->tm_mon)  ||
      (year     != tm->tm_year) ||
      (cal_day  != tm->tm_mday) ||
      (cal_mon  != tm->tm_mon)  ||
      (cal_year != tm->tm_year))
    {
      mday = tm->tm_mday;
      mon  = tm->tm_mon;
      year = tm->tm_year;

      gtk_calendar_select_day (GTK_CALENDAR (widget),mday);
      gtk_calendar_select_month (GTK_CALENDAR (widget), mon, year + 1900);

    }
  return TRUE;
}


int 
main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *main_frame;
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *clock;
  GtkWidget *vbox;
  GtkWidget *calendar;

  XFCE_palette *pal;
  
  signal_setup();

  gtk_set_locale ();
  bindtextdomain(PACKAGE, XFCE_LOCALE_DIR);
  textdomain (PACKAGE);
  gtk_init (&argc, &argv);

  pal = newpal ();
  defpal (pal);
  loadpal (pal);
  applypal (pal, NULL);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  
  gtk_window_set_title (GTK_WINDOW (window), _("XFClock - XFce Clock & Calendar"));
  
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
		      GTK_SIGNAL_FUNC (gtk_exit), NULL);
  
  main_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME(main_frame), GTK_SHADOW_OUT);
  gtk_container_add (GTK_CONTAINER (window), main_frame);
  gtk_widget_show (main_frame); 

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (main_frame), hbox);
  gtk_widget_show (hbox);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
  gtk_widget_show(vbox);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
  gtk_box_pack_start (GTK_BOX(vbox), frame, TRUE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (frame), 5);
  gtk_widget_show (frame); 

  clock = my_gtk_clock_new();
  gtk_widget_set_usize(GTK_WIDGET(clock), 100, 100);
  gtk_container_add (GTK_CONTAINER (frame), clock);
  my_gtk_clock_set_relief (MY_GTK_CLOCK(clock), TRUE);
  gtk_widget_show (clock);

  calendar = gtk_calendar_new ();
  gtk_object_set_data (GTK_OBJECT (window), "calendar", calendar);
  gtk_widget_show (calendar);
  gtk_box_pack_start (GTK_BOX (hbox), calendar, TRUE, TRUE, 0);
  gtk_calendar_display_options (GTK_CALENDAR (calendar),GTK_CALENDAR_SHOW_HEADING | GTK_CALENDAR_SHOW_DAY_NAMES | GTK_CALENDAR_SHOW_WEEK_NUMBERS);

  update_xfclock_date_timer (calendar);
  gtk_timeout_add (30000 /* 30 secs */,
                   (GtkFunction) update_xfclock_date_timer,
                   (gpointer) calendar);

  reg_xfce_app(window, pal);

  gtk_widget_show (window);
  set_icon (window, "XFClock", xfclock_icon_xpm);
  
  gtk_main ();
  
  return 0;
}
