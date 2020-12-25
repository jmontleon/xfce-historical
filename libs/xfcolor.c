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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include "gtkinvisible.h"

#include "xfcolor.h"
#include "my_string.h"
#include "fileutil.h"
#include "xfce-common.h"
#include "lightdark.h"

#ifdef DMALLOC
#  include "dmalloc.h"
#endif

#define GTK_WIDGET_USER_STYLE(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_USER_STYLE) != 0)
#define atom_read_colors_name "_XFCE_READ_COLORS"

typedef struct
{
  GtkStyle *old;
  GtkStyle *new;
}
gtkstyle_pair;

static GdkAtom atom_read_colors = GDK_NONE;
static GdkAtom atom_rcfiles = GDK_NONE;
static gboolean REPAINT_IN_PROGRESS = FALSE;

char *rccolor = "xfcolors";
char *gtkrcfile = "/.gtkrc";

unsigned long
get_pixel_from_palette (XFCE_palette * p, short int col_index)
{
  return p->cm[col_index]->bg[GTK_STATE_NORMAL].pixel;
}

void
get_rgb_from_palette (XFCE_palette * p, short int col_index, int *r, int *g,
		      int *b)
{
  *r = p->r[col_index];
  *g = p->g[col_index];
  *b = p->b[col_index];
}

int
brightness (int r, int g, int b)
{
  return ((100 * (r + g + b) / 765));
}

int
brightness_pal (const XFCE_palette * p, int index)
{
  if ((p) && (index >= 0) && (index < NB_XFCE_COLORS))
    return (brightness ((short int) p->r[index],
			(short int) p->g[index], (short int) p->b[index]));
  else
    return (0);
}

char *
color_to_hex (char *s, const XFCE_palette * p, int index)
{
  char *t;
  t = (char *) malloc (3 * sizeof (char));

  if ((p) && (s) && (index >= 0) && (index < NB_XFCE_COLORS))
    {
      sprintf (s, "#%02X%02X%02X", (short int) p->r[index],
	       (short int) p->g[index], (short int) p->b[index]);
      return (s);
    }
  return (NULL);
}

void
set_selcolor (XFCE_palette * p, int color_index, gdouble color_table[])
{
  color_table[0] = ((double) p->r[color_index]) / 255;
  color_table[1] = ((double) p->g[color_index]) / 255;
  color_table[2] = ((double) p->b[color_index]) / 255;
}

void
set_palcolor (XFCE_palette * p, int color_index, gdouble color_table[])
{
  p->r[color_index] = (int) (color_table[0] * 255);
  p->g[color_index] = (int) (color_table[1] * 255);
  p->b[color_index] = (int) (color_table[2] * 255);
}

XFCE_palette *
newpal (void)
{
  XFCE_palette *p;

  p = (XFCE_palette *) malloc (sizeof (XFCE_palette));
  p->fnt = (char *) malloc (MAXSTRLEN);
  p->texture = (char *) malloc (MAXSTRLEN);
  strcpy (p->fnt, DEFAULTFONT);
  strcpy (p->texture, "");
  return (p);
}

void
set_font (XFCE_palette * p, char *font)
{
  if ((font) && (strlen (font)))
    strcpy (p->fnt, font);
  else
    strcpy (p->fnt, DEFAULTFONT);
}

void
set_texture (XFCE_palette * p, char *texture)
{
  if (texture && (strlen (texture)))
    strcpy (p->texture, texture);
  else
    strcpy (p->texture, "");
}

GtkStyle *
palcolor (XFCE_palette * p, short int index)
{
  if ((p) && (index >= 0) && (index < NB_XFCE_COLORS))
    return ((GtkStyle *) p->cm[index]);
  return (NULL);
}

void
freepal (XFCE_palette * p)
{
  int i;

  for (i = 0; i < NB_XFCE_COLORS; i++)
    {
      while (p->cm[i]->attach_count > 0)
	gtk_style_detach (p->cm[i]);
    }
  free (p->fnt);
  free (p->texture);
  free (p);
}

