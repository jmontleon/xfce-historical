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

#ifndef __MY_GTK_CLOCK_H__
#define __MY_GTK_CLOCK_H__


#include <gdk/gdk.h>
#include <gtk/gtkadjustment.h>
#include <gtk/gtkwidget.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define MY_GTK_CLOCK(obj)          GTK_CHECK_CAST (obj, my_gtk_clock_get_type (), MyGtkClock)
#define MY_GTK_CLOCK_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, my_gtk_clock_get_type (), MyGtkClockClass)
#define GTK_IS_MY_CLOCK(obj)       GTK_CHECK_TYPE (obj, my_gtk_clock_get_type ())

#define UPDATE_DELAY_LENGTH        30000 /* Update clock every 30 secs */

typedef struct _MyGtkClock        MyGtkClock;
typedef struct _MyGtkClockClass   MyGtkClockClass;

struct _MyGtkClock
{
  GtkWidget widget;
  GtkStyle  *parent_style;
  /* Dimensions of clock components */
  gint radius;
  gint internal;
  gint pointer_width;

  gboolean relief;

  /* ID of update timer, or 0 if none */
  guint32 timer;

  gfloat hrs_angle;
  gfloat min_angle;
};

struct _MyGtkClockClass
{
  GtkWidgetClass parent_class;
};


GtkWidget*     my_gtk_clock_new                    (void);
guint          my_gtk_clock_get_type               (void);
void           my_gtk_clock_set_relief             (MyGtkClock *clock, 
                                                    gboolean relief);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __MY_GTK_CLOCK_H__ */
/* example-end */
