/*

   ORIGINAL FILE NAME : sendinfo.c

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

   The following code is mostly taken from Rob NATION's code of FVWM;
   So here comes his copyright notice :

   ***************************************************************************
   * This module, and the entire ModuleDebugger program, and the concept for *
   * interfacing this module to the Window Manager, are all original work    *
   * by Robert Nation                                                        *
   *                                                                         *
   * Copyright 1994, Robert Nation. No guarantees or warantees or anything   *
   * are provided or implied in any way whatsoever. Use this program at your *
   * own risk. Permission to use this program for any purpose is given,      *
   * as long as the copyright is kept intact.                                *
   ***************************************************************************

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "sendinfo.h"

void 
sendinfo (int *fd, char *message, unsigned long window)
{
  int w;

  if (message != NULL)
    {
      write (fd[0], &window, sizeof (unsigned long));
      w = strlen (message);
      write (fd[0], &w, sizeof (int));
      write (fd[0], message, w);
      w = 1;
      write (fd[0], &w, sizeof (int));
    }
}

int 
readpacket (int fd, unsigned long *header, unsigned long **body)
{
  int count, total, count2, body_length;
  char *cbody;
  extern void DeadPipe (int);

  if ((count = read (fd, header, HEADER_SIZE * sizeof (unsigned long))) > 0)
    {
      if (header[0] == START_FLAG)
	{
	  body_length = header[2] - HEADER_SIZE;
	  *body = (unsigned long *) malloc (body_length * sizeof (unsigned long));
	  cbody = (char *) (*body);
	  total = 0;
	  while (total < body_length * sizeof (unsigned long))
	    {
	      if ((count2 = read (fd, &cbody[total],
			 body_length * sizeof (unsigned long) - total)) > 0)
		{
		  total += count2;
		}
	    }
	}
      else
	count = 0;
    }
  return (count);
}
