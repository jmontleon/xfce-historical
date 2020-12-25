/*

   ORIGINAL FILE NAME : xfbd.c

   ********************************************************************
   *                                                                  *
   *           X F C E  - Written by O. Fourdan (c) 1997              *
   *                                                                  *
   *           This software is absolutely free of charge             *
   *                                                                  *
   ********************************************************************

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR (O. FOURDAN) BE 
   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
 */



#include <stdlib.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>
#include "constant.h"
#include "forms.h"
#include "color.h"
#include "xfbd.h"
#include "my_string.h"
#include "my_goodies.h"
#include "fileutil.h"
#include "empty.h"


/*
   Global variables and constants
 */

#define RCFILE   ".xfbdrc"
#define NOBACK   "None"
#define BACKTYPE "*.xpm"

char *backdrp;
char *display_name = NULL;
XFCE_palette *palette;
FD_backdrp *fd_backdrp;
Pixmap rootXpm;

void
display_error(char *s)
{
  fprintf (stderr, "%s\n", s);
  my_alert (s, 1);
}

int
setroot (char *str)
{
/****************************************************************************
 * This is an all new program to set the root window to an Xpm pixmap.
 * Copyright 1993, Rob Nation 
 * You may use this file for anything you want, as long as the copyright
 * is kept intact. No guarantees of any sort are made in any way regarding
 * this program or anything related to it.
 ****************************************************************************/

  XWindowAttributes root_attr;
  XpmAttributes xpm_attributes;
  Pixmap shapeMask;
  int val;

  if (my_strncasecmp (str, NOBACK, strlen (NOBACK)) && existfile (str))
    {
      XGetWindowAttributes (fl_display, fl_root, &root_attr);
      xpm_attributes.colormap = root_attr.colormap;
      xpm_attributes.valuemask = XpmSize | XpmReturnPixels | XpmColormap;
      if ((val = XpmReadFileToPixmap (fl_display, fl_root, str,
				      &rootXpm, &shapeMask,
				      &xpm_attributes)) != XpmSuccess)
	{
	  if (val == XpmOpenFailed)
	    display_error("XFBD Error :\nCannot open file");
	  else if (val == XpmColorFailed)
	    display_error("XFBD Error :\nCannot allocate colors\n");
	  else if (val == XpmFileInvalid)
	    display_error("XFBD Error :\nFile format is invalid\n");
	  else if (val == XpmColorError)
	    display_error("XFBD Error :\nColor invalid\n");
	  else if (val == XpmNoMemory)
	    display_error("XFBD Error :\nNot enough Memory\n");
	  return (0);
	}
      XSetWindowBackgroundPixmap (fl_display, fl_root, rootXpm);
      XClearWindow (fl_display, fl_root);
    }
  return (1);
}

void
defroot (char *str)
{
/****************************************************************************
 * This is an all new program to set the root window to an Xpm pixmap.
 * Copyright 1993, Rob Nation 
 * You may use this file for anything you want, as long as the copyright
 * is kept intact. No guarantees of any sort are made in any way regarding
 * this program or anything related to it.
 ****************************************************************************/

  unsigned char *data;
  Atom prop, type;
  int format;
  unsigned long length, after;

  if (my_strncasecmp (str, NOBACK, strlen (NOBACK)) && existfile (str))
    {
      if (setroot (str))
        {
          prop = XInternAtom (fl_display, "_XSETROOT_ID", False);
          (void) XGetWindowProperty (fl_display, fl_root, prop, 0L, 1L, True, AnyPropertyType, &type, &format, &length, &after, &data);
          if ((type == XA_PIXMAP) && (format == 32) && (length == 1) && (after == 0))
	    XKillClient (fl_display, *((Pixmap *) data));
          XChangeProperty (fl_display, fl_root, prop, XA_PIXMAP, 32, PropModeReplace, (unsigned char *) &rootXpm, 1);
          XSetCloseDownMode (fl_display, RetainPermanent);
        }
    }
  free (backdrp);
  free (fd_backdrp);
  freepal (palette);
  fl_finish ();
  exit (0);
}

