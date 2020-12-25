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

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <stdlib.h>
#include <X11/Xlib.h>
#include "constant.h"
#include "configfile.h"
#include "my_string.h"
#include "xfce-common.h"
#include "xfce_main.h"
#include "selects.h"
#include "popup.h"
#include "xfce.h"
#include "fileutil.h"
#include "my_intl.h"

char *rcfile = ".xfce3rc";
int nl = 0;
int buffersize = 127;

void 
syntax_error (char *s)
{
  fprintf (stderr, _("XFce : Syntax error in configuration file\n(%s)\n"), s);
  my_alert (_("Syntax error in configuration file\nAborting"));
  end_XFCE (2);
}

void 
data_error (char *s)
{
  fprintf (stderr, _("XFce : Data mismatch error in config file\n(%s)\n"), s);
  my_alert (_("Data mismatch error in configuration file\nAborting"));
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
  return ((!feof(f)) ? p : NULL);
}

config * 
initconfig(config *newconf)
{
   if (!newconf)
     newconf=(config *) malloc (sizeof (config));
   newconf->panel_x = -1;
   newconf->panel_y = -1;
   newconf->wm = 0;
   newconf->visible_screen = 4;
   newconf->visible_popup = 6;
   newconf->select_icon_size = 1; /* Medium size */
   newconf->popup_icon_size = 1; /* Medium size */
   newconf->colorize_root = 0;
   newconf->gradient_root = 0;
   newconf->detach_menu = 1;
   newconf->gradient_active_title = 1;
   newconf->gradient_inactive_title = 1;
   newconf->clicktofocus = 1;
   newconf->opaquemove = 1;
   newconf->autoraise = 0;
   newconf->iconpos = 0; /* Top of screen */
   newconf->fonts[0] = (char *) malloc (sizeof(char) * MAXSTRLEN);
   newconf->fonts[1] = (char *) malloc (sizeof(char) * MAXSTRLEN);
   newconf->fonts[2] = (char *) malloc (sizeof(char) * MAXSTRLEN);
   strcpy(newconf->fonts[0], XFWM_TITLEFONT);
   strcpy(newconf->fonts[1], XFWM_MENUFONT);
   strcpy(newconf->fonts[2], XFWM_ICONFONT);
   
   return newconf;
}

void
backupconfig (char *extension)
{
  char homedir  [MAXSTRLEN + 1];
  char buffer   [MAXSTRLEN + 1];
  char backname [MAXSTRLEN + 1];
  FILE *copyfile;
  FILE *backfile;
  int nb_read;

  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/");
  strcat (homedir, rcfile);
  /*
     Backup any existing config file before creating a new one 
   */
  if (existfile (homedir))
    {
      strcpy (backname, homedir);
      strcat (backname, extension);
      backfile = fopen (backname, "w");
      copyfile = fopen (homedir, "r");
      if ((backfile) && (copyfile))
	{
	  while ((nb_read = fread (buffer, 1, MAXSTRLEN, copyfile)) > 0)
	    {
	      fwrite (buffer, 1, nb_read, backfile);
	    }
	  fflush (backfile);
	  fclose (backfile);
	  fclose (copyfile);
	}
    }
}