XFCE_palette *
copypal (XFCE_palette * d, const XFCE_palette * s)
{
  int i;

  if (d && s)
    {
      for (i = 0; i < NB_XFCE_COLORS; i++)
	{
	  d->cm[i] = gtk_style_copy (s->cm[i]);
	  d->r[i] = s->r[i];
	  d->g[i] = s->g[i];
	  d->b[i] = s->b[i];
	}
      strcpy (d->fnt, s->fnt);
      strcpy (d->texture, s->texture);
      return (d);
    }
  return (NULL);
}

XFCE_palette *
copyvaluepal (XFCE_palette * d, const XFCE_palette * s)
{
  int i;

  if (d && s)
    {
      for (i = 0; i < NB_XFCE_COLORS; i++)
	{
	  d->r[i] = s->r[i];
	  d->g[i] = s->g[i];
	  d->b[i] = s->b[i];
	}
      strcpy (d->fnt, s->fnt);
      strcpy (d->texture, s->texture);
      return (d);
    }
  return (NULL);
}

void
makepalstyle (XFCE_palette * pal, int i)
{
  GtkStyle *style;
  style = applycolor (pal, i, pal->r[i], pal->g[i], pal->b[i]);
  pal->cm[i] = style;
}

void
initpal (XFCE_palette * p)
{
  if (p)
    {
      if (p->fnt)
	strcpy (p->fnt, DEFAULTFONT);
      if (p->texture)
	strcpy (p->texture, "");
    }
}

void
write_style_to_gtkrc_file (FILE * f, XFCE_palette * p, int i,
			   gboolean textured)
{
  int howbright;

  howbright = brightness (p->r[i], p->g[i], p->b[i]);

  fprintf (f, "style \"xfce_%u\"\n", i);
  fprintf (f, "{\n");
  if ((p->fnt) && strlen (p->fnt))
    fprintf (f, "  font              = \"%s\"\n", p->fnt);

  if ((textured) && (p->texture) && (strlen (p->texture)))
    {
      fprintf (f, "  bg_pixmap[NORMAL] = \"%s\"\n", p->texture);
      fprintf (f, "  bg_pixmap[ACTIVE] = \"%s\"\n", p->texture);
      fprintf (f, "  bg_pixmap[INSENSITIVE] = \"%s\"\n", p->texture);
      fprintf (f, "  bg_pixmap[PRELIGHT] = \"%s\"\n", p->texture);
    }

  if (howbright < fadeblack)
    {
      fprintf (f, "  fg[NORMAL]        = \"#ffffff\"\n");
      fprintf (f, "  text[NORMAL]      = \"#ffffff\"\n");
      fprintf (f, "  fg[ACTIVE]        = \"#ffffff\"\n");
      fprintf (f, "  text[ACTIVE]      = \"#ffffff\"\n");
      fprintf (f, "  fg[PRELIGHT]      = \"#ffffff\"\n");
      fprintf (f, "  text[PRELIGHT]    = \"#ffffff\"\n");
      fprintf (f, "  fg[SELECTED]      = \"#ffffff\"\n");
      fprintf (f, "  text[SELECTED]    = \"#ffffff\"\n");
      fprintf (f, "  fg[INSENSITIVE]   = \"#%02X%02X%02X\"\n",
	       (short int) shift (p->r[i], LO_MULT) >> 1,
	       (short int) shift (p->g[i], LO_MULT) >> 1,
	       (short int) shift (p->b[i], LO_MULT) >> 1);
      fprintf (f, "  text[INSENSITIVE] = \"#%02X%02X%02X\"\n",
	       (short int) shift (p->r[i], LO_MULT) >> 1,
	       (short int) shift (p->g[i], LO_MULT) >> 1,
	       (short int) shift (p->b[i], LO_MULT) >> 1);
    }
  else
    {
      fprintf (f, "  fg[NORMAL]        = \"#000000\"\n");
      fprintf (f, "  text[NORMAL]      = \"#000000\"\n");
      fprintf (f, "  fg[ACTIVE]        = \"#000000\"\n");
      fprintf (f, "  text[ACTIVE]      = \"#000000\"\n");
      fprintf (f, "  fg[PRELIGHT]      = \"#000000\"\n");
      fprintf (f, "  text[PRELIGHT]    = \"#000000\"\n");
      fprintf (f, "  fg[SELECTED]      = \"#000000\"\n");
      fprintf (f, "  text[SELECTED]    = \"#000000\"\n");
      fprintf (f, "  fg[INSENSITIVE]   = \"#000000\"\n");
      fprintf (f, "  text[INSENSITIVE] = \"#000000\"\n");
    }

  fprintf (f, "  bg[NORMAL]        = \"#%02X%02X%02X\"\n",
	   (short int) p->r[i], (short int) p->g[i], (short int) p->b[i]);

  fprintf (f, "  base[NORMAL]      = \"#%02X%02X%02X\"\n",
	   (short int) p->r[i], (short int) p->g[i], (short int) p->b[i]);

  fprintf (f, "  bg[ACTIVE]        = \"#%02X%02X%02X\"\n",
	   (short int) shift (p->r[i], LO_MULT),
	   (short int) shift (p->g[i], LO_MULT),
	   (short int) shift (p->b[i], LO_MULT));

  fprintf (f, "  base[ACTIVE]      = \"#%02X%02X%02X\"\n",
	   (short int) shift (p->r[i], LO_MULT),
	   (short int) shift (p->g[i], LO_MULT),
	   (short int) shift (p->b[i], LO_MULT));

  fprintf (f, "  bg[PRELIGHT]      = \"#%02X%02X%02X\"\n",
	   (short int) p->r[i], (short int) p->g[i], (short int) p->b[i]);

  fprintf (f, "  base[PRELIGHT]    = \"#%02X%02X%02X\"\n",
	   (short int) p->r[i], (short int) p->g[i], (short int) p->b[i]);

  fprintf (f, "  bg[SELECTED]      = \"#%02X%02X%02X\"\n",
	   (short int) shift (p->r[i], HI_MULT),
	   (short int) shift (p->g[i], HI_MULT),
	   (short int) shift (p->b[i], HI_MULT));

  fprintf (f, "  base[SELECTED]    = \"#%02X%02X%02X\"\n",
	   (short int) shift (p->r[i], HI_MULT),
	   (short int) shift (p->g[i], HI_MULT),
	   (short int) shift (p->b[i], HI_MULT));

  fprintf (f, "  bg[INSENSITIVE]   = \"#%02X%02X%02X\"\n",
	   (short int) p->r[i], (short int) p->g[i], (short int) p->b[i]);

  fprintf (f, "  base[INSENSITIVE] = \"#%02X%02X%02X\"\n",
	   (short int) p->r[i], (short int) p->g[i], (short int) p->b[i]);
  fprintf (f, "}\n");
}

