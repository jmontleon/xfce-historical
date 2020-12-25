/*

   ORIGINAL FILE NAME : xfmouse_main.c

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


#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#include <stdio.h>
#include <stdlib.h>

#include "forms.h"
#include "xfmouse.h"

#define RCFILE                  ".xfmouserc"

FD_xfmouse *fd_xfmouse;
XFMouse mouseval;
res_XFMouse   rxfmouse;
XFCE_palette *palette;
char         *homedir,
             *tempstr;

void 
create_resources (res_XFMouse * r)
{
  r->xfmousetitle      = (char *) malloc (TITLELEN);
  r->oklabel           = (char *) malloc (BUTTONLEN);
  r->applylabel        = (char *) malloc (BUTTONLEN);
  r->cancellabel       = (char *) malloc (BUTTONLEN);
  r->buttonlabel       = (char *) malloc (BUTTONLEN);
  r->motionlabel       = (char *) malloc (BUTTONLEN);
  r->leftlabel         = (char *) malloc (BUTTONLEN);
  r->rightlabel        = (char *) malloc (BUTTONLEN);
  r->accelerationlabel = (char *) malloc (BUTTONLEN);
  r->thresholdlabel    = (char *) malloc (BUTTONLEN);
  r->tinyfont          = (char *) malloc (TEXTLEN);
  r->regularfont       = (char *) malloc (TEXTLEN);
  r->boldfont          = (char *) malloc (TEXTLEN);
  r->bigfont           = (char *) malloc (TEXTLEN);
  r->titlefont         = (char *) malloc (TEXTLEN);
}


void 
load_resources (res_XFMouse * r)
{
  FL_resource res[] =
  {
    {
      "dialog.xfmouseTitle",
      "XForm.xfmouseTitle",
      FL_STRING,
      r->xfmousetitle,
      "XFMouse - XFce Mouse Configurator",
      TITLELEN
    },
    {"button.okLabel",
     "XForm.okLabel",
     FL_STRING,
     r->oklabel,
     "Ok",
     BUTTONLEN
    },
    {
      "button.applyLabel",
      "XForm.applyLabel",
      FL_STRING,
      r->applylabel,
      "Apply",
      BUTTONLEN
    },
    {
      "button.cancelLabel",
      "XForm.cancelLabel",
      FL_STRING,
      r->cancellabel,
      "Cancel",
      BUTTONLEN
    },
    {
      "xfmouse.buttonLabel",
      "XForm.buttonLabel",
      FL_STRING,
      r->buttonlabel,
      "Button Settings",
      BUTTONLEN
    },
    {
      "xfmouse.motionLabel",
      "XForm.motionLabel",
      FL_STRING,
      r->motionlabel,
      "Motion Settings",
      BUTTONLEN
    },
    {
      "xfmouse.leftLabel",
      "XForm.leftLabel",
      FL_STRING,
      r->leftlabel,
      "Left",
      BUTTONLEN
    },
    {
      "xfmouse.rightLabel",
      "XForm.rightLabel",
      FL_STRING,
      r->rightlabel,
      "Right",
      BUTTONLEN
    },
    {
      "xfmouse.accelerationLabel",
      "XForm.accelerationLabel",
      FL_STRING,
      r->accelerationlabel,
      "Acceleration",
      BUTTONLEN
    },
    {
      "xfmouse.thresholdLabel",
      "XForm.thresholdLabel",
      FL_STRING,
      r->thresholdlabel,
      "Threshold",
      BUTTONLEN
    },
    {
      "tiny.font",
      "XForm.tiny.font",
      FL_STRING,
      r->tinyfont,
      "-adobe-helvetica-medium-r-*-*-10-*-75-75-*-*-*-*",
      TEXTLEN
    },
    {
      "regular.font",
      "XForm.regular.font",
      FL_STRING,
      r->regularfont,
      "-adobe-helvetica-medium-r-*-*-12-*-75-75-*-*-*-*",
      TEXTLEN
    },
    {
      "bold.font",
      "XForm.bold.font",
      FL_STRING,
      r->boldfont,
      "-adobe-helvetica-bold-r-*-*-12-*-75-75-*-*-*-*",
      TEXTLEN
    },
    {
      "big.font",
      "XForm.big.font",
      FL_STRING,
      r->bigfont,
      "-adobe-helvetica-medium-r-*-*-14-*-75-75-*-*-*-*",
      TEXTLEN
    },
    {
      "title.font",
      "XForm.title.font",
      FL_STRING,
      r->titlefont,
      "-adobe-helvetica-bold-r-*-*-14-*-75-75-*-*-*-*",
      TEXTLEN
    },
  };
  fl_get_app_resources (res, 15);
}

void 
free_resources (res_XFMouse * r)
{
  free (r->xfmousetitle);
  free (r->oklabel);
  free (r->applylabel);
  free (r->cancellabel);
  free (r->buttonlabel);
  free (r->motionlabel);
  free (r->leftlabel);
  free (r->rightlabel);
  free (r->accelerationlabel);
  free (r->thresholdlabel);
  free (r->tinyfont);
  free (r->regularfont);
  free (r->boldfont);
  free (r->bigfont);
  free (r->titlefont);
}

void
display_error(char *s)
{
  fprintf (stderr, "%s\n", s);
  my_alert (s, 1);
}


void
readstr (int i, char *str, FILE *f)
{
  if (str) 
    if (f) {
      fgets (str, i - 1, f);
      if (strlen (str))
        str[strlen (str) - 1] = 0;
    }
    else
      strcpy(str, "\0");
}

void
loadcfg (XFMouse *s)
{
  FILE *f = NULL;

  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/");
  strcat (homedir, RCFILE);
  if (existfile (homedir))
    {
      f = fopen (homedir, "r");
    }
  else
    {
      strcpy (homedir, XFCE_DIR);
      strcat (homedir, "/");
      strcat (homedir, RCFILE);
      if (existfile (homedir))
	{
	  f = fopen (homedir, "r");
	}
      else
        {
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
      readstr (10, tempstr, f);
      s->button = (my_strncasecmp (tempstr, "Right", strlen ("Right")) == 0);
      readstr (10, tempstr, f);
      s->accel = atoi (tempstr);
      readstr (10, tempstr, f);
      s->thresh = atoi (tempstr);
      if (s->accel < ACCEL_MIN)
        s->accel = ACCEL_MIN;
      else if (s->accel > ACCEL_MAX)
        s->accel = ACCEL_MIN;
      if (s->thresh < THRESH_MIN)
        s->thresh = THRESH_MIN;
      else if (s->thresh > THRESH_MAX)
        s->thresh = THRESH_MIN;
      fclose (f);
    }
  else
    {
      s->button = 1;
      s->accel  = 2 * DENOMINATOR;
      s->thresh = 4;
    }
}

int
savecfg (XFMouse *s)
{
  FILE *f;

  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/");
  strcat (homedir, RCFILE);

  if ((f = fopen (homedir, "w")))
    {
      fprintf (f, "%s\n", ((s->button) ? "Right" : "Left"));
      fprintf (f, "%i\n", s->accel);
      fprintf (f, "%i\n", s->thresh);
      fclose (f);
    }
  return ((f != NULL));
}

void
startgui(int argc, char *argv[])
{
   FL_IOPT flopt;
   int version, revision;
   char *xfce_lang, *env_value, *classname;


   if (FL_INCLUDE_VERSION != fl_library_version (&version, &revision))
     {
       display_error("XFMouse : Header and library version don't match\n");
       fprintf (stderr, "   Header: %d.%d\n", FL_VERSION, FL_REVISION);
       fprintf (stderr, "  Library: %d.%d\n", version, revision);
       exit (1);
     }

   flopt.borderWidth = -BORDERWIDTH;
   fl_set_defaults (FL_PDBorderWidth, &flopt);
   xfce_lang = (char *) malloc (3 * sizeof (char));
   env_value = (char *) getenv ("XFCE_LANG");
   if (env_value) {
     strncpy (xfce_lang, env_value, 2);
     xfce_lang[2] = '\0';
   }
   else
     xfce_lang[0] = '\0';

   classname = (char *) malloc (8 * sizeof (char));
   strcpy (classname, "XFce");
   if ((xfce_lang) && (strlen (xfce_lang))) {
     strcat (classname, "-");
     strcat (classname, xfce_lang);
   }

   if (!fl_initialize (&argc, argv, classname, 0, 0)) 
     exit (1);

   setlocale( LC_ALL, "");

   free (xfce_lang);
   free (classname);

   create_resources (&rxfmouse);
   load_resources (&rxfmouse);

   fl_set_font_name (XFTINY, rxfmouse.tinyfont);
   fl_set_font_name (XFREG,  rxfmouse.regularfont);
   fl_set_font_name (XFBOLD, rxfmouse.boldfont);
   fl_set_font_name (XFBIG,  rxfmouse.bigfont);
   fl_set_font_name (XFTIT,  rxfmouse.titlefont);

   palette = newpal ();
   initpal (palette);
   defpal (palette);
   loadpal (palette);
   applypal (palette);

   fd_xfmouse = create_form_xfmouse();

   /* show the first form */
   show_fd_xfmouse(rxfmouse.xfmousetitle, &mouseval);
   fl_set_cursor_color(XFCE_CURS, palcolor(palette, 0), FL_WHITE);
   fl_set_cursor_color(MENU_CURS, palcolor(palette, 0), FL_WHITE);
   fl_do_forms();
}

int main(int argc, char *argv[])
{
   homedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
   tempstr = (char *) malloc (16 * sizeof (char));
   loadcfg (&mouseval);

  if (argc == 2 && strcmp (argv[1], "-i") == 0)
    {
      /* show the first form */
     startgui(argc, argv);
    }
  if ((argc == 1) || (argc == 2 && strcmp (argv[1], "-d") == 0))
    {
     if (!fl_initialize (&argc, argv, "", 0, 0)) 
       exit (1);
      mouse_values (&mouseval);
      fl_finish ();
      return (0);
    }

  fprintf (stderr, "Usage : %s [OPTIONS]\n", argv[0]);
  fprintf (stderr, "   Where OPTIONS are :\n");
  fprintf (stderr, "   -i : interactive\n");
  fprintf (stderr, "   -d : apply configuration and exit (default)\n\n");
  fprintf (stderr, "%s is part of the XFce distribution, written by Olivier Fourdan\n\n", argv[0]);

  return 0;
}
