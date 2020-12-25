
/*

   ORIGINAL FILE NAME : command.c

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


#include "command.h"
#include "forms.h"
#include <stdlib.h>
#include "selects.h"
#include "resources.h"
#include "constant.h"

void 
inactive_choice (FL_OBJECT * obj)
{
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL10);
  fl_clear_choice (obj);
  fl_set_object_lstyle (obj, XFCE_INACTIVELABEL);
  fl_set_object_lcol (obj, XFCE_COL8);
  fl_deactivate_object (obj);
}

void 
active_choice (FL_OBJECT * obj)
{
  fl_set_object_color (obj, XFCE_COL5, XFCE_COL10);
  if (fl_get_choice_maxitems (obj) == 0)
    init_choice_str (obj);
  fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_activate_object (obj);
}

FD_def_command *
create_form_def_command (Pixmap icon, Pixmap mask)
{
  FL_OBJECT *obj;
  FD_def_command *fdui = (FD_def_command *) fl_calloc (1, sizeof (*fdui));

  fdui->defcom = fl_bgn_form (FL_FLAT_BOX, 440, 170);
  fdui->def_command = obj = fl_add_box (XFCE_BORDER, 0, 0, 440, 170, "");
  obj = fl_add_frame (FL_ENGRAVED_FRAME, 10, 10, 420, 90, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  fdui->command_input = obj = fl_add_input (FL_NORMAL_INPUT, 130, 20, 200, 30, rxfce.defcomcommand);
  fl_set_object_color (obj, XFCE_COL5, XFCE_COL5);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
  fl_set_object_callback (obj, DM_com_cb, 0);
  fdui->br_command = obj = fl_add_button (FL_NORMAL_BUTTON, 340, 20, 80, 30, rxfce.browselabel);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL3);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, br_com_cb, 0);
  fdui->icon_choice = obj = fl_add_choice (FL_DROPLIST_CHOICE, 130, 60, 200, 30, rxfce.defcomicon);
  fl_set_object_boxtype (obj, FL_DOWN_BOX);
  fl_set_object_color (obj, XFCE_COL5, XFCE_COL10);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
  fl_set_choice_fontsize (obj, XFCE_NORMAL_SIZE);
  fl_set_choice_fontstyle (obj, XFBOLD);
  fl_set_choice_align (obj, FL_ALIGN_LEFT);
  fl_set_object_callback (obj, DM_com_cb, 0);
  fdui->br_icon = obj = fl_add_button (FL_NORMAL_BUTTON, 340, 60, 80, 30, rxfce.browselabel);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL3);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, br_icon_cb, 0);
  obj = fl_add_frame (FL_ENGRAVED_FRAME, 90, 110, 260, 50, "");
  fdui->defcom_ok_but = obj = fl_add_button (FL_NORMAL_BUTTON, 110, 120, 100, 30, rxfce.oklabel);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL3);
  fl_set_button_shortcut (obj, "^O^M#O", 1);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, OK_defcom_cb, 0);
  fdui->defcom_cancel_but = obj = fl_add_button (FL_NORMAL_BUTTON, 230, 120, 100, 30, rxfce.cancellabel);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL3);
  fl_set_button_shortcut (obj, "^[^C#C", 1);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, CANCEL_defcom_cb, 0);
  fl_end_form ();

  fl_set_form_atclose (fdui->defcom, (FL_FORM_ATCLOSE) command_atclose, 0);
  fl_set_form_icon (fdui->defcom, icon, mask);

  return (fdui);
}
/*---------------------------------------*/
