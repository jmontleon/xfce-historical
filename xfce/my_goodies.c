/*

   ORIGINAL FILE NAME : my_goodies.c

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




#include "forms.h"
#include "my_goodies.h"
#include "my_string.h"

void 
my_alert (const char *s, int n)
{
  char *s1, *s2, *s3;
  if (s)
  {
    s1 = (char *) nextl ((char *) s);
    s2 = (char *) nextl (NULL);
    s3 = (char *) nextl (NULL);
    fl_show_alert (s1, s2, s3, n);
  }
}

int 
my_question (const char *s, int n)
{
  if (s)
  {
#if FL_INCLUDE_VERSION>=84
    return (fl_show_question (s, n));
#else
    char *s1, *s2, *s3;
    s1 = (char *) nextl ((char *) s);
    s2 = (char *) nextl (NULL);
    s3 = (char *) nextl (NULL);
    return (fl_show_question (s1, s2, s3));
#endif
  }
  return (0);
}
