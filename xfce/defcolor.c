/*

   ORIGINAL FILE NAME : defcolor.c

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
#include <stdlib.h>
#include "extern.h"
#include "defcolor.h"
#include "color.h"
#include "resources.h"

int current_col;

FD_def_color *
create_form_def_color (Pixmap icon, Pixmap mask)
{
  FL_OBJECT *obj;
  FD_def_color *fdui = (FD_def_color *) fl_calloc (1, sizeof (*fdui));

  fdui->defcolor = fl_bgn_form (FL_FLAT_BOX, 390, 400);
  obj = fl_add_box (XFCE_BORDER, 0, 0, 390, 400, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  fl_add_frame (FL_ENGRAVED_FRAME, 10, 10, 370, 320, "");
  fl_add_frame (FL_ENGRAVED_FRAME, 10, 340, 370, 50, "");
  fl_add_frame (FL_ENGRAVED_FRAME, 20, 20, 350, 80, "");
  fl_add_frame (FL_ENGRAVED_FRAME, 20, 110, 350, 110, "");
  fl_add_frame (FL_ENGRAVED_FRAME, 20, 230, 350, 60, "");
  fdui->rs = obj = fl_add_valslider (FL_HOR_SLIDER, 30, 30, 330, 20, "");
  fl_set_object_color (obj, XFCE_COL5, FL_RED);
  fl_set_object_lsize (obj, FL_DEFAULT_SIZE);
  fl_set_object_lstyle (obj, FL_FIXED_STYLE);
  fl_set_slider_bounds (obj, 0, 255);
  fl_set_slider_precision (obj, 0);
  fl_set_object_callback (obj, create_rgb, 0);
  fl_set_slider_return (obj, FL_RETURN_CHANGED);
  fdui->gs = obj = fl_add_valslider (FL_HOR_SLIDER, 30, 50, 330, 20, "");
  fl_set_object_color (obj, XFCE_COL5, FL_GREEN);
  fl_set_object_lsize (obj, FL_DEFAULT_SIZE);
  fl_set_object_lstyle (obj, FL_FIXED_STYLE);
  fl_set_slider_bounds (obj, 0, 255);
  fl_set_slider_precision (obj, 0);
  fl_set_object_callback (obj, create_rgb, 1);
  fl_set_slider_return (obj, FL_RETURN_CHANGED);
  fdui->bs = obj = fl_add_valslider (FL_HOR_SLIDER, 30, 70, 330, 20, "");
  fl_set_object_color (obj, XFCE_COL5, FL_BLUE);
  fl_set_object_lsize (obj, FL_DEFAULT_SIZE);
  fl_set_object_lstyle (obj, FL_FIXED_STYLE);
  fl_set_slider_bounds (obj, 0, 255);
  fl_set_slider_precision (obj, 0);
  fl_set_object_callback (obj, create_rgb, 2);
  fl_set_slider_return (obj, FL_RETURN_CHANGED);
  fdui->color_button[0] = obj = fl_add_button (FL_RADIO_BUTTON, 30, 120, 75, 40, "");
  fl_set_object_color (obj, palette->cm[0], palette->cm[0]);
  fl_set_object_callback (obj, select_color, 0);
  fdui->color_button[1] = obj = fl_add_button (FL_RADIO_BUTTON, 115, 120, 75, 40, "");
  fl_set_object_color (obj, palette->cm[1], palette->cm[1]);
  fl_set_object_callback (obj, select_color, 1);
  fdui->color_button[2] = obj = fl_add_button (FL_RADIO_BUTTON, 200, 120, 75, 40, "");
  fl_set_object_color (obj, palette->cm[2], palette->cm[2]);
  fl_set_object_callback (obj, select_color, 2);
  fdui->color_button[3] = obj = fl_add_button (FL_RADIO_BUTTON, 285, 120, 75, 40, "");
  fl_set_object_color (obj, palette->cm[3], palette->cm[3]);
  fl_set_object_callback (obj, select_color, 3);
  fdui->color_button[4] = obj = fl_add_button (FL_RADIO_BUTTON, 30, 170, 75, 40, "");
  fl_set_object_color (obj, palette->cm[4], palette->cm[4]);
  fl_set_object_callback (obj, select_color, 4);
  fdui->color_button[5] = obj = fl_add_button (FL_RADIO_BUTTON, 115, 170, 75, 40, "");
  fl_set_object_color (obj, palette->cm[5], palette->cm[5]);
  fl_set_object_callback (obj, select_color, 5);
  fdui->color_button[6] = obj = fl_add_button (FL_RADIO_BUTTON, 200, 170, 75, 40, "");
  fl_set_object_color (obj, palette->cm[6], palette->cm[6]);
  fl_set_object_callback (obj, select_color, 6);
  fdui->color_button[7] = obj = fl_add_button (FL_RADIO_BUTTON, 285, 170, 75, 40, "");
  fl_set_object_color (obj, palette->cm[7], palette->cm[7]);
  fl_set_object_callback (obj, select_color, 7);
  fdui->color_workspace = obj = fl_add_checkbutton (FL_PUSH_BUTTON, 30, 230, 310, 30, rxfce.setuprepaint);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFBOLD + FL_SHADOW_STYLE);
  fl_set_choice_align (obj, FL_ALIGN_CENTER);
  fl_set_object_callback (obj, color_workspace_cb, 0);
  fdui->detach_menu = obj = fl_add_checkbutton (FL_PUSH_BUTTON, 30, 260, 310, 30, rxfce.setupdetach);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFBOLD + FL_SHADOW_STYLE);
  fl_set_choice_align (obj, FL_ALIGN_CENTER);
  fl_set_object_callback (obj, detach_menu_cb, 0);
  fdui->defcolor_load = obj = fl_add_button (FL_NORMAL_BUTTON, 20, 300, 110, 20, rxfce.loadlabel);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFBOLD + FL_ENGRAVED_STYLE);
  fl_set_object_callback (obj, defcolor_load_cb, 0);
  fdui->defcolor_save = obj = fl_add_button (FL_NORMAL_BUTTON, 140, 300, 110, 20, rxfce.savelabel);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFBOLD + FL_ENGRAVED_STYLE);
  fl_set_object_callback (obj, defcolor_save_cb, 0);
  fdui->defcolor_default = obj = fl_add_button (FL_NORMAL_BUTTON, 260, 300, 110, 20, rxfce.defaultlabel);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFBOLD + FL_ENGRAVED_STYLE);
  fl_set_object_callback (obj, defcolor_default_cb, 0);
  fdui->defcolor_OK = obj = fl_add_button (FL_NORMAL_BUTTON, 20, 350, 110, 30, rxfce.oklabel);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_button_shortcut (obj, "^O^M#O", 1);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFBOLD + FL_ENGRAVED_STYLE);
  fl_set_object_callback (obj, defcolor_ok_cb, 0);
  fdui->defcolor_apply = obj = fl_add_button (FL_NORMAL_BUTTON, 140, 350, 110, 30, rxfce.applylabel);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFBOLD + FL_ENGRAVED_STYLE);
  fl_set_object_callback (obj, defcolor_apply_cb, 0);
  fdui->defcolor_cancel = obj = fl_add_button (FL_NORMAL_BUTTON, 260, 350, 110, 30, rxfce.cancellabel);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_button_shortcut (obj, "^[^C#C", 1);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFBOLD + FL_ENGRAVED_STYLE);
  fl_set_object_callback (obj, defcolor_cancel_cb, 0);
  fl_end_form ();

  fl_set_form_atclose (fdui->defcolor, (FL_FORM_ATCLOSE) defcolor_atclose, 0);
  fl_set_form_icon (fdui->defcolor, icon, mask);

  return (fdui);
}
