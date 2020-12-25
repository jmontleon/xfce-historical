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

#include <gtk/gtkmain.h>
#include "myhandler.h"

/* Forward declarations */

static void my_handler_class_init               (MyHandlerClass    *klass);
static void my_handler_init                     (MyHandler         *handle);
static void my_handler_finalize                 (GtkObject        *object);

static void my_handler_paint                    (GtkWidget        *widget,
					       GdkRectangle         *area);
static void my_handler_draw                     (GtkWidget        *widget,
					       GdkRectangle         *area);
static void my_handler_size_request             (GtkWidget      *widget,
					       GtkRequisition *requisition);
static void my_handler_size_allocate            (GtkWidget     *widget,
					       GtkAllocation *allocation);
static gint my_handler_expose                   (GtkWidget        *widget,
						GdkEventExpose   *event);

/* Local data */

static GtkBinClass *parent_class = NULL;

guint
my_handler_get_type ()
{
  static guint handle_type = 0;

  if (!handle_type)
    {
      GtkTypeInfo handle_info =
      {
	"MyHandler",
	sizeof (MyHandler),
	sizeof (MyHandlerClass),
	(GtkClassInitFunc) my_handler_class_init,
	(GtkObjectInitFunc) my_handler_init,
	(gpointer) NULL,
	(gpointer) NULL,
      };

      handle_type = gtk_type_unique (gtk_bin_get_type (), &handle_info);
    }

  return handle_type;
}

static void
my_handler_class_init (MyHandlerClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;

  parent_class = gtk_type_class (gtk_bin_get_type ());

  object_class->finalize = my_handler_finalize;

  widget_class->expose_event = my_handler_expose;
  widget_class->draw = my_handler_draw;
  widget_class->size_request = my_handler_size_request;
  widget_class->size_allocate = my_handler_size_allocate;
}

static void
my_handler_init (MyHandler *handle)
{
  g_return_if_fail (handle != NULL);
  g_return_if_fail (GTK_IS_MY_HANDLER (handle));
  
  handle->shadow_type = GTK_SHADOW_OUT;
  handle->orientation = GTK_ORIENTATION_HORIZONTAL;
}

GtkWidget*
my_handler_new (void)
{
  MyHandler *handle;

  handle = gtk_type_new (my_handler_get_type ());

  return GTK_WIDGET (handle);
}

static void
my_handler_finalize (GtkObject *object)
{
  MyHandler *handle;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_MY_HANDLER (object));


  handle = MY_HANDLER (object);

  (* GTK_OBJECT_CLASS (parent_class)->finalize) (object);
}


static void
my_handler_size_allocate (GtkWidget     *widget,
			 GtkAllocation *allocation)
{
  MyHandler *handle;
  GtkBin *bin;
  GtkAllocation child_allocation;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_MY_HANDLER (widget));
  g_return_if_fail (allocation != NULL);

  handle = MY_HANDLER (widget);
  bin = GTK_BIN (widget);

  if (GTK_WIDGET_MAPPED (widget) &&
      ((widget->allocation.x != allocation->x) ||
       (widget->allocation.y != allocation->y) ||
       (widget->allocation.width != allocation->width) ||
       (widget->allocation.height != allocation->height)) &&
      (widget->allocation.width != 0) &&
      (widget->allocation.height != 0))
     gtk_widget_queue_clear (widget);

  widget->allocation = *allocation;

  if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
      child_allocation.x = (GTK_CONTAINER (handle)->border_width +
			    GTK_WIDGET (handle)->style->klass->xthickness);
      child_allocation.width = MAX(1, (gint)allocation->width - child_allocation.x * 2);

      child_allocation.y = (GTK_CONTAINER (handle)->border_width +
			    GTK_WIDGET (handle)->style->klass->ythickness);
      child_allocation.height = MAX (1, ((gint)allocation->height - child_allocation.y -
					 (gint)GTK_CONTAINER (handle)->border_width -
					 (gint)GTK_WIDGET (handle)->style->klass->ythickness));

      child_allocation.x += allocation->x;
      child_allocation.y += allocation->y;

      gtk_widget_size_allocate (bin->child, &child_allocation);
    }
}