void
create_gtkrc_file (XFCE_palette * p)
{
  FILE *f;
  char homedir[MAXSTRLEN + 1];
  char lineread[80];
  gboolean gtkrc_by_xfce = TRUE;

  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, gtkrcfile);

  if (existfile (homedir) && ((f = fopen (homedir, "r"))))
    {
      fgets (lineread, 79, f);
      if (strlen (lineread))
	{
	  lineread[strlen (lineread) - 1] = 0;
	  gtkrc_by_xfce =
	    !strncmp (XFCE3GTKRC, lineread, strlen (XFCE3GTKRC));
	}
      fclose (f);
    }

  if (gtkrc_by_xfce)
    {
      if ((f = fopen (homedir, "w")))
	{
	  fprintf (f, "%s\n\n", XFCE3GTKRC);
	  fprintf (f, "pixmap_path \"%s:.:/\"\n\n", build_path ("/"));
	  write_style_to_gtkrc_file (f, p, 1, FALSE);
	  write_style_to_gtkrc_file (f, p, 4, FALSE);
	  write_style_to_gtkrc_file (f, p, 5, FALSE);
	  write_style_to_gtkrc_file (f, p, 7, TRUE);
	  fprintf (f, "style \"tooltips-style\" {\n");
	  fprintf (f, "  bg[NORMAL] = \"#ffffc0\"\n");
	  fprintf (f, "  fg[NORMAL] = \"#000000\"\n");
	  fprintf (f, "  bg_pixmap[NORMAL] = \"<none>\"\n");
	  fprintf (f, "}\n\n");
	  fprintf (f, "widget_class \"*GtkProgress*\" style \"xfce_1\"\n");
	  fprintf (f, "widget_class \"*Gtk*List*\"    style \"xfce_4\"\n");
	  fprintf (f, "widget_class \"*Gtk*Tree*\"    style \"xfce_4\"\n");
	  fprintf (f, "widget_class \"*GtkText*\"     style \"xfce_5\"\n");
	  fprintf (f, "widget_class \"*GtkEntry*\"    style \"xfce_5\"\n");
	  fprintf (f, "widget_class \"*\"             style \"xfce_7\"\n");
	  fprintf (f,
		   "widget       \"gtk-tooltips\"  style \"tooltips-style\"\n");
	  /* fprintf (f, "class \"GtkWidget\"            style \"xfce_7\"\n"); */
	  fclose (f);
	}
    }
}

