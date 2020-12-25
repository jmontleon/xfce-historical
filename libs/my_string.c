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



#include <string.h>
#include <ctype.h>
#include "my_string.h"

char buffer[512];
char hexnum[] =
{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

char *
skiphead (char *s)
{
  char *res = s;
/*
  if (res)
    while (((!isascii(*res)) || (!isprint(*res)) || (isspace(*res)) || (iscntrl(*res))) && (*res != 0))
      res++;
*/
  /* Made it 8bit clean */
  if (res)
    while (((isspace(*res)) || (iscntrl(*res))) && (*res != 0))
      res++;
  return (res);
}

char *
nextl (char *s)
{
  char *res = NULL;
  static char *blank = " ";

  if (s)
  {
    strncpy (buffer, s, 512);
  }
  res = strtok (s ? buffer : NULL, "\n");
  return (res ? res : blank);
}

char *
skiptail (char *s)
{
  char *res;
  int i, j;
  if (s)
    {
      i = 0;
      j = strlen (s);
      res = (char *) (s + (strlen (s) - 1) * sizeof (char));
/*
      while (((!isascii(*res)) || (!isprint(*res)) || (isspace(*res)) || (iscntrl(*res)) || (*res == '&')) && (i++ < j))
	res--;
*/
      /* Made it 8bit clean */
      while (((isspace(*res)) || (iscntrl(*res)) || (*res == '&')) && (i++ < j))
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
      if (!n) {
	return (0); }
      if (!my_casecmp (*p1, *p2)) {
	return (*p1 - *p2); }
      p1++;
      p2++;
      n--;
    }
}

char *
tohex(char *s, short int n)
{
   if (s)
   {
      s[0] = hexnum[(n / 16)];
      s[1] = hexnum[(n % 16)];
      s[2] = '\0';
   }
   return (s);
}

char *
my_strrchr(char *s, char c)
{
  char *p1, *p2;
  
  if (!s) 
    return (NULL);
  if (!(*s))
    return (s);
  p1 = p2 = s;
  for (;;) {
    if (*p1++ == c)
      p2 = p1;
    if (!(*p1))
      return (p2);
  }
  return (s);
}
  
