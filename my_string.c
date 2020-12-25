/*

   ORIGINAL FILE NAME : my_string.c

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



#include <string.h>
#include <ctype.h>
#include "my_string.h"

char buffer[512];

char *
skiphead (char *s)
{
  char *res = s;
  if (res)
    while (((*res == ' ') || (*res == '\t')) && (*res != 0))
      res++;
  return (res);
}

char *
nextl (char *s)
{
  char *res;
  static char *blank = " ";

  if (s)
    strncpy (buffer, s, 512);
  res = strtok (s ? buffer : NULL, "\n");
  return (res ? res : blank);
}

char *
skiptail (char *s)
{
  char *res;
  register int i, j;
  if (s)
    {
      i = 0;
      j = strlen (s);
      res = (char *) (s + (strlen (s) - 1) * sizeof (char));
      while (((*res == ' ') || (*res == '\t') || (*res == '&')) && (i++ < j))
	res--;
      *(++res) = 0;
    }
  return (s);
}

char *
cleanup (char *s)
{
  char *t;
  if (!s)
    return (NULL);
  t = skiphead (s);
  if (strlen (t))
    return (skiptail (t));
  return (t);
}

char 
my_casecmp (char a, char b)
{
  return (((a == b) || (a == toupper (b)) || (toupper (a) == b)));
}

char 
my_strncasecmp (char *a, char *b, int n)
{
  char *p1, *p2;

  p1 = a;
  p2 = b;
  if (!p1 || !p2)
    return (1);
  for (;;)
    {
      if (!n)
	return (0);
      if (!my_casecmp (*p1, *p2))
	return (*p1 - *p2);
      p1++;
      p2++;
      n--;
    }
}