GtkStyle *
applycolor (XFCE_palette * p, short int col_index, int r, int g, int b)
{
  int howbright;
  GtkStyle *style = NULL;

  p->r[col_index] = r;
  p->g[col_index] = g;
  p->b[col_index] = b;
  style = gtk_style_new ();
  gdk_font_unref (style->font);
  if ((style->font = gdk_font_load (p->fnt)) == NULL)
    if ((style->font = gdk_font_load (DEFAULTFONT)) == NULL)
      style->font = gdk_font_load ("fixed");
  howbright = brightness (p->r[col_index], p->g[col_index], p->b[col_index]);
  if (howbright < fadeblack)
    {
      style->fg[GTK_STATE_NORMAL].red = COLOR_GDK;
      style->fg[GTK_STATE_NORMAL].blue = COLOR_GDK;
      style->fg[GTK_STATE_NORMAL].green = COLOR_GDK;

      style->text[GTK_STATE_NORMAL].red = COLOR_GDK;
      style->text[GTK_STATE_NORMAL].blue = COLOR_GDK;
      style->text[GTK_STATE_NORMAL].green = COLOR_GDK;

      style->fg[GTK_STATE_ACTIVE].red = COLOR_GDK;
      style->fg[GTK_STATE_ACTIVE].blue = COLOR_GDK;
      style->fg[GTK_STATE_ACTIVE].green = COLOR_GDK;

      style->text[GTK_STATE_ACTIVE].red = COLOR_GDK;
      style->text[GTK_STATE_ACTIVE].blue = COLOR_GDK;
      style->text[GTK_STATE_ACTIVE].green = COLOR_GDK;

      style->fg[GTK_STATE_PRELIGHT].red = COLOR_GDK;
      style->fg[GTK_STATE_PRELIGHT].blue = COLOR_GDK;
      style->fg[GTK_STATE_PRELIGHT].green = COLOR_GDK;

      style->text[GTK_STATE_PRELIGHT].red = COLOR_GDK;
      style->text[GTK_STATE_PRELIGHT].blue = COLOR_GDK;
      style->text[GTK_STATE_PRELIGHT].green = COLOR_GDK;

      style->fg[GTK_STATE_SELECTED].red = COLOR_GDK;
      style->fg[GTK_STATE_SELECTED].blue = COLOR_GDK;
      style->fg[GTK_STATE_SELECTED].green = COLOR_GDK;

      style->text[GTK_STATE_SELECTED].red = COLOR_GDK;
      style->text[GTK_STATE_SELECTED].blue = COLOR_GDK;
      style->text[GTK_STATE_SELECTED].green = COLOR_GDK;

      style->fg[GTK_STATE_INSENSITIVE].red =
	shift (p->r[col_index], LO_MULT) << 7;
      style->fg[GTK_STATE_INSENSITIVE].blue =
	shift (p->b[col_index], LO_MULT) << 7;
      style->fg[GTK_STATE_INSENSITIVE].green =
	shift (p->g[col_index], LO_MULT) << 7;

      style->text[GTK_STATE_INSENSITIVE].red =
	shift (p->r[col_index], LO_MULT) << 7;
      style->text[GTK_STATE_INSENSITIVE].blue =
	shift (p->b[col_index], LO_MULT) << 7;
      style->text[GTK_STATE_INSENSITIVE].green =
	shift (p->g[col_index], LO_MULT) << 7;
    }
  else
    {
      style->fg[GTK_STATE_NORMAL].red = 0;
      style->fg[GTK_STATE_NORMAL].blue = 0;
      style->fg[GTK_STATE_NORMAL].green = 0;

      style->text[GTK_STATE_NORMAL].red = 0;
      style->text[GTK_STATE_NORMAL].blue = 0;
      style->text[GTK_STATE_NORMAL].green = 0;

      style->fg[GTK_STATE_ACTIVE].red = 0;
      style->fg[GTK_STATE_ACTIVE].blue = 0;
      style->fg[GTK_STATE_ACTIVE].green = 0;

      style->text[GTK_STATE_ACTIVE].red = 0;
      style->text[GTK_STATE_ACTIVE].blue = 0;
      style->text[GTK_STATE_ACTIVE].green = 0;

      style->fg[GTK_STATE_PRELIGHT].red = 0;
      style->fg[GTK_STATE_PRELIGHT].blue = 0;
      style->fg[GTK_STATE_PRELIGHT].green = 0;

      style->text[GTK_STATE_PRELIGHT].red = 0;
      style->text[GTK_STATE_PRELIGHT].blue = 0;
      style->text[GTK_STATE_PRELIGHT].green = 0;

      style->fg[GTK_STATE_SELECTED].red = 0;
      style->fg[GTK_STATE_SELECTED].blue = 0;
      style->fg[GTK_STATE_SELECTED].green = 0;

      style->text[GTK_STATE_SELECTED].red = 0;
      style->text[GTK_STATE_SELECTED].blue = 0;
      style->text[GTK_STATE_SELECTED].green = 0;

      style->fg[GTK_STATE_INSENSITIVE].red = 0;
      style->fg[GTK_STATE_INSENSITIVE].blue = 0;
      style->fg[GTK_STATE_INSENSITIVE].green = 0;

      style->text[GTK_STATE_INSENSITIVE].red = 0;
      style->text[GTK_STATE_INSENSITIVE].blue = 0;
      style->text[GTK_STATE_INSENSITIVE].green = 0;
    }

  style->bg[GTK_STATE_NORMAL].red = p->r[col_index] << 8;
  style->bg[GTK_STATE_NORMAL].blue = p->b[col_index] << 8;
  style->bg[GTK_STATE_NORMAL].green = p->g[col_index] << 8;

  style->base[GTK_STATE_NORMAL].red = p->r[col_index] << 8;
  style->base[GTK_STATE_NORMAL].blue = p->b[col_index] << 8;
  style->base[GTK_STATE_NORMAL].green = p->g[col_index] << 8;

  style->bg[GTK_STATE_ACTIVE].red = shift (p->r[col_index], LO_MULT) << 8;
  style->bg[GTK_STATE_ACTIVE].blue = shift (p->b[col_index], LO_MULT) << 8;
  style->bg[GTK_STATE_ACTIVE].green = shift (p->g[col_index], LO_MULT) << 8;

  style->base[GTK_STATE_ACTIVE].red = shift (p->r[col_index], LO_MULT) << 8;
  style->base[GTK_STATE_ACTIVE].blue = shift (p->b[col_index], LO_MULT) << 8;
  style->base[GTK_STATE_ACTIVE].green = shift (p->g[col_index], LO_MULT) << 8;

  style->bg[GTK_STATE_PRELIGHT].red = p->r[col_index] << 8;
  style->bg[GTK_STATE_PRELIGHT].blue = p->b[col_index] << 8;
  style->bg[GTK_STATE_PRELIGHT].green = p->g[col_index] << 8;

  style->base[GTK_STATE_PRELIGHT].red = p->r[col_index] << 8;
  style->base[GTK_STATE_PRELIGHT].blue = p->b[col_index] << 8;
  style->base[GTK_STATE_PRELIGHT].green = p->g[col_index] << 8;

  style->bg[GTK_STATE_SELECTED].red = shift (p->r[col_index], HI_MULT) << 8;
  style->bg[GTK_STATE_SELECTED].blue = shift (p->b[col_index], HI_MULT) << 8;
  style->bg[GTK_STATE_SELECTED].green = shift (p->g[col_index], HI_MULT) << 8;

  style->base[GTK_STATE_SELECTED].red = shift (p->r[col_index], HI_MULT) << 8;
  style->base[GTK_STATE_SELECTED].blue =
    shift (p->b[col_index], HI_MULT) << 8;
  style->base[GTK_STATE_SELECTED].green =
    shift (p->g[col_index], HI_MULT) << 8;

  style->bg[GTK_STATE_INSENSITIVE].red = p->r[col_index] << 8;
  style->bg[GTK_STATE_INSENSITIVE].blue = p->b[col_index] << 8;
  style->bg[GTK_STATE_INSENSITIVE].green = p->g[col_index] << 8;

  style->base[GTK_STATE_INSENSITIVE].red = p->r[col_index] << 8;
  style->base[GTK_STATE_INSENSITIVE].blue = p->b[col_index] << 8;
  style->base[GTK_STATE_INSENSITIVE].green = p->g[col_index] << 8;

  return style;
}

