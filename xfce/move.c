
/*

   ORIGINAL FILE NAME : move.c

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
#include "move.h"

int PRESSED;
FL_Coord dx, dy;
FL_Coord ox, oy;

int 
move_handler (FL_OBJECT * ob, int event, FL_Coord mx, FL_Coord my,
	       int key, void *xev)
{
  int x, y;
  unsigned int keymask;

  switch (event)
    {
    case FL_ENTER:
      break;
    case FL_LEAVE:
      break;
    case FL_MOUSE:
      if (PRESSED) {
        fl_get_mouse(&x, &y, &keymask);
        if ((ox != x) || (oy != y)) {
    	  XMoveWindow (fl_display, ob->form->window,  x + dx, y + dy);
          ox = x;
          oy = y;
        }
      }
      break;
    case FL_PUSH:
      if (!PRESSED) {
        PRESSED = 1;
        fl_get_mouse(&x, &y, &keymask);
        dx = ob->form->x - x;
        dy = ob->form->y - y;
        ox = x;
        oy = y;
      }
      break;
    case FL_RELEASE:
      if (PRESSED) {
        PRESSED = 0;
        fl_get_mouse(&x, &y, &keymask);
        XMoveWindow (fl_display, ob->form->window,  x + dx, y + dy);
      }
      break;
    }
  return (0);
}

void
set_move_button (FL_OBJECT * ob)
{
  PRESSED = 0;
  fl_set_object_posthandler (ob, move_handler);
}
