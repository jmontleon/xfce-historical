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
#include <sys/types.h>
#include <sys/wait.h>

#if defined ___AIX || defined _AIX || defined __QNX__ || defined ___AIXV3 || defined AIXV3 || defined _SEQUENT_
#include <sys/select.h>
#endif

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include "xfcolor.h"
#include "my_string.h"
#include "fileutil.h"
#include "xfce-common.h"
#include "lightdark.h"

#ifndef HAVE_SNPRINTF
#  include "snprintf.h"
#endif

#ifdef DMALLOC
#  include "dmalloc.h"
#endif

#define GTK_WIDGET_USER_STYLE(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_USER_STYLE) != 0)
#define atom_read_colors_name "_XFCE_READ_COLORS"

#define LIGHT 1.15
#define DARK  0.90

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
char *xfcegtkrcfile = "/.xfcegtkrc";
char *gtktemprcfile = "/.temp.xfcegtkrc";

char *resmap []= { "", /* Color 0 */
                   "*Menu*background", "", /* Color 1 */
	           "", /* Color 2 */
	           "", /* Color 3 */
	           "*text*background","*list*background","*Text*background",
                   "*List*background", "", /* Color 4 */
	           "*Scrollbar*foreground", "", /* Color 5 */
	           "", /* Color 6 */
	           "*background", "*Background",
                   "OpenWindows*WindowColor",
	           "Window.Color.Background",
	           "netscape*background",
                   "Netscape*background",
	           ".netscape*background",
                   "Ddd*background",
	           "Emacs*Background",
                   "Emacs*backgroundToolBarColor", "", /* Color 7 */
	           "*foreground", "*Foreground",
                   "OpenWindows.WindowForeground",
	           "Window.Color.Foreground",
	           "netscape*foreground",
                   "Netscape*foreground",
	           ".netscape*foreground",
                   "Ddd*foreground",
	           "Emacs*Foreground",
                   "Emacs*foregroundToolBarColor", "", /* Text Color */
                   NULL
	   	 };
                   
char *fntmap [] = { "*fontList", 
                     "Netscape*fontList", 
                     ".netscape*fontList", 
                     NULL 
                  };

char *additionnal [] = { "*DtTerm*shadowThickness: 1",
                         "*enableButtonTab: True",
                         "*enableEtchedInMenu: True",
                         "*enableMenuInCascade: True",
                         "*enableMultiKeyBindings: True",
                         "*enableThinThickness: True",
                         NULL
		       };

char *xres [] = { "/.Xdefaults",
                  "/.Xresources",
                  "/.xrdb",
                  NULL
                };

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

void apply_xpalette (XFCE_palette * p, gboolean add_or_remove)
{
  char buffer [256];
  char mode [8];
  int i = 0;
  int color = 0;
  int tube [2];
  int sonpid;
  char c [20];
  char *args[4] = { "xrdb", NULL, "-all", NULL };
  char *tempstr;

  /* Add or remove the properties to the display */
  if (add_or_remove)
    strcpy (mode, "-load");
  else
    strcpy (mode, "-remove");
  args[1] = mode;
  
  /* Initiate communication pipe... */
  if (pipe (tube) == -1)
    {
      fprintf (stderr, "XFce : cannot open pipe\n");
      return;
    }

  /* ...And fork */
  switch (sonpid = fork ())
    {
      case 0:
        close (0); dup (tube [0]);
        close (tube [0]);
        close (tube [1]);
        execvp (args[0], args);
        perror ("exec failed");
        _exit (0);
        break;
      case -1:
        fprintf (stderr, "XFce : cannot execute fork()\n");
        close (tube [0]);
        close (tube [1]);
        return;
        break;
      default:
       break;  
    }
  i = 0;
  while (resmap [i]) 
    {
      if (!strlen (resmap [i]))
        color++;
      else
        {
          color_to_hex (c, p, color);
          if (color < 8)
            snprintf (buffer, 255,"%s: %s\n", resmap [i], c); 
          else
            {
              int howbright;

              howbright = brightness (p->r[7], p->g[7], p->b[7]);
              if (howbright < fadeblack)
                snprintf (buffer, 255,"%s: #FFFFFF\n", resmap [i]); 
              else
                snprintf (buffer, 255,"%s: #000000\n", resmap [i]); 
            }
          write (tube [1], buffer, strlen (buffer));
        }
      i++;
    }
  i = 0;
  while (fntmap [i]) 
    {
      snprintf (buffer, 255,"%s: %s\n", fntmap [i], p->fnt);
      write (tube [1], buffer, strlen (buffer));
      i++;
    }
  i = 0;
  while (additionnal [i]) 
    {
      snprintf (buffer, 255,"%s\n", additionnal [i]);
      write (tube [1], buffer, strlen (buffer));
      i++;
    }
  close (tube [0]);
  close (tube [1]);
  /* Make sure the child process has completed before continuing */
#if HAVE_WAITPID
  waitpid (sonpid, NULL, WUNTRACED);
#elif HAVE_WAIT4
  wait4 (sonpid, NULL, WUNTRACED, NULL);
#else
# error One of waitpid or wait4 is needed.
#endif
  
  /* Once we've applied xfce colors, merge with user's defined settings */
  i = 0;
  tempstr = (char *) malloc (sizeof (char) * MAXSTRLEN);
  while (xres [i])
    {
      strcpy (tempstr, (char *) getenv ("HOME"));
      strcat (tempstr, xres [i]);
      if (existfile (tempstr))
        {
          strcpy (tempstr, "xrdb -merge -all ");
          strcat (tempstr, (char *) getenv ("HOME"));
          strcat (tempstr, xres [i]);
          system (tempstr);
        }
      i++;
    }
  free (tempstr);
}

