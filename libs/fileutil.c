/*  gxfce
 *  Copyright (C) 1999 Olivier Fourdan (fourdan@xfce.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "constant.h"
#include "sendinfo.h"
#include "my_string.h"

int 
existfile (const char *s)
{
  struct stat buf;
  if (!stat (s, &buf))
    {
      return (buf.st_size != 0);
    }
  return (0);
}

void 
my_sleep (int n)
{
  struct timeval value;

  if (n > 0)
    {
      value.tv_usec = n % 1000000;
      value.tv_sec = n / 1000000;
      (void) select (1, 0, 0, 0, &value);
    }
}

void 
exec_comm (char *comm, int wm)
{
  char *command;
  char *toexec;

  command = (char *) malloc ((MAXSTRLEN + 1) * sizeof (char));
  if (comm)
    {
      toexec = cleanup (comm);
      /*
         Still got something to do ? 
       */
      if (strlen (toexec) && my_strncasecmp (toexec, "None", strlen ("None")))
        {
	  if (my_strncasecmp (toexec, "Module ", strlen ("Module ")))
	    {
	      /*
		Do we need to start an xterm to execute the command ?
	       */
	      if (!my_strncasecmp (toexec, "Term ", strlen ("Term ")))
		{
		  strcpy (command, "exec ");
		  strcat (command, TERMINAL);
		  strcat (command, " -e ");
		  strcat (command, toexec + strlen ("Term ")*sizeof(char));
		}
	      else
	        {
		  /*
		     Add 'exec ' if not present, to reduce processes nbr 
		   */
		  if (my_strncasecmp (toexec, "exec ", strlen ("exec ")))
		    {
		      strcpy (command, "exec ");
		      strcat (command, toexec);
		    }
		  else
	            {
		      strcpy (command, toexec);
		    }
		}
	      switch (fork ())
		{
		case 0:
		  execl (DEFAULT_SHELL, DEFAULT_SHELL, "-c", command, NULL);
		  perror ("XFce");
		  break;
		  exit (-1);
		  break;
		case -1:
		  fprintf (stderr, "XFce : cannot execute fork()\n");
		  break;
		default:
		  break;
		}
	    }
	  else
	    {
	      /*
		 This is a module : tell FVWM to execute it 
	       */ 
	      if (wm)
		sendinfo (fd, toexec, 0);
	      else
		fprintf (stderr, "Needs XFwm to execute modules !\n");
	    }
       }
    }
  free (command);
}
