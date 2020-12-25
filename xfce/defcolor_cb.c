

/*

   ORIGINAL FILE NAME : defcolor_cb.c

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
#include "defcolor.h"
#include "color.h"
#include "extern.h"
#include "fileutil.h"
#include "configfile.h"
#include "pulldown.h"
#include "info.h"
#include "XFCE.h"
#include "sendinfo.h"
#include "extern.h"
#include "xfwm_cmd.h"
#include "my_goodies.h"
#include "resources.h"

static short int orig_colorize_root;
static short int orig_gradient_root;
static short int orig_detach_menu;
static short int orig_clicktofocus;
static short int orig_opaquemove;
static short int orig_autoraise;

void
update_options_gradient (void)
{
  fl_redraw_object (fd_def_color->fake_back_gradient);
  if (COLORIZE_ROOT) {
    fl_set_object_lstyle (fd_def_color->gradient_workspace, XFCE_ACTIVELABEL);
    fl_set_object_lcol (fd_def_color->gradient_workspace, XFCE_COL9);
    fl_activate_object (fd_def_color->gradient_workspace);
  }
  else {
    GRADIENT_ROOT = 0;
    fl_set_object_lstyle (fd_def_color->gradient_workspace, XFCE_INACTIVELABEL);
    fl_set_object_lcol (fd_def_color->gradient_workspace, XFCE_COL8);
    fl_deactivate_object (fd_def_color->gradient_workspace);
  }
  fl_redraw_object (fd_def_color->gradient_workspace);
}

void
update_options_autoraise (void)
{
  fl_redraw_object (fd_def_color->fake_back_autoraise);
  if (CLICKTOFOCUS) {
    AUTORAISE = 0;
    fl_set_object_lstyle (fd_def_color->autoraise, XFCE_INACTIVELABEL);
    fl_set_object_lcol (fd_def_color->autoraise, XFCE_COL8);
    fl_deactivate_object (fd_def_color->autoraise);
  }
  else {
    fl_set_object_lstyle (fd_def_color->autoraise, XFCE_ACTIVELABEL);
    fl_set_object_lcol (fd_def_color->autoraise, XFCE_COL9);
    fl_activate_object (fd_def_color->autoraise);
  }
  fl_redraw_object (fd_def_color->autoraise);
}

void 
show_defcolor (char *title)
{
  fl_deactivate_all_forms ();
  fl_set_button (fd_def_color->color_button[0], 1);
  select_color (fd_def_color->color_button[0], 0);
  fl_set_button (fd_def_color->color_workspace, (COLORIZE_ROOT == 1));
  fl_set_button (fd_def_color->gradient_workspace, (GRADIENT_ROOT == 1) && (COLORIZE_ROOT == 1));
  fl_set_button (fd_def_color->autoraise, (CLICKTOFOCUS == 0) && (AUTORAISE == 1));
  update_options_gradient ();
  update_options_autoraise ();
  fl_set_button (fd_def_color->detach_menu, (DETACH_MENU == 1));
  fl_set_button (fd_def_color->clicktofocus, (CLICKTOFOCUS == 1));
  fl_set_button (fd_def_color->opaquemove, (OPAQUEMOVE == 1));
  fl_show_form (fd_def_color->defcolor, FL_PLACE_CENTER, FL_TRANSIENT, title);
  fl_set_cursor (fd_def_color->defcolor->window, XFCE_CURS);
  /*
     Save original values if user cancels actions...
   */
  orig_colorize_root=COLORIZE_ROOT;
  orig_gradient_root=COLORIZE_ROOT;
  orig_detach_menu=DETACH_MENU;
  orig_clicktofocus=CLICKTOFOCUS;
  orig_opaquemove=OPAQUEMOVE;
  orig_autoraise=AUTORAISE;
}

void 
hide_defcolor (void)
{
  fl_hide_form (fd_def_color->defcolor);
  fl_activate_all_forms ();
}

void 
apply_colors (void)
{
  int i;

  applypal (palette);
  fl_set_cursor_color(XFCE_CURS, palcolor(palette, 0), FL_WHITE);
  fl_set_cursor_color(MENU_CURS, palcolor(palette, 0), FL_WHITE);
  COLORIZE_ROOT = fl_get_button (fd_def_color->color_workspace);
  GRADIENT_ROOT = fl_get_button (fd_def_color->gradient_workspace);
  DETACH_MENU = fl_get_button (fd_def_color->detach_menu);
  CLICKTOFOCUS = fl_get_button (fd_def_color->clicktofocus);
  OPAQUEMOVE = fl_get_button (fd_def_color->opaquemove);
  if (FVWM == 2) {
     apply_wm_colors (fd, palette);
     sendinfo(fd, (CLICKTOFOCUS ? FOCUSCLICK_CMD : FOCUSMOUSE_CMD) , 0);
     sendinfo(fd, (OPAQUEMOVE ? OPAQUEMOVEON_CMD : OPAQUEMOVEOFF_CMD), 0);
     sendinfo(fd, (AUTORAISE ? AUTORAISEON_CMD : AUTORAISEOFF_CMD), 0);
  }
  for (i = 0; i < NBMENUS; i++)
#if FL_INCLUDE_VERSION>=84
    if (fl_form_is_visible (menus[i]->pullmenu))
#endif
      fl_redraw_form (menus[i]->pullmenu);
#if FL_INCLUDE_VERSION>=84
  if (fl_form_is_visible (fd_info->info))
#endif
    fl_redraw_form (fd_info->info);
  /*
     Repaint screen if necessary 
   */
  if ((FVWM) && (COLORIZE_ROOT))
    update_screen (-1);
  fl_redraw_form (fd_def_color->defcolor);
  fl_redraw_form (fd_XFCE->XFCE);
}

