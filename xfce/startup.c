

/*

   ORIGINAL FILE NAME : startup.c

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
#include "startup.h"
#include "constant.h"
#include "extern.h"
#include "resources.h"

FD_startup *fd_startup;
extern char *logo;

FD_startup *
create_form_startup (void)
{
  FL_OBJECT *obj;
  FD_startup *fdui = (FD_startup *) fl_calloc (1, sizeof (*fdui));

  fdui->startup = fl_bgn_form (FL_FLAT_BOX, 250, 140);
  obj = fl_add_box (FL_UP_BOX, 0, 0, 250, 140, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  if (fl_get_visual_depth () > 4)
    {
      fdui->logo = obj = fl_add_pixmap (FL_NORMAL_PIXMAP, 10, 10, 230, 83, "");
    }
  else
    {
      fdui->logo = obj = fl_add_text (FL_NORMAL_TEXT, 10, 10, 230, 83, "XFCE");
      fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
      fl_set_object_lalign (obj, FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
      fl_set_object_lsize (obj, XFCE_HUGE_SIZE);
      fl_set_object_lcol (obj, XFCE_COL10);
      fl_set_object_lstyle (obj, XFTIT + FL_ENGRAVED_STYLE);
    }
  fl_set_object_boxtype (obj, FL_NO_BOX);
  obj = fl_add_text (FL_NORMAL_TEXT, 10, 100, 230, 35, rxfce.startuptext);
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  fl_set_object_lalign (obj, FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
  fl_set_object_lsize (obj, XFCE_MEDIUM_SIZE);
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lstyle (obj, XFBOLD + FL_ENGRAVED_STYLE);
  fl_end_form ();

  return (fdui);
}

void 
open_startup (void)
{
  char *xfcedir;

  fd_startup = create_form_startup ();
  xfcedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  strcpy (xfcedir, XFCE_DIR);
  strcat (xfcedir, logo);
  if (fl_get_visual_depth () > 4)
    fl_set_pixmap_file (fd_startup->logo, xfcedir);

  /*
     show the first form 
   */
  fl_show_form (fd_startup->startup, FL_PLACE_CENTER, FL_NOBORDER, "Starting ...");
  fl_set_cursor (fd_startup->startup->window, XFCE_CURS);
  free (xfcedir);
}

void 
close_startup (void)
{
  fl_hide_form (fd_startup->startup);
  fl_free_form (fd_startup->startup);
  free (fd_startup);
}

/*---------------------------------------*/
