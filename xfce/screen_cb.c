
/*

   ORIGINAL FILE NAME : screen_cb.c

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
#include "fileutil.h"
#include "configfile.h"
#include "extern.h"
#include "screen.h"
#include "XFCE.h"
#include "constant.h"
#include "my_string.h"
#include "my_goodies.h"
#include "resources.h"

void 
show_screen (int data, char *title)
{
  fl_set_input (fd_def_screen->screen_input, fd_XFCE->screen[data]->label);
  fl_set_object_callback (fd_def_screen->OKbutton, OK_screen_cb, data);
  fl_deactivate_all_forms ();
  fl_show_form (fd_def_screen->def_screen, FL_PLACE_CENTER, FL_TRANSIENT, title);
  fl_set_cursor (fd_def_screen->def_screen->window, XFCE_CURS);
}

void 
hide_screen (void)
{
  fl_activate_all_forms ();
  fl_hide_form (fd_def_screen->def_screen);
}

void 
DM_screen_cb (FL_OBJECT * ob, long data)
{
  ;
}

void 
OK_screen_cb (FL_OBJECT * ob, long data)
{
  char *s;
  int w;

  s = cleanup ((char *) fl_get_input (fd_def_screen->screen_input));
  /*
     Avoid empty names or readconfig will fail on next run 
   */
  if (!strlen (s))
    s = screen_names[data];
  w = fl_get_string_width (ob->lstyle, ob->lsize, s, strlen (s));
  if (w + 10 > fd_XFCE->screen[data]->w)
    my_alert (rxfce.toolong, 1);
  else
    {
      strcpy (fd_XFCE->screen[data]->label, s);
      hide_screen ();
      fl_redraw_object (fd_XFCE->screen[data]);
#ifdef ALLWAYS_SAVE
      writetoconfig ();
#endif
    }
}

void 
CANCEL_screen_cb (FL_OBJECT * ob, long data)
{
  hide_screen ();
}

int 
screen_atclose (FL_FORM * f, long *d)
{
  hide_screen ();
  return (0);
}
