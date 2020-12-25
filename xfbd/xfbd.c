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
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>

#ifdef HAVE_GDK_IMLIB
  #include <fcntl.h>
  #include <gdk_imlib.h>
#else
#ifdef HAVE_GDK_PIXBUF
  #include <gdk-pixbuf/gdk-pixbuf.h>
#endif
#endif

#include "my_intl.h"
#include "xfbd_cb.h"
#include "xfbd.h"
#include "constant.h"
#include "xfcolor.h"
#include "xpmext.h"
#include "my_string.h"
#include "fileutil.h"
#include "xfce-common.h"
#include "empty.h"

#include "xfbd_icon.xpm"

void
display_error(char *s)
{
  fprintf (stderr, "%s\n", s);
  my_alert (s);
}

int
setroot (char *str)
{
#ifdef HAVE_GDK_IMLIB
  GdkImlibImage         *im;
  GdkPixmapPrivate      *pixmap_private;
  GdkPixmap             *rootXpm       = 0;
  Pixmap                 xpixmap;
#else
#ifdef HAVE_GDK_PIXBUF
  GdkPixmapPrivate      *pixmap_private;
  GdkPixbuf             *Pixbuf;
  GdkWindow             *root;
  GdkPixmap             *rootXpm       = 0;
  GdkBitmap             *shapeMask     = 0;
  Pixmap                 xpixmap;
#else
  Display *tmpDpy;
  XWindowAttributes root_attr;
  int val;
  Pixmap                 rootXpm       = 0;
  Pixmap                 shapeMask     = 0;
  XpmAttributes          xpm_attributes;
#endif /* HAVE_GDK_PIXBUF */
#endif /* HAVE_GDK_IMLIB */

  int pix_width                 = 0;
  int pix_height                = 0;

  if (my_strncasecmp (str, NOBACK, strlen (NOBACK)) && existfile (str))
    {
#ifdef HAVE_GDK_IMLIB
      im=gdk_imlib_load_image(str);
      if (im) 
        {
          if ((im->rgb_width > (gdk_screen_width() / 2)) && (im->rgb_height > (gdk_screen_height() / 2)))
          {
            pix_width  = gdk_screen_width();
            pix_height = gdk_screen_height();
          }
          else
          {
            pix_width  = im->rgb_width;
            pix_height = im->rgb_height;
          }
          gdk_imlib_render(im,pix_width,pix_height);
	  rootXpm=gdk_imlib_copy_image(im);
	  gdk_window_set_back_pixmap(GDK_ROOT_PARENT(), rootXpm, 0);
	  pixmap_private = (GdkPixmapPrivate*) rootXpm;
          xpixmap = duppix(pixmap_private->xwindow, pix_width, pix_height);
          setPixmapProperty(xpixmap);
          gdk_window_clear(GDK_ROOT_PARENT());
          gdk_flush();
          gdk_imlib_destroy_image(im);
	}
#else     
#ifdef HAVE_GDK_PIXBUF
      Pixbuf = gdk_pixbuf_new_from_file (str);
      if (Pixbuf)
        {
          if ((gdk_pixbuf_get_width (Pixbuf) > (gdk_screen_width() / 2)) && 
	      (gdk_pixbuf_get_height (Pixbuf) > (gdk_screen_height() / 2)))
          {
            pix_width  = gdk_screen_width();
            pix_height = gdk_screen_height();
          }
          else
          {
            pix_width  = gdk_pixbuf_get_width (Pixbuf);
            pix_height = gdk_pixbuf_get_height (Pixbuf);
          }
          gdk_pixbuf_scale_simple (Pixbuf, pix_width, 
                                           pix_height,
                                           GDK_INTERP_NEAREST);
          gdk_pixbuf_render_pixmap_and_mask (Pixbuf, &rootXpm, &shapeMask, 0);
	  gdk_window_set_back_pixmap(GDK_ROOT_PARENT(), rootXpm, 0);
	  pixmap_private = (GdkPixmapPrivate*) rootXpm;
          xpixmap = duppix(pixmap_private->xwindow, pix_width, pix_height);
          setPixmapProperty(xpixmap);
          gdk_window_clear(GDK_ROOT_PARENT());
          gdk_flush();
          gdk_pixbuf_unref (Pixbuf);
	}
#else
      if (!(tmpDpy = XOpenDisplay(""))) 
        return 0;

      XGetWindowAttributes (tmpDpy, GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT()), &root_attr);
      xpm_attributes.colormap = root_attr.colormap;
      xpm_attributes.valuemask = XpmSize | XpmReturnPixels | XpmColormap;
      if ((val = XpmReadFileToPixmap (tmpDpy, GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT()), str,
				      &rootXpm, &shapeMask,
				      &xpm_attributes)) != XpmSuccess)
	{
	  if (val == XpmOpenFailed)
	    display_error(_("Error\nCannot open file"));
	  else if (val == XpmColorFailed)
	    display_error(_("Error\nCannot allocate colors"));
	  else if (val == XpmFileInvalid)
	    display_error(_("Error\nFile format invalid"));
	  else if (val == XpmColorError)
	    display_error(_("Error\nColor definition invalid"));
	  else if (val == XpmNoMemory)
	    display_error(_("Error\nNot enough Memory"));
	  return (0);
	}
      pix_width  = xpm_attributes.width;
      pix_height = xpm_attributes.height;
      XSetWindowBackgroundPixmap (tmpDpy, GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT()), rootXpm);
      setPixmapProperty(rootXpm);
      XClearWindow (tmpDpy, GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT()));
      XpmFreeAttributes(&xpm_attributes);
      XSetCloseDownMode(tmpDpy, RetainPermanent);
      XCloseDisplay(tmpDpy);
