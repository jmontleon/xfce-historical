
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
#include "resources.h"

void 
show_defcommand (int data, char *title)
{
  fl_deactivate_all_forms ();
  fl_set_object_callback (fd_def_command->defcom_ok_but, OK_defcom_cb, data);
  fl_set_input (fd_def_command->command_input, selects[data].command);
  if (data == NBSELECTS)
    {
      inactive_choice (fd_def_command->icon_choice);
    }
  else
    {
      active_choice (fd_def_command->icon_choice);
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
      set_command (data, fselect);
      fl_set_input (fd_def_command->command_input, selects[data].command);
      fl_redraw_form (fd_def_command->defcom);
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
  return (0);
}
