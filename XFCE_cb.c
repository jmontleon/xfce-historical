
/*

   ORIGINAL FILE NAME : XFCE_cb.c

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
#include "resources.h"
#include "my_string.h"
#include "my_goodies.h"
#include "pulldown.h"
#include "addicon.h"
#include "command.h"
#include "screen.h"
#include "info.h"
#include "defcolor.h"
#include "configfile.h"
#include "fileutil.h"
#include "selects.h"
#include "sendinfo.h"
#include "constant.h"
#include "extern.h"
#include "fvwm_cmd.h"
#include "tiptools.h"

void 
move_cb (FL_OBJECT * ob, long data)
{
  Window win;
  int x, y, w, h;

  if (FVWM)
    {
      win = compute_wm_window (fd_XFCE->XFCE->window, &x, &y, &w, &h);
      fl_set_mouse ((FL_Coord) x, (FL_Coord) y);
      sendinfo (fd, MOVE_CMD, win);
    }
}

void 
iconify_cb (FL_OBJECT * ob, long data)
{
  Window win;
  int x, y, w, h;

  if (FVWM)
    {
      win = compute_wm_window (fd_XFCE->XFCE->window, &x, &y, &w, &h);
      sendinfo (fd, ICON_CMD, win);
    }
}

void 
update_screen (int no_screen)
{
  static int working_screen = 0;
  int i;

  FL_COLOR backcolor;
  if (FVWM)
    {
      /*
         If parameter is negative, just redisplay root window with
         proper color 
       */
      if (no_screen >= 0)
	working_screen = no_screen;
      for (i = 0; i < NBSCREENS; i++)
	fl_set_button (fd_XFCE->screen[i],
		       (working_screen == i));
      if (COLORIZE_ROOT)
	{
	  backcolor = screen_colors[working_screen % 4];
	  XSetWindowBackground (fl_display, fl_root, fl_get_pixel (backcolor));
	  XClearWindow (fl_display, fl_root);
	  XFlush (fl_display);

	}
    }
}

void 
switch_to_screen (int no_screen)
{
  char command[256];
  if (FVWM)
    {
      sprintf (command, "%s 0 %d\n", DESK_CMD, no_screen);
      sendinfo (fd, command, 0);
    }
}

void 
popup_cb (FL_OBJECT * ob, long data)
{
  FL_Coord x, y, w, h;

  /*
     change callback function so it closes popup when button is pressed 
   */
  fl_set_object_callback (ob, close_popup, data);
  /*
     Then show menu and wait for events 
   */
  /*
     Display down arrow in button 
   */
  ToggleButtonDn (ob);
  /*
     Set default as attached 
   */
  menus[data]->isdetached = 0;
  /*
     Then displays items of menu 
   */
  fl_get_object_geometry (ob, &x, &y, &w, &h);
  popup_pulldown (menus[data], fd_XFCE->XFCE->x + x + w / 2, fd_XFCE->XFCE->y + y, data);
}

void 
select_cb (FL_OBJECT * ob, long data)
{
  if (((((FL_BUTTON_STRUCT *) ob->spec)->mousebut) == BUTTONCHANGE)
      || !(my_strncasecmp (selects[data].command, "None", strlen ("None")))
      || !(my_strncasecmp (selects[data].command, "Nothing", strlen ("Nothing"))))
    show_defcommand (data, rxfce.defcomtitle);
  else
    exec_comm (selects[data].command);
}

void 
screen_cb (FL_OBJECT * ob, long data)
{
  if (FVWM)
    switch_to_screen (data);
  else
    my_alert (rxfce.FVWMonly, 0);
  if ((((FL_BUTTON_STRUCT *) ob->spec)->mousebut) == BUTTONCHANGE)
    show_screen (data, rxfce.screentitle);
}

void 
info_cb (FL_OBJECT * ob, long data)
{
  show_info (rxfce.infotitle);
}

void 
quit_cb (FL_OBJECT * ob, long data)
{
  int rep;

  /*
     Always write configuration file, even if user cancels the action 
   */
  writetoconfig ();
  rep = (ob == NULL);
  if (!rep)
    rep = (my_question (rxfce.quit, 0) == 1);
  if (rep)
    {
      if (FVWM)
	sendinfo (fd, QUIT_CMD, 0);
      end_XFCE (0);
    }
}

void 
setup_cb (FL_OBJECT * ob, long data)
{
  show_defcolor (rxfce.setuptitle);
}

void 
close_popup (FL_OBJECT * ob, long data)
{
  ToggleButtonUp (fd_XFCE->popup[data]);
  fl_set_object_callback (fd_XFCE->popup[data], popup_cb, data);
  fl_hide_form (menus[data]->pullmenu);
}

/*
   callbacks for form pullmenu 
 */

void 
detach_cb (FL_OBJECT * ob, long data)
{
  fl_hide_form (menus[data]->pullmenu);
  menus[data]->isdetached = 1;
  popup_pulldown (menus[data], -1, -1, data);
}

void 
pullmenu_cb (FL_OBJECT * ob, long data)
{
  int menucol, menulig;
  int x, y;

  menucol = data / NBMAXITEMS;
  menulig = data % NBMAXITEMS;
  fl_get_winorigin (menus[menucol]->pullmenu->window, &x, &y);
  if ((((FL_BUTTON_STRUCT *) ob->spec)->mousebut) == BUTTONCHANGE)
    {
      set_addiconform (fd_addiconform, menucol, menulig);
      set_addicon_cb (fd_addiconform, menucol, menulig);
      close_popup (ob, menucol);
      show_addiconform (rxfce.modifytitle);
    }
  else
    {
      if (!menus[menucol]->isdetached)
	close_popup (ob, menucol);
      exec_comm (menus[menucol]->menuitem[menulig]->command);
    }
}

void 
addicon_cb (FL_OBJECT * ob, long data)
{
  int menucol;

  menucol = data % NBMAXITEMS;
  close_popup (ob, menucol);
  if (menus[menucol]->nbitems < NBMAXITEMS)
    {
      set_addiconform (fd_addiconform, menucol, -1);
      /*
         That's a convention : If the parameter is negative, we add an icon 
       */
      set_addicon_cb (fd_addiconform, -menucol, -1);
      show_addiconform (rxfce.additemtitle);
    }
  else
    my_alert (rxfce.menufull, 1);
}

void 
end_XFCE (int n)
{
  free_selects ();
  free_item_menus ();
  freepal (palette);
  free (fd_addiconform);
  free (fd_info);
  free (fd_def_command);
  free (fd_def_screen);
  free (fd_def_color);
  free (fd_XFCE);
  free_resources (&rxfce);
  fl_finish ();
  exit (n);
}

int 
XFCE_atclose (FL_FORM * f, long *d)
{
  quit_cb (NULL, 0);
  return (0);
}
