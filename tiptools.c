
/*

   ORIGINAL FILE NAME : tiptools.c

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


#include <X11/Xlib.h>
#include "forms.h"
#include "constant.h"
#include "tiptools.h"

Window 
compute_wm_window (Window s, int *x, int *y, int *w, int *h)
{
  Window son, par, wj;
  Window *wpj;
  unsigned int ij;
  int bw, dep;

  son = par = s;
  while (par != fl_root)
    {
      son = par;
      XGetGeometry (fl_display, par, &wj, x, y, w, h, &bw, &dep);
      w += bw;
      h += bw;
      XQueryTree (fl_display, par, &wj, &par, &wpj, &ij);
    }
  return (son);
}

void 
ToggleObjectUp (FL_OBJECT * ob)
{
  fl_set_object_color (ob, ob->u_ldata, ob->col2);
  fl_set_object_boxtype (ob, FL_FLAT_BOX);
}

void 
ToggleObjectDn (FL_OBJECT * ob)
{
  if (ob->boxtype != FL_DOWN_BOX)
    {
      ob->u_ldata = ob->col1;
      fl_set_object_color (ob, XFCE_COL3, ob->col2);
      fl_set_object_boxtype (ob, FL_DOWN_BOX);
    }
}

void 
ToggleSelectUp (FL_OBJECT * ob)
{
  fl_set_object_color (ob, XFCE_COL8, XFCE_COL1);
  fl_set_object_boxtype (ob, FL_FLAT_BOX);
}

int 
post_handler (FL_OBJECT * ob, int event,
	      FL_Coord mx, FL_Coord my, int key, void *xev)
{
  static int cnt;
  FL_Coord py;

  if (!ob->u_vdata)
    return (0);

  if (event == FL_ENTER)
    {
      fl_set_object_automatic (ob, 1);
      cnt = 1;
    }
  else if ((event == FL_STEP) && ((cnt++ % 25) == 0))
    {
      fl_set_object_automatic (ob, 0);
      py = ((ob->form->y + ob->y + ob->h + 30 < fl_scrh)
	    ? (ob->form->y + ob->y + 5 + ob->h)
	    : (ob->form->y + ob->y - 30));
      fl_show_oneliner (ob->u_vdata, ob->form->x + ob->x, py);

    }
  else if ((event == FL_LEAVE) || (event == FL_PUSH))
    {
      fl_set_object_automatic (ob, 0);
      fl_hide_oneliner ();
    }
  return (0);
}

void 
set_tip (FL_OBJECT * ob, char *s)
{
  ob->u_vdata = s;
  fl_set_object_posthandler (ob, post_handler);
}

int 
press_handler (FL_OBJECT * ob, int event, FL_Coord mx, FL_Coord my,
	       int key, void *xev)
{
  switch (event)
    {
    case FL_PUSH:
    case FL_MOUSE:
      ToggleObjectDn (ob);
      break;
    case FL_RELEASE:
      ToggleObjectUp (ob);
      break;
    }
  return (0);
}

void 
set_press_button (FL_OBJECT * ob)
{
  fl_set_object_prehandler (ob, press_handler);
}
