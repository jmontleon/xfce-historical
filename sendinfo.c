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
*/

/* Portion taken from Rob Nation's code of FVWM */

#include <stdio.h>
#include <ctype.h>

void sendinfo(int *fd,char *message,unsigned long window)
{
  int w;

  if(message != NULL)
    {
      write(fd[0],&window, sizeof(unsigned long));
      w=strlen(message);
      write(fd[0],&w,sizeof(int));
      write(fd[0],message,w);

      /* keep going */
      w=1;
      write(fd[0],&w,sizeof(int));
    }
}
