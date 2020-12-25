
/*

   ORIGINAL FILE NAME : info_cb.c

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
#include "constant.h"
#include "info.h"

void 
show_info (char *title)
{
  /*
     fl_deactivate_all_forms(); 
   */
  fl_show_form (fd_info->info, FL_PLACE_CENTER, FL_TRANSIENT, title);
  fl_set_cursor (fd_info->info->window, XFCE_CURS);
}

void 
hide_info (void)
{
  fl_hide_form ((FL_FORM *) fd_info->info);
  /*
     fl_activate_all_forms(); 
   */
}
/*
   callbacks for form info 
 */
void 
OK_info_cb (FL_OBJECT * ob, long data)
{
  hide_info ();
}

int 
info_atclose (FL_FORM * f, long *d)
{
  hide_info ();
  return (0);
}