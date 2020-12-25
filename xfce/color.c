
/*

   ORIGINAL FILE NAME : color.c

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
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include "forms.h"
#include "color.h"
#include "constant.h"
#include "my_string.h"
#include "sendinfo.h"

#define top		125	/*
				   Percentage for top border color    
				 */
#define lft		125	/*
				   Percentage for left border color   
				 */
#define rgt		 60	/*
				   Percentage for right border color  
				 */
#define bot		 60	/*
				   Percentage for bottom border color 
				 */
#define bright  	 85	/*
				   Percentage of brightness           
				 */
#define dark    	 25	/*
				   Percentage of darkness             
				 */
#define fadeblack 	 10
#define fadewhite 	 95

char *rccolor = "/.xfcolors";

int 
shift (int col, int val)
{
  return (((((col * val) / 100) < 255) ? ((col * val) / 100) : 255));
}

int 
brightness (int r, int g, int b)
{
  return ((100 * (r + g + b) / 765));
}

int 
brightness_pal (const XFCE_palette * p, int index)
{
  if ((p) && (index >= 0) && (index < NB_XFCE_COLORS))
    return (brightness ( (short int) p->r[index],
                         (short int) p->g[index],
                         (short int) p->b[index]));
  else
    return (0);
}

char *
color_to_hex(char *s, const XFCE_palette * p, int index)
{
  char *t;
  t=(char *) malloc (3 * sizeof (char));

  if ((p) && (s) && (index >= 0) && (index < NB_XFCE_COLORS))
  {
    sprintf(s,"#%02X%02X%02X",(short int) p->r[index],(short int) p->g[index],(short int) p->b[index]);
    return (s);
  }
  return (NULL);
}

void 
apply_wm_colors(int *fd, const XFCE_palette *p)
{
  char *s, *t, *u, *v;
  int howbright, howdark;


  if (p)
  {
     s=(char *) malloc (10  * sizeof (char));
     t=(char *) malloc (10  * sizeof (char));
     u=(char *) malloc (255 * sizeof (char));
     v=(char *) malloc (10  * sizeof (char));
    
     howbright = brightness_pal (p, 6);
     howdark   = brightness_pal (p, 3);
     color_to_hex(s, p, 7);
     sprintf(u, "ActiveColor %s %s\n", (howbright > bright) ? "black" : "white", s);
     sendinfo(fd, u, 0);
     sprintf(u, "InactiveColor %s %s\n", (howdark < dark) ? "white" : "black", s);
     sendinfo(fd, u, 0);

     howbright = brightness_pal (p, 2);
     howdark   = brightness_pal (p, 7);
     color_to_hex(t, p, 2);
     sprintf(u, "MenuColor %s %s %s %s\n", (howdark < dark)     ? "white" : "black", s,
                                           (howbright > bright) ? "black" : "white", t);
     sendinfo(fd, u, 0);

     color_to_hex(t, p, 6);
     color_to_hex(v, p, 5);
     if (fl_get_visual_depth () > 8)
       sprintf(u, "TitleStyle Active Gradient %s %s\n", t, v);
     else
       sprintf(u, "TitleStyle Active Solid %s\n", t);
     sendinfo(fd, u, 0);

     color_to_hex(t, p, 3);
     if (fl_get_visual_depth () > 8)
       sprintf(u, "TitleStyle Inactive Gradient %s %s\n", t, s);
     else
       sprintf(u, "TitleStyle Inactive Solid %s\n", t);
     sendinfo(fd, u, 0);

     color_to_hex(t, p, 0);
     sprintf(u, "CursorColor %s white\n", t);
     sendinfo(fd, u, 0);

     sendinfo(fd, "Refresh 0", 0);
     free(s);
     free(t);
     free(u);
     free(v);
   }
}

XFCE_palette *
newpal (void)
{
  return (((XFCE_palette *) malloc (sizeof (XFCE_palette))));
}

