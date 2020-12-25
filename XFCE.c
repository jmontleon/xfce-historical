/*

   ORIGINAL FILE NAME : XFCE.c

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
#include "XFCE.h"
#include "screen.h"
#include "extern.h"
#include "selects.h"
#include "tiptools.h"

/*
   Load pixmap files 
 */

#include "minbutup.h"
#include "minbutdn.h"
#include "minipnt.h"
#include "mininf.h"
#include "minilock.h"
#include "handle.h"
#include "close.h"
#include "iconify.h"

void 
ToggleButtonUp (FL_OBJECT * obj)
{
  fl_set_pixmap_data (obj, minbutup);
  fl_set_button (obj, 0);
}

void 
ToggleButtonDn (FL_OBJECT * obj)
{
  fl_set_pixmap_data (obj, minbutdn);
  fl_set_button (obj, 1);
}

FD_XFCE *
create_form_XFCE (Pixmap icon, Pixmap mask)
{
  int i;
  FL_OBJECT *obj;
  FD_XFCE *fdui = (FD_XFCE *) fl_calloc (1, sizeof (*fdui));

  fdui->XFCE = fl_bgn_form (FL_FLAT_BOX, 784, 76);
  obj = fl_add_box (FL_BORDER_BOX, 0, 0, 784, 76, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  obj = fl_add_box (FL_UP_BOX, 0, 16, 784, 60, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  obj = fl_add_box (FL_FLAT_BOX, 250, 0, 284, 16, "");
  fl_set_object_color (obj, XFCE_COL2, XFCE_COL2);
  if (FVWM)
    {
      /*
         The close button on the upper left corner 
       */
      fdui->close = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 0, 0, 16, 16, "");
#if FL_INCLUDE_VERSION>=86
      fl_set_pixmapbutton_focus_outline (obj, 0);
#endif
      fl_set_object_boxtype (obj, FL_UP_BOX);
      fl_set_pixmap_data (obj, close);
      fl_set_object_callback (obj, quit_cb, 0);
      fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
      /*
         The handle/move button on the upper left corner 
       */
      fdui->noneL = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 16, 0, 54, 16, "");
#if FL_INCLUDE_VERSION>=86
      fl_set_pixmapbutton_focus_outline (obj, 0);
#endif
      fl_set_object_boxtype (obj, FL_UP_BOX);
      fl_set_pixmap_data (obj, handle);
      fl_set_object_callback (obj, move_cb, 0);
      fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
      /*
         The iconify button on the upper right corner 
       */
      fdui->icon = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 768, 0, 16, 16, "");
#if FL_INCLUDE_VERSION>=86
      fl_set_pixmapbutton_focus_outline (obj, 0);
#endif
      fl_set_object_boxtype (obj, FL_UP_BOX);
      fl_set_pixmap_data (obj, iconify);
      fl_set_object_callback (obj, iconify_cb, 0);
      fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
      /*
         The handle/move button on the upper right corner 
       */
      fdui->noneR = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 714, 0, 54, 16, "");
#if FL_INCLUDE_VERSION>=86
      fl_set_pixmapbutton_focus_outline (obj, 0);
#endif
      fl_set_object_boxtype (obj, FL_UP_BOX);
      fl_set_pixmap_data (obj, handle);
      fl_set_object_callback (obj, move_cb, 0);
      fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
    }
  else
    {
      fdui->noneL = obj = fl_add_box (FL_UP_BOX, 0, 0, 70, 16, "");
      fl_set_object_color (obj, XFCE_COL2, XFCE_COL2);
      fdui->noneR = obj = fl_add_box (FL_UP_BOX, 714, 0, 70, 16, "");
      fl_set_object_color (obj, XFCE_COL2, XFCE_COL2);
    }
  obj = fl_add_box (FL_FRAME_BOX, 14, 23, 46, 46, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  fdui->clock = obj = fl_add_clock (FL_ANALOG_CLOCK, 17, 26, 40, 40, "");
#if FL_INCLUDE_VERSION>=87
  fl_set_object_boxtype (obj, FL_OVAL3D_DOWNBOX);
#else
#if FL_INCLUDE_VERSION>=86
  /*
     A little bug in version 0.86 avoid use of OVAL3D_DOWNBOX 
   */
  fl_set_object_boxtype (obj, FL_ROUNDED3D_DOWNBOX);
#else
  fl_set_object_boxtype (obj, FL_RFLAT_BOX);
#endif
#endif
  fl_set_object_color (obj, XFCE_COL3, XFCE_COL8);


  fdui->selects[0] = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 73, 19, 54, 54, "");
  ToggleSelectUp (obj);
  fl_set_object_callback (obj, select_cb, 0);
  set_press_button (obj);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 1);
