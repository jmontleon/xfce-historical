/*

   ORIGINAL FILE NAME : mwmdecor.c

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


#include "mwmdecor.h"

void defdecor (Display * d, Window w, unsigned long decor)
{
  MWMHints decoration;

#if USEMWMHINTS != 0
  decoration.fl    = MWM_HINTS_DECORATIONS;
  decoration.fn    = 0;
  decoration.decor = decor;
  decoration.mode  = 0;
  XChangeProperty(d, w, _XA_MwmAtom, _XA_MwmAtom, 32, PropModeReplace,
    	          (unsigned char *) &decoration, PROP_MOTIF_WM_HINTS_ELEMENTS);
#endif
}
