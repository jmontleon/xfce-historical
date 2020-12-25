/*

   ORIGINAL FILE NAME : xfsound.c

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
#include "xfsound.h"
#include "../xfce/color.h"
#include "../xfce/constant.h"

FD_xfsound *create_form_xfsound(void)
{
  FL_OBJECT *obj;
  FD_xfsound *fdui = (FD_xfsound *) fl_calloc(1, sizeof(*fdui));

  fdui->xfsound = fl_bgn_form(FL_FLAT_BOX, 460, 250);
  obj = fl_add_box(XFCE_BORDER,0,0,460,250,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,10,440,170,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,20,120,420,50,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,190,440,50,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,20,20,420,90,"");
  fdui->eventlist = obj = fl_add_choice(FL_DROPLIST_CHOICE,120,30,190,30,rxfsnd.eventlabel);
    fl_set_object_boxtype (obj, FL_DOWN_BOX);
    fl_set_object_color (obj, XFCE_COL5, XFCE_COL10);
    fl_set_object_lcol (obj, XFCE_COL9);
    fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
    fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
    fl_set_choice_fontsize (obj, XFCE_NORMAL_SIZE);
    fl_set_choice_fontstyle (obj, XFBOLD);
    fl_set_choice_align (obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj,eventlist_cb,0);
  fdui->soundfile = obj = fl_add_input(FL_NORMAL_INPUT,120,70,190,30,rxfsnd.sndfilelabel);
    fl_set_object_color (obj, XFCE_COL5, XFCE_COL5);
    fl_set_object_lcol (obj, XFCE_COL9);
    fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
    fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
    fl_set_object_callback(obj,soundfile_cb,0);
  fdui->test = obj = fl_add_button(FL_NORMAL_BUTTON,320,30,110,30,rxfsnd.testlabel);
    fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
    fl_set_object_lsize (obj, FL_NORMAL_SIZE);
    fl_set_object_lcol (obj, XFCE_COL10);
    fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
    fl_set_object_callback(obj,testfile_cb,0);
  fdui->browsefile = obj = fl_add_button(FL_NORMAL_BUTTON,320,70,110,30,rxfsnd.browselabel);
    fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
    fl_set_object_lsize (obj, FL_NORMAL_SIZE);
    fl_set_object_lcol (obj, XFCE_COL10);
    fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
    fl_set_object_callback(obj,browsefile_cb,0);
  fdui->doplay = obj = fl_add_checkbutton(FL_PUSH_BUTTON,30,130,120,30,rxfsnd.playsndlabel);
    fl_set_object_lcol (obj, XFCE_COL9);
    fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
    fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
    fl_set_object_callback(obj,doplay_cb,0);
  fdui->play_com = obj = fl_add_input(FL_NORMAL_INPUT,240,130,160,30,rxfsnd.commandlabel);
    fl_set_object_color (obj, XFCE_COL5, XFCE_COL5);
    fl_set_object_lcol (obj, XFCE_COL9);
    fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
    fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
    fl_set_object_callback(obj,play_com_cb,0);
  fdui->internal = obj = fl_add_button(FL_NORMAL_BUTTON,410,130,20,15, "I");
    fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
    fl_set_object_lsize (obj, FL_NORMAL_SIZE);
    fl_set_object_lcol (obj, XFCE_COL10);
    fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
    fl_set_object_callback(obj,internal_cb,0);
  fdui->defaultcmd = obj = fl_add_button(FL_NORMAL_BUTTON,410,145,20,15,"D");
    fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
    fl_set_object_lsize (obj, FL_NORMAL_SIZE);
    fl_set_object_lcol (obj, XFCE_COL10);
    fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
    fl_set_object_callback(obj,defaultcmd_cb,0);
  fdui->ok = obj = fl_add_button(FL_NORMAL_BUTTON,40,200,110,30,rxfsnd.oklabel);
    fl_set_button_shortcut (obj, "^O^M#O", 1);
    fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
    fl_set_object_lsize (obj, FL_NORMAL_SIZE);
    fl_set_object_lcol (obj, XFCE_COL10);
    fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
    fl_set_object_callback(obj,ok_cb,0);
  fdui->apply = obj = fl_add_button(FL_NORMAL_BUTTON,170,200,120,30,rxfsnd.applylabel);
    fl_set_button_shortcut (obj, "^A#A", 1);
    fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
    fl_set_object_lsize (obj, FL_NORMAL_SIZE);
    fl_set_object_lcol (obj, XFCE_COL10);
    fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
    fl_set_object_callback(obj,apply_cb,0);
  fdui->cancel = obj = fl_add_button(FL_NORMAL_BUTTON,310,200,110,30,rxfsnd.cancellabel);
    fl_set_button_shortcut (obj, "^[^C#C", 1);
    fl_set_object_color (obj, XFCE_COLB, XFCE_COL3);
    fl_set_object_lsize (obj, FL_NORMAL_SIZE);
    fl_set_object_lcol (obj, XFCE_COL10);
    fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
    fl_set_object_callback(obj,cancel_cb,0);
  fl_end_form();

  fdui->xfsound->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

