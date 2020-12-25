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

inline FL_OBJECT *
option_button (int t, FL_COORD x, FL_COORD y, FL_COORD w, FL_COORD h, char * l)
{
  FL_OBJECT * obj;
#if FL_INCLUDE_VERSION>=86
  obj = fl_add_round3dbutton (t, x, y, w, h, l);
#else
  obj = fl_add_checkbutton  (t, x, y, w, h, l);
#endif
  fl_set_object_boxtype (obj, FL_NO_BOX);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL1);
  return obj;
}


FD_def_color *
create_form_def_color (Pixmap icon, Pixmap mask)
{
  FL_OBJECT *obj;
  FD_def_color *fdui = (FD_def_color *) fl_calloc (1, sizeof (*fdui));

  fdui->defcolor = fl_bgn_form (FL_FLAT_BOX, 330, 455);
  obj = fl_add_box (XFCE_BORDER, 0, 0, 330, 455, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  fl_add_frame (FL_ENGRAVED_FRAME, 10, 10, 310, 384, "");
  fl_add_frame (FL_ENGRAVED_FRAME, 10, 396, 310, 50, "");
  fl_add_frame (FL_ENGRAVED_FRAME, 20, 20, 290, 65, "");
  fl_add_frame (FL_ENGRAVED_FRAME, 20, 95, 290, 70, "");
  fl_add_frame (FL_ENGRAVED_FRAME, 20, 175, 290, 180, "");
  fdui->rs = obj = fl_add_valslider (FL_HOR_SLIDER, 30, 30, 270, 15, "");
  fl_set_object_color (obj, XFCE_COL5, FL_RED);
  fl_set_object_lsize (obj, FL_DEFAULT_SIZE);
  fl_set_object_lstyle (obj, FL_FIXED_STYLE);
  fl_set_slider_bounds (obj, 0, 255);
  fl_set_slider_precision (obj, 0);
  fl_set_object_callback (obj, create_rgb, 0);
  fl_set_slider_return (obj, FL_RETURN_CHANGED);
  fdui->gs = obj = fl_add_valslider (FL_HOR_SLIDER, 30, 45, 270, 15, "");
  fl_set_object_color (obj, XFCE_COL5, FL_GREEN);
  fl_set_object_lsize (obj, FL_DEFAULT_SIZE);
  fl_set_object_lstyle (obj, FL_FIXED_STYLE);
  fl_set_slider_bounds (obj, 0, 255);
  fl_set_slider_precision (obj, 0);
  fl_set_object_callback (obj, create_rgb, 1);
  fl_set_slider_return (obj, FL_RETURN_CHANGED);
  fdui->bs = obj = fl_add_valslider (FL_HOR_SLIDER, 30, 60, 270, 15, "");
  fl_set_object_color (obj, XFCE_COL5, FL_BLUE);
  fl_set_object_lsize (obj, FL_DEFAULT_SIZE);
  fl_set_object_lstyle (obj, FL_FIXED_STYLE);
  fl_set_slider_bounds (obj, 0, 255);
  fl_set_slider_precision (obj, 0);
  fl_set_object_callback (obj, create_rgb, 2);
  fl_set_slider_return (obj, FL_RETURN_CHANGED);
  fdui->color_button[0] = obj = fl_add_button (FL_RADIO_BUTTON, 30, 105, 60, 20, "");
  fl_set_object_color (obj, palette->cm[0], palette->cm[0]);
  fl_set_object_callback (obj, select_color, 0);
  fdui->color_button[1] = obj = fl_add_button (FL_RADIO_BUTTON, 100, 105, 60, 20, "");
  fl_set_object_color (obj, palette->cm[1], palette->cm[1]);
  fl_set_object_callback (obj, select_color, 1);
  fdui->color_button[2] = obj = fl_add_button (FL_RADIO_BUTTON, 170, 105, 60, 20, "");
  fl_set_object_color (obj, palette->cm[2], palette->cm[2]);
  fl_set_object_callback (obj, select_color, 2);
  fdui->color_button[3] = obj = fl_add_button (FL_RADIO_BUTTON, 240, 105, 60, 20, "");
  fl_set_object_color (obj, palette->cm[3], palette->cm[3]);
  fl_set_object_callback (obj, select_color, 3);
  fdui->color_button[4] = obj = fl_add_button (FL_RADIO_BUTTON, 30, 135, 60, 20, "");
  fl_set_object_color (obj, palette->cm[4], palette->cm[4]);
  fl_set_object_callback (obj, select_color, 4);
  fdui->color_button[5] = obj = fl_add_button (FL_RADIO_BUTTON, 100, 135, 60, 20, "");
  fl_set_object_color (obj, palette->cm[5], palette->cm[5]);
  fl_set_object_callback (obj, select_color, 5);
  fdui->color_button[6] = obj = fl_add_button (FL_RADIO_BUTTON, 170, 135, 60, 20, "");
  fl_set_object_color (obj, palette->cm[6], palette->cm[6]);
  fl_set_object_callback (obj, select_color, 6);
  fdui->color_button[7] = obj = fl_add_button (FL_RADIO_BUTTON, 240, 135, 60, 20, "");
  fl_set_object_color (obj, palette->cm[7], palette->cm[7]);
  fl_set_object_callback (obj, select_color, 7);
  fdui->color_workspace = obj = option_button (FL_PUSH_BUTTON, 30, 175, 330, 30, rxfce.setuprepaint);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
  fl_set_choice_align (obj, FL_ALIGN_CENTER);
  fl_set_object_callback (obj, color_workspace_cb, 0);
  fdui->fake_back_gradient = obj = fl_add_box (FL_FLAT_BOX, 30, 205, 270, 30, "");
  fl_set_object_color (obj, XFCE_COLB, XFCE_COLB);
  fdui->gradient_workspace = obj = option_button (FL_PUSH_BUTTON, 30, 205, 330, 30, rxfce.setupgradient);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
  fl_set_choice_align (obj, FL_ALIGN_CENTER);
  fl_set_object_callback (obj, gradient_workspace_cb, 0);
  fdui->detach_menu = obj = option_button (FL_PUSH_BUTTON, 30, 235, 270, 30, rxfce.setupdetach);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
  fl_set_choice_align (obj, FL_ALIGN_CENTER);
  fl_set_object_callback (obj, detach_menu_cb, 0);
  fdui->clicktofocus = obj = option_button (FL_PUSH_BUTTON, 30, 265, 270, 30, rxfce.clicktofocus);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
  fl_set_choice_align (obj, FL_ALIGN_CENTER);
  fl_set_object_callback (obj, clicktofocus_cb, 0);
  fdui->fake_back_autoraise = obj = fl_add_box (FL_FLAT_BOX, 30, 295, 270, 30, "");
  fl_set_object_color (obj, XFCE_COLB, XFCE_COLB);
  fdui->autoraise = obj = option_button (FL_PUSH_BUTTON, 30, 295, 270, 30, rxfce.autoraise);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
  fl_set_choice_align (obj, FL_ALIGN_CENTER);
  fl_set_object_callback (obj, autoraise_cb, 0);
  fdui->opaquemove = obj = option_button (FL_PUSH_BUTTON, 30, 325, 270, 30, rxfce.opaquemove);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
  fl_set_choice_align (obj, FL_ALIGN_CENTER);
  fl_set_object_callback (obj, opaquemove_cb, 0);
  fdui->defcolor_load = obj = fl_add_button (FL_NORMAL_BUTTON, 20, 365, 90, 20, rxfce.loadlabel);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, defcolor_load_cb, 0);
  fdui->defcolor_save = obj = fl_add_button (FL_NORMAL_BUTTON, 120, 365, 90, 20, rxfce.savelabel);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, defcolor_save_cb, 0);
  fdui->defcolor_default = obj = fl_add_button (FL_NORMAL_BUTTON, 220, 365, 90, 20, rxfce.defaultlabel);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, defcolor_default_cb, 0);
  fdui->defcolor_OK = obj = fl_add_button (FL_NORMAL_BUTTON, 20, 405, 90, 30, rxfce.oklabel);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_button_shortcut (obj, "^O^M#O", 1);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, defcolor_ok_cb, 0);
  fdui->defcolor_apply = obj = fl_add_button (FL_NORMAL_BUTTON, 120, 405, 90, 30, rxfce.applylabel);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, defcolor_apply_cb, 0);
  fdui->defcolor_cancel = obj = fl_add_button (FL_NORMAL_BUTTON, 220, 405, 90, 30, rxfce.cancellabel);
  fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
  fl_set_button_shortcut (obj, "^[^C#C", 1);
  fl_set_object_lsize (obj, FL_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, defcolor_cancel_cb, 0);
  fl_end_form ();

  fl_set_form_atclose (fdui->defcolor, (FL_FORM_ATCLOSE) defcolor_atclose, 0);
  fl_set_form_icon (fdui->defcolor, icon, mask);

  return (fdui);
}