#endif /* HAVE_GDK_PIXBUF */
#endif /* HAVE_GDK_IMLIB */
    }
  return (1);
}

void
readstr (char *str)
{
  char *homedir;
  FILE *f = NULL;

  homedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/."RCFILE);
  if (existfile (homedir))
    {
      f = fopen (homedir, "r");
    }
  else
    {
      fprintf (stderr, "%s File not found.\n", homedir);
      strcpy (homedir, XFCE_CONFDIR);
      strcat (homedir, "/"RCFILE);
      if (existfile (homedir))
	{
	  f = fopen (homedir, "r");
	}
    }
  if (f) 
    {
      fgets (str, FILENAME_MAX - 1, f);
      if (strlen (str))
        str[strlen (str) - 1] = 0;
    }
  else
    strcpy (str, NOBACK);
  free (homedir);
}

int
writestr (char *str)
{
  char *homedir;
  FILE *f;

  homedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/."RCFILE);

  if ((f = fopen (homedir, "w")))
    {
      fprintf (f, "%s\n", str);
      fclose (f);
    }
  free (homedir);
  return ((f != NULL));
}

void
display_back (char *str)
{
  if ((str) && (my_strncasecmp (str, NOBACK, strlen (NOBACK)) && existfile (str)))
    MySetPixmapFile (preview_pixmap, preview_pixmap_frame, str);
  else
    MySetPixmapData (preview_pixmap, preview_pixmap_frame, empty);
}

