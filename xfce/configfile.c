

/*

   ORIGINAL FILE NAME : configfile.c

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
#include <X11/Xlib.h>
#include "forms.h"
#include "XFCE.h"
#include "selects.h"
#include "pulldown.h"
#include "addicon.h"
#include "constant.h"
#include "extern.h"
#include "screen.h"
#include "configfile.h"
#include "my_string.h"
#include "my_goodies.h"
#include "fileutil.h"
#include "command.h"
#include "tiptools.h"
#include "resources.h"

char *rcfile = ".xfcerc";
int nl = 0;
int buffersize = 256;

void 
syntax_error (char *s)
{
  fprintf (stderr, "XFCE :\nSyntax error in config file\n<%s>\n", s);
  my_alert (rxfce.syntax, 1);
  end_XFCE (2);
}

void 
data_error (char *s)
{
  fprintf (stderr, "XFCE :\nData mismatch error in config file\n<%s>\n", s);
  my_alert (rxfce.datamis, 1);
  end_XFCE (3);
}

char *
nextline (FILE * f, char *lineread)
{
  char *p;
  do
    {
      nl++;
      fgets (lineread, MAXSTRLEN + 1, f);
      lineread[strlen (lineread) - 1] = 0;
      p = skiphead (lineread);
    }
  while (!strlen (p) && !feof (f));
  if (strlen (p))
    skiptail (p);
  if (feof (f))
    syntax_error (rxfce.filetrunc);
  return (((strlen (p)) ? p : NULL));
}

void 
writetoconfig (void)
{
  char *homedir;
  char *buffer;
  char *backname;
  FILE *configfile = NULL;
  FILE *copyfile;
  FILE *backfile;
  int i, j;
  int nb_read;
  FL_Coord x, y, w, h;


  homedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/");
  strcat (homedir, rcfile);
  /*
     Backup any existing config file before creating a new one 
   */
  if (existfile (homedir))
    {
      backname = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
      buffer = (char *) malloc ((buffersize + 1) * sizeof (char));
      strcpy (backname, homedir);
      strcat (backname, BACKUPEXT);
      backfile = fopen (backname, "w");
      copyfile = fopen (homedir, "r");
      if ((configfile) && (copyfile))
	{
	  while ((nb_read = fread (buffer, 1, buffersize, copyfile)) > 0)
	    {
	      fwrite (buffer, 1, nb_read, backfile);
	    }
	  fflush (backfile);
	  fclose (backfile);
	  fclose (copyfile);
	}
      free (backname);
      free (buffer);
    }
  configfile = fopen (homedir, "w");

  if (!configfile)
    my_alert (rxfce.notcreate, 1);
  else
    {
      fprintf (configfile, "[Coords]\n");
      compute_wm_window (fd_XFCE->XFCE->window, &x, &y, &w, &h);
      fprintf (configfile, "\t%i\n", x);
      fprintf (configfile, "\t%i\n", y);
      fprintf (configfile, "[ButtonLabels]\n");
      for (i = 0; i < NBSCREENS; i++)
	fprintf (configfile, "\t%s\n", fd_XFCE->screen[i]->label);
      fprintf (configfile, "[External_Icons]\n");
      for (i = 0; i < NBSELECTS; i++)
        if((strlen(selects[i].ext_icon)) 
            && (strlen(selects[i].ext_icon)))
  	  fprintf (configfile, "\t%s\n", selects[i].ext_icon);
        else
	  fprintf (configfile, "\tNone\n");
      fprintf (configfile, "[Icons]\n");
      fprintf (configfile, "\t%s\n", save_icon_str ());
      fprintf (configfile, "[WorkSpace]\n");
      fprintf (configfile, COLORIZE_ROOT ? "\tRepaint\n" : "\tNoRepaint\n");
      fprintf (configfile, GRADIENT_ROOT ? "\tGradient\n" : "\tSolid\n");
      fprintf (configfile, "[Lock]\n");
      fprintf (configfile, "\t%s\n", selects[NBSELECTS].command);
      fprintf (configfile, "[MenuOption]\n");
      fprintf (configfile, DETACH_MENU ? "\tDetach\n" : "\tNoDetach\n");
      fprintf (configfile, "[XFwmOption]\n");
      fprintf (configfile, CLICKTOFOCUS ? "\tClickToFocus\n" : "\tFollowMouse\n");
      fprintf (configfile, OPAQUEMOVE ? "\tOpaqueMove\n" : "\tNoOpaqueMove\n");
      fprintf (configfile, AUTORAISE ? "\tAutoraise\n" : "\tNoAutoraise\n");
      fprintf (configfile, "[Commands]\n");
      for (i = 0; i < NBSELECTS; i++)
	if (strlen (selects[i].command))
	  {
	    fprintf (configfile, "\t%s\n", selects[i].command);
	  }
	else
	  {
	    fprintf (configfile, "\tNone\n");
	  }
      for (i = 0; i < NBMENUS; i++)
	{
	  fprintf (configfile, "[Menu%u]\n", i + 1);
	  fprintf (configfile, "%u\n", menus[i]->nbitems);
	  for (j = 0; j < menus[i]->nbitems; j++)
	    {
	      fprintf (configfile, "\t%s\n",
		       menus[i]->menuitem[j]->label);
	      fprintf (configfile, "\t%s\n",
		       menus[i]->menuitem[j]->pixfile);
	      fprintf (configfile, "\t%s\n",
		       menus[i]->menuitem[j]->command);
	    }

	}
      fflush (configfile);
      fclose (configfile);
    }
  free (homedir);
}