void 
writeconfig (void)
{
  char homedir  [MAXSTRLEN + 1];
  FILE *configfile = NULL;
  int i, j;
  int x, y;

  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/");
  strcat (homedir, rcfile);
  /*
     Backup any existing config file before creating a new one 
   */
  if (existfile (homedir))
      backupconfig (BACKUPEXT);

  configfile = fopen (homedir, "w");

  if (!configfile)
    my_alert (_("Cannot create file"));
  else
    {
      fprintf (configfile, "%s\n", XFCE3SIG);
      fprintf (configfile, "[Coords]\n");
      gdk_window_get_root_origin (gxfce->window, &x, &y);
      fprintf (configfile, "\t%i\n", x);
      fprintf (configfile, "\t%i\n", y);
      fprintf (configfile, "[ButtonLabels]\n");
      for (i = 0; i < NBSCREENS; i++)
	fprintf (configfile, "\t%s\n", get_gxfce_screen_label(i));
      fprintf (configfile, "[External_Icons]\n");
      for (i = 0; i < NBSELECTS; i++)
        if(get_exticon_str(i) && (strlen(get_exticon_str(i))))
  	  fprintf (configfile, "\t%s\n", get_exticon_str(i));
        else
	  fprintf (configfile, "\tNone\n");
      fprintf (configfile, "[Popups]\n");
      fprintf (configfile, "\t%i\n", current_config.visible_popup);
      fprintf (configfile, "[Icons]\n");
      fprintf (configfile, "\t%s\n", save_icon_str ());
      fprintf (configfile, "[WorkSpace]\n");
      fprintf (configfile, current_config.colorize_root ? "\tRepaint\n" : "\tNoRepaint\n");
      fprintf (configfile, current_config.gradient_root ? "\tGradient\n" : "\tSolid\n");
      fprintf (configfile, "[Lock]\n");
      fprintf (configfile, "\t%s\n", get_command(NBSELECTS));
      fprintf (configfile, "[MenuOption]\n");
      fprintf (configfile, current_config.detach_menu ? "\tDetach\n" : "\tNoDetach\n");
      fprintf (configfile, "[XFwmOption]\n");
      fprintf (configfile, current_config.clicktofocus ? "\tClickToFocus\n" : "\tFollowMouse\n");
      fprintf (configfile, current_config.opaquemove ? "\tOpaqueMove\n" : "\tNoOpaqueMove\n");
      fprintf (configfile, current_config.autoraise ? "\tAutoraise\n" : "\tNoAutoraise\n");
      fprintf (configfile, current_config.gradient_active_title ? "\tGradientActive\n" : "\tOpaqueActive\n");
      fprintf (configfile, current_config.gradient_inactive_title ? "\tGradientInactive\n" : "\tOpaqueInactive\n");
      switch (current_config.iconpos)
        {
	  case 1:
            fprintf (configfile, "\tIconsOnLeft\n");
	    break;
	  case 2:
            fprintf (configfile, "\tIconsOnBottom\n");
	    break;
	  case 3:
            fprintf (configfile, "\tIconsOnRight\n");
	    break;
	  default:
            fprintf (configfile, "\tIconsOnTop\n");
	}
      fprintf (configfile, "\t%s\n", current_config.fonts[0]);
      fprintf (configfile, "\t%s\n", current_config.fonts[1]);
      fprintf (configfile, "\t%s\n", current_config.fonts[2]);
      fprintf (configfile, "[Screens]\n");
      fprintf (configfile, "\t%i\n", current_config.visible_screen);
      fprintf (configfile, "[Sizes]\n");
      switch (current_config.select_icon_size)
        {
	  case 0:
            fprintf (configfile, "\tSmallPanelIcons\n");
	    break;
	  case 2:
            fprintf (configfile, "\tLargePanelIcons\n");
	    break;
	  default:
            fprintf (configfile, "\tMediumPanelIcons\n");
        }
      switch (current_config.popup_icon_size)
        {
	  case 0:
            fprintf (configfile, "\tSmallMenuIcons\n");
	    break;
	  case 2:
            fprintf (configfile, "\tLargeMenuIcons\n");
	    break;
	  default:
            fprintf (configfile, "\tMediumMenuIcons\n");
        }
      fprintf (configfile, "[Commands]\n");
      for (i = 0; i < NBSELECTS; i++)
	if (strlen (selects[i].command))
	    fprintf (configfile, "\t%s\n", get_command(i));
	else
	    fprintf (configfile, "\tNone\n");
      for (i = 0; i < NBPOPUPS; i++)
	{
	  fprintf (configfile, "[Menu%u]\n", i + 1);
	  for (j = 0; j < get_popup_menu_entries(i); j++)
	    {
	      fprintf (configfile, "\t%s\n",
		       get_popup_entry_label(i, j));
	      fprintf (configfile, "\t%s\n",
		       get_popup_entry_icon(i, j));
	      fprintf (configfile, "\t%s\n",
		       get_popup_entry_command(i, j));
	    }

	}
      fflush (configfile);
      fclose (configfile);
    }
}