#endif
  fdui->selects[1] = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 133, 19, 54, 54, "");
  ToggleSelectUp (obj);
  fl_set_object_callback (obj, select_cb, 1);
  set_press_button (obj);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 1);
#endif
  fdui->selects[2] = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 193, 19, 54, 54, "");
  ToggleSelectUp (obj);
  fl_set_object_callback (obj, select_cb, 2);
  set_press_button (obj);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 1);
#endif
  fdui->selects[3] = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 537, 19, 54, 54, "");
  ToggleSelectUp (obj);
  fl_set_object_callback (obj, select_cb, 3);
  set_press_button (obj);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 1);
#endif
  fdui->selects[4] = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 597, 19, 54, 54, "");
  ToggleSelectUp (obj);
  fl_set_object_callback (obj, select_cb, 4);
  set_press_button (obj);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 1);
#endif
  fdui->selects[5] = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 657, 19, 54, 54, "");
  ToggleSelectUp (obj);
  fl_set_object_callback (obj, select_cb, 5);
  set_press_button (obj);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 1);
#endif
  fdui->selects[6] = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 717, 19, 54, 54, "");
  ToggleSelectUp (obj);
  fl_set_object_callback (obj, select_cb, 6);
  set_press_button (obj);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 1);
#endif

  fdui->popup[0] = obj = fl_add_pixmapbutton (FL_PUSH_BUTTON, 70, 0, 60, 16, "");
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
  fl_set_object_callback (obj, popup_cb, 0);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 0);
#endif
  fdui->popup[1] = obj = fl_add_pixmapbutton (FL_PUSH_BUTTON, 130, 0, 60, 16, "");
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
  fl_set_object_callback (obj, popup_cb, 1);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 0);
#endif
  fdui->popup[2] = obj = fl_add_pixmapbutton (FL_PUSH_BUTTON, 190, 0, 60, 16, "");
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
  fl_set_object_callback (obj, popup_cb, 2);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 0);
#endif
  fdui->popup[3] = obj = fl_add_pixmapbutton (FL_PUSH_BUTTON, 534, 0, 60, 16, "");
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
  fl_set_object_callback (obj, popup_cb, 3);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 0);
#endif
  fdui->popup[4] = obj = fl_add_pixmapbutton (FL_PUSH_BUTTON, 594, 0, 60, 16, "");
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
  fl_set_object_callback (obj, popup_cb, 4);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 0);
#endif
  fdui->popup[5] = obj = fl_add_pixmapbutton (FL_PUSH_BUTTON, 654, 0, 60, 16, "");
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
  fl_set_object_callback (obj, popup_cb, 5);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 0);
