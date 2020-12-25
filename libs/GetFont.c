#include "configure.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "utils.h"

#ifdef DMALLOC
#  include "dmalloc.h"
#endif

/*
 * ** loads font or "fixed" on failure
 */
XFontStruct *
GetFontOrFixed (Display * disp, char *fontname, XFontSet * fontset)
{
  XFontStruct *fnt = NULL;

  *fontset = NULL;
  if (strchr (fontname, ','))
    {				/* FontSet specified */
      char **missing_fontlist = NULL;
      int missing_fontnum = 0;
      char *default_str = NULL;

      *fontset = XCreateFontSet (disp, fontname, &missing_fontlist,
				 &missing_fontnum, &default_str);
      if (!(*fontset) || missing_fontnum > 0)
	{
	  fprintf (stderr,
		   "[GetFontOrFixed]: WARNING -- can't get fontset %s, trying 'fixed'",
		   fontname);
	  /* fixed should always be avail, so try that */
	  if ((fnt = XLoadQueryFont (disp, "fixed")) == NULL)
	    {
	      fprintf (stderr,
		       "[GetFontOrFixed]: ERROR -- can't get font 'fixed'");
	    }
	  if (missing_fontlist)
	    XFreeStringList (missing_fontlist);
	  *fontset = NULL;
	}
    }
  else if ((fnt = XLoadQueryFont (disp, fontname)) == NULL)
    {
      fprintf (stderr,
	       "[GetFontOrFixed]: WARNING -- can't get font %s, trying 'fixed'",
	       fontname);
      /* fixed should always be avail, so try that */
      if ((fnt = XLoadQueryFont (disp, "fixed")) == NULL)
	{
	  fprintf (stderr,
		   "[GetFontOrFixed]: ERROR -- can't get font 'fixed'");
	}
    }
  return fnt;
}