void 
resetconfig (void)
{
  char homedir  [MAXSTRLEN + 1];
  FILE *configfile;
  int i;

  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/");
  strcat (homedir, rcfile);
  configfile = fopen (homedir, "w");
  if (!configfile)
    my_alert (_("Cannot reset configuration file"));
  else
    {
      fprintf (stderr, _("Creating new config file...\n"));
      fprintf (configfile, "%s\n", XFCE3SIG);
      fprintf (configfile, "[Coords]\n");
      fprintf (configfile, "\t%i\n", -1);
      fprintf (configfile, "\t%i\n", -1);
      fprintf (configfile, "[ButtonLabels]\n");
      for (i = 0; i < NBSCREENS; i++)
	fprintf (configfile, "\t%s\n", screen_names[i]);
      fprintf (configfile, "[External_Icons]\n");
      for (i = 0; i < NBSELECTS; i++)
	fprintf (configfile, "\tNone\n");
      fprintf (configfile, "[Popups]\n");
      fprintf (configfile, "\t6\n");
      fprintf (configfile, "[Icons]\n");
      fprintf (configfile, "\t%s\n", DEFAULT_ICON_SEQ);
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
      fprintf (configfile, "\tGradientActive\n");
      fprintf (configfile, "\tGradientInactive\n");
      fprintf (configfile, "\tIconsOnTop\n");
      fprintf (configfile, "\t%s\n", XFWM_TITLEFONT);
      fprintf (configfile, "\t%s\n", XFWM_MENUFONT);
      fprintf (configfile, "\t%s\n", XFWM_ICONFONT);
      fprintf (configfile, "[Screens]\n");
      fprintf (configfile, "\t4\n");
      fprintf (configfile, "[Sizes]\n");
      fprintf (configfile, "\tMediumPanelIcons\n");
      fprintf (configfile, "\tSmallMenuIcons\n");
      fprintf (configfile, "[Commands]\n");
      for (i = 0; i < NBSELECTS; i++)
	fprintf (configfile, "\tNone\n");
      for (i = 0; i < NBPOPUPS; i++)
	{
	  fprintf (configfile, "[Menu%u]\n", i + 1);
	}
      fflush (configfile);
      fclose (configfile);
    }
}


