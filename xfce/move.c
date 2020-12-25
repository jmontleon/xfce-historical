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
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>

#include <X11/Xlib.h>
#include "configfile.h"
#include "move.h"

GdkGC  *DrawGC;
static int xgrabcount = 0;
static gboolean pressed = FALSE;

void
MyXGrabServer (Display * disp)
{
  if (xgrabcount == 0)
    {
      XGrabServer (disp);
    }
  ++xgrabcount;
}

void
MyXUngrabServer (Display * disp)
{
  if (--xgrabcount < 0)		/* should never happen */
    {
      xgrabcount = 0;
    }
  if (xgrabcount == 0)
    {
      XUngrabServer (disp);
    }
}

void
CreateDrawGC (GdkWindow *w)
{
  GdkGCValues gcv;
  GdkGCValuesMask gcm;
  GdkColor col;
  
  col.red   = 0;
  col.green = 0;
  col.blue  = 0;
  
  gcm = GDK_GC_FUNCTION | GDK_GC_LINE_WIDTH | GDK_GC_FOREGROUND | GDK_GC_SUBWINDOW | GDK_GC_LINE_STYLE;
  gcv.function = GDK_XOR;
  gcv.line_width = 4;
  gcv.foreground = col;
  gcv.subwindow_mode = GDK_INCLUDE_INFERIORS;
  gcv.line_style = GDK_LINE_SOLID;
  DrawGC = gdk_gc_new_with_values (w, &gcv, gcm);
}

void
MoveOutline (int x, int y, int width, int height)
{
  static int lastx = 0;
  static int lasty = 0;
  static int lastWidth = 0;
  static int lastHeight = 0;
 
  if (x == lastx && y == lasty && width == lastWidth && height == lastHeight)
    return;

  /* undraw the old one, if any */
  if (lastWidth || lastHeight)
    {
      gdk_draw_rectangle (GDK_ROOT_PARENT(), DrawGC, FALSE, lastx, lasty, lastWidth, lastHeight); 
    }

  lastx = x;
  lasty = y;
  lastWidth = width;
  lastHeight = height;

  /* draw the new one, if any */
  if (lastWidth || lastHeight)
    {
      gdk_draw_rectangle (GDK_ROOT_PARENT(), DrawGC, FALSE, lastx, lasty, lastWidth, lastHeight); 
    }
}

static void
move_pressed (GtkWidget *widget, 
              GdkEventButton *event,
	      gpointer *topwin)

{
  CursorOffset *p;
  gint xp, yp;
  gint upositionx = 0;
  gint upositiony = 0;
  gint uwidth = 0;
  gint uheight = 0;
  
  /* ignore double and triple click */
  if (event->type != GDK_BUTTON_PRESS)
    return;

  p = gtk_object_get_user_data (GTK_OBJECT(widget));
  gdk_window_get_root_origin (((GtkWidget *) topwin)->window, &upositionx, &upositiony);
  gdk_window_get_size(((GtkWidget *) topwin)->window, &uwidth, &uheight);

  p->x = (int) event->x_root - upositionx;
  p->y = (int) event->y_root - upositiony;
  xp = (int) event->x_root;
  yp = (int) event->y_root;

  if (!current_config.opaquemove) 
    MyXGrabServer(GDK_DISPLAY());

  gtk_grab_add (widget);
  gdk_pointer_grab (widget->window, TRUE,
		    GDK_BUTTON_RELEASE_MASK |
		    GDK_BUTTON_MOTION_MASK,
		    NULL, NULL, 0);
  if (!current_config.opaquemove)
    MoveOutline(xp - p->x, yp - p->y, uwidth, uheight);
  pressed = TRUE;
}

static void
move_released (GtkWidget *widget,
	      GdkEventMotion *event,
	      gpointer *topwin)
{
  gint xp, yp;
  CursorOffset * p;

  pressed = FALSE;

  p = gtk_object_get_user_data (GTK_OBJECT (widget));

  xp = (int) event->x_root;
  yp = (int) event->y_root;

  if(!current_config.opaquemove) {
    MoveOutline(0, 0, 0, 0);
    XSync(GDK_DISPLAY(), True);
    MyXUngrabServer(GDK_DISPLAY());
  }
  XMoveWindow (GDK_DISPLAY(), GDK_WINDOW_XWINDOW(((GtkWidget *)topwin)->window),
                              ((xp - p->x) > 0) ? (xp - p->x) : 0,
                              ((yp - p->y) > 0) ? (yp - p->y) : 0);
  gtk_grab_remove (widget);
  gdk_pointer_ungrab (0);
  writeconfig();
}

static void
move_motion ( GtkWidget      *widget, 
	      GdkEventMotion *event,
	      gpointer *topwin)
{
  gint xp, yp;
  CursorOffset * p;
  gint uwidth = 0;
  gint uheight = 0;

  if (!pressed) return;
  
  p = gtk_object_get_user_data (GTK_OBJECT (widget));

  xp = (int) event->x_root;
  yp = (int) event->y_root;

  if(!current_config.opaquemove) {
      gdk_window_get_size(((GtkWidget *) topwin)->window, &uwidth, &uheight);
      MoveOutline(xp - p->x, yp - p->y, uwidth, uheight);
  }
  else
    XMoveWindow (GDK_DISPLAY(), GDK_WINDOW_XWINDOW(((GtkWidget *)topwin)->window),  xp - p->x, yp - p->y);
}

void
create_move_button (GtkWidget *widget, GtkWidget *toplevel)
{
  CursorOffset* icon_pos;

  icon_pos = g_new (CursorOffset, 1);
  gtk_object_set_user_data(GTK_OBJECT(widget), icon_pos);

  gtk_widget_set_events (widget, 
			 GDK_BUTTON_MOTION_MASK |
			 GDK_BUTTON_PRESS_MASK);

  gtk_signal_connect (GTK_OBJECT (widget), "button_press_event",
		      GTK_SIGNAL_FUNC (move_pressed),
		      toplevel);
  gtk_signal_connect (GTK_OBJECT (widget), "button_release_event",
		      GTK_SIGNAL_FUNC (move_released),
		      toplevel);
  gtk_signal_connect (GTK_OBJECT (widget), "motion_notify_event",
		      GTK_SIGNAL_FUNC (move_motion),
		      toplevel);
}