void 
resetconfig (void)
{
  char *homedir;
  FILE *configfile;
  int i;

  homedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/");
  strcat (homedir, rcfile);
  configfile = fopen (homedir, "w");
  if (!configfile)
    my_alert (rxfce.notreset, 1);
  else
    {
      fprintf (stderr, "Creating new config file...");
      fprintf (configfile, "[Coords]\n");
      fprintf (configfile, "\t%u\n", (fl_scrw - fd_XFCE->XFCE->w) / 2);
      fprintf (configfile, "\t%u\n", fl_scrh - fd_XFCE->XFCE->h - 20);
      fprintf (configfile, "[ButtonLabels]\n");
      for (i = 0; i < NBSCREENS; i++)
	fprintf (configfile, "\t%s\n", screen_names[i]);
      fprintf (configfile, "[External_Icons]\n");
      for (i = 0; i < NBSELECTS; i++)
	fprintf (configfile, "\tNone\n");
      fprintf (configfile, "[Icons]\n");
      fprintf (configfile, "\t0,1,2,3,4,5,6\n");
      fprintf (configfile, "[WorkSpace]\n");
      fprintf (configfile, "\tRepaint\n");
      fprintf (configfile, "\tGradient\n");
      fprintf (configfile, "[Lock]\n");
      fprintf (configfile, "\tNone\n");
      fprintf (configfile, "[MenuOption]\n");
      fprintf (configfile, "\tDetach\n");
      fprintf (configfile, "[XFwmOption]\n");
      fprintf (configfile, "\tClickToFocus\n");
      fprintf (configfile, "\tOpaqueMove\n");
      fprintf (configfile, "\tNoAutoRaise\n");
      fprintf (configfile, "[Commands]\n");
      for (i = 0; i < NBSELECTS; i++)
	fprintf (configfile, "\tNone\n");
      for (i = 0; i < NBMENUS; i++)
	{
	  fprintf (configfile, "[Menu%u]\n", i + 1);
	  fprintf (configfile, "%u\n", 0);
	}
      fflush (configfile);
      fclose (configfile);
    }
  free (homedir);
}