void
get_rgb_from_palette (XFCE_palette * p, short int col_index, int *r, int *g,
		      int *b)
{
  *r = p->r[col_index];
  *g = p->g[col_index];
  *b = p->b[col_index];
}

char *
color_to_hex (char *s, const XFCE_palette * p, int index)
{
  if (!s)
    s = (char *) malloc (8 * sizeof (char));

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

unsigned long
get_pixel_from_palette (XFCE_palette * p, int color_index)
{
  XColor color;
  XWindowAttributes attributes;
  
  XGetWindowAttributes (GDK_DISPLAY (), 
                        GDK_WINDOW_XWINDOW (GDK_ROOT_PARENT ()), 
                        &attributes);
  color.red   = p->r[color_index] * 255;
  color.green = p->g[color_index] * 255;
  color.blue  = p->b[color_index] * 255;

  if (!XAllocColor (GDK_DISPLAY (), attributes.colormap, &color)) {
    fprintf (stderr, "*** Warning *** Cannot allocate color\n");
    return (WhitePixel (GDK_DISPLAY (), GDK_WINDOW_XWINDOW (GDK_ROOT_PARENT ())));
  }
  return color.pixel;
}

XFCE_palette *
newpal (void)
{
  XFCE_palette *p;

  p = (XFCE_palette *) malloc (sizeof (XFCE_palette));
  p->fnt = NULL;
  p->texture = NULL;
  p->engine  = NULL;

  initpal (p);
  return (p);
}

void
set_font (XFCE_palette * p, char *fnt)
{
  if (fnt && (strlen (fnt)))
    {
      p->fnt = realloc (p->fnt, (strlen (fnt) + 1) * sizeof (char));
      strcpy (p->fnt, fnt);
    }
  else
    {
      p->fnt = realloc (p->fnt, sizeof (char));
      strcpy (p->fnt, "");
    }
}

void
set_texture (XFCE_palette * p, char *texture)
{
  if (texture && (strlen (texture)))
    {
      p->texture = realloc (p->texture, (strlen (texture) + 1) * sizeof (char));
      strcpy (p->texture, texture);
    }
  else
    {
      p->texture = realloc (p->texture, sizeof (char));
      strcpy (p->texture, "");
    }
}

void
set_engine (XFCE_palette * p, char *engine)
{
  if (engine && (strlen (engine)))
    {
      p->engine = realloc (p->engine, (strlen (engine) + 1) * sizeof (char));
      strcpy (p->engine, engine);
    }
  else
    {
      p->engine = realloc (p->engine, sizeof (char));
      strcpy (p->engine, "");
    }
}

void
freepal (XFCE_palette * p)
{
  if (p->fnt)     
    free (p->fnt);
  if (p->texture) 
    free (p->texture);
  if (p->engine)  
    free (p->engine);
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
	  d->r[i] = s->r[i];
	  d->g[i] = s->g[i];
	  d->b[i] = s->b[i];
	}
      d->fnt     = realloc (d->fnt,     (strlen (s->fnt) + 1) * sizeof (char));
      d->texture = realloc (d->texture, (strlen (s->texture) + 1) * sizeof (char));
      d->engine  = realloc (d->engine,  (strlen (s->engine) + 1) * sizeof (char));
      strcpy (d->fnt,     s->fnt);
      strcpy (d->texture, s->texture);
      strcpy (d->engine,  s->engine);
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
      d->fnt     = realloc (d->fnt,     (strlen (s->fnt) + 1) * sizeof (char));
      d->texture = realloc (d->texture, (strlen (s->texture) + 1) * sizeof (char));
      d->engine  = realloc (d->engine,  (strlen (s->engine) + 1) * sizeof (char));
      strcpy (d->fnt,     s->fnt);
      strcpy (d->texture, s->texture);
      strcpy (d->engine,  s->engine);
      return (d);
    }
  return (NULL);
}

