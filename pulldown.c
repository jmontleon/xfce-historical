/*

   ORIGINAL FILE NAME : pulldown.c

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
#include "pulldown.h"
#include "addiconpix.h"
#include "defaulticon.h"
#include "tiptools.h"
#include "XFCE.h"
#include "extern.h"
#include "constant.h"
#include "fileutil.h"
#include "resources.h"


FD_pullmenu *
create_form_menu (int no_menu, Pixmap icon, Pixmap mask)
{
  FL_OBJECT *obj;
  int i;

  FD_pullmenu *fdui = (FD_pullmenu *) fl_calloc (1, sizeof (*fdui));

  /*
     The size of the form depends on how many items are displayed 
   */
  /*
     Since we create it empty, there is only 1 item displayed  
   */

  fdui->pullmenu = fl_bgn_form (FL_NO_BOX,
				150 + ICONWIDTH + 2 * BORDERWIDTH,
			   ICONHEIGHT * (NBMAXITEMS + 1) + 2 * BORDERWIDTH);
  fdui->botframe = obj = fl_add_box (FL_UP_BOX, 0, 0,
				     150 + ICONWIDTH + 2 * BORDERWIDTH,
				     ICONHEIGHT + 2 * BORDERWIDTH,
				     "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  /*
     The first object is always AddIcon button 
   */
  /*
     Extra bonus : A little frame around this first item 
   */
  fl_add_box (FL_FRAME_BOX, BORDERWIDTH,
	      BORDERWIDTH,
	      150 + ICONWIDTH,
	      ICONHEIGHT, "");
  fdui->addicon = obj = fl_add_pixmap (FL_NORMAL_PIXMAP,
				       BORDERWIDTH + 2,
				       BORDERWIDTH + 2,
				       ICONWIDTH - 2,
				       ICONHEIGHT - 4, "");
  fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
  fl_set_pixmap_data (obj, addiconpix);
  fl_set_object_boxtype (obj, FL_DOWN_BOX);
  fdui->addbutton = obj = fl_add_button (FL_NORMAL_BUTTON,
					 ICONWIDTH + BORDERWIDTH,
					 BORDERWIDTH + 2,
					 150 - 2,
					 ICONHEIGHT - 4,
					 rxfce.pulldownadd);
  fl_set_object_boxtype (obj, FL_FLAT_BOX);
  fl_set_object_color (obj, FL_COL1, XFCE_COL3);
  fl_set_object_lcol (obj, XFCE_COL9);
  fl_set_object_lalign (obj, FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  fl_set_object_lsize (obj, XFCE_MEDIUM_SIZE);
  fl_set_object_lstyle (obj, XFTIT + FL_SHADOW_STYLE);
  fl_set_object_callback (obj, addicon_cb, no_menu);
  set_press_button (obj);
  /*
     Create the "detach" button 
   */
  fdui->detach = obj = fl_add_button (FL_NORMAL_BUTTON,
				      0,
				      2 * BORDERWIDTH + ICONHEIGHT,
				      ICONWIDTH + 150 + 2 * BORDERWIDTH,
				      DETACH_HEIGHT,
				      "- - - - - - - - - - - - - - - -");
  fl_set_object_lcol (obj, XFCE_COL10);
  fl_set_object_lalign (obj, FL_ALIGN_CENTER);
  fl_set_object_lsize (obj, XFCE_TINY_SIZE);
  fl_set_object_lstyle (obj, XFREG + FL_ENGRAVED_STYLE);
  fl_set_object_boxtype (obj, FL_UP_BOX);
  fl_set_object_color (obj, FL_COL1, XFCE_COL1);
  fl_set_object_callback (obj, detach_cb, no_menu);
  /*
     Create all buttons empty, that will be filled later 
   */
  for (i = 0; i < NBMAXITEMS; i++)
    {
      /*
         Memory allocation 
       */
      fdui->menuitem[i] = (ST_item *) malloc (sizeof (ST_item));
      fdui->menuitem[i]->command = (char *) malloc (sizeof (char) * MAXSTRLEN + 1);
      fdui->menuitem[i]->pixfile = (char *) malloc (sizeof (char) * MAXSTRLEN + 1);
      fdui->menuitem[i]->label = (char *) malloc (sizeof (char) * 256);
      /*
         Initialization 
       */
      strcpy (fdui->menuitem[i]->command, "");
      strcpy (fdui->menuitem[i]->pixfile, "Default");
      strcpy (fdui->menuitem[i]->label, "Empty");
      fdui->menuitem[i]->icon = obj = fl_add_pixmap (FL_NORMAL_PIXMAP,
						     BORDERWIDTH,
		     ICONHEIGHT * (i + 1) + 2 * BORDERWIDTH + DETACH_HEIGHT,
						     ICONWIDTH,
						     ICONHEIGHT, "");
      fl_set_object_color (obj, XFCE_COL8, XFCE_COL8);
      fl_set_object_boxtype (obj, FL_DOWN_BOX);
      fdui->menuitem[i]->button = obj = fl_add_button (FL_NORMAL_BUTTON,
						    ICONWIDTH + BORDERWIDTH,
		     ICONHEIGHT * (i + 1) + 2 * BORDERWIDTH + DETACH_HEIGHT,
						       150,
						       ICONHEIGHT, "");
      fl_set_object_boxtype (obj, FL_FLAT_BOX);
      fl_set_object_color (obj, FL_COL1, XFCE_COL3);
      fl_set_object_lcol (obj, XFCE_COL9);
      fl_set_object_lalign (obj, FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
      fl_set_object_lsize (obj, XFCE_MEDIUM_SIZE);
      fl_set_object_lstyle (obj, XFTIT + FL_SHADOW_STYLE);
      fl_set_object_callback (obj, pullmenu_cb, no_menu * NBMAXITEMS + i);
      set_press_button (obj);
      /*
         Tip shows the command line 
       */
      set_tip (obj, fdui->menuitem[i]->command);
    }
  fl_end_form ();
  fdui->nbitems = 0;
  fdui->no_menu = no_menu;
  fdui->ix = (FL_Coord) 0;
  fdui->iy = (FL_Coord) 0;

  fl_set_form_icon (fdui->pullmenu, icon, mask);
  fl_set_form_atclose (fdui->pullmenu, (FL_FORM_ATCLOSE) pulldown_atclose,
		       &fdui->no_menu);

  return (fdui);
}

void 
set_item_menu (FD_pullmenu * menu, int no_item,
	       char *command, char *pixfile, char *label)
{
  if (label)
    {
      strcpy (menu->menuitem[no_item]->label, label);
      strcpy (menu->menuitem[no_item]->button->label, label);
    }
  if (command)
    strcpy (menu->menuitem[no_item]->command, command);
  if ((pixfile) && (existfile (pixfile)))
    {
      strcpy (menu->menuitem[no_item]->pixfile, pixfile);
      fl_set_pixmap_file (menu->menuitem[no_item]->icon,
			  menu->menuitem[no_item]->pixfile);
    }
  else
    {
      strcpy (menu->menuitem[no_item]->pixfile, "Default");
      fl_set_pixmap_data (menu->menuitem[no_item]->icon, defaulticon);
    }
}

void 
add_item_menu (FD_pullmenu * menu, char *command, char *pixfile,
	       char *label)
{
  if (menu->nbitems < NBMAXITEMS)
    {
      set_item_menu (menu, menu->nbitems, command, pixfile, label);
      menu->nbitems++;
      reorg_pulldown (menu);
    }
}

void 
reorg_pulldown (FD_pullmenu * menu)
{
  int i;
  for (i = 0; i < menu->nbitems; i++)
    {
      fl_set_object_position (menu->menuitem[i]->icon,
			      BORDERWIDTH,
			      ICONHEIGHT * (i + 1) + BORDERWIDTH);
      fl_set_object_position (menu->menuitem[i]->button,
			      ICONWIDTH + BORDERWIDTH,
			      ICONHEIGHT * (i + 1) + BORDERWIDTH);
    }
  fl_set_object_position (menu->detach,
			  0,
			ICONHEIGHT * (menu->nbitems + 1) + 2 * BORDERWIDTH);
  for (i = menu->nbitems; i < NBMAXITEMS; i++)
    {
      fl_set_object_position (menu->menuitem[i]->icon,
			      BORDERWIDTH,
		    ICONHEIGHT * (i + 1) + 2 * BORDERWIDTH + DETACH_HEIGHT);
      fl_set_object_position (menu->menuitem[i]->button,
			      ICONWIDTH + BORDERWIDTH,
		    ICONHEIGHT * (i + 1) + 2 * BORDERWIDTH + DETACH_HEIGHT);
    }
  fl_set_object_size (menu->botframe,
		      150 + ICONWIDTH + 2 * BORDERWIDTH,
		      ICONHEIGHT * (menu->nbitems + 1) + 2 * BORDERWIDTH);
}

int 
pulldown_atclose (FL_FORM * f, long *d)
{
  close_popup (NULL, *d);
  return (0);
}

void 
popup_pulldown (FD_pullmenu * menu, FL_Coord px, FL_Coord py, long data)
{
  int n;
  FL_Coord x, y, w, h;
  int place, deco;
  char popup_title[32];

  sprintf (popup_title, "Popup menu %li", data + 1);
  n = ICONHEIGHT * (menu->nbitems + 1);
  w = (FL_Coord) (150 + ICONWIDTH + 2 * BORDERWIDTH);
  h = (FL_Coord) (n + 2 * BORDERWIDTH);
  if (px > 0)
    x = menu->ix = (FL_Coord) (px - (150 + ICONWIDTH) / 2 - BORDERWIDTH);
  else
    x = menu->ix;
  if (py > 0)
    y = menu->iy = (FL_Coord) (py - n - 2 * BORDERWIDTH);
  else
    y = menu->iy;
  if ((!menu->isdetached) && (menu->nbitems > 0) && (DETACH_MENU))
    {
      h += (FL_Coord) DETACH_HEIGHT;
      y -= (FL_Coord) DETACH_HEIGHT;
    }
  menu->pullmenu->w = (FL_Coord) w;
  menu->pullmenu->h = (FL_Coord) h;
  fl_set_form_position (menu->pullmenu, (x > 0) ? x : 0, (y > 0) ? y : 0);
  place = FL_FIX_SIZE | FL_PLACE_GEOMETRY;
  deco = (menu->isdetached) ? FL_TRANSIENT : FL_NOBORDER;
  fl_show_form (menu->pullmenu, place, deco, popup_title);
  fl_set_cursor (menu->pullmenu->window, MENU_CURS);
  fl_do_forms ();
}

void 
free_item_menus (void)
{
  int i, j;

  for (i = 0; i < NBMENUS; i++)
    {
      for (j = 0; j < NBMAXITEMS; j++)
	{
	  free (menus[i]->menuitem[j]->label);
	  free (menus[i]->menuitem[j]->pixfile);
	  free (menus[i]->menuitem[j]->command);
	  free (menus[i]->menuitem[j]);
	}
      free (menus[i]);
    }
}

/*---------------------------------------*/
