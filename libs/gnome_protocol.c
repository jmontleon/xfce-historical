/*  gxfce
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

#include <gnome_protocol.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

Atom _XA_WIN_WORKSPACE;
Atom _XA_WIN_WORKSPACE_COUNT;
Atom _XA_WIN_STATE;
/* Atom _XA_WIN_WORKSPACE_NAMES; */

void create_gnome_atoms(void)
{
  _XA_WIN_WORKSPACE = XInternAtom(GDK_DISPLAY(), "_WIN_WORKSPACE", False);
  _XA_WIN_WORKSPACE_COUNT = XInternAtom(GDK_DISPLAY(), "_WIN_WORKSPACE_COUNT", False);
  _XA_WIN_STATE = XInternAtom(GDK_DISPLAY(), "_WIN_STATE", False);
  /* _XA_WIN_WORKSPACE_NAMES = XInternAtom(GDK_DISPLAY(), "_WIN_WORKSPACE_NAMES", False); */
}

void gnome_change_desk(int desk)
{
  XEvent xev;
  XChangeProperty (GDK_DISPLAY(), GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT()), _XA_WIN_WORKSPACE, XA_CARDINAL, 32,
                   PropModeReplace,  (unsigned char *) &desk, 1);

  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.window = GDK_ROOT_WINDOW();
  xev.xclient.message_type = _XA_WIN_WORKSPACE;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = desk;
  xev.xclient.data.l[1] = gdk_time_get();

  XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False,
  	     SubstructureNotifyMask, (XEvent*) &xev);
}

void gnome_set_desk_count (int desk)
{
  XChangeProperty (GDK_DISPLAY(), GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT()), _XA_WIN_WORKSPACE_COUNT, XA_CARDINAL, 32,
                   PropModeReplace, (unsigned char *) &desk, 1);

}

void gnome_sticky(GdkWindow *win)
{
    long flags=WinStateAllWorkspaces;

    if(win != NULL)
    {
       XChangeProperty (GDK_DISPLAY(), (Window) GDK_WINDOW_XWINDOW(win), _XA_WIN_STATE, XA_CARDINAL,
                     32, PropModeReplace, (unsigned char*)&flags, 1);
    }  
}

static gint get_current_workspace(void)
{
    Atom type;
    int format;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char *prop;
    CARD32 wws = 0;



    if((XGetWindowProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(),
                           _XA_WIN_WORKSPACE,
                           0, 1, False, XA_CARDINAL,
                           &type, &format, &nitems,
                           &bytes_after, &prop)) == 1)
    {
        return (-1);
    }
    
    wws = *(CARD32 *)prop;

    return wws;
}

gint gnome_desk_change(GdkEventProperty *event)
{
  gint desk = -1;
  
  g_return_val_if_fail (event != NULL, FALSE);
  
  if ((event->atom == _XA_WIN_WORKSPACE) && (event->state == GDK_PROPERTY_NEW_VALUE))
    desk = get_current_workspace();
  return (desk);
}

void gnome_set_root_event(GtkSignalFunc f)
{
  GtkWidget *dummy_win;

  g_return_if_fail (f != NULL);

  dummy_win = gtk_window_new(GTK_WINDOW_POPUP);
  gtk_widget_realize(dummy_win);
  gdk_window_set_user_data(GDK_ROOT_PARENT(), dummy_win);
  gdk_window_ref(GDK_ROOT_PARENT());
  gdk_xid_table_insert(&(GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT())), 
		       GDK_ROOT_PARENT());
  gtk_signal_connect (GTK_OBJECT (dummy_win), "event", 
                      GTK_SIGNAL_FUNC (f), 
		      NULL);
  XSelectInput(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT()), 
	       PropertyChangeMask);
}