static void
my_handler_size_request (GtkWidget      *widget,
			GtkRequisition *requisition)
{
  MyHandler *handle;
  GtkBin *bin;
  gint tmp_height;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_MY_HANDLER (widget));
  g_return_if_fail (requisition != NULL);

  handle = MY_HANDLER (widget);
  bin = GTK_BIN (widget);

  requisition->width = (GTK_CONTAINER (widget)->border_width +
			GTK_WIDGET (widget)->style->klass->xthickness) * 2;

  tmp_height = 1;

  requisition->height = tmp_height + (GTK_CONTAINER (widget)->border_width +
				      GTK_WIDGET (widget)->style->klass->ythickness) * 2;

  if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
      GtkRequisition child_requisition;
      
      gtk_widget_size_request (bin->child, &child_requisition);

      requisition->width += child_requisition.width;
      requisition->height += child_requisition.height;
    }
}


static gint
my_handler_expose (GtkWidget      *widget,
		  GdkEventExpose *event)
{
  GtkBin *bin;
  GdkEventExpose child_event;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_MY_HANDLER (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (GTK_WIDGET_DRAWABLE (widget))
    {
      bin = GTK_BIN (widget);

      my_handler_paint (widget, &event->area);

      child_event = *event;
      if (bin->child &&
	  GTK_WIDGET_NO_WINDOW (bin->child) &&
	  gtk_widget_intersect (bin->child, &event->area, &child_event.area))
	gtk_widget_event (bin->child, (GdkEvent*) &child_event);
    }

  return FALSE;
}

static void
my_handler_paint (GtkWidget      *widget,
             GdkRectangle         *area)
{
  MyHandler *handle;
  gint x, y;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (area != NULL);
  g_return_if_fail (GTK_IS_MY_HANDLER (widget));

  handle = MY_HANDLER (widget);

  if (GTK_WIDGET_DRAWABLE (widget))
    {

      x = GTK_CONTAINER (handle)->border_width;
      y = GTK_CONTAINER (handle)->border_width;

      gtk_paint_handle(widget->style, widget->window, 
                      widget->state, handle->shadow_type,
                      area, widget, "handler",
		      widget->allocation.x + x,
		      widget->allocation.y + y,
		      widget->allocation.width - x * 2,
		      widget->allocation.height - y * 2, 
                      handle->orientation);
    }

}

static void
my_handler_draw (GtkWidget      *widget,
             GdkRectangle         *area)
{
  MyHandler *handle;
  GdkRectangle child_area;
  GtkBin *bin;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (area != NULL);
  g_return_if_fail (GTK_IS_MY_HANDLER (widget));

  handle = MY_HANDLER (widget);

  if (GTK_WIDGET_DRAWABLE (widget))
    {
      bin = GTK_BIN (widget);

      my_handler_paint(widget, area);

      if (bin->child && gtk_widget_intersect (bin->child, area, &child_area))
	gtk_widget_draw (bin->child, &child_area);
    }

}

void
my_handler_set_shadow_type (MyHandler *handle, GtkShadowType  type)
{
  g_return_if_fail (handle != NULL);
  g_return_if_fail (GTK_IS_MY_HANDLER (handle));
  
  if ((GtkShadowType) handle->shadow_type != type)
    {
      handle->shadow_type = type;

      if (GTK_WIDGET_DRAWABLE (handle))
	{
	  gtk_widget_queue_clear (GTK_WIDGET (handle));
	}
      gtk_widget_queue_resize (GTK_WIDGET (handle));
    }
}

void
my_handler_set_orientation (MyHandler *handle, GtkOrientation orientation)
{
  g_return_if_fail (handle != NULL);
  g_return_if_fail (GTK_IS_MY_HANDLER (handle));
  
  if ((GtkShadowType) handle->orientation != orientation)
    {
      handle->orientation = orientation;

      if (GTK_WIDGET_DRAWABLE (handle))
	{
	  gtk_widget_queue_clear (GTK_WIDGET (handle));
	}
      gtk_widget_queue_resize (GTK_WIDGET (handle));
    }
}