void
initpal (XFCE_palette * p)
{
  if (p)
    {
      p->fnt     = realloc (p->fnt,     (strlen (DEFAULTFONT) + 1) * sizeof (char));
      p->texture = realloc (p->texture, sizeof (char));
      p->engine  = realloc (p->engine,  (strlen (DEFAULTENGINE) + 1) * sizeof (char));
      strcpy (p->texture, "");
      strcpy (p->engine, DEFAULTENGINE);
      strcpy (p->fnt, DEFAULTFONT);
    }
}

void
write_style_to_gtkrc_file (FILE * f, XFCE_palette * p, int i,
			   char * template, gboolean textured)
{
  int howbright;

  howbright = brightness (p->r[i], p->g[i], p->b[i]);

  fprintf (f, "style \"%s%u\"\n", template, i);
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
  else
    {
      fprintf (f, "  bg_pixmap[NORMAL] = \"<none>\"\n");
      fprintf (f, "  bg_pixmap[ACTIVE] = \"<none>\"\n");
      fprintf (f, "  bg_pixmap[INSENSITIVE] = \"<none>\"\n");
      fprintf (f, "  bg_pixmap[PRELIGHT] = \"<none>\"\n");
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
	       (short int) shift (p->r[i], DARK) >> 1,
	       (short int) shift (p->g[i], DARK) >> 1,
	       (short int) shift (p->b[i], DARK) >> 1);
      fprintf (f, "  text[INSENSITIVE] = \"#%02X%02X%02X\"\n",
	       (short int) shift (p->r[i], DARK) >> 1,
	       (short int) shift (p->g[i], DARK) >> 1,
	       (short int) shift (p->b[i], DARK) >> 1);
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
	   (short int) shift (p->r[i], DARK),
	   (short int) shift (p->g[i], DARK),
	   (short int) shift (p->b[i], DARK));

  fprintf (f, "  base[ACTIVE]      = \"#%02X%02X%02X\"\n",
	   (short int) shift (p->r[i], DARK),
	   (short int) shift (p->g[i], DARK),
	   (short int) shift (p->b[i], DARK));

  fprintf (f, "  bg[PRELIGHT]      = \"#%02X%02X%02X\"\n",
	   (short int) shift (p->r[i], LIGHT),
	   (short int) shift (p->g[i], LIGHT),
	   (short int) shift (p->b[i], LIGHT));

  fprintf (f, "  base[PRELIGHT]    = \"#%02X%02X%02X\"\n",
	   (short int) p->r[i], (short int) p->g[i], (short int) p->b[i]);

  if (howbright > brightlim)
    {
      fprintf (f, "  bg[SELECTED]      = \"#%02X%02X%02X\"\n",
	       (short int) shift (p->r[i], DARK),
	       (short int) shift (p->g[i], DARK),
               (short int) shift (p->b[i], DARK));
      fprintf (f, "  base[SELECTED]    = \"#%02X%02X%02X\"\n",
	       (short int) shift (p->r[i], DARK),
	       (short int) shift (p->g[i], DARK),
	       (short int) shift (p->b[i], DARK));
    }
  else
    {
      fprintf (f, "  bg[SELECTED]      = \"#%02X%02X%02X\"\n",
	       (short int) shift (p->r[i], LIGHT),
	       (short int) shift (p->g[i], LIGHT),
               (short int) shift (p->b[i], LIGHT));
      fprintf (f, "  base[SELECTED]    = \"#%02X%02X%02X\"\n",
	       (short int) shift (p->r[i], LIGHT),
	       (short int) shift (p->g[i], LIGHT),
	       (short int) shift (p->b[i], LIGHT));
    }

  fprintf (f, "  bg[INSENSITIVE]   = \"#%02X%02X%02X\"\n",
	   (short int) p->r[i], (short int) p->g[i], (short int) p->b[i]);

  fprintf (f, "  base[INSENSITIVE] = \"#%02X%02X%02X\"\n",
	   (short int) p->r[i], (short int) p->g[i], (short int) p->b[i]);
  if ((p->engine) && (strlen (p->engine)) && my_strncasecmp (p->engine, "gtk", 3))
    fprintf (f, "  engine \"%s\" {}\n", p->engine);
  fprintf (f, "}\n");
}