void 
readconfig (void)
{
  char *homedir;
  char *lineread;
  char *p;
  FILE *configfile;
  char *dummy;
  char *label;
  char *pixfile;
  char *command;


  int i, j, ni, px, py;

  nl = 0;
  homedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  lineread = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  label = (char *) malloc (256 * sizeof (char));
  pixfile = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  command = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  dummy = (char *) malloc (16 * sizeof (char));
  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/");
  strcat (homedir, rcfile);
  if (existfile (homedir))
    {
      configfile = fopen (homedir, "r");
    }
  else
    {
      fprintf (stderr, "XFCE : %s File not found.\n", homedir);
      strcpy (homedir, XFCE_DIR);
      strcat (homedir, "/");
      strcat (homedir, rcfile);
      if (existfile (homedir))
	{
	  configfile = fopen (homedir, "r");
	}
      else
	{
	  fprintf (stderr, "XFCE : %s File not found.\n", homedir);
	  strcpy (homedir, XFCE_DIR);
	  strcat (homedir, "/sample");
	  strcat (homedir, rcfile);
	  configfile = fopen (homedir, "r");
	}
    }
  if (!configfile)
    {
      my_alert (rxfce.notopen, 1);
      fprintf (stderr, "XFCE : %s File not found.\n", homedir);
      resetconfig ();
      strcpy (homedir, (char *) getenv ("HOME"));
      strcat (homedir, "/");
      strcat (homedir, rcfile);
      configfile = fopen (homedir, "r");
    }
  if (!configfile)
    my_alert (rxfce.notopen, 1);
  else
    {
      p = nextline (configfile, lineread);
      if (my_strncasecmp (p, "[Coords]", strlen ("[Coords]")))
	syntax_error (p);
      p = nextline (configfile, lineread);
      px = atoi (p);
      p = nextline (configfile, lineread);
      py = atoi (p);
      if (px < 0) 
        px = (fl_scrw - fd_XFCE->XFCE->w) / 2;
      if (py < 0)
        py = (fl_scrh - fd_XFCE->XFCE->h - 20);
      fl_set_form_position (fd_XFCE->XFCE, (px > 0) ? px : 0, (py > 0) ? py : 0);
      p = nextline (configfile, lineread);
      if (my_strncasecmp (p, "[ButtonLabels]", strlen ("[ButtonLabels]")))
	syntax_error (p);
      for (i = 0; i < NBSCREENS; i++)
	{
	  p = nextline (configfile, lineread);
	  strcpy (fd_XFCE->screen[i]->label, p);
	}
      p = nextline (configfile, lineread);
      if (!my_strncasecmp (p, "[External_Icons]", strlen ("[External_Icons]")))
	{
          for (i = 0; i < NBSELECTS; i++) {
  	    p = nextline (configfile, lineread);
            set_exticon_str(i, p);
          }
          p = nextline (configfile, lineread);
        }
      else
        for (i = 0; i < NBSELECTS; i++)
          set_exticon_str(i, "None");
      if (!my_strncasecmp (p, "[Icons]", strlen ("[Icons]")))
	{
	  p = nextline (configfile, lineread);
	  load_icon_str (p);
	  setup_icon ();
	  p = nextline (configfile, lineread);
	}
      else
	default_icon_str ();
      if (!my_strncasecmp (p, "[WorkSpace]", strlen ("[WorkSpace]")))
	{
	  p = nextline (configfile, lineread);
	  COLORIZE_ROOT = (my_strncasecmp (p, "Repaint", strlen ("Repaint")) == 0);
	  p = nextline (configfile, lineread);
          if (my_strncasecmp (p, "[Lock]", strlen ("[Lock]"))) {
            GRADIENT_ROOT = (my_strncasecmp (p, "Gradient", strlen ("Gradient")) == 0);
            p = nextline (configfile, lineread);
          }
          else
            GRADIENT_ROOT = COLORIZE_ROOT;
	}
      else
	COLORIZE_ROOT = GRADIENT_ROOT = 1;
      if (!my_strncasecmp (p, "[Lock]", strlen ("[Lock]")))
	{
	  p = nextline (configfile, lineread);
	  set_command (NBSELECTS, p);
	  p = nextline (configfile, lineread);
	}
      else
	strcpy (selects[NBSELECTS].command, "None");
      if (!my_strncasecmp (p, "[MenuOption]", strlen ("[MenuOption]")))
	{
	  p = nextline (configfile, lineread);
	  DETACH_MENU = (my_strncasecmp (p, "Detach", strlen ("Detach")) == 0);
	  p = nextline (configfile, lineread);
	}
      else
	DETACH_MENU = 1;
      if (!my_strncasecmp (p, "[XFwmOption]", strlen ("[XFwmOption]")))
	{
	  p = nextline (configfile, lineread);
	  CLICKTOFOCUS = (my_strncasecmp (p, "ClickToFocus", strlen ("ClickToFocus")) == 0);
	  p = nextline (configfile, lineread);
          if (my_strncasecmp (p, "[Commands]", strlen ("[Commands]"))) {
  	    OPAQUEMOVE = (my_strncasecmp (p, "OpaqueMove", strlen ("OpaqueMove")) == 0);
	    p = nextline (configfile, lineread);
          }
          else
            OPAQUEMOVE = 1;
          if (my_strncasecmp (p, "[Commands]", strlen ("[Commands]"))) {
  	    AUTORAISE = (my_strncasecmp (p, "AutoRaise", strlen ("AutoRaise")) == 0);
	    p = nextline (configfile, lineread);
          }
          else
            AUTORAISE = 0;
	}
      else {
	CLICKTOFOCUS = 1;
        OPAQUEMOVE = 1;
        AUTORAISE = 0;
      }
      if (my_strncasecmp (p, "[Commands]", strlen ("[Commands]")))
	syntax_error (p);
      for (i = 0; i < NBSELECTS; i++)
	{
	  p = nextline (configfile, lineread);
	  set_command (i, p);
	}
      for (i = 0; i < NBMENUS; i++)
	{
	  p = nextline (configfile, lineread);
	  sprintf (dummy, "[Menu%u]", i + 1);
	  if (my_strncasecmp (p, dummy, strlen ("dummy")))
	    syntax_error (p);
	  p = nextline (configfile, lineread);
	  ni = atoi (p);
	  if (ni > NBMAXITEMS)
	    data_error ("Too many items in menu.");
	  for (j = 0; j < ni; j++)
	    {
	      p = nextline (configfile, lineread);
	      strcpy (label, p);
	      p = nextline (configfile, lineread);
	      strcpy (pixfile, p);
	      p = nextline (configfile, lineread);
	      strcpy (command, p);
	      add_item_menu (menus[i], command, pixfile, label);
	    }
	}
      fclose (configfile);
    }
  free (homedir);
  free (lineread);
  free (label);
  free (pixfile);
  free (command);
  free (dummy);
}
