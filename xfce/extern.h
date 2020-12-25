/*

   ORIGINAL FILE NAME : extern.h

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



#ifndef __EXTERNALS_H__
#define __EXTERNALS_H__

#include "resources.h"
#include <X11/Xlib.h>

GC  DrawGC;

short int FVWM;
short int COLORIZE_ROOT;
short int GRADIENT_ROOT;
short int DETACH_MENU;
short int CLICKTOFOCUS;
short int OPAQUEMOVE;
short int AUTORAISE;
res_XFCE rxfce;

#endif