/***********************************************************************/
/*
  Intenal function that recursively scans all widgets 
  and replace old style by new one
 */
static void
widget_reset_style_recurse (GtkWidget * widget, gpointer p)
{
  if ((GTK_IS_INVISIBLE (widget)) || (!GTK_IS_WIDGET (widget)))
    return;
  if (gtk_widget_get_style (widget) == ((gtkstyle_pair *) p)->old)
    gtk_widget_set_style (widget, ((gtkstyle_pair *) p)->new);
  if (GTK_IS_CONTAINER (widget))
    {
      gtk_container_forall (GTK_CONTAINER (widget),
			    widget_reset_style_recurse, p);
    }
}

static void
widget_reset_styles (GtkWidget * widget, GtkStyle * old, GtkStyle * new)
{
  static gtkstyle_pair p;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WIDGET (widget));

  p.new = new;
  p.old = old;

  widget_reset_style_recurse (widget, &p);
}

/***********************************************************************/

void
applypal (XFCE_palette * p, GtkWidget * toplevel_widget)
{
  int i;
  GList *toplevels;
  GtkStyle *style;

  if (REPAINT_IN_PROGRESS)
    return;
  REPAINT_IN_PROGRESS = TRUE;
  /* Flush pending events */
  while (gtk_events_pending ())
    gtk_main_iteration_do (TRUE);
  for (i = 0; i < NB_XFCE_COLORS; i++)
    {
      style = applycolor (p, i, p->r[i], p->g[i], p->b[i]);
      gtk_widget_push_style (style);
      if ((toplevel_widget != NULL) && GTK_IS_WIDGET (toplevel_widget))
	{
	  widget_reset_styles (toplevel_widget, p->cm[i], style);
	}
      else
	{
	  toplevels = gdk_window_get_toplevels ();
	  while (toplevels)
	    {
	      GtkWidget *widget;
	      gdk_window_get_user_data (toplevels->data,
					(gpointer *) & widget);
	      if ((widget) && GTK_IS_WIDGET (widget))
		widget_reset_styles (widget, p->cm[i], style);
	      toplevels = toplevels->next;
	    }
	  g_list_free (toplevels);
	}
      p->cm[i] = style;
      gtk_widget_pop_style ();
    }
  gtk_widget_set_default_style (p->cm[7]);
  REPAINT_IN_PROGRESS = FALSE;
}