void 
readconfig (void)
{
  char homedir  [MAXSTRLEN + 1];
  char lineread [MAXSTRLEN + 1];
  char pixfile  [MAXSTRLEN + 1];
  char command  [MAXSTRLEN + 1];
  char label    [256];
  char dummy    [16];
  char *p;
  FILE *configfile;


  int i, j;

  nl = 0;
  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, "/");
  strcat (homedir, rcfile);
  if (existfile (homedir))
    {
      configfile = fopen (homedir, "r");
    }
  else
    {
      fprintf (stderr, _("XFce : %s File not found.\n"), homedir);
      strcpy (homedir, XFCE_DIR);
      strcat (homedir, "/");
      strcat (homedir, rcfile);
      if (existfile (homedir))
	{
	  configfile = fopen (homedir, "r");
	}
      else
	{
	  fprintf (stderr, _("XFce : %s File not found.\n"), homedir);
	  strcpy (homedir, XFCE_DIR);
	  strcat (homedir, "/sample");
	  strcat (homedir, rcfile);
	  configfile = fopen (homedir, "r");
	}
    }
  if (!configfile)
    {
      my_alert (_("Cannot open configuration file"));
      fprintf (stderr, _("XFce : %s File not found.\n"), homedir);
      resetconfig ();
      strcpy (homedir, (char *) getenv ("HOME"));
      strcat (homedir, "/");
      strcat (homedir, rcfile);
      configfile = fopen (homedir, "r");
    }
  if (!configfile)
    my_alert (_("Cannot open configuration file"));
  else
    {
      p = nextline (configfile, lineread);
      if (my_strncasecmp (p, XFCE3SIG, strlen (XFCE3SIG))) {
        my_alert (_("Does not looks like an XFce 3 configuration file !"));
        if (my_yesno_dialog(_("Do you want to reset the configuration file ?\n \
	(The previous file will be saved with \".orig\" extension)"))) {
          backupconfig (".orig");
	  fclose (configfile);
	  resetconfig ();
	  strcpy (homedir, (char *) getenv ("HOME"));
	  strcat (homedir, "/");
	  strcat (homedir, rcfile);
	  configfile = fopen (homedir, "r");
          if (!configfile) {
            my_alert (_("Cannot open new config, Giving up..."));
  	    data_error(_("Cannot load configuration file"));
	  }
	  /* Skipping first line */
          p = nextline (configfile, lineread);
	}
	else
	  syntax_error(_("Cannot use old version of XFce configuration files"));
      }
      p = nextline (configfile, lineread);
      if (my_strncasecmp (p, "[Coords]", strlen ("[Coords]")))
	syntax_error (p);
      p = nextline (configfile, lineread);
      current_config.panel_x = atoi (p);
      p = nextline (configfile, lineread);
      current_config.panel_y = atoi (p);
      p = nextline (configfile, lineread);
      if (my_strncasecmp (p, "[ButtonLabels]", strlen ("[ButtonLabels]")))
	syntax_error (p);
      i = 0;
      p = nextline (configfile, lineread);
      while ((i < NBSCREENS) && (my_strncasecmp (p, "[External_Icons]", strlen ("[External_Icons]"))))
	{
	  set_gxfce_screen_label(i, p);
          p = nextline (configfile, lineread);
	  i++;
	}
      if (!my_strncasecmp (p, "[External_Icons]", strlen ("[External_Icons]")))
	{
          for (i = 0; i < NBSELECTS  + 1; i++) {
    	    p = nextline (configfile, lineread);
	    if ((my_strncasecmp (p, "[Icons]", strlen ("[Icons]")))
	        && (my_strncasecmp (p, "[Popups]", strlen ("[Popups]")))) {
              set_exticon_str(i, p);
            }
	    else {
	      break;
	    }
          }
        }
      if (!my_strncasecmp (p, "[Popups]", strlen ("[Popups]")))
	{
	  p = nextline (configfile, lineread);
          current_config.visible_popup = atoi (p);
	  if ((current_config.visible_popup) > NBPOPUPS)
	    current_config.visible_popup = NBPOPUPS;
	  p = nextline (configfile, lineread);
	}
      if (!my_strncasecmp (p, "[Icons]", strlen ("[Icons]")))
	{
	  p = nextline (configfile, lineread);
	  load_icon_str (p);
	  setup_icon ();
	  p = nextline (configfile, lineread);
	}
      else {
	default_icon_str ();
        setup_icon ();
      }
      if (!my_strncasecmp (p, "[WorkSpace]", strlen ("[WorkSpace]")))
	{
	  p = nextline (configfile, lineread);
	  current_config.colorize_root = (my_strncasecmp (p, "Repaint", strlen ("Repaint")) == 0);
	  p = nextline (configfile, lineread);
          current_config.gradient_root = (my_strncasecmp (p, "Gradient", strlen ("Gradient")) == 0);
          p = nextline (configfile, lineread);
	}
      if (!my_strncasecmp (p, "[Lock]", strlen ("[Lock]")))
	{
	  p = nextline (configfile, lineread);
	  set_command (NBSELECTS, p);
	  p = nextline (configfile, lineread);
	}
      if (!my_strncasecmp (p, "[MenuOption]", strlen ("[MenuOption]")))
	{
	  p = nextline (configfile, lineread);
	  current_config.detach_menu = (my_strncasecmp (p, "Detach", strlen ("Detach")) == 0);
	  p = nextline (configfile, lineread);
	}
      if (!my_strncasecmp (p, "[XFwmOption]", strlen ("[XFwmOption]")))
	{
	  p = nextline (configfile, lineread);
	  current_config.clicktofocus = (my_strncasecmp (p, "ClickToFocus", strlen ("ClickToFocus")) == 0);
	  p = nextline (configfile, lineread);
  	  current_config.opaquemove = (my_strncasecmp (p, "OpaqueMove", strlen ("OpaqueMove")) == 0);
	  p = nextline (configfile, lineread);
  	  current_config.autoraise = (my_strncasecmp (p, "AutoRaise", strlen ("AutoRaise")) == 0);
	  p = nextline (configfile, lineread);
  	  current_config.gradient_active_title = (my_strncasecmp (p, "GradientActive", strlen ("GradientActive")) == 0);
	  p = nextline (configfile, lineread);
  	  current_config.gradient_inactive_title = (my_strncasecmp (p, "GradientInactive", strlen ("GradientInactive")) == 0);
	  p = nextline (configfile, lineread);
          if (!my_strncasecmp (p, "IconsOnLeft", strlen ("IconsOnLeft")))
	    current_config.iconpos = 1;
	  else if (!my_strncasecmp (p, "IconsOnBottom", strlen ("IconsOnBottom")))
	    current_config.iconpos = 2;
	  else if (!my_strncasecmp (p, "IconsOnRight", strlen ("IconsOnRight")))
	    current_config.iconpos = 3;
	  else
	    current_config.iconpos = 0;
	  p = nextline (configfile, lineread);
	  strcpy(current_config.fonts[0], p);
	  p = nextline (configfile, lineread);
	  strcpy(current_config.fonts[1], p);
	  p = nextline (configfile, lineread);
	  strcpy(current_config.fonts[2], p);
	  p = nextline (configfile, lineread);
	}
      if (!my_strncasecmp (p, "[Screens]", strlen ("[Screens]")))
	{
  	  p = nextline (configfile, lineread);
	  current_config.visible_screen = atoi(p);
 	  p = nextline (configfile, lineread);
	}
      if (!my_strncasecmp (p, "[Sizes]", strlen ("[Sizes]")))
	{
  	  p = nextline (configfile, lineread);
          if (!my_strncasecmp (p, "SmallPanelIcons", strlen ("SmallPanelIcons")))
	    current_config.select_icon_size = 0;
	  else if (!my_strncasecmp (p, "LargePanelIcons", strlen ("LargePanelIcons")))
	    current_config.select_icon_size = 2;
	  else
	    current_config.select_icon_size = 1;
  	  p = nextline (configfile, lineread);
          if (!my_strncasecmp (p, "SmallMenuIcons", strlen ("SmallMenuIcons")))
	    current_config.popup_icon_size = 0;
	  else if (!my_strncasecmp (p, "LargeMenuIcons", strlen ("LargeMenuIcons")))
	    current_config.popup_icon_size = 2;
	  else
	    current_config.popup_icon_size = 1;
  	  p = nextline (configfile, lineread);
	 }
      if (!my_strncasecmp (p, "[Commands]", strlen ("[Commands]")))
        {
          p = nextline (configfile, lineread);
          for (i = 0; i < NBSELECTS; i++)
    	    {
  	      set_command (i, p);
              p = nextline (configfile, lineread);
	    }
	}
      i = 0;
      sprintf (dummy, "[Menu%u]", i + 1);
      while ((p) && (!my_strncasecmp (p, dummy, strlen (dummy))) && (i++ < NBPOPUPS))
	{
	  sprintf (dummy, "[Menu%u]", i + 1);
	  p = nextline (configfile, lineread);
	  j = 0;
	  while ((p) && (my_strncasecmp (p, dummy, strlen (dummy))))
	    {
	      strcpy (label, (p ? p :"None"));
	      p = nextline (configfile, lineread);
	      strcpy (pixfile, (p ? p :"Default"));
	      p = nextline (configfile, lineread);
	      strcpy (command, (p ? p :"None"));
	      if (j++ < NBMAXITEMS)
	        add_popup_entry (i - 1, label, pixfile, command);
	      p = nextline (configfile, lineread);
	    }
	}
      fclose (configfile);
    }
}

void
update_config_screen_visible(int i)
{
  if (i <= NBSCREENS)
    current_config.visible_screen = i;
  else
    current_config.visible_screen = NBSCREENS;
}