#endif

  fdui->centralframe = obj = fl_add_box (FL_UP_BOX, 252, 0, 280, 76, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  obj = fl_add_box (FL_DOWN_BOX, 295, 10, 94, 24, "");
  fdui->screen[0] = obj = fl_add_button (FL_RADIO_BUTTON, 297, 12, 90, 20, "");
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_object_color (obj, screen_colors[0 % 4], screen_colors[0 % 4]);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFBOLD + FL_SHADOW_STYLE);
  fl_set_object_lalign (obj, FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  fl_set_object_callback (obj, screen_cb, 0);
  fl_set_button (obj, 1);
  obj = fl_add_box (FL_DOWN_BOX, 395, 10, 94, 24, "");
  fdui->screen[1] = obj = fl_add_button (FL_RADIO_BUTTON, 397, 12, 90, 20, "");
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_object_color (obj, screen_colors[1 % 4], screen_colors[1 % 4]);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFBOLD + FL_SHADOW_STYLE);
  fl_set_object_lalign (obj, FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  fl_set_object_callback (obj, screen_cb, 1);
  obj = fl_add_box (FL_DOWN_BOX, 295, 42, 94, 24, "");
  fdui->screen[2] = obj = fl_add_button (FL_RADIO_BUTTON, 297, 44, 90, 20, "");
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_object_color (obj, screen_colors[2 % 4], screen_colors[2 % 4]);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFBOLD + FL_SHADOW_STYLE);
  fl_set_object_lalign (obj, FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  fl_set_object_callback (obj, screen_cb, 2);
  obj = fl_add_box (FL_DOWN_BOX, 395, 42, 94, 24, "");
  fdui->screen[3] = obj = fl_add_button (FL_RADIO_BUTTON, 397, 44, 90, 20, "");
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_object_color (obj, screen_colors[3 % 4], screen_colors[3 % 4]);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFBOLD + FL_SHADOW_STYLE);
  fl_set_object_lalign (obj, FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  fl_set_object_callback (obj, screen_cb, 3);
  fdui->selects[NBSELECTS] = obj
    = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 257, 8, 35, 28, "");
  fl_set_object_boxtype (obj, FL_FLAT_BOX);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL1);
  fl_set_object_lsize (obj, XFCE_TINY_SIZE);
  fl_set_object_lstyle (obj, XFTINY);
  fl_set_object_callback (obj, select_cb, NBSELECTS);
  set_press_button (obj);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 1);
#endif
  fdui->INFO = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 257, 40, 35, 28, "");
  fl_set_object_boxtype (obj, FL_FLAT_BOX);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL1);
  fl_set_object_lsize (obj, XFCE_TINY_SIZE);
  fl_set_object_lstyle (obj, XFTINY);
  fl_set_object_callback (obj, info_cb, 0);
  set_tip (obj, "About XFCE...");
  set_press_button (obj);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 1);
#endif
  fdui->SETUP = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 492, 8, 35, 28, "");
  fl_set_object_boxtype (obj, FL_FLAT_BOX);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL1);
  fl_set_object_lsize (obj, XFCE_TINY_SIZE);
  fl_set_object_lstyle (obj, XFTINY);
  fl_set_object_callback (obj, setup_cb, 0);
  set_tip (obj, "Setup...");
  set_press_button (obj);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, 1);
#endif
  fdui->QUIT = obj = fl_add_button (FL_NORMAL_BUTTON, 492, 44, 35, 20, "QUIT");
  fl_set_object_boxtype (obj, FL_FRAME_BOX);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL3);
  fl_set_object_lsize (obj, XFCE_TINY_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFTINY);
  fl_set_object_callback (obj, quit_cb, 0);

  fl_end_form ();

  fl_set_pixmap_data (fdui->selects[NBSELECTS], minilock);
  fl_set_pixmap_data (fdui->INFO, mininf);
  fl_set_pixmap_data (fdui->SETUP, minipnt);

  for (i = 0; i < NBMENUS; i++)
    {
      ToggleButtonUp (fdui->popup[i]);
      fl_set_pixmap_data (fdui->popup[i], minbutup);
    }

  for (i = 0; i < NBSELECTS + 1; i++)
    set_tip (fdui->selects[i], selects[i].command);

  fl_set_form_atclose (fdui->XFCE, (FL_FORM_ATCLOSE) XFCE_atclose, 0);
  fl_set_form_icon (fdui->XFCE, icon, mask);

  return (fdui);
}

/*---------------------------------------*/
