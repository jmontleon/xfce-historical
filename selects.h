/*

   ORIGINAL FILE NAME : selects.h

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



#ifndef __SELECTS_H__
#define __SELECTS_H__

#include "extern.h"
#include "constant.h"
#include <string.h>

typedef struct
  {
    char *command;		/*
				   command line to execute 
				 */
    int icon_nbr;		/*
				   icon nbr for this item  
				 */
  }
ST_select;

ST_select selects[NBSELECTS + 1];	/*
					   One more for the lock 
					 */

void alloc_selects (void);
void free_selects (void);

int load_icon_str (char *);
char *save_icon_str (void);
void setup_icon (void);
void default_icon_str (void);
int get_icon_nbr (int);
void set_icon_nbr (int, int);
void init_choice_str (FL_OBJECT *);
void set_choice_value (FL_OBJECT *, int);

#endif
