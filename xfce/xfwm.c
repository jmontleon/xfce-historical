/*  gxfce
 *  Copyright (C) 1999 Olivier Fourdan (fourdan@csi.com)
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>

#include <glib.h>
#include <gtk/gtk.h>
#include "my_intl.h"
#include "xfwm.h"
#include "my_string.h"
#include "fileutil.h"
#include "configfile.h"
#include "sendinfo.h"
#include "xfce_cb.h"
#include "xfce.h"
#include "xpmext.h"
#include "xfcolor.h"
#include "selects.h"
#include "constant.h"

static guint watchpipe_tmout = -1;

void 
assume_event (unsigned long type, unsigned long *body)
{
  int i;
  switch (type)
    {
    case M_NEW_DESK:
      i = (long) body[0];
      if ((i >= 0) && (i < NBSCREENS))
	update_screen (i);
      break;
    default:
      break;
    }
}

void 
deadpipe (int fd, void *d)
{
  fprintf(stderr, _("XFCE: Communication pipe closed. Exiting...\n"));
  gtk_main_quit();
  free_selects();
  exit (0);
}

static gint
watchpipe (gpointer d)
{
  unsigned long header[HEADER_SIZE];
  unsigned long *body;
  /*
      Is XFwm still alive ???
   */
  sendinfo (fd, NOP_CMD, 0);
  /*
      And now, the real stuff...
   */
  if (readpacket (fd[1], header, &body) > 0)
    {
      assume_event (header[1], body);
      free (body);
    }

  return TRUE;
}

int 
initwm (int argc, char *argv[])
{
  if (argc < 6)
    {
      return (0);
    }

  /*
     Load pipe 
   */
  fd[0] = atoi (argv[1]);
  fd[1] = atoi (argv[2]);
  fcntl (fd[1], F_SETFL, O_NDELAY);
  /*
     if -nowm is defined, don't use XFwm interface
   */
  if (argc > 7)
    if (!my_strncasecmp(argv[7], "-nowm", strlen("-nowm")))
      return (FVWM);

  /*
     Are we using XFwm or another FVWM's compatible Window Manager ?
   */
  if (argc > 6)
    if (!my_strncasecmp(argv[6], "XFwm", strlen("XFwm")))
      return (XFWM);

  return (FVWM);
}

void 
switch_to_screen (XFCE_palette *pal, int no_screen)
{
  char command[256];
  if (no_screen != get_current_screen()) {
    if (current_config.colorize_root)
      ApplyRootColor (pal, (current_config.gradient_root != 0), get_screen_color(no_screen));
    if (current_config.wm)
      {
	sprintf (command, DESK_CMD, no_screen);
	sendinfo (fd, command, 0);
      }
  }
}

void
quit_wm(void)
{
   if (current_config.wm) 
     sendinfo (fd, QUIT_CMD, 0);
}

void
init_watchpipe(void)
{
  if (current_config.wm) 
    watchpipe_tmout = gtk_timeout_add(50, (GtkFunction)watchpipe, 0);
}

void
stop_watchpipe (void)
{
  if (watchpipe_tmout >= 0)
    gtk_timeout_remove(watchpipe_tmout);
}

void 
apply_wm_colors(const XFCE_palette *p)
{
  char *s, *t, *u, *v;
  int howbright, howdark;


  if ((p) && (current_config.wm == XFWM))
  {
     s=(char *) malloc (10  * sizeof (char));
     t=(char *) malloc (10  * sizeof (char));
     u=(char *) malloc (255 * sizeof (char));
     v=(char *) malloc (10  * sizeof (char));
    
     howbright = brightness_pal (p, 6);
     howdark   = brightness_pal (p, 3);
     color_to_hex(s, p, 7);
     sprintf(u, ACTIVECOLOR_CMD, (howbright > bright) ? "black" : "white", s);
     sendinfo(fd, u, 0);
     sprintf(u, INACTIVECOLOR_CMD, (howdark < dark) ? "white" : "black", s);
     sendinfo(fd, u, 0);

     howbright = brightness_pal (p, 2);
     howdark   = brightness_pal (p, 7);
     color_to_hex(t, p, 2);
     sprintf(u, MENUCOLOR_CMD, (howdark < dark)     ? "white" : "black", s,
                               (howbright > bright) ? "black" : "white", t);
     sendinfo(fd, u, 0);

     color_to_hex(t, p, 6);
     color_to_hex(v, p, 5);
     if ((DEFAULT_DEPTH > 8) && (current_config.gradient_active_title))
       sprintf(u, TITLESTYLE_CMD, ACTIVE, GRADIENT, t, v);
     else
       sprintf(u, TITLESTYLE_CMD, ACTIVE, SOLID, t, "");
     sendinfo(fd, u, 0);

     color_to_hex(t, p, 3);
     if ((DEFAULT_DEPTH > 8) && (current_config.gradient_inactive_title))
       sprintf(u, TITLESTYLE_CMD, INACTIVE, GRADIENT, t, s);
     else
       sprintf(u, TITLESTYLE_CMD, INACTIVE, SOLID, t, "");
     sendinfo(fd, u, 0);

     color_to_hex(t, p, 0);
     sprintf(u, CURSORCOLOR_CMD, t, "white");
     sendinfo(fd, u, 0);

     sendinfo(fd, REFRESH_CMD, 0);
     free(s);
     free(t);
     free(u);
     free(v);
   }
}

void
apply_wm_fonts(void)
{
  char *s;
  if(current_config.wm == XFWM)
  {
    s = (char *) malloc (sizeof (char) * MAXSTRLEN);
    if ((current_config.fonts[0]) && strlen(current_config.fonts[0]))
      sprintf(s, WINDOWFONT_CMD, current_config.fonts[0]);
    else
      sprintf(s, WINDOWFONT_CMD, XFWM_TITLEFONT);
    sendinfo(fd, s, 0);
    if ((current_config.fonts[1]) && strlen(current_config.fonts[1]))
      sprintf(s, MENUFONT_CMD, current_config.fonts[1]);
    else
      sprintf(s, MENUFONT_CMD, XFWM_MENUFONT);
    sendinfo(fd, s, 0);
    if ((current_config.fonts[2]) && strlen(current_config.fonts[2]))
      sprintf(s, ICONFONT_CMD, current_config.fonts[2]);
    else
      sprintf(s, ICONFONT_CMD, XFWM_ICONFONT);
    sendinfo(fd, s, 0);
    free(s);
  }
}

void
apply_wm_iconpos(void)
{
  char *s;
  if(current_config.wm == XFWM)
  {
    s = (char *) malloc (sizeof (char) * 255);
    sprintf(s, ICONPOS_CMD, current_config.iconpos);
    sendinfo(fd, s, 0);
    sprintf(s , ARRANGEICONS_CMD, 0, NBSCREENS - 1);
    sendinfo(fd, s, 0);
    free(s);
  }
}

void
apply_wm_options(void)
{
  if(current_config.wm == XFWM)
  {
     sendinfo(fd, (current_config.clicktofocus ? FOCUSCLICK_CMD : FOCUSMOUSE_CMD) , 0);
     sendinfo(fd, (current_config.opaquemove ? OPAQUEMOVEON_CMD : OPAQUEMOVEOFF_CMD), 0);
     sendinfo(fd, (current_config.autoraise ? AUTORAISEON_CMD : AUTORAISEOFF_CMD), 0);
  }
}