void
create_gtkrc_file (XFCE_palette * p, char *name)
{
  FILE *f;
  char tempstr[MAXSTRLEN];
  char lineread[80];
  gboolean gtkrc_by_xfce = TRUE;
  int i;

  strcpy (tempstr, (char *) getenv ("HOME"));
  strcat (tempstr, (name ? name : gtkrcfile));

  if (existfile (tempstr) && ((f = fopen (tempstr, "r"))))
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
      if ((f = fopen (tempstr, "w")))
	{
	  fprintf (f, "%s\n\n", XFCE3GTKRC);
	  fprintf (f, "pixmap_path \"%s:.:/\"\n\n", build_path ("/"));
	  for (i = 1; i < 8; i++) /* Color 0 is used for the mouse pointer only */
            write_style_to_gtkrc_file (f, p, i, "xfce_", (i == 7));
	  fprintf (f, "style \"tooltips-style\" {\n");
	  fprintf (f, "  bg[NORMAL] = \"#ffffc0\"\n");
	  fprintf (f, "  fg[NORMAL] = \"#000000\"\n");
	  fprintf (f, "  bg_pixmap[NORMAL] = \"<none>\"\n");
	  fprintf (f, "}\n\n");
	  fprintf (f, "class        \"*\"               style \"xfce_7\"\n");
	  fprintf (f, "widget       \"*gxfce_color1*\"  style \"xfce_1\"\n");
	  fprintf (f, "widget       \"*gxfce_color2*\"  style \"xfce_2\"\n");
	  fprintf (f, "widget       \"*gxfce_color3*\"  style \"xfce_3\"\n");
	  fprintf (f, "widget       \"*gxfce_color4*\"  style \"xfce_4\"\n");
	  fprintf (f, "widget       \"*gxfce_color5*\"  style \"xfce_5\"\n");
	  fprintf (f, "widget       \"*gxfce_color6*\"  style \"xfce_6\"\n");
	  fprintf (f, "widget       \"*gxfce_color7*\"  style \"xfce_7\"\n");
	  fprintf (f, "widget_class \"*Gtk*Progress*\"  style \"xfce_1\"\n");
	  fprintf (f, "widget_class \"*Gtk*List*\"      style \"xfce_4\"\n");
	  fprintf (f, "widget_class \"*Gtk*Tree*\"      style \"xfce_4\"\n");
	  fprintf (f, "widget_class \"*Gtk*Text*\"      style \"xfce_5\"\n");
	  fprintf (f, "widget_class \"*Gtk*Entry*\"     style \"xfce_5\"\n");
	  fprintf (f, "widget_class \"*Gtk*Spin*\"      style \"xfce_5\"\n");
	  fprintf (f, "widget       \"gtk-tooltips\"   style \"tooltips-style\"\n");
	  fclose (f);
	}
    }
}

