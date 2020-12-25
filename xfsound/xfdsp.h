/*

   ORIGINAL FILE NAME : xfdsp.h

   ********************************************************************
   *                                                                  *
   *           X F C E  - Written by O. Fourdan (c) 1997              *
   *                                                                  *
   *           This software is absolutely free of charge             *
   *                                                                  *
   *           This file is contributed by :                          *
   *                                                                  *
   *            Copyright (c) 1998 Alex Fiori                         *
   *            uground ind. - sbase division                         *
   *           [ http://sbase.uground.org - pmci@uground.org ]        *
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

#ifndef __XFDSP_H__
#define __XFDSP_H__

#define XF_IND		1
#define ST_GET		0
#define DSP_NAME	"/dev/dsp"

typedef int ST_CONFIG[3];

int i_play (char *); /* internal player */
int setcard (void);
int *cardctl (int, ST_CONFIG, int);

#endif
