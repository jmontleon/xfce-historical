
/*

   ORIGINAL FILE NAME : xpmext.c

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
#include "empty.h"
#include "forms.h"
#include "color.h"

void GradientXpmImage(int sizex,int sizey,XpmImage *Imagedest, 
                   int r1, int g1, int b1, int r2, int g2, int b2, int ncolors)
{
  register int i, j, v;
  
  if (ncolors <= 0) 
       ncolors = 2;
  else if (ncolors > 255)
       ncolors = 255;

  Imagedest->width=sizex;
  Imagedest->height=sizey;
  Imagedest->cpp=1;
  Imagedest->ncolors=ncolors;
  Imagedest->colorTable=malloc(ncolors*sizeof(XpmColor));
  Imagedest->data=malloc(sizex*sizey*sizeof(int));

  for (i=0; i<ncolors; i++) {
        (Imagedest->colorTable)[i].string=NULL;
        (Imagedest->colorTable)[i].symbolic=NULL;
        (Imagedest->colorTable)[i].m_color=NULL;
        (Imagedest->colorTable)[i].g4_color=NULL;
        (Imagedest->colorTable)[i].g_color=NULL;
        (Imagedest->colorTable)[i].c_color=malloc(14);
        sprintf((Imagedest->colorTable)[i].c_color,"#%02X00%02X00%02X00",
        						(r1 + (i * (r2 - r1) / ncolors)),
        						(g1 + (i * (g2 - g1) / ncolors)),
        						(b1 + (i * (b2 - b1) / ncolors)));
  }
  for (i = 0; i<sizey; i++)
    for (j = 0; j<sizex; j++) {
       v = ((ncolors * i / sizey) + (i % 2) + (j % 2));
       Imagedest->data[i*sizex+j] = ((v < ncolors) ? v : ncolors);
    }
}

/* The following function is from DFM (c) 1997 by Achim Kaiser */

void XpmShapeToFit(int newsizex,int newsizey,XpmImage *I,XpmImage *Imagedest)
{
  int getr,getg,getb;
  int *shrinknumbers;
  double *shrinkred;
  double *shrinkgreen;
  double *shrinkblue;
  int *myshape;
  XColor *mycolor;
  int i,k,r,g,b,x,y;

  mycolor=malloc(I->ncolors*sizeof(XColor));
  myshape=malloc(I->ncolors*sizeof(int));

  Imagedest->width=newsizex;
  Imagedest->height=newsizey;
  Imagedest->cpp=1;
  Imagedest->ncolors = 65;
  Imagedest->colorTable=malloc(Imagedest->ncolors*sizeof(XpmColor));
  Imagedest->data=malloc(newsizex*newsizey*sizeof(int));

  shrinknumbers=malloc(newsizex*newsizey*sizeof(int));
  shrinkred  =malloc(newsizex*newsizey*sizeof(double));
  shrinkgreen=malloc(newsizex*newsizey*sizeof(double));
  shrinkblue =malloc(newsizex*newsizey*sizeof(double));

  for (r=0;r<4;r++) {
    getr=(r<<2)|r;
    for (g=0;g<4;g++) {
      getg=(g<<2)|g;
      for (b=0;b<4;b++) {
        getb=(b<<2)|b;
        i=r*16+g*4+b;
        (Imagedest->colorTable)[i].string=NULL;
        (Imagedest->colorTable)[i].symbolic=NULL;
        (Imagedest->colorTable)[i].m_color=NULL;
        (Imagedest->colorTable)[i].g4_color=NULL;
        (Imagedest->colorTable)[i].g_color=NULL;
        (Imagedest->colorTable)[i].c_color=malloc(5);
        sprintf((Imagedest->colorTable)[i].c_color,"#%X%X%X", getr,getg,getb);
      }
    }
  }
  (Imagedest->colorTable)[Imagedest->ncolors - 1].string=NULL;
  (Imagedest->colorTable)[Imagedest->ncolors - 1].symbolic=NULL;
  (Imagedest->colorTable)[Imagedest->ncolors - 1].m_color=NULL;
  (Imagedest->colorTable)[Imagedest->ncolors - 1].g4_color=NULL;
  (Imagedest->colorTable)[Imagedest->ncolors - 1].g_color=NULL;
  (Imagedest->colorTable)[Imagedest->ncolors - 1].c_color=malloc(5);
  sprintf((Imagedest->colorTable)[Imagedest->ncolors - 1].c_color,"None");
  for (i=0;i<(I->ncolors);i++) {
    if (XParseColor(fl_display,XDefaultColormap(fl_display,XDefaultScreen(fl_display)),
		    (I->colorTable)[i].c_color,&(mycolor[i]))==0) 
      myshape[i]=True;
    else
      myshape[i]=False;
  }
  for (i=0;i<newsizey*newsizex;i++) {
    shrinknumbers[i]=0;
    shrinkred[i]=0.0; 
    shrinkgreen[i]=0.0; 
    shrinkblue[i]=0.0; 
  }
  k=0;
  for (y=0;y<(I->height);y++) {
    for (x=0;x<(I->width);x++) {
      if (myshape[I->data[k]]==False) {
	i=(int)((newsizex-1)*(double)x/(I->width-1))+newsizex*
	  (int)((newsizey-1)*(double)y/(I->height-1));
        shrinknumbers[i]++;
        shrinkred[i]=shrinkred[i]+mycolor[I->data[k]].red; 
        shrinkgreen[i]=shrinkgreen[i]+mycolor[I->data[k]].green; 
        shrinkblue[i]=shrinkblue[i]+mycolor[I->data[k]].blue; 
      }
      k++;
    }
  }
  for (i=0;i<newsizey*newsizex;i++) {
    if (shrinknumbers[i]==0)
      Imagedest->data[i]=Imagedest->ncolors - 1;
    else
      Imagedest->data[i]=(((int)(shrinkred[i]/shrinknumbers[i])>>14)<<4)|
	                     (((int)(shrinkgreen[i]/shrinknumbers[i])>>14)<<2)|
	                     ( (int)(shrinkblue[i]/shrinknumbers[i])>>14);
  }

  free(mycolor);
  free(myshape);
  free(shrinknumbers);
  free(shrinkred);
  free(shrinkgreen);
  free(shrinkblue);
}

