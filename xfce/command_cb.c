
/*

   ORIGINAL FILE NAME : command_cb.c

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



#include "forms.h"
#include "fileutil.h"
#include "configfile.h"
#include "command.h"
#include "pulldown.h"
#include "extern.h"
#include "my_string.h"
#include "selects.h"
#include "XFCE.h"
#include "resources.h"

void 
show_defcommand (int data, char *title)
{
  fl_deactivate_all_forms ();
  fl_set_object_callback (fd_def_command->defcom_ok_but, OK_defcom_cb, data);
  fl_set_object_callback (fd_def_command->br_icon, br_icon_cb, data);
  fl_set_object_callback (fd_def_command->br_command, br_com_cb, data);
  fl_set_input (fd_def_command->command_input, selects[data].command);
  if (data == NBSELECTS)
    {
      inactive_choice (fd_def_command->icon_choice);
      fl_deactivate_object (fd_def_command->br_icon);
    }
  else
    {
      active_choice (fd_def_command->icon_choice);
      fl_activate_object (fd_def_command->br_icon);
      set_choice_value (fd_def_command->icon_choice, data);
    }
  fl_show_form (fd_def_command->defcom, FL_PLACE_CENTER, FL_TRANSIENT, title);
  fl_set_cursor (fd_def_command->defcom->window, XFCE_CURS);
}

void 
hide_defcommand (void)
{
  fl_hide_form (fd_def_command->defcom);
  fl_activate_all_forms ();
}

void 
set_command (int no_sel, char *s)
{
  if (!strlen (s) || !my_strncasecmp (s, "None", strlen ("None")))
    {
      strcpy (selects[no_sel].command, "None");
    }
  else
    {
      strcpy (selects[no_sel].command, s);
    }
}

void 
DM_com_cb (FL_OBJECT * ob, long data)
{
  ;
}

void 
br_com_cb (FL_OBJECT * ob, long data)
{
  char *fselect;
  fselect = skiphead ((char *) fl_show_fselector (rxfce.selexec,
						  XBINDIR, "*", ""));
  if (fselect)
    {
      if (strlen (fselect))
	cleanup (fselect);
      fl_set_input (fd_def_command->command_input, fselect);
    }
}

void 
br_icon_cb (FL_OBJECT * ob, long data)
{
  const char *fselect;
  fselect = fl_show_fselector (rxfce.selicon, XFCE_ICONS, "*.xpm", "");
  if ((fselect) && (strlen (fselect)))
      {
          strcpy (selects[data].ext_icon, fselect);
          set_choice_value (fd_def_command->icon_choice, 0);
          fl_set_choice (fd_def_command->icon_choice, NB_PANEL_ICONS + 1);
      }
  else
      {
          strcpy (selects[data].ext_icon, "None");
      }
}

void 
OK_defcom_cb (FL_OBJECT * ob, long data)
{
  char *s;

  s = cleanup ((char *) fl_get_input (fd_def_command->command_input));
  set_command (data, s);
  if (data < NBSELECTS)
    set_icon_nbr (data, fl_get_choice (fd_def_command->icon_choice) - 1);
  hide_defcommand ();
#ifdef ALLWAYS_SAVE
  writetoconfig ();
#endif
  fl_redraw_form (fd_XFCE->XFCE);
}

void 
CANCEL_defcom_cb (FL_OBJECT * ob, long data)
{
  hide_defcommand ();
}

int 
command_atclose (FL_FORM * f, long *d)
{
  hide_defcommand ();
  fl_redraw_form (fd_XFCE->XFCE);
  return (0);
}