void 
create_rgb (FL_OBJECT * obj, long c)
{
  int r, g, b;

  fl_freeze_form (fd_def_color->defcolor);
  r = fl_get_slider_value (fd_def_color->rs);
  g = fl_get_slider_value (fd_def_color->gs);
  b = fl_get_slider_value (fd_def_color->bs);
  fl_mapcolor (palette->cm[current_col], r, g, b);
  palette->r[current_col] = r;
  palette->g[current_col] = g;
  palette->b[current_col] = b;
  fl_redraw_object (fd_def_color->color_button[current_col]);
  fl_unfreeze_form (fd_def_color->defcolor);
}

void 
select_color (FL_OBJECT * ob, long c)
{
  fl_freeze_form (fd_def_color->defcolor);
  fl_set_slider_value (fd_def_color->rs, palette->r[c]);
  fl_set_slider_value (fd_def_color->gs, palette->g[c]);
  fl_set_slider_value (fd_def_color->bs, palette->b[c]);
  fl_unfreeze_form (fd_def_color->defcolor);
  current_col = c;
}

void 
defcolor_load_cb (FL_OBJECT * ob, long data)
{
  const char *fselect;

  fl_freeze_form (fd_def_color->defcolor);
  fselect = fl_show_fselector (rxfce.paltoload, XFCE_PAL, PAL_PATTERN, "");
  if (fselect)
    if (strlen (fselect))
      {
	if (existfile (fselect))
	  {
	    if (!loadnamepal (palette, fselect))
	      my_alert (rxfce.notload, 1);
	  }
	else
	  {
	    my_alert (rxfce.notfound, 1);
	  }
	apply_colors ();
	select_color (fd_def_color->color_button[current_col], current_col);
	fl_redraw_form (fd_def_color->defcolor);
	fl_redraw_form (fd_XFCE->XFCE);
      }
  fl_unfreeze_form (fd_def_color->defcolor);
}

void 
defcolor_save_cb (FL_OBJECT * ob, long data)
{
  const char *fselect;
  int rep = 1;

  fl_freeze_form (fd_def_color->defcolor);
  fselect = fl_show_fselector (rxfce.paltosave, XFCE_PAL, PAL_PATTERN, "");
  if (fselect)
    if (strlen (fselect))
      {
	if (existfile (fselect))
	  rep = (my_question (rxfce.overwrite, 1) == 1);
	if (rep)
	  if (!savenamepal (palette, fselect))
	    my_alert (rxfce.notwrite, 1);
      }
  fl_unfreeze_form (fd_def_color->defcolor);
}

void 
defcolor_default_cb (FL_OBJECT * ob, long data)
{
  fl_freeze_form (fd_def_color->defcolor);
  defpal (palette);
  apply_colors ();
  select_color (fd_def_color->color_button[current_col], current_col);
  fl_unfreeze_form (fd_def_color->defcolor);
}

void 
defcolor_ok_cb (FL_OBJECT * ob, long data)
{
  hide_defcolor ();
  apply_colors ();
  if (!savepal (palette))
    my_alert (rxfce.notsave, 1);
#ifdef ALLWAYS_SAVE
  writetoconfig ();
#endif
}

void 
defcolor_apply_cb (FL_OBJECT * ob, long data)
{
  fl_freeze_form (fd_def_color->defcolor);
  apply_colors ();
  fl_unfreeze_form (fd_def_color->defcolor);
}

void 
defcolor_cancel_cb (FL_OBJECT * ob, long data)
{
  hide_defcolor ();
  fl_set_button (fd_def_color->color_workspace,    (orig_colorize_root == 1));
  fl_set_button (fd_def_color->gradient_workspace, (orig_gradient_root == 1));
  fl_set_button (fd_def_color->detach_menu,        (orig_detach_menu   == 1));
  fl_set_button (fd_def_color->clicktofocus,       (orig_clicktofocus  == 1));
  fl_set_button (fd_def_color->opaquemove,         (orig_opaquemove    == 1));
  if (!loadpal (palette))
    my_alert (rxfce.notload, 1);
  apply_colors ();
}

void 
color_workspace_cb (FL_OBJECT * ob, long data)
{
  COLORIZE_ROOT = fl_get_button (fd_def_color->color_workspace);
  fl_set_button (fd_def_color->gradient_workspace, (GRADIENT_ROOT == 1) && (COLORIZE_ROOT == 1));
  update_options_gradient ();
}

void 
gradient_workspace_cb (FL_OBJECT * ob, long data)
{
  GRADIENT_ROOT = fl_get_button (fd_def_color->gradient_workspace);
  color_workspace_cb (ob, data);
}

void 
detach_menu_cb (FL_OBJECT * ob, long data)
{
  /*
     Nothing to do actually 
   */
}

void 
clicktofocus_cb (FL_OBJECT * ob, long data)
{
  CLICKTOFOCUS = fl_get_button (fd_def_color->clicktofocus);
  fl_set_button (fd_def_color->autoraise, (CLICKTOFOCUS == 0) && (AUTORAISE == 1));
  update_options_autoraise ();
}

void 
autoraise_cb (FL_OBJECT * ob, long data)
{
  AUTORAISE = fl_get_button (fd_def_color->autoraise);
  clicktofocus_cb (ob, data);
}

void 
opaquemove_cb (FL_OBJECT * ob, long data)
{
  /*
     Nothing to do actually 
   */
}

int 
defcolor_atclose (FL_FORM * f, long *d)
{
  hide_defcolor ();
  return (0);
}
