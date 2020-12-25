

/****************************************************************************
 * This module is all new
 * by Rob Nation 
 * Copyright 1993 Robert Nation. No restrictions are placed on this code,
 * as long as the copyright notice is preserved
 ****************************************************************************/


#include "configure.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>

#include <X11/Xproto.h>
#include <X11/Xatom.h>


#include "xfwm.h"
#include "menus.h"
#include "misc.h"
#include "parse.h"
#include "screen.h"
#include "version.h"

char *white = "white";
char *black = "black";

extern char *Hiback;
extern char *Hifore;

#define shift(col, val) (col * val) / 100

/****************************************************************************
 *
 * This routine computes the shadow color from the background color
 *
 ****************************************************************************/
Pixel
GetShadow (Pixel background)
{
  XColor bg_color;
  XWindowAttributes attributes;

  XGetWindowAttributes (dpy, Scr.Root, &attributes);

  bg_color.pixel = background;
  XQueryColor (dpy, attributes.colormap, &bg_color);

  bg_color.red = shift (bg_color.red, 45);
  bg_color.green = shift (bg_color.green, 45);
  bg_color.blue = shift (bg_color.blue, 45);

  if (!XAllocColor (dpy, attributes.colormap, &bg_color))
    {
      nocolor ("alloc shadow", "");
      bg_color.pixel = background;
    }

  return bg_color.pixel;
}

/****************************************************************************
 *
 * This routine computes the hilight color from the background color
 *
 ****************************************************************************/
Pixel
GetHilite (Pixel background)
{
  XColor bg_color, white_p;
  XWindowAttributes attributes;
  unsigned int r, g, b;

  XGetWindowAttributes (dpy, Scr.Root, &attributes);

  bg_color.pixel = background;
  XQueryColor (dpy, attributes.colormap, &bg_color);
  white_p.pixel = GetColor (white);
  XQueryColor (dpy, attributes.colormap, &white_p);

  r = shift (bg_color.red, 145);
  g = shift (bg_color.green, 145);
  b = shift (bg_color.blue, 145);

  bg_color.red = ((r <= white_p.red) ? r : white_p.red);
  bg_color.green = ((g <= white_p.green) ? g : white_p.green);
  bg_color.blue = ((b <= white_p.blue) ? b : white_p.blue);

  if (!XAllocColor (dpy, attributes.colormap, &bg_color))
    {
      nocolor ("alloc hilight", "");
      bg_color.pixel = background;
    }
  return bg_color.pixel;
}

/***********************************************************************
 *
 *  Procedure:
 *	CreateGCs - open fonts and create all the needed GC's.  I only
 *		    want to do this once, hence the first_time flag.
 *
 ***********************************************************************/
void
CreateGCs (void)
{
  XGCValues gcv;
  unsigned long gcm;

  /* create scratch GC's */
  gcm = GCFunction | GCPlaneMask | GCGraphicsExposures | GCLineWidth;
  gcv.line_width = 0;
  gcv.function = GXcopy;
  gcv.plane_mask = AllPlanes;
  gcv.graphics_exposures = False;

  Scr.ScratchGC1 = XCreateGC (dpy, Scr.Root, gcm, &gcv);
  Scr.ScratchGC2 = XCreateGC (dpy, Scr.Root, gcm, &gcv);
  Scr.ScratchGC3 = XCreateGC (dpy, Scr.Root, gcm, &gcv);

  Scr.TransMaskGC = XCreateGC (dpy, Scr.Root, gcm, &gcv);
}

XColor
GetXColor (char *name)
{
  XColor color;

  color.pixel = 0;
  if (!XParseColor (dpy, Scr.XfwmRoot.attr.colormap, name, &color))
    {
      nocolor ("parse", name);
    }
  else if (!XAllocColor (dpy, Scr.XfwmRoot.attr.colormap, &color))
    {
      nocolor ("alloc", name);
    }
  return color;
}

/****************************************************************************
 * 
 * Loads a single color
 *
 ****************************************************************************/
Pixel
GetColor (char *name)
{
  return GetXColor (name).pixel;
}

/****************************************************************************
 * 
 * Allocates a nonlinear color gradient (veliaa@rpi.edu)
 *
 ****************************************************************************/
Pixel *
AllocNonlinearGradient (char *s_colors[], int clen[],
			int nsegs, int npixels)
{
  Pixel *pixels = (Pixel *) safemalloc (sizeof (Pixel) * npixels);
  int i = 0, curpixel = 0, perc = 0;
  if (nsegs < 1)
    {
      xfwm_msg (ERR, "AllocNonlinearGradient",
		"must specify at least one segment");
      free (pixels);
      return NULL;
    }
  for (; i < npixels; i++)
    pixels[i] = 0;

  for (i = 0; (i < nsegs) && (curpixel < npixels) && (perc <= 100); ++i)
    {
      Pixel *p;
      int j = 0, n = clen[i] * npixels / 100;
      p = AllocLinearGradient (s_colors[i], s_colors[i + 1], n);
      if (!p)
	{
	  xfwm_msg (ERR, "AllocNonlinearGradient",
		    "couldn't allocate gradient");
	  free (pixels);
	  return NULL;
	}
      for (; j < n; ++j)
	pixels[curpixel + j] = p[j];
      perc += clen[i];
      curpixel += n;
      free (p);
    }
  for (i = curpixel; i < npixels; ++i)
    pixels[i] = pixels[i - 1];
  return pixels;
}

/****************************************************************************
 * 
 * Allocates a linear color gradient (veliaa@rpi.edu)
 *
 ****************************************************************************/
Pixel *
AllocLinearGradient (char *s_from, char *s_to, int npixels)
{
  Pixel *pixels;
  XColor from, to, c;
  int r, dr, g, dg, b, db;
  int i = 0, got_all = 1;

  if (npixels < 1)
    return NULL;
  if (!s_from || !XParseColor (dpy, Scr.XfwmRoot.attr.colormap, s_from, &from))
    {
      nocolor ("parse", s_from);
      return NULL;
    }
  if (!s_to || !XParseColor (dpy, Scr.XfwmRoot.attr.colormap, s_to, &to))
    {
      nocolor ("parse", s_to);
      return NULL;
    }
  c = from;
  r = from.red;
  dr = (to.red - from.red) / npixels;
  g = from.green;
  dg = (to.green - from.green) / npixels;
  b = from.blue;
  db = (to.blue - from.blue) / npixels;
  pixels = (Pixel *) safemalloc (sizeof (Pixel) * npixels);
  c.flags = DoRed | DoGreen | DoBlue;
  for (; i < npixels; ++i)
    {
      if (!XAllocColor (dpy, Scr.XfwmRoot.attr.colormap, &c))
	got_all = 0;
      pixels[i] = c.pixel;
      c.red = (unsigned short) (r += dr);
      c.green = (unsigned short) (g += dg);
      c.blue = (unsigned short) (b += db);
    }
  if (!got_all)
    {
      char s[256];
      sprintf (s, "color gradient %s to %s", s_from, s_to);
      nocolor ("alloc", s);
    }
  return pixels;
}

void
nocolor (char *note, char *name)
{
  xfwm_msg (ERR, "nocolor", "can't %s color %s", note, name);
}
