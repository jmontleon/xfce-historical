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
#include "constant.h"
#include "extern.h"

static short int PRESSED;
static int xgrabcount = 0;

void
MyXGrabServer (Display * disp)
{
  if (xgrabcount == 0)
    {
      XGrabServer (disp);
    }
  ++xgrabcount;
}

void
MyXUngrabServer (Display * disp)
{
  if (--xgrabcount < 0)		/* should never happen */
    {
      xgrabcount = 0;
    }
  if (xgrabcount == 0)
    {
      XUngrabServer (disp);
    }
}

void
CreateDrawGC (void)
{
  XGCValues gcv;
  unsigned long gcm;

  gcm = GCFunction | GCLineWidth | GCForeground | GCSubwindowMode;
  gcv.function = GXxor;
  gcv.line_width = 4;
  gcv.foreground = (BlackPixel (fl_display, fl_screen) ^ WhitePixel (fl_display, fl_screen));
  gcv.subwindow_mode = IncludeInferiors;
  DrawGC = XCreateGC (fl_display, fl_root, gcm, &gcv);
}

void
MoveOutline (Window root, int x, int y, int width, int height)
{
  static int lastx = 0;
  static int lasty = 0;
  static int lastWidth = 0;
  static int lastHeight = 0;

  if (x == lastx && y == lasty && width == lastWidth && height == lastHeight)
    return;

  /* undraw the old one, if any */
  if (lastWidth || lastHeight)
    {
      XDrawRectangle (fl_display, fl_root, DrawGC, lastx, lasty, lastWidth, lastHeight);
    }

  lastx = x;
  lasty = y;
  lastWidth = width;
  lastHeight = height;

  /* draw the new one, if any */
  if (lastWidth || lastHeight)
    {
      XDrawRectangle (fl_display, fl_root, DrawGC, lastx, lasty, lastWidth, lastHeight);
    }
}

void
move_loop (FL_OBJECT * ob, FL_Coord mx, FL_Coord my)
{
  static FL_Coord dx, dy;
  static FL_Coord ox, oy;
  int x, y;
  unsigned int keymask;
  register Bool finished = False;
  XEvent event;

  XSync (fl_display, 0);
  fl_deactivate_form (ob->form);

  dx = -mx;
  dy = -my;
  x = ox = ob->form->x + mx;
  y = oy = ob->form->y + my;

  if(!OPAQUEMOVE) {
    MyXGrabServer(fl_display);
    MoveOutline(fl_root, x + dx, y + dy, ob->form->w, ob->form->h);
  }

  while (!finished)
    {
      XMaskEvent (fl_display, ButtonPressMask | ButtonReleaseMask |
	               PointerMotionMask | KeyPressMask, &event);
      switch (event.type)
      {
        case ButtonPress:
        case ButtonRelease:
	case KeyPress:
          finished = True;
          break;
 	case MotionNotify:
          fl_get_mouse(&x, &y, &keymask);
          if ((ox != x) || (oy != y)) {
            if(!OPAQUEMOVE)
              MoveOutline(fl_root, x + dx, y + dy, ob->form->w, ob->form->h);
            else
    	      XMoveWindow (fl_display, ob->form->window,  x + dx, y + dy);
            ox = x;
            oy = y;
          }
        default:
 	  break;
      }
    }
  fl_get_mouse(&x, &y, &keymask);
  if(!OPAQUEMOVE)
    MoveOutline(fl_root, 0, 0, 0, 0);
  fl_set_form_position (ob->form,  ((x + dx) >= 0 ? (x + dx) : 0),
                                   ((y + dy) >= 0 ? (y + dy) : 0));
  XSync(fl_display, True);
  if(!OPAQUEMOVE) {
     MyXUngrabServer(fl_display);
  }
  fl_activate_form (ob->form);
}

int 
move_handler (FL_OBJECT * ob, int event, FL_Coord mx, FL_Coord my,
	       int key, void *xev)
{
  switch (event)
    {
    case FL_PUSH:
      move_loop (ob, mx, my);
      break;
    default:
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
