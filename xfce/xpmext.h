
/*

   ORIGINAL FILE NAME : xpmext.h

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



#ifndef __XPMEXT_H__
#define __XPMEXT_H__

#include "constant.h"

extern int BuildXpmGradient(int, int, int, int, int, int);
extern void ApplyRootColor (FL_COLOR, short int);
int MyCreateXpmFromFile(char *, int, int, Pixmap *, Pixmap *);
int MyCreateXpmFromData(char **, int, int, Pixmap *, Pixmap *);
void MySetPixmapData(FL_OBJECT *, char **);
void MySetPixmapFile(FL_OBJECT *, char *);

#endif
