

/****************************************************************************
 * This module is all original code 
 * by Rob Nation 
 * Copyright 1993, Robert Nation
 *     You may use this code for any purpose, as long as the original
 *     copyright remains in the source code and all documentation
 ****************************************************************************/

/***********************************************************************
 *
 * code for parsing the xfwm style command
 *
 ***********************************************************************/
#include "configure.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "xfwm.h"
#include "menus.h"
#include "misc.h"
#include "parse.h"
#include "screen.h"

void
ProcessNewStyle (XEvent * eventp,
		 Window w,
		 XfwmWindow * tmp_win,
		 unsigned long context,
		 char *text,
		 int *Module)
{
  char *name, *line, *restofline;
  char *icon_name = NULL;
  char *forecolor = NULL;
  char *backcolor = NULL;
  unsigned long off_buttons = 0;
  unsigned long on_buttons = 0;

  int desknumber = 0, bw = 0;
  unsigned long off_flags = 0;
  unsigned long on_flags = 0;

  restofline = GetNextToken (text, &name);
  /* in case there was no argument! */
  if ((name == NULL) || (restofline == NULL))
    return;

  off_flags |= MWM_DECOR_FLAG;
  off_flags |= MWM_FUNCTIONS_FLAG;

  while (isspace (*restofline) && (*restofline != 0))
    restofline++;
  line = restofline;

  if (restofline == NULL)
    return;
  while ((*restofline != 0) && (*restofline != '\n'))
    {
      while (isspace (*restofline))
	restofline++;
      switch (tolower (restofline[0]))
	{
	case 'a':
	  if (mystrncasecmp (restofline, "ACTIVEPLACEMENT", 15) == 0)
	    {
	      restofline += 15;
	      on_flags |= RANDOM_PLACE_FLAG;
	    }
	  break;
	case 'b':
	  if (mystrncasecmp (restofline, "BorderWidth", 11) == 0)
	    {
	      restofline += 11;
	      off_flags |= BW_FLAG;
	      sscanf (restofline, "%d", &bw);
	      while (isspace (*restofline))
		restofline++;
	      while ((!isspace (*restofline)) && (*restofline != 0) &&
		     (*restofline != ',') && (*restofline != '\n'))
		restofline++;
	      while (isspace (*restofline))
		restofline++;
	    }
	  break;
	case 'c':
	  break;
	case 'd':
	  if (mystrncasecmp (restofline, "DUMBPLACEMENT", 13) == 0)
	    {
	      restofline += 13;
	      on_flags |= SMART_PLACE_FLAG;
	    }
	  break;
	case 'e':
	  break;
	case 'f':
	  break;
	case 'g':
	  break;
	case 'h':
	  break;
	case 'i':
	  break;
	case 'j':
	  break;
	case 'k':
	  break;
	case 'l':
	  break;
	case 'm':
	  break;
	case 'n':
	  if (mystrncasecmp (restofline, "NOTITLE", 7) == 0)
	    {
	      restofline += 7;
	      off_flags |= NOTITLE_FLAG;
	    }
	  break;
	case 'o':
	  break;
	case 'p':
	  break;
	case 'q':
	  break;
	case 'r':
	  if (mystrncasecmp (restofline, "RANDOMPLACEMENT", 15) == 0)
	    {
	      restofline += 15;
	      off_flags |= RANDOM_PLACE_FLAG;
	    }
	  break;
	case 's':
	  if (mystrncasecmp (restofline, "SMARTPLACEMENT", 14) == 0)
	    {
	      restofline += 14;
	      off_flags |= SMART_PLACE_FLAG;
	    }
	  else if (mystrncasecmp (restofline, "StickyIcon", 10) == 0)
	    {
	      restofline += 10;
	      off_flags |= STICKY_ICON_FLAG;
	    }
	  else if (mystrncasecmp (restofline, "StartIconic", 11) == 0)
	    {
	      restofline += 11;
	      off_flags |= START_ICONIC_FLAG;
	    }
	  else if (mystrncasecmp (restofline, "StaysOnTop", 10) == 0)
	    {
	      restofline += 10;
	      off_flags |= STAYSONTOP_FLAG;
	    }
	  else if (mystrncasecmp (restofline, "Sticky", 6) == 0)
	    {
	      off_flags |= STICKY_FLAG;
	      restofline += 6;
	    }
	  else if (mystrncasecmp (restofline, "STARTSONDESK", 12) == 0)
	    {
	      restofline += 12;
	      off_flags |= STARTSONDESK_FLAG;
	      sscanf (restofline, "%d", &desknumber);
	      while (isspace (*restofline))
		restofline++;
	      while ((!isspace (*restofline)) && (*restofline != 0) &&
		     (*restofline != ',') && (*restofline != '\n'))
		restofline++;
	      while (isspace (*restofline))
		restofline++;
	    }
	  break;
	case 't':
	  if (mystrncasecmp (restofline, "TITLE", 5) == 0)
	    {
	      restofline += 5;
	      on_flags |= NOTITLE_FLAG;
	    }
	  break;
	case 'u':
	  break;
	case 'v':
	  break;
	case 'w':
	  if (mystrncasecmp (restofline, "WindowListSkip", 14) == 0)
	    {
	      restofline += 14;
	      off_flags |= LISTSKIP_FLAG;
	    }
	  else if (mystrncasecmp (restofline, "WindowListHit", 13) == 0)
	    {
	      restofline += 13;
	      on_flags |= LISTSKIP_FLAG;
	    }
	  break;
	case 'x':
	  break;
	case 'y':
	  break;
	case 'z':
	  break;
	default:
	  break;
	}

      while (isspace (*restofline))
	restofline++;
      if (*restofline == ',')
	restofline++;
      else if ((*restofline != 0) && (*restofline != '\n'))
	{
	  xfwm_msg (ERR, "ProcessNewStyle",
		    "bad style command: %s", restofline);
	  return;
	}
    }

  /* capture default icons */
  if (strcmp (name, "*") == 0)
    {
      if (off_flags & ICON_FLAG)
	Scr.DefaultIcon = icon_name;
      off_flags &= ~ICON_FLAG;
      icon_name = NULL;
    }

  AddToList (name, icon_name, off_flags, on_flags, desknumber, bw,
     forecolor, backcolor, off_buttons, on_buttons);
}


void
AddToList (char *name,
	   char *icon_name,
	   unsigned long off_flags,
	   unsigned long on_flags,
	   int desk,
	   int bw,
	   char *forecolor,
	   char *backcolor,
	   unsigned long off_buttons,
	   unsigned long on_buttons)
{
  name_list *nptr, *lastptr = NULL;

  for (nptr = Scr.TheList; nptr != NULL; nptr = nptr->next)
    {
      lastptr = nptr;
    }

  nptr = (name_list *) safemalloc (sizeof (name_list));
  nptr->next = NULL;
  nptr->name = name;
  nptr->on_flags = on_flags;
  nptr->off_flags = off_flags;
  nptr->value = icon_name;
  nptr->Desk = desk;
  nptr->border_width = bw;
  nptr->ForeColor = forecolor;
  nptr->BackColor = backcolor;
  nptr->off_buttons = off_buttons;
  nptr->on_buttons = on_buttons;

  if (lastptr != NULL)
    lastptr->next = nptr;
  else
    Scr.TheList = nptr;
}