FL_COLOR
palcolor(XFCE_palette * p, short int index)
{
  if ((p) && (index >= 0) && (index < NB_XFCE_COLORS))
    return ((FL_COLOR) p->cm[index]);
  return (0);
}

void 
freepal (XFCE_palette * p)
{
  free (p);
}

XFCE_palette *
copypal (XFCE_palette * d, const XFCE_palette * s)
{
  int i;

  if (d && s)
    {
      for (i = 0; i < NB_XFCE_COLORS; i++)
	{
	  d->cm[i] = s->cm[i];
	  d->r[i] = s->r[i];
	  d->g[i] = s->g[i];
	  d->b[i] = s->b[i];
	}
      return (d);
    }
  return (NULL);
}

void 
initpal (XFCE_palette * p)
{
  p->cm[0] = XFCE_COL1;
  p->cm[1] = XFCE_COL2;
  p->cm[2] = XFCE_COL3;
  p->cm[3] = XFCE_COL4;
  p->cm[4] = XFCE_COL5;
  p->cm[5] = XFCE_COL6;
  p->cm[6] = XFCE_COL7;
  p->cm[7] = XFCE_COL8;
}

void 
applypal (XFCE_palette * p)
{
  int howbright;

  fl_mapcolor (p->cm[0], p->r[0], p->g[0], p->b[0]);
  fl_mapcolor (p->cm[1], p->r[1], p->g[1], p->b[1]);
  fl_mapcolor (p->cm[2], p->r[2], p->g[2], p->b[2]);
  fl_mapcolor (p->cm[3], p->r[3], p->g[3], p->b[3]);
  fl_mapcolor (p->cm[4], p->r[4], p->g[4], p->b[4]);
  fl_mapcolor (p->cm[5], p->r[5], p->g[5], p->b[5]);
  fl_mapcolor (p->cm[6], p->r[6], p->g[6], p->b[6]);
  fl_mapcolor (p->cm[7], p->r[7], p->g[7], p->b[7]);
  fl_mapcolor (FL_MCOL, p->r[2], p->g[2], p->b[2]);
  fl_mapcolor (FL_COL1, p->r[7], p->g[7], p->b[7]);
  fl_mapcolor (XFCE_COLB, p->r[7], p->g[7], p->b[7]);
  /*
     How do we handle borders colors, and labels ? 
   */
  howbright = brightness (p->r[7], p->g[7], p->b[7]);
  if (howbright < fadeblack)
    {
      fl_mapcolor (XFCE_COL9, 255, 255, 255);
      fl_mapcolor (XFCE_COL10, 255, 255, 255);
      fl_mapcolor (FL_BLACK, 255, 255, 255);
      fl_mapcolor (FL_WHITE, 255, 255, 255);
      fl_mapcolor (FL_TOP_BCOL, 255, 255, 255);
      fl_mapcolor (FL_LEFT_BCOL, 255, 255, 255);
      fl_mapcolor (FL_RIGHT_BCOL, 255, 255, 255);
      fl_mapcolor (FL_BOTTOM_BCOL, 255, 255, 255);
    }
  else if (howbright > fadewhite)
    {
      fl_mapcolor (XFCE_COL9, 0, 0, 0);
      fl_mapcolor (XFCE_COL10, 0, 0, 0);
      fl_mapcolor (FL_BLACK, 0, 0, 0);
      fl_mapcolor (FL_WHITE, 0, 0, 0);
      fl_mapcolor (FL_TOP_BCOL, 0, 0, 0);
      fl_mapcolor (FL_LEFT_BCOL, 0, 0, 0);
      fl_mapcolor (FL_RIGHT_BCOL, 0, 0, 0);
      fl_mapcolor (FL_BOTTOM_BCOL, 0, 0, 0);
    }
  else
    {
      if (howbright > bright)
	fl_mapcolor (XFCE_COL9, 0, 0, 0);
      else
	fl_mapcolor (XFCE_COL9, 255, 255, 255);
      if (howbright < dark)
	fl_mapcolor (XFCE_COL10, 255, 255, 255);
      else
	fl_mapcolor (XFCE_COL10, 0, 0, 0);
      fl_mapcolor (FL_BLACK, 0, 0, 0);
      fl_mapcolor (FL_WHITE, 255, 255, 255);
      fl_mapcolor (FL_TOP_BCOL, shift (p->r[7], top),
		   shift (p->g[7], top),
		   shift (p->b[7], top));
      fl_mapcolor (FL_LEFT_BCOL, shift (p->r[7], lft),
		   shift (p->g[7], lft),
		   shift (p->b[7], lft));
      fl_mapcolor (FL_RIGHT_BCOL, shift (p->r[7], rgt),
		   shift (p->g[7], rgt),
		   shift (p->b[7], rgt));
      fl_mapcolor (FL_BOTTOM_BCOL, shift (p->r[7], bot),
		   shift (p->g[7], bot),
		   shift (p->b[7], bot));
    }
}

