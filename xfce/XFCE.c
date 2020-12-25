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
#include "move.h"

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

FL_OBJECT *
add_popup_button (int n, FL_COORD x, FL_COORD y)
{
  FL_OBJECT * obj;
  
  obj = fl_add_pixmapbutton (FL_PUSH_BUTTON, x, y, 60, 16, "");
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
  fl_set_object_callback (obj, popup_cb, n);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, HILIGHTWIDTH);
#endif
  return obj;
}

FL_OBJECT *
add_genpix_button (FL_COORD x, FL_COORD y,FL_COORD v, FL_COORD h)
{
  FL_OBJECT * obj;
  
  obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, x, y, v, h, "");
  set_press_button (obj, XFCE_COL8, XFCE_COL1);
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, HILIGHTSELECT);
#endif
  return obj;
}

FL_OBJECT *
add_select_button (int n, FL_COORD x, FL_COORD y)
{
  FL_OBJECT * obj;
  
  obj = add_genpix_button (x, y, 54, 54);
  fl_set_object_callback (obj, select_cb, n);
  return obj;
}

FL_OBJECT *
add_mini_button (FL_COORD x, FL_COORD y)
{
  return add_genpix_button (x, y, 35, 28);
}

FL_OBJECT *
add_screen_button (int n, FL_COORD x, FL_COORD y)
{
  FL_OBJECT * obj;

  obj = fl_add_box (FL_DOWN_BOX, x - BORDERWIDTH, y - BORDERWIDTH, 90 + 2 * BORDERWIDTH, 20 + 2 * BORDERWIDTH, "");
  obj = fl_add_button (FL_RADIO_BUTTON, x, y, 90, 20, "");
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_object_color (obj, screen_colors[n % 4], screen_colors[n % 4]);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFBOLD + FL_SHADOW_STYLE);
  fl_set_object_lalign (obj, FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  fl_set_object_callback (obj, screen_cb, n);
  
  return obj;
}


FD_XFCE *
create_form_XFCE (Pixmap icon, Pixmap mask)
{
  int i;
  FL_OBJECT *obj;
  FD_XFCE *fdui = (FD_XFCE *) fl_calloc (1, sizeof (*fdui));

  fdui->XFCE = fl_bgn_form (FL_NO_BOX, 792, 84);
  obj = fl_add_box (FL_UP_BOX, 0, 0, 792, 84, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  obj = fl_add_box (FL_BORDER_BOX, 4, 4, 784, 76, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  obj = fl_add_box (FL_UP_BOX, 4, 20, 784, 60, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  obj = fl_add_box (FL_FLAT_BOX, 254, 4, 284, 16, "");
  fl_set_object_color (obj, XFCE_COL2, XFCE_COL2);
  /*
     The close button on the upper left corner 
   */
  fdui->close = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 4, 4, 16, 16, "");
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, HILIGHTWIDTH);
#endif
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_pixmap_data (obj, close);
  fl_set_object_callback (obj, quit_cb, 0);
  fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
  /*
     The iconify button on the upper right corner 
   */
  fdui->icon = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 772, 4, 16, 16, "");
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, HILIGHTWIDTH);
#endif
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_pixmap_data (obj, iconify);
  fl_set_object_callback (obj, iconify_cb, 0);
  fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
  /*
      The handle/move button on the upper left corner 
  */
  fdui->noneL = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 20, 4, 54, 16, "");
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, HILIGHTWIDTH);
#endif
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_pixmap_data (obj, handle);
  fl_set_object_callback (obj, move_cb, 0);
  fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
  set_move_button (obj);
  /*
      The handle/move button on the upper right corner 
  */
  fdui->noneR = obj = fl_add_pixmapbutton (FL_NORMAL_BUTTON, 718, 4, 54, 16, "");
#if FL_INCLUDE_VERSION>=86
  fl_set_pixmapbutton_focus_outline (obj, HILIGHTWIDTH);
#endif
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_pixmap_data (obj, handle);
  fl_set_object_callback (obj, move_cb, 0);
  fl_set_object_color (obj, XFCE_COL2, XFCE_COL1);
  set_move_button (obj);

  obj = fl_add_box (FL_FRAME_BOX, 18, 27, 46, 46, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  fdui->clock = obj = fl_add_clock (FL_ANALOG_CLOCK, 21, 30, 40, 40, "");
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

  fdui->selects[0] = add_select_button (0,  77, 23);
  fdui->selects[1] = add_select_button (1, 137, 23);
  fdui->selects[2] = add_select_button (2, 197, 23);
  fdui->selects[3] = add_select_button (3, 541, 23);
  fdui->selects[4] = add_select_button (4, 601, 23);
  fdui->selects[5] = add_select_button (5, 661, 23);
  fdui->selects[6] = add_select_button (6, 721, 23);

  fdui->popup[0]   = add_popup_button (0,  74, 4);
  fdui->popup[1]   = add_popup_button (1, 134, 4);
  fdui->popup[2]   = add_popup_button (2, 194, 4);
  fdui->popup[3]   = add_popup_button (3, 538, 4);
  fdui->popup[4]   = add_popup_button (4, 598, 4);
  fdui->popup[5]   = add_popup_button (5, 658, 4);

  fdui->centralframe = obj = fl_add_box (FL_UP_BOX, 256, 4, 280, 76, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);

  fdui->screen[0] = obj = add_screen_button (0, 301, 16);
  fl_set_button (obj, 1);
  fdui->screen[1] = add_screen_button (1, 401, 16);
  fdui->screen[2] = add_screen_button (2, 301, 48);
  fdui->screen[3] = add_screen_button (3, 401, 48);

  fdui->selects[NBSELECTS] = obj = add_mini_button (261, 12);
  fl_set_object_callback (obj, select_cb, NBSELECTS);

  fdui->INFO = obj = add_mini_button (261, 44);
  fl_set_object_callback (obj, info_cb, 0);
  set_tip (obj, "About XFCE...");

  fdui->SETUP = obj = add_mini_button (496, 12);
  fl_set_object_callback (obj, setup_cb, 0);
  set_tip (obj, "Setup...");

  fdui->QUIT = obj = fl_add_button (FL_NORMAL_BUTTON, 496, 48, 35, 20, "QUIT");
  fl_set_object_boxtype (obj, FL_FRAME_BOX);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL3);
  fl_set_object_lsize (obj, XFCE_TINY_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFTINY);
  fl_set_object_callback (obj, quit_cb, 0);
  set_tip (obj, "Whoops...");

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