int 
MyCreatePixmapFromXpmImage(XpmImage *Img, Pixmap *pixmap, Pixmap *mask)
{
  XpmAttributes attributes;
  int status;

  attributes.valuemask = XpmReturnPixels | XpmRGBCloseness;
  attributes.red_closeness = 0;
  attributes.green_closeness = 0;
  attributes.blue_closeness = 0;


  status = XpmCreatePixmapFromXpmImage(fl_display,
                                   fl_root, 
                                   Img, 
                                   pixmap, 
                                   mask, 
                                   &attributes);
  return (status); 
}

int 
MyCreateXpmFromFile(char *filename, int w, int h, Pixmap *pixmap, Pixmap *mask)
{
  int result;
  XpmImage I;
  XpmImage I2;

  result = XpmReadFileToXpmImage(filename, &I, NULL);
  if (result == 0)
  {
     if (((w > 0) && (h > 0) && ((I.width > w) || (I.height > h))) || fl_get_visual_depth () <= 8 )
     {
       XpmShapeToFit( ((I.width > w)  ? w : I.width),
                      ((I.height > h) ? h : I.height), &I, &I2);
       result = MyCreatePixmapFromXpmImage(&I2, pixmap, mask);
       XpmFreeXpmImage(&I2);
     }
     else
     {
       MyCreatePixmapFromXpmImage (&I, pixmap, mask);
     }
    XpmFreeXpmImage(&I);
  }
  return (result);
}

int 
MyCreateXpmFromData(char **data, int w, int h, Pixmap *pixmap, Pixmap *mask)
{
  int result;
  XpmImage I;
  XpmImage I2;

  result = XpmCreateXpmImageFromData (data, &I, NULL);
  if (result == 0)
  {
     if ((w > 0) && (h > 0) && ((I.width > w) || (I.height > h)))
     {
       XpmShapeToFit( ((I.width > w)  ? w : I.width),
                      ((I.height > h) ? h : I.height),&I,&I2);
       result = MyCreatePixmapFromXpmImage(&I2, pixmap, mask);
       XpmFreeXpmImage(&I2);
     }
     else
     {
       MyCreatePixmapFromXpmImage(&I, pixmap, mask);
     }
     XpmFreeXpmImage(&I);
  }
  return (result);
}

void
MySetPixmapData(FL_OBJECT * obj, char **data)
{
  Pixmap p = 0, m = 0;

  /* A dirty trick otherwise XForms generates an Xlib error */
  fl_free_pixmap_pixmap(obj);
  fl_set_pixmap_data (obj, empty);
  if ((data) && (MyCreateXpmFromData (data, obj->w - 2, obj->h - 2, &p, &m) == 0))
  {
    fl_free_pixmap_pixmap(obj);
    fl_set_pixmap_pixmap (obj, p, m);
  }
}

void
MySetPixmapFile(FL_OBJECT * obj, char *filename)
{
  Pixmap p = 0, m = 0;

  /* A dirty trick otherwise XForms generates an Xlib error */
  fl_free_pixmap_pixmap(obj);
  fl_set_pixmap_data (obj, empty);
  if ((filename) && (MyCreateXpmFromFile (filename, obj->w - 2, obj->h - 2, &p, &m) == 0))
  {
    fl_free_pixmap_pixmap(obj);
    fl_set_pixmap_pixmap (obj, p, m);
  }
}

int
BuildXpmGradient(int r1, int g1, int b1, int r2, int g2, int b2)
{
   XpmImage I;
   Pixmap rootXpm = 0;
   Pixmap mask = 0;
   int status;
   int gradient_steps;

   if (fl_get_visual_depth() > 16) 
     gradient_steps = 128;
   else
     gradient_steps = 64;

   GradientXpmImage(10, fl_scrw, &I, r1, g1, b1, r2, g2, b2, gradient_steps);
   status = MyCreatePixmapFromXpmImage(&I, &rootXpm, &mask);
   XpmFreeXpmImage(&I);
   if (status == XpmSuccess) {
      XSetWindowBackgroundPixmap (fl_display, fl_root, rootXpm);
      XClearWindow (fl_display, fl_root);
      XFlush (fl_display);
      if (rootXpm) XFreePixmap (fl_display, rootXpm);
      if (mask) XFreePixmap (fl_display, mask);
   }
   return (status);
}

void
ApplyRootColor (FL_COLOR backcolor, short int gradient)
{
   int r, g, b;

   fl_getmcolor(backcolor, &r, &g, &b);

   if ((fl_get_visual_depth() >= 16) && (gradient))
       BuildXpmGradient ( shift (r, 135), shift (g, 135), shift (b, 135),
                          shift (r, 55),  shift (g, 55),  shift (b, 55) );
   else
   {
      XSetWindowBackground (fl_display, fl_root, fl_get_pixel (backcolor));
      XClearWindow (fl_display, fl_root);
      XFlush (fl_display);
   }
}

