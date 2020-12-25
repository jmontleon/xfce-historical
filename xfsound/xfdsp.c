/*

   ORIGINAL FILE NAME : xfdsp.c

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


/*	Hi Oliver! :)
	I'm using XFCE, really rules.
	Well, this is my first contrib for you, and I
	hope I could help you with anything!

	uground ind. - sbase division
	Copyright (c) 1998 Alex Fiori
	[ http://sbase.uground.org - pmci@uground.org ]

	xfdsp internal sound driver for XFCE
	[ http://www.linux-kheops.com/pub/xfce ]

	NOTE: I hope xfsound doesn't need to use
	external players (like sox) anymore. 

	Compile: gcc -Wall -c xfdsp.c
	You can use -DDEBUG to see the errors
*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#if defined(linux)
  #include <sys/soundcard.h> /* guess :) */
#endif

#include "xfdsp.h"

#ifdef DEBUG /* use perror() with -DDEBUG */
#include <errno.h>
#endif


int masterfd;

#if defined(linux)
int i_play (char *soundfile)
{
	char *buffer[256];
	int fp, next, len;
	ST_CONFIG curr;

	setcard ();
	cardctl (masterfd, curr, ST_GET);

	if ((fp = open (soundfile, O_RDONLY, 0)) == -1)
	{
		#ifdef DEBUG
		perror ("open");
		#endif
                return (-1);
	}

	next = sizeof (buffer);

	while ((next > 0) && (len = read (fp, buffer, next)) > 0)
	{
		if (write (masterfd, buffer, len) == -1)
		{
			#ifdef DEBUG
			perror ("write");
			#endif
                        return (-1);
		}

		if (len < next) next = 0;
	}

	close (masterfd); /* done */
	close (fp);
	return 0;
}

int setcard (void)
{
	if ((masterfd = open (DSP_NAME, O_WRONLY, 0)) == -1)
	{
		#ifdef DEBUG
		perror ("open");
		#endif
                return (-1);
	}

	return 0;
}

int *cardctl (int fp, ST_CONFIG parm, int st_flag)
{
	static ST_CONFIG temp;
	int error;

	if (st_flag)
	{
		if (ioctl (fp, SOUND_PCM_WRITE_BITS, &parm[0]) == -1)
		{
			#ifdef DEBUG
			perror ("ioctl");
			#endif
		}
		if (ioctl (fp, SOUND_PCM_WRITE_CHANNELS, &parm[1]) == -1)
		{
			#ifdef DEBUG
			perror ("ioctl");
			#endif
		}
		if (ioctl (fp, SOUND_PCM_WRITE_RATE, &parm[2]) == -1)
		{
			#ifdef DEBUG
			perror ("ioctl");
			#endif
		}
	}
	else
	{
		error = ioctl (fp, SOUND_PCM_READ_BITS, &parm[0]);
		ioctl(fp, SOUND_PCM_READ_CHANNELS, &parm[1]);
		ioctl(fp, SOUND_PCM_READ_RATE, &parm[2]);
	}

	return (temp);
}
#else
int i_play (char *soundfile)
{
  return (-1);
}

int setcard (void)
{
  return (-1);
}

int *cardctl (int fp, ST_CONFIG parm, int st_flag)
{
  return (NULL);
}

#endif