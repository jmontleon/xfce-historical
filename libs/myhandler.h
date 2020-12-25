/*  gxfce
 *  Copyright (C) 2000 Olivier Fourdan (fourdan@xfce.org)
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

#ifndef __MY_HANDLER_H__
#define __MY_HANDLER_H__


#include <gdk/gdk.h>
#include <gtk/gtkbin.h>


#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */


#define MY_HANDLER(obj)              GTK_CHECK_CAST (obj, my_handler_get_type (), MyHandler)
#define MY_HANDLER_CLASS(klass)      GTK_CHECK_CLASS_CAST (klass, my_handler_get_type (), MyHandlerClass)
#define GTK_IS_MY_HANDLER(obj)       GTK_CHECK_TYPE (obj, my_handler_get_type ())

  typedef struct _MyHandler MyHandler;
  typedef struct _MyHandlerClass MyHandlerClass;

  struct _MyHandler
  {
    GtkBin bin;
    GtkWidget widget;

    GtkShadowType shadow_type;
    GtkOrientation orientation;
  };

  struct _MyHandlerClass
  {
    GtkBinClass parent_class;
  };

  GtkWidget *my_handler_new (void);
  guint my_handler_get_type (void);
  void my_handler_set_shadow_type (MyHandler * handle, GtkShadowType type);
  void my_handler_set_orientation (MyHandler * handle,
				   GtkOrientation orientation);
#ifdef __cplusplus
}
#endif				/* __cplusplus */


#endif				/* __MY_HANDLER_H__ */
/* example-end */
