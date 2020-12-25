/*

   ORIGINAL FILE NAME : defcolor.h

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



#ifndef FD_defcolor_h_
#define FD_defcolor_h_

#include "constant.h"
#include "color.h"

extern int current_col;

extern void show_defcolor (char *);
extern void hide_defcolor (void);
extern void create_rgb (FL_OBJECT *, long);
extern void select_color (FL_OBJECT *, long);
extern void defcolor_load_cb (FL_OBJECT *, long);
extern void defcolor_save_cb (FL_OBJECT *, long);
extern void defcolor_default_cb (FL_OBJECT *, long);
extern void defcolor_apply_cb (FL_OBJECT *, long);
extern void defcolor_ok_cb (FL_OBJECT *, long);
extern void defcolor_cancel_cb (FL_OBJECT *, long);
extern void color_workspace_cb (FL_OBJECT *, long);
extern void gradient_workspace_cb (FL_OBJECT *, long);
extern void detach_menu_cb (FL_OBJECT *, long);
extern void clicktofocus_cb (FL_OBJECT *, long);
extern void autoraise_cb (FL_OBJECT *, long);
extern void opaquemove_cb (FL_OBJECT *, long);
int defcolor_atclose (FL_FORM *, long *);

/**** Forms and Objects ****/

typedef struct
  {
    FL_FORM *defcolor;
    FL_OBJECT *rs;
    FL_OBJECT *gs;
    FL_OBJECT *bs;
    FL_OBJECT *color_button[NB_XFCE_COLORS];
    FL_OBJECT *defcolor_load;
    FL_OBJECT *defcolor_save;
    FL_OBJECT *defcolor_default;
    FL_OBJECT *color_workspace;
    FL_OBJECT *fake_back_gradient;
    FL_OBJECT *fake_back_autoraise;
    FL_OBJECT *gradient_workspace;
    FL_OBJECT *detach_menu;
    FL_OBJECT *clicktofocus;
    FL_OBJECT *autoraise;
    FL_OBJECT *opaquemove;
    FL_OBJECT *defcolor_OK;
    FL_OBJECT *defcolor_apply;
    FL_OBJECT *defcolor_cancel;
    void *vdata;
    long ldata;
  }
FD_def_color;

extern FD_def_color *create_form_def_color (Pixmap, Pixmap);

FD_def_color *fd_def_color;
XFCE_palette *palette;

#endif /*
          FD_defcolor_h_ 
        */