void
readstr (char *str)
{
  char *homedir;
  char *lineread;
  char *pixfile;
  FILE *f = NULL;

  homedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  lineread = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  pixfile = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/");
  strcat (homedir, RCFILE);
  if (existfile (homedir))
    {
      f = fopen (homedir, "r");
    }
  else
    {
      fprintf (stderr, "%s File not found.\n", homedir);
      strcpy (homedir, XFCE_DIR);
      strcat (homedir, "/");
      strcat (homedir, RCFILE);
      if (existfile (homedir))
	{
	  f = fopen (homedir, "r");
	}
      else
        {
          fprintf (stderr, "%s File not found.\n", homedir);
          strcpy (homedir, XFCE_DIR);
          strcat (homedir, "/");
          strcat (homedir, "/sample");
          strcat (homedir, RCFILE);
          if (existfile (homedir))
   	    {
	      f = fopen (homedir, "r");
	    }
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
}

int
writestr (char *str)
{
  char *homedir;
  char *lineread;
  char *pixfile;
  FILE *f;

  homedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  lineread = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  pixfile = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/");
  strcat (homedir, RCFILE);

  if ((f = fopen (homedir, "w")))
    {
      fprintf (f, "%s\n", str);
      fclose (f);
    }
  return ((f != NULL));
}

/* callbacks and freeobj handles for form backdrp */

void
display_back (char *str)
{
  if (my_strncasecmp (str, NOBACK, strlen (NOBACK)) && existfile (str))
    fl_set_pixmap_file (fd_backdrp->preview, str);
  else
    fl_set_pixmap_data (fd_backdrp->preview, empty);
  fl_redraw_form (fd_backdrp->backdrp);
}

void
show_form (char *title)
{
  display_back (backdrp);
  fl_set_input (fd_backdrp->filename_inp, backdrp);
  fl_show_form (fd_backdrp->backdrp, FL_PLACE_FREE_CENTER, FL_FULLBORDER, title);
  fl_set_cursor (fd_backdrp->backdrp->window, XFCE_CURS);
}

void
hide_form (void)
{
  fl_hide_form (fd_backdrp->backdrp);
}

void
load_cb (FL_OBJECT * ob, long data)
{
  const char *fselect;

  fselect = fl_show_fselector ("Backdrops", XFCE_BACKDROPS, BACKTYPE, "");
  if (fselect)
    if (strlen (fselect))
      {
	fl_set_input (fd_backdrp->filename_inp, fselect);
	strcpy (backdrp, fselect);
	display_back (backdrp);
      }
}

void
update_cb (FL_OBJECT * ob, long data)
{
  backdrp = cleanup ((char *) fl_get_input (fd_backdrp->filename_inp));
  if (!strlen (backdrp))
    strcpy (backdrp, NOBACK);
  display_back (backdrp);
  setroot (backdrp);
}

void
cancel_cb (FL_OBJECT * ob, long data)
{
  hide_form ();
  free (backdrp);
  free (fd_backdrp);
  freepal (palette);
  fl_finish ();
  exit (0);
}

void
ok_cb (FL_OBJECT * ob, long data)
{
  update_cb (NULL, 0);
  writestr (backdrp);
  hide_form ();
  defroot (backdrp);
}

void
clear_cb (FL_OBJECT * ob, long data)
{
  fl_set_input (fd_backdrp->filename_inp, NOBACK);
  strcpy (backdrp, NOBACK);
  display_back (backdrp);
}

void
form_atclose (FL_FORM * f, long *data)
{
  /* Closing the window is equivalent to canceling the action */
  hide_form ();
}

FD_backdrp *
create_form_backdrp (void)
{
  FL_OBJECT *obj;
  FD_backdrp *fdui = (FD_backdrp *) fl_calloc (1, sizeof (*fdui));

  fdui->backdrp = fl_bgn_form (FL_NO_BOX, 380, 410);
  fl_set_form_minsize (fdui->backdrp, 380, 410);
  fl_set_form_maxsize (fdui->backdrp, 570, 615);
  obj = fl_add_box (FL_UP_BOX, 0, 0, 380, 410, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);

  obj = fl_add_frame (FL_ENGRAVED_FRAME, 10, 350, 360, 50, "");
  obj = fl_add_box (FL_DOWN_BOX, 10, 10, 360, 290, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);

  fdui->preview = obj = fl_add_pixmap (FL_NORMAL_PIXMAP, 20, 20, 340, 270, "");

  fdui->ok_btn = obj = fl_add_button (FL_NORMAL_BUTTON, 20, 360, 60, 30, "Ok");
  fl_set_button_shortcut (obj, "^O^M#O", 1);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFBOLD + FL_ENGRAVED_STYLE);
  fl_set_object_callback (obj, ok_cb, 0);

  fdui->load_btn = obj = fl_add_button (FL_NORMAL_BUTTON, 90, 360, 60, 30, "Load");
  fl_set_button_shortcut (obj, "^L#L", 1);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFBOLD + FL_ENGRAVED_STYLE);
  fl_set_object_callback (obj, load_cb, 0);

  fdui->apply_btn = obj = fl_add_button (FL_NORMAL_BUTTON, 160, 360, 60, 30, "Apply");
  fl_set_button_shortcut (obj, "^A#A", 1);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFBOLD + FL_ENGRAVED_STYLE);
  fl_set_object_callback (obj, update_cb, 0);

  fdui->clear_btn = obj = fl_add_button (FL_NORMAL_BUTTON, 230, 360, 60, 30, "Clear");
  fl_set_button_shortcut (obj, "^E#E", 1);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFBOLD + FL_ENGRAVED_STYLE);
  fl_set_object_callback (obj, clear_cb, 0);

  fdui->cancel_btn = obj = fl_add_button (FL_NORMAL_BUTTON, 300, 360, 60, 30, "Cancel");
  fl_set_button_shortcut (obj, "^[^C#C", 1);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFBOLD + FL_ENGRAVED_STYLE);
  fl_set_object_callback (obj, cancel_cb, 0);

  fdui->filename_inp = obj = fl_add_input (FL_NORMAL_INPUT, 80, 310, 220, 30, "Filename :");
  fl_set_object_color (obj, XFCE_COL5, XFCE_COL5);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFBOLD + FL_SHADOW_STYLE);
  fl_set_object_callback (obj, update_cb, 0);
  fl_end_form ();

  return (fdui);
}