void 
defpal (XFCE_palette * p)
{
  initpal (p);
  p->r[0] = 255;
  p->g[0] = 0;
  p->b[0] = 0;
  p->r[1] = 164;
  p->g[1] = 164;
  p->b[1] = 168;
  p->r[2] = 128;
  p->g[2] = 145;
  p->b[2] = 164;
  p->r[3] = 120;
  p->g[3] = 125;
  p->b[3] = 135;
  p->r[4] = 80;
  p->g[4] = 135;
  p->b[4] = 180;
  p->r[5] = 200;
  p->g[5] = 155;
  p->b[5] = 170;
  p->r[6] = 190;
  p->g[6] = 85;
  p->b[6] = 100;
  p->r[7] = 184;
  p->g[7] = 184;
  p->b[7] = 192;
}

int 
savenamepal (XFCE_palette * p, const char *name)
{
  FILE *f;
  int i;

  if ((f = fopen (name, "w")))
    {
      for (i = 0; i < NB_XFCE_COLORS; i++)
	fprintf (f, "%i %i %i\n", p->r[i], p->g[i], p->b[i]);
      fclose (f);
    }
  return ((f != NULL));
}

int 
loadnamepal (XFCE_palette * p, const char *name)
{
  char *lineread, *a;
  FILE *f;
  int i, err = 0;
  XFCE_palette *backup_pal;

  backup_pal = newpal ();
  if (!copypal (backup_pal, p))
    defpal (backup_pal);

  lineread = (char *) malloc ((80) * sizeof (char));

  if ((f = fopen (name, "r")))
    {
      for (i = 0; i < NB_XFCE_COLORS; i++)
	{
	  fgets (lineread, 79, f);
	  if (strlen (lineread))
	    {
	      lineread[strlen (lineread) - 1] = 0;
	      if ((a = strtok (lineread, " ")))
		p->r[i] = atoi (a);
	      else
		err = 1;
	      if ((a = strtok (NULL, " ")))
		p->g[i] = atoi (a);
	      else
		err = 1;
	      if ((a = strtok (NULL, " ")))
		p->b[i] = atoi (a);
	      else
		err = 1;
	    }
	}
      fclose (f);
    }
  free (lineread);
  if (err)
    copypal (p, backup_pal);
  freepal (backup_pal);
  return ((!err && (f != NULL)));
}

int 
savepal (XFCE_palette * p)
{
  char *homedir;
  int x;

  homedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, rccolor);
  x = savenamepal (p, homedir);
  free (homedir);
  return (x);
}

int 
loadpal (XFCE_palette * p)
{
  char *homedir;
  int x;

  homedir = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  strcpy (homedir, (char *) getenv ("HOME"));
  strcat (homedir, rccolor);
  x = loadnamepal (p, homedir);
  free (homedir);
  return (x);
}