void
defpal (XFCE_palette * p)
{
  int i;

  initpal (p);
  p->r[0] = 50;
  p->g[0] = 50;
  p->b[0] = 50;
  p->r[1] = 197;
  p->g[1] = 189;
  p->b[1] = 182;
  p->r[2] = 197;
  p->g[2] = 189;
  p->b[2] = 182;
  p->r[3] = 197;
  p->g[3] = 189;
  p->b[3] = 182;
  p->r[4] = 197;
  p->g[4] = 189;
  p->b[4] = 182;
  p->r[5] = 197;
  p->g[5] = 189;
  p->b[5] = 182;
  p->r[6] = 145;
  p->g[6] = 129;
  p->b[6] = 160;
  p->r[7] = 180;
  p->g[7] = 178;
  p->b[7] = 176;
  strcpy (p->fnt, DEFAULTFONT);
  for (i = 0; i < NB_XFCE_COLORS; i++)
    {
      p->cm[i] = applycolor (p, i, p->r[i], p->g[i], p->b[i]);
      gdk_font_unref (p->cm[i]->font);
      if ((p->cm[i]->font = gdk_font_load (p->fnt)) == NULL)
	p->cm[i]->font = gdk_font_load ("fixed");
    }
  gtk_widget_set_default_style (p->cm[7]);
}