void
create_temp_gtkrc_file (XFCE_palette * p)
{
  FILE *f;
  char tempstr[MAXSTRLEN];
  int i;

  strcpy (tempstr, (char *) getenv ("HOME"));
  strcat (tempstr, gtktemprcfile);

  if ((f = fopen (tempstr, "w")))
    {
      fprintf (f, "%s\n\n", XFCE3GTKRC);
      fprintf (f, "pixmap_path \"%s:.:/\"\n\n", build_path ("/"));
      for (i = 0; i < 8; i++)
        write_style_to_gtkrc_file (f, p, i, "temp_xfce_", FALSE);
      fprintf (f, "widget       \"*temp_color0*\"  style \"temp_xfce_0\"\n");
      fprintf (f, "widget       \"*temp_color1*\"  style \"temp_xfce_1\"\n");
      fprintf (f, "widget       \"*temp_color2*\"  style \"temp_xfce_2\"\n");
      fprintf (f, "widget       \"*temp_color3*\"  style \"temp_xfce_3\"\n");
      fprintf (f, "widget       \"*temp_color4*\"  style \"temp_xfce_4\"\n");
      fprintf (f, "widget       \"*temp_color5*\"  style \"temp_xfce_5\"\n");
      fprintf (f, "widget       \"*temp_color6*\"  style \"temp_xfce_6\"\n");
      fprintf (f, "widget       \"*temp_color7*\"  style \"temp_xfce_7\"\n");
      fclose (f);
    }
}

void
apply_temp_gtkrc_file (XFCE_palette * p, GtkWidget *w) {
  char tempstr[MAXSTRLEN];

  strcpy (tempstr, (char *) getenv ("HOME"));
  strcat (tempstr, gtktemprcfile);
  create_temp_gtkrc_file (p);
  gtk_rc_parse (tempstr);
  gtk_widget_reset_rc_styles (w);
}

void
defpal (XFCE_palette * p)
{
  initpal (p);
  p->r[0] = 250;
  p->g[0] = 100;
  p->b[0] = 90;
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
  initpal (p);
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
      if ((p->engine) && strlen (p->engine))
	fprintf (f, "engine=%s\n", p->engine);
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
  gboolean font = FALSE;
  
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
      initpal (p);
      while (fgets (lineread, MAXSTRLEN - 1, f))
	{
	  if (strlen (lineread))
	    {
	      lineread[strlen (lineread) - 1] = 0;
              if (!my_strncasecmp (lineread, "engine=", strlen ("engine=")))
                {
                   char *s = &lineread[strlen ("engine=")];
                   p->engine = realloc (p->engine, (strlen (s) + 1) * sizeof (char));
	           strcpy (p->engine, s);
                }
              else if (!font)
                {
                   p->fnt = realloc (p->fnt, (strlen (lineread) + 1) * sizeof (char));
	           strcpy (p->fnt, lineread);
                   font = TRUE;
                }
              else
	        {
                  p->texture = realloc (p->texture, (strlen (lineread) + 1) * sizeof (char));
	          strcpy (p->texture, lineread);
	        }
            }
          }
      fclose (f);
    }
  free (lineread);
  return ((!err && (f != NULL)));
}

int
savepal (XFCE_palette * p)
{
  char *tempstr;
  int x;

  tempstr = (char *) malloc ((MAXSTRLEN) * sizeof (char));
  strcpy (tempstr, (char *) getenv ("HOME"));
  strcat (tempstr, "/.");
  strcat (tempstr, rccolor);
  x = savenamepal (p, tempstr);
  free (tempstr);
  return (x);
}

int
loadpal (XFCE_palette * p)
{
  char *tempstr;
  int x;

  tempstr = (char *) malloc ((MAXSTRLEN) * sizeof (char));
  strcpy (tempstr, (char *) getenv ("HOME"));
  strcat (tempstr, "/.");
  strcat (tempstr, rccolor);
  x = loadnamepal (p, tempstr);
  if (!x)
    {
      strcpy (tempstr, XFCE_CONFDIR);
      strcat (tempstr, "/");
      strcat (tempstr, rccolor);
      x = loadnamepal (p, tempstr);
    }
  free (tempstr);
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

void init_xfce_rcfile (void)
{
  char rcfile[MAXSTRLEN];

  strcpy (rcfile, (char *) getenv ("HOME"));
  strcat (rcfile, gtkrcfile);
  gtk_rc_parse(rcfile);
  strcpy (rcfile, (char *) getenv ("HOME"));
  strcat (rcfile, xfcegtkrcfile);
  gtk_rc_parse(rcfile);
  gtk_rc_add_default_file (rcfile);
  strcpy (rcfile, (char *) getenv ("HOME"));
  strcat (rcfile, gtktemprcfile);
  gtk_rc_parse(rcfile);
  gtk_rc_add_default_file (rcfile);
  gtk_rc_init ();
}
