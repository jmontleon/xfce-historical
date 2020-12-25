
/*

   ORIGINAL FILE NAME : addicon_cb.c

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
#include "addicon.h"
#include "empty.h"
#include "fileutil.h"
#include "pulldown.h"
#include "constant.h"
#include "configfile.h"
#include "extern.h"
#include "my_string.h"
#include "my_goodies.h"
#include "xpmext.h"
#include "resources.h"

#include "defaulticon.h"

extern FD_addiconform *fd_addiconform;

void 
show_addiconform (char *title)
{
  fl_deactivate_all_forms ();
  fl_show_form (fd_addiconform->addiconform, FL_PLACE_CENTER, FL_TRANSIENT, title);
  fl_set_cursor (fd_addiconform->addiconform->window, XFCE_CURS);
}

void 
hide_addiconform (void)
{
  fl_hide_form (fd_addiconform->addiconform);
  fl_activate_all_forms ();
}

void 
remove_icondata (FD_addiconform * f, int no_menu, int pos)
{
  int i, last;
  if (menus[no_menu]->nbitems > 0)
    {
      last = --menus[no_menu]->nbitems;
      for (i = pos; i < menus[no_menu]->nbitems; i++)
	set_item_menu (menus[no_menu], i, menus[no_menu]->menuitem[i + 1]->command,
		       menus[no_menu]->menuitem[i + 1]->pixfile,
		       menus[no_menu]->menuitem[i + 1]->label);
      reorg_pulldown (menus[no_menu]);
      strcpy (menus[no_menu]->menuitem[last]->label, "Empty");
      strcpy (menus[no_menu]->menuitem[last]->pixfile, "Default");
      strcpy (menus[no_menu]->menuitem[last]->command, "");
    }
}

void 
redraw_pixmap (const char *pixfile)
{
  MySetPixmapData (fd_addiconform->previewpix, empty);
  if (existfile (pixfile))
    {
     MySetPixmapFile (fd_addiconform->previewpix, (char *) pixfile);
    }
  else
    {
      MySetPixmapData (fd_addiconform->previewpix, defaulticon);
    }
  fl_redraw_form (fd_addiconform->addiconform);
}

/*
   callbacks for form addiconform 
 */

void 
dummy_callback (FL_OBJECT * ob, long data)
{
  ;
}

void 
br_comline_cb (FL_OBJECT * ob, long data)
{
  const char *fselect;

  fselect = fl_show_fselector (rxfce.selexec, XBINDIR, "*", "");
  if (fselect)
    if (strlen (fselect))
      fl_set_input (fd_addiconform->comline_input, fselect);
}

void 
br_iconpix_cb (FL_OBJECT * ob, long data)
{
  const char *fselect;
  fselect = fl_show_fselector (rxfce.selicon, XFCE_ICONS, "*.xpm", "");
  if (fselect)
    if (strlen (fselect))
      {
	redraw_pixmap (fselect);
	if (existfile (fselect))
	  {
	    fl_set_input (fd_addiconform->pixfile_input, fselect);
	  }
	else
	  {
	    fl_set_input (fd_addiconform->pixfile_input, "Default");
	  }
	fl_redraw_form (fd_addiconform->addiconform);
      }
}

void 
update_pixfile (FL_OBJECT * ob, long data)
{
  const char *s;
  s = fl_get_input (fd_addiconform->pixfile_input);
  redraw_pixmap (s);
}

void 
ai_ok_cb (FL_OBJECT * ob, long data)
{
  char *s1, *s2, *s3;
  int x1, x3;

  s1 = cleanup ((char *) fl_get_input (fd_addiconform->comline_input));
  s2 = cleanup ((char *) fl_get_input (fd_addiconform->pixfile_input));
  s3 = cleanup ((char *) fl_get_input (fd_addiconform->label_input));
  x1 = strlen (s1);
  x3 = strlen (s3);
  if (x1 && x3)
    {
      if (data < 0)
	add_item_menu (menus[-data / NBMAXITEMS], s1, s2, s3);
      else
	set_item_menu (menus[data / NBMAXITEMS], data % NBMAXITEMS, s1, s2, s3);
      hide_addiconform ();
#ifdef ALLWAYS_SAVE
      writetoconfig ();
#endif
    }
  else
    my_alert (rxfce.fillout, 1);
}

void 
ai_cancel_cb (FL_OBJECT * ob, long data)
{
  hide_addiconform ();
}

void 
ai_remove_cb (FL_OBJECT * ob, long data)
{
  if (data >= 0)
    if (my_question (rxfce.delentry, 0) == 1)
      {
	remove_icondata (fd_addiconform, data / NBMAXITEMS,
			 data % NBMAXITEMS);
#ifdef ALLWAYS_SAVE
	writetoconfig ();
#endif
      }
  hide_addiconform ();
}

int 
addicon_atclose (FL_FORM * f, long *d)
{
  hide_addiconform ();
  return (0);
}
