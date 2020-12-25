#include "forms.h"
#include <stdlib.h>
#include "xfmouse.h"

#include "xfmouse.xpm"

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


FD_xfmouse *create_form_xfmouse(void)
{
  FL_OBJECT *obj;
  FD_xfmouse *fdui = (FD_xfmouse *) fl_calloc(1, sizeof(*fdui));

  fdui->xfmouse = fl_bgn_form(FL_NO_BOX, 330, 260);
  obj = fl_add_box(FL_UP_BOX,0,0,330,260,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,10,310,180,"");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,20,20,290,70,rxfmouse.buttonlabel);
    fl_set_object_lcol (obj, XFCE_COL10);
    fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
    fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,20,100,290,80,rxfmouse.motionlabel);
    fl_set_object_lcol (obj, XFCE_COL10);
    fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
    fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,200,310,50,"");
  obj = fl_add_pixmap(FL_NORMAL_PIXMAP,30,30,50,60,"");
    fl_set_pixmap_data(obj,xfmouse_xpm);
  fdui->leftbtn = obj = option_button(FL_PUSH_BUTTON,140,40,30,30,rxfmouse.leftlabel);
    fl_set_object_lcol (obj, XFCE_COL9);
    fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
    fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
    fl_set_choice_fontstyle (obj, XFBOLD);
    fl_set_object_callback(obj,leftbtn_cb,0);
    fl_set_button(obj, 1);
  fdui->rightbtn = obj = option_button(FL_PUSH_BUTTON,220,40,30,30,rxfmouse.rightlabel);
    fl_set_object_lcol (obj, XFCE_COL9);
    fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
    fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
    fl_set_choice_fontsize (obj, XFCE_NORMAL_SIZE);
    fl_set_choice_fontstyle (obj, XFBOLD);
    fl_set_object_callback(obj,rightbtn_cb,0);
  fdui->accel = obj = fl_add_slider(FL_HOR_SLIDER,100,110,200,20,rxfmouse.accelerationlabel);
    fl_set_object_color (obj, XFCE_COL5, XFCE_COL8);
    fl_set_object_lcol (obj, XFCE_COL9);
    fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
    fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
    fl_set_choice_fontsize (obj, XFCE_NORMAL_SIZE);
    fl_set_choice_fontstyle (obj, XFBOLD);
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_callback(obj,accel_cb,0);
    fl_set_slider_bounds (obj, ACCEL_MIN, ACCEL_MAX);
    /* fl_set_slider_size(obj, 0.15); */
  fdui->thresh = obj = fl_add_slider(FL_HOR_SLIDER,100,150,200,20,rxfmouse.thresholdlabel);
    fl_set_object_color (obj, XFCE_COL5, XFCE_COL8);
    fl_set_object_lcol (obj, XFCE_COL9);
    fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
    fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
    fl_set_choice_fontsize (obj, XFCE_NORMAL_SIZE);
    fl_set_choice_fontstyle (obj, XFBOLD);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_callback(obj,thresh_cb,0);
    fl_set_slider_bounds (obj, THRESH_MIN, THRESH_MAX);
    /* fl_set_slider_size(obj, 0.15); */
  fdui->ok = obj = fl_add_button(FL_NORMAL_BUTTON,20,210,90,30,rxfmouse.oklabel);
    fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
    fl_set_button_shortcut (obj, "^O^M#O", 1);
    fl_set_object_lsize (obj, FL_NORMAL_SIZE);
    fl_set_object_lcol (obj, XFCE_COL10);
    fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
    fl_set_object_callback(obj,ok_cb,0);
  fdui->apply = obj = fl_add_button(FL_NORMAL_BUTTON,120,210,90,30,rxfmouse.applylabel);
    fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
    fl_set_object_lsize (obj, FL_NORMAL_SIZE);
    fl_set_object_lcol (obj, XFCE_COL10);
    fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
    fl_set_object_callback(obj,apply_cb,0);
  fdui->cancel = obj = fl_add_button(FL_NORMAL_BUTTON,220,210,90,30,rxfmouse.cancellabel);
    fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
    fl_set_button_shortcut (obj, "^[^C#C", 1);
    fl_set_object_lsize (obj, FL_NORMAL_SIZE);
    fl_set_object_lcol (obj, XFCE_COL10);
    fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
    fl_set_object_callback(obj,cancel_cb,0);
  fl_end_form();

  fdui->xfmouse->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

