
/*

   ORIGINAL FILE NAME : addicon.c

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
#include "forms.h"
#include "addicon.h"
#include "pulldown.h"
#include "constant.h"
#include "empty.h"
#include "extern.h"
#include "resources.h"
#include "fileutil.h"
#include "xpmext.h"
#include "defaulticon.h"

FD_addiconform *
create_form_addiconform (Pixmap icon, Pixmap mask)
{
  FL_OBJECT *obj;
  FD_addiconform *fdui = (FD_addiconform *) fl_calloc (1, sizeof (*fdui));

  fdui->addiconform = fl_bgn_form (FL_FLAT_BOX, 600, 210);
  obj = fl_add_box (XFCE_BORDER, 0, 0, 600, 210, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  fdui->iconframe = obj = fl_add_frame (FL_ENGRAVED_FRAME, 20, 20, 90, 90, "");
  fdui->previewpix = obj = fl_add_pixmap (FL_NORMAL_PIXMAP, 30, 30, 70, 70, "");
  fdui->comline_input = obj = fl_add_input (FL_NORMAL_INPUT, 250, 20, 240, 30, rxfce.additemcommand);
  fl_set_object_color (obj, XFCE_COL5, XFCE_COL5);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
  fl_set_object_callback (obj, dummy_callback, 0);
  fdui->pixfile_input = obj = fl_add_input (FL_NORMAL_INPUT, 250, 60, 240, 30, rxfce.additemiconfile);
  fl_set_object_color (obj, XFCE_COL5, XFCE_COL5);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
  fl_set_object_callback (obj, update_pixfile, 0);
  fdui->label_input = obj = fl_add_input (FL_NORMAL_INPUT, 250, 100, 240, 30, rxfce.additemlabel);
  fl_set_object_color (obj, XFCE_COL5, XFCE_COL5);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lstyle (obj, XFCE_ACTIVELABEL);
  fl_set_object_callback (obj, dummy_callback, 0);
  obj = fl_add_frame (FL_ENGRAVED_FRAME, 130, 10, 460, 130, "");
  obj = fl_add_text (FL_NORMAL_TEXT, 10, 120, 110, 40, rxfce.additempreview);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  fl_set_object_lalign (obj, FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fdui->br_comline = obj = fl_add_button (FL_NORMAL_BUTTON, 500, 20, 80, 30, rxfce.browselabel);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL3);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, br_comline_cb, 0);
  fdui->br_iconpix = obj = fl_add_button (FL_NORMAL_BUTTON, 500, 60, 80, 30, rxfce.browselabel);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL3);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, br_iconpix_cb, 0);
  obj = fl_add_frame (FL_ENGRAVED_FRAME, 155, 150, 410, 50, "");
  fdui->OK = obj = fl_add_button (FL_NORMAL_BUTTON, 175, 160, 110, 30, rxfce.oklabel);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL3);
  fl_set_button_shortcut (obj, "^O^M#O", 1);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, ai_ok_cb, 0);
  fdui->ai_cancel = obj = fl_add_button (FL_NORMAL_BUTTON, 305, 160, 110, 30, rxfce.cancellabel);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL3);
  fl_set_button_shortcut (obj, "^[^C#C", 1);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, ai_cancel_cb, 0);
  fdui->ai_remove = obj = fl_add_button (FL_NORMAL_BUTTON, 435, 160, 110, 30, rxfce.removelabel);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL3);
  fl_set_object_lsize (obj, XFCE_NORMAL_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFCE_BUTSTYLE);
  fl_set_object_callback (obj, ai_remove_cb, 0);
  fl_end_form ();

  fl_set_form_atclose (fdui->addiconform, (FL_FORM_ATCLOSE) addicon_atclose, 0);
  fl_set_form_icon (fdui->addiconform, icon, mask);

  return (fdui);
}

void 
set_addicon_cb (FD_addiconform * f, int no_menu, int pos)
{
  fl_set_object_callback (f->OK, ai_ok_cb, no_menu * NBMAXITEMS + pos);
  fl_set_object_callback (f->ai_remove, ai_remove_cb, no_menu * NBMAXITEMS + pos);
}

void 
set_addiconform (FD_addiconform * f, int no_menu, int pos)
{
  char *pixfile = menus[no_menu]->menuitem[pos]->pixfile;
  char *label = menus[no_menu]->menuitem[pos]->label;
  char *command = menus[no_menu]->menuitem[pos]->command;

  if (pos < 0)
    {
      MySetPixmapData (fd_addiconform->previewpix, empty);
      fl_set_input (f->pixfile_input, "");
      fl_set_input (f->label_input, "");
      fl_set_input (f->comline_input, "");
    }
  else
    {
      if (existfile (pixfile))
	{
	  MySetPixmapFile (fd_addiconform->previewpix, pixfile);
	}
      else
	{
	  MySetPixmapData (fd_addiconform->previewpix, defaulticon);
	}
      fl_set_input (f->pixfile_input, pixfile);
      fl_set_input (f->label_input, label);
      fl_set_input (f->comline_input, command);
    }
}

/*---------------------------------------*/