/*---------------------------------------*/

int 
main (int argc, char *argv[])
{
  FL_IOPT flopt;
  int version, revision;

  if (FL_INCLUDE_VERSION != fl_library_version (&version, &revision))
    {
      display_error("XFBD : Header and library version don't match\n");
      fprintf (stderr, "   Header: %d.%d\n", FL_VERSION, FL_REVISION);
      fprintf (stderr, "  Library: %d.%d\n", version, revision);
      exit (1);
    }

  flopt.borderWidth = -BORDERWIDTH;
  fl_set_defaults (FL_PDBorderWidth, &flopt);
  if (!fl_initialize (&argc, argv, 0, 0, 0))
    exit(1);

#ifdef USE_SET_FONT_NAME
  fl_set_font_name (XFTINY, XFCE_TINY);
  fl_set_font_name (XFREG, XFCE_REGULAR);
  fl_set_font_name (XFBOLD, XFCE_BOLD);
  fl_set_font_name (XFBIG, XFCE_BIG);
  fl_set_font_name (XFTIT, XFCE_TITLE);
#endif

  palette = newpal ();
  initpal (palette);
  defpal (palette);
  loadpal (palette);
  applypal (palette);
  fl_set_cursor_color(XFCE_CURS, palcolor(palette, 0), FL_WHITE);
  fl_set_cursor_color(MENU_CURS, palcolor(palette, 0), FL_WHITE);

  fl_set_goodies_font (FL_BOLD_STYLE, FL_NORMAL_SIZE);
  fl_set_oneliner_font (FL_NORMAL_STYLE, FL_NORMAL_SIZE);
  fl_set_oneliner_color (FL_BLUE, FL_WHEAT);
  fl_setpup_default_cursor(MENU_CURS);

  fd_backdrp = create_form_backdrp ();
  fl_set_form_atclose (fd_backdrp->backdrp, (FL_FORM_ATCLOSE) form_atclose, 0);

  /* fill-in form initialization code */
  backdrp = (char *) malloc (sizeof (char) * FILENAME_MAX);
  readstr (backdrp);

  if (argc == 2 && strcmp (argv[1], "-i") == 0)
    {
      /* show the first form */
      show_form ("XFbd Backdrop Management");
      fl_do_forms ();
    }
  if ((argc == 1) || (argc == 2 && strcmp (argv[1], "-d") == 0))
    {
      defroot (backdrp);
      return (0);
    }

  fprintf (stderr, "Usage : %s [OPTIONS]\n", argv[0]);
  fprintf (stderr, "   Where OPTIONS are :\n");
  fprintf (stderr, "   -i : interactive, prompts for backdrop to display\n");
  fprintf (stderr, "   -d : display, reads configuration and exit (default)\n\n");
  fprintf (stderr, "%s is part of the XFCE distribution, written by Olivier Fourdan\n\n", argv[0]);

  return (-1);
}