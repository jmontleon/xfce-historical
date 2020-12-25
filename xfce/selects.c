
/*

   ORIGINAL FILE NAME : selects.c

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
#include "selects.h"
#include "fileutil.h"
#include "extern.h"
#include "XFCE.h"

/*
   Load pixmap files 
 */
#include "file1.h"
#include "file2.h"
#include "mail.h"
#include "write.h"
#include "term.h"
#include "man.h"
#include "paint.h"
#include "print.h"
#include "multimedia.h"
#include "games.h"
#include "schedule.h"
#include "network.h"
#include "defaulticon.h"

static char *icon_name[] =
{
  "Files related",
  "Terminals/Connections",
  "Mail tools/browsers",
  "Print manager",
  "Configuration",
  "Misc. tools/Utilities",
  "Manual/Help browser",
  "Writing/Text tools",
  "Schedule/Appointments",
  "Multimedia",
  "Games",
  "Network",
  "External..."};

static char **icon_data[] =
{
  file1,
  term,
  mail,
  print,
  paint,
  file2,
  man,
  write,
  schedule,
  multimedia,
  games,
  network,
  defaulticon};

void 
alloc_selects (void)
{
  int i;

  for (i = 0; i < NBSELECTS + 1; i++) {
    selects[i].command  = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
    selects[i].ext_icon = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  }
}

void 
free_selects (void)
{
  int i;

  for (i = 0; i < NBSELECTS + 1; i++) {
    free (selects[i].command);
    free (selects[i].ext_icon);
  }

}

int 
load_icon_str (char *str)
{
  char *a;
  int i, error = 0;

  if ((a = strtok (str, ",")))
    {
      selects[0].icon_nbr = atoi (a);
      for (i = 1; i < NBSELECTS; i++)
	if ((a = strtok (NULL, ",")))
	  selects[i].icon_nbr = (((atoi(a) < NB_PANEL_ICONS) 
                                  || (atoi(a) == 99)) ? atoi(a) : i);
	else
	  {
	    error = i + 1;
	    break;
	  }
    }
  else
    error = 1;
  if (error)
    for (i = 0; i < NBSELECTS; i++)
      selects[i].icon_nbr = i;
  return (error);
}

char *
save_icon_str (void)
{
  static char str[NBSELECTS * 3 + 1];
  char *temp;
  int i;

  temp = (char *) malloc (4);
  sprintf (str, "%i", selects[0].icon_nbr);
  for (i = 1; i < NBSELECTS; i++)
    {
      sprintf (temp, ",%i", selects[i].icon_nbr);
      strcat (str, temp);
    }
  free (temp);
  return (str);
}

void
set_exticon_str(int i, char *s)
{
  if (i < NBSELECTS) {
    if ((s) && (strlen(s)))
       strcpy(selects[i].ext_icon, s);
    else
       strcpy(selects[i].ext_icon, "None");
  }
}

void 
setup_icon (void)
{
  int i;

  for (i = 0; i < NBSELECTS; i++)
    if ((selects[i].icon_nbr >= 0) && (selects[i].icon_nbr < NB_PANEL_ICONS))
      fl_set_pixmap_data (fd_XFCE->selects[i],
			  icon_data[selects[i].icon_nbr]);
    else {
      if ((selects[i].icon_nbr == 99) 
           && (selects[i].ext_icon) 
           && (existfile (selects[i].ext_icon)))
         fl_set_pixmap_file (fd_XFCE->selects[i], selects[i].ext_icon);
      else
        fl_set_pixmap_data (fd_XFCE->selects[i], defaulticon);
    }
}

void 
default_icon_str (void)
{
  char *s;

  s = (char *) malloc (NBSELECTS * 3 + 1);
  strcpy (s, "0,1,2,3,4,5,6");
  load_icon_str (s);
  setup_icon ();
  free (s);
}

int 
get_icon_nbr (int no_cmd)
{
  int i;

  i = selects[no_cmd].icon_nbr;
  return ((i < NB_PANEL_ICONS) ? i : 99);
}

void 
set_icon_nbr (int no_cmd, int icon_nbr)
{
  if (icon_nbr < NB_PANEL_ICONS) {
    selects[no_cmd].icon_nbr = icon_nbr;
    fl_set_pixmap_data (fd_XFCE->selects[no_cmd], icon_data[icon_nbr]);
  }
  else {
    selects[no_cmd].icon_nbr = 99;
    if ((selects[no_cmd].ext_icon) && (existfile (selects[no_cmd].ext_icon)))
         fl_set_pixmap_file (fd_XFCE->selects[no_cmd],selects[no_cmd].ext_icon);
      else
        fl_set_pixmap_data (fd_XFCE->selects[no_cmd], defaulticon);
  }
}

void 
init_choice_str (FL_OBJECT * obj)
{
  int i;

  for (i = 0; i < (NB_PANEL_ICONS + 1); i++)
    fl_addto_choice (obj, icon_name[i]);
}

void 
set_choice_value (FL_OBJECT * obj, int no_cmd)
{
  if (selects[no_cmd].icon_nbr < NB_PANEL_ICONS)
    fl_set_choice (obj, selects[no_cmd].icon_nbr + 1);
  else
    fl_set_choice (obj, NB_PANEL_ICONS + 1);

}
