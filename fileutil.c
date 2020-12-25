/*

   ORIGINAL FILE NAME : fileutil.c

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



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "constant.h"
#include "extern.h"
#include "sendinfo.h"
#include "my_string.h"

int 
existfile (const char *s)
{
  FILE *f;
  if ((f = fopen (s, "r")))
    {
      fclose (f);
      return (1);
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
exec_comm (char *comm)
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
	if (my_strncasecmp (toexec, "Module ", strlen ("Module ")))
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
	      strcpy (command, toexec);
	    switch (fork ())
	      {
	      case 0:
		execl (DEFAULT_SHELL, DEFAULT_SHELL, "-c", command, NULL);
		perror ("XFCE");
		break;
		exit (-1);
		break;
	      case -1:
		fprintf (stderr, "XFCE : cannot execute fork()\n");
		break;
	      default:
		break;
	      }
	  }
	else
	  /*
	     This is a module : tell FVWM to execute it 
	   */ 
	  if (FVWM)
	    sendinfo (fd, toexec, 0);
	  else
	    fprintf (stderr, "Needs FVWM to execute modules !\n");
    }
  free (command);
}