int
savenamepal (XFCE_palette * p, const char *name)
{
  FILE *f;
  int i;

  if ((f = fopen (name, "w")))
    {
      for (i = 0; i < NB_XFCE_COLORS; i++)
	fprintf (f, "%i %i %i\n", p->r[i], p->g[i], p->b[i]);
      if ((p->fnt) && strlen (p->fnt))
	fprintf (f, "%s\n", p->fnt);
      if ((p->texture) && strlen (p->texture))
	fprintf (f, "%s\n", p->texture);
      fclose (f);
    }
  return ((f != NULL));
}

int
loadnamepal (XFCE_palette * p, const char *name)
{
  char *lineread, *a;
  FILE *f;
  int i, err = 0;

  lineread = (char *) malloc ((80) * sizeof (char));

  if ((f = fopen (name, "r")))
    {
      for (i = 0; i < NB_XFCE_COLORS; i++)
	{
	  fgets (lineread, 79, f);
	  if (strlen (lineread))
	    {
	      lineread[strlen (lineread) - 1] = 0;
	      if ((a = strtok (lineread, " ")))
		p->r[i] = atoi (a);
	      else
		err = 1;
	      if ((a = strtok (NULL, " ")))
		p->g[i] = atoi (a);
	      else
		err = 1;
	      if ((a = strtok (NULL, " ")))
		p->b[i] = atoi (a);
	      else
		err = 1;
	    }
	}
      fgets (lineread, MAXSTRLEN - 1, f);
      if (!feof (f))
	{
	  if (strlen (lineread))
	    {
	      lineread[strlen (lineread) - 1] = 0;
	      strcpy (p->fnt, lineread);
	    }
	}
      else
	strcpy (p->fnt, DEFAULTFONT);
      /* And the texture, if available */
      fgets (lineread, MAXSTRLEN - 1, f);
      if (!feof (f))
	{
	  if (strlen (lineread))
	    {
	      lineread[strlen (lineread) - 1] = 0;
	      strcpy (p->texture, lineread);
	    }
	}
      else
	strcpy (p->texture, "");
      fclose (f);
    }
  free (lineread);
  return ((!err && (f != NULL)));
}

int
savepal (XFCE_palette * p)
{
  char *homedir;
  int x;

  homedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/.");
  strcat (homedir, rccolor);
  x = savenamepal (p, homedir);
  free (homedir);
  return (x);
}

int
loadpal (XFCE_palette * p)
{
  char *homedir;
  int x;

  homedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/.");
  strcat (homedir, rccolor);
  x = loadnamepal (p, homedir);
  if (!x)
    {
      strcpy (homedir, XFCE_CONFDIR);
      strcat (homedir, "/");
      strcat (homedir, rccolor);
      x = loadnamepal (p, homedir);
    }
  free (homedir);
  return (x);
}

gint
xfce_window_client_event (GtkWidget * widget,
			  GdkEventClient * event, gpointer p)
{
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_WINDOW (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (!atom_read_colors)
    atom_read_colors = gdk_atom_intern (atom_read_colors_name, FALSE);

  if ((event->message_type == atom_read_colors) && (p))
    {
      loadpal ((XFCE_palette *) p);
      applypal ((XFCE_palette *) p, NULL);
    }
  return FALSE;
}

void
reg_xfce_app (GtkWidget * widget, XFCE_palette * p)
{
  /*
     g_return_if_fail (widget != NULL);
     g_return_if_fail (GTK_IS_WINDOW (widget));

     gtk_signal_connect (GTK_OBJECT (widget), "client_event", 
     GTK_SIGNAL_FUNC (xfce_window_client_event), 
     (gpointer) p);
   */
}

void
applypal_to_all (void)
{
  GdkEventClient sev;
  int i;

  /*
     if (!atom_read_colors)
     atom_read_colors = gdk_atom_intern(atom_read_colors_name, FALSE);
   */
  if (!atom_rcfiles)
    atom_rcfiles = gdk_atom_intern ("_GTK_READ_RCFILES", FALSE);

  for (i = 0; i < 5; i++)
    sev.data.l[i] = 0;
  sev.data_format = 32;
  sev.message_type = atom_rcfiles;
  gdk_event_send_clientmessage_toall ((GdkEvent *) & sev);
}

gboolean repaint_in_progress (void)
{
  return REPAINT_IN_PROGRESS;
}