GtkWidget*
create_xfbd (XFCE_palette * pal)
{
  GtkWidget *frame1;
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *label1;
  GtkWidget *frame3;
  GtkWidget *hbuttonbox1;
  GtkWidget *ok_button;
  GtkWidget *browse_button;
  GtkWidget *apply_button;
  GtkWidget *clear_button;
  GtkWidget *cancel_button;
  GtkAccelGroup *accel_group;

  xfbd = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (xfbd, "xfbd");
  gtk_object_set_data (GTK_OBJECT (xfbd), "xfbd", xfbd);
  gtk_window_set_title (GTK_WINDOW (xfbd), _("XFbd - XFce Backdrop Manager"));
  gtk_window_position (GTK_WINDOW (xfbd), GTK_WIN_POS_CENTER);

  frame1 = gtk_frame_new (NULL);
  gtk_widget_set_name (frame1, "frame1");
  gtk_object_set_data (GTK_OBJECT (xfbd), "frame1", frame1);
  gtk_widget_show (frame1);
  gtk_container_add (GTK_CONTAINER (xfbd), frame1);
  gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_OUT);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox1, "vbox1");
  gtk_object_set_data (GTK_OBJECT (xfbd), "vbox1", vbox1);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (frame1), vbox1);

  preview_pixmap_frame = gtk_frame_new (NULL);
  gtk_widget_set_name (preview_pixmap_frame, "preview_pixmap_frame");
  gtk_object_set_data (GTK_OBJECT (xfbd), "preview_pixmap_frame", preview_pixmap_frame);
  gtk_widget_set_usize (preview_pixmap_frame, 500, 300);
  gtk_widget_show (preview_pixmap_frame);
  gtk_box_pack_start (GTK_BOX (vbox1), preview_pixmap_frame, TRUE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (preview_pixmap_frame), 5);
  gtk_frame_set_shadow_type (GTK_FRAME (preview_pixmap_frame), GTK_SHADOW_IN);

  preview_pixmap = MyCreateFromPixmapData (xfbd, empty);
  if (preview_pixmap == NULL)
    g_error (_("Couldn't create pixmap"));
  gtk_widget_set_name (preview_pixmap, "preview_pixmap");
  gtk_object_set_data (GTK_OBJECT (xfbd), "preview_pixmap", preview_pixmap);
  gtk_widget_show (preview_pixmap);
  gtk_container_add (GTK_CONTAINER (preview_pixmap_frame), preview_pixmap);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox1, "hbox1");
  gtk_object_set_data (GTK_OBJECT (xfbd), "hbox1", hbox1);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  label1 = gtk_label_new (_("Filename : "));
  gtk_widget_set_name (label1, "label1");
  gtk_object_set_data (GTK_OBJECT (xfbd), "label1", label1);
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox1), label1, FALSE, TRUE, 5);
  gtk_misc_set_alignment (GTK_MISC (label1), 1, 0.5);

  filename_entry = gtk_entry_new ();
  gtk_widget_set_name (filename_entry, "filename_entry");
  gtk_object_set_data (GTK_OBJECT (xfbd), "filename_entry", filename_entry);
  /* gtk_widget_set_style(filename_entry, pal->cm[4]); */
  gtk_widget_show (filename_entry);
  gtk_box_pack_start (GTK_BOX (hbox1), filename_entry, TRUE, TRUE, 5);

  frame3 = gtk_frame_new (NULL);
  gtk_widget_set_name (frame3, "frame3");
  gtk_object_set_data (GTK_OBJECT (xfbd), "frame3", frame3);
  gtk_widget_show (frame3);
  gtk_box_pack_start (GTK_BOX (vbox1), frame3, FALSE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (frame3), 5);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_set_name (hbuttonbox1, "hbuttonbox1");
  gtk_object_set_data (GTK_OBJECT (xfbd), "hbuttonbox1", hbuttonbox1);
  gtk_widget_show (hbuttonbox1);
  gtk_container_add (GTK_CONTAINER (frame3), hbuttonbox1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), GTK_BUTTONBOX_SPREAD);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox1), 0);
  gtk_button_box_set_child_size (GTK_BUTTON_BOX (hbuttonbox1), 0, 0);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (hbuttonbox1), 0, 0);

  ok_button = gtk_button_new_with_label (_("Ok"));
  gtk_widget_set_name (ok_button, "ok_button");
  gtk_object_set_data (GTK_OBJECT (xfbd), "ok_button", ok_button);
  gtk_widget_show (ok_button);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), ok_button);
  gtk_container_border_width (GTK_CONTAINER (ok_button), 5);
  GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);
  accel_group = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (xfbd), accel_group);
  gtk_widget_add_accelerator (ok_button, "clicked", accel_group,
                              GDK_Return, 0, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (ok_button, "clicked", accel_group,
                              GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  browse_button = gtk_button_new_with_label (_("Browse ..."));
  gtk_widget_set_name (browse_button, "browse_button");
  gtk_object_set_data (GTK_OBJECT (xfbd), "browse_button", browse_button);
  gtk_widget_show (browse_button);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), browse_button);
  gtk_container_border_width (GTK_CONTAINER (browse_button), 5);
  GTK_WIDGET_SET_FLAGS (browse_button, GTK_CAN_DEFAULT);
  gtk_widget_add_accelerator (browse_button, "clicked", accel_group,
                              GDK_b, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  apply_button = gtk_button_new_with_label (_("Apply"));
  gtk_widget_set_name (apply_button, "apply_button");
  gtk_object_set_data (GTK_OBJECT (xfbd), "apply_button", apply_button);
  gtk_widget_show (apply_button);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), apply_button);
  gtk_container_border_width (GTK_CONTAINER (apply_button), 5);
  GTK_WIDGET_SET_FLAGS (apply_button, GTK_CAN_DEFAULT);
  gtk_widget_add_accelerator (apply_button, "clicked", accel_group,
                              GDK_a, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  clear_button = gtk_button_new_with_label (_("Clear"));
  gtk_widget_set_name (clear_button, "clear_button");
  gtk_object_set_data (GTK_OBJECT (xfbd), "clear_button", clear_button);
  gtk_widget_show (clear_button);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), clear_button);
  gtk_container_border_width (GTK_CONTAINER (clear_button), 5);
  GTK_WIDGET_SET_FLAGS (clear_button, GTK_CAN_DEFAULT);
  gtk_widget_add_accelerator (clear_button, "clicked", accel_group,
                              GDK_Delete, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  cancel_button = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_set_name (cancel_button, "cancel_button");
  gtk_object_set_data (GTK_OBJECT (xfbd), "cancel_button", cancel_button);
  gtk_widget_show (cancel_button);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), cancel_button);
  gtk_container_border_width (GTK_CONTAINER (cancel_button), 5);
  GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);
  gtk_widget_add_accelerator (cancel_button, "clicked", accel_group,
                              GDK_Escape, 0, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (cancel_button, "clicked", accel_group,
                              GDK_c, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  gtk_signal_connect (GTK_OBJECT (apply_button), "clicked",
				 GTK_SIGNAL_FUNC (apply_cb),
				 NULL);
  gtk_signal_connect (GTK_OBJECT (ok_button), "clicked",
				 GTK_SIGNAL_FUNC (ok_cb),
				 NULL);
  gtk_signal_connect (GTK_OBJECT (browse_button), "clicked",
				 GTK_SIGNAL_FUNC (browse_cb),
				 NULL);
  gtk_signal_connect (GTK_OBJECT (clear_button), "clicked",
				 GTK_SIGNAL_FUNC (clear_cb),
				 NULL);
  gtk_signal_connect (GTK_OBJECT (cancel_button), "clicked",
				 GTK_SIGNAL_FUNC (cancel_cb),
				 NULL);
  gtk_signal_connect (GTK_OBJECT (xfbd), "delete_event",
                          GTK_SIGNAL_FUNC (delete_event), NULL);
  gtk_signal_connect (GTK_OBJECT (xfbd), "destroy",
                      GTK_SIGNAL_FUNC (cancel_cb), NULL);
  return xfbd;
}

int
main (int argc, char *argv[])
{
  XFCE_palette *pal;

  signal_setup();
  
  gtk_set_locale ();
  bindtextdomain(PACKAGE, XFCE_LOCALE_DIR);
  textdomain (PACKAGE);
  gtk_init (&argc, &argv);
#ifdef HAVE_GDK_IMLIB
  gdk_imlib_init();
/* Get gdk to use imlib's visual and colormap */
  gtk_widget_push_visual(gdk_imlib_get_visual());
  gtk_widget_push_colormap(gdk_imlib_get_colormap());  
#endif


  backdrp = (char *) malloc (sizeof (char) * FILENAME_MAX);
  readstr (backdrp);

  if (argc == 2 && strcmp (argv[1], "-i") == 0)
    {
      pal = newpal ();
      defpal (pal);
      loadpal (pal);
      applypal (pal, NULL);
      xfbd = create_xfbd (pal);
      gtk_entry_set_text(GTK_ENTRY(filename_entry), backdrp);
      gtk_entry_set_position(GTK_ENTRY(filename_entry)  , 0);
      display_back (backdrp);
      gtk_widget_show (xfbd);
      set_icon (xfbd, "XFbd", xfbd_icon_xpm);
      reg_xfce_app(xfbd, pal);
      gtk_main ();
    }
  if ((argc == 1) || (argc == 2 && strcmp (argv[1], "-d") == 0))
    {
      setroot (backdrp);
      return (0);
    }

  fprintf (stderr, _("Usage : %s [OPTIONS]\n"), argv[0]);
  fprintf (stderr, _("   Where OPTIONS are :\n"));
  fprintf (stderr, _("   -i : interactive, prompts for backdrop to display\n"));
  fprintf (stderr, _("   -d : display, reads configuration and exit (default)\n\n"));
  fprintf (stderr, _("%s is part of the XFce distribution, written by Olivier Fourdan\n\n"), argv[0]);

  return (-1);

  return 0;
}

